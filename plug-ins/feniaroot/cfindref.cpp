#include "profiler.h"
#include "phase.h"
#include "function.h"

#include "characterwrapper.h"
#include "wrappersplugin.h"
#include "wrappermanager.h"
#include "sysdep.h"
#include "structs.h"
#include "../runtime/utils.h"
#include "comm.h"

#include <set>

using Scripting::CodeSource;

typedef Scripting::Object::id_t objid_t;
typedef Scripting::Function::id_t funid_t;
typedef Scripting::CodeSource::id_t csid_t;

class DepsBuilder : public Scripting::DereferenceListener
{
public:

    virtual void notify(Scripting::Function *f) {
        cs_deps[f->source.source->getId()][f->getId()].push_back(cur_id);
    }
    virtual void notify(Scripting::Object *o) {
        obj_deps[o->getId()].push_back(cur_id);
    }

    void run() {
        Scripting::gc = false;
        instance = this;

        for(Scripting::Object::Manager::iterator i = Scripting::Object::manager->begin();i != Scripting::Object::manager->end();i++) {
            // destroy a single handler and check how it affected the counters on other objects
            cur_id = i->getId();

            i->backup();
        }

        instance = 0;
        Scripting::Object::manager->recover();
        
        WrappersPlugin::linkTargets(); // TODO FIXME what if there are more wrapper plugins out there?

        Scripting::gc = true;
    }

    objid_t cur_id;

    map<csid_t, map<funid_t, vector<objid_t> > > cs_deps;
    map<objid_t, vector<objid_t> > obj_deps;
};

void cmd_findrefs(struct char_data *ch, DLString &args)
{
    if (args.empty( )) {
	send_to_char("���������: {Wfindrefs {x<cs id>\r\n", ch);
	return;
    }

    try {
        Profiler prof;

        prof.start();

        DepsBuilder builder;
        builder.run();

        prof.stop();

        ostringstream os;

        os << "Deps tree was built in " << prof.msec() << " ms." << endl;

        CodeSource::Manager::iterator it;

        try {
            it = CodeSource::manager->find( args.toInt( ) );
        } catch( ... ) {
            return;
        }

        if(it == CodeSource::manager->end( ) ) {
            send_to_char("No such CodeSource\r\n", ch);
            return;
        }

        CodeSource &cs = *it;

        const map<funid_t, vector<objid_t> > csdeps = builder.cs_deps[cs.getId()];

        std::set<objid_t> roots;

        for(map<funid_t, vector<objid_t> >::const_iterator i=csdeps.begin();i != csdeps.end();i++) {
            roots.insert(i->second.begin(), i->second.end());
        }

        std::set<objid_t> all = roots, next;

        do {
            next.clear();

            for(std::set<objid_t>::iterator i=all.begin();i != all.end();i++) {
                const vector<objid_t> &deps = builder.obj_deps[*i];

                for(std::vector<objid_t>::const_iterator j=deps.begin();j != deps.end();j++)
                    if(all.find(*j) == all.end())
                        next.insert(*j);
            }

            all.insert(next.begin(), next.end());

        } while(next.size() != 0);

        for(std::set<objid_t>::iterator i=all.begin();i != all.end();i++) {
            Scripting::Object &obj = Scripting::Object::manager->at(*i);
            os << Register(&obj).repr();
            WrapperBase *wrap = obj.hasHandler() ? obj.getHandler().getDynamicPointer<WrapperBase>() : NULL;
            if(wrap) {
                os << ", ID=" << wrap->getID();
            }
            os << endl;
        }

        page_string(ch->desc, os.str().c_str(), 1);

    } catch (const ::Exception &e) {
	send_to_char(e.what(), ch);
    }
}


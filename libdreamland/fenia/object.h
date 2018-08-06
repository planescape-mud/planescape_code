/* $Id: object.h,v 1.1.2.5.6.2 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/* $Id: object.h,v 1.1.2.5.6.2 2009/10/11 18:35:36 rufina Exp $
 * 
 * unicorn, Forgotten Dungeon, 2004
 */

#ifndef __OBJECT_H__
#define __OBJECT_H__

#include <ext/rb_tree>
#include <sys/time.h>

using namespace __gnu_cxx;

#include <xmlpointer.h>
#include <xmlcontainer.h>
// MOC_SKIP_BEGIN
#include "exceptions.h"
// MOC_SKIP_END
#include "handler.h"
#include "txncontext.h"
#include "manager-decl.h"

#include "phase.h"

#define OBJID_ROOT 1

class Timer;

namespace Scripting {

class Object : public XMLVariable {
public:
    typedef unsigned long int id_t;
    struct selectId : public unary_function<Object, id_t> {
	const id_t &operator () (const Object &o) {
	    return o.getId();
	}
    };
    typedef rb_tree<id_t, Object, selectId, less<id_t> > Map;
    class Manager;

    struct NotRecoveredException : public Exception {
	NotRecoveredException() : Exception("Object not recovered") { }

	virtual ~NotRecoveredException( ) throw( );
    };

    Object();
    Object(id_t i);

    void changed();
    
    void setHandler(Handler::Pointer h) {
	handler = h;
	if(handler)
	    handler->setSelf(this);
    }
    Handler::Pointer getHandler() {
	if(handler)
	    return handler.getPointer();

	throw NotRecoveredException();
    }
    bool hasHandler() {
	return !!handler;
    }
    
    const id_t &getId() const {
	return id;
    }

    virtual void fromXML( const XMLNode::Pointer& node ) throw( ExceptionBadType );
    virtual bool toXML( XMLNode::Pointer& node ) const;

    void backup();
    void recover();

    void save();

    /* refcnt GC */
    int refcnt;
    
    void link() {
	refcnt++;
    }
    void unlink() {
	refcnt--;
	
	if(refcnt <= 0 && handler && Scripting::gc) 
	    finalize();
    }

    void finalize();

    static Manager *manager;

private:
    void toList(Object &);
    void fromList();
    
    Object *prev, *next;
    bool justCreated;
    id_t id;
    
    XMLPointer<Handler> handler;
    XMLNode::Pointer backupNode;

public:
    bool dynamicHandler;
};

class Object::Manager : public BaseManager<Object>, public DbContext {
public:
    Manager();
    virtual ~Manager();

    void open( );
    void close( );

    void put( id_t id, const string &s );

    virtual void seq( id_t, Data & );

    bool tlim(const Timer *);
    bool syncPut(const Timer *);
    bool syncDel(const Timer *);
    bool sync(const Timer *);

    void backup();
    void recover();

    Object & allocate();

    Object changed, deleted;
};

}

#endif

#ifndef PLUGINNATIVE_H
#define PLUGINNATIVE_H

#include <sstream>

#include "native.h"
#include "codesource.h"
#include "wrapperbase.h"
#include "schedulerwrapper.h"
#include "logstream.h"

using namespace Scripting;

#ifdef _never_defined_parsed_only_by_moc_
using namespace Scripting;

template <typename T>
class PluginNativeImpl : public NativeImpl<T> { }

template <typename T>
class PluginWrapperImpl : public PluginNativeImpl<T>, public WrapperBase { }
#endif

extern void fenia_wiznet(const CodeSource::Pointer &codeSource, const DLString &key, const DLString &what);

// MOC_SKIP_BEGIN
template <typename T>
class PluginNativeImpl : public NativeImpl<T>
{
public:
    typedef NativeImpl<T> Super;

    virtual bool setNativeField(const Register &key, const Register &val) {
        PlugLock lock;
        return Super::setNativeField(key, val);
    }

    virtual bool getNativeField(const Register &key, Register &retval) {
        PlugLock lock;
        return Super::getNativeField(key, retval);
    }

    virtual bool callNativeMethod(const Register &key, const RegisterList &args, Register &retval) {
        PlugLock lock;
        return Super::callNativeMethod(key, args, retval);
    }
};


template <typename T>
class PluginWrapperImpl : public PluginNativeImpl<T>, 
                          public WrapperBase 
{                        
public:
    typedef PluginWrapperImpl<T> GutsContainer;

    void traitsAPI( ostringstream& buf ) const {
        Guts::const_iterator i;
        
        buf << endl << endl << "&WRuntime fields:&x" << endl;
        for (i = guts.begin(); i != guts.end(); i++)
            buf << "&x" << Lex::getThis()->getName(i->first) << "&x" << endl;
    }

    virtual void croak(const Register &key, const ::Exception &e) const {
        Register prog;
    
        // Complain about exception to every immortal in the game.    
        if (triggerFunction(key, prog)) {
            const CodeSource::Pointer &codeSource = prog.toFunction()->source.source;
            fenia_wiznet(codeSource, key.toString(), e.what());
        }
        
        // By default just output exception to system logs.
        WrapperBase::croak(key, e);
    }
};
// MOC_SKIP_END

#endif

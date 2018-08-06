/* $Id: root.h,v 1.1.4.8.6.2 2007/09/21 21:23:53 margo Exp $
 *
 * ruffina, 2004
 */

#ifndef __ROOT_H__
#define __ROOT_H__

#include <xmlvariablecontainer.h>

// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "exceptions.h"
// MOC_SKIP_END
#include "xmlregister.h"
#include "pluginnativeimpl.h"

using Scripting::XMLRegister;
using Scripting::Register;
using Scripting::RegisterList;

class Root : public PluginNativeImpl<Root>, 
             public NativeHandler, 
             public XMLVariableContainer 
{
XML_OBJECT
NMI_OBJECT
public:
    
    Root() { }

    virtual void setSelf(Scripting::Object *s) {
        self = s;
    }
   
    XML_VARIABLE XMLRegister tmp, scheduler;
private:
    Scripting::Object *self;
};

#endif /* __ROOT_H__ */

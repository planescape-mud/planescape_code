/* $Id: objectwrapper.h,v 1.1.4.10.6.1 2007/09/11 00:09:28 margo Exp $
 *
 * ruffina, 2004
 */

#ifndef _OBJECTWRAPPER_H_
#define _OBJECTWRAPPER_H_

// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "xmlregister.h"
#include "exceptions.h"
// MOC_SKIP_END
#include "pluginnativeimpl.h"

// MOC_SKIP_BEGIN
struct obj_data;
// MOC_SKIP_END

using Scripting::Register;
using Scripting::RegisterList;

class ObjectWrapper : public PluginWrapperImpl<ObjectWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<ObjectWrapper> Pointer;

    ObjectWrapper( );

    virtual void setSelf( Scripting::Object * );
    virtual void extract( bool );
    void setTarget( struct obj_data * );
    void checkTarget( ) const throw( Scripting::Exception );
    struct obj_data * getTarget() const;
    
private:
    struct obj_data *target;
};

#endif 

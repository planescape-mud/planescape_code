/* $Id: objectwrapper.cpp,v 1.1.4.33.6.14 2009/02/15 01:42:54 margo Exp $
 *
 * ruffina, 2004
 */

#include <iostream>

#include "logstream.h"

#include "roomwrapper.h"
#include "objectwrapper.h"
#include "characterwrapper.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "nativeext.h"

#include "wrap_utils.h"
#include "subr.h"

#include "structs.h"
#include "db.h"
#include "comm.h"

using namespace std;

NMI_INIT(ObjectWrapper, "предмет")


ObjectWrapper::ObjectWrapper( ) : target( NULL )
{
    
}

void ObjectWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void ObjectWrapper::extract( bool count )
{
    if (target) {
	target->wrapper = 0;
	target = 0;
    } else {
	LogStream::sendError() << "Object wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void ObjectWrapper::setTarget( struct obj_data *target )
{
    this->target = target;
    id = target->guid;
}

struct obj_data * ObjectWrapper::getTarget() const
{
    checkTarget();
    return target;
}

void ObjectWrapper::checkTarget( ) const throw( Scripting::Exception )
{
    if (zombie.getValue())
	throw Scripting::Exception( "Non existent object referenced" );

    if (target == NULL) 
	throw Scripting::Exception( "Object is offline" );
}

NMI_GET( ObjectWrapper, online, "" )
{
    return Register( target != NULL );
}

NMI_GET( ObjectWrapper, dead, "" )
{
    return Register( zombie.getValue() );
}

#define GETWRAP(x) NMI_GET(ObjectWrapper, x, "") { \
    checkTarget(); \
    return WrapperManager::getThis( )->getWrapper(target->x); \
}

/*
 * FIELDS
 */

NMI_GET( ObjectWrapper, description, "описание предмета на полу")
{
    checkTarget( );
    return Register( target->description );
}
/*
 * Methods
 */
NMI_INVOKE( ObjectWrapper, api, "печатает этот API" )
{   
    ostringstream buf;
    Scripting::traitsAPI<ObjectWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( ObjectWrapper, rtapi, "печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( ObjectWrapper, clear, "очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}


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
#include "../runtime/handler.h"
#include "db.h"
#include "utils.h"
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

// Remove object from room or char or another obj,
// before calling obj_to_* functions.
static void obj_from_anywhere( struct obj_data *obj )
{
    if (obj->in_room != NOWHERE)
        obj_from_room(obj);
    else if (obj->carried_by != NULL)
        obj_from_char(obj);
    else if (obj->in_obj != NULL)
        obj_from_obj(obj);
}

NMI_INVOKE( ObjectWrapper, obj_to_char , "дать предмет в руки чару, переданному в параметрах")
{
    CharacterWrapper *chWrap;

    checkTarget();
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    chWrap = wrapper_cast<CharacterWrapper>(args.front( ));

    obj_from_anywhere( target );
    ::obj_to_char( target, chWrap->getTarget( ) );
    return Register( );
}

NMI_INVOKE( ObjectWrapper, obj_to_room, "положить предмет в комнату, переданную в параметрах")
{
    RoomWrapper *roomWrap;

    checkTarget();
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    roomWrap = wrapper_cast<RoomWrapper>(args.front( ));

    obj_from_anywhere( target );
    ::obj_to_room( target, roomWrap->getTarget( ) );
    return Register( );
}

NMI_INVOKE( ObjectWrapper, obj_to_obj, "положить предмет в контейнер, переданный в параметрах")
{
    ObjectWrapper *objWrap;

    checkTarget();
    if (args.empty( ))
        throw Scripting::NotEnoughArgumentsException( );

    objWrap = wrapper_cast<ObjectWrapper>(args.front( ));

    obj_from_anywhere( target );
    ::obj_to_obj( target, objWrap->getTarget( ) );
    return Register( );
}

NMI_INVOKE( ObjectWrapper, create, "создать предмет с этим прототипом")
{
    checkTarget();
    
    obj_data *obj = read_object(target->item_number, REAL, TRUE);
    return wrap(obj);
}

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


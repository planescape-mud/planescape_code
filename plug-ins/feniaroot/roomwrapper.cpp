/* $Id: roomwrapper.cpp,v 1.1.4.21.6.18 2009/03/04 09:18:56 rufina Exp $
 *
 * ruffina, 2004
 */

#include <sys/time.h>
#include <iostream>

#include "logstream.h"

#include "objectwrapper.h"
#include "roomwrapper.h"
#include "characterwrapper.h"
#include "wrappermanager.h"
#include "nativeext.h"
#include "reglist.h"
#include "wrap_utils.h"
#include "subr.h"

#include "structs.h"
#include "db.h"
#include "comm.h"

using namespace std;

NMI_INIT(RoomWrapper, "комната")

RoomWrapper::RoomWrapper( ) : target( NULL )
{
}

void RoomWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );

    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void RoomWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        LogStream::sendError() << "Room wrapper: extract without target" << endl;
    }

    GutsContainer::extract( count );
}

void RoomWrapper::setTarget( struct room_data *r )
{
    target = r;
    id = r->guid;
}

void RoomWrapper::checkTarget( ) const throw( Scripting::Exception )
{
    if (zombie.getValue())
	throw Scripting::Exception( "Room is dead" );
	
    if (target == NULL)
	throw Scripting::Exception( "Room is offline" );
}

struct room_data * RoomWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

/*
 * FIELDS
 */

#define GETWRAP(x) NMI_GET(RoomWrapper, x, "") { \
    checkTarget(); \
    return WrapperManager::getThis( )->getWrapper(target->x); \
}

NMI_GET( RoomWrapper, name , "название комнаты")
{
    checkTarget();
    return Register(target->name);
}

NMI_GET( RoomWrapper, vnum, "виртуальный номер комнаты")
{
    checkTarget();
    return Register(target->number);
}

/*
 * METHODS
 */
NMI_INVOKE( RoomWrapper, api, "печатает этот API" )
{   
    ostringstream buf;
    Scripting::traitsAPI<RoomWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( RoomWrapper, rtapi, "печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( RoomWrapper, clear, "очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}

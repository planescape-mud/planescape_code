/* $Id: wrappermanager.cpp,v 1.1.2.5.6.1 2007/09/21 21:23:53 margo Exp $
 *
 * ruffina, 2004
 */
#include "wrappermanager.h"
#include "characterwrapper.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "subr.h"

#include "fenia/register-impl.h"

#include "class.h"
#include "structs.h"
#include "db.h"
#include "comm.h"

void WrapperManager::initialization( )
{
    FeniaManager::wrapperManager.setPointer( this );
}

void WrapperManager::destruction( )
{
    FeniaManager::wrapperManager.clear( );
}

WrapperManager * WrapperManager::getThis( )
{
    return static_cast<WrapperManager *>( FeniaManager::wrapperManager.getPointer( ) );
}
/*
 * wrapping methods
 */
Scripting::Register WrapperManager::getWrapper( struct char_data *ch )
{
    if (!ch)
        return Scripting::Register( );

    return wrapperAux<CharacterWrapper>( ch->guid, ch ); 
}

Scripting::Register WrapperManager::getWrapper( struct obj_data *obj )
{
    if (!obj)
        return Scripting::Register( );

    return wrapperAux<ObjectWrapper>( obj->guid, obj ); 
}

Scripting::Register WrapperManager::getWrapper( struct room_data *room )
{
    if (!room)
        return Scripting::Register( );

    return wrapperAux<RoomWrapper>( room->guid, room ); 
}

template <typename WrapperType, typename TargetType>
Scripting::Register WrapperManager::wrapperAux( long long id, TargetType t )
{
    if (!t->wrapper) {
        typename WrapperType::Pointer wrapper( NEW );

        wrapper->setTarget( t );
        t->wrapper = &Scripting::Object::manager->allocate( );
        t->wrapper->setHandler( wrapper );
    }

    return Scripting::Register( t->wrapper );
}

/*
 * link methods
 */

void WrapperManager::linkWrapper( struct char_data * ch )
{
    linkAux<CharacterWrapper>( ch->guid, ch );
}

void WrapperManager::linkWrapper( struct obj_data * obj )
{
    linkAux<ObjectWrapper>( obj->guid, obj );
}

void WrapperManager::linkWrapper( struct room_data * room )
{
    linkAux<RoomWrapper>( room->guid, room );
}

void WrapperManager::getTarget( const Scripting::Register &reg, struct char_data *& ch )
{
    ch = wrapper_cast<CharacterWrapper>( reg )->getTarget( );
}

void WrapperManager::getTarget( const Scripting::Register &reg, struct obj_data *& obj )
{
    obj = wrapper_cast<ObjectWrapper>( reg )->getTarget( );
}

void WrapperManager::getTarget( const Scripting::Register &reg, struct room_data *& room )
{
    room = wrapper_cast<RoomWrapper>( reg )->getTarget( );
}

template <typename WrapperType, typename TargetType>
void WrapperManager::linkAux( long long id, TargetType t )
{
    WrapperMap::iterator i;
    
    i = map.find( id );

    if (i == map.end( ))
        return;

    t->wrapper = i->second;
    wrapper_cast<WrapperType>(t->wrapper)->setTarget( t );
}

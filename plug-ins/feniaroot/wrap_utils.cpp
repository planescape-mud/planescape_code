/* $Id$
 *
 * ruffina, 2009
 */
#include "wrap_utils.h"

#include "structs.h"
#include "db.h"
#include "comm.h"

#include "characterwrapper.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "wrappermanager.h"

#include "subr.h"

Register wrap( struct char_data * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}
Register wrap( struct obj_data * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}
Register wrap( struct room_data * wt )
{
    return WrapperManager::getThis( )->getWrapper( wt ); 
}
const Register & get_unique_arg( const RegisterList &args )
{
    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );

    return args.front( );
}

int args2number( const RegisterList &args )
{
    return get_unique_arg( args ).toNumber( );
}

DLString args2string( const RegisterList &args )
{
    return get_unique_arg( args ).toString( );
}

struct char_data * arg2character( const Register &reg )
{
    return wrapper_cast<CharacterWrapper>( reg )->getTarget( );
}

struct room_data * arg2room( const Register &reg )
{
    return wrapper_cast<RoomWrapper>( reg )->getTarget( );
}

struct obj_data * arg2item( const Register &reg )
{
    return wrapper_cast<ObjectWrapper>( reg )->getTarget( );
}

void args2buf(const RegisterList &args, char *buf, size_t bufsize)
{
    strncpy(buf, args2string(args).c_str(), bufsize);
    buf[bufsize - 1] = 0;
}

const Register & arg_one( const RegisterList &args )
{
    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );

    return args.front( );
}

const Register & arg_two( const RegisterList &args )
{
    if (args.size( ) < 2)
       throw Scripting::NotEnoughArgumentsException( );

    RegisterList::const_iterator iter = args.begin( );
    iter++;
    return *iter;
}


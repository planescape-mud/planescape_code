/* $Id: regcontainerext.cpp,v 1.1.2.4.6.3 2008/01/09 12:09:25 rufina Exp $
 *
 * ruffina, 2004
 */
#include <sstream>

#include "regcontainer.h"
#include "nativeext.h"
#include "fenia/object.h"
#include "register-impl.h"
#include "reglist.h"

using namespace Scripting;

NMI_GET(RegContainer, keys, "список ключей") 
{
    RegList::Pointer rc(NEW);

    Map::const_iterator i;
    
    for(i = map.begin(); i != map.end(); i++)
        rc->push_back( i->first );
    
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_GET(RegContainer, values, "список значений") 
{
    RegList::Pointer rc(NEW);

    Map::const_iterator i;
    
    for(i = map.begin(); i != map.end(); i++)
        rc->push_back( i->second );
    
    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}

NMI_INVOKE(RegContainer, size, "") 
{
    return Register( (int)map.size( ) );
}

NMI_INVOKE(RegContainer, api, "") 
{
    ostringstream buf;

    traitsAPI<RegContainer>( buf );

    return Register( buf.str( ) );
}

NMI_INVOKE( RegContainer, clear, "очистка всех runtime полей" )
{
    map.clear( );
    self->changed( );
    return Register( );
}

NMI_INVOKE( RegContainer, clone , "")
{
    ::Pointer<RegContainer> rc(NEW);
    Map::const_iterator i;
    
    for(i = map.begin(); i != map.end(); i++)
        (*rc)->map[i->first] = i->second;

    Scripting::Object *obj = &Scripting::Object::manager->allocate();
    obj->setHandler(rc);

    return Register( obj );
}


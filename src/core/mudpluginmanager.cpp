/* $Id$
 *
 * ruffina, 2009
 */
#include "mudpluginmanager.h"
#include "planescape.h"

DLString MudPluginManager::getTablePath( ) const
{
    return mud->getLibexecDir( ).getPath( ); 
}


/* $Id$
 *
 * ruffina, 2009
 */
#ifndef MUDPLUGINMANAGER_H
#define MUDPLUGINMANAGER_H

#include "pluginmanager.h"

/*
 * MudPluginManager: a plugin manager specific for this mud
 */
class MudPluginManager : public PluginManager {
public:
    virtual DLString getTablePath( ) const;
};

#endif

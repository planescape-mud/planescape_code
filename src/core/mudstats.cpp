/* $Id$
 *
 * ruffina, 2009
 */
#include "logstream.h"
#include "mudstats.h"
#include "planescape.h"
#include "structs.h"
#include "comm-decl.h"

MudStats::MudStats()
		: lastID(1)
{
    checkDuplicate(mud->getStats());
}

MudStats::~MudStats()
{
}

DLString MudStats::getConfigDirPath() const
{
    return mud->getDbDir().getPath();
}

bool MudStats::load()
{
    if (!getConfigFile().exist())
        LogStream::sendWarning() 
            << "PlaneScape runtime data doesn't exist, will create new" << endl;
    else if (!loadConfig())
        return false;

    if (!mud->modCopyOver)
        high = 0;

    bootVersion++;

    if (!saveConfig())
        return false;

    return true;
}

bool MudStats::save()
{
    return saveConfig();
}

void MudStats::update()
{
    struct descriptor_data *d;

    players = 0;

    for (d = descriptor_list; d; d = d->next) {
        if (d->connected != CON_PLAYING)
            continue;
        if (!d->character)
            continue;
        if (d->character->immortal())
            continue;

        players++;
    }

    if (players > high) {
        high = players;
        time_high = time(0);
    }

    if (high > peak) {
        peak = high;
        time_peak = time(0);
    }

    save();
}

long long MudStats::genID()
{
    long long ver = ((long long)bootVersion) << 32;
    lastID++;
    return ver + lastID;
}


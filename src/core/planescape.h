/* $Id$
 *
 * ruffina, 2009
 */
#ifndef PLANESCAPE_H
#define PLANESCAPE_H

#include "oneallocate.h"
#include "xmlvariablecontainer.h"
#include "xmloptions.h"
#include "xmlconfigurable.h"
#include "dldirectory.h"
#include "timer.h"

class MudPluginManager;
class MudScheduler;
class DbEnvContext;
class FeniaManager;
class ProcessManager;
class MudStats;
class TextFileLoader;

/*
 * PlaneScape
 */
class PlaneScape : public OneAllocate, 
                   public XMLConfigurableWithPath,
                   public XMLVariableContainer 
{
XML_OBJECT 
public:
    PlaneScape();
    virtual ~ PlaneScape();

    void load();
    void startup();
    void loop();
    void shutdown();
    void save();

    bool isShutdown() const;

    inline DbEnvContext *getDbEnv() const;
    inline MudScheduler *getScheduler();
    inline MudPluginManager *getPluginManager();
    inline ProcessManager *getProcessManager();
    inline MudStats *getStats();
    inline TextFileLoader *getTextFileLoader();

    inline const DLDirectory & getBaseDir() const;
    inline DLDirectory getShareDir() const;
    inline DLDirectory getLibexecDir() const;
    inline DLDirectory getDbDir() const;
    inline DLDirectory getTextDir() const;

    inline int getPulsePerSecond() const;
    inline int getMissedPulses() const;

public:
    // options stored in config file, can be overriden by commandline options 
                 XMLBooleanOption<false> modCopyOver;   /* are we booting in copyover mode? */
    XML_VARIABLE XMLBooleanOption<false> modMini;       /* mini-mud mode */
    XML_VARIABLE XMLBooleanOption<true>  modQuietLoad;  /* no extra boot messages */
    XML_VARIABLE XMLIntegerOption<0>     modRestrict;   /* min level allowed to enter the game */
    XML_VARIABLE XMLBooleanOption<false> modNoSpecials; /* suppress assignment of special routines */
    XML_VARIABLE XMLStringOption         shareDir;      /* dir used for data loading */
    XML_VARIABLE XMLStringOption         logPattern;    /* log file name (with date patterns) */
    XML_VARIABLE XMLIntegerOption<0>     port;          /* game server port */
   
    // paths stored in config file, non-overrideable
    XML_VARIABLE XMLString killScriptFile, pauseFile;

    XML_VARIABLE XMLString playerDir, playerAliasDir, playerObjsDir, playerVarsDir, playerTextDir;
    XML_VARIABLE XMLString playerIndexFile;

    XML_VARIABLE XMLString ideaFile, typosFile, bugsFile;

    XML_VARIABLE XMLString mailSpoolDir, boardsDir, mailFile;
    XML_VARIABLE XMLString adminDir, banFile, proxyFile;
    XML_VARIABLE XMLString xnameFile, znameFile;
    XML_VARIABLE XMLString shopFile;
    
    XML_VARIABLE XMLString indexFile, indexFileMini;
    // all paths relative to share dir
    XML_VARIABLE XMLString wldDir, mobDir, objDir, zonDir, scrDir;
    
    // relative to share dir
    XML_VARIABLE XMLString helpDir;

    // all paths relative to share dir
    XML_VARIABLE XMLString messFile, socialsFile, adverbFile;
    XML_VARIABLE XMLString cursesFile;
    XML_VARIABLE XMLString spellsFile, xskillsFile, skillsFile, bodyFile, classesFile;
    XML_VARIABLE XMLString armorTemplFile, weaponTemplFile, missTemplFile;
    XML_VARIABLE XMLString setsFile, hitMessageFile, enchantFile, worldLootFile;
    XML_VARIABLE XMLString xformatFile, materialsFile;
    XML_VARIABLE XMLString horseShopFile;

    
    // global heartbeat pulses counter, heartbeat functions must 
    // take care to increment and rollover the counter
    int pulse;

    DLString execFileName;

protected:
    // number of game pulses per second
    XML_VARIABLE XMLInteger pulsePerSecond;
    // max seconds allowed to be missed 
    XML_VARIABLE XMLInteger maxMissedSeconds;

    // starting dir for all relative path calculations
    XML_VARIABLE XMLString basePath;
    XML_VARIABLE XMLBoolean baseAbsolute;
    DLDirectory baseDir;

    XML_VARIABLE XMLString feniaDbDir;
    XML_VARIABLE XMLString feniaScriptDir;

    // directory with plugin configuration and pluggable modules 
    XML_VARIABLE XMLString libexecDir;

    XML_VARIABLE XMLString dbDir;
    // relative to share dir
    XML_VARIABLE XMLString textDir;

    // instance of plugin manager 
    ::Pointer<MudPluginManager> pluginManager;

    // instance of game scheduler
    ::Pointer<MudScheduler> scheduler;

    ::Pointer<FeniaManager> feniaManager;
    ::Pointer<ProcessManager> processManager;

    ::Pointer<MudStats> stats;
    ::Pointer<TextFileLoader> textFileLoader;

    DbEnvContext *dbEnv;
    
private:
    static void signalReloadAllPlugins(int signo);
    static void signalReloadChangedPlugins(int signo);
    void setSignals();
    
    // finish time of previous iteration 
    Timer last_time;
    void setLastTime();
    
    // how long it took to process previous iteration
    Timer process_time;

    // pulses missed during previous iteration
    int missed_pulses;
    void countMissedPulses();

    // how many passes per second we do
    Timer opt_time;
    void setOptTime();

    // sleep if no connections
    void waitUp();

    // sleep to resynchronize with the next pulse
    void sleepUp();
};

/*
 * the one and only instance
 */
extern PlaneScape *mud;

/*
 * inlines
 */
inline DbEnvContext *PlaneScape::getDbEnv() const
{
    return dbEnv;
}
inline const DLDirectory & PlaneScape::getBaseDir() const
{
    return baseDir;
}
inline DLDirectory PlaneScape::getShareDir() const
{
    return DLDirectory(baseDir, shareDir);
}
inline DLDirectory PlaneScape::getLibexecDir() const 
{
    return DLDirectory(baseDir, libexecDir);
}
inline DLDirectory PlaneScape::getDbDir() const
{
    return DLDirectory(baseDir, dbDir);
}
inline DLDirectory PlaneScape::getTextDir() const
{
    return DLDirectory(getShareDir(), textDir);
}

inline int PlaneScape::getPulsePerSecond() const 
{
    return pulsePerSecond.getValue();
}

inline int PlaneScape::getMissedPulses() const 
{
    return missed_pulses;
}
inline MudScheduler *PlaneScape::getScheduler()
{
    return scheduler.getPointer();
}

inline MudPluginManager *PlaneScape::getPluginManager()
{
    return pluginManager.getPointer();
}

inline ProcessManager *PlaneScape::getProcessManager()
{
    return processManager.getPointer();
}
inline MudStats *PlaneScape::getStats()
{
    return stats.getPointer();
}

inline TextFileLoader *PlaneScape::getTextFileLoader()
{
    return textFileLoader.getPointer();
}

#endif

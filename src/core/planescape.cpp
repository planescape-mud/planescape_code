/* $Id$
 *
 * ruffina, 2009
 */
#include "sysdep.h"
#include "structs.h"
#include "db-decl.h"
#include "comm-decl.h"

#include "logstream.h"
#include "dl_math.h"
#include "planescape.h"
#include "mudpluginmanager.h"
#include "mudscheduler.h"
#include "txncontext.h"
#include "feniamanager.h"
#include "process.h"
#include "mudstats.h"
#include "textfileloader.h"
#include "mudfile.h"

PlaneScape *mud = NULL;

PlaneScape::PlaneScape()
              : pulse(0), 
		pulsePerSecond(10),
		missed_pulses(0)
{
    checkDuplicate(mud);
    mud = this;

    dbEnv = new DbEnvContext();
    pluginManager.construct();
    scheduler.construct();
    feniaManager.construct();
    processManager.construct();
    stats.construct();
    textFileLoader.construct();
}

PlaneScape::~PlaneScape()
{
    feniaManager->close();

    processManager.clear();
    feniaManager.clear();
    scheduler.clear();
    pluginManager.clear();
    stats.clear();
    textFileLoader.clear();

    dbEnv->close();

    mud = NULL;
}

void PlaneScape::load()
{
    if (!loadConfig())
	throw Exception("Unable to load PlaneScape configuration");
    
    baseDir = DLDirectory(basePath);

    if (baseAbsolute)
        baseDir.toAbsolutePath();
    
    if (!logPattern.empty()) {
        LogStream::redirect(
                new FileLogStream(MudFile(logPattern).getPath()));
    }

    /* We don't want to restart if we crash before we get up. */
    MudFile(killScriptFile).touch();

    notice("Loading runtime data");
    if (!stats->load())
	throw Exception("Unable to refresh PlaneScape runtime data");
    else
        notice("Boot version %ld", stats->bootVersion.getValue());

    dbEnv->open(MudFile(feniaDbDir).getPath());
    feniaManager->open();
    feniaManager->load();

    notice("Loading plugins...");
    pluginManager->loadAll();
    notice("Loaded %d shared libraries", pluginManager->size());

    LogStream::sendNotice() 
	<< "Using '" << shareDir << "' for data loading." << endl;

    setSignals();
    
    /* init random number generator */
    init_mm();
}

void PlaneScape::startup()
{
    scheduler->tickStartup();

    /* If we made it this far, we will be able to restart without problem. */
    MudFile(killScriptFile).remove();
}

/*
 * This method contains the main loop which drives the entire MUD.  It
 * cycles once every 0.10 seconds and is responsible for ticking
 * game scheduler and checking requests for plugins realod.
 * All game logic (accepting new connections, input/output, "heartbeat"
 * functions) will be called from corresponding scheduler tasks.
 */
void PlaneScape::loop()
{    
    /* 
     * Initialize various time values 
     */
    setLastTime();
    setOptTime();

    notice("Старт игры.");

    /* 
     * The Main Loop.  The Big Cheese.  The Top Dog.  The Head Honcho.  The.. 
     */
    while (!isShutdown()) {	
	/* 
	 * Game scheduler runs
	 */
	scheduler->tick();

	/* 
	 * Sleep if we don't have any connections 
	 */
	waitUp();
	
	/*
	 * Calculate missed pulses.
	 * Sleep to resynchronize with the next upcoming pulse.
	 */
	sleepUp();

#ifdef CIRCLE_UNIX
	/* 
	 * Update tics for deadlock protection (UNIX only) 
	 */
	tics++;
#endif
        /*
         * Threads pulse
         */
	processManager->yield();
        /*
         * Check if need to initiate plugin reload
         */
	pluginManager->checkReloadRequest();
    }
}

void PlaneScape::shutdown()
{    
    scheduler->tickShutdown();
}

void PlaneScape::save()
{
    feniaManager->sync(0);

    if (!stats->save())
	throw Exception("Unable to save PlaneScape runtime data");
}

void PlaneScape::waitUp()
{    
    fd_set input_set;

    if (descriptor_list) 
	return;
    
    notice("Нет соединений. Переход в режим сна.");
    FD_ZERO(&input_set);
    FD_SET(mother_desc, &input_set);

    if (select(mother_desc + 1, &input_set, (fd_set *) 0, (fd_set *) 0, NULL) 
	< 0) {
	if (errno == EINTR)
	    notice("Waking up to process signal");
	else
	    syserr("Select coma");
    } else
	notice("Новое соединение. Подьем!");

    setLastTime();
}

void PlaneScape::setLastTime()
{
    last_time.update();
}

void PlaneScape::setOptTime()
{
    if (pulsePerSecond <= 0)
	throw Exception( "Number of pulses per second is <= 0" );

    opt_time.setUSec(1000000 / pulsePerSecond);
    opt_time.setSec(0);
}

void PlaneScape::sleepUp()
{
    Timer before_sleep, now, timeout;

    /*
     * At this point, we have completed all input, output and heartbeat
     * activity from the previous iteration, so we have to put ourselves
     * to sleep until the next 0.1 second tick.  The first step is to
     * calculate how long we took processing the previous iteration.
     */
    before_sleep.update(); /* current time */
    process_time = before_sleep - last_time;

    /*
     * If we were asleep for more than one pass, count missed pulses and sleep
     * until we're resynchronized with the next upcoming pulse.
     */
    countMissedPulses();

    /* Calculate the time we should wake up */
    last_time = before_sleep + (opt_time - process_time);

    /* Now keep sleeping until that time has come */
    now.update();
    timeout = last_time - now;

    if (missed_pulses == 1) {
        Scripting::Object::manager->sync(&last_time);

        now.update();
        timeout = last_time - now;
    }

    /* Go to sleep */
    do {
	timeout.sleep();
	now.update();
	timeout = last_time - now;
    } while (!timeout.elapsed());
}

/*
 * We will execute as many pulses as necessary--just one if we haven't
 * missed any pulses, or make up for lost time if we missed a few
 * pulses by sleeping for too long.
 */
void PlaneScape::countMissedPulses()
{
    if (process_time < opt_time) {
	missed_pulses = 0;
    } else {
	missed_pulses = process_time.getSec() * pulsePerSecond;
	missed_pulses += process_time.getUSec() / opt_time.getUSec();
	process_time.setSec(0);
	process_time.setUSec(process_time.getUSec() % opt_time.getUSec());
    }

    missed_pulses++;

    if (missed_pulses <= 0) {
	warn("SYSERR: Missed %d pulses, time going backwards!", missed_pulses);
	missed_pulses = 1;
    }

    /* If we missed more than N seconds worth of pulses, just do N secs */
    if (missed_pulses > (maxMissedSeconds * pulsePerSecond)) {
	warn("SYSERR: Missed %d seconds worth of pulses.", missed_pulses / pulsePerSecond);
	missed_pulses = maxMissedSeconds * pulsePerSecond;
    }
}

void PlaneScape::signalReloadAllPlugins(int signo)
{
    notice("Caught signal %d, reloading all plugins", signo);
    PluginManager::getThis()->setReloadAllRequest();
}

void PlaneScape::signalReloadChangedPlugins(int signo)
{
    notice("Caught signal %d, reloading changed plugins", signo);
    PluginManager::getThis()->setReloadChangedRequest();
}

void PlaneScape::setSignals()
{
    notice("Setting PlaneScape signals");
    signal(SIGUSR1, &signalReloadAllPlugins);
    signal(SIGUSR2, &signalReloadChangedPlugins);
}

/*
 * TODO
 */
/*
 * vars and functions needed during runtime (game loop)
 */
extern int tics;                       /* for extern checkpointing */
int shutting_down(void);
int copyover_down(void);
#if defined(CIRCLE_WINDOWS) || defined(CIRCLE_MACINTOSH) || defined(_MINGW_)
void gettimeofday(struct timeval *t, struct timezone *dummy);
#endif


int lastmessage = 0;
int shutting_down(void)
{
 if (!circle_shutdown)
    return (false);
 if (!shutdown_time || time(NULL) >= shutdown_time)
    return (true);
 if (lastmessage == shutdown_time || lastmessage == time(NULL))
    return (false);
 return (false);
}

int lastmessage_c = 0;

int copyover_down(void)
{
 if (!circle_copyover)
    return (false);

 if (!copyover_time || time(NULL) >= copyover_time)
    return (true);
 if (lastmessage_c == copyover_time || lastmessage_c == time(NULL))
    return (false);
 return (false);
}


bool PlaneScape::isShutdown() const
{
    return shutting_down() || copyover_down();
}





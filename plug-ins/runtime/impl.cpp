/* $Id$
 *
 * ruffina, 2009
 */
#include "so.h"
#include "plugin.h"
#include "date.h"
#include "planescape.h"
#include "mudscheduler.h"
#include "schedulertask.h"
#include "iomanager.h"

#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "exp_fn.h"
#include "case.h"
#include "handler.h"
#include "ban.h"
#include "help.h"
#include "act.social.h"
#include "iptoaddr.h"
#include "handler.h"

struct SingleInitTask : public virtual Plugin, public virtual SchedulerTask {
    typedef ::Pointer<SingleInitTask> Pointer;

    virtual void initialization() {
        mud->getScheduler()->putTaskStartup(Pointer(this));
    }
    virtual void destruction() {
        mud->getScheduler()->slay(Pointer(this ));
    }
};

struct RecurrentInitTask : public virtual Plugin, public virtual SchedulerTask {
    typedef ::Pointer<RecurrentInitTask> Pointer;

    virtual void run() {
        init();
    }
    virtual void initialization() {
        mud->getScheduler()->putTaskNOW(Pointer(this));
    }
    virtual void destruction() {
        destroy();
        mud->getScheduler()->slay(Pointer(this ));
    }
    virtual void init() = 0;
    virtual void destroy() = 0;
};

struct LoopingTask : public virtual Plugin, public virtual SchedulerTask {
    typedef ::Pointer<LoopingTask> Pointer;
    
    virtual void after() {
        mud->getScheduler()->putTaskInitiate(Pointer(this));
    }
    virtual void initialization() {
        mud->getScheduler()->putTaskNOW(Pointer(this));
    }
    virtual void destruction() {
        mud->getScheduler()->slay(Pointer(this ));
    }
};

struct ShutdownTask : public virtual Plugin, public virtual SchedulerTask {
    typedef ::Pointer<ShutdownTask> Pointer;
    
    virtual void initialization() {
        mud->getScheduler()->putTaskShutdown(Pointer(this));
    }
    virtual void destruction() {
        mud->getScheduler()->slay(Pointer(this ));
    }
};

/*--------------------------------------------------------------------*/
struct BootStartTask : public RecurrentInitTask {
    virtual void init() {
        cases_init();
        expfn_init();
        specials_init();
    }
    virtual void destroy() {
        cases_destroy();
        expfn_destroy();
        specials_destroy();
    }
    virtual int getPriority() const {
	return SCDP_BOOT_START;
    }
};

struct BootDbTask : public SingleInitTask {
    virtual void run() {
        boot_db();
    }
    virtual int getPriority() const {
	return SCDP_BOOT_START + 10;
    }
};

struct BootEndTask : public RecurrentInitTask {
    virtual void init() {
        signal_init();
	
        commands_init();
        adverb_init();
        socials_init();
        ban_init();
        help_init();
        iptoaddr_init();
    }
    virtual void destroy() {
    	commands_destroy();
        adverb_destroy();
        socials_destroy();
        ban_destroy();
        help_destroy();
        iptoaddr_destroy();

        signal_destroy();
    }
    virtual int getPriority() const {
	return SCDP_BOOT_END;
    }
};

struct InitGameTask : public SingleInitTask {
    virtual void run() {
        max_players = get_max_players();

        if (!mud->modCopyOver) { /* If copyover mother_desc is already set up */
            mother_desc = init_socket(mud->port);
        }
        else {
            /* reload players */
            copyover_recover();
        }
    }
    virtual int getPriority() const {
	return SCDP_INITIAL;
    }
};

struct RebootWarningTask : public LoopingTask {
    virtual void run() {
	warn_shutdown();
	warn_copyover();
    }
    virtual int getPriority() const {
	return SCDP_INITIAL;
    }
};

struct HeartbeatTask : public LoopingTask {
    virtual void run() {
	int missed_pulses = mud->getMissedPulses();

        /* Execute the heartbeat functions */
	while (missed_pulses--) {
	    heartbeat(++(mud->pulse));
	}

        /* Roll pulse over after 10 hours */
	if (mud->pulse >= 10 * Date::SECOND_IN_HOUR * mud->getPulsePerSecond())
	    mud->pulse = 0;
    }
    virtual int getPriority() const {
	return SCDP_HEARTBEAT;
    }
};

struct EndGameTask : public ShutdownTask {
    virtual void run() {
	end_game();
    }
    virtual int getPriority() const {
	return SCDP_SHUTDOWN;
    }
};

extern "C"
{
    SO::PluginList initialize_runtime( )
    {
	SO::PluginList ppl;

	Plugin::registerPlugin<BootStartTask>( ppl );
	Plugin::registerPlugin<BootDbTask>( ppl );
	Plugin::registerPlugin<BootEndTask>( ppl );
	Plugin::registerPlugin<InitGameTask>( ppl );
	Plugin::registerPlugin<RebootWarningTask>( ppl );
	Plugin::registerPlugin<HeartbeatTask>( ppl );
	Plugin::registerPlugin<EndGameTask>( ppl );
	Plugin::registerPlugin<IOManager>( ppl );

	return ppl;
    }
}

/* $Id$
 *
 * ruffina, 2009
 */
#include "mudscheduler.h"
#include "schedulertask.h"
#include "schedulerqueue.h"
#include "schedulerprioritymap.h"
#include "planescape.h"

const long MudScheduler::SHUTDOWN_TIME = 0xFFFFFFF0;
const long MudScheduler::STARTUP_TIME = 0;

MudScheduler::MudScheduler( ) 
{
    checkDuplicate( mud->getScheduler( ) );
}

MudScheduler::~MudScheduler( ) 
{
}

void MudScheduler::tickStartup()
{
    time = STARTUP_TIME;
    tick();
}

void MudScheduler::tickShutdown()
{
    time = SHUTDOWN_TIME;
    tick();
}

void MudScheduler::putTaskStartup( SchedulerTask::Pointer task ) 
{
    queue.put( STARTUP_TIME, task );
}

void MudScheduler::putTaskShutdown( SchedulerTask::Pointer task ) 
{
    queue.put( SHUTDOWN_TIME, task );
}

void MudScheduler::putTaskInSecond( SchedulerTask::Pointer task ) 
{
    queue.put( time + mud->getPulsePerSecond( ), task );
}

void MudScheduler::putTaskInSecond( long offset, SchedulerTask::Pointer task ) 
{
    queue.put( time + offset * mud->getPulsePerSecond( ), task );
}


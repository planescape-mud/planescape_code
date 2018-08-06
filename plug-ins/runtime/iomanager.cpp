/* $Id$
 *
 * ruffina, 2009
 */
#include "so.h"

#include "iomanager.h"
#include "mudscheduler.h"
#include "planescape.h"

#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "house.h"
#include "events.h"
#include "screen.h"
#include "constants.h"
#include "spells.h"
#include "xboot.h"
#include "planescape.h"

char end_line[] = { IAC, GA, 0 };

void IOTask::after()
{
    mud->getScheduler()->putTaskInitiate( Pointer(this) );
}

int IOInitTask::getPriority() const
{
    return SCDP_IOINIT;
}

void IOInitTask::run()
{
    IOManager::getThis()->ioInit();
}


int IOPollTask::getPriority() const
{
    return SCDP_IOPOLL;
}

void IOPollTask::run()
{
    IOManager::getThis()->ioPoll();
}


int IOReadTask::getPriority() const
{
    return SCDP_IOREAD;
}

void IOReadTask::run()
{
    IOManager::getThis()->ioRead();
}

int IOWriteTask::getPriority() const
{
    return SCDP_IOWRITE;
}

void IOWriteTask::run()
{
    IOManager::getThis()->ioWrite();
}

IOManager *IOManager::thisClass = 0;

IOManager::IOManager()
{
    checkDuplicate(thisClass);
    thisClass = this;
}
IOManager::~IOManager()
{
    thisClass = 0;
}

void IOManager::initialization()
{
    mud->getScheduler()->putTaskNOW( IOInitTask::Pointer( NEW ) );
    mud->getScheduler()->putTaskNOW( IOPollTask::Pointer( NEW ) );
    mud->getScheduler()->putTaskNOW( IOReadTask::Pointer( NEW ) );
    mud->getScheduler()->putTaskNOW( IOWriteTask::Pointer( NEW ) );
}

void IOManager::destruction()
{
    mud->getScheduler()->slay( IOInitTask::Pointer( NEW ) );
    mud->getScheduler()->slay( IOPollTask::Pointer( NEW ) );
    mud->getScheduler()->slay( IOReadTask::Pointer( NEW ) );
    mud->getScheduler()->slay( IOWriteTask::Pointer( NEW ) );
}

void IOManager::ioInit()
{
    struct descriptor_data *d;
    
    /* Set up the input, output, and exception sets for select(). */
    FD_ZERO(&input_set);
    FD_ZERO(&output_set);
    FD_ZERO(&exc_set);
    FD_SET(mother_desc, &input_set);

    maxdesc = mother_desc;
    for (d = descriptor_list; d; d = d->next) {
#ifndef CIRCLE_WINDOWS
	if (d->descriptor > maxdesc)
	    maxdesc = d->descriptor;
#endif
	FD_SET(d->descriptor, &input_set);
	FD_SET(d->descriptor, &output_set);
	FD_SET(d->descriptor, &exc_set);
    }
}

void IOManager::ioPoll()
{
    struct descriptor_data *d, *next_d;
    static struct timeval null_time = { 0, 0 };

    /* Poll (without blocking) for new input, output, and exceptions */
    if (select(maxdesc + 1, &input_set, &output_set, &exc_set, &null_time) < 0)
    {
	logsystem("SYSERR: select poll");
	throw Exception();
    }

    /* If there are new connections waiting, accept them. */
    if (FD_ISSET(mother_desc, &input_set))
	new_descriptor(mother_desc);

    /* Kick out the freaky folks in the exception set and marked for close */
    for (d = descriptor_list; d; d = next_d) {
	next_d = d->next;
	if (FD_ISSET(d->descriptor, &exc_set)) {
	    FD_CLR(d->descriptor, &input_set);
	    FD_CLR(d->descriptor, &output_set);
	    close_socket(d, TRUE);
	}
    }
}

void IOManager::ioRead()
{
    struct descriptor_data *d, *next_d;
    char comm[MAX_INPUT_LENGTH];

    /* Process descriptors with input pending */
    for (d = descriptor_list; d; d = next_d) {
	next_d = d->next;
	if (FD_ISSET(d->descriptor, &input_set))
	    if (process_input(d) < 0)
		close_socket(d, FALSE);
    }

    /* Process commands we just read from process_input */
    for (d = descriptor_list; d; d = next_d) {
	int aliased = 0;
	next_d = d->next;

	if (d->character) {
	    if (GET_WAIT_STATE(d->character))
		continue;
	}
	if (!get_from_q(&d->input, comm, &aliased)) {
	    if (STATE(d) != CON_PLAYING && STATE(d) != CON_DISCONNECT
		&& time(NULL) - d->input_time > 300 && d->character
		&& !IS_GOD(d->character)
		)
		close_socket(d, TRUE);
	    continue;
	}
	
	d->input_time = time(NULL);
	if (d->character) {	/* Reset the idle timer & pull char back from void if necessary */
	    d->character->char_specials.timer = 0;
	    if (STATE(d) == CON_PLAYING && GET_WAS_IN(d->character) != NOWHERE) {	
		GET_WAS_IN(d->character) = NOWHERE;
		act("1+и приш1(ел,ла,ло,ли) в себя.", "Км", d->character);
		REMOVE_BIT(PLR_FLAGS(d->character, PLR_DROPLINK),
			   PLR_DROPLINK);
		GET_WAIT_STATE(d->character) = 1;
		load_pets(d->character);
	    }
	}

	d->has_prompt = 0;

	if (d->showstr_count)	/* Reading something w/ pager */
	    show_string(d, comm);
	else if (d->str)
	    string_add(d, comm);
	else if (STATE(d) != CON_PLAYING)	/* In menus, etc. */
	    nanny(d, comm);
	else {		/* else: we're playing normally. */
	    if (aliased)	/* To prevent recursive aliases. */
		d->has_prompt = 1;	/* To get newline before next cmd output. */
	    else if (perform_alias(d, comm))	/* Run it through aliasing system */
		get_from_q(&d->input, comm, &aliased);
	    command_interpreter(d->character, comm);	/* Send it to interpreter */
	}
    }
}

void IOManager::ioWrite()
{
    struct descriptor_data *d, *next_d;

    for (d = descriptor_list; d; d = next_d) {
	next_d = d->next;
	if ((!d->has_prompt || *(d->output))
	    && FD_ISSET(d->descriptor, &output_set)
	    ) {
	    if (process_output(d) < 0)
		close_socket(d, FALSE);	// закрыл соединение
	    else
		d->has_prompt = 1;	// признак того, что промпт уже выводил
	    // следующий после команды или очередной 
	    // порции вывода                                 
	}
    }

    /* Print prompts for other descriptors who had no other output */
    /* Kick out folks in the CON_CLOSE or CON_DISCONNECT state */
    for (d = descriptor_list; d; d = next_d) {
	next_d = d->next;
	if (STATE(d) == CON_CLOSE || STATE(d) == CON_DISCONNECT)
	    close_socket(d, FALSE);
    }

    for (d = descriptor_list; d; d = d->next) {
	if (d->character && !d->has_prompt && !FIGHTING(d->character)) {
	    SEND_TO_Q(make_prompt(d), d);
	    SEND_TO_Q(end_line, d);
	    d->has_prompt = 2;
	}
    }
}


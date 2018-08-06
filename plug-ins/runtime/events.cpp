
#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "events.h"
#include "xboot.h"

char *event_status_bar(struct char_data *ch)
{
    struct event_item_data *events = events_list;

    static char buf[MAX_STRING_LENGTH];

    *buf = '\0';

    while (events) {
        if (events->params && events->params->actor == ch && events->show_meter)
            sprintf(buf, "[&m%s&n] ", meter_barf(ch, events->ctime, events->time));
        events = events->next;
    }

    return (buf);
}

void free_event(struct event_item_data *event)
{
    struct event_item_data *curr;

    if (events_list == event) {
        events_list = event->next;
    } else {
        curr = events_list;
        while (curr && (curr->next != event))
            curr = curr->next;
        if (!curr)
            return;             /* failed to find it */
        curr->next = curr->next->next;
    }

    if (event->params->sarg)
        free((char *) event->params->sarg);
    if (event->params->action)
        free((char *) event->params->action);
    if (event->params->status)
        free((char *) event->params->status);
    if (event->params->vaction)
        free((char *) event->params->vaction);
    if (event->params->vstatus)
        free((char *) event->params->vstatus);
    if (event->params->raction)
        free((char *) event->params->raction);
    if (event->params->rstatus)
        free((char *) event->params->rstatus);
    if (event->params->sto_actor)
        free((char *) event->params->sto_actor);
    if (event->params->sto_victim)
        free((char *) event->params->sto_victim);
    if (event->params->sto_room)
        free((char *) event->params->sto_room);
    if (event->params->bto_actor)
        free((char *) event->params->bto_actor);
    if (event->params->bto_victim)
        free((char *) event->params->bto_victim);
    if (event->params->bto_room)
        free((char *) event->params->bto_room);
    if (event->params)
        free(event->params);

    /*  event->params->actor=NULL;
       event->params->victim=NULL;
       event->params->object=NULL;  */

    free(event);
}

void add_event(int time, int script, AEVENT(*func), struct event_param_data *params)
{
    struct event_item_data *e;  //,*prev, *curr;
    struct event_param_data *p;

    CREATE(e, struct event_item_data, 1);
    CREATE(p, struct event_param_data, 1);

    e->time = time;
    e->ctime = time;
    e->stopflag = params->stopflag;
    e->show_meter = params->show_meter;
    e->saved = params->saved;

    p->actor = params->actor;
    p->victim = params->victim;
    p->object = params->object;

    p->narg[0] = params->narg[0];
    p->narg[1] = params->narg[1];
    p->narg[2] = params->narg[2];
    p->narg[3] = params->narg[3];
    p->sarg = str_dup(params->sarg);
    p->action = str_dup(params->action);
    p->status = str_dup(params->status);
    p->vaction = str_dup(params->vaction);
    p->vstatus = str_dup(params->vstatus);
    p->raction = str_dup(params->raction);
    p->rstatus = str_dup(params->rstatus);
    p->sto_actor = str_dup(params->sto_actor);
    p->sto_victim = str_dup(params->sto_victim);
    p->sto_room = str_dup(params->sto_room);
    p->bto_actor = str_dup(params->bto_actor);
    p->bto_victim = str_dup(params->bto_victim);
    p->bto_room = str_dup(params->bto_room);

    e->params = p;
    e->script = script;
    e->func = func;

    if (e->params->sto_actor && e->params->actor)
        act(e->params->sto_actor, "íÍĞ", e->params->actor, e->params->victim, e->params->object);

    if (e->params->sto_victim && e->params->victim)
        act(e->params->sto_victim, "ÍíĞ", e->params->actor, e->params->victim, e->params->object);

    if (e->params->sto_room && e->params->actor)
        act(e->params->sto_room, "ëÍÍĞ", e->params->actor, e->params->victim, e->params->object);

    if (events_list != NULL)
        e->next = events_list;

    events_list = e;

}



void pulse_events(int pulse)
{
    struct event_item_data *e = events_list, *del = NULL;

    if (!(pulse % PULSE_CHAR_EVENT))
        while (e)
            if (--(e->ctime) == 0) {
                if (e->script)  //÷ÙĞÏÌÎÑÅÍ ÓËÒÉĞÔ
                    go_script(e->script, e->params->actor);
                if (e->func)
                    e->func(e->params);

                //÷Ù×ÏÄÉÍ ÉÎÆÕ Ï ×ÙĞÏÌÎÅÎÉÉ
                if (e->params->object) {
                    if (e->params->action && e->params->actor)
                        act(e->params->action, "íÍĞ", e->params->actor, e->params->victim,
                            e->params->object);
                    if (e->params->vaction && e->params->actor && e->params->victim)
                        act(e->params->vaction, "ÍíĞ", e->params->actor, e->params->victim,
                            e->params->object);
                    if (e->params->raction && e->params->actor)
                        act(e->params->raction, "ëÍÍĞ", e->params->actor, e->params->victim,
                            e->params->object);
                } else if (e->params->victim) {
                    if (e->params->action && e->params->actor)
                        act(e->params->action, "íÍ", e->params->actor, e->params->victim);
                    if (e->params->vaction && e->params->actor && e->params->victim)
                        act(e->params->vaction, "Íí", e->params->actor, e->params->victim);
                    if (e->params->raction && e->params->actor)
                        act(e->params->raction, "ëÍÍ", e->params->actor, e->params->victim);
                } else {
                    if (e->params->action && e->params->actor)
                        act(e->params->action, "í", e->params->actor);
                    if (e->params->raction && e->params->actor)
                        act(e->params->raction, "ëÍ", e->params->actor);
                }
                del = e;
                e = e->next;
                free_event(del);
            } else
                e = e->next;

}

const char *get_line_event(struct char_data *ch)
{
    struct event_item_data *e = events_list;

    while (e) {
        if (e->params && e->params->actor != ch) {
            e = e->next;
            continue;
        }
        if (e->params && e->params->rstatus)
            return (e->params->rstatus);
    }

    return (NULL);
}


const char *get_status_event(struct char_data *ch)
{
    struct event_item_data *e = events_list;

    while (e) {
        if (e->params && e->params->actor != ch) {
            e = e->next;
            continue;
        }
        if (e->params && e->params->status)
            return (e->params->status);
    }

    return (NULL);
}

void init_event_param(struct event_param_data *params)
{
    params->actor = NULL;
    params->victim = NULL;
    params->object = NULL;
    params->stopflag = 0;
    params->show_meter = 0;
    params->saved = 0;
    params->sarg = NULL;
    params->narg[0] = 0;
    params->narg[1] = 0;
    params->narg[2] = 0;
    params->narg[3] = 0;
    params->action = NULL;
    params->status = NULL;
    params->vaction = NULL;
    params->vstatus = NULL;
    params->raction = NULL;
    params->rstatus = NULL;
    params->sto_actor = NULL;
    params->sto_victim = NULL;
    params->sto_room = NULL;
    params->bto_actor = NULL;
    params->bto_victim = NULL;
    params->bto_room = NULL;
}

void extract_event_from_char(struct char_data *ch)
{
    struct event_item_data *e = events_list, *del = NULL;

    while (e) {
        if (e->params && e->params->victim == ch) {
            del = e;
            e = e->next;
            stop_events(del->params->victim, STOP_ALL);
            free_event(del);
        } else if (e->params && e->params->actor == ch) {
            del = e;
            e = e->next;
            stop_events(del->params->actor, STOP_ALL);
            free_event(del);
        } else
            e = e->next;
    }
}

void extract_event_from_object(struct obj_data *object)
{
    struct event_item_data *e = events_list, *del = NULL;

    while (e) {
        if (e->params && e->params->object == object) {
            del = e;
            e = e->next;
            if (del->params->actor)
                stop_event(del);
            free_event(del);
        } else
            e = e->next;
    }
}


void stop_event(struct event_item_data *e)
{
    if (e->params) {
        if (e->params->actor && e->params->bto_actor)
            act(e->params->bto_actor, "í", e->params->actor);
        if (e->params->actor && e->params->victim && e->params->bto_victim)
            act(e->params->bto_victim, "Íí", e->params->actor, e->params->victim);
        if (e->params->actor && e->params->bto_room)
            act(e->params->bto_room, "ëÍ", e->params->actor);
    }
}

void stop_events(struct char_data *ch, int stop)
{
    struct event_item_data *e = events_list, *del = NULL;

    while (e) {
        if (e->params && e->params->actor == ch
            && (e->stopflag == STOP_ALL || (e->stopflag == STOP_HIDDEN && stop == FALSE)
                || (stop == STOP_ALL && e->stopflag != STOP_NONE))) {
            if (e->params->actor && e->params->bto_actor)
                act(e->params->bto_actor, "í", e->params->actor);
            if (e->params->actor && e->params->victim && e->params->bto_victim)
                act(e->params->bto_victim, "Íí", e->params->actor, e->params->victim);
            if (e->params->actor && e->params->bto_room)
                act(e->params->bto_room, "ëÍ", e->params->actor);
            del = e;
            e = e->next;
            free_event(del);
        } else
            e = e->next;
    }
}



int event_count(void)
{
    struct event_item_data *e = events_list;
    int count = 0;

    while (e) {
        count++;
        e = e->next;
    }

    return (count);
}

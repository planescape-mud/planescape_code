#ifndef EVENTS_H
#define EVENTS_H

#include "events-decl.h"

/***************************************
 * Функции системы отложенных действий */
/***************************************/

#define PULSE_CHAR_EVENT 10

void init_event_param(struct event_param_data *params);
void add_event(int time, int script, AEVENT(*func), struct event_param_data *params);
const char *get_line_event(struct char_data *ch);
const char *get_status_event(struct char_data *ch);
void extract_event_from_char(struct char_data *ch);
void extract_event_from_object(struct obj_data *object);
void stop_events(struct char_data *ch, int stop);
void stop_event(struct event_item_data *e);
char *event_status_bar(struct char_data *ch);
int event_count(void);
void pulse_events(int pulse);


#endif

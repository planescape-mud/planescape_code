#ifndef COMM_H
#define COMM_H
/* ************************************************************************
*   File: comm.h                                        Part of CircleMUD *
*  Usage: header file: prototypes of public communication functions       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "comm-decl.h"

#define send_to_char(txt, ch) send_to_charf(ch, "%s", txt)

#define NUM_RESERVED_DESCS 8
#define COPYOVER_FILE "copyover.dat"

/* comm.cpp */
#ifdef _MINGW_
void gettimeofday(struct timeval *t, struct timezone *dummy);
#endif

void send_to_all(const char *messg, ...);
void send_to_charf(struct char_data *ch, const char *messg, ...);
void send_stat_char(struct char_data *ch);
void send_to_room(const char *messg, room_rnum room, int to_awake);
void send_to_weather(int type_weather, int num_zone);
void send_to_outdoor(const char *messg, int control);
void send_to_gods(const char *messg);
void perform_to_all(const char *messg, struct char_data *ch);
void close_socket(struct descriptor_data *d, int derect);

void perform_act(const char *orig, struct char_data *ch,
                 struct obj_data *obj, const void *vict_obj, struct char_data *to);

void act(const char *str, int hide_invisible, struct char_data *ch,
         struct obj_data *obj, const void *vict_obj, int type);

void act(const char *str, const char *format, ...);

int posi_value(int real, int max);
int posi_value5(int real, int max);

void add_snooper(struct char_data *ch);
void del_snooper(struct char_data *ch);

void echo_on(struct descriptor_data *d);
void echo_off(struct descriptor_data *d);

unsigned long get_ip(const char *addr);

#define SUN_CONTROL     (0 << 1)
#define WEATHER_CONTROL (1 << 1)

#define TO_ROOM  1
#define TO_VICT  2
#define TO_NOTVICT 3
#define TO_CHAR  4
#define TO_ROOM_HIDE    5       /* В комнату, но только тем, кто чувствует жизнь */
#define CHECK_DEAF      64
#define TO_SLEEP 128            /* to char, even if sleeping */

/* I/O functions */
int write_to_descriptor(socket_t desc, const char *txt, size_t total);
int write_to_descriptor(socket_t desc, const char *txt);
void write_to_q(const char *txt, struct txt_q *queue, int aliased);
void write_to_output(const char *txt, struct descriptor_data *d);

int get_max_players(void);
int process_output(struct descriptor_data *t);
int process_input(struct descriptor_data *t);
int get_from_q(struct txt_q *queue, char *dest, int *aliased);
int new_descriptor(socket_t s);
socket_t init_socket(ush_int port);
char *make_prompt(struct descriptor_data *point);
void heartbeat(int pulse);

void end_game();
void copyover_recover();
void warn_shutdown();
void warn_copyover();

void signal_init();
void signal_destroy();

int toggle_compression(struct descriptor_data *d);

#define SEND_TO_Q(messg, desc)  write_to_output((messg), desc)
#define SEND_TO_SOCKET(messg, desc) write_to_descriptor((desc), (messg), strlen(messg))

#define USING_SMALL(d) ((d)->output == (d)->small_outbuf)
#define USING_LARGE(d)  ((d)->output == (d)->large_outbuf)

/* color.cpp */
void proc_color(char *inbuf, int colour);

#endif

#ifndef MOBMAX_H
#define MOBMAX_H
/* ************************************************************************
*   File: mobmax.h                                      Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for замакса по мобам   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

int clear_kill_vnum(struct char_data *vict, int vnum);
void inc_kill_vnum(struct char_data *ch, int vnum, int incvalue);
int get_kill_vnum(struct char_data *ch, int vnum);
void save_mkill(struct char_data *ch, FILE * saved);
void free_mkill(struct char_data *ch);
void mob_lev_count(void);

extern int num_mob_lev[MAX_MOB_LEVEL + 1];

#endif

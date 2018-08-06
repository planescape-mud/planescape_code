#ifndef MUD_PK_H
#define MUD_PK_H
/* ************************************************************************
*   File: pk.h                                          Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for ПК система         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

int dec_pk_values(struct char_data *ch, struct char_data *victim, int pkills, int prevenge);
int inc_pk_values(struct char_data *killer, struct char_data *victim, int pkills, int prevenge);
int inc_pk_thiefs(struct char_data *ch, struct char_data *victim);
int may_pkill(struct char_data *revenger, struct char_data *killer);
int inc_pkill(struct char_data *victim, struct char_data *killer, int pkills, int prevenge);
int dec_pkill(struct char_data *victim, struct char_data *killer);
void inc_pkill_group(struct char_data *victim, struct char_data *killer, int pkills, int prevenge);
int dec_pkill_group(struct char_data *victim, struct char_data *killer);
void pk_list_sprintf(struct char_data *ch, char *buff);
int KILLER(struct char_data *killer);

// Добавлено Дажьбогом
#define MAX_PK_CHAR 30
#define FirstPK 1
#define SecondPK 5
#define ThirdPK  10
#define FourthPK 20
#define FifthPK  MAX_PK_CHAR

#endif

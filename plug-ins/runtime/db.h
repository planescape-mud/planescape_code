#ifndef MUD_DB_H
#define MUD_DB_H
/* ************************************************************************
*   File: db.h                                          Part of CircleMUD *
*  Usage: header file for database handling                               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "db-decl.h"

/* arbitrary constants used by index_boot() (must be unique) */
#define DB_BOOT_WLD 0
#define DB_BOOT_MOB 1
#define DB_BOOT_OBJ 2
#define DB_BOOT_ZON 3
#define DB_BOOT_SHP 4
#define DB_BOOT_SOCIAL 7
#define DB_BOOT_ADV 9

#define LIB_A       "A-E"
#define LIB_F       "F-J"
#define LIB_K       "K-O"
#define LIB_P       "P-T"
#define LIB_U       "U-Z"
#define LIB_Z       "ZZZ"

#define PET_SUF  "pets"
#define SUF_OBJS "objs"
#define TEXT_SUF_OBJS "textobjs"
#define TIME_SUF_OBJS "timeobjs"
#define SUF_TEXT "text"
#define SUF_ALIAS "alias"
#define SUF_MEM  "mem"
#define SUF_QST  "qst"
#define SUF_PLAYER  "player"
#define SUF_PKILLER "pkiller"
#define SUF_QUESTS  "xsave"
#define SUF_PMKILL "mobkill"
#define SUF_LOGGER "log"

/* public procedures in db.c */
void delete_players(void);
void tag_argument(char *argument, char *tag);
void boot_db(void);
void index_boot(int mode);
int create_entry(char *name);
void zone_update(void);
void new_reset_zone(zone_rnum zone);
room_rnum real_room(room_vnum vnum);
char *fread_string(FILE * fl, char *error);
long get_id_by_name(char *name);
long cmp_ptable_by_name(char *name, int len);
long get_ptable_by_name(char *name);
void update_ptable_data(struct char_data *ch);
int get_room_mob_number(int vnum_mob, int vnum_room);
char *get_name_by_id(long id);
char *get_name_by_unique(long id);
void delete_unique(struct char_data *ch);
int correct_unique(int unique);
void set_time(struct time_info_data times);

void get_one_line(FILE * fl, char *buf);
void asciiflag_conv(const char *flag, void *value);

int load_char(char *name, struct char_data *char_element);
void save_char(struct char_data *ch, room_rnum load_room);
void save_player_index(void);
void init_char(struct char_data *ch);
int delete_char(char *name);
void rename_char(struct char_data *ch, char *oname);

char *alias_vmobile(int vnumber);
char *name_vmobile(int vnumber);
struct char_data *read_mobile(mob_vnum nr, int type);
mob_rnum real_mobile(mob_vnum vnum);
zone_rnum real_zone_vnum(zone_vnum vnum);
int vnum_mobile(char *searchname, struct char_data *ch);
struct char_data *find_mobile(mob_vnum nr);
int mobs_in_room(int m_num, int r_num);

int obj_in_room(int o_num, int r_num);
struct obj_data *create_obj(void);
obj_rnum real_object(obj_vnum vnum);
struct obj_data *read_object(obj_vnum nr, int type, int load);
int vnum_object(char *searchname, struct char_data *ch);

#define REAL          0
#define VIRTUAL       (1 << 0)
#define OBJ_NO_CALC   (1 << 1)

#endif

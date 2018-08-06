#ifndef GAMECONFIG_H
#define GAMECONFIG_H

/*
 * See gameconfig.cc for explanations
 */

extern int free_crashrent_period;
extern int free_rebootrent_period;
extern int free_rent;
extern int idle_max_level;
extern int idle_rent_time;
extern int idle_void;
extern int auto_save;
extern int autosave_time;
extern int track_through_doors;
extern int siteok_everyone;
extern int nameserver_is_slow;
extern int load_into_inventory;
extern int max_playing;
extern int max_bad_pws;

extern const char *OK;
extern const char *NOPERSON;
extern const char *NOEFFECT;
extern const char *ONLYSAME;

extern const char *MENU;
extern const char *START_MESSG;
extern const char *WELC_MESSG;

extern const char *DFLT_IP;

extern room_vnum frozen_start_room;
extern room_vnum helled_start_room;
extern room_vnum immort_start_room;
extern room_vnum jail_start_room;
extern room_vnum mortal_start_room;
extern room_vnum named_start_room;
extern room_vnum unreg_start_room;

int calc_loadroom(struct char_data *ch);

#endif

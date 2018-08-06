#ifndef MUD_DB_DECL_H
#define MUD_DB_DECL_H
/* ************************************************************************
*   File: db.h                                          Part of CircleMUD *
*  Usage: header file for database handling                               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/* structure for the reset commands */
struct reset_com {
    char command;   /* current command                      */

    bool if_flag; /* if TRUE: exe only if preceding exe'd */
    int arg1;  /*                                      */
    int arg2;  /* Arguments to the command             */
    int arg3;  /*                                      */
    int  arg4;
    int  oldarg1;
    int  oldarg2;
    int  oldarg3;
    int  oldarg4;
    int  line;  /* line number this command appears on  */
    char *sarg1;  /* string argument                      */
    char *sarg2;  /* string argument                      */

    /*
    *  Commands:              *
    *  'M': Read a mobile     *
    *  'O': Read an object    *
    *  'G': Give obj to mob   *
    *  'P': Put obj in obj    *
    *  'G': Obj to char       *
    *  'E': Obj to char equip *
    *  'D': Set state of door *
    *  'T': Trigger command   *
    */
};


/* zone definition structure. for the 'zone-table'   */
struct zone_data {
    char *name_maj;
    char *name;      /* name of this zone                  */
    int lifespan;           /* how long between resets (minutes)  */
    int age;                /* current age of this zone (minutes) */
    room_vnum top;           /* upper limit for rooms in this zone */
    byte plane;             /* Грань зоны (0 - сигил, 1 - баатор и etc */
    byte type;               /* Тип зоны */
    int recall;
    int time_offset;     /* -/+ в тиках по времени относительно GTW */
    char *author;
    char *description;

    int reset_mode;         /* conditions for reset (see below)   */
    zone_vnum number;     /* virtual number of this zone   */
    struct reset_com *cmd;   /* command table for reset           */

    struct weather_data weather_info; // погода в зоне
    struct time_info_data time_info; // время в зоне
    byte noplayer; // если 1 запрет для игрока
    /*
     * Reset mode:
     *   0: Don't reset, and don't update age.
     *   1: Reset if no PC's are located in zone.
     *   2: Just reset.
     */
};

/* for queueing zones for update   */
struct reset_q_element {
    zone_rnum zone_to_reset;            /* ref to zone_data */
    struct reset_q_element *next;
};


/* structure for the update queue     */
struct reset_q_type {
    struct reset_q_element *head;
    struct reset_q_element *tail;
};

extern struct reset_q_type reset_q;    /* queue of zones to be reset    */
extern int zone_update_timer;

#define OBJECT_SAVE_ACTIVITY 300
#define PLAYER_SAVE_ACTIVITY 300
#define MAX_SAVED_ITEMS      500

struct player_index_element {
    char   *name;
    long    id;
    long    unique;
    int     level;
    int     last_logon;
    int     activity;         // When player be saved and checked
    int     timer;
};

extern struct player_index_element *player_table;       /* index to plr file     */
extern int top_of_p_table;         /* ref to top of table           */
extern int top_of_p_file;          /* ref of size of p file         */
extern long top_idnum;             /* highest idnum in use          */
extern int now_entrycount;

extern const struct new_flag clear_flags; /* to use as zero flags */


extern long beginning_of_time;

extern struct room_data *world; /* array of rooms                */
extern room_rnum top_of_world;    /* ref to top element of world   */

extern struct char_data *character_list; /* global linked list of  chars  */
extern long max_id;   /* for unique mob/obj id's       */

extern struct index_data *mob_index;   /* index table for mobile file   */
extern struct Mobile *mob_proto;    /* prototypes for mobs           */
extern mob_rnum top_of_mobt;       /* top of mobile index table     */

extern struct obj_data *object_list;    /* global linked list of objs    */
extern struct index_data *obj_index;   /* index table for object file   */
extern struct obj_data *obj_proto;     /* prototypes for objs           */
extern obj_rnum top_of_objt;       /* top of object index table     */

extern struct zone_data *zone_table;   /* zone table                    */
extern zone_rnum top_of_zone_table;/* top element of zone tab       */
extern int count_zones[NUM_ZONES];
extern struct set_items *set_table;    /* table of item sets */
extern int top_of_sets;            /* top element of sets table */

extern int no_mail;                /* mail disabled?                */
extern time_t boot_time;           /* time of mud boot              */
extern room_rnum r_mortal_start_room;  /* rnum of mortal start room     */
extern room_rnum r_immort_start_room;  /* rnum of immort start room     */
extern room_rnum r_frozen_start_room;  /* rnum of frozen start room     */
extern room_rnum r_helled_start_room;
extern room_rnum r_jail_start_room;
extern room_rnum r_named_start_room;
extern room_rnum r_unreg_start_room;

extern std::vector<struct bug_abuse_data> bug_abuse;

extern struct time_info_data time_info;/* the infomation about the time    */
extern struct weather_data weather_info; /* the infomation about the weather */

/* boot-time globals */
extern int imbs; /* currently booting mobile */
extern int iobj; /* currently booting object */
extern int room_nr; /* currently booting room */
extern int zone_nr; /* currently booting zone */

#endif

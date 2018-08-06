#include "sysdep.h"
#include "structs.h"
#include "comm-decl.h"
#include "db-decl.h"
#include "mail-decl.h"
#include "boards-decl.h"
#include "interp-decl.h"
#include "spells-decl.h"
#include "events-decl.h"


/*
 * boards.cc
 */
/*
 * format: vnum, read lvl, write lvl, remove lvl, filename, 0 at end
 * Be sure to also change NUM_OF_BOARDS in board.h
 */
struct board_info_type board_info[NUM_OF_BOARDS] = {
    {135, LVL_IMMORT, LVL_GOD, LVL_GRGOD, "board.immort", 0},
    {5195, 0, 3, LVL_GOD, "board.51", 0}
};

char *msg_storage[INDEX_SIZE];
int msg_storage_taken[INDEX_SIZE];
int num_of_msgs[NUM_OF_BOARDS];
struct board_msginfo msg_index[NUM_OF_BOARDS][MAX_BOARD_MESSAGES];
bool boards_loaded = false;

/*
 * comm.cc
 */
struct ip_table_type *ip_table;
socket_t mother_desc;           /* main server socket */
struct descriptor_data *descriptor_list = NULL;         /* master desc list */
struct txt_block *bufpool = 0;  /* pool of large output buffers */
int buf_largecount = 0;         /* # of large buffers which exist */
int buf_overflows = 0;          /* # of overflows of output */
int buf_switches = 0;           /* # of switches from small to large buf */
int circle_shutdown = 0;        /* clean shutdown */
int circle_copyover = 0;        /* clean copyover */
int circle_reboot   = 0;        /* reboot the game after a shutdown */
int shutdown_time   = 0;        /* reboot at this time */
int copyover_time   = 0;        /* copyover at this time */
int max_players = 0;            /* max descriptors available */
int last_desc = 0;              /* last descriptor number */
int tics = 0;                   /* for extern checkpointing */
long OutBytes = 0;              /* total bytes sent */
long InBytes = 0;               /* total bytes received */
int mins_since_crashsave = 0;

/*
 * db.cc
 */
#if defined(CIRCLE_MACINTOSH)
long beginning_of_time = -1561789232;
#else
long beginning_of_time = 107283540;
#endif

int room_nr = 0;
int zone_nr = 0;


struct room_data *world = NULL; /* array of rooms                */
room_rnum top_of_world = -1;     /* ref to top element of world   */

struct char_data *character_list = NULL;        /* global linked list of
                                                                 * chars         */
long max_id = MOBOBJ_ID_BASE;   /* for unique mob/obj id's       */

struct index_data *mob_index;   /* index table for mobile file   */
struct Mobile *mob_proto;    /* prototypes for mobs           */
mob_rnum top_of_mobt = -1;       /* top of mobile index table     */

struct obj_data *object_list = NULL;    /* global linked list of objs    */
struct index_data *obj_index;   /* index table for object file   */
struct obj_data *obj_proto;     /* prototypes for objs           */
obj_rnum top_of_objt = -1;       /* top of object index table     */

struct zone_data *zone_table;   /* zone table                    */
zone_rnum top_of_zone_table = 0;/* top element of zone tab       */
int count_zones[NUM_ZONES];
struct set_items *set_table;
int top_of_sets = 0;
struct player_index_element *player_table = NULL;       /* index to plr file     */
int top_of_p_table = 0;         /* ref to top of table           */
int top_of_p_file = 0;          /* ref of size of p file         */
long top_idnum = 0;             /* highest idnum in use          */

int no_mail = 0;                /* mail disabled?                */
time_t boot_time = 0;           /* time of mud boot              */
room_rnum r_mortal_start_room;  /* rnum of mortal start room     */
room_rnum r_immort_start_room;  /* rnum of immort start room     */
room_rnum r_frozen_start_room;  /* rnum of frozen start room     */
room_rnum r_helled_start_room;
room_rnum r_jail_start_room;
room_rnum r_named_start_room;
room_rnum r_unreg_start_room;

std::vector<struct bug_abuse_data> bug_abuse;

struct time_info_data time_info;/* the infomation about the time    */
struct weather_data weather_info;       /* the infomation about the weather */
struct reset_q_type reset_q;    /* queue of zones to be reset    */
int zone_update_timer = 0;

int now_entrycount = false;

int imbs = 0; /* currently booting mobile */
int iobj = 0; /* currently booting object */

/*
 * events.cc
 */
struct event_item_data *events_list = NULL;

/*
 * fight.cc
 */
struct char_data *combat_list = NULL; /* head of l-list of fighting chars */
struct char_data *next_combat_list = NULL;
int in_use = 0;

/*
 * gameconfig.cc
 */
/* TODO recalc real start rooms*/

/*
 * mail.cc
 */
mail_index_type *mail_index = NULL; /* list of recs in the mail file  */
position_list_type *free_list = NULL; /* list of free positions in file */
long file_end_pos = 0;   /* length of file */

/*
 * mobmax.cc
 */
int num_mob_lev[MAX_MOB_LEVEL+1];

/*
 * spec_proc.cc
 */
/* TODO reload horse shops */

/*
 * spell_parser.cc
 */
struct skill_info_type   skill_info[TOP_SKILL_DEFINE + 1];
// внутренние номера некоторых популярных заклинаний
int spellnum_db;

/*
 * xmaterials.cc
 */
struct material_data *material_table = NULL;
/*
 * xscripts.cc
 */
vars_storage global_vars; /* Список глобальных переменных */

/*
 * constants.cc
 */
/*
0 - сила
1 - тело
2 - ловкость
3 - интелект
4 - разум
5 - обаяние
6 - здоровье
7 - мана
8 - атака
9 - защита
*/
float add_classes[][13] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //маг
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //воин
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //вор
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //охот
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //жрец
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //некромант
};


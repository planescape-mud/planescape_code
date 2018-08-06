#ifndef MUD_UTILS_H
#define MUD_UTILS_H
/* ************************************************************************
*   File: utils.h                                       Part of CircleMUD *
*  Usage: header file: utility macros and prototypes of utility funcs     *                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#include "logstream.h"
#include "interpreter.h"
#include "dl_strings.h"
#include "dl_ctype.h"
#include "dl_math.h"

#include "memutils.h"
#include "gameconfig.h"

/* external declarations and prototypes **********************************/

extern char AltToKoi[];
extern char KoiToAlt[];
extern char WinToKoi[];
extern char KoiToWin[];
extern char KoiToWin2[];
extern char AltToLat[];

#define log                    notice

/* public functions in utils.c */
double getPartyMobXPFull(int playerlvl, int highestlvl, int sumlvls, int moblvl, bool elite,
                         int rest);
void event_mob(struct char_data *ch, struct char_data *victim, int event_type, int arg);
void inc_honor(struct char_data *killer, struct char_data *victim, int total_killers, int maxlevel);
void join_party(struct char_data *leader, struct char_data *ch);
void leave_party(struct char_data *ch);
void die_party(struct char_data *ch);
void change_leader(struct char_data *oleader, struct char_data *nleader, int lead);
void main_gexp(struct char_data *actor, int exp, int type, int noexp, int group);
void inc_exp_script_num(struct char_data *ch, int vnum, int incvalue);
void change_pet_name(struct char_data *ch, struct char_data *cvict);
int get_exp_script_vnum(struct char_data *ch, int vnum);
int max_exp_gain_pc(struct char_data *ch);
int max_exp_hit_pc(struct char_data *ch);
int max_exp_loss_pc(struct char_data *ch);
void go_jail(struct char_data *ch, struct char_data *vict, int time);
bool check_abuse(int id, int owner, int count);
void stop_defence(struct char_data *ch);
bool check_killer_zone(struct char_data *ch);
void char_period_update(struct char_data *ch, int pulse);
void world_period_update(int rnum);
void check_sets(struct char_data *ch);
void clear_mob_specials(struct char_data *mob);
void set_door_code(char *pcode, int nums);
void recalc_mob(struct char_data *mob);
void clean_flee(struct char_data *ch);
void add_flee(struct char_data *ch, struct char_data *flee);
int get_flee(struct char_data *ch, int id);
void ClearPathLabel(void);
void SetPathLabel(int world_count);
int TracePath(int start_world, int &find_world, byte MobSpec, CPathRetDir * ret, int max);
void GetExitTrapMessage(struct room_direction_data *exit, struct P_message &pMess);
void GetObjTrapMessage(struct obj_trap_data *trap, struct P_message &pMess);
void GetEnterTrapMessage(struct room_trap_data *trap, struct P_message &pMess);
void GetForceDirMessage(struct room_forcedir_data *fd, struct P_message &pMess);
int speedy(struct char_data *ch);
void ShowMessage(struct char_data *ch, struct char_data *victim, struct obj_data *weapObj,
                 struct P_message &message, int dam, int hit, int type, int location,
                 struct P_message &addMessage, int adam);
void ShowMessage(struct char_data *ch, struct char_data *victim, struct obj_data *weapObj,
                 struct P_message &message, int dam, int hit, int type, int location);
void ShowHitMessage(struct char_data *ch, struct char_data *victim, struct obj_data *weapObj,
                    struct obj_data *missObj, int dam, int hit, int type, int location);
void del_spaces(char *str);
void appear(struct char_data *ch);
char *string_corrector(char *Buffer);
char *string_corrector_load(char *Buffer);
char *strbraker(const char *BufferIn, int Wide, int);
char *strformat(char *text, int width, int FVir, char topl, char top, char topr, char left,
                char right, char bottoml, char bottom, char bottomr);
void add_mob_command(struct char_data *mob, int no_cmd, int obj_vnum, char *arg, int mob_vnum,
                     char *err, int script, int extract, int tool, int lroom, int linv,
                     char *active, char *active_room, char *to_char, char *to_room, int xscript);
void add_weapon_damage(struct weapon_data *weapon, int type, int min, int max);
struct char_data *get_char_by_id(long id);
int check_wld_damage(int bits, struct char_data *victim);
int level_exp(struct char_data *ch, int level);
int level_exp_mob(int level);
void make_ghola(struct char_data *ch, int type, int level);
struct obj_data *have_lockpick(struct char_data *ch);
struct time_info_data *real_time_passed(time_t t2, time_t t1);
struct time_info_data *mud_time_passed(time_t t2, time_t t1);
char *meter_bar(struct char_data *ch, int current, int max, ubyte pos);
char *meter_barf(struct char_data *ch, int current, int max);

void prune_crlf(char *txt);
int get_followers_num(struct char_data *ch, int type);
void restore_soul(struct char_data *soul, struct obj_data *corpse, bool exp);
void mudlog(const char *str, int type, int level, int file);
void mudlogf(int type, int level, int file, const char *fmt, ...);
void speaklog(const char *str, int level, int file);
int calc_alignment(struct char_data *ch);
int ok_damage_shopkeeper(struct char_data *ch, struct char_data *victim);
bool check_spells_attacked(struct char_data *ch, struct char_data *vict, int stopflag, int far);
int calc_need_improove(struct char_data *ch, int level);
int calc_improove_skill(struct char_data *ch, int skill_no, struct char_data *victim, int tlevel);
int calc_like_skill(struct char_data *ch, struct char_data *victim, int skill_no,
                    std::vector < int >&cit, std::vector < int >&cot);
int calc_like_skill(struct char_data *ch, struct char_data *victim, int skill_no, int prob);
int improove_skill(struct char_data *ch, struct char_data *victim, int tlevel, int skill_no);
int check_portal(int vnum_obj, int rnum_room);
int get_skill_abil(struct char_data *ch, int skill_no);
int sprintbit(bitvector_t vektor, const char *names[], char *result, const char *div);
void sprintbits(struct new_flag flags, const char *names[], char *result, const char *div);
void sprinttype(int type, const char *names[], char *result);
int get_line(FILE * fl, char *buf);
int get_filename(const char *orig_name, char *filename, int mode);
struct time_info_data *age_old(struct char_data *ch);
int age(struct char_data *ch);
int num_pc_in_room(struct room_data *room);
int num_all_in_room(struct room_data *room);
void core_dump_real(const char *, int);
int replace_str(char **string, char *pattern, char *replacement, int rep_all, int max_size);
void koi_to_alt(char *str, int len);
void koi_to_win(char *str, int len);
void koi_to_winz(char *str, int len);
void format_text(char **ptr_string, int mode, struct descriptor_data *d, int maxlen);
int check_moves(struct char_data *ch, int how_moves);
void to_koi(char *str, int);
void from_koi(char *str, int);
int real_sector(int room);
int check_class(struct char_data *ch, int nclass);
int get_max_class(struct char_data *ch);
int get_max_levelclass(struct char_data *ch);
int get_extra_class(struct char_data *ch);
int check_ip(char *ip);
void add_ip_table(char *ip, long timer);
void add_class(struct char_data *ch, int nclass, int level, int favorite);
void del_class(struct char_data *ch, int nclass);
void add_message(struct char_data *ch, int no_command, int stopflag, int script,
                 char *sarg,
                 char *mess_to_char, char *mess_to_vict, char *mess_to_other, char *mess_to_room);
void add_message(struct room_data *room, int no_command, int stopflag, int script,
                 char *mess_to_char, char *mess_to_vict, char *mess_to_other, char *mess_to_room);
void add_message(struct obj_data *obj, int no_command, int stopflag, int script,
                 char *mess_to_char, char *mess_to_vict, char *mess_to_other, char *mess_to_room);
int get_sphere(char *name);
int get_position(char *name);
int get_extralevel(char *name);
int equip_in_metall(struct char_data *ch);
int get_const_obj_temp(struct obj_data *obj);
void do_mob_cast(struct char_data *ch, struct char_data *victim, int spellnum);
int check_spell_mob(struct char_data *ch, int spellnum);
long get_dsu_exp(struct char_data *ch);
long get_levelexp(struct char_data *ch, int level, int add);
long get_level_mob(struct char_data *mob, int level);
void add_death_obj(struct char_data *mob, int vnum, int perc);
void check_position(struct char_data *ch);
char *ascii_time(time_t t);
struct char_data *random_victim(struct char_data *ch, struct char_data *vict, int attacktype);
void add_obj_visible(struct obj_data *obj, struct char_data *ch);
bool check_obj_visible(const struct char_data *ch, const struct obj_data *obj);
bool check_tfind_char(struct char_data *ch, struct room_direction_data *exit);
bool check_tfind_char(struct char_data *ch, struct obj_data *obj);
void add_tfind_char(struct char_data *ch, struct room_direction_data *exit);
void add_psphere_char(struct char_data *ch, struct room_data *room);
void add_tfind_char(struct char_data *ch, struct obj_data *obj);
void add_victim_visible(struct char_data *ch, struct char_data *victim);
int check_victim_visible(struct char_data *ch, struct char_data *victim);
void add_victim_not_attack(struct char_data *ch, struct char_data *victim);
bool check_victim_not_attack(struct char_data *ch, struct char_data *victim);
bool check_psphere_char(struct char_data *ch, struct room_data *room);
void add_victim_not_moved(struct char_data *ch, struct char_data *victim);
bool check_victim_not_moved(struct char_data *ch, struct char_data *victim);
bool check_victim_may_attack(struct char_data *ch, struct char_data *victim);
bool check_victim_may_moved(struct char_data *ch, struct char_data *victim);
bool check_toroom_repulsion(struct char_data *ch, int in_room);
bool distract_psphere(struct room_data *room, long bitvector, int damage);
bool distract_magic_parry(struct char_data *ch, int damage);
bool distract_bones_wall(struct char_data *ch, int damage);
void horse_master_change(struct char_data *ch, struct char_data *horse);
int is_spell_type(int spell_no);
int get_skill_class_level(struct char_data *ch, int skill_no);
void check_enter_trap(struct char_data *ch, int dir);
void add_inv_obj(struct char_data *mob, int vnum, int perc);
void add_eq_obj(struct char_data *mob, int vnum, int pos);
void add_tatoo_obj(struct char_data *mob, int vnum, int pos);
void add_shop_type(struct char_data *mob, int type);
void add_shop_obj(struct char_data *mob, int vnum, int count);
void del_shop_obj(struct char_data *mob, int vnum, int count);
void add_spechit(struct char_data *mob, byte type, int hit, int spell, int pos[POS_NUMBER],
                 ubyte dnd, ubyte dsd, int dam, ubyte percent, int power, const char *to_victim,
                 const char *to_room, int saves[NUM_SAV], char *property);
void add_materials_proto(struct obj_data *obj);
void add_material(struct obj_data *obj, int type, int value, int main);
void add_last_damage(struct char_data *ch, struct char_data *victim, int number,
                     struct P_damage &damage, struct P_message &message);
char *get_material_name(int type, int pad);
struct material_data *get_material_param(int type);
int is_metall(struct obj_data *obj);
int equip_in_metall(struct char_data *ch);
int get_durab_obj(struct obj_data *obj);
int get_save_obj(struct obj_data *obj, int save);
void add_container_obj(struct obj_data *obj, int vnum, int count);
int check_fight_command(struct char_data *ch);
int check_transport_command(struct char_data *ch);
void tascii(int *pointer, int num_planes, char *ascii);
char *name_infra(const struct char_data *ch, int pad);
int get_sex_infra(const struct char_data *ch);
struct obj_data *SortObjects(struct obj_data *obj, int type);
void create_fragments(struct obj_data *object);
int get_main_material_type(struct obj_data *obj);
void stop_guarding(struct char_data *ch);
int check_memory(struct char_data *ch, int vnum_room);
bool add_memory(struct char_data *ch, int vnum_room, long time, const char *desc);
bool del_memory(struct char_data *ch, int pos);
int create_skelet(struct obj_data *corpse);
void mob_alarm(struct char_data *victim, struct char_data *ch);
void plague_to_char(struct char_data *ch, struct char_data *victim, int modifier);
void preplague_to_char(struct char_data *ch, struct char_data *victim, int modifier);
void immplague_to_char(struct char_data *ch);
const char *get_pers_name(struct char_data *ch, struct char_data *vict, int pad);
void add_distance(struct char_data *ch, struct char_data *victim, int distance, bool show);
int check_distance(struct char_data *ch, struct char_data *victim);
void clean_distance(struct char_data *ch);
void recalc_params(struct char_data *ch);
void recalc_realtime(struct char_data *ch);
void add_missile_damage(struct missile_data *missile, int typ, int min, int max);
struct obj_data *get_missile(struct char_data *ch, struct obj_data *weapObj);
void add_missed(struct char_data *ch, struct char_data *victim);
int get_missed(struct char_data *ch, struct char_data *victim);
int count_mob_vnum(long n);
int count_obj_vnum(long n);

#define core_dump()             core_dump_real(__FILE__, __LINE__)

/* undefine MAX and MIN so that our functions are used instead */
#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

#define SF_EMPTY       (1 << 0)
#define SF_FOLLOWERDIE (1 << 1)
#define SF_MASTERDIE   (1 << 2)
#define SF_CHARMLOST   (1 << 3)

#define SKL_DEX 1
#define SKL_INT 2
#define SKL_WIS 3
#define SKL_CON 4
#define SKL_STR 5
#define SKL_CHA 6
#define SKL_SIZE 7

#define SORT_NAME    0
#define SORT_NUMBER  1

#define EXP_IMPL 100000000
#define CHAR_DRUNKED               10*SECS_PER_MUD_TICK
#define CHAR_MORTALLY_DRUNKED      16*SECS_PER_MUD_TICK

#define MAX(a, b) ((a) > (b) ? (a): (b))
#define MIN(a, b) ((a) < (b) ? (a): (b))
void PHRASE(char *arg);
char *CAP(const char *txt);
char *DAP(const char *txt);

//#define KtoW(c) ((ubyte)(c) < 128 ? (c) : KoiToWin[(ubyte)(c)-128])

#define KtoW(c) ((ubyte)(c) < 128 ? (c) : KoiToWin[(ubyte)(c) & 0x7F])

#define KtoW2(c) ((ubyte)(c) < 128 ? (c) : KoiToWin2[(ubyte)(c)-128])
#define KtoA(c) ((ubyte)(c) < 128 ? (c) : KoiToAlt[(ubyte)(c)-128])
//#define WtoK(c) ((ubyte)(c) < 128 ? (c) : WinToKoi[(ubyte)(c)-128])

#define WtoK(c) ((ubyte)(c) < 128 ? (c) : WinToKoi[(ubyte)(c) & 0x7F])

#define AtoK(c) ((ubyte)(c) < 128 ? (c) : AltToKoi[(ubyte)(c)-128])
#define AtoL(c) ((ubyte)(c) < 128 ? (c) : AltToLat[(ubyte)(c)-128])



/* various constants *****************************************************/

/* defines for mudlog() */
#define OFF     0
#define BRF     1
#define NRM     2
#define CMP     3

/* get_filename() */
#define CRASH_FILE        0
#define ETEXT_FILE        1
#define ALIAS_FILE        2
#define SCRIPT_VARS_FILE  3
#define PLAYERS_FILE      4
#define PKILLERS_FILE     5
#define XPLAYERS_FILE     6
#define PMKILL_FILE       7
#define TEXT_CRASH_FILE   8
#define TIME_CRASH_FILE   9
#define LOGGER_FILE   10
#define PET_FILE      11
#define QUEST_FILE    12

/* breadth-first searching */
#define BFS_ERROR                   -1
#define BFS_ALREADY_THERE       -2
#define BFS_NO_PATH                 -3

/*
 * XXX: These constants should be configurable. See act.informative.c
 *      and utils.c for other places to change.
 */
/* mud-life time */
#define HOURS_PER_DAY          24
#define DAYS_PER_MONTH         28
#define MONTHS_PER_YEAR        14
#define SECS_PER_PLAYER_AFFECT 1
#define TIME_KOEFF             3
#define SECS_PER_MUD_ROUND 3
#define SECS_PER_MUD_TICK          60
#define SECS_PER_MUD_HOUR    6 * SECS_PER_MUD_TICK
#define SECS_PER_MUD_DAY           (HOURS_PER_DAY*SECS_PER_MUD_HOUR)
#define SECS_PER_MUD_MONTH         (DAYS_PER_MONTH*SECS_PER_MUD_DAY)
#define SECS_PER_MUD_YEAR          (MONTHS_PER_YEAR*SECS_PER_MUD_MONTH)
#define NEWMOONSTART               27
#define NEWMOONSTOP                1
#define HALFMOONSTART              7
#define FULLMOONSTART              13
#define FULLMOONSTOP               15
#define LASTHALFMOONSTART          21
#define MOON_CYCLE                                 28
#define WEEK_CYCLE                                 7
#define POLY_WEEK_CYCLE                    9


/* real-life time (remember Real Life?) */
#define SECS_PER_REAL_MIN       60
#define SECS_PER_REAL_HOUR      (60*SECS_PER_REAL_MIN)
#define SECS_PER_REAL_DAY       (24*SECS_PER_REAL_HOUR)
#define SECS_PER_REAL_YEAR      (365*SECS_PER_REAL_DAY)


#define WEATHER(i)  zone_table[i].weather_info
#define GET_COMMSTATE(ch)      (IS_NPC(ch) ? 0 : (ch)->pc()->specials.saved.Prelimit)
#define SET_COMMSTATE(ch,val)  ((ch)->pc()->specials.saved.Prelimit = (val))
#define IS_HERO(ch)             (!IS_NPC(ch) && (GET_LEVEL(ch) == LVL_IMMORT || GET_COMMSTATE(ch)))
#define IS_IMMORTAL(ch)     (!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_IMMORT || GET_COMMSTATE(ch)))
#define IS_GOD(ch)      (!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_GOD || GET_COMMSTATE(ch)))
#define IS_HIGOD(ch)          (!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_HIGOD    || GET_COMMSTATE(ch)))
#define IS_GRGOD(ch)        (!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_GRGOD  || GET_COMMSTATE(ch)))
#define IS_IMPL(ch)         (!IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_IMPL   || GET_COMMSTATE(ch)))

#define IS_BITS(mask, bitno) (IS_SET(mask,(1 << bitno)))
#define IS_CASTER(ch)        (IS_BITS(MASK_CASTER,GET_CLASS(ch)))
#define IS_MAGE(ch)          (IS_BITS(MASK_MAGES, GET_CLASS(ch)))
#define MAY_LIKES(ch)   ((!AFF_FLAGGED(ch, AFF_CHARM) || AFF_FLAGGED(ch, AFF_HELPER)) && \
                         AWAKE(ch) && GET_WAIT(ch) <= 0)


/* string utils **********************************************************/


#define YESNO(a) ((a) ? "Да" : "Нет")
#define ONOFF(a) ((a) ? "Вкл" : "Выкл")

#define LOWER(c)   (dl_tolower(c))
#define UPPER(c)   (dl_toupper(c))

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')


/* basic bitvector utils *************************************************/


#define IS_SET(flag,bit)  ((flag & 0x3FFFFFFF) & (bit))
#define SET_BIT(var,bit)  ((var) |= (bit & 0x3FFFFFFF))
#define REMOVE_BIT(var,bit)  ((var) &= ~(bit & 0x3FFFFFFF))
#define TOGGLE_BIT(var,bit) ((var) = (var) ^ (bit & 0x3FFFFFFF))

#define MOB_FLAGS(ch,flag)  (GET_FLAG((ch)->char_specials.saved.act,flag))
#define PLR_FLAGS(ch,flag)  (GET_FLAG((ch)->char_specials.saved.act,flag))
#define PRF_FLAGS(ch,flag)  (GET_FLAG((ch)->pc()->specials.saved.pref, flag))
#define AFF_FLAGS(ch,flag)  (GET_FLAG((ch)->char_specials.saved.affected_by, flag))
#define NPC_FLAGS(ch,flag)  (GET_FLAG((ch)->npc()->specials.npc_flags, flag))
#define GET_MAX_FACTOR(ch)  (ch->npc()->specials.MaxFactor)
#define EXTRA_FLAGS(ch,flag)(GET_FLAG((ch)->Temporary, flag))
#define ROOM_FLAGS(loc,flag)(GET_FLAG(world[(loc)].room_flags, flag))
#define DOOR_FLAGS(exit,flag)  (GET_FLAG((exit)->exit_data, flag))
#define TRANSP_FLAGS(obj,flag) (GET_FLAG((obj)->transpt->transp_flag,flag))
#define DESC_FLAGS(d)   ((d)->options)
#define GET_MOVE_TYPE(ch) ((ch)->npc()->specials.move_type)
#define GET_MOVE_STR(ch) ((ch)->npc()->specials.move_str)


/* See http://www.circlemud.org/~greerga/todo.009 to eliminate MOB_ISNPC. */
#define IS_NPC(ch)              (IS_SET(MOB_FLAGS(ch, MOB_ISNPC), MOB_ISNPC))
#define IS_MOB(ch)          (IS_NPC(ch) && GET_MOB_RNUM(ch) >= 0)
#define IS_SHOPKEEPER(ch) (IS_MOB(ch) && mob_index[GET_MOB_RNUM(ch)].func == shoper)

#define MOB_FLAGGED(ch, flag)   (IS_NPC(ch) && IS_SET(MOB_FLAGS(ch,flag), (flag)))
#define PLR_FLAGGED(ch, flag)   (!IS_NPC(ch) && IS_SET(PLR_FLAGS(ch,flag), (flag)))
#define AFF_FLAGGED(ch, flag)   (IS_SET(AFF_FLAGS(ch,flag), (flag)))
#define PRF_FLAGGED(ch, flag)   (!IS_NPC(ch) && IS_SET(PRF_FLAGS(ch,flag), (flag)))
#define NPC_FLAGGED(ch, flag)   (IS_SET(NPC_FLAGS(ch,flag), (flag)))
#define EXTRA_FLAGGED(ch, flag) (IS_SET(EXTRA_FLAGS(ch,flag), (flag)))
#define ROOM_FLAGGED(loc, flag) (IS_SET(ROOM_FLAGS((loc),(flag)), (flag)))
#define DOOR_FLAGGED(exit,flag) (IS_SET(DOOR_FLAGS((exit),(flag)), (flag)))
#define TRANSP_FLAGGED(obj,flag) (IS_SET(TRANSP_FLAGS((obj),(flag)), (flag)))
#define OBJVAL_FLAGS(obj,flag)     (GET_FLAG((obj)->bflag, flag))
#define OBJVAL_FLAGGED(obj, flag)    (IS_SET(OBJVAL_FLAGS((obj),(flag)), (flag)))
#define OBJWEAR_FLAGGED(obj, flag)   (IS_SET((obj)->obj_flags.wear_flags, (flag)))
#define DESC_FLAGGED(d, flag) (IS_SET(DESC_FLAGS(d), (flag)))
#define OBJ_FLAGGED(obj, flag)       (IS_SET(GET_OBJ_EXTRA(obj,flag), (flag)))
#define IS_BURIED(obj)        (OBJ_FLAGGED(obj, ITEM_BURIED))
#define IS_FLY(ch)                   (AFF_FLAGGED(ch,AFF_FLY) && GET_POS(ch) == POS_FLYING)
#define SET_EXTRA(ch,skill,vict)   ({(ch)->extra_attack.used_skill = skill; \
        (ch)->extra_attack.victim     = vict;})
#define GET_EXTRA_SKILL(ch)          ((ch)->extra_attack.used_skill)
#define GET_EXTRA_VICTIM(ch)         ((ch)->extra_attack.victim)
#define SET_CAST(ch,snum,dch,dobj)   ({(ch)->cast_attack.spellnum  = snum; \
        (ch)->cast_attack.tch       = dch; \
        (ch)->cast_attack.tobj      = dobj;})
#define GET_CAST_SPELL(ch)         ((ch)->cast_attack.spellnum)
#define GET_CAST_CHAR(ch)          ((ch)->cast_attack.tch)
#define GET_CAST_OBJ(ch)           ((ch)->cast_attack.tobj)
#define GET_FSKILL(ch)  ((ch)->fskill)

/* IS_AFFECTED for backwards compatibility */
#define IS_AFFECTED(ch, skill) (AFF_FLAGGED(ch, skill))

#define PLR_TOG_CHK(ch,flag) ((TOGGLE_BIT(PLR_FLAGS(ch, flag), (flag))) & (flag))
#define PRF_TOG_CHK(ch,flag) ((TOGGLE_BIT(PRF_FLAGS(ch, flag), (flag))) & (flag))
#define PRF_REMOVE_CHK(ch,flag) (REMOVE_BIT(PRF_FLAGS(ch, flag), (flag)))
#define PRF_SET_CHK(ch,flag) (SET_BIT(PRF_FLAGS(ch, flag), (flag)))


/* room utils ************************************************************/

//ADD BY SLOWN
#define RM_BLOOD(rm)   world[rm].blood

#define SECT(room)      (world[(room)].sector_type)
#define GET_SPELLHAUNT(room) (world[(room)].spellhaunt)
#define GET_ROOM_SKY(room) (world[room].weather.duration > 0 ? \
                            world[room].weather.sky : weather_info.sky)

#define IS_MOONLIGHT(room) ((GET_ROOM_SKY(room) == SKY_LIGHTNING && \
                             zone_table[world[(room)].zone].weather_info.moon_day >= FULLMOONSTART && \
                             zone_table[world[(room)].zone].weather_info.moon_day <= FULLMOONSTOP))

#define IS_TIMEDARK(room)  ((world[room].gdark > world[room].glight) || \
                            (!(world[room].light+world[room].fires+world[room].glight) && \
                             (ROOM_FLAGGED(room, ROOM_DARK) || \
                              (SECT(room) != SECT_INSIDE && \
                               ( zone_table[world[(room)].zone].weather_info.sunlight == SUN_SET || \
                                 zone_table[world[(room)].zone].weather_info.sunlight == SUN_DARK )) ) ) )

#define IS_DARK_CHECK(room)  ((!(world[room].light+world[room].fires+world[room].glight) && \
                               ROOM_FLAGGED(room,ROOM_DARK)))

#define IS_DEFAULTDARK(room) (ROOM_FLAGGED(room, ROOM_DARK) || \
                              (SECT(room) != SECT_INSIDE && \
                               ( zone_table[world[(room)].zone].weather_info.sunlight == SUN_SET || \
                                 zone_table[world[(room)].zone].weather_info.sunlight == SUN_DARK )) )

#define IS_CLOUD(room) ((world[room].weather.sky == SKY_RAINING) && \
                        (zone_table[world[room].zone].plane == PLAN_SIGIL) && \
                        (SECT(room) != SECT_INSIDE) &&\
                        !(world[room].light+world[room].fires))

#define IS_LCLOUD(room) ((world[room].weather.sky == SKY_CLOUDY) && \
                         (zone_table[world[room].zone].plane == PLAN_SIGIL) && \
                         (SECT(room) != SECT_INSIDE) &&\
                         !(world[room].light+world[room].fires))

#define IS_VLCLOUD(room) ((world[room].weather.sky == SKY_CLOUDLESS) && \
                          (zone_table[world[room].zone].plane == PLAN_SIGIL) && \
                          (SECT(room) != SECT_INSIDE) &&\
                          !(world[room].light+world[room].fires))

#define IS_CLOUD_NL(room) ((world[room].weather.sky == SKY_RAINING) && \
                           (SECT(room) != SECT_INSIDE) &&\
                           (zone_table[world[room].zone].plane == PLAN_SIGIL))

#define IS_LCLOUD_NL(room) ((world[room].weather.sky == SKY_CLOUDY) && \
                            (SECT(room) != SECT_INSIDE) &&\
                            (zone_table[world[room].zone].plane == PLAN_SIGIL))

#define IS_VLCLOUD_NL(room) ((world[room].weather.sky == SKY_CLOUDLESS) && \
                             (SECT(room) != SECT_INSIDE) &&\
                             (zone_table[world[room].zone].plane == PLAN_SIGIL))

#define IS_DARK(room)      ((world[room].gdark > world[room].glight) || \
                            (!(world[room].gdark < world[room].glight) && \
                             !(world[room].light+world[room].fires) && \
                             (ROOM_FLAGGED(room, ROOM_DARK) || \
                              (SECT(room) != SECT_INSIDE && !ROOM_FLAGGED(room, ROOM_INDOORS) &&\
                               ( zone_table[world[(room)].zone].weather_info.sunlight == SUN_SET || \
                                 (zone_table[world[(room)].zone].weather_info.sunlight == SUN_DARK && \
                                  !IS_MOONLIGHT(room)) )) ) ) )

#define IS_DARKTHIS(room)      ((world[room].gdark > world[room].glight) || \
                                (!(world[room].gdark < world[room].glight) && \
                                 !(world[room].light+world[room].fires) && \
                                 (ROOM_FLAGGED(room, ROOM_DARK) || \
                                  (SECT(room) != SECT_INSIDE && \
                                   (zone_table[world[(room)].zone].weather_info.sunlight == SUN_DARK && \
                                    !IS_MOONLIGHT(room) )) ) ) )

#define IS_DARKSIDE(room)      ((world[room].gdark > world[room].glight) || \
                                (!(world[room].gdark < world[room].glight) && \
                                 !(world[room].light+world[room].fires) && \
                                 (ROOM_FLAGGED(room, ROOM_DARK) || \
                                  (SECT(room) != SECT_INSIDE && \
                                   ( zone_table[world[(room)].zone].weather_info.sunlight == SUN_SET  || \
                                     zone_table[world[(room)].zone].weather_info.sunlight == SUN_RISE || \
                                     (zone_table[world[(room)].zone].weather_info.sunlight == SUN_DARK && \
                                      !IS_MOONLIGHT(room) )) ) ) )


#define IS_LIGHT(room)     (!IS_DARK(room))

#define VALID_RNUM(rnum)        ((rnum) >= 0 && (rnum) <= top_of_world)
#define GET_ROOM_VNUM(rnum) \
    ((room_vnum)(VALID_RNUM(rnum) ? world[(rnum)].number : NOWHERE))
#define GET_ROOM_SPEC(room) (VALID_RNUM(room) ? world[(room)].func : NULL)

/* char utils ************************************************************/

#define IS_MANA_CASTER(ch) (check_class((ch),CLASS_MAGIC_USER) || check_class((ch),CLASS_PRIEST) || check_class((ch),CLASS_NECRO))
#define IN_ROOM(ch)     ((ch)->in_room)
#define GET_ZONE(ch) zone_table[world[IN_ROOM(ch)].zone]
#define GET_WAS_IN(ch)  ((ch)->was_in_room)
#define GET_AGE(ch)     (age_old(ch)->year)
#define GET_REAL_AGE(ch) (GET_AGE(ch) + GET_AGE_ADD(ch))

#define GET_HONOR(ch) ((ch)->player.honor)
#define GET_PC_NAME(ch) ((ch)->player.name)
#define GET_NAME(ch)    (IS_NPC(ch) ? \
                         (ch)->player.short_descr : GET_PC_NAME(ch))
#define GET_NAMES(ch)    ((ch)->player.names)
#define GET_TITLE(ch)   ((ch)->player.title)
#define GET_RTITLE(ch)   ((ch)->player.title_r)
#define GET_FRACTION(ch) ((ch)->player.fraction)
#define GET_FRANK(ch)    ((ch)->player.rank)
#define GET_LEVEL(ch)   ((ch)->player.level)
#define GET_LEVEL_ADD(ch)   ((ch)->add_abils.level_add)
#define GET_FACT_LEVEL(ch) ((ch)->add_abils.fact_level)
#define GET_MANA(ch)          ((ch)->ManaMemStored)
#define GET_BASE_MANA(ch) (GET_REAL_INT(ch)*1.75)
#define GET_BASE_SLOT(ch) (GET_REAL_INT(ch)/10)
#define GET_MAX_MANA(ch)       ((ch)->ManaMemNeeded)
#define GET_MANA_ADD(ch)    ((ch)->add_abils.mana_add)
#define GET_REAL_MAX_MANA(ch) (GET_MAX_MANA(ch) + GET_MANA_ADD(ch))
#define GET_MANA_COST(ch,spellnum) (SPELL_MANA(spellnum))
#define GET_MOVE_COST(ch,spellnum) (SPELL_MANA(spellnum)/15)
#define GET_SPEED(ch)  (ch->speed)
#define GET_SPEED_ADD(ch) ((ch)->add_abils.speed_add)
#define GET_REAL_SPEED(ch)        (speedy(ch)+GET_SPEED_ADD(ch))
#define BATTLECNTR(ch)        ((ch)->BattleCounter)
#define EXTRACT_TIMER(ch)     ((ch)->ExtractTimer)
#define CLR_MEMORY(ch)  (memset((ch)->Memory,0,MAX_SPELLS+1))
#define FORGET_ALL(ch) ({CLR_MEMORY(ch);memset((ch)->real_abils.SplMem,0,MAX_SPELLS+1);})
#define GET_PASSWD(ch)   ((ch)->player.passwd)
#define GET_PFILEPOS(ch) ((ch)->pfilepos)
#define IS_KILLER(ch)    ((ch)->points.pk_counter)
#define GET_FICTION(ch)  ((ch)->fiction)
#define GET_FICTION_PAGE(ch)  ((ch)->fiction_page)
#define CHECK_AGRO(ch,vict)   \
    (MOB_FLAGGED(ch, MOB_AGGRESSIVE)    || \
     (MOB_FLAGGED(ch, MOB_AGGRGOOD) && IS_GOODS(vict)) || \
     (MOB_FLAGGED(ch, MOB_AGGREVIL) && IS_EVILS(vict)) || \
     (MOB_FLAGGED(ch, MOB_AGGRNEUTRAL) && IS_NEUTRAL(vict)) || \
     (MOB_FLAGGED(ch, MOB_AGGR_DAY) && \
      (zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_RISE || \
       zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_LIGHT)) || \
     (MOB_FLAGGED(ch, MOB_AGGR_NIGHT) && \
      (zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_DARK || \
       zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_SET)))


/* (MOB_FLAGGED(ch, MOB_AGGR_WINTER) && zone_table[world[IN_ROOM(ch)].zone].weather_info.season == SEASON_WINTER) || \
 (MOB_FLAGGED(ch, MOB_AGGR_SPRING) && zone_table[world[IN_ROOM(ch)].zone].weather_info.season == SEASON_SPRING) || \
 (MOB_FLAGGED(ch, MOB_AGGR_SUMMER) && zone_table[world[IN_ROOM(ch)].zone].weather_info.season == SEASON_SUMMER)  || \
 (MOB_FLAGGED(ch, MOB_AGGR_AUTUMN) && zone_table[world[IN_ROOM(ch)].zone].weather_info.season == SEASON_AUTUMN)) */


#define GET_LASTTELL(ch) ((ch)->pc()->specials.saved.lasttell)
#define GET_TELL(ch,i)   ((ch)->pc()->specials.saved.remember)[i]
#define GET_MEMORY(ch,i) ((ch)->char_specials.memory[(i)].vnum_room)
#define GET_TIMEMEM(ch,i) ((ch)->char_specials.memory[(i)].times)
#define GET_LASTMEM(ch)         ((ch)->char_specials.lastmemory)
#define GET_DESCMEM(ch,i) ((ch)->char_specials.memory[(i)].desc)
//#define GET_MAXMEM(ch)  (MAX(1,GET_REAL_WIS(ch)/3))
#define GET_MAXMEM(ch)  10
#define GET_MAXTIME(ch)  (MAX(1440,GET_REAL_INT(ch)*2880))

/**** Adding by me */
#define GET_AF_BATTLE(ch,flag) (IS_SET(GET_FLAG((ch)->BattleAffects, flag),flag))
#define SET_AF_BATTLE(ch,flag) (SET_BIT(GET_FLAG((ch)->BattleAffects,flag),flag))
#define CLR_AF_BATTLE(ch,flag) (REMOVE_BIT(GET_FLAG((ch)->BattleAffects, flag),flag))
#define NUL_AF_BATTLE(ch)      (GET_FLAG((ch)->BattleAffects, 0) = \
                                GET_FLAG((ch)->BattleAffects, INT_ONE) = \
                                        GET_FLAG((ch)->BattleAffects, INT_TWO) = \
                                                GET_FLAG((ch)->BattleAffects, INT_THREE) = 0)
#define GET_GLORY(ch)          ((ch)->pc()->specials.saved.glory)
#define GET_EMAIL(ch)          ((ch)->pc()->specials.saved.EMail)
#define GET_LASTIP(ch)         ((ch)->pc()->specials.saved.LastIP)
#define GET_GOD_FLAG(ch,flag)  (IS_SET((ch)->pc()->specials.saved.GodsLike,flag))
#define SET_GOD_FLAG(ch,flag)  (SET_BIT((ch)->pc()->specials.saved.GodsLike,flag))
#define CLR_GOD_FLAG(ch,flag)  (REMOVE_BIT((ch)->pc()->specials.saved.GodsLike,flag))
#define GET_UNIQUE(ch)         ((ch)->pc()->specials.saved.unique)
#define GET_HOUSE_UID(ch)      ((ch)->pc()->specials.saved.HouseUID)
#define GET_HOUSE_RANK(ch)     ((ch)->pc()->specials.saved.HouseRank)
#define LAST_LOGON(ch)         ((ch)->pc()->specials.saved.LastLogon)

#define GODS_DURATION(ch)  ((ch)->pc()->specials.saved.GodsDuration)
#define MUTE_DURATION(ch)  ((ch)->pc()->specials.saved.MuteDuration)
#define DUMB_DURATION(ch)  ((ch)->pc()->specials.saved.DumbDuration)
#define FREEZE_DURATION(ch)  ((ch)->pc()->specials.saved.FreezeDuration)
#define HELL_DURATION(ch)  ((ch)->pc()->specials.saved.HellDuration)
#define NAME_DURATION(ch)  ((ch)->pc()->specials.saved.NameDuration)
#define NAME_GOD(ch)  ((ch)->pc()->specials.saved.NameGod)
#define NAME_ID_GOD(ch)  ((ch)->pc()->specials.saved.NameIDGod)

/*
 * I wonder if this definition of GET_REAL_LEVEL should be the definition
 * of GET_LEVEL?  JE
 * по понятным причинам закоментирован
#define GET_REAL_LEVEL(ch) \
   (ch->desc && ch->desc->original ? GET_LEVEL(ch->desc->original) : \
    GET_LEVEL(ch)) */

#define POSI(val)      ((val < 50) ? ((val > 0) ? val : 1) : 50)
#define VPOSI(val,min,max)      ((val < max) ? ((val > min) ? val : min) : max)
#define GET_MISSED(ch) ((ch)->missed_rnd)
#define GET_CLASS(ch)   ((ch)->player.chclass)
#define GET_HLEVEL(ch,x) ((ch)->player.hlevel[(x)])
#define GET_FVCLASS(ch) (GET_HLEVEL(ch,0))
#define GET_GODS(ch) ((ch)->player.gods)
#define GET_HOME(ch)    ((ch)->player.hometown)
#define GET_HEIGHT(ch)  ((ch)->player.height)
#define GET_HEIGHT_ADD(ch) ((ch)->add_abils.height_add)
#define GET_REAL_HEIGHT(ch) (GET_HEIGHT(ch) + GET_HEIGHT_ADD(ch))
#define GET_WEIGHT(ch)  ((ch)->player.weight)
#define GET_WEIGHT_ADD(ch) ((ch)->add_abils.weight_add)
#define GET_REAL_WEIGHT(ch) (GET_WEIGHT(ch) + GET_WEIGHT_ADD(ch))
#define GET_ALL_WEIGHT(ch) (GET_REAL_WEIGHT(ch) + IS_CARRYING_W(ch)+IS_WEARING_W(ch))
#define GET_SEX(ch)     ((ch)->player.sex)
#define GET_LOWS(ch) ((ch)->player.Lows)
#define GET_RELIGION(ch) ((ch)->player.Religion)
#define GET_RACE(ch) ((ch)->player.race)
#define GET_REMORT(ch) ((ch)->player.remort)
#define GET_MOB_TYPE(ch) ((ch)->player.mob_type)
#define GET_MOB_VID(ch) ((ch)->player.mob_vid)
#define GET_IMPROOVE(ch,i) ((ch)->pc()->specials.saved.Improove[i])
#define GET_SIDE(ch) ((ch)->player.Side)
#define GET_PAD(ch,i)    ((ch)->player.PNames[i])
#define GET_DRUNK_STATE(ch) ((ch)->pc()->specials.saved.DrunkState)
#define SPEAKING(ch)        ((ch)->pc()->specials.saved.speaking)
#define GET_SPEAKING(ch) (IS_NPC(ch) ? SKILL_LANG_COMMON : SPEAKING(ch))
#define GET_EYES(ch) ((ch)->eyes)
#define GET_REAL_PRT_SKILL(ch) (GET_LEVEL(ch))
#define GET_COUNT_DEATH(ch) ((ch)->death_count)
#define GET_COUNT_RESSURECT(ch) ((ch)->ress_count)
#define GET_COUNT_CAPSOUL(ch) ((ch)->capsoul_count)



#define GET_STR(ch)     ((ch)->real_abils.str)
#define GET_STR_ADD(ch) ((ch)->add_abils.str_add)
#define GET_STR_ROLL(ch) ((ch)->add_abils.str_roll)
#define GET_REAL_STR(ch) (POSI(GET_STR(ch) + GET_STR_ADD(ch)+GET_STR_ROLL(ch)+GET_REMORT(ch)))
#define GET_DEX(ch)     ((ch)->real_abils.dex)
#define GET_DEX_ADD(ch) ((ch)->add_abils.dex_add)
#define GET_DEX_ROLL(ch) ((ch)->add_abils.dex_roll)
#define GET_REAL_DEX(ch) (POSI(GET_DEX(ch)+GET_DEX_ADD(ch)+GET_DEX_ROLL(ch)+GET_REMORT(ch)))
#define GET_INT(ch)     ((ch)->real_abils.intel)
#define GET_INT_ADD(ch) ((ch)->add_abils.intel_add)
#define GET_INT_ROLL(ch) ((ch)->add_abils.intel_roll)
#define GET_REAL_INT(ch) (POSI(GET_INT(ch) + GET_INT_ADD(ch)+GET_INT_ROLL(ch)+GET_REMORT(ch)))
#define GET_WIS(ch)     ((ch)->real_abils.wis)
#define GET_WIS_ADD(ch) ((ch)->add_abils.wis_add)
#define GET_WIS_ROLL(ch) ((ch)->add_abils.wis_roll)
#define GET_REAL_WIS(ch) (POSI(GET_WIS(ch) + GET_WIS_ADD(ch)+GET_WIS_ROLL(ch)+GET_REMORT(ch)))
#define GET_CON(ch)     ((ch)->real_abils.con)
#define GET_CON_ADD(ch) ((ch)->add_abils.con_add)
#define GET_CON_ROLL(ch) ((ch)->add_abils.con_roll)
#define GET_REAL_CON(ch) (POSI(GET_CON(ch) + GET_CON_ADD(ch) + GET_CON_ROLL(ch)+GET_REMORT(ch)))
#define GET_CHA(ch)     ((ch)->real_abils.cha)
#define GET_CHA_ADD(ch) ((ch)->add_abils.cha_add)
#define GET_CHA_ROLL(ch) ((ch)->add_abils.cha_roll)
#define GET_REAL_CHA(ch) (POSI(GET_CHA(ch) + GET_CHA_ADD(ch) + GET_CHA_ROLL(ch)+GET_REMORT(ch)))
#define GET_SIZE(ch)    ((ch)->real_abils.size)
#define GET_SIZE_ADD(ch)  ((ch)->add_abils.size_add)
#define GET_REAL_SIZE(ch) (VPOSI(GET_SIZE(ch) + GET_SIZE_ADD(ch), 1, 32768))
#define GET_POS_SIZE(ch)  (POSI(GET_REAL_SIZE(ch) >> 1))
#define GET_HR(ch)            ((ch)->real_abils.hitroll)
#define GET_HR_ADD(ch)    ((ch)->add_abils.hr_add)
#define GET_REAL_HR(ch)   (GET_HR(ch)+GET_HR_ADD(ch))
#define GET_DR(ch)            ((ch)->real_abils.damroll)
#define GET_DR_ADD(ch)    ((ch)->add_abils.dr_add)
#define GET_REAL_DR(ch)   (GET_DR(ch)+GET_DR_ADD(ch))
#define GET_AC(ch)            ((ch)->real_abils.armor)
#define GET_AC_ADD(ch)    ((ch)->add_abils.ac_add)
#define GET_AC_WEAR(ch)  ((ch)->add_abils.ac_wear)
#define GET_AC_RT(ch)   ((ch)->realtime_abils.ac_rt)
//для физических атак
#define GET_REAL_AC(ch)      (GET_AC_ADD(ch)+GET_AC_WEAR(ch)+GET_AC_RT(ch))
//для ментальных атак
#define GET_MENTAL_AC(ch)    (GET_AC(ch)+GET_AC_ADD(ch)+GET_AC_RT(ch))
#define GET_REMEMORY(ch)       ((ch)->add_abils.rememory_add)
#define GET_POISON(ch)           ((ch)->add_abils.poison_add)
#define GET_CAST_SUCCESS(ch) ((ch)->add_abils.cast_success)
#define GET_PRAY(ch)         ((ch)->add_abils.pray_add)
#define GET_LCK(ch)     ((ch)->real_abils.lck)
#define GET_LCK_ADD(ch) ((ch)->add_abils.lck_add)
#define GET_REAL_LCK(ch) (POSI(GET_LCK(ch) + GET_LCK_ADD(ch)))
#define GET_HLT(ch)     ((ch)->real_abils.health)
#define GET_HLT_ADD(ch) ((ch)->add_abils.health_add)
#define GET_REAL_HLT(ch) (GET_HLT(ch)+GET_HLT_ADD(ch))

#define IS_MAX_EXP(ch) (GET_EXP(ch) > level_exp(ch, LVL_IMMORT))
#define MAX_EXP(ch) level_exp(ch, LVL_IMMORT))
#define GET_HP_PERC(ch) ((int)(GET_HIT(ch) * 100 / GET_MAX_HIT(ch)))
#define GET_EXP(ch)               ((ch)->points.exp)
#define GET_HIT(ch)               ((ch)->points.hit)
#define GET_MAX_HIT(ch)       ((ch)->points.max_hit)
#define GET_INIT_HIT(ch)       ((ch)->points.init_hit)
#define GET_INIT_MANA(ch)       ((ch)->points.init_mana)
#define GET_INIT_MOVE(ch)       ((ch)->points.init_move)
#define GET_MAX_HIT(ch)       ((ch)->points.max_hit)
#define GET_REAL_MAX_HIT(ch)  (GET_MAX_HIT(ch) + GET_HIT_ADD(ch))
#define GET_MOVE(ch)          ((ch)->points.move)
#define GET_MAX_MOVE(ch)      ((ch)->points.max_move)
#define GET_REAL_MAX_MOVE(ch) (GET_MAX_MOVE(ch) + GET_MOVE_ADD(ch))
#define GET_GOLD(ch)          ((ch)->points.gold)
#define GET_BANK_GOLD(ch)     ((ch)->points.bank_gold)


#define GET_MANAREG(ch)   ((ch)->add_abils.manareg)
#define GET_HITREG(ch)    ((ch)->add_abils.hitreg)
#define GET_MOVEREG(ch)   ((ch)->add_abils.movereg)
#define GET_POWER(ch)   (IS_NPC(ch) ? (ch)->npc()->specials.powered : 0)
#define GET_POWER_ADD(ch) ((ch)->add_abils.powered_add)
#define GET_ARMOUR(ch,x)    ((ch)->add_abils.armour[(x)])
#define GET_AGE_ADD(ch)   ((ch)->add_abils.age_add)
#define GET_HIT_ADD(ch)   ((ch)->add_abils.hit_add)
#define GET_MOVE_ADD(ch)  ((ch)->add_abils.move_add)
#define GET_SAVE3(ch,i)    ((ch)->add_abils.apply_saving_throw_3[i])
#define GET_LIKES(ch)     ((ch)->npc()->specials.LikeWork)
#define GET_INC_MAGIC(ch,i)  ((ch)->add_abils.inc_magic[i])
#define GET_LOAD_ROOM(ch)     ((ch)->npc()->specials.LoadRoom)
// next 7 macroses not used
#define GET_SPEC_TYPE(ch)  ((ch)->npc()->specials.SpecType)
#define GET_SPEC_SKILL(ch)  ((ch)->npc()->specials.SpecSkill)
#define GET_SPEC_DAMROLL(ch)  ((ch)->npc()->specials.SpecDamRoll)
#define GET_SPEC_DAMDICE(ch)  ((ch)->npc()->specials.SpecDamDice)
#define GET_SPEC_DAMSIZE(ch)  ((ch)->npc()->specials.SpecDamSize)
#define GET_SPEC_TOVICT(ch)  ((ch)->npc()->specials.MessToVict)
#define GET_SPEC_TOROOM(ch)  ((ch)->npc()->specials.MessToRoom)


#define GET_POS(ch)           ((ch)->char_specials.position)
#define GET_IDNUM(ch)     ((ch)->char_specials.saved.idnum)
#define GET_ID(x)         ((x)->id)
#define GET_INDEX(ch)     (get_ptable_by_name(GET_NAME(ch)))
#define GET_TICKS(ch)   ((ch)->ticks_in_mud)
#define IS_WEARING_W(ch)  ((ch)->char_specials.wear_weight)
#define IS_CARRYING_W(ch) ((ch)->char_specials.carry_weight)
#define IS_CARRYING_N(ch) ((ch)->char_specials.carry_items)
#define FIGHTING(ch)      ((ch)->char_specials.fighting)
#define HUNTING(ch)       ((ch)->char_specials.hunting)
#define HUNT_NAME(ch)   ((ch)->char_specials.hunt_name)
#define HUNT_STEP(ch)     ((ch)->char_specials.hunt_step)
#define GUARDING(ch)           ((ch)->char_specials.guarding)
#define TOUCHING(ch)      ((ch)->Touching)
#define GET_OBJ_LOCATE(ch) ((ch)->char_specials.obj_locate)
#define GET_CHAR_LOCATE(ch) ((ch)->char_specials.char_locate)

#define ALIG_NEUT 0
#define ALIG_GOOD 1
#define ALIG_EVIL 2
#define ALIG_EVIL_LESS     -500
#define ALIG_GOOD_MORE     500
//ADD SLOWN 05.03.2002
#define ALIG_GA 801             /* абсолютно добрый   */
#define ALIG_GG 601             /* добрый             */
#define ALIG_GN 401             /* добрый-нейтральный */
#define ALIG_NG 201             /* нейтральный-добрый */
#define ALIG_NN 0               /* добрый             */
#define ALIG_NE -201            /* нейтральный-злой   */
#define ALIG_EN -401            /* злой-нейтральный   */
#define ALIG_EE -601            /* злой               */
#define ALIG_EA -801            /* абсолютно злой     */
#define NUM_ALIGN 9

#define CLASS_PRIMER    0
#define CLASS_PLANER    1


#define GET_ALIGNMENT(ch)     ((ch)->char_specials.saved.alignment)

#define OK_GAIN_EXP(ch,victim) (IS_NPC(victim) ? 1 : 0)
#define MAX_EXP_PERCENT   80

#define GET_COND(ch, i)         ((ch)->pc()->specials.saved.conditions[(i)])
#define GET_LOADROOM(ch)        ((ch)->pc()->specials.saved.load_room)
#define GET_INVIS_LEV(ch)       (IS_NPC(ch) ? 0 : (ch)->pc()->specials.saved.invis_level)
#define GET_WIMP_LEV(ch)        (IS_NPC(ch) ? ch->npc()->specials.wimp_level : ((ch)->pc()->specials.saved.wimp_level))
#define GET_FREEZE_LEV(ch)      ((ch)->pc()->specials.saved.freeze_level)
#define GET_BAD_PWS(ch)         ((ch)->pc()->specials.saved.bad_pws)
#define GET_TALK(ch, i)         ((ch)->pc()->specials.saved.talks[i])
#define POOFIN(ch)              ((ch)->pc()->specials.poofin)
#define POOFOUT(ch)             ((ch)->pc()->specials.poofout)
#define RENTABLE(ch)            ((ch)->pc()->specials.may_rent)
#define GET_ALIASES(ch)             ((ch)->pc()->specials.aliases)
#define GET_LAST_TELL(ch)           ((ch)->pc()->specials.last_tell)


#define GET_SKILL_LAGR(ch, i)        ((ch)->real_abils.SkillsLagRound[i])
#define SET_SKILL(ch, i)        ((ch)->real_abils.Skills[i])
#define GET_SKILL(ch, i)        (IS_NPC(ch) ? GET_SKILL_MOB(ch, i) : \
                                 MIN(calc_need_improove((ch),get_skill_class_level((ch),i)) ,(ch)->real_abils.Skills[i]))

#define GET_SKILL_MOB(ch, i)        (((ch)->real_abils.Skills[i] > 0) ? calc_need_improove(ch,GET_LEVEL(ch)) : 0 )
#define GET_SKILL_ADD(ch,i)     ((ch)->add_abils.skills_add[i]) //увеличивает на % умения

//ADDED BY HMEPAS
#define GET_SKILL_LEVEL(ch, i)  ((ch)->real_abils.ExtraSkills[i])
#define SET_SKILL_LEVEL(ch, i, pct)     ((ch)->real_abils.ExtraSkills[i] = pct)


#define GET_SPELL_TYPE(ch, i)   ((ch)->real_abils.SplKnw[i])
#define GET_SPELL_MEM(ch, i)    ((ch)->real_abils.SplMem[i])
#define SET_SPELL(ch, i, pct)   ((ch)->real_abils.SplMem[i] = pct)
#define GET_ENCHANT_TYPE(ch,i) ((ch)->real_abils.EntKnw[i])

#define GET_EQ(ch, i)           ((ch)->equipment[i])
#define GET_TATOO(ch, i)           ((ch)->tatoo[i])

#define GET_MOB_SPEC(ch)        (IS_MOB(ch) ? mob_index[(ch)->nr].func : NULL)
#define GET_MOB_RNUM(mob)       ((mob)->nr)
#define GET_MOB_VNUM(mob)       (IS_MOB(mob) ? \
                                 mob_index[GET_MOB_RNUM(mob)].vnum : -1)

#define GET_DEFAULT_POS(ch)     ((ch)->npc()->specials.default_pos)
#define GET_MOVE_TO(ch) (ch->npc()->specials.move_to)
#define GET_DEST(ch)        (((ch)->npc()->specials.dest_count ? \
                              (ch)->npc()->specials.dest[(ch)->npc()->specials.dest_pos] : \
                              NOWHERE))
#define GET_ACTIVITY(ch)    ((ch)->npc()->specials.activity)
#define GET_GOLD_NoDs(ch)   ((ch)->npc()->specials.GoldNoDs)
#define GET_GOLD_SiDs(ch)   ((ch)->npc()->specials.GoldSiDs)
#define GET_HORSESTATE(ch)  ((ch)->npc()->specials.HorseState)
#define GET_LASTROOM(ch)    ((ch)->npc()->specials.LastRoom)
#define ON_HORSE_FLY(ch) (on_horse(ch) && (AFF_FLAGGED(get_horse_on(ch),AFF_FLY) || MOB_FLAGGED(get_horse_on(ch),MOB_FLYING)))
#define ON_HORSE_SWIM(ch) (on_horse(ch) && MOB_FLAGGED(get_horse_on(ch),MOB_SWIMMING))
#define OFF_HORSE_FLY(ch) (on_horse(ch) && (!AFF_FLAGGED(get_horse_on(ch),AFF_FLY) && !MOB_FLAGGED(get_horse_on(ch),MOB_FLYING)))
#define OFF_HORSE_SWIM(ch) (on_horse(ch) && (!AFF_FLAGGED(get_horse_on(ch),AFF_WATERBREATH) && !MOB_FLAGGED(get_horse_on(ch),MOB_SWIMMING)))

#define STRENGTH_APPLY_INDEX(ch) \
    ( GET_REAL_STR(ch) )

#define AWAKE(ch) (GET_POS(ch) > POS_SLEEPING && !AFF_FLAGGED(ch,AFF_SLEEP))
#define CAN_SEE_IN_DARK(ch) \
    (AFF_FLAGGED(ch, AFF_DARKVISION) || AFF_FLAGGED(ch, AFF_INFRAVISION) || affected_by_spell(ch, SPELL_INFRAVISION) || (!IS_NPC(ch) && (PRF_FLAGGED(ch, PRF_HOLYLIGHT))))

#define CAN_SEE_IN_DARK_ALL(ch) \
    (AFF_FLAGGED(ch, AFF_DARKVISION) || (!IS_NPC(ch) && (PRF_FLAGGED(ch, PRF_HOLYLIGHT))))

#define IS_GOODA(ch)         (GET_ALIGNMENT(ch) >= ALIG_GA)
#define IS_GOOD(ch)          ((GET_ALIGNMENT(ch) < ALIG_GA) && (GET_ALIGNMENT(ch) >= ALIG_GG))
#define IS_GOODN(ch)         ((GET_ALIGNMENT(ch) < ALIG_GG) && (GET_ALIGNMENT(ch) >= ALIG_GN))
#define IS_NEUTRALG(ch)      ((GET_ALIGNMENT(ch) < ALIG_GN) && (GET_ALIGNMENT(ch) >= ALIG_NG))
#define IS_NEUTRAL(ch)       ((GET_ALIGNMENT(ch) < ALIG_NG) && (GET_ALIGNMENT(ch) > ALIG_NE))
#define IS_NEUTRALE(ch)      ((GET_ALIGNMENT(ch) <= ALIG_NE) && (GET_ALIGNMENT(ch) > ALIG_EN))
#define IS_EVILN(ch)         ((GET_ALIGNMENT(ch) <= ALIG_EN) && (GET_ALIGNMENT(ch) > ALIG_EE))
#define IS_EVIL(ch)          ((GET_ALIGNMENT(ch) <= ALIG_EE) && (GET_ALIGNMENT(ch) > ALIG_EA))
#define IS_EVILA(ch)         (GET_ALIGNMENT(ch) <= ALIG_EA)
#define IS_GOODS(ch)        (IS_GOODA(ch) || \
                             IS_GOOD(ch)  || \
                             IS_GOODN(ch))
#define IS_NEUTRALS(ch)     (IS_NEUTRALG(ch) || \
                             IS_NEUTRAL(ch)  || \
                             IS_NEUTRALE(ch))
#define IS_EVILS(ch)        (IS_EVILA(ch) || \
                             IS_EVIL(ch)  || \
                             IS_EVILN(ch))

#define SAME_ALIGN(ch,vict)  ((IS_GOODS(ch) && IS_GOODS(vict)) ||\
                              (IS_EVILS(ch) && IS_EVILS(vict)) ||\
                              (IS_NEUTRALS(ch) && IS_NEUTRALS(vict)))

#define SAME_ALIGN_HELP(ch,vict) ((IS_GOODS(ch) && (IS_GOODS(vict) || IS_NEUTRALS(vict))) || \
                                  (IS_NEUTRAL(ch) && (IS_GOODS(vict) || IS_NEUTRALS(vict) || IS_EVILS(vict))) || \
                                  (IS_EVILS(ch) && IS_EVILS(vict)))

#define SAME_ALIGN_HELPER(ch,vict) ((IS_GOODS(ch) && (IS_GOODS(vict) || IS_NEUTRALS(vict))) || \
                                    (IS_EVILS(ch) && IS_EVILS(vict)) || \
                                    (IS_NEUTRALS(ch) && IS_NEUTRALS(vict)))

#define GET_CH_SUF_1(ch) (GET_SEX(ch) == SEX_NEUTRAL ? "о" :\
                          GET_SEX(ch) == SEX_MALE ? ""  :\
                          GET_SEX(ch) == SEX_FEMALE ? "а" : "и")
#define GET_CH_SUF_2(ch) (GET_SEX(ch) == SEX_NEUTRAL ? "ось" :\
                          GET_SEX(ch) == SEX_MALE ? "ся"  :\
                          GET_SEX(ch) == SEX_FEMALE ? "ась" : "ись")
#define GET_CH_SUF_3(ch) (GET_SEX(ch) == SEX_NEUTRAL ? "ое" :\
                          GET_SEX(ch) == SEX_MALE ? "ый"  :\
                          GET_SEX(ch) == SEX_FEMALE ? "ая" : "ые")
#define GET_CH_SUF_4(ch) (GET_SEX(ch) == SEX_NEUTRAL ? "ло" :\
                          GET_SEX(ch) == SEX_MALE ? ""  :\
                          GET_SEX(ch) == SEX_FEMALE ? "ла" : "ли")
#define GET_CH_SUF_5(ch) (GET_SEX(ch) == SEX_NEUTRAL ? "ло" :\
                          GET_SEX(ch) == SEX_MALE ? "ел"  :\
                          GET_SEX(ch) == SEX_FEMALE ? "ла" : "ли")
#define GET_CH_SUF_6(ch) (GET_SEX(ch) == SEX_NEUTRAL ? "о" :\
                          GET_SEX(ch) == SEX_MALE ? ""  :\
                          GET_SEX(ch) == SEX_FEMALE ? "а" : "ы")
#define GET_CH_SUF_7(ch) (GET_SEX(ch) == SEX_NEUTRAL ? "о" :\
                          GET_SEX(ch) == SEX_MALE ? "им"  :\
                          GET_SEX(ch) == SEX_FEMALE ? "ей" : "ы")

#define GET_CH_SUF_8(ch) (GET_SEX(ch) == SEX_NEUTRAL ? "о" :\
                          GET_SEX(ch) == SEX_MALE ? "ым"  :\
                          GET_SEX(ch) == SEX_FEMALE ? "ой" : "ы")

#define GET_CH_SUF_9(ch) (GET_SEX(ch) == SEX_NEUTRAL ? "ым" :\
                          GET_SEX(ch) == SEX_MALE ? "ым"  :\
                          GET_SEX(ch) == SEX_FEMALE ? "ой" : "ыми")

#define GET_CH_SUF_10(ch) (GET_SEX(ch) == SEX_NEUTRAL ? "ого" :\
                           GET_SEX(ch) == SEX_MALE ? "ого"  :\
                           GET_SEX(ch) == SEX_FEMALE ? "ой" : "ыми")

#define GET_CH_SUF_11(ch) (GET_SEX(ch) == SEX_NEUTRAL ? "ие" :\
                           GET_SEX(ch) == SEX_MALE ? "ий"  :\
                           GET_SEX(ch) == SEX_FEMALE ? "ая" : "ими")



#define GET_CH_VIS_SUF_1(ch,och) (!CAN_SEE(och,ch) ? "" :\
                                  (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                                   (get_sex_infra(ch) == SEX_NEUTRAL ? "о" :\
                                    get_sex_infra(ch) == SEX_MALE ? ""  :\
                                    get_sex_infra(ch) == SEX_FEMALE ? "а" : "и") : \
                                           GET_SEX(ch) == SEX_NEUTRAL ? "о" :\
                                           GET_SEX(ch) == SEX_MALE ? ""  :\
                                           GET_SEX(ch) == SEX_FEMALE ? "а" : "и"))

#define GET_CH_VIS_SUF_2(ch,och) (!CAN_SEE(och,ch) ? "ся" :\
                                  (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                                   (get_sex_infra(ch) == SEX_NEUTRAL ? "ось" :\
                                    get_sex_infra(ch) == SEX_MALE ? "ся"  :\
                                    get_sex_infra(ch) == SEX_FEMALE ? "ась" : "ись") : \
                                           GET_SEX(ch) == SEX_NEUTRAL ? "ось" :\
                                           GET_SEX(ch) == SEX_MALE ? "ся"  :\
                                           GET_SEX(ch) == SEX_FEMALE ? "ась" : "ись"))

#define GET_CH_VIS_SUF_3(ch,och) (!CAN_SEE(och,ch) ? "ый" :\
                                  (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                                   (get_sex_infra(ch) == SEX_NEUTRAL ? "ое" :\
                                    get_sex_infra(ch) == SEX_MALE ? "ый"  :\
                                    get_sex_infra(ch) == SEX_FEMALE ? "ая" : "ые") : \
                                           GET_SEX(ch) == SEX_NEUTRAL ? "ое" :\
                                           GET_SEX(ch) == SEX_MALE ? "ый"  :\
                                           GET_SEX(ch) == SEX_FEMALE ? "ая" : "ые"))

#define GET_CH_VIS_SUF_4(ch,och) (!CAN_SEE(och,ch) ? "" :\
                                  (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                                   (get_sex_infra(ch) == SEX_NEUTRAL ? "ло" :\
                                    get_sex_infra(ch) == SEX_MALE ? ""  :\
                                    get_sex_infra(ch) == SEX_FEMALE ? "ла" : "ли") : \
                                           GET_SEX(ch) == SEX_NEUTRAL ? "ло" :\
                                           GET_SEX(ch) == SEX_MALE ? ""  :\
                                           GET_SEX(ch) == SEX_FEMALE ? "ла" : "ли"))

#define GET_CH_VIS_SUF_5(ch,och) (!CAN_SEE(och,ch) ? "ел" :\
                                  (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                                   (get_sex_infra(ch) == SEX_NEUTRAL ? "ло" :\
                                    get_sex_infra(ch) == SEX_MALE ? "ел"  :\
                                    get_sex_infra(ch) == SEX_FEMALE ? "ла" : "ли") : \
                                           GET_SEX(ch) == SEX_NEUTRAL ? "ло" :\
                                           GET_SEX(ch) == SEX_MALE ? "ел"  :\
                                           GET_SEX(ch) == SEX_FEMALE ? "ла" : "ли"))

#define GET_CH_VIS_SUF_6(ch,och) (!CAN_SEE(och,ch) ? "" :\
                                  (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                                   (get_sex_infra(ch) == SEX_NEUTRAL ? "о" :\
                                    get_sex_infra(ch) == SEX_MALE ? ""  :\
                                    get_sex_infra(ch) == SEX_FEMALE ? "а" : "ы") : \
                                           GET_SEX(ch) == SEX_NEUTRAL ? "о" :\
                                           GET_SEX(ch) == SEX_MALE ? ""  :\
                                           GET_SEX(ch) == SEX_FEMALE ? "а" : "ы"))

#define GET_CH_VIS_SUF_7(ch,och) (!CAN_SEE(och,ch) ? "им" :\
                                  (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                                   (get_sex_infra(ch) == SEX_NEUTRAL ? "им" :\
                                    get_sex_infra(ch) == SEX_MALE ? "им"  :\
                                    get_sex_infra(ch) == SEX_FEMALE ? "ой" : "ими") : \
                                           GET_SEX(ch) == SEX_NEUTRAL ? "им" :\
                                           GET_SEX(ch) == SEX_MALE ? "им"  :\
                                           GET_SEX(ch) == SEX_FEMALE ? "ой" : "ими"))
#define GET_CH_VIS_SUF_8(ch,och) (!CAN_SEE(och,ch) ? "ого" :\
                                  (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                                   (get_sex_infra(ch) == SEX_NEUTRAL ? "ого" :\
                                    get_sex_infra(ch) == SEX_MALE ? "ого"  :\
                                    get_sex_infra(ch) == SEX_FEMALE ? "ой" : "ыми") : \
                                           GET_SEX(ch) == SEX_NEUTRAL ? "ого" :\
                                           GET_SEX(ch) == SEX_MALE ? "ого"  :\
                                           GET_SEX(ch) == SEX_FEMALE ? "ой" : "ыми"))

#define GET_CH_VIS_SUF_9(ch,och) (!CAN_SEE(och,ch) ? "ий" :\
                                  (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                                   (get_sex_infra(ch) == SEX_NEUTRAL ? "ие" :\
                                    get_sex_infra(ch) == SEX_MALE ? "ий"  :\
                                    get_sex_infra(ch) == SEX_FEMALE ? "ая" : "ие") : \
                                           GET_SEX(ch) == SEX_NEUTRAL ? "ие" :\
                                           GET_SEX(ch) == SEX_MALE ? "ий"  :\
                                           GET_SEX(ch) == SEX_FEMALE ? "ая" : "ие"))

#define GET_CH_VIS_SUF_10(ch,och) (!CAN_SEE(och,ch) ? "ым" :\
                                   (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                                    (get_sex_infra(ch) == SEX_NEUTRAL ? "ым" :\
                                     get_sex_infra(ch) == SEX_MALE ? "ым"  :\
                                     get_sex_infra(ch) == SEX_FEMALE ? "ой" : "ыми") : \
                                            GET_SEX(ch) == SEX_NEUTRAL ? "ым" :\
                                            GET_SEX(ch) == SEX_MALE ? "ым"  :\
                                            GET_SEX(ch) == SEX_FEMALE ? "ой" : "ыми"))

#define GET_OBJ_DURAB(obj) (get_durab_obj(obj))
#define GET_OBJ_SAVE(obj,type) (get_save_obj(obj,type))
#define GET_OBJ_SEX(obj) ((obj)->obj_flags.Obj_sex)
#define GET_OBJ_PERCENT(obj) ((obj)->shance)


#define GET_OBJ_SUF_1(obj) (GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "о" :\
                            GET_OBJ_SEX(obj) == SEX_MALE ? ""  :\
                            GET_OBJ_SEX(obj) == SEX_FEMALE ? "а" : "и")
#define GET_OBJ_SUF_2(obj) (GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ось" :\
                            GET_OBJ_SEX(obj) == SEX_MALE ? "ся"  :\
                            GET_OBJ_SEX(obj) == SEX_FEMALE ? "ась" : "ись")
#define GET_OBJ_SUF_3(obj) (GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ое" :\
                            GET_OBJ_SEX(obj) == SEX_MALE ? "ый"  :\
                            GET_OBJ_SEX(obj) == SEX_FEMALE ? "ая" : "ые")
#define GET_OBJ_SUF_4(obj) (GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ло" :\
                            GET_OBJ_SEX(obj) == SEX_MALE ? ""  :\
                            GET_OBJ_SEX(obj) == SEX_FEMALE ? "ла" : "ли")
#define GET_OBJ_SUF_5(obj) (GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ло" :\
                            GET_OBJ_SEX(obj) == SEX_MALE ? "ел"  :\
                            GET_OBJ_SEX(obj) == SEX_FEMALE ? "ла" : "ли")
#define GET_OBJ_SUF_6(obj) (GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "о" :\
                            GET_OBJ_SEX(obj) == SEX_MALE ? ""  :\
                            GET_OBJ_SEX(obj) == SEX_FEMALE ? "а" : "ы")
#define GET_OBJ_SUF_7(obj) (GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ым" :\
                            GET_OBJ_SEX(obj) == SEX_MALE ? "ым"  :\
                            GET_OBJ_SEX(obj) == SEX_FEMALE ? "ой" : "ыми")
#define GET_OBJ_SUF_8(obj) (GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ого" :\
                            GET_OBJ_SEX(obj) == SEX_MALE ? "ого"  :\
                            GET_OBJ_SEX(obj) == SEX_FEMALE ? "ой" : "ыми")
#define GET_OBJ_SUF_9(obj) (GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ого" :\
                            GET_OBJ_SEX(obj) == SEX_MALE ? "ое"  :\
                            GET_OBJ_SEX(obj) == SEX_FEMALE ? "ая" : "ие")


#define GET_OBJ_VIS_SUF_1(obj,ch) (!CAN_SEE_OBJ(ch,obj) ? "о" :\
                                   GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "о" :\
                                   GET_OBJ_SEX(obj) == SEX_MALE ? ""  :\
                                   GET_OBJ_SEX(obj) == SEX_FEMALE ? "а" : "и")
#define GET_OBJ_VIS_SUF_2(obj,ch) (!CAN_SEE_OBJ(ch,obj) ? "ось" :\
                                   GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ось" :\
                                   GET_OBJ_SEX(obj) == SEX_MALE ? "ся"  :\
                                   GET_OBJ_SEX(obj) == SEX_FEMALE ? "ась" : "ись")
#define GET_OBJ_VIS_SUF_3(obj,ch) (!CAN_SEE_OBJ(ch,obj) ? "ый" :\
                                   GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ое" :\
                                   GET_OBJ_SEX(obj) == SEX_MALE ? "ый"  :\
                                   GET_OBJ_SEX(obj) == SEX_FEMALE ? "ая" : "ые")
#define GET_OBJ_VIS_SUF_4(obj,ch) (!CAN_SEE_OBJ(ch,obj) ? "ло" :\
                                   GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ло" :\
                                   GET_OBJ_SEX(obj) == SEX_MALE ? ""  :\
                                   GET_OBJ_SEX(obj) == SEX_FEMALE ? "ла" : "ли")
#define GET_OBJ_VIS_SUF_5(obj,ch) (!CAN_SEE_OBJ(ch,obj) ? "ло" :\
                                   GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ло" :\
                                   GET_OBJ_SEX(obj) == SEX_MALE ? "ел"  :\
                                   GET_OBJ_SEX(obj) == SEX_FEMALE ? "ла" : "ли")
#define GET_OBJ_VIS_SUF_6(obj,ch) (!CAN_SEE_OBJ(ch,obj) ? "о" :\
                                   GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "о" :\
                                   GET_OBJ_SEX(obj) == SEX_MALE ? ""  :\
                                   GET_OBJ_SEX(obj) == SEX_FEMALE ? "а" : "ы")
#define GET_OBJ_VIS_SUF_7(obj,ch) (!CAN_SEE_OBJ(ch,obj) ? "ым" :\
                                   GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ым" :\
                                   GET_OBJ_SEX(obj) == SEX_MALE ? "ым"  :\
                                   GET_OBJ_SEX(obj) == SEX_FEMALE ? "ой" : "ыми")
#define GET_OBJ_VIS_SUF_8(obj,ch) (!CAN_SEE_OBJ(ch,obj) ? "ого" :\
                                   GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ого" :\
                                   GET_OBJ_SEX(obj) == SEX_MALE ? "ого"  :\
                                   GET_OBJ_SEX(obj) == SEX_FEMALE ? "ой" : "ыми")
#define GET_OBJ_VIS_SUF_9(obj,ch) (!CAN_SEE_OBJ(ch,obj) ? "ий" :\
                                   GET_OBJ_SEX(obj) == SEX_NEUTRAL ? "ие" :\
                                   GET_OBJ_SEX(obj) == SEX_MALE ? "ий"  :\
                                   GET_OBJ_SEX(obj) == SEX_FEMALE ? "ая" : "ие")


/* These three deprecated. */
#define WAIT_STATE(ch, cycle) do { GET_WAIT_STATE(ch) = (cycle); } while(0)
#define CHECK_WAIT(ch)        ((ch)->wait > 0)
#define GET_MOB_HOLD(ch)      (AFF_FLAGGED((ch),AFF_HOLD) ? 1 : 0)
#define GET_MOB_SIELENCE(ch)  (AFF_FLAGGED((ch),AFF_SIELENCE) ? 1 : 0)
/* New, preferred macro. */
/* Wait_State показывает время в пульсах */
#define GET_WAIT_STATE(ch)    ((ch)->wait)
/* Wait показывает время в раундах */
#define GET_WAIT(ch)          GET_WAIT_STATE(ch)

/* descriptor-based utils ************************************************/

/* Hrm, not many.  We should make more. -gg 3/4/99 */
#define STATE(d)        ((d)->connected)


/* object utils **********************************************************/

#define GET_OBJ_ALIAS(obj)      ((obj)->short_description)
#define GET_OBJ_NAME(obj) ((obj)->names)
#define GET_OBJ_PNAME(obj,pad)  ((obj)->PNames[pad])
#define GET_OBJ_DESC(obj)       ((obj)->description)
#define GET_OBJ_AFFECTS(obj)    ((obj)->obj_flags.affects)
#define GET_OBJ_ANTI(obj)       ((obj)->obj_flags.anti_flag)
#define GET_OBJ_NO(obj)         ((obj)->obj_flags.no_flag)
#define GET_OBJ_ACT(obj)        ((obj)->action_description)
#define GET_OBJ_POS(obj)        ((obj)->obj_flags.worn_on)
#define GET_OBJ_TYPE(obj)       ((obj)->obj_flags.type_flag)
#define GET_OBJ_TOOL(obj) ((obj)->obj_flags.type_tool)
#define GET_ARM_TYPE(obj)       ((obj)->obj_flags.type_arm)
#define GET_OBJ_COST(obj)       ((obj)->obj_flags.cost)
#define GET_OBJ_EXTRA(obj,flag) (GET_FLAG((obj)->obj_flags.extra_flags,flag))
#define GET_SPEC_PROPERTY(h,flag) (GET_FLAG((h)->property,flag))
#define GET_OBJ_AFF(obj,flag)   (GET_FLAG((obj)->obj_flags.affects,flag))
#define GET_OBJ_AFFECT(obj) ((obj)->obj_flags.bitvector)
#define _GET_OBJ_ANTI(obj,flag) (GET_FLAG((obj)->obj_flags.anti_flag,flag))
#define _GET_OBJ_NO(obj,flag) (GET_FLAG((obj)->obj_flags.no_flag,flag))
#define GET_OBJ_BFLAG(obj,flag) (GET_FLAG((obj)->obj_flags.bflag,flag))
#define GET_OBJ_WEAR(obj)       ((obj)->obj_flags.wear_flags)
#define GET_OBJ_OWNER(obj)      ((obj)->obj_flags.Obj_owner)
#define GET_OBJ_VAL(obj, val)   ((obj)->obj_flags.value[(val)])
#define GET_OBJ_COV(obj, val)   ((obj)->obj_flags.cover[(val)])
#define GET_OBJ_POWER(obj)      ((obj)->powered)
//ADDED BY HMEPAS
#define GET_LIGHT_VAL(obj)      ((obj)->obj_flags.light)
#define GET_LIGHT_BRT(obj)      ((obj)->obj_flags.bright)
#define GET_LIGHT_ON(obj)       ((obj)->obj_flags.value[3])
#define GET_OBJ_LIMIT(obj) ((obj)->limit)
#define GET_OBJ_WEIGHT(obj)     ((obj)->obj_flags.weight)
#define GET_OBJ_COUNTITEMS(obj)     ((obj)->obj_flags.countitems)
#define GET_OBJ_TIMER(obj)      ((obj)->obj_flags.Obj_timer)
#define GET_OBJ_TIMELOAD(obj) ((obj)->obj_flags.Obj_time_load)
#define GET_OBJ_QUALITY(obj)    ((obj)->obj_flags.quality)
#define GET_OBJ_DESTROY(obj) ((obj)->obj_flags.Obj_destroyer)
#define GET_OBJ_SKILL(obj)      ((obj)->weapon->skill)
#define GET_OBJ_CUR(obj)    ((obj)->obj_flags.Obj_cur)
#define GET_OBJ_MAX(obj)    ((obj)->obj_flags.Obj_max)
#define GET_OBJ_ZONE(obj)   ((obj)->obj_flags.Obj_zone)
#define GET_OBJ_TEMP(obj)   ((obj)->obj_flags.Obj_temp)
#define GET_OBJ_TEMP_ADD(obj)   ((obj)->obj_flags.Obj_temp_add)
#define GET_OBJ_REAL_TEMP(obj) (GET_OBJ_TEMP(obj) + GET_OBJ_TEMP_ADD(obj))
#define GET_OBJ_RNUM(obj)       ((obj)->item_number)
#define OBJ_GET_LASTROOM(obj) ((obj)->room_was_in)
#define GET_OBJ_VNUM(obj)       (GET_OBJ_RNUM(obj) >= 0 ? \
                                 obj_index[GET_OBJ_RNUM(obj)].vnum : -1)
#define OBJ_WHERE(obj) ((obj)->worn_by    ? IN_ROOM(obj->worn_by) : \
                        (obj)->carried_by ? IN_ROOM(obj->carried_by) : (obj)->in_room)
#define IS_OBJ_STAT(obj,stat)   (IS_SET(GET_FLAG((obj)->obj_flags.extra_flags, \
                                        stat), stat))
#define IS_OBJ_ANTI(obj,stat)   (IS_SET(GET_FLAG((obj)->obj_flags.anti_flag, \
                                        stat), stat))
#define IS_OBJ_NO(obj,stat)         (IS_SET(GET_FLAG((obj)->obj_flags.no_flag, \
                                     stat), stat))
#define IS_OBJ_AFF(obj,stat)    (IS_SET(GET_FLAG((obj)->obj_flags.affects, \
                                        stat), stat))

#define IS_CORPSE(obj)          (GET_OBJ_TYPE(obj) == ITEM_CORPSE)

#define GET_OBJ_SPEC(obj) ((obj)->item_number >= 0 ? \
                           (obj_index[(obj)->item_number].func) : NULL)

#define CAN_WEAR(obj, part) (IS_SET((obj)->obj_flags.wear_flags, (part)))
#define CAN_WEAR_ANY(obj) (((obj)->obj_flags.wear_flags>0) && ((obj)->obj_flags.wear_flags != ITEM_WEAR_TAKE) )

#define IS_LOCKPICK(obj) (GET_OBJ_TOOL(obj) == TOOL_LOCKPICK)
#define IS_HAMMER(obj)  (GET_OBJ_TOOL(obj) == TOOL_HAMMER)
#define IS_SHOVEL(obj)  (GET_OBJ_TOOL(obj) == TOOL_SHOVEL)

/* compound utilities and other macros **********************************/

/*
 * Used to compute CircleMUD version. To see if the code running is newer
 * than 3.0pl13, you would use: #if _CIRCLEMUD > CIRCLEMUD_VERSION(3,0,13)
 */
#define CIRCLEMUD_VERSION(major, minor, patchlevel) \
    (((major) << 16) + ((minor) << 8) + (patchlevel))

#define HSHR(ch,och) (!CAN_SEE(och,ch) ? "его" :\
                      (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                       (get_sex_infra(ch) == SEX_NEUTRAL ? "его" :\
                        get_sex_infra(ch) == SEX_MALE ? "его"  :\
                        get_sex_infra(ch) == SEX_FEMALE ? "ее" : "их") : \
                               (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "его": \
                                               (GET_SEX(ch) == SEX_FEMALE ? "ее" : "их")) :"его")))

#define HSSH(ch,och) (!CAN_SEE(och,ch) ? "он" :\
                      (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                       (get_sex_infra(ch) == SEX_NEUTRAL ? "оно" :\
                        get_sex_infra(ch) == SEX_MALE ? "он"  :\
                        get_sex_infra(ch) == SEX_FEMALE ? "она" : "они") : \
                               (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "он": (GET_SEX(ch) == SEX_FEMALE ? "она" : "они")) :"оно")))

#define HMHR(ch,och) (!CAN_SEE(och,ch) ? "ему" :\
                      (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(och)) && !CAN_SEE_IN_DARK_ALL(och) ? \
                       (get_sex_infra(ch) == SEX_NEUTRAL ? "ему" :\
                        get_sex_infra(ch) == SEX_MALE ? "ему"  :\
                        get_sex_infra(ch) == SEX_FEMALE ? "ей" : "им") : \
                               (GET_SEX(ch) ? (GET_SEX(ch)==SEX_MALE ? "ему": (GET_SEX(ch) == SEX_FEMALE ? "ей" : "им")) :"ему")))

#define OSHR(ch) (GET_OBJ_SEX(ch) ? (GET_OBJ_SEX(ch)==SEX_MALE ? "его": (GET_OBJ_SEX(ch) == SEX_FEMALE ? "ее" : "их")) :"его")
#define OSSH(ch) (GET_OBJ_SEX(ch) ? (GET_OBJ_SEX(ch)==SEX_MALE ? "он": (GET_OBJ_SEX(ch) == SEX_FEMALE ? "она" : "они")) :"оно")
#define OMHR(ch) (GET_OBJ_SEX(ch) ? (GET_OBJ_SEX(ch)==SEX_MALE ? "ему": (GET_OBJ_SEX(ch) == SEX_FEMALE ? "ей" : "им")) :"ему")


#define DSHR(ch, door)  (EXIT(ch,door)->sex == SEX_FEMALE ? "а" : \
                         EXIT(ch,door)->sex == SEX_NEUTRAL ? "о" : \
                         EXIT(ch,door)->sex == SEX_MALE ? "" : "ы")

#define DSHM(ch, door)  (EXIT(ch,door)->sex == SEX_FEMALE ? "ую" : \
                         EXIT(ch,door)->sex == SEX_NEUTRAL ? "ое" : \
                         EXIT(ch,door)->sex == SEX_MALE ? "ый" : "ые")

#define DSHP(ch, door)  (EXIT(ch,door)->sex == SEX_FEMALE ? "ая" : \
                         EXIT(ch,door)->sex == SEX_NEUTRAL ? "ое" : \
                         EXIT(ch,door)->sex == SEX_MALE ? "ый" : "ое")

/* Various macros building up to CAN_SEE */
#define MAY_SEE(sub,obj) (!AFF_FLAGGED((sub),AFF_BLIND) && \
                          (!IS_DARK(IN_ROOM(sub)) || \
                           ((AFF_FLAGGED((sub),AFF_INFRAVISION) && !IS_UNDEAD(obj) && !IS_REPTILE(obj)) || AFF_FLAGGED((sub),AFF_DARKVISION))) && \
                          (!MOB_FLAGGED((obj),MOB_CLONE)) && \
                          (!AFF_FLAGGED((obj),AFF_INVISIBLE) || AFF_FLAGGED((sub),AFF_DETECT_INVIS)) && \
                          (!IS_UNDEAD(obj) || AFF_FLAGGED((sub),AFF_DETECT_UNDEAD)))

#define MAY_ATTACK(sub)  (!AFF_FLAGGED((sub),AFF_CHARM)     && \
                          !IS_HORSE((sub))                  && \
                          !AFF_FLAGGED((sub),AFF_STOPFIGHT) && \
                          !AFF_FLAGGED((sub),AFF_STUNE) && \
                          !AFF_FLAGGED((sub),AFF_HOLD)      && \
                          !MOB_FLAGGED((sub),MOB_NOFIGHT)   && \
                          GET_WAIT(sub) <= 0                && \
                          !FIGHTING(sub)                    && \
                          GET_POS(sub) >= POS_RESTING)

#define MAY_EXP(sub)     (!AFF_FLAGGED((sub),AFF_CHARM)     && \
                          !IS_HORSE((sub))                  && \
                          !AFF_FLAGGED((sub),AFF_STOPFIGHT) && \
                          !AFF_FLAGGED((sub),AFF_HOLD)      && \
                          !MOB_FLAGGED((sub),MOB_NOFIGHT)   && \
                          GET_POS(sub) >= POS_RESTING)

#define MAY_MOVE(sub)    (!AFF_FLAGGED((sub),AFF_CHARM)     && \
                          !IS_HORSE((sub))                  && \
                          !AFF_FLAGGED((sub),AFF_STOPFIGHT) && \
                          !AFF_FLAGGED((sub),AFF_STUNE) && \
                          !AFF_FLAGGED((sub),AFF_HOLD)      && \
                          GET_WAIT(sub) <= 0                && \
                          GET_POS(sub) >= POS_RESTING)

#define MAY_MOVE2(sub)   (!AFF_FLAGGED((sub),AFF_STOPFIGHT) && \
                          !AFF_FLAGGED((sub),AFF_STUNE) && \
                          !AFF_FLAGGED((sub),AFF_HOLD)      && \
                          GET_WAIT(sub) <= 0                && \
                          GET_POS(sub) >= POS_RESTING)

#define LIGHT_OK(sub)   (!AFF_FLAGGED(sub, AFF_BLIND) && \
                         (IS_LIGHT((sub)->in_room) || AFF_FLAGGED((sub), AFF_DARKVISION) || AFF_FLAGGED((sub), AFF_INFRAVISION)))


#define INVIS_OK(sub, obj) \
    (!AFF_FLAGGED((sub), AFF_BLIND) && \
     !MOB_FLAGGED((obj),MOB_CLONE) && \
     ((!AFF_FLAGGED((obj),AFF_INVISIBLE) || \
       AFF_FLAGGED((sub),AFF_DETECT_INVIS) \
      ) && \
      ((!AFF_FLAGGED((obj), AFF_SNEAK) && !AFF_FLAGGED((obj), AFF_HIDE) && !AFF_FLAGGED((obj), AFF_CAMOUFLAGE)) || \
       check_victim_visible((struct char_data *)sub,(struct char_data *)obj) || AFF_FLAGGED((sub), AFF_SENSE_LIFE) \
      ) \
     ) \
    )


/*
#define INVIS_OK(sub,obj) \
 (!AFF_FLAGGED(sub,AFF_BLIND) && \
         !MOB_FLAGGED((obj),MOB_CLONE) && \
   (!AFF_FLAGGED(obj,AFF_INVISIBLE) || AFF_FLAGGED(sub,AFF_DETECT_INVIS)) \
  )

*/
#define HERE(ch)  ((IS_NPC(ch) || (ch)->desc))

//HERE(obj) && \  убрано из MORT_CAN_SEE


#define MORT_CAN_SEE(sub, obj) ((((IS_GOD(obj) && HERE(obj)) || !IS_GOD(obj)) && \
                                 INVIS_OK(sub,obj) && \
                                 (IS_LIGHT((obj)->in_room) || \
                                  (AFF_FLAGGED((sub),AFF_SENSE_LIFE) && !IS_UNDEAD(obj)) ||   \
                                  (same_group((struct char_data *)sub,(struct char_data *)obj)) || \
                                  ((AFF_FLAGGED((sub), AFF_INFRAVISION) || \
                                    affected_by_spell((sub),SPELL_INFRAVISION)) && \
                                   !IS_UNDEAD(obj) && !IS_REPTILE(obj)) ||\
                                  AFF_FLAGGED((sub), AFF_DARKVISION) \
                                 ) \
                                ) || (IS_UNDEAD(obj) && IS_AFFECTED(sub,AFF_DETECT_UNDEAD)))
#define IMM_CAN_SEE(sub, obj) \
    (MORT_CAN_SEE(sub, obj) || \
     (!IS_NPC(sub) && (PRF_FLAGGED(sub, PRF_HOLYLIGHT))))

#define SELF(sub, obj)  ((sub) == (obj))

/* Can subject see character "obj"?  */
#define CAN_SEE(sub, obj) (SELF(sub, obj) || \
                           ((GET_LEVEL(sub) >= (IS_NPC(obj) ? 0 : GET_INVIS_LEV(obj))) && \
                            GET_POS(sub) != POS_SLEEPING && \
                            IMM_CAN_SEE(sub, obj)))


/* Can subject see character "obj" without light */
#define MORT_CAN_SEE_CHAR(sub, obj) (HERE(obj) && \
                                     GET_POS(sub) != POS_SLEEPING && \
                                     INVIS_OK(sub,obj))

#define IMM_CAN_SEE_CHAR(sub, obj) \
    (MORT_CAN_SEE_CHAR(sub, obj) || (!IS_NPC(sub) && (PRF_FLAGGED(sub, PRF_HOLYLIGHT))))

#define CAN_SEE_CHAR(sub, obj) (SELF(sub, obj) || \
                                ((GET_LEVEL(sub) >= (IS_NPC(obj) ? 0 : GET_INVIS_LEV(obj))) && \
                                 IMM_CAN_SEE_CHAR(sub, obj)))


/* End of CAN_SEE */


#define INVIS_OK_OBJ(sub, obj) \
    (!IS_OBJ_STAT((obj), ITEM_INVISIBLE) || AFF_FLAGGED((sub), AFF_DETECT_INVIS))

/* Is anyone carrying this object and if so, are they visible? */

#define CAN_SEE_OBJ_CARRIER(sub, obj) \
    ((!obj->carried_by || CAN_SEE(sub, obj->carried_by)) && \
     (!obj->worn_by    || CAN_SEE(sub, obj->worn_by)))
/*
#define MORT_CAN_SEE_OBJ(sub, obj) \
  (LIGHT_OK(sub) && INVIS_OK_OBJ(sub, obj) && CAN_SEE_OBJ_CARRIER(sub, obj))
 */

//&& CAN_SEE_OBJ_CARRIER(sub, obj)

#define MORT_CAN_SEE_OBJ(sub, obj) \
    (INVIS_OK_OBJ(sub, obj) && !AFF_FLAGGED(sub, AFF_BLIND) && CAN_SEE_OBJ_CARRIER(sub, obj) && \
     (IS_LIGHT(IN_ROOM(sub)) || OBJ_FLAGGED(obj, ITEM_GLOW) ||\
      AFF_FLAGGED(sub, AFF_DARKVISION)))

/*||\
(IS_CORPSE(obj) && AFF_FLAGGED(sub, AFF_INFRAVISION)))) */

#define CAN_SEE_OBJ(sub, obj) \
    (obj->worn_by    == sub || \
     obj->carried_by == sub || \
     (PLR_FLAGGED((sub),PLR_SOUL) && IS_CORPSE(obj)) ||\
     ((!IS_SOUL(sub) && (obj->in_obj && (obj->in_obj->worn_by == sub || obj->in_obj->carried_by == sub))) || \
      (MORT_CAN_SEE_OBJ(sub, obj) && !IS_BURIED(obj) &&  \
       (!IS_OBJ_STAT(obj,ITEM_HIDDEN) || check_obj_visible(sub,obj))) || \
      (!IS_NPC(sub) && PRF_FLAGGED((sub), PRF_HOLYLIGHT))))

#define CAN_CARRY_OBJ(ch,obj)  \
    (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
     ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
    (CAN_WEAR((obj), ITEM_WEAR_TAKE) && CAN_CARRY_OBJ((ch),(obj)) && \
     CAN_SEE_OBJ((ch),(obj)))

#define MAX_MOVE_W(ch) (GET_REAL_STR(ch) * 6000)

#define OK_MOVE_W(ch)  (IS_CARRYING_W(ch)+IS_WEARING_W(ch) < MAX_MOVE_W(ch))

#define OK_WEAR(ch,obj) (GET_OBJ_WEIGHT(obj) <= GET_REAL_STR(ch) * 1200)

#define CAN_CARRY_W(ch) (GET_REAL_STR(ch) * 4000)

#define CAN_CARRY_N(ch) (5 + (GET_REAL_DEX(ch) >> 1) + (GET_LEVEL(ch) >> 1))

#define OK_BOTH(ch,obj)  (GET_OBJ_WEIGHT(obj) <= GET_REAL_STR(ch)*1400)

#define OK_WIELD(ch,obj) (GET_OBJ_WEIGHT(obj) <= GET_REAL_STR(ch)*800)

#define OK_HELD(ch,obj)  (GET_OBJ_WEIGHT(obj) <= GET_REAL_STR(ch)*600)

#define OK_HELDN(ch,obj)  (GET_OBJ_WEIGHT(obj) <= GET_REAL_STR(ch)*200)

#define GET_PAD_PERS(pad) ((pad) == 5 ? "ком-то" :\
                           (pad) == 4 ? "кем-то" :\
                           (pad) == 3 ? "кого-то" :\
                           (pad) == 2 ? "кому-то" :\
                           (pad) == 1 ? "кого-то" : "кто-то")

//(PLR_FLAGGED(ch,PLR_SOUL) ? name_infra(ch, pad) :

/*#define PERS(ch,vict,pad) (PLR_FLAGGED(vict,PLR_SOUL) ? name_infra(ch, pad) : \
                           (CAN_SEE(vict, ch) ? \
                           ((IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(vict)) && \
                             !same_group((struct char_data *)ch,(struct char_data *)vict) && \
                             !CAN_SEE_IN_DARK_ALL(vict)) ? name_infra(ch, pad): \
                            check_incognito((struct char_data *)ch) ? hide_race((struct char_data *)ch,pad) : GET_PAD(ch,pad)) : (check_know(vict,ch) ? name_infra(ch, pad) : GET_PAD_PERS(pad))))*/

#define PERS(ch,vict,pad) get_pers_name(ch,vict,pad)

#define OBJS(obj,vict) (CAN_SEE_OBJ((vict), (obj)) ? \
                        (obj)->short_description  : "что-то")

#define GET_PAD_OBJ(pad)  ((pad) == 5 ? "чем-то" :\
                           (pad) == 4 ? "чем-то" :\
                           (pad) == 3 ? "что-то" :\
                           (pad) == 2 ? "чему-то" :\
                           (pad) == 1 ? "чего-то" : "что-то")


#define OBJN(obj,vict,pad) (CAN_SEE_OBJ((vict), (obj))  ? \
                            ((obj)->PNames[pad]) ? (obj)->PNames[pad] : (obj)->short_description \
                            : GET_PAD_OBJ(pad))

#define EXITDATA(room,door) ((room >= 0 && room <= top_of_world) ? \
                             world[room].dir_option[door] : NULL)

#define EXIT(ch, door)  (world[(ch)->in_room].dir_option[door])


#define CAN_GO(ch, door) (EXIT(ch,door) && \
                          ((EXIT(ch,door)->to_room != NOWHERE) || \
                           (EXIT(ch,door)->timer)) && \
                          !DOOR_FLAGGED(EXIT(ch,door),EXIT_CLOSED))
#define CAN_GO2(ch, door) (EXIT(ch,door) && \
                           ((EXIT(ch,door)->to_room != NOWHERE) || \
                            (EXIT(ch,door)->timer)))


#define CAN_GO_DATA(in_room, door) (EXITDATA(in_room,door) && \
                                    ((EXITDATA(in_room,door)->to_room != NOWHERE) || \
                                     (EXITDATA(in_room,door)->timer)) && \
                                    !DOOR_FLAGGED(EXITDATA(in_room,door),EXIT_CLOSED))

#define CLASS_ABBR(ch) (IS_NPC(ch) ? "--" : class_abbrevs[IS_NPC(ch) ? (int)GET_CLASS(ch) : (int)GET_FVCLASS(ch)])

#define IS_MAGIC_USER(ch)       (IS_NPC(ch) ? GET_CLASS(ch) == CLASS_MAGIC_USER : check_class((ch),CLASS_MAGIC_USER) > 0)
#define IS_WARRIOR(ch)          (IS_NPC(ch) ? GET_CLASS(ch) == CLASS_WARRIOR : check_class((ch),CLASS_WARRIOR) > 0)
#define IS_THIEF(ch)            (IS_NPC(ch) ? GET_CLASS(ch) == CLASS_THIEF  : check_class((ch),CLASS_THIEF) > 0)
#define IS_RANGER(ch)           (IS_NPC(ch) ? GET_CLASS(ch) == CLASS_RANGER : check_class((ch),CLASS_RANGER) > 0)
#define IS_PRIEST(ch)           (IS_NPC(ch) ? GET_CLASS(ch) == CLASS_PRIEST : check_class((ch),CLASS_PRIEST) > 0)
#define IS_NECRO(ch)            (IS_NPC(ch) ? GET_CLASS(ch) == CLASS_NECRO : check_class((ch),CLASS_NECRO) > 0)

//
#define IS_HUMAN(ch)            (GET_RACE(ch) == RACE_HUMAN)
#define IS_ORC(ch)  (GET_RACE(ch) == RACE_ORC)
#define IS_GNOME(ch)    (GET_RACE(ch) == RACE_GNOME)
#define IS_ELVES(ch)  (GET_RACE(ch) == RACE_ELVES)
#define IS_HALFELVES(ch) (GET_RACE(ch) == RACE_HALFELVES)
#define IS_BARIAUR(ch)  (GET_RACE(ch) == RACE_BARIAUR)
#define IS_TIEFLING(ch)  (GET_RACE(ch) == RACE_TIEFLING)
#define IS_AASIMAR(ch)  (GET_RACE(ch) == RACE_AASIMAR)
#define IS_BIRD(ch)    (GET_RACE(ch) == RACE_BIRD)
#define IS_ANIMAL(ch)    (GET_RACE(ch) == RACE_ANIMAL || GET_RACE(ch) == RACE_RODENT || GET_RACE(ch) == RACE_FISH || GET_RACE(ch) == RACE_BIRD || GET_RACE(ch) == RACE_SNAKE || GET_RACE(ch) == RACE_REPTILE )
#define IS_DRAGON(ch)  (GET_RACE(ch) == RACE_DRAGON)
#define IS_HUMANOID(ch)  (GET_RACE(ch) == RACE_HUMANOID)
#define IS_GENASI(ch)  (GET_RACE(ch) == RACE_GENASI)
#define IS_REPTILE(ch)  (GET_RACE(ch) == RACE_REPTILE)
#define IS_CONSTRUCTION(ch) (GET_RACE(ch) == RACE_CONSTRUCTION)
#define IS_PLANT(ch)  (GET_RACE(ch) == RACE_PLANT)
#define IS_SLIME(ch)  (GET_RACE(ch) == RACE_SLIME)
#define IS_SNAKE(ch)  (GET_RACE(ch) == RACE_SNAKE)
#define IS_INSECT(ch)  (GET_RACE(ch) == RACE_INSECT)
#define IS_SKELET(ch)  (GET_RACE(ch) == RACE_SKELET)
#define IS_GHOST(ch)  (GET_RACE(ch) == RACE_GHOST)
#define IS_UNDEAD(ch)  (IS_GHOST(ch) || IS_SKELET(ch) || MOB_FLAGGED(ch,MOB_CORPSE) || GET_MOB_TYPE(ch) == TMOB_UNDEAD)
#define IS_SOUL(ch)  (PLR_FLAGGED(ch,PLR_SOUL))

#define LIKE_ROOM(ch) ((IS_MAGIC_USER(ch) && ROOM_FLAGGED((ch)->in_room, ROOM_MAGIC_USER)) || \
                       (IS_WARRIOR(ch) && ROOM_FLAGGED((ch)->in_room, ROOM_WARRIOR)) || \
                       (IS_THIEF(ch) && ROOM_FLAGGED((ch)->in_room, ROOM_THIEF)) || \
                       (IS_RANGER(ch) && ROOM_FLAGGED((ch)->in_room, ROOM_RANGER)))

#define OUTSIDE(ch) (!ROOM_FLAGGED((ch)->in_room, ROOM_INDOORS))
#define IS_HORSE(ch) (IS_NPC(ch) && (ch->master) && AFF_FLAGGED(ch, AFF_HORSE))
#define IS_MOUNT(ch) (IS_NPC(ch) && (ch->master) && MOB_FLAGGED(ch, MOB_MOUNTING))

#define NEW_EQUIP(ch, where, what)      {if (!GET_EQ(ch, where)) {              \
            obj = read_object(what, VIRTUAL,TRUE) ;    \
            equip_char(ch, obj, where);          \
        }                                      \
    }

#define NEW_INV(ch, what)       {obj = read_object(what, VIRTUAL,TRUE);                              \
        if (obj) {                                                     \
            if (IS_CARRYING_N(ch) < CAN_CARRY_N(ch) &&                   \
                    (IS_CARRYING_W(ch)+GET_OBJ_WEIGHT(obj) <= CAN_CARRY_W(ch))) \
                obj_to_char(obj, ch);                                      \
            else                                                         \
                extract_obj(obj);                                          \
        }                                                              \
    }

int on_horse(struct char_data *ch);
int has_horse(struct char_data *ch, int same_room);
struct char_data *get_horse(struct char_data *ch);
struct char_data *get_horse_on(struct char_data *ch);
void horse_drop(struct char_data *ch);
void make_horse(struct char_data *horse, struct char_data *ch);
void check_horse(struct char_data *ch);
void die_follower(struct char_data *ch);
void add_follower(struct char_data *ch, struct char_data *leader, int type);
int stop_follower(struct char_data *ch, int mode);
bool circle_follow(struct char_data *ch, struct char_data *victim);

int same_group(struct char_data *ch, struct char_data *tch);
char *only_title(struct char_data *ch);
char *race_or_title(struct char_data *ch, int pad);
char *race_or_title2(struct char_data *ch);
char *hide_race(struct char_data *ch, int pad);
char *race_or_title_enl(struct char_data *ch);
int pc_duration(struct char_data *ch, int cnst, int level, int level_divisor, int min, int max);
void paste_mobiles(int zone);
struct obj_data *check_incognito(struct char_data *ch);

/* Modifier functions */

/* PADS for something ****************************************************/
const char *desc_count(int how_many, int of_what);

#define WHAT_DAY      0
#define WHAT_HOUR     1
#define WHAT_YEAR     2
#define WHAT_POINT    3
#define WHAT_MINa     4
#define WHAT_MINu     5
#define WHAT_MONEYa   6
#define WHAT_MONEYu   7
#define WHAT_THINGa   8
#define WHAT_THINGu   9
#define WHAT_LEVEL    10
#define WHAT_PERSONA  11
#define WHAT_MOVEu    12
#define WHAT_ONEa     13
#define WHAT_ONEu     14
#define WHAT_SEC      15
#define WHAT_DEGREE   16



/* some awaking cases */
#define AW_HIDE       (1 << 0)
#define AW_INVIS      (1 << 1)
#define AW_CAMOUFLAGE (1 << 2)
#define AW_SNEAK      (1 << 3)

#define ACHECK_AFFECTS (1 << 0)
#define ACHECK_LIGHT   (1 << 1)
#define ACHECK_HUMMING (1 << 2)
#define ACHECK_GLOWING (1 << 3)
#define ACHECK_WEIGHT  (1 << 4)

int check_awake(struct char_data *ch, int what);
int awake_hide(struct char_data *ch);
int awake_invis(struct char_data *ch);
int awake_camouflage(struct char_data *ch);
int awake_sneak(struct char_data *ch);
int awaking(struct char_data *ch, int mode);

/* OS compatibility ******************************************************/


/* there could be some strange OS which doesn't have NULL... */
#ifndef NULL
#define NULL (void *)0
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE  (!FALSE)
#endif

/* defines for fseek */
#ifndef SEEK_SET
#define SEEK_SET        0
#define SEEK_CUR        1
#define SEEK_END        2
#endif

/*
 * NOCRYPT can be defined by an implementor manually in sysdep.h.
 * CIRCLE_CRYPT is a variable that the 'configure' script
 * automatically sets when it determines whether or not the system is
 * capable of encrypting.
 */
#if defined(NOCRYPT) || !defined(CIRCLE_CRYPT)
#define CRYPT(a,b) (a)
#else
#define CRYPT(a,b) ((char *) crypt((a),(b)))
#endif

#define SENDOK(ch)      ((ch)->desc && \
                         (to_sleeping || AWAKE(ch)) && \
                         !PLR_FLAGGED((ch), PLR_WRITING))



#define a_isspace(c) (strchr(" \f\n\r\t\v",(c)) != NULL)
#define a_ihspace(c) (strchr("*\f\n\r\t\v",(c)) != NULL)
#define a_inspace(c) (strchr("~",(c)) != NULL)
#define a_idspace(c) (strchr(".",(c)) != NULL)

#define a_isalpha(c) dl_isalpha(c)
#define a_isalnum(c) dl_isalnum(c)

#define IS_DIGIT(x) isdigit(x)

#define number(x, y) number_range(x, y)
#define RNDSKILL dice(1,100)

#endif                          /* MUD_UTILS_H */

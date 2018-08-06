#ifndef HANDLER_H
#define HANDLER_H
/* ************************************************************************
*   File: handler.h                                     Part of CircleMUD *
*  Usage: header file: prototypes of handling and utility functions       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "fight.h"

void change_fighting(struct char_data *ch, int need_stop);
void skill_modify(struct char_data *ch, int loc, int mod);

/* handling the affected-structures */
void affect_total(struct char_data *ch);
void affect_modify(struct char_data *ch, byte loc, int mod, bitvector_t bitv, bool add);
void affect_to_char(struct char_data *ch, struct affected_type *af);
void affect_remove(struct char_data *ch, struct affected_type *af);
void affect_from_char(struct char_data *ch, int type);
int affected_by_spell(struct char_data *ch, int type);
int affected_by_spell(struct obj_data *obj, int type);
int affected_by_spell_real(struct obj_data *obj, int type);
int affected_by_spell_real(struct char_data *ch, int type);
struct affected_type *get_affect_by_vector(struct char_data *ch, int vector);
int affect_join_char(struct char_data *ch, struct affected_type *aff);
void timed_to_char(struct char_data *ch, struct timed_type *timed);
void timed_from_char(struct char_data *ch, struct timed_type *timed);
int timed_by_skill(struct char_data *ch, int skill);

/* utility */
char *money_desc(int amount, int padis);
struct obj_data *create_money(int amount);
int isname(const char *str, const char *namelist);
int isfullname(const char *str, const char *namelist);
char *fname(const char *namelist);
int get_number(char **name);

/* ******** objects *********** */

void obj_to_char(struct obj_data *object, struct char_data *ch);
void obj_from_char(struct obj_data *object);

int preequip_char(struct char_data *ch, struct obj_data *obj, int pos);
int preequip_tatoo(struct char_data *ch, struct obj_data *obj, int where);
void postequip_char(struct char_data *ch, struct obj_data *obj);
void equiped_char(struct char_data *ch, struct obj_data *obj, int pos);
void equip_char(struct char_data *ch, struct obj_data *obj, int pos);
void equip_tatoo(struct char_data *ch, struct obj_data *obj, int pos);
struct obj_data *unequip_char(struct char_data *ch, int pos);
struct obj_data *unequip_tatoo(struct char_data *ch, int pos);

struct obj_data *get_obj_in_list_num(int num, struct obj_data *list);
struct obj_data *get_obj_num(obj_rnum nr);

int obj_decay(struct obj_data *object);
void obj_to_room(struct obj_data *object, room_rnum room);
void obj_from_room(struct obj_data *object);
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to);
void obj_from_obj(struct obj_data *obj);
void object_list_new_owner(struct obj_data *list, struct char_data *ch);

void extract_obj(struct obj_data *obj);
void extract_obj(struct obj_data *obj, int decay);

/* ******* characters ********* */

struct char_data *get_char_room(char *name, room_rnum room);
struct char_data *get_char_num(mob_rnum nr);

void char_from_room(struct char_data *ch);
void char_to_room(struct char_data *ch, room_rnum room);
void extract_char(struct char_data *ch, int clear_objs);

void update_char_objects(struct char_data *ch);

/* find if character can see */
struct char_data *get_char_room_vis2(struct char_data *ch, char *name, int *number);
struct char_data *get_char_room_vis(struct char_data *ch, char *name);
struct char_data *get_player_vis(struct char_data *ch, char *name, int inroom);
struct char_data *get_player_soul(struct char_data *ch, char *name, int inroom);

struct char_data *get_char_vis(struct char_data *ch, char *name, int where);
struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, struct obj_data *list);
struct obj_data *get_obj_in_list_vis2(struct char_data *ch, char *name,
                                      int *number, struct obj_data *list);
struct obj_data *get_obj_vis(struct char_data *ch, char *name);
struct obj_data *get_object_in_equip_vis(struct char_data *ch,
                                         char *arg, struct obj_data *equipment[], int *j);
int get_obj_pos_in_equip_vis(struct char_data *ch, char *arg, int *num,
                             struct obj_data *equipment[]);
struct char_data *find_char(struct char_data *ch, char *name);



/* find all dots */

int find_all_dots(char *arg);

#define FIND_INDIV 0
#define FIND_ALL 1
#define FIND_ALLDOT 2


/* Generic Find */

int generic_find(char *arg, bitvector_t bitvector, struct char_data *ch,
                 struct char_data **tar_ch, struct obj_data **tar_obj);

#define FIND_CHAR_ROOM     (1 << 0)
#define FIND_CHAR_WORLD    (1 << 1)
#define FIND_OBJ_INV       (1 << 2)
#define FIND_OBJ_ROOM      (1 << 3)
#define FIND_OBJ_WORLD     (1 << 4)
#define FIND_OBJ_EQUIP     (1 << 5)
#define FIND_CHAR_DISCONNECTED (1 << 6)
#define FIND_OBJ_TATOO     (1 << 7)

/* light */
#define LIGHT_NO    0
#define LIGHT_YES   1
#define LIGHT_UNDEF 2
void check_light(struct char_data *ch,
                 int was_equip, int was_single, int was_holylight, int was_holydark, int koef);

/* act.comm.cc */
void curses_check(char *words);
extern const char *languages_d[];

/* act.informative.cc */
void exam_obj_to_char(struct obj_data *object, struct char_data *ch, int bits, const char *arg);
char *diag_weapon_to_char(struct obj_data *obj, int show_wear);
void look_at_room(struct char_data *ch, int mode);
void read_book(struct obj_data *obj, struct char_data *ch);
char *find_exdesc(const char *word, struct extra_descr_data *list);
void commands_init();
void commands_destroy();

/* act.item.cc */
int focn(struct char_data *ch, int vnum);
void reci(struct char_data *ch, int vnum, int count);
int get_check_money(struct char_data *ch, struct obj_data *obj);
int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg);
void exit_trap_active(struct char_data *ch, int door, bool tbreak);
struct char_data *give_find_vict(struct char_data *ch, char *arg);
int tatoo_to_char(struct char_data *ch, struct char_data *keeper, struct obj_data *tatoo);
void name_to_drinkcon(struct obj_data *obj, int type);
void name_from_drinkcon(struct obj_data *obj);
void perform_remove(struct char_data *ch, int pos);
int perform_drop(struct char_data *ch, struct obj_data *obj, byte mode, const int sname,
                 room_rnum RDR);
int perform_get_from_room(struct char_data *ch, struct obj_data *obj);
void go_close_fiction(struct char_data *ch, struct obj_data *obj);
void go_break(struct char_data *ch, struct obj_data *obj);
void go_pick_door(struct char_data *ch, byte door);
void go_open_door(struct char_data *ch, byte door);
void go_close_door(struct char_data *ch, byte door);
void go_lock_door(struct char_data *ch, byte door);
void go_unlock_door(struct char_data *ch, byte door);
void get_pick_cont(struct char_data *ch, struct obj_data *obj);
void get_pick_door(struct char_data *ch, int door);
void corpse_desc_skelet(struct obj_data *obj);
void corpse_desc_rotten(struct obj_data *obj);


/* act.movement.cc */
int has_boat(struct char_data *ch);
int skip_sneaking(struct char_data *ch, struct char_data *vict);
int skip_camouflage(struct char_data *ch, struct char_data *vict);
int legal_dir(struct char_data *ch, int dir, int need_specials_check, int show_msg);
int do_simple_move(struct char_data *ch, int dir, int need_specials_check, bool show);
int perform_move(struct char_data *ch, int dir, int following, int checkmob);
int has_key(struct char_data *ch, obj_vnum key);
int legal_dir(struct char_data *ch, int dir, int need_specials_check, int show_msg);
int remove_char_from_obj(struct char_data *ch, struct obj_data *obj);
void go_enter(struct char_data *ch, struct obj_data *obj);
int check_death_trap(struct char_data *ch);
extern const int Reverse[];

/* act.offensive.cc */
int check_pkill(struct char_data *ch, struct char_data *victim, char *arg);
int onhorse(struct char_data *ch);
int set_hit(struct char_data *ch, struct char_data *victim);
int go_rescue(struct char_data *ch, struct char_data *vict, struct char_data *tmp_ch);
void go_addshot(struct char_data *ch, struct char_data *victim);
void go_backstab(struct char_data *ch, struct char_data *vict);
void go_bash(struct char_data *ch, struct char_data *vict);
void go_block(struct char_data *ch);
void go_bloodlet(struct char_data *ch, struct char_data *vict);
void go_chopoff(struct char_data *ch, struct char_data *vict);
void go_circlestab(struct char_data *ch, struct char_data *victim);
void go_counteract(struct char_data *ch, struct char_data *vict);
void go_critichit(struct char_data *ch, struct char_data *victim, int pos);
void go_dir_flee(struct char_data *ch, int direction);
void go_disarm(struct char_data *ch, struct char_data *vict);
void go_kick(struct char_data *ch, struct char_data *vict);
void go_mighthit(struct char_data *ch, struct char_data *victim);
void go_multyparry(struct char_data *ch);
void go_parry(struct char_data *ch);
void go_protect(struct char_data *ch, struct char_data *vict);
void go_stupor(struct char_data *ch, struct char_data *vict);
void go_throw(struct char_data *ch, struct char_data *vict);
void go_touch(struct char_data *ch, struct char_data *vict);

/* act.other.cc */
int low_charm(struct char_data *ch);
void go_dig(struct char_data *ch);
void go_steal(struct char_data *ch, struct char_data *vict, char *obj_name);


/* act.wizard.cc */
void perform_immort_vis(struct char_data *ch);

/* class.cc */
void advance_level(struct char_data *ch);
void decrease_level(struct char_data *ch);
int level_exp(struct char_data *ch, int level);
int saving_throws_3(struct char_data *ch, int type);
int parse_race(char);
void check_stats(struct char_data *ch);
void init_stats(struct char_data *ch);
void roll_stat(struct char_data *ch);
int generate_eyes(struct char_data *ch);
void do_start(struct char_data *ch, int newbie);
void do_newbie(struct char_data *ch);
void display_align(struct char_data *ch);
void display_gods(struct char_data *ch);
int parse_race(char arg);
int parse_align(struct char_data *ch, int arg);
int parse_gods(struct char_data *ch, int arg);
int invalid_no_class(struct char_data *ch, struct obj_data *obj);
int invalid_anti_class(struct char_data *ch, struct obj_data *obj);
int invalid_unique(struct char_data *ch, struct obj_data *obj);
extern const char *race_menu;

/* graph.cc */
void _do_track(struct char_data *ch, char *name);
int find_first_step(room_rnum src, room_rnum target, struct char_data *ch);
int check_trackon(struct char_data *ch, int room);
int go_track(struct char_data *ch, struct char_data *victim, int skill_no);

/* limits.cc */
int mana_gain(struct char_data *ch);
int hit_gain(struct char_data *ch);
int move_gain(struct char_data *ch);
void set_title(struct char_data *ch, const char *title);
void set_rtitle(struct char_data *ch, char *title);
void gain_exp(struct char_data *ch, int gain, bool show_mess);
void gain_condition(struct char_data *ch, int condition, int value);
void check_idling(struct char_data *ch);
void point_update(void);
int update_pos(struct char_data *victim);
void hour_update();
void tics_update();
void illness_update(void);
void obj_drop_down(struct obj_data *object, room_rnum room);
void obj_swim_down(struct obj_data *object, room_rnum room);
void ch_swim_down(struct char_data *ch);
void ch_drop_down(struct char_data *ch);
void affect_to_object(struct obj_data *obj, struct C_obj_affected_type *af);
void affect_from_object(struct obj_data *obj, int type);
void affect_obj_removed(struct obj_data *obj, struct C_obj_affected_type *af);
void affect_join_fspell_object(struct obj_data *obj, struct C_obj_affected_type *af);
void affect_join_object(struct obj_data *obj,
                        struct C_obj_affected_type *af, bool add_dur,
                        bool avg_dur, bool add_mod, bool avg_mod);
int affected_object_by_spell(struct obj_data *obj, int type);
void extra_modify_object(struct obj_data *obj, bitvector_t bitv, bool add);
void no_modify_object(struct obj_data *obj, bitvector_t bitv, bool add);
void anti_modify_object(struct obj_data *obj, bitvector_t bitv, bool add);
void affect_to_room(struct room_data *room, struct room_affect_data *af);
void affect_from_room(struct room_data *room, int type);
void affect_room_removed(struct room_data *room, struct room_affect_data *af);
void affect_join_room(struct room_data *room, struct room_affect_data *af,
                      bool add_dur, bool avg_dur, bool add_mod, bool avg_mod);
void affect_join_fspell_room(struct room_data *room, struct room_affect_data *af);
int affected_room_by_spell(struct room_data *room, int type);
int affected_room_by_spell_real(struct room_data *room, int type);
int affected_room_by_bitvector(struct room_data *room, bitvector_t bitv);
long get_spell_onwer_by_bitvector(struct room_data *room, bitvector_t bitv);
int calc_warn_dam(struct char_data *ch, int temp);

/* magic.cc */
int general_savingthrow(struct char_data *ch, int type, int ext_apply);
int general_savingthrow_3(struct char_data *ch, int type, int ext_apply);
int get_spell_value(struct char_data *ch, int spell_no);
long get_spell_owner(struct char_data *ch, int spell_no);
long get_spell_owner_real(struct char_data *ch, int spellnum);
void broth_weapon(struct char_data *ch, int damage);
bool miracle_action(struct char_data *ch, int type);
void battle_affect_update(struct char_data *ch);
void bt_affect_update(void);
void player_affect_update(void);
void show_spell_on(int aff, struct char_data *ch);
void show_spell_off(int aff, struct char_data *ch, obj_data * obj);
int mag_damage(int spellnum, int dam, struct char_data *ch,
               struct char_data *victim, int show_mess, int type_damage, int set);
int get_saves3(struct char_data *ch, int type);
int train_magic(struct char_data *ch, int spell_no, struct char_data *vict);

/* spec_procs.cc */
char *list_skills(struct char_data *ch, int type);
void list_spells(struct char_data *ch, struct char_data *victim, int);
void list_prays(struct char_data *ch, struct char_data *victim);
void list_enchants(struct char_data *ch, struct char_data *victim);
void npc_groupbattle(struct char_data *ch);
int npc_battle_scavenge(struct char_data *ch);
void npc_wield(struct char_data *ch);
void npc_armor(struct char_data *ch);
int npc_track(struct char_data *ch);
int npc_scavenge(struct char_data *ch);
int npc_loot(struct char_data *ch);
int npc_move(struct char_data *ch, int dir, int need_specials_check);
void npc_remove_staff(struct char_data *ch);
void npc_group(struct char_data *ch);
void npc_groupbattle(struct char_data *ch);
int npc_walk(struct char_data *ch);
void npc_steal(struct char_data *ch);
void npc_light(struct char_data *ch);
void spec_func_assign_room(room_rnum rn, const char *name);
void spec_func_assign_mob(mob_rnum imbs, const char *name);
void spec_func_assign_obj(obj_rnum iobj, const char *name);
void specials_destroy();
void specials_init();

/* weather.cc */
void weather_and_time(int mode);
int get_room_sky(int rnum);
extern const int sunrise[][2];

#endif

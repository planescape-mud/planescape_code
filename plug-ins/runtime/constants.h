#ifndef CONSTANTS_H
#define CONSTANTS_H

struct class_app_type {
    int    unknown_weapon_fault;
    int    koef_con;
    int    base_con;
    int    min_con;
    int    max_con;

    struct extra_affects_type *extra_affects;
    struct obj_affected_type  *extra_modifiers;
};

struct race_app_type {
    struct extra_affects_type *extra_affects;
    struct obj_affected_type  *extra_modifiers;
    struct skill_modify_type *extra_skills;
};

struct pray_affect_type {
    int  metter;
    int  location;
    int  modifier;
    long bitvector;
    int  battleflag;
};

struct drink_conds_data {
    int max_cond;   // сколько помещается кондиций
    int dec_cond;   // расход кондиций
};

/* Attacktypes with grammar */
struct attack_hit_type {
    const char   *singular;
    const char   *plural;
    const char   *fluar;
};



extern int wear_bitvectors[];
extern const char *missile_name[];
extern const char *missiles_name[];
extern const char *armor_class[];
extern const char *what_damage_type[];
extern const char *what_damage_type_eng[];
extern const char *damage_name[];
extern const char *saves_name[];
extern float add_classes[][13];
extern const char *body_name[];
extern const int debugz;
extern const char *dirs[];
extern const char *DirIs[];
extern const int rev_dir[];
extern int ExtraSkillValues[];
extern const int ExtraSkillValuesCol;
extern const char *ExtraSkillNames[];
extern const char *ExtraSkillNames2[];
extern const char *t_zone[];
extern const char *sphere_name[];
extern const char *sphere_name5[];
extern const char *sphere_names[];
extern const char *dig_messages[];
extern const char *blood_messages[];
extern const char *shot_types[];
extern const int  shot_damage[];
extern const int flee_koef[];
extern const char *imm_name[];
extern const char *DirsFrom[];
extern const char *DirsTo[];
extern const char *DirsTo_2[];
extern const char *room_bits[];
extern const char *room_affects[];
extern const char *exit_bits[];
extern const char *sector_types[];
extern const char *genders[];
extern const char *position_types[];
extern const char *position_types_sml[];
extern const char *player_bits[];
extern const char *action_bits[];
extern const char *function_bits[];
extern const char *preference_bits[];
extern const char *affected_bits[];
extern const char *connected_types[];
extern const int wear_order_index[NUM_WEARS];
extern const char *where[];
extern const char *where_tatoo[];
extern const char *equipment_types[];
extern const char *item_types[];
extern const char *item_types_stat[];
extern const char *wear_bits[];
extern const char *extra_bits[];
extern const char *apply_types[];
extern const char *material_name[];
extern const char *anti_bits[];
extern const char *no_bits[];
extern const char *no_bits_stat[];
extern const char *anti_bits_stat[];
extern const char *material_type[];
extern const int material_value[];
extern const char *tatoo_color [];
extern const char *container_bits[];
extern const char *drinks[];
extern const char *drinknames[];
extern const char *gods_name[];
extern const char *gods_name_2[];
extern const char *color_liquid[];
extern const char *eyes_color[];
extern const char *fullness[];
extern const char *spell_wear_off_msg[];
extern const char *class_abbrevs[];
extern const char *class_name[];
extern const char *align_name[];
extern const char *weekdays[];
extern const char *month_name[];
extern const char *ingradient_bits[];
extern const int exp_align[][ALIGN_NUM];
extern float change_align[][ALIGN_NUM];
extern const char *race_name[][NUM_SEXES];
extern const char *race_name_pad_male[];
extern const char *race_name_pad_female[];
extern const char *pray_metter[];
extern const char *pray_whom[];
extern struct class_app_type class_app[];
extern struct race_app_type race_app[];
extern struct pray_affect_type pray_affect[];
extern int movement_loss[];
extern int drink_aff[][3];
extern const int koef_spell[];
extern const char *weather_mess[2][8];
extern  const char *weapon_affects[];

#define STRANGE_ROOM     0

#define FIRE_MOVES       20
#define TRAP_MOVES       25
#define LOOKING_MOVES    5
#define HEARING_MOVES    2
#define LOOKHIDE_MOVES   3
#define SNEAK_MOVES      1
#define CAMOUFLAGE_MOVES 1
#define PICKLOCK_MOVES   20
#define CRASH_MOVES     15
#define TRACK_MOVES      3
#define SENSE_MOVES      5
#define HIDETRACK_MOVES  10
#define FIND_MOVES  25

extern int HORSE_VNUM;
extern int HORSE_COST;
extern int START_SWORD;
extern int START_CLUB;
extern int START_KNIFE;
extern int START_SCROLL;
extern int START_BOTTLE;
extern int CREATE_LIGHT;
extern int START_LIGHT;
extern int START_BREAD;
extern int START_ARMOR;
extern int START_LEGS;
extern int START_ARMS;
extern int START_FEET;
extern int START_SHIELD;
extern int START_WRUNE;
extern int START_ERUNE;
extern int START_ARUNE;
extern int START_FRUNE;
extern int START_LUK;

extern const int max_damage[];
extern const int max_armor[];
extern struct drink_conds_data thirst_race[NUM_RACES];
extern struct drink_conds_data drunk_race[NUM_RACES];
extern struct drink_conds_data full_race[NUM_RACES];
extern struct drink_conds_data sleep_race[NUM_RACES];
extern struct attack_hit_type attack_hit_text[];
extern const char *attack_hit_mess[];
extern const char *attack_name[];
extern const char *obj_temp[];
extern const int fire_damage_obj[];
extern const int move_wait_race[NUM_RACES];
extern const int eyes_race_evil[NUM_RACES][NUM_EYES];
extern const int eyes_race_neut[NUM_RACES][NUM_EYES];
extern const int eyes_race_good[NUM_RACES][NUM_EYES];
extern const int spells_defence_war[];
extern const int spells_defence[];
extern const int spells_health[];
extern const int spells_helped[];
extern const int spells_harmed[];
extern const int spells_damages[];
extern const int quality_at_durab[];
extern const int quality_at_ac[];
extern const int quality_at_cost[];
extern const int cost_hotel[];
extern const int max_obj_cost[];
extern const int wpos_to_wear[];
extern const int wear_to_wpos[];
extern const float mob_to_arm[NUM_RACES][NUM_CLASSES];
extern const int learn_charm[][2];
extern const char *action_name[];
extern const char *tool_name[];
extern const float progress_skill[];
#endif

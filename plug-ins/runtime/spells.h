#ifndef SPELLS_H
#define SPELLS_H
/* ************************************************************************
*   File: spells.h                                      Part of CircleMUD *
*  Usage: header file: constants and fn prototypes for spell system       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "spells-decl.h"

//Формула повреждений
#define SPLDAMAGE dice(x,y)+(z*level/w)

#define GET_SPHERE_NAME(i)     (sphere_name(i))
#define GET_SPHERE_NAME3(i)    (sphere_name(i))


#define ASPELL(spellname) \
    void    spellname(int spellnum, int level, struct char_data *ch, \
                      struct char_data *victim, struct obj_data *obj, int show_mess)

#define MANUAL_SPELL(spellname) spellname(spellnum,level, caster, cvict, ovict, show_mess);
#define SPECIAL_SPELL(spellname) spellname(level,ch,victim,NULL,FALSE);

/* basic magic calling functions */
ASPELL(skill_identify);
ASPELL(spell_identify);
ASPELL(spell_recall);
ASPELL(spell_cast_hold);
ASPELL(spell_de_mind);
//
ASPELL(spell_protect_from_evil);
ASPELL(spell_aid);
ASPELL(spell_circle_aganist_evil);
ASPELL(spell_holy_smite);
ASPELL(spell_dispel_evil);
ASPELL(spell_blade_barrier);
ASPELL(spell_holy_word);
ASPELL(spell_holy_aura);
ASPELL(spell_summon_monster);
ASPELL(spell_endure_elements);
ASPELL(spell_heat_metall);
ASPELL(spell_searing_light);
ASPELL(spell_fire_shield);
ASPELL(spell_flame_strike);
ASPELL(spell_fire_seeds);
ASPELL(spell_sunbeam);
ASPELL(spell_sunburst);
ASPELL(spell_fire_crown);
ASPELL(spell_prisma_sphere);
ASPELL(spell_cure);
ASPELL(spell_cure_mass);
ASPELL(spell_heal);
ASPELL(spell_heal_mass);
ASPELL(spell_remove_hold);
ASPELL(spell_remove_poison);
ASPELL(spell_remove_blind);
ASPELL(spell_remove_plaque);
ASPELL(spell_refresh);
ASPELL(spell_refresh_mass);
ASPELL(spell_ressurect);
ASPELL(spell_remove_curse);
ASPELL(spell_magic_weapon);
ASPELL(spell_magic_vestment);
ASPELL(spell_divine_power);
ASPELL(spell_flame_strike_war);
ASPELL(spell_power_word_stune);
ASPELL(spell_power_word_blind);
ASPELL(spell_power_word_kill);
ASPELL(spell_protect_from_good);
ASPELL(spell_desecrate);
ASPELL(spell_unholy_blight);
ASPELL(spell_dispel_good);
ASPELL(spell_create_undead);
ASPELL(spell_create_shadow);
ASPELL(spell_blasphemy);
ASPELL(spell_unholy_aura);
ASPELL(spell_summon_monster_2);
ASPELL(spell_endure_elements_p);
ASPELL(spell_bull_strengh);
ASPELL(spell_power_armor);
ASPELL(spell_magic_immunity);
ASPELL(spell_righteous_might);
ASPELL(spell_stone_skin);
ASPELL(spell_grasping_hand);
ASPELL(spell_clenched_fist);
ASPELL(spell_crushing_fist);
ASPELL(spell_fast_move);
ASPELL(spell_locate_object);
ASPELL(spell_levitation);
ASPELL(spell_locate_person);
ASPELL(spell_fly);
ASPELL(spell_teleport);
ASPELL(spell_free_moves);
ASPELL(spell_dimension_door);
ASPELL(spell_astral_project);
ASPELL(spell_sanctuary);
ASPELL(spell_oshield);
ASPELL(spell_imun_elements);
ASPELL(spell_save_will);
ASPELL(spell_nomagic_field);
ASPELL(spell_repulsion);
ASPELL(spell_mind_blank);
ASPELL(spell_prisma_sphere);
ASPELL(spell_entropic_shield);
ASPELL(spell_fpantacle);
ASPELL(spell_infravision);
ASPELL(spell_magic_parry);
ASPELL(spell_expand_skill);
ASPELL(spell_evil_fate);
ASPELL(spell_fast_learn);
ASPELL(spell_mislead);
ASPELL(spell_miracle);
ASPELL(spell_spirit_weapon);
ASPELL(spell_consecrate);
ASPELL(spell_aspect_god);
ASPELL(spell_broth_weapon);
ASPELL(spell_detect_magic);
ASPELL(spell_detect_undead);
ASPELL(spell_animate);
ASPELL(spell_protect_undead);
ASPELL(spell_cause_fear);
ASPELL(spell_skull_snare);
ASPELL(spell_animate_skl);
ASPELL(spell_make_skelet);
ASPELL(spell_freeze_blood);
ASPELL(spell_shadow_death);
ASPELL(spell_wakeup_dead);
ASPELL(spell_or);
ASPELL(spell_steel_bones);
ASPELL(spell_energy_undead);
ASPELL(spell_ghost_fear);
ASPELL(spell_slaved_shadow);
ASPELL(spell_make_ghola);
ASPELL(spell_implant_weapon);
ASPELL(spell_is_undead);
ASPELL(spell_bones_wall);
ASPELL(spell_bones_pick);
ASPELL(spell_thr_death);
ASPELL(spell_foul_flesh);
ASPELL(spell_prismatic_skin);
ASPELL(spell_death_arrows);
ASPELL(spell_shadow_protect);
ASPELL(spell_freedom_undead);
ASPELL(spell_mass_fear);
ASPELL(spell_death_weapon);
ASPELL(spell_bury);
ASPELL(spell_poison_fog);
ASPELL(spell_ressurect_necro);

ASPELL(spell_summon_horse);

/*
 * sp_define.h
 */
typedef struct t_spell_func_rec T_SPELL_FUNC_REC, *PT_SPELL_FUNC_REC;
typedef void (*PT_SPELL_FUNCTION) (int spellnum, int level, struct char_data * ch,
                                   struct char_data * victim, struct obj_data * obj, int show_mess);

struct t_spell_func_rec {
    const char *name;
    PT_SPELL_FUNCTION func;
};

/*
 * spell_parser.cc
 */
int find_skill_num(char *name);
int find_spell_num(char *name);
int find_spell_num(int spell_no);
int call_magic(struct char_data *caster, struct char_data *cvict,
               struct obj_data *ovict, int spellnum, int level, int casttype, int show_mess);
void mag_objectmagic(struct char_data *ch, struct obj_data *obj, char *argument);
int cast_spell(struct char_data *ch, struct char_data *tch, struct obj_data *tobj, int spellnum);
int may_kill_here(struct char_data *ch, struct char_data *victim);
int may_kill_here(struct char_data *ch, struct char_data *victim, int show);
void assign_skills(void);
const char *skill_name(int num);
const char *spell_name(int num);
int check_spell_agro(int spellnum, struct char_data *victim, int spell_agro);

/*
 * spells.cc
 */
void mort_show_char_values(struct char_data *victim, struct char_data *ch, int fullness);
void mort_show_obj_values(struct obj_data *obj, struct char_data *ch, int fullness);
void act_affect_mess(int spellnum, struct char_data *ch,
                     struct char_data *victim, int show_mess, int type);

#endif

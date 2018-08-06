#ifndef FIGHT_H
#define FIGHT_H

extern struct char_data *combat_list;   /* head of l-list of fighting chars */
extern struct char_data *next_combat_list;
extern int in_use;

int calc_damage_resist(struct char_data *ch, struct char_data *victim,
                       int dam, int hitloc, int hittype, int attacktype, int was_critic, int far);
void set_fighting(struct char_data *ch, struct char_data *victim);
void stop_fighting(struct char_data *ch, int switch_others);
void hit(struct char_data *ch, struct char_data *victim, int type, int weapon);
int _damage(struct char_data *ch, struct char_data *victim, int weapon,
            int style, int power, int set, struct P_damage &damage, struct P_message &message);
int _damage(struct char_data *ch, struct char_data *victim, int weapon,
            int style, int power, int set);
int get_attack_hit(struct char_data *ch, struct char_data *victim,
                   int skill, int victim_ac, int weapon);
int damage_wld(struct char_data *victim, room_rnum count, int show_mess);
int damage_obj(struct char_data *victim, struct obj_data *obj, int dam,
               int attacktype, int show_mess);
bool is_critic(struct char_data *ch, int koef, int slevel);
int real_attack_type(int type);
void die(struct char_data *ch, struct char_data *killer);
void death_cry(struct char_data *ch, struct char_data *victim);
void death(struct char_data *ch);
void char_dam_message(int dam, struct char_data *ch, struct char_data *victim, int attacktype);
int compute_ac_magic(struct char_data *ch, struct char_data *victim);
int get_weapon_damage(struct char_data *ch, struct char_data *victim,
                      struct obj_data *weapObj,
                      struct obj_data *missileObj, int isCritic,
                      int location, int afar, int slevel, int dual);
void perform_violence(void);
void perform_violence_magic(void);
void end_battle_round(void);
void clear_battle_affect(struct char_data *ch);
void mob_switch(struct char_data *ch);
void remove_last_attack(struct char_data *ch);

#endif

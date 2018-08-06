#ifndef AI_H
#define AI_H
/* Набор поведений монстров */

/*
 * ai.cc
 */
int mob_use(struct char_data *ch);
void mob_fight(struct char_data *ch);
int mob_casting_fight(struct char_data *ch);
void mob_casting_peace(struct char_data *ch);
bool check_in_zone(struct char_data *ch);
struct char_data *need_fa(struct char_data *ch, int spell_no);
struct char_data *need_heal(struct char_data *ch);
struct char_data *need_heal_battle(struct char_data *ch);

#define STYPE_NONE 0
#define STYPE_DAMAGE 1
#define STYPE_HELP 2
#define STYPE_DEFENCE 3
#define STYPE_HARM 4

/*
 * mobact.cc
 */
int extra_aggressive(struct char_data *ch, struct char_data *victim);
void mobile_activity(int activity_level);
void mobile_agress(int activity_level);
int attack_best(struct char_data *ch, struct char_data *vict);

#endif

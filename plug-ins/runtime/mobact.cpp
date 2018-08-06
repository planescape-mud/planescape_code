/* ************************************************************************
*   File: mobact.c                                      Part of CircleMUD *
*  Usage: Functions for generating intelligent (?) behavior in mobiles    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "wrapperbase.h"
#include "register-impl.h"

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "spells.h"
#include "constants.h"
#include "pk.h"
#include "ai.h"

/* extern functions */
ACMD(do_get);
ACMD(do_stand);
ACMD(do_rest);
ACMD(do_sleep);
ACMD(do_sit);
ACMD(do_wake);
ACMD(do_horseon);
ACMD(do_return);

/* local functions */
void npc_check_hide(struct char_data *ch);

#define MOB_AGGR_TO_ALIGN (MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL | MOB_AGGR_GOOD)
#define KILL_FIGHTING   (1 << 0)
#define CHECK_HITS      (1 << 10)
#define SKIP_CAMOUFLAGE (1 << 12)
#define SKIP_SNEAKING   (1 << 13)
#define KILL_ALL (1 << 14)
#define KILL_FLAGER (1 << 15)

int extra_aggressive(struct char_data *ch, struct char_data *victim)
{
    int time_ok = FALSE, no_time = TRUE, month_ok = FALSE, no_month = TRUE, agro = FALSE;

    if (!IS_NPC(ch))
        return (FALSE);

    if (IS_NPC(victim))
        return (FALSE);

    if (MOB_FLAGGED(ch, MOB_AGGRESSIVE))
        return (TRUE);

    if (MOB_FLAGGED(ch, MOB_XENO))
        return (TRUE);

    if (victim && MOB_FLAGGED(ch, MOB_AGGRGOOD) && !IS_NPC(victim) && IS_GOODS(victim))
        agro = TRUE;

    if (victim && MOB_FLAGGED(ch, MOB_AGGR_SLEEP) &&
        !IS_NPC(victim) && GET_POS(victim) == POS_SLEEPING)
        agro = TRUE;

    if (victim && MOB_FLAGGED(ch, MOB_AGGREVIL) && !IS_NPC(victim) && IS_EVILS(victim))
        agro = TRUE;

    if (victim && MOB_FLAGGED(ch, MOB_AGGRNEUTRAL) && !IS_NPC(victim) && IS_NEUTRALS(victim))
        agro = TRUE;

    if (MOB_FLAGGED(ch, MOB_AGGR_DAY)) {
        no_time = FALSE;
        if (weather_info.sunlight == SUN_RISE || weather_info.sunlight == SUN_LIGHT)
            time_ok = TRUE;
    }

    if (MOB_FLAGGED(ch, MOB_AGGR_NIGHT)) {
        no_time = FALSE;
        if (weather_info.sunlight == SUN_DARK || weather_info.sunlight == SUN_SET)
            time_ok = TRUE;
    }

    if (agro || !no_time || !no_month)
        return ((no_time || time_ok) && (no_month || month_ok));
    else
        return (FALSE);
}

int attack_best(struct char_data *ch, struct char_data *victim)
{
    if (victim) {
        if (GET_SKILL_MOB(ch, SKILL_BACKSTAB) && !FIGHTING(victim)) {
            go_backstab(ch, victim);
            return (TRUE);
        }

        if (GET_SKILL_MOB(ch, SKILL_ADDSHOT)) {
            struct obj_data *wield;

            wield = GET_EQ(ch, WEAR_WIELD) ? GET_EQ(ch, WEAR_WIELD) : GET_EQ(ch, WEAR_BOTHS);
            if (wield
                && (GET_OBJ_SKILL(wield) == SKILL_BOWS
                    || GET_OBJ_SKILL(wield) == SKILL_CROSSBOWS)) {
                go_addshot(ch, victim);
                return (TRUE);
            }
        }

        if (GET_SKILL_MOB(ch, SKILL_BASH)) {
            go_bash(ch, victim);
            return (TRUE);
        }
        /*if (GET_SKILL_MOB(ch, SKILL_STUPOR))
           {
           go_stupor(ch,victim);
           return (TRUE);
           } */
        if (GET_SKILL_MOB(ch, SKILL_BLOODLET)) {
            go_bloodlet(ch, victim);
            return (TRUE);
        }
        if (GET_SKILL_MOB(ch, SKILL_MIGHTHIT) && !timed_by_skill(ch, SKILL_MIGHTHIT)) {
            go_mighthit(ch, victim);
            return (TRUE);
        }
        if (GET_SKILL_MOB(ch, SKILL_DISARM)) {
            go_disarm(ch, victim);
            return (FALSE);
        }
        if (GET_SKILL_MOB(ch, SKILL_CHOPOFF)) {
            go_chopoff(ch, victim);
            set_hit(ch, victim);
            return (TRUE);
        }
        if (!FIGHTING(ch)) {
            _damage(ch, victim, WEAP_RIGHT, 0, C_POWER, TRUE);
            return (TRUE);
        }
    }


    return (FALSE);
}

bool agro_mob_attack(struct char_data * ch, int extmode)
{
    struct char_data *vict, *victim = NULL;
    int hp_victim = 9999;


    if (IN_ROOM(ch) == NOWHERE)
        return (FALSE);

    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
        if (vict == ch)
            continue;

        if (PRF_FLAGGED(vict, PRF_NOHASSLE) || !MAY_SEE(ch, vict) || !may_kill_here(ch, vict))
            continue;

        if ((check_victim_not_moved(vict, ch) && !check_victim_may_moved(vict, ch)) ||
            (check_victim_not_attack(vict, ch) && !check_victim_may_attack(vict, ch)))
            // skip sneaking, hiding and camouflaging pc
            if (IS_SET(extmode, SKIP_SNEAKING)) {
                skip_sneaking(vict, ch);
                if (EXTRA_FLAGGED(vict, EXTRA_FAILSNEAK))
                    REMOVE_BIT(AFF_FLAGS(vict, AFF_SNEAK), AFF_SNEAK);
                if (AFF_FLAGGED(vict, AFF_SNEAK))
                    continue;
            }
        if (IS_SET(extmode, SKIP_CAMOUFLAGE)) {
            skip_camouflage(vict, ch);
            if (EXTRA_FLAGGED(vict, EXTRA_FAILCAMOUFLAGE))
                REMOVE_BIT(AFF_FLAGS(vict, AFF_CAMOUFLAGE), AFF_CAMOUFLAGE);
        }

        if (!CAN_SEE(ch, vict))
            continue;

        if ((IS_UNDEAD(ch) && IS_AFFECTED(vict, AFF_IS_UNDEAD))
            || affected_by_spell(ch, SPELL_CAUSE_FEAR))
            continue;

        if (IS_SET(extmode, KILL_FIGHTING) &&
            FIGHTING(vict) &&
            FIGHTING(vict) != ch &&
            IS_NPC(FIGHTING(vict)) &&
            !AFF_FLAGGED(FIGHTING(vict), AFF_CHARM) &&
            !AFF_FLAGGED(FIGHTING(vict), AFF_HELPER) &&
            !NPC_FLAGGED(FIGHTING(vict), NPC_NOHELPED) && SAME_ALIGN_HELP(ch, FIGHTING(vict))) {
            hp_victim = GET_HIT(FIGHTING(vict));
            victim = vict;
            break;
        } else if (IS_SET(extmode, KILL_FLAGER)
                   && KILLER(vict)
                   && (!IS_NPC(FIGHTING(vict))
                       || (!IS_ANIMAL(FIGHTING(vict))
                           && !NPC_FLAGGED(FIGHTING(vict), NPC_NOHELPED)))
                   && !MOB_FLAGGED(vict, MOB_GUARD)) {
            hp_victim = GET_HIT(FIGHTING(vict));
            victim = vict;
            break;
        } else
            if (IS_SET(extmode, KILL_ALL) && hp_victim > GET_HIT(vict)
                && !MOB_FLAGGED(vict, MOB_XENO)) {
            //log("%s атакует %s",GET_NAME(ch),GET_NAME(vict));
            hp_victim = GET_HIT(vict);
            victim = vict;
        } else if (extra_aggressive(ch, vict) && hp_victim > GET_HIT(vict)) {
            hp_victim = GET_HIT(vict);
            victim = vict;
        }
    }

    if (victim && victim != ch) {
        do_stand(ch, 0, 0, 0, 0);
        if ((IS_SET(extmode, KILL_FIGHTING) || IS_SET(extmode, KILL_FLAGER)) && FIGHTING(victim)
            && GET_POS(ch) == POS_STANDING && !check_victim_not_attack(victim, ch)
            && !check_victim_not_moved(victim, ch)) {
            act("$n вступил$g в битву на стороне $N1.", FALSE, ch, 0, FIGHTING(victim), TO_NOTVICT);
            act("$n вступил$g в битву на Вашей стороне.", FALSE, ch, 0, FIGHTING(victim), TO_VICT);
        }

        if (!attack_best(ch, victim) && !FIGHTING(ch)) {
            _damage(ch, victim, WEAP_RIGHT, 0, C_POWER, TRUE);
        }
        GET_WAIT(ch) = PULSE_VIOLENCE;
        return (TRUE);
    }

    return (FALSE);
}

bool memory_mob_attack(struct char_data * ch, int extmode)
{
    struct char_data *vict, *victim = NULL;
    int hp_victim = 9999;

    if (IN_ROOM(ch) == NOWHERE)
        return (FALSE);

    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
        if (vict == ch)
            continue;

        if (check_victim_not_moved(vict, ch) || check_victim_not_attack(vict, ch))
            continue;

        if (PRF_FLAGGED(vict, PRF_NOHASSLE) ||
            IS_NPC(vict) || !MAY_SEE(ch, vict) || !may_kill_here(ch, vict))
            continue;

        if (ch->pk_list.count(GET_ID(vict))) {
            // skip sneaking, hiding and camouflaging pc
            if (IS_SET(extmode, SKIP_SNEAKING)) {
                skip_sneaking(vict, ch);
                if (EXTRA_FLAGGED(vict, EXTRA_FAILSNEAK))
                    REMOVE_BIT(AFF_FLAGS(vict, AFF_SNEAK), AFF_SNEAK);
                if (AFF_FLAGGED(vict, AFF_SNEAK))
                    continue;
            }
            if (IS_SET(extmode, SKIP_CAMOUFLAGE)) {
                skip_camouflage(vict, ch);
                if (EXTRA_FLAGGED(vict, EXTRA_FAILCAMOUFLAGE))
                    REMOVE_BIT(AFF_FLAGS(vict, AFF_CAMOUFLAGE), AFF_CAMOUFLAGE);
            }
            if (!CAN_SEE(ch, vict))
                continue;

            if (hp_victim > GET_HIT(vict))
                victim = vict;
        }
    }

    if (victim) {
        do_stand(ch, 0, 0, 0, 0);
        if (!attack_best(ch, victim) && !FIGHTING(ch))
            _damage(ch, victim, WEAP_RIGHT, 0, C_POWER, TRUE);
        return (TRUE);
    }

    return (FALSE);
}

int helper_mob_attack(struct char_data *ch)
{
    int in_room;
    struct char_data *master, *vict;

    master = ch->master;
    if (!master || !FIGHTING(master))
        return (FALSE);

    in_room = IN_ROOM(ch);

    if (in_room == NOWHERE)
        return (FALSE);

    for (vict = world[in_room].people; vict; vict = vict->next_in_room)
//  if (FIGHTING(vict) == master &&
        if (FIGHTING(master) == vict &&
            !SAME_ALIGN_HELPER(ch, vict) && CAN_SEE(ch, vict) &&
            !check_victim_not_attack(vict, ch) && !check_victim_not_moved(vict, ch)) {
            do_stand(ch, 0, 0, 0, 0);

            act("Вы присоединились к битве, помогая $N2!", FALSE, ch, 0, master, TO_CHAR);
            act("$N решил$G помочь Вам в битве!", 0, master, 0, ch, TO_CHAR);
            act("$n вступил$g в бой на стороне $N1.", FALSE, ch, 0, master, TO_NOTVICT);

            if (!attack_best(ch, vict) && !FIGHTING(ch))
                _damage(ch, vict, WEAP_RIGHT, 0, C_POWER, TRUE);

            WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
            return (TRUE);
        }

    return (FALSE);
}

/* Вызывается для монстра ch на проверку о его агрессии */
void do_aggressive_mob(struct char_data *ch, int check_sneak)
{
    int mode = check_sneak ? SKIP_SNEAKING : 0, level;


    if (IN_ROOM(ch) == NOWHERE)
        return;

    if (!IS_NPC(ch) || !MAY_ATTACK(ch) || AFF_FLAGGED(ch, AFF_BLIND) ||
        AFF_FLAGGED(ch, AFF_HOLD) || AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_STUNE))
        return;

    /* Монстр проверяет бьют его лошадь или нет */
    if (has_horse(ch, TRUE) && get_horse(ch) && !FIGHTING(ch) && GET_POS(ch) > POS_SLEEPING) {
        if (FIGHTING(get_horse(ch))) {
            if (GET_POS(ch) <= POS_SITTING)
                do_stand(ch, 0, 0, 0, 0);
            _damage(ch, FIGHTING(get_horse(ch)), WEAP_RIGHT, 0, C_POWER, TRUE);
            return;
        }
    }

    /* Алгоритм нападения для агромонстров различных типов */
    if ((MOB_FLAGGED(ch, MOB_AGGRESSIVE) ||
         MOB_FLAGGED(ch, MOB_AGGR_SLEEP) ||
         MOB_FLAGGED(ch, MOB_AGGR_DAY) ||
         MOB_FLAGGED(ch, MOB_AGGR_NIGHT) ||
         MOB_FLAGGED(ch, MOB_AGGRGOOD) ||
         MOB_FLAGGED(ch, MOB_AGGREVIL) || MOB_FLAGGED(ch, MOB_AGGRNEUTRAL)
         /*      MOB_FLAGGED(ch, MOB_AGGR_WINTER) ||
            MOB_FLAGGED(ch, MOB_AGGR_SUMMER) ||
            MOB_FLAGGED(ch, MOB_AGGR_SPRING) ||
            MOB_FLAGGED(ch, MOB_AGGR_AUTUMN) */
        ) && agro_mob_attack(ch, mode | SKIP_CAMOUFLAGE))
        return;

    /* Ксеноморфы */
    if (MOB_FLAGGED(ch, MOB_XENO) && agro_mob_attack(ch, mode | KILL_ALL | SKIP_CAMOUFLAGE))
        return;

    /* Охраники */
    if (MOB_FLAGGED(ch, MOB_GUARD) && agro_mob_attack(ch, mode | KILL_FLAGER | SKIP_CAMOUFLAGE))
        return;

    /* Помогающие другим монстрам */
    if (MOB_FLAGGED(ch, MOB_HELPER) && agro_mob_attack(ch, mode | KILL_FIGHTING | SKIP_CAMOUFLAGE))
        return;

    /* Злопамятные монстры */
    if (!ch->pk_list.empty())
        if (memory_mob_attack(ch, mode | SKIP_CAMOUFLAGE))
            return;

    if ((level = affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_POISON_FOG))) {
        struct char_data *owner = NULL;
        int id = get_spell_onwer_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_POISON_FOG);

        if ((owner = get_char_by_id(id)))
            if (!IS_UNDEAD(ch) && !IS_CONSTRUCTION(ch) && !same_group(owner, ch) && owner != ch &&
                may_kill_here(owner, ch, FALSE)) {
                do_stand(ch, 0, 0, 0, 0);
                if (!attack_best(ch, owner) && !FIGHTING(ch))
                    _damage(ch, owner, WEAP_RIGHT, 0, C_POWER, TRUE);
            }
    }

    /* Нанятые помогать монстры */
    if (IS_AFFECTED(ch, AFF_HELPER) && helper_mob_attack(ch))
        return;

}

void mob_object_check(struct char_data *ch)
{
    ACMD(do_remove);
    ACMD(do_drop);
    struct obj_data *tobj, *tobj_next;
    int pos, dam = 0;

    if (ch == NULL)
        return;

    for (pos = 0; pos < NUM_WEARS; pos++)
        if (GET_EQ(ch, pos)) {
            tobj = GET_EQ(ch, pos);
            if (is_metall(tobj) && GET_OBJ_REAL_TEMP(tobj) >= 30) {
                dam = calc_warn_dam(ch, get_const_obj_temp(tobj));
                if (dam > 0)
                    do_remove(ch, GET_OBJ_PNAME(tobj, 0), 0, 0, 1);
            }
        }

    if (ch->carrying)
        for (tobj = ch->carrying; tobj; tobj = tobj_next) {
            tobj_next = tobj->next_content;
            if (is_metall(tobj) && GET_OBJ_REAL_TEMP(tobj) >= 30) {
                dam = calc_warn_dam(ch, get_const_obj_temp(tobj));
                if (dam > 0)
                    do_drop(ch, GET_OBJ_PNAME(tobj, 0), SCMD_DROP, 0, 1);
            }
        }

}

// Проверка на агрессию
void mobile_agress(int activity_level)
{
    struct char_data *ch, *next_ch;

    for (ch = character_list; ch; ch = next_ch) {
        next_ch = ch->next;

        if (!IS_MOB(ch))
            continue;

        /*if (GET_WAIT(ch) > 0)
           GET_WAIT(ch)--;
           else
           GET_WAIT(ch) = 0; */

        do_aggressive_mob(ch, FALSE);
    }
}

static void mprog_pulse(struct char_data *ch)
{
    FENIA_VOID_CALL(ch, "Pulse", "", ch);
    FENIA_PROTO_VOID_CALL(ch->npc(), "Pulse", "C", ch);
}

// AI монстра каждый пульс
void mobile_activity(int activity_level)
{
    struct char_data *ch, *next_ch, *vict, *first;
    struct room_direction_data *rdata = NULL;
    int door, found, max, was_in, activity_lev, std_lev;

    std_lev = activity_level % PULSE_MOBILE;

    for (ch = character_list; ch; ch = next_ch) {
        next_ch = ch->next;

        if (!IS_MOB(ch))
            continue;

        if (MOB_FLAGGED(ch, MOB_CLONE)) {
            if (EXTRACT_TIMER(ch) > 0) {
                EXTRACT_TIMER(ch)--;
                if (!EXTRACT_TIMER(ch)) {
                    send_to_charf(ch, "Ваше время истекло.\r\n");
                    if (ch->desc && ch->desc->original)
                        do_return(ch, 0, 0, 0, 0);

                    extract_char(ch, FALSE);
                    continue;
                }
            }
            continue;
        }
        //Если уже пришли в локацию, то отменяем движение
        if (ch->npc()->specials.move_to == IN_ROOM(ch))
            ch->npc()->specials.move_to = NOWHERE;

        /*   if (GET_WAIT(ch) > 0)
           {
           GET_WAIT(ch)--;
           continue;
           }
           else
           GET_WAIT(ch) = 0; */

        if (ch->npc()->specials.move_to != NOWHERE)
            activity_lev = activity_level % (2 RL_SEC);
        else if (IS_AFFECTED(ch, AFF_CHARM) && ch->master && IN_ROOM(ch) != IN_ROOM(ch->master))
            activity_lev = GET_ACTIVITY(ch) + number(0, 2);
        else if (ch->npc()->specials.speed <= 0)
            activity_lev = std_lev;
        else
            activity_lev = activity_level % (ch->npc()->specials.speed RL_PSEC);

        if (GET_ACTIVITY(ch) != activity_lev ||
            (was_in = IN_ROOM(ch)) == NOWHERE || GET_ROOM_VNUM(IN_ROOM(ch)) % 100 == 99)
            continue;

        do_aggressive_mob(ch, FALSE);

        //надо бы выкинуть лишние вещи
//   if (MAY_MOVE(ch))
        mob_object_check(ch);

        // Удаляем левых купленых лошадей
        if (!ch->master && AFF_FLAGGED(ch, AFF_HORSE_BUY) && !EXTRACT_TIMER(ch)) {
            //sprintf(buf,"Удаляю ездовое %s",GET_NAME(ch));
            //mudlog(buf, CMP, LVL_GOD, TRUE);
            //extract_char(ch,FALSE);
            EXTRACT_TIMER(ch) = number(40, 80);;
            continue;
        }

        if (ch->master &&
            MOB_FLAGGED(ch, MOB_MOUNTING) &&
            !EXTRACT_TIMER(ch) &&
            !AFF_FLAGGED(ch, AFF_TETHERED) && (IN_ROOM(ch) != IN_ROOM(ch->master))
            //!AFF_FLAGGED(ch,AFF_HORSE) &&
            //!AFF_FLAGGED(ch->master, AFF_HORSE)
            ) {
            //sprintf(buf,"Удаляю ездовое2 %s %s",GET_NAME(ch),MOB_FLAGGED(ch, MOB_MOUNTING) ? "да":"нет");
            //mudlog(buf, CMP, LVL_GOD, TRUE);
            EXTRACT_TIMER(ch) = number(100, 160);
            continue;
        }
        // Удаляем разчармленых монстров
        if (EXTRACT_TIMER(ch) > 0 && GET_POS(ch) == POS_STANDING) {
            if (ch->master && (IN_ROOM(ch->master) == IN_ROOM(ch)))
                EXTRACT_TIMER(ch) = 0;
            else {
                EXTRACT_TIMER(ch)--;
                if (!EXTRACT_TIMER(ch)) {
                    extract_char(ch, FALSE);
                    continue;
                }
            }
        }

        /* Дошли */
        if (ch->npc()->specials.move_to != NOWHERE && IN_ROOM(ch) != NOWHERE &&
            ch->npc()->specials.move_to == IN_ROOM(ch))
            ch->npc()->specials.move_to = NOWHERE;

        //Специальная обработка для потеряных чармисов
        if (IS_AFFECTED(ch, AFF_CHARM) && ch->master && IN_ROOM(ch) != IN_ROOM(ch->master)) {
            int dr;

            dr = find_first_step(IN_ROOM(ch), IN_ROOM(ch->master), ch);

            if ((dr >= 0 && dr < NUM_OF_DIRS) &&
                GET_POS(ch) == POS_STANDING &&
                EXIT(ch, dr) && EXIT(ch, dr)->to_room != NOWHERE &&
                legal_dir(ch, dr, TRUE, FALSE) &&
                !world[EXIT(ch, dr)->to_room].forbidden &&
                !IS_MOUNT(ch) &&
                !AFF_FLAGGED(ch, AFF_TETHERED) &&
                (!MOB_FLAGGED(ch, MOB_STAY_ZONE) ||
                 world[EXIT(ch, dr)->to_room].zone == world[ch->in_room].zone)) {
                if (GET_POS(ch) == POS_SITTING)
                    do_stand(ch, 0, 0, 0, 0);
                npc_move(ch, dr, 1);
                npc_group(ch);
            }
            continue;
        }
        //Вдруг моб воюет
        if (FIGHTING(ch) && GET_MISSED(ch) <= 0)
            mob_fight(ch);

        // If the mob has no specproc, do the default actions
        if (FIGHTING(ch) ||
            GET_POS(ch) <= POS_STUNNED ||
            GET_WAIT(ch) > 0 ||
            AFF_FLAGGED(ch, AFF_CHARM) ||
            AFF_FLAGGED(ch, AFF_HOLD) ||
            AFF_FLAGGED(ch, AFF_STOPFIGHT) ||
            AFF_FLAGGED(ch, AFF_STUNE) || AFF_FLAGGED(ch, AFF_SLEEP))
            continue;

        if (IS_HORSE(ch)) {
            if (GET_POS(ch) < POS_FIGHTING)
                do_stand(ch, 0, 0, 0, 0);
            //GET_POS(ch) = POS_STANDING;
            continue;
        }
        //Изменение положения монстра
        if (GET_POS(ch) == POS_SLEEPING && GET_DEFAULT_POS(ch) > POS_SLEEPING) {
            GET_POS(ch) = GET_DEFAULT_POS(ch);
            do_wake(ch, 0, SCMD_WAKE, 0, 0);
        }

        if (!AWAKE(ch))
            continue;

        for (vict = world[ch->in_room].people, max = FALSE; vict; vict = vict->next_in_room) {
            if (ch == vict)
                continue;
            if (FIGHTING(vict) == ch)
                break;
            if (!IS_NPC(vict) && CAN_SEE(ch, vict) && (may_pkill(ch, vict) == PC_REVENGE_PC))
                max = TRUE;
        }

        // Монстр атакован
        if (vict)
            continue;

        // Монстр должен отдохнуть
        /*if (!max && !AFF_FLAGGED(ch, AFF_HOLD)  &&
           GET_HIT(ch) < GET_REAL_MAX_HIT(ch) &&
           GET_POS(ch) > POS_RESTING &&
           GET_HIT(ch) > GET_WIMP_LEV(ch) &&
           !NPC_FLAGGED(ch, NPC_NOREST)) */
        int zkill = check_killer_zone(ch);

        if (!max && !NPC_FLAGGED(ch, NPC_NOREST) &&
            GET_HIT(ch) < GET_REAL_MAX_HIT(ch) && !zkill &&
            GET_POS(ch) > POS_RESTING && !IS_UNDEAD(ch))
            //&& (int)((float)GET_HIT(ch)*100/(float)GET_REAL_MAX_HIT(ch)) > GET_WIMP_LEV(ch))
        {
            do_rest(ch, 0, 0, 0, 0);
        } else
            if ((max && !AFF_FLAGGED(ch, AFF_HOLD) &&
                 (GET_HIT(ch) < GET_REAL_MAX_HIT(ch)) && GET_POS(ch) == POS_RESTING) || zkill) {
            do_stand(ch, 0, 0, 0, 0);
        }
        // Монстр возвращается в исходную позицию по умолчанию
        if (GET_HIT(ch) >= GET_REAL_MAX_HIT(ch) &&
            GET_POS(ch) != GET_DEFAULT_POS(ch) && GET_POS(ch) >= POS_SLEEPING)
            switch (GET_DEFAULT_POS(ch)) {
                case POS_STANDING:
                    do_stand(ch, 0, 0, 0, 0);
                    break;
                case POS_SITTING:
                    do_sit(ch, 0, 0, 0, 0);
                    break;
                case POS_RESTING:
                    do_rest(ch, 0, 0, 0, 0);
                    break;
                case POS_SLEEPING:
                    do_sleep(ch, 0, 0, 0, 0);
                    break;
            }
        //Может надо взобраться на лошадь?
        if (has_horse(ch, TRUE) && get_horse(ch))
            if (GET_POS(get_horse(ch)) == POS_STANDING && GET_POS(ch) == GET_DEFAULT_POS(ch))
                do_horseon(ch, 0, 0, 0, 0);

        // Если не воюем надо бы проверить ситуацию в мире
        if (!FIGHTING(ch))
            mob_casting_peace(ch);

        // Если монстр атакован кем-то
        if (FIGHTING(ch) || GET_WAIT(ch) > 0)
            continue;

        //Спецобработчики
        // Моб подбирает предметы
        if (MOB_FLAGGED(ch, MOB_SCAVENGER) && !MOB_FLAGGED(ch, MOB_NOFIGHT))
            npc_scavenge(ch);

        // Моб лутит трупы
        if (MOB_FLAGGED(ch, MOB_LOOTER) && !MOB_FLAGGED(ch, MOB_NOFIGHT))
            npc_loot(ch);

        npc_remove_staff(ch);

        // Моб вооружается
        if (NPC_FLAGGED(ch, NPC_WIELDING) && !MOB_FLAGGED(ch, MOB_NOFIGHT))
            npc_wield(ch);

        // Моб одевается
        if (NPC_FLAGGED(ch, NPC_ARMORING))
            npc_armor(ch);

        // Моб ворует
        if (GET_SKILL_MOB(ch, SKILL_STEAL) && !MOB_FLAGGED(ch, MOB_NOFIGHT))
            npc_steal(ch);

        // Устрановка аффектов монстра
        if (GET_POS(ch) == POS_STANDING && NPC_FLAGGED(ch, NPC_INVIS))
            SET_BIT(AFF_FLAGS(ch, AFF_INVISIBLE), AFF_INVISIBLE);
        /* if (GET_POS(ch) == POS_STANDING && NPC_FLAGGED(ch, NPC_MOVEFLY))
           SET_BIT(AFF_FLAGS(ch, AFF_FLY), AFF_FLY); */

        // Монстр прячется
        if (GET_POS(ch) == POS_STANDING && GET_SKILL_MOB(ch, SKILL_HIDE) &&
            !IS_AFFECTED(ch, AFF_HIDE) && !MOB_FLAGGED(ch, MOB_NOFIGHT)) {
            struct affected_type af;
            struct char_data *tch;
            int skill = GET_SKILL_MOB(ch, SKILL_HIDE);

            af.type = find_spell_num(SPELL_HIDE);
            af.duration = MAX(SECS_PER_MUD_ROUND, skill * 9);
            af.modifier = skill;
            af.location = APPLY_NONE;
            af.main = TRUE;
            af.owner = GET_ID(ch);
            af.battleflag = 0;
            af.bitvector = AFF_HIDE;
            affect_to_char(ch, &af);

            for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
                if (tch == ch)
                    continue;
                add_victim_visible(tch, ch);
            }
            affect_total(ch);
        }
        //Монстр маскируется
        if (GET_POS(ch) == POS_STANDING && GET_SKILL_MOB(ch, SKILL_CAMOUFLAGE)
            && !MOB_FLAGGED(ch, MOB_NOFIGHT)) {
            if (GET_SKILL_MOB(ch, SKILL_CAMOUFLAGE) >= number(0, 100))
                SET_BIT(AFF_FLAGS(ch, AFF_CAMOUFLAGE), AFF_CAMOUFLAGE);
            else
                REMOVE_BIT(AFF_FLAGS(ch, AFF_CAMOUFLAGE), AFF_CAMOUFLAGE);
            affect_total(ch);
        }
        door = BFS_ERROR;
        npc_group(ch);

        // Мобы идут к точке назначения
        if (door == BFS_ERROR &&
            MOB_FLAGGED(ch, MOB_HELPER) &&
            !MOB_FLAGGED(ch, MOB_SENTINEL) &&
            !AFF_FLAGGED(ch, AFF_BLIND) && !ch->master && GET_POS(ch) == POS_STANDING) {
            for (found = FALSE, door = 0; door < NUM_OF_DIRS; door++) {
                for (rdata = EXIT(ch, door), max = MAX(1, GET_REAL_INT(ch) / 10); max > 0 && !found;
                     max--) {
                    if (!rdata || rdata->to_room == NOWHERE || IS_SET(rdata->exit_info, EXIT_CLOSED)
                        || !legal_dir(ch, door, TRUE, FALSE) || world[rdata->to_room].forbidden
                        || IS_DARK(rdata->to_room) || (MOB_FLAGGED(ch, MOB_STAY_ZONE)
                                                       && world[IN_ROOM(ch)].zone !=
                                                       world[rdata->to_room].zone))
                        break;
                    for (first = world[rdata->to_room].people; first; first = first->next_in_room)
                        if (IS_NPC(first) && !AFF_FLAGGED(first, AFF_CHARM) &&
                            !IS_HORSE(first) && CAN_SEE(ch, first) &&
                            FIGHTING(first) && SAME_ALIGN(ch, first)) {
                            found = TRUE;
                            break;
                        }
                    rdata = world[rdata->to_room].dir_option[door];
                }
                if (found)
                    break;
            }
            if (!found)
                door = BFS_ERROR;
        }
        if (GET_POS(ch) > POS_FIGHTING &&
            door == BFS_ERROR && ch->npc()->specials.move_to != NOWHERE) {
            door = find_first_step(ch->in_room, ch->npc()->specials.move_to, ch);
        } else
            /*if (NPC_FLAGGED(ch, NPC_TRACK) &&
               !ch->pk_list.empty() &&
               GET_POS(ch) > POS_FIGHTING &&
               !AFF_FLAGGED(ch,AFF_BLIND) &&
               door == BFS_ERROR)
               {
               log("Трек 000");
               log("Трек имя %s",GET_NAME(ch));
               door = npc_track(ch);
               log("Трек ***");
               }
               else */
        if (GET_DEST(ch) != NOWHERE && GET_POS(ch) > POS_FIGHTING && door == BFS_ERROR) {
            door = npc_walk(ch);
        }
        if (door == BFS_ERROR) {
            door = number(0, 18);
        }

        if (door == BFS_ALREADY_THERE) {
            do_aggressive_mob(ch, FALSE);
            continue;
        }
        /* Передвижения монстра */
        if ((!MOB_FLAGGED(ch, MOB_SENTINEL) || GET_MOVE_TO(ch) != NOWHERE) &&
            (door >= 0 && door < NUM_OF_DIRS) &&
            ((GET_POS(ch) == POS_SITTING && GET_DEFAULT_POS(ch) == POS_SITTING && number(1, 5) == 5)
             || GET_POS(ch) == POS_STANDING) &&
            EXIT(ch, door) && EXIT(ch, door)->to_room != NOWHERE &&
            legal_dir(ch, door, TRUE, FALSE) &&
            !world[EXIT(ch, door)->to_room].forbidden &&
            !IS_MOUNT(ch) &&
            !AFF_FLAGGED(ch, AFF_TETHERED) &&
            (!MOB_FLAGGED(ch, MOB_STAY_ZONE) ||
             world[EXIT(ch, door)->to_room].zone == world[ch->in_room].zone)) {
            if (GET_POS(ch) == POS_SITTING)
                do_stand(ch, 0, 0, 0, 0);
            npc_move(ch, door, 1);
            npc_group(ch);
            npc_check_hide(ch); //проверяем прячущихся (для прокачки скилла only)
            npc_groupbattle(ch);
        }
        npc_light(ch);

        /*   if (was_in != IN_ROOM(ch))
           do_aggressive_mob(ch, FALSE); */

	// Феня-триггер
	mprog_pulse(ch);
    }
}

void npc_check_hide(struct char_data *ch)
{
    int prob, percent;
    struct char_data *vict;

    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
        if (IS_NPC(vict))
            continue;
        if (!IS_AFFECTED(vict, AFF_HIDE))
            continue;

        int skill = GET_SKILL(vict, SKILL_HIDE);

        /*проверка монстра на агро
           по просьбам трудящихся типа Эритнула - убрано
           if (!CHECK_AGRO(ch,vict) ||
           (MOB_FLAGGED(ch, MOB_MEMORY) &&
           may_pkill(ch,vict) != PC_REVENGE_PC))
           continue; */

//if (!IS_AFFECTED(vict,AFF_INVISIBLE))
        if (skill > 25) {
            if (extra_aggressive(ch, vict)
                || (MOB_FLAGGED(ch, MOB_MEMORY) && may_pkill(ch, vict) == PC_REVENGE_PC))
                improove_skill(vict, ch, 0, SKILL_HIDE);
        } else {
            improove_skill(vict, ch, 0, SKILL_HIDE);
        }

        if (affected_by_spell(vict, SPELL_HIDE)) {
            percent = number(1, 100);

            prob = skill + GET_REAL_DEX(vict) - GET_REAL_INT(ch);

            if (prob < percent)
                add_victim_visible(vict, ch);
        }
    }
}

/* ************************************************************************
*   File: magic.c                                       Part of CircleMUD *
*  Usage: low-level functions for magic; spell template code              *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "handler.h"
#include "db.h"
#include "ai.h"
#include "interpreter.h"
#include "screen.h"
#include "constants.h"
#include "xspells.h"
#include "xboot.h"
#include "pk.h"
#include "case.h"


/*
 * Saving throws are now in class.c as of bpl13.
 */


/*
 * Negative apply_saving_throw[] values make saving throws better!
 * Then, so do negative modifiers.  Though people may be used to
 * the reverse of that. It's due to the code modifying the target
 * saving throw instead of the random number of the character as
 * in some other systems.
 */

int get_saves3(struct char_data *ch, int type)
{
    int save;

    save = saving_throws_3(ch, type);
    save += GET_SAVE3(ch, type);

    return (save);
}

/* ext_apply сколько % шанса, возвращает 1 если защита сейв прошел */
int general_savingthrow_3(struct char_data *ch, int type, int ext_apply)
{
    int save;

// базовое значние сейва

    save = saving_throws_3(ch, type);
    save = number(1, save) + GET_SAVE3(ch, type);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "Спасбросок [%s] ваш:%d против %d = %s\r\n", saves_name[type], save,
                      ext_apply, (save >= ext_apply ? "сохранился" : "нет"));

    if (save >= ext_apply)
        return (TRUE);

    return (FALSE);
}


int general_savingthrow(struct char_data *ch, int type, int ext_apply)
{
    log("Вызвали сейв параметры %s %d %d", GET_NAME(ch), type, ext_apply);
    return (FALSE);
}




/* affect_update: called from comm.c (causes spells to wear off) */

void show_spell_off(int aff, char_data * ch, obj_data * obj)
{
    if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_WRITING) && GET_POS(ch) <= POS_SLEEPING)
        return;

    if (Spl.GetItem(aff)->GetItem(SPL_AFF_MESS)->GetNumberItem()) {
        CItem *mess = Spl.GetItem(aff)->GetItem(SPL_AFF_MESS)->GetItem(0);

        if (mess) {
            if (mess->GetItem(SPL_AMESS_ECH)->GetString())
                act(mess->GetItem(SPL_AMESS_ECH)->GetString(), "!Мп", ch, obj);
            if (mess->GetItem(SPL_AMESS_ERM)->GetString())
                act(mess->GetItem(SPL_AMESS_ERM)->GetString(), "Кмп", ch, obj);
        }
    }
}

void show_spell_on(int aff, char_data * ch)
{
    if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_WRITING))
        return;

    if (Spl.GetItem(aff))
        if (Spl.GetItem(aff)->GetItem(SPL_AFF_MESS)->GetNumberItem()) {
            CItem *mess = Spl.GetItem(aff)->GetItem(SPL_AFF_MESS)->GetItem(0);

            if (mess) {
                if (mess->GetItem(SPL_AMESS_SCH)->GetString())
                    act(mess->GetItem(SPL_AMESS_SCH)->GetString(), "!М", ch);
                if (mess->GetItem(SPL_AMESS_SRM)->GetString())
                    act(mess->GetItem(SPL_AMESS_SRM)->GetString(), "Км", ch);
            } else
                send_to_charf(ch, "Не найдено сообщение\r\n");
        }

}

void bt_affect_update(void)
{
    struct char_data *i, *i_next;

    for (i = character_list; i; i = i_next) {
        i_next = i->next;
        battle_affect_update(i);
    }

}

void player_affect_update(void)
{
    struct affected_type *af, *next;
    struct char_data *i, *i_next;
    struct timed_type *timed, *timed_next;
    int spell_charm = find_spell_num(SPELL_CHARM);
    int spell_summon = find_spell_num(SPELL_SUMMON_ANGEL);
    int spell_summon2 = find_spell_num(SPELL_SUMMON_MONSTER_2);

    //int spell_hold = find_spell_num(SPELL_HOLD);
    int was_charmed = FALSE, was_helper = FALSE, charmed_msg = FALSE;

    for (i = character_list; i; i = i_next) {
        i_next = i->next;
        charmed_msg = FALSE;
        was_charmed = FALSE;
        was_helper = FALSE;

        for (af = i->affected; af; af = next) {
            next = af->next;
            if (af->battleflag)
                continue;
            //if (af->type == spell_hold) continue;
            if (af->duration >= 1) {
                af->duration--;
                if ((af->type == spell_charm ||
                     af->type == spell_summon || af->type == spell_summon2) &&
                    !charmed_msg && af->duration == SECS_PER_MUD_TICK && IS_NPC(i)) {
                    act("$n начал$g растерянно оглядываться по сторонам.", FALSE, i, 0, 0, TO_ROOM);
                    charmed_msg = TRUE;
                }
            } else if (af->duration == -1)
                af->duration = -1;
            else {
                if ((af->type > 0) && (af->type <= MAX_SPELLS)) {
                    if (!af->next || (af->next->type != af->type) || (af->next->duration > 0)) {
                        if (af->type > 0 && af->type <= LAST_USED_SPELL) {
                            if (af->main)
                                show_spell_off(af->type, i, NULL);
                            if (af->type == spell_charm || af->bitvector == AFF_CHARM)
                                was_charmed = TRUE;
                            if (af->type == spell_summon)
                                was_helper = TRUE;
                            if (af->type == spell_summon2)
                                was_helper = TRUE;
                        }
                    }
                }
                affect_remove(i, af);
            }
        }

        for (timed = i->timed; timed; timed = timed_next) {
            timed_next = timed->next;
            if (timed->time > 1)
                timed->time--;
            else if ((timed->time = 1)) {
                timed->time--;
                timed_from_char(i, timed);
            }
        }

        //log("[PLAYER_AFFECT_UPDATE->AFFECT_TOTAL] Start");
        //affect_total(i);
        //log("[PLAYER_AFFECT_UPDATE->AFFECT_TOTAL] Stop");
        if (check_death_trap(i))
            continue;
        if (was_charmed && IS_NPC(i))
            stop_follower(i, SF_CHARMLOST | SF_MASTERDIE);

        if (was_helper && IS_NPC(i))
            stop_follower(i, SF_CHARMLOST);
    }
}


/* This file update battle affects only */
void battle_affect_update(struct char_data *ch)
{
    struct affected_type *af, *next;


    //if (GET_MOB_VNUM(ch) ==105) log("%s: АФФЕКТ АПДЕЙТ",GET_NAME(ch));

    //if (affected_by_spell(ch,SPELL_BROTH_WEAPON))
    // if (GET_MOB_VNUM(ch) ==105)  log("%s: НАЙДЕНО БРАТСТВО %d",GET_NAME(ch),ch->damage_rnd);
    //if (affected_by_spell(ch,SPELL_BROTH_WEAPON) && ch->damage_rnd)
    //{
    //if (GET_MOB_VNUM(ch) ==105)  log("%s: НАЙДЕНО БРАТСТВО",GET_NAME(ch));
    // broth_weapon(ch, ch->damage_rnd);
//  }
    for (af = ch->affected; af; af = next) {
        next = af->next;
        if (!af->battleflag)
            continue;
        if (af->duration >= 1) {
            af->duration--;
        } else if (af->duration == -1)  /* No action */
            af->duration = -1;  /* GODs only! unlimited */
        else {
            if ((af->type > 0) && (af->type <= MAX_SPELLS)) {
                if (!af->next || (af->next->type != af->type) || (af->next->duration > 0)) {
                    if (af->type > 0 && af->type <= LAST_USED_SPELL) {
                        show_spell_off(af->type, ch, NULL);
                    }
                }
            }
            affect_remove(ch, af);
        }
    }
    //log("[BATTLE_AFFECT_UPDATE->AFFECT_TOTAL] Start");
    //affect_total(ch);
    //log("[BATTLE_AFFECT_UPDATE->AFFECT_TOTAL] Stop");
}



/*
 *  mag_materials:
 *  Checks for up to 3 vnums (spell reagents) in the player's inventory.
 *
 * No spells implemented in Circle 3.0 use mag_materials, but you can use
 * it to implement your own spells which require ingredients (i.e., some
 * heal spell which requires a rare herb or some such.)
 */

int mag_item_ok(struct char_data *ch, struct obj_data *obj, int spelltype)
{
    int num = 0;

    if ((!IS_SET(GET_OBJ_SKILL(obj), ITEM_RUNES) && spelltype == SPELL_RUNES) ||
        (IS_SET(GET_OBJ_SKILL(obj), ITEM_RUNES) && spelltype != SPELL_RUNES))
        return (FALSE);

    if (IS_SET(GET_OBJ_SKILL(obj), ITEM_CHECK_USES) && GET_OBJ_VAL(obj, 2) <= 0)
        return (FALSE);

    if (IS_SET(GET_OBJ_SKILL(obj), ITEM_CHECK_LAG)) {
        num = 0;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LAG1s))
            num += 1;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LAG2s))
            num += 2;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LAG4s))
            num += 4;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LAG8s))
            num += 8;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LAG16s))
            num += 16;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LAG32s))
            num += 32;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LAG64s))
            num += 64;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LAG128s))
            num += 128;
        if (GET_OBJ_VAL(obj, 3) + num >= time(NULL))
            return (FALSE);
    }

    if (IS_SET(GET_OBJ_SKILL(obj), ITEM_CHECK_LEVEL)) {
        num = 0;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LEVEL1))
            num += 1;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LEVEL2))
            num += 2;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LEVEL4))
            num += 4;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LEVEL8))
            num += 8;
        if (IS_SET(GET_OBJ_VAL(obj, 0), MI_LEVEL16))
            num += 16;
        if (GET_LEVEL(ch) < num)
            return (FALSE);
    }

    return (TRUE);
}


void extract_item(struct char_data *ch, struct obj_data *obj, int spelltype)
{
    int extract = FALSE;

    if (!obj)
        return;

    GET_OBJ_VAL(obj, 3) = time(NULL);

    if (IS_SET(GET_OBJ_SKILL(obj), ITEM_CHECK_USES)) {
        GET_OBJ_VAL(obj, 2)--;
        if (GET_OBJ_VAL(obj, 2) <= 0 && IS_SET(GET_OBJ_SKILL(obj), ITEM_DECAY_EMPTY))
            extract = TRUE;
    } else if (spelltype != SPELL_RUNES)
        extract = TRUE;

    if (extract) {
        if (spelltype == SPELL_RUNES)
            act("$o рассыпал$U у Вас в руках.", FALSE, ch, obj, 0, TO_CHAR);
        obj_from_char(obj);
        extract_obj(obj);
    }
}






void do_sacrifice(struct char_data *ch, int dam)
{
    GET_HIT(ch) = MIN(GET_HIT(ch) + MAX(1, dam),
                      GET_REAL_MAX_HIT(ch) + GET_REAL_MAX_HIT(ch) * GET_LEVEL(ch) / 10);
    update_pos(ch);
}

/*
 * Every spell that does damage comes through here.  This calculates the
 * amount of damage, adds in any modifiers, determines what the saves are,
 * tests for save and calls damage().
 *
 * -1 = dead, otherwise the amount of damage done.
 */

int pc_duration(struct char_data *ch, int cnst, int level, int level_divisor, int min, int max)
{
    int result = 0;

    /* if (IS_NPC(ch))
       {result = cnst;
       if (level > 0 && level_divisor > 0)
       level  = level / level_divisor;
       else
       level  = 0;
       if (min > 0)
       level = MIN(level, min);
       if (max > 0)
       level = MAX(level, max);
       return (level + result);
       } */
    result = cnst * SECS_PER_MUD_TICK;
    if (level > 0 && level_divisor > 0)
        level = level * SECS_PER_MUD_TICK / level_divisor;
    else
        level = 0;
    if (min > 0)
        level = MIN(level, min * SECS_PER_MUD_TICK);
    if (max > 0)
        level = MAX(level, max * SECS_PER_MUD_TICK);
    result = (level + result) / SECS_PER_PLAYER_AFFECT;
    return (result);
}


struct char_list_data {
    struct char_data *ch;
    struct char_list_data *next;
};




int train_magic(struct char_data *ch, int spell_no, struct char_data *vict)
{
    int ii, ni, skill_no, prob = 0, spell_agro = 0;
    char buf[MAX_STRING_LENGTH];
    struct char_data *victim = vict;

    if (IS_NPC(ch))
        return 0;

    /* Получаем школу заклинания */
    skill_no = SPELL_SPHERE(spell_no) + TYPE_SPHERE_SKILL;

    if (GET_SKILL(ch, skill_no) <= 0)
        return (0);


    asciiflag_conv((char *) SPELL_AGRO(spell_no), &spell_agro);
    if (!check_spell_agro(spell_no, vict, spell_agro)) {
        if (GET_LEVEL(victim) > GET_LEVEL(ch))
            victim = ch;
    }

    prob = GET_SKILL(ch, skill_no);
    prob = calc_like_skill(ch, victim, skill_no, prob);

    if (GET_SKILL(ch, skill_no) > SPELL_LSKILL(spell_no)) {
        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&rВы достигли предела тренировки этим спелом [%d]>[%d]&n\r\n",
                          GET_SKILL(ch, skill_no), SPELL_LSKILL(spell_no));
        return prob;
    }

    ni = calc_improove_skill(ch, skill_no, victim, 0);
    ii = dice(1, ni);


    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&rТренировка '%s' шанс 1 к %d выпало %d должно %d&n\r\n",
                      skill_info[skill_no].name.c_str(), ni, ii, ni / 2);

    if (ii == ni / 2
        && GET_SKILL(ch, skill_no) < calc_need_improove(ch, get_skill_class_level(ch, skill_no))
        && !GET_IMPROOVE(ch, skill_no)) {
        GET_IMPROOVE(ch, skill_no) = TRUE;

        if (fmod(GET_SKILL(ch, skill_no), 10)) {
            sprintf(buf, "%sВы почувствовали себя увереннее в %s.%s\r\n",
                    CCICYN(ch, C_NRM),
                    get_name_pad((char *) sphere_names[(int) SPELL_SPHERE(spell_no)], PAD_PRD,
                                 PAD_OBJECT), CCNRM(ch, C_NRM));

            send_to_char(buf, ch);
            SET_SKILL(ch, skill_no)++;

            if (!fmod(GET_SKILL(ch, skill_no), 10)) {
                sprintf(buf, "%sВам следует посетить гильдию для дальнейшего изучения %s.%s\r\n",
                        CCIGRN(ch, C_NRM),
                        get_name_pad((char *) sphere_names[(int) SPELL_SPHERE(spell_no)], PAD_ROD,
                                     PAD_OBJECT), CCNRM(ch, C_NRM));
                send_to_char(buf, ch);
            }
        }
    }

    return (prob);
}

int get_spell_value(struct char_data *ch, int spell_no)
{
    struct affected_type *aff;
    int result = 0;

    if (ch->affected)
        for (aff = ch->affected; aff; aff = aff->next)
            if (aff->type == find_spell_num(spell_no))
                result = aff->modifier;

    return (result);
}

long get_spell_owner(struct char_data *ch, int spell_no)
{
    struct affected_type *aff;

    spell_no = find_spell_num(spell_no);
    if (ch->affected)
        for (aff = ch->affected; aff; aff = aff->next)
            if (aff->type == spell_no)
                return (aff->owner);

    return (FALSE);
}

long get_spell_owner_real(struct char_data *ch, int spellnum)
{
    struct affected_type *aff;

    if (ch->affected)
        for (aff = ch->affected; aff; aff = aff->next)
            if (aff->type == spellnum)
                return (aff->owner);

    return (FALSE);
}

int mag_damage(int spellnum, int dam, struct char_data *ch, struct char_data *victim, int show_mess,
               int type_damage, int set)
{
    struct P_damage damage;
    struct P_message message;

    damage.valid = true;
    damage.type = type_damage;
    damage.power = GET_POWER(ch);
    damage.far_min = TRUE;
    damage.armor = FALSE;
    damage.magic = TRUE;
    damage.check_ac = A_POWER;
    damage.weapObj = NULL;
    damage.dam = dam;
    damage.hLoc = WEAR_BODY;


    switch (type_damage) {
        case HIT_FIRE:
            damage.dam += GET_INC_MAGIC(ch, 2);
            break;
        case HIT_COLD:
            damage.dam += GET_INC_MAGIC(ch, 3);
            break;
        case HIT_ELECTRO:
            damage.dam += GET_INC_MAGIC(ch, 4);
            break;
        case HIT_ACID:
        case HIT_POISON:
            damage.dam += GET_INC_MAGIC(ch, 5);
            break;
        case HIT_POSITIVE:
            damage.dam += GET_INC_MAGIC(ch, 7);
            break;
        case HIT_NEGATIVE:
            damage.dam += GET_INC_MAGIC(ch, 6);
            break;
    }

    damage.dam += GET_INC_MAGIC(ch, 1);
    if (show_mess)
        GetSpellMessage(spellnum, message);

    return (_damage(ch, victim, 0, 0, A_POWER, set, damage, message));

}

//соимость эффекта
int cost_miracle[] = { 5, 100, 30, 75 };

bool miracle_action(struct char_data *ch, int type)
{
    int spellnum = find_spell_num(SPELL_MIRACLE), need;
    struct affected_type *hjp, *next;
    bool result = FALSE;
    room_rnum fnd_room = NOWHERE;
    int _recall(struct char_data *ch);

    if (type >= NUM_MIRC)
        return (result);

    if ((need = affected_by_spell(ch, SPELL_MIRACLE)) < 10)
        return (result);

    switch (type) {
        case MIRC_MOVE:
            act("Вы почувствовали, что невидимая сила восстановила Вашу бодрость.", FALSE, ch, 0, 0,
                TO_CHAR);
            act("Тело $n1 на миг вспыхнуло нежно-голубым светом.", FALSE, ch, 0, 0, TO_ROOM);
            GET_MOVE(ch) = GET_REAL_MAX_MOVE(ch);
            result = TRUE;
            break;
        case MIRC_DIE:
            int nn = number(1, 20);

            if (nn <= 10) {     //переносим жертву куда нить
                if ((fnd_room = _recall(ch)) != NOWHERE) {
                    act("Вы чуть не умерли, но произошло божественное чудо!", FALSE, ch, 0, 0,
                        TO_CHAR);
                    act("1и почти умер1(,ла,ло,ли), но произошло божественное чудо!\r\nНеведомая сила переместила 1ев за миг до смерти!", "Км", ch);
                    if (GET_HIT(ch) <= 10)
                        GET_HIT(ch) = 10;
                    char_from_room(ch);
                    char_to_room(ch, fnd_room);
                    check_horse(ch);
                    look_at_room(ch, 0);
                    act("\r\nНеобыкновенная сила переместила Вас!", FALSE, ch, 0, 0, TO_CHAR);
                    act("$n материализовал$u перед Вами.", FALSE, ch, 0, 0, TO_ROOM);
                    stop_fighting(ch, TRUE);
                    result = TRUE;
                    type = MIRC_FIGHT;
                }
            } else if (nn > 10 && nn <= 19) {
                act("Вы чуть не умерли, но что-то наполнило Вас жизненной силой!", FALSE, ch, 0, 0,
                    TO_CHAR);
                act("$n почти умер$q, но тело $s наполнилось жизненной силой!", FALSE, ch, 0, 0,
                    TO_ROOM);
                GET_HIT(ch) = GET_REAL_MAX_HIT(ch);
                result = TRUE;
                type = MIRC_HEAL;
            } else if (nn == 20) {
                struct char_data *tch = NULL, *next_tch = NULL, *victim = NULL;

                victim = FIGHTING(ch);
                if (victim && IS_NPC(victim) && GET_LEVEL(victim) <= GET_LEVEL(ch)) {
                    act("Вы чуть не умерли, как необыкновенная сила уничтожила Ваших врагов!",
                        FALSE, ch, 0, 0, TO_CHAR);
                    if (GET_HIT(ch) <= 0)
                        GET_HIT(ch) = 1;
                    for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
                        next_tch = tch->next_in_room;
                        if (tch == ch)
                            continue;
                        if (!IS_NPC(victim) || GET_LEVEL(victim) > GET_LEVEL(ch))
                            continue;
                        if (tch == victim || same_group(tch, victim)) {
                            if (25 >= GET_REAL_LCK(tch)) {
                                act("$n чуть не умер$q, как необыкновенная сила уничтожила Вас!",
                                    FALSE, ch, 0, tch, TO_VICT);
                                death_cry(tch, NULL);
                                die(tch, NULL);
                            } else
                                act("$n чуть не умер$q, как необыкновенная сила хотела уничтожить Вас!", FALSE, ch, 0, tch, TO_VICT);
                        } else
                            act("$n чуть ен умер$q, как необыкновенная сила уничтожила $s врагов!",
                                FALSE, ch, 0, tch, TO_VICT);
                    }
                    type = MIRC_DIE;
                    result = TRUE;
                }
            }
            break;

    }

    if (result)
        for (hjp = ch->affected; hjp; hjp = next) {
            next = hjp->next;
            if (hjp->type == spellnum) {
                hjp->modifier -= cost_miracle[type];
                if (hjp->modifier < 20) {
                    if (hjp->main)
                        show_spell_off(hjp->type, ch, NULL);
                    affect_remove(ch, hjp);
                }
            }
        }

    return (result);
}


void broth_weapon(struct char_data *ch, int damage)
{
    long own_id;
    int spell_no = SPELL_BROTH_WEAPON;
    int spellnum = find_spell_num(SPELL_BROTH_WEAPON);
    struct char_data *owner = NULL, *tch, *healer = NULL;
    int hp = 0;

    own_id = get_spell_owner(ch, spell_no);
    owner = get_char_by_id(own_id);
    if (owner)                  //есть через кого нить пускать
        for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
            if (same_group(owner, tch) && same_group(ch, tch) && affected_by_spell_real(tch, spellnum)) {       //в одной группе и есть жрец
                if (own_id != get_spell_owner(tch, spell_no))
                    continue;
                //ищем самого толстого и снего снимает жизнь.
                if (hp < GET_HIT(tch)) {
                    hp = GET_HIT(tch);
                    healer = tch;
                }
            }
        }

    if (healer && healer != ch) {
        act("Вы передали часть своей жизненной энергии $N2.", FALSE, healer, 0, ch, TO_CHAR);
        act("$n передал часть своей жизненной энергии Вам.", FALSE, healer, 0, ch, TO_VICT);
        GET_HIT(healer) -= damage;
        GET_HIT(ch) = MIN(GET_REAL_MAX_HIT(ch), GET_HIT(ch) + damage);
        update_pos(healer);
    }

}

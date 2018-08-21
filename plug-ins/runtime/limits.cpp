/* ************************************************************************
*   File: limits.c                                      Part of CircleMUD *
*  Usage: limits & gain funcs for HMV, exp, hunger/thirst, idle time      *
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
#include "boards.h"
#include "utils.h"
#include "spells.h"
#include "comm.h"
#include "db.h"
#include "mobmax.h"
#include "handler.h"
#include "screen.h"
#include "interpreter.h"
#include "constants.h"
#include "xspells.h"
#include "xboot.h"

/* local functions */
void check_plague(struct char_data *ch);
void check_plague(struct obj_data *obj);
void check_obj_damage(struct char_data *ch);

/* When age < 20 return the value p0 */
/* When age in 20..29 calculate the line between p1 & p2 */
/* When age in 30..34 calculate the line between p2 & p3 */
/* When age in 35..49 calculate the line between p3 & p4 */
/* When age in 50..74 calculate the line between p4 & p5 */
/* When age >= 75 return the value p6 */
int graf(int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6)
{

    if (age < 20)
        return (p0);            /* < 20   */
    else if (age <= 29)
        return (int) (p1 + (((age - 20) * (p2 - p1)) / 10));    /* 20..29 */
    else if (age <= 34)
        return (int) (p2 + (((age - 30) * (p3 - p2)) / 5));     /* 30..34 */
    else if (age <= 49)
        return (int) (p3 + (((age - 35) * (p4 - p3)) / 15));    /* 35..59 */
    else if (age <= 74)
        return (int) (p4 + (((age - 50) * (p5 - p4)) / 25));    /* 50..74 */
    else
        return (p6);            /* >= 80 */
}


/*
 * The hit_limit, mana_limit, and move_limit functions are gone.  They
 * added an unnecessary level of complexity to the internal structure,
 * weren't particularly useful, and led to some annoying bugs.  From the
 * players' point of view, the only difference the removal of these
 * functions will make is that a character's age will now only affect
 * the HMV gain per tick, and _not_ the HMV maximums.
 */

/* Прибавление маны в игровой час */
int mana_gain(struct char_data *ch)
{
    int gain, percent = GET_REAL_HLT(ch);       //коэф поправки;

    if (IS_NPC(ch)) {
        gain = GET_LEVEL(ch) * 2;
        percent = 100;
    } else {
        if (!ch->desc || STATE(ch->desc) != CON_PLAYING)
            return (0);
        if (GET_LEVEL(ch) <= 3)
            gain = GET_REAL_MAX_MANA(ch) / 3;
        else if (GET_LEVEL(ch) <= 10)
            gain = GET_REAL_MAX_MANA(ch) / 5;
        else
            gain = GET_REAL_MAX_MANA(ch) / 8;
    }

    if (GET_INT(ch) < 14)
        percent += 0;
    else if (GET_INT(ch) < 18)
        percent += 2;
    else if (GET_INT(ch) < 22)
        percent += 5;
    else if (GET_INT(ch) < 26)
        percent += 8;
    else
        percent += 10;


    if (LIKE_ROOM(ch))
        percent += 50;          //50% за любимую комнату

    if (world[IN_ROOM(ch)].fires)       //10% за каждый костер в комнате
        percent += MAX(1, 2 + world[IN_ROOM(ch)].fires * 10);

    if (!IS_NPC(ch)) {
        if (GET_COND(ch, FULL) == 0)
            percent -= 15;
        if (GET_COND(ch, THIRST) == 0)
            percent -= 15;
        if (GET_COND(ch, SLEEP) == 0)
            percent -= 5;
        if (GET_COND(ch, DRUNK) >= CHAR_DRUNKED)
            percent -= 75;
    }

    if (AFF_FLAGGED(ch, AFF_HOLD))
        percent -= 50;

    percent += GET_HITREG(ch);

    if ((AFF_FLAGGED(ch, AFF_POISON) || AFF_FLAGGED(ch, AFF_CURSE)
         || AFF_FLAGGED(ch, AFF_HAEMORRAGIA)) && percent > 0 && !IS_UNDEAD(ch)
        && !IS_CONSTRUCTION(ch))
        percent /= 30;

// Поправка на положение
    if (IN_ROOM(ch) != NOWHERE && IS_UNDEAD(ch) &&
        affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_THRDEATH))
        percent += 50;
    else if (AFF_FLAGGED(ch, AFF_SLEEP))
        percent -= 25;
    else if (FIGHTING(ch))
        percent = -50;
    else
        switch (GET_POS(ch)) {
            case POS_SLEEPING:
                percent += 200;
                break;
            case POS_RESTING:
                percent += 100;
                break;
            case POS_SITTING:
                percent += 40;
                break;
            case POS_FIGHTING:
                percent = -50;
                break;
        }

    if (percent > 0) {
        gain = gain * percent / 100;

        if (affected_by_spell(ch, SPELL_PLAGUE))
            gain = 0;

        int add;

        if ((add = affected_by_spell(ch, SPELL_MEDITATION)))
            gain += 2 * add;
    } else
        gain = 0;


    if (IS_SOUL(ch))
        gain = 0;

    gain = MAX(0, gain);

    return (gain);
}


/* Прибавление жизни в игровой час */
int hit_gain(struct char_data *ch)
{
    int gain, add, percent = GET_REAL_HLT(ch);

    if (IS_NPC(ch) && NPC_FLAGGED(ch, NPC_NOREGAIN))
        return (0);

    /* if (IS_MOB(ch))
       {
       if (GET_LEVEL(ch) <= 3)
       gain = GET_REAL_MAX_HIT(ch)/3;
       else if (GET_LEVEL(ch) <= 10)
       gain = GET_REAL_MAX_HIT(ch)/5;
       else
       gain = GET_REAL_MAX_HIT(ch)/8;
       percent = 100;
       }
       else
       {

       } */

    if (!IS_NPC(ch) && (!ch->desc || STATE(ch->desc) != CON_PLAYING))
        return (0);

    if (GET_LEVEL(ch) <= 3)
        gain = GET_REAL_MAX_HIT(ch) / 3;
    else if (GET_LEVEL(ch) <= 10)
        gain = GET_REAL_MAX_HIT(ch) / 5;
    else
        gain = GET_REAL_MAX_HIT(ch) / 8;

    if (GET_CON(ch) < 14)
        percent += 0;
    else if (GET_CON(ch) < 18)
        percent += 5;
    else if (GET_CON(ch) < 22)
        percent += 10;
    else if (GET_CON(ch) < 26)
        percent += 15;
    else
        percent += 20;

    if (LIKE_ROOM(ch))
        percent += 20;

    if (world[IN_ROOM(ch)].fires)       //5% за каждый костер в комнате
        percent += MAX(1, 5 + world[IN_ROOM(ch)].fires * 10);


    if (!IS_NPC(ch)) {
        if (GET_COND(ch, FULL) == 0)
            percent -= 15;
        if (GET_COND(ch, THIRST) == 0)
            percent -= 15;
        if (GET_COND(ch, SLEEP) == 0)
            percent -= 10;
    }

    percent += GET_HITREG(ch);

    if ((AFF_FLAGGED(ch, AFF_POISON) || AFF_FLAGGED(ch, AFF_HAEMORRAGIA)) && percent > 0 &&
        !IS_UNDEAD(ch) && !IS_CONSTRUCTION(ch))
        percent /= 20;

    if (IN_ROOM(ch) != NOWHERE && IS_UNDEAD(ch) &&
        affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_THRDEATH))
        percent += 50;
    else if (IS_UNDEAD(ch) && AFF_FLAGGED(ch, AFF_CHARM))
        percent = 0;
    else if (AFF_FLAGGED(ch, AFF_SLEEP))
        percent -= 75;
    else if (FIGHTING(ch))
        percent = 0;
    else
        switch (GET_POS(ch)) {
            case POS_SLEEPING:
                percent += 200;
                break;
            case POS_RESTING:
                percent += 100;
                break;
            case POS_SITTING:
                percent += 50;
                break;
            case POS_STUNNED:
            case POS_INCAP:
            default:
                percent += 0;
                break;
            case POS_FIGHTING:
                percent = 0;
                break;
        }

    percent = MAX(0, MIN(500, percent));

    if (percent > 0) {
        gain = gain * percent / 100;
        if ((add = affected_by_spell(ch, SPELL_BANDAGE)))
            gain += 2 * add;
    } else
        gain = 0;

    if (GET_POS(ch) == POS_MORTALLYW)
        gain = -1;

    if (affected_by_spell(ch, SPELL_PLAGUE))
        gain = 0;

    if (AFF_FLAGGED(ch, AFF_HAEMORRAGIA) && !IS_UNDEAD(ch) && !IS_CONSTRUCTION(ch))
        gain = 0;

    if (IS_SOUL(ch))
        gain = 0;

    return gain;
}


/* Прибавление бодрости в игровой час */
int move_gain(struct char_data *ch)
{
    int gain, percent = GET_REAL_HLT(ch);;      //коэф поправки

    if (IS_MOB(ch)) {
        gain = MAX(30, GET_LEVEL(ch) * 2);
        percent = 100;
    } else {
        if (!ch->desc || STATE(ch->desc) != CON_PLAYING)
            return (0);
        gain = GET_REAL_MAX_MOVE(ch) / 6;       //базовый хитгейн
    }

    if (GET_CON(ch) < 14)
        percent += 0;
    else if (GET_CON(ch) < 18)
        percent += 5;
    else if (GET_CON(ch) < 22)
        percent += 10;
    else if (GET_CON(ch) < 26)
        percent += 15;
    else
        percent += 20;

    if (LIKE_ROOM(ch))
        percent += 25;          //50% за любимую комнату

    if (world[IN_ROOM(ch)].fires)       //10% за каждый костер в комнате
        percent += MAX(1, 5 + world[IN_ROOM(ch)].fires * 10);

// Поправка на положение
    if (AFF_FLAGGED(ch, AFF_SLEEP))
        percent -= 75;
    else
        switch (GET_POS(ch)) {
            case POS_SLEEPING:
                percent += 200;
                break;
            case POS_RESTING:
                percent += 100;
                break;
            case POS_SITTING:
                percent += 20;
                break;
            case POS_FIGHTING:
                percent = 0;
                break;
        }

    if (!IS_NPC(ch)) {
        if (GET_COND(ch, FULL) == 0)
            percent -= 15;
        if (GET_COND(ch, THIRST) == 0)
            percent -= 15;
        if (GET_COND(ch, SLEEP) == 0)
            percent -= 10;
        if (affected_by_spell(ch, SPELL_HIDE))
            percent -= 50;
        if (affected_by_spell(ch, SPELL_CAMOUFLAGE))
            percent -= 50;
    }

    percent += GET_MOVEREG(ch);

//уменьшаем в 4 раза бонус
    if ((AFF_FLAGGED(ch, AFF_POISON) || AFF_FLAGGED(ch, AFF_HAEMORRAGIA)) && percent > 0 &&
        !IS_UNDEAD(ch) && !IS_CONSTRUCTION(ch))
        percent /= 4;

// Расчет gain от percent
    percent = MAX(0, MIN(300, percent));
    gain = gain * percent / 100;

    if (IN_ROOM(ch) != NOWHERE && IS_UNDEAD(ch))
        if (affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_THRDEATH))
            gain *= 3;

//в бою не отдыхаем
    if (gain > 0 && (FIGHTING(ch) || GET_POS(ch) == POS_FIGHTING))
        gain = 0;

    if (affected_by_spell(ch, SPELL_PLAGUE))
        gain = 0;

    if (IS_SOUL(ch))
        gain = 0;

    return (gain);
}

#define MINUTE            60
#define UPDATE_PC_ON_BEAT TRUE

int interpolate(int min_value, int pulse)
{
    int sign = 1, int_p, frac_p, i, carry, x, y;

    if (min_value < 0) {
        sign = -1;
        min_value = -min_value;
    }
    int_p = min_value / MINUTE;
    frac_p = min_value % MINUTE;
    if (!frac_p)
        return (sign * int_p);
    pulse = time(NULL) % MINUTE + 1;
    x = MINUTE;
    y = 0;
    for (i = 0, carry = 0; i <= pulse; i++) {
        y += frac_p;
        if (y >= x) {
            x += MINUTE;
            carry = 1;
        } else
            carry = 0;
    }
    return (sign * (int_p + carry));
}


void gain_sleep(struct char_data *ch)
{
    int percent = 1;


    if (LIKE_ROOM(ch))
        percent += 2;

    if (AFF_FLAGGED(ch, AFF_SLEEP))
        percent += 3;


    if (GET_POS(ch) != POS_SLEEPING)
        gain_condition(ch, SLEEP, -1);
    else if (GET_COND(ch, SLEEP) < 48 * SECS_PER_MUD_TICK)
        gain_condition(ch, SLEEP, 18 * percent);

}

void set_title(struct char_data *ch, const char *title)
{
    if (GET_TITLE(ch) != NULL) {
        free(GET_TITLE(ch));
        GET_TITLE(ch) = NULL;
    }

    if (!title)
        return;
    char sub_buf[MAX_TITLE_LENGTH + 1];

    strncpy(sub_buf, title, MAX_TITLE_LENGTH);
    sub_buf[MAX_TITLE_LENGTH] = '\0';

    if (*title) {
        if (GET_TITLE(ch))
            free(GET_TITLE(ch));

        GET_TITLE(ch) = str_dup(sub_buf);
        send_to_char("Боги одобрили Ваш титул.\r\n", ch);
    }
}

void set_rtitle(struct char_data *ch, char *title)
{

    if (!title)
        return;

    if (title && strlen(title) > MAX_TITLE_LENGTH)
        title[MAX_TITLE_LENGTH] = '\0';

    if (GET_RTITLE(ch) != NULL) {
        free(GET_RTITLE(ch));
        GET_RTITLE(ch) = NULL;
    }

    if (title && *title) {
        if (GET_RTITLE(ch))
            free(GET_RTITLE(ch));
        GET_RTITLE(ch) = str_dup(title);
        send_to_charf(ch, "Вы отправили запрос на титул \"%s\".\r\n", title);
    }
}


#define MAX_REMORT 3

void gain_exp(struct char_data *ch, int gain, bool show_mess)
{
    int num_levels = 0, next_this_level = 0;
    char buf[MAX_STRING_LENGTH];

    if (!IS_NPC(ch) && ((GET_LEVEL(ch) < 0 || GET_LEVEL(ch) >= LVL_IMMORT)))
        return;

    if (gain < 0) {
        //gain = MIN(max_exp_loss_pc(ch), gain);
        if (show_mess)
            send_to_charf(ch, "Вы потеряли %d %s опыта.\r\n", (gain * -1),
                          desc_count((gain * -1), WHAT_POINT));
    } else if (gain > 0) {
        gain = MIN(max_exp_gain_pc(ch), gain);
        if ((unsigned int) GET_EXP(ch) >
            ((unsigned int) get_levelexp(ch, GET_LEVEL(ch) + 1, 1) * 10) / 9) {
            if (show_mess)
                send_to_charf(ch, "На этом уровне Вы прекратили получать опыт.\r\n");
            return;
        }

        if (show_mess)
		{
            send_to_charf(ch, "Ваш опыт повысился на %d %s.\r\n", gain, desc_count(gain, WHAT_POINT));
			if (get_dsu_exp(ch) < 1 && !ch->pc()->specials.msg_zero_dsu)
			{
				send_to_charf(ch, "&GПоздравляем! Теперь у Вас достаточно опыта, чтобы повысить свой уровень.&n\r\n");
				ch->pc()->specials.msg_zero_dsu = false;
			}
		}
    } else {
        if (show_mess)
            send_to_charf(ch, "Ваш опыт не изменился.\r\n");
    }

    if (IS_NPC(ch)) {
        if (GET_LEVEL(ch) >= 60 || !IS_AFFECTED(ch, AFF_CHARM))
            return;

        /*if (ch->master)
           send_to_charf(ch->master,"%s мой опыт повысился на %d очков.\r\n",GET_NAME(ch),gain); */

        GET_EXP(ch) += gain;

        if (GET_EXP(ch) >= level_exp(ch, GET_LEVEL(ch) + 1)) {
            int next_level = check_class(ch, GET_CLASS(ch));

            GET_LEVEL(ch) += 1;
            send_to_charf(ch, "%sВы достигли следующего уровня !%s\r\n", CCWHT(ch, C_NRM),
                          CCNRM(ch, C_NRM));

            if (ch->master)
                send_to_charf(ch->master, "Способности %s улучшились.\r\n", GET_PAD(ch, 1), gain);

            add_class(ch, GET_CLASS(ch), next_level + 1, 0);
            advance_level(ch);
            affect_total(ch);
        }
        return;
    }

    if (gain > 0) {
        gain = MIN(max_exp_gain_pc(ch), gain);

        GET_EXP(ch) += gain;


        //Востановление уровня
        /*   if (GET_LEVEL(ch) < LVL_IMMORT &&
           GET_EXP(ch) >= get_levelexp(ch, GET_REAL_LEVEL(ch)+1, 1) &&
           GET_HLEVEL(ch,GET_REAL_LEVEL(ch)+1 ) != CLASS_UNDEFINED) */
        if (GET_LEVEL(ch) < LVL_IMMORT && get_dsu_exp(ch) <= 0 &&
            (GET_HLEVEL(ch, GET_LEVEL(ch) + 1) != CLASS_UNDEFINED &&
             GET_HLEVEL(ch, GET_LEVEL(ch) + 1) != -1)) {
            //log ("Восс: get_exp %d exp_level %d",GET_EXP(ch),get_levelexp(ch, GET_LEVEL(ch), 1));

            if (!IS_MAX_EXP(ch))
                GET_LEVEL(ch)++;

            GET_CLASS(ch) = GET_HLEVEL(ch, (int) GET_LEVEL(ch));
            next_this_level = check_class(ch, GET_CLASS(ch));
            next_this_level++;
            add_class(ch, GET_CLASS(ch), next_this_level, 0);
            advance_level(ch);
            send_to_charf(ch, "%sВы воcстановили свой уровень!%s\r\n", CCCYN(ch, C_NRM),
                          CCNRM(ch, C_NRM));
            sprintf(buf, "%s восстановил уровень %d.", GET_NAME(ch), GET_LEVEL(ch));
            mudlog(buf, BRF, MAX(LVL_HIGOD, GET_INVIS_LEV(ch)), TRUE);
        }

        if (IS_MAX_EXP(ch) && !GET_GOD_FLAG(ch, GF_REMORT)) {
            act("&WВаше тело исчерпало свои возможности!&n", FALSE, ch, 0, 0, TO_CHAR);
            free_mkill(ch);
            SET_GOD_FLAG(ch, GF_REMORT);
        }
    }

    else if (gain < 0) {
        gain = MAX(-max_exp_loss_pc(ch), gain);
        GET_EXP(ch) += gain;
        if (GET_EXP(ch) < 0)
            GET_EXP(ch) = 0;

        //while (GET_LEVEL(ch) > 1 &&
        //       GET_EXP(ch) < get_levelexp(ch, GET_REAL_LEVEL(ch), 0))
        if (GET_LEVEL(ch) > 1 && GET_EXP(ch) < get_levelexp(ch, GET_LEVEL(ch), 0)) {
            log("%d: get_exp %ld exp_level %ld", num_levels, GET_EXP(ch),
                get_levelexp(ch, GET_LEVEL(ch), 0));

            num_levels++;
            decrease_level(ch);

            GET_LEVEL(ch)--;

            GET_CLASS(ch) = GET_HLEVEL(ch, (int) GET_LEVEL(ch));

            send_to_charf(ch, "%sВы потеряли уровнь!%s\r\n", CCIRED(ch, C_NRM), CCNRM(ch, C_NRM));
            sprintf(buf, "%s потерял уровень.", GET_NAME(ch));
            mudlog(buf, BRF, MAX(LVL_HIGOD, GET_INVIS_LEV(ch)), TRUE);
        }

        if (!IS_MAX_EXP(ch) && GET_GOD_FLAG(ch, GF_REMORT)) {
            act("&WВы вновь можете набирать опыт.&n", FALSE, ch, 0, 0, TO_CHAR);
            CLR_GOD_FLAG(ch, GF_REMORT);
        }

    }
}



void gain_condition(struct char_data *ch, int condition, int value)
{
    if (IS_NPC(ch) || GET_COND(ch, condition) == -1)    /* No change */
        return;

    if (FIGHTING(ch))
        return;                 /* во время боя деремся а не жрать просим */

//send_to_charf(ch,"Устанавливаю %d в + %d\r\n",condition,value);
    GET_COND(ch, condition) += value;

    switch (condition) {
        case DRUNK:
            GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
            GET_COND(ch, condition) =
                MIN(drunk_race[(int) GET_RACE(ch)].max_cond * 6 * SECS_PER_MUD_TICK,
                    GET_COND(ch, condition));
            break;
        case FULL:
            GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
            GET_COND(ch, condition) =
                MIN(full_race[(int) GET_RACE(ch)].max_cond * 6 * SECS_PER_MUD_TICK,
                    GET_COND(ch, condition));
            break;
        case THIRST:
            GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
            GET_COND(ch, condition) =
                MIN(thirst_race[(int) GET_RACE(ch)].max_cond * 6 * SECS_PER_MUD_TICK,
                    GET_COND(ch, condition));
            break;
        case SLEEP:
            GET_COND(ch, condition) = MAX(0, GET_COND(ch, condition));
            GET_COND(ch, condition) =
                MIN(sleep_race[(int) GET_RACE(ch)].max_cond * 6 * SECS_PER_MUD_TICK,
                    GET_COND(ch, condition));
            break;
        default:
            break;
    }

    if (GET_COND(ch, condition) == SECS_PER_MUD_TICK && GET_POS(ch) > POS_SLEEPING) {
        switch (condition) {
            case FULL:
                send_to_char("Вы испытываете легкий голод.\r\n", ch);
                act("У 1р заурчало в животе.", "Км", ch);
                return;
            case THIRST:
                send_to_char("У Вас пересохло в горле.\r\n", ch);
                act("1и облизнул1(,а,о,и) пересохшие губы.", "Км", ch);
                return;
            case SLEEP:
                send_to_char("Неожиданно для себя, Вы сладко зевнули.\r\n", ch);
                act("1и сонно зевнул1(,а,о,и).", "Км", ch);
                return;
            default:
                break;
        }
    }


    if (GET_COND(ch, condition) == 1 && !PLR_FLAGGED(ch, PLR_WRITING))
        switch (condition) {
            case FULL:
                send_to_char("Вы голодны.\r\n", ch);
                return;
            case THIRST:
                send_to_char("Вы хотите пить.\r\n", ch);
                return;
            case SLEEP:
                send_to_char("Вы хотите спать.\r\n", ch);
                return;
            case DRUNK:
                if (AFF_FLAGGED(ch, AFF_DRUNKED))
                    GET_COND(ch, DRUNK) =
                        MAX(drunk_race[(int) GET_RACE(ch)].max_cond / 2, GET_COND(ch, DRUNK));
                if (GET_COND(ch, DRUNK) < (drunk_race[(int) GET_RACE(ch)].max_cond / 2))
                    send_to_char("Вы протрезвели.\r\n", ch);
                GET_DRUNK_STATE(ch) = MAX(GET_DRUNK_STATE(ch), GET_COND(ch, DRUNK));
                return;
            default:
                break;
        }

}

// Предмет падает
void obj_drop_down(struct obj_data *object, room_rnum room)
{
    room_rnum down_rnum;
    int count = 1, i;


    if (!EXITDATA(room, DOWN)) {
        if (SECT(room) == SECT_FLYING) {        //Предмет улетел в пропасть
            act("$o0, стремительно удаляясь, полетел$G вниз.", FALSE, world[room].people, object, 0,
                TO_ROOM);
            act("$o0, стремительно удаляясь, полетел$G вниз.", FALSE, world[room].people, object, 0,
                TO_CHAR);
            extract_obj(object);
        }
        return;
    }

    /* Предмету есть куда падать */
    if (EXITDATA(room, DOWN)->to_room != NOWHERE) {
        down_rnum = room;
        for (;;) {              //получаем рномер дна
            down_rnum = EXITDATA(down_rnum, DOWN)->to_room;
            if ((!EXITDATA(down_rnum, DOWN) || EXITDATA(down_rnum, DOWN)->to_room == NOWHERE))
                break;

            if (SECT(down_rnum) != SECT_FLYING)
                break;

            if (down_rnum != room) {
                struct char_data *tch;

                for (tch = world[down_rnum].people; tch; tch = tch->next_in_room)
                    act("Падающ$V вниз $o полетел$G мимо Вас.", FALSE, tch, object, 0, TO_CHAR);
            }
            count++;
        }

        act("$o0, стремительно удаляясь, полетел$G вниз.", FALSE, world[room].people, object, 0,
            TO_ROOM);
        act("$o0, стремительно удаляясь, полетел$G вниз.", FALSE, world[room].people, object, 0,
            TO_CHAR);

        log("Падение предмета");
        obj_from_room(object);
        obj_to_room(object, down_rnum);
        switch (get_main_material_type(object)) {
            case 1:            //металл
                i = 15;
                break;
            case 2:            //камень
                i = 10;
                break;
            case 3:            //дерево
                i = 30;
                break;
            default:
                i = 0;
                break;
        }

        GET_OBJ_CUR(object) -= i * count;

        if (GET_OBJ_CUR(object) <= 10) {
            act("Упавш$V сверху $o рассыпал$U от удара о землю.", FALSE, world[down_rnum].people,
                object, 0, TO_ROOM);
            act("Упавш$V сверху $o рассыпал$U от удара о землю.", FALSE, world[down_rnum].people,
                object, 0, TO_CHAR);
            create_fragments(object);
            extract_obj(object);
            return;
        } else {
            act("$o упал$G рядом с Вами.", FALSE, world[down_rnum].people, object, 0, TO_ROOM);
            act("$o упал$G рядом с Вами.", FALSE, world[down_rnum].people, object, 0, TO_CHAR);
        }

        //Тут же тонет
        if (((SECT(down_rnum) == SECT_WATER_SWIM || SECT(down_rnum) == SECT_WATER_NOSWIM) &&
             !OBJ_FLAGGED(object, ITEM_SWIMMING)))
            obj_swim_down(object, down_rnum);
    }

}


/* Предмет тонет */
void obj_swim_down(struct obj_data *object, room_rnum room)
{
    room_rnum down_rnum;
    int count = 1;

    if (!EXITDATA(room, DOWN)) {
        if (SECT(room) == SECT_WATER_NOSWIM) {
            act("$o утонул$G.", FALSE, world[room].people, object, 0, TO_ROOM);
            act("$o утонул$G.", FALSE, world[room].people, object, 0, TO_CHAR);
            extract_obj(object);
        }
        return;
    }

    /* Предмету есть куда тонуть */
    if (EXITDATA(room, DOWN)->to_room != NOWHERE) {
        down_rnum = room;
        for (;;) {              //получаем рномер дна
            down_rnum = EXITDATA(down_rnum, DOWN)->to_room;
            if (!EXITDATA(down_rnum, DOWN) || EXITDATA(down_rnum, DOWN)->to_room == NOWHERE)
                break;
            if (down_rnum != room) {
                struct char_data *tch;

                for (tch = world[down_rnum].people; tch; tch = tch->next_in_room)
                    act("Тонущ$V $o проплыл$G мимо Вас.", FALSE, tch, object, 0, TO_CHAR);
            }
            count++;
        }

        if (SECT(room) == SECT_WATER_SWIM || SECT(room) == SECT_WATER_NOSWIM)
            act("@1 с плеском уш@1(ел,ла,ло,ли) под воду.", "КМп", world[room].people, object);
        else
            act("@1 утонул@1(,а,о,и).", "МКп", world[room].people, object);

        obj_from_room(object);
        obj_to_room(object, down_rnum);
        GET_OBJ_CUR(object) -= 3 * count;
        act("@1 плавно опустил@1(ся,ась,ось,ись) на дно.", "КМп", world[down_rnum].people, object);
    }

}

#define ROOM_DST 119

/* Персонаж падает */
void ch_drop_down(struct char_data *ch)
{
    int count = 1, dam;
    char msg[256];
    room_rnum down_rnum, room = IN_ROOM(ch);
    char buf[MAX_STRING_LENGTH];

//Пропасть
    if (!EXIT(ch, DOWN) && SECT(room) == SECT_FLYING) {
        send_to_charf(ch, "\r\nЗависнув на мгновение в воздухе, Вы улетели в пропасть.\r\n");
        send_to_charf(ch, "Вы разбились!\r\n\r\n");
        act("Зависнув на мгновение в воздухе, $n утелел$g в пропасть.", FALSE, ch, 0, 0, TO_ROOM);
        sprintf(msg, "Персонаж %s свалил%s в пропасть в локации #%d.", GET_NAME(ch),
                GET_CH_SUF_2(ch), GET_ROOM_VNUM(IN_ROOM(ch)));
        mudlog(msg, CMP, LVL_HIGOD, TRUE);
        char_from_room(ch);
        char_to_room(ch, real_room(ROOM_DST));
        GET_HIT(ch) = 1;
        GET_MOVE(ch) = 1;
        GET_MANA(ch) = 1;
        death_cry(ch, NULL);
        die(ch, NULL);
        return;
    }

    send_to_charf(ch, "\r\nЗависнув на мгновение в воздухе, Вы рухнули вниз.\r\n\r\n");
    act("Зависнув на мгновение в воздухе, $n рухнул$g вниз.", FALSE, ch, 0, 0, TO_ROOM);

    /* Персонажу есть куда падать */
    if (EXITDATA(room, DOWN)->to_room != NOWHERE) {
        down_rnum = room;
        for (;;) {              //получаем рномер дна
            down_rnum = EXITDATA(down_rnum, DOWN)->to_room;
            if (!EXITDATA(down_rnum, DOWN) || EXITDATA(down_rnum, DOWN)->to_room == NOWHERE)
                break;

            if (SECT(down_rnum) != SECT_FLYING)
                break;

            if (down_rnum != room) {
                struct char_data *tch;

                for (tch = world[down_rnum].people; tch; tch = tch->next_in_room)
                    act("Падающ$V сверху $N пролетел$G мимо Вас.", FALSE, tch, 0, ch, TO_CHAR);
            }
            count++;
        }

        dam = count * 50;

        char_from_room(ch);
        char_to_room(ch, down_rnum);
        look_at_room(ch, FALSE);

        if (dam && IS_AFFECTED(ch, AFF_LEVIT) && GET_POS(ch) == POS_FLYING) {
            if (SECT(down_rnum) == SECT_WATER_SWIM || SECT(down_rnum) == SECT_WATER_NOSWIM) {
                send_to_charf(ch, "\r\nВы плавно приземлились на поверхность воды.\r\n");
                act("Падая сверху $n плавно приземлил$u на поверхность воды.", FALSE, ch, 0, 0,
                    TO_ROOM);
            } else {
                send_to_charf(ch, "\r\nВы плавно приземлились на землю.\r\n");
                act("Падая сверху $n плавно приземлил$u на землю.", FALSE, ch, 0, 0, TO_ROOM);
            }
        } else {
            if (SECT(down_rnum) == SECT_WATER_SWIM || SECT(down_rnum) == SECT_WATER_NOSWIM) {
                send_to_charf(ch, "\r\nВы упали в воду, подняв каскад брызг.\r\n");
                act("Упавш$v сверху $n поднял$g каскад брызг.", FALSE, ch, 0, 0, TO_ROOM);
            } else {
                send_to_charf(ch, "\r\nВы ударились о твердую поверхность.\r\n");
                act("Упав сверху $n ударил$u о твердую поверхность.", FALSE, ch, 0, 0, TO_ROOM);
                GET_HIT(ch) -= dam;
                update_pos(ch);
                if (GET_POS(ch) == POS_DEAD) {
                    send_to_charf(ch, "Вы разбились!\r\n");
                    act("$n разбился от удара о землю.", FALSE, ch, 0, 0, TO_ROOM);
                    if (IN_ROOM(ch) != NOWHERE) {
                        sprintf(buf, "%s умер забившись о землю в локации [%d] %s.", GET_NAME(ch),
                                GET_ROOM_VNUM(IN_ROOM(ch)), world[IN_ROOM(ch)].name);
                        mudlog(buf, CMP, LVL_GOD, TRUE);
                    }
                    GET_HIT(ch) = 1;
                    GET_MOVE(ch) = 1;
                    GET_MANA(ch) = 1;
                    death_cry(ch, NULL);
                    die(ch, NULL);
                }
            }
        }

    }
}

void ch_swim_down(struct char_data *ch)
{
    int count = 1;
    char msg[256];
    room_rnum down_rnum, room = IN_ROOM(ch);

    if (!EXIT(ch, DOWN) && SECT(room) == SECT_WATER_NOSWIM) {
        send_to_charf(ch, "Обессилев, Вы камнем пошли на дно.\r\n");
        send_to_charf(ch, "Вы утонули.\r\n");
        act("Слабо сопротивляясь, $n погрузил$u под воду.", FALSE, ch, 0, 0, TO_ROOM);
        sprintf(msg, "Персонаж %s утонул%s в локации #%d.", GET_NAME(ch), GET_CH_SUF_1(ch),
                GET_ROOM_VNUM(IN_ROOM(ch)));
        mudlog(msg, CMP, LVL_HIGOD, TRUE);
        char_from_room(ch);
        char_to_room(ch, real_room(ROOM_DST));
        GET_HIT(ch) = 1;
        GET_MOVE(ch) = 1;
        GET_MANA(ch) = 1;
        death_cry(ch, NULL);
        die(ch, NULL);
        return;
    }

    if (!EXIT(ch, DOWN))
        return;

    send_to_charf(ch, "Обессилев, Вы камнем пошли на дно.\r\n");
    act("Слабо сопротивляясь, $n погрузил$u под воду.", FALSE, ch, 0, 0, TO_ROOM);


    /* Предмету есть куда тонуть */
    if (EXIT(ch, DOWN)->to_room != NOWHERE) {
        down_rnum = room;
        for (;;) {              //получаем рномер дна
            down_rnum = EXITDATA(down_rnum, DOWN)->to_room;
            if (!EXITDATA(down_rnum, DOWN) || EXITDATA(down_rnum, DOWN)->to_room == NOWHERE)
                break;
            if (down_rnum != room) {
                struct char_data *tch;

                for (tch = world[down_rnum].people; tch; tch = tch->next_in_room)
                    act("Тонущ$V $N проплыл$G мимо Вас.", FALSE, tch, 0, ch, TO_ROOM);
            }
            count++;
        }


        char_from_room(ch);
        char_to_room(ch, down_rnum);
        look_at_room(ch, FALSE);

        /*  if (EXITDATA(down_rnum,DOWN))
           act("$n притонул$g сверху.",TRUE,ch,0,0,TO_ROOM);
           else
           act("$n притонул$g сверху на дно.",TRUE,ch,0,0,TO_ROOM); */
    }
}

void check_idling(struct char_data *ch)
{
    char buf[MAX_STRING_LENGTH];

    if (ch->desc && ch->desc->character)
        return;

    if (!RENTABLE(ch)) {
        if (++(ch->char_specials.timer) > idle_void) {
            if (GET_WAS_IN(ch) == NOWHERE && ch->in_room != NOWHERE) {
                GET_WAS_IN(ch) = ch->in_room;
                /*if (FIGHTING(ch))
                   {
                   stop_fighting(FIGHTING(ch),FALSE);
                   stop_fighting(ch,TRUE);
                   } */
                SET_BIT(PLR_FLAGS(ch, PLR_DROPLINK), PLR_DROPLINK);
                act("1+и погрузил1(ся,ась,ось,ись) в транс, перестав воспринимать действительность.", "Км", ch);
                save_pets(ch);
                save_vars(ch);
                save_quests(ch);
                save_char(ch, NOWHERE);
                xsave_rent(ch, RENT_LD, FALSE);
                //char_from_room(ch);
                //char_to_room(ch, STRANGE_ROOM);
            } else if (ch->char_specials.timer > idle_rent_time) {
                save_pets(ch);
                if (ch->in_room != NOWHERE)
                    char_from_room(ch);
                char_to_room(ch, STRANGE_ROOM);
                if (ch->desc) {
                    STATE(ch->desc) = CON_DISCONNECT;
                    ch->desc->character = NULL;
                    ch->desc = NULL;
                }
                save_vars(ch);
                save_quests(ch);
                save_char(ch, NOWHERE);
                if (free_rent)
                    xsave_rent(ch, RENT_NORMAL, TRUE);
                else
                    xsave_rent(ch, RENT_LD, TRUE);
                sprintf(buf, "%s помещен на постой по таймеру отключения.", GET_NAME(ch));
                mudlog(buf, CMP, LVL_GOD, TRUE);
                extract_char(ch, FALSE);
            }
        }
    }
}

/* Update PCs, NPCs, and objects */
#define NO_DESTROY(obj) ((obj)->carried_by   || \
                         (obj)->worn_by      || \
                         (obj)->in_obj       || \
                         GET_OBJ_TYPE(obj) == ITEM_TATOO || \
                         GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN || \
                         OBJ_FLAGGED(obj, ITEM_NODECAY))
#define NO_TIMER(obj)   (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN)
/* || OBJ_FLAGGED(obj, ITEM_NODECAY))*/

int up_obj_where(struct obj_data *obj)
{
    if (obj->in_obj)
        return up_obj_where(obj->in_obj);
    else
        return OBJ_WHERE(obj);
}

void tics_update(void)
{
    struct descriptor_data *i;

    return;

    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING || i->character == NULL)
            continue;


    }
}



void hour_update(void)
{
    int i;

    for (i = 0; i < top_of_world; i++)
        if (world[i].period)
            world_period_update(i);
}

void pk_update(struct char_data *i)
{
    PK_Memory_type::iterator pk;

    // декрементируем таймер, убираем элементы с нулевым таймером
    for (pk = i->pk_list.begin(); pk != i->pk_list.end();)
        if (pk->second > 1) {
            pk->second--;
            ++pk;
        } else {
            i->pk_list.erase(pk++);
        }
}

//Апдейт каждый игровой пульс
void beat_points_update(int pulse)
{
    struct char_data *i, *next_char;
    int restore, count, n, vnum_room, keys, dam, cont = 0;
    int sp_foul_flesh = find_spell_num(SPELL_FOUL_FLESH);
    struct obj_data *j, *next_thing, *jj, *next_thing2;
    struct C_obj_affected_type *af_obj, *af_obj_next;
    struct room_affect_data *af_room, *af_room_next;
    char buf[MAX_STRING_LENGTH];

    if (!UPDATE_PC_ON_BEAT)
        return;

    for (j = object_list; j; j = next_thing) {
        next_thing = j->next;

        //Изменение действия эффектов
        for (af_obj = j->C_affected; af_obj; af_obj = af_obj_next) {
            af_obj_next = af_obj->next;

            if (af_obj->type == spellnum_db)
                continue;

            if (af_obj->duration >= 1)
                af_obj->duration--;
            else if (af_obj->duration == -1)
                af_obj->duration = -1;
            else
                affect_obj_removed(j, af_obj);
        }

        if (j && ((OBJ_FLAGGED(j, ITEM_ZONEDECAY) && GET_OBJ_ZONE(j) != NOWHERE && up_obj_where(j) != NOWHERE && GET_OBJ_ZONE(j) != world[up_obj_where(j)].zone) || ((GET_OBJ_TIMER(j) - (time(0) - GET_OBJ_TIMELOAD(j)) / 60) <= 0 && !NO_TIMER(j)) || (GET_OBJ_DESTROY(j) == 0 && !NO_DESTROY(j)))) {
                                                                /**** рассыпание обьекта */
            for (jj = j->contains; jj; jj = next_thing2) {
                next_thing2 = jj->next_content;
                obj_from_obj(jj);
                if (j->in_obj)
                    obj_to_obj(jj, j->in_obj);
                else if (j->worn_by)
                    obj_to_char(jj, j->worn_by);
                else if (j->carried_by)
                    obj_to_char(jj, j->carried_by);
                else if (j->in_room != NOWHERE)
                    obj_to_room(jj, j->in_room);
                else {
                    log("ОШИБКА: extract %s from %s to NOTHING (2)", jj->PNames[0], j->PNames[0]);
                    extract_obj(jj);
                }
            }
            if (j->worn_by) {
                struct char_data *k = j->worn_by;

                switch (j->worn_on) {
                    case WEAR_LIGHT:
                    case WEAR_SHIELD:
                    case WEAR_WIELD:
                    case WEAR_HOLD:
                    case WEAR_BOTHS:
                        act("$o развалил$U у Вас в руках.", FALSE, j->worn_by, j, 0, TO_CHAR);
                        break;
                    default:
                        act("@1и рассыпал@1(ся,ась,ось,ись) прямо на Вас.", "Мп", j->worn_by, j);
                        break;
                }

                unequip_char(j->worn_by, j->worn_on);
                if (k) {
                    check_sets(k);
                    affect_total(k);
                }
            } else if (j->carried_by) {
                act("$o развалил$U в Ваших руках.", FALSE, j->carried_by, j, 0, TO_CHAR);
                obj_from_char(j);
            } else if (j->in_room != NOWHERE && GET_OBJ_TIMER(j) != -1) {
                if (world[j->in_room].people) {
                    act("$o рассыпал$U в прах, который был развеян ветром.",
                        FALSE, world[j->in_room].people, j, 0, TO_CHAR);
                    act("$o рассыпал$U в прах, который был развеян ветром.",
                        FALSE, world[j->in_room].people, j, 0, TO_ROOM);
                }
                log("Распание предмета2 %s", j->name);
                obj_from_room(j);
                log("Распание предмета2end");
            } else if (j->in_obj)
                obj_from_obj(j);
            extract_obj(j);
            continue;
        }
        //повреждения внутри обьекта
        for (jj = j->contains; jj; jj = next_thing2) {
            next_thing2 = jj->next_content;
            if (GET_OBJ_TEMP_ADD(jj) > 30) {
                dam = MAX(0, 100 - get_save_obj(jj, TYPE_FIRE));
                dam = interpolate(dam, pulse);
                GET_OBJ_CUR(j) -= dam;
            }
        }
        if (GET_OBJ_CUR(j) <= 0) {
            if (j->worn_by)
                act("$o развалил$U под воздействием высокой температуры.", FALSE, j->worn_by, j, 0,
                    TO_CHAR);
            else if (j->carried_by)
                act("$o развалил$U под воздействием высокой температуры.", FALSE, j->carried_by, j,
                    0, TO_CHAR);
            else if (j->in_room != NOWHERE)
                if (world[j->in_room].people) {
                    act("$o развалил$U под воздействием высокой температуры.", FALSE,
                        world[j->in_room].people, j, 0, TO_CHAR);
                    act("$o развалил$U под воздействием высокой температуры.", FALSE,
                        world[j->in_room].people, j, 0, TO_ROOM);
                }
            extract_obj(j);
            continue;
        }
        // Тонущие, падающие, и сыпящиеся обьекты.
        if (j->contains)
            cont = TRUE;
        else
            cont = FALSE;
        if (obj_decay(j))
            if (cont)
                next_thing = object_list;

        //Скелет и осквернение
        /*    if (IS_CORPSE(j) && GET_OBJ_VAL(j, 3) == 4)
           if (IN_ROOM(j) != NOWHERE &&
           affected_room_by_bitvector(&world[IN_ROOM(j)],ROOM_AFF_DESECRATE) &&
           !affected_room_by_bitvector(&world[IN_ROOM(j)],ROOM_AFF_SANCTUARY))
           create_skelet(j); */
    }

    /* Тонущие, падающие, и сыпящиеся обьекты.
       for (j = object_list; j; j = next_thing)
       {
       next_thing = j->next;
       if (j->contains)
       {
       cont = TRUE;
       }
       else
       {
       cont = FALSE;
       }
       if (obj_decay(j))
       {
       if (cont)
       {
       next_thing = object_list;
       }
       }
       } */

    for (i = character_list; i; i = next_char) {
        next_char = i->next;

        pk_update(i);
        char_period_update(i, pulse);

        if (!IS_NPC(i)) {       //Обработка игроков
            gain_condition(i, FULL, full_race[(int) GET_RACE(i)].dec_cond);
            gain_condition(i, DRUNK, drunk_race[(int) GET_RACE(i)].dec_cond);
            gain_condition(i, THIRST, thirst_race[(int) GET_RACE(i)].dec_cond);
            gain_sleep(i);
            // посаженый в лабиринты
            if (PLR_FLAGGED(i, PLR_HELLED) && HELL_DURATION(i) && HELL_DURATION(i) <= time(NULL)) {
                restore = PLR_TOG_CHK(i, PLR_HELLED);
                if ((restore = GET_LOADROOM(i)) == NOWHERE)
                    restore = calc_loadroom(i);
                restore = real_room(restore);
                if (restore == NOWHERE) {
                    if (GET_LEVEL(i) >= LVL_IMMORT)
                        restore = r_immort_start_room;
                    else
                        restore = r_mortal_start_room;
                }
                act("Ваше время пребывания в лабиринте закончилось.", FALSE, i, 0, 0, TO_CHAR);
                char_from_room(i);
                char_to_room(i, restore);
                look_at_room(i, restore);
                act("В воздухе образовался портал украшенный геометрическими фигурами.", FALSE, i,
                    0, 0, TO_ROOM);
                act("$n пулей вылетел$g из портала.", FALSE, i, 0, 0, TO_ROOM);
                act("Портал украшенный геометрическими фигурами рассыпался в пыль.", FALSE, i, 0, 0,
                    TO_ROOM);
            }
            // снятие простуды
            if (PLR_FLAGGED(i, PLR_MUTE) && MUTE_DURATION(i) != 0 && MUTE_DURATION(i) <= time(NULL)) {
                restore = PLR_TOG_CHK(i, PLR_MUTE);
                send_to_char("Ваше горло прошло.\r\n", i);
            }
            // снятие молчанки
            if (PLR_FLAGGED(i, PLR_DUMB) && DUMB_DURATION(i) != 0 && DUMB_DURATION(i) <= time(NULL)) {
                restore = PLR_TOG_CHK(i, PLR_DUMB);
                send_to_char("У Вас образовался рот.\r\n", i);
            }

            if (PLR_FLAGGED(i, PLR_FROZEN) &&
                FREEZE_DURATION(i) != 0 && FREEZE_DURATION(i) <= time(NULL)) {
                restore = PLR_TOG_CHK(i, PLR_FROZEN);
                act("В воздухе образовался пылающий огнем портал.", FALSE, i, 0, 0,
                    TO_ROOM | TO_CHAR);
                act("Огненый шар вылетевший из портала расстопил ледяной панцирь вокруг $n1.",
                    FALSE, i, 0, 0, TO_ROOM);
                act("Огненый шар вылетевший из портала расстопил ледяной панцирь вокруг Вас.",
                    FALSE, i, 0, 0, TO_ROOM);
                act("Пылающий огнем портал потух и превратился в пепел.", FALSE, i, 0, 0,
                    TO_ROOM | TO_CHAR);
            }
            //Всякая хрень от Былин может пригодится когда нить
            if (PLR_FLAGGED(i, PLR_NAMED) && NAME_DURATION(i) && NAME_DURATION(i) <= time(NULL)) {
                restore = PLR_TOG_CHK(i, PLR_NAMED);
                send_to_char("Вас выпустили из комнаты имени.\r\n", i);
                if ((restore = GET_LOADROOM(i)) == NOWHERE)
                    restore = calc_loadroom(i);
                restore = real_room(restore);
                if (restore == NOWHERE) {
                    if (GET_LEVEL(i) >= LVL_IMMORT)
                        restore = r_immort_start_room;
                    else
                        restore = r_mortal_start_room;
                }
                char_from_room(i);
                char_to_room(i, restore);
                look_at_room(i, restore);
                act("Окутанный розовым дымом $n появил$u в центре комнаты.", FALSE, i, 0, 0,
                    TO_ROOM);
            }
            if (GET_GOD_FLAG(i, GF_GODSLIKE) &&
                GODS_DURATION(i) != 0 && GODS_DURATION(i) <= time(NULL)) {
                CLR_GOD_FLAG(i, GF_GODSLIKE);
                send_to_char("Вы более не под защитой Богов.\r\n", i);
            }
            if (GET_GOD_FLAG(i, GF_GODSCURSE) &&
                GODS_DURATION(i) != 0 && GODS_DURATION(i) <= time(NULL)) {
                CLR_GOD_FLAG(i, GF_GODSCURSE);
                send_to_char("Боги более не в обиде на Вас.\r\n", i);
            }

        }                       //Конец обработки игроков

        //if (GET_POS(i) >= POS_STUNNED) //Гейн
        if (IN_ROOM(i) != NOWHERE) {
            //Восстанавливаем жизнь
            restore = hit_gain(i);
            restore = interpolate(restore, pulse);
            int val = 0;

            if ((val = affected_by_spell_real(i, sp_foul_flesh)))
                GET_HIT(i) -= dice(1, val);
            else if (GET_POS(i) < POS_INCAP)
                GET_HIT(i) -= restore;
            else if (GET_HIT(i) < GET_REAL_MAX_HIT(i))
                GET_HIT(i) = MIN(GET_HIT(i) + restore, GET_REAL_MAX_HIT(i));
            else if (GET_HIT(i) > GET_MAX_HIT(i) + GET_HIT_ADD(i))
                GET_HIT(i)--;

            if (GET_HIT(i) >= GET_REAL_MAX_HIT(i) && AFF_FLAGGED(i, AFF_BANDAGE))
                affect_from_char(i, SPELL_BANDAGE);



            // Восстанавливаем бодрость
            restore = move_gain(i);
            restore = interpolate(restore, pulse);
            if (real_sector(IN_ROOM(i)) == SECT_WATER_NOSWIM && !has_boat(i) &&
                !(IS_AFFECTED(i, AFF_FLY) && GET_POS(i) == POS_FLYING)
                && !IS_AFFECTED(i, AFF_WATERWALK) && !MOB_FLAGGED(i, MOB_FLYING)
                && !MOB_FLAGGED(i, MOB_SWIMMING) && !ON_HORSE_FLY(i) && !ON_HORSE_SWIM(i)
                && !PRF_FLAGGED(i, PRF_NOHASSLE))
                GET_MOVE(i) = MAX(0, GET_MOVE(i) - interpolate(50, pulse));
            else if (GET_MOVE(i) < GET_REAL_MAX_MOVE(i))
                GET_MOVE(i) = MIN(GET_MOVE(i) + restore, GET_REAL_MAX_MOVE(i));

            if (GET_MOVE(i) > GET_MAX_MOVE(i) + GET_MOVE_ADD(i))
                GET_MOVE(i)--;

            // Восстанавливаем ману
            if (IS_MANA_CASTER(i)) {
                restore = mana_gain(i);
                restore = interpolate(restore, pulse);
                if (GET_MANA(i) != GET_REAL_MAX_MANA(i))
                    GET_MANA(i) = MIN(GET_MANA(i) + restore, GET_REAL_MAX_MANA(i));
                if (GET_MANA(i) >= GET_REAL_MAX_MANA(i) && AFF_FLAGGED(i, AFF_MEDITATION))
                    affect_from_char(i, SPELL_MEDITATION);


                if (GET_MANA(i) > GET_MAX_MANA(i) + GET_MANA_ADD(i))
                    GET_MANA(i)--;

            }
            //Уменьшаем время мема
            if (IS_MANA_CASTER(i))
                for (n = 1; n <= TOP_SPELL_DEFINE; n++)
                    if (GET_SPELL_MEM(i, n))
                        GET_SPELL_MEM(i, n)--;
        }                       //Конец гейна

        //повреждения от оружия на себе
        if (!(pulse % 3))
            check_obj_damage(i);

        recalc_realtime(i);

        /*if (GET_POS(i) &&
           GET_POS(i) <= POS_STUNNED) */
        update_pos(i);

        if (GET_POS(i) == POS_DEAD) {
            GET_HIT(i) = 1;
            GET_MOVE(i) = 1;
            GET_MANA(i) = 1;
            death_cry(i, NULL);
            die(i, NULL);
        }

    }                           //Конец обработки персонажей

    for (count = 0; count <= top_of_world; count++) {
        //Обрабатываем эффекты локации
        for (af_room = world[count].affects; af_room; af_room = af_room_next) {
            af_room_next = af_room->next;
            if (af_room->duration >= 1)
                af_room->duration--;
            else if (af_room->duration == -1)
                af_room->duration = -1;
            else
                affect_room_removed(&world[count], af_room);
        }

        for (n = 0; n < NUM_OF_DIRS; n++) {
            if (EXITDATA(count, n) && world[count].dir_option[n]->type_port) {
                keys = world[count].dir_option[n]->key_port;
                vnum_room = check_portal(keys, count);

                if ((vnum_room && !(world[count].dir_option[n]->timer)) ||
                    (keys == -1
                     && world[count].dir_option[n]->open >=
                     zone_table[world[count].zone].time_info.hours
                     && world[count].dir_option[n]->close <=
                     zone_table[world[count].zone].time_info.hours)) {
                    if (world[count].dir_option[n]->mess_to_open)
                        act(world[count].dir_option[n]->mess_to_open, "КМ", world[count].people);
                    world[count].dir_option[n]->timer = world[count].dir_option[n]->time;
                    world[count].dir_option[n]->active_room = vnum_room;
                }
            }
        }

        // Обрабатываем порталы
        for (n = 0; n < NUM_OF_DIRS; n++) {
            if (EXITDATA(count, n) && world[count].dir_option[n]->timer) {
                keys = world[count].dir_option[n]->key_port;
                vnum_room = check_portal(keys, count);
                if (!vnum_room
                    || (keys == -1
                        && world[count].dir_option[n]->open <
                        zone_table[world[count].zone].time_info.hours
                        && world[count].dir_option[n]->close >
                        zone_table[world[count].zone].time_info.hours)) {
                    world[count].dir_option[n]->timer--;
                    if (!world[count].dir_option[n]->timer) {
                        sprintf(buf, "%s", world[count].dir_option[n]->mess_to_close);
                        act(buf, FALSE, world[count].people, 0, 0, TO_ROOM);
                        act(buf, FALSE, world[count].people, 0, 0, TO_CHAR);
                    }
                }
            }
        }
    }

}

void pulse_update(int pulse)
{
    int count;
    int spell_water = find_spell_num(SPELL_WATERD);
    struct char_data *tch, *next_ch;
    char msg[MAX_INPUT_LENGTH];
    struct char_data *i, *next_char;
    void forcedir(struct char_data *ch, int rnum, struct room_forcedir_data *fd);


    for (i = character_list; i; i = next_char) {
        next_char = i->next;
        if (PRF_FLAGGED(i, PRF_NOHASSLE) || GET_WAIT_STATE(i) < 0)
            GET_WAIT_STATE(i) = 0;

        if (GET_WAIT_STATE(i) > 0)
            GET_WAIT_STATE(i)--;
    }

    for (count = 0; count <= top_of_world; count++) {
        //Обрабатываем течения
        if (world[count].forcedir) {
            struct room_forcedir_data *fd;

            for (fd = world[count].forcedir; fd; fd = fd->next)
                //if (!(pulse % (fd->timer+number(1,3))))
                if (!(pulse % fd->timer)) {
                    for (tch = world[count].people; tch; tch = next_ch) {
                        next_ch = tch->next_in_room;
                        forcedir(tch, count, fd);
                    }
                }
        }
        //Обрабатываем повреждения в локации
        if (world[count].damage && !(pulse % world[count].damage->chance)) {
            struct char_data *tch, *cnext;

            for (tch = world[count].people; tch; tch = cnext) {
                cnext = tch->next_in_room;
                if (IS_SOUL(tch))
                    continue;
                if ((world[count].damage->chance) < number(1, 100))
                    continue;
                if (tch && check_wld_damage(world[count].damage->type, tch))
                    damage_wld(tch, count, TRUE);
            }
        }
        //Задыхаемся под водой
        if (real_sector(count) == SECT_UNDERWATER && !(pulse % 30)) {
            for (tch = world[count].people; tch; tch = next_ch) {
                next_ch = tch->next_in_room;
                if (!IS_AFFECTED(tch, AFF_WATERBREATH) &&
                    !MOB_FLAGGED(tch, MOB_ONLYSWIMMING) &&
                    !MOB_FLAGGED(tch, MOB_SWIMMING) &&
                    !PRF_FLAGGED(tch, PRF_NOHASSLE) && !IS_SOUL(tch) && !IS_UNDEAD(tch)) {
                    struct P_damage damage;
                    struct P_message message;

                    damage.valid = true;
                    damage.type = HIT_NEGATIVE;
                    int dam = GET_MAX_HIT(tch) / 10;

                    damage.dam = dam;
                    damage.check_ac = A_POWER;
                    damage.power = A_POWER;
                    damage.armor = FALSE;
                    damage.location = TRUE;

                    GetSpellMessage(spell_water, message);
                    if (_damage(tch, tch, 0, 0, A_POWER, FALSE, damage, message) == RD_KILL) {
                        sprintf(msg, "Персонаж %s умер%s под водой в локации #%d.", GET_NAME(tch),
                                GET_CH_SUF_4(tch), GET_ROOM_VNUM(IN_ROOM(tch)));
                        mudlog(msg, CMP, LVL_HIGOD, TRUE);
                    }
                }
            }
        } else if (real_sector(count) == SECT_FLYING) {
            for (tch = world[count].people; tch; tch = next_ch) {
                next_ch = tch->next_in_room;
                // падаем в воздухе
                if (!(IS_AFFECTED(tch, AFF_FLY) && GET_POS(tch) == POS_FLYING)
                    && !MOB_FLAGGED(tch, MOB_FLYING) && !ON_HORSE_FLY(tch)
                    && !PRF_FLAGGED(tch, PRF_NOHASSLE) && !IS_SOUL(tch))
                    ch_drop_down(tch);
            }
        } else if (real_sector(count) == SECT_WATER_NOSWIM || real_sector(count) == SECT_UNDERWATER) {
            for (tch = world[count].people; tch; tch = next_ch) {
                next_ch = tch->next_in_room;
                //тонем
                if (!(IS_AFFECTED(tch, AFF_FLY) && GET_POS(tch) == POS_FLYING)
                    && !IS_AFFECTED(tch, AFF_WATERWALK) && !MOB_FLAGGED(tch, MOB_LEVITING)
                    && !MOB_FLAGGED(tch, MOB_FLYING) && !MOB_FLAGGED(tch, MOB_ONLYSWIMMING)
                    && !MOB_FLAGGED(tch, MOB_SWIMMING) && !GET_MOVE(tch)
                    && !PRF_FLAGGED(tch, PRF_NOHASSLE) && !has_boat(tch) && !ON_HORSE_FLY(tch)
                    && !ON_HORSE_SWIM(tch) && !IS_SOUL(tch))
                    ch_swim_down(tch);
            }
        }
    }
}

static void mprog_tick(struct char_data *ch)
{
    FENIA_VOID_CALL(ch, "Tick", "", ch);
    FENIA_PROTO_VOID_CALL(ch->npc(), "Tick", "C", ch);

}

/* Апдейт каждый тик */
void point_update(void)
{
    struct char_data *i, *next_char;
    struct obj_data *j, *next_thing, *jj, *next_thing2;
    struct affected_type *af, *next;
    struct track_data *track, *next_track, *temp;
    struct descriptor_data *d;
    int count, spellnum, mana, restore;


    for (d = descriptor_list; d; d = d->next) {
        if (STATE(d) != CON_PLAYING || d->character == NULL)
            continue;
        GET_TICKS(d->character)++;
        if (!PLR_FLAGGED(d->character, PLR_WRITING) &&
            !IS_GOD(d->character) && d->character->char_specials.timer <= idle_void)
            d->has_prompt = 0;
    }

    for (i = character_list; i; i = next_char) {
        next_char = i->next;

        if (IS_SOUL(i)) {
            if (--EXTRACT_TIMER(i) <= 0)
                death(i);
            continue;
        }

        /* Обсчет для игроков */
        if (!IS_NPC(i)) {
            /* считаем ренту */
            if (RENTABLE(i) < time(NULL))
                RENTABLE(i) = 0;
            /*if (age_old(i)->month == 0 && age_old(i)->day == 0 && age_old(i)->hours == 0)
               send_to_charf(i,"У Вас сегодня День Рождения. Вам исполнилось %d %s.\r\n",GET_AGE(i), desc_count(GET_AGE(i), WHAT_YEAR)); */
            /* Считаем время памяти локаций */
            for (count = 0; count <= GET_MAXMEM(i); count++)
                if (GET_MEMORY(i, count))
                    if (--GET_TIMEMEM(i, count) <= 0)
                        del_memory(i, count);
        }

        /* Для всех  */
        //Снимаем время с HOLDа
        int nhl = find_spell_num(SPELL_HOLD);

        for (af = i->affected; af; af = next) {
            next = af->next;
            if (af->type != nhl)
                continue;
            if (af->duration >= 1)
                af->duration--;
            else if (af->duration == -1)
                af->duration = -1;
            else {
                if (!af->next || (af->next->type != af->type) || (af->next->duration > 0)) {
                    if (af->type > 0 && af->type <= MAX_SPELLS && *spell_wear_off_msg[af->type])
                        show_spell_off(af->type, i, NULL);
                }
                affect_remove(i, af);
            }
        }

        //update_char_objects(i);

        if (!IS_NPC(i) && GET_LEVEL(i) < idle_max_level)
            check_idling(i);

        mprog_tick(i);

    }                           //Конец обсчета персонажей

    /* Расчет локаций */
    for (count = 0; count <= top_of_world; count++) {
        if (world[count].fires) {
            switch (get_room_sky(count)) {
                case SKY_CLOUDY:
                    mana = number(1, 2);
                    break;
                case SKY_CLOUDLESS:
                    mana = number(1, 3);
                    break;
                case SKY_RAINING:
                    mana = 4;
                    break;
                default:
                    mana = 1;
            }
            world[count].fires -= MIN(mana, world[count].fires);
            if (world[count].fires <= 0) {
                act("Костер затух.", FALSE, world[count].people, 0, 0, TO_ROOM);
                act("Костер затух.", FALSE, world[count].people, 0, 0, TO_CHAR);
                world[count].fires = 0;
            }
        }                       //Конец расчет костра

        if (world[count].forbidden)
            world[count].forbidden--;

        if (world[count].portal_time) {
            world[count].portal_time--;
            if (!world[count].portal_time) {
                act("Пентаграмма медленно растаяла.", FALSE, world[count].people, 0, 0, TO_ROOM);
                act("Пентаграмма медленно растаяла.", FALSE, world[count].people, 0, 0, TO_CHAR);
            }
        }

        if (RM_BLOOD(count) > 0)
            RM_BLOOD(count) -= 1;

        world[count].glight = MAX(0, world[count].glight);
        world[count].gdark = MAX(0, world[count].gdark);

        for (track = world[count].track, temp = NULL; track; track = next_track) {
            next_track = track->next;
            switch (real_sector(count)) {
                case SECT_FLYING:
                case SECT_UNDERWATER:
                case SECT_SECRET:
                case SECT_WATER_SWIM:
                case SECT_WATER_NOSWIM:
                case SECT_INSIDE:
                    spellnum = 31;
                    break;
                case SECT_THICK_ICE:
                case SECT_NORMAL_ICE:
                case SECT_THIN_ICE:
                    spellnum = 16;
                    break;
                case SECT_CITY:
                    spellnum = 4;
                    break;
                case SECT_FIELD:
                case SECT_FIELD_RAIN:
                    spellnum = 2;
                    break;
                case SECT_FIELD_SNOW:
                    spellnum = 1;
                    break;
                case SECT_FOREST:
                case SECT_FOREST_RAIN:
                    spellnum = 2;
                    break;
                case SECT_FOREST_SNOW:
                    spellnum = 1;
                    break;
                case SECT_HILLS:
                case SECT_HILLS_RAIN:
                    spellnum = 4;
                    break;
                case SECT_HILLS_SNOW:
                    spellnum = 2;
                    break;
                case SECT_MOUNTAIN:
                    spellnum = 4;
                    break;
                case SECT_MOUNTAIN_SNOW:
                    spellnum = 2;
                    break;
                default:
                    spellnum = 2;
            }

            for (mana = 0, restore = FALSE; mana < NUM_OF_DIRS; mana++) {
                if ((track->time_income[mana] <<= spellnum))
                    restore = TRUE;
                if ((track->time_outgone[mana] <<= spellnum))
                    restore = TRUE;
            }
            if (!restore) {
                if (temp)
                    temp->next = next_track;
                else
                    world[count].track = next_track;
                free(track);
            } else
                temp = track;
        }                       //Конец трека


    }                           //Конец расчета локаций

    /* Расчет предметов */
    for (j = object_list; j; j = next_thing) {
        next_thing = j->next;
        struct C_obj_affected_type *af, *next_af;

        //Изменение температуры
        if (GET_OBJ_TEMP_ADD(j) != 0) {
            if (GET_OBJ_TEMP_ADD(j) < 0)
                GET_OBJ_TEMP_ADD(j) += 10;
            else
                GET_OBJ_TEMP_ADD(j) -= 10;
        }
        //Изменение силы яда
        for (af = j->C_affected; af; af = next_af) {
            next_af = af->next;
            if (af->type == spellnum_db)
                continue;
            if (af->location == APPLY_POISON) {
                af->modifier -= 10;
                if (af->modifier <= 0) {
                    if (af->main && j->worn_by)
                        show_spell_off(af->type, j->worn_by, NULL);
                    affect_obj_removed(j, af);
                }
            }
        }

        /* свет */
        if (GET_OBJ_TYPE(j) == ITEM_LIGHT) {
            int iz;
            bool mhave = TRUE;
            struct char_data *ch = NULL;

            if (j->worn_by)
                ch = j->worn_by;
            else if (j->carried_by)
                ch = j->carried_by;
            else {
                ch = world[IN_ROOM(j)].people;
                mhave = FALSE;
            }

            if (GET_LIGHT_VAL(j) > 0 && GET_LIGHT_ON(j)) {
                iz = --GET_LIGHT_VAL(j);
                if (iz == 1) {
                    log("Свет1");
                    if (mhave) {
                        log("Свет1.1");
                        act("Ваш$G $o замерцал$G и начал$G угасать.", FALSE, ch, j, 0, TO_CHAR);
                        act("$o $n1 замерцал$G и начал$G угасать.", FALSE, ch, j, 0, TO_ROOM);
                        log("Свет1.2");
                    } else {
                        log("Свет2.1");
                        log("Предмет %s %d", GET_OBJ_PNAME(j, 0), IN_ROOM(j));
                        if (ch && IN_ROOM(j) != NOWHERE) {
                            act("$o замерцал$G и начал$G угасать.", FALSE, ch, j, 0, TO_CHAR);
                            act("$o замерцал$G и начал$G угасать.", FALSE, ch, j, 0, TO_ROOM);
                        }
                        log("Свет2.2");
                    }
                    log("Свет***");
                } else if (iz <= 0) {
                    if (mhave && IN_ROOM(j) != NOWHERE) {
                        act("Ваш$G $o погас$Q.", FALSE, ch, j, 0, TO_CHAR);
                        act("$o $n1 погас$Q.", FALSE, ch, j, 0, TO_ROOM);
                    } else if (IN_ROOM(j) != NOWHERE) {
                        act("$o погас$Q.", FALSE, ch, j, 0, TO_CHAR);
                        act("$o погас$Q.", FALSE, ch, j, 0, TO_ROOM);
                    }
                    GET_LIGHT_ON(j) = FALSE;
                    if (IN_ROOM(j) != NOWHERE) {
                        if (world[IN_ROOM(j)].light > 0)
                            world[IN_ROOM(j)].light -= 1;
                    }
                    if (OBJ_FLAGGED(j, ITEM_DECAY))
                        extract_obj(j);
                }
            }
        }

        if (IS_CORPSE(j)) {
            /* timer count down */

            if ((GET_OBJ_TIMER(j) - (time(0) - GET_OBJ_TIMELOAD(j)) / 60) == 11)
                corpse_desc_rotten(j);

            if ((GET_OBJ_TIMER(j) - (time(0) - GET_OBJ_TIMELOAD(j)) / 60) == 6)
                corpse_desc_skelet(j);

            if ((GET_OBJ_TIMER(j) - (time(0) - GET_OBJ_TIMELOAD(j)) / 60) <= 0) {
                for (jj = j->contains; jj; jj = next_thing2) {
                    next_thing2 = jj->next_content;     /* Next in inventory */
                    obj_from_obj(jj);
                    if (j->in_obj)
                        obj_to_obj(jj, j->in_obj);
                    else if (j->carried_by)
                        obj_to_char(jj, j->carried_by);
                    else if (j->in_room != NOWHERE)
                        obj_to_room(jj, j->in_room);
                    else {
                        log("ОШИБКА: extract %s from %s to NOTHING (1)", jj->PNames[0],
                            j->PNames[0]);
                        extract_obj(jj);
                    }
                }
                if (j->carried_by) {
                    act("$p превратил$U в пыль в Ваших руках.", FALSE, j->carried_by, j, 0,
                        TO_CHAR);
                    obj_from_char(j);
                } else if (j->in_room != NOWHERE && GET_OBJ_TIMER(j) != -1) {
                    if (world[j->in_room].people) {
                        act("$o3 рассыпал$U в пыль, которую развеял ветер.", TRUE,
                            world[j->in_room].people, j, 0, TO_ROOM);
                        act("$o3 рассыпал$U в пыль, которую развеял ветер.", TRUE,
                            world[j->in_room].people, j, 0, TO_CHAR);

                        if (IS_CORPSE(j)) {     //number(1,100) < 101)
                            struct obj_data *prh = NULL;

                            if ((prh = read_object(10, VIRTUAL, TRUE)))
                                obj_to_room(prh, j->in_room);
                            else {
                                //
                            }
                        }

                    }
                    obj_from_room(j);
                } else if (j->in_obj)
                    obj_from_obj(j);

                extract_obj(j);
            }
        }                       //Конец расчет трупа
        else {
            if ((GET_OBJ_TIMER(j) - (time(0) - GET_OBJ_TIMELOAD(j)) / 60) <= 0) {
                //j = NULL;
                extract_obj(j);
            } else if (GET_OBJ_DESTROY(j) > 0 && !NO_DESTROY(j))
                GET_OBJ_DESTROY(j)--;

            // Обработку Яда на оружие идет по другому
            // else /* decay poision && other affects */
            /*
               for (count = 0; count < MAX_OBJ_AFFECT; count++)
               if (j->affected[count].location == APPLY_POISON)
               {
               j->affected[count].modifier--;
               if (j->affected[count].modifier <= 0)
               {
               j->affected[count].location = APPLY_NONE;
               j->affected[count].modifier = 0;
               }
               }
             */
        }
    }                           //Конец рассчета предметов

}

//повреждение от горячих металов
int calc_warn_dam(struct char_data *ch, int temp)
{
    byte saves;
    int dam = 0;

    switch (temp) {
        case 6:
            dam = dice(1, 8);
            saves = 0;
            break;
        case 7:
            dam = dice(2, 6);
            saves = 1;
            break;
        case 8:
            dam = dice(3, 6);
            saves = 2;
            break;
        default:
            return (0);
            break;
    }

    return (dam);
}

void check_obj_damage(struct char_data *ch)
{
    ACMD(do_remove);
    ACMD(do_drop);
    struct obj_data *tobj, *onext;
    int pos, dam = 0;

    if (ch == NULL)
        return;
    if (MOB_FLAGGED(ch, MOB_NOFIGHT))
        return;

    for (pos = 0; pos < NUM_WEARS; pos++)
        if (GET_EQ(ch, pos)) {
            tobj = GET_EQ(ch, pos);
            if (is_metall(tobj) && GET_OBJ_REAL_TEMP(tobj) >= 30) {
                dam = calc_warn_dam(ch, get_const_obj_temp(tobj));
                if (dam > 0) {
                    damage_obj(ch, tobj, dam, SPELL_HEAT_METALL, TRUE);
                    if (IS_NPC(ch)) {
                        do_remove(ch, GET_OBJ_PNAME(tobj, 0), 0, 0, 1);
                        do_drop(ch, GET_OBJ_PNAME(tobj, 0), SCMD_DROP, 0, 1);
#if 0
                        /* this flag doesn't work for npc, should be done another way */
                        SET_BIT(PRF_FLAGS(ch, PRF_NOGIVE), PRF_NOGIVE);
#endif
                    }
                }
            }
        }

    if (!ch)
        return;

    if (ch->carrying)
        for (tobj = ch->carrying; tobj; tobj = onext) {
            onext = tobj->next_content;
            if (is_metall(tobj) && GET_OBJ_REAL_TEMP(tobj) >= 30) {
                dam = calc_warn_dam(ch, get_const_obj_temp(tobj));
                if (dam > 0) {
                    damage_obj(ch, tobj, dam, SPELL_HEAT_METALL, TRUE);
                    if (IS_NPC(ch)) {
                        do_drop(ch, GET_OBJ_PNAME(tobj, 0), SCMD_DROP, 0, 1);
#if 0
                        /* this flag doesn't work for npc, should be done another way */
                        SET_BIT(PRF_FLAGS(ch, PRF_NOGIVE), PRF_NOGIVE);
#endif
                    }
                }
            }
        }

}

void affect_to_object(struct obj_data *obj, struct C_obj_affected_type *af)
{
    struct C_obj_affected_type *affected_alloc;

    CREATE(affected_alloc, struct C_obj_affected_type, 1);

    *affected_alloc = *af;
    affected_alloc->next = obj->C_affected;
    obj->C_affected = affected_alloc;

    extra_modify_object(obj, af->extra, TRUE);
    no_modify_object(obj, af->no_flag, TRUE);
    anti_modify_object(obj, af->anti_flag, TRUE);

    /* if (obj->worn_by)
       affect_total(obj->worn_by); */
}

void affect_from_object(struct obj_data *obj, int type)
{
    struct C_obj_affected_type *af, *next;

    type = find_spell_num(type);
    for (af = obj->C_affected; af; af = next) {
        next = af->next;
        if (af->type == type)
            affect_obj_removed(obj, af);
    }

}

void affect_obj_removed(struct obj_data *obj, struct C_obj_affected_type *af)
{
    int type;
    struct C_obj_affected_type *temp;

    if (obj->C_affected == NULL) {
        log("ОШИБКА: affect_obj_remove(%s) не имеет аффектов...", GET_OBJ_PNAME(obj, 0));
        return;
    }

    type = af->type;

    extra_modify_object(obj, af->extra, FALSE);
    no_modify_object(obj, af->no_flag, FALSE);
    anti_modify_object(obj, af->anti_flag, FALSE);

    REMOVE_FROM_LIST(af, obj->C_affected, next);

    if (obj->worn_by)
        affect_total(obj->worn_by);

    if (obj->worn_by && type != SPELL_DB && af->main)
        if (GET_ID(obj->worn_by) == af->owner || AFF_FLAGGED(obj->worn_by, AFF_DETECT_MAGIC))
            show_spell_off(type, obj->worn_by, obj);

    free(af);
}


void affect_join_object(struct obj_data *obj, struct C_obj_affected_type *af,
                        bool add_dur, bool avg_dur, bool add_mod, bool avg_mod)
{
    struct C_obj_affected_type *hjp;
    bool found = FALSE;

    for (hjp = obj->C_affected; !found && hjp && af->location; hjp = hjp->next) {
        if ((hjp->type == af->type) && (hjp->location == af->location)) {
            if (add_dur)
                af->duration += hjp->duration;
            if (avg_dur)
                af->duration /= 2;
            if (add_mod)
                af->modifier += hjp->modifier;
            if (avg_mod)
                af->modifier /= 2;
            affect_obj_removed(obj, hjp);
            affect_to_object(obj, af);
            found = TRUE;
        }
    }

    if (!found)
        affect_to_object(obj, af);
}

void affect_join_fspell_object(struct obj_data *obj, struct C_obj_affected_type *af)
{
    struct C_obj_affected_type *hjp;
    bool found = FALSE;

    for (hjp = obj->C_affected; !found && hjp; hjp = hjp->next) {
        if ((hjp->type == af->type) && (hjp->location == af->location)) {
            if (hjp->modifier < af->modifier)
                hjp->modifier = af->modifier;
            if (hjp->duration < af->duration)
                hjp->duration = af->duration;
            found = TRUE;
        }
    }
    if (!found) {
        affect_to_object(obj, af);
    }
}

int affected_object_by_spell(struct obj_data *obj, int type)
{
    struct C_obj_affected_type *hjp;

    type = find_spell_num(type);

    for (hjp = obj->C_affected; hjp; hjp = hjp->next)
        if (hjp->type == type) {
            if (hjp->modifier > 0)
                return (hjp->modifier);
            else
                return (TRUE);
        }

    return (FALSE);
}

void extra_modify_object(struct obj_data *obj, bitvector_t bitv, bool add)
{
    if (add)
        SET_BIT(GET_OBJ_EXTRA(obj, bitv), bitv);
    else
        REMOVE_BIT(GET_OBJ_EXTRA(obj, bitv), bitv);
}

void no_modify_object(struct obj_data *obj, bitvector_t bitv, bool add)
{
    if (add)
        SET_BIT(_GET_OBJ_NO(obj, bitv), bitv);
    else
        REMOVE_BIT(_GET_OBJ_NO(obj, bitv), bitv);
}

void anti_modify_object(struct obj_data *obj, bitvector_t bitv, bool add)
{
    if (add)
        SET_BIT(_GET_OBJ_ANTI(obj, bitv), bitv);
    else
        REMOVE_BIT(_GET_OBJ_ANTI(obj, bitv), bitv);
}

/* Эффекты для локации */
void affect_to_room(struct room_data *room, struct room_affect_data *af)
{
    struct room_affect_data *affected_alloc;

    CREATE(affected_alloc, struct room_affect_data, 1);

    *affected_alloc = *af;
    affected_alloc->next = room->affects;
    room->affects = affected_alloc;

}

void affect_from_room(struct room_data *room, int type)
{
    struct room_affect_data *af, *next;

    type = find_spell_num(type);
    for (af = room->affects; af; af = next) {
        next = af->next;
        if (af->type == type)
            affect_room_removed(room, af);
    }

}

void affect_room_removed(struct room_data *room, struct room_affect_data *af)
{
    int type;
    struct room_affect_data *temp;

    if (room->affects == NULL) {
        log("ОШИБКА: affect_room_remove(%d) не имеет аффектов...", room->number);
        return;
    }

    type = af->type;


    if (af->bitvector == ROOM_AFF_PSPHERE) {
        act("Призматическая сфера рассыпалась на мелкие кусочки.", FALSE, room->people, 0, 0,
            TO_CHAR);
        act("Призматическая сфера рассыпалась на мелкие кусочки.", FALSE, room->people, 0, 0,
            TO_ROOM);
    }

    REMOVE_FROM_LIST(af, room->affects, next);
    free(af);

}


void affect_join_room(struct room_data *room, struct room_affect_data *af,
                      bool add_dur, bool avg_dur, bool add_mod, bool avg_mod)
{
    struct room_affect_data *hjp;
    bool found = FALSE;

    for (hjp = room->affects; !found && hjp; hjp = hjp->next) {
        if ((hjp->type == af->type)) {
            if (add_dur)
                af->duration += hjp->duration;
            if (avg_dur)
                af->duration /= 2;
            if (add_mod)
                af->modifier += hjp->modifier;
            if (avg_mod)
                af->modifier /= 2;
            affect_room_removed(room, hjp);
            affect_to_room(room, af);
            found = TRUE;
        }
    }

    if (!found)
        affect_to_room(room, af);
}

void affect_join_fspell_room(struct room_data *room, struct room_affect_data *af)
{
    struct room_affect_data *hjp;
    bool found = FALSE;

    for (hjp = room->affects; !found && hjp; hjp = hjp->next) {
        if ((hjp->type == af->type) && (hjp->bitvector == af->bitvector)) {
            if (hjp->modifier < af->modifier)
                hjp->modifier = af->modifier;
            if (hjp->duration < af->duration)
                hjp->duration = af->duration;
            found = TRUE;
        }
    }
    if (!found) {
        affect_to_room(room, af);
    }
}

int affected_room_by_spell(struct room_data *room, int type)
{
    struct room_affect_data *hjp;

    type = find_spell_num(type);
    for (hjp = room->affects; hjp; hjp = hjp->next)
        if (hjp->type == type) {
            if (hjp->modifier > 0)
                return (hjp->modifier);
            else
                return (TRUE);
        }

    return (FALSE);
}

int affected_room_by_spell_real(struct room_data *room, int type)
{
    struct room_affect_data *hjp;

    for (hjp = room->affects; hjp; hjp = hjp->next)
        if (hjp->type == type) {
            if (hjp->modifier > 0)
                return (hjp->modifier);
            else
                return (TRUE);
        }

    return (FALSE);
}

long get_spell_onwer_by_bitvector(struct room_data *room, bitvector_t bitv)
{
    struct room_affect_data *hjp;

    for (hjp = room->affects; hjp; hjp = hjp->next)
        if (hjp->bitvector == bitv)
            return (hjp->owner);

    return (-1);
}

int affected_room_by_bitvector(struct room_data *room, bitvector_t bitv)
{
    struct room_affect_data *hjp;

    for (hjp = room->affects; hjp; hjp = hjp->next)
        if (hjp->bitvector == bitv)
            return (hjp->modifier);

    return (FALSE);
}

void check_plague(struct obj_data *obj)
{
    struct char_data *tch;
    struct C_obj_affected_type *af;
    int modifier;

    if (IN_ROOM(obj) == NOWHERE)
        return;

    if (ROOM_FLAGGED(IN_ROOM(obj), ROOM_PEACEFUL))      //заражение в мирке невозможно
        return;

    for (af = obj->C_affected; af; af = af->next)
        if (af->bitvector == AFF_PLAGUE) {
            modifier = af->modifier;

            for (tch = world[IN_ROOM(obj)].people; tch; tch = tch->next_in_room) {
                if (affected_by_spell(tch, SPELL_PLAGUE) || affected_by_spell(tch, SPELL_PREPLAGUE))
                    continue;
                if (IS_SOUL(tch))
                    continue;
                if (IS_NPC(tch))
                    continue;
                if (PRF_FLAGGED(tch, PRF_CODERINFO))
                    send_to_charf(tch, "Чума от %s.\r\n", GET_OBJ_PNAME(obj, 0));
                if (!general_savingthrow_3(tch, SAV_FORT, modifier / 12))
                    preplague_to_char(tch, tch, modifier);
            }
            break;
        }
}

void check_plague(struct char_data *ch)
{
    struct char_data *tch, *owner = NULL;
    int modifier, oid;
    int spell_plague = find_spell_num(SPELL_PLAGUE);
    int spell_preplague = find_spell_num(SPELL_PREPLAGUE);

    if (IN_ROOM(ch) == NOWHERE)
        return;

    if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL))       //заражение в мирке невозможно
        return;

    if (!IS_NPC(ch) && affected_by_spell_real(ch, spell_preplague)) {   //зараза от инкубационного PC
        modifier = affected_by_spell_real(ch, spell_preplague);
        oid = get_spell_owner_real(ch, spell_preplague);
        owner = get_char_by_id(oid);
        for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
            if (tch == ch)
                continue;
            if (IS_NPC(tch))
                continue;
            if (IS_SOUL(tch))
                continue;
            if (owner && same_group(tch, owner))
                continue;
            if (affected_by_spell_real(tch, spell_plague) ||
                affected_by_spell_real(tch, spell_preplague))
                continue;
            if (PRF_FLAGGED(tch, PRF_CODERINFO))
                send_to_charf(tch, "Чума от %s.\r\n", GET_NAME(ch));
            if (!general_savingthrow_3(tch, SAV_FORT, modifier / 30))
                preplague_to_char(ch, tch, modifier);
        }
    } else if (!IS_NPC(ch) && affected_by_spell(ch, SPELL_PLAGUE)) {    //зараза от PC
        modifier = affected_by_spell(ch, SPELL_PLAGUE);
        oid = get_spell_owner(ch, SPELL_PLAGUE);
        owner = get_char_by_id(oid);
        for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
            if (tch == ch)
                continue;
            if (IS_NPC(tch))
                continue;
            if (IS_SOUL(tch))
                continue;
            if (owner && same_group(tch, owner))
                continue;
            if (affected_by_spell(tch, SPELL_PLAGUE) || affected_by_spell(tch, SPELL_PREPLAGUE))
                continue;
            if (PRF_FLAGGED(tch, PRF_CODERINFO))
                send_to_charf(tch, "Чума от %s.\r\n", GET_NAME(ch));
            if (!general_savingthrow_3(tch, SAV_FORT, modifier / 10))
                preplague_to_char(ch, tch, modifier);
        }
    } else if (IS_NPC(ch) && NPC_FLAGGED(ch, NPC_PLAGUE)) {     //зараза от NPC
        modifier = GET_LEVEL(ch) * 5;
        for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
            if (tch == ch)
                continue;
            if (IS_NPC(tch))
                continue;
            if (IS_SOUL(tch))
                continue;
            if (affected_by_spell(tch, SPELL_PLAGUE) || affected_by_spell(tch, SPELL_PREPLAGUE))
                continue;
            if (PRF_FLAGGED(tch, PRF_CODERINFO))
                send_to_charf(tch, "Чума от %s.\r\n", GET_NAME(ch));
            if (!general_savingthrow_3(tch, SAV_FORT, modifier / 10))
                preplague_to_char(ch, tch, modifier);
        }
    }
}


static const char *plague_mess[][2] = {
    {"У Вас резко поднялась температура. Начался жар.",
     "Лицо $n1 покрылось испариной, $m явно очень плохо."},
    {"Поры Вашего тела начали кровоточить.",
     "Поры тела $n1 начали кровоточить. Похоже, $e умирает."},
    {"Ваше тело покрылось кровоточащими язвами.", "Тело $n1 покрылось кровоточащими язвами."}
};

void illness_update(void)
{
    struct char_data *i, *next_char, *ch, *tch, *tch_next;
    struct obj_data *j, *next_thing;
    int x, y, z, w, level, spellnum, wrld, dam = 0;
    int spellnum_pfog = find_spell_num(SPELL_POISON_FOG);
    int spellnum_cure = find_spell_num(SPELL_CURE_MASS);
    long id;

    // Персонажи
    for (i = character_list; i; i = next_char) {
        int dam = 0;

        next_char = i->next;
        if (!GET_POS(i))
            continue;
        if (IS_SOUL(i))
            continue;
        if (IS_UNDEAD(i) || IS_CONSTRUCTION(i))
            continue;
        //чума
        check_plague(i);
        //кровотечение
        if (FIGHTING(i) == NULL) {
            spellnum = find_spell_num(SPELL_HAEMORRAGIA);
            dam = 0;
            if ((level = affected_by_spell_real(i, spellnum))) {
                struct P_damage damage;
                struct P_message message;

                if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
                    sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d",
                           &x, &y, &z, &w);
                    dam = SPLDAMAGE;
                }

                damage.valid = true;
                damage.type = Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt();
                damage.dam = dam;
                damage.check_ac = A_POWER;
                damage.armor = TRUE;
                GetSpellMessage(spellnum, message);
                long id = get_spell_owner_real(i, spellnum);
                struct char_data *own = get_char_by_id(id);

                if (own)
                    _damage(own, i, 0, 0, A_POWER, FALSE, damage, message);
                else
                    _damage(i, i, 0, 0, A_POWER, FALSE, damage, message);
            }
        }
        //яд
        spellnum = find_spell_num(SPELL_POISON);
        dam = 0;
        if ((level = affected_by_spell_real(i, spellnum))) {
            struct P_damage damage;
            struct P_message message;

            if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
                sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x,
                       &y, &z, &w);
                dam = SPLDAMAGE;
            }

            damage.valid = true;
            damage.type = Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt();
            damage.dam = dam;
            damage.check_ac = A_POWER;
            GetSpellMessage(find_spell_num(SPELL_POISON), message);
            long id = get_spell_owner_real(i, spellnum);
            struct char_data *own = get_char_by_id(id);

            if (own)
                _damage(own, i, 0, 0, A_POWER, FALSE, damage, message);
            else
                _damage(i, i, 0, 0, A_POWER, FALSE, damage, message);
        }
        //чума
        spellnum = find_spell_num(SPELL_PLAGUE);
        dam = 0;
        if ((level = affected_by_spell_real(i, spellnum))) {    // повреждения от чумы
            struct P_damage damage;
            struct P_message message;

            if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
                sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x,
                       &y, &z, &w);
                dam = SPLDAMAGE;
            }

            int mess = number(0, 2);

            act(plague_mess[mess][0], FALSE, i, 0, 0, TO_CHAR);
            act(plague_mess[mess][1], FALSE, i, 0, 0, TO_ROOM);

            damage.valid = true;
            damage.type = Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt();
            damage.dam = dam;
            damage.check_ac = A_POWER;
            //GetSpellMessage(find_spell_num(SPELL_PLAGUE), message);
            _damage(i, i, 0, 0, A_POWER, FALSE, damage, message);
        }
        update_pos(i);
    }

    /* Расчет предметов */
    for (j = object_list; j; j = next_thing) {
        next_thing = j->next;
        //чума
        check_plague(j);
    }


    for (wrld = 0; wrld < top_of_world; wrld++) {
        level = 0;
        if ((level = affected_room_by_bitvector(&world[wrld], ROOM_AFF_POISON_FOG))) {
            if (Spl.GetItem(spellnum_pfog)->GetItem(SPL_DAMAGE))
                sscanf(Spl.GetItem(spellnum_pfog)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d",
                       &x, &y, &z, &w);
            id = get_spell_onwer_by_bitvector(&world[wrld], ROOM_AFF_POISON_FOG);

            if ((ch = get_char_by_id(id)))
                for (tch = world[wrld].people; tch; tch = tch_next) {
                    tch_next = tch->next_in_room;
                    dam = SPLDAMAGE;
                    if (IS_UNDEAD(tch) || IS_CONSTRUCTION(tch))
                        continue;
                    if (same_group(ch, tch) || ch == tch)
                        continue;

                    if (!may_kill_here(ch, tch, FALSE))
                        continue;

                    mag_damage(spellnum_pfog, dam, ch, tch, TRUE,
                               Spl.GetItem(spellnum_pfog)->GetItem(SPL_TDAMAGE)->GetInt(), FALSE);
                    update_pos(tch);

                }
        }
        level = 0;
        if ((level = affected_room_by_bitvector(&world[wrld], ROOM_AFF_CURE))) {
            float zz;
            bool result;

            if (Spl.GetItem(spellnum_cure)->GetItem(SPL_DAMAGE))
                sscanf(Spl.GetItem(spellnum_cure)->GetItem(SPL_DAMAGE)->GetString(), "%dd%d+%f", &x,
                       &y, &zz);

            id = get_spell_onwer_by_bitvector(&world[wrld], ROOM_AFF_CURE);
            if ((ch = get_char_by_id(id)))
                for (tch = world[wrld].people; tch; tch = tch->next_in_room) {
                    result = FALSE;
                    dam = dice(x, y) + (int) ((float) level / (float) zz);
                    dam += GET_INC_MAGIC(ch, 0);
                    if (IS_UNDEAD(tch) || IS_CONSTRUCTION(tch))
                        continue;
                    if (same_group(ch, tch) || ch == tch) {
                        if (GET_HIT(tch) < GET_REAL_MAX_HIT(tch)) {
                            GET_HIT(tch) = MIN(GET_HIT(tch) + dam, GET_REAL_MAX_HIT(tch));
                            result = TRUE;
                        }
                    }
                    if (result) {
                        act_affect_mess(spellnum_cure, ch, tch, TRUE, TYPE_MESS_HIT);
                        update_pos(tch);
                    }
                }
        }

    }

}

/* ************************************************************************
*   File: spec_procs.c                                  Part of CircleMUD *
*  Usage: implementation of special procedures for mobiles/objects/rooms  *
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
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "constants.h"
#include "pk.h"
#include "xspells.h"
#include "xenchant.h"
#include "xboot.h"
#include "planescape.h"
#include "dlfileop.h"
#include "mudfile.h"

#include "wrapperbase.h"
#include "register-impl.h"

/* extern functions */
ACMD(do_drop);
ACMD(do_hold);
ACMD(do_remove);
ACMD(do_wield);
ACMD(do_wear);
ACMD(do_get);
ACMD(do_stand);

/* special functions */
SPECIAL(guild);
SPECIAL(bank);
SPECIAL(shoper);
SPECIAL(shop_hrs);
SPECIAL(postmaster);
SPECIAL(trainer);

SPECIAL(gen_board);
SPECIAL(scriptboard);

SPECIAL(hotel);
SPECIAL(dump);

#define LEARN_COST(ch) (((level_exp(ch, GET_LEVEL(ch)+1) - level_exp(ch, GET_LEVEL(ch)))*5)/100)

//(((level_exp(GET_LEVEL(ch))+1 - level_exp(GET_LEVEL(ch))) *5)/100)

/* ********************************************************************
*  Special procedures for mobiles                                     *
******************************************************************** */

char *how_good(struct char_data *ch, int percent)
{
    static char out_str[128];

    if (percent < 0)
        strcpy(out_str, "!Ошибка! ");
    else if (percent == 0)
        sprintf(out_str, "не изучено");
    else if (percent <= 10)
        sprintf(out_str, "ужасно");
    else if (percent <= 20)
        sprintf(out_str, "очень плохо");
    else if (percent <= 30)
        sprintf(out_str, "плохо");
    else if (percent <= 40)
        sprintf(out_str, "ниже среднего");
    else if (percent <= 50)
        sprintf(out_str, "средне");
    else if (percent <= 60)
        sprintf(out_str, "выше среднего");
    else if (percent <= 70)
        sprintf(out_str, "хорошо");
    else if (percent <= 80)
        sprintf(out_str, "очень хорошо");
    else if (percent <= 90)
        sprintf(out_str, "превосходно");
    else if (percent <= 100)
        sprintf(out_str, "cовершенно");
    else if (percent <= 110)
        sprintf(out_str, "идеально");
    else if (percent <= 130)
        sprintf(out_str, "сверхестеств");
    else
        sprintf(out_str, "божествено");

    return out_str;
}

char *list_skills(struct char_data *ch, int type)
{
    static char bufwar[MAX_STRING_LENGTH];
    int sortpos, i = 0;
    char *color;
    int count = 0;

    *bufwar = '\0';

    for (sortpos = 1; sortpos <= TOP_SKILL_DEFINE; sortpos++)
        if (GET_SKILL(ch, sortpos)) {
            bool bb = FALSE;

            if (skill_info[sortpos].name.size() == 0 ||
                skill_info[sortpos].name.at(0) == '!' || skill_info[sortpos].skill_type != type)
                continue;

            if (timed_by_skill(ch, sortpos))
                color = (char *) CCIBLU(ch, C_NRM);
            else if (GET_SKILL(ch, sortpos) < SET_SKILL(ch, sortpos)) {
                color = (char *) CCIBLK(ch, C_NRM);
                bb = TRUE;
            } else
                color = (char *) CCNRM(ch, C_NRM);

            count += sprintf(bufwar + count, "%s%22s &K%-13s%1s&n%s",
                             color,
                             skill_info[sortpos].name.c_str(),
                             how_good(ch, GET_SKILL(ch, sortpos)),
                             (!fmod(GET_SKILL(ch, sortpos), 10) &&
                              (GET_SKILL(ch, sortpos) != 150) && !bb) ? "+" : "",
                             (i % 2) ? "\r\n" : "");
            i++;
        }
    if (count > 0)
        strcat(bufwar, "\r\n");

    return (bufwar);
}


void list_enchants(struct char_data *ch, struct char_data *victim)
{
    char *enchant_name;
    int number, i, enchant_no, count = 0, is_ok = FALSE;
    char buf[MAX_STRING_LENGTH], names[MAX_STRING_LENGTH];

    if (ch == NULL || victim == NULL)
        return;

    number = xEnchant.GetNumberItem();

    for (i = 0; i < number; i++) {
        if (!IS_SET(GET_ENCHANT_TYPE(ch, ENCHANT_NO(i)), SPELL_TEMP | SPELL_KNOW) && !IS_GOD(ch))
            continue;

        enchant_no = ENCHANT_NO(i);
        enchant_name = ENCHANT_NAME(i);

        if (count % 88 < 10) {
            count += sprintf(names + count, "%s%28s", "\r\n", enchant_name);
        } else {
            count += sprintf(names + count, "%s%-28s", "  ", enchant_name);
        }
        is_ok = TRUE;
    }

    sprintf(buf, "%sВы знаете следующие рецепты:%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));

    if (is_ok)
        sprintf(buf + strlen(buf), "%s", names);
    else
        sprintf(buf + strlen(buf), "\r\nВы не знаете ни одного рецепта зачарования.");


    sprintf(buf + strlen(buf), "\r\n");
    page_string(ch->desc, buf, 1);

}

void list_spells(struct char_data *ch, struct char_data *victim, int class_type)
{
    char names[NUM_SPHERE][10][MAX_STRING_LENGTH];
    char *color, *spell_name;
    int number, i, j, spell_no, count[NUM_SPHERE][10], is_ok = FALSE, sphere, splevel = 0;
    char buf[MAX_STRING_LENGTH];

    if (ch == NULL || victim == NULL)
        return;

    if (!IS_GOD(victim) && !IS_NPC(victim) && !IS_MAGIC_USER(victim) && !IS_NECRO(victim)) {
        if (ch == victim)
            send_to_char("Вы ничего не понимаете в магии.\r\n", ch);
        else
            send_to_charf(ch, "%s ничего не понимает в магии.\r\n", GET_NAME(victim));
        return;
    }

    for (i = 0; i < NUM_SPHERE; i++) {
        for (j = 0; j < 10; j++) {
            *names[i][j] = '\0';
            count[i][j] = 0;
        }
    }

    number = Spl.GetNumberItem();

    for (i = 0; i < number; i++) {
        CItem *spell = Spl.GetItem(i);

        spell_no = spell->GetItem(SPL_NUMBER)->GetInt();
        spell_name = spell->GetItem(SPL_NAME)->GetString();
        sphere = spell->GetItem(SPL_SPHERE)->GetInt();

        if (sphere_class[sphere] != class_type)
            continue;

        if (!GET_SPELL_TYPE(victim, spell_no))
            continue;
        if (!spell_name || *spell_name == '!')
            continue;

        if (GET_SPELL_MEM(victim, spell_no))
            color = (char *) CCIBLU(ch, C_NRM);
        else if (GET_MANA(ch) >= GET_MANA_COST(ch, i))
            color = (char *) CCNRM(ch, C_NRM);
        else
            color = (char *) CCINRM(ch, C_NRM);


        if (sphere < 0)
            continue;

        splevel = MAX(0, SPELL_LEVEL(i) / 10);
        if (count[sphere][splevel] % 88 < 10) {
            count[sphere][splevel] += sprintf(names[sphere][splevel] + count[sphere][splevel],
                                              "%s%s%28s%s", color, "\r\n", spell_name, CCNRM(ch,
                                                                                             C_NRM));
        } else {
            count[sphere][splevel] += sprintf(names[sphere][splevel] + count[sphere][splevel],
                                              "%s%s%-28s%s", color, "  ", spell_name, CCNRM(ch,
                                                                                            C_NRM));
        }
        is_ok = TRUE;
    }

    sprintf(buf, "%sВы знаете следующие заклинания:%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));

    if (is_ok) {
        for (i = 0; i < NUM_SPHERE; i++) {
            if (*count[i])
                sprintf(buf + strlen(buf), "\r\n&g%27s:&n", sphere_name[i]);
            for (j = 0; j < 10; j++) {
                if (count[i][j]) {
                    sprintf(buf + strlen(buf), "\r\n&g%26s %d&n", "Круг", j);
                    sprintf(buf + strlen(buf), "%s", names[i][j]);
                }
            }
        }
    } else
        sprintf(buf + strlen(buf), "\r\nВы не знаете ни одного заклинания.");


    sprintf(buf + strlen(buf), "\r\n");
    page_string(ch->desc, buf, 1);

}

void list_prays(struct char_data *ch, struct char_data *victim)
{
    char names[NUM_SPHERE][11][MAX_INPUT_LENGTH];
    char *color, *spell_name;
    int number, i, j, spell_no, count[NUM_SPHERE][11], is_ok = FALSE, sphere, splevel = 0;
    char buf[MAX_STRING_LENGTH];

    if (ch == NULL || victim == NULL)
        return;

    if (!IS_GOD(victim) && !IS_NPC(victim) && !IS_PRIEST(victim)) {
        if (ch == victim)
            send_to_char("Вас не научили молиться.\r\n", ch);
        else
            send_to_charf(ch, "%s не научили молиться.\r\n", GET_NAME(victim));
        return;
    }

    for (i = 0; i < NUM_SPHERE; i++) {
        for (j = 0; j < 11; j++) {
            *names[i][j] = '\0';
            count[i][j] = 0;
        }
    }

    number = Spl.GetNumberItem();

    for (i = 0; i < number; i++) {
        CItem *spell = Spl.GetItem(i);

        spell_no = spell->GetItem(SPL_NUMBER)->GetInt();
        spell_name = spell->GetItem(SPL_NAME)->GetString();
        sphere = spell->GetItem(SPL_SPHERE)->GetInt();

        if (sphere_class[sphere] != CLASS_PRIEST)
            continue;

        if (!GET_SPELL_TYPE(victim, spell_no))
            continue;
        if (!spell_name || *spell_name == '!')
            continue;

        if (GET_SPELL_MEM(victim, spell_no))
            color = (char *) CCIBLU(ch, C_NRM);
        else if (GET_MANA(ch) >= GET_MANA_COST(ch, i))
            color = (char *) CCNRM(ch, C_NRM);
        else
            color = (char *) CCINRM(ch, C_NRM);


        if (sphere < 0)
            continue;

        splevel = MAX(0, SPELL_LEVEL(i) / 10);
        sprintf(names[sphere][splevel] + strlen(names[sphere][splevel]),
                "%s%28s%s", color, spell_name, CCNRM(ch, C_NRM));
        is_ok = TRUE;
    }

    sprintf(buf, "%sВы знаете следующие молитвы:%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));

    if (is_ok) {
        for (i = 0; i < NUM_SPHERE; i++) {
            int line = 0;

            if (*names[i][0])
                sprintf(buf + strlen(buf), "\r\n&g%27s:&n\r\n", sphere_name[i]);
            for (j = 0; j < 11; j++) {
                if (*names[i][j]) {
                    if (line % 2)
                        sprintf(buf + strlen(buf), "\r\n%s", names[i][j]);
                    else
                        sprintf(buf + strlen(buf), "%s", names[i][j]);

                    line++;
                }
            }
        }
    } else
        sprintf(buf + strlen(buf), "\r\nВы не знаете ни одной молитвы.");


    sprintf(buf + strlen(buf), "\r\n");
    page_string(ch->desc, buf, 1);

}

struct shop_horse_data {
    int shopper;                /* продавец */
    int vmob;                   /* vnum лошади */
    int price;                  /* цена лошади */
};

struct shop_horse_data *shop_horse_info = NULL;

int ExtraSkillValues[] = { 0, 30, 50, 80, 100, 120 };


// FUNCTION IS ADDED BY HMEPAS
/* Названия уровня (новичок, мастер, etc)
         по его номеру.
         mode = 1 -- уменительный падеж (кто?)
   mode = 2 -- родительный падеж (кого?)
*/
const char *extra_level_name(int level, int mode)
{
    if (level < 0 || level > ExtraSkillValuesCol - 1) {
        log("ERROR: function extra_level_name(), level: %i", level);
        return NULL;
    }

    switch (mode) {
        case 1:
            return ExtraSkillNames[level];
        case 2:
            return ExtraSkillNames2[level];
        default:
            log("ERROR: function extra_level_name(), mode: %i", mode);
            return NULL;
    }
}

/* По значению скилла и уровня мастерства
   возвращает следующий допустимый для
   этого скилла уровень мастерства        */
int get_next_extra_level_by_skill(int skill_value, int ext_skill_value)
{
    int i;

    if (skill_value <= 0 || ext_skill_value == 255)
        return -1;

    for (i = ExtraSkillValuesCol - 1; i > 0; i--)
        if (skill_value >= ExtraSkillValues[i] && ext_skill_value == i - 1)
            return i;
    return -1;
}

static void mprog_train_level(struct char_data *ch, struct char_data *teacher, int level)
{
	FENIA_VOID_CALL(teacher, "TrainLevel", "Ci", ch, level);
    FENIA_PROTO_VOID_CALL(teacher->npc(), "TrainLevel", "CCi", teacher, ch, level);
}

#define SCMD_LEARN 1
#define SCMD_LEV 2

// Обучаем профессиям
SPECIAL(trainer)
{
    int skill_no = -1, command = 0, found = FALSE, found2 = FALSE, i, j;
    struct char_data *victim = (struct char_data *) me;
    int next_level = 0;
    int next_this_level = 0;
    char buf[MAX_STRING_LENGTH];

    *buf = '\0';

    if (IS_NPC(ch))
        return (0);

    if (CMD_IS("тренировать") || CMD_IS("practice"))
        command = SCMD_LEV;
    else if (CMD_IS("учить") || CMD_IS("learn"))
        command = SCMD_LEARN;
    else
        return (0);

    skip_spaces(&argument);

    switch (command) {
        case SCMD_LEARN:
            if (IS_PRIEST(ch) && IS_PRIEST(victim) && GET_GODS(ch) != GET_GODS(victim)) {
                return (0);
                /*
                   sprintf(buf,"Для обучения пройдите в свою гильдию.");
                   act(buf,FALSE, ch, 0, victim, TO_CHAR);
                   return(1); */
            }
            if (!*argument) {
                sprintf(buf + strlen(buf), "Вы можете выучить следующее:\r\n");
                found = FALSE;
                int wclass = check_class(ch, CLASS_WARRIOR);

                for (i = 1; i <= MAX_SKILLS; i++) {
                    found2 = FALSE;
                    if (GET_SKILL(victim, i)) {
                        if ((i == SKILL_COURAGE) && !IS_ORC(ch))
                            continue;

                        if ((i == SKILL_RUNUP) && (!IS_BARIAUR(ch) || !wclass))
                            continue;

                        if ((i == SKILL_SATTACK) && !IS_GNOME(ch))
                            continue;

                        if (i == SKILL_HOLYLIGHT && !IS_TIEFLING(ch) && !IS_AASIMAR(ch))
                            continue;

                        for (j = 0; j < NUM_CLASSES; j++) {

                            if (skill_info[i].learn_level[j][(int) GET_GODS(ch)] &&
                                !fmod(SET_SKILL(ch, i), 10) &&
                                check_class(ch,
                                            j) >= skill_info[i].learn_level[j][(int) GET_GODS(ch)])
                                found2 = TRUE;
                        }

                        if (IS_GNOME(ch) && i == SKILL_AXES && !fmod(SET_SKILL(ch, i), 10))
                            found2 = TRUE;

                        if (found2 &&
                            (calc_need_improove(ch, get_skill_class_level(ch, skill_no)) >=
                             SET_SKILL(ch, skill_no))) {
                            sprintf(buf + strlen(buf), "умение %s%s%s\r\n", CCICYN(ch, C_NRM),
                                    skill_name(i), CCNRM(ch, C_NRM));
                            found = TRUE;
                        }
                    }
                }
                if (!found) {
                    act("$N сказал$G: 'Тебе нечему тут учиться.'", FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else {
                    if (GET_LEVEL(ch) > 1)
                        sprintf(buf + strlen(buf),
                                "\r\nВам нужно %s%d%s очков опыта на каждое умение.\r\n", CCIGRN(ch,
                                                                                                 C_NRM),
                                LEARN_COST(ch), CCNRM(ch, C_NRM));
                    else
                        sprintf(buf + strlen(buf), "\r\n");

                    sprintf(buf + strlen(buf),
                            "Наберите %sучить название умения%s, чтобы выучить нужное умение.\r\n",
                            CCICYN(ch, C_NRM), CCNRM(ch, C_NRM));
                    send_to_char(buf, ch);
                    return (1);
                }
            }                   /* КОНЕЦ ПОКАЗА СПИСКА УМЕНИЙ */
            found2 = FALSE;
            for (i = 1; i <= MAX_SKILLS; i++)
                if (SET_SKILL(victim, i) && isname(argument, skill_info[i].name.c_str())) {
                    skill_no = i;
                    if (SET_SKILL(victim, skill_no)) {
                        if ((skill_no == SKILL_COURAGE) && !IS_ORC(ch))
                            found2 = FALSE;
                        else if ((skill_no == SKILL_SATTACK) && !IS_GNOME(ch))
                            found2 = FALSE;
                        else if (skill_no == SKILL_HOLYLIGHT && !IS_TIEFLING(ch) && !IS_AASIMAR(ch))
                            found2 = FALSE;
                        if (skill_no == SKILL_AXES && IS_GNOME(ch))
                            found2 = TRUE;
                        else
                            for (j = 0; j < NUM_CLASSES; j++)
                                if (skill_info[skill_no].learn_level[j][(int) GET_GODS(ch)] &&
                                    check_class(ch,
                                                j) >=
                                    skill_info[skill_no].learn_level[j][(int) GET_GODS(ch)])
                                    found2 = TRUE;
                    }
                    if (!found2) {
                        send_to_char("Вы не удовлетворяете условиям обучения.\r\n", ch);
                        return (1);
                    }

                    if (GET_EXP(ch) < LEARN_COST(ch) && GET_LEVEL(ch) > 1) {
                        sprintf(buf, "Вам не хватает %ld очков опыта.",
                                LEARN_COST(ch) - GET_EXP(ch));
                        act(buf, FALSE, ch, 0, victim, TO_CHAR);
                        return (1);
                    }

                    if (get_levelexp(ch, GET_LEVEL(ch), 1) >= (GET_EXP(ch) - LEARN_COST(ch))
                        && GET_LEVEL(ch) > 1) {
                        sprintf(buf, "Вы рискуете потерять слишком много опыта.");
                        act(buf, FALSE, ch, 0, victim, TO_CHAR);
                        return (1);
                    }

                    if (SET_SKILL(ch, skill_no)) {
                        if (SET_SKILL(ch, skill_no) >= GET_SKILL(victim, skill_no)) {
                            send_to_charf(ch,
                                          "С Вашими знаниями Вы сами могли бы работать тут учителем.\r\n");
                            return (1);
                        }
                        if (fmod(SET_SKILL(ch, skill_no), 10)) {
                            act("Вам не хватает практики в этом умении.", FALSE, ch, 0, victim,
                                TO_CHAR);
                        } else {
                            sprintf(buf, "$N повысил$G Ваше умение %s%s%s",
                                    CCICYN(ch, C_NRM), skill_name(skill_no), CCNRM(ch, C_NRM));
                            act(buf, FALSE, ch, 0, victim, TO_CHAR);
                            act("$n прошел$g курс повышения умения.", TRUE, ch, 0, 0, TO_ROOM);
                            SET_SKILL(ch, skill_no)++;
                            if (GET_LEVEL(ch) > 1)
                                GET_EXP(ch) -= LEARN_COST(ch);
                        }
                    } else {
                        sprintf(buf, "$N научил$G Вас умению %s%s%s",
                                CCICYN(ch, C_NRM), skill_name(skill_no), CCNRM(ch, C_NRM));
                        act(buf, FALSE, ch, 0, victim, TO_CHAR);
                        act("$n прошел$g курс обучения новому умению.", TRUE, ch, 0, 0, TO_ROOM);
                        SET_SKILL(ch, skill_no) = 5;
                        SET_SKILL_LEVEL(ch, skill_no, 0);       //ставим эктрафлаг на уровень BASIC
                        if (GET_LEVEL(ch) > 1)
                            GET_EXP(ch) -= LEARN_COST(ch);
                    }
                    return (1);
                }
            if (!found2) {
                act("$N сказал$G: 'Я не знаю такого умения.'", FALSE, ch, 0, victim, TO_CHAR);
                return (1);
            }
            /* УЧИТЬ */
            break;
        case SCMD_LEV:
            //send_to_charf(ch,"Ваш gods %s, учитель gods %s\r\n",gods_name[(int)GET_GODS(ch)],gods_name[(int)GET_GODS(victim)]);
            if (IS_NECRO(victim) && !IS_EVILS(ch)) {
                act("Вам нечему здесь тренироваться.", FALSE, ch, 0, 0, TO_CHAR);
                return (1);
            }

            if (IS_PRIEST(victim) && check_class(ch, CLASS_NECRO)) {
                act("Вам нечему здесь тренироваться.", FALSE, ch, 0, 0, TO_CHAR);
                return (1);
            }

            if ((IS_PRIEST(victim) && GET_GODS(ch) != GET_GODS(victim)))
//       || (!IS_PRIEST(ch) && IS_PRIEST(victim)))
            {
                return (0);
                /*
                   sprintf(buf,"Для тренировки пройдите в свою гильдию.");
                   act(buf,FALSE, ch, 0, victim, TO_CHAR);
                   return(1); */
            }

            /*   if (IS_PRIEST(ch) && !IS_PRIEST(victim))
               {
               sprintf(buf,"Для тренировки пройдите в свою гильдию.");
               act(buf,FALSE, ch, 0, victim, TO_CHAR);
               return(1);
               } */

            next_level = GET_LEVEL(ch) + 1;
            next_this_level = check_class(ch, GET_CLASS(victim));
            next_this_level++;

            if (!*argument) {
                if (next_level >= LVL_IMMORT) {
                    sprintf(buf, "Вы достигли своего максимума в развитии.");
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else if (next_this_level > GET_LEVEL(victim)) {
                    sprintf(buf, "Вы не можете больше здесь тренироваться.");
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else if (next_this_level >= LVL_IMMORT) {
                    sprintf(buf, "Вы достигли максимального развития в профессии %s%sа%s.",
                            CCICYN(ch, C_NRM),
                            class_name[(int) GET_CLASS(victim)], CCNRM(ch, C_NRM));
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else if (IS_PRIEST(ch) && IS_PRIEST(victim) && (GET_GODS(ch) != GET_GODS(victim))) {
                    /*sprintf(buf,"Для тренировки пройдите в свою гильдию.");
                       act(buf,FALSE, ch, 0, victim, TO_CHAR); */
                    return (0);
                } else {
                    sprintf(buf, "Вы можете тренироваться:\r\n"
                            "профессия %s%s%s %dго уровня\r\n",
                            CCICYN(ch, C_NRM),
                            class_name[(int) GET_CLASS(victim)], CCNRM(ch, C_NRM), next_this_level);
                    sprintf(buf + strlen(buf),
                            "\r\nВам нужно %s%ld%s очков опыта, для тренировки.\r\n", CCIGRN(ch,
                                                                                             C_NRM),
                            get_levelexp(ch, next_level, 1), CCNRM(ch, C_NRM));
                    sprintf(buf + strlen(buf),
                            "Наберите %sтренировать уровень%s, чтобы начать тренировку.", CCICYN(ch,
                                                                                                 C_NRM),
                            CCNRM(ch, C_NRM));
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                }
            }

            if (!strn_cmp(argument, "уровень", strlen(argument)) ||
                !strn_cmp(argument, "level", strlen(argument))) {
                if (next_level >= LVL_IMMORT) {
                    sprintf(buf, "Вы достигли своего максимума в развитии.");
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else if (next_this_level > GET_LEVEL(victim)) {
                    sprintf(buf, "Вы не можете больше здесь тренироваться.");
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else if (get_dsu_exp(ch) > 0) {
                    // не хватает опыта
                    sprintf(buf, "Вам не хватает %ld опыта.", get_dsu_exp(ch));
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else {
                    if (!GET_LEVEL(ch)) {
                        GET_FVCLASS(ch) = GET_CLASS(victim);
                        GET_HLEVEL(ch, 1) = GET_CLASS(victim);
                    }
                    // учим
                    if (!IS_MAX_EXP(ch)) {
                        GET_LEVEL(ch) = next_level;
                        GET_HLEVEL(ch, next_level) = GET_CLASS(victim);
                    }

                    GET_CLASS(ch) = GET_CLASS(victim);

                    if (!IS_MAX_EXP(ch))
                        add_class(ch, GET_CLASS(victim), next_this_level, 0);
                    else
                        add_class(ch, GET_CLASS(victim), next_this_level, 1);

                    advance_level(ch);
                    sprintf(buf, "Вы стали %sом %dго уровня.",
                            class_name[(int) GET_CLASS(victim)], next_this_level);
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);

                    sprintf(buf, "$n повысил$g свою квалификацию в профессии %sа.",
                            class_name[(int) GET_CLASS(victim)]);
					mprog_train_level(ch, victim, GET_LEVEL(ch));
                    act(buf, TRUE, ch, 0, victim, TO_ROOM);
                    if (GET_LEVEL(ch) == LVL_ROLL && !PRF_FLAGGED(ch, PRF_ROLL)) {
                        SET_BIT(PRF_FLAGS(ch, PRF_ROLL), PRF_ROLL);
                        roll_stat(ch);
                    }
                    return (1);
                }
            }
            sprintf(buf, "$N сказал$G: 'Что-то я не пойму чего ты от меня хочешь?'");
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
            return (1);
            break;
    }

    return (0);

}

int GUILDS_MONO_USED = 0;
int GUILDS_POLY_USED = 0;
int SHOP_HRS_USED = 0;


#define SCMD_LIST 0
#define SCMD_BUY  1
#define SCMD_SELL 2

void sell_horse(struct char_data *ch, struct char_data *shoper, char *argument)
{
    int price, i;
    struct char_data *horse = NULL;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (*arg) {
        if (has_horse(ch, FALSE)) {
            horse = get_horse(ch);
            REMOVE_BIT(AFF_FLAGS(horse, AFF_HORSE), AFF_HORSE);
        }
        horse = get_char_vis(ch, arg, FIND_CHAR_ROOM);
    } else
        horse = get_horse(ch);

    if (horse == NULL) {
        act("$N сказал$G Вам: 'У Вас нет ездового животного.'", FALSE, ch, 0, shoper, TO_CHAR);
        return;
    }

    if (on_horse(ch)) {
        act("$N сказал$G Вам: 'Вам необходимо спешится.'", FALSE, ch, 0, shoper, TO_CHAR);
        return;
    }
    if (IN_ROOM(horse) != IN_ROOM(shoper)) {
        act("$N сказал$G Вам: 'Необходимо чтобы животное было рядом.'", FALSE, ch, 0, shoper,
            TO_CHAR);
        return;
    }

    for (i = 0; i < SHOP_HRS_USED; i++) {
        if (shop_horse_info[i].shopper == GET_MOB_VNUM(shoper)) {
            if (shop_horse_info[i].vmob == GET_MOB_VNUM(horse)) {
                price = shop_horse_info[i].price;
                price = (price * GET_HORSESTATE(horse)) / 200;
                sprintf(buf, "$N купил у Вас %s за %d %s.",
                        GET_PAD(horse, 1), price, desc_count(price, WHAT_MONEYu));
                act(buf, FALSE, ch, 0, shoper, TO_CHAR);
                act("$n продал$g $N3.", FALSE, ch, 0, horse, TO_ROOM);
                GET_GOLD(ch) += price;
                stop_follower(horse, SF_EMPTY);
                extract_char(horse, FALSE);
                *buf = '\0';
                return;
            }
        }
    }
    act("$N сказал$G: 'Я покупаю только тех животных которых продаю.'",
        FALSE, ch, 0, shoper, TO_CHAR);
}

void buy_horse(char *arg, struct char_data *ch, struct char_data *shoper)
{
    int buyvnum = 0, i, index = 0;
    char *tmpname;
    struct char_data *tmp_mob = NULL, *horse = NULL;

    skip_spaces(&arg);

    if (!*arg) {
        send_to_char("Кого покупаем?\r\n", ch);
        return;
    }


    if (is_positive_number(arg))
        for (i = 0; i < SHOP_HRS_USED; i++) {
            if (shop_horse_info[i].shopper == GET_MOB_VNUM(shoper)) {
                index++;
                if (index == atoi(arg)) {
                    if (GET_GOLD(ch) < shop_horse_info[i].price) {
                        send_to_char("У Вас не хватает денег.\r\n", ch);
                        return;
                    }
                    GET_GOLD(ch) -= shop_horse_info[i].price;
                    /* Процедура покупки животного */
                    buyvnum = shop_horse_info[i].vmob;
                }
            }
    } else
        for (i = 0; i < SHOP_HRS_USED; i++) {
            if (shop_horse_info[i].shopper == GET_MOB_VNUM(shoper)) {
                tmpname = alias_vmobile(shop_horse_info[i].vmob);
                if (isname(arg, tmpname)) {
                    if (GET_GOLD(ch) < shop_horse_info[i].price) {
                        send_to_char("У Вас не хватает денег.\r\n", ch);
                        return;
                    }
                    GET_GOLD(ch) -= shop_horse_info[i].price;
                    /* Процедура покупки животного */
                    buyvnum = shop_horse_info[i].vmob;
                }
            }
        }

    if (!(tmp_mob = read_mobile(buyvnum, VIRTUAL))) {
        act("$N сказал$G: 'У меня нет такого животного.'", FALSE, ch, 0, shoper, TO_CHAR);
        return;
    }

    if (has_horse(ch, FALSE)) {
        horse = get_horse(ch);
        REMOVE_BIT(AFF_FLAGS(horse, AFF_HORSE), AFF_HORSE);
        //stop_follower(horse,SF_EMPTY);
        //extract_char(horse,FALSE);
    }

    act("$N теперь Ваш$G.", FALSE, ch, 0, tmp_mob, TO_CHAR);
    act("$n купил$g $N3.", FALSE, ch, 0, tmp_mob, TO_ROOM);
    make_horse(tmp_mob, ch);
    char_to_room(tmp_mob, IN_ROOM(ch));


}

SPECIAL(shop_hrs)
{
    int command = 0, i, found = 0, index = 1;
    struct char_data *shoper = (struct char_data *) me;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return (0);

    if (CMD_IS("список") || CMD_IS("list"))
        command = SCMD_LIST;
    else if (CMD_IS("купить") || CMD_IS("buy"))
        command = SCMD_BUY;
    else if (CMD_IS("продать") || CMD_IS("sell"))
        command = SCMD_SELL;
    else
        return (0);

    switch (command) {
        case SCMD_LIST:
            strcpy(buf, " ##  Животное        (цена)\r\n");
            strcat(buf, "---------------------------\r\n");

            for (i = 0; i < SHOP_HRS_USED; i++) {
                if (shop_horse_info[i].shopper == GET_MOB_VNUM(shoper)) {
                    sprintf(buf + strlen(buf), "%2d. %s &K(&G%d&K %s)&n\r\n",
                            index,
                            CAP(name_vmobile(shop_horse_info[i].vmob)),
                            shop_horse_info[i].price,
                            desc_count(shop_horse_info[i].price, WHAT_MONEYa));
                    index++;
                    found = 1;
                }
            }
            if (!found)
                sprintf(buf, "Животных пока нет.\r\n");
            break;

        case SCMD_BUY:
            buy_horse(argument, ch, shoper);
            break;

        case SCMD_SELL:
            sell_horse(ch, shoper, argument);
            break;
    }

    page_string(ch->desc, buf, 1);
    return (1);
}

void init_hoursekeep(void)
{
    /* Загружаем спиоск магазинов где продают ездовых животных */

    FILE *hrsf;
    char str[MAX_INPUT_LENGTH];
    struct shop_horse_data shop_horse;
    int now_shoper = 0, ints, vnum_h, price_h;

    DLFileRead file(ShareFile(mud->horseShopFile));

    if (!file.open())
        return;

    hrsf = file.getFP();

    while (get_line(hrsf, str)) {
        if (!str[0] || str[0] == ';' || strlen(str) == 0)
            continue;

        if (str[0] == '$') {
            now_shoper = 0;
            continue;
        }

        if (!now_shoper) {
            mob_rnum rnum;

            now_shoper = atoi(str);
            if (now_shoper <= 0 || (rnum = real_mobile(now_shoper)) < 0) {
                log("ERROR: Cann't assign shopper %s in hrs_shop.lst", str);
                return;
            }

            mob_index[rnum].func = shop_hrs;
            continue;
        } else {
            shop_horse.shopper = now_shoper;
            ints = sscanf(str, "%i %i", &vnum_h, &price_h);
            if (real_mobile(vnum_h) < 0) {
                log("ОШИБКА: Нет такого моба в маде, строка %s в hrs_shop.lst", str);
                _exit(1);
            }
            shop_horse.vmob = vnum_h;
            shop_horse.price = price_h;
            log("Добавлен моб #%d в магазин к #%d по цене %d",
                shop_horse.vmob, shop_horse.shopper, shop_horse.price);

            if (!shop_horse_info)
                CREATE(shop_horse_info, struct shop_horse_data, SHOP_HRS_USED + 1);

            else
                RECREATE(shop_horse_info, struct shop_horse_data, SHOP_HRS_USED + 1);

            shop_horse_info[SHOP_HRS_USED++] = shop_horse;
        }
    }
}

//Обучаем умениям
SPECIAL(guild)
{
    int skill_no = -1, command = 0, found = FALSE, found2 = FALSE, i, j;
    struct char_data *victim = (struct char_data *) me;
    int next_level = 0;
    int next_this_level = 0;
    char buf[MAX_STRING_LENGTH];

    *buf = '\0';

    if (IS_NPC(ch))
        return (0);

    if (CMD_IS("учить") || CMD_IS("learn"))
        command = SCMD_LEARN;
    else
        return (0);

    skip_spaces(&argument);

    switch (command) {
        case SCMD_LEARN:
            if (IS_PRIEST(ch) && IS_PRIEST(victim) && GET_GODS(ch) != GET_GODS(victim)) {
                return (0);
                /*
                   sprintf(buf,"Для обучения пройдите в свою гильдию.");
                   act(buf,FALSE, ch, 0, victim, TO_CHAR);
                   return(1); */
            }
            if (!*argument) {
                sprintf(buf + strlen(buf), "Вы можете выучить следующее:\r\n");
                found = FALSE;
                int wclass = check_class(ch, CLASS_WARRIOR);

                for (i = 1; i <= MAX_SKILLS; i++) {
                    found2 = FALSE;
                    if (SET_SKILL(victim, i)) {
                        if ((i == SKILL_COURAGE) && !IS_ORC(ch))
                            continue;

                        if ((i == SKILL_RUNUP) && (!IS_BARIAUR(ch) || !wclass))
                            continue;

                        if ((i == SKILL_SATTACK) && !IS_GNOME(ch))
                            continue;

                        if (i == SKILL_HOLYLIGHT && !IS_TIEFLING(ch) && !IS_AASIMAR(ch))
                            continue;

                        for (j = 0; j < NUM_CLASSES; j++) {

                            if (skill_info[i].learn_level[j][(int) GET_GODS(ch)] &&
                                !fmod(SET_SKILL(ch, i), 10) &&
                                check_class(ch,
                                            j) >= skill_info[i].learn_level[j][(int) GET_GODS(ch)])
                                found2 = TRUE;
                        }

                        if (IS_GNOME(ch) && i == SKILL_AXES && !fmod(SET_SKILL(ch, i), 10))
                            found2 = TRUE;

                        if (found2 &&
                            (calc_need_improove(ch, get_skill_class_level(ch, skill_no)) >=
                             SET_SKILL(ch, skill_no))) {
                            sprintf(buf + strlen(buf), "умение %s%s%s\r\n", CCICYN(ch, C_NRM),
                                    skill_name(i), CCNRM(ch, C_NRM));
                            found = TRUE;
                        }
                    }
                }
                if (!found) {
                    act("$N сказал$G: 'Тебе нечему тут учиться.'", FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else {
                    if (GET_LEVEL(ch) > 1)
                        sprintf(buf + strlen(buf),
                                "\r\nВам нужно %s%d%s очков опыта на каждое умение.\r\n", CCIGRN(ch,
                                                                                                 C_NRM),
                                LEARN_COST(ch), CCNRM(ch, C_NRM));
                    else
                        sprintf(buf + strlen(buf), "\r\n");

                    sprintf(buf + strlen(buf),
                            "Наберите %sучить название умения%s, чтобы выучить нужное умение.\r\n",
                            CCICYN(ch, C_NRM), CCNRM(ch, C_NRM));
                    send_to_char(buf, ch);
                    return (1);
                }
            }                   /* КОНЕЦ ПОКАЗА СПИСКА УМЕНИЙ */
            found2 = FALSE;
            for (i = 1; i <= MAX_SKILLS; i++)
                if (SET_SKILL(victim, i) && isname(argument, skill_info[i].name.c_str())) {
                    skill_no = i;
                    if (SET_SKILL(victim, skill_no)) {
                        if ((skill_no == SKILL_COURAGE) && !IS_ORC(ch))
                            found2 = FALSE;
                        else if ((skill_no == SKILL_SATTACK) && !IS_GNOME(ch))
                            found2 = FALSE;
                        else if (skill_no == SKILL_HOLYLIGHT && !IS_TIEFLING(ch) && !IS_AASIMAR(ch))
                            found2 = FALSE;
                        if (skill_no == SKILL_AXES && IS_GNOME(ch))
                            found2 = TRUE;
                        else
                            for (j = 0; j < NUM_CLASSES; j++)
                                if (skill_info[skill_no].learn_level[j][(int) GET_GODS(ch)] &&
                                    check_class(ch,
                                                j) >=
                                    skill_info[skill_no].learn_level[j][(int) GET_GODS(ch)])
                                    found2 = TRUE;
                    }
                    if (!found2) {
                        send_to_char("Вы не удовлетворяете условиям обучения.\r\n", ch);
                        return (1);
                    }

                    if (GET_EXP(ch) < LEARN_COST(ch) && GET_LEVEL(ch) > 1) {
                        sprintf(buf, "Вам не хватает %ld очков опыта.",
                                LEARN_COST(ch) - GET_EXP(ch));
                        act(buf, FALSE, ch, 0, victim, TO_CHAR);
                        return (1);
                    }

                    if (get_levelexp(ch, GET_LEVEL(ch), 1) >= (GET_EXP(ch) - LEARN_COST(ch))
                        && GET_LEVEL(ch) > 1) {
                        sprintf(buf, "Вы рискуете потерять слишком много опыта.");
                        act(buf, FALSE, ch, 0, victim, TO_CHAR);
                        return (1);
                    }

                    if (SET_SKILL(ch, skill_no)) {
                        if (SET_SKILL(ch, skill_no) >= SET_SKILL(victim, skill_no)) {
                            sprintf(buf,
                                    "С Вашими знаниями Вы сами могли бы работать тут учителем.");
                            act(buf, FALSE, ch, 0, victim, TO_CHAR);
                            return (1);
                        }
                        if (fmod(SET_SKILL(ch, skill_no), 10)) {
                            act("Вам не хватает практики в этом умении.", FALSE, ch, 0, victim,
                                TO_CHAR);
                        } else {
                            sprintf(buf, "$N повысил$G Ваше умение %s%s%s",
                                    CCICYN(ch, C_NRM), skill_name(skill_no), CCNRM(ch, C_NRM));
                            act(buf, FALSE, ch, 0, victim, TO_CHAR);
                            act("$n прошел$g курс повышения умения.", TRUE, ch, 0, 0, TO_ROOM);
                            SET_SKILL(ch, skill_no)++;
                            if (GET_LEVEL(ch) > 1)
                                GET_EXP(ch) -= LEARN_COST(ch);
                        }
                    } else {
                        sprintf(buf, "$N научил$G Вас умению %s%s%s",
                                CCICYN(ch, C_NRM), skill_name(skill_no), CCNRM(ch, C_NRM));
                        act(buf, FALSE, ch, 0, victim, TO_CHAR);
                        act("$n прошел$g курс обучения новому умению.", TRUE, ch, 0, 0, TO_ROOM);
                        SET_SKILL(ch, skill_no) = 5;
                        SET_SKILL_LEVEL(ch, skill_no, 0);       //ставим эктрафлаг на уровень BASIC
                        if (GET_LEVEL(ch) > 1)
                            GET_EXP(ch) -= LEARN_COST(ch);
                    }
                    return (1);
                }
            if (!found2) {
                act("$N сказал$G: 'Я не знаю такого умения.'", FALSE, ch, 0, victim, TO_CHAR);
                return (1);
            }
            /* УЧИТЬ */
            break;
        case SCMD_LEV:
            //send_to_charf(ch,"Ваш gods %s, учитель gods %s\r\n",gods_name[(int)GET_GODS(ch)],gods_name[(int)GET_GODS(victim)]);
            if (IS_NECRO(victim) && !IS_EVILS(ch)) {
                act("Вам не чему здесь тренироваться.", FALSE, ch, 0, 0, TO_CHAR);
                return (1);
            }

            if (IS_PRIEST(victim) && check_class(ch, CLASS_NECRO)) {
                act("Вам не чему здесь тренироваться.", FALSE, ch, 0, 0, TO_CHAR);
                return (1);
            }

            if ((IS_PRIEST(victim) && GET_GODS(ch) != GET_GODS(victim)))
//       || (!IS_PRIEST(ch) && IS_PRIEST(victim)))
            {
                return (0);
                /*
                   sprintf(buf,"Для тренировки пройдите в свою гильдию.");
                   act(buf,FALSE, ch, 0, victim, TO_CHAR);
                   return(1); */
            }

            /*   if (IS_PRIEST(ch) && !IS_PRIEST(victim))
               {
               sprintf(buf,"Для тренировки пройдите в свою гильдию.");
               act(buf,FALSE, ch, 0, victim, TO_CHAR);
               return(1);
               } */

            next_level = GET_LEVEL(ch) + 1;
            next_this_level = check_class(ch, GET_CLASS(victim));
            next_this_level++;

            if (!*argument) {
                if (next_level >= LVL_IMMORT) {
                    sprintf(buf, "Вы достигли своего максимума в развитии.");
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else if (next_this_level > GET_LEVEL(victim)) {
                    sprintf(buf, "Вы не можете больше здесь тренироваться.");
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else if (next_this_level >= LVL_IMMORT) {
                    sprintf(buf, "Вы достигли максимального развития в профессии %s%sа%s.",
                            CCICYN(ch, C_NRM),
                            class_name[(int) GET_CLASS(victim)], CCNRM(ch, C_NRM));
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else if (IS_PRIEST(ch) && IS_PRIEST(victim) && (GET_GODS(ch) != GET_GODS(victim))) {
                    /*sprintf(buf,"Для тренировки пройдите в свою гильдию.");
                       act(buf,FALSE, ch, 0, victim, TO_CHAR); */
                    return (0);
                } else {
                    sprintf(buf, "Вы можете тренироваться:\r\n"
                            "профессия %s%s%s %dго уровня\r\n",
                            CCICYN(ch, C_NRM),
                            class_name[(int) GET_CLASS(victim)], CCNRM(ch, C_NRM), next_this_level);
                    sprintf(buf + strlen(buf),
                            "\r\nВам нужно %s%ld%s очков опыта, для тренировки.\r\n", CCIGRN(ch,
                                                                                             C_NRM),
                            get_levelexp(ch, next_level, 1), CCNRM(ch, C_NRM));
                    sprintf(buf + strlen(buf),
                            "Наберите %sтренировать уровень%s, чтобы начать тренировку.", CCICYN(ch,
                                                                                                 C_NRM),
                            CCNRM(ch, C_NRM));
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                }
            }

            if (!strn_cmp(argument, "уровень", strlen(argument)) ||
                !strn_cmp(argument, "level", strlen(argument))) {
                if (next_level >= LVL_IMMORT) {
                    sprintf(buf, "Вы достигли своего максимума в развитии.");
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else if (next_this_level > GET_LEVEL(victim)) {
                    sprintf(buf, "Вы не можете больше здесь тренироваться.");
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else if (get_dsu_exp(ch) > 0) {
                    // не хватает опыта
                    sprintf(buf, "Вам не хватает %ld опыта.", get_dsu_exp(ch));
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);
                    return (1);
                } else {
                    if (!GET_LEVEL(ch)) {
                        GET_FVCLASS(ch) = GET_CLASS(victim);
                        GET_HLEVEL(ch, 1) = GET_CLASS(victim);
                    }
                    // учим
                    if (!IS_MAX_EXP(ch)) {
                        GET_LEVEL(ch) = next_level;
                        GET_HLEVEL(ch, next_level) = GET_CLASS(victim);
                    }

                    GET_CLASS(ch) = GET_CLASS(victim);

                    if (!IS_MAX_EXP(ch))
                        add_class(ch, GET_CLASS(victim), next_this_level, 0);
                    else
                        add_class(ch, GET_CLASS(victim), next_this_level, 1);

                    advance_level(ch);
                    sprintf(buf, "Вы стали %sом %dго уровня.",
                            class_name[(int) GET_CLASS(victim)], next_this_level);
                    act(buf, FALSE, ch, 0, victim, TO_CHAR);

                    sprintf(buf, "$n повысил$g свою квалификацию в профессии %sа.",
                            class_name[(int) GET_CLASS(victim)]);
                    act(buf, TRUE, ch, 0, victim, TO_ROOM);
                    if (GET_LEVEL(ch) == LVL_ROLL && !PRF_FLAGGED(ch, PRF_ROLL)) {
                        SET_BIT(PRF_FLAGS(ch, PRF_ROLL), PRF_ROLL);
                        roll_stat(ch);
                    }
                    return (1);
                }
            }
            sprintf(buf, "$N сказал$G: 'Что-то я не пойму чего ты от меня хочешь?'");
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
            return (1);
            break;
    }

    return (0);
}



SPECIAL(horse_keeper)
{
    struct char_data *victim = (struct char_data *) me, *horse = NULL;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return (0);

    if (!CMD_IS("лошадь") && !CMD_IS("horse"))
        return (0);


    skip_spaces(&argument);

    if (!*argument) {
        if (has_horse(ch, FALSE)) {
            act("$N поинтересовал$U: \"$n, зачем тебе второй скакун ? У тебя ведь одно седалище.\"",
                FALSE, ch, 0, victim, TO_CHAR);
            return (TRUE);
        }
        sprintf(buf, "$N сказал$G: \"Я продам тебе скакуна за %d %s.\"",
                HORSE_COST, desc_count(HORSE_COST, WHAT_MONEYa));
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
        return (TRUE);
    }

    if (!strn_cmp(argument, "купить", strlen(argument)) ||
        !strn_cmp(argument, "buy", strlen(argument))) {
        if (has_horse(ch, FALSE)) {
            act("$N засмеял$U: \"$n, ты шутишь, у тебя же есть скакун.\"",
                FALSE, ch, 0, victim, TO_CHAR);
            return (TRUE);
        }
        if (GET_GOLD(ch) < HORSE_COST) {
            act("\"Ступай отсюда, злыдень, у тебя нет таких денег!\"-заорал$G $N",
                FALSE, ch, 0, victim, TO_CHAR);
            return (TRUE);
        }
        if (!(horse = read_mobile(HORSE_VNUM, VIRTUAL))) {
            act("\"Извини, у меня нет для тебя скакуна.\"-смущенно произнес$Q $N",
                FALSE, ch, 0, victim, TO_CHAR);
            return (TRUE);
        }
        make_horse(horse, ch);
        char_to_room(horse, IN_ROOM(ch));
        sprintf(buf, "$N оседлал$G %s и отдал$G %s Вам.", GET_PAD(horse, 3), HSHR(horse, ch));
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
        sprintf(buf, "$N оседлал$G %s и отдал$G %s $n2.", GET_PAD(horse, 3), HSHR(horse, ch));
        act(buf, FALSE, ch, 0, victim, TO_ROOM);
        GET_GOLD(ch) -= HORSE_COST;
        SET_BIT(PLR_FLAGS(ch, PLR_CRASH), PLR_CRASH);
        return (TRUE);
    }


    if (!strn_cmp(argument, "продать", strlen(argument)) ||
        !strn_cmp(argument, "sell", strlen(argument))) {
        if (!has_horse(ch, TRUE)) {
            act("$N засмеял$U: \"$n, ты не влезешь в мое стойло.\"", FALSE, ch, 0, victim, TO_CHAR);
            return (TRUE);
        }
        if (on_horse(ch)) {
            act("\"Я не собираюсь платить еще и за всадника.\"-усмехнул$U $N",
                FALSE, ch, 0, victim, TO_CHAR);
            return (TRUE);
        }

        if (!(horse = get_horse(ch)) || GET_MOB_VNUM(horse) != HORSE_VNUM) {
            act("\"Извини, твой скакун мне не подходит.\"- заявил$G $N",
                FALSE, ch, 0, victim, TO_CHAR);
            return (TRUE);
        }

        if (IN_ROOM(horse) != IN_ROOM(victim)) {
            act("\"Извини, твой скакун где-то бродит.\"- заявил$G $N",
                FALSE, ch, 0, victim, TO_CHAR);
            return (TRUE);
        }

        extract_char(horse, FALSE);
        sprintf(buf, "$N расседлал$G %s и отвел$G %s в стойло.", GET_PAD(horse, 3),
                HSHR(horse, ch));
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
        sprintf(buf, "$N расседлал$G %s и отвел$G %s в стойло.", GET_PAD(horse, 3),
                HSHR(horse, ch));
        act(buf, FALSE, ch, 0, victim, TO_ROOM);
        GET_GOLD(ch) += (HORSE_COST >> 1);
        SET_BIT(PLR_FLAGS(ch, PLR_CRASH), PLR_CRASH);
        return (TRUE);
    }

    return (0);
}


int get_last_move(struct track_data *track)
{
    int last = 0x00, last_i = BFS_ERROR, i;

    for (i = 0; i < NUM_OF_DIRS; i++) {
        if (track->time_outgone[i] >> last) {
            last = track->time_outgone[i];
            last_i = i;
        }
    }
    return last_i;
}

int npc_track(struct char_data *ch)
{
    struct char_data *vict;
    int door = BFS_ERROR;
    struct track_data *track;

    if (IN_ROOM(ch) == NOWHERE)
        return (BFS_ERROR);

    for (vict = character_list; vict && door == BFS_ERROR; vict = vict->next) {
        if (vict && CAN_SEE(ch, vict) && IN_ROOM(vict) != NOWHERE)
            if (ch->pk_list.count(GET_ID(vict)) && (!MOB_FLAGGED(ch, MOB_STAY_ZONE) ||
                                                    world[IN_ROOM(ch)].zone ==
                                                    world[IN_ROOM(vict)].zone)) {
                for (track = world[IN_ROOM(ch)].track; track && door == BFS_ERROR;
                     track = track->next)
                    if (track->who == GET_ID(vict))
                        door = get_last_move(track);

                if (door == BFS_ERROR)
                    door = go_track(ch, vict, SKILL_TRACK);
                act("Похоже, $n кого-то выслеживает.", FALSE, ch, 0, 0, TO_ROOM);
            }
    }



    /*sprintf(buf,"$n: дверь %d",door);
       act(buf,FALSE,ch,0,0,TO_ROOM); */
    return (door);
}


int item_nouse(struct obj_data *obj)
{
    switch (GET_OBJ_TYPE(obj)) {
        case ITEM_LIGHT:
            if (GET_LIGHT_VAL(obj) == 0)
                return (TRUE);
            break;
        case ITEM_SCROLL:
        case ITEM_POTION:
            if (!GET_OBJ_VAL(obj, 1) && !GET_OBJ_VAL(obj, 2) && !GET_OBJ_VAL(obj, 3))
                return (TRUE);
            break;
        case ITEM_STAFF:
        case ITEM_WAND:
            if (!GET_OBJ_VAL(obj, 2))
                return (TRUE);
            break;
        case ITEM_OTHER:
        case ITEM_TOOLS:
        case ITEM_CONTAINER:
        case ITEM_NOTE:
//case ITEM_DRINKCON:
//case ITEM_FOOD:
        case ITEM_PEN:
        case ITEM_BOAT:
        case ITEM_FOUNTAIN:
        case ITEM_CORPSE:
            return (TRUE);
            break;
    }
    return (FALSE);
}

void npc_dropunuse(struct char_data *ch)
{
    struct obj_data *obj, *nobj;

    for (obj = ch->carrying; obj; obj = nobj) {
        nobj = obj->next_content;
        if (item_nouse(obj)) {
            act("$n выбросил$g $o3.", FALSE, ch, obj, 0, FALSE);
            obj_from_char(obj);
            obj_to_room(obj, IN_ROOM(ch));
        }
    }
}



int npc_scavenge(struct char_data *ch)
{
    int max = 1;
    struct obj_data *obj, *best_obj;

    if (IS_SHOPKEEPER(ch))
        return (FALSE);

    if (GET_POS(ch) != GET_DEFAULT_POS(ch))
        return (FALSE);

    npc_dropunuse(ch);

    if (world[IN_ROOM(ch)].contents && !number(0, 1)) {
        max = 1;
        best_obj = NULL;
        //Выбираем самую ценную вещь
        for (obj = world[IN_ROOM(ch)].contents; obj; obj = obj->next_content) {
            if (OBJ_FLAGGED(obj, ITEM_NODECAY) || OBJ_FLAGGED(obj, ITEM_NODROP))
                continue;
            if (CAN_GET_OBJ(ch, obj) && GET_OBJ_COST(obj) > max) {
                best_obj = obj;
                max = GET_OBJ_COST(obj);
            }
        }
        if (best_obj != NULL)
            perform_get_from_room(ch, best_obj);
    }

    return (TRUE);
}


int npc_loot(struct char_data *ch)
{
    int max = FALSE;
    struct obj_data *obj, *loot_obj, *next_loot, *cobj, *cnext_obj;

    if (IS_SHOPKEEPER(ch))
        return (FALSE);
    if (GET_POS(ch) != GET_DEFAULT_POS(ch))
        return (FALSE);

    npc_dropunuse(ch);
    if (world[ch->in_room].contents && number(0, GET_REAL_INT(ch)) > 10) {
        for (obj = world[ch->in_room].contents; obj; obj = obj->next_content)
            // if (OBJ_FLAGGED(obj, ITEM_NODECAY) || OBJ_FLAGGED(obj, ITEM_NODROP))
            //  continue;

            if (CAN_SEE_OBJ(ch, obj) && IS_CORPSE(obj))
                for (loot_obj = obj->contains; loot_obj; loot_obj = next_loot) {
                    next_loot = loot_obj->next_content;
                    //            if (OBJ_FLAGGED(loot_obj, ITEM_NODECAY) || OBJ_FLAGGED(loot_obj, ITEM_NODROP))
                    //             continue;
                    if (GET_OBJ_TYPE(loot_obj) == ITEM_CONTAINER) {
                        if (IS_CORPSE(loot_obj) || OBJVAL_FLAGGED(loot_obj, EXIT_LOCKED))
                            continue;
                        for (cobj = loot_obj->contains; cobj; cobj = cnext_obj) {
                            cnext_obj = cobj->next_content;
                            //                   if (OBJ_FLAGGED(cobj, ITEM_NODECAY) || OBJ_FLAGGED(cobj, ITEM_NODROP))
                            //                    continue;
                            if (CAN_GET_OBJ(ch, cobj) && !item_nouse(cobj)) {
                                act("$n взял$g $o3 из $O1.", TRUE, ch, cobj, obj, TO_ROOM);
                                //get_check_money(ch, cobj);
                                obj_from_obj(cobj);
                                obj_to_char(cobj, ch);
                                max++;
                            }
                        }
                    } else if (CAN_GET_OBJ(ch, loot_obj) && !item_nouse(loot_obj)) {
                        act("$n взял$g $o3 из $O1.", TRUE, ch, loot_obj, obj, TO_ROOM);
                        //get_check_money(ch, loot_obj);
                        obj_from_obj(loot_obj);
                        obj_to_char(loot_obj, ch);
                        max++;
                    }
                }
    }
    return (max);
}

int npc_move(struct char_data *ch, int dir, int need_specials_check)
{
    /* room_rnum was_in;
       struct    follow_type *k, *next; */
    int need_close = FALSE, need_lock = FALSE;
    struct room_direction_data *rdata = NULL;
    int retval = FALSE;
    struct follow_type *f;
    struct char_data *tch = NULL;


    if (ch && ch->followers)
        for (f = ch->followers; f; f = f->next) {
            tch = f->follower;
            if (GET_POS(tch) != POS_STANDING ||
                AFF_FLAGGED(tch, AFF_HOLD) ||
                AFF_FLAGGED(tch, AFF_STOPFIGHT) || AFF_FLAGGED(tch, AFF_STUNE))
                return (FALSE);
        }

    if (ch == NULL || dir < 0 || dir >= NUM_OF_DIRS || FIGHTING(ch))
        return (FALSE);
    else if (!EXIT(ch, dir) || EXIT(ch, dir)->to_room == NOWHERE)
        return (FALSE);
    else if (ch->master && IN_ROOM(ch) == IN_ROOM(ch->master)) {
        return (FALSE);
    } else if (DOOR_FLAGGED(EXIT(ch, dir), EXIT_CLOSED)) {
        rdata = EXIT(ch, dir);
        if (DOOR_FLAGGED(rdata, EXIT_LOCKED)) {
            if (has_key(ch, rdata->key)) {
                go_unlock_door(ch, dir);
                need_lock = TRUE;
            } else
                return (FALSE);
        }
        if (GET_REAL_INT(ch) >= 5 || GET_DEST(ch) != NOWHERE) {
            go_open_door(ch, dir);
            need_close = TRUE;
        }
    }

    /* Монстра крадется */
    if (GET_POS(ch) == POS_STANDING && GET_SKILL_MOB(ch, SKILL_SNEAK)
        && !IS_AFFECTED(ch, AFF_SNEAK)) {
        struct affected_type af;
        int skill = GET_SKILL_MOB(ch, SKILL_SNEAK);

        affect_from_char(ch, SPELL_SNEAK);
        af.type = find_spell_num(SPELL_SNEAK);
        af.location = APPLY_NONE;
        af.modifier = skill;
        af.duration = SECS_PER_MUD_ROUND;       //Даем 1 секунду
        af.bitvector = AFF_SNEAK;
        af.battleflag = 0;
        af.owner = GET_ID(ch);
        af.main = TRUE;
        affect_join_char(ch, &af);
        affect_total(ch);
    }

    retval = perform_move(ch, dir, 1, FALSE);
    /*
       if (!ch->followers)
       retval = do_simple_move(ch, dir, need_specials_check);
       else
       {was_in = ch->in_room;
       if ((retval = do_simple_move(ch, dir, need_specials_check)))
       {for (k = ch->followers; k; k = next)
       {next = k->next;
       if (FIGHTING(k->follower)    ||
       GET_MOB_HOLD(k->follower)||
       GET_POS(ch) <= POS_STUNNED
       )
       continue;
       if (k->follower->in_room == was_in)
       {if (IS_NPC(ch))
       {if (GET_POS(k->follower) < POS_FIGHTING)
       {act("$n поднял$u на ноги.",FALSE,k->follower,0,0,TO_ROOM);
       GET_POS(k->follower) = POS_STANDING;
       }
       }
       else
       if (GET_POS(k->follower) < POS_STANDING))
       continue;
       perform_move(k->follower, dir, 1, FALSE);
       }
       }
       }
       }
     */
    if (need_close) {
        if (retval)
            go_close_door(ch, rev_dir[dir]);
        else
            go_close_door(ch, dir);
    }

    if (need_lock) {
        if (retval)
            go_lock_door(ch, rev_dir[dir]);
        else
            go_lock_door(ch, dir);
    }

    return (retval);
}


void npc_remove_staff(struct char_data *ch)
{
    struct obj_data *obj = NULL;

    if ((obj = GET_EQ(ch, WEAR_HOLD)) != NULL) {
        if (GET_OBJ_TYPE(obj) == ITEM_WAND || GET_OBJ_TYPE(obj) == ITEM_STAFF)
            if (GET_OBJ_VAL(obj, 2) <= 0)
                do_remove(ch, GET_OBJ_PNAME(obj, 0), 0, 0, 0);
    }
}

int calculate_weapon_class(struct char_data *ch, struct obj_data *weapon)
{
    int damage = 0, hits = 0, i;

    if (!weapon || GET_OBJ_TYPE(weapon) != ITEM_WEAPON)
        return (0);

    hits = GET_SKILL_MOB(ch, GET_OBJ_SKILL(weapon));
    damage = (GET_OBJ_VAL(weapon, 1) + 1) * (GET_OBJ_VAL(weapon, 2)) / 2;
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
        if (weapon->affected[i].location == APPLY_DAMROLL)
            damage += weapon->affected[i].modifier;
        if (weapon->affected[i].location == APPLY_HITROLL)
            hits += weapon->affected[i].modifier * 10;
    }

    return (damage + (hits > 200 ? 10 : hits / 20));
}

void best_weapon(struct char_data *ch, struct obj_data *sweapon, struct obj_data **dweapon)
{
    if (*dweapon == NULL) {
        if (calculate_weapon_class(ch, sweapon) > 0)
            *dweapon = sweapon;
    } else if (calculate_weapon_class(ch, sweapon) > calculate_weapon_class(ch, *dweapon))
        *dweapon = sweapon;
}

void npc_wield(struct char_data *ch)
{
    struct obj_data *obj, *next, *right = NULL, *left = NULL, *both = NULL, *temp = NULL;

    if (IS_SHOPKEEPER(ch))
        return;

    if (GET_EQ(ch, WEAR_HOLD) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_WEAPON)
        left = GET_EQ(ch, WEAR_HOLD);
    if (GET_EQ(ch, WEAR_WIELD) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_WIELD)) == ITEM_WEAPON)
        right = GET_EQ(ch, WEAR_WIELD);
    if (GET_EQ(ch, WEAR_BOTHS) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_BOTHS)) == ITEM_WEAPON)
        both = GET_EQ(ch, WEAR_BOTHS);

    if (((left && right) || (both)))
        return;

    for (obj = ch->carrying; obj; obj = next) {
        next = obj->next_content;
        if (GET_OBJ_TYPE(obj) != ITEM_WEAPON)
            continue;
        if (CAN_WEAR(obj, ITEM_WEAR_WIELD) && OK_WIELD(ch, obj))
            best_weapon(ch, obj, &right);
        else if (CAN_WEAR(obj, ITEM_WEAR_HOLD) && OK_HELD(ch, obj))
            best_weapon(ch, obj, &left);
        else if (CAN_WEAR(obj, ITEM_WEAR_BOTHS) && OK_BOTH(ch, obj))
            best_weapon(ch, obj, &both);
    }

    if (both && calculate_weapon_class(ch, both) > calculate_weapon_class(ch, left) +
        calculate_weapon_class(ch, right)) {
        if (both == GET_EQ(ch, WEAR_BOTHS))
            return;

        if (GET_EQ(ch, WEAR_WIELD)) {
            temp = GET_EQ(ch, WEAR_WIELD);
            do_remove(ch, GET_OBJ_PNAME(temp, 0), 0, 0, 0);
        }
        if (GET_EQ(ch, WEAR_HOLD)) {
            temp = GET_EQ(ch, WEAR_HOLD);
            do_remove(ch, GET_OBJ_PNAME(temp, 0), 0, 0, 0);
        }
        do_wield(ch, GET_OBJ_PNAME(both, 0), 0, 0, 0);
    } else {
        if (left && GET_EQ(ch, WEAR_HOLD) != left) {
            if (GET_EQ(ch, WEAR_BOTHS)) {
                temp = GET_EQ(ch, WEAR_BOTHS);
                do_remove(ch, GET_OBJ_PNAME(temp, 0), 0, 0, 0);
            }
            if (GET_EQ(ch, WEAR_HOLD)) {
                temp = GET_EQ(ch, WEAR_HOLD);
                do_remove(ch, GET_OBJ_PNAME(temp, 0), 0, 0, 0);
            }
            do_hold(ch, GET_OBJ_PNAME(left, 0), 0, 0, 0);
        }

        if (right && GET_EQ(ch, WEAR_WIELD) != right) {
            if (GET_EQ(ch, WEAR_BOTHS)) {
                temp = GET_EQ(ch, WEAR_BOTHS);
                do_remove(ch, GET_OBJ_PNAME(temp, 0), 0, 0, 0);
            }
            if (GET_EQ(ch, WEAR_WIELD)) {
                temp = GET_EQ(ch, WEAR_WIELD);
                do_remove(ch, GET_OBJ_PNAME(temp, 0), 0, 0, 0);
            }
            do_wield(ch, GET_OBJ_PNAME(right, 0), 0, 0, 0);
        }
    }
}

void npc_armor(struct char_data *ch)
{
    struct obj_data *obj, *next;
    int where = 0;

    if (GET_REAL_INT(ch) < 10 || IS_SHOPKEEPER(ch))
        return;

    for (obj = ch->carrying; obj; obj = next) {
        next = obj->next_content;
        if (GET_OBJ_TYPE(obj) != ITEM_ARMOR)
            continue;
        if (CAN_WEAR(obj, ITEM_WEAR_FINGER))
            where = WEAR_FINGER_R;
        if (CAN_WEAR(obj, ITEM_WEAR_NECK))
            where = WEAR_NECK_1;
        if (CAN_WEAR(obj, ITEM_WEAR_BODY))
            where = WEAR_BODY;
        if (CAN_WEAR(obj, ITEM_WEAR_HEAD))
            where = WEAR_HEAD;
        if (CAN_WEAR(obj, ITEM_WEAR_LEGS))
            where = WEAR_LEGS;
        if (CAN_WEAR(obj, ITEM_WEAR_FEET))
            where = WEAR_FEET;
        if (CAN_WEAR(obj, ITEM_WEAR_HANDS))
            where = WEAR_HANDS;
        if (CAN_WEAR(obj, ITEM_WEAR_ARMS))
            where = WEAR_ARMS;
        if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))
            where = WEAR_SHIELD;
        if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))
            where = WEAR_ABOUT;
        if (CAN_WEAR(obj, ITEM_WEAR_WAIST))
            where = WEAR_WAIST;
        if (CAN_WEAR(obj, ITEM_WEAR_WRIST))
            where = WEAR_WRIST_R;

        if (!where)
            continue;

        if ((where == WEAR_FINGER_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R))
            if (GET_EQ(ch, where))
                where++;
        if (where == WEAR_SHIELD && (GET_EQ(ch, WEAR_BOTHS) || GET_EQ(ch, WEAR_HOLD)))
            continue;
        if (GET_EQ(ch, where)) {
            if (GET_REAL_INT(ch) < 15)
                continue;
            if (GET_OBJ_VAL(obj, 0) + GET_OBJ_VAL(obj, 1) * 3 <=
                GET_OBJ_VAL(GET_EQ(ch, where), 0) + GET_OBJ_VAL(GET_EQ(ch, where), 1) * 3)
                continue;
            do_remove(ch, GET_OBJ_PNAME(GET_EQ(ch, where), 0), 0, 0, 0);
        }
        do_wear(ch, GET_OBJ_PNAME(obj, 0), 0, 0, 0);
        break;
    }
}

void npc_light(struct char_data *ch)
{
    struct obj_data *obj, *next;

    if (IS_SHOPKEEPER(ch))
        return;

    if (AFF_FLAGGED(ch, AFF_INFRAVISION) || affected_by_spell(ch, SPELL_INFRAVISION))
        return;

    if (AFF_FLAGGED(ch, AFF_DARKVISION))
        return;

    if ((obj = GET_EQ(ch, WEAR_LIGHT)) && (GET_OBJ_VAL(obj, 2) == 0 || !IS_DARK(IN_ROOM(ch)))) {
        do_remove(ch, GET_OBJ_PNAME(obj, 0), 0, 0, 0);
    }

    if (!GET_EQ(ch, WEAR_LIGHT) && IS_DARK(IN_ROOM(ch)))
        for (obj = ch->carrying; obj; obj = next) {
            next = obj->next_content;
            if (GET_OBJ_TYPE(obj) != ITEM_LIGHT)
                continue;
            if (GET_OBJ_VAL(obj, 2) == 0) {
                do_drop(ch, GET_OBJ_PNAME(obj, 0), 0, 0, 0);
                continue;
            }
            do_wear(ch, GET_OBJ_PNAME(obj, 0), 0, 0, 0);
            return;
        }
}


int npc_battle_scavenge(struct char_data *ch)
{
    int max = FALSE;
    struct obj_data *obj, *next_obj = NULL;

    if (IS_SHOPKEEPER(ch))
        return (FALSE);

    if (world[IN_ROOM(ch)].contents && number(0, GET_REAL_INT(ch)) > 10)
        for (obj = world[IN_ROOM(ch)].contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (CAN_GET_OBJ(ch, obj) &&
                (GET_OBJ_TYPE(obj) == ITEM_ARMOR || GET_OBJ_TYPE(obj) == ITEM_WEAPON)) {
                do_get(ch, GET_OBJ_PNAME(obj, 0), 0, 0, 0);
                max = TRUE;
            }
        }
    return (max);
}


int npc_walk(struct char_data *ch)
{
    int rnum, door = BFS_ERROR;

    if (GET_DEST(ch) == NOWHERE || (rnum = real_room(GET_DEST(ch))) == NOWHERE)
        return (BFS_ERROR);

    if (IN_ROOM(ch) == rnum) {
        if (ch->npc()->specials.dest_count == 1)
            return (NUM_OF_DIRS);
        if (ch->npc()->specials.dest_pos == ch->npc()->specials.dest_count - 1 &&
            ch->npc()->specials.dest_dir >= 0)
            ch->npc()->specials.dest_dir = -1;
        if (!ch->npc()->specials.dest_pos && ch->npc()->specials.dest_dir < 0)
            ch->npc()->specials.dest_dir = 0;
        ch->npc()->specials.dest_pos += ch->npc()->specials.dest_dir >= 0 ? 1 : -1;
        if ((rnum = real_room(GET_DEST(ch))) == NOWHERE || rnum == IN_ROOM(ch))
            return (BFS_ERROR);
        else
            return (npc_walk(ch));
    }

    door = find_first_step(ch->in_room, real_room(GET_DEST(ch)), ch);
    // log("MOB %s walk to room %d at dir %d", GET_NAME(ch), GET_DEST(ch), door);

    return (door);
}

void do_npc_steal(struct char_data *ch, struct char_data *victim)
{
    struct obj_data *obj, *best = NULL;
    int gold;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(victim) || IS_SHOPKEEPER(ch) || FIGHTING(victim))
        return;

    if (GET_LEVEL(victim) >= LVL_IMMORT)
        return;

    if (!CAN_SEE(ch, victim))
        return;

    if (!may_kill_here(ch, victim))
        return;

    gold = (int) ((GET_GOLD(victim) * number(1, 10)) / 100);
    if (gold > 0) {
        sprintf(buf, "монет");
        go_steal(ch, victim, buf);
    } else {
        for (obj = victim->carrying; obj; obj = obj->next_content)
            if (CAN_SEE_OBJ(ch, obj) && (!best || GET_OBJ_COST(obj) > GET_OBJ_COST(best)))
                best = obj;

        if (best) {
            sprintf(buf, "%s", GET_OBJ_PNAME(best, 0));
            go_steal(ch, victim, buf);
        }
    }

}

void npc_steal(struct char_data *ch)
{
    struct char_data *cons;

    if (GET_POS(ch) != POS_STANDING || IS_SHOPKEEPER(ch) || FIGHTING(ch))
        return;

    for (cons = world[ch->in_room].people; cons; cons = cons->next_in_room)
        if (!IS_NPC(cons) && !IS_IMMORTAL(cons) && (number(0, GET_REAL_INT(ch)) > 10)) {
            do_npc_steal(ch, cons);
            return;
        }
    return;
}

void npc_group(struct char_data *ch)
{
    struct char_data *vict = NULL;

    if (ch->helpers.empty() || AFF_FLAGGED(ch, AFF_CHARM) || IN_ROOM(ch) == NOWHERE)
        return;

    SET_BIT(AFF_FLAGS(ch, AFF_GROUP), AFF_GROUP);

    for (vict = world[IN_ROOM(ch)].people; vict; vict = vict->next_in_room) {
        if (!IS_NPC(vict) ||
            AFF_FLAGGED(vict, AFF_CHARM) ||
            AFF_FLAGGED(vict, AFF_HOLD) ||
            ch->helpers.count(GET_MOB_VNUM(vict)) == 0 ||
            GET_WAIT(vict) > 0 || GUARDING(vict) || FIGHTING(vict))
            continue;

        if (!vict->master) {
            add_follower(vict, ch, FLW_GROUP);
            //join_party(ch,vict);
        } else if (vict->master != ch && (!vict->master->helpers.count(GET_MOB_VNUM(vict))
                                          || IN_ROOM(vict) != IN_ROOM(vict->master))) {
            stop_follower(vict, SF_EMPTY);
            add_follower(vict, ch, FLW_GROUP);
            //join_party(ch,vict);
        }
        if (GET_SKILL_MOB(vict, SKILL_GUARD))
            GUARDING(vict) = ch;
        SET_BIT(AFF_FLAGS(vict, AFF_GROUP), AFF_GROUP);
    }
}

void npc_groupbattle(struct char_data *ch)
{
    struct follow_type *k;
    struct char_data *tch, *helper;

    if (!IS_NPC(ch) ||
        !FIGHTING(ch) ||
        AFF_FLAGGED(ch, AFF_CHARM) || IN_ROOM(ch) == NOWHERE || !AFF_FLAGGED(ch, AFF_GROUP))
        return;

    k = ch->master ? ch->master->followers : ch->followers;
    tch = ch->master ? ch->master : ch;
    for (; k; (k = tch ? k : k->next), tch = NULL) {
        helper = tch ? tch : k->follower;
        if (helper == ch)
            continue;

        if (IN_ROOM(ch) == IN_ROOM(helper) &&
            !FIGHTING(helper) &&
            IS_NPC(helper) && CAN_SEE(helper, FIGHTING(ch)) && MAY_ATTACK(helper)) {
            do_stand(helper, 0, 0, 0, 0);
            act("1и вступил1(,а,о,и) в бой на стороне 2р.", "Кмм", helper, ch);
            act("1и приш1(ел,ла,ло,ли) к Вам на помощь.", "мМ", helper, ch);
            set_fighting(helper, FIGHTING(ch));
        }
    }
}


SPECIAL(dump)
{
    struct obj_data *k;
    int value = 0;

    for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
        act("$p рассыпал$U в прах!", FALSE, 0, k, 0, TO_ROOM);
        extract_obj(k);
    }

    if (!CMD_IS("drop") || !CMD_IS("бросить"))
        return (0);

    do_drop(ch, argument, cmd, 0, 0);

    for (k = world[ch->in_room].contents; k; k = world[ch->in_room].contents) {
        act("$p рассыпал$U в прах!", FALSE, 0, k, 0, TO_ROOM);
        value += MAX(1, MIN(1, GET_OBJ_COST(k) / 10));
        extract_obj(k);
    }

    if (value) {
        send_to_char("Боги оценили Вашу жертву.\r\n", ch);
        act("$n оценен$y Богами.", TRUE, ch, 0, 0, TO_ROOM);
        if (GET_LEVEL(ch) < 3)
            gain_exp(ch, value, TRUE);
        else
            GET_GOLD(ch) += value;
    }
    return (1);
}



/* ********************************************************************
*  Special procedures for mobiles                                      *
******************************************************************** */

#define PET_PRICE(pet) (GET_LEVEL(pet) * 300)



/* ********************************************************************
*  Special procedures for objects                                     *
******************************************************************** */


SPECIAL(bank)
{
    struct char_data *victim = (struct char_data *) me;
    int amount, amountb;
    char buf[MAX_STRING_LENGTH];

    if (CMD_IS("balance") || CMD_IS("баланс") || CMD_IS("сальдо")) {
        if (GET_BANK_GOLD(ch) > 0)
            sprintf(buf, "У Вас на счету %ld %s.\r\n",
                    GET_BANK_GOLD(ch), desc_count(GET_BANK_GOLD(ch), WHAT_MONEYa));
        else
            sprintf(buf, "У Вас нет денег.\r\n");
        send_to_char(buf, ch);
        return (1);
    } else if (CMD_IS("deposit") || CMD_IS("вложить") || CMD_IS("вклад")) {
        if ((amount = atoi(argument)) <= 0) {
            send_to_char("Сколько Вы хотите вложить ?\r\n", ch);
            return (1);
        }
        if (GET_GOLD(ch) < amount) {
            send_to_char("О такой сумме Вы можете только мечтать!\r\n", ch);
            return (1);
        }
        amountb = (amount * 5) / 100;
        if (amountb < 1)
            amountb = 1;
        GET_GOLD(ch) -= amount;
        GET_BANK_GOLD(ch) += (amount - amountb);
        sprintf(buf, "Вы вложили %d %s.\r\n", amount, desc_count(amount, WHAT_MONEYu));
        sprintf(buf + strlen(buf), "За финансовую операцию $N взял$G %d %s.", amountb,
                desc_count(amountb, WHAT_MONEYu));
        act(buf, TRUE, ch, 0, victim, TO_CHAR);
        act("$n произвел$g финансовую операцию.", TRUE, ch, 0, FALSE, TO_ROOM);
        return (1);
    } else if (CMD_IS("withdraw") || CMD_IS("получить")) {
        if ((amount = atoi(argument)) <= 0) {
            send_to_char("Уточните количество денег, которые Вы хотите получить ?\r\n", ch);
            return (1);
        }
        if (GET_BANK_GOLD(ch) < amount) {
            send_to_char("Да Вы отродясь столько денег не видели!\r\n", ch);
            return (1);
        }
        amountb = (amount * 5) / 100;
        if (amountb < 1)
            amountb = 1;
        GET_GOLD(ch) += (amount - amountb);
        GET_BANK_GOLD(ch) -= amount;
        sprintf(buf, "Вы сняли %d %s.\r\n", amount, desc_count(amount, WHAT_MONEYu));
        sprintf(buf + strlen(buf), "За финансовую операцию $N взял$G %d %s.", amountb,
                desc_count(amountb, WHAT_MONEYu));
        act(buf, TRUE, ch, 0, victim, TO_CHAR);
        act("$n произвел$g финансовую операцию.", TRUE, ch, 0, FALSE, TO_ROOM);
        return (1);
    } else
        return (0);
}


int check_obj(struct char_data *ch, struct obj_data *obj, int type)
{
    struct obj_data *cobj = NULL;
    int result = 0;

    if (GET_OBJ_COST(obj) > max_obj_cost[type]) {
        send_to_charf(ch, "%s нельзя оставить на хранение в этой комнате.\r\n",
                      CAP(GET_OBJ_PNAME(obj, 3)));
        result++;
    }

    if (OBJ_FLAGGED(obj, ITEM_NORENT)) {
        send_to_charf(ch, "%s нельзя унести с собой на постой.\r\n", CAP(GET_OBJ_PNAME(obj, 3)));
        result++;
    }

    for (cobj = obj->contains; cobj; cobj = cobj->next_content)
        result += check_obj(ch, cobj, type);

    return (result);
}


SPECIAL(hotel)
{
    int save_room = NOWHERE, j;
    int quality = 0, chr = FALSE;
    struct char_data *tch, *master = NULL;
    struct obj_data *obj;
    char buf[MAX_STRING_LENGTH];

    if (CMD_IS("rent") || CMD_IS("постой")) {
        if (IS_NPC(ch)) {
            send_to_charf(ch, "Монстры не могут уходить на постой.\r\n");
            return (1);
        }

        if (RENTABLE(ch)) {
            send_to_charf(ch, "Вы учавствовали в агрессивных действиях. Постой невозможен.\r\n");
            return (1);
        }

        struct room_data *room = (struct room_data *) me;

        save_room = room->number;
        GET_LOADROOM(ch) = save_room;

        for (tch = room->people; tch; tch = tch->next_in_room)
            if (GET_MOB_VNUM(tch) == room->hotel->master) {
                master = tch;
                break;
            }

        if (room->hotel->type)
            quality = room->hotel->type;

        /* Проверяем стоимость предметов */
        for (j = 0; j < NUM_WEARS; j++)
            if (GET_EQ(ch, j))
                chr += check_obj(ch, GET_EQ(ch, j), quality);

        for (obj = ch->carrying; obj; obj = obj->next_content)
            chr += check_obj(ch, obj, quality);

        if (chr)
            return (TRUE);

        if (room->hotel->MessChar)
            act(room->hotel->MessChar, "Мм", ch, master);

        if (room->hotel->MessRoom)
            act(room->hotel->MessRoom, "Кмм", ch, master);

        if (master) {
            sprintf(buf, "$N сообщил$G Вам, что стоимость постоя составит &G%d %s&n в сутки.",
                    cost_hotel[quality], desc_count(cost_hotel[quality], WHAT_MONEYa));
            int gold = GET_GOLD(ch) + GET_BANK_GOLD(ch), day = 0;

            if (gold > 0 && cost_hotel[quality])
                day = gold / cost_hotel[quality];
            if (!cost_hotel[quality])
                sprintf(buf + strlen(buf), "\r\nВы можете проживать до следующего вайпа.");
            else if (day >= 1)
                sprintf(buf + strlen(buf),
                        "\r\nИмеющихся в Вашем распоряжении средств хватит на &C%d %s&n постоя.",
                        day, desc_count(day, WHAT_DAY));
            else
                sprintf(buf + strlen(buf), "\r\nВаших денег не хватит даже на одни сутки постоя.");
            act(buf, FALSE, ch, 0, master, TO_CHAR);
        } else {
            if (cost_hotel[quality]) {
                sprintf(buf, "Cтоимость постоя составит &G%d %s&n в сутки.", cost_hotel[quality],
                        desc_count(cost_hotel[quality], WHAT_MONEYa));
                int gold = GET_GOLD(ch) + GET_BANK_GOLD(ch), day = 0;

                if (gold > 0 && cost_hotel[quality])
                    day = gold / cost_hotel[quality];
                if (!cost_hotel[quality])
                    sprintf(buf + strlen(buf), "\r\nВы можете проживать до следующего вайпа.");
                else if (day >= 1)
                    sprintf(buf + strlen(buf),
                            "\r\nИмеющихся в Вашем распоряжении средств хватит на &C%d %s&n постоя.",
                            day, desc_count(day, WHAT_DAY));
                else
                    sprintf(buf + strlen(buf), "\r\nВаших денег не хватит даже на сутки постоя.");
            } else
                sprintf(buf, "Плата за постой в этой комнате не взимается.");
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        save_pets(ch);
        xsave_rent(ch, RENT_NORMAL, TRUE);
        write_aliases(ch);
        sprintf(buf, "%s уш%s на постой в локации #%d %s", GET_NAME(ch), GET_CH_SUF_5(ch),
                save_room, room->name);
        mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
        save_vars(ch);
        save_quests(ch);
        extract_char(ch, FALSE);

        return (1);
    } else if (CMD_IS("предложение")) {
        struct room_data *room = (struct room_data *) me;

        save_room = room->number;

        if (IS_NPC(ch)) {
            send_to_charf(ch, "Монстры не могут уходить на постой.\r\n");
            return (1);
        }

        GET_LOADROOM(ch) = save_room;

        if (RENTABLE(ch)) {
            send_to_charf(ch, "Вы учавствовали в агрессивных действиях. Постой невозможен.\r\n");
            return (1);
        }

        for (tch = room->people; tch; tch = tch->next_in_room)
            if (GET_MOB_VNUM(tch) == room->hotel->master) {
                master = tch;
                break;
            }

        if (room->hotel->type)
            quality = room->hotel->type;

        /* Проверяем стоимость предметов */
        for (j = 0; j < NUM_WEARS; j++)
            if (GET_EQ(ch, j))
                chr += check_obj(ch, GET_EQ(ch, j), quality);

        for (obj = ch->carrying; obj; obj = obj->next_content)
            chr += check_obj(ch, obj, quality);

        if (master) {
            sprintf(buf, "$N сообщил$G Вам, что стоимость постоя составит &G%d %s&n в сутки.",
                    cost_hotel[quality], desc_count(cost_hotel[quality], WHAT_MONEYa));
            int gold = GET_GOLD(ch) + GET_BANK_GOLD(ch), day = 0;

            if (gold > 0 && cost_hotel[quality])
                day = gold / cost_hotel[quality];
            if (!cost_hotel[quality])
                sprintf(buf + strlen(buf), "\r\nВы можете проживать до следующего вайпа.");
            else if (day >= 1)
                sprintf(buf + strlen(buf),
                        "\r\nИмеющихся в Вашем распоряжении средств хватит на &C%d %s&n постоя.",
                        day, desc_count(day, WHAT_DAY));
            else
                sprintf(buf + strlen(buf), "\r\nВаших денег не хватит даже на cутки постоя.");
            act(buf, FALSE, ch, 0, master, TO_CHAR);
        } else {
            sprintf(buf, "Cтоимость постоя составит &G%d %s&n в сутки.", cost_hotel[quality],
                    desc_count(cost_hotel[quality], WHAT_MONEYa));
            int gold = GET_GOLD(ch) + GET_BANK_GOLD(ch), day = 0;

            if (gold > 0 && cost_hotel[quality])
                day = gold / cost_hotel[quality];
            if (!cost_hotel[quality])
                sprintf(buf + strlen(buf), "\r\nВы можете проживать до следующего вайпа.");
            else if (day >= 1)
                sprintf(buf + strlen(buf),
                        "\r\nИмеющихся в Вашем распоряжении средств хватит на &C%d %s&n постоя.",
                        day, desc_count(day, WHAT_DAY));
            else
                sprintf(buf + strlen(buf), "\r\nВаших денег не хватит даже на сутки постоя.");
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
        }

        return (1);
    }

    return (0);
}


/*-----------------------------------------------------------------------*/
struct spec_func_info {
    const char *name;
     SPECIAL(*func);
};

const struct spec_func_info spec_func_table[] = {
    {"guild", guild},
    {"bank", bank},
    {"shoper", shoper},
    {"shop_hrs", shop_hrs},
    {"postmaster", postmaster},
    {"trainer", trainer},

    {"gen_board", gen_board},
    {"scriptboard", scriptboard},

    {"hotel", hotel},
    {"dump", dump},

    {0, 0}
};

void spec_func_assign(SPECIAL(*&ptr_func), char *&ptr_name, const char *name)
{
    if (!name) {
        log("ОШИБКА: Установка спецпроцедуры с нулевым именем");
        return;
    }

    for (int i = 0; spec_func_table[i].name; i++)
        if (strcmp(name, spec_func_table[i].name) == 0) {
            ptr_func = spec_func_table[i].func;
            if (ptr_name != name)
                str_reassign(ptr_name, name);
            return;
        }

    log("ОШИБКА: Неизвестная спецпроцедура '%s'", name);
}

void spec_func_assign_mob(mob_rnum imbs, const char *name)
{
    spec_func_assign(mob_index[imbs].func, mob_index[imbs].func_name, name);
}

void spec_func_assign_obj(obj_rnum iobj, const char *name)
{
    spec_func_assign(obj_index[iobj].func, obj_index[iobj].func_name, name);
}

void spec_func_assign_room(room_rnum rn, const char *name)
{
    spec_func_assign(world[rn].func, world[rn].func_name, name);
}

void specials_init()
{
    log("Устанавливаю спецпроцедуры для мобов, предметов, локаций");

    /* before boot_db, not to do double job */
    for (int i = 0; i < top_of_mobt; i++)
        if (mob_index[i].func_name)
            spec_func_assign_mob(i, mob_index[i].func_name);

    for (int i = 0; i < top_of_objt; i++)
        if (obj_index[i].func_name)
            spec_func_assign_obj(i, obj_index[i].func_name);

    for (int i = 0; i <= top_of_world; i++)
        if (world[i].func_name)
            spec_func_assign_room(i, world[i].func_name);
}

void specials_destroy()
{
    for (int i = 0; i < top_of_mobt; i++)
        mob_index[i].func = NULL;

    for (int i = 0; i < top_of_objt; i++)
        obj_index[i].func = NULL;

    for (int i = 0; i <= top_of_world; i++)
        world[i].func = NULL;
}

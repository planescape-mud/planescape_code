
/* ************************************************************************
*   File: act.informative.c                             Part of CircleMUD *
*  Usage: Player-level commands of an informative nature                  *
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
#include "act.social.h"
#include "db.h"
#include "spells.h"
#include "constants.h"
#include "screen.h"
#include "pk.h"
#include "house.h"
#include "case.h"
#include "xspells.h"
#include "xboot.h"
#include "events.h"
#include "planescape.h"
#include "mudstats.h"
#include "textfileloader.h"

/* extern functions */
SPECIAL(shoper);

/* local functions */
char *look_in_container(struct char_data *ch, struct obj_data *obj, int bits);
char *look_in_drink(struct char_data *ch, struct obj_data *obj, int bits);
void show_locates(struct char_data *ch);
void print_object_location(int num, struct obj_data *obj, struct char_data *ch, int recur);
void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode, int show_state,
                      int how);
void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, int show);
char *show_obj(struct obj_data *object, struct char_data *ch, int how);
char *buf_obj_to_char(struct obj_data *list, struct char_data *ch);

ACMD(do_affects);
ACMD(do_look);
ACMD(do_gold);
ACMD(do_score);
ACMD(do_inventory);
ACMD(do_equipment);
ACMD(do_time);
ACMD(do_weather);
ACMD(do_who);
ACMD(do_users);
ACMD(do_gen_ps);
void perform_mortal_where(struct char_data *ch, char *arg);
void perform_immort_where(struct char_data *ch, char *arg);

ACMD(do_where);
ACMD(do_levels);
ACMD(do_consider);
ACMD(do_diagnose);
ACMD(do_color);
ACMD(do_toggle);

ACMD(do_commands);
ACMD(do_peek);
ACMD(do_hearing);
ACMD(do_scan);
char *diag_char_to_char(struct char_data *i, int type);
const char *list_one_char(struct char_data *i, struct char_data *ch, int skill_mode, int count);
void list_char_to_char(struct char_data *list, struct char_data *ch);
char *do_auto_exits(struct char_data *ch);

ACMD(do_exits);
void look_in_direction(struct char_data *ch, int dir);
char *find_exdesc(const char *word, struct extra_descr_data *list);
char *find_exdesc_number(int number, struct extra_descr_data *list);
void look_at_target(struct char_data *ch, char *arg, int subcmd);


#define EXIT_SHOW_WALL    (1 << 0)
#define EXIT_SHOW_LOOKING (1 << 1)
/*
 * This function screams bitvector... -gg 6/45/98
 */

const char *Dirs[NUM_OF_DIRS + 1] = { "Север",
    "Восток",
    "Юг",
    "Запад",
    "Верх",
    "Низ",
    "\n"
};

const char *ObjState[8][2] = { {"рассыпается", "рассыпается"},
{"очень плохо", "в очень плохом состоянии"},
{"плохо", "в плохом состоянии"},
{"неплохо", "в неплохом состоянии"},
{"средне", "в рабочем состоянии"},
{"хорошо", "в хорошем состоянии"},
{"очень хорошо", "в очень хорошем состоянии"},
{"великолепно", "в великолепном состоянии"}
};

char *diag_obj_to_char(struct char_data *i, struct obj_data *obj, int mode)
{
    static char out_str[80] = "\0";
    char *color;
    int percent;


    if (GET_OBJ_MAX(obj) > 0)
        percent = 100 * GET_OBJ_CUR(obj) / GET_OBJ_MAX(obj);
    else
        percent = -1;

    if (percent >= 100) {
        percent = 7;
        color = (char *) CCWHT(i, C_NRM);
    } else if (percent >= 90) {
        percent = 6;
        color = (char *) CCIGRN(i, C_NRM);
    } else if (percent >= 75) {
        percent = 5;
        color = (char *) CCGRN(i, C_NRM);
    } else if (percent >= 50) {
        percent = 4;
        color = (char *) CCIYEL(i, C_NRM);
    } else if (percent >= 30) {
        percent = 3;
        color = (char *) CCIRED(i, C_NRM);
    } else if (percent >= 15) {
        percent = 2;
        color = (char *) CCRED(i, C_NRM);
    } else if (percent > 0) {
        percent = 1;
        color = (char *) CCNRM(i, C_NRM);
    } else {
        percent = 0;
        color = (char *) CCINRM(i, C_NRM);
    }
    if (mode == 1)
        sprintf(out_str, " %s<%s>%s", color, ObjState[percent][0], CCNRM(i, C_NRM));
    else if (mode == 2)
        sprintf(out_str, "%s", ObjState[percent][1]);
    return out_str;
}


const char *weapon_class[] = {
    "посохи",
    "секиры",
    "мечи",
    "кинжалы",
    "палицы",
    "кистени",
    "кнуты",
    "копья",
    "луки",
    "арбалеты",
    "двухручное",
    "огнестрельное"
};


char *diag_weapon_to_char(struct obj_data *obj, int show_wear)
{
    static char out_str[MAX_STRING_LENGTH];
    int skill = 0;

    *out_str = '\0';
    switch (GET_OBJ_TYPE(obj)) {
        case ITEM_WEAPON:
            switch (GET_OBJ_SKILL(obj)) {
                case SKILL_STAFFS:
                    skill = 1;
                    break;
                case SKILL_AXES:
                    skill = 2;
                    break;
                case SKILL_SWORDS:
                    skill = 3;
                    break;
                case SKILL_DAGGERS:
                    skill = 4;
                    break;
                case SKILL_MACES:
                    skill = 5;
                    break;
                case SKILL_FLAILS:
                    skill = 6;
                    break;
                case SKILL_WHIPS:
                    skill = 7;
                    break;
                case SKILL_SPAEKS:
                    skill = 8;
                    break;
                case SKILL_BOWS:
                    skill = 9;
                    break;
                case SKILL_CROSSBOWS:
                    skill = 10;
                    break;
                case SKILL_BOTHHANDS:
                    skill = 11;
                    break;
                case SKILL_SHOOT:
                    skill = 12;
                    break;
                default:
                    sprintf(out_str, "Не принадлежит к известным типам оружия!\r\n");
                    break;
            }

            if (skill)
                sprintf(out_str, "%s принадлежит к классу '%s'.\r\n",
                        CAP(GET_OBJ_PNAME(obj, 0)), weapon_class[skill - 1]);
        default:
            if (show_wear) {
                if (CAN_WEAR(obj, ITEM_WEAR_FINGER))
                    sprintf(out_str + strlen(out_str), "Можно надеть на палец.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_NECK))
                    sprintf(out_str + strlen(out_str), "Можно надеть на шею.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_BODY))
                    sprintf(out_str + strlen(out_str), "Можно надеть на туловище.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_FACE))
                    sprintf(out_str + strlen(out_str), "Можно надеть на лицо.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_HEAD))
                    sprintf(out_str + strlen(out_str), "Можно надеть на голову.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_LEGS))
                    sprintf(out_str + strlen(out_str), "Можно надеть на ноги.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_FEET))
                    sprintf(out_str + strlen(out_str), "Можно обуть.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_HANDS))
                    sprintf(out_str + strlen(out_str), "Можно надеть на кисти.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_ARMS))
                    sprintf(out_str + strlen(out_str), "Можно надеть на руки.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))
                    sprintf(out_str + strlen(out_str), "Можно использовать как щит.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))
                    sprintf(out_str + strlen(out_str), "Можно надеть на плечи.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_WAIST))
                    sprintf(out_str + strlen(out_str), "Можно надеть на пояс.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_WRIST))
                    sprintf(out_str + strlen(out_str), "Можно надеть на запястья.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_WIELD) && !CAN_WEAR(obj, ITEM_WEAR_BOTHS))
                    sprintf(out_str + strlen(out_str), "Можно держать в правой руке.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_HOLD) && !CAN_WEAR(obj, ITEM_WEAR_BOTHS))
                    sprintf(out_str + strlen(out_str), "Можно держать в левой руке.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_BOTHS))
                    sprintf(out_str + strlen(out_str), "Можно взять в обе руки.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_TAIL))
                    sprintf(out_str + strlen(out_str), "Можно надеть на хвост.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_EARS))
                    sprintf(out_str + strlen(out_str), "Можно надеть в уши.\r\n");
            }
    }

    return (out_str);
}


char *buf_obj_to_char(struct obj_data *list, struct char_data *ch)
{
    static char out_str[MAX_STRING_LENGTH];
    struct obj_data *i, *push = NULL;
    bool found = FALSE;
    int push_count = 0;

    *out_str = '\0';

    for (i = list; i; i = i->next_content) {
        if (CAN_SEE_OBJ(ch, i)) {
            if (!push) {
                push = i;
                push_count = 1;
            } else
                if ((GET_OBJ_VNUM(push) == -1 &&
                     str_cmp(i->names, push->names)) ||
                    (i->missile && push->missile
                     && i->obj_flags.value[2] != push->obj_flags.value[2])
                    || GET_OBJ_VNUM(i) != GET_OBJ_VNUM(push)) {
                strcat(out_str, show_obj(push, ch, push_count));

                push = i;
                push_count = 1;
            } else
                push_count++;
            found = TRUE;
        }
    }
    if (push && push_count)
        strcat(out_str, show_obj(push, ch, push_count));

    return (out_str);
}


void list_obj_to_char(struct obj_data *list, struct char_data *ch, int mode, int show)
{
    struct obj_data *i, *push = NULL;
    bool found = FALSE;
    int push_count = 0;

    for (i = list; i; i = i->next_content) {
        if (IS_SOUL(ch) && !IS_CORPSE(i))
            continue;

        if (CAN_SEE_OBJ(ch, i) &&
            (!OBJ_FLAGGED(i, ITEM_NOVISIBLE) ||
             (OBJ_FLAGGED(i, ITEM_NOVISIBLE) && PRF_FLAGGED(ch, PRF_HOLYLIGHT)
             )
            )
            ) {
            if (!push) {
                push = i;
                push_count = 1;
            } else
                if ((GET_OBJ_VNUM(push) == -1 &&
                     str_cmp(i->description, push->description)) ||
                    GET_OBJ_VNUM(i) != GET_OBJ_VNUM(push)) {
                show_obj_to_char(push, ch, mode, show, push_count);
                push = i;
                push_count = 1;
            } else
                push_count++;
            found = TRUE;
        }
    }
    if (push && push_count)
        show_obj_to_char(push, ch, mode, show, push_count);

    if (!found && show) {
        if (show == 1)
            send_to_char(" Внутри пусто.\r\n", ch);
        else
            send_to_char("Вы ничего не несете.\r\n", ch);
    }
}

char *diag_char_to_char(struct char_data *i, int type)
{
    int percent;
    static char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

// Смотрим на жизнь

    if (GET_REAL_MAX_HIT(i) > 0)
        percent = (100 * GET_HIT(i)) / GET_REAL_MAX_HIT(i);
    else
        percent = -1;           /* How could MAX_HIT be < 1?? */

    if (type)
        strcpy(buf1, hide_race(i, 0));
    else
        strcpy(buf1, HSSH(i, i));

    PHRASE(buf1);


    if (percent >= 100) {
        sprintf(buf2, " в великолепном состоянии");
        strcat(buf1, buf2);
    } else
     if (percent >= 90) {
        sprintf(buf2, " слегка поцарапан%s", GET_CH_SUF_6(i));
        strcat(buf1, buf2);
    } else if (percent >= 75) {
        sprintf(buf2, " легко ранен%s", GET_CH_SUF_6(i));
        strcat(buf1, buf2);
    } else if (percent >= 50) {
        sprintf(buf2, " ранен%s", GET_CH_SUF_6(i));
        strcat(buf1, buf2);
    } else if (percent >= 30) {
        sprintf(buf2, " тяжело ранен%s", GET_CH_SUF_6(i));
        strcat(buf1, buf2);
    } else if (percent >= 15) {
        sprintf(buf2, " смертельно ранен%s", GET_CH_SUF_6(i));
        strcat(buf1, buf2);
    } else if (percent >= 0)
        strcat(buf1, " в ужасном состоянии");
    else
        strcat(buf1, " умирает");

    /*  if (AFF_FLAGGED(ch, AFF_DETECT_POISON))
       if (AFF_FLAGGED(i, AFF_POISON))
       {sprintf(buf2," (отравлен%s)", GET_CH_SUF_6(i));
       strcat(buf, buf2);
       } */


    if (!IS_NPC(i) || (IS_NPC(i) && AFF_FLAGGED(i, AFF_CHARM))) {


        if (GET_REAL_MAX_MOVE(i) > 0)
            percent = (100 * GET_MOVE(i)) / GET_REAL_MAX_MOVE(i);
        else
            percent = -1;       /* How could MAX_HIT be < 1?? */


        if (percent >= 100) {
            sprintf(buf2, " и выглядит отдохнувш%s", GET_CH_SUF_7(i));
            strcat(buf1, buf2);
        } else
         if (percent >= 90) {
            sprintf(buf2, " и выглядит немного уставш%s", GET_CH_SUF_7(i));
            strcat(buf1, buf2);
        } else if (percent >= 75) {
            sprintf(buf2, " и выглядит уставш%s", GET_CH_SUF_7(i));
            strcat(buf1, buf2);
        } else if (percent >= 50) {
            sprintf(buf2, " и выглядит утомленн%s", GET_CH_SUF_8(i));
            strcat(buf1, buf2);
        } else if (percent >= 30) {
            sprintf(buf2, " и выглядит сильно уставш%s", GET_CH_SUF_7(i));
            strcat(buf1, buf2);
        } else if (percent >= 15) {
            sprintf(buf2, " и выглядит изможденн%s", GET_CH_SUF_8(i));
            strcat(buf1, buf2);
        } else if (percent >= 5) {
            sprintf(buf2, " и выглядит истощенн%s", GET_CH_SUF_8(i));
            strcat(buf1, buf2);
        } else {
            sprintf(buf2, " и валится с ног от усталости");
            strcat(buf1, buf2);
        }
    }
    strcat(buf1, ".");

    return buf1;

}

char *show_height(struct char_data *ch, char h_str[20])
{
    int hei;

    hei = GET_REAL_SIZE(ch);

    if (hei < 0)
        sprintf(h_str, "микроскопического");
    else if (hei < 15)
        sprintf(h_str, "крошечного");
    else if (hei < 30)
        sprintf(h_str, "маленького");
    else if (hei < 45)
        sprintf(h_str, "небольшого");
    else if (hei < 60)
        sprintf(h_str, "среднего");
    else if (hei < 80)
        sprintf(h_str, "высокого");
    else if (hei < 100)
        sprintf(h_str, "гигантского");
    else
        sprintf(h_str, "титанического");

    return h_str;
}

char *show_cha(struct char_data *ch, char c_str[20])
{
    int cha;

    cha = GET_REAL_CHA(ch);

    if (affected_by_spell(ch, SPELL_PLAGUE))
        sprintf(c_str, "покрытом язвами");
    else if (cha < 6)
        sprintf(c_str, "уродливом");
    else if (cha < 8)
        sprintf(c_str, "безобразном");
    else if (cha < 12)
        sprintf(c_str, "ничем не примечательном");
    else if (cha < 16)
        sprintf(c_str, "симпатичном");
    else if (cha < 20)
        sprintf(c_str, "привлекательном");
    else if (cha < 23)
        sprintf(c_str, "красивом");
    else
        sprintf(c_str, "божественной красоты");

    return c_str;
}

char *show_age(struct char_data *ch, char a_str[20])
{
    int agez = GET_REAL_AGE(ch);

    if (agez < 10)
        sprintf(a_str, "детском");
    else if (agez < 20)
        sprintf(a_str, "юном");
    else if (agez < 30)
        sprintf(a_str, "молодом");
    else if (agez < 60)
        sprintf(a_str, "взрослом");
    else if (agez < 95)
        sprintf(a_str, "морщинистом");
    else
        sprintf(a_str, "дряхлом");

    return a_str;
}

char *show_int(struct char_data *ch, char a_str[20])
{
    int intz = GET_REAL_INT(ch);

    if (AFF_FLAGGED(ch, AFF_BLIND))
        sprintf(a_str, "слепые");
    else {
        if (intz < 11)
            sprintf(a_str, "мутные");
        else if (intz < 14)
            sprintf(a_str, "тусклые");
        else if (intz < 17)
            sprintf(a_str, "прозрачные");
        else if (intz < 21)
            sprintf(a_str, "ясные");
        else if (intz < 24)
            sprintf(a_str, "блестящие");
        else
            sprintf(a_str, "проницательные");
    }

    return a_str;
}

char *show_str(struct char_data *ch, char a_str[20])
{
    int str = GET_REAL_STR(ch);


    if (GET_SEX(ch) == SEX_MALE) {
        if (str < 8)
            sprintf(a_str, "хилый");
        else if (str < 14)
            sprintf(a_str, "обычный");
        else if (str < 18)
            sprintf(a_str, "жилистый");
        else if (str < 23)
            sprintf(a_str, "мускулистый");
        else if (str < 25)
            sprintf(a_str, "могучий");
        else
            sprintf(a_str, "накачанный");
    } else if (GET_SEX(ch) == SEX_FEMALE) {
        if (str < 8)
            sprintf(a_str, "хилая");
        else if (str < 14)
            sprintf(a_str, "обычная");
        else if (str < 18)
            sprintf(a_str, "жилистая");
        else if (str < 23)
            sprintf(a_str, "мускулистая");
        else if (str < 25)
            sprintf(a_str, "могучая");
        else
            sprintf(a_str, "накачанная");
    } else if (GET_SEX(ch) == SEX_NEUTRAL) {
        if (str < 8)
            sprintf(a_str, "хилое");
        else if (str < 14)
            sprintf(a_str, "обычное");
        else if (str < 18)
            sprintf(a_str, "жилистое");
        else if (str < 23)
            sprintf(a_str, "мускулистое");
        else if (str < 25)
            sprintf(a_str, "могучее");
        else
            sprintf(a_str, "накачанное");
    } else {
        if (str < 8)
            sprintf(a_str, "хилые");
        else if (str < 14)
            sprintf(a_str, "обычные");
        else if (str < 18)
            sprintf(a_str, "жилистые");
        else if (str < 23)
            sprintf(a_str, "мускулистые");
        else if (str < 25)
            sprintf(a_str, "могучие");
        else
            sprintf(a_str, "накачанные");
    }

    return a_str;
}

char *show_con(struct char_data *ch, char a_str[20])
{
    int str = GET_REAL_CON(ch);

    if (GET_SEX(ch) == SEX_MALE) {
        if (str < 8)
            sprintf(a_str, "костлявый");
        else if (str < 12)
            sprintf(a_str, "худой");
        else if (str < 16)
            sprintf(a_str, "стройный");
        else if (str < 19)
            sprintf(a_str, "в меру упитанный");
        else if (str < 23)
            sprintf(a_str, "крупный");
        else if (str < 26)
            sprintf(a_str, "толстый");
        else
            sprintf(a_str, "жирный");
    } else if (GET_SEX(ch) == SEX_FEMALE) {
        if (str < 8)
            sprintf(a_str, "костлявая");
        else if (str < 12)
            sprintf(a_str, "худая");
        else if (str < 16)
            sprintf(a_str, "стройная");
        else if (str < 19)
            sprintf(a_str, "в меру упитанная");
        else if (str < 23)
            sprintf(a_str, "крупная");
        else if (str < 25)
            sprintf(a_str, "толстая");
        else
            sprintf(a_str, "жирная");
    } else if (GET_SEX(ch) == SEX_NEUTRAL) {
        if (str < 8)
            sprintf(a_str, "костлявое");
        else if (str < 12)
            sprintf(a_str, "худое");
        else if (str < 16)
            sprintf(a_str, "стройное");
        else if (str < 19)
            sprintf(a_str, "в меру упитанное");
        else if (str < 23)
            sprintf(a_str, "крупное");
        else if (str < 25)
            sprintf(a_str, "толстое");
        else
            sprintf(a_str, "жирное");
    } else {
        if (str < 8)
            sprintf(a_str, "костлявые");
        else if (str < 12)
            sprintf(a_str, "худые");
        else if (str < 16)
            sprintf(a_str, "стройные");
        else if (str < 19)
            sprintf(a_str, "в меру упитанные");
        else if (str < 23)
            sprintf(a_str, "крупные");
        else if (str < 25)
            sprintf(a_str, "толстые");
        else
            sprintf(a_str, "жирные");
    }

    return a_str;
}

const char *POS_STATE[] = { " лежит без дыхания",
    " лежит при смерти",
    " умирает",
    " лежит без сознания",
    " спит",
    " отдыхает",
    " сидит",
    " сражается",
    " стоит",
    " летает"
};


const char *list_one_char(struct char_data *i, struct char_data *ch, int skill_mode, int count)
{
    int sector = SECT_CITY;
    int point = 0;
    bool lower = FALSE;
    char *desc;
    static char buf[MAX_STRING_LENGTH];

    const char *positions[] = { "лежит здесь умирая",
        "лежит здесь, при смерти",
        "лежит здесь, без сознания",
        "лежит здесь, в обмороке",
        "спит здесь",
        "отдыхает здесь",
        "сидит здесь",
        "сражается",
        "стоит здесь",
        "летает здесь"
    };

    *buf = '\0';

    // Ездовой с хозяином
    if (IS_HORSE(i) && on_horse(i->master)) {
        if (ch == i->master) {
            *buf = '\0';
            if (PRF_FLAGGED(ch, PRF_ROOMFLAGS) && IS_MOB(i))
                sprintf(buf, "[%6d] ", GET_MOB_VNUM(i));
            sprintf(buf + strlen(buf), "%s несет Вас на своей спине.\r\n", CAP(GET_NAME(i)));
            PHRASE(buf);
        }
        return (buf);
    }


    if (IS_SOUL(i)) {
        if (PRF_FLAGGED(ch, PRF_HOLYLIGHT) || IS_SOUL(ch))
            sprintf(buf, "Призрак %s медленно парит над своим трупом.", GET_PAD(i, 1));

        return (buf);
    } else                      // Не видим персонажа
    if (!CAN_SEE(ch, i)) {
        skill_mode = check_awake(i, ACHECK_AFFECTS | ACHECK_LIGHT | ACHECK_HUMMING |
                                 ACHECK_GLOWING | ACHECK_WEIGHT);

        if (IS_SET(skill_mode, ACHECK_AFFECTS)) {
            REMOVE_BIT(skill_mode, ACHECK_AFFECTS);
            sprintf(buf + strlen(buf), "магический ореол%s", skill_mode ? ", " : " ");
        }
        if (IS_SET(skill_mode, ACHECK_LIGHT) && !IS_AFFECTED(ch, AFF_BLIND)) {
            REMOVE_BIT(skill_mode, ACHECK_LIGHT);
            sprintf(buf + strlen(buf), "яркий свет%s", skill_mode ? ", " : " ");
        }
        if (IS_SET(skill_mode, ACHECK_GLOWING) && IS_SET(skill_mode, ACHECK_HUMMING)
            && !IS_AFFECTED(ch, AFF_DEAFNESS) && !IS_AFFECTED(ch, AFF_BLIND)) {
            REMOVE_BIT(skill_mode, ACHECK_GLOWING);
            REMOVE_BIT(skill_mode, ACHECK_HUMMING);
            sprintf(buf + strlen(buf), "шум и блеск экипировки%s", skill_mode ? ", " : " ");
        }
        if (IS_SET(skill_mode, ACHECK_GLOWING) && !IS_AFFECTED(ch, AFF_BLIND)) {
            REMOVE_BIT(skill_mode, ACHECK_GLOWING);
            sprintf(buf + strlen(buf), "блеск экипировки%s", skill_mode ? ", " : " ");
        }
        if (IS_SET(skill_mode, ACHECK_HUMMING) && !IS_AFFECTED(ch, AFF_DEAFNESS)) {
            REMOVE_BIT(skill_mode, ACHECK_HUMMING);
            sprintf(buf + strlen(buf), "шум экипироки%s", skill_mode ? ", " : " ");
        }
        if (IS_SET(skill_mode, ACHECK_WEIGHT) && !IS_AFFECTED(ch, AFF_DEAFNESS)) {
            REMOVE_BIT(skill_mode, ACHECK_WEIGHT);
            sprintf(buf + strlen(buf), "бряцание металла%s", skill_mode ? ", " : " ");
        }
        strcat(buf, "выдает чье-то присутствие.");
        return (buf);
    }

    *buf = '\0';

    if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
        if (IS_MOB(i))
            sprintf(buf, "[%6d] ", GET_MOB_VNUM(i));
        else
            sprintf(buf, "[%6d] ", GET_LEVEL(i));
    }
    // Наклоности
    if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN)) {
        if (IS_EVILS(i))
            strcat(buf, "(темная аура) ");
        else if (IS_GOODS(i))
            strcat(buf, "(светлая аура) ");
    }

    if (!IS_NPC(i) && same_group(ch, i) && (!i->desc || PLR_FLAGGED(i, PLR_DROPLINK))) {
        sprintf(buf + strlen(buf), "Погруженн%s в транс ", GET_CH_SUF_3(i));
        lower = TRUE;
    }


    if (IS_NPC(i) && !NPC_FLAGGED(i, NPC_MISLEAD) &&
        !i->is_transpt && (!FIGHTING(i)) && i->player.long_descr &&
        //GET_POS(i) == GET_DEFAULT_POS(i) &&
        //IN_ROOM(ch) == IN_ROOM(i) &&
        !AFF_FLAGGED(i, AFF_CHARM) && !IS_MOUNT(i)) {   // Монстр по дефолту
        const char *mess = '\0';

        point = 1;

        if (on_horse(i) && i->npc()->specials.vnum_horse > 0) {
            if ((desc = find_exdesc("верхом", i->player.ex_description)) != NULL)
                sprintf(buf + strlen(buf), "%s ", desc);
            else
                sprintf(buf + strlen(buf), "%s сидит верхом на %s. ",
                        CAP(PERS(i, ch, 0)), PERS(get_horse_on(i), ch, 5));
        } else if (i->trap_object && IN_ROOM(i) == IN_ROOM(i->trap_object))
            sprintf(buf + strlen(buf), "%s %s обездвиженн%s %s.",
                    lower ? GET_NAME(i) : CAP(GET_NAME(i)), positions[(int) GET_POS(i)],
                    GET_CH_SUF_3(i), GET_OBJ_PNAME(i->trap_object, 4));
        else if ((mess = get_line_event(i)))
            sprintf(buf + strlen(buf), "%s %s. ", lower ? GET_NAME(i) : CAP(GET_NAME(i)), mess);
        else if (GET_POS(i) == GET_DEFAULT_POS(i))
            strcat(buf, CAP(string_corrector(i->player.long_descr)));
        else
            sprintf(buf + strlen(buf), "%s %s. ", lower ? GET_NAME(i) : CAP(GET_NAME(i)),
                    positions[(int) GET_POS(i)]);

        if (count > 1)
            sprintf(buf + strlen(buf), "[%d]", count);
    } else
        if (IS_NPC(i) && !NPC_FLAGGED(i, NPC_MISLEAD) &&
            !i->is_transpt && (!FIGHTING(i)) && i->player.charm_descr &&
            //GET_POS(i) == GET_DEFAULT_POS(i) &&
            //IN_ROOM(ch) == IN_ROOM(i) &&
            AFF_FLAGGED(i, AFF_CHARM) && !IS_MOUNT(i)) {
        const char *mess = '\0';

        point = 1;

        if (on_horse(i) && i->npc()->specials.vnum_horse > 0) {
            if ((desc = find_exdesc("верхом", i->player.ex_description)) != NULL)
                sprintf(buf + strlen(buf), "%s ", desc);
            else
                sprintf(buf + strlen(buf), "%s сидит верхом на %s. ",
                        CAP(PERS(i, ch, 0)), PERS(get_horse_on(i), ch, 5));
        } else if (i->trap_object && IN_ROOM(i) == IN_ROOM(i->trap_object))
            sprintf(buf + strlen(buf), "%s %s обездвиженн%s %s.",
                    lower ? GET_NAME(i) : CAP(GET_NAME(i)), positions[(int) GET_POS(i)],
                    GET_CH_SUF_3(i), GET_OBJ_PNAME(i->trap_object, 4));
        else if ((mess = get_line_event(i)))
            sprintf(buf + strlen(buf), "%s %s. ", lower ? GET_NAME(i) : CAP(GET_NAME(i)), mess);
        else if (GET_POS(i) == GET_DEFAULT_POS(i) || GET_POS(i) == POS_STANDING) {
            if (i->master && i->master == ch)
                strcat(buf, CAP(string_corrector(i->player.charm_descr_me)));
            else
                strcat(buf, CAP(string_corrector(i->player.charm_descr)));
        } else
            sprintf(buf + strlen(buf), "%s %s. ", lower ? GET_NAME(i) : CAP(GET_NAME(i)),
                    positions[(int) GET_POS(i)]);

        if (count > 1)
            sprintf(buf + strlen(buf), "[%d]", count);
    } else {                    //Остальные случаи
        if (AFF_FLAGGED(i, AFF_BLIND)) {
            sprintf(buf + strlen(buf), lower ? "и ослепленн%s " : "Ослепленн%s ", GET_CH_SUF_3(i));
            lower = TRUE;
        }
        if (IS_NPC(i) && !NPC_FLAGGED(i, NPC_MISLEAD)) {
            strcat(buf, lower ? GET_NAME(i) : CAP(GET_NAME(i)));
        } else {
            if (check_incognito(i)) {
                if (lower)
                    strcat(buf, DAP(hide_race(i, 0)));
                else
                    strcat(buf, CAP(hide_race(i, 0)));
            } else {
                if (lower)
                    strcat(buf,
                           (!GET_TITLE(i)
                            || !*GET_TITLE(i)) ? DAP(race_or_title(i, 0)) : race_or_title(i, 0));
                else
                    strcat(buf, CAP(race_or_title(i, 0)));
            }

        }
        PHRASE(buf);


        if ((affected_by_spell(i, SPELL_FIRE_BLADES) ||
             (affected_by_spell(i, SPELL_FIRE_SHIELD) &&
              affected_by_spell(i, SPELL_BLADE_BARRIER))))
            sprintf(buf + strlen(buf), ", окруженн%s барьером огненных лезвий,", GET_CH_SUF_3(i));
        else if (affected_by_spell(i, SPELL_BLADE_BARRIER))
            sprintf(buf + strlen(buf), ", окруженн%s барьером из лезвий,", GET_CH_SUF_3(i));

        //strcat(buf," ");

        if (!FIGHTING(i)) {
            const char *mess = '\0';

            if ((mess = get_line_event(i)))
                sprintf(buf + strlen(buf), " %s", mess);
            else if (on_horse(i))
                sprintf(buf + strlen(buf), " сидит верхом на %s", PERS(get_horse_on(i), ch, 5));
            else if (i->is_transpt)
                sprintf(buf + strlen(buf), " сидит в %s%s",
                        GET_OBJ_PNAME(i->is_transpt, 5),
                        (i->is_transpt == ch->is_transpt) ? " рядом с Вами" : "");
            else if (IS_NPC(i) && i->npc()->specials.transpt)
                sprintf(buf + strlen(buf), " запряжен%s в %s", GET_CH_SUF_6(i),
                        GET_OBJ_PNAME(i->npc()->specials.transpt, 3));
            else if (IS_MOUNT(i) && AFF_FLAGGED(i, AFF_TETHERED))
                sprintf(buf + strlen(buf), " привязан%s здесь", GET_CH_SUF_6(i));
            else if (GET_POS(i) > POS_SLEEPING && AFF_FLAGGED(i, AFF_HIDE))
                sprintf(buf + strlen(buf), " прячется здесь");
            else if (GET_POS(i) > POS_SLEEPING && AFF_FLAGGED(i, AFF_CAMOUFLAGE))
                sprintf(buf + strlen(buf), " замаскировал%s здесь", GET_CH_SUF_2(i));
            else if ((sector = real_sector(IN_ROOM(i))) == SECT_FLYING)
                strcat(buf, " летает здесь");
            else if (sector == SECT_UNDERWATER)
                strcat(buf, " плавает здесь");
            else if (GET_POS(i) == POS_FLYING && AFF_FLAGGED(i, AFF_FLY))
                sprintf(buf + strlen(buf), " летает над %s",
                        (sector == SECT_WATER_SWIM
                         || sector == SECT_WATER_NOSWIM) ? "водой" : "землей");
            else if (GET_POS(i) == POS_FLYING && AFF_FLAGGED(i, AFF_LEVIT))
                sprintf(buf + strlen(buf), " плавно парит над %s",
                        (sector == SECT_WATER_SWIM
                         || sector == SECT_WATER_NOSWIM) ? "водой" : "землей");
            else if (sector == SECT_WATER_SWIM || sector == SECT_WATER_NOSWIM) {
                if (AFF_FLAGGED(i, AFF_WATERWALK))
                    strcat(buf, " стоит на воде");
                else
                    strcat(buf, " плавает здесь");
            } else
                sprintf(buf + strlen(buf), " %s", positions[(int) GET_POS(i)]);

            if (AFF_FLAGGED(i, AFF_HOLYLIGHT))
                sprintf(buf + strlen(buf), ", освещая комнату");

            if (AFF_FLAGGED(i, AFF_INVISIBLE))
                sprintf(buf + strlen(buf), ", укрывшись заклинанием невидимости");
            if (i->trap_object && IN_ROOM(i) == IN_ROOM(i->trap_object))
                sprintf(buf + strlen(buf), ", обездвиженн%s %s", GET_CH_SUF_3(i),
                        GET_OBJ_PNAME(i->trap_object, 4));
        } else {
            if (GET_POS(i) < POS_SITTING)
                sprintf(buf + strlen(buf), " %s", positions[(int) GET_POS(i)]);
            else {
                if (GET_POS(i) == POS_SITTING)
                    strcat(buf, " сидит здесь и сражается c ");
                else
                    strcat(buf, " сражается c ");

                if (i->in_room != FIGHTING(i)->in_room) {
                    strcat(buf, "чьей-то тенью");
                    stop_fighting(i, TRUE);
                } else if (FIGHTING(i) == ch)
                    strcat(buf, "ВАМИ");
                else {
                    if (check_incognito(FIGHTING(i)))
                        strcat(buf, hide_race(FIGHTING(i), 4));
                    else
                        strcat(buf, PERS(FIGHTING(i), ch, 4));
                }
                point = TRUE;
                strcat(buf, "!");
            }
        }

    }

    if (AFF_FLAGGED(ch, AFF_DETECT_POISON))
        if (AFF_FLAGGED(i, AFF_POISON))
            sprintf(buf + strlen(buf), " (отравлен%s)", GET_CH_SUF_6(i));

//      if (!IS_NPC(i) && same_group(ch,i) && (!i->desc || PLR_FLAGGED(i,PLR_DROPLINK)))
//       sprintf(buf+strlen(buf), " (потерял%s связь)", GET_CH_SUF_1(i));

    if (!IS_NPC(i) && PLR_FLAGGED(i, PLR_WRITING))
        strcat(buf, " (пишет)");

    if (!point)
        strcat(buf, ".");

    return (buf);
}

void list_char_to_char(struct char_data *list, struct char_data *ch)
{
    struct char_data *i;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];

    *buf1 = '\0';
    *buf2 = '\0';
    *buf3 = '\0';


    for (i = list; i; i = i->next_in_room) {
        if (PLR_FLAGGED(i, PLR_SOUL) &&
            !(PLR_FLAGGED(ch, PLR_SOUL) || (PRF_FLAGGED(ch, PRF_HOLYLIGHT))))
            continue;

        if (IS_NPC(i) && NPC_FLAGGED(i, NPC_HIDDEN) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT))
            continue;

        if (ch != i && IS_NPC(i)) {
            if (NPC_FLAGGED(i, NPC_ISOBJECT)) {
                if (!PRF_FLAGGED(ch, PRF_THEME))
                    strcat(buf3, CCIBLK(ch, C_NRM));
                else
                    strcat(buf3, CCIYEL(ch, C_NRM));

                if (PLR_FLAGGED(ch, PLR_SOUL)) {
                    //Мобы-предметы не видим.
                } else
                    if (IS_DARK(IN_ROOM(i)) &&
                        (PRF_FLAGGED(ch, PRF_HOLYLIGHT) || AFF_FLAGGED(ch, AFF_DARKVISION))) {
                    strcat(buf3, list_one_char(i, ch, 0, 1));
                    if (!(IS_HORSE(i) && on_horse(i->master)))
                        strcat(buf3, "\r\n");
                } else
                    if (((CAN_SEE(ch, i) &&
                          ((!AFF_FLAGGED(i, AFF_HIDE) && !AFF_FLAGGED(i, AFF_SNEAK)
                            && !AFF_FLAGGED(i, AFF_CAMOUFLAGE)) || (check_victim_visible(ch, i)
                                                                    || PRF_FLAGGED(ch,
                                                                                   PRF_HOLYLIGHT)
                                                                    || AFF_FLAGGED(ch,
                                                                                   AFF_SENSE_LIFE))))
                         || awaking(i, AW_HIDE | AW_INVIS | AW_CAMOUFLAGE))) {
                    strcat(buf3, list_one_char(i, ch, 0, 1));
                    if (!(IS_HORSE(i) && on_horse(i->master)))
                        strcat(buf3, "\r\n");
                }

                if (!PRF_FLAGGED(ch, PRF_THEME))
                    strcat(buf3, CCYEL(ch, C_NRM));
                else
                    strcat(buf3, CCIRED(ch, C_NRM));
            } else {
                if (IS_SOUL(ch) && !IS_SOUL(i)) {
                    sprintf(buf1 + strlen(buf1), "Силуэт %s едва различим в темноте.\r\n",
                            GET_SEX(i) == SEX_FEMALE ?
                            get_name_pad((char *) race_name_pad_female[(int) GET_RACE(i)], PAD_ROD,
                                         PAD_MONSTER) : get_name_pad((char *)
                                                                     race_name_pad_male[(int)
                                                                                        GET_RACE
                                                                                        (i)],
                                                                     PAD_ROD, PAD_MONSTER));
                } else if (CAN_SEE(ch, i)
                           && (IS_DARK(IN_ROOM(i)) && !AFF_FLAGGED(ch, AFF_DARKVISION))
                           && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)
                           &&
                           ((!AFF_FLAGGED(i, AFF_HIDE) && !AFF_FLAGGED(i, AFF_SNEAK)
                             && !AFF_FLAGGED(i, AFF_CAMOUFLAGE)) || (check_victim_visible(ch, i)
                                                                     || PRF_FLAGGED(ch,
                                                                                    PRF_HOLYLIGHT)
                                                                     || AFF_FLAGGED(ch,
                                                                                    AFF_SENSE_LIFE))
                            || awaking(i, AW_HIDE | AW_INVIS | AW_CAMOUFLAGE)))
                {
                    sprintf(buf1 + strlen(buf1), "Силуэт %s едва различим в темноте.\r\n",
                            GET_SEX(i) == SEX_FEMALE ?
                            get_name_pad((char *) race_name_pad_female[(int) GET_RACE(i)], PAD_ROD,
                                         PAD_MONSTER) : get_name_pad((char *)
                                                                     race_name_pad_male[(int)
                                                                                        GET_RACE
                                                                                        (i)],
                                                                     PAD_ROD, PAD_MONSTER));
                } else
                    if (((CAN_SEE(ch, i)
                          &&
                          ((!AFF_FLAGGED(i, AFF_HIDE) && !AFF_FLAGGED(i, AFF_SNEAK)
                            && !AFF_FLAGGED(i, AFF_CAMOUFLAGE)) || (check_victim_visible(ch, i)
                                                                    || PRF_FLAGGED(ch,
                                                                                   PRF_HOLYLIGHT)
                                                                    || AFF_FLAGGED(ch,
                                                                                   AFF_SENSE_LIFE))))
                         || awaking(i, AW_HIDE | AW_INVIS | AW_CAMOUFLAGE))) {
                    strcat(buf1, list_one_char(i, ch, 0, 1));
                    if (!(IS_HORSE(i) && on_horse(i->master)))
                        strcat(buf1, "\r\n");
                } else
                    if (IS_DARK(i->in_room) &&
                        IN_ROOM(i) == IN_ROOM(ch) && !CAN_SEE_IN_DARK(ch) &&
                        (AFF_FLAGGED(i, AFF_INFRAVISION)
                         || affected_by_spell(i, SPELL_INFRAVISION)))
                    strcat(buf1, "Горячее дыхание выдает чье-то присутствие.\r\n");
            }
        }                       //конец NPC
        else if (ch != i && !IS_NPC(i)) {
            if (IS_SOUL(ch)) {
                if (IS_SOUL(i))
                    sprintf(buf2 + strlen(buf2), "%s\r\n", list_one_char(i, ch, 0, 1));
                else
                    sprintf(buf2 + strlen(buf2), "Силуэт %s едва различим в темноте.\r\n",
                            GET_SEX(i) == SEX_FEMALE ?
                            get_name_pad((char *) race_name_pad_female[(int) GET_RACE(i)], PAD_ROD,
                                         PAD_MONSTER) : get_name_pad((char *)
                                                                     race_name_pad_male[(int)
                                                                                        GET_RACE
                                                                                        (i)],
                                                                     PAD_ROD, PAD_MONSTER));
            } else if (CAN_SEE(ch, i) && (IS_DARK(IN_ROOM(i)) && !AFF_FLAGGED(ch, AFF_DARKVISION))
                       && !PRF_FLAGGED(ch, PRF_HOLYLIGHT)
                       &&
                       ((!AFF_FLAGGED(i, AFF_HIDE) && !AFF_FLAGGED(i, AFF_SNEAK)
                         && !AFF_FLAGGED(i, AFF_CAMOUFLAGE)) || (check_victim_visible(ch, i)
                                                                 || PRF_FLAGGED(ch, PRF_HOLYLIGHT)
                                                                 || AFF_FLAGGED(ch, AFF_SENSE_LIFE))
                        || awaking(i, AW_HIDE | AW_INVIS | AW_CAMOUFLAGE)
                       )
                ) {
                sprintf(buf2 + strlen(buf2), "Силуэт %s едва различим в темноте.\r\n",
                        GET_SEX(i) == SEX_FEMALE ?
                        get_name_pad((char *) race_name_pad_female[(int) GET_RACE(i)], PAD_ROD,
                                     PAD_MONSTER) : get_name_pad((char *) race_name_pad_male[(int)
                                                                                             GET_RACE
                                                                                             (i)],
                                                                 PAD_ROD, PAD_MONSTER));
            } else
                if (((CAN_SEE(ch, i)
                      &&
                      ((!AFF_FLAGGED(i, AFF_HIDE) && !AFF_FLAGGED(i, AFF_SNEAK)
                        && !AFF_FLAGGED(i, AFF_CAMOUFLAGE)) || (check_victim_visible(ch, i)
                                                                || PRF_FLAGGED(ch, PRF_HOLYLIGHT)
                                                                || AFF_FLAGGED(ch,
                                                                               AFF_SENSE_LIFE))))
                     || awaking(i, AW_HIDE | AW_INVIS | AW_CAMOUFLAGE))) {
                strcat(buf2, list_one_char(i, ch, 0, 1));
                if (!(IS_HORSE(i) && on_horse(i->master)))
                    strcat(buf2, "\r\n");
            } else
                if (IS_DARK(i->in_room) &&
                    IN_ROOM(i) == IN_ROOM(ch) && !CAN_SEE_IN_DARK(ch) &&
                    (AFF_FLAGGED(i, AFF_INFRAVISION) || affected_by_spell(i, SPELL_INFRAVISION)))
                strcat(buf2, "Горячее дыхание выдает чье-то присутствие.\r\n");
        }
    }
    send_to_charf(ch, buf3);
    send_to_charf(ch, buf1);
    send_to_charf(ch, buf2);
}

const char *dirs_r[] = {
    "С",
    "В",
    "Ю",
    "З",
    "П",
    "О",
    "\n"
};

const char *dirs_e[] = {
    "N",
    "E",
    "S",
    "W",
    "U",
    "D",
    "\n"
};

char *do_auto_exits(struct char_data *ch)
{
    int door, slen = 0;
    char buf[MAX_STRING_LENGTH];
    static char buf2[MAX_STRING_LENGTH];

    *buf = '\0';

    for (door = 0; door < NUM_OF_DIRS; door++) {
        if (EXIT(ch, door) &&
            EXIT(ch, door)->to_room != NOWHERE && !DOOR_FLAGGED(EXIT(ch, door), EXIT_HIDDEN)) {
            if (!DOOR_FLAGGED(EXIT(ch, door), EXIT_CLOSED)) {
                if (PRF_FLAGGED(ch, PRF_EXITRUS))
                    slen += sprintf(buf + slen, "%s ", dirs_r[door]);
                else
                    slen += sprintf(buf + slen, "%s ", dirs_e[door]);
            } else {
                if (PRF_FLAGGED(ch, PRF_EXITRUS))
                    slen += sprintf(buf + slen, "(%s) ", dirs_r[door]);
                else
                    slen += sprintf(buf + slen, "(%s) ", dirs_e[door]);
            }
        } else {
            if (EXIT(ch, door) && EXIT(ch, door)->timer) {
                if (PRF_FLAGGED(ch, PRF_EXITRUS))
                    slen += sprintf(buf + slen, "%s ", dirs_r[door]);
                else
                    slen += sprintf(buf + slen, "%s ", dirs_e[door]);
            }
        }
    }

    if (PRF_FLAGGED(ch, PRF_EXITRUS))
        sprintf(buf2, "%s[ Выходы: %s]%s\r\n", CCCYN(ch, C_NRM),
                *buf ? buf : "Нет ", CCNRM(ch, C_NRM));
    else
        sprintf(buf2, "%s[ Exits: %s]%s\r\n", CCCYN(ch, C_NRM),
                *buf ? buf : "None ", CCNRM(ch, C_NRM));

    return (buf2);
}

ACMD(do_where_im)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf, "Вы находитесь в зоне: %s\r\n", zone_table[world[IN_ROOM(ch)].zone].name);
	send_to_char(buf, ch);
}

ACMD(do_exits)
{
    int door;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if (AFF_FLAGGED(ch, AFF_BLIND)) {
        send_to_char("Вы слепы!\r\n", ch);
        return;
    }
    for (door = 0; door < NUM_OF_DIRS; door++)
        if (EXIT(ch, door) &&
            EXIT(ch, door)->to_room != NOWHERE && !DOOR_FLAGGED(EXIT(ch, door), EXIT_CLOSED)
            ) {
            if (IS_GOD(ch))
                sprintf(buf2, "%-5s - [%6d] %s\r\n", Dirs[door],
                        GET_ROOM_VNUM(EXIT(ch, door)->to_room),
                        world[EXIT(ch, door)->to_room].name);
            else {
                sprintf(buf2, "%-5s - ", Dirs[door]);
                if (IS_DARK(EXIT(ch, door)->to_room) && !CAN_SEE_IN_DARK(ch))
                    strcat(buf2, "слишком темно\r\n");
                else {
                    strcat(buf2, world[EXIT(ch, door)->to_room].name);
                    strcat(buf2, "\r\n");
                }
            }
            strcat(buf, CAP(buf2));
        }
    send_to_char("Видимые выходы:\r\n", ch);
    if (*buf)
        send_to_char(buf, ch);
    else
        send_to_char(" Нету!\r\n", ch);
}


#define MAX_FIRES 6
const char *Fires[MAX_FIRES] = { "тлеет небольшая кучка угольков",
    "тлеет небольшая кучка угольков",
    "еле-еле теплиться огонек",
    "догорает небольшой костер",
    "весело горит костер",
    "ярко пылает костер"
};

#define TAG_NIGHT       "<night>"
#define TAG_DAY         "<day>"

int paste_description(char *string, const char *tag, int need)
{
    char *pos;
    char buf[MAX_STRING_LENGTH];

    if (!*string || !*tag)
        return (FALSE);

    if ((pos = str_str(string, tag))) {
        if (need) {
            for (; *pos && *pos != '>'; pos++);
            if (*pos)
                pos++;
            if (*pos == 'R') {
                pos++;
                buf[0] = '\0';
            }
            strcpy(buf, pos);
            if ((pos = str_str(buf, tag)))
                *pos = '\0';
            return (TRUE);
        } else {
            *pos = '\0';
            if ((pos = str_str(string, tag)))
                strcat(string, pos + strlen(tag));
        }
    }
    return (FALSE);
}


char *show_extend_room(char *description, struct char_data *ch)
{
    int found = FALSE, i;
    char string[MAX_STRING_LENGTH], *pos;
    static char buf[MAX_STRING_LENGTH];

    if (!description || !*description)
        return NULL;

    strcpy(string, description);
    if ((pos = strchr(description, '<')))
        *pos = '\0';
    strcpy(buf, description);
    if (pos)
        *pos = '<';


    i = world[IN_ROOM(ch)].zone;
    found = found || paste_description(string, TAG_NIGHT,
                                       (zone_table[i].weather_info.sunlight == SUN_SET
                                        || zone_table[i].weather_info.sunlight == SUN_DARK));
    found = found
        || paste_description(string, TAG_DAY,
                             (zone_table[i].weather_info.sunlight == SUN_RISE
                              || zone_table[i].weather_info.sunlight == SUN_LIGHT));

    for (i = strlen(buf); i > 0 && *(buf + i) == '\n'; i--) {
        *(buf + i) = '\0';
        if (i > 0 && *(buf + i) == '\r')
            *(buf + --i) = '\0';
    }

    return buf;
}

void look_at_room(struct char_data *ch, int ignore_brief)
{
    char buf_room[8912];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    int n;

    if (!ch->desc)
        return;

    if (GET_POS(ch) <= POS_SLEEPING)
        return;

    if ((IS_DARK(ch->in_room) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT) &&
         !IS_AFFECTED(ch, AFF_DARKVISION)) || AFF_FLAGGED(ch, AFF_BLIND)) {
        send_to_char("Слишком темно...\r\n", ch);
        if (AFF_FLAGGED(ch, AFF_BLIND))
            return;
        *buf_room = '\0';
        *buf = '\0';
    } else {
        if (!IS_SOUL(ch)) {
            if (!PRF_FLAGGED(ch, PRF_THEME))
                strcpy(buf_room, CCCYN(ch, C_NRM));
            else
                strcpy(buf_room, CCICYN(ch, C_NRM));

            if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
                sprintbits(world[ch->in_room].room_flags, room_bits, buf, ";");
                sprintf(buf2, "[%6d] %s [ %s]\r\n", GET_ROOM_VNUM(IN_ROOM(ch)),
                        world[ch->in_room].name, buf);
                strcat(buf_room, buf2);
            } else if (IS_CLOUD(IN_ROOM(ch)) && (!PRF_FLAGGED(ch, PRF_HOLYLIGHT)))
                strcat(buf_room, "Густой туман\r\n");
            else if (!IS_DARK_CHECK(IN_ROOM(ch)) || PRF_FLAGGED(ch, PRF_HOLYLIGHT))
			{
				if (PRF_FLAGGED(ch, PRF_MAPPER))
				{
					sprintf(buf_room + strlen(buf_room), "%s [%d]\r\n",
                        get_name_pad(world[ch->in_room].name, PAD_IMN, PAD_OBJECT), GET_ROOM_VNUM(IN_ROOM(ch)));
				}
				else
				{
                sprintf(buf_room + strlen(buf_room), "%s\r\n",
                        get_name_pad(world[ch->in_room].name, PAD_IMN, PAD_OBJECT));
				}
			}

            strcat(buf_room, CCNRM(ch, C_NRM));
        } else
            strcpy(buf_room, "");

        if (IS_DARK(IN_ROOM(ch)) &&
            (!PRF_FLAGGED(ch, PRF_HOLYLIGHT) && !IS_AFFECTED(ch, AFF_DARKVISION)))
            ignore_brief = TRUE;

        if (!PRF_FLAGGED(ch, PRF_BRIEF) || ignore_brief) {
            if (IS_SOUL(ch))
                strcat(buf_room, "Сгустившиеся тени мешают как следует оглядеться.\r\n");
            else if (IS_DARK(IN_ROOM(ch)) &&
                     (!PRF_FLAGGED(ch, PRF_HOLYLIGHT) && !IS_AFFECTED(ch, AFF_DARKVISION)))
                sprintf(buf_room + strlen(buf_room), "Слишком темно...\r\n");
            else if (IS_CLOUD(IN_ROOM(ch)) && (!PRF_FLAGGED(ch, PRF_HOLYLIGHT)))
                strcat(buf_room,
                       "Сгустившийся над городом туман мешает Вам что-либо разглядеть.\r\n");
            else
                /*
                   if (ROOM_FLAGGED(ch->in_room, ROOM_DEATH) ||
                   (ROOM_FLAGGED(ch->in_room, ROOM_FLYDEATH) &&
                   !IS_AFFECTED(ch, AFF_FLY))) */

            if ((zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_SET ||
                     zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_DARK) &&
                    world[IN_ROOM(ch)].description_night)
                strcat(buf_room,
                       strbraker(world[ch->in_room].description_night, ch->sw,
                                 PRF_FLAGGED(ch, PRF_AUTOFRM)));
            else if (IS_SOUL(ch))
                strcat(buf_room, "Сгустившиеся тени мешают как следует оглядеться.\r\n");
            else if (world[ch->in_room].description)
                strcat(buf_room,
                       strbraker(world[ch->in_room].description, ch->sw,
                                 PRF_FLAGGED(ch, PRF_AUTOFRM)));
            else
                strcat(buf_room,
                       strbraker
                       ("Данная зона находится в процессе разработки, поэтому билдер еще не успел добавить сюда описание.",
                        ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)));

        }
    }
    /* autoexits */
    if (PRF_FLAGGED(ch, PRF_AUTOEXIT))
        strcat(buf_room, do_auto_exits(ch));


    //ADD BY SLOWN
    /* Показываем кровь в комнате */
    if (RM_BLOOD(ch->in_room) > 0 && !IS_SOUL(ch)) {
        sprintf(buf, "%s%s%s\r\n",
                CCRED(ch, C_NRM), blood_messages[(int) RM_BLOOD(ch->in_room)], CCNRM(ch, C_NRM));
        strcat(buf_room, buf);
    }

    if (world[IN_ROOM(ch)].fires && !IS_SOUL(ch)) {
        sprintf(buf, "%sВ центре %s.%s\r\n",
                CCRED(ch, C_NRM),
                Fires[MIN(world[IN_ROOM(ch)].fires, MAX_FIRES - 1)], CCNRM(ch, C_NRM));
        strcat(buf_room, buf);
    }

    if (IN_ROOM(ch) != NOWHERE && !ROOM_FLAGGED(IN_ROOM(ch), ROOM_NOWEATHER) && !IS_SOUL(ch)) {
        *buf = '\0';
        switch (real_sector(IN_ROOM(ch))) {
            case SECT_FIELD_SNOW:
            case SECT_FOREST_SNOW:
            case SECT_HILLS_SNOW:
            case SECT_MOUNTAIN_SNOW:
                sprintf(buf, "Снежный ковер лежит у Вас под ногами.\r\n");
                break;
            case SECT_FIELD_RAIN:
            case SECT_FOREST_RAIN:
            case SECT_HILLS_RAIN:
                sprintf(buf, "Месиво из грязи у Вас под ногами.\r\n");
                break;
            case SECT_THICK_ICE:
                sprintf(buf, "У Вас под ногами толстый лед.\r\n");
                break;
            case SECT_NORMAL_ICE:
                sprintf(buf, "У Вас под ногами лед.\r\n");
                break;
            case SECT_THIN_ICE:
                sprintf(buf, "У Вас под ногами тонкий лед.\r\n");
                break;
        }                       // switch

        if (*buf)
            strcat(buf_room, buf);
    }

    *buf = '\0';
    for (n = 0; n < NUM_OF_DIRS; n++)
        if (EXITDATA(IN_ROOM(ch), n) && world[IN_ROOM(ch)].dir_option[n]->timer
            && world[IN_ROOM(ch)].dir_option[n]->portal_description)
            sprintf(buf, "%s%s%s\r\n",
                    CCIMAG(ch, C_NRM),
                    world[IN_ROOM(ch)].dir_option[n]->portal_description, CCNRM(ch, C_NRM));
        else if (AFF_FLAGGED(ch, AFF_DETECT_MAGIC) &&
                 EXITDATA(IN_ROOM(ch), n) && world[IN_ROOM(ch)].dir_option[n]->portal_description)
            sprintf(buf, "%sВы заметили небольшое свечение %s.%s\r\n",
                    CCMAG(ch, C_NRM), DirsTo_2[n], CCNRM(ch, C_NRM));
    strcat(buf_room, buf);


    if (world[IN_ROOM(ch)].portal_time && !IS_SOUL(ch)) {
        sprintf(buf, "%sЛазурная пентаграмма переливается здесь.%s\r\n",
                CCIBLU(ch, C_NRM), CCNRM(ch, C_NRM));
        strcat(buf_room, buf);
    }

    send_to_char(buf_room, ch);

    if (!PRF_FLAGGED(ch, PRF_THEME))
        send_to_char(CCIBLK(ch, C_NRM), ch);
    else
        send_to_char(CCIYEL(ch, C_NRM), ch);

    if (affected_room_by_spell(&world[IN_ROOM(ch)], SPELL_PRISMA_SPHERE)) {
        if (check_psphere_char(ch, &world[IN_ROOM(ch)]))
            send_to_charf(ch, "Призматическая сфера закрывает Вас от различных атак.\r\n");
        else
            send_to_charf(ch, "Призматическая сфера переливается всеми цветами радуги.\r\n");
    }
    //Показываем обьекты поисков
    show_locates(ch);
    if (affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_THRDEATH))
        send_to_charf(ch, "Кучка пепла коптит на всю округу мешая всем нормально дышать.\r\n");

    list_obj_to_char(world[ch->in_room].contents, ch, 0, FALSE);

    if (!PRF_FLAGGED(ch, PRF_THEME))
        send_to_char(CCYEL(ch, C_NRM), ch);
    else
        send_to_char(CCIRED(ch, C_NRM), ch);
    list_char_to_char(world[ch->in_room].people, ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
}

/*
<Zodgrot> 0 - стоит большой орк (прямой алиас)
[00:24] <Zodgrot> стоит груумш
[00:24] <Zodgrot> -1- стоит большой орк (на сей раз раса и пол)
[00:24] <Zodgrot> стоит большой божественный орк
[00:24] <Zodgrot> -2- стоит большой орк (раса)
[00:24] <Zodgrot> стоит большой орк
[00:24] <Zodgrot> -3- виден большой силуэт
[00:24] <Zodgrot> виден чейто большой силуэт
[00:24] <Zodgrot> -4- виден неясный силуэт
[00:24] <Zodgrot> виден чейто неясный силуэт
[00:24] <Zodgrot> -5- нихрена не видно
*/
//Показываем персонажу ch персонажа tch в зависмости от освещения light

const char *in_size[9][NUM_SEXES] = {
    {"микроскопическое", "миксроскопический", "микроскопическая", "микроскопические"},
    {"крошечное", "крошечный", "крошечная", "крошечные"},
    {"маленькое", "маленький", "маленькая", "маленькие"},
    {"небольшое", "небольшой", "небольшая", "небольшие"},
    {"крупное", "крупный", "крупная", "крупные"},
    {"гигантское", "гигантский", "гигантская", "гигантские"},
    {"огромное", "огромный", "огромная", "огромные"},
    {"исполинское", "исполинский", "исполинская", "исполинские"},
    {"титанические", "титанический", "титаническая", "титанические"},
};

const char *in_go[9] = {
    "идет",
    "бежит",
    "плывет",
    "прыгает",
    "летает",
    "ползает",
    "скачет",
    "катится",
    "ковыляет"
};

char *show_in_height(struct char_data *ch, char h_str[20], int sex)
{
    int hei;

    hei = GET_REAL_SIZE(ch);

    if (hei < 5)
        sprintf(h_str, in_size[0][sex]);
    else if (hei < 15)
        sprintf(h_str, in_size[1][sex]);
    else if (hei < 30)
        sprintf(h_str, in_size[2][sex]);
    else if (hei < 45)
        sprintf(h_str, in_size[3][sex]);
    else if (hei < 60)
        sprintf(h_str, in_size[4][sex]);
    else if (hei < 75)
        sprintf(h_str, in_size[5][sex]);
    else if (hei < 90)
        sprintf(h_str, in_size[6][sex]);
    else if (hei < 105)
        sprintf(h_str, in_size[7][sex]);
    else
        sprintf(h_str, in_size[8][sex]);

    return h_str;
}


void exam_at_char(struct char_data *i, struct char_data *ch, char *arg)
{
    int j, found, push_count = 0;
    char r[80], h[20], a[20], c[20], s[20], t[20], *desc;
    struct obj_data *tmp_obj, *push = NULL;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if (!ch->desc)
        return;

    if (*arg) {
        int pos;

        if ((desc = find_exdesc(arg, i->player.ex_description)) != NULL) {
            send_to_charf(ch, strbraker(desc, ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)));
            return;
        }

        if ((pos = get_obj_pos_in_equip_vis(ch, arg, NULL, i->equipment)) >= 0) {
            struct obj_data *object = GET_EQ(i, pos);

            if (PRF_FLAGGED(i, PRF_EXAMINE)) {
                act("2и не позволил2(,а,о,и) Вам осмотреть 2ер @1в.", "Ммп", ch, i, object);
                return;
            }

            act("Подойдя к 2д, Вы внимательно осмотрели 2ер @1в.", "Ммп", ch, i, object);
            act("1+и подошел1(,а,о,и) к Вам и внимательно осмотрел1(,а,о,и) Ваш@1(,у,е,и) @1в.",
                "мМп", ch, i, object);
            act("1+и подошел1(,а,о,и) к 2+д и внимательно осмотрел1(,а,о,и) 2ер @1в.", "Кммп", ch,
                i, object);

            if (object->main_description)
                strcpy(buf,
                       strbraker(string_corrector(object->main_description), ch->sw,
                                 PRF_FLAGGED(ch, PRF_AUTOFRM)));
            else
                strcpy(buf, "Ничего особенного.\r\n");

            if (affected_object_by_spell(object, SPELL_MAGIC_VESTMENT) ||
                affected_object_by_spell(object, SPELL_MAGIC_VESTMENT_P) ||
                affected_object_by_spell(object, SPELL_MAGIC_WEAPON)) {
                if (IS_OBJ_STAT(object, ITEM_GOODAURA))
                    sprintf(buf + strlen(buf), " Небесно-голубая аура окружает %s.",
                            GET_OBJ_PNAME(object, 3));
                if (IS_OBJ_STAT(object, ITEM_DARKAURA))
                    sprintf(buf + strlen(buf), " Зловеще-зеленая аура окружает %s.",
                            GET_OBJ_PNAME(object, 3));
                if (IS_OBJ_STAT(object, ITEM_DEATHAURA))
                    sprintf(buf + strlen(buf), " Темно-призрачная аура окружает %s.",
                            GET_OBJ_PNAME(object, 3));
            }

            strcat(buf, diag_weapon_to_char(object, TRUE));

            if ((GET_OBJ_TYPE(object) == ITEM_DRINKCON) || (GET_OBJ_TYPE(object) == ITEM_FOUNTAIN)) {
                sprintf(buf + strlen(buf), "%s\r\n", look_in_drink(ch, object, 0));
            }

            if (!OBJ_FLAGGED(object, ITEM_NODECAY) && GET_OBJ_TYPE(object) != ITEM_FURNITURE
                && !IS_CORPSE(object)) {
                sprintf(buf + strlen(buf), "%s %s.", CAP(GET_OBJ_PNAME(object, 0)),
                        diag_obj_to_char(ch, object, 2));
            }
            send_to_charf(ch, "%s\r\n", buf);
            return;
        }

        send_to_charf(ch, "У %s нет '%s'.\r\n", GET_PAD(i, 1), arg);
        return;
    }

    if (ch != i) {
        if (IS_GOD(ch))
            act("Вам стало жутко, когда 1*и взглянул1(,а,о,и) на Вас.", "мМ", ch, i);
        else
            act("1*и взглянул1(,а,о,и) на Вас.", "мМ", ch, i);

        act("1*и внимательно осмотрел1(,а,о,и) 2*в.", "Кмм", ch, i);
    } else
        act("1*и осмотрел1(,а,о,и) себя.", "Км", ch);

    if (!AFF_FLAGGED(i, PRF_COMPACT) || !AFF_FLAGGED(i, PRF_BRIEF))
        sprintf(buf + strlen(buf), "\r\n");

    if (IS_UNDEAD(i) && !i->player.description) {
        int tsex, percent;

        if (AFF_FLAGGED(i, AFF_FLY) && GET_POS(i) == POS_FLYING)
            sprintf(buf2, "летает");
        else if (AFF_FLAGGED(i, AFF_LEVIT) && GET_POS(i) == POS_FLYING)
            sprintf(buf2, "парит");
        else
            sprintf(buf2, POS_STATE[(int) GET_POS(i)] + 1);

        sprintf(buf, "Перед Вами %s ", buf2);
        switch (GET_MOB_VID(i)) {
            case VMOB_SKELET:
                sprintf(buf2, "скелет");
                tsex = SEX_MALE;
                break;
            case VMOB_ZOMBIE:
                sprintf(buf2, "разлагающееся тело");
                tsex = SEX_NEUTRAL;
                break;
            case VMOB_MUMIE:
                sprintf(buf2, "тщательно забальзамированная мумия");
                tsex = SEX_FEMALE;
                break;
            case VMOB_GHOLA:
                sprintf(buf2, "гниющее тело");
                tsex = SEX_NEUTRAL;
                break;
            default:
                sprintf(buf2, "неизвестный Вам тип нежити");
                tsex = SEX_MALE;
                break;

        }
        if (GET_MOB_VID(i) == VMOB_SKELET)
            sprintf(buf + strlen(buf), "%s %s роста. ", buf2, show_height(i, h));
        else if (GET_MOB_VID(i) == VMOB_GHOLA)
            sprintf(buf + strlen(buf), "%s %s %s роста. ", buf2, GET_SEX(i) == SEX_FEMALE ?
                    get_name_pad((char *) race_name_pad_female[(int) GET_RACE(i)], PAD_ROD,
                                 PAD_MONSTER) : get_name_pad((char *) race_name_pad_male[(int)
                                                                                         GET_RACE
                                                                                         (i)],
                                                             PAD_ROD, PAD_MONSTER), show_height(i,
                                                                                                h));
        else
            sprintf(buf + strlen(buf), "%s неупокоенн%s %s %s роста. ", buf2,
                    get_sex_infra(i) == SEX_FEMALE ? "ой" : "ого",
                    get_sex_infra(i) == SEX_FEMALE ?
                    get_name_pad((char *) race_name_pad_female[(int) GET_RACE(i)], PAD_ROD,
                                 PAD_MONSTER) : get_name_pad((char *) race_name_pad_male[(int)
                                                                                         GET_RACE
                                                                                         (i)],
                                                             PAD_ROD, PAD_MONSTER), show_height(i,
                                                                                                h));
        sprintf(buf + strlen(buf), "В %s %s  глазницах %s %s%s. ", HSHR(i, i),
                GET_MOB_VID(i) == VMOB_SKELET ? "пустых" : "запавших", affected_by_spell(i,
                                                                                         SPELL_ENERGY_UNDEAD)
                ? "ярко" : "едва различимо",
                i->master ? "горит ровный зеленый огонь" : "пылает неистовое зеленое пламя",
                GET_MOB_VID(i) ==
                VMOB_GHOLA ? ", а неестестенно увеличенные зубы свирепо ощерены" : "");

        if (GET_REAL_MAX_HIT(i) > 0)
            percent = (100 * GET_HIT(i)) / GET_REAL_MAX_HIT(i);
        else
            percent = -1;       /* How could MAX_HIT be < 1?? */

        if (percent >= 100)
            sprintf(buf2, "совершенно неповрежденн%s и готов%s к бою", GET_CH_SUF_8(i),
                    GET_CH_SUF_8(i));
        else if (percent >= 80)
            sprintf(buf2, "немного поврежденн%s, но готов%s к бою", GET_CH_SUF_8(i),
                    GET_CH_SUF_8(i));
        else if (percent >= 60)
            sprintf(buf2, "изрядно потрепанн%s, но готов%s к бою", GET_CH_SUF_8(i),
                    GET_CH_SUF_8(i));
        else if (percent >= 40)
            sprintf(buf2, "сильно потрепанн%s и с трудом сохраняет равновесие", GET_CH_SUF_8(i));
        else if (percent >= 20)
            sprintf(buf2, "так, словно скоро развалится");
        else
            sprintf(buf2, "так, словно может рассыпаться в любой момент");

        sprintf(a, HSSH(i, i));
        sprintf(buf + strlen(buf), "%s выглядит %s.", CAP(a), buf2);

        send_to_char(strbraker(string_corrector(buf), ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)), ch);
    } else if (i->player.description && *i->player.description) {
        if (IS_NPC(i))
            send_to_char(strbraker
                         (string_corrector(i->player.description), ch->sw,
                          PRF_FLAGGED(ch, PRF_AUTOFRM)), ch);
        else
            send_to_char(i->player.description, ch);
        send_to_charf(ch, CAP(diag_char_to_char(i, TRUE)));
        send_to_char("\r\n", ch);
    } else if (!IS_NPC(i) || (IS_NPC(i) && NPC_FLAGGED(i, NPC_MISLEAD))) {

        if (AFF_FLAGGED(i, AFF_FLY) && GET_POS(i) == POS_FLYING)
            sprintf(buf2, "летает");
        else if (AFF_FLAGGED(i, AFF_LEVIT) && GET_POS(i) == POS_FLYING)
            sprintf(buf2, "парит");
        else
            sprintf(buf2, POS_STATE[(int) GET_POS(i)]);

        sprintf(buf, "Перед Вами %s %s %s %s %s роста. ",
                buf2,
                show_str(i, s),
                show_con(i, c), race_name[(int) GET_RACE(i)][(int) GET_SEX(i)], show_height(i, h));
        sprintf(buf + strlen(buf), "На %s %s лице выделяются %s %s глаза.",
                show_cha(i, r),
                IS_AFFECTED(i, AFF_IS_UNDEAD) ? "покрытом трупными пятнами" : show_age(i, a),
                show_int(i, t), eyes_color[(int) GET_EYES(i)]);

        sprintf(buf + strlen(buf), " %s", CAP(diag_char_to_char(i, FALSE)));
        send_to_char(strbraker(string_corrector(buf), ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)), ch);

    }

    if (AFF_FLAGGED(i, AFF_CHARM) && i->master == ch && low_charm(i)
        )
        act("1и скоро перестанет следовать за Вами.", "мМ", i, ch);

    if (IS_HORSE(i) && i->master == ch) {
        strcpy(buf, "\r\nЭто Ваш скакун. Он ");
        if (GET_HORSESTATE(i) <= 0)
            strcat(buf, "вот-вот отбросит копыта.");
        else if (GET_HORSESTATE(i) <= 20)
            strcat(buf, "очень устал.");
        else if (GET_HORSESTATE(i) <= 80)
            strcat(buf, "в хорошем состоянии.");
        else
            strcat(buf, "бодр и свеж.");
        send_to_char(buf, ch);
    };

    //diag_char_to_char(i, ch);

    found = FALSE;
    for (j = 0; !found && j < NUM_WEARS; j++)
        if ((GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j))) ||
            (GET_TATOO(i, j) && CAN_SEE_OBJ(ch, GET_TATOO(i, j))))
            found = TRUE;

    if (found) {
        act("1и одет1(,а,о,ы):", "мМ", i, ch);

        for (j = 0; j < NUM_WEARS; j++) {
            if (GET_EQ(i, wear_order_index[j])
                && CAN_SEE_OBJ(ch, GET_EQ(i, wear_order_index[j]))) {
                send_to_charf(ch, " %-20s", where[wear_order_index[j]]);
                show_obj_to_char(GET_EQ(i, wear_order_index[j]), ch, 1, ch == i, 1);
            } else
                if (GET_TATOO(i, wear_order_index[j]) &&
                    CAN_SEE_OBJ(ch, GET_TATOO(i, wear_order_index[j]))) {
                send_to_charf(ch, " %-20s", where_tatoo[wear_order_index[j]]);
                show_obj_to_char(GET_TATOO(i, wear_order_index[j]), ch, 1, ch == i, 1);
            }
        }
    }

    if (ch != i && IS_IMMORTAL(ch))
    {
        found = FALSE;
        act("\r\nВы попытались заглянуть в 2ев ношу:", "Мм", ch, i);

        for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
            if (CAN_SEE_OBJ(ch, tmp_obj) && (number(0, 30) < GET_LEVEL(ch))) {
                if (!push) {
                    push = tmp_obj;
                    push_count = 1;
                } else if (GET_OBJ_VNUM(push) != GET_OBJ_VNUM(tmp_obj) || GET_OBJ_VNUM(push) == -1) {
                    show_obj_to_char(push, ch, 1, ch == i, push_count);
                    push = tmp_obj;
                    push_count = 1;
                } else
                    push_count++;
                found = TRUE;
            }
        }
        if (push && push_count)
            show_obj_to_char(push, ch, 1, ch == i, push_count);
        if (!found)
            send_to_char("...но ничего не обнаружили.\r\n", ch);
    }

}

void exam_obj_to_char(struct obj_data *object, struct char_data *ch, int bits, const char *arg)
{
    char *desc;
    byte sw = ch->sw, rn = FALSE, found = FALSE;
    char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
    bool owner = FALSE;
    struct char_data *tch;


    if ((object->carried_by && object->carried_by == ch) ||
        (object->worn_by && object->worn_by == ch))
        owner = TRUE;

    if (GET_OBJ_TYPE(object) == ITEM_CORPSE) {
        for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
            if (ch != tch) {
                if (GET_ID(ch) == GET_OBJ_VAL(object, 1))
                    act("1+и обшарил1(,а,о,и) труп своей предыдущей инкарнации.", "мМ", ch, tch);
                else if (GET_ID(tch) != GET_OBJ_VAL(object, 1))
                    act("1+и обшарил1(,а,о,и) @1+в.", "мМп", ch, tch, object);
                else if (GET_ID(ch) != GET_OBJ_VAL(object, 1))
                    act("1+и обшарил1(,а,о,и) Ваш труп.", "мМ", ch, tch);
            } else {
                if (GET_ID(tch) != GET_OBJ_VAL(object, 1))
                    act("Вы обшарили @1+в.", "Мп", ch, object);
                else
                    act("Вы обшарили свой труп.", "М", ch);
            }
        }
    } else if (GET_OBJ_TYPE(object) != ITEM_CONTAINER || IS_OBJ_STAT(object, ITEM_EXTERN)) {
        if (owner)
            act("1*и внимательно осмотрел1(,а,о,и) сво@1(й,ю,е,и) @1в.", "Кмп", ch, object);
        else
            act("1*и внимательно осмотрел1(,а,о,и) @1в.", "Кмп", ch, object);
    } else {
        if (OBJVAL_FLAGGED(object, EXIT_CLOSED))
            act("1*и хотел1(,а,о,и) заглянуть в @1и.", "Кмп", ch, object);
        else {
            if (owner)
                act("1*и заглянул1(,а,о,и) в сво@1(й,ю,е,и) @1в.", "Кмп", ch, object);
            else
                act("1*и заглянул1(,а,о,и) в @1в.", "Кмп", ch, object);
        }
    }

    if (*arg) {
        if (((desc = find_exdesc(arg, object->ex_description)) != NULL)
            && GET_OBJ_TYPE(object) != ITEM_FICTION) {
            send_to_charf(ch, strbraker(desc, ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)));
            return;
        } else {
            send_to_charf(ch, "У %s нет '%s'.\r\n", GET_OBJ_PNAME(object, 1), arg);
            return;
        }
    }

    if (GET_OBJ_TYPE(object) == ITEM_NOTE) {
        if (object->action_description) {
            strcpy(buf, "Вы прочитали следующее:\r\n\r\n");
            strcat(buf, object->action_description);
            send_to_charf(ch, buf);
        } else
            send_to_char("Чисто.\r\n", ch);
        return;
    }


    if (object->main_description)
        strcpy(buf, object->main_description);
    else
        strcpy(buf, "Ничего особенного.\r\n");

    if (affected_object_by_spell(object, SPELL_MAGIC_VESTMENT) ||
        affected_object_by_spell(object, SPELL_MAGIC_VESTMENT_P) ||
        affected_object_by_spell(object, SPELL_MAGIC_WEAPON)) {
        if (IS_OBJ_STAT(object, ITEM_GOODAURA))
            sprintf(buf + strlen(buf), " Небесно-голубая аура окружает %s.",
                    GET_OBJ_PNAME(object, 3));
        if (IS_OBJ_STAT(object, ITEM_DARKAURA))
            sprintf(buf + strlen(buf), " Зловеще-зеленая аура окружает %s.",
                    GET_OBJ_PNAME(object, 3));
        if (IS_OBJ_STAT(object, ITEM_DEATHAURA))
            sprintf(buf + strlen(buf), " Темно-призрачная аура окружает %s.",
                    GET_OBJ_PNAME(object, 3));
    }

    if (GET_OBJ_TYPE(object) == ITEM_BOOK) {
        int spellnum, i, result = FALSE;

        for (i = 1; i <= 3; i++) {
            if (GET_OBJ_VAL(object, i) <= 0)
                continue;

            spellnum = find_spell_num(GET_OBJ_VAL(object, i));

            if (SPELL_LEVEL(spellnum) <= GET_SKILL(ch, SPELL_SPHERE(spellnum) + TYPE_SPHERE_SKILL)
                && !result) {
                result = TRUE;
            }
        }

        if (!result)
            sprintf(buf + strlen(buf), " %s каж%s Вам бесполезн%s.", CAP(GET_OBJ_PNAME(object, 0)),
                    GET_OBJ_SEX(object) == SEX_POLY ? "уться" : "ется", GET_OBJ_SUF_7(object));
    }

    if (GET_OBJ_TYPE(object) == ITEM_FURNITURE)
        send_to_char(buf, ch);
    else
        send_to_char(strbraker(string_corrector(buf), sw, PRF_FLAGGED(ch, PRF_AUTOFRM)), ch);

    if (GET_OBJ_TYPE(object) == ITEM_TATOO) {
        send_to_charf(ch, "%s наколота ", CAP(GET_OBJ_PNAME(object, 0)));
        sprinttype(object->obj_flags.Obj_mater, tatoo_color, buf);
        strcat(buf, " чернилами.\r\n");
        send_to_char(buf, ch);
        return;
    }
//Удаляем перенос строки
    buf[strlen(buf) - 2] = '\0';

    *buf = 0;

    strcat(buf, diag_weapon_to_char(object, TRUE));

    if ((GET_OBJ_TYPE(object) == ITEM_DRINKCON) || (GET_OBJ_TYPE(object) == ITEM_FOUNTAIN)) {
        strcat(buf, look_in_drink(ch, object, bits));
        rn = TRUE;
        found = TRUE;
    }

    if ((GET_OBJ_TYPE(object) == ITEM_CONTAINER) || (GET_OBJ_TYPE(object) == ITEM_CORPSE)) {
        strcat(buf, look_in_container(ch, object, bits));
        found = TRUE;
    }


    if (GET_OBJ_TYPE(object) == ITEM_LIGHT) {
        if ((GET_LIGHT_VAL(object) == -1 || GET_LIGHT_VAL(object) > 1) && GET_LIGHT_ON(object))
            sprintf(buf, "%s светится.", CAP((char *) OBJN(object, ch, 0)));
        else if (GET_LIGHT_VAL(object) == 0 && !GET_LIGHT_ON(object))
            sprintf(buf, "%s погас%s.", CAP((char *) OBJN(object, ch, 0)), GET_OBJ_SUF_4(object));
        else if (GET_LIGHT_VAL(object) > 0 && GET_LIGHT_VAL(object) < 2)
            sprintf(buf, "%s скоро погаснет.", CAP((char *) OBJN(object, ch, 0)));
        else if (!GET_LIGHT_ON(object))
            sprintf(buf, "%s погашен%s.", CAP((char *) OBJN(object, ch, 0)), GET_OBJ_SUF_1(object));
        found = TRUE;
        rn = TRUE;
    }

    switch (get_const_obj_temp(object)) {
        case 0:
            sprintf(buf + strlen(buf), "%s покрыт%s инеем.\r\n", CAP(GET_OBJ_PNAME(object, 0)),
                    GET_OBJ_SUF_1(object));
            break;
        case 1:
            sprintf(buf + strlen(buf), "%s отливает синевой.\r\n", CAP(GET_OBJ_PNAME(object, 0)));
            break;
        case 2:
            sprintf(buf + strlen(buf), "%s источает холод.\r\n", CAP(GET_OBJ_PNAME(object, 0)));
            break;
        case 6:
            sprintf(buf + strlen(buf), "%s источает тепло.\r\n", CAP(GET_OBJ_PNAME(object, 0)));
            break;
        case 7:
            sprintf(buf + strlen(buf), "%s источает жар.\r\n", CAP(GET_OBJ_PNAME(object, 0)));
            break;
        case 8:
            sprintf(buf + strlen(buf), "%s раскален%s до красна.\r\n",
                    CAP(GET_OBJ_PNAME(object, 0)), GET_OBJ_SUF_1(object));
            break;
    }

    if (!OBJ_FLAGGED(object, ITEM_NODECAY) && GET_OBJ_TYPE(object) != ITEM_FURNITURE
        && !IS_CORPSE(object)) {
        found = FALSE;
        sprintf(buf + strlen(buf), "%s%s %s.",
                (rn ? "\r\n" : ""), CAP(GET_OBJ_PNAME(object, 0)), diag_obj_to_char(ch, object, 2));

    }

    if (GET_OBJ_TYPE(object) == ITEM_FICTION && OBJVAL_FLAGGED(object, EXIT_CLOSED)) {
        sprintf(buf2, "%s закрыт%s.", GET_OBJ_PNAME(object, 0), GET_OBJ_SUF_6(object));
        sprintf(buf + strlen(buf), "\r\n%s", CAP(buf2));
    }

    if (found && !(GET_OBJ_TYPE(object) == ITEM_FOUNTAIN || GET_OBJ_TYPE(object) == ITEM_LIGHT))
        buf[strlen(buf) - 2] = '\0';

    if (GET_OBJ_TYPE(object) == ITEM_TRANSPORT) {
        if (object->transpt && object->transpt->people.size()) {
            sprintf(buf + strlen(buf), "В %s сид%s: ", GET_OBJ_PNAME(object, 5),
                    object->transpt->people.size() == 1 ? "ит" : "ят");
            for (int i = 0; i < (int) object->transpt->people.size(); i++) {
                sprintf(buf + strlen(buf), "%s%s", CAP(GET_NAME(object->transpt->people[i])),
                        (i + 1) < (int) object->transpt->people.size()? ", " : "");
            }
        } else
            sprintf(buf + strlen(buf), "%s пуст%s.", CAP(GET_OBJ_PNAME(object, 0)),
                    GET_OBJ_SUF_6(object));
    }


    strcat(buf, "\r\n");
    send_to_charf(ch, buf);
}

/*****************************************************************************/
void read_book(struct obj_data *obj, struct char_data *ch)
{
    int page;
    char *desc;
    char buf[MAX_STRING_LENGTH];

    if (!ch->desc)
        return;

    if (GET_OBJ_TYPE(obj) != ITEM_FICTION) {
        send_to_charf(ch, "Это не книга.\r\n");
        return;
    }

    if (OBJVAL_FLAGGED(obj, EXIT_CLOSED)) {
        act("@1и закрыт@1(,а,о,ы).\r\nПрежде, чем попробуете читать, откройте @1ер.", "Мп", ch,
            obj);
        return;
    }

    if (!obj->ex_description) {
        act("В @1п ничего не написано.", "Мп", ch, obj);
        return;
    }

    page = obj->page;

    if ((desc = find_exdesc_number(page, obj->ex_description)) != NULL) {
        strcpy(buf, desc);
        act("1и углубил1(ся,ась,ось,ись) в чтение @1р.", "Кмп", ch, obj);
    } else
        strcpy(buf, "Нет такой страницы.\r\n");

    send_to_charf(ch, "%s\r\n", buf);
//send_to_char(strbraker(string_corrector(buf), ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)), ch);
//send_to_charf(ch,"\r\n\r\n%20s: %d ]\r\n","[ Страница",page);
}

/****************************************************************************/
char *look_in_char(struct char_data *ch, struct char_data *tch, int light)
{
    int sex = 0;
    char h[20], name[256], position[256], size[256];
    static char buf2[MAX_STRING_LENGTH];

    *buf2 = '\0';

    if (IS_MOUNT(tch) && AFF_FLAGGED(tch, AFF_HORSE))
        return (buf2);

    if (GET_RACE(tch) == RACE_ANIMAL)
        sex = SEX_NEUTRAL;
    else if (GET_RACE(tch) == RACE_BIRD)
        sex = SEX_FEMALE;
    else
        sex = GET_SEX(tch);


    {
        if ((AFF_FLAGGED(tch, AFF_FLY) ||
             AFF_FLAGGED(tch, AFF_LEVIT)) && (IS_MOB(tch) || GET_POS(tch) == POS_FLYING)) {
            if (IS_MOB(tch) && MOB_FLAGGED(tch, MOB_SENTINEL))
                sprintf(position, "летает");
            else
                sprintf(position, "летит");
        } else
            if (IS_MOB(tch) && MOB_FLAGGED(tch, MOB_SENTINEL)
                && GET_POS(tch) == GET_DEFAULT_POS(tch) && !FIGHTING(tch))
            sprintf(position, "стоит");
        else if (IS_MOB(tch) && !NPC_FLAGGED(tch, NPC_MISLEAD) &&
                 GET_POS(tch) == GET_DEFAULT_POS(tch) && GET_DEFAULT_POS(tch) == POS_STANDING)
            sprintf(position, "%s", in_go[(int) GET_MOVE_TYPE(tch)]);
        else if (FIGHTING(tch))
            sprintf(position, "сражается");
        else
            sprintf(position, "%s", POS_STATE[(int) GET_POS(tch)] + 1);
    }

    sprintf(size, "%s", show_in_height(tch, h, sex));

    if (light <= 10) {          //светло все хорошо видно
        if (on_horse(tch)) {
            sprintf(buf2, "%s сидит верхом на %s", GET_NAME(tch), PERS(get_horse_on(tch), ch, 5));
        } else {
            sprintf(name, "%s", GET_NAME(tch));
            sprintf(buf2, "%s %s", position, name);
        }
    } else if (light <= 20) {   //светло почти все видно
        if (on_horse(tch)) {
            sprintf(buf2, "%s сидит верхом на ком-то",
                    race_name[(int) GET_RACE(tch)][(int) GET_SEX(tch)]);
        } else {
            sprintf(name, "%s", name_infra(tch, 0));
            sprintf(buf2, "%s %s", position, name);
        }
    } else if (light <= 50) {   //легкий туман
        sprintf(size, "%s", show_in_height(tch, h, SEX_MALE));
        sprintf(buf2, "Виден чей-то %s силуэт", size);
    } else if (light <= 80) {   //средний туман
        sprintf(buf2, "Виден чей-то неясный силуэт");
    }

    return CAP(buf2);
}


void look_in_direction(struct char_data *ch, int dir)
{
    int count = 0, probe, percent, all_ok = 0, light_in = 0, light_out = 0;
    struct room_direction_data *rdata = NULL;
    char bufs[16], *buffer = NULL, room_name[256];
    struct char_data *tch;
    char buf[MAX_STRING_LENGTH];

    *buf = '\0';
    *room_name = '\0';
    *bufs = '\0';

    if ((CAN_GO(ch, dir) && !DOOR_FLAGGED(EXIT(ch, dir), EXIT_HIDDEN)) ||
        (CAN_GO(ch, dir) && DOOR_FLAGGED(EXIT(ch, dir), EXIT_HIDDEN) &&
         EXIT(ch, dir)->to_room != NOWHERE && world[EXIT(ch, dir)->to_room].people) ||
        (EXIT(ch, dir) && EXIT(ch, dir)->to_room != NOWHERE &&
         !DOOR_FLAGGED(EXIT(ch, dir), EXIT_HIDDEN))) {
        rdata = EXIT(ch, dir);
        strcpy(bufs, DirsTo_2[dir]);
        bufs[0] = UPPER(bufs[0]);

        if ((ROOM_FLAGGED(IN_ROOM(ch), ROOM_FCIRCLE) ||
             (ROOM_FLAGGED(IN_ROOM(ch), ROOM_CIRCLE) && dir < UP)) && !IS_AFFECTED(ch, AFF_ORENT))
            do {
                dir = number(0, 5);
            } while (!EXIT(ch, dir) || ch->vdir[dir]);
        ch->vdir[dir] = TRUE;

        count += sprintf(buf, "%11s:%s ", bufs, (!PRF_FLAGGED(ch, PRF_THEME)) ?
                         CCCYN(ch, C_NRM) : CCIBLK(ch, C_NRM));
        //Дверь закрыта и мы ничего не видим
        if (DOOR_FLAGGED(rdata, EXIT_CLOSED) && !IS_CLOUD_NL(rdata->to_room)) {
            if (rdata->exit_name) {
                if (rdata->general_description
                    && strcmp(rdata->general_description, rdata->exit_name))
                    count += sprintf(buf + count, "%s", rdata->general_description);
                else
                    count +=
                        sprintf(buf + count, "Закрыт%s %s.", DSHP(ch, dir),
                                get_name_pad(rdata->exit_name, PAD_IMN, PAD_OBJECT));
            } else
                count += sprintf(buf + count, "Закрыто что-то отдаленно напоминающее дверь.");

            count += sprintf(buf + count, "%s", CCNRM(ch, C_NRM));
            send_to_charf(ch, "%s\r\n", buf);
            return;
        };

        //Проверяем наличия умения "следопыт"
        if (PRF_FLAGGED(ch, PRF_TRACKON)) {
            std::vector < int >vit;
            std::vector < int >vot;

            all_ok = FALSE;
            percent = number(1, 100);
            //Параметры для атаки
            vit.push_back(GET_REAL_WIS(ch));
            vit.push_back(GET_REAL_INT(ch));
            //Параметры для защиты
            vot.push_back(dice(1, 40));

            probe = calc_like_skill(ch, NULL, SKILL_TRACKON, vit, vot);

            if (percent <= 5)
                probe = 100;
            else if (percent >= 95)
                probe = 0;

            if (PRF_FLAGGED(ch, PRF_CODERINFO))
                send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", probe, percent);

            if (GET_MOVE(ch) > 0) {
                improove_skill(ch, NULL, 0, SKILL_TRACKON);
                if (probe > percent && check_trackon(ch, rdata->to_room))
                    all_ok = TRUE;
                GET_MOVE(ch)--;
            }
        }
        //расчет видимости
        if (IS_TIMEDARK(rdata->to_room) && !IS_AFFECTED(ch, AFF_DARKVISION)) {
            sprintf(room_name, "Слишком темно");
            light_out += 100;
        } else if (IS_CLOUD_NL(rdata->to_room)) {
            sprintf(room_name, "Густой туман");
            light_out += 50;
        } else if (IS_LCLOUD_NL(rdata->to_room)) {
            sprintf(room_name, "Туман");
            light_out += 20;
        } else {
            if (IS_VLCLOUD_NL(rdata->to_room))
                light_out += 10;

            if (EXIT(ch, dir)->general_description && !EXIT(ch, dir)->exit_name)
                sprintf(room_name, "%s", EXIT(ch, dir)->general_description);
            else
                sprintf(room_name, "%s",
                        get_name_pad(world[rdata->to_room].name, PAD_IMN, PAD_OBJECT));
        }

        //расчет видимости
        if (IS_TIMEDARK(IN_ROOM(ch)) && !IS_AFFECTED(ch, AFF_DARKVISION))
            light_in += 100;
        else if (IS_CLOUD_NL(IN_ROOM(ch)))
            light_in += 50;
        else if (IS_LCLOUD_NL(IN_ROOM(ch)))
            light_in += 20;
        else if (IS_VLCLOUD_NL(IN_ROOM(ch)))
            light_out += 10;

        //Выводим показ локации
        count += sprintf(buf + count, "%s.\r\n", room_name);

        //Выводим персонажей в локации
        count += sprintf(buf + count, "%s", !PRF_FLAGGED(ch, PRF_THEME) ?
                         CCYEL(ch, C_NRM) : CCIRED(ch, C_NRM));
        for (tch = world[rdata->to_room].people; tch; tch = tch->next_in_room) {
            if (!CAN_SEE(ch, tch) || (IS_NPC(tch) && NPC_FLAGGED(tch, NPC_ISOBJECT))
                || IS_SOUL(tch))
                continue;

            if (IS_NPC(tch) && NPC_FLAGGED(tch, NPC_HIDDEN) && !PRF_FLAGGED(ch, PRF_HOLYLIGHT))
                continue;

            int light = (light_in + light_out) / 2;

            buffer = look_in_char(ch, tch, light);

            if (*buffer)
                //count += sprintf(buf+count, "[%d] %s.\r\n",light,buffer);
                count += sprintf(buf + count, "%s.\r\n", buffer);
        }
        count += sprintf(buf + count, "%s", CCNRM(ch, C_NRM));


        send_to_charf(ch, buf);
    } else
        if (EXIT(ch, dir) &&
            !DOOR_FLAGGED(EXIT(ch, dir), EXIT_HIDDEN) && EXIT(ch, dir)->general_description) {
        if (dir == DOWN)
            strcpy(bufs, "Под ногами");
        else if (dir == UP)
            strcpy(bufs, "Над головой");
        else if (dir == EAST)
            strcpy(bufs, "На востоке");
        else if (dir == WEST)
            strcpy(bufs, "На западе");
        else if (dir == SOUTH)
            strcpy(bufs, "На юге");
        else if (dir == NORTH)
            strcpy(bufs, "На севере");


        count = 0;
        count += sprintf(buf + count, "%11s:%s ", bufs, (!PRF_FLAGGED(ch, PRF_THEME)) ?
                         CCCYN(ch, C_NRM) : CCIBLK(ch, C_NRM));
        count += sprintf(buf + count, "%s\r\n", EXIT(ch, dir)->general_description);
        count += sprintf(buf + count, "%s", CCNRM(ch, C_NRM));
        send_to_charf(ch, buf);
    } else
        if (EXIT(ch, dir) &&
            DOOR_FLAGGED(EXIT(ch, dir), EXIT_HIDDEN) && EXIT(ch, dir)->general_description) {
        strcpy(bufs, DirsTo_2[dir]);
        bufs[0] = UPPER(bufs[0]);
        count = 0;
        count += sprintf(buf + count, "%11s:%s ", bufs, (!PRF_FLAGGED(ch, PRF_THEME)) ?
                         CCCYN(ch, C_NRM) : CCIBLK(ch, C_NRM));
        count += sprintf(buf + count, "%s\r\n", EXIT(ch, dir)->general_description);
        count += sprintf(buf + count, "%s", CCNRM(ch, C_NRM));
        send_to_charf(ch, buf);
    }

}

void hear_in_direction(struct char_data *ch, int dir, int info_is)
{
    int count = 0;
    struct room_direction_data *rdata;
    struct char_data *tch;
    char buf[MAX_STRING_LENGTH];

    if (CAN_GO(ch, dir)) {
        rdata = EXIT(ch, dir);
        count += sprintf(buf, "%8s:%s ", Dirs[dir], CCIBLK(ch, C_NRM));

        if (DOOR_FLAGGED(rdata, EXIT_CLOSED) && rdata->keyword) {
            count += sprintf(buf + count, "%s (закрыто).\r\n", CAP(rdata->keyword));
            send_to_char(buf, ch);
            send_to_char(CCNRM(ch, C_NRM), ch);
            return;
        };
        count += sprintf(buf + count, "\r\n");
        send_to_char(buf, ch);
        for (count = 0, tch = world[rdata->to_room].people; tch; tch = tch->next_in_room) {
            if (!AFF_FLAGGED(tch, AFF_SNEAK) && !AFF_FLAGGED(tch, AFF_HIDE)) {
                if (IS_NPC(tch)) {
                    send_to_char("Вы слышите чью-то возню.\r\n", ch);
                } else {
                    send_to_char("Там кто-то есть.\r\n", ch);
                }
                count++;
            }
        }
        if (!count)
            send_to_char("Тишина и покой.\r\n", ch);
        send_to_char(CCNRM(ch, C_NRM), ch);
    } else {
        if (info_is & EXIT_SHOW_WALL)
            send_to_char("И что Вы хотите услышать?\r\n", ch);
        send_to_char(CCNRM(ch, C_NRM), ch);
    }
}


//added by HMEPAS
char *look_in_drink(struct char_data *ch, struct obj_data *obj, int bits)
{
    static char out_str[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    *out_str = '\0';

    if (ch == NULL || obj == NULL) {
        log("ОШИБКА: look_in_drink указатель на объект или персонажа пустой");
        strcat(out_str, "Системная ошибка #8993.");
        return (out_str);
    }

    if (GET_OBJ_VAL(obj, 1) <= 0)
        strcat(out_str, "Пусто.");
    else {
        sprinttype(GET_OBJ_VAL(obj, 2), color_liquid, buf2);
        sprintf(out_str, "%s наполнен%s %s %s%s жидкостью.",
                CAP(GET_OBJ_PNAME(obj, 0)),
                GET_OBJ_SUF_6(obj), fullness[(GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0)],
                buf2,
                (AFF_FLAGGED(ch, AFF_DETECT_POISON) &&
                 GET_OBJ_VAL(obj, 3) > 0 ? "(отравленной)" : ""));
    }

    return (out_str);
}

char *look_in_container(struct char_data *ch, struct obj_data *obj, int bits)
{
    static char out_str[MAX_STRING_LENGTH];

    *out_str = '\0';

    if (ch == NULL || obj == NULL) {
        log("ОШИБКА: look_in_container указатель на объект или персонаж пустой");
        strcat(out_str, "Системная ошибка #8994.");
        return (out_str);
    }


    if (OBJVAL_FLAGGED(obj, EXIT_CLOSED))
        sprintf(out_str, "Закрыт%s.\r\n", GET_OBJ_SUF_6(obj));
    else if (IS_OBJ_STAT(obj, ITEM_EXTERN)) {
        sprintf(out_str + strlen(out_str), "На %s:\r\n", OBJN(obj, ch, 5));
        if (!obj->contains || !CAN_SEE_OBJ(ch, obj->contains))
            sprintf(out_str + strlen(out_str), "ничего не лежит.\r\n");
        else
            sprintf(out_str + strlen(out_str), "%s", buf_obj_to_char(obj->contains, ch));
    } else {
        if (!obj->contains || !CAN_SEE_OBJ(ch, obj->contains))
            sprintf(out_str + strlen(out_str), "Внутри %s ничего нет.\r\n", OBJN(obj, ch, 1));
        else {
            sprintf(out_str + strlen(out_str), "%s:\r\n", CAP((char *) OBJN(obj, ch, 0)));
            sprintf(out_str + strlen(out_str), "%s", buf_obj_to_char(obj->contains, ch));
        }
    }

    return (out_str);
}

// end of adding

char *find_exdesc(const char *word, struct extra_descr_data *list)
{
    struct extra_descr_data *i;

    for (i = list; i; i = i->next) {
        if (isname(word, i->keyword))
            return (i->description);
    }
    return (NULL);
}

char *find_exdesc_number(int number, struct extra_descr_data *list)
{
    struct extra_descr_data *i;

    for (i = list; i; i = i->next) {
        if (number == atoi(i->keyword))
            return (i->description);
    }
    return (NULL);
}

void look_at_target(struct char_data *ch, char *arg, int subcmd)
{
    int bits;
    struct char_data *found_char = NULL;
    struct obj_data *found_obj = NULL;
    char *desc, *what, whatp[MAX_INPUT_LENGTH], where[MAX_INPUT_LENGTH];
    int where_bits =
        FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP | FIND_CHAR_ROOM | FIND_OBJ_TATOO;

    if (!ch->desc)
        return;

    if (!*arg) {
        send_to_char("На что Вы так мечтаете посмотреть?\r\n", ch);
        return;
    }

    half_chop(arg, whatp, where);
    what = whatp;

    if (isname(where, "земля комната room ground"))
        where_bits = FIND_OBJ_ROOM | FIND_CHAR_ROOM;
    else if (isname(where, "инвентарь inventory"))
        where_bits = FIND_OBJ_INV | FIND_CHAR_ROOM;
    else if (isname(where, "экипировка equipment"))
        where_bits = FIND_OBJ_EQUIP | FIND_CHAR_ROOM;

    bits = generic_find(what, where_bits, ch, &found_char, &found_obj);


    /* Смотрим на персонажа */
    if (found_char != NULL) {
        if (subcmd == SCMD_READ) {
            act("На 2+п ничего не написано.", "Мм", ch, found_char);
            return;
        }

        exam_at_char(found_char, ch, where);

        return;
    }

    /*Смотрим на предмет */
    if (found_obj != NULL) {
        if (subcmd == SCMD_READ && GET_OBJ_TYPE(found_obj) == ITEM_FICTION)
            read_book(found_obj, ch);
        else
            exam_obj_to_char(found_obj, ch, bits, where);
        return;
    }


    /* Ищем доп.описания в локации */
    if ((desc = find_exdesc(what, world[ch->in_room].ex_description)) != NULL) {
        page_string(ch->desc, strbraker(desc, ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)), 1);
        return;
    }

    send_to_charf(ch, "Нет ничего похожего на '%s'.\r\n", what);
}

char *show_obj(struct obj_data *object, struct char_data *ch, int how)
{
    static char out_str[MAX_STRING_LENGTH];

    *out_str = '\0';

    if (PRF_FLAGGED(ch, PRF_ROOMFLAGS))
        sprintf(out_str, "[%6d] ", GET_OBJ_VNUM(object));

    if (object->short_description)
        strcat(out_str, object->short_description);

    if (how > 1)
        sprintf(out_str + strlen(out_str), " [%d]", how);

    if (object->missile && object->obj_flags.value[2] > 1)
        sprintf(out_str + strlen(out_str), " (%d)", object->obj_flags.value[2]);

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_ROOMFLAGS))
        sprintf(out_str + strlen(out_str), " {%s}", object->name);

    if (how <= 1) {
        if (IS_OBJ_STAT(object, ITEM_INVISIBLE))
            sprintf(out_str + strlen(out_str), " (невидим%s)", GET_OBJ_SUF_6(object));
        if (IS_OBJ_STAT(object, ITEM_GOODAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(out_str, " (небесно-голубая аура)");

        if (IS_OBJ_STAT(object, ITEM_DARKAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(out_str, " (зловеще-зеленая аура)");

        if (IS_OBJ_STAT(object, ITEM_DEATHAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(out_str, " (призрачная аура)");


        if (IS_OBJ_STAT(object, ITEM_MAGIC) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(out_str, " (синяя аура)");
        if (IS_OBJ_STAT(object, ITEM_POISONED) && AFF_FLAGGED(ch, AFF_DETECT_POISON))
            sprintf(out_str + strlen(out_str), " (отравлен%s)", GET_OBJ_SUF_6(object));

        if (IS_OBJ_STAT(object, ITEM_GLOW))
            strcat(out_str, " (мягко светится)");
        if (IS_OBJ_STAT(object, ITEM_HUM) && !IS_AFFECTED(ch, AFF_DEAFNESS))
            strcat(out_str, " (тихо шумит)");
        if (IS_OBJ_STAT(object, ITEM_FIRE))
            strcat(out_str, " (ярко горит)");

        if ((GET_OBJ_TYPE(object) == ITEM_LIGHT && GET_LIGHT_VAL(object) > 0
             && GET_LIGHT_ON(object)) || (GET_OBJ_TYPE(object) != ITEM_LIGHT
                                          && GET_LIGHT_VAL(object) > 0))
            strcat(out_str, " (ярко светиться)");
    }
    strcat(out_str, "\r\n");
    return (out_str);
}

void show_obj_to_char(struct obj_data *object, struct char_data *ch,
                      int mode, int show_state, int how)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    *buf = '\0';
    *buf2 = '\0';

    if ((mode < 5) && PRF_FLAGGED(ch, PRF_ROOMFLAGS))
        sprintf(buf, "[%6d] ", GET_OBJ_VNUM(object));

    if ((mode == 0) && object->description) {
        if (IS_CORPSE(object) && GET_OBJ_VAL(object, 1) == GET_ID(ch)) {
            if (SECT(IN_ROOM(object)) == SECT_WATER_SWIM ||
                SECT(IN_ROOM(object)) == SECT_WATER_NOSWIM ||
                SECT(IN_ROOM(object)) == SECT_UNDERWATER)
                sprintf(buf2, "Ваш труп лежит на дне.");
            else
                sprintf(buf2, "Ваш труп лежит здесь.");
        } else
            if (SECT(IN_ROOM(object)) == SECT_WATER_SWIM ||
                SECT(IN_ROOM(object)) == SECT_WATER_NOSWIM ||
                SECT(IN_ROOM(object)) == SECT_UNDERWATER)
            sprintf(buf2, "%s леж%s на дне.", object->PNames[0],
                    (GET_OBJ_SEX(object) == SEX_POLY) ? "ат" : "ит");
        else if (GET_OBJ_TYPE(object) == ITEM_TRAP && object->trap_victim) {
            if (object->trap_victim == ch)
                sprintf(buf2, "%s держит Вас за ногу.", object->PNames[0]);
            else
                return;
        } else
            strcpy(buf2, object->description);

        strcat(buf, CAP(buf2));

        if (how > 1)
            sprintf(buf + strlen(buf), " [%d]", how);

        if (IS_OBJ_STAT(object, ITEM_INVISIBLE))
            sprintf(buf + strlen(buf), " (невидим%s)", GET_OBJ_SUF_6(object));
        if (IS_OBJ_STAT(object, ITEM_GOODAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (небесно-голубая аура)");
        if (IS_OBJ_STAT(object, ITEM_DARKAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (зловеще-зеленая аура)");
        if (IS_OBJ_STAT(object, ITEM_DEATHAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (призрачная аура)");


        if (IS_OBJ_STAT(object, ITEM_MAGIC) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (синяя аура)");
        if (IS_OBJ_STAT(object, ITEM_POISONED) && AFF_FLAGGED(ch, AFF_DETECT_POISON))
            sprintf(buf + strlen(buf), " (отравлен%s)", GET_OBJ_SUF_6(object));
        if (IS_OBJ_STAT(object, ITEM_GLOW))
            strcat(buf, " (мягко светится)");
        if (IS_OBJ_STAT(object, ITEM_HUM) && !IS_AFFECTED(ch, AFF_DEAFNESS))
            strcat(buf, " (тихо шумит)");
        if (IS_OBJ_STAT(object, ITEM_FIRE))
            strcat(buf, " (ярко горит)");
        if ((GET_OBJ_TYPE(object) == ITEM_LIGHT && GET_LIGHT_VAL(object) > 0
             && GET_LIGHT_ON(object)) || (GET_OBJ_TYPE(object) != ITEM_LIGHT
                                          && GET_LIGHT_VAL(object) > 0))
            strcat(buf, " (ярко светится)");

        send_to_charf(ch, "%s\r\n", CAP(buf));
        return;
    } else
        if (object->short_description &&
            ((mode == 1) || (mode == 2) || (mode == 3) || (mode == 4))) {
        if (mode == 1)
            strcat(buf, " ");
        strcat(buf, object->short_description);
        if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_ROOMFLAGS))
            sprintf(buf + strlen(buf), " [%s]", object->name);
    } else if (mode == 5) {
        if (GET_OBJ_TYPE(object) == ITEM_NOTE) {
            if (object->action_description) {
                strcpy(buf, "Вы прочитали следующее :\r\n\r\n");
                strcat(buf, object->action_description);
                /////////////////////////////////////////
                if PRF_FLAGGED
                    (ch, PRF_AUTOFRM)
                        page_string(ch->desc, strbraker(buf, ch->sw, 1), 1);
                else
                    page_string(ch->desc, strbraker(buf, ch->sw, 0), 1);
                //////////////////////////////////
            } else
                send_to_char("Чисто.\r\n", ch);
            return;
        } else if (GET_OBJ_TYPE(object) == ITEM_DRINKCON)
            /* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
            strcpy(buf, "Это емкость для жидкости.");
    }

    if (how > 1)
        sprintf(buf + strlen(buf), " [%d]", how);
    if (mode != 3 && how <= 1) {
        if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
            sprintf(buf2, " (невидим%s)", GET_OBJ_SUF_6(object));
            strcat(buf, buf2);
        }
        if (IS_OBJ_STAT(object, ITEM_GOODAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (небесно-голубая аура)");
        if (IS_OBJ_STAT(object, ITEM_DARKAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (зловеще-зеленая аура)");
        if (IS_OBJ_STAT(object, ITEM_DEATHAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (призрачная аура)");
        if (IS_OBJ_STAT(object, ITEM_MAGIC) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (синяя аура)");
        if (IS_OBJ_STAT(object, ITEM_POISONED) && AFF_FLAGGED(ch, AFF_DETECT_POISON)) {
            sprintf(buf2, " (отравлен%s)", GET_OBJ_SUF_6(object));
            strcat(buf, buf2);
        }
        if (IS_OBJ_STAT(object, ITEM_GLOW))
            strcat(buf, " (мягко светится)");
        if (IS_OBJ_STAT(object, ITEM_HUM))
            strcat(buf, " (тихо шумит)");
        if (IS_OBJ_STAT(object, ITEM_FIRE))
            strcat(buf, " (ярко горит)");
        if ((GET_OBJ_TYPE(object) == ITEM_LIGHT && GET_LIGHT_VAL(object) > 0
             && GET_LIGHT_ON(object)) || (GET_OBJ_TYPE(object) != ITEM_LIGHT
                                          && GET_LIGHT_VAL(object) > 0))
            strcat(buf, " (ярко светиться)");
    }
    strcat(buf, "\r\n");
    if (mode >= 5) {
        strcat(buf, diag_weapon_to_char(object, TRUE));
        if (show_state) {
            *buf2 = '\0';
            if (mode == 1 && how <= 1) {        /*if (GET_OBJ_TYPE(object) == ITEM_LIGHT)
                                                   {if (GET_LIGHT_VAL(object) == -1)
                                                   strcpy(buf2," (вечный свет)");
                                                   else
                                                   if (GET_LIGHT_VAL(object) == 0)
                                                   sprintf(buf2," (погас%s)",
                                                   GET_OBJ_SUF_4(object));
                                                   else
                                                   sprintf(buf2," (%d %s)",
                                                   GET_LIGHT_VAL(object),desc_count(GET_LIGHT_VAL(object),WHAT_HOUR));
                                                   }
                                                   else */
                // if (GET_OBJ_CUR(object) < GET_OBJ_MAX(object))
                sprintf(buf2, " %s", diag_obj_to_char(ch, object, 1));
                if (GET_OBJ_TYPE(object) == ITEM_CONTAINER || GET_OBJ_TYPE(object) == ITEM_CORPSE) {
                    if (object->contains)
                        strcat(buf2, " (есть содержимое)");
                    else
                        sprintf(buf2 + strlen(buf2), " (пуст%s)", GET_OBJ_SUF_1(object));
                }
            } else if (mode >= 2 && how <= 1) { /*if(GET_OBJ_TYPE(object) == ITEM_LIGHT)
                                                   {if (GET_LIGHT_VAL(object) == -1)
                                                   sprintf(buf2,"\r\n%s дает вечный свет.",
                                                   CAP((char *) OBJN(object,ch,0)));
                                                   else
                                                   if (GET_LIGHT_VAL(object) == 0)
                                                   sprintf(buf2,"\r\n%s погас%s.",
                                                   CAP((char *) OBJN(object,ch,0)), GET_OBJ_SUF_4(object));
                                                   else
                                                   sprintf(buf2,"\r\n%s будет светить %d %s.",
                                                   CAP((char *) OBJN(object,ch,0)),GET_LIGHT_VAL(object),desc_count(GET_LIGHT_VAL(object),WHAT_HOUR));
                                                   }
                                                   else */
                if (GET_OBJ_CUR(object) < GET_OBJ_MAX(object))
                    sprintf(buf2, "\r\n%s %s.",
                            CAP((char *) OBJN(object, ch, 0)), diag_obj_to_char(ch, object, 2));
            }
            strcat(buf, buf2);
        }

    }
    page_string(ch->desc, buf, TRUE);
}



ACMD(do_look)
{
    char arg2[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];

    if (!ch->desc)
        return;

    if (AFF_FLAGGED(ch, AFF_BLIND)) {
        send_to_char("Вы слепы!\r\n", ch);
        return;
    }

    half_chop(argument, arg, arg2);

    if (!*arg && subcmd == SCMD_EXAMINE) {
        send_to_charf(ch, "Что Вы хотите осмотреть?\r\n");
        return;
    }

    if (!*arg && subcmd == SCMD_READ) {
        send_to_charf(ch, "Что Вы хотите прочитать?\r\n");
        return;
    }

    if (!*arg)
        look_at_room(ch, 1);
    else {
        /*  if (((look_type = search_block(arg, dirs, FALSE)) >= 0) ||
           ((look_type = search_block(arg, Dirs, FALSE)) >= 0))
           {
           send_to_char("Вы посмотрели на:\r\n",ch);
           look_in_direction(ch, look_type);
           return;
           }
           else */
        {
            if (is_abbrev(arg, "at") || is_abbrev(arg, "на"))
                look_at_target(ch, arg2, subcmd);
            else
                look_at_target(ch, argument, subcmd);
            return;
        }
        send_to_charf(ch, "Здесь нет '%s'\r\n", arg);
    }
}

ACMD(do_scan)
{
    int i;

    if (!ch->desc)
        return;

    if (GET_POS(ch) <= POS_SLEEPING)
        send_to_char("Виделся часто сон беспокойный...\r\n", ch);
    else if (AFF_FLAGGED(ch, AFF_BLIND))
        send_to_char("Вы слепы!\r\n", ch);
    else {
        send_to_char("Вы посмотрели по сторонам:\r\n", ch);
        for (i = 0; i < NUM_OF_DIRS; i++)
            look_in_direction(ch, i);

        /* Убираем флаги проверенных локаций */
        for (int vdir = 0; vdir < NUM_OF_DIRS; vdir++)
            ch->vdir[vdir] = FALSE;
    }
}


void go_look_hide(struct char_data *ch, struct char_data *i)
{
    int found, push_count = 0, prob, percent;
    struct obj_data *tmp_obj, *push = NULL;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_LOOK_HIDE) : GET_SKILL(ch, SKILL_LOOK_HIDE);

    if (!may_kill_here(ch, i))
        return;

    if (ch == i) {
        send_to_char("Вы внимательно пригляделись к своему имуществу:\r\n", ch);
        send_to_char("Вы заметили у себя много интересных вещей.\r\n", ch);
        return;
    }

    if (FIGHTING(i)) {
        send_to_char("Ваша цель слишком быстро перемещается!\r\n", ch);
        return;
    }

    if (GET_MOVE(ch) < 1) {
        send_to_charf(ch, "У Вас не хватит сил.\r\n");
        return;
    }

    WAIT_STATE(ch, PULSE_VIOLENCE);

    if (ch != i) {
        found = FALSE;
        if (!IS_GOD(ch))
            GET_MOVE(ch)--;
        act("Вы пригляделись к имуществу 2р:", "Мм", ch, i);
        prob = GET_REAL_INT(ch) + GET_REAL_INT(ch) + GET_REAL_WIS(ch) + skill;
        percent = GET_REAL_INT(i) + GET_REAL_WIS(i) + GET_REAL_WIS(i) +
            GET_SAVE3(i, SAV_REFL) + saving_throws_3(i, SAV_REFL) + RNDSKILL;

        if (!IS_SHOPKEEPER(i) && CAN_SEE(i, ch))
            improove_skill(ch, i, 0, SKILL_LOOK_HIDE);

        if (CAN_SEE(i, ch))
            prob = prob / 2;
        if (IS_DARKTHIS(IN_ROOM(i)))
            prob = 0;

        /*if (percent <= 5)
           prob = 100; */

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);

        if (percent <= prob && GET_GOLD(i) >= 1) {
            act("Вы заметили, что у 2р есть #1 монет#1(а,ы,).", "Ммч", ch, i, GET_GOLD(i));
            found = TRUE;
        }

        for (tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
            if (CAN_SEE_OBJ(ch, tmp_obj) && number(1, 100) <= prob) {
                if (!push) {
                    push = tmp_obj;
                    push_count = 1;
                } else if (GET_OBJ_VNUM(push) != GET_OBJ_VNUM(tmp_obj) || GET_OBJ_VNUM(push) == -1) {
                    show_obj_to_char(push, ch, 1, ch == i, push_count);
                    push = tmp_obj;
                    push_count = 1;
                } else
                    push_count++;
                found = TRUE;
            }
        }
        if (push && push_count)
            show_obj_to_char(push, ch, 1, ch == i, push_count);


        if (!found)
            send_to_char("...но ничего не обнаружили.\r\n", ch);

        if (CAN_SEE(i, ch) && prob + GET_REAL_LCK(ch) < number(1, 50)) {
            act("2+и заметил2(,а,о,и) Вашу попытку рассмотреть 2ев имущество.", "Мм", ch, i);
            act("Вы заметили, что 1+и пытается узнать о содержимом Ваших карманов.", "мМ", ch, i);
            act("2+и заметил2(,а,о,и), что 1+и попытал1(ася,ась,ось,ись) заглянуть к н2ед в карман.", "Кмм", ch, i);
            inc_pk_thiefs(ch, i);
            if (IS_SHOPKEEPER(i))
                act("2и пригрозил(,а,о,и) позвать охрану!", "Мм", ch, i);
            else if (IS_NPC(i) && (dice(1, 20) > GET_REAL_LCK(ch))) {
                check_position(i);
                _damage(i, ch, WEAP_RIGHT, 0, C_POWER, TRUE);
            }
        }
    }
}

ACMD(do_peek)
{
    struct char_data *tmp_char;
    struct obj_data *tmp_object;
    char arg[MAX_STRING_LENGTH];
    int bits;

    if (!check_fight_command(ch))
        return;

    if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_LOOK_HIDE)) {
        send_to_char("Вы не знаете как.\r\n", ch);
        return;
    }

    if (GET_POS(ch) < POS_SLEEPING) {
        send_to_char("Виделся часто сон беспокойный...\r\n", ch);
        return;
    } else if (AFF_FLAGGED(ch, AFF_BLIND)) {
        send_to_char("Вы слепы!\r\n", ch);
        return;
    }

    if (FIGHTING(ch)) {
        send_to_char("Вы сражаетесь за свою жизнь.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("К кому Вы хотите приглядеться?\r\n", ch);
        return;
    }

    bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
                        FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

    if (tmp_char != NULL) {
        go_look_hide(ch, tmp_char);
        return;
    }
    send_to_charf(ch, "Здесь нет \'%s\'.\r\n", arg);

}


ACMD(do_hearing)
{
    int i;

    if (!ch->desc)
        return;


    if (GET_POS(ch) < POS_SLEEPING)
        send_to_char("Вам начали слышаться голоса предков, зовущие Вас к себе.\r\n", ch);
    if (GET_POS(ch) == POS_SLEEPING)
        send_to_char("Морфей медленно задумчиво провел рукой по струнам и заиграл колыбельную.\r\n",
                     ch);
    else
     if (check_moves(ch, HEARING_MOVES)) {
        send_to_char("Вы начали сосредоточенно прислушиваться.\r\n", ch);
        for (i = 0; i < NUM_OF_DIRS; i++)
            hear_in_direction(ch, i, 0);
        GET_MISSED(ch)++;
        if (!IS_IMMORTAL(ch))
            WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
    }
}


ACMD(do_gold)
{
    char buf[MAX_STRING_LENGTH];

    int count = 0;

    if (GET_GOLD(ch) == 0)
        send_to_char("У Вас в кошельке пусто!\r\n", ch);
    else if (GET_GOLD(ch) == 1)
        send_to_char("У Вас в кошельке всего одна монета.\r\n", ch);
    else {
        count +=
            sprintf(buf, "У Вас в кошельке есть %d %s.\r\n", GET_GOLD(ch),
                    desc_count(GET_GOLD(ch), WHAT_MONEYa));
        send_to_char(buf, ch);
    }
}


const char *wear_text[] = {
    "Вы ничего не несете.",
    "Вы не чувствуете веса вещей.",
    "Вы слегка нагружены.",
    "Вы сильно нагружены.",
    "Вы сгибаетесь под весом вещей.",
    "Вы не способны сдвинуться с места."
};

ACMD(do_score)
{
    struct time_info_data playing_time;
    int ticks, tick_day, tick_hours, next_level;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    *buf = '\0';
    *buf2 = '\0';

    if (IS_NPC(ch))
        return;


    if (IS_SOUL(ch)) {
        sprintf(buf, "Вы неупокоенная душа %s %s %d уровня",
                GET_SEX(ch) ==
                SEX_FEMALE ? get_name_pad((char *) race_name_pad_female[(int) GET_RACE(ch)], 1,
                                          PAD_MONSTER) : get_name_pad((char *)
                                                                      race_name_pad_male[(int)
                                                                                         GET_RACE
                                                                                         (ch)], 1,
                                                                      PAD_MONSTER), GET_PAD(ch, 1),
                GET_LEVEL(ch));
        sprintf(buf + strlen(buf), ", Вам %d %s.\r\n", GET_AGE(ch),
                desc_count(GET_AGE(ch), WHAT_YEAR));
        if (GET_GODS(ch))
            sprintf(buf + strlen(buf), "Вы поклоняетесь %s.\r\n", gods_name_2[(int) GET_GODS(ch)]);
        else if (GET_SEX(ch) == SEX_FEMALE)
            sprintf(buf + strlen(buf), "Вы атеистка.\r\n");
        else
            sprintf(buf + strlen(buf), "Вы атеист.\r\n");

        sprintf(buf + strlen(buf), "Через %d %s Вы переродитесь.\r\n", EXTRACT_TIMER(ch),
                desc_count(EXTRACT_TIMER(ch), WHAT_MINu));
        sprintf(buf + strlen(buf), "Вы медленно парите над своим мертвым телом.\r\n");
        send_to_charf(ch, buf);
        return;
    }

    if (!NAME_GOD(ch)) {
        sprintf(buf + strlen(buf), "&RВаше имя не одобрено.&n\r\n");
    } else if (NAME_GOD(ch) < 1000) {
        sprintf(buf + strlen(buf), "&RВаше имя запретил кто-то из Богов.&n\r\n");
    }

    sprintf(buf + strlen(buf), "Вы %s %s",
            race_name[(int) GET_RACE(ch)][(int) GET_SEX(ch)], GET_NAME(ch));

    if (GET_LEVEL(ch))
        sprintf(buf + strlen(buf), " %d уровня", GET_LEVEL(ch) + GET_LEVEL_ADD(ch));

    sprintf(buf + strlen(buf), ", Вам %d %s.", GET_AGE(ch), desc_count(GET_AGE(ch), WHAT_YEAR));


    if (age_old(ch)->month == 0 && age_old(ch)->day == 0) {
        sprintf(buf2, " &GУ Вас сегодня День Рождения.&n\r\n");
        strcat(buf, buf2);
    } else
        strcat(buf, "\r\n");

    if (GET_RTITLE(ch))
        sprintf(buf + strlen(buf), "Вы запросили титул: \"%s\"\r\n", GET_RTITLE(ch));
    else if (GET_TITLE(ch))
        sprintf(buf + strlen(buf), "Ваш титул: \"%s\"\r\n", only_title(ch));

    ////////////

    if (GET_GODS(ch))
        sprintf(buf + strlen(buf), "Вы поклоняетесь %s.\r\n", gods_name_2[(int) GET_GODS(ch)]);
    else
        sprintf(buf + strlen(buf), "Вы атеист.\r\n");


    *buf2 = '\0';
    for (int icls = 0; icls < NUM_CLASSES; icls++) {
        if (ch->classes[icls] > 0)
            sprintf(buf2 + strlen(buf2), "%10s %-2d уровня.\r\n", class_name[icls],
                    ch->classes[icls]);

    }

    if (*buf2)
        sprintf(buf + strlen(buf), "Вы владеете следующими профессиями:\r\n%s", buf2);
    else
        sprintf(buf + strlen(buf), "Вы не знаете ни одной профессии.\r\n");
    send_to_char(buf, ch);

    ////////////
    *buf = '\0';

    if (IS_MANA_CASTER(ch))
        sprintf(buf + strlen(buf),
                "У Вас сейчас %d(%d) %s жизни, %d(%d) %s маны и %d(%d) %s бодрости.\r\n",
                GET_HIT(ch), GET_REAL_MAX_HIT(ch), desc_count(GET_HIT(ch), WHAT_ONEa),
                GET_MANA(ch), GET_REAL_MAX_MANA(ch), desc_count(GET_HIT(ch), WHAT_ONEa),
                GET_MOVE(ch), GET_REAL_MAX_MOVE(ch), desc_count(GET_MOVE(ch), WHAT_ONEa));
    else
        sprintf(buf + strlen(buf),
                "У Вас сейчас %d(%d) %s жизни и %d(%d) %s бодрости.\r\n",
                GET_HIT(ch), GET_REAL_MAX_HIT(ch), desc_count(GET_HIT(ch), WHAT_ONEa),
                GET_MOVE(ch), GET_REAL_MAX_MOVE(ch), desc_count(GET_MOVE(ch), WHAT_ONEa));


    sprintf(buf + strlen(buf), "Ваш опыт составляет %ld %s.\r\n",
            GET_EXP(ch), desc_count(GET_EXP(ch), WHAT_POINT));

    next_level = get_dsu_exp(ch);

    if (GET_LEVEL(ch) < LVL_IMMORT) {
        if (next_level > 0)
            sprintf(buf + strlen(buf), "Вам осталось набрать %d %s до следующего уровня.\r\n",
                    next_level, desc_count(next_level, WHAT_POINT));
        else
            sprintf(buf + strlen(buf), "У Вас достаточно опыта, чтобы повысить свой уровень.\r\n");
    }
    if (GET_HONOR(ch))
        sprintf(buf + strlen(buf), "У вас %ld %s чести.\r\n",
                GET_HONOR(ch), desc_count(GET_HONOR(ch), WHAT_POINT));


    sprintf(buf + strlen(buf), "У Вас есть %d %s наличными.\r\n",
            GET_GOLD(ch), desc_count(GET_GOLD(ch), WHAT_MONEYa));

    /*  if (GET_BANK_GOLD(ch) > 0)
       sprintf(buf + strlen(buf)," и еще %ld %s лежат в банке.\r\n",
       GET_BANK_GOLD(ch), desc_count(GET_BANK_GOLD(ch),WHAT_MONEYa));
       else
       strcat(buf,".\r\n");       */

    if (GET_GLORY(ch))
        sprintf(buf + strlen(buf), "Вы заслужили %d %s славы.\r\n",
                GET_GLORY(ch), desc_count(GET_GLORY(ch), WHAT_POINT));

    playing_time = *real_time_passed((time(0) - ch->player.time.logon) + ch->player.time.played, 0);

    sprintf(buf + strlen(buf), "Вы говорите %s.\r\n", languages_d[SPEAKING(ch) - MIN_LANGUAGES]);

    ticks = GET_TICKS(ch);
    tick_day = ticks / 1440;
    tick_hours = (ticks - (tick_day * 1440)) / 60;

    sprintf(buf + strlen(buf), "Вы играете %d %s %d %s реального времени.\r\n",
            tick_day, desc_count(tick_day, WHAT_DAY),
            tick_hours, desc_count(tick_hours, WHAT_HOUR));


    if (IS_CARRYING_W(ch) + IS_WEARING_W(ch)) {
        int percent = (((IS_CARRYING_W(ch) + IS_WEARING_W(ch)) * 100) / MAX_MOVE_W(ch)) / 25;
        int wr_t = MAX(MIN(percent + 1, 5), 1);

        sprintf(buf + strlen(buf), "%s\r\n", wear_text[wr_t]);
    } else
        sprintf(buf + strlen(buf), "Вы ничего не несете.\r\n");

    if (!on_horse(ch))
        switch (GET_POS(ch)) {
            case POS_DEAD:
                strcat(buf, "&RВы МЕРТВЫ!&n\r\n");
                break;
            case POS_MORTALLYW:
                strcat(buf, "Вы смертельно ранены и нуждаетесь в помощи!\r\n");
                break;
            case POS_INCAP:
                strcat(buf, "Вы без сознания и медленно умираете...\r\n");
                break;
            case POS_STUNNED:
                strcat(buf, "Вы в потеряли сознание!\r\n");
                break;
            case POS_SLEEPING:
                strcat(buf, "Вы спите.\r\n");
                break;
            case POS_RESTING:
                strcat(buf, "Вы отдыхаете.\r\n");
                break;
            case POS_SITTING:
                strcat(buf, "Вы сидите.\r\n");
                break;
            case POS_FIGHTING:
                if (FIGHTING(ch))
                    sprintf(buf + strlen(buf), "Вы сражаетесь с %s.\r\n", GET_PAD(FIGHTING(ch), 4));
                else
                    strcat(buf, "Вы машете кулаками по воздуху.\r\n");
                break;
            case POS_STANDING:
                strcat(buf, "Вы стоите.\r\n");
                break;
            case POS_FLYING:
                strcat(buf, "Вы летаете.\r\n");
                break;
            default:
                strcat(buf, "You are floating.\r\n");
                break;
        }
    send_to_char(buf, ch);

    if (ch->trap_object)
        send_to_charf(ch, "Вы попали в %s.\r\n", GET_OBJ_PNAME(ch->trap_object, 0));

    if (ch->master)
        send_to_charf(ch, "Вы следуете за %s.\r\n", hide_race(ch->master, 4));
    if (ch->party_leader)
        send_to_charf(ch, "Вы состоите в группе %s.\r\n", hide_race(ch->party_leader, 1));
    if (ch->party)
        send_to_charf(ch, "Вы лидер группы.\r\n");

    if (GUARDING(ch))
        send_to_charf(ch, "Вы охраняете %s.\r\n", GET_PAD(GUARDING(ch), 3));

    if (ch->is_transpt) {
        *buf = '\0';
        sprintf(buf + strlen(buf), "Вы сидите в %s", GET_OBJ_PNAME(ch->is_transpt, 5));
        if (ch->is_transpt->transpt->driver)
            sprintf(buf + strlen(buf), " и управляете");
        sprintf(buf + strlen(buf), ".\r\n");
        send_to_char(buf, ch);
    }

    const char *mess = '\0';

    if ((mess = get_status_event(ch)))
        send_to_charf(ch, "%s.\r\n", mess);

    *buf = '\0';

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) && IS_GOD(ch))
        strcat(buf, "Вы чувствуете здесь мир и спокойствие.\r\n");

    if (GET_COND(ch, FULL) > 0 && GET_COND(ch, FULL) <= SECS_PER_MUD_TICK)
        strcat(buf, "Вы немного проголодались.\r\n");
    if (GET_COND(ch, THIRST) > 0 && GET_COND(ch, THIRST) <= SECS_PER_MUD_TICK)
        strcat(buf, "Вы хотите попить.\r\n");
    if (GET_COND(ch, SLEEP) > 0 &&
        GET_COND(ch, SLEEP) <= SECS_PER_MUD_TICK && GET_POS(ch) != POS_SLEEPING)
        strcat(buf, "Вы давно не спали.\r\n");

    strcat(buf, CCIRED(ch, C_NRM));
    if (GET_COND(ch, DRUNK) >= CHAR_DRUNKED) {
        strcat(buf, "Вы пьяны.\r\n");
    }

    if (GET_COND(ch, FULL) == 0)
        strcat(buf, "Вы хотите есть.\r\n");
    if (GET_COND(ch, THIRST) == 0)
        strcat(buf, "Вас мучает жажда.\r\n");
    if (GET_COND(ch, SLEEP) == 0)
        strcat(buf, "Вы хотите спать.\r\n");
    strcat(buf, CCNRM(ch, C_NRM));

    /*  if (PRF_FLAGGED(ch, PRF_SUMMONABLE))
       strcat(buf, "\r\nВы можете быть призваны.\r\n"); */

    if (has_horse(ch, FALSE)) {
        if (on_horse(ch))
            sprintf(buf + strlen(buf), "Вы верхом на %s.\r\n", GET_PAD(get_horse(ch), 5));
        else
            sprintf(buf + strlen(buf), "У Вас есть %s.\r\n", GET_NAME(get_horse(ch)));
    }

    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    if (RENTABLE(ch)) {
        sprintf(buf, "%sВ связи с боевыми действиями Вы не можете уйти на постой.%s\r\n",
                CCIRED(ch, C_NRM), CCNRM(ch, C_NRM));
        send_to_char(buf, ch);
    }
    if (PLR_FLAGGED(ch, PLR_HELLED) && HELL_DURATION(ch) && HELL_DURATION(ch) > time(NULL)
        ) {
        int hrs = (HELL_DURATION(ch) - time(NULL)) / 3600;
        int mins = ((HELL_DURATION(ch) - time(NULL)) % 3600 + 59) / 60;

        sprintf(buf, "Вам предстоит провести в аду еще %d %s %d %s.\r\n",
                hrs, desc_count(hrs, WHAT_HOUR), mins, desc_count(mins, WHAT_MINu));
        send_to_char(buf, ch);
    }
}


ACMD(do_inventory)
{
    char buf[MAX_STRING_LENGTH];

    strcpy(buf, "У Вас в руках:\r\n");
    if (ch->carrying)
        strcat(buf, buf_obj_to_char(ch->carrying, ch));
    else
        strcat(buf, "ничего нет.\r\n");
    send_to_charf(ch, buf);
}


ACMD(do_equipment)
{
    int i, found = 0;

    send_to_char("У Вас в экипировке:\r\n", ch);
    for (i = 0; i < NUM_WEARS; i++) {
        /*      if (GET_EQ(ch, i))
           {if (CAN_SEE_OBJ(ch, GET_EQ(ch, i)))
           {send_to_char(where[i], ch);
           show_obj_to_char(GET_EQ(ch, i), ch, 1, TRUE, 1); */

        if (GET_EQ(ch, wear_order_index[i])) {
            if (CAN_SEE_OBJ(ch, GET_EQ(ch, wear_order_index[i]))) {
                send_to_charf(ch, " %-20s", where[wear_order_index[i]]);
                show_obj_to_char(GET_EQ(ch, wear_order_index[i]), ch, 1, TRUE, 1);
                found = TRUE;
            } else {
                send_to_charf(ch, " %-20s", where[wear_order_index[i]]);
                send_to_char("что-то.\r\n", ch);
                found = TRUE;
            }
        } else if (GET_TATOO(ch, wear_order_index[i])) {
            send_to_charf(ch, " %-20s", where_tatoo[wear_order_index[i]]);
            show_obj_to_char(GET_TATOO(ch, wear_order_index[i]), ch, 1, TRUE, 1);
            found = TRUE;
        }

    }
    if (!found) {
        if (GET_SEX(ch) == SEX_FEMALE)
            send_to_char("Вы раздеты.\r\n", ch);
        else
            send_to_char("Вы раздеты.\r\n", ch);
    }
}


ACMD(do_time)
{
    char buf[MAX_STRING_LENGTH];
    int day;

    if (IS_NPC(ch))
        return;
    sprintf(buf, "Сейчас ");

    switch (zone_table[world[IN_ROOM(ch)].zone].time_info.hours % 24) {
        case 0:
            sprintf(buf + strlen(buf), "полночь, ");
            break;
        case 1:
            sprintf(buf + strlen(buf), "1 час ночи, ");
            break;
        case 2:
        case 3:
        case 4:
            sprintf(buf + strlen(buf), "%d часа ночи, ",
                    zone_table[world[IN_ROOM(ch)].zone].time_info.hours);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            sprintf(buf + strlen(buf), "%d часов утра, ",
                    zone_table[world[IN_ROOM(ch)].zone].time_info.hours);
            break;
        case 12:
            sprintf(buf + strlen(buf), "полдень, ");
            break;
        case 13:
            sprintf(buf + strlen(buf), "1 час пополудни, ");
            break;
        case 14:
        case 15:
        case 16:
            sprintf(buf + strlen(buf), "%d часа пополудни, ",
                    zone_table[world[IN_ROOM(ch)].zone].time_info.hours - 12);
            break;
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
            sprintf(buf + strlen(buf), "%d часов вечера, ",
                    zone_table[world[IN_ROOM(ch)].zone].time_info.hours - 12);
            break;
    }

    strcat(buf, weekdays[zone_table[world[IN_ROOM(ch)].zone].weather_info.week_day]);
    switch (zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight) {
        case SUN_DARK:
            strcat(buf, ", темно");
            break;
        case SUN_SET:
            strcat(buf, ", сумрачно");
            break;
        case SUN_LIGHT:
            strcat(buf, ", светло");
            break;
        case SUN_RISE:
            strcat(buf, ", рассветает");
            break;
    }
    strcat(buf, ".\r\n");
    send_to_char(buf, ch);

    day = zone_table[world[IN_ROOM(ch)].zone].time_info.day + 1;        /* day in [1..35] */

    *buf = '\0';
    sprintf(buf + strlen(buf), "%s, %dй День, Год %d",
            month_name[(int) zone_table[world[IN_ROOM(ch)].zone].time_info.month], day,
            zone_table[world[IN_ROOM(ch)].zone].time_info.year);
    switch (zone_table[world[IN_ROOM(ch)].zone].weather_info.season) {
        case SEASON_WINTER:
            strcat(buf, ", зима");
            break;
        case SEASON_SPRING:
            strcat(buf, ", весна");
            break;
        case SEASON_SUMMER:
            strcat(buf, ", лето");
            break;
        case SEASON_AUTUMN:
            strcat(buf, ", осень");
            break;
    }
    strcat(buf, ".\r\n");
    send_to_char(buf, ch);
}

int get_moon(int sky)
{
    if (weather_info.sunlight == SUN_RISE ||
        weather_info.sunlight == SUN_LIGHT || sky == SKY_RAINING)
        return (0);
    else if (weather_info.moon_day <= NEWMOONSTOP || weather_info.moon_day >= NEWMOONSTART)
        return (1);
    else if (weather_info.moon_day < HALFMOONSTART)
        return (2);
    else if (weather_info.moon_day < FULLMOONSTART)
        return (3);
    else if (weather_info.moon_day <= FULLMOONSTOP)
        return (4);
    else if (weather_info.moon_day < LASTHALFMOONSTART)
        return (5);
    else
        return (6);
    return (0);
}



ACMD(do_weather)
{
    int sky = world[IN_ROOM(ch)].weather.sky, weather_type = weather_info.weather_type;
    char buf[MAX_STRING_LENGTH];

    const char *sky_look[] = { "Небо облачное",
        "Небо пасмурное",
        "Небо покрыто тяжелыми тучами",
        "Небо ясное"
    };

    const char *sky_look_sigil[] = { "Небольшой смог",
        "Смог",
        "Густой смог",
        "Ясно"
    };

    if (OUTSIDE(ch)) {
        *buf = '\0';
        if (world[IN_ROOM(ch)].weather.duration > 0) {
            sky = world[IN_ROOM(ch)].weather.sky;
            weather_type = world[IN_ROOM(ch)].weather.weather_type;
        }
        sprintf(buf + strlen(buf),
                "%s.\r\n%s\r\n",
                (GET_ZONE(ch).plane == PLAN_SIGIL) ? sky_look_sigil[sky] : sky_look[sky],
                (weather_info.change >=
                 0 ? "Атмосферное давление повышается." : "Атмосферное давление понижается."));
        sprintf(buf + strlen(buf), "Температура: %d %s.\r\n", weather_info.temperature,
                desc_count(weather_info.temperature, WHAT_DEGREE));

        if (IS_SET(weather_info.weather_type, WEATHER_BIGWIND))
            strcat(buf, "Сильный ветер.\r\n");
        else if (IS_SET(weather_info.weather_type, WEATHER_MEDIUMWIND))
            strcat(buf, "Умеренный ветер.\r\n");
        else if (IS_SET(weather_info.weather_type, WEATHER_LIGHTWIND))
            strcat(buf, "Легкий ветерок.\r\n");

        if (IS_SET(weather_type, WEATHER_BIGSNOW))
            strcat(buf, "Валит снег.\r\n");
        else if (IS_SET(weather_type, WEATHER_MEDIUMSNOW))
            strcat(buf, "Снегопад.\r\n");
        else if (IS_SET(weather_type, WEATHER_LIGHTSNOW))
            strcat(buf, "Легкий снежок.\r\n");

        if (IS_SET(weather_type, WEATHER_GRAD))
            strcat(buf, "Дождь с градом.\r\n");
        else if (IS_SET(weather_type, WEATHER_BIGRAIN))
            strcat(buf, "Сильный ливень.\r\n");
        else if (IS_SET(weather_type, WEATHER_MEDIUMRAIN))
            strcat(buf, "Идет дождь.\r\n");
        else if (IS_SET(weather_type, WEATHER_LIGHTRAIN))
            strcat(buf, "Моросит дождик.\r\n");

        send_to_char(buf, ch);
    } else
        send_to_char("Вы ничего не можете сказать о погоде сегодня.\r\n", ch);
    if (IS_GOD(ch)) {
        sprintf(buf, "День: %d Месяц: %s Час: %d Такт = %d\r\n"
                "Температура =%-5d, за день = %-8d, за неделю = %-8d\r\n"
                "Давление    =%-5d, за день = %-8d, за неделю = %-8d\r\n"
                "Выпало дождя = %d(%d), Лед = %d(%d). Погода = %08x(%08x).\r\n",
                time_info.day, month_name[time_info.month], time_info.hours, weather_info.hours_go,
                weather_info.temperature, weather_info.temp_last_day, weather_info.temp_last_week,
                weather_info.pressure, weather_info.press_last_day, weather_info.press_last_week,
                weather_info.rainlevel, world[IN_ROOM(ch)].weather.rainlevel,
                weather_info.icelevel, world[IN_ROOM(ch)].weather.icelevel,
                weather_info.weather_type, world[IN_ROOM(ch)].weather.weather_type);
        send_to_char(buf, ch);
    }
}



#define IMM_WHO_FORMAT \
    "Формат: кто [минуров[-максуров]] [-n имя] [-c профлист] [-s] [-o] [-q] [-r] [-z] [-h]\r\n"

#define MORT_WHO_FORMAT \
    "Формат: кто [имя]\r\n"


ACMD(do_who)
{
    struct descriptor_data *d;
    struct char_data *tch;
    char name_search[MAX_INPUT_LENGTH] = "\0",
        name[MAX_STRING_LENGTH] = "\0",
        lvl[MAX_STRING_LENGTH] = "\0",
        imms[MAX_STRING_LENGTH] = "\0",
        morts[MAX_STRING_LENGTH] = "\0", buf[MAX_EXTEND_LENGTH] = "\0";
    int num_imms = 0, num_morts = 0, i;
    bool only_gods = FALSE;


    skip_spaces(&argument);
    strcpy(name_search, argument);

    if (!str_cmp(name_search, "боги") && strlen(name_search) == 4)
        only_gods = TRUE;

    sprintf(imms, "%sБессмертные%s\r\n-----------------\r\n", CCICYN(ch, C_NRM), CCNRM(ch, C_NRM));
    sprintf(morts, "%sСмертные%s\r\n-----------------\r\n", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));

    for (d = descriptor_list; d; d = d->next) {
        if (STATE(d) != CON_PLAYING)
            continue;
        if (d->original)
            tch = d->original;
        else if (!(tch = d->character))
            continue;

        if (only_gods && !IS_IMMORTAL(tch))
            continue;

        if (!CAN_SEE_CHAR(ch, tch))
            continue;

        if ((!IS_IMMORTAL(ch) && !NAME_GOD(tch)) && (tch != ch))
            continue;

        if (!only_gods && *name_search && !isname(name_search, GET_NAME(tch)))
            continue;

        *name = '\0';

        if (IS_GOD(ch)) {
            char rot[MAX_STRING_LENGTH] = "\0";

            *lvl = '\0';
            for (i = 0; i < NUM_CLASSES; i++)
                if (tch->classes[i])
                    sprintf(lvl + strlen(lvl), "%s%d ", class_abbrevs[i], tch->classes[i]);

            if (!NAME_GOD(tch))
                sprintf(rot, "%s %s(%s,%s,%s,%s,%s)",
                        race_name[(int) GET_RACE(tch)][(int) GET_SEX(tch)], GET_PAD(tch, 0),
                        GET_PAD(tch, 1), GET_PAD(tch, 2), GET_PAD(tch, 3), GET_PAD(tch, 4),
                        GET_PAD(tch, 5));
            else
                sprintf(rot, "%s", race_or_title2(tch));

            sprintf(name, "[%5d] %-30s %s", GET_LEVEL(tch), rot, lvl);
        } else
            sprintf(name, "%s", race_or_title2(tch));

        if (PRF_FLAGGED(tch, PRF_CODERINFO) && IS_HIGOD(ch))
            sprintf(name + strlen(name), " (тест)");
        if (IS_SOUL(tch) && IS_GOD(ch))
            sprintf(name + strlen(name), " (призрак)");
        if (GET_INVIS_LEV(tch) && IS_GOD(ch))
            sprintf(name + strlen(name), " (и%d)", GET_INVIS_LEV(tch));
        else if (AFF_FLAGGED(tch, AFF_INVISIBLE) && IS_GOD(ch))
            sprintf(name + strlen(name), " (невидим%s)", GET_CH_SUF_6(tch));
        if (AFF_FLAGGED(tch, AFF_HIDE) && IS_GOD(ch))
            strcat(name, " (прячется)");
        if (AFF_FLAGGED(tch, AFF_CAMOUFLAGE) && IS_GOD(ch))
            strcat(name, " (маскируется)");
        if (PLR_FLAGGED(tch, PLR_MAILING) && IS_GOD(ch))
            strcat(name, " (отправляет письмо)");
        else if (PLR_FLAGGED(tch, PLR_SCRIPTING) && IS_GOD(ch))
            strcat(name, " (пишет сценарий)");
        else if (PLR_FLAGGED(tch, PLR_WRITING) && IS_GOD(ch))
            strcat(name, " (пишет)");
        if (PRF_FLAGGED(tch, PRF_NOHOLLER) && IS_GOD(ch))
            sprintf(name + strlen(name), " (глух%s)", GET_CH_SUF_1(tch));
        if (PRF_FLAGGED(tch, PRF_NOTELL) && IS_GOD(ch))
            sprintf(name + strlen(name), " (занят%s)", GET_CH_SUF_6(tch));
        if (PLR_FLAGGED(tch, PLR_MUTE) && IS_GOD(ch))
            sprintf(name + strlen(name), " (молчит)");
        if (PLR_FLAGGED(tch, PLR_DUMB) && IS_GOD(ch))
            sprintf(name + strlen(name), " (нем%s)", GET_CH_SUF_6(tch));

        strcat(name, "\r\n");

        if (IS_IMMORTAL(tch)) {
            strcat(imms, name);
            num_imms++;
        } else {
            strcat(morts, name);
            num_morts++;
        }
    }

    if (num_imms)
        strcat(buf, imms);
    if (num_morts) {
        if (num_imms)
            strcat(buf, "\r\n");
        strcat(buf, morts);
    }
    if (num_morts + num_imms == 0) {
        send_to_char("\r\nВы никого не видите.\r\n", ch);
        return;
    }

    sprintf(buf + strlen(buf), "\r\nСейчас в игре:");

    if (num_imms)
        sprintf(buf + strlen(buf), " бессмертных %d%s", num_imms, (num_morts) ? "," : ".");

    if (num_morts)
        sprintf(buf + strlen(buf), " смертных %d.", num_morts);


    strcat(buf, "\r\n");

    if (!only_gods && !*name_search)
        sprintf(buf + strlen(buf),
                "Максимальное кол-во с момента перезагрузки: %d %s.\r\n",
                mud->getStats()->getHigh(), desc_count(mud->getStats()->getHigh(), WHAT_PERSONA));

    page_string(ch->desc, buf, 1);
}


#define USERS_FORMAT \
    "Формат: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c classlist] [-o] [-p]\r\n"
#define MAX_LIST_LEN 200
ACMD(do_users)
{
    char *get_addr_ip(char *arg);
    const char *format = "%3d %-7s %-12s %-14s %-3s %-8s ";
    char line[200], line2[220], idletime[10], classname[20];
    char state[30] = "\0", *timeptr, mode;
    char name_search[MAX_INPUT_LENGTH] = "\0", host_search[MAX_INPUT_LENGTH];

// Хорс
    char host_by_name[MAX_INPUT_LENGTH] = "\0";
    struct descriptor_data *list_players[MAX_LIST_LEN];
    struct descriptor_data *d_tmp;
    int count_pl;
    int cycle_i, is, flag_change;
    unsigned long a1, a2;
    int showemail = 0, locating = 0;
    char sorting = '!';
    register struct char_data *ci;

// ---
    struct char_data *tch, *t, *t_tmp;
    struct descriptor_data *d;
    int low = 0, high = LVL_IMPL, num_can_see = 0;
    int outlaws = 0, playing = 0, deadweight = 0;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];

    host_search[0] = name_search[0] = '\0';

    strcpy(buf, argument);
    while (*buf) {
        half_chop(buf, arg, buf1);
        if (*arg == '-') {
            mode = *(arg + 1);  /* just in case; we destroy arg in the switch */
            switch (mode) {
                case 'o':
                case 'k':
                    outlaws = 1;
                    playing = 1;
                    strcpy(buf, buf1);
                    break;
                case 'p':
                    playing = 1;
                    strcpy(buf, buf1);
                    break;
                case 'd':
                    deadweight = 1;
                    strcpy(buf, buf1);
                    break;
                case 'l':
                    playing = 1;
                    half_chop(buf1, arg, buf);
                    sscanf(arg, "%d-%d", &low, &high);
                    break;
                case 'n':
                    playing = 1;
                    half_chop(buf1, name_search, buf);
                    break;
                case 'h':
                    playing = 1;
                    half_chop(buf1, host_search, buf);
                    break;
                case 'u':
                    playing = 1;
                    half_chop(buf1, host_by_name, buf);
                    break;
                case 'w':
                    playing = 1;
                    locating = 1;
                    strcpy(buf, buf1);
                    break;
                case 'e':
                    showemail = 1;
                    strcpy(buf, buf1);
                    break;
                case 's':
                    sorting = 'i';
                    sorting = *(arg + 2);
                    strcpy(buf, buf1);
                    break;
                default:
                    send_to_char(USERS_FORMAT, ch);
                    return;
            }                   /* end of switch */

        } else {                /* endif */
            strcpy(name_search, arg);
            strcpy(buf, buf1);
        }
    }                           /* end while (parser) */
    if (showemail) {
        strcpy(line, "Ном Професс    Имя         Состояние    Idl Логин    E-mail\r\n");
    } else {
        strcpy(line, "Ном Професс    Имя         Состояние    Idl Логин    Сайт\r\n");
    }
    strcat(line,
           "--- ------- ------------ -------------- --- -------- ------------------------\r\n");
    send_to_char(line, ch);

    one_argument(argument, arg);

// Хорс
    if (strlen(host_by_name) != 0)
        strcpy(host_search, "!");
    for (d = descriptor_list, count_pl = 0; d && count_pl < MAX_LIST_LEN; d = d->next, count_pl++) {
        list_players[count_pl] = d;
        if (d->original)
            tch = d->original;
        else if (!(tch = d->character))
            continue;
        if (host_by_name != 0)
            if (isname(host_by_name, GET_NAME(tch)))
                strcpy(host_search, d->host);
    }
    if (sorting != '!') {
        is = 1;
        while (is) {
            is = 0;
            for (cycle_i = 1; cycle_i < count_pl; cycle_i++) {
                flag_change = 0;
                d = list_players[cycle_i - 1];
                if (d->original)
                    t = d->original;
                else
                    t = d->character;
                d_tmp = list_players[cycle_i];
                if (d_tmp->original)
                    t_tmp = d_tmp->original;
                else
                    t_tmp = d_tmp->character;
                switch (sorting) {
                    case 'n':
                        if (strcoll(t ? t->player.name : "", t_tmp ? t_tmp->player.name : "") > 0)
                            flag_change = 1;
                        break;
                    case 'e':
                        if (strcoll(t ? GET_EMAIL(t) : "", t_tmp ? GET_EMAIL(t_tmp) : "") > 0)
                            flag_change = 1;
                        break;
                    default:
                        a1 = get_ip((const char *) d->host);
                        a2 = get_ip((const char *) d_tmp->host);

                        if (a1 > a2)
                            flag_change = 1;
                }
                if (flag_change) {
                    list_players[cycle_i - 1] = d_tmp;
                    list_players[cycle_i] = d;
                    is = 1;
                }
            }
        }
    }

    for (cycle_i = 0; cycle_i < count_pl; cycle_i++) {
        d = (struct descriptor_data *) list_players[cycle_i];
// ---
        if (STATE(d) != CON_PLAYING && playing)
            continue;
        if (STATE(d) == CON_PLAYING && deadweight)
            continue;
        if (STATE(d) == CON_PLAYING) {
            if (d->original)
                tch = d->original;
            else if (!(tch = d->character))
                continue;

            if (*host_search && !strstr(d->host, host_search))
                continue;
            if (*name_search && !isname(name_search, GET_NAME(tch)))
                continue;
            if (!CAN_SEE(ch, tch) || GET_LEVEL(tch) < low || GET_LEVEL(tch) > high)
                continue;
            if (outlaws && !IS_KILLER(tch))
                continue;
            if (GET_INVIS_LEV(ch) > GET_LEVEL(ch))
                continue;

            if (d->original)
                sprintf(classname, "[%2d %s]", GET_LEVEL(d->original), CLASS_ABBR(d->original));
            else
                sprintf(classname, "[%2d %s]", GET_LEVEL(d->character), CLASS_ABBR(d->character));
        } else
            strcpy(classname, "   -   ");
// Добавленно Стрибогом
        if (GET_LEVEL(ch) < LVL_GOD)
            strcpy(classname, "   -   ");
//--
        timeptr = ascii_time(d->login_time);
        timeptr += 11;
        *(timeptr + 8) = '\0';

        if (STATE(d) == CON_PLAYING && d->original)
            strcpy(state, "Switched");
        else
            strcpy(state, connected_types[STATE(d)]);

        if (d->character && STATE(d) == CON_PLAYING && !IS_GOD(d->character))
            sprintf(idletime, "%3d", d->character->char_specials.timer *
                    SECS_PER_MUD_TICK / SECS_PER_REAL_MIN);
        else
            strcpy(idletime, "");

        if (d->character && d->character->player.name) {
            if (d->original)
                sprintf(line, format, d->desc_num, classname,
                        d->original->player.name, state, idletime, timeptr);
            else
                sprintf(line, format, d->desc_num, classname,
                        d->character->player.name, state, idletime, timeptr);
        } else
            sprintf(line, format, d->desc_num, "   -   ", "UNDEFINED", state, idletime, timeptr);
// Хорс
        if (showemail) {
            sprintf(line2, "[%s]",
                    d->original ? GET_EMAIL(d->original) : d->character ? GET_EMAIL(d->
                                                                                    character) :
                    "");
            strcat(line, line2);
        } else if (d->host && *d->host) {
            sprintf(line2, "[%s](%s)", d->host, get_addr_ip(d->host));
            strcat(line, line2);
        } else
            strcat(line, "[Неизвестный хост]");
        if (locating)
            if ((STATE(d) == CON_PLAYING)) {
                ci = (d->original ? d->original : d->character);
                if (ci && CAN_SEE(ch, ci) && (ci->in_room != NOWHERE)) {
                    if (d->original)
                        sprintf(line2, " [%6d] %s (in %s)",
                                GET_ROOM_VNUM(IN_ROOM(d->character)),
                                world[d->character->in_room].name, GET_NAME(d->character));
                    else
                        sprintf(line2, " [%6d] %s",
                                GET_ROOM_VNUM(IN_ROOM(ci)), world[ci->in_room].name);
                }
                strcat(line, line2);
            }
//--
        strcat(line, "\r\n");
        if (STATE(d) != CON_PLAYING) {
            sprintf(line2, "%s%s%s", CCGRN(ch, C_SPR), line, CCNRM(ch, C_SPR));
            strcpy(line, line2);
        }
        if (STATE(d) != CON_PLAYING ||
            (STATE(d) == CON_PLAYING && d->character && CAN_SEE(ch, d->character))) {
            send_to_char(line, ch);
            num_can_see++;
        }
    }

    sprintf(line, "\r\n%d видимых соединений.\r\n", num_can_see);
    send_to_char(line, ch);

    send_to_charf(ch, "Входящий трафик : %8ld Кбайт (%ld байт).\r\n", InBytes / 1024, InBytes);
    send_to_charf(ch, "Исходящий трафик: %8ld Кбайт (%ld байт).\r\n", OutBytes / 1024, OutBytes);

}

/* Generic page_string function for displaying text */
ACMD(do_gen_ps)
{

    DLString key;

    switch (subcmd) {
        case SCMD_CREDITS:
            key = "credits";
            break;
        case SCMD_NEWS:
            key = "news";
            break;
        case SCMD_INFO:
            key = "info";
            break;
        case SCMD_IMMLIST:
            key = "immlist";
            break;
        case SCMD_HANDBOOK:
            key = "handbook";
            break;
        case SCMD_POLICIES:
            key = "policies";
            break;
        case SCMD_MOTD:
            key = "motd";
            break;
        case SCMD_IMOTD:
            key = "imotd";
            break;
        case SCMD_CLEAR:
            send_to_char("\033[H\033[J", ch);
            return;
        case SCMD_VERSION:
            send_to_char("МПМ Грани Мира, версия 0.99.30-03-06\r\n", ch);
            return;
        case SCMD_WHOAMI:
            send_to_charf(ch, "%s\r\n", GET_NAME(ch));
            return;
        default:
            log("SYSERR: Unhandled case in do_gen_ps. (%d)", subcmd);
            return;
    }

    const DLString & text = mud->getTextFileLoader()->get(key);

    if (!text.empty())
        page_string(ch->desc, text.c_str(), 1);
}


void perform_mortal_where(struct char_data *ch, char *arg)
{
    register struct char_data *i;
    register struct descriptor_data *d;
    char buf[MAX_STRING_LENGTH];

    send_to_char("Кто много знает, тот плохо спит.\r\n", ch);
    return;

    if (!*arg) {
        send_to_char("Игроки, находящиеся в зоне\r\n--------------------\r\n", ch);
        for (d = descriptor_list; d; d = d->next) {
            if (STATE(d) != CON_PLAYING || d->character == ch)
                continue;
            if ((i = (d->original ? d->original : d->character)) == NULL)
                continue;
            if (i->in_room == NOWHERE || !CAN_SEE(ch, i))
                continue;
            if (world[ch->in_room].zone != world[i->in_room].zone)
                continue;
            sprintf(buf, "%-20s - %s\r\n", GET_NAME(i), world[i->in_room].name);
            send_to_char(buf, ch);
        }
    } else {                    /* print only FIRST char, not all. */
        for (i = character_list; i; i = i->next) {
            if (i->in_room == NOWHERE || i == ch)
                continue;
            if (!CAN_SEE(ch, i) || world[i->in_room].zone != world[ch->in_room].zone)
                continue;
            if (!isname(arg, i->player.name))
                continue;
            sprintf(buf, "%-25s - %s\r\n", GET_NAME(i), world[i->in_room].name);
            send_to_char(buf, ch);
            return;
        }
        send_to_char("Никого похожего с этим именем нет.\r\n", ch);
    }
}


void print_object_location(int num, struct obj_data *obj, struct char_data *ch, int recur)
{
    char buf[MAX_STRING_LENGTH];

    if (num > 0)
        sprintf(buf, "O%3d. %-25s - ", num, obj->short_description);
    else
        sprintf(buf, "%33s", " - ");

    if (obj->in_room > NOWHERE) {
        sprintf(buf + strlen(buf), "[%6d] %s\r\n",
                GET_ROOM_VNUM(IN_ROOM(obj)), world[obj->in_room].name);
        send_to_char(buf, ch);
    } else if (obj->carried_by) {
        sprintf(buf + strlen(buf), "затарено %s\r\n", PERS(obj->carried_by, ch, 4));
        send_to_char(buf, ch);
    } else if (obj->worn_by) {
        sprintf(buf + strlen(buf), "одет на %s\r\n", PERS(obj->worn_by, ch, 1));
        send_to_char(buf, ch);
    } else if (obj->in_obj) {
        sprintf(buf + strlen(buf), "лежит в %s%s ",
                obj->in_obj->short_description, (recur ? ", который находится " : " "));
        send_to_char(buf, ch);
        if (recur)
            print_object_location(0, obj->in_obj, ch, recur);
    } else {
        sprintf(buf + strlen(buf), "находится где-то там, далеко-далеко.\r\n");
        send_to_char(buf, ch);
    }
}



void perform_immort_where(struct char_data *ch, char *arg)
{
    register struct char_data *i;
    register struct obj_data *k;
    struct descriptor_data *d;
    int num = 0, found = 0;
    char buf[MAX_EXTEND_LENGTH];

    if (!*arg) {
        send_to_char("ИГРОКИ\r\n------\r\n", ch);
        for (d = descriptor_list; d; d = d->next)
            if (STATE(d) == CON_PLAYING) {
                i = (d->original ? d->original : d->character);
                if (i && CAN_SEE(ch, i) && (i->in_room != NOWHERE)) {
                    if (d->original)
                        sprintf(buf, "%-20s - [%6d] %s (in %s)\r\n",
                                GET_NAME(i), GET_ROOM_VNUM(IN_ROOM(d->character)),
                                world[d->character->in_room].name, GET_NAME(d->character));
                    else
                        sprintf(buf, "%-20s - [%6d] %s\r\n", GET_NAME(i),
                                GET_ROOM_VNUM(IN_ROOM(i)), world[i->in_room].name);
                    send_to_char(buf, ch);
                }
            }
    } else {
        for (i = character_list; i; i = i->next) {
            if (CAN_SEE(ch, i) && i->in_room != NOWHERE && isname(arg, i->player.name)) {
                found = 1;
                sprintf(buf, "M%3d. %-25s - [%6d] %s\r\n", ++num, GET_NAME(i),
                        GET_ROOM_VNUM(IN_ROOM(i)), world[IN_ROOM(i)].name);
                send_to_char(buf, ch);
            }
        }
        for (num = 0, k = object_list; k; k = k->next)
            if (CAN_SEE_OBJ(ch, k) && isname(arg, k->name)) {
                found = 1;
                print_object_location(++num, k, ch, TRUE);
            }
        if (!found)
            send_to_char("Нет ничего похожего.\r\n", ch);
    }
}



ACMD(do_where)
{
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (IS_GOD(ch))
        perform_immort_where(ch, arg);
    else
        perform_mortal_where(ch, arg);
}



ACMD(do_levels)
{
    int i;
    int la, lb, lc;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
        send_to_char("Боги уже придумали Ваш уровень.\r\n", ch);
        return;
    }
    *buf = '\0';

    for (i = 1; i < LVL_IMMORT; i++) {
        la = level_exp(ch, i);
        lb = level_exp(ch, i + 1) - 1;
        lc = (level_exp(ch, i + 1) - level_exp(ch, i)) / 100;

        if (IS_GOD(ch))
            sprintf(buf + strlen(buf), "%2d: %8d-%-8d | %-8d\r\n", i, la, lb, lc);
        else
            sprintf(buf + strlen(buf), "%s%2d: %8d-%-8d%s\r\n",
                    GET_LEVEL(ch) == i ? CCIBLU(ch, C_SPR) : "", i, la, lb, CCNRM(ch, C_SPR));

    }
    sprintf(buf + strlen(buf), "%2d: %8d (сверхвозможности)\r\n",
            LVL_IMMORT, level_exp(ch, LVL_IMMORT));
    page_string(ch->desc, buf, 1);
}



ACMD(do_consider)
{
    struct char_data *victim;
    int diff;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, buf);

    if (IS_DARK(ch->in_room)) {
        send_to_charf(ch, "В темноте сложно оценить возможности соперника.\r\n");
        return;
    }

    if (!(victim = get_char_vis(ch, buf, FIND_CHAR_ROOM))) {
        send_to_char("Кого Вы хотите оценить ?\r\n", ch);
        return;
    }
    if (victim == ch) {
        send_to_char("Сам себя? Легко!\r\n", ch);
        return;
    }
    if (!IS_NPC(victim)) {
        send_to_char("Нападаете на игроков на свой страх и риск.\r\n", ch);
        return;
    }
    diff = (GET_LEVEL(victim) - GET_LEVEL(ch));

    if (diff <= -10)
        send_to_char("С первого удара.\r\n", ch);
    else if (diff <= -5)
        send_to_char("Проблем не будет.\r\n", ch);
    else if (diff <= -2)
        send_to_char("Легко.\r\n", ch);
    else if (diff <= -1)
        send_to_char("Сравнительно легко.\r\n", ch);
    else if (diff == 0)
        send_to_char("Равный поединок.\r\n", ch);
    else if (diff <= 1)
        send_to_char("У Вас есть шанс.\r\n", ch);
    else if (diff <= 2)
        send_to_char("Вам потребуется везение.\r\n", ch);
    else if (diff <= 3)
        send_to_char("Грозный соперник.\r\n", ch);
    else if (diff <= 5)
        send_to_char("Вы берете на себя слишком много.\r\n", ch);
    else if (diff <= 10)
        send_to_char("Должно быть, Вам нравится умирать.\r\n", ch);
    else if (diff <= 100)
        send_to_char("Вы умрете быстро и тихо.\r\n", ch);

}




const char *ctypes[] = { "выключен", "простой", "обычный", "полный", "\n"
};

int get_widthdisp(struct char_data *ch)
{
    return ch->sw;
}

int get_highdisp(struct char_data *ch)
{
    return ch->sh;
}

ACMD(do_color)
{
    int tp;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    one_argument(argument, arg);

    if (!*arg) {
        sprintf(buf, "%s %sцветовой%s режим.\r\n",
                ctypes[COLOR_LEV(ch)], CCIRED(ch, C_SPR), CCNRM(ch, C_OFF));
        send_to_char(CAP(buf), ch);
        return;
    }
    if (((tp = search_block(arg, ctypes, FALSE)) == -1)) {
        send_to_char("Устанавливает цветовой режим.\r\n", ch);
        send_to_char("Использование: режим цвет <выкл|простой|полный>\r\n", ch);
        return;
    }
    REMOVE_BIT(PRF_FLAGS(ch, PRF_COLOR_1), PRF_COLOR_1);
    REMOVE_BIT(PRF_FLAGS(ch, PRF_COLOR_2), PRF_COLOR_2);

    SET_BIT(PRF_FLAGS(ch, PRF_COLOR_1), (PRF_COLOR_1 * (tp & 1)));
    SET_BIT(PRF_FLAGS(ch, PRF_COLOR_1), (PRF_COLOR_2 * (tp & 2) >> 1));

    sprintf(buf, "%s %sцветовой%s режим.\r\n", ctypes[tp], CCIGRN(ch, C_SPR), CCNRM(ch, C_OFF));
    send_to_char(CAP(buf), ch);
}


ACMD(do_toggle)
{
    /*  if (IS_NPC(ch))
       return; */
    char buf2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (GET_WIMP_LEV(ch) == 0)
        strcpy(buf2, "нет");
    else
        sprintf(buf2, "%-3d", GET_WIMP_LEV(ch));

    if (GET_LEVEL(ch) >= LVL_IMMORT) {
        sprintf(buf,
                "Не выследить: %-3s\r\n"
                "Вечный свет  : %-3s\r\n"
                "Флаги комнат: %-3s\r\n",
                ONOFF(PRF_FLAGGED(ch, PRF_NOHASSLE)),
                ONOFF(PRF_FLAGGED(ch, PRF_HOLYLIGHT)), ONOFF(PRF_FLAGGED(ch, PRF_ROOMFLAGS))
            );
        send_to_char(buf, ch);
    }

    sprintf(buf,
            "КОМАНДА            : ЗНАЧЕНИЕ\r\n"
            "-------------------:------------------------------------------\r\n"
            "реж жизнь          : Уровень жизни в строке статуса       %-3s\r\n"
            "реж бодрость       : Уровень бодрости в строке статуса    %-3s\r\n"
            "реж мана           : Текущее кол-во магической энергии    %-3s\r\n"
            "реж очки           : Опыт в строке статуса                %-3s\r\n"
            "реж выходы         : Показывать выходы в строке статуса   %-3s\r\n"
            "реж разделитель    : Резделитель в названиях заклинаний   %-3s\r\n"
            "реж указатель      : Указатель жизни в строке боя         %-3s\r\n"
            "реж промптбоя      : Псевдографический указатель жизни    %-3s\r\n"
            "реж постой         : Оплачивать постой через банк         %-3s\r\n"
            "реж занят          : Принимать обращения других игроков   %-3s\r\n"
            "реж орать          : Канал \"орать\"                        %-3s\r\n"
            "реж болтать        : Канал \"болтать\"                      %-3s\r\n"
            "реж поздравления   : Канал \"поздравления\"                 %-3s\r\n"
            "реж цензура        : Фильтрация нецензурных слов          %-3s\r\n"
            "реж краткий        : Краткий режим                        %-3s\r\n"
            "реж сжатый         : Сжатый режим                         %-3s\r\n"
            "реж повтор         : Показывать свои сообщения игрокам    %-3s\r\n"
            "реж карта          : Режим автокартографии                %-3s\r\n"
            "реж цвет           : Цветовой режим                       %s\r\n"
            "реж осторожность   : Уровень осторожности персонажа       %-3s\r\n"
            "реж форматирование : Автоматическое форматирование        %-3s\r\n"
            "реж ширина         : Текущая ширина экрана                %-3d\r\n"
            "реж высота         : Текущая высота экрана                %-3d\r\n"
            "реж советы         : Показывать советы при старте         %-3s\r\n"
            "реж мобы           : Группировать одинаковых мобов        %-3s\r\n"
            "реж объекты        : Группировать одинаковые предметы     %-3s\r\n"
            "реж системабоя     : Показывать собственное состояние     %-3s\r\n"
            "реж следование     : Персонажи могут следовать за Вами    %-3s\r\n"
            "реж братьпредметы  : Персонажи могут давать Вам предметы  %-3s\r\n"
            "реж сменацвета     : Смена цветовой гаммы мада            %-3s\r\n"
            "реж осмотр         : Игроки могут рассматривать вашу экип.%-3s\r\n"
            "реж русвыход       : Отображение автовыходов в рус. языке %-3s\r\n"
            "реж сообщения      : Показ только собст. боевых сообщений %-3s\r\n"
			"реж маппер         : Показ уникального номера комнаты     %-3s\r\n"
            "реж миникарта      : Показ миникарты                      %-3s\r\n", 
            ONOFF(PRF_FLAGGED(ch, PRF_DISPHP)),
            ONOFF(PRF_FLAGGED(ch, PRF_DISPMOVE)),
            ONOFF(PRF_FLAGGED(ch, PRF_DISPMANA)),
            ONOFF(PRF_FLAGGED(ch, PRF_DISPEXP)),
            ONOFF(PRF_FLAGGED(ch, PRF_DISPEXITS)),
            ch->divd,
            ch->divr,
            ONOFF(!PRF_FLAGGED(ch, PRF_PROMPT)),
            ONOFF(!PRF_FLAGGED(ch, PRF_BANK_RENT)),
            ONOFF(!PRF_FLAGGED(ch, PRF_NOTELL)),
            ONOFF(!PRF_FLAGGED(ch, PRF_NOHOLLER)),
            ONOFF(!PRF_FLAGGED(ch, PRF_NOGOSS)),
            ONOFF(!PRF_FLAGGED(ch, PRF_NOGRATZ)),
            ONOFF(!PRF_FLAGGED(ch, PRF_CURSES)),
            ONOFF(PRF_FLAGGED(ch, PRF_BRIEF)),
            ONOFF(PRF_FLAGGED(ch, PRF_COMPACT)),
            YESNO(!PRF_FLAGGED(ch, PRF_NOREPEAT)),
            ONOFF(PRF_FLAGGED(ch, PRF_AUTOEXIT)),
            ctypes[COLOR_LEV(ch)],
            buf2,
            ONOFF(PRF_FLAGGED(ch, PRF_AUTOFRM)),
            get_widthdisp(ch),
            get_highdisp(ch),
            ONOFF(PRF_FLAGGED(ch, PRF_TIPS)),
            ONOFF(PRF_FLAGGED(ch, PRF_MOBILES)),
            ONOFF(PRF_FLAGGED(ch, PRF_OBJECTS)),
            ONOFF(PRF_FLAGGED(ch, PRF_DISPBOI)),
            YESNO(!PRF_FLAGGED(ch, PRF_NOFOLLOW)),
            YESNO(!PRF_FLAGGED(ch, PRF_NOGIVE)),
            YESNO(PRF_FLAGGED(ch, PRF_THEME)),
            YESNO(!PRF_FLAGGED(ch, PRF_EXAMINE)),
            YESNO(PRF_FLAGGED(ch, PRF_EXITRUS)), ONOFF(PRF_FLAGGED(ch, PRF_SELFMESS)),
			YESNO(PRF_FLAGGED(ch, PRF_MAPPER)),
            YESNO(PRF_FLAGGED(ch, PRF_MINIMAP))
        );

    page_string(ch->desc, buf, TRUE);
}


struct sort_struct {
    int sort_pos;
    byte is_social;
} *cmd_sort_info = NULL;

int num_of_cmds;


void commands_init(void)
{
    int a, b, tmp;

    log("Сортировка команд.");

    num_of_cmds = 0;

    /*
     * first, count commands (num_of_commands is actually one greater than the
     * number of commands; it inclues the '\n'.
     */
    while (*cmd_info[num_of_cmds].command != '\n')
        num_of_cmds++;

    /* create data array */
    CREATE(cmd_sort_info, struct sort_struct, num_of_cmds);

    /* initialize it */
    for (a = 1; a < num_of_cmds; a++) {
        cmd_sort_info[a].sort_pos = a;
        cmd_sort_info[a].is_social = FALSE;
    }

    /* Sort.  'a' starts at 1, not 0, to remove 'RESERVED' */
    for (a = 1; a < num_of_cmds - 1; a++)
        for (b = a + 1; b < num_of_cmds; b++)
            if (strcmp(cmd_info[cmd_sort_info[a].sort_pos].command,
                       cmd_info[cmd_sort_info[b].sort_pos].command) > 0) {
                tmp = cmd_sort_info[a].sort_pos;
                cmd_sort_info[a].sort_pos = cmd_sort_info[b].sort_pos;
                cmd_sort_info[b].sort_pos = tmp;
            }
}

void commands_destroy()
{
	FREEPTR(cmd_sort_info);
}



ACMD(do_commands)
{
    int no, i, cmd_num, num_of;
    int wizhelp = 0, socials = 0;
    struct char_data *vict;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (*arg) {
        if (!(vict = get_char_vis(ch, arg, FIND_CHAR_WORLD)) || IS_NPC(vict)) {
            send_to_char("Кто это?\r\n", ch);
            return;
        }
        if (GET_LEVEL(ch) < GET_LEVEL(vict) && !GET_COMMSTATE(ch)) {
            send_to_char("Вы не можете узнать команды для персонажа выше Вас уровнем.\r\n", ch);
            return;
        }
    } else
        vict = ch;

    if (subcmd == SCMD_SOCIALS)
        socials = 1;
    else if (subcmd == SCMD_WIZHELP)
        wizhelp = 1;

    sprintf(buf, "Следующие %s%s доступны %s:\r\n",
            wizhelp ? "привелегированные " : "",
            socials ? "социалы" : "команды", vict == ch ? "Вам" : GET_PAD(vict, 2));

    if (socials)
        num_of = top_of_socialk + 1;
    else
        num_of = num_of_cmds - 1;

    /* cmd_num starts at 1, not 0, to remove 'RESERVED' */
    for (no = 1, cmd_num = socials ? 0 : 1; cmd_num < num_of; cmd_num++)
        if (socials) {
            sprintf(buf + strlen(buf), "%-19s", soc_keys_list[cmd_num].keyword);
            if (!(no % 4))
                strcat(buf, "\r\n");
            no++;
        } else {
            i = cmd_sort_info[cmd_num].sort_pos;
            if (cmd_info[i].minimum_level >= 0 &&
                (GET_LEVEL(vict) >= cmd_info[i].minimum_level || GET_COMMSTATE(vict)) &&
                (cmd_info[i].minimum_level >= LVL_IMMORT) == wizhelp &&
                (wizhelp || socials == cmd_sort_info[i].is_social)) {
                if (subcmd == SCMD_ALIAS && !*cmd_info[i].command_alias)
                    continue;
                if (subcmd != SCMD_ALIAS && !*cmd_info[i].command)
                    continue;

                sprintf(buf + strlen(buf), "%-15s",
                        subcmd == SCMD_ALIAS ? cmd_info[i].command_alias : cmd_info[i].command);

                if (!(no % 5))
                    strcat(buf, "\r\n");
                no++;
            }
        }

    strcat(buf, "\r\n");

    page_string(ch->desc, buf, 1);
}


ACMD(do_affects)
{
    struct affected_type *aff;
    char buf[MAX_STRING_LENGTH];

    if (AFF_FLAGGED(ch, AFF_BLIND))
        send_to_charf(ch, "Вы слепы!\r\n");
    if (AFF_FLAGGED(ch, AFF_INVISIBLE))
        send_to_charf(ch, "Ваше тело прозрачно.\r\n");
    if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN))
        send_to_charf(ch, "Вы чувствуете добро и зло.\r\n");
    if (AFF_FLAGGED(ch, AFF_DETECT_INVIS))
        send_to_charf(ch, "Вы способны видеть невидимое.\r\n");
    if (AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
        send_to_charf(ch, "Вы чувствуете магию.\r\n");
    if (AFF_FLAGGED(ch, AFF_SENSE_LIFE))
        send_to_charf(ch, "Вы чувствуете жизнь.\r\n");
    if (AFF_FLAGGED(ch, AFF_WATERWALK))
        send_to_charf(ch, "Вы способны ходить по воде.\r\n");
    if (AFF_FLAGGED(ch, AFF_GROUP))
        send_to_charf(ch, "Вы состоите в группе.\r\n");
    if (AFF_FLAGGED(ch, AFF_CURSE))
        send_to_charf(ch, "Вы чувствуете себя скверно.\r\n");
    if (AFF_FLAGGED(ch, AFF_INFRAVISION))
        send_to_charf(ch, "Вы способны чувствовать тепло.\r\n");
    if (AFF_FLAGGED(ch, AFF_DARKVISION))
        send_to_charf(ch, "Ваши глаза способны видеть в темноте.\r\n");
    if (AFF_FLAGGED(ch, AFF_POISON))
        send_to_charf(ch, "Вы чувствуете тошноту и головокружение.\r\n");
    if (AFF_FLAGGED(ch, AFF_SLEEP))
        send_to_charf(ch, "Вы не способны проснуться.\r\n");
    if (AFF_FLAGGED(ch, AFF_NOTRACK))
        send_to_charf(ch, "Вы не оставляете следов.\r\n");
    if (AFF_FLAGGED(ch, AFF_TETHERED))  // только для ездовых
        send_to_charf(ch, "Вас кто-то привязал.\r\n");
    if (AFF_FLAGGED(ch, AFF_SNEAK) && IS_GOD(ch))       //для внутренего использования
        send_to_charf(ch, "Вы крадетесь.\r\n");
    if (AFF_FLAGGED(ch, AFF_HIDE) && IS_GOD(ch))
        send_to_charf(ch, "Вы спрятались.\r\n");
    if (AFF_FLAGGED(ch, AFF_COURAGE))
        send_to_charf(ch, "Вы в бешенстве.\r\n");
    if (AFF_FLAGGED(ch, AFF_CHARM))
        send_to_charf(ch, "Вы кому-то подчиняетесь.\r\n");
    if (AFF_FLAGGED(ch, AFF_HOLD))
        send_to_charf(ch, "Ваша воля парализована.\r\n");
    if (AFF_FLAGGED(ch, AFF_FLY))
        send_to_charf(ch, "Вы способны летать.\r\n");
    else if (AFF_FLAGGED(ch, AFF_LEVIT))
        send_to_charf(ch, "Вы способны левитировать.\r\n");
    if (AFF_FLAGGED(ch, AFF_SIELENCE))
        send_to_charf(ch, "Ваш язык не подчиняется Вам.\r\n");
    if (AFF_FLAGGED(ch, AFF_AWARNESS))
        send_to_charf(ch, "Вы в напряженном состоянии.\r\n");
//  if (AFF_FLAGGED(ch,AFF_HORSE))
//    send_to_charf(ch, "Вас запрягли.\r\n");
    if (AFF_FLAGGED(ch, AFF_NOFLEE))
        send_to_charf(ch, "Вам не сбежать.\r\n");
    /*  if (AFF_FLAGGED(ch,AFF_SINGLELIGHT)) *///внутрений индефикатор
    /*    send_to_charf(ch, "свет.\r\n"); */
    if (AFF_FLAGGED(ch, AFF_HOLYLIGHT))
        send_to_charf(ch, "Ваше тело светится священным светом.\r\n");
    if (AFF_FLAGGED(ch, AFF_HOLYDARK))
        send_to_charf(ch, "Ваше тело окружено темным облаком поглащающим свет.\r\n");
    if (AFF_FLAGGED(ch, AFF_DETECT_POISON))
        send_to_charf(ch, "Вы способны определить яд.\r\n");
    if (AFF_FLAGGED(ch, AFF_DRUNKED))
        send_to_charf(ch, "Вы пьяны.\r\n");
    if (AFF_FLAGGED(ch, AFF_HAEMORRAGIA))
        send_to_charf(ch, "Ваше тело кровоточит.\r\n");
    if (AFF_FLAGGED(ch, AFF_BANDAGE))
        send_to_charf(ch, "Ваши раны перевязаны бинтами.\r\n");
    if (AFF_FLAGGED(ch, AFF_CAMOUFLAGE) && IS_GOD(ch))
        send_to_charf(ch, "Вы замаскировались.\r\n");
    if (AFF_FLAGGED(ch, AFF_WATERBREATH))
        send_to_charf(ch, "Вы способны дышать под водой.\r\n");
    if (AFF_FLAGGED(ch, AFF_SLOW))
        send_to_charf(ch, "Ваши действия замедлены.\r\n");
    if (AFF_FLAGGED(ch, AFF_FASTER))
        send_to_charf(ch, "Вы чувствуете необычайную легкость в ногах.\r\n");
    /*  if (AFF_FLAGGED(ch,AFF_HOLYAURA))
       send_to_charf(ch, "Силы добра поддерживают Вас.\r\n"); */
    if (AFF_FLAGGED(ch, AFF_UNHOLYAURA))
        send_to_charf(ch, "Силы зла окружают Вас.\r\n");
    if (AFF_FLAGGED(ch, AFF_DETECT_MAGIC) &&
        affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_DESECRATE) && IS_EVILS(ch))
        send_to_charf(ch, "Здесь Вы чувствуете приятную поддержку.\r\n");

    if (ch->set_number != -1 && ch->set_variante != -1) {
        std::vector < struct set_variante_data >vc = *set_table[ch->set_number].variante;
        struct set_variante_data vrnt = vc[ch->set_variante];

        if (vrnt.score)
            send_to_charf(ch, "%s\r\n", vrnt.score);
    }



    if (PLR_FLAGGED(ch, PLR_MUTE) && MUTE_DURATION(ch) != 0 && MUTE_DURATION(ch) > time(NULL)
        ) {                     //int hrs  = (MUTE_DURATION(ch) - time(NULL)) / 3600;
        //int mins = ((MUTE_DURATION(ch) - time(NULL)) % 3600 + 59) / 60;
        sprintf(buf, "Вы заболели и охрипли.\r\n");
        send_to_char(buf, ch);
    }
    if (PLR_FLAGGED(ch, PLR_DUMB) && DUMB_DURATION(ch) != 0 && DUMB_DURATION(ch) > time(NULL)
        ) {                     //int hrs  = (DUMB_DURATION(ch) - time(NULL)) / 3600;
        //int mins = ((DUMB_DURATION(ch) - time(NULL)) % 3600 + 59) / 60;
        sprintf(buf, "У Вас исчез рот.\r\n");
        send_to_char(buf, ch);
    }
    if (!IS_NPC(ch) && GET_GOD_FLAG(ch, GF_GODSCURSE) && GODS_DURATION(ch)) {   //int hrs  = (GODS_DURATION(ch) - time(NULL)) / 3600;
        //int mins = ((GODS_DURATION(ch) - time(NULL)) % 3600 + 59) / 60;
        sprintf(buf, "Вы прокляты Богами.\r\n");

        strcat(buf, "\r\n");
        send_to_char(buf, ch);
    }
    if (ch->affected)
        for (aff = ch->affected; aff; aff = aff->next) {
            //send_to_charf(ch,"aff->type %d\r\n",aff->type);
            if (aff->main && Spl.GetItem(aff->type)->GetItem(SPL_SCORE)->GetString())
                act(Spl.GetItem(aff->type)->GetItem(SPL_SCORE)->GetString(), "!М", ch);
        }
}

void show_locates(struct char_data *ch)
{
    int dir;

    if (GET_OBJ_LOCATE(ch)) {
        CPathRetDir dirs;

        dir = TracePath(IN_ROOM(ch), IN_ROOM(GET_OBJ_LOCATE(ch)), 0, &dirs, 0);
        if (dir == -1 || dir == 0) {
            send_to_charf(ch, "Путеводная нить плавно рассеялась в воздухе.\r\n");
            GET_OBJ_LOCATE(ch) = 0;
        } else if (dir > 0) {
            send_to_charf(ch, "Призрачная нить тянется %s.\r\n", DirsTo[dirs.Direction]);
            SET_BIT(ch->locate_dirs, 1 << dirs.Direction);
        } else
            send_to_charf(ch, "Призрачная нить запуталась в клубок.\r\n");
    } else if (GET_CHAR_LOCATE(ch)) {
        CPathRetDir dirs;

        dir = TracePath(IN_ROOM(ch), IN_ROOM(GET_CHAR_LOCATE(ch)), 0, &dirs, 0);
        if (dir == -1 || dir == 0) {
            send_to_charf(ch, "Путеводная нить плавно рассеялась в воздухе.\r\n");
            GET_CHAR_LOCATE(ch) = 0;
        } else if (dir > 0) {
            send_to_charf(ch, "Призрачная нить тянется %s.\r\n", DirsTo[dirs.Direction]);
            SET_BIT(ch->locate_dirs, 1 << dirs.Direction);
        } else
            send_to_charf(ch, "Призрачная нить запуталась в клубок.\r\n");
    }

}

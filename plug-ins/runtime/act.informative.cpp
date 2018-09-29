
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

const char *Dirs[NUM_OF_DIRS + 1] = { "�����",
    "������",
    "��",
    "�����",
    "����",
    "���",
    "\n"
};

const char *ObjState[8][2] = { {"�����������", "�����������"},
{"����� �����", "� ����� ������ ���������"},
{"�����", "� ������ ���������"},
{"�������", "� �������� ���������"},
{"������", "� ������� ���������"},
{"������", "� ������� ���������"},
{"����� ������", "� ����� ������� ���������"},
{"�����������", "� ������������ ���������"}
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
    "������",
    "������",
    "����",
    "�������",
    "������",
    "�������",
    "�����",
    "�����",
    "����",
    "��������",
    "����������",
    "�������������"
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
                    sprintf(out_str, "�� ����������� � ��������� ����� ������!\r\n");
                    break;
            }

            if (skill)
                sprintf(out_str, "%s ����������� � ������ '%s'.\r\n",
                        CAP(GET_OBJ_PNAME(obj, 0)), weapon_class[skill - 1]);
        default:
            if (show_wear) {
                if (CAN_WEAR(obj, ITEM_WEAR_FINGER))
                    sprintf(out_str + strlen(out_str), "����� ������ �� �����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_NECK))
                    sprintf(out_str + strlen(out_str), "����� ������ �� ���.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_BODY))
                    sprintf(out_str + strlen(out_str), "����� ������ �� ��������.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_FACE))
                    sprintf(out_str + strlen(out_str), "����� ������ �� ����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_HEAD))
                    sprintf(out_str + strlen(out_str), "����� ������ �� ������.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_LEGS))
                    sprintf(out_str + strlen(out_str), "����� ������ �� ����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_FEET))
                    sprintf(out_str + strlen(out_str), "����� �����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_HANDS))
                    sprintf(out_str + strlen(out_str), "����� ������ �� �����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_ARMS))
                    sprintf(out_str + strlen(out_str), "����� ������ �� ����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))
                    sprintf(out_str + strlen(out_str), "����� ������������ ��� ���.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))
                    sprintf(out_str + strlen(out_str), "����� ������ �� �����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_WAIST))
                    sprintf(out_str + strlen(out_str), "����� ������ �� ����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_WRIST))
                    sprintf(out_str + strlen(out_str), "����� ������ �� ��������.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_WIELD) && !CAN_WEAR(obj, ITEM_WEAR_BOTHS))
                    sprintf(out_str + strlen(out_str), "����� ������� � ������ ����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_HOLD) && !CAN_WEAR(obj, ITEM_WEAR_BOTHS))
                    sprintf(out_str + strlen(out_str), "����� ������� � ����� ����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_BOTHS))
                    sprintf(out_str + strlen(out_str), "����� ����� � ��� ����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_TAIL))
                    sprintf(out_str + strlen(out_str), "����� ������ �� �����.\r\n");
                if (CAN_WEAR(obj, ITEM_WEAR_EARS))
                    sprintf(out_str + strlen(out_str), "����� ������ � ���.\r\n");
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
            send_to_char(" ������ �����.\r\n", ch);
        else
            send_to_char("�� ������ �� ������.\r\n", ch);
    }
}

char *diag_char_to_char(struct char_data *i, int type)
{
    int percent;
    static char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

// ������� �� �����

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
        sprintf(buf2, " � ������������ ���������");
        strcat(buf1, buf2);
    } else
     if (percent >= 90) {
        sprintf(buf2, " ������ ���������%s", GET_CH_SUF_6(i));
        strcat(buf1, buf2);
    } else if (percent >= 75) {
        sprintf(buf2, " ����� �����%s", GET_CH_SUF_6(i));
        strcat(buf1, buf2);
    } else if (percent >= 50) {
        sprintf(buf2, " �����%s", GET_CH_SUF_6(i));
        strcat(buf1, buf2);
    } else if (percent >= 30) {
        sprintf(buf2, " ������ �����%s", GET_CH_SUF_6(i));
        strcat(buf1, buf2);
    } else if (percent >= 15) {
        sprintf(buf2, " ���������� �����%s", GET_CH_SUF_6(i));
        strcat(buf1, buf2);
    } else if (percent >= 0)
        strcat(buf1, " � ������� ���������");
    else
        strcat(buf1, " �������");

    /*  if (AFF_FLAGGED(ch, AFF_DETECT_POISON))
       if (AFF_FLAGGED(i, AFF_POISON))
       {sprintf(buf2," (��������%s)", GET_CH_SUF_6(i));
       strcat(buf, buf2);
       } */


    if (!IS_NPC(i) || (IS_NPC(i) && AFF_FLAGGED(i, AFF_CHARM))) {


        if (GET_REAL_MAX_MOVE(i) > 0)
            percent = (100 * GET_MOVE(i)) / GET_REAL_MAX_MOVE(i);
        else
            percent = -1;       /* How could MAX_HIT be < 1?? */


        if (percent >= 100) {
            sprintf(buf2, " � �������� ���������%s", GET_CH_SUF_7(i));
            strcat(buf1, buf2);
        } else
         if (percent >= 90) {
            sprintf(buf2, " � �������� ������� ������%s", GET_CH_SUF_7(i));
            strcat(buf1, buf2);
        } else if (percent >= 75) {
            sprintf(buf2, " � �������� ������%s", GET_CH_SUF_7(i));
            strcat(buf1, buf2);
        } else if (percent >= 50) {
            sprintf(buf2, " � �������� ��������%s", GET_CH_SUF_8(i));
            strcat(buf1, buf2);
        } else if (percent >= 30) {
            sprintf(buf2, " � �������� ������ ������%s", GET_CH_SUF_7(i));
            strcat(buf1, buf2);
        } else if (percent >= 15) {
            sprintf(buf2, " � �������� ���������%s", GET_CH_SUF_8(i));
            strcat(buf1, buf2);
        } else if (percent >= 5) {
            sprintf(buf2, " � �������� ��������%s", GET_CH_SUF_8(i));
            strcat(buf1, buf2);
        } else {
            sprintf(buf2, " � ������� � ��� �� ���������");
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
        sprintf(h_str, "�����������������");
    else if (hei < 15)
        sprintf(h_str, "����������");
    else if (hei < 30)
        sprintf(h_str, "����������");
    else if (hei < 45)
        sprintf(h_str, "����������");
    else if (hei < 60)
        sprintf(h_str, "��������");
    else if (hei < 80)
        sprintf(h_str, "��������");
    else if (hei < 100)
        sprintf(h_str, "�����������");
    else
        sprintf(h_str, "�������������");

    return h_str;
}

char *show_cha(struct char_data *ch, char c_str[20])
{
    int cha;

    cha = GET_REAL_CHA(ch);

    if (affected_by_spell(ch, SPELL_PLAGUE))
        sprintf(c_str, "�������� ������");
    else if (cha < 6)
        sprintf(c_str, "���������");
    else if (cha < 8)
        sprintf(c_str, "�����������");
    else if (cha < 12)
        sprintf(c_str, "����� �� ��������������");
    else if (cha < 16)
        sprintf(c_str, "�����������");
    else if (cha < 20)
        sprintf(c_str, "���������������");
    else if (cha < 23)
        sprintf(c_str, "��������");
    else
        sprintf(c_str, "������������ �������");

    return c_str;
}

char *show_age(struct char_data *ch, char a_str[20])
{
    int agez = GET_REAL_AGE(ch);

    if (agez < 10)
        sprintf(a_str, "�������");
    else if (agez < 20)
        sprintf(a_str, "����");
    else if (agez < 30)
        sprintf(a_str, "�������");
    else if (agez < 60)
        sprintf(a_str, "��������");
    else if (agez < 95)
        sprintf(a_str, "�����������");
    else
        sprintf(a_str, "�������");

    return a_str;
}

char *show_int(struct char_data *ch, char a_str[20])
{
    int intz = GET_REAL_INT(ch);

    if (AFF_FLAGGED(ch, AFF_BLIND))
        sprintf(a_str, "������");
    else {
        if (intz < 11)
            sprintf(a_str, "������");
        else if (intz < 14)
            sprintf(a_str, "�������");
        else if (intz < 17)
            sprintf(a_str, "����������");
        else if (intz < 21)
            sprintf(a_str, "�����");
        else if (intz < 24)
            sprintf(a_str, "���������");
        else
            sprintf(a_str, "��������������");
    }

    return a_str;
}

char *show_str(struct char_data *ch, char a_str[20])
{
    int str = GET_REAL_STR(ch);


    if (GET_SEX(ch) == SEX_MALE) {
        if (str < 8)
            sprintf(a_str, "�����");
        else if (str < 14)
            sprintf(a_str, "�������");
        else if (str < 18)
            sprintf(a_str, "��������");
        else if (str < 23)
            sprintf(a_str, "�����������");
        else if (str < 25)
            sprintf(a_str, "�������");
        else
            sprintf(a_str, "����������");
    } else if (GET_SEX(ch) == SEX_FEMALE) {
        if (str < 8)
            sprintf(a_str, "�����");
        else if (str < 14)
            sprintf(a_str, "�������");
        else if (str < 18)
            sprintf(a_str, "��������");
        else if (str < 23)
            sprintf(a_str, "�����������");
        else if (str < 25)
            sprintf(a_str, "�������");
        else
            sprintf(a_str, "����������");
    } else if (GET_SEX(ch) == SEX_NEUTRAL) {
        if (str < 8)
            sprintf(a_str, "�����");
        else if (str < 14)
            sprintf(a_str, "�������");
        else if (str < 18)
            sprintf(a_str, "��������");
        else if (str < 23)
            sprintf(a_str, "�����������");
        else if (str < 25)
            sprintf(a_str, "�������");
        else
            sprintf(a_str, "����������");
    } else {
        if (str < 8)
            sprintf(a_str, "�����");
        else if (str < 14)
            sprintf(a_str, "�������");
        else if (str < 18)
            sprintf(a_str, "��������");
        else if (str < 23)
            sprintf(a_str, "�����������");
        else if (str < 25)
            sprintf(a_str, "�������");
        else
            sprintf(a_str, "����������");
    }

    return a_str;
}

char *show_con(struct char_data *ch, char a_str[20])
{
    int str = GET_REAL_CON(ch);

    if (GET_SEX(ch) == SEX_MALE) {
        if (str < 8)
            sprintf(a_str, "���������");
        else if (str < 12)
            sprintf(a_str, "�����");
        else if (str < 16)
            sprintf(a_str, "��������");
        else if (str < 19)
            sprintf(a_str, "� ���� ���������");
        else if (str < 23)
            sprintf(a_str, "�������");
        else if (str < 26)
            sprintf(a_str, "�������");
        else
            sprintf(a_str, "������");
    } else if (GET_SEX(ch) == SEX_FEMALE) {
        if (str < 8)
            sprintf(a_str, "���������");
        else if (str < 12)
            sprintf(a_str, "�����");
        else if (str < 16)
            sprintf(a_str, "��������");
        else if (str < 19)
            sprintf(a_str, "� ���� ���������");
        else if (str < 23)
            sprintf(a_str, "�������");
        else if (str < 25)
            sprintf(a_str, "�������");
        else
            sprintf(a_str, "������");
    } else if (GET_SEX(ch) == SEX_NEUTRAL) {
        if (str < 8)
            sprintf(a_str, "���������");
        else if (str < 12)
            sprintf(a_str, "�����");
        else if (str < 16)
            sprintf(a_str, "��������");
        else if (str < 19)
            sprintf(a_str, "� ���� ���������");
        else if (str < 23)
            sprintf(a_str, "�������");
        else if (str < 25)
            sprintf(a_str, "�������");
        else
            sprintf(a_str, "������");
    } else {
        if (str < 8)
            sprintf(a_str, "���������");
        else if (str < 12)
            sprintf(a_str, "�����");
        else if (str < 16)
            sprintf(a_str, "��������");
        else if (str < 19)
            sprintf(a_str, "� ���� ���������");
        else if (str < 23)
            sprintf(a_str, "�������");
        else if (str < 25)
            sprintf(a_str, "�������");
        else
            sprintf(a_str, "������");
    }

    return a_str;
}

const char *POS_STATE[] = { " ����� ��� �������",
    " ����� ��� ������",
    " �������",
    " ����� ��� ��������",
    " ����",
    " ��������",
    " �����",
    " ���������",
    " �����",
    " ������"
};


const char *list_one_char(struct char_data *i, struct char_data *ch, int skill_mode, int count)
{
    int sector = SECT_CITY;
    int point = 0;
    bool lower = FALSE;
    char *desc;
    static char buf[MAX_STRING_LENGTH];

    const char *positions[] = { "����� ����� ������",
        "����� �����, ��� ������",
        "����� �����, ��� ��������",
        "����� �����, � ��������",
        "���� �����",
        "�������� �����",
        "����� �����",
        "���������",
        "����� �����",
        "������ �����"
    };

    *buf = '\0';

    // ������� � ��������
    if (IS_HORSE(i) && on_horse(i->master)) {
        if (ch == i->master) {
            *buf = '\0';
            if (PRF_FLAGGED(ch, PRF_ROOMFLAGS) && IS_MOB(i))
                sprintf(buf, "[%6d] ", GET_MOB_VNUM(i));
            sprintf(buf + strlen(buf), "%s ����� ��� �� ����� �����.\r\n", CAP(GET_NAME(i)));
            PHRASE(buf);
        }
        return (buf);
    }


    if (IS_SOUL(i)) {
        if (PRF_FLAGGED(ch, PRF_HOLYLIGHT) || IS_SOUL(ch))
            sprintf(buf, "������� %s �������� ����� ��� ����� ������.", GET_PAD(i, 1));

        return (buf);
    } else                      // �� ����� ���������
    if (!CAN_SEE(ch, i)) {
        skill_mode = check_awake(i, ACHECK_AFFECTS | ACHECK_LIGHT | ACHECK_HUMMING |
                                 ACHECK_GLOWING | ACHECK_WEIGHT);

        if (IS_SET(skill_mode, ACHECK_AFFECTS)) {
            REMOVE_BIT(skill_mode, ACHECK_AFFECTS);
            sprintf(buf + strlen(buf), "���������� �����%s", skill_mode ? ", " : " ");
        }
        if (IS_SET(skill_mode, ACHECK_LIGHT) && !IS_AFFECTED(ch, AFF_BLIND)) {
            REMOVE_BIT(skill_mode, ACHECK_LIGHT);
            sprintf(buf + strlen(buf), "����� ����%s", skill_mode ? ", " : " ");
        }
        if (IS_SET(skill_mode, ACHECK_GLOWING) && IS_SET(skill_mode, ACHECK_HUMMING)
            && !IS_AFFECTED(ch, AFF_DEAFNESS) && !IS_AFFECTED(ch, AFF_BLIND)) {
            REMOVE_BIT(skill_mode, ACHECK_GLOWING);
            REMOVE_BIT(skill_mode, ACHECK_HUMMING);
            sprintf(buf + strlen(buf), "��� � ����� ����������%s", skill_mode ? ", " : " ");
        }
        if (IS_SET(skill_mode, ACHECK_GLOWING) && !IS_AFFECTED(ch, AFF_BLIND)) {
            REMOVE_BIT(skill_mode, ACHECK_GLOWING);
            sprintf(buf + strlen(buf), "����� ����������%s", skill_mode ? ", " : " ");
        }
        if (IS_SET(skill_mode, ACHECK_HUMMING) && !IS_AFFECTED(ch, AFF_DEAFNESS)) {
            REMOVE_BIT(skill_mode, ACHECK_HUMMING);
            sprintf(buf + strlen(buf), "��� ���������%s", skill_mode ? ", " : " ");
        }
        if (IS_SET(skill_mode, ACHECK_WEIGHT) && !IS_AFFECTED(ch, AFF_DEAFNESS)) {
            REMOVE_BIT(skill_mode, ACHECK_WEIGHT);
            sprintf(buf + strlen(buf), "�������� �������%s", skill_mode ? ", " : " ");
        }
        strcat(buf, "������ ���-�� �����������.");
        return (buf);
    }

    *buf = '\0';

    if (PRF_FLAGGED(ch, PRF_ROOMFLAGS)) {
        if (IS_MOB(i))
            sprintf(buf, "[%6d] ", GET_MOB_VNUM(i));
        else
            sprintf(buf, "[%6d] ", GET_LEVEL(i));
    }
    // ����������
    if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN)) {
        if (IS_EVILS(i))
            strcat(buf, "(������ ����) ");
        else if (IS_GOODS(i))
            strcat(buf, "(������� ����) ");
    }

    if (!IS_NPC(i) && same_group(ch, i) && (!i->desc || PLR_FLAGGED(i, PLR_DROPLINK))) {
        sprintf(buf + strlen(buf), "���������%s � ����� ", GET_CH_SUF_3(i));
        lower = TRUE;
    }


    if (IS_NPC(i) && !NPC_FLAGGED(i, NPC_MISLEAD) &&
        !i->is_transpt && (!FIGHTING(i)) && i->player.long_descr &&
        //GET_POS(i) == GET_DEFAULT_POS(i) &&
        //IN_ROOM(ch) == IN_ROOM(i) &&
        !AFF_FLAGGED(i, AFF_CHARM) && !IS_MOUNT(i)) {   // ������ �� �������
        const char *mess = '\0';

        point = 1;

        if (on_horse(i) && i->npc()->specials.vnum_horse > 0) {
            if ((desc = find_exdesc("������", i->player.ex_description)) != NULL)
                sprintf(buf + strlen(buf), "%s ", desc);
            else
                sprintf(buf + strlen(buf), "%s ����� ������ �� %s. ",
                        CAP(PERS(i, ch, 0)), PERS(get_horse_on(i), ch, 5));
        } else if (i->trap_object && IN_ROOM(i) == IN_ROOM(i->trap_object))
            sprintf(buf + strlen(buf), "%s %s �����������%s %s.",
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
            if ((desc = find_exdesc("������", i->player.ex_description)) != NULL)
                sprintf(buf + strlen(buf), "%s ", desc);
            else
                sprintf(buf + strlen(buf), "%s ����� ������ �� %s. ",
                        CAP(PERS(i, ch, 0)), PERS(get_horse_on(i), ch, 5));
        } else if (i->trap_object && IN_ROOM(i) == IN_ROOM(i->trap_object))
            sprintf(buf + strlen(buf), "%s %s �����������%s %s.",
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
    } else {                    //��������� ������
        if (AFF_FLAGGED(i, AFF_BLIND)) {
            sprintf(buf + strlen(buf), lower ? "� ���������%s " : "���������%s ", GET_CH_SUF_3(i));
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
            sprintf(buf + strlen(buf), ", ��������%s �������� �������� ������,", GET_CH_SUF_3(i));
        else if (affected_by_spell(i, SPELL_BLADE_BARRIER))
            sprintf(buf + strlen(buf), ", ��������%s �������� �� ������,", GET_CH_SUF_3(i));

        //strcat(buf," ");

        if (!FIGHTING(i)) {
            const char *mess = '\0';

            if ((mess = get_line_event(i)))
                sprintf(buf + strlen(buf), " %s", mess);
            else if (on_horse(i))
                sprintf(buf + strlen(buf), " ����� ������ �� %s", PERS(get_horse_on(i), ch, 5));
            else if (i->is_transpt)
                sprintf(buf + strlen(buf), " ����� � %s%s",
                        GET_OBJ_PNAME(i->is_transpt, 5),
                        (i->is_transpt == ch->is_transpt) ? " ����� � ����" : "");
            else if (IS_NPC(i) && i->npc()->specials.transpt)
                sprintf(buf + strlen(buf), " ��������%s � %s", GET_CH_SUF_6(i),
                        GET_OBJ_PNAME(i->npc()->specials.transpt, 3));
            else if (IS_MOUNT(i) && AFF_FLAGGED(i, AFF_TETHERED))
                sprintf(buf + strlen(buf), " ��������%s �����", GET_CH_SUF_6(i));
            else if (GET_POS(i) > POS_SLEEPING && AFF_FLAGGED(i, AFF_HIDE))
                sprintf(buf + strlen(buf), " �������� �����");
            else if (GET_POS(i) > POS_SLEEPING && AFF_FLAGGED(i, AFF_CAMOUFLAGE))
                sprintf(buf + strlen(buf), " ������������%s �����", GET_CH_SUF_2(i));
            else if ((sector = real_sector(IN_ROOM(i))) == SECT_FLYING)
                strcat(buf, " ������ �����");
            else if (sector == SECT_UNDERWATER)
                strcat(buf, " ������� �����");
            else if (GET_POS(i) == POS_FLYING && AFF_FLAGGED(i, AFF_FLY))
                sprintf(buf + strlen(buf), " ������ ��� %s",
                        (sector == SECT_WATER_SWIM
                         || sector == SECT_WATER_NOSWIM) ? "�����" : "������");
            else if (GET_POS(i) == POS_FLYING && AFF_FLAGGED(i, AFF_LEVIT))
                sprintf(buf + strlen(buf), " ������ ����� ��� %s",
                        (sector == SECT_WATER_SWIM
                         || sector == SECT_WATER_NOSWIM) ? "�����" : "������");
            else if (sector == SECT_WATER_SWIM || sector == SECT_WATER_NOSWIM) {
                if (AFF_FLAGGED(i, AFF_WATERWALK))
                    strcat(buf, " ����� �� ����");
                else
                    strcat(buf, " ������� �����");
            } else
                sprintf(buf + strlen(buf), " %s", positions[(int) GET_POS(i)]);

            if (AFF_FLAGGED(i, AFF_HOLYLIGHT))
                sprintf(buf + strlen(buf), ", ������� �������");

            if (AFF_FLAGGED(i, AFF_INVISIBLE))
                sprintf(buf + strlen(buf), ", ��������� ����������� �����������");
            if (i->trap_object && IN_ROOM(i) == IN_ROOM(i->trap_object))
                sprintf(buf + strlen(buf), ", �����������%s %s", GET_CH_SUF_3(i),
                        GET_OBJ_PNAME(i->trap_object, 4));
        } else {
            if (GET_POS(i) < POS_SITTING)
                sprintf(buf + strlen(buf), " %s", positions[(int) GET_POS(i)]);
            else {
                if (GET_POS(i) == POS_SITTING)
                    strcat(buf, " ����� ����� � ��������� c ");
                else
                    strcat(buf, " ��������� c ");

                if (i->in_room != FIGHTING(i)->in_room) {
                    strcat(buf, "����-�� �����");
                    stop_fighting(i, TRUE);
                } else if (FIGHTING(i) == ch)
                    strcat(buf, "����");
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
            sprintf(buf + strlen(buf), " (��������%s)", GET_CH_SUF_6(i));

//      if (!IS_NPC(i) && same_group(ch,i) && (!i->desc || PLR_FLAGGED(i,PLR_DROPLINK)))
//       sprintf(buf+strlen(buf), " (�������%s �����)", GET_CH_SUF_1(i));

    if (!IS_NPC(i) && PLR_FLAGGED(i, PLR_WRITING))
        strcat(buf, " (�����)");

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
                    //����-�������� �� �����.
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
                    sprintf(buf1 + strlen(buf1), "������ %s ���� �������� � �������.\r\n",
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
                    sprintf(buf1 + strlen(buf1), "������ %s ���� �������� � �������.\r\n",
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
                    strcat(buf1, "������� ������� ������ ���-�� �����������.\r\n");
            }
        }                       //����� NPC
        else if (ch != i && !IS_NPC(i)) {
            if (IS_SOUL(ch)) {
                if (IS_SOUL(i))
                    sprintf(buf2 + strlen(buf2), "%s\r\n", list_one_char(i, ch, 0, 1));
                else
                    sprintf(buf2 + strlen(buf2), "������ %s ���� �������� � �������.\r\n",
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
                sprintf(buf2 + strlen(buf2), "������ %s ���� �������� � �������.\r\n",
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
                strcat(buf2, "������� ������� ������ ���-�� �����������.\r\n");
        }
    }
    send_to_charf(ch, buf3);
    send_to_charf(ch, buf1);
    send_to_charf(ch, buf2);
}

const char *dirs_r[] = {
    "�",
    "�",
    "�",
    "�",
    "�",
    "�",
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
        sprintf(buf2, "%s[ ������: %s]%s\r\n", CCCYN(ch, C_NRM),
                *buf ? buf : "��� ", CCNRM(ch, C_NRM));
    else
        sprintf(buf2, "%s[ Exits: %s]%s\r\n", CCCYN(ch, C_NRM),
                *buf ? buf : "None ", CCNRM(ch, C_NRM));

    return (buf2);
}

ACMD(do_where_im)
{
	char buf[MAX_STRING_LENGTH];
	sprintf(buf, "�� ���������� � ����: %s\r\n", zone_table[world[IN_ROOM(ch)].zone].name);
	send_to_char(buf, ch);
}

ACMD(do_exits)
{
    int door;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if (AFF_FLAGGED(ch, AFF_BLIND)) {
        send_to_char("�� �����!\r\n", ch);
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
                    strcat(buf2, "������� �����\r\n");
                else {
                    strcat(buf2, world[EXIT(ch, door)->to_room].name);
                    strcat(buf2, "\r\n");
                }
            }
            strcat(buf, CAP(buf2));
        }
    send_to_char("������� ������:\r\n", ch);
    if (*buf)
        send_to_char(buf, ch);
    else
        send_to_char(" ����!\r\n", ch);
}


#define MAX_FIRES 6
const char *Fires[MAX_FIRES] = { "����� ��������� ����� ��������",
    "����� ��������� ����� ��������",
    "���-��� ��������� ������",
    "�������� ��������� ������",
    "������ ����� ������",
    "���� ������ ������"
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
        send_to_char("������� �����...\r\n", ch);
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
                strcat(buf_room, "������ �����\r\n");
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
                strcat(buf_room, "������������ ���� ������ ��� ������� ����������.\r\n");
            else if (IS_DARK(IN_ROOM(ch)) &&
                     (!PRF_FLAGGED(ch, PRF_HOLYLIGHT) && !IS_AFFECTED(ch, AFF_DARKVISION)))
                sprintf(buf_room + strlen(buf_room), "������� �����...\r\n");
            else if (IS_CLOUD(IN_ROOM(ch)) && (!PRF_FLAGGED(ch, PRF_HOLYLIGHT)))
                strcat(buf_room,
                       "������������ ��� ������� ����� ������ ��� ���-���� ����������.\r\n");
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
                strcat(buf_room, "������������ ���� ������ ��� ������� ����������.\r\n");
            else if (world[ch->in_room].description)
                strcat(buf_room,
                       strbraker(world[ch->in_room].description, ch->sw,
                                 PRF_FLAGGED(ch, PRF_AUTOFRM)));
            else
                strcat(buf_room,
                       strbraker
                       ("������ ���� ��������� � �������� ����������, ������� ������ ��� �� ����� �������� ���� ��������.",
                        ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)));

        }
    }
    /* autoexits */
    if (PRF_FLAGGED(ch, PRF_AUTOEXIT))
        strcat(buf_room, do_auto_exits(ch));


    //ADD BY SLOWN
    /* ���������� ����� � ������� */
    if (RM_BLOOD(ch->in_room) > 0 && !IS_SOUL(ch)) {
        sprintf(buf, "%s%s%s\r\n",
                CCRED(ch, C_NRM), blood_messages[(int) RM_BLOOD(ch->in_room)], CCNRM(ch, C_NRM));
        strcat(buf_room, buf);
    }

    if (world[IN_ROOM(ch)].fires && !IS_SOUL(ch)) {
        sprintf(buf, "%s� ������ %s.%s\r\n",
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
                sprintf(buf, "������� ����� ����� � ��� ��� ������.\r\n");
                break;
            case SECT_FIELD_RAIN:
            case SECT_FOREST_RAIN:
            case SECT_HILLS_RAIN:
                sprintf(buf, "������ �� ����� � ��� ��� ������.\r\n");
                break;
            case SECT_THICK_ICE:
                sprintf(buf, "� ��� ��� ������ ������� ���.\r\n");
                break;
            case SECT_NORMAL_ICE:
                sprintf(buf, "� ��� ��� ������ ���.\r\n");
                break;
            case SECT_THIN_ICE:
                sprintf(buf, "� ��� ��� ������ ������ ���.\r\n");
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
            sprintf(buf, "%s�� �������� ��������� �������� %s.%s\r\n",
                    CCMAG(ch, C_NRM), DirsTo_2[n], CCNRM(ch, C_NRM));
    strcat(buf_room, buf);


    if (world[IN_ROOM(ch)].portal_time && !IS_SOUL(ch)) {
        sprintf(buf, "%s�������� ����������� ������������ �����.%s\r\n",
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
            send_to_charf(ch, "�������������� ����� ��������� ��� �� ��������� ����.\r\n");
        else
            send_to_charf(ch, "�������������� ����� ������������ ����� ������� ������.\r\n");
    }
    //���������� ������� �������
    show_locates(ch);
    if (affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_THRDEATH))
        send_to_charf(ch, "����� ����� ������ �� ��� ������ ����� ���� ��������� ������.\r\n");

    list_obj_to_char(world[ch->in_room].contents, ch, 0, FALSE);

    if (!PRF_FLAGGED(ch, PRF_THEME))
        send_to_char(CCYEL(ch, C_NRM), ch);
    else
        send_to_char(CCIRED(ch, C_NRM), ch);
    list_char_to_char(world[ch->in_room].people, ch);
    send_to_char(CCNRM(ch, C_NRM), ch);
}

/*
<Zodgrot> 0 - ����� ������� ��� (������ �����)
[00:24] <Zodgrot> ����� ������
[00:24] <Zodgrot> -1- ����� ������� ��� (�� ��� ��� ���� � ���)
[00:24] <Zodgrot> ����� ������� ������������ ���
[00:24] <Zodgrot> -2- ����� ������� ��� (����)
[00:24] <Zodgrot> ����� ������� ���
[00:24] <Zodgrot> -3- ����� ������� ������
[00:24] <Zodgrot> ����� ����� ������� ������
[00:24] <Zodgrot> -4- ����� ������� ������
[00:24] <Zodgrot> ����� ����� ������� ������
[00:24] <Zodgrot> -5- ������� �� �����
*/
//���������� ��������� ch ��������� tch � ���������� �� ��������� light

const char *in_size[9][NUM_SEXES] = {
    {"����������������", "�����������������", "����������������", "����������������"},
    {"���������", "���������", "���������", "���������"},
    {"���������", "���������", "���������", "���������"},
    {"���������", "���������", "���������", "���������"},
    {"�������", "�������", "�������", "�������"},
    {"����������", "����������", "����������", "����������"},
    {"��������", "��������", "��������", "��������"},
    {"�����������", "�����������", "�����������", "�����������"},
    {"������������", "������������", "������������", "������������"},
};

const char *in_go[9] = {
    "����",
    "�����",
    "������",
    "�������",
    "������",
    "�������",
    "������",
    "�������",
    "��������"
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
                act("2� �� ��������2(,�,�,�) ��� ��������� 2�� @1�.", "���", ch, i, object);
                return;
            }

            act("������� � 2�, �� ����������� ��������� 2�� @1�.", "���", ch, i, object);
            act("1+� �������1(,�,�,�) � ��� � ����������� ��������1(,�,�,�) ���@1(,�,�,�) @1�.",
                "���", ch, i, object);
            act("1+� �������1(,�,�,�) � 2+� � ����������� ��������1(,�,�,�) 2�� @1�.", "����", ch,
                i, object);

            if (object->main_description)
                strcpy(buf,
                       strbraker(string_corrector(object->main_description), ch->sw,
                                 PRF_FLAGGED(ch, PRF_AUTOFRM)));
            else
                strcpy(buf, "������ ����������.\r\n");

            if (affected_object_by_spell(object, SPELL_MAGIC_VESTMENT) ||
                affected_object_by_spell(object, SPELL_MAGIC_VESTMENT_P) ||
                affected_object_by_spell(object, SPELL_MAGIC_WEAPON)) {
                if (IS_OBJ_STAT(object, ITEM_GOODAURA))
                    sprintf(buf + strlen(buf), " �������-������� ���� �������� %s.",
                            GET_OBJ_PNAME(object, 3));
                if (IS_OBJ_STAT(object, ITEM_DARKAURA))
                    sprintf(buf + strlen(buf), " �������-������� ���� �������� %s.",
                            GET_OBJ_PNAME(object, 3));
                if (IS_OBJ_STAT(object, ITEM_DEATHAURA))
                    sprintf(buf + strlen(buf), " �����-���������� ���� �������� %s.",
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

        send_to_charf(ch, "� %s ��� '%s'.\r\n", GET_PAD(i, 1), arg);
        return;
    }

    if (ch != i) {
        if (IS_GOD(ch))
            act("��� ����� �����, ����� 1*� ��������1(,�,�,�) �� ���.", "��", ch, i);
        else
            act("1*� ��������1(,�,�,�) �� ���.", "��", ch, i);

        act("1*� ����������� ��������1(,�,�,�) 2*�.", "���", ch, i);
    } else
        act("1*� ��������1(,�,�,�) ����.", "��", ch);

    if (!AFF_FLAGGED(i, PRF_COMPACT) || !AFF_FLAGGED(i, PRF_BRIEF))
        sprintf(buf + strlen(buf), "\r\n");

    if (IS_UNDEAD(i) && !i->player.description) {
        int tsex, percent;

        if (AFF_FLAGGED(i, AFF_FLY) && GET_POS(i) == POS_FLYING)
            sprintf(buf2, "������");
        else if (AFF_FLAGGED(i, AFF_LEVIT) && GET_POS(i) == POS_FLYING)
            sprintf(buf2, "�����");
        else
            sprintf(buf2, POS_STATE[(int) GET_POS(i)] + 1);

        sprintf(buf, "����� ���� %s ", buf2);
        switch (GET_MOB_VID(i)) {
            case VMOB_SKELET:
                sprintf(buf2, "������");
                tsex = SEX_MALE;
                break;
            case VMOB_ZOMBIE:
                sprintf(buf2, "������������� ����");
                tsex = SEX_NEUTRAL;
                break;
            case VMOB_MUMIE:
                sprintf(buf2, "��������� ������������������ �����");
                tsex = SEX_FEMALE;
                break;
            case VMOB_GHOLA:
                sprintf(buf2, "������� ����");
                tsex = SEX_NEUTRAL;
                break;
            default:
                sprintf(buf2, "����������� ��� ��� ������");
                tsex = SEX_MALE;
                break;

        }
        if (GET_MOB_VID(i) == VMOB_SKELET)
            sprintf(buf + strlen(buf), "%s %s �����. ", buf2, show_height(i, h));
        else if (GET_MOB_VID(i) == VMOB_GHOLA)
            sprintf(buf + strlen(buf), "%s %s %s �����. ", buf2, GET_SEX(i) == SEX_FEMALE ?
                    get_name_pad((char *) race_name_pad_female[(int) GET_RACE(i)], PAD_ROD,
                                 PAD_MONSTER) : get_name_pad((char *) race_name_pad_male[(int)
                                                                                         GET_RACE
                                                                                         (i)],
                                                             PAD_ROD, PAD_MONSTER), show_height(i,
                                                                                                h));
        else
            sprintf(buf + strlen(buf), "%s ����������%s %s %s �����. ", buf2,
                    get_sex_infra(i) == SEX_FEMALE ? "��" : "���",
                    get_sex_infra(i) == SEX_FEMALE ?
                    get_name_pad((char *) race_name_pad_female[(int) GET_RACE(i)], PAD_ROD,
                                 PAD_MONSTER) : get_name_pad((char *) race_name_pad_male[(int)
                                                                                         GET_RACE
                                                                                         (i)],
                                                             PAD_ROD, PAD_MONSTER), show_height(i,
                                                                                                h));
        sprintf(buf + strlen(buf), "� %s %s  ��������� %s %s%s. ", HSHR(i, i),
                GET_MOB_VID(i) == VMOB_SKELET ? "������" : "��������", affected_by_spell(i,
                                                                                         SPELL_ENERGY_UNDEAD)
                ? "����" : "���� ���������",
                i->master ? "����� ������ ������� �����" : "������ ��������� ������� �����",
                GET_MOB_VID(i) ==
                VMOB_GHOLA ? ", � ������������ ����������� ���� ������� �������" : "");

        if (GET_REAL_MAX_HIT(i) > 0)
            percent = (100 * GET_HIT(i)) / GET_REAL_MAX_HIT(i);
        else
            percent = -1;       /* How could MAX_HIT be < 1?? */

        if (percent >= 100)
            sprintf(buf2, "���������� ������������%s � �����%s � ���", GET_CH_SUF_8(i),
                    GET_CH_SUF_8(i));
        else if (percent >= 80)
            sprintf(buf2, "������� ����������%s, �� �����%s � ���", GET_CH_SUF_8(i),
                    GET_CH_SUF_8(i));
        else if (percent >= 60)
            sprintf(buf2, "������� ���������%s, �� �����%s � ���", GET_CH_SUF_8(i),
                    GET_CH_SUF_8(i));
        else if (percent >= 40)
            sprintf(buf2, "������ ���������%s � � ������ ��������� ����������", GET_CH_SUF_8(i));
        else if (percent >= 20)
            sprintf(buf2, "���, ������ ����� ����������");
        else
            sprintf(buf2, "���, ������ ����� ����������� � ����� ������");

        sprintf(a, HSSH(i, i));
        sprintf(buf + strlen(buf), "%s �������� %s.", CAP(a), buf2);

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
            sprintf(buf2, "������");
        else if (AFF_FLAGGED(i, AFF_LEVIT) && GET_POS(i) == POS_FLYING)
            sprintf(buf2, "�����");
        else
            sprintf(buf2, POS_STATE[(int) GET_POS(i)]);

        sprintf(buf, "����� ���� %s %s %s %s %s �����. ",
                buf2,
                show_str(i, s),
                show_con(i, c), race_name[(int) GET_RACE(i)][(int) GET_SEX(i)], show_height(i, h));
        sprintf(buf + strlen(buf), "�� %s %s ���� ���������� %s %s �����.",
                show_cha(i, r),
                IS_AFFECTED(i, AFF_IS_UNDEAD) ? "�������� �������� �������" : show_age(i, a),
                show_int(i, t), eyes_color[(int) GET_EYES(i)]);

        sprintf(buf + strlen(buf), " %s", CAP(diag_char_to_char(i, FALSE)));
        send_to_char(strbraker(string_corrector(buf), ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)), ch);

    }

    if (AFF_FLAGGED(i, AFF_CHARM) && i->master == ch && low_charm(i)
        )
        act("1� ����� ���������� ��������� �� ����.", "��", i, ch);

    if (IS_HORSE(i) && i->master == ch) {
        strcpy(buf, "\r\n��� ��� ������. �� ");
        if (GET_HORSESTATE(i) <= 0)
            strcat(buf, "���-��� �������� ������.");
        else if (GET_HORSESTATE(i) <= 20)
            strcat(buf, "����� �����.");
        else if (GET_HORSESTATE(i) <= 80)
            strcat(buf, "� ������� ���������.");
        else
            strcat(buf, "���� � ����.");
        send_to_char(buf, ch);
    };

    //diag_char_to_char(i, ch);

    found = FALSE;
    for (j = 0; !found && j < NUM_WEARS; j++)
        if ((GET_EQ(i, j) && CAN_SEE_OBJ(ch, GET_EQ(i, j))) ||
            (GET_TATOO(i, j) && CAN_SEE_OBJ(ch, GET_TATOO(i, j))))
            found = TRUE;

    if (found) {
        act("1� ����1(,�,�,�):", "��", i, ch);

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
        act("\r\n�� ���������� ��������� � 2�� ����:", "��", ch, i);

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
            send_to_char("...�� ������ �� ����������.\r\n", ch);
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
                    act("1+� �������1(,�,�,�) ���� ����� ���������� ����������.", "��", ch, tch);
                else if (GET_ID(tch) != GET_OBJ_VAL(object, 1))
                    act("1+� �������1(,�,�,�) @1+�.", "���", ch, tch, object);
                else if (GET_ID(ch) != GET_OBJ_VAL(object, 1))
                    act("1+� �������1(,�,�,�) ��� ����.", "��", ch, tch);
            } else {
                if (GET_ID(tch) != GET_OBJ_VAL(object, 1))
                    act("�� �������� @1+�.", "��", ch, object);
                else
                    act("�� �������� ���� ����.", "�", ch);
            }
        }
    } else if (GET_OBJ_TYPE(object) != ITEM_CONTAINER || IS_OBJ_STAT(object, ITEM_EXTERN)) {
        if (owner)
            act("1*� ����������� ��������1(,�,�,�) ���@1(�,�,�,�) @1�.", "���", ch, object);
        else
            act("1*� ����������� ��������1(,�,�,�) @1�.", "���", ch, object);
    } else {
        if (OBJVAL_FLAGGED(object, EXIT_CLOSED))
            act("1*� �����1(,�,�,�) ��������� � @1�.", "���", ch, object);
        else {
            if (owner)
                act("1*� ��������1(,�,�,�) � ���@1(�,�,�,�) @1�.", "���", ch, object);
            else
                act("1*� ��������1(,�,�,�) � @1�.", "���", ch, object);
        }
    }

    if (*arg) {
        if (((desc = find_exdesc(arg, object->ex_description)) != NULL)
            && GET_OBJ_TYPE(object) != ITEM_FICTION) {
            send_to_charf(ch, strbraker(desc, ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)));
            return;
        } else {
            send_to_charf(ch, "� %s ��� '%s'.\r\n", GET_OBJ_PNAME(object, 1), arg);
            return;
        }
    }

    if (GET_OBJ_TYPE(object) == ITEM_NOTE) {
        if (object->action_description) {
            strcpy(buf, "�� ��������� ���������:\r\n\r\n");
            strcat(buf, object->action_description);
            send_to_charf(ch, buf);
        } else
            send_to_char("�����.\r\n", ch);
        return;
    }


    if (object->main_description)
        strcpy(buf, object->main_description);
    else
        strcpy(buf, "������ ����������.\r\n");

    if (affected_object_by_spell(object, SPELL_MAGIC_VESTMENT) ||
        affected_object_by_spell(object, SPELL_MAGIC_VESTMENT_P) ||
        affected_object_by_spell(object, SPELL_MAGIC_WEAPON)) {
        if (IS_OBJ_STAT(object, ITEM_GOODAURA))
            sprintf(buf + strlen(buf), " �������-������� ���� �������� %s.",
                    GET_OBJ_PNAME(object, 3));
        if (IS_OBJ_STAT(object, ITEM_DARKAURA))
            sprintf(buf + strlen(buf), " �������-������� ���� �������� %s.",
                    GET_OBJ_PNAME(object, 3));
        if (IS_OBJ_STAT(object, ITEM_DEATHAURA))
            sprintf(buf + strlen(buf), " �����-���������� ���� �������� %s.",
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
            sprintf(buf + strlen(buf), " %s ���%s ��� ���������%s.", CAP(GET_OBJ_PNAME(object, 0)),
                    GET_OBJ_SEX(object) == SEX_POLY ? "�����" : "����", GET_OBJ_SUF_7(object));
    }

    if (GET_OBJ_TYPE(object) == ITEM_FURNITURE)
        send_to_char(buf, ch);
    else
        send_to_char(strbraker(string_corrector(buf), sw, PRF_FLAGGED(ch, PRF_AUTOFRM)), ch);

    if (GET_OBJ_TYPE(object) == ITEM_TATOO) {
        send_to_charf(ch, "%s �������� ", CAP(GET_OBJ_PNAME(object, 0)));
        sprinttype(object->obj_flags.Obj_mater, tatoo_color, buf);
        strcat(buf, " ���������.\r\n");
        send_to_char(buf, ch);
        return;
    }
//������� ������� ������
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
            sprintf(buf, "%s ��������.", CAP((char *) OBJN(object, ch, 0)));
        else if (GET_LIGHT_VAL(object) == 0 && !GET_LIGHT_ON(object))
            sprintf(buf, "%s �����%s.", CAP((char *) OBJN(object, ch, 0)), GET_OBJ_SUF_4(object));
        else if (GET_LIGHT_VAL(object) > 0 && GET_LIGHT_VAL(object) < 2)
            sprintf(buf, "%s ����� ��������.", CAP((char *) OBJN(object, ch, 0)));
        else if (!GET_LIGHT_ON(object))
            sprintf(buf, "%s �������%s.", CAP((char *) OBJN(object, ch, 0)), GET_OBJ_SUF_1(object));
        found = TRUE;
        rn = TRUE;
    }

    switch (get_const_obj_temp(object)) {
        case 0:
            sprintf(buf + strlen(buf), "%s ������%s �����.\r\n", CAP(GET_OBJ_PNAME(object, 0)),
                    GET_OBJ_SUF_1(object));
            break;
        case 1:
            sprintf(buf + strlen(buf), "%s �������� �������.\r\n", CAP(GET_OBJ_PNAME(object, 0)));
            break;
        case 2:
            sprintf(buf + strlen(buf), "%s �������� �����.\r\n", CAP(GET_OBJ_PNAME(object, 0)));
            break;
        case 6:
            sprintf(buf + strlen(buf), "%s �������� �����.\r\n", CAP(GET_OBJ_PNAME(object, 0)));
            break;
        case 7:
            sprintf(buf + strlen(buf), "%s �������� ���.\r\n", CAP(GET_OBJ_PNAME(object, 0)));
            break;
        case 8:
            sprintf(buf + strlen(buf), "%s ��������%s �� ������.\r\n",
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
        sprintf(buf2, "%s ������%s.", GET_OBJ_PNAME(object, 0), GET_OBJ_SUF_6(object));
        sprintf(buf + strlen(buf), "\r\n%s", CAP(buf2));
    }

    if (found && !(GET_OBJ_TYPE(object) == ITEM_FOUNTAIN || GET_OBJ_TYPE(object) == ITEM_LIGHT))
        buf[strlen(buf) - 2] = '\0';

    if (GET_OBJ_TYPE(object) == ITEM_TRANSPORT) {
        if (object->transpt && object->transpt->people.size()) {
            sprintf(buf + strlen(buf), "� %s ���%s: ", GET_OBJ_PNAME(object, 5),
                    object->transpt->people.size() == 1 ? "��" : "��");
            for (int i = 0; i < (int) object->transpt->people.size(); i++) {
                sprintf(buf + strlen(buf), "%s%s", CAP(GET_NAME(object->transpt->people[i])),
                        (i + 1) < (int) object->transpt->people.size()? ", " : "");
            }
        } else
            sprintf(buf + strlen(buf), "%s ����%s.", CAP(GET_OBJ_PNAME(object, 0)),
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
        send_to_charf(ch, "��� �� �����.\r\n");
        return;
    }

    if (OBJVAL_FLAGGED(obj, EXIT_CLOSED)) {
        act("@1� ������@1(,�,�,�).\r\n������, ��� ���������� ������, �������� @1��.", "��", ch,
            obj);
        return;
    }

    if (!obj->ex_description) {
        act("� @1� ������ �� ��������.", "��", ch, obj);
        return;
    }

    page = obj->page;

    if ((desc = find_exdesc_number(page, obj->ex_description)) != NULL) {
        strcpy(buf, desc);
        act("1� �������1(��,���,���,���) � ������ @1�.", "���", ch, obj);
    } else
        strcpy(buf, "��� ����� ��������.\r\n");

    send_to_charf(ch, "%s\r\n", buf);
//send_to_char(strbraker(string_corrector(buf), ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)), ch);
//send_to_charf(ch,"\r\n\r\n%20s: %d ]\r\n","[ ��������",page);
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
                sprintf(position, "������");
            else
                sprintf(position, "�����");
        } else
            if (IS_MOB(tch) && MOB_FLAGGED(tch, MOB_SENTINEL)
                && GET_POS(tch) == GET_DEFAULT_POS(tch) && !FIGHTING(tch))
            sprintf(position, "�����");
        else if (IS_MOB(tch) && !NPC_FLAGGED(tch, NPC_MISLEAD) &&
                 GET_POS(tch) == GET_DEFAULT_POS(tch) && GET_DEFAULT_POS(tch) == POS_STANDING)
            sprintf(position, "%s", in_go[(int) GET_MOVE_TYPE(tch)]);
        else if (FIGHTING(tch))
            sprintf(position, "���������");
        else
            sprintf(position, "%s", POS_STATE[(int) GET_POS(tch)] + 1);
    }

    sprintf(size, "%s", show_in_height(tch, h, sex));

    if (light <= 10) {          //������ ��� ������ �����
        if (on_horse(tch)) {
            sprintf(buf2, "%s ����� ������ �� %s", GET_NAME(tch), PERS(get_horse_on(tch), ch, 5));
        } else {
            sprintf(name, "%s", GET_NAME(tch));
            sprintf(buf2, "%s %s", position, name);
        }
    } else if (light <= 20) {   //������ ����� ��� �����
        if (on_horse(tch)) {
            sprintf(buf2, "%s ����� ������ �� ���-��",
                    race_name[(int) GET_RACE(tch)][(int) GET_SEX(tch)]);
        } else {
            sprintf(name, "%s", name_infra(tch, 0));
            sprintf(buf2, "%s %s", position, name);
        }
    } else if (light <= 50) {   //������ �����
        sprintf(size, "%s", show_in_height(tch, h, SEX_MALE));
        sprintf(buf2, "����� ���-�� %s ������", size);
    } else if (light <= 80) {   //������� �����
        sprintf(buf2, "����� ���-�� ������� ������");
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
        //����� ������� � �� ������ �� �����
        if (DOOR_FLAGGED(rdata, EXIT_CLOSED) && !IS_CLOUD_NL(rdata->to_room)) {
            if (rdata->exit_name) {
                if (rdata->general_description
                    && strcmp(rdata->general_description, rdata->exit_name))
                    count += sprintf(buf + count, "%s", rdata->general_description);
                else
                    count +=
                        sprintf(buf + count, "������%s %s.", DSHP(ch, dir),
                                get_name_pad(rdata->exit_name, PAD_IMN, PAD_OBJECT));
            } else
                count += sprintf(buf + count, "������� ���-�� ��������� ������������ �����.");

            count += sprintf(buf + count, "%s", CCNRM(ch, C_NRM));
            send_to_charf(ch, "%s\r\n", buf);
            return;
        };

        //��������� ������� ������ "��������"
        if (PRF_FLAGGED(ch, PRF_TRACKON)) {
            std::vector < int >vit;
            std::vector < int >vot;

            all_ok = FALSE;
            percent = number(1, 100);
            //��������� ��� �����
            vit.push_back(GET_REAL_WIS(ch));
            vit.push_back(GET_REAL_INT(ch));
            //��������� ��� ������
            vot.push_back(dice(1, 40));

            probe = calc_like_skill(ch, NULL, SKILL_TRACKON, vit, vot);

            if (percent <= 5)
                probe = 100;
            else if (percent >= 95)
                probe = 0;

            if (PRF_FLAGGED(ch, PRF_CODERINFO))
                send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", probe, percent);

            if (GET_MOVE(ch) > 0) {
                improove_skill(ch, NULL, 0, SKILL_TRACKON);
                if (probe > percent && check_trackon(ch, rdata->to_room))
                    all_ok = TRUE;
                GET_MOVE(ch)--;
            }
        }
        //������ ���������
        if (IS_TIMEDARK(rdata->to_room) && !IS_AFFECTED(ch, AFF_DARKVISION)) {
            sprintf(room_name, "������� �����");
            light_out += 100;
        } else if (IS_CLOUD_NL(rdata->to_room)) {
            sprintf(room_name, "������ �����");
            light_out += 50;
        } else if (IS_LCLOUD_NL(rdata->to_room)) {
            sprintf(room_name, "�����");
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

        //������ ���������
        if (IS_TIMEDARK(IN_ROOM(ch)) && !IS_AFFECTED(ch, AFF_DARKVISION))
            light_in += 100;
        else if (IS_CLOUD_NL(IN_ROOM(ch)))
            light_in += 50;
        else if (IS_LCLOUD_NL(IN_ROOM(ch)))
            light_in += 20;
        else if (IS_VLCLOUD_NL(IN_ROOM(ch)))
            light_out += 10;

        //������� ����� �������
        count += sprintf(buf + count, "%s.\r\n", room_name);

        //������� ���������� � �������
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
            strcpy(bufs, "��� ������");
        else if (dir == UP)
            strcpy(bufs, "��� �������");
        else if (dir == EAST)
            strcpy(bufs, "�� �������");
        else if (dir == WEST)
            strcpy(bufs, "�� ������");
        else if (dir == SOUTH)
            strcpy(bufs, "�� ���");
        else if (dir == NORTH)
            strcpy(bufs, "�� ������");


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
            count += sprintf(buf + count, "%s (�������).\r\n", CAP(rdata->keyword));
            send_to_char(buf, ch);
            send_to_char(CCNRM(ch, C_NRM), ch);
            return;
        };
        count += sprintf(buf + count, "\r\n");
        send_to_char(buf, ch);
        for (count = 0, tch = world[rdata->to_room].people; tch; tch = tch->next_in_room) {
            if (!AFF_FLAGGED(tch, AFF_SNEAK) && !AFF_FLAGGED(tch, AFF_HIDE)) {
                if (IS_NPC(tch)) {
                    send_to_char("�� ������� ���-�� �����.\r\n", ch);
                } else {
                    send_to_char("��� ���-�� ����.\r\n", ch);
                }
                count++;
            }
        }
        if (!count)
            send_to_char("������ � �����.\r\n", ch);
        send_to_char(CCNRM(ch, C_NRM), ch);
    } else {
        if (info_is & EXIT_SHOW_WALL)
            send_to_char("� ��� �� ������ ��������?\r\n", ch);
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
        log("������: look_in_drink ��������� �� ������ ��� ��������� ������");
        strcat(out_str, "��������� ������ #8993.");
        return (out_str);
    }

    if (GET_OBJ_VAL(obj, 1) <= 0)
        strcat(out_str, "�����.");
    else {
        sprinttype(GET_OBJ_VAL(obj, 2), color_liquid, buf2);
        sprintf(out_str, "%s ��������%s %s %s%s ���������.",
                CAP(GET_OBJ_PNAME(obj, 0)),
                GET_OBJ_SUF_6(obj), fullness[(GET_OBJ_VAL(obj, 1) * 3) / GET_OBJ_VAL(obj, 0)],
                buf2,
                (AFF_FLAGGED(ch, AFF_DETECT_POISON) &&
                 GET_OBJ_VAL(obj, 3) > 0 ? "(�����������)" : ""));
    }

    return (out_str);
}

char *look_in_container(struct char_data *ch, struct obj_data *obj, int bits)
{
    static char out_str[MAX_STRING_LENGTH];

    *out_str = '\0';

    if (ch == NULL || obj == NULL) {
        log("������: look_in_container ��������� �� ������ ��� �������� ������");
        strcat(out_str, "��������� ������ #8994.");
        return (out_str);
    }


    if (OBJVAL_FLAGGED(obj, EXIT_CLOSED))
        sprintf(out_str, "������%s.\r\n", GET_OBJ_SUF_6(obj));
    else if (IS_OBJ_STAT(obj, ITEM_EXTERN)) {
        sprintf(out_str + strlen(out_str), "�� %s:\r\n", OBJN(obj, ch, 5));
        if (!obj->contains || !CAN_SEE_OBJ(ch, obj->contains))
            sprintf(out_str + strlen(out_str), "������ �� �����.\r\n");
        else
            sprintf(out_str + strlen(out_str), "%s", buf_obj_to_char(obj->contains, ch));
    } else {
        if (!obj->contains || !CAN_SEE_OBJ(ch, obj->contains))
            sprintf(out_str + strlen(out_str), "������ %s ������ ���.\r\n", OBJN(obj, ch, 1));
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
        send_to_char("�� ��� �� ��� �������� ����������?\r\n", ch);
        return;
    }

    half_chop(arg, whatp, where);
    what = whatp;

    if (isname(where, "����� ������� room ground"))
        where_bits = FIND_OBJ_ROOM | FIND_CHAR_ROOM;
    else if (isname(where, "��������� inventory"))
        where_bits = FIND_OBJ_INV | FIND_CHAR_ROOM;
    else if (isname(where, "���������� equipment"))
        where_bits = FIND_OBJ_EQUIP | FIND_CHAR_ROOM;

    bits = generic_find(what, where_bits, ch, &found_char, &found_obj);


    /* ������� �� ��������� */
    if (found_char != NULL) {
        if (subcmd == SCMD_READ) {
            act("�� 2+� ������ �� ��������.", "��", ch, found_char);
            return;
        }

        exam_at_char(found_char, ch, where);

        return;
    }

    /*������� �� ������� */
    if (found_obj != NULL) {
        if (subcmd == SCMD_READ && GET_OBJ_TYPE(found_obj) == ITEM_FICTION)
            read_book(found_obj, ch);
        else
            exam_obj_to_char(found_obj, ch, bits, where);
        return;
    }


    /* ���� ���.�������� � ������� */
    if ((desc = find_exdesc(what, world[ch->in_room].ex_description)) != NULL) {
        page_string(ch->desc, strbraker(desc, ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)), 1);
        return;
    }

    send_to_charf(ch, "��� ������ �������� �� '%s'.\r\n", what);
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
            sprintf(out_str + strlen(out_str), " (�������%s)", GET_OBJ_SUF_6(object));
        if (IS_OBJ_STAT(object, ITEM_GOODAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(out_str, " (�������-������� ����)");

        if (IS_OBJ_STAT(object, ITEM_DARKAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(out_str, " (�������-������� ����)");

        if (IS_OBJ_STAT(object, ITEM_DEATHAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(out_str, " (���������� ����)");


        if (IS_OBJ_STAT(object, ITEM_MAGIC) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(out_str, " (����� ����)");
        if (IS_OBJ_STAT(object, ITEM_POISONED) && AFF_FLAGGED(ch, AFF_DETECT_POISON))
            sprintf(out_str + strlen(out_str), " (��������%s)", GET_OBJ_SUF_6(object));

        if (IS_OBJ_STAT(object, ITEM_GLOW))
            strcat(out_str, " (����� ��������)");
        if (IS_OBJ_STAT(object, ITEM_HUM) && !IS_AFFECTED(ch, AFF_DEAFNESS))
            strcat(out_str, " (���� �����)");
        if (IS_OBJ_STAT(object, ITEM_FIRE))
            strcat(out_str, " (���� �����)");

        if ((GET_OBJ_TYPE(object) == ITEM_LIGHT && GET_LIGHT_VAL(object) > 0
             && GET_LIGHT_ON(object)) || (GET_OBJ_TYPE(object) != ITEM_LIGHT
                                          && GET_LIGHT_VAL(object) > 0))
            strcat(out_str, " (���� ���������)");
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
                sprintf(buf2, "��� ���� ����� �� ���.");
            else
                sprintf(buf2, "��� ���� ����� �����.");
        } else
            if (SECT(IN_ROOM(object)) == SECT_WATER_SWIM ||
                SECT(IN_ROOM(object)) == SECT_WATER_NOSWIM ||
                SECT(IN_ROOM(object)) == SECT_UNDERWATER)
            sprintf(buf2, "%s ���%s �� ���.", object->PNames[0],
                    (GET_OBJ_SEX(object) == SEX_POLY) ? "��" : "��");
        else if (GET_OBJ_TYPE(object) == ITEM_TRAP && object->trap_victim) {
            if (object->trap_victim == ch)
                sprintf(buf2, "%s ������ ��� �� ����.", object->PNames[0]);
            else
                return;
        } else
            strcpy(buf2, object->description);

        strcat(buf, CAP(buf2));

        if (how > 1)
            sprintf(buf + strlen(buf), " [%d]", how);

        if (IS_OBJ_STAT(object, ITEM_INVISIBLE))
            sprintf(buf + strlen(buf), " (�������%s)", GET_OBJ_SUF_6(object));
        if (IS_OBJ_STAT(object, ITEM_GOODAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (�������-������� ����)");
        if (IS_OBJ_STAT(object, ITEM_DARKAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (�������-������� ����)");
        if (IS_OBJ_STAT(object, ITEM_DEATHAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (���������� ����)");


        if (IS_OBJ_STAT(object, ITEM_MAGIC) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (����� ����)");
        if (IS_OBJ_STAT(object, ITEM_POISONED) && AFF_FLAGGED(ch, AFF_DETECT_POISON))
            sprintf(buf + strlen(buf), " (��������%s)", GET_OBJ_SUF_6(object));
        if (IS_OBJ_STAT(object, ITEM_GLOW))
            strcat(buf, " (����� ��������)");
        if (IS_OBJ_STAT(object, ITEM_HUM) && !IS_AFFECTED(ch, AFF_DEAFNESS))
            strcat(buf, " (���� �����)");
        if (IS_OBJ_STAT(object, ITEM_FIRE))
            strcat(buf, " (���� �����)");
        if ((GET_OBJ_TYPE(object) == ITEM_LIGHT && GET_LIGHT_VAL(object) > 0
             && GET_LIGHT_ON(object)) || (GET_OBJ_TYPE(object) != ITEM_LIGHT
                                          && GET_LIGHT_VAL(object) > 0))
            strcat(buf, " (���� ��������)");

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
                strcpy(buf, "�� ��������� ��������� :\r\n\r\n");
                strcat(buf, object->action_description);
                /////////////////////////////////////////
                if PRF_FLAGGED
                    (ch, PRF_AUTOFRM)
                        page_string(ch->desc, strbraker(buf, ch->sw, 1), 1);
                else
                    page_string(ch->desc, strbraker(buf, ch->sw, 0), 1);
                //////////////////////////////////
            } else
                send_to_char("�����.\r\n", ch);
            return;
        } else if (GET_OBJ_TYPE(object) == ITEM_DRINKCON)
            /* ITEM_TYPE == ITEM_DRINKCON||FOUNTAIN */
            strcpy(buf, "��� ������� ��� ��������.");
    }

    if (how > 1)
        sprintf(buf + strlen(buf), " [%d]", how);
    if (mode != 3 && how <= 1) {
        if (IS_OBJ_STAT(object, ITEM_INVISIBLE)) {
            sprintf(buf2, " (�������%s)", GET_OBJ_SUF_6(object));
            strcat(buf, buf2);
        }
        if (IS_OBJ_STAT(object, ITEM_GOODAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (�������-������� ����)");
        if (IS_OBJ_STAT(object, ITEM_DARKAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (�������-������� ����)");
        if (IS_OBJ_STAT(object, ITEM_DEATHAURA) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (���������� ����)");
        if (IS_OBJ_STAT(object, ITEM_MAGIC) && AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
            strcat(buf, " (����� ����)");
        if (IS_OBJ_STAT(object, ITEM_POISONED) && AFF_FLAGGED(ch, AFF_DETECT_POISON)) {
            sprintf(buf2, " (��������%s)", GET_OBJ_SUF_6(object));
            strcat(buf, buf2);
        }
        if (IS_OBJ_STAT(object, ITEM_GLOW))
            strcat(buf, " (����� ��������)");
        if (IS_OBJ_STAT(object, ITEM_HUM))
            strcat(buf, " (���� �����)");
        if (IS_OBJ_STAT(object, ITEM_FIRE))
            strcat(buf, " (���� �����)");
        if ((GET_OBJ_TYPE(object) == ITEM_LIGHT && GET_LIGHT_VAL(object) > 0
             && GET_LIGHT_ON(object)) || (GET_OBJ_TYPE(object) != ITEM_LIGHT
                                          && GET_LIGHT_VAL(object) > 0))
            strcat(buf, " (���� ���������)");
    }
    strcat(buf, "\r\n");
    if (mode >= 5) {
        strcat(buf, diag_weapon_to_char(object, TRUE));
        if (show_state) {
            *buf2 = '\0';
            if (mode == 1 && how <= 1) {        /*if (GET_OBJ_TYPE(object) == ITEM_LIGHT)
                                                   {if (GET_LIGHT_VAL(object) == -1)
                                                   strcpy(buf2," (������ ����)");
                                                   else
                                                   if (GET_LIGHT_VAL(object) == 0)
                                                   sprintf(buf2," (�����%s)",
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
                        strcat(buf2, " (���� ����������)");
                    else
                        sprintf(buf2 + strlen(buf2), " (����%s)", GET_OBJ_SUF_1(object));
                }
            } else if (mode >= 2 && how <= 1) { /*if(GET_OBJ_TYPE(object) == ITEM_LIGHT)
                                                   {if (GET_LIGHT_VAL(object) == -1)
                                                   sprintf(buf2,"\r\n%s ���� ������ ����.",
                                                   CAP((char *) OBJN(object,ch,0)));
                                                   else
                                                   if (GET_LIGHT_VAL(object) == 0)
                                                   sprintf(buf2,"\r\n%s �����%s.",
                                                   CAP((char *) OBJN(object,ch,0)), GET_OBJ_SUF_4(object));
                                                   else
                                                   sprintf(buf2,"\r\n%s ����� ������� %d %s.",
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
        send_to_char("�� �����!\r\n", ch);
        return;
    }

    half_chop(argument, arg, arg2);

    if (!*arg && subcmd == SCMD_EXAMINE) {
        send_to_charf(ch, "��� �� ������ ���������?\r\n");
        return;
    }

    if (!*arg && subcmd == SCMD_READ) {
        send_to_charf(ch, "��� �� ������ ���������?\r\n");
        return;
    }

    if (!*arg)
        look_at_room(ch, 1);
    else {
        /*  if (((look_type = search_block(arg, dirs, FALSE)) >= 0) ||
           ((look_type = search_block(arg, Dirs, FALSE)) >= 0))
           {
           send_to_char("�� ���������� ��:\r\n",ch);
           look_in_direction(ch, look_type);
           return;
           }
           else */
        {
            if (is_abbrev(arg, "at") || is_abbrev(arg, "��"))
                look_at_target(ch, arg2, subcmd);
            else
                look_at_target(ch, argument, subcmd);
            return;
        }
        send_to_charf(ch, "����� ��� '%s'\r\n", arg);
    }
}

ACMD(do_scan)
{
    int i;

    if (!ch->desc)
        return;

    if (GET_POS(ch) <= POS_SLEEPING)
        send_to_char("������� ����� ��� �����������...\r\n", ch);
    else if (AFF_FLAGGED(ch, AFF_BLIND))
        send_to_char("�� �����!\r\n", ch);
    else {
        send_to_char("�� ���������� �� ��������:\r\n", ch);
        for (i = 0; i < NUM_OF_DIRS; i++)
            look_in_direction(ch, i);

        /* ������� ����� ����������� ������� */
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
        send_to_char("�� ����������� ������������ � ������ ���������:\r\n", ch);
        send_to_char("�� �������� � ���� ����� ���������� �����.\r\n", ch);
        return;
    }

    if (FIGHTING(i)) {
        send_to_char("���� ���� ������� ������ ������������!\r\n", ch);
        return;
    }

    if (GET_MOVE(ch) < 1) {
        send_to_charf(ch, "� ��� �� ������ ���.\r\n");
        return;
    }

    WAIT_STATE(ch, PULSE_VIOLENCE);

    if (ch != i) {
        found = FALSE;
        if (!IS_GOD(ch))
            GET_MOVE(ch)--;
        act("�� ������������ � ��������� 2�:", "��", ch, i);
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
            send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

        if (percent <= prob && GET_GOLD(i) >= 1) {
            act("�� ��������, ��� � 2� ���� #1 �����#1(�,�,).", "���", ch, i, GET_GOLD(i));
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
            send_to_char("...�� ������ �� ����������.\r\n", ch);

        if (CAN_SEE(i, ch) && prob + GET_REAL_LCK(ch) < number(1, 50)) {
            act("2+� �������2(,�,�,�) ���� ������� ����������� 2�� ���������.", "��", ch, i);
            act("�� ��������, ��� 1+� �������� ������ � ���������� ����� ��������.", "��", ch, i);
            act("2+� �������2(,�,�,�), ��� 1+� �������1(���,���,���,���) ��������� � �2�� � ������.", "���", ch, i);
            inc_pk_thiefs(ch, i);
            if (IS_SHOPKEEPER(i))
                act("2� ���������(,�,�,�) ������� ������!", "��", ch, i);
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
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (GET_POS(ch) < POS_SLEEPING) {
        send_to_char("������� ����� ��� �����������...\r\n", ch);
        return;
    } else if (AFF_FLAGGED(ch, AFF_BLIND)) {
        send_to_char("�� �����!\r\n", ch);
        return;
    }

    if (FIGHTING(ch)) {
        send_to_char("�� ���������� �� ���� �����.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("� ���� �� ������ ������������?\r\n", ch);
        return;
    }

    bits = generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM |
                        FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

    if (tmp_char != NULL) {
        go_look_hide(ch, tmp_char);
        return;
    }
    send_to_charf(ch, "����� ��� \'%s\'.\r\n", arg);

}


ACMD(do_hearing)
{
    int i;

    if (!ch->desc)
        return;


    if (GET_POS(ch) < POS_SLEEPING)
        send_to_char("��� ������ ��������� ������ �������, ������� ��� � ����.\r\n", ch);
    if (GET_POS(ch) == POS_SLEEPING)
        send_to_char("������ �������� ��������� ������ ����� �� ������� � ������� �����������.\r\n",
                     ch);
    else
     if (check_moves(ch, HEARING_MOVES)) {
        send_to_char("�� ������ �������������� ��������������.\r\n", ch);
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
        send_to_char("� ��� � �������� �����!\r\n", ch);
    else if (GET_GOLD(ch) == 1)
        send_to_char("� ��� � �������� ����� ���� ������.\r\n", ch);
    else {
        count +=
            sprintf(buf, "� ��� � �������� ���� %d %s.\r\n", GET_GOLD(ch),
                    desc_count(GET_GOLD(ch), WHAT_MONEYa));
        send_to_char(buf, ch);
    }
}


const char *wear_text[] = {
    "�� ������ �� ������.",
    "�� �� ���������� ���� �����.",
    "�� ������ ���������.",
    "�� ������ ���������.",
    "�� ���������� ��� ����� �����.",
    "�� �� �������� ���������� � �����."
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
        sprintf(buf, "�� ������������ ���� %s %s %d ������",
                GET_SEX(ch) ==
                SEX_FEMALE ? get_name_pad((char *) race_name_pad_female[(int) GET_RACE(ch)], 1,
                                          PAD_MONSTER) : get_name_pad((char *)
                                                                      race_name_pad_male[(int)
                                                                                         GET_RACE
                                                                                         (ch)], 1,
                                                                      PAD_MONSTER), GET_PAD(ch, 1),
                GET_LEVEL(ch));
        sprintf(buf + strlen(buf), ", ��� %d %s.\r\n", GET_AGE(ch),
                desc_count(GET_AGE(ch), WHAT_YEAR));
        if (GET_GODS(ch))
            sprintf(buf + strlen(buf), "�� ������������ %s.\r\n", gods_name_2[(int) GET_GODS(ch)]);
        else if (GET_SEX(ch) == SEX_FEMALE)
            sprintf(buf + strlen(buf), "�� ��������.\r\n");
        else
            sprintf(buf + strlen(buf), "�� ������.\r\n");

        sprintf(buf + strlen(buf), "����� %d %s �� ������������.\r\n", EXTRACT_TIMER(ch),
                desc_count(EXTRACT_TIMER(ch), WHAT_MINu));
        sprintf(buf + strlen(buf), "�� �������� ������ ��� ����� ������� �����.\r\n");
        send_to_charf(ch, buf);
        return;
    }

    if (!NAME_GOD(ch)) {
        sprintf(buf + strlen(buf), "&R���� ��� �� ��������.&n\r\n");
    } else if (NAME_GOD(ch) < 1000) {
        sprintf(buf + strlen(buf), "&R���� ��� �������� ���-�� �� �����.&n\r\n");
    }

    sprintf(buf + strlen(buf), "�� %s %s",
            race_name[(int) GET_RACE(ch)][(int) GET_SEX(ch)], GET_NAME(ch));

    if (GET_LEVEL(ch))
        sprintf(buf + strlen(buf), " %d ������", GET_LEVEL(ch) + GET_LEVEL_ADD(ch));

    sprintf(buf + strlen(buf), ", ��� %d %s.", GET_AGE(ch), desc_count(GET_AGE(ch), WHAT_YEAR));


    if (age_old(ch)->month == 0 && age_old(ch)->day == 0) {
        sprintf(buf2, " &G� ��� ������� ���� ��������.&n\r\n");
        strcat(buf, buf2);
    } else
        strcat(buf, "\r\n");

    if (GET_RTITLE(ch))
        sprintf(buf + strlen(buf), "�� ��������� �����: \"%s\"\r\n", GET_RTITLE(ch));
    else if (GET_TITLE(ch))
        sprintf(buf + strlen(buf), "��� �����: \"%s\"\r\n", only_title(ch));

    ////////////

    if (GET_GODS(ch))
        sprintf(buf + strlen(buf), "�� ������������ %s.\r\n", gods_name_2[(int) GET_GODS(ch)]);
    else
        sprintf(buf + strlen(buf), "�� ������.\r\n");


    *buf2 = '\0';
    for (int icls = 0; icls < NUM_CLASSES; icls++) {
        if (ch->classes[icls] > 0)
            sprintf(buf2 + strlen(buf2), "%10s %-2d ������.\r\n", class_name[icls],
                    ch->classes[icls]);

    }

    if (*buf2)
        sprintf(buf + strlen(buf), "�� �������� ���������� �����������:\r\n%s", buf2);
    else
        sprintf(buf + strlen(buf), "�� �� ������ �� ����� ���������.\r\n");
    send_to_char(buf, ch);

    ////////////
    *buf = '\0';

    if (IS_MANA_CASTER(ch))
        sprintf(buf + strlen(buf),
                "� ��� ������ %d(%d) %s �����, %d(%d) %s ���� � %d(%d) %s ��������.\r\n",
                GET_HIT(ch), GET_REAL_MAX_HIT(ch), desc_count(GET_HIT(ch), WHAT_ONEa),
                GET_MANA(ch), GET_REAL_MAX_MANA(ch), desc_count(GET_HIT(ch), WHAT_ONEa),
                GET_MOVE(ch), GET_REAL_MAX_MOVE(ch), desc_count(GET_MOVE(ch), WHAT_ONEa));
    else
        sprintf(buf + strlen(buf),
                "� ��� ������ %d(%d) %s ����� � %d(%d) %s ��������.\r\n",
                GET_HIT(ch), GET_REAL_MAX_HIT(ch), desc_count(GET_HIT(ch), WHAT_ONEa),
                GET_MOVE(ch), GET_REAL_MAX_MOVE(ch), desc_count(GET_MOVE(ch), WHAT_ONEa));


    sprintf(buf + strlen(buf), "��� ���� ���������� %ld %s.\r\n",
            GET_EXP(ch), desc_count(GET_EXP(ch), WHAT_POINT));

    next_level = get_dsu_exp(ch);

    if (GET_LEVEL(ch) < LVL_IMMORT) {
        if (next_level > 0)
            sprintf(buf + strlen(buf), "��� �������� ������� %d %s �� ���������� ������.\r\n",
                    next_level, desc_count(next_level, WHAT_POINT));
        else
            sprintf(buf + strlen(buf), "� ��� ���������� �����, ����� �������� ���� �������.\r\n");
    }
    if (GET_HONOR(ch))
        sprintf(buf + strlen(buf), "� ��� %ld %s �����.\r\n",
                GET_HONOR(ch), desc_count(GET_HONOR(ch), WHAT_POINT));


    sprintf(buf + strlen(buf), "� ��� ���� %d %s ���������.\r\n",
            GET_GOLD(ch), desc_count(GET_GOLD(ch), WHAT_MONEYa));

    /*  if (GET_BANK_GOLD(ch) > 0)
       sprintf(buf + strlen(buf)," � ��� %ld %s ����� � �����.\r\n",
       GET_BANK_GOLD(ch), desc_count(GET_BANK_GOLD(ch),WHAT_MONEYa));
       else
       strcat(buf,".\r\n");       */

    if (GET_GLORY(ch))
        sprintf(buf + strlen(buf), "�� ��������� %d %s �����.\r\n",
                GET_GLORY(ch), desc_count(GET_GLORY(ch), WHAT_POINT));

    playing_time = *real_time_passed((time(0) - ch->player.time.logon) + ch->player.time.played, 0);

    sprintf(buf + strlen(buf), "�� �������� %s.\r\n", languages_d[SPEAKING(ch) - MIN_LANGUAGES]);

    ticks = GET_TICKS(ch);
    tick_day = ticks / 1440;
    tick_hours = (ticks - (tick_day * 1440)) / 60;

    sprintf(buf + strlen(buf), "�� ������� %d %s %d %s ��������� �������.\r\n",
            tick_day, desc_count(tick_day, WHAT_DAY),
            tick_hours, desc_count(tick_hours, WHAT_HOUR));


    if (IS_CARRYING_W(ch) + IS_WEARING_W(ch)) {
        int percent = (((IS_CARRYING_W(ch) + IS_WEARING_W(ch)) * 100) / MAX_MOVE_W(ch)) / 25;
        int wr_t = MAX(MIN(percent + 1, 5), 1);

        sprintf(buf + strlen(buf), "%s\r\n", wear_text[wr_t]);
    } else
        sprintf(buf + strlen(buf), "�� ������ �� ������.\r\n");

    if (!on_horse(ch))
        switch (GET_POS(ch)) {
            case POS_DEAD:
                strcat(buf, "&R�� ������!&n\r\n");
                break;
            case POS_MORTALLYW:
                strcat(buf, "�� ���������� ������ � ���������� � ������!\r\n");
                break;
            case POS_INCAP:
                strcat(buf, "�� ��� �������� � �������� ��������...\r\n");
                break;
            case POS_STUNNED:
                strcat(buf, "�� � �������� ��������!\r\n");
                break;
            case POS_SLEEPING:
                strcat(buf, "�� �����.\r\n");
                break;
            case POS_RESTING:
                strcat(buf, "�� ���������.\r\n");
                break;
            case POS_SITTING:
                strcat(buf, "�� ������.\r\n");
                break;
            case POS_FIGHTING:
                if (FIGHTING(ch))
                    sprintf(buf + strlen(buf), "�� ���������� � %s.\r\n", GET_PAD(FIGHTING(ch), 4));
                else
                    strcat(buf, "�� ������ �������� �� �������.\r\n");
                break;
            case POS_STANDING:
                strcat(buf, "�� ������.\r\n");
                break;
            case POS_FLYING:
                strcat(buf, "�� �������.\r\n");
                break;
            default:
                strcat(buf, "You are floating.\r\n");
                break;
        }
    send_to_char(buf, ch);

    if (ch->trap_object)
        send_to_charf(ch, "�� ������ � %s.\r\n", GET_OBJ_PNAME(ch->trap_object, 0));

    if (ch->master)
        send_to_charf(ch, "�� �������� �� %s.\r\n", hide_race(ch->master, 4));
    if (ch->party_leader)
        send_to_charf(ch, "�� �������� � ������ %s.\r\n", hide_race(ch->party_leader, 1));
    if (ch->party)
        send_to_charf(ch, "�� ����� ������.\r\n");

    if (GUARDING(ch))
        send_to_charf(ch, "�� ��������� %s.\r\n", GET_PAD(GUARDING(ch), 3));

    if (ch->is_transpt) {
        *buf = '\0';
        sprintf(buf + strlen(buf), "�� ������ � %s", GET_OBJ_PNAME(ch->is_transpt, 5));
        if (ch->is_transpt->transpt->driver)
            sprintf(buf + strlen(buf), " � ����������");
        sprintf(buf + strlen(buf), ".\r\n");
        send_to_char(buf, ch);
    }

    const char *mess = '\0';

    if ((mess = get_status_event(ch)))
        send_to_charf(ch, "%s.\r\n", mess);

    *buf = '\0';

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) && IS_GOD(ch))
        strcat(buf, "�� ���������� ����� ��� � �����������.\r\n");

    if (GET_COND(ch, FULL) > 0 && GET_COND(ch, FULL) <= SECS_PER_MUD_TICK)
        strcat(buf, "�� ������� �������������.\r\n");
    if (GET_COND(ch, THIRST) > 0 && GET_COND(ch, THIRST) <= SECS_PER_MUD_TICK)
        strcat(buf, "�� ������ ������.\r\n");
    if (GET_COND(ch, SLEEP) > 0 &&
        GET_COND(ch, SLEEP) <= SECS_PER_MUD_TICK && GET_POS(ch) != POS_SLEEPING)
        strcat(buf, "�� ����� �� �����.\r\n");

    strcat(buf, CCIRED(ch, C_NRM));
    if (GET_COND(ch, DRUNK) >= CHAR_DRUNKED) {
        strcat(buf, "�� �����.\r\n");
    }

    if (GET_COND(ch, FULL) == 0)
        strcat(buf, "�� ������ ����.\r\n");
    if (GET_COND(ch, THIRST) == 0)
        strcat(buf, "��� ������ �����.\r\n");
    if (GET_COND(ch, SLEEP) == 0)
        strcat(buf, "�� ������ �����.\r\n");
    strcat(buf, CCNRM(ch, C_NRM));

    /*  if (PRF_FLAGGED(ch, PRF_SUMMONABLE))
       strcat(buf, "\r\n�� ������ ���� ��������.\r\n"); */

    if (has_horse(ch, FALSE)) {
        if (on_horse(ch))
            sprintf(buf + strlen(buf), "�� ������ �� %s.\r\n", GET_PAD(get_horse(ch), 5));
        else
            sprintf(buf + strlen(buf), "� ��� ���� %s.\r\n", GET_NAME(get_horse(ch)));
    }

    strcat(buf, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);
    if (RENTABLE(ch)) {
        sprintf(buf, "%s� ����� � ������� ���������� �� �� ������ ���� �� ������.%s\r\n",
                CCIRED(ch, C_NRM), CCNRM(ch, C_NRM));
        send_to_char(buf, ch);
    }
    if (PLR_FLAGGED(ch, PLR_HELLED) && HELL_DURATION(ch) && HELL_DURATION(ch) > time(NULL)
        ) {
        int hrs = (HELL_DURATION(ch) - time(NULL)) / 3600;
        int mins = ((HELL_DURATION(ch) - time(NULL)) % 3600 + 59) / 60;

        sprintf(buf, "��� ��������� �������� � ��� ��� %d %s %d %s.\r\n",
                hrs, desc_count(hrs, WHAT_HOUR), mins, desc_count(mins, WHAT_MINu));
        send_to_char(buf, ch);
    }
}


ACMD(do_inventory)
{
    char buf[MAX_STRING_LENGTH];

    strcpy(buf, "� ��� � �����:\r\n");
    if (ch->carrying)
        strcat(buf, buf_obj_to_char(ch->carrying, ch));
    else
        strcat(buf, "������ ���.\r\n");
    send_to_charf(ch, buf);
}


ACMD(do_equipment)
{
    int i, found = 0;

    send_to_char("� ��� � ����������:\r\n", ch);
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
                send_to_char("���-��.\r\n", ch);
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
            send_to_char("�� �������.\r\n", ch);
        else
            send_to_char("�� �������.\r\n", ch);
    }
}


ACMD(do_time)
{
    char buf[MAX_STRING_LENGTH];
    int day;

    if (IS_NPC(ch))
        return;
    sprintf(buf, "������ ");

    switch (zone_table[world[IN_ROOM(ch)].zone].time_info.hours % 24) {
        case 0:
            sprintf(buf + strlen(buf), "�������, ");
            break;
        case 1:
            sprintf(buf + strlen(buf), "1 ��� ����, ");
            break;
        case 2:
        case 3:
        case 4:
            sprintf(buf + strlen(buf), "%d ���� ����, ",
                    zone_table[world[IN_ROOM(ch)].zone].time_info.hours);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            sprintf(buf + strlen(buf), "%d ����� ����, ",
                    zone_table[world[IN_ROOM(ch)].zone].time_info.hours);
            break;
        case 12:
            sprintf(buf + strlen(buf), "�������, ");
            break;
        case 13:
            sprintf(buf + strlen(buf), "1 ��� ���������, ");
            break;
        case 14:
        case 15:
        case 16:
            sprintf(buf + strlen(buf), "%d ���� ���������, ",
                    zone_table[world[IN_ROOM(ch)].zone].time_info.hours - 12);
            break;
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
            sprintf(buf + strlen(buf), "%d ����� ������, ",
                    zone_table[world[IN_ROOM(ch)].zone].time_info.hours - 12);
            break;
    }

    strcat(buf, weekdays[zone_table[world[IN_ROOM(ch)].zone].weather_info.week_day]);
    switch (zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight) {
        case SUN_DARK:
            strcat(buf, ", �����");
            break;
        case SUN_SET:
            strcat(buf, ", ��������");
            break;
        case SUN_LIGHT:
            strcat(buf, ", ������");
            break;
        case SUN_RISE:
            strcat(buf, ", ����������");
            break;
    }
    strcat(buf, ".\r\n");
    send_to_char(buf, ch);

    day = zone_table[world[IN_ROOM(ch)].zone].time_info.day + 1;        /* day in [1..35] */

    *buf = '\0';
    sprintf(buf + strlen(buf), "%s, %d� ����, ��� %d",
            month_name[(int) zone_table[world[IN_ROOM(ch)].zone].time_info.month], day,
            zone_table[world[IN_ROOM(ch)].zone].time_info.year);
    switch (zone_table[world[IN_ROOM(ch)].zone].weather_info.season) {
        case SEASON_WINTER:
            strcat(buf, ", ����");
            break;
        case SEASON_SPRING:
            strcat(buf, ", �����");
            break;
        case SEASON_SUMMER:
            strcat(buf, ", ����");
            break;
        case SEASON_AUTUMN:
            strcat(buf, ", �����");
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

    const char *sky_look[] = { "���� ��������",
        "���� ���������",
        "���� ������� �������� ������",
        "���� �����"
    };

    const char *sky_look_sigil[] = { "��������� ����",
        "����",
        "������ ����",
        "����"
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
                 0 ? "����������� �������� ����������." : "����������� �������� ����������."));
        sprintf(buf + strlen(buf), "�����������: %d %s.\r\n", weather_info.temperature,
                desc_count(weather_info.temperature, WHAT_DEGREE));

        if (IS_SET(weather_info.weather_type, WEATHER_BIGWIND))
            strcat(buf, "������� �����.\r\n");
        else if (IS_SET(weather_info.weather_type, WEATHER_MEDIUMWIND))
            strcat(buf, "��������� �����.\r\n");
        else if (IS_SET(weather_info.weather_type, WEATHER_LIGHTWIND))
            strcat(buf, "������ �������.\r\n");

        if (IS_SET(weather_type, WEATHER_BIGSNOW))
            strcat(buf, "����� ����.\r\n");
        else if (IS_SET(weather_type, WEATHER_MEDIUMSNOW))
            strcat(buf, "��������.\r\n");
        else if (IS_SET(weather_type, WEATHER_LIGHTSNOW))
            strcat(buf, "������ ������.\r\n");

        if (IS_SET(weather_type, WEATHER_GRAD))
            strcat(buf, "����� � ������.\r\n");
        else if (IS_SET(weather_type, WEATHER_BIGRAIN))
            strcat(buf, "������� ������.\r\n");
        else if (IS_SET(weather_type, WEATHER_MEDIUMRAIN))
            strcat(buf, "���� �����.\r\n");
        else if (IS_SET(weather_type, WEATHER_LIGHTRAIN))
            strcat(buf, "������� ������.\r\n");

        send_to_char(buf, ch);
    } else
        send_to_char("�� ������ �� ������ ������� � ������ �������.\r\n", ch);
    if (IS_GOD(ch)) {
        sprintf(buf, "����: %d �����: %s ���: %d ���� = %d\r\n"
                "����������� =%-5d, �� ���� = %-8d, �� ������ = %-8d\r\n"
                "��������    =%-5d, �� ���� = %-8d, �� ������ = %-8d\r\n"
                "������ ����� = %d(%d), ��� = %d(%d). ������ = %08x(%08x).\r\n",
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
    "������: ��� [�������[-��������]] [-n ���] [-c ��������] [-s] [-o] [-q] [-r] [-z] [-h]\r\n"

#define MORT_WHO_FORMAT \
    "������: ��� [���]\r\n"


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

    if (!str_cmp(name_search, "����") && strlen(name_search) == 4)
        only_gods = TRUE;

    sprintf(imms, "%s�����������%s\r\n-----------------\r\n", CCICYN(ch, C_NRM), CCNRM(ch, C_NRM));
    sprintf(morts, "%s��������%s\r\n-----------------\r\n", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));

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
            sprintf(name + strlen(name), " (����)");
        if (IS_SOUL(tch) && IS_GOD(ch))
            sprintf(name + strlen(name), " (�������)");
        if (GET_INVIS_LEV(tch) && IS_GOD(ch))
            sprintf(name + strlen(name), " (�%d)", GET_INVIS_LEV(tch));
        else if (AFF_FLAGGED(tch, AFF_INVISIBLE) && IS_GOD(ch))
            sprintf(name + strlen(name), " (�������%s)", GET_CH_SUF_6(tch));
        if (AFF_FLAGGED(tch, AFF_HIDE) && IS_GOD(ch))
            strcat(name, " (��������)");
        if (AFF_FLAGGED(tch, AFF_CAMOUFLAGE) && IS_GOD(ch))
            strcat(name, " (�����������)");
        if (PLR_FLAGGED(tch, PLR_MAILING) && IS_GOD(ch))
            strcat(name, " (���������� ������)");
        else if (PLR_FLAGGED(tch, PLR_SCRIPTING) && IS_GOD(ch))
            strcat(name, " (����� ��������)");
        else if (PLR_FLAGGED(tch, PLR_WRITING) && IS_GOD(ch))
            strcat(name, " (�����)");
        if (PRF_FLAGGED(tch, PRF_NOHOLLER) && IS_GOD(ch))
            sprintf(name + strlen(name), " (����%s)", GET_CH_SUF_1(tch));
        if (PRF_FLAGGED(tch, PRF_NOTELL) && IS_GOD(ch))
            sprintf(name + strlen(name), " (�����%s)", GET_CH_SUF_6(tch));
        if (PLR_FLAGGED(tch, PLR_MUTE) && IS_GOD(ch))
            sprintf(name + strlen(name), " (������)");
        if (PLR_FLAGGED(tch, PLR_DUMB) && IS_GOD(ch))
            sprintf(name + strlen(name), " (���%s)", GET_CH_SUF_6(tch));

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
        send_to_char("\r\n�� ������ �� ������.\r\n", ch);
        return;
    }

    sprintf(buf + strlen(buf), "\r\n������ � ����:");

    if (num_imms)
        sprintf(buf + strlen(buf), " ����������� %d%s", num_imms, (num_morts) ? "," : ".");

    if (num_morts)
        sprintf(buf + strlen(buf), " �������� %d.", num_morts);


    strcat(buf, "\r\n");

    if (!only_gods && !*name_search)
        sprintf(buf + strlen(buf),
                "������������ ���-�� � ������� ������������: %d %s.\r\n",
                mud->getStats()->getHigh(), desc_count(mud->getStats()->getHigh(), WHAT_PERSONA));

    page_string(ch->desc, buf, 1);
}


#define USERS_FORMAT \
    "������: users [-l minlevel[-maxlevel]] [-n name] [-h host] [-c classlist] [-o] [-p]\r\n"
#define MAX_LIST_LEN 200
ACMD(do_users)
{
    char *get_addr_ip(char *arg);
    const char *format = "%3d %-7s %-12s %-14s %-3s %-8s ";
    char line[200], line2[220], idletime[10], classname[20];
    char state[30] = "\0", *timeptr, mode;
    char name_search[MAX_INPUT_LENGTH] = "\0", host_search[MAX_INPUT_LENGTH];

// ����
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
        strcpy(line, "��� �������    ���         ���������    Idl �����    E-mail\r\n");
    } else {
        strcpy(line, "��� �������    ���         ���������    Idl �����    ����\r\n");
    }
    strcat(line,
           "--- ------- ------------ -------------- --- -------- ------------------------\r\n");
    send_to_char(line, ch);

    one_argument(argument, arg);

// ����
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
// ���������� ���������
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
// ����
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
            strcat(line, "[����������� ����]");
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

    sprintf(line, "\r\n%d ������� ����������.\r\n", num_can_see);
    send_to_char(line, ch);

    send_to_charf(ch, "�������� ������ : %8ld ����� (%ld ����).\r\n", InBytes / 1024, InBytes);
    send_to_charf(ch, "��������� ������: %8ld ����� (%ld ����).\r\n", OutBytes / 1024, OutBytes);

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
            send_to_char("��� ����� ����, ������ 0.99.30-03-06\r\n", ch);
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

    send_to_char("��� ����� �����, ��� ����� ����.\r\n", ch);
    return;

    if (!*arg) {
        send_to_char("������, ����������� � ����\r\n--------------------\r\n", ch);
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
        send_to_char("������ �������� � ���� ������ ���.\r\n", ch);
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
        sprintf(buf + strlen(buf), "�������� %s\r\n", PERS(obj->carried_by, ch, 4));
        send_to_char(buf, ch);
    } else if (obj->worn_by) {
        sprintf(buf + strlen(buf), "���� �� %s\r\n", PERS(obj->worn_by, ch, 1));
        send_to_char(buf, ch);
    } else if (obj->in_obj) {
        sprintf(buf + strlen(buf), "����� � %s%s ",
                obj->in_obj->short_description, (recur ? ", ������� ��������� " : " "));
        send_to_char(buf, ch);
        if (recur)
            print_object_location(0, obj->in_obj, ch, recur);
    } else {
        sprintf(buf + strlen(buf), "��������� ���-�� ���, ������-������.\r\n");
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
        send_to_char("������\r\n------\r\n", ch);
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
            send_to_char("��� ������ ��������.\r\n", ch);
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
        send_to_char("���� ��� ��������� ��� �������.\r\n", ch);
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
    sprintf(buf + strlen(buf), "%2d: %8d (����������������)\r\n",
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
        send_to_charf(ch, "� ������� ������ ������� ����������� ���������.\r\n");
        return;
    }

    if (!(victim = get_char_vis(ch, buf, FIND_CHAR_ROOM))) {
        send_to_char("���� �� ������ ������� ?\r\n", ch);
        return;
    }
    if (victim == ch) {
        send_to_char("��� ����? �����!\r\n", ch);
        return;
    }
    if (!IS_NPC(victim)) {
        send_to_char("��������� �� ������� �� ���� ����� � ����.\r\n", ch);
        return;
    }
    diff = (GET_LEVEL(victim) - GET_LEVEL(ch));

    if (diff <= -10)
        send_to_char("� ������� �����.\r\n", ch);
    else if (diff <= -5)
        send_to_char("������� �� �����.\r\n", ch);
    else if (diff <= -2)
        send_to_char("�����.\r\n", ch);
    else if (diff <= -1)
        send_to_char("������������ �����.\r\n", ch);
    else if (diff == 0)
        send_to_char("������ ��������.\r\n", ch);
    else if (diff <= 1)
        send_to_char("� ��� ���� ����.\r\n", ch);
    else if (diff <= 2)
        send_to_char("��� ����������� �������.\r\n", ch);
    else if (diff <= 3)
        send_to_char("������� ��������.\r\n", ch);
    else if (diff <= 5)
        send_to_char("�� ������ �� ���� ������� �����.\r\n", ch);
    else if (diff <= 10)
        send_to_char("������ ����, ��� �������� �������.\r\n", ch);
    else if (diff <= 100)
        send_to_char("�� ������ ������ � ����.\r\n", ch);

}




const char *ctypes[] = { "��������", "�������", "�������", "������", "\n"
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
        sprintf(buf, "%s %s��������%s �����.\r\n",
                ctypes[COLOR_LEV(ch)], CCIRED(ch, C_SPR), CCNRM(ch, C_OFF));
        send_to_char(CAP(buf), ch);
        return;
    }
    if (((tp = search_block(arg, ctypes, FALSE)) == -1)) {
        send_to_char("������������� �������� �����.\r\n", ch);
        send_to_char("�������������: ����� ���� <����|�������|������>\r\n", ch);
        return;
    }
    REMOVE_BIT(PRF_FLAGS(ch, PRF_COLOR_1), PRF_COLOR_1);
    REMOVE_BIT(PRF_FLAGS(ch, PRF_COLOR_2), PRF_COLOR_2);

    SET_BIT(PRF_FLAGS(ch, PRF_COLOR_1), (PRF_COLOR_1 * (tp & 1)));
    SET_BIT(PRF_FLAGS(ch, PRF_COLOR_1), (PRF_COLOR_2 * (tp & 2) >> 1));

    sprintf(buf, "%s %s��������%s �����.\r\n", ctypes[tp], CCIGRN(ch, C_SPR), CCNRM(ch, C_OFF));
    send_to_char(CAP(buf), ch);
}


ACMD(do_toggle)
{
    /*  if (IS_NPC(ch))
       return; */
    char buf2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (GET_WIMP_LEV(ch) == 0)
        strcpy(buf2, "���");
    else
        sprintf(buf2, "%-3d", GET_WIMP_LEV(ch));

    if (GET_LEVEL(ch) >= LVL_IMMORT) {
        sprintf(buf,
                "�� ���������: %-3s\r\n"
                "������ ����  : %-3s\r\n"
                "����� ������: %-3s\r\n",
                ONOFF(PRF_FLAGGED(ch, PRF_NOHASSLE)),
                ONOFF(PRF_FLAGGED(ch, PRF_HOLYLIGHT)), ONOFF(PRF_FLAGGED(ch, PRF_ROOMFLAGS))
            );
        send_to_char(buf, ch);
    }

    sprintf(buf,
            "�������            : ��������\r\n"
            "-------------------:------------------------------------------\r\n"
            "��� �����          : ������� ����� � ������ �������       %-3s\r\n"
            "��� ��������       : ������� �������� � ������ �������    %-3s\r\n"
            "��� ����           : ������� ���-�� ���������� �������    %-3s\r\n"
            "��� ����           : ���� � ������ �������                %-3s\r\n"
            "��� ������         : ���������� ������ � ������ �������   %-3s\r\n"
            "��� �����������    : ����������� � ��������� ����������   %-3s\r\n"
            "��� ���������      : ��������� ����� � ������ ���         %-3s\r\n"
            "��� ���������      : ����������������� ��������� �����    %-3s\r\n"
            "��� ������         : ���������� ������ ����� ����         %-3s\r\n"
            "��� �����          : ��������� ��������� ������ �������   %-3s\r\n"
            "��� �����          : ����� \"�����\"                        %-3s\r\n"
            "��� �������        : ����� \"�������\"                      %-3s\r\n"
            "��� ������������   : ����� \"������������\"                 %-3s\r\n"
            "��� �������        : ���������� ����������� ����          %-3s\r\n"
            "��� �������        : ������� �����                        %-3s\r\n"
            "��� ������         : ������ �����                         %-3s\r\n"
            "��� ������         : ���������� ���� ��������� �������    %-3s\r\n"
            "��� �����          : ����� ���������������                %-3s\r\n"
            "��� ����           : �������� �����                       %s\r\n"
            "��� ������������   : ������� ������������ ���������       %-3s\r\n"
            "��� �������������� : �������������� ��������������        %-3s\r\n"
            "��� ������         : ������� ������ ������                %-3d\r\n"
            "��� ������         : ������� ������ ������                %-3d\r\n"
            "��� ������         : ���������� ������ ��� ������         %-3s\r\n"
            "��� ����           : ������������ ���������� �����        %-3s\r\n"
            "��� �������        : ������������ ���������� ��������     %-3s\r\n"
            "��� ����������     : ���������� ����������� ���������     %-3s\r\n"
            "��� ����������     : ��������� ����� ��������� �� ����    %-3s\r\n"
            "��� �������������  : ��������� ����� ������ ��� ��������  %-3s\r\n"
            "��� ����������     : ����� �������� ����� ����            %-3s\r\n"
            "��� ������         : ������ ����� ������������� ���� ����.%-3s\r\n"
            "��� ��������       : ����������� ����������� � ���. ����� %-3s\r\n"
            "��� ���������      : ����� ������ �����. ������ ��������� %-3s\r\n"
			"��� ������         : ����� ����������� ������ �������     %-3s\r\n"
            "��� ���������      : ����� ���������                      %-3s\r\n", 
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

    log("���������� ������.");

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
            send_to_char("��� ���?\r\n", ch);
            return;
        }
        if (GET_LEVEL(ch) < GET_LEVEL(vict) && !GET_COMMSTATE(ch)) {
            send_to_char("�� �� ������ ������ ������� ��� ��������� ���� ��� �������.\r\n", ch);
            return;
        }
    } else
        vict = ch;

    if (subcmd == SCMD_SOCIALS)
        socials = 1;
    else if (subcmd == SCMD_WIZHELP)
        wizhelp = 1;

    sprintf(buf, "��������� %s%s �������� %s:\r\n",
            wizhelp ? "����������������� " : "",
            socials ? "�������" : "�������", vict == ch ? "���" : GET_PAD(vict, 2));

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
        send_to_charf(ch, "�� �����!\r\n");
    if (AFF_FLAGGED(ch, AFF_INVISIBLE))
        send_to_charf(ch, "���� ���� ���������.\r\n");
    if (AFF_FLAGGED(ch, AFF_DETECT_ALIGN))
        send_to_charf(ch, "�� ���������� ����� � ���.\r\n");
    if (AFF_FLAGGED(ch, AFF_DETECT_INVIS))
        send_to_charf(ch, "�� �������� ������ ���������.\r\n");
    if (AFF_FLAGGED(ch, AFF_DETECT_MAGIC))
        send_to_charf(ch, "�� ���������� �����.\r\n");
    if (AFF_FLAGGED(ch, AFF_SENSE_LIFE))
        send_to_charf(ch, "�� ���������� �����.\r\n");
    if (AFF_FLAGGED(ch, AFF_WATERWALK))
        send_to_charf(ch, "�� �������� ������ �� ����.\r\n");
    if (AFF_FLAGGED(ch, AFF_GROUP))
        send_to_charf(ch, "�� �������� � ������.\r\n");
    if (AFF_FLAGGED(ch, AFF_CURSE))
        send_to_charf(ch, "�� ���������� ���� �������.\r\n");
    if (AFF_FLAGGED(ch, AFF_INFRAVISION))
        send_to_charf(ch, "�� �������� ����������� �����.\r\n");
    if (AFF_FLAGGED(ch, AFF_DARKVISION))
        send_to_charf(ch, "���� ����� �������� ������ � �������.\r\n");
    if (AFF_FLAGGED(ch, AFF_POISON))
        send_to_charf(ch, "�� ���������� ������� � ��������������.\r\n");
    if (AFF_FLAGGED(ch, AFF_SLEEP))
        send_to_charf(ch, "�� �� �������� ����������.\r\n");
    if (AFF_FLAGGED(ch, AFF_NOTRACK))
        send_to_charf(ch, "�� �� ���������� ������.\r\n");
    if (AFF_FLAGGED(ch, AFF_TETHERED))  // ������ ��� �������
        send_to_charf(ch, "��� ���-�� ��������.\r\n");
    if (AFF_FLAGGED(ch, AFF_SNEAK) && IS_GOD(ch))       //��� ���������� �������������
        send_to_charf(ch, "�� ���������.\r\n");
    if (AFF_FLAGGED(ch, AFF_HIDE) && IS_GOD(ch))
        send_to_charf(ch, "�� ����������.\r\n");
    if (AFF_FLAGGED(ch, AFF_COURAGE))
        send_to_charf(ch, "�� � ���������.\r\n");
    if (AFF_FLAGGED(ch, AFF_CHARM))
        send_to_charf(ch, "�� ����-�� ������������.\r\n");
    if (AFF_FLAGGED(ch, AFF_HOLD))
        send_to_charf(ch, "���� ���� ������������.\r\n");
    if (AFF_FLAGGED(ch, AFF_FLY))
        send_to_charf(ch, "�� �������� ������.\r\n");
    else if (AFF_FLAGGED(ch, AFF_LEVIT))
        send_to_charf(ch, "�� �������� ������������.\r\n");
    if (AFF_FLAGGED(ch, AFF_SIELENCE))
        send_to_charf(ch, "��� ���� �� ����������� ���.\r\n");
    if (AFF_FLAGGED(ch, AFF_AWARNESS))
        send_to_charf(ch, "�� � ����������� ���������.\r\n");
//  if (AFF_FLAGGED(ch,AFF_HORSE))
//    send_to_charf(ch, "��� ��������.\r\n");
    if (AFF_FLAGGED(ch, AFF_NOFLEE))
        send_to_charf(ch, "��� �� �������.\r\n");
    /*  if (AFF_FLAGGED(ch,AFF_SINGLELIGHT)) *///��������� �����������
    /*    send_to_charf(ch, "����.\r\n"); */
    if (AFF_FLAGGED(ch, AFF_HOLYLIGHT))
        send_to_charf(ch, "���� ���� �������� ��������� ������.\r\n");
    if (AFF_FLAGGED(ch, AFF_HOLYDARK))
        send_to_charf(ch, "���� ���� �������� ������ ������� ����������� ����.\r\n");
    if (AFF_FLAGGED(ch, AFF_DETECT_POISON))
        send_to_charf(ch, "�� �������� ���������� ��.\r\n");
    if (AFF_FLAGGED(ch, AFF_DRUNKED))
        send_to_charf(ch, "�� �����.\r\n");
    if (AFF_FLAGGED(ch, AFF_HAEMORRAGIA))
        send_to_charf(ch, "���� ���� ����������.\r\n");
    if (AFF_FLAGGED(ch, AFF_BANDAGE))
        send_to_charf(ch, "���� ���� ���������� �������.\r\n");
    if (AFF_FLAGGED(ch, AFF_CAMOUFLAGE) && IS_GOD(ch))
        send_to_charf(ch, "�� ���������������.\r\n");
    if (AFF_FLAGGED(ch, AFF_WATERBREATH))
        send_to_charf(ch, "�� �������� ������ ��� �����.\r\n");
    if (AFF_FLAGGED(ch, AFF_SLOW))
        send_to_charf(ch, "���� �������� ���������.\r\n");
    if (AFF_FLAGGED(ch, AFF_FASTER))
        send_to_charf(ch, "�� ���������� ����������� �������� � �����.\r\n");
    /*  if (AFF_FLAGGED(ch,AFF_HOLYAURA))
       send_to_charf(ch, "���� ����� ������������ ���.\r\n"); */
    if (AFF_FLAGGED(ch, AFF_UNHOLYAURA))
        send_to_charf(ch, "���� ��� �������� ���.\r\n");
    if (AFF_FLAGGED(ch, AFF_DETECT_MAGIC) &&
        affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_DESECRATE) && IS_EVILS(ch))
        send_to_charf(ch, "����� �� ���������� �������� ���������.\r\n");

    if (ch->set_number != -1 && ch->set_variante != -1) {
        std::vector < struct set_variante_data >vc = *set_table[ch->set_number].variante;
        struct set_variante_data vrnt = vc[ch->set_variante];

        if (vrnt.score)
            send_to_charf(ch, "%s\r\n", vrnt.score);
    }



    if (PLR_FLAGGED(ch, PLR_MUTE) && MUTE_DURATION(ch) != 0 && MUTE_DURATION(ch) > time(NULL)
        ) {                     //int hrs  = (MUTE_DURATION(ch) - time(NULL)) / 3600;
        //int mins = ((MUTE_DURATION(ch) - time(NULL)) % 3600 + 59) / 60;
        sprintf(buf, "�� �������� � �������.\r\n");
        send_to_char(buf, ch);
    }
    if (PLR_FLAGGED(ch, PLR_DUMB) && DUMB_DURATION(ch) != 0 && DUMB_DURATION(ch) > time(NULL)
        ) {                     //int hrs  = (DUMB_DURATION(ch) - time(NULL)) / 3600;
        //int mins = ((DUMB_DURATION(ch) - time(NULL)) % 3600 + 59) / 60;
        sprintf(buf, "� ��� ����� ���.\r\n");
        send_to_char(buf, ch);
    }
    if (!IS_NPC(ch) && GET_GOD_FLAG(ch, GF_GODSCURSE) && GODS_DURATION(ch)) {   //int hrs  = (GODS_DURATION(ch) - time(NULL)) / 3600;
        //int mins = ((GODS_DURATION(ch) - time(NULL)) % 3600 + 59) / 60;
        sprintf(buf, "�� �������� ������.\r\n");

        strcat(buf, "\r\n");
        send_to_char(buf, ch);
    }
    if (ch->affected)
        for (aff = ch->affected; aff; aff = aff->next) {
            //send_to_charf(ch,"aff->type %d\r\n",aff->type);
            if (aff->main && Spl.GetItem(aff->type)->GetItem(SPL_SCORE)->GetString())
                act(Spl.GetItem(aff->type)->GetItem(SPL_SCORE)->GetString(), "!�", ch);
        }
}

void show_locates(struct char_data *ch)
{
    int dir;

    if (GET_OBJ_LOCATE(ch)) {
        CPathRetDir dirs;

        dir = TracePath(IN_ROOM(ch), IN_ROOM(GET_OBJ_LOCATE(ch)), 0, &dirs, 0);
        if (dir == -1 || dir == 0) {
            send_to_charf(ch, "���������� ���� ������ ���������� � �������.\r\n");
            GET_OBJ_LOCATE(ch) = 0;
        } else if (dir > 0) {
            send_to_charf(ch, "���������� ���� ������� %s.\r\n", DirsTo[dirs.Direction]);
            SET_BIT(ch->locate_dirs, 1 << dirs.Direction);
        } else
            send_to_charf(ch, "���������� ���� ���������� � ������.\r\n");
    } else if (GET_CHAR_LOCATE(ch)) {
        CPathRetDir dirs;

        dir = TracePath(IN_ROOM(ch), IN_ROOM(GET_CHAR_LOCATE(ch)), 0, &dirs, 0);
        if (dir == -1 || dir == 0) {
            send_to_charf(ch, "���������� ���� ������ ���������� � �������.\r\n");
            GET_CHAR_LOCATE(ch) = 0;
        } else if (dir > 0) {
            send_to_charf(ch, "���������� ���� ������� %s.\r\n", DirsTo[dirs.Direction]);
            SET_BIT(ch->locate_dirs, 1 << dirs.Direction);
        } else
            send_to_charf(ch, "���������� ���� ���������� � ������.\r\n");
    }

}

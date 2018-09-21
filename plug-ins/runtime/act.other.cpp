/* ************************************************************************
*   File: act.other.c                                   Part of CircleMUD *
*  Usage: Miscellaneous player-level commands                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include <string>
#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "ban.h"
#include "help.h"
#include "spells.h"
#include "screen.h"
#include "house.h"
#include "constants.h"
#include "events.h"
#include "pk.h"
#include "case.h"
#include "xenchant.h"
#include "xquests.h"
#include "xboot.h"
#include "planescape.h"
#include "dlfileop.h"
#include "mudfile.h"
#include "date.h"

/* extern functions */
SPECIAL(shoper);

/* local functions */
//ADD BY SLOWN
void print_group(struct char_data *ch);

ACMD(do_wimpy);
ACMD(do_gen_tog);
ACMD(do_toggle);
ACMD(do_color);
ACMD(do_set_width);
ACMD(do_set_height);

char *print_obj(struct obj_data *obj)
{
    char *diag_weapon_to_char(struct obj_data *obj, int show_wear);
    int drndice = 0, drsdice = 0, found = FALSE;
    struct material_data_list *m;
    struct C_obj_affected_type *af;
    static char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    sprintf(buf, "������� %s\r\n", CAP(obj->short_description));
    sprintf(buf + strlen(buf), "%s", diag_weapon_to_char(obj, TRUE));

    sprintf(buf + strlen(buf), "��������: ");
    if (obj->ex_description != NULL && obj->ex_description->description != NULL)
        strcpy(buf2, obj->ex_description->description);
    else
        strcpy(buf2, "������ ����������.");
    sprintf(buf + strlen(buf), "%s", strbraker(string_corrector(buf2), 80, FALSE));


    sprintf(buf + strlen(buf), "���: ");
    sprinttype(GET_OBJ_TYPE(obj), item_types, buf2);
    sprintf(buf + strlen(buf), "%s ", buf2);
    sprintf(buf + strlen(buf), "\r\n��������: ");
    if (obj->materials) {
        for (m = obj->materials; m; m = m->next) {
            sprintf(buf + strlen(buf), "%s%s (%d) ",
                    get_name_pad(get_material_param(m->number_mat)->name, PAD_IMN, PAD_OBJECT),
                    (m->main ? "+" : ""), m->value);
        }
    } else
        sprintf(buf + strlen(buf), "����������");
    strcat(buf, "\r\n");
    sprintf(buf + strlen(buf), "���: %d �����, ���������: %d %s.\r\n",
            GET_OBJ_WEIGHT(obj), GET_OBJ_COST(obj), desc_count(GET_OBJ_COST(obj), WHAT_MONEYa));

    sprintf(buf + strlen(buf), "�����������: ");
    sprintbits(obj->obj_flags.no_flag, no_bits, buf2, ",");
    sprintf(buf + strlen(buf), "%s ", buf2);
    sprintbits(obj->obj_flags.anti_flag, anti_bits, buf2, ",");
    sprintf(buf + strlen(buf), "%s.\r\n", buf2);

    found = FALSE;
    sprintf(buf + strlen(buf), "�������: ");
    for (af = obj->C_affected; af; af = af->next) {
        if (af->bitvector) {
            found = TRUE;
            sprintbit(af->bitvector, affected_bits, buf2, ",");
            sprintf(buf + strlen(buf), "%s ", buf2);
        }
    }
    sprintf(buf + strlen(buf), "%s\r\n", found ? "" : "���");

    found = FALSE;
    sprintf(buf + strlen(buf), "�������:\r\n");
    for (af = obj->C_affected; af; af = af->next) {
        if (af->location && af->modifier) {
            found = TRUE;
            sprinttype(af->location, apply_types, buf2);
            sprintf(buf + strlen(buf), "�������� %s �� %d\r\n", buf2, af->modifier);
        }
    }
    sprintf(buf + strlen(buf), "%s", found ? "" : "���\r\n");

    switch (GET_OBJ_TYPE(obj)) {
        case ITEM_ARMOR:
            drndice = GET_OBJ_VAL(obj, 0);
            drsdice = GET_OBJ_VAL(obj, 1);
            sprintf(buf + strlen(buf), "�������� �����: %d ", drndice);
            sprintf(buf + strlen(buf), "��������� �������: %d, �������: %d, �������: %d,\r\n",
                    GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 3));
            break;
        case ITEM_WEAPON:
            struct weapon_damage_data *n;

            if (obj->weapon && obj->weapon->damages) {
                sprintf(buf + strlen(buf), "�������� &G%d&n, ��������� �����������:\r\n",
                        obj->powered);
                for (n = obj->weapon->damages; n; n = n->next)
                    sprintf(buf + strlen(buf),
                            " &C%-10s&n: ����������� &G%d&n, ������������ &G%d&n, �������: &G%d&n\r\n",
                            damage_name[n->type_damage], n->min_damage, n->max_damage,
                            (n->min_damage + n->max_damage) / 2);
            }
            if (obj->spec_weapon)
                sprintf(buf + strlen(buf), "���� c���������� �����������\r\n");
            break;
        case ITEM_SCROLL:
        case ITEM_POTION:
        case ITEM_BOOK:
            sprintf(buf + strlen(buf), "�������� ����������: ");
            if (GET_OBJ_VAL(obj, 1) >= 1 && GET_OBJ_VAL(obj, 1) < MAX_SPELLS)
                sprintf(buf + strlen(buf), " %s", spell_name(GET_OBJ_VAL(obj, 1)));
            if (GET_OBJ_VAL(obj, 2) >= 1 && GET_OBJ_VAL(obj, 2) < MAX_SPELLS)
                sprintf(buf + strlen(buf), " %s", spell_name(GET_OBJ_VAL(obj, 2)));
            if (GET_OBJ_VAL(obj, 3) >= 1 && GET_OBJ_VAL(obj, 3) < MAX_SPELLS)
                sprintf(buf + strlen(buf), " %s", spell_name(GET_OBJ_VAL(obj, 3)));
            strcat(buf, ".\r\n");
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
            sprintf(buf + strlen(buf), "�������� ����������: ");
            if (GET_OBJ_VAL(obj, 3) >= 1 && GET_OBJ_VAL(obj, 3) < MAX_SPELLS)
                sprintf(buf + strlen(buf), " %s", spell_name(GET_OBJ_VAL(obj, 3)));
            sprintf(buf + strlen(buf), " ,������� %d (�������� %d).\r\n",
                    GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2));
            break;
    }

    return buf;
}

ACMD(do_output)
{
    void koi_to_win(char *str, int size);
    FILE *saved;
    struct obj_data *obj;
    char buf[MAX_STRING_LENGTH];
    int i;

    DLFileWrite file(mud->getDbDir(), "planescape.hjt");

    if (!file.open()) {
        sprintf(buf, "�� ���� ������� ���� �� (%s)", file.getCPath());
        mudlog(buf, CMP, LVL_IMPL, TRUE);
        return;
    }

    saved = file.getFP();

//�������������� ������
    strcpy(buf, "<hj-Treepad version 0.9>\r\n");
    sprintf(buf + strlen(buf), "<node>\r\n");
    sprintf(buf + strlen(buf), "Planescape\r\n");
    sprintf(buf + strlen(buf), "0\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

//����� ��������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "������\r\n");
    sprintf(buf + strlen(buf), "1\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

//��������� �� ������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_HEAD))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� �� ����
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "����\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_FACE))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� �� ���
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "���\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_EARS))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� �� ���
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "���\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_NECK))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� �� ��������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "��������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        //if (GET_OBJ_TYPE(obj) != ITEM_ARMOR) continue;
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_BODY))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� �� ����
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "����\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_ARMS))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� �� ������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "��������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_HANDS))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� �� ��������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "��������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_WRIST))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� �� ������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_FINGER))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� �� ����
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "�� ����\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_LEGS))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� �� ������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "�� ������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_FEET))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� �� ����
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "�����\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_WAIST) ||
                (GET_OBJ_TYPE(obj) == ITEM_CONTAINER || GET_OBJ_TYPE(obj) == ITEM_DRINKCON))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� ������ ����
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "�������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_ABOUT) ||
                (GET_OBJ_TYPE(obj) == ITEM_CONTAINER || GET_OBJ_TYPE(obj) == ITEM_DRINKCON))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������� ����
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "����\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (!CAN_WEAR(obj, ITEM_WEAR_SHIELD))
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//����� ������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "������\r\n");
    sprintf(buf + strlen(buf), "1\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

//������� ������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "����\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || GET_OBJ_SKILL(obj) != SKILL_SWORDS)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//�������� ������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "������� � ����\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || GET_OBJ_SKILL(obj) != SKILL_DAGGERS)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//���������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || GET_OBJ_SKILL(obj) != SKILL_MACES)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//������ ������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || GET_OBJ_SKILL(obj) != SKILL_STAFFS)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//�������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "�������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || GET_OBJ_SKILL(obj) != SKILL_FLAILS)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//�����
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "�����\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || GET_OBJ_SKILL(obj) != SKILL_SPAEKS)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "������ � ������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || GET_OBJ_SKILL(obj) != SKILL_AXES)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//�����
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "�����\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || GET_OBJ_SKILL(obj) != SKILL_WHIPS)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }


//����
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "����\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || GET_OBJ_SKILL(obj) != SKILL_BOWS)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//��������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "��������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || GET_OBJ_SKILL(obj) != SKILL_CROSSBOWS)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//�����
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "�����\r\n");
    sprintf(buf + strlen(buf), "1\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

//������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_SCROLL)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//�������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "�������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_POTION)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//�����
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "���������� �����\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_BOOK)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

//������� � ���.������
    sprintf(buf, "<node>\r\n");
    sprintf(buf + strlen(buf), "��������� �������\r\n");
    sprintf(buf + strlen(buf), "2\r\n");
    sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n\r\n");
    proc_color(buf, FALSE);
    koi_to_win(buf, strlen(buf));
    fprintf(saved, "%s", buf);

    for (i = 0; i <= top_of_objt; i++) {
        obj = &obj_proto[i];
        {
            if (GET_OBJ_TYPE(obj) != ITEM_WAND || GET_OBJ_TYPE(obj) != ITEM_STAFF)
                continue;
            sprintf(buf, "<node>\r\n");
            sprintf(buf + strlen(buf), "[%d] %s\r\n", GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0));
            sprintf(buf + strlen(buf), "3\r\n");
            sprintf(buf + strlen(buf), "%s\r\n", print_obj(obj));
            sprintf(buf + strlen(buf), "<end node> 5P9i0s8y19Z\r\n");
            proc_color(buf, FALSE);
            koi_to_win(buf, strlen(buf));
            fprintf(saved, "%s", buf);
        }
    }

    send_to_charf(ch, "�������� ���� ������ ����������� � ���� %s\r\n", file.getCPath());
}

ACMD(do_holylight)
{
    int percent;
    struct timed_type timed;
    struct affected_type af;


    if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_HOLYLIGHT)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (!IS_GOD(ch) && timed_by_skill(ch, SKILL_HOLYLIGHT)) {
        send_to_char("� ��� �� ������ �� ��� ���.\r\n", ch);
        return;
    }

    if (FIGHTING(ch)) {
        send_to_char("�� ����� ��� ��� �� �������� �����������������!\r\n", ch);
        return;
    }

    act("�� ������������� ���� ������ �� ������ �������.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n ����� ����$g ���� �����.", TRUE, ch, 0, 0, TO_ROOM);

    percent = GET_SKILL(ch, SKILL_HOLYLIGHT) / 4;
    if (percent < 1)
        percent = 1;
    if (percent > 24)
        percent = 24;
    percent = pc_duration(ch, percent, 24, 0, 0, 0);
    improove_skill(ch, 0, 0, SKILL_HOLYLIGHT);

    act("���� ����� ����������� ��������� ������� ����� �������.", FALSE, ch, 0, 0, TO_CHAR);
    act("����� $n1 �� ��� ��������� ����� ������.", TRUE, ch, 0, 0, TO_ROOM);

    af.type = find_spell_num(SPELL_HOLYLIGHT);
    af.duration = percent;
    af.location = 0;
    af.modifier = 0;
    af.main = TRUE;
    af.owner = GET_ID(ch);
    af.bitvector = AFF_DARKVISION;
    affect_join_char(ch, &af);

    timed.skill = SKILL_HOLYLIGHT;
    timed.time = 24 * SECS_PER_MUD_TICK;
    timed_to_char(ch, &timed);
}

void go_dig(struct char_data *ch)
{
    struct obj_data *obj;
    int found_item = 0;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if (!check_fight_command(ch))
        return;

    sprintf(buf, "%s", dig_messages[real_sector(ch->in_room)]);
    act(buf, TRUE, ch, 0, 0, TO_CHAR);

    if (real_sector(ch->in_room) == SECT_INSIDE ||
        real_sector(ch->in_room) == SECT_WATER_NOSWIM ||
        real_sector(ch->in_room) == SECT_FLYING ||
        real_sector(ch->in_room) == SECT_SECRET ||
        real_sector(ch->in_room) == SECT_THIN_ICE ||
        real_sector(ch->in_room) == SECT_NORMAL_ICE || real_sector(ch->in_room) == SECT_THICK_ICE)
        return;

    act("$n �����$g �������� � �����.", TRUE, ch, 0, 0, TO_ROOM);

    obj = world[IN_ROOM(ch)].contents;

    if (obj != NULL) {
        if (IS_BURIED(obj)) {
            REMOVE_BIT(GET_OBJ_EXTRA(obj, ITEM_BURIED), ITEM_BURIED);

            if (CAN_SEE_OBJ(ch, obj)) {
                found_item = 1; /* player found something */
                switch (GET_OBJ_SEX(obj)) {
                    case SEX_MALE:
                        strcpy(buf, "�� ����� ��������� ����� $o.");
                        strcpy(buf2, "$n ���$y ��������� ����� $o.");
                        break;
                    case SEX_FEMALE:
                        strcpy(buf, "�� ����� ��������� ����� $o3.");
                        strcpy(buf2, "$n ���$y ��������� ����� $o3.");
                        break;
                    case SEX_NEUTRAL:
                        strcpy(buf, "�� ����� ��������� ����� $o3.");
                        strcpy(buf2, "$n ���$y ��������� ����� $o3.");
                        break;
                    default:
                        strcpy(buf, "�� ����� ��������� ����� $o3.");
                        strcpy(buf2, "$n ���$y ��������� ����� $o3.");
                        break;
                }
                act(buf, TRUE, ch, obj, 0, TO_CHAR);
                act(buf2, TRUE, ch, obj, 0, TO_ROOM);
                obj_from_room(obj);
                obj_to_char(obj, ch);
                if (obj->carried_by == ch)
                    get_check_money(ch, obj);
            } else {
                SET_BIT(GET_OBJ_EXTRA(obj, ITEM_BURIED), ITEM_BURIED);
            }
        }
    }

    if (!found_item)
        send_to_char("�� ������ �� �����.\r\n", ch);
}

ACMD(do_set_width)
{
    char buf[MAX_STRING_LENGTH];

    int width;

    if (IS_NPC(ch) || !ch->desc)
        return;
    one_argument(argument, buf);
    if (!*buf) {
        send_to_char("��������� ������ ������.\r\n", ch);
        send_to_char("������: ����� ������ <��������>\r\n", ch);
        send_to_char(buf, ch);
        return;
    }
    width = atoi(buf);
    if (width <= 0 || width >= 120) {
        send_to_char("�� ����� ������������ �� �������� �������� ������.\r\n", ch);
        return;
    }
    if (width <= 40) {
        send_to_char("�� ��������� ����� ������� �������� ����� 40.\r\n", ch);
        return;
    }
    ch->sw = width;
    sprintf(buf, "������ ������ ������������ � %i.\r\n", ch->sw);
    send_to_char(buf, ch);
    *buf = '\0';
    return;
}

ACMD(do_set_height)
{
    char buf[MAX_STRING_LENGTH];

    int hieght;

    if (IS_NPC(ch) || !ch->desc)
        return;
    one_argument(argument, buf);
    if (!*buf) {
        send_to_char("��������� ������ ������.\r\n", ch);
        send_to_char("������: ����� ������ <��������>\r\n", ch);
        send_to_char(buf, ch);
        return;
    }
    hieght = atoi(buf);
    if (hieght <= 0 || hieght >= 60) {
        send_to_char("�� ����� ������������ �� �������� �������� ������. \r\n", ch);
        return;
    }
    if (hieght <= 10) {
        send_to_char("�� ��������� ����� ������� �������� ����� 10 \r\n", ch);
        return;
    }
    ch->sh = hieght;
    sprintf(buf, "������ ������ ������������ � %i. \r\n", ch->sh);
    send_to_char(buf, ch);
    *buf = '\0';
    return;
}

ACMD(do_quit)
{
    if (!IS_GOD(ch)) {
        send_to_charf(ch,
                      "����� �������� ����, ����������� ������� (������) � ����������� ������.\r\n");
        return;
    }

    save_pets(ch);
    xsave_rent(ch, RENT_NORMAL, TRUE);
    write_aliases(ch);
    save_vars(ch);
    save_quests(ch);
    extract_char(ch, FALSE);
}

ACMD(do_load_weapon)
{

    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    struct obj_data *missile;
    struct obj_data *weapon = NULL;

    int tmp = 0;
    int num_needed = 0;
    int num_ammo = 0;

    two_arguments(argument, arg1, arg2);

    if (!*arg1) {
        send_to_char("��� ����� ��������?\r\n", ch);
        return;
    }

    if (!*arg2) {
        send_to_char("��� ��������? �������?\r\n", ch);
        return;
    }

    if (!(weapon = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
        send_to_char("� ��� ����� ���.\r\n", ch);
        return;
    }

    if (!get_object_in_equip_vis(ch, arg1, ch->equipment, &tmp)) {
        send_to_char("����� �������� ����� ����� ������ � ����.\r\n", ch);
        return;
    }
    if (GET_OBJ_TYPE(weapon) != ITEM_FIREWEAPON) {
        send_to_char("��� ������ �� ����� ��������.\r\n", ch);
        return;
    }

    missile = get_obj_in_list_vis(ch, arg2, ch->carrying);
    if (!missile) {
        send_to_char("��� �� ������ ��������?\r\n", ch);
        return;
    }
    if (GET_OBJ_TYPE(missile) != ITEM_MISSILE) {
        send_to_char("��� �� ��������.\r\n", ch);
        return;
    }
    if (GET_OBJ_VAL(missile, 0) != GET_OBJ_VAL(weapon, 0)) {
        send_to_char("��� ��������� �� �������� � ����� ������.\r\n", ch);
        return;
    }
    num_needed = GET_OBJ_VAL(weapon, 2) - GET_OBJ_VAL(weapon, 3);
    if (!num_needed) {
        send_to_char("��� ��������.\r\n", ch);
        return;
    }
    num_ammo = GET_OBJ_VAL(missile, 3);
    if (!num_ammo) {
        /* shouldn't really get here.. this one's for Murphy:) */
        send_to_char("�����.\r\n", ch);
        extract_obj(missile);
        return;
    }
    if (num_ammo <= num_needed) {
        GET_OBJ_VAL(weapon, 3) += num_ammo;
        extract_obj(missile);
    } else {
        GET_OBJ_VAL(weapon, 3) += num_needed;
        GET_OBJ_VAL(missile, 3) -= num_needed;
    }
    act("�� �������� $p", FALSE, ch, weapon, 0, TO_CHAR);
    act("$n ������� $p", FALSE, ch, weapon, 0, TO_ROOM);
}


ACMD(do_save)
{
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch) || !ch->desc)
        return;

    /* Only tell the char we're saving if they actually typed "save" */
    if (cmd) {                  /*
                                 * This prevents item duplication by two PC's using coordinated saves
                                 * (or one PC with a house) and system crashes. Note that houses are
                                 * still automatically saved without this enabled. This code assumes
                                 * that guest immortals aren't trustworthy. If you've disabled guest
                                 * immortal advances from mortality, you may want < instead of <=.
                                 */
        if (auto_save && GET_LEVEL(ch) <= LVL_IMMORT) {
            send_to_char("��������� ��������.\r\n", ch);
            write_aliases(ch);
            return;
        }
        sprintf(buf, "��������� %s � ������.\r\n", GET_NAME(ch));
        send_to_char(buf, ch);
    }

    write_aliases(ch);
    save_char(ch, NOWHERE);
    xsave_rent(ch, RENT_NORMAL, FALSE);

//  if (ROOM_FLAGGED(ch->in_room, ROOM_HOUSE_CRASH))
//      House_crashsave(GET_ROOM_VNUM(IN_ROOM(ch)));
}


/* generic function for commands which are normally overridden by
   special procedures - i.e., shop commands, mail commands, etc. */
ACMD(do_not_here)
{
    send_to_char("��� ������� ���������� � ���� �����!\r\n", ch);
}

int awake_others(struct char_data *ch)
{
    int i;

    if (IS_NPC(ch) && !AFF_FLAGGED(ch, AFF_CHARM))
        return (FALSE);

    if (IS_GOD(ch))
        return (FALSE);

    if (AFF_FLAGGED(ch, AFF_SINGLELIGHT) || AFF_FLAGGED(ch, AFF_HOLYLIGHT))
        return (TRUE);

    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i))
            if ((GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_ARMOR &&
                 (GET_EQ(ch, i)->obj_flags.Obj_mater <= MAT_MITHRIL ||
                  GET_EQ(ch, i)->obj_flags.Obj_mater == MAT_METAL)) ||
                OBJ_FLAGGED(GET_EQ(ch, i), ITEM_HUM) || OBJ_FLAGGED(GET_EQ(ch, i), ITEM_GLOW)
                )
                return (TRUE);
    }
    return (FALSE);
}

int check_awake(struct char_data *ch, int what)
{
    int i, retval = 0;

    if (!IS_GOD(ch)) {
        if (IS_SET(what, ACHECK_LIGHT) &&
            IS_DEFAULTDARK(IN_ROOM(ch)) &&
            (AFF_FLAGGED(ch, AFF_SINGLELIGHT) || AFF_FLAGGED(ch, AFF_HOLYLIGHT)
            )
            )
            SET_BIT(retval, ACHECK_LIGHT);

        for (i = 0; i < NUM_WEARS; i++) {
            if (!GET_EQ(ch, i))
                continue;

            if (IS_SET(what, ACHECK_HUMMING) && OBJ_FLAGGED(GET_EQ(ch, i), ITEM_HUM)
                )
                SET_BIT(retval, ACHECK_HUMMING);

            if (IS_SET(what, ACHECK_GLOWING) && OBJ_FLAGGED(GET_EQ(ch, i), ITEM_GLOW)
                )
                SET_BIT(retval, ACHECK_GLOWING);

            if (IS_SET(what, ACHECK_LIGHT) &&
                IS_DEFAULTDARK(IN_ROOM(ch)) &&
                GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_LIGHT &&
                GET_LIGHT_VAL(GET_EQ(ch, i)) && GET_LIGHT_ON(GET_EQ(ch, i))
                )
                SET_BIT(retval, ACHECK_LIGHT);

        }

    }


    if (IS_SET(what, ACHECK_WEIGHT) && equip_in_metall(ch) > 20)
        SET_BIT(retval, ACHECK_WEIGHT);

    return (retval);
}

int awake_hide(struct char_data *ch)
{
    if (IS_GOD(ch))
        return (FALSE);
    return check_awake(ch,
                       ACHECK_AFFECTS | ACHECK_LIGHT | ACHECK_HUMMING | ACHECK_GLOWING |
                       ACHECK_WEIGHT);
}

int awake_invis(struct char_data *ch)
{
    if (IS_GOD(ch))
        return (FALSE);
    return check_awake(ch, ACHECK_AFFECTS | ACHECK_LIGHT | ACHECK_HUMMING | ACHECK_GLOWING);
}

int awake_camouflage(struct char_data *ch)
{
    if (IS_GOD(ch))
        return (FALSE);
    return check_awake(ch, ACHECK_AFFECTS | ACHECK_LIGHT | ACHECK_HUMMING | ACHECK_GLOWING);
}

int awake_sneak(struct char_data *ch)
{
    if (IS_GOD(ch))
        return (FALSE);
    return check_awake(ch,
                       ACHECK_AFFECTS | ACHECK_LIGHT | ACHECK_HUMMING | ACHECK_GLOWING |
                       ACHECK_WEIGHT);
}

int awaking(struct char_data *ch, int mode)
{
    if (IS_GOD(ch))
        return (FALSE);
    if (IS_SET(mode, AW_HIDE) && awake_hide(ch))
        return (TRUE);
    if (IS_SET(mode, AW_INVIS) && awake_invis(ch))
        return (TRUE);
    if (IS_SET(mode, AW_CAMOUFLAGE) && awake_camouflage(ch))
        return (TRUE);
    if (IS_SET(mode, AW_SNEAK) && awake_sneak(ch))
        return (TRUE);
    return (FALSE);
}

int char_humming(struct char_data *ch)
{
    int i;

    if (IS_NPC(ch) && !AFF_FLAGGED(ch, AFF_CHARM))
        return (FALSE);

    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i) && OBJ_FLAGGED(GET_EQ(ch, i), ITEM_HUM))
            return (TRUE);
    }
    return (FALSE);
}

int char_glowing(struct char_data *ch)
{
    int i;

    if (IS_NPC(ch) && !AFF_FLAGGED(ch, AFF_CHARM))
        return (FALSE);

    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i) && OBJ_FLAGGED(GET_EQ(ch, i), ITEM_GLOW))
            return (TRUE);
    }
    return (FALSE);
}

ACMD(do_camouflage)
{
    struct affected_type af;
    struct timed_type timed;
    ubyte prob, percent;

    if (!check_fight_command(ch))
        return;

    if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_CAMOUFLAGE)) {
        send_to_char("�� �� �� ������ ���.\r\n", ch);
        return;
    }

    if (on_horse(ch)) {
        send_to_char("��� ����� ���������� ���������.\r\n", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_BLADE_BARRIER)) {
        send_to_charf(ch, "������ �� ������ ������ ��� ���������������.\r\n");
        return;
    }

    if (timed_by_skill(ch, SKILL_CAMOUFLAGE)) {
        send_to_char("� ��� �� ������ ���.\r\n", ch);
        return;
    }

    if (IS_IMMORTAL(ch))
        affect_from_char(ch, SPELL_CAMOUFLAGE);

    if (affected_by_spell(ch, SPELL_CAMOUFLAGE)) {
        send_to_char("�� ��� ������������.\r\n", ch);
        return;
    }

    send_to_char("�� ������ �������������.\r\n", ch);
    REMOVE_BIT(EXTRA_FLAGS(ch, EXTRA_FAILCAMOUFLAGE), EXTRA_FAILCAMOUFLAGE);
    percent = number(1, 100);
    std::vector < int >vit;
    std::vector < int >vot;

    //��������� ��� �����
    vit.push_back(GET_REAL_DEX(ch));
    vit.push_back(GET_REAL_WIS(ch));
    //��������� ��� ������
    vot.push_back(number(1, 40));
    prob = calc_like_skill(ch, NULL, SKILL_CAMOUFLAGE, vit, vot);

    af.type = find_spell_num(SPELL_CAMOUFLAGE);
    af.duration = GET_SKILL(ch, SKILL_CAMOUFLAGE) * 3;
    af.modifier = world[IN_ROOM(ch)].zone;
    af.location = APPLY_NONE;
    af.battleflag = 0;

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);
    if (percent > prob)
        af.bitvector = 0;
    else
        af.bitvector = AFF_CAMOUFLAGE;
    affect_join_char(ch, &af);
    if (!IS_IMMORTAL(ch)) {
        timed.skill = SKILL_CAMOUFLAGE;
        timed.time = 2 * SECS_PER_MUD_TICK;
        timed_to_char(ch, &timed);
    }
}

ACMD(do_hide)
{
    struct affected_type af;
    struct char_data *tch;

    if (!check_fight_command(ch))
        return;

    if (!GET_SKILL(ch, SKILL_HIDE)) {
        send_to_char("�� �� �� ������ ���.\r\n", ch);
        return;
    }

    if (on_horse(ch)) {
        send_to_char("��� ����� ���������� ���������.\r\n", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_BLADE_BARRIER)) {
        send_to_charf(ch, "������ �� ������ ������ ��� ����������.\r\n");
        return;
    }


    affect_from_char(ch, SPELL_HIDE);

    if (affected_by_spell(ch, SPELL_HIDE)) {
        send_to_char("�� ��� ��������� ����������.\r\n", ch);
        return;
    }

    if (AFF_FLAGGED(ch, AFF_BLIND)) {
        send_to_char("�� ����� � �� ������ ���� ���������.\r\n", ch);
        return;
    }


    send_to_char("�� ����������� ����������.\r\n", ch);
    REMOVE_BIT(EXTRA_FLAGS(ch, EXTRA_FAILHIDE), EXTRA_FAILHIDE);

    af.type = find_spell_num(SPELL_HIDE);
    af.duration = GET_SKILL(ch, SKILL_HIDE) * 25;
    af.modifier = GET_SKILL(ch, SKILL_HIDE);
    af.location = APPLY_NONE;
    af.main = TRUE;
    af.owner = GET_ID(ch);
    af.battleflag = 0;
    af.bitvector = AFF_HIDE;
    affect_join_char(ch, &af);

    //��������� ��������� ��� ��� ����� � ���� ������ � �������
    for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
        if (tch == ch || !CAN_SEE(tch, ch))
            continue;
        add_victim_visible(tch, ch);
    }


}

void do_gold_steal(struct char_data *ch, struct char_data *vict)
{
    struct timed_type timed;
    char buf[MAX_STRING_LENGTH];

    int gold;

    if (!GET_GOLD(vict)) {
        act("�� ������ �� ������.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    } else {
        gold = (int) (GET_GOLD(vict) * number(1, GET_SKILL(ch, SKILL_STEAL)) / 100);

        gold = MIN(gold, GET_GOLD(vict));

        if (gold <= 0)
            gold = 1;

        GET_GOLD(ch) += gold;
        GET_GOLD(vict) -= gold;
        if (gold > 1) {
            sprintf(buf, "�� ������ %d %s.\r\n", gold, desc_count(gold, WHAT_MONEYu));
            send_to_char(buf, ch);
        } else
            send_to_char("�� ������ ���� ������.\r\n", ch);

        timed.skill = SKILL_STEAL;
        timed.time  = SECS_PER_MUD_TICK / 12;
        timed_to_char(ch, &timed);

    }
}

void go_steal(struct char_data *ch, struct char_data *vict, char *obj_name)
{
    struct timed_type timed;
    int percent, prob, is_ok = 0, eq_pos;
    struct obj_data *obj;

    if (!vict)
        return;

    if (!IS_GOD(ch) && FIGHTING(vict)) {
        act("$N ������� ������ ������������.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }

    if (!IS_GOD(ch) && ROOM_FLAGGED(IN_ROOM(vict), ROOM_ARENA)) {
        send_to_char("��������� ��� �������� �����������.\r\n", ch);
        return;
    }

    if (timed_by_skill(ch, SKILL_STEAL)) {
        send_to_char("� ��� �� ������ ���.\r\n", ch);
        return;
    }

    if (str_cmp(obj_name, "coins") && str_cmp(obj_name, "gold") && str_cmp(obj_name, "������") && str_cmp(obj_name, "�����") && str_cmp(obj_name, "������")) {  // ����� �����
        if (!(obj = get_obj_in_list_vis(ch, obj_name, vict->carrying))) {
            for (eq_pos = 0; eq_pos < NUM_WEARS; eq_pos++)
                if (GET_EQ(vict, eq_pos) &&
                    (isname(obj_name, GET_EQ(vict, eq_pos)->name)) &&
                    CAN_SEE_OBJ(ch, GET_EQ(vict, eq_pos))) {
                    obj = GET_EQ(vict, eq_pos);
                    break;
                }
            if (!obj) {
                act("� �$S ����� ���.", FALSE, ch, 0, vict, TO_CHAR);
                return;
            } else {            /* It is equipment */
                //��������� �� ���� ���� � �� �������
                struct mess_p_data *k;
                bool found = FALSE;
                int count;

                for (k = obj->mess_data; k; k = k->next, count++)
                    if (k->command == CMD_REMOVE && k->stoping)
                        found = TRUE;
                if (found) {
                    is_ok = 0;
                } else {
                    percent = number(1, 100);
                    std::vector < int >vit;
                    std::vector < int >vot;

                    //��������� ��� �����
                    vit.push_back(GET_REAL_DEX(ch));
                    vit.push_back(GET_REAL_INT(ch));
                    //��������� ��� ������
                    vot.push_back(GET_REAL_DEX(vict));
                    vot.push_back(GET_REAL_WIS(vict));
                    prob = calc_like_skill(ch, vict, SKILL_STEAL, vit, vot);
                    improove_skill(ch, vict, 0, SKILL_STEAL);

                    if (percent <= 5)
                        prob = 100;
                    else if (percent >= 95)
                        prob = 0;

                    if (GET_POS(vict) > POS_SLEEPING)
                        prob /= 100;    // ���� ������ �� ����

                    if (PRF_FLAGGED(ch, PRF_CODERINFO))
                        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);
                    if (prob > 0 && prob >= percent) {
                        if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
                            send_to_char("�� �� ������� ������ ������� ���������.\r\n", ch);
                            return;
                        } else if (IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj) > CAN_CARRY_W(ch)) {
                            send_to_char("�� �� ������� ������ ����� ���.\r\n", ch);
                            return;
                        } else {
                            act("�� ������� $N3 � ����� $o3.", FALSE, ch, obj, vict, TO_CHAR);
                            //act("$n �����$g $o3 � $N1.", FALSE, ch, obj, vict, TO_NOTVICT);
                            obj_to_char(unequip_char(vict, eq_pos), ch);
                            is_ok = 1;
                            check_sets(vict);
                            affect_total(vict);
                            timed.skill = SKILL_STEAL;
                            timed.time  = SECS_PER_MUD_TICK / 12;
                            timed_to_char(ch, &timed);
                        }
                    }
                }
            }
        } else {                /* obj found in inventory */
            //��������� �� ���� ���� � �� �������
            struct mess_p_data *k;
            bool found = FALSE;
            int count;

            for (k = obj->mess_data; k; k = k->next, count++)
                if (k->command == CMD_REMOVE && k->stoping)
                    found = TRUE;

            if (found) {
                is_ok = 0;
            } else {
                percent = number(1, 100);
                std::vector < int >vit;
                std::vector < int >vot;

                vit.push_back(GET_REAL_DEX(ch));
                vit.push_back(GET_REAL_INT(ch));
                vot.push_back(GET_REAL_DEX(vict));
                vot.push_back(GET_REAL_WIS(vict));
                prob = calc_like_skill(ch, vict, SKILL_STEAL, vit, vot);
                improove_skill(ch, vict, 0, SKILL_STEAL);

                if (!CAN_SEE(vict, ch))
                    prob *= 2;

                prob -= (GET_OBJ_WEIGHT(obj) / 1000);   /* Make heavy harder */

                if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NARROW))
                    prob = (prob * 15) / 10;

                if (percent <= 5)
                    prob = 100;
                else if (percent >= 95)
                    prob = 0;

                if (PRF_FLAGGED(ch, PRF_CODERINFO))
                    send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);
                if (prob > 0 && prob >= percent) {
                    if (IS_CARRYING_N(ch) + 1 < CAN_CARRY_N(ch)) {
                        if (IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj) < CAN_CARRY_W(ch)) {
                            obj_from_char(obj);
                            obj_to_char(obj, ch);
                            act("�� ������ $o3 � $N1!", FALSE, ch, obj, vict, TO_CHAR);
                            is_ok = 1;
                            timed.skill = SKILL_STEAL;
                            timed.time  = SECS_PER_MUD_TICK / 12;
                            timed_to_char(ch, &timed);
                        }
                    } else {
                        send_to_char("�� �� ������ ������� �����.\r\n", ch);
                    }
                }
            }
        }
    } else {                    //����� �����
        if (GET_GOLD(vict) && !IS_DARKTHIS(IN_ROOM(vict))) {
            percent = number(1, 100);
            std::vector < int >vit;
            std::vector < int >vot;

            vit.push_back(GET_REAL_DEX(ch));
            vit.push_back(GET_REAL_INT(ch));
            vot.push_back(GET_REAL_DEX(vict));
            vot.push_back(GET_REAL_WIS(vict));
            prob = calc_like_skill(ch, vict, SKILL_STEAL, vit, vot);
            improove_skill(ch, vict, 0, SKILL_STEAL);

            if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NARROW))
                prob = (prob * 15) / 10;

            if (!CAN_SEE(vict, ch))
                prob *= 2;

            if (percent <= 5)
                prob = 100;
            else if (percent >= 95)
                prob = 0;

            if (PRF_FLAGGED(ch, PRF_CODERINFO))
                send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);
            if (prob > 0 && prob >= percent) {
                do_gold_steal(ch, vict);
                is_ok = 1;
            }
        }
    }
    if (!is_ok) {
        send_to_char("�� �� ������ �������! \r\n", ch);
        timed.skill = SKILL_STEAL;
        timed.time  = SECS_PER_MUD_TICK / 3;
        timed_to_char(ch, &timed);
        if (GET_POS(vict) == POS_SLEEPING && !IS_AFFECTED(vict, AFF_SLEEP)
            && !IS_AFFECTED(vict, AFF_MEDITATION)) {
            act("����� ������������ ��������� �� ��������� $N1.", FALSE, ch, 0, vict, TO_CHAR);
            act("������ ��� �� ������������� ��� $n �������� ��� ���������!", FALSE, ch, 0, vict,
                TO_VICT | TO_SLEEP);
            act("$n �����$u ������� ���-�� � $N1.", TRUE, ch, 0, vict, TO_NOTVICT);
            act("$N1 ��������� � ���.", TRUE, ch, 0, vict, TO_NOTVICT);
            GET_POS(vict) = POS_SITTING;
            inc_pk_thiefs(ch, vict);
            appear(ch);
            if (IS_NPC(vict) && (dice(1, 20) > GET_REAL_LCK(ch)) && CAN_SEE(vict, ch))
                _damage(vict, ch, WEAP_RIGHT, 0, TRUE, C_POWER);
        } else {
            act("$n �����$u ��������� ���!", FALSE, ch, 0, vict, TO_VICT);
            act("$n �����$u ������� � $N1.", TRUE, ch, 0, vict, TO_NOTVICT);
            inc_pk_thiefs(ch, vict);
            appear(ch);
            if (IS_NPC(vict) && (dice(1, 20) > GET_REAL_LCK(ch)) && CAN_SEE(vict, ch) &&
                AWAKE(vict) && MAY_ATTACK(vict)) {
                check_position(vict);
                _damage(vict, ch, WEAP_RIGHT, 0, TRUE, C_POWER);
            }
        }
    }
}

/*   */

ACMD(do_steal)
{
    struct char_data *vict;
    char vict_name[MAX_INPUT_LENGTH], obj_name[MAX_INPUT_LENGTH];


    if (!check_fight_command(ch))
        return;

    if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_STEAL)) {
        send_to_char("�� �� �� ������ ���.\r\n", ch);
        return;
    }

    if (on_horse(ch)) {
        send_to_char("������ ��� ������� ��������������.\r\n", ch);
        return;
    }

    two_arguments(argument, obj_name, vict_name);

    if (!(vict = get_char_vis(ch, vict_name, FIND_CHAR_ROOM))) {
        send_to_char("������� � ���� ?\r\n", ch);
        return;
    } else if (vict == ch) {
        send_to_char("�� ������ ������ ���� � ������ ����.\r\n", ch);
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (IS_SHOPKEEPER(vict)) {
        send_to_charf(ch, "��������� � ��������� ���� �� �����������.\r\n");
        return;
    }

    if (!ok_damage_shopkeeper(ch, vict)) {
        return;
    }

    go_steal(ch, vict, obj_name);
}

ACMD(do_skills)
{
    char bufwp[8912], bufw[8912], buft[8912], bufr[8912], bufm[8912], bufp[8912], bufc[8912],
        bufo[8912];
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "&c�� ������ ��������� ������:\r\n");

    skip_spaces(&argument);

    if (IS_NPC(ch))
        return;

    strcpy(bufwp, list_skills(ch, SKILL_TWEAPON));
    strcpy(bufw, list_skills(ch, SKILL_TWARRIOR));
    strcpy(buft, list_skills(ch, SKILL_TTHIEF));
    strcpy(bufr, list_skills(ch, SKILL_TRANGER));
    strcpy(bufm, list_skills(ch, SKILL_TMAGIC));
    strcpy(bufp, list_skills(ch, SKILL_TPRIEST));
    strcpy(bufc, list_skills(ch, SKILL_TRACE));
    strcpy(bufo, list_skills(ch, SKILL_TOTHER));

    if (*bufwp) {
        sprintf(buf + strlen(buf), "%24s", "&g��������� ������:&n\r\n");
        strcat(buf, bufwp);
    }

    if (*bufw) {
        sprintf(buf + strlen(buf), "%24s", "&g������ �����:&n\r\n");
        strcat(buf, bufw);
    }

    if (*buft) {
        sprintf(buf + strlen(buf), "%24s", "&g������ �����:&n\r\n");
        strcat(buf, buft);
    }

    if (*bufr) {
        sprintf(buf + strlen(buf), "%24s", "&g������ ��������:&n\r\n");
        strcat(buf, bufr);
    }
    if (*bufm) {
        sprintf(buf + strlen(buf), "%24s", "&g������ ����:&n\r\n");
        strcat(buf, bufm);
    }
    if (*bufp) {
        sprintf(buf + strlen(buf), "%24s", "&g������ �����:&n\r\n");
        strcat(buf, bufp);
    }
    if (*bufc) {
        sprintf(buf + strlen(buf), "%24s", "&g������� ������:&n\r\n");
        strcat(buf, bufc);
    }
    if (*bufo) {
        sprintf(buf + strlen(buf), "%24s", "&g������ ������:&n\r\n");
        strcat(buf, bufo);
    }

    if (!*buf)
        strcpy(buf, "� ��� ��� �� ������ ������.\r\n");

    //send_to_charf(ch,buf);

    page_string(ch->desc, buf, 1);
}

ACMD(do_spells)
{
    if (IS_MAGIC_USER(ch))
        list_spells(ch, ch, CLASS_MAGIC_USER);

    if (IS_NECRO(ch))
        list_spells(ch, ch, CLASS_NECRO);

}

ACMD(do_prays)
{
    list_prays(ch, ch);
}

ACMD(do_list_enchant)
{
    int enchnum, numadd, i;
    char *s;
    char buf[MAX_STRING_LENGTH];

    s = strtok(argument, ch->divd);
    if (s == NULL) {
        list_enchants(ch, ch);
        return;
    }

    s = strtok(NULL, ch->divd);

    if (s == NULL) {
        send_to_charf(ch, "�������� ������� ������ ���� ��������� � �������: &C'%s'&n.\r\n",
                      ch->divd);
        return;
    }

    enchnum = find_enchant_num(s);

    if (enchnum < 0) {
        send_to_charf(ch, "�� �� ������ ������ ������� �����������.\r\n");
        return;
    }

    if (!IS_SET(GET_ENCHANT_TYPE(ch, ENCHANT_NO(enchnum)), SPELL_TEMP | SPELL_KNOW) && !IS_GOD(ch)) {
        send_to_charf(ch, "�� �� ������ ������ ������� �����������.\r\n");
        return;
    }

    sprintf(buf, "������: &C%s&n\r\n", ENCHANT_NAME(enchnum));
    sprintf(buf + strlen(buf), "%s",
            strbraker(ENCHANT_DESC(enchnum), ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)));

    numadd = xEnchant.GetItem(enchnum)->GetItem(ECH_INCLUDE)->GetNumberItem();
    sprintf(buf + strlen(buf), "����������:\r\n");
    for (i = 0; i < numadd; i++) {
        CItem *include = xEnchant.GetItem(enchnum)->GetItem(ECH_INCLUDE)->GetItem(i);
        int rnum = real_object(include->GetItem(ECH_INC_TYPE)->GetInt());

        if (rnum == -1) {
            sprintf(buf + strlen(buf), " &R������� %d �� ������ � ���� ������!&n\r\n",
                    include->GetItem(ECH_INC_TYPE)->GetInt());
            continue;
        }
        sprintf(buf + strlen(buf), " &C%s&n ���������� &G%d&n\r\n",
                obj_proto[rnum].short_description, include->GetItem(ECH_INC_COUNT)->GetInt());
    }

    send_to_charf(ch, buf);
}

ACMD(do_visible)
{
    if (IS_IMMORTAL(ch)) {
        perform_immort_vis(ch);
        return;
    }

    if (AFF_FLAGGED(ch, AFF_INVISIBLE) || AFF_FLAGGED(ch, AFF_CAMOUFLAGE)) {
        appear(ch);
        send_to_char("�� ����� ������� ����������.\r\n", ch);
    } else
        send_to_char("�� � ��� ������.\r\n", ch);
}

ACMD(do_courage)
{
    struct obj_data *obj;
    struct affected_type af[2];
    struct timed_type timed;
    char buf[MAX_STRING_LENGTH];
    int prob;

    if (IS_NPC(ch))             /* Cannot use GET_COND() on mobs. */
        return;

    if (!GET_SKILL(ch, SKILL_COURAGE)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (timed_by_skill(ch, SKILL_COURAGE)) {
        send_to_char("�� �� ������ ������� ����� ������� � ���������.\r\n", ch);
        return;
    }

    timed.skill = SKILL_COURAGE;
    timed.time = 6 * SECS_PER_MUD_TICK;
    timed_to_char(ch, &timed);

    improove_skill(ch, 0, 0, SKILL_COURAGE);
    af[0].type = find_spell_num(SPELL_COURAGE);
    af[0].duration = ((GET_SKILL(ch, SKILL_COURAGE) / 20) + 1) * SECS_PER_MUD_TICK;
    af[0].modifier = (GET_SKILL(ch, SKILL_COURAGE) / 10) + 1;
    af[0].location = APPLY_DAMROLL;
    af[0].bitvector = AFF_NOFLEE;
    af[0].battleflag = 0;
    af[0].main = TRUE;
    af[1].type = find_spell_num(SPELL_COURAGE);
    af[1].duration = af[0].duration;
    af[1].modifier = -20;
    af[1].location = APPLY_AC;
    af[1].bitvector = AFF_NOFLEE;
    af[1].battleflag = 0;
    af[1].main = FALSE;

    for (prob = 0; prob < 2; prob++)
        affect_join_char(ch, &af[prob]);

    send_to_char("�� ������ � ���������.\r\n", ch);
    if ((obj = GET_EQ(ch, WEAR_WIELD)) || (obj = GET_EQ(ch, WEAR_BOTHS)))
        strcpy(buf, "����� $n1 �������� ������ � $e ������� ����$g � ����� $o3.");
    else
        strcpy(buf, "����� $n1 �������� ������.");
    act(buf, FALSE, ch, obj, 0, TO_ROOM);
}



ACMD(do_title)
{
    skip_spaces(&argument);
    delete_doubledollar(argument);
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        send_to_char("� ����� ��� �����?\r\n", ch);
    else if (GET_LEVEL(ch) < LVL_HICHAR && GET_REMORT(ch) == 0) {
        sprintf(buf, "����� ����� ��������� ������ ����� %d ������.\r\n", LVL_HICHAR);
        send_to_char(buf, ch);
    } else if (PLR_FLAGGED(ch, PLR_NOTITLE))
        send_to_char("��� ������� ������ ��� ���!\r\n", ch);
    else if (strstr(argument, "(") || strstr(argument, ")"))
        send_to_char("����� �� ����� ��������� �������� ( ��� ).\r\n", ch);
    else if (strlen(argument) > MAX_TITLE_LENGTH) {
        sprintf(buf, "��������, ����� ������ ���� �� ������ %d ��������.\r\n", MAX_TITLE_LENGTH);
        send_to_char(buf, ch);
    } else if (!*argument) {
        set_title(ch, argument);
        send_to_char("�� ���������� �� ����� �������.\r\n", ch);
        return;
    } else if (GET_LEVEL(ch) >= LVL_IMMORT) {
        set_title(ch, argument);
        sprintf(buf, "������. ��� ������ ����� \"%s\".\r\n", race_or_title(ch, 0));
        send_to_char(buf, ch);
    } else {
        sprintf(buf, "%s ����������%s ����� ����� '%s'", GET_NAME(ch), GET_CH_SUF_1(ch), argument);
        mudlog(buf, CMP, LVL_GOD, TRUE);
        set_rtitle(ch, argument);
    }
}


int low_charm(struct char_data *ch)
{
    struct affected_type *aff;

    for (aff = ch->affected; aff; aff = aff->next)
        if ((aff->type == SPELL_CHARM || aff->type == SKILL_CHARM) && aff->duration <= 60)
            return (TRUE);
    return (FALSE);
}

void print_one_line(struct char_data *ch, struct char_data *k, int leader, int header)
{
    char buf[MAX_STRING_LENGTH];

    const char *WORD_STATE[] = {
        "�������� ������",      //0
        "������ �����",         //1
        "�����",                //2
        "����� �����",          //3
        "���������",            //4
        "������"                //5
    };
    const char *WORD_STATE_ME[] = {
        "��������� ������",     //0
        "������ ������",        //1
        "������",               //2
        "����� ������",         //3
        "����������",           //4
        "�������"               //5
    };
    const char *MOVE_STATE[] = {
        "��� ���",              //0
        "����� �����",          //1
        "�����",                //2
        "������ �����",         //3
        "���������",            //4
        "����"                  //5
    };
    const char *MOVE_STATE_ME[] = {
        "��� ���",              //0
        "����� ������",         //1
        "������",               //2
        "������ ������",        //3
        "���������",            //4
        "�����"                 //5
    };

    const char *POS_STATE[] = { "����",
        "��� ������",
        "�������",
        "�.��������",
        "����",
        "��������",
        "�����",
        "���������",
        "�����",
        "������"
    };

    const char *POS_STATE_ME[] = { "������",
        "��� ������",
        "��������",
        "�.��������",
        "�����",
        "���������",
        "������",
        "����������",
        "������",
        "�������"
    };

    sprintf(buf, "%s%-20s: %-15s %-12s %-12s %s%s%s",
            leader ? "&Y" : "",
            (ch == k) ? "��" : CAP(GET_NAME(k)),
            (ch ==
             k) ? WORD_STATE_ME[posi_value5(GET_HIT(k),
                                            GET_REAL_MAX_HIT(k))] :
            WORD_STATE[posi_value5(GET_HIT(k), GET_REAL_MAX_HIT(k))],
            (ch ==
             k) ? MOVE_STATE_ME[posi_value5(GET_MOVE(k),
                                            GET_REAL_MAX_MOVE(k))] :
            MOVE_STATE[posi_value5(GET_MOVE(k), GET_REAL_MAX_MOVE(k))],
            (ch == k) ? POS_STATE_ME[(int) GET_POS(k)] : POS_STATE[(int) GET_POS(k)],
            IN_ROOM(ch) == IN_ROOM(k) ? "*" : " ", (AFF_FLAGGED(k, AFF_SINGLELIGHT)
                                                    || AFF_FLAGGED(k, AFF_HOLYLIGHT)
                                                    || (GET_EQ(k, WEAR_LIGHT)
                                                        && GET_OBJ_VAL(GET_EQ(k, WEAR_LIGHT),
                                                                       2))) ? " �" : " ",
            leader ? "&n" : "");

    /*sprintf(buf,"%-20s: %s , %s , %s %s%s %s%s%s%s\r\n",
       (ch==k) ? "��" : CAP(GET_NAME(k)),
       WORD_STATE[posi_value5(GET_HIT(k),GET_REAL_MAX_HIT(k))+1],
       MOVE_STATE[posi_value5(GET_MOVE(k),GET_REAL_MAX_MOVE(k))+1],
       POS_STATE[(int) GET_POS(k)],
       IN_ROOM(ch) == IN_ROOM(k) ? "�����" : "",
       leader ? ", ����� :":":",
       (AFF_FLAGGED(k,AFF_FLY) && GET_POS(k) == POS_FLYING) ? " ������" : "",
       AFF_FLAGGED(k,AFF_INVISIBLE) ? " �������" : "",
       AFF_FLAGGED(k,AFF_HOLD) ? " �������" : "",
       (AFF_FLAGGED(k,AFF_SINGLELIGHT)     ||
       AFF_FLAGGED(k,AFF_HOLYLIGHT)   ||
       (GET_EQ(k,WEAR_LIGHT) && GET_OBJ_VAL(GET_EQ(k,WEAR_LIGHT),2))) ? " ����" : ""
       ); */
    send_to_charf(ch, "%s\r\n", buf);
//     act(buf,FALSE,ch,0,k,TO_CHAR);
}


void print_group(struct char_data *ch)
{
    int gfound = 0, cfound = 0;
    struct char_data *k;
    struct follow_type *f;

    if (AFF_FLAGGED(ch, AFF_GROUP)) {
        send_to_charf(ch, "%-20s: %-15s %-12s %-12s %s\r\n",
                      "���� ������:", "�����", "��������", "���������", "�������������");

        k = (ch->party_leader ? ch->party_leader : ch);
        //���������� ������
        if (AFF_FLAGGED(k, AFF_GROUP) && ch != k)
            print_one_line(ch, k, TRUE, gfound++);
        //���������� ����
        print_one_line(ch, ch, FALSE, gfound++);
        for (f = k->party; f; f = f->next) {
            if (!AFF_FLAGGED(f->follower, AFF_GROUP) || ch == f->follower)
                continue;
            print_one_line(ch, f->follower, FALSE, gfound++);
        }
    }
    for (f = ch->followers; f; f = f->next) {
        if (!AFF_FLAGGED(f->follower, AFF_CHARM))
            continue;
        if (!cfound)
            send_to_charf(ch, "%-20s: %-15s %-12s %-12s %s\r\n",
                          "���� �������������:", "�����", "��������", "���������", "�������������");
        print_one_line(ch, f->follower, FALSE, cfound++);
    }
    if (!gfound && !cfound)
        send_to_char("�� �� �������� � ������.\r\n", ch);
}

ACMD(do_group)
{
    struct char_data *victim = NULL;
    struct follow_type *f;
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    int len;

    *arg1 = '\0';
    *arg2 = '\0';
    argument = two_arguments(argument, arg1, arg2);

    if (!*arg1) {
        print_group(ch);
        return;
    }

    len = strlen(arg1);
    if ((strncmp(arg1, "��������", len) && strncmp(arg1, "��������", len) &&
         strncmp(arg1, "����������", len)) && !*arg2) {
        send_to_charf(ch, "������ �������: ������ (��������) [����]\r\n");
        return;
    }

    victim = get_char_vis(ch, arg2, FIND_CHAR_WORLD);
    if (!victim) {
        send_to_charf(ch, "�� �� ������ ������ � ����� ������.\r\n");
        return;
    }

    if (!strncmp("����������", arg1, len)) {
        if (ch->party_leader) {
            send_to_charf(ch, "������������� � ������ ���������� �����.\r\n");
            return;
        }
        if (IS_NPC(victim)) {
            send_to_charf(ch, "���������� ����� ������ �������.\r\n");
            return;
        }
        act("�� ��������� ����������� 2� �������� � ���� ������.", "��", ch, victim);
        act("1� ��������1(,�,�,�) ����������� �������� � 1�� ������.", "��", ch, victim);
        victim->invite_char = ch;
        victim->invite_time = time(0);
    } else if (!strncmp("��������", arg1, len)) {
        if (ch->invite_char == NULL) {
            send_to_charf(ch, "��� ����� �� ��������� � ������.\r\n");
            return;
        }
        join_party(ch->invite_char, ch);
        act("�� ������� ����������� 2�.", "��", ch, ch->invite_char);
        act("1� ������1(,�,�,�) ���� �����������.", "��", ch, ch->invite_char);
        ch->invite_char = NULL;
        ch->invite_time = 0;
    } else if (!strncmp("��������", arg1, len)) {
        if (ch->party) {
            send_to_charf(ch, "������ ��� �������� ������, �� ������ �������� ���������.\r\n");
            return;
        } else {
            struct char_data *l = ch->party_leader;

            if (!l) {
                send_to_charf(ch, "�� �� �������� � ������.\r\n");
                return;
            }
            act("�� �������� ������ 2�.", "��", ch, l);
            leave_party(ch);
        }
    } else if (!strncmp("�����", arg1, len)) {
        if (!ch->party) {
            send_to_charf(ch, "�� �� ����� ������.\r\n");
            return;
        }
        change_leader(ch, victim, TRUE);
    } else if (!strncmp("���������", arg1, len)) {
        if (!ch->party) {
            send_to_charf(ch, "�� �� ����� ������.\r\n");
            return;
        }
        if (victim->party_leader != ch) {
            act("2� �� ������� � ����� ������.", "��", ch, victim);
            return;
        }
        act("�� ��������� �� ����� ������ 2�.", "��", ch, victim);
        act("1� ��������1(,�,�,�) ��� �� ����� ������.", "��", ch, victim);
        leave_party(victim);
        if (!ch->party) {
            send_to_charf(ch, "���� ������ ��������������.\r\n");
            leave_party(ch);
            return;
        }

        for (f = ch->party; f; f = f->next)
            act("1� ��������1(,�,�,�) �� ������ 2�.", "���", ch, victim, f->follower);
    } else if (!strncmp("����������", arg1, len)) {
        if (!ch->party) {
            send_to_charf(ch, "����� ������� ����� ������� ������ ����� ������.\r\n");
            return;
        }
        send_to_charf(ch, "�� �������������� ������.\r\n");
        die_party(ch);
    } else
        send_to_charf(ch, "������: ����������� �������.\r\n");
}

ACMD(do_report)
{
    struct char_data *k;
    struct follow_type *f;
    char buf[MAX_STRING_LENGTH];

    if (!AFF_FLAGGED(ch, AFF_GROUP) && !AFF_FLAGGED(ch, AFF_CHARM)
        ) {
        send_to_char("��� ������ ����������� ���� ���������!\r\n", ch);
        return;
    }
    if (IS_NPC(ch)) {
        sprintf(buf, "%s �������%s: %d �� %d �����, %d �� %d ��������\r\n",
                GET_NAME(ch), GET_CH_SUF_1(ch),
                GET_HIT(ch), GET_REAL_MAX_HIT(ch), GET_MOVE(ch), GET_REAL_MAX_MOVE(ch));
        PHRASE(buf);
        k = (ch->master ? ch->master : ch);
        for (f = k->followers; f; f = f->next)
            if (AFF_FLAGGED(f->follower, AFF_GROUP) && f->follower != ch)
                send_to_char(buf, f->follower);
        if (k != ch)
            send_to_char(buf, k);

    } else {
        sprintf(buf, "%s �������%s: %d �� %d �����, %d �� %d ��������",
                GET_NAME(ch), GET_CH_SUF_1(ch),
                GET_HIT(ch), GET_REAL_MAX_HIT(ch), GET_MOVE(ch), GET_REAL_MAX_MOVE(ch));
        if (IS_MANA_CASTER(ch))
            sprintf(buf + strlen(buf), ", %d �� %d ����", GET_MANA(ch), GET_REAL_MAX_MANA(ch));

        sprintf(buf + strlen(buf), ", %ld ���.\r\n", get_dsu_exp(ch));

        PHRASE(buf);
        k = (ch->party_leader ? ch->party_leader : ch);
        for (f = k->followers; f; f = f->next)
            if (AFF_FLAGGED(f->follower, AFF_GROUP) && f->follower != ch)
                send_to_char(buf, f->follower);
        if (k != ch)
            send_to_char(buf, k);

        *buf = '\0';
        sprintf(buf, "�� ��������: %d �� %d �����, %d �� %d ��������",
                GET_HIT(ch), GET_REAL_MAX_HIT(ch), GET_MOVE(ch), GET_REAL_MAX_MOVE(ch));
        if (IS_MANA_CASTER(ch))
            sprintf(buf + strlen(buf), ", %d �� %d ����", GET_MANA(ch), GET_REAL_MAX_MANA(ch));

        sprintf(buf + strlen(buf), ", %ld ���.\r\n", get_dsu_exp(ch));

        send_to_char(buf, ch);

    }

}




ACMD(do_split)
{
    int amount, num, share, rest;
    struct char_data *k;
    struct follow_type *f;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    one_argument(argument, buf);

    if (is_positive_number(buf)) {
        amount = atoi(buf);
        if (amount <= 0) {
            send_to_char("� ��� �� ��� ���������� ������� ?\r\n", ch);
            return;
        }
        if (amount > GET_GOLD(ch)) {
            send_to_char("��� �� ����� ������� �����?\r\n", ch);
            return;
        }

        k = (ch->party_leader ? ch->party_leader : ch);

        if (AFF_FLAGGED(k, AFF_GROUP) && (k->in_room == ch->in_room))
            num = 1;
        else
            num = 0;

        for (f = k->party; f; f = f->next)
            if (AFF_FLAGGED(f->follower, AFF_GROUP) &&
                !IS_NPC(f->follower) && IN_ROOM(f->follower) == IN_ROOM(ch)
                )
                num++;

        if (num && AFF_FLAGGED(ch, AFF_GROUP)) {
            share = amount / num;
            rest = amount % num;
        } else {
            send_to_char("� ��� �� ������ ��������?\r\n", ch);
            return;
        }

        GET_GOLD(ch) -= share * (num - 1);

        sprintf(buf, "%s ��������%s %d %s; ��� ��������� %d.\r\n",
                GET_NAME(ch), GET_CH_SUF_1(ch), amount, desc_count(amount, WHAT_MONEYu), share);
        if (AFF_FLAGGED(k, AFF_GROUP) && IN_ROOM(k) == IN_ROOM(ch) && !IS_NPC(k) && k != ch) {
            GET_GOLD(k) += share;
            send_to_char(buf, k);
        }
        for (f = k->party; f; f = f->next) {
            if (AFF_FLAGGED(f->follower, AFF_GROUP) &&
                !IS_NPC(f->follower) && IN_ROOM(f->follower) == IN_ROOM(ch) && f->follower != ch) {
                GET_GOLD(f->follower) += share;
                send_to_char(buf, f->follower);
            }
        }
        sprintf(buf, "�� ��������� %d %s, �� %d %s �������.\r\n",
                amount, desc_count(amount, WHAT_MONEYu), share, desc_count(share, WHAT_MONEYu));
        if (rest)
            sprintf(buf + strlen(buf), "� %d %s �� �������� ����.\r\n",
                    rest, desc_count(rest, WHAT_MONEYu));
        send_to_char(buf, ch);
    } else {
        send_to_char("������� �� ������ ���������?\r\n", ch);
        return;
    }
}



ACMD(do_use)
{
    struct obj_data *mag_item = NULL;
    int where_bits, do_hold;
    struct char_data *tmp_char;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if (IS_ANIMAL(ch))
        return;

    half_chop(argument, arg, buf);
    if (!*arg) {
        sprintf(buf2, "��� �� ������ %s?\r\n", CMD_NAME);
        send_to_char(buf2, ch);
        return;
    }

    switch (subcmd) {
            //��������
        case SCMD_RECITE:
            //�������
        case SCMD_QUAFF:
            where_bits = FIND_OBJ_INV | FIND_OBJ_EQUIP;
            generic_find(arg, where_bits, ch, &tmp_char, &mag_item);
            if (!mag_item) {
                sprintf(buf2, "� ��� ��� '%s'.\r\n", arg);
                send_to_char(buf2, ch);
                return;
            };

            if (subcmd == SCMD_QUAFF && GET_OBJ_TYPE(mag_item) != ITEM_POTION) {
                send_to_char("������� �� ������ ������ �����!\r\n", ch);
                return;
            }
            if (subcmd == SCMD_RECITE && GET_OBJ_TYPE(mag_item) != ITEM_SCROLL) {
                send_to_char("�������� ����� ������ ������!\r\n", ch);
                return;
            }

            if (subcmd == SCMD_RECITE && AFF_FLAGGED(ch, AFF_BLIND)) {
                send_to_charf(ch, "�� ����� � �� ������� � �������� ������.\r\n");
                return;
            }

            if (GET_EQ(ch, WEAR_HOLD) != mag_item) {
                do_hold = GET_EQ(ch, WEAR_BOTHS) ? WEAR_BOTHS : WEAR_HOLD;
                if (GET_EQ(ch, do_hold)) {
                    act("�� ���������� ������������ $o3.", FALSE, ch, GET_EQ(ch, do_hold), 0,
                        TO_CHAR);
                    act("$n ���������$g ������������ $o3.", FALSE, ch, GET_EQ(ch, do_hold), 0,
                        TO_ROOM);
                    obj_to_char(unequip_char(ch, do_hold), ch);
                    check_sets(ch);
                    affect_total(ch);
                }
                if (GET_EQ(ch, WEAR_HOLD))
                    obj_to_char(unequip_char(ch, WEAR_HOLD), ch);

                act("�� ����� $o3 � ����� ����.", FALSE, ch, mag_item, 0, TO_CHAR);
                act("$n ����$g $o3 � ����� ����.", FALSE, ch, mag_item, 0, TO_ROOM);
                obj_from_char(mag_item);
                equip_char(ch, mag_item, WEAR_HOLD);
                if (subcmd == SCMD_RECITE &&
                    IS_DARK(ch->in_room) &&
                    (!PRF_FLAGGED(ch, PRF_HOLYLIGHT) && !IS_AFFECTED(ch, AFF_DARKVISION))) {
                    send_to_charf(ch, "� ������� �� �� ������ ��������� ������.\r\n");
                    return;
                }
            }
            break;
            //���������
        case SCMD_USE:

            where_bits = FIND_OBJ_INV | FIND_OBJ_EQUIP;
            generic_find(arg, where_bits, ch, &tmp_char, &mag_item);
            if (!mag_item) {
                sprintf(buf2, "� ��� ��� '%s'.\r\n", arg);
                send_to_char(buf2, ch);
                return;
            };

            if (GET_OBJ_TYPE(mag_item) != ITEM_STAFF && GET_OBJ_TYPE(mag_item) != ITEM_WAND) {
                send_to_charf(ch,
                              "��������� ����� ������ ��������� ������� � ���������� ������.\r\n");
                return;
            }

            if (GET_EQ(ch, WEAR_HOLD) != mag_item) {
                do_hold = GET_EQ(ch, WEAR_BOTHS) ? WEAR_BOTHS : WEAR_HOLD;
                if (GET_EQ(ch, do_hold)) {
                    act("�� ���������� ������������ $o3.", FALSE, ch, GET_EQ(ch, do_hold), 0,
                        TO_CHAR);
                    act("$n ���������$g ������������ $o3.", FALSE, ch, GET_EQ(ch, do_hold), 0,
                        TO_ROOM);
                    obj_to_char(unequip_char(ch, do_hold), ch);
                    check_sets(ch);
                    affect_total(ch);
                }
                if (GET_EQ(ch, WEAR_HOLD))
                    obj_to_char(unequip_char(ch, WEAR_HOLD), ch);
                act("�� ����� $o3 � ����� ����.", FALSE, ch, mag_item, 0, TO_CHAR);
                act("$n ����$g $o3 � ����� ����.", FALSE, ch, mag_item, 0, TO_ROOM);
                obj_from_char(mag_item);
                equip_char(ch, mag_item, WEAR_HOLD);
            }
            break;
    }

    /* send_to_charf(ch,"&B*�������*&n");
       return; */
    mag_objectmagic(ch, mag_item, buf);
}


ACMD(do_wimpy)
{
    int wimp_lev;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    /* 'wimp_level' is a player_special. -gg 2/25/98 */
    if (IS_NPC(ch))
        return;

    one_argument(argument, arg);

    if (!*arg) {
        if (GET_WIMP_LEV(ch)) {
            sprintf(buf, "�� ����������� ������ ��� %d%% �����.\r\n", GET_WIMP_LEV(ch));
            send_to_char(buf, ch);
            return;
        } else {
            send_to_char("�� ������ ������� �� ��������� ����� �����.\r\n", ch);
            return;
        }
    }
    if (IS_DIGIT(*arg)) {
        if ((wimp_lev = atoi(arg)) != 0) {
            if (wimp_lev < 0)
                send_to_char("0 ���������?\r\n", ch);
            else if (wimp_lev > 100)
                send_to_char("������ 100% ���������� ����������!\r\n", ch);
            else if (wimp_lev < 1)
                send_to_char("������ 1% ���������� ����������!\r\n", ch);
            else {
                sprintf(buf, "�� ������� �� ���������� %d%% �����.\r\n", wimp_lev);
                send_to_char(buf, ch);
                GET_WIMP_LEV(ch) = wimp_lev;
            }
        } else {
            send_to_char("�� ������ ������� �� ��������� ����� �����.\r\n", ch);
            GET_WIMP_LEV(ch) = 0;
        }
    } else
        send_to_char
            ("��������, ��� ���������� ������ �������� ����� �� ���������� ������� (0 - ������� �� ������).\r\n",
             ch);
}


ACMD(do_display)
{
    size_t i;

    if (IS_NPC(ch)) {
        send_to_char("� ����� ��� �������?\r\n", ch);
        return;
    }
    skip_spaces(&argument);

    if (!*argument) {
        send_to_char("������: ������ { { � | � | � | � | � | � | � } | ��� | ��� }\r\n", ch);
        return;
    }
    if (!str_cmp(argument, "on") || !str_cmp(argument, "all") ||
        !str_cmp(argument, "���") || !str_cmp(argument, "���")) {
        SET_BIT(PRF_FLAGS(ch, PRF_DISPHP), PRF_DISPHP);
        SET_BIT(PRF_FLAGS(ch, PRF_DISPMANA), PRF_DISPMANA);
        SET_BIT(PRF_FLAGS(ch, PRF_DISPMOVE), PRF_DISPMOVE);
        SET_BIT(PRF_FLAGS(ch, PRF_DISPEXITS), PRF_DISPEXITS);
        SET_BIT(PRF_FLAGS(ch, PRF_DISPGOLD), PRF_DISPGOLD);
        SET_BIT(PRF_FLAGS(ch, PRF_DISPLEVEL), PRF_DISPLEVEL);
        SET_BIT(PRF_FLAGS(ch, PRF_DISPEXP), PRF_DISPEXP);
    } else
        if (!str_cmp(argument, "off") || !str_cmp(argument, "none") ||
            !str_cmp(argument, "����") || !str_cmp(argument, "���")) {
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPHP), PRF_DISPHP);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPMANA), PRF_DISPMANA);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPMOVE), PRF_DISPMOVE);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPEXITS), PRF_DISPEXITS);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPGOLD), PRF_DISPGOLD);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPLEVEL), PRF_DISPLEVEL);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPEXP), PRF_DISPEXP);
    } else {
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPHP), PRF_DISPHP);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPMANA), PRF_DISPMANA);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPMOVE), PRF_DISPMOVE);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPEXITS), PRF_DISPEXITS);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPGOLD), PRF_DISPGOLD);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPLEVEL), PRF_DISPLEVEL);
        REMOVE_BIT(PRF_FLAGS(ch, PRF_DISPEXP), PRF_DISPEXP);

        for (i = 0; i < strlen(argument); i++) {
            switch (LOWER(argument[i])) {
                case 'h':
                case '�':
                    SET_BIT(PRF_FLAGS(ch, PRF_DISPHP), PRF_DISPHP);
                    break;
                case 'w':
                case '�':
                    SET_BIT(PRF_FLAGS(ch, PRF_DISPMANA), PRF_DISPMANA);
                    break;
                case 'm':
                case '�':
                    SET_BIT(PRF_FLAGS(ch, PRF_DISPMOVE), PRF_DISPMOVE);
                    break;
                case 'e':
                case '�':
                    SET_BIT(PRF_FLAGS(ch, PRF_DISPEXITS), PRF_DISPEXITS);
                    break;
                case 'g':
                case '�':
                    SET_BIT(PRF_FLAGS(ch, PRF_DISPGOLD), PRF_DISPGOLD);
                    break;
                case 'l':
                case '�':
                    SET_BIT(PRF_FLAGS(ch, PRF_DISPLEVEL), PRF_DISPLEVEL);
                    break;
                case 'x':
                case '�':
                    SET_BIT(PRF_FLAGS(ch, PRF_DISPEXP), PRF_DISPEXP);
                    break;
                default:
                    send_to_char("������: ������ { { � | � | � | � | � | � | � } | ��� | ��� }\r\n",
                                 ch);
                    return;
            }
        }
    }

    send_to_char(OK, ch);
}

ACMD(do_gen_write)
{
    MudFile file;

    if (IS_NPC(ch))
        return;

    skip_spaces(&argument);
    delete_doubledollar(argument);

    if (!*argument) {
        send_to_char("��� ������ �� ������� ��������?\r\n", ch);
        return;
    }

    mudlogf(CMP, LVL_GOD, TRUE,
            "(%s) [%6d] %s: %s", CMD_NAME, GET_ROOM_VNUM(IN_ROOM(ch)), GET_NAME(ch), argument);

    send_to_char("�������!\r\n", ch);

    switch (subcmd) {
        case SCMD_BUG:
            file = MudFile(mud->bugsFile);
            break;
        case SCMD_TYPO:
            file = MudFile(mud->typosFile);
            break;
        case SCMD_IDEA:
            file = MudFile(mud->ideaFile);
            break;
        default:
            return;
    }

    if (file.getSize() > 500000) {
        mudlogf(CMP, LVL_GOD, TRUE, "���� %s ����������!", file.getCPath());
        return;
    }

    DLFileAppend(file).printf("[ ][%s][%6d] %s: %s\n",
                              Date::getCurrentTimeAsString("%b %e %T %Y").c_str(),
                              GET_ROOM_VNUM(IN_ROOM(ch)), GET_NAME(ch), argument);
}



#define TOG_OFF 0
#define TOG_ON  1
const char *gen_tog_type[] = { "�����", "mapping",
    "�������", "brief",
    "������", "compact",
    "����", "color",
    "������", "norepeat",
    "�����", "notell",
    "�������", "nogossip",
    "�������", "noshout",
    "�����", "noholler",
    "������������", "nogratz",
    "�������", "curses",
    "�������", "quest",
    "1��������������", "1automem",
    "���������", "nohassle",
    "������", "nosummon",
    "�������", "nowiz",
    "�����", "roomflags",
    "����������", "slowns",
    "������������", "trackthru",
    "������������", "holylight",
    "�����", "coder",
    "��������������", "formating",
    "������", "width",
    "������", "tips",
    "����", "mobs",
    "�������", "objects",
    "����������", "systemfight",
    "������", "1000",
    "�������", "100000",
    "������������", "wimpy",
    "�����", "hp",
    "��������", "move",
    "����", ",mana",
    "����", "exp",
    "!!������", "!!gold",
    "������", "exits",
    "�������", "level",
    "������", "height",
    "����������", "follow",
    "�������������", "takeobject",
    "����������", "changecolor",
    "������", "examine",
    "��������", "rusextit",
    "���������", "deviate",
    "�����������", "devide",
    "���������", "pointer",
    "��������", "prompt",
    "������", "rent",
    "���������", "messages",
    "������", "mapper",
    "\n"
};




struct gen_tog_param_type {
    int level;
    int subcmd;
} gen_tog_param[] = { {
0, SCMD_AUTOEXIT}, {
0, SCMD_BRIEF}, {
0, SCMD_COMPACT}, {
0, SCMD_COLOR}, {
0, SCMD_NOREPEAT}, {
0, SCMD_NOTELL}, {
0, SCMD_NOGOSSIP}, {
0, SCMD_NOSHOUT}, {
0, SCMD_NOHOLLER}, {
0, SCMD_NOGRATZ}, {
0, SCMD_CURSES}, {
LVL_GOD, SCMD_QUEST}, {
0, SCMD_AUTOMEM}, {
LVL_GRGOD, SCMD_NOHASSLE}, {
LVL_GRGOD, SCMD_NOSUMMON}, {
LVL_GOD, SCMD_NOWIZ}, {
LVL_GRGOD, SCMD_ROOMFLAGS}, {
LVL_IMPL, SCMD_SLOWNS}, {
LVL_GOD, SCMD_TRACK}, {
LVL_GOD, SCMD_HOLYLIGHT}, {
LVL_IMPL, SCMD_CODERINFO}, {
0, SCMD_AUTOFRM}, {
0, SCMD_WIDTH}, {
0, SCMD_TIPS}, {
0, SCMD_GRMOB}, {
0, SCMD_GROBJ}, {
0, SCMD_DISPBOI}, {
0, SCMD_SHOWKILL}, {
0, SCMD_SHOWMEGA}, {
0, SCMD_WIMPY}, {
0, SCMD_HP}, {
0, SCMD_MOVE}, {
0, SCMD_MANA}, {
0, SCMD_SCORE}, {
0, SCMD_GOLD}, {
0, SCMD_EXITS}, {
0, SCMD_LEVEL}, {
0, SCMD_HEIGHT}, {
0, SCMD_NOFOLLOW}, {
0, SCMD_NOGIVE}, {
0, SCMD_THEME}, {
0, SCMD_EXAMIN}, {
0, SCMD_EXITRUS}, {
0, SCMD_AUTODEVTE}, {
0, SCMD_DIVD}, {
0, SCMD_DIVR}, {
0, SCMD_PROMPT}, {
0, SCMD_BANK_RENT}, {
0, SCMD_SELFMESS}, {
0, SCMD_MAPPER},
};

ACMD(do_mode)
{
    int i, showhelp = FALSE;
    char arg[MAX_STRING_LENGTH];

    /*  if (IS_NPC(ch))
       return; */

    argument = one_argument(argument, arg);

    if (!*arg) {
        do_toggle(ch, argument, 0, 0, 0);
        return;
    } else if ((i = search_block(arg, gen_tog_type, FALSE)) < 0)
        showhelp = TRUE;
    else if (GET_LEVEL(ch) < gen_tog_param[i >> 1].level && !GET_COMMSTATE(ch)) {
        send_to_char("��� ������� ��� ����������.\r\n", ch);
        showhelp = TRUE;
    } else {
        do_gen_tog(ch, argument, 0, gen_tog_param[i >> 1].subcmd, 0);
    }

    if (showhelp)
        send_to_charf(ch, "����������� �����.\r\n");
}

ACMD(do_gen_tog)
{
    long result;

    const char *tog_messages[][2] = {
        {"�� �������� �� �������.\r\n",
         "�� ������ ���� ��������.\r\n"},
        {"������������ ���������.\r\n",
         "������������ ��������.\r\n"},
        {"������� ����� ��������.\r\n",
         "������� ����� �������.\r\n"},
        {"������ ����� ��������.\r\n",
         "������ ����� �������.\r\n"},
        {"� ��� ����� ����������.\r\n",
         "�� ����� � �����������.\r\n"},
        {"�� �� ������� ������� ����������� ���������.\r\n",
         "�� ��������� ������ ����������� ���������.\r\n"},
        {"�� ������� ��, ��� ����.\r\n",
         "�� ����� � ����, ��� ����.\r\n"},
        {"�� ������� ��� ��������.\r\n",
         "�� ����� � ��������.\r\n"},
        {"�� ������� ��� ������������.\r\n",
         "�� ����� � �������������.\r\n"},
        {"You can now hear the Wiz-channel.\r\n",
         "You are now deaf to the Wiz-channel.\r\n"},
        {"�� ������ �� ���������� �������.\r\n",
         "�� ���������� �������!\r\n"},
        {"�� ������ �� ������ ������ ����� ������.\r\n",
         "�� �������� ��������� ����� ������.\r\n"},
        {"���� ��������� ����� �������������.\r\n",
         "���� ��������� �� ����� ������������� ���.\r\n"},
        {"���������� ���������.\r\n",
         "���������� ��������.\r\n"},
        {"Nameserver_is_slow changed to NO; IP addresses will now be resolved.\r\n",
         "Nameserver_is_slow changed to YES; sitenames will no longer be resolved.\r\n"},
        {"����� ����������� ��������.\r\n",
         "����� ����������� �������.\r\n"},
        {"�� �� ������ ����� ��������� ������ �����.\r\n",
         "�� �������� ����������� ������ �����.\r\n"},
        {"\r\n",
         "\r\n"},
        {"����� ������ �������������� ���������� ��������.\r\n",
         "����� ������ �������������� ���������� �������.\r\n"},
        {"�������������� ���������� ���������.\r\n",
         "�������������� ���������� ��������.\r\n"},
        {"������ ���������.\r\n",
         "������ ���������.\r\n"},
        {"\r\n",
         "\r\n"},
        {"�� ������� ��� �����.\r\n",
         "�� ����� � ������.\r\n"},
        {"�� ��������� ������������������.\r\n",
         "�� �������� ������������������.\r\n"},
        {"\r\n",
         "\r\n"},
        {"�� ��������� ����� �������.\r\n",
         "�� �������� ����� �������.\r\n"},
        {"����������� ����� ���������.\r\n",
         "����������� ����� ��������.\r\n"},
        {"����������� �������� ���������.\r\n",
         "����������� �������� ��������.\r\n"},
        {"����� ������ ��������� �� ����� ��� ��������.\r\n",
         "����� ������ ��������� �� ����� ��� �������.\r\n"},
        {"��� ���������� ��� ������.\r\n",
         "�������� ���� ����� ���������� ��� ������ �� 1000.\r\n"},
        {"��� ���������� ��� ������.\r\n",
         "�������� ���� ����� ���������� ��� ������ �� 100000.\r\n"},
        {"\r\n",
         "\r\n"},
        {"����� ����� � ������� ��������.\r\n",
         "����� ����� � ������� �������.\r\n"},
        {"����� �������� � ������� ��������.\r\n",
         "����� �������� � ������� �������.\r\n"},
        {"����� ���� � ������� ��������.\r\n",
         "����� ���� � ������� �������.\r\n"},
        {"����� ����� � ������� ��������.\r\n",
         "����� ����� � ������� �������.\r\n"},
        {"����� ���������� � ������� ��������.\r\n",
         "����� ���������� � ������� �������.\r\n"},
        {"����� ������� � ������� ��������.\r\n",
         "����� ������� � ������� �������.\r\n"},
        {"����� ������ � ������� ��������.\r\n",
         "����� ������ � ������� �������.\r\n"},
        {"\r\n",
         "\r\n"},
        {"�� ���� ����� ����������� ����� ��������.\r\n",
         "�� ���� ����� �� ���������.\r\n"},
        {"�� ������� ����� �������.\r\n",
         "�� ������������� ��������� ��������.\r\n"},
        {"�� ����������� ����������� ������� ������.\r\n",
         "�� ����������� �������������� ������� ������.\r\n",},
        {"�� ��������� ���������� ����������� ���� ����.\r\n",
         "�� �� ��������� ������ ��������������� � ����� �����.\r\n"},
        {"����������� ����������� �� ���������� �����.\r\n",
         "����������� ����������� �� ������� �����.\r\n"},
        {"�� ������ �� ������ �������� ������������� ���������� �� ���� ����������.\r\n",
         "�� ������� �������� ������������� ���������� �� ���� ����������.\r\n"},
        {"\r\n",
         "\r\n"},
        {"\r\n",
         "\r\n"},
        {"�� �������� ����������������� ��������� ����� �� ����� ���.\r\n",
         "�� ��������� ����������������� ��������� ����� �� ����� ���.\r\n"},
        {"�� ����������� ������ ����� ����, � ����� �� �������� �����.\r\n",
         "�� ����������� ������ �� �������� �����, � ����� ����� ����.\r\n"},
        {"�� ������ ��� ��������� ���.\r\n",
         "�� ������ ������ ���� ��������� ���.\r\n"},
        {"������ ����� � ��������� ������� ����� ������������ �� �������������� �����.\r\n",
         "������ ����� � ��������� ������� �� ����� ������������ �� �������������� �����.\r\n"},
    };


    if (IS_NPC(ch))
        return;

    switch (subcmd) {
        case SCMD_SELFMESS:
            result = PRF_TOG_CHK(ch, PRF_SELFMESS);
            break;
        case SCMD_NOSUMMON:
            result = PRF_TOG_CHK(ch, PRF_SUMMONABLE);
            break;
        case SCMD_NOHASSLE:
            result = PRF_TOG_CHK(ch, PRF_NOHASSLE);
            break;
        case SCMD_BRIEF:
            result = PRF_TOG_CHK(ch, PRF_BRIEF);
            break;
        case SCMD_COMPACT:
            result = PRF_TOG_CHK(ch, PRF_COMPACT);
            break;
        case SCMD_NOTELL:
            result = PRF_TOG_CHK(ch, PRF_NOTELL);
            break;
        case SCMD_CURSES:
            result = PRF_TOG_CHK(ch, PRF_CURSES);
            break;
        case SCMD_NOHOLLER:
            result = PRF_TOG_CHK(ch, PRF_NOHOLLER);
            break;
        case SCMD_NOGOSSIP:
            result = PRF_TOG_CHK(ch, PRF_NOGOSS);
            break;
        case SCMD_NOSHOUT:
            result = PRF_TOG_CHK(ch, PRF_NOSHOUT);
            break;
        case SCMD_NOGRATZ:
            result = PRF_TOG_CHK(ch, PRF_NOGRATZ);
            break;
        case SCMD_NOWIZ:
            result = PRF_TOG_CHK(ch, PRF_NOWIZ);
            break;
        case SCMD_QUEST:
            result = PRF_TOG_CHK(ch, PRF_QUEST);
            break;
        case SCMD_ROOMFLAGS:
            result = PRF_TOG_CHK(ch, PRF_ROOMFLAGS);
            break;
        case SCMD_NOREPEAT:
            result = PRF_TOG_CHK(ch, PRF_NOREPEAT);
            break;
        case SCMD_HOLYLIGHT:
            result = PRF_TOG_CHK(ch, PRF_HOLYLIGHT);
            break;
        case SCMD_AUTOEXIT:
            result = PRF_TOG_CHK(ch, PRF_AUTOEXIT);
            break;
        case SCMD_TRACK:
            result = (track_through_doors = !track_through_doors);
            break;
        case SCMD_CODERINFO:
            result = PRF_TOG_CHK(ch, PRF_CODERINFO);
            break;
        case SCMD_AUTOMEM:
            result = PRF_TOG_CHK(ch, PRF_AUTOMEM);
            break;
        case SCMD_COLOR:
            do_color(ch, argument, 0, 0, 0);
            return;
            break;
        case SCMD_AUTOFRM:
            result = PRF_TOG_CHK(ch, PRF_AUTOFRM);
            break;
        case SCMD_WIDTH:
            do_set_width(ch, argument, 0, 0, 0);
            return;
            break;
        case SCMD_HEIGHT:
            do_set_height(ch, argument, 0, 0, 0);
            return;
            break;
        case SCMD_TIPS:
            result = PRF_TOG_CHK(ch, PRF_TIPS);
            break;
        case SCMD_GRMOB:
            result = PRF_TOG_CHK(ch, PRF_MOBILES);
            break;
        case SCMD_GROBJ:
            result = PRF_TOG_CHK(ch, PRF_OBJECTS);
            break;
        case SCMD_DISPBOI:
            result = PRF_TOG_CHK(ch, PRF_DISPBOI);
            break;
        case SCMD_SHOWKILL:
            result = PRF_TOG_CHK(ch, PRF_SHOWKILL);
            PRF_REMOVE_CHK(ch, PRF_SHOWMEGA);
            break;
        case SCMD_SHOWMEGA:
            result = PRF_TOG_CHK(ch, PRF_SHOWMEGA);
            PRF_REMOVE_CHK(ch, PRF_SHOWKILL);
            break;
        case SCMD_BANK_RENT:
            result = PRF_TOG_CHK(ch, PRF_BANK_RENT);
            break;
        case SCMD_HP:
            result = PRF_TOG_CHK(ch, PRF_DISPHP);
            break;
        case SCMD_MOVE:
            result = PRF_TOG_CHK(ch, PRF_DISPMOVE);
            break;
        case SCMD_MANA:
            result = PRF_TOG_CHK(ch, PRF_DISPMANA);
            break;
        case SCMD_SCORE:
            result = PRF_TOG_CHK(ch, PRF_DISPEXP);
            break;
        case SCMD_GOLD:
            result = PRF_TOG_CHK(ch, PRF_DISPGOLD);
            break;
        case SCMD_EXITS:
            result = PRF_TOG_CHK(ch, PRF_DISPEXITS);
            break;
        case SCMD_LEVEL:
            result = PRF_TOG_CHK(ch, PRF_DISPLEVEL);
            break;

        case SCMD_EXAMIN:
            result = PRF_TOG_CHK(ch, PRF_EXAMINE);
            break;

        case SCMD_EXITRUS:
            result = PRF_TOG_CHK(ch, PRF_EXITRUS);
            PRF_SET_CHK(ch, PRF_AUTOEXIT);
            break;

        case SCMD_DIVD:
            skip_spaces(&argument);

            if (!*argument)
                send_to_charf(ch, "����� ����������� �� ������ ������������?\r\n");
            if (argument[0] == ' ')
                send_to_charf(ch, "������ ������ ��������� �� ����������.\r\n");
            if (strlen(argument) > 1)
                send_to_charf(ch, "������������ ����� ���� ������ ���� ������.\r\n");
            else {
                send_to_charf(ch, "�� ����������� ����������� '%s'.\r\n", argument);
                strcpy(ch->divd, argument);
            }
            return;
            break;

        case SCMD_DIVR:
            skip_spaces(&argument);

            if (!*argument)
                send_to_charf(ch, "����� ��������� �� ������ ������������?\r\n");
            if (argument[0] == ' ')
                send_to_charf(ch, "������ ������ ������� �� ����������.\r\n");
            if (strlen(argument) > 1)
                send_to_charf(ch, "���������� ����� ���� ������ ���� ������.\r\n");
            else {
                send_to_charf(ch, "�� ����������� ��������� '%s'.\r\n", argument);
                strcpy(ch->divr, argument);
            }
            return;
            break;

        case SCMD_WIMPY:
            do_wimpy(ch, argument, 0, 0, 0);
            return;
            break;
        case SCMD_NOFOLLOW:
            result = PRF_TOG_CHK(ch, PRF_NOFOLLOW);
            break;
        case SCMD_NOGIVE:
            result = PRF_TOG_CHK(ch, PRF_NOGIVE);
            break;

        case SCMD_THEME:
            result = PRF_TOG_CHK(ch, PRF_THEME);
            break;

        case SCMD_PROMPT:
            result = PRF_TOG_CHK(ch, PRF_PROMPT);
            break;
        case SCMD_MAPPER:
            result = PRF_TOG_CHK(ch, PRF_MAPPER);
            break;

#if defined(HAVE_ZLIB)
        case SCMD_COMPRESS:
            result = toggle_compression(ch->desc);
            break;
#else
        case SCMD_COMPRESS:
            send_to_char("Compression not supported.\r\n", ch);
            return;
#endif
        default:
            log("SYSERR: Unknown subcmd %d in do_gen_toggle.", subcmd);
            return;
    }

    if (result)
        send_to_char(tog_messages[subcmd][TOG_ON], ch);
    else
        send_to_char(tog_messages[subcmd][TOG_OFF], ch);

    return;
}

ACMD(do_pray)
{
    struct affected_type af[1];
    struct timed_type timed;

    if (GET_POS(ch) == POS_FIGHTING) {
        send_to_char("� ��� ������������ ����������.\r\n", ch);
        return;
    }

    if (onhorse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }

    if (!GET_SKILL(ch, SKILL_PRAY)) {
        send_to_char("�� �� ������ ������������.\r\n", ch);
        return;
    }

    if (timed_by_skill(ch, SKILL_PRAY)) {
        send_to_char("�� �� ������ ��� ����� ������������.\r\n", ch);
        return;
    }

    improove_skill(ch, 0, 0, SKILL_PRAY);

    act("�� ����������� � �������� ������������ ���.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n ��������$u � �������� ������������ ���.", FALSE, ch, 0, 0, TO_ROOM);

    af[0].type = find_spell_num(SPELL_MEDITATION);
    af[0].bitvector = AFF_MEDITATION;
    af[0].modifier = GET_SKILL(ch, SKILL_PRAY);
    af[0].duration = -1;
    af[0].battleflag = 0;
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;
    af[0].location = APPLY_NONE;

    affect_join_char(ch, af);

    GET_POS(ch) = POS_SLEEPING;

    timed.skill = SKILL_PRAY;
    timed.time = SECS_PER_MUD_TICK * 12;
    timed_to_char(ch, &timed);

}

ACMD(do_trackon)
{

    if (!check_fight_command(ch))
        return;

    if (IS_NPC(ch))
        return;

    if (!GET_SKILL(ch, SKILL_TRACKON)) {
        send_to_charf(ch, "��� �� ������� ����� ������.\r\n");
        return;
    }

    if (!PRF_FLAGGED(ch, PRF_TRACKON)) {
        SET_BIT(PRF_FLAGS(ch, PRF_TRACKON), PRF_TRACKON);
        act("�� ����� ���������� ���� ������������.", FALSE, ch, 0, 0, TO_CHAR);
    } else {
        REMOVE_BIT(PRF_FLAGS(ch, PRF_TRACKON), PRF_TRACKON);
        act("�� ��������� ���������� ���� ������������.", FALSE, ch, 0, 0, TO_CHAR);
    }
}

ACMD(do_notrainer)
{
    send_to_charf(ch, "����� ��� ��������.\r\n");
}

ACMD(do_noshop)
{
    send_to_charf(ch, "�� �� ����� �� ������ ��������.\r\n");
}

ACMD(do_nomail)
{
    send_to_charf(ch, "�� �� ����� �� ������ ��������� ���������.\r\n");
}

ACMD(do_norent)
{
    send_to_charf(ch, "������, ��� ������ �� ������, ����� ������ � ����������� �����.\r\n");
}

ACMD(do_nobank)
{
    send_to_charf(ch, "����� ��� ��������.\r\n");
}

/**************/


ACMD(do_help)
{
    struct help_index_data *helpz;
    int minlen, found = FALSE, type = 0;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];


    skip_spaces(&argument);

    if (!*argument) {
        const DLString & text = mud->getTextFileLoader()->get("summary");
        if (!text.empty())
            page_string(ch->desc, text.c_str(), 1);
        return;
    }

    if (!help_system) {
        send_to_char("������ ����������.\r\n", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (*arg && *argument) {
        if (isname(arg, "�������") || isname(arg, "����������"))
            type = 4;
        else if (isname(arg, "������"))
            type = 3;
    } else
        argument = arg;

    skip_spaces(&argument);


    minlen = strlen(argument);

    for (helpz = help_system; helpz; helpz = helpz->next) {
        if (helpz->type == 4 && type != 4)
            continue;

        // if (!strn_cmp(argument, helpz->keyword,minlen))
        if (isname(argument, helpz->keyword)) {
            sprintf(buf, "\r\n&c%s&n\r\n\r\n", helpz->title);
            if (helpz->format)
                sprintf(buf + strlen(buf), "������: %s\r\n\r\n", helpz->format);
            if (!helpz->entry)
                sprintf(buf2, " ");
            else
                sprintf(buf2, "%s", helpz->entry);
            sprintf(buf + strlen(buf), strbraker(buf2, ch->sw, 1));
            found = TRUE;
            break;
        }
    }

    if (found) {
        //page_string(ch->desc, buf, 0);
        proc_color(buf, (clr(ch, C_NRM)));
        //send_to_charf(ch,buf);
        page_string(ch->desc, buf, 1);
    } else {
        send_to_charf(ch, "��� ������� �� ��������� ����.\r\n");
        sprintf(buf, "%s ����������%s ������� �� ���� '%s'", GET_NAME(ch), GET_CH_SUF_1(ch),
                argument);
        mudlog(buf, CMP, LVL_GOD, TRUE);
    }
}


ACMD(do_articles)
{
    struct help_index_data *helpz;
    char buf[MAX_STRING_LENGTH];

    if (!help_system) {
        send_to_char("������ ����������.\r\n", ch);
        return;
    }

    sprintf(buf, "���� ������� ��� \"����� ����\"\r\n");
    for (helpz = help_system; helpz; helpz = helpz->next)
        sprintf(buf + strlen(buf), " %s\r\n", helpz->title);

    page_string(ch->desc, buf, 1);
//send_to_charf(ch,buf);

}

ACMD(do_connect)
{
    struct tm *tb;
    time_t tmp;
    char buf[MAX_STRING_LENGTH];

    tmp = time(0) - ch->desc->login_time;
    tb = localtime(&tmp);

    sprintf(buf, "���������� � ����������:\r\n");
    sprintf(buf + strlen(buf), "��� IP-�����: %s\r\n", ch->desc->host);
    sprintf(buf + strlen(buf), "����� �����������: %s\r\n", ascii_time(ch->desc->login_time));
    sprintf(buf + strlen(buf), "����������: %ld ����� (%ld ����).\r\n", ch->desc->InBytes / 1024,
            ch->desc->InBytes);
    sprintf(buf + strlen(buf), "�������   : %ld ����� (%ld ����).\r\n", ch->desc->OutBytes / 1024,
            ch->desc->OutBytes);
#if defined(HAVE_ZLIB)
    sprintf(buf + strlen(buf), "�������� ����������: %s.\r\n",
            ch->desc->deflate == NULL ? "���" : (ch->desc->mccp_version ==
                                                 2 ? "MCCPv2" : "MCCPv1"));
#endif

    send_to_charf(ch, buf);
}




AEVENT(event_place_memory)
{
    struct char_data *ch = params->actor;
    int room = params->narg[0];
    const char *sarg = params->sarg;

    send_to_charf(ch, "�� ����������� � ������ %s.\r\n",
                  DAP(get_name_pad(world[room].name, PAD_VIN, PAD_OBJECT)));

    if (*sarg)
        add_memory(ch, GET_ROOM_VNUM(room), GET_MAXTIME(ch), sarg);
    else
        add_memory(ch, GET_ROOM_VNUM(room), GET_MAXTIME(ch), 0);

}

ACMD(do_place)
{
    int i, r_num;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    *buf = '\0';
    *arg = '\0';

    if (!*argument) {
        bool found = FALSE;

        for (i = 0; i <= GET_MAXMEM(ch); i++)
            if ((r_num = real_room(GET_MEMORY(ch, i))) != NOWHERE) {
                sprintf(buf + strlen(buf), "%2d) %s\r\n", i + 1,
                        GET_DESCMEM(ch, i) ? GET_DESCMEM(ch, i) : get_name_pad(world[r_num].name,
                                                                               PAD_IMN,
                                                                               PAD_OBJECT));
                found = TRUE;
            }

        if (found)
            send_to_charf(ch, "�� ������� ��������� �����:\r\n%s", buf);
        else
            send_to_charf(ch, "�� �� ������� �� ������ �����.\r\n");

        return;
    }

    argument = one_argument(argument, arg);
    skip_spaces(&argument);

    if (isname(arg, "���������")) {
        if (check_memory(ch, GET_ROOM_VNUM(IN_ROOM(ch))) != -1)
            send_to_charf(ch, "�� ��� ������ ��� �����.\r\n");
        else {
            if (GET_LASTMEM(ch) >= GET_MAXMEM(ch)) {
                send_to_charf(ch, "�� �� � ��������� ��������� ��� ����� ����.\r\n");
                return;
            }

            if ((IS_DARK(ch->in_room) &&
                 (!PRF_FLAGGED(ch, PRF_HOLYLIGHT) &&
                  !IS_AFFECTED(ch, AFF_DARKVISION))) || AFF_FLAGGED(ch, AFF_BLIND)) {
                send_to_charf(ch,
                              "�� �� � ��������� ��������� �����, ����� ������ �� ������ ����������.\r\n");
                return;
            }

            send_to_charf(ch, "�� ���������������, ������� ����������� � ������ %s.\r\n",
                          DAP(get_name_pad(world[IN_ROOM(ch)].name, PAD_VIN, PAD_OBJECT)));
            sprintf(buf, "$n ������������$u, ������� ����������� � ������ %s.",
                    DAP(get_name_pad(world[IN_ROOM(ch)].name, PAD_VIN, PAD_OBJECT)));
            act(buf, TRUE, ch, 0, 0, TO_ROOM);

            struct event_param_data params;

            init_event_param(&params);

            params.stopflag = STOP_HIDDEN;
            params.show_meter = TRUE;
            params.actor = ch;
            params.narg[0] = IN_ROOM(ch);
            params.sarg = argument;
            params.status = "�� ����������� ���������";
            params.rstatus = "����������� ����������� ���������";
            params.bto_actor = "�� ���������� ���������� ���������";
            add_event(SECS_PER_MUD_TICK, 0, event_place_memory, &params);
        }
    } else if (isname(arg, "������")) {
        if (!*argument)
            send_to_charf(ch, "����� ����� �� ������ ������?\r\n");
        else {
            if (!is_positive_number(argument)) {
                send_to_charf(ch, "������� ����� ������� � ����� ������.\r\n");
                return;
            }
            int num = atoi(argument);

            num--;
            if (check_memory(ch, GET_MEMORY(ch, num)) != -1) {
                r_num = real_room(GET_MEMORY(ch, num));

                send_to_charf(ch, "�� ������� ������ %s.\r\n",
                              GET_DESCMEM(ch, num) ? GET_DESCMEM(ch,
                                                                 num) : (r_num !=
                                                                         -1 ?
                                                                         get_name_pad(world[r_num].
                                                                                      name, PAD_VIN,
                                                                                      PAD_OBJECT) :
                                                                         "����� ��� ��������� '�������������'"));
                del_memory(ch, num);
            } else
                send_to_charf(ch, "� �� � �� ������� ����� �����.\r\n");
        }
    } else if (isname(arg, "���������")) {
        if (!*argument)
            send_to_charf(ch, "����� ����� �� ������ ���������?\r\n");
        else {
            if (!is_positive_number(argument)) {
                send_to_charf(ch, "������� ����� ������� � ����� ������.\r\n");
                return;
            }
            int num = atoi(argument);

            num--;
            if (check_memory(ch, GET_MEMORY(ch, num)) != -1) {
                r_num = real_room(GET_MEMORY(ch, num));
                if (r_num != NOWHERE) {
                    send_to_charf(ch,
                                  "������ ����� �� ����������� ���� ����� ��� ��������� '%s'.\r\n",
                                  GET_DESCMEM(ch, num) ? GET_DESCMEM(ch,
                                                                     num) :
                                  get_name_pad(world[r_num].name, PAD_IMN, PAD_OBJECT));
                    send_to_charf(ch,
                                  strbraker(world[r_num].description, ch->sw,
                                            PRF_FLAGGED(ch, PRF_AUTOFRM)));
                } else {
                    send_to_charf(ch,
                                  "������ ����� �� ���������� ����������� ���� ����� ��� ��������� '%s'.\r\n",
                                  GET_DESCMEM(ch, num) ? GET_DESCMEM(ch, num) : world[r_num].name);
                    send_to_charf(ch, "�� ������ �� ������ ���������.\r\n");
                }
            } else
                send_to_charf(ch, "� �� � �� ������� ����� �����.\r\n");
        }
    }

}


ACMD(do_zone)
{
    int zn;
    char buf[MAX_STRING_LENGTH];

    if (!*argument || !argument) {
        sprintf(buf,
                "%-12s %-54s %s\r\n--------------------------------------------------------------------------------\r\n",
                "   ������", "�������� ����", "�����(�)");
        for (zn = 0; zn <= top_of_zone_table; zn++)
            if (zone_table[zn].type > 0 && zone_table[zn].type < 7)
                sprintf(buf + strlen(buf), "%-12s %-54s %s\r\n", t_zone[(int) zone_table[zn].type],
                        zone_table[zn].name, zone_table[zn].author);

        page_string(ch->desc, buf, 1);
    } else {
        skip_spaces(&argument);
        for (zn = 0; zn <= top_of_zone_table; zn++)
            if (zone_table[zn].type)
                if (is_abbrev(argument, zone_table[zn].name)) {
                    send_to_charf(ch, "��������: %s\r\n", zone_table[zn].name);
                    send_to_charf(ch, "�����(�): %s\r\n", zone_table[zn].author);
                    send_to_charf(ch, "��������������: \r\n");
                    send_to_charf(ch, "���������: \r\n");
                    return;
                }
        send_to_charf(ch,
                      "����� ������� ������� �� �������. ���������� ������ �������� ���������.\r\n");
    }
}



ACMD(do_maketrap)
{
    char buf[MAX_STRING_LENGTH];
    int prob, percent;
    bool fail = FALSE;
    struct obj_data *trap;
    struct timed_type timed;
    struct char_data *k;
    struct follow_type *f;

    if (on_horse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }

    if (!GET_SKILL(ch, SKILL_MAKETRAP)) {
        send_to_charf(ch, "����� �� ������ ��� ������������� �������.\r\n");
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) ||
        SECT(IN_ROOM(ch)) == SECT_WATER_SWIM ||
        SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM ||
        SECT(IN_ROOM(ch)) == SECT_FLYING ||
        SECT(IN_ROOM(ch)) == SECT_UNDERWATER || SECT(IN_ROOM(ch)) == SECT_SECRET) {
        send_to_char("� ���� ������� ������ ���������� ������.\r\n", ch);
        return;
    }

    if (SECT(IN_ROOM(ch)) != SECT_FIELD &&
        SECT(IN_ROOM(ch)) != SECT_FOREST && SECT(IN_ROOM(ch)) != SECT_HILLS) {
        send_to_charf(ch, "������ ����� ���������� ������ � ����� ��������.\r\n");
        return;
    }


    if (!check_moves(ch, TRAP_MOVES))
        return;

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
        fail = TRUE;

    percent = number(1, 100);
    std::vector < int >vit;
    std::vector < int >vot;

    vit.push_back(GET_REAL_DEX(ch));
    vit.push_back(GET_REAL_WIS(ch));
    vot.push_back(number(1, 40));
    prob = calc_like_skill(ch, NULL, SKILL_MAKETRAP, vit, vot);


    if (!timed_by_skill(ch, SKILL_MAKETRAP))
        improove_skill(ch, 0, 0, SKILL_MAKETRAP);

    if (percent <= 5)
        prob = 100;
    else if (percent >= 95)
        prob = 0;

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (prob >= percent && !fail) {
        *buf = '\0';

        trap = create_obj();
        trap->item_number = NOTHING;
        trap->in_room = NOWHERE;
        GET_OBJ_SEX(trap) = SEX_MALE;

        trap->description = str_dup("��������� ��������������� ��������� ������ ���������� �����.");


        trap->short_description = str_dup("��������� ������");
        trap->names = str_dup("�������(��) ������()");
        trap->name = str_dup("��������� ������");
        trap->PNames[0] = str_dup("��������� ������");
        trap->PNames[1] = str_dup("���������� �������");
        trap->PNames[2] = str_dup("���������� �������");
        trap->PNames[3] = str_dup("��������� ������");
        trap->PNames[4] = str_dup("���������� ��������");
        trap->PNames[5] = str_dup("���������� �������");

        GET_OBJ_TYPE(trap) = ITEM_TRAP;
        GET_OBJ_WEAR(trap) = ITEM_WEAR_TAKE;
        GET_OBJ_EXTRA(trap, ITEM_NODONATE) |= ITEM_NODONATE;
        GET_OBJ_EXTRA(trap, ITEM_NOSELL) |= ITEM_NOSELL;
        GET_OBJ_EXTRA(trap, ITEM_NOSELL) |= ITEM_NORENT;
        GET_OBJ_EXTRA(trap, ITEM_HIDDEN) |= ITEM_HIDDEN;

        GET_OBJ_QUALITY(trap) = 5;
        GET_OBJ_WEIGHT(trap) = 10000;   //10�� ���
        GET_OBJ_CUR(trap) = GET_OBJ_MAX(trap) = 30;
        GET_OBJ_TEMP(trap) = 0;

        GET_OBJ_VAL(trap, 0) = GET_SKILL(ch, SKILL_MAKETRAP);
        GET_OBJ_VAL(trap, 1) = TRUE;

        k = (ch->master ? ch->master : ch);
        for (f = k->followers; f; f = f->next) {
            if (!AFF_FLAGGED(f->follower, AFF_GROUP) || ch == f->follower)
                continue;
            add_obj_visible(trap, f->follower);
        }

        k = (ch->party_leader ? ch->party_leader : ch);
        for (f = k->followers; f; f = f->next) {
            if (!AFF_FLAGGED(f->follower, AFF_GROUP) || ch == f->follower)
                continue;
            add_obj_visible(trap, f->follower);
        }

        add_obj_visible(trap, ch);

        obj_to_room(trap, ch->in_room);
        GET_OBJ_TIMER(trap) = (MAX(1, GET_SKILL(ch, SKILL_MAKETRAP) / 10));
        act("�� ��������� ���������� $o.", FALSE, ch, trap, 0, TO_CHAR);
        act("$n ��������� ���������$g $o.", FALSE, ch, trap, 0, TO_ROOM);

    } else
        send_to_charf(ch, "�� �� ������ ���������� ������.\r\n");

    timed.skill = SKILL_MAKETRAP;
    timed.time = SECS_PER_MUD_TICK;
    timed_to_char(ch, &timed);

}

AEVENT(event_fireball)
{
    struct char_data *victim = params->victim;
    struct char_data *ch = params->actor;

    if (IN_ROOM(ch) != IN_ROOM(victim)) {
        send_to_charf(ch, "��� �������� ��� ������ ����-��.\r\n");
        act("������� ��� ������ � ����������� �����������.", FALSE, ch, 0, 0, TO_ROOM);
        return;
    }

}

ACMD(do_fireball)
{
    struct char_data *victim;
    char arg[MAX_STRING_LENGTH];
    struct event_param_data params;

    one_argument(argument, arg);

    if (!(victim = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_char("� ���� �� ������ ��������� �������� ���?\r\n", ch);
        return;
    }

    if (ch == victim) {
        send_to_charf(ch, "� ������ ����?\r\n");
        return;
    }

    if (!may_kill_here(ch, victim))
        return;

    init_event_param(&params);
    params.actor = ch;
    params.victim = victim;
    params.sto_actor = "�� ������� ������� ��� � 2�.";

    params.action = "&Y�������� ������� ��� ������ ����� 2�.&n";
    params.vaction = "&R�������� ������� ��� ������ ����� ���.&n";
    params.raction = "�������� ������� ��� ������ ����� 2�.";

    add_event(2, 0, event_fireball, &params);

}

AEVENT(event_balsm)
{
    struct obj_data *corpse = params->object;
    struct char_data *ch = params->actor;
    int skill = GET_SKILL(ch, SKILL_BALSAM) + 40;

    if (number(1, 100) > skill) {
        act("��� �� ������� ���������������� @1�.", "��", ch, corpse);
        act("1+� �� ������� ���������������� @1�.", "���", ch, corpse);
    } else {
        act("�������� ������� ���� �� ���������������� @1�.", "��", ch, corpse);
        act("�������� ������� ���� 1� ���������������$g @1�.", "���", ch, corpse);
        GET_OBJ_VAL(corpse, 3) = 5;     //������� ���������������
        GET_OBJ_TIMER(corpse) = 45;
        improove_skill(ch, ch, 0, SKILL_BALSAM);
    }

}

ACMD(do_balsam)
{
    struct obj_data *corpse = NULL;
    struct char_data *tmp_char = NULL;
    char arg[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];
    struct event_param_data params;

    if (!check_fight_command(ch))
        return;


    one_argument(argument, arg);

    generic_find(arg, FIND_OBJ_ROOM | FIND_OBJ_INV, ch, &tmp_char, &corpse);

    if (!corpse) {
        send_to_charf(ch, "�� �� ������ ����� '%s'.\r\n", arg);
        return;
    }

    if (!GET_SKILL(ch, SKILL_BALSAM)) {
        send_to_charf(ch, "�� �� ������ ��������������.\r\n");
        return;
    }

    if (!IS_CORPSE(corpse)) {
        act("���������������� ����� ������ ����.", FALSE, ch, corpse, 0, TO_CHAR);
        return;
    }

    if ((GET_OBJ_VAL(corpse, 3) == 3) || (GET_OBJ_VAL(corpse, 3) == 4)) {
        act("���������������� ����� ������ ������ ����.", FALSE, ch, corpse, 0, TO_CHAR);
        return;
    }

    if (GET_OBJ_VAL(corpse, 3) == 5) {
        act("$o ��� ���������������.", FALSE, ch, corpse, 0, TO_CHAR);
        return;
    }

    act("�� ������ �������������� $o.", FALSE, ch, corpse, 0, TO_CHAR);
    act("$n �������$u ��� $o4, ��������� � �������� ���������������.", FALSE, ch, corpse, 0,
        TO_ROOM);

    init_event_param(&params);
    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.actor = ch;
    params.object = corpse;
    sprintf(buf, "�� ������������� %s", GET_OBJ_PNAME(corpse, 0));
    sprintf(buf2, "������������ %s", GET_OBJ_PNAME(corpse, 0));
    params.status = buf;
    params.rstatus = buf2;
    params.bto_actor = "�� ���������� ������� ���������������";

    add_event(30, 0, event_balsm, &params);
}


AEVENT(event_orent)
{
    struct char_data *ch = params->actor;
    int skill = GET_SKILL(ch, SKILL_ORENT);

    if (number(1, 100) > skill) {
        act("��� �� ������� ���������������� �� ���������.", "�", ch);
        act("1*� �� ������� ���������������� �� ���������.", "��", ch);
    } else {
        act("�� ���������������� �� ���������.", "�", ch);
        act("1*� �������������1(��,���,���,���) �� ���������.", "��", ch);
        SET_BIT(AFF_FLAGS(ch, AFF_ORENT), AFF_ORENT);
    }

    improove_skill(ch, ch, 0, SKILL_ORENT);
}

ACMD(do_orent)
{
    struct event_param_data params;

    init_event_param(&params);

    if (!GET_SKILL(ch, SKILL_ORENT)) {
        send_to_charf(ch, "��� �� ������� �������������� � ���������� ���������.\r\n");
        return;
    }
    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.actor = ch;
    params.sto_actor = "�� ������ ���������������.";
    params.sto_room = "1*� ����1(,�,�,�) ����������� ����������� �����������.";
    params.status = "�� �������� �����������";
    params.rstatus = "������� �����������";
    params.bto_actor = "�� ���������� ���������������";

    add_event(12, 0, event_orent, &params);
}


AEVENT(event_dance)
{
    int prob, percent, skill;
    struct char_data *ch = params->actor;

    skill = GET_SKILL(ch, SKILL_RDANCE);
    prob = GET_REAL_WIS(ch) + GET_REAL_WIS(ch) + GET_REAL_WIS(ch) + skill + RNDSKILL;
    percent = 70 + RNDSKILL;
    improove_skill(ch, ch, 0, SKILL_RDANCE);

    if (prob >= percent) {
        act("���� �������� �� ��� ������. ������� ����� �����!", "�", ch);
        act("1� ��������1(,�,�,�) ���� �����, � � 1�� ������ �������� ������ �������.", "��", ch);
        struct affected_type af;

        af.duration = SECS_PER_MUD_TICK * (1 + skill / 10);
        af.type = find_spell_num(SPELL_DANCE);
        af.modifier = skill;
        af.location = 0;
        af.bitvector = AFF_DANCE;
        af.battleflag = 0;
        af.main = TRUE;
        af.owner = GET_ID(ch);
        affect_join_char(ch, &af);
        struct affected_type af1;

        af1.duration = SECS_PER_MUD_TICK * (5 + skill / 5);
        af1.type = find_spell_num(SPELL_DANCE);
        af1.modifier = MAX(5, skill / 5);
        af1.location = APPLY_HITROLL;
        af1.bitvector = AFF_NOTHING;
        af1.battleflag = 0;
        af1.main = FALSE;
        af1.owner = GET_ID(ch);
        affect_join_char(ch, &af1);
    } else {
        act("���� �������� ������� � ������ �������.", "�", ch);
        act("1� ��������1(,�,�,�) ���� �����, � �� 1�� ���� ��������� �������������� ���������.",
            "��", ch);
    }

}

ACMD(do_dance)
{
    struct timed_type timed;
    struct event_param_data params;

    init_event_param(&params);

    if (onhorse(ch))
        return;

    if (!check_fight_command(ch))
        return;

    if (!GET_SKILL(ch, SKILL_RDANCE)) {
        send_to_charf(ch, "��� �� ������� ���������!\r\n");
        return;
    }

    if (timed_by_skill(ch, SKILL_RDANCE)) {
        send_to_char("� ��� �� ������ ���.\r\n", ch);
        return;
    }
    if (IN_ROOM(ch) == NOWHERE)
        return;

    if (!world[IN_ROOM(ch)].fires) {
        send_to_charf(ch, "��������� ���������� ����� ����� ������ ����� � �������.\r\n");
        return;
    }

    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.actor = ch;
    params.sto_actor = "�� ������ ��������� ������ ������, �������� ����� �����.";
    params.sto_room = "1� �����1(,�,�,�) ��������� ������ ������, �������� ����� �����.";
    params.status = "�� �������� ������ ������";
    params.rstatus = "��������� ���������� ����� ��������";
    params.bto_actor = "�� ���������� ��������� ������ ������.";
    params.bto_room = "1� ���������1(,�,�,�) ��������� ������ ������.";

    add_event(45, 0, event_dance, &params);

    timed.skill = SKILL_RDANCE;
    timed.time = 5 * SECS_PER_MUD_TICK;
    timed_to_char(ch, &timed);
}


AEVENT(event_learn)
{
    struct char_data *victim = params->victim;
    struct char_data *ch = params->actor;
    int prob, percent, level = GET_SKILL(ch, SKILL_LEARN), l;
    int skill = params->narg[0];


    if (!victim)
        return;

    prob = GET_REAL_WIS(ch) + GET_REAL_WIS(ch) + GET_REAL_WIS(ch) + level + RNDSKILL;
    percent = 70 + RNDSKILL;
    improove_skill(ch, victim, 0, SKILL_LEARN);
    if (prob >= percent) {
        act("�� ������� ������� ��2(���,��,��,��) 2� ������ '%1'.", "���", ch, victim,
            skill_info[skill].name.c_str());
        l = calc_need_improove(victim, GET_LEVEL(victim));
        l = MIN(150, MAX(0, l));
        SET_SKILL(victim, skill) = l;
        GET_LIKES(victim) = level;
    } else {
        act("��� �� �������� ������� 2�.", "��", ch, victim);
    }
    act("1� ��������1(,�,�,�) ����������� 2�.", "���", ch, victim);
}

ACMD(do_learn_charm)
{
    struct timed_type timed;
    struct event_param_data params;

    init_event_param(&params);
    char arg[MAX_INPUT_LENGTH], *s, *t;
    char buf1[255], buf2[255], buf3[255];
    struct follow_type *f;
    struct char_data *victim = NULL;
    int level = GET_SKILL(ch, SKILL_LEARN), i, skill = 0;

    *arg = '\0';

    if (onhorse(ch))
        return;

    if (!check_fight_command(ch))
        return;


    if (!GET_SKILL(ch, SKILL_LEARN)) {
        send_to_charf(ch, "�� �� ������ ������� ��������!\r\n");
        return;
    }

    if (!*argument) {
        send_to_charf(ch, "�� ������ ������� �������� ��������� �������:\r\n");
        for (i = 0; i < 6; i++)
            if (learn_charm[i][1] <= level)
                send_to_charf(ch, " %s\r\n", skill_info[learn_charm[i][0]].name.c_str());
        return;
    }

    s = strtok(argument, ch->divd);
    if (s == NULL) {
        send_to_charf(ch, "������ ������ �� ������ ������� ��������?\r\n");
        return;
    }
    s = strtok(NULL, ch->divd);
    if (s == NULL) {
        send_to_charf(ch, "�������� ������ ������ ���� ��������� � �������: &C'%s'&n.\r\n",
                      ch->divd);
        return;
    }
    t = strtok(NULL, "\0");
// s - �������� ������
// t - ��� ���������
    if (!t) {
        send_to_charf(ch, "����� �������� �� ������ �������?\r\n");
        return;
    }

    skip_spaces(&t);

    for (i = 0; i < 6; i++)
        if (!strcmp(s, skill_info[learn_charm[i][0]].name.c_str())) {
            if (level < learn_charm[i][1]) {
                send_to_charf(ch, "�� �� �������� ����� ������� ��������.\r\n");
                return;
            } else {
                skill = learn_charm[i][0];
                break;
            }
        }

    if (!skill) {
        send_to_charf(ch, "������ ������ �� �� ������� ������� ���� ����!\r\n");
        return;
    }

    for (f = ch->followers; f; f = f->next) {
        if (f->type != FLW_CHARM)
            continue;
        if (isname(t, f->follower->player.name) || isfullname(t, f->follower->player.names)) {
            victim = f->follower;
        }
    }

    if (!victim) {
        send_to_charf(ch, "� ��� ��� ����������� ��������� '%s'.\r\n", t);
        return;
    }

    /*if (GET_SKILL_MOB(victim, skill) >= level) {
        act("���2(,�,�,�) 2� ��� ������ ����� ������.", "��", ch, victim);
        return;
    }*/

    if (timed_by_skill(ch, SKILL_LEARN)) {
        send_to_char("�� �� ������ ��� ����� ����������� ��������.\r\n", ch);
        return;
    }

    if (IN_ROOM(ch) != IN_ROOM(victim)) {
        send_to_charf(ch, "���� �������� ������ �� ���.\r\n");
        return;
    }

    if (GET_POS(victim) != POS_STANDING) {
        send_to_charf(ch, "���� �������� �� ����� ������������� ������.\r\n");
        return;
    }

    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.actor = ch;

    act("�� ������ ������� 2+� ������ '%1'.", "���", ch, victim, skill_info[skill].name.c_str());
    act("1� �����1(,�,�,�) ������������ 2+�.", "���", ch, victim);
    sprintf(buf1, "�� ����������� %s.", GET_PAD(victim, 3));
    sprintf(buf2, "���������� %s.", GET_PAD(victim, 3));
    sprintf(buf3, "�� ���������� ������������ %s.", GET_PAD(victim, 3));


    params.status = buf1;
    params.rstatus = buf2;
    params.bto_actor = buf3;

    params.victim = victim;
    params.narg[0] = skill;

    add_event(45, 0, event_learn, &params);

    timed.skill = SKILL_LEARN;
    timed.time = 15 * SECS_PER_MUD_TICK;
    timed_to_char(ch, &timed);
}


char *dot_argument(char *argument, char *first_arg)
{
    char *begin = first_arg;

    if (!argument) {
        log("SYSERR: one_argument received a NULL pointer!");
        *first_arg = '\0';
        return (NULL);
    }

    do {
        skip_spaces(&argument);

        first_arg = begin;
        while (*argument && !a_idspace(*argument)) {
            *(first_arg++) = LOWER(*argument);
            argument++;
        }

        *first_arg = '\0';
    } while (fill_word(begin));

    return (argument);
}

struct char_data *find_character(struct char_data *ch, int in_room, char *argument, int flags,
                                 bool see)
{
    char name[MAX_STRING_LENGTH];
    int number;

    if (!*argument)
        return (NULL);

    argument = dot_argument(argument, name);

    send_to_charf(ch, "���������: %s | %s\r\n", argument, name);

    if (is_positive_number(name)) {
        number = atoi(name);
        strcpy(name, argument);
    } else
        number = 1;


    send_to_charf(ch, "���������: %d . %s\r\n", number, name);

    return (NULL);
}

ACMD(do_findt)
{

    find_character(ch, 0, argument, 0, 0);

}


ACMD(do_setnick)
{
    char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];;
    bool found = FALSE;
    room_rnum org_room;
    struct char_data *victim;
    struct follow_type *k, *k_next;
    int i = 0;

    half_chop(argument, name, message);

    if (!*name || !*message) {
        send_to_char("����� �� ������ ���� ���?\r\n", ch);
        return;
    }

    if (strlen(message) < MIN_NAME_LENGTH ||
        strlen(message) > MAX_NAME_LENGTH ||
        !Is_Valid_Name(message) ||
        !Valid_Name(message) || fill_word(strcpy(buf, message)) || reserved_word(buf)) {
        send_to_charf(ch, "������������ ��������, ���������� ���-�� ������.\r\n");
        return;
    }

    if (cmp_ptable_by_name(message, strlen(message)) >= 0) {
        send_to_charf(ch, "�������� ������ � ������ ������������� ������.\r\n");
        return;
    }

    org_room = ch->in_room;
    for (k = ch->followers; k; k = k_next) {
        k_next = k->next;
        if (k->type != FLW_CHARM && k->type != FLW_UNDEAD)
            continue;
        if (isname(name, k->follower->player.name) || isfullname(name, k->follower->player.names)) {
            if (strlen(message) > MIN(12, GET_REAL_INT(k->follower))) {
                send_to_charf(ch, "���� �������� �� �������� ����� ������� ���.\r\n");
                return;
            }
            int rnum = GET_MOB_RNUM(k->follower);

            act("1� ���-�� ���������1(,�,�,�) �� ��� 2�.", "���", ch, k->follower);
            act("�� ���� ����� ������ 2�: '%1'.", "���", ch, k->follower, message);
            if (org_room == k->follower->in_room && AFF_FLAGGED(k->follower, AFF_CHARM)) {
                victim = k->follower;

                sprintf(name, "%s �� ������^'%s'", victim->player.names, message);
                str_reassign_proto(victim->player.names, mob_proto[rnum].player.names, name);

                for (i = 0; i < NUM_PADS; i++)
                    str_reassign_proto(GET_PAD(victim, i),
                                       mob_proto[rnum].player.PNames[i],
                                       get_name_pad(name, i, PAD_MONSTER));

                str_reassign_proto(GET_NAME(victim),
                                   mob_proto[rnum].player.short_descr,
                                   get_name_pad(name, PAD_IMN, PAD_MONSTER));

                str_reassign_proto(victim->player.name,
                                   mob_proto[rnum].player.name,
                                   get_name_pad(name, PAD_IMN, PAD_MONSTER));

                change_pet_name(ch, victim);
                found = TRUE;
            }
        }
    }

    if (!found)
        send_to_char("���� �� ������ ���� ���?\r\n", ch);
}

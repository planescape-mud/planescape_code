/* ************************************************************************
*   File: spells.c                                      Part of CircleMUD *
*  Usage: Implementation of "manual spells".  Circle 2.2 spell compat.    *
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
#include "constants.h"
#include "interpreter.h"
#include "screen.h"
#include "house.h"
#include "pk.h"
#include "case.h"
#include "xspells.h"
#include "xbody.h"
#include "xboot.h"
#include "events.h"
#include "ai.h"

/* local functions */
int compute_armour_wear_m(struct char_data *ch, int type, int hitloc);

/* local globals */
int what_sky = SKY_CLOUDLESS;
char cast_argument[MAX_STRING_LENGTH];

/*
 * Special spells appear below.
 */

#define SUMMON_FAIL "Попытка перемещения не удалась.\r\n"
#define MIN_NEWBIE_ZONE  20
#define MAX_NEWBIE_ZONE  79
#define MAX_SUMMON_TRIES 2000

int sphere_class[NUM_SPHERE] = {
    CLASS_MAGIC_USER,           // Зачарования
    CLASS_MAGIC_USER,           // Прорицания
    CLASS_NECRO,                // Некромантия
    CLASS_MAGIC_USER,           // Превращения
    CLASS_MAGIC_USER,           // Проявления
    CLASS_MAGIC_USER,           // Вызывания
    CLASS_MAGIC_USER,           // Иллюзия
    CLASS_MAGIC_USER,           // Отречения
    CLASS_PRIEST,               //Добро
    CLASS_PRIEST,               //Исцеление
    CLASS_PRIEST,               //Свет
    CLASS_PRIEST,               //Солнце
    CLASS_PRIEST,               //Война
    CLASS_PRIEST,               //Защита
    CLASS_PRIEST,               //Природа
    CLASS_PRIEST,               //Удача
    CLASS_PRIEST,               //Путешествия
    CLASS_PRIEST,               //Сила
    CLASS_PRIEST,               //Магия
    CLASS_PRIEST,               //Зло
    CLASS_PRIEST,               //Смерть
    CLASS_PRIEST                //Разрушения
};

void mort_show_obj_values(struct obj_data *obj, struct char_data *ch, int fullness)
{
    int i, found, drndice = 0, drsdice = 0, kg, gr;
    int spell_enchant = find_spell_num(SPELL_ENCHANT);
    struct C_obj_affected_type *af;
    struct material_data_list *m;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    sprintf(buf, "Предмет: &C%s&n, ", CAP(get_name_pad(GET_OBJ_NAME(obj), PAD_IMN, PAD_OBJECT)));
    sprinttype(GET_OBJ_TYPE(obj), item_types_stat, buf2);
    sprintf(buf + strlen(buf), "Тип: &C%s&n.\r\n", buf2);
    send_to_char(buf, ch);

    strcpy(buf, diag_weapon_to_char(obj, TRUE));
    if (*buf) {
        //strcat(buf,"\r\n");
        send_to_char(buf, ch);
    }


    kg = GET_OBJ_WEIGHT(obj) / 1000;
    gr = GET_OBJ_WEIGHT(obj) - (kg * 1000);
    if (kg < 10)
        gr = gr;
    else if (kg >= 10)
        gr = gr / 100;
    else if (kg >= 50)
        gr = gr / 1000;
    else
        gr = 0;

    if (kg)
        sprintf(buf, "Вес: &G%d.%d&n кг", kg, gr);
    else
        sprintf(buf, "Вес: &G%d&n грамм", gr);

    sprintf(buf + strlen(buf), ", Стоимость: &G%d&n %s.\r\n",
            GET_OBJ_COST(obj), desc_count(GET_OBJ_COST(obj), WHAT_MONEYa));
    send_to_char(buf, ch);

    if (obj->materials) {
        byte sec = 0;

        strcpy(buf, "Изготовлен из:&C");
        for (m = obj->materials; m; m = m->next) {
            sprintf(buf + strlen(buf), "%s%s", (sec) ? ", " : " ",
                    get_name_pad((get_material_param(m->number_mat)->name), PAD_ROD, PAD_OBJECT));
            sec = 1;
        }
        strcat(buf, "&n.\r\n");
    } else
        strcpy(buf, "Материал предмета не известен.\r\n");

    send_to_char(buf, ch);

    sprintf(buf, "Температура: &C%s&n, ", obj_temp[get_const_obj_temp(obj)]);
    if (fullness)
        sprintf(buf + strlen(buf), "Таймер: &G%ld&n\r\n",
                GET_OBJ_TIMER(obj) - ((time(0) - GET_OBJ_TIMELOAD(obj)) / 60));
    send_to_char(buf, ch);

    if (obj->obj_flags.no_flag.flags[0]) {
        send_to_char("Недоступен для &C", ch);
        sprintbits(obj->obj_flags.no_flag, no_bits_stat, buf, ",");
        strcat(buf, "&n.\r\n");
        send_to_char(buf, ch);
    }

    if (obj->obj_flags.anti_flag.flags[0]) {
        send_to_char("Неудобен для: &C", ch);
        sprintbits(obj->obj_flags.anti_flag, anti_bits_stat, buf, ",");
        strcat(buf, "&n.\r\n");
        send_to_char(buf, ch);
    }

    /*
       send_to_char("Имеет дополнения: &C", ch);
       sprintbits(obj->obj_flags.extra_flags, extra_bits, buf, ",");
       strcat(buf, "&n.\r\n");
       send_to_char(buf, ch);
     */

    switch (GET_OBJ_TYPE(obj)) {
        case ITEM_SCROLL:
        case ITEM_POTION:
            sprintf(buf, "Содержит заклинания: ");
            if (GET_OBJ_VAL(obj, 1) >= 1 && GET_OBJ_VAL(obj, 1) < MAX_SPELLS)
                sprintf(buf + strlen(buf), " &C%s&n", spell_name(GET_OBJ_VAL(obj, 1)));
            if (GET_OBJ_VAL(obj, 2) >= 1 && GET_OBJ_VAL(obj, 2) < MAX_SPELLS)
                sprintf(buf + strlen(buf), " &C%s&n", spell_name(GET_OBJ_VAL(obj, 2)));
            if (GET_OBJ_VAL(obj, 3) >= 1 && GET_OBJ_VAL(obj, 3) < MAX_SPELLS)
                sprintf(buf + strlen(buf), " &C%s&n", spell_name(GET_OBJ_VAL(obj, 3)));
            strcat(buf, ".\r\n");
            send_to_char(buf, ch);
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
            sprintf(buf, "Вызывает заклинания: ");
            log("spell %d", GET_OBJ_VAL(obj, 3));
            if (GET_OBJ_VAL(obj, 3) >= 1 && GET_OBJ_VAL(obj, 3) < MAX_SPELLS)
                sprintf(buf + strlen(buf), " &C%s&n", spell_name(GET_OBJ_VAL(obj, 3)));
            sprintf(buf + strlen(buf), " ,зарядов &G%d&n (осталось &G%d&n).\r\n",
                    GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2));
            send_to_char(buf, ch);
            break;
        case ITEM_WEAPON:
            struct weapon_damage_data *n;

            if (obj->weapon && obj->weapon->damages) {
                sprintf(buf, "Усиление &G%d&n, Наносимые повреждения:\r\n", obj->powered);
                for (n = obj->weapon->damages; n; n = n->next)
                    sprintf(buf + strlen(buf),
                            " &C%-10s&n: минимальное &G%d&n, максимальное &G%d&n, среднее: &G%d&n\r\n",
                            damage_name[n->type_damage], n->min_damage, n->max_damage,
                            (n->min_damage + n->max_damage) / 2);
                send_to_char(buf, ch);
            }
            if (obj->spec_weapon && IS_GOD(ch))
                send_to_char("Есть cпециальные повреждения\r\n", ch);

            break;
        case ITEM_ARMOR:
            drndice = GET_OBJ_VAL(obj, 0);
            drsdice = GET_OBJ_VAL(obj, 1);
            sprintf(buf, "Защищает: &G%d&n Тип: &C%s&n\r\n", drndice,
                    armor_class[(int) GET_ARM_TYPE(obj)]);
            sprintf(buf + strlen(buf),
                    "Поглощает режущее: &G%d&n, колющее: &G%d&n, ударное: &G%d&n,\r\n",
                    GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 3));
            sprintf(buf + strlen(buf),
                    "Покрывает режущее: &G%d&n, колющее: &G%d&n, ударное: &G%d&n,\r\n",
                    GET_OBJ_COV(obj, 0), GET_OBJ_COV(obj, 1), GET_OBJ_COV(obj, 2));
            send_to_char(buf, ch);
            break;
        case ITEM_BOOK:
            break;
        case ITEM_CONTAINER:
            kg = GET_OBJ_VAL(obj, 0) / 1000;
            gr = GET_OBJ_VAL(obj, 0) - (kg * 1000);
            char bufm[256];

            *bufm = '\0';
            if (GET_OBJ_VAL(obj, 3))
                sprintf(bufm, " и уменьшает вес предметов на &G%d&n%%", GET_OBJ_VAL(obj, 3));

            if (kg < 10)
                gr = gr;
            else if (kg >= 10)
                gr = gr / 100;
            else if (kg >= 50)
                gr = gr / 1000;
            else
                gr = 0;

            if (kg)
                sprintf(buf, "Вместимость: &G%d.%d&n кг%s\r\n", kg, gr, bufm);
            else
                sprintf(buf, "Вместимость: &G%d&n грамм%s\r\n", gr, bufm);
            *bufm = '\0';
            for (gr = 0; gr < NUM_ITEMS; gr++)
                if (obj->obj_flags.bnotfit[gr])
                    sprintf(bufm + strlen(bufm), " %s", item_types[gr]);

            if (*bufm)
                sprintf(buf + strlen(buf), "Непомещается:&C%s&n\r\n", bufm);

            send_to_char(buf, ch);
            break;
        case ITEM_INGRADIENT:

            sprintbit(GET_OBJ_SKILL(obj), ingradient_bits, buf2, ",");
            sprintf(buf, "%s.\r\n", buf2);
            send_to_char(buf, ch);

            if (IS_SET(GET_OBJ_SKILL(obj), ITEM_CHECK_USES)) {
                sprintf(buf, "можно применить &G%d&n раз.\r\n", GET_OBJ_VAL(obj, 2));
                send_to_char(buf, ch);
            }

            if (IS_SET(GET_OBJ_SKILL(obj), ITEM_CHECK_LAG)) {
                sprintf(buf, "можно применить 1 раз в %d сек", (i = GET_OBJ_VAL(obj, 0) & 0xFF));
                if (GET_OBJ_VAL(obj, 3) == 0 || GET_OBJ_VAL(obj, 3) + i < time(NULL))
                    strcat(buf, "(можно применять).\r\n");
                else
                    sprintf(buf + strlen(buf), "(осталось %ld сек).\r\n",
                            GET_OBJ_VAL(obj, 3) + i - time(NULL));
                send_to_char(buf, ch);
            }

            if (IS_SET(GET_OBJ_SKILL(obj), ITEM_CHECK_LEVEL)) {
                sprintf(buf, "можно применить c %d уровня.\r\n", (GET_OBJ_VAL(obj, 0) >> 8) & 0x1F);
                send_to_char(buf, ch);
            }

            if ((i = real_object(GET_OBJ_VAL(obj, 1))) >= 0) {
                sprintf(buf, "прототип %s%s%s.\r\n",
                        CCICYN(ch, C_NRM), (obj_proto + i)->PNames[0], CCNRM(ch, C_NRM));
                send_to_char(buf, ch);
            }
            break;

    }

    if (obj->obj_flags.affects.flags[0] || obj->C_affected) {
        send_to_char("Накладывает следующие эффекты: &C", ch);
        sprintbits(obj->obj_flags.affects, weapon_affects, buf, ",");
        for (af = obj->C_affected; af; af = af->next)
            if (af->bitvector)
                sprintbit(af->bitvector, affected_bits, buf, ",");
        strcat(buf, "&n.\r\n");
        send_to_char(buf, ch);
    }

    found = FALSE;
    for (af = obj->C_affected; af; af = af->next)
        if (af->location && af->modifier) {
            if (!found) {
                send_to_char("Обладает дополнительными свойствами: \r\n", ch);
                found = TRUE;
            }

            drndice = af->location;
            drsdice = af->modifier;
            sprinttype(drndice, apply_types, buf2);
            sprintf(buf, "   %s%s%s изменяет на %s%s%d%s%s.\r\n",
                    CCICYN(ch, C_NRM), buf2, CCNRM(ch, C_NRM),
                    CCIGRN(ch, C_NRM),
                    drsdice > 0 ? "+" : "", drsdice,
                    (af->type == spell_enchant) ? " (зачаровано)" : "", CCNRM(ch, C_NRM));
            send_to_char(buf, ch);
        }

}

#define IDENT_SELF_LEVEL 6

void mort_show_char_values(struct char_data *victim, struct char_data *ch, int fullness)
{
    struct affected_type *aff;
    int found = FALSE, val0, val1, val2, val3, i;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int hclass[NUM_CLASSES], decl;

    sprintf(buf, "Имя: &C%s&n ", GET_NAME(victim));
    sprintf(buf + strlen(buf), "Падежи: &C%s/%s/%s/%s/%s/%s&n\r\n",
            GET_PAD(victim, 0), GET_PAD(victim, 1), GET_PAD(victim, 2),
            GET_PAD(victim, 3), GET_PAD(victim, 4), GET_PAD(victim, 5));

    sprintf(buf + strlen(buf), "Уровень: &G%d&n, Раса: &G%s&n, Профессия: ",
            GET_LEVEL(victim), race_name[(int) GET_RACE(victim)][(int) GET_SEX(victim)]);
    for (i = 0; i < NUM_CLASSES; i++)
        hclass[i] = 0;
    if (GET_LEVEL_ADD(ch) < 0) {
        for (decl = 0; decl > GET_LEVEL_ADD(ch); decl--) {
            if ((GET_LEVEL(ch) - decl) < 1)
                break;          //Ничего 1го уровня опустить нельзя
            i = GET_HLEVEL(ch, GET_LEVEL(ch) + decl);
            hclass[i]++;
        }
    }
    for (int icls = 0; icls < NUM_CLASSES; icls++)
        if (victim->classes[icls])
            sprintf(buf + strlen(buf), "&C%s&n(&G%d&n) ", class_name[icls], victim->classes[icls]);

    sprintf(buf + strlen(buf), "\r\n");

    val0 = GET_REAL_HEIGHT(victim);
    val1 = GET_REAL_WEIGHT(victim);
    val2 = GET_REAL_SIZE(victim);

    sprintf(buf + strlen(buf), "Рост: &G%d&n cм, Вес: &G%d&n кг, Размер: %d\r\n", val0, val1, val2);

    val0 = GET_REAL_AGE(victim);
    val1 = GET_HIT(victim);
    val2 = GET_REAL_MAX_HIT(victim);
    sprintf(buf + strlen(buf),
            "Возраст: &G%d&n лет, Здоровье текущее: &G%d&n хп, общее: &G%d&n хп\r\n", val0, val1,
            val2);

    sprintf(buf + strlen(buf), "%24s&G%3d&n %18s\r\n", "Защита:", GET_REAL_AC(victim), "Броня:");
    if (IS_NPC(victim)) {
        val1 = victim->npc()->specials.armor[ARM_SLASH];
        val2 = victim->npc()->specials.armor[ARM_PICK];
        val3 = victim->npc()->specials.armor[ARM_BLOW];
        sprintf(buf + strlen(buf), "%24s: &G%5d&n(режущ) &G%3d&n(колющ) &G%3d&n(ударн)\r\n",
                "все тело", val1, val2, val3);
        found = TRUE;
    } else
        for (i = 0; i < NUM_WEARS; i++)
            if ((GET_EQ(victim, wear_order_index[i]) != NULL) &&
                (GET_OBJ_TYPE(GET_EQ(victim, wear_order_index[i])) == ITEM_ARMOR)) {
                val1 = compute_armour_wear_m(victim, ARM_SLASH, wear_order_index[i]);
                val2 = compute_armour_wear_m(victim, ARM_PICK, wear_order_index[i]);
                val3 = compute_armour_wear_m(victim, ARM_BLOW, wear_order_index[i]);
                sprintf(buf + strlen(buf),
                        "%24s:      &G%5d&n(режущ) &G%3d&n(колющ) &G%3d&n(ударн)\r\n",
                        where[wear_order_index[i]], val1, val2, val3);
                found = TRUE;
            }
    if (!found)
        sprintf(buf + strlen(buf),
                "%21s%s: &G0&n     &G0&n(режущ)    &G0&n(колющ)    &G0&n(ударн)\r\n", "гол",
                GET_CH_SUF_3(ch));

    val0 = GET_REAL_HR(victim);
    val1 = GET_REAL_DR(victim);
    sprintf(buf + strlen(buf), "Атака: &G%d&n, Повреждения: &G%d&n, ", val0, val1);

    val1 = speedy(ch);
    sprintf(buf + strlen(buf), "Скорость: &G%d&n\r\n", val1);


    sprintf(buf + strlen(buf), "Выносливость &G%d&n, Рефлекс &G%d&n, Воля &G%d&n\r\n",
            GET_SAVE3(victim, SAV_FORT) + saving_throws_3(victim, SAV_FORT),
            GET_SAVE3(victim, SAV_REFL) + saving_throws_3(victim, SAV_REFL),
            GET_SAVE3(victim, SAV_WILL) + saving_throws_3(victim, SAV_WILL));

    sprintf(buf + strlen(buf),
            "Огонь &G%d&n, Холод &G%d&n, Электричество &G%d&n, Кислота &G%d&n, Яд &G%d&n\r\n",
            GET_SAVE3(victim, SAV_FIRE) + saving_throws_3(victim, SAV_FIRE), GET_SAVE3(victim,
                                                                                       SAV_COLD) +
            saving_throws_3(victim, SAV_COLD), GET_SAVE3(victim,
                                                         SAV_ELECTRO) + saving_throws_3(victim,
                                                                                        SAV_ELECTRO),
            GET_SAVE3(victim, SAV_ACID) + saving_throws_3(victim, SAV_ACID), GET_SAVE3(victim,
                                                                                       SAV_POISON) +
            saving_throws_3(victim, SAV_POISON));

    val0 = GET_REAL_STR(victim);
    val1 = GET_REAL_CON(victim);
    val2 = GET_REAL_DEX(victim);
    sprintf(buf + strlen(buf), "Сила: &G%d&n, Тело: &G%d&n, Ловкость: &G%d&n, ", val0, val1, val2);

    val0 = GET_REAL_INT(victim);
    val1 = GET_REAL_WIS(victim);
    val2 = GET_REAL_CHA(victim);
    sprintf(buf + strlen(buf), "Интелект: &G%d&n, Мудрость: &G%d&n, Обаяние: &G%d&n.\r\n", val0,
            val1, val2);

    found = FALSE;

    for (aff = victim->affected, found = FALSE; aff; aff = aff->next) {
        if (aff->location != APPLY_NONE && aff->modifier != 0) {
            if (!found) {
                sprintf(buf + strlen(buf), "Дополнительные свойства:\r\n%s", CCIYEL(ch, C_NRM));
                found = TRUE;
            }
            sprinttype(aff->location, apply_types, buf2);
            sprintf(buf + strlen(buf), " %s изменяет на %s%d\r\n", buf2,
                    aff->modifier > 0 ? "+" : "", aff->modifier);
        }
    }
    sprintf(buf + strlen(buf), "%s", CCNRM(ch, C_NRM));

    sprintf(buf + strlen(buf), "Аффекты:\r\n%s", CCICYN(ch, C_NRM));

    sprintbits(victim->char_specials.saved.affected_by, affected_bits, buf2, "\r\n");
    sprintf(buf + strlen(buf), "%s\r\n", buf2);
    sprintf(buf + strlen(buf), "%s", CCNRM(ch, C_NRM));
    send_to_char(buf, ch);

    if (IS_NPC(victim) && (GET_REAL_INT(victim) >= 10 + number(0, 5)) &&
        CAN_SEE(victim, ch) && victim->master != ch) {
        act("$N2 не понравилость, что Вы изучаете $S.", TRUE, ch, 0, victim, TO_CHAR);
        act("$N2 не понравилость, что $n изучает $S.", TRUE, ch, 0, victim, TO_NOTVICT);
        _damage(victim, ch, WEAP_RIGHT, 0, C_POWER, TRUE);
    }

}

ASPELL(skill_identify)
{
    if (obj)
        mort_show_obj_values(obj, ch, 100);
    else if (victim)
        mort_show_char_values(victim, ch, 100);
}

/******************************** Текстовые сообщения ***********************/
/* int type - Выбор типа сообщения                                          */
/* 0 - Перед заклинанием                                                    */
/* 1 - Заклинание прошло                                                    */
/* 2 - Заклинание непрошло                                                  */
/* 3 - После заклинания             */
/****************************************************************************/
void act_affect_mess(int spellnum, struct char_data *ch, struct char_data *victim, int show_mess,
                     int type)
{
    const char *to_char = NULL, *to_room = NULL, *to_victim = NULL;
    CItem *spell = Spl.GetItem(spellnum);

    if (!spell->GetItem(SPL_MESSAGES_AFFECT))
        return;

    CItem *amess = spell->GetItem(SPL_MESSAGES_AFFECT)->GetItem(0);

    if (!amess)
        return;

//Сообщения перед началом
    if (type == TYPE_MESS_KILL) {
        to_char = amess->GetItem(SPL_MESSAGE_KIL_CHAR)->GetString();
        to_victim = amess->GetItem(SPL_MESSAGE_KIL_VICT)->GetString();
        to_room = amess->GetItem(SPL_MESSAGE_KIL_ROOM)->GetString();
    }
//Сообщения заклинание прошло
    if (type == TYPE_MESS_HIT) {
        if (victim && victim == ch && amess->GetItem(SPL_MESSAGE_HIT_ME)->GetString())
            to_char = amess->GetItem(SPL_MESSAGE_HIT_ME)->GetString();
        else
            to_char = amess->GetItem(SPL_MESSAGE_HIT_CHAR)->GetString();

        if (victim && victim != ch)
            to_victim = amess->GetItem(SPL_MESSAGE_HIT_VICT)->GetString();

        if (victim && victim == ch && amess->GetItem(SPL_MESSAGE_HIT_MROM)->GetString())
            to_room = amess->GetItem(SPL_MESSAGE_HIT_MROM)->GetString();
        else
            to_room = amess->GetItem(SPL_MESSAGE_HIT_ROOM)->GetString();
    }
//Сообщения заклинание не прошло
    if (type == TYPE_MESS_FAIL) {
        if (victim && victim == ch && amess->GetItem(SPL_MESSAGE_MIS_ME)->GetString())
            to_char = amess->GetItem(SPL_MESSAGE_MIS_ME)->GetString();
        else
            to_char = amess->GetItem(SPL_MESSAGE_MIS_CHAR)->GetString();

        if (victim && victim != ch)
            to_victim = amess->GetItem(SPL_MESSAGE_MIS_VICT)->GetString();

        if (victim && victim == ch && amess->GetItem(SPL_MESSAGE_MIS_MROM)->GetString())
            to_room = amess->GetItem(SPL_MESSAGE_MIS_MROM)->GetString();
        else
            to_room = amess->GetItem(SPL_MESSAGE_MIS_ROOM)->GetString();
    }

//Сообщения перед концом
    if (type == TYPE_MESS_GOD) {
        if (victim && victim == ch && amess->GetItem(SPL_MESSAGE_GOD_ME)->GetString())
            to_char = amess->GetItem(SPL_MESSAGE_GOD_ME)->GetString();
        else
            to_char = amess->GetItem(SPL_MESSAGE_GOD_CHAR)->GetString();

        if (victim && victim != ch)
            to_victim = amess->GetItem(SPL_MESSAGE_GOD_VICT)->GetString();

        if (victim && victim == ch && amess->GetItem(SPL_MESSAGE_GOD_MROM)->GetString())
            to_room = amess->GetItem(SPL_MESSAGE_GOD_MROM)->GetString();
        else
            to_room = amess->GetItem(SPL_MESSAGE_GOD_ROOM)->GetString();
    }

    if (show_mess) {
        if (to_char)
            act(to_char, "Мм", ch, victim);

        if (to_victim && victim && victim != ch)
            act(to_victim, "мМ", ch, victim);
        else if (to_victim && !to_char && victim == ch)
            act(to_victim, "Мм", ch, victim);

        if (to_room && victim && victim != ch)
            act(to_room, "Кмм", ch, victim);
        else if (to_room)
            act(to_room, "Км", ch);
    }
}


void act_object_mess(int spellnum, struct char_data *ch, struct obj_data *obj, int show_mess,
                     int type)
{
    const char *to_char = NULL, *to_room = NULL;
    CItem *spell = Spl.GetItem(spellnum);

    if (!spell->GetItem(SPL_MESSAGES_OBJECT))
        return;

    CItem *omess = spell->GetItem(SPL_MESSAGES_OBJECT)->GetItem(0);


    if (!omess)
        return;

//Сообщения перед началом
    if (type == TYPE_MESS_KILL) {
        to_char = omess->GetItem(SPL_MESSAGE_KIL_CHAR)->GetString();
        to_room = omess->GetItem(SPL_MESSAGE_KIL_ROOM)->GetString();
    }
//Сообщения заклинание прошло
    if (type == TYPE_MESS_HIT) {
        to_char = omess->GetItem(SPL_MESSAGE_HIT_CHAR)->GetString();
        to_room = omess->GetItem(SPL_MESSAGE_HIT_ROOM)->GetString();
    }
//Сообщения заклинание не прошло
    if (type == TYPE_MESS_FAIL) {
        to_char = omess->GetItem(SPL_MESSAGE_MIS_CHAR)->GetString();
        to_room = omess->GetItem(SPL_MESSAGE_MIS_ROOM)->GetString();
    }
//Сообщения перед концом
    if (type == TYPE_MESS_GOD) {
        to_char = omess->GetItem(SPL_MESSAGE_GOD_CHAR)->GetString();
        to_room = omess->GetItem(SPL_MESSAGE_GOD_ROOM)->GetString();
    }

    if (to_char)
        act(to_char, "Мп", ch, obj);

    if (to_room)
        act(to_room, "Кмп", ch, obj);

}

/****************************************************************************/


ASPELL(spell_identify)
{
    if (obj) {
        if (OBJ_FLAGGED(obj, ITEM_NOIDENT)) {
            act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
            return;
        }
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_HIT);
        mort_show_obj_values(obj, ch, 100);
    } else if (victim) {
        if (victim != ch && victim->master != ch) {
            act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
            return;
        }
        if (GET_LEVEL(victim) < 3) {
            send_to_char("Вы можете опознать себя только достигнув третьего уровня.\r\n", ch);
            return;
        }
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
        mort_show_char_values(victim, ch, 100);
    }
}


/*
 * Cannot use this spell on an equipped object or it will mess up the
 * wielding character's hit/dam totals.
 */
int compute_armour_wear_m(struct char_data *ch, int type, int hitloc)
{
    int armour = 0, arm = 0;
    struct obj_data *obj;

    switch (type) {
        case ARM_SLASH:
            armour += GET_ARMOUR(ch, ARM_SLASH);
            break;
        case ARM_PICK:
            armour += GET_ARMOUR(ch, ARM_PICK);
            break;
        case ARM_BLOW:
            armour += GET_ARMOUR(ch, ARM_BLOW);
            break;
    }

    if ((GET_EQ(ch, hitloc) != NULL) && (GET_OBJ_TYPE(GET_EQ(ch, hitloc)) == ITEM_ARMOR)) {
        obj = GET_EQ(ch, hitloc);
        arm = (GET_OBJ_VAL(obj, type + 1) * GET_OBJ_CUR(obj) / MAX(1, GET_OBJ_MAX(obj)));
        armour += arm;
    }

    return armour;
}

int _recall(struct char_data *victim)
{
    room_rnum to_room = NOWHERE, fnd_room = NOWHERE;
    int i, zone = -1;

    if (victim->trap_object && dice(1, 2) == 2)
        return (-1);


    if (IN_ROOM(victim) != NOWHERE)
        zone = world[IN_ROOM(victim)].zone;


    if ((to_room = real_room(zone_table[zone].recall)) == NOWHERE)
        if ((to_room = real_room(GET_LOADROOM(victim))) == NOWHERE)
            to_room = real_room(calc_loadroom(victim));

    if (to_room == NOWHERE)
        return (-1);

    if (!IS_GOD(victim) && ROOM_FLAGGED(IN_ROOM(victim), ROOM_NORECALL))
        return (-1);


    for (i = 0; i < MAX_SUMMON_TRIES; i++) {
        fnd_room = number(0, top_of_world);
        if (world[to_room].zone == world[fnd_room].zone &&
            SECT(fnd_room) != SECT_SECRET &&
            SECT(fnd_room) != SECT_FLYING &&
            !ROOM_FLAGGED(fnd_room, ROOM_DEATH | ROOM_TUNNEL) &&
            !ROOM_FLAGGED(fnd_room, ROOM_NOTELEPORT) &&
            !ROOM_FLAGGED(fnd_room, ROOM_NORECALL) &&
            !ROOM_FLAGGED(fnd_room, ROOM_FLYDEATH) &&
            (!ROOM_FLAGGED(fnd_room, ROOM_GODROOM) || IS_IMMORTAL(victim)) &&
            (!ROOM_FLAGGED(fnd_room, ROOM_PRIVATE)))
            break;
    }

    if (i >= MAX_SUMMON_TRIES)
        return (-1);

    return (fnd_room);
}

ASPELL(spell_recall)
{
    room_rnum fnd_room = NOWHERE;
    struct follow_type *f;

    if (!victim || IS_NPC(victim) || IN_ROOM(ch) != IN_ROOM(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }

    /* Только на согрупника */
    if (!IS_NPC(ch) && !same_group(ch, victim)) {
        send_to_charf(ch, ONLYSAME);
        return;
    }

    if ((fnd_room = _recall(victim)) == NOWHERE) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    char_from_room(victim);
    char_to_room(victim, fnd_room);
    //перемещаем последователей очарок, ездовых, нежить, призваных
    for (f = victim->followers; f; f = f->next)
        if (f->type != FLW_GROUP && IS_NPC(f->follower)) {
            char_from_room(f->follower);
            char_to_room(f->follower, fnd_room);
        }

    check_horse(victim);
    look_at_room(victim, 0);

    act_affect_mess(spellnum, victim, 0, show_mess, TYPE_MESS_GOD);

}

ASPELL(spell_cast_hold)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;
    i = MAX(1, level - affected_by_spell(victim, SPELL_FREE_MOVES));

    if (general_savingthrow_3(victim, SAV_REFL, i) || IS_AFFECTED(victim, AFF_HOLD)) {
        //act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].level = 0;
        af[i].battleflag = 0;
        af[i].main = FALSE;
        af[i].location = APPLY_NONE;
    }

    //само заклинание
    af[0].type = find_spell_num(SPELL_HOLD);
    af[0].duration = MAX(1, level / 30) * SECS_PER_MUD_TICK;
    af[0].bitvector = AFF_HOLD;
    af[0].modifier = MAX(1, level / 10);
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);

    /* af[1].type      = spellnum;
       af[1].duration  = af[0].duration;
       af[1].bitvector = AFF_HOLD;
       af[1].modifier  = level;
       af[1].main    = FALSE;
       af[1].owner     = af[0].owner; */

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }
    //выводим сообщения
    //act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_protect_from_evil)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i, dam;


    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].level = 0;
        af[i].battleflag = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Проверка на доброту
    if (IS_EVILS(victim) && ch != victim) {
        if (!may_kill_here(ch, victim))
            return;
        dam = 1;
        return;
    }
    //само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(SECS_PER_MUD_HOUR, (level / 20) * SECS_PER_MUD_HOUR);
    af[0].bitvector = AFF_PROTECT_EVIL;
    af[0].modifier = level / 8;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);
    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    //выводим сообщения
    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_aid)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    /* if (affected_by_spell_real(victim,spellnum) > level)
       {
       if (show_mess)
       act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
       return;
       }              */

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].level = 0;
        af[i].battleflag = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 25) * SECS_PER_MUD_HOUR;
    af[0].location = APPLY_HITROLL;
    af[0].modifier = MAX(1, level / 30);
    af[0].level = level;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);

    af[1].type = af[0].type;
    af[1].duration = af[0].duration;
    af[1].location = APPLY_DAMROLL;
    af[1].modifier = MAX(1, level / 15);
    af[1].level = level;
    af[1].owner = af[0].owner;

    af[2].type = af[0].type;
    af[2].duration = af[0].duration;
    af[2].location = APPLY_HIT;
    af[2].modifier = MAX(1, level / 3);
    af[2].level = level;
    af[2].owner = af[0].owner;


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    if (show_mess)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_circle_aganist_evil)
{
    struct char_data *tch, *next_tch;
    int spellnum_p = find_spell_num(SPELL_PROTECT_FROM_EVIL);

    if (show_mess)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;
        if (PRF_FLAGGED(tch, PRF_NOHASSLE))
            continue;
        if (!may_kill_here(ch, tch))
            continue;
        spell_protect_from_evil(spellnum_p, level, ch, tch, NULL, show_mess);
    }

}

ASPELL(spell_holy_smite)
{
    struct affected_type af[1];
    int ll, dam = 0, x, y, z, w;


    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if (!may_kill_here(ch, victim))
        return;

    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }
//Кидаем спас-бросок на рефлекс
    ll = MAX(1, level / 4);     //(от 1 до 38%)
    if (ll > 0 && general_savingthrow_3(victim, SAV_REFL, ll))
        dam /= 2;


    if (IS_GOODS(victim))
        dam = 0;

    dam = MAX(0, dam);

    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);

//ослепляем от (1% до 15%)
    ll = MAX(1, level / 10);
    if (ll > 0 && !general_savingthrow_3(victim, SAV_POSITIVE, ll) && IS_UNDEAD(victim) &&
        !AFF_FLAGGED(victim, AFF_BLIND)) {
        if (show_mess)
            act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

        af[0].type = spellnum;
        af[0].bitvector = 0;
        af[0].modifier = 0;
        af[0].battleflag = 0;
        af[0].main = TRUE;
        af[0].location = APPLY_NONE;

        af[0].type = find_spell_num(SPELL_BLIND);
        af[0].duration = PULSE_VIOLENCE / 10;
        af[0].bitvector = AFF_BLIND;
        af[0].owner = GET_ID(ch);
        af[0].modifier = level;
        af[0].main = TRUE;

        //накладываем эффекты
        if (af[0].bitvector || af[0].location != APPLY_NONE)
            affect_join_char(victim, af);

    }

}


ASPELL(spell_dispel_evil)
{
    int dam = 0, x, y, z, w;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

//сами из себя зло не выгоняем
    if (ch == victim)
        return;

    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }

    if (IS_GOODS(victim)) {
        if (show_mess)
            act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
        dam /= 2;
        GET_HIT(victim) = MAX(GET_HIT(victim) + dam, GET_MAX_HIT(victim));
    } else if (IS_NEUTRALS(victim)) {
        if (show_mess)
            act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    } else
        mag_damage(spellnum, dam, ch, victim, show_mess,
                   Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);

}

ASPELL(spell_blade_barrier)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(5, (level / 6));
    af[0].battleflag = TRUE;
    af[0].modifier = level;
    af[0].bitvector = AFF_NOTHING;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);

    af[1].type = spellnum;
    af[1].duration = af[0].duration;
    af[1].battleflag = TRUE;
    af[1].location = APPLY_AC;
    af[1].modifier = MAX(1, level / 10);
    af[1].owner = af[0].owner;


    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    if (show_mess) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
        if (affected_by_spell(victim, SPELL_FIRE_SHIELD))
            act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_GOD);
    }
}

ASPELL(spell_holy_word)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i, dam = 1, x, y, z, w;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

//Жертва не может услышать слово
    /*if (GET_POS(victim) <= POS_SLEEPING || AFF_FLAGGED(victim,AFF_DEAFNESS))
       {
       if (show_mess)
       act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
       return;
       } */
    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }

    if (!IS_GOODS(victim)) {    //1..5%
        if (!general_savingthrow_3(victim, SAV_POSITIVE, level / 30)) {
            //паралич,слабость,слепота
            af[0].type = find_spell_num(SPELL_HOLD);
            af[0].duration = MAX(1, level / 40);
            af[0].battleflag = TRUE;
            af[0].modifier = level;
            af[0].main = TRUE;
            af[0].bitvector = AFF_HOLD;
            af[0].owner = GET_ID(ch);

            af[1].type = find_spell_num(SPELL_BLIND);
            af[1].duration = af[0].duration;
            af[1].bitvector = AFF_BLIND;
            af[1].owner = af[0].owner;
            af[1].modifier = level;
            if (!IS_AFFECTED(victim, AFF_BLIND))
                af[1].main = TRUE;

            af[2].type = spellnum;
            af[2].location = APPLY_STR;
            af[2].duration = af[0].duration;
            af[2].modifier = -MAX(1, level / 20);
            af[2].owner = af[0].owner;
        } else                  //1..10%
        if (!general_savingthrow_3(victim, SAV_POSITIVE, level / 15)) { //слабость,слепота
            af[0].type = find_spell_num(SPELL_BLIND);
            af[0].duration = MAX(1, level / 25);
            af[0].battleflag = TRUE;
            af[0].modifier = level;
            if (!IS_AFFECTED(victim, AFF_BLIND))
                af[0].main = TRUE;
            af[0].bitvector = AFF_BLIND;
            af[0].owner = GET_ID(ch);

            af[1].type = spellnum;
            af[1].location = APPLY_STR;
            af[1].duration = af[0].duration;
            af[1].battleflag = TRUE;
            af[1].modifier = -MAX(1, level / 20);
            af[1].owner = af[0].owner;
        }                       //1..15%
        else if (!general_savingthrow_3(victim, SAV_POSITIVE, level / 10)) {    //слабость
            af[0].type = spellnum;
            af[0].duration = MAX(1, level / 20);
            af[0].battleflag = TRUE;
            af[0].location = APPLY_STR;
            af[0].modifier = -MAX(1, level / 20);
            af[0].main = TRUE;
            af[0].owner = GET_ID(ch);
        }
        // воля 1..10%
        if (!general_savingthrow_3(victim, SAV_WILL, level / 15)) {     //жертва пропустила уровень/2 (75% маск) наносим 1/6 от жизни
            //send_to_charf(ch,"умер\r\n");
            dam = dam * 2;
        }
    } else
        dam = 0;

    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_holy_aura)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;


    /* Только на согрупника
       if (!IS_NPC(ch) && !same_group(ch,victim))
       {
       send_to_charf(ch,ONLYSAME);
       return;
       } */

    if (IS_EVILS(victim)) {
        if (show_mess)
            act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(2 * SECS_PER_MUD_TICK, (level / 6) * SECS_PER_MUD_TICK);
    af[0].bitvector = AFF_HOLYAURA;
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;
    af[1].type = af[0].type;
    af[1].duration = af[0].duration;
    af[1].location = APPLY_AC;
    af[1].modifier = MAX(1, level / 10);
    af[1].owner = af[0].owner;


    //если нейтрал то поправляем чуток
    if (IS_NEUTRALS(victim))
        af[1].modifier /= 3;

    //выводим сообщения
    if (show_mess)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}


ASPELL(spell_summon_monster)
{
    struct char_data *mob = NULL;
    struct affected_type af;
    mob_vnum mob_num = 0;
    int l[3], mlevel, i;

    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        if (sscanf
            (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d", l, l + 1,
             l + 2) != 3)
            mob_num = 114;
        else
            mob_num = l[number(0, 2)];
    }

    if (!(mob = read_mobile(-mob_num, VIRTUAL))) {
        send_to_charf(ch, "Ошибка #sp1801. Срочно сообщите Богам.\r\n", ch);
        return;
    }

    if (get_followers_num(ch, FLW_SUMMON)) {
        send_to_charf(ch, "За Вами не может следовать столько призывных существ.\r\n");
        return;
    }

    mlevel = level / 4;
    GET_LEVEL(mob) = MIN(30, mlevel);
    for (i = 0; i < NUM_CLASSES; i++)
        if (check_class(mob, i))
            add_class(mob, i, mlevel, 1);

    recalc_params(mob);
    char_to_room(mob, ch->in_room);

    af.type = spellnum;
    af.duration = MAX(2 * SECS_PER_MUD_TICK, (level / 8) * SECS_PER_MUD_TICK);
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_HELPER;
    af.battleflag = 0;
    af.owner = GET_ID(ch);

    affect_to_char(mob, &af);

    af.type = spellnum;
    af.duration = MAX(2 * SECS_PER_MUD_TICK, (level / 8) * SECS_PER_MUD_TICK);
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_NOTHING;
    af.battleflag = 0;
    af.owner = GET_ID(ch);

    affect_to_char(ch, &af);

    SET_BIT(MOB_FLAGS(mob, MOB_ANGEL), MOB_ANGEL);

    act_affect_mess(spellnum, ch, mob, show_mess, TYPE_MESS_HIT);
    add_follower(mob, ch, FLW_SUMMON);
    GET_HIT(mob) = GET_REAL_MAX_HIT(mob);
    GET_MOVE(mob) = GET_REAL_MAX_MOVE(mob);
}

ASPELL(spell_infravision)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(1, level) * SECS_PER_MUD_TICK;
    //af[0].bitvector = AFF_INFRAVISION;
    af[0].bitvector = AFF_NOTHING;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_detect_magic)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(1, level) * SECS_PER_MUD_TICK;
    af[0].bitvector = AFF_DETECT_MAGIC;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_endure_elements)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i, duration = MAX(SECS_PER_MUD_TICK, (level / 20) * SECS_PER_MUD_TICK);


    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    if (what_sky == SAV_FIRE) {
        af[0].type = find_spell_num(SPELL_ELEM_FIRE);
        af[0].duration = duration;
        af[0].location = APPLY_SAVING_FIRE;
        af[0].modifier = dice(1, level / 5);
        af[0].main = TRUE;
        af[0].owner = GET_ID(ch);
    } else if (what_sky == SAV_COLD) {
        af[0].type = find_spell_num(SPELL_ELEM_COLD);
        af[0].duration = duration;
        af[0].location = APPLY_SAVING_COLD;
        af[0].modifier = dice(1, level / 5);
        af[0].main = TRUE;
        af[0].owner = GET_ID(ch);
    } else if (what_sky == SAV_ELECTRO) {
        af[0].type = find_spell_num(SPELL_ELEM_ELEC);
        af[0].duration = duration;
        af[0].location = APPLY_SAVING_ELECTRO;
        af[0].modifier = dice(1, level / 5);
        af[0].main = TRUE;
        af[0].owner = GET_ID(ch);
    } else if (what_sky == SAV_ACID) {
        af[0].type = find_spell_num(SPELL_ELEM_ACID);
        af[0].duration = duration;
        af[0].location = APPLY_SAVING_ACID;
        af[0].modifier = dice(1, level / 5);
        af[0].main = TRUE;
        af[0].owner = GET_ID(ch);
    }
    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

#define HEAT_TEMP (level/2+dice(1,10))

ASPELL(spell_heat_metall)
{
    struct obj_data *tobj, *jj, *next_thing2;
    int pos, add = 0;

    if (ch == NULL)
        return;

    if (obj != NULL) {
        if (is_metall(obj)) {
            add = GET_OBJ_TEMP_ADD(obj);
            add += HEAT_TEMP;
            GET_OBJ_TEMP_ADD(obj) = MIN(add, 120);
        }
        //Грем внутри
        for (jj = obj->contains; jj; jj = next_thing2) {
            next_thing2 = jj->next_content;

            if (is_metall(jj)) {
                add = GET_OBJ_TEMP_ADD(jj);
                add += HEAT_TEMP;;
                GET_OBJ_TEMP_ADD(jj) = MIN(add, 120);
            }
        }
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_HIT);
    } else if (victim != NULL) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

        //Греем в экипировке
        for (pos = 0; pos < NUM_WEARS; pos++)
            if (GET_EQ(victim, pos)) {
                tobj = GET_EQ(victim, pos);
                if (is_metall(tobj)) {
                    add = GET_OBJ_TEMP_ADD(tobj);
                    add += HEAT_TEMP;
                    GET_OBJ_TEMP_ADD(tobj) = MIN(add, 120);
                }
                //Грем внутри
                for (jj = tobj->contains; jj; jj = next_thing2) {
                    next_thing2 = jj->next_content;

                    if (is_metall(jj)) {
                        add = GET_OBJ_TEMP_ADD(jj);
                        add += HEAT_TEMP;
                        GET_OBJ_TEMP_ADD(jj) = MIN(add, 120);
                    }
                }

            }
        //Греем в инвентаре
        if (victim->carrying)
            for (tobj = victim->carrying; tobj; tobj = tobj->next_content) {
                if (is_metall(tobj)) {
                    add = GET_OBJ_TEMP_ADD(tobj);
                    add += HEAT_TEMP;
                    GET_OBJ_TEMP_ADD(tobj) = MIN(add, 120);
                }
                //Грем внутри
                for (jj = tobj->contains; jj; jj = next_thing2) {
                    next_thing2 = jj->next_content;

                    if (is_metall(jj)) {
                        add = GET_OBJ_TEMP_ADD(jj);
                        add += HEAT_TEMP;
                        GET_OBJ_TEMP_ADD(jj) = MIN(add, 120);
                    }
                }
            }
    }

}

ASPELL(spell_searing_light)
{
    int dam = 0, x, y, z, w;

    if (!may_kill_here(ch, victim))
        return;

    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }

    if (IS_UNDEAD(victim))
        dam = (int) ((float) dam * 1.5);
    else if (IS_CONSTRUCTION(victim))
        dam /= 2;

    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);
}

ASPELL(spell_fire_shield)
{
    struct affected_type af;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    af.type = spellnum;
    af.bitvector = 0;
    af.modifier = 0;
    af.battleflag = 0;
    af.level = 0;
    af.main = 0;
    af.location = APPLY_NONE;

    af.type = spellnum;
    af.duration = MAX(5, (level / 5));
    af.modifier = level;
    af.battleflag = TRUE;
    af.bitvector = AFF_FIRESHIELD;
    af.main = TRUE;
    af.owner = GET_ID(ch);

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    if (affected_by_spell(victim, SPELL_BLADE_BARRIER))
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_GOD);

//накладываем эффекты
    affect_join_char(victim, &af);

}

ASPELL(spell_flame_strike)
{
    int dam = 0, ll, x, y, z, w;
    struct char_data *next_tch, *tch;

    if (SPELL_SPHERE(spellnum) == SP_WAR) {
        if (ch == NULL)
            return;
        if (!OUTSIDE(ch)) {
            act("Небольшая, но яркая вспышка осветила комнату.", FALSE, ch, 0, 0, TO_CHAR);
            act("Небольшая, но яркая вспышка осветила комнату.", FALSE, ch, 0, 0, TO_ROOM);
            return;
        }

        /* Расчет повреждений */
        if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
            sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y,
                   &z, &w);
            dam = SPLDAMAGE;
        }

        for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
            next_tch = tch->next_in_room;
            if (ch == tch)
                continue;
            if (PRF_FLAGGED(tch, PRF_NOHASSLE))
                continue;
            if (same_group(ch, tch))
                continue;
            if (!may_kill_here(ch, tch))
                continue;
            mag_damage(spellnum, dam, ch, tch, show_mess,
                       Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);
        }
        return;
    }

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if (!may_kill_here(ch, victim))
        return;

    if (!OUTSIDE(ch)) {
        act("Небольшая, но яркая вспышка осветила комнату.", FALSE, ch, 0, 0, TO_CHAR);
        act("Небольшая, но яркая вспышка осветила комнату.", FALSE, ch, 0, 0, TO_ROOM);
        return;
    }

    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        ll = MAX(x, level / y);
        dam = dice(ll, z);
    }

    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);

}

ASPELL(spell_fire_seeds)
{
    int dam = 0, ll, x, y, z, w, in_room = IN_ROOM(victim), i;
    struct char_data *tch, *vict_temp, *next_tch;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if (!may_kill_here(ch, victim))
        return;

    vict_temp = victim;

    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }
    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);

    ll = MAX(1, level / 10);
    for (tch = world[in_room].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;
        if (PRF_FLAGGED(tch, PRF_NOHASSLE))
            continue;
        if (!may_kill_here(ch, tch))
            continue;

        if (ll <= 0)
            continue;
        if (same_group(ch, tch) || FIGHTING(ch) == FIGHTING(tch) || tch == vict_temp || tch == ch)
            continue;

        if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
            sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y,
                   &z, &w);
            dam = SPLDAMAGE;
        }
        mag_damage(spellnum, dam, ch, tch, show_mess,
                   Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);
        ll--;
    }

    for (i = ll; i > 0; i--)
        if (dice(1, 10) <= 3) { // 30% шаров только потом попадает
            if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
                sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x,
                       &y, &z, &w);
                dam = SPLDAMAGE;
            }
            mag_damage(spellnum, dam, ch, victim, show_mess,
                       Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);
        }

}

ASPELL(spell_sunbeam)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(2 * SECS_PER_MUD_TICK, (level / 10) * SECS_PER_MUD_TICK);
    af[0].bitvector = AFF_SUNBEAM;
    af[0].modifier = level;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);

    //новый каст или апдейт
    if (affected_by_spell_real(ch, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(ch, af + i);
    }

}

ASPELL(spell_sunburst)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int dam = 0, x, y, z, w, in_room = IN_ROOM(ch), i;
    int spell_blind = find_spell_num(SPELL_BLIND);
    struct char_data *tch, *next_tch;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL)
        return;

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    for (tch = world[in_room].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;
        if (PRF_FLAGGED(tch, PRF_NOHASSLE))
            continue;
        if (same_group(ch, tch))
            continue;
        if (!may_kill_here(ch, tch))
            continue;
        if (tch == ch)
            continue;

        if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
            sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y,
                   &z, &w);
            dam = SPLDAMAGE;
        }

        /*if (IS_UNDEAD(tch))
           dam *=2; */

        if (IS_AFFECTED(tch, AFF_BLIND))
            dam = 0;

        // 1..10%
        if (level > 0 && !IS_UNDEAD(tch) && general_savingthrow_3(tch, SAV_REFL, level / 15)) {
            dam /= 2;
            mag_damage(spellnum, dam, ch, tch, show_mess,
                       Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);
        } else {
            mag_damage(spellnum, dam, ch, tch, show_mess,
                       Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);
            for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
                af[i].type = spellnum;
                af[i].bitvector = 0;
                af[i].modifier = 0;
                af[i].battleflag = 0;
                af[i].level = 0;
                af[i].main = 0;
                af[i].location = APPLY_NONE;
            }
            //слепота
            af[0].type = spell_blind;
            af[0].duration = PULSE_VIOLENCE / 10;
            af[0].bitvector = AFF_BLIND;
            af[0].modifier = level;
            if (!IS_AFFECTED(tch, AFF_BLIND))
                af[0].main = TRUE;
            af[0].owner = GET_ID(ch);

            //накладываем эффекты
            for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
                if (af[i].bitvector || af[i].location != APPLY_NONE)
                    affect_join_char(tch, af + i);
            }
        }
    }
}

ASPELL(spell_fire_crown)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if ((!IS_NPC(ch) && !same_group(ch, victim)) ||
        (same_group(ch, victim) && IS_NPC(victim) && !IS_AFFECTED(victim, AFF_GROUP))) {
        send_to_charf(ch, ONLYSAME);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(2 * SECS_PER_MUD_TICK, (level / 8) * SECS_PER_MUD_TICK);
    af[0].modifier = level;
    af[0].bitvector = AFF_FIRECROWN;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);
    af[1].type = af[0].type;
    af[1].duration = af[0].duration;
    af[1].modifier = 1;
    af[1].bitvector = AFF_HOLYLIGHT;
    af[1].owner = af[0].owner;

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_cure)
{
    int restore = 0, result = TRUE, x, y;
    float z;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || IN_ROOM(victim) == NOWHERE || victim == NULL)
        return;

    if (IS_UNDEAD(victim) || IS_CONSTRUCTION(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%dd%d+%f", &x, &y, &z);
        restore = dice(x, y) + (int) ((float) level / (float) z);
        restore += GET_INC_MAGIC(ch, 0) * 70 / 100;
    }

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "Лечим %d хп (%dd%d+%d/%.2f)\r\n", restore, x, y, level, z);

    if (GET_HIT(victim) < GET_REAL_MAX_HIT(victim))
        GET_HIT(victim) = MIN(GET_HIT(victim) + restore, GET_REAL_MAX_HIT(victim));
    else
        result = FALSE;

    if (IS_UNDEAD(victim) || IS_CONSTRUCTION(victim) || !result)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    update_pos(victim);
}

ASPELL(spell_cure_mass)
{
    int i, in_room = 0;
    struct room_affect_data af[MAX_SPELL_AFFECTS];
    struct affected_type caf;
    bool accum_affect = FALSE, accum_duration = FALSE;
    bool update_spell = FALSE;
    struct room_data *room;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].owner = 0;
        af[i].duration = 0;
    }

    caf.type = spellnum;
    caf.bitvector = AFF_NOTHING;
    caf.modifier = level;
    caf.level = level;
    caf.duration = 1;
    caf.battleflag = 0;
    caf.main = TRUE;
    caf.owner = GET_ID(ch);
    caf.location = APPLY_NONE;

    in_room = IN_ROOM(ch);
    room = &world[in_room];

    af[0].bitvector = ROOM_AFF_CURE;
    af[0].duration = MAX(SECS_PER_MUD_ROUND, (level / 10) * SECS_PER_MUD_ROUND);
    af[0].modifier = level;
    af[0].owner = GET_ID(ch);

    if (affected_room_by_spell_real(room, spellnum))
        update_spell = TRUE;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
        if (af[i].bitvector) {
            if (update_spell)
                affect_join_fspell_room(room, af + i);
            else
                affect_join_room(room, af + i, accum_duration, FALSE, accum_affect, FALSE);
        }

    affect_join_char(ch, &caf);
}

ASPELL(spell_heal)
{
    int restore, result = TRUE;
    float koef;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || IN_ROOM(victim) == NOWHERE || victim == NULL)
        return;

    if (IS_UNDEAD(victim) || IS_CONSTRUCTION(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }
    koef = (float) level / (float) 100;
    restore = (int) ((float) GET_REAL_MAX_HIT(victim) * (float) koef) - dice(1, 4);
    restore += GET_INC_MAGIC(ch, 0);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "Лечим %d хп коэф=%.2f\r\n", restore, koef);

    if (GET_HIT(victim) < GET_REAL_MAX_HIT(victim))
        GET_HIT(victim) = MIN(GET_HIT(victim) + restore, GET_REAL_MAX_HIT(victim));
    else
        result = FALSE;

    if (IS_UNDEAD(victim) || IS_CONSTRUCTION(victim) || !result)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    if (affected_by_spell(victim, SPELL_BLIND))
        affect_from_char(victim, SPELL_BLIND);

    update_pos(victim);
}

ASPELL(spell_heal_mass)
{
    int restore, result = TRUE;
    float koef;
    struct affected_type af[1];
    struct char_data *tch;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL)
        return;

    af[0].type = spellnum;
    af[0].duration = 1;
    af[0].modifier = 1;
    af[0].battleflag = 0;
    af[0].bitvector = AFF_NOTHING;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);

    affect_join_char(ch, af);

    koef = (float) level *0.50;

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "Коэф=%.2f\r\n", koef);

    for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
        result = FALSE;
        if (same_group(ch, tch) || tch == ch) {
            restore = (int) ((float) GET_REAL_MAX_HIT(tch) * (float) koef) - dice(1, 6);
            restore += GET_INC_MAGIC(ch, 0) * 60 / 100;
            if (GET_HIT(tch) < GET_REAL_MAX_HIT(tch)) {
                GET_HIT(tch) = MIN(GET_HIT(tch) + restore, GET_REAL_MAX_HIT(tch));
                result = TRUE;
            }
        }

        if (IS_UNDEAD(tch) || !result)
            act_affect_mess(spellnum, ch, tch, show_mess, TYPE_MESS_FAIL);
        else
            act_affect_mess(spellnum, ch, tch, show_mess, TYPE_MESS_HIT);

        update_pos(tch);
    }

}

ASPELL(spell_remove_hold)
{
    int result = FALSE, save_val;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || IN_ROOM(victim) == NOWHERE || victim == NULL)
        return;

    save_val = Spl.GetItem(spellnum)->GetItem(SPL_SAVES)->GetInt();

    if (affected_by_spell(victim, SPELL_HOLD) &&
        GET_SKILL(ch, TYPE_SPHERE_SKILL + SPELL_SPHERE(spellnum)) > number(save_val, 150)) {
        affect_from_char(victim, SPELL_HOLD);
        result = TRUE;
    }

    if (!result)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_remove_poison)
{
    int result = FALSE, save_val;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || IN_ROOM(victim) == NOWHERE || victim == NULL)
        return;

    if (!affected_by_spell(victim, SPELL_POISON)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_KILL);
        return;
    }

    save_val = affected_by_spell(victim, SPELL_POISON);
    if (affected_by_spell(victim, SPELL_POISON) && number(level, 100) >= MIN(100, save_val)) {
        affect_from_char(victim, SPELL_POISON);
        result = TRUE;
    }

    if (!result)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    /* else
       act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT); */

}

ASPELL(spell_remove_blind)
{
    int result = FALSE, save_val;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || IN_ROOM(victim) == NOWHERE || victim == NULL)
        return;

    if (!affected_by_spell(victim, SPELL_BLIND) || !IS_AFFECTED(victim, AFF_BLIND)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_KILL);
        return;
    }

    level = MIN(100, level);
    save_val = affected_by_spell(victim, SPELL_BLIND);

    if (save_val && number(level, 100) >= MIN(100, save_val)) {
        affect_from_char(victim, SPELL_BLIND);
        result = TRUE;
    }

    if (!result)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_remove_plaque)
{
    int result = FALSE, save_val;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || IN_ROOM(victim) == NOWHERE || victim == NULL)
        return;

//save_val = Spl.GetItem(spellnum)->GetItem(SPL_SAVES)->GetInt();

    save_val = affected_by_spell(victim, SPELL_PLAGUE);

    if (!affected_by_spell(victim, SPELL_PLAGUE)) {
        //act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        result = FALSE;
    } else if (number(level, 100) >= MIN(100, save_val)) {
        affect_from_char(victim, SPELL_PLAGUE);
        result = TRUE;
    }

    if (!result)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);


}

ASPELL(spell_refresh)
{
    int restore = 0, result = TRUE, x, y, z;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || IN_ROOM(victim) == NOWHERE || victim == NULL)
        return;

    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%dd%d+%d", &x, &y, &z);
        restore = dice(x, y) + (level / z);
    }

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "Восстанавливаем %d мв\r\n", restore);

    if (GET_MOVE(victim) < GET_REAL_MAX_MOVE(victim))
        GET_MOVE(victim) = MIN(GET_MOVE(victim) + restore, GET_REAL_MAX_MOVE(victim));
    else
        result = FALSE;

    if (!result)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_refresh_mass)
{
    int restore = 0, result = TRUE, x, y, z;
    struct char_data *tch, *next_tch;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL)
        return;

    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%dd%d+%d", &x, &y, &z);
        restore = dice(x, y) + (level / z);
    }

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "Восстанавливаем %d мв\r\n", restore);

    for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;
        if (same_group(ch, tch) || ch == tch) {
            result = TRUE;
            if (GET_MOVE(tch) < GET_REAL_MAX_MOVE(tch))
                GET_MOVE(tch) = MIN(GET_MOVE(tch) + restore, GET_REAL_MAX_MOVE(tch));
            else
                result = FALSE;

            if (!result)
                act_affect_mess(spellnum, ch, tch, show_mess, TYPE_MESS_FAIL);
            else
                act_affect_mess(spellnum, ch, tch, show_mess, TYPE_MESS_HIT);
        }
    }
}

AEVENT(event_ressurect)
{
    int spellnum = find_spell_num(SPELL_RESSURECT);
    int isok = params->narg[0];
    struct char_data *ch = params->actor;
    struct char_data *victim = params->victim;

    if (isok && IS_SOUL(victim) && victim->desc) {
        GET_CON_ROLL(victim)--;
        restore_soul(victim, NULL, TRUE);
    } else if (IN_ROOM(victim) != NOWHERE) {
        struct obj_data *tobj, *next_obj;

        for (tobj = world[IN_ROOM(victim)].contents; tobj; tobj = next_obj) {
            next_obj = tobj->next_content;
            if (!IS_CORPSE(tobj))
                continue;
            if (GET_OBJ_VAL(tobj, 1) == GET_ID(victim)) //Нашли труп
                act_object_mess(spellnum, ch, tobj, TRUE, TYPE_MESS_FAIL);
        }
    } else
        act_object_mess(spellnum, ch, NULL, TRUE, TYPE_MESS_KILL);
}

ASPELL(spell_ressurect)
{
    struct event_param_data params;
    struct char_data *tch, *next_ch, *vict = NULL;

    if (obj == NULL) {
        act_object_mess(spellnum, ch, NULL, TRUE, TYPE_MESS_HIT);
        return;
    }

    if (!IS_CORPSE(obj)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_GOD);
        return;
    }

    for (tch = world[IN_ROOM(obj)].people; tch; tch = next_ch) {
        next_ch = tch->next_in_room;
        if (!IS_SOUL(tch))
            continue;
        if (GET_OBJ_VAL(obj, 1) == GET_ID(tch)) {
            vict = tch;
            break;
        }
    }

    if (!vict) {
        act_object_mess(spellnum, ch, NULL, TRUE, TYPE_MESS_HIT);
        return;
    }

    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char buf4[MAX_STRING_LENGTH];
    char buf5[MAX_STRING_LENGTH];
    char buf6[MAX_STRING_LENGTH];

    sprintf(buf1, "Вы начали воскрешать %s.", GET_OBJ_PNAME(obj, 0));
    sprintf(buf2, "1+и начал1(,а,о,и) произносить молитву над %s", GET_OBJ_PNAME(obj, 4));
    sprintf(buf3, "произносит молитву над %s", GET_OBJ_PNAME(obj, 4));
    sprintf(buf4, "Вы прекратили молиться");
    sprintf(buf5, "1+и прекратил1(,а,о,и) молиться");
    sprintf(buf6, "Вы проводите процедуру воскрешения %s", GET_OBJ_PNAME(obj, 1));

    init_event_param(&params);

    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.actor = ch;
    params.victim = vict;
    params.narg[0] = TRUE;
    params.sto_actor = buf1;
    params.sto_room = buf2;
    params.status = buf6;
    params.rstatus = buf3;
    params.bto_actor = buf4;
    params.bto_room = buf5;

    add_event(30, 0, event_ressurect, &params);
}

ASPELL(spell_magic_weapon)
{
    int i;
    bool update_spell = FALSE;
    struct C_obj_affected_type af[MAX_SPELL_AFFECTS];
    bool accum_affect = FALSE, accum_duration = FALSE, success = TRUE;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || obj == NULL)
        return;

    if (GET_OBJ_TYPE(obj) != ITEM_WEAPON) {
        act("Синее облако окутало $o3 и медлено расстворилось.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].duration = 0;
        af[i].location = APPLY_NONE;
        af[i].extra = 0;
        af[i].no_flag = 0;
        af[i].anti_flag = 0;
        af[i].main = 0;
    }

    af[0].type = spellnum;
    af[0].location = APPLY_DAMROLL;
    af[0].duration = MAX(2 * SECS_PER_MUD_TICK, (level / 4) * SECS_PER_MUD_TICK);
    af[0].modifier = MAX(1, level / 25);
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;

    if (IS_EVILS(ch)) {
        af[0].no_flag = ITEM_NO_GOOD | ITEM_NO_NEUTRAL;
        af[0].anti_flag = ITEM_AN_GOOD | ITEM_AN_NEUTRAL;
        af[0].extra = ITEM_DARKAURA;
    } else {
        af[0].no_flag = ITEM_NO_EVIL | ITEM_NO_NEUTRAL;
        af[0].anti_flag = ITEM_AN_EVIL | ITEM_AN_NEUTRAL;
        af[0].extra = ITEM_GOODAURA;
    }
    af[1].type = af[0].type;
    af[1].location = APPLY_HITROLL;
//af[1].bitvector= AFF_LEVIT;
    af[1].modifier = af[0].modifier;
    af[1].duration = af[0].duration;
    af[1].owner = af[0].owner;

    if (affected_object_by_spell(obj, spellnum) && success)
        update_spell = TRUE;

//накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE) {
            if (update_spell)
                affect_join_fspell_object(obj, af + i);
            else
                affect_join_object(obj, af + i, accum_duration, FALSE, accum_affect, FALSE);
        }
    }

    if (IS_EVILS(ch))
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
    else
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_magic_vestment)
{
    int i;
    bool accum_affect = FALSE, accum_duration = FALSE, success = TRUE;
    bool update_spell = FALSE;
    struct C_obj_affected_type af[MAX_SPELL_AFFECTS];

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || obj == NULL)
        return;

    if (GET_OBJ_TYPE(obj) != ITEM_ARMOR ||
        (!CAN_WEAR(obj, ITEM_WEAR_SHIELD) && !CAN_WEAR(obj, ITEM_WEAR_BODY))) {
        act("Синее облако окутало $o3 и медлено расстворилось.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].duration = 0;
        af[i].location = APPLY_NONE;
        af[i].extra = 0;
        af[i].no_flag = 0;
        af[i].anti_flag = 0;
        af[i].main = 0;
    }

    af[0].type = spellnum;
    af[0].location = APPLY_AC;
    af[0].modifier = MAX(1, level / 5);
    af[0].duration = MAX(2 * SECS_PER_MUD_TICK, (level / 4) * SECS_PER_MUD_TICK);
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;

    if (IS_EVILS(ch)) {
        af[0].no_flag = ITEM_NO_GOOD | ITEM_NO_NEUTRAL;
        af[0].anti_flag = ITEM_AN_GOOD | ITEM_AN_NEUTRAL;
        af[0].extra = ITEM_DARKAURA;
    } else {
        af[0].no_flag = ITEM_NO_EVIL | ITEM_NO_NEUTRAL;
        af[0].anti_flag = ITEM_AN_EVIL | ITEM_AN_NEUTRAL;
        af[0].extra = ITEM_GOODAURA;
    }

    if (affected_object_by_spell(obj, spellnum) && success)
        update_spell = TRUE;

//накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE) {
            if (update_spell)
                affect_join_fspell_object(obj, af + i);
            else
                affect_join_object(obj, af + i, accum_duration, FALSE, accum_affect, FALSE);
        }
    }

    if (IS_EVILS(ch))
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
    else
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_power_armor)
{
    int i;
    bool accum_affect = FALSE, accum_duration = FALSE, success = TRUE;
    bool update_spell = FALSE;
    struct C_obj_affected_type af[MAX_SPELL_AFFECTS];

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || obj == NULL)
        return;

    if (GET_OBJ_TYPE(obj) != ITEM_ARMOR ||
        (!CAN_WEAR(obj, ITEM_WEAR_SHIELD) && !CAN_WEAR(obj, ITEM_WEAR_BODY))) {
        act("Синее облако окутало $o3 и медлено расстворилось.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].duration = 0;
        af[i].location = APPLY_NONE;
        af[i].extra = 0;
        af[i].no_flag = 0;
        af[i].anti_flag = 0;
        af[i].main = 0;
    }


    if (IS_EVILS(ch)) {
        af[0].no_flag = ITEM_NO_GOOD | ITEM_NO_NEUTRAL;
        af[0].anti_flag = ITEM_AN_GOOD | ITEM_AN_NEUTRAL;
        af[0].extra = ITEM_DARKAURA;
    } else {
        af[0].no_flag = ITEM_NO_EVIL | ITEM_NO_NEUTRAL;
        af[0].anti_flag = ITEM_AN_EVIL | ITEM_AN_NEUTRAL;
        af[0].extra = ITEM_GOODAURA;
    }
    af[0].main = TRUE;
    af[0].type = spellnum;
    af[0].location = APPLY_ARMOUR0;
    af[0].modifier = MAX(5, level / 6);
    af[0].duration = MAX(2 * SECS_PER_MUD_TICK, (level / 4) * SECS_PER_MUD_TICK);
    af[0].owner = af[0].owner;
    af[1].location = APPLY_ARMOUR1;
    af[1].modifier = af[0].modifier;
    af[1].duration = af[0].duration;
    af[1].owner = af[0].owner;
    af[2].location = APPLY_ARMOUR2;
    af[2].modifier = af[0].modifier;
    af[2].duration = af[0].duration;
    af[2].owner = af[0].owner;

    if (affected_object_by_spell(obj, spellnum) && success)
        update_spell = TRUE;

//накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE) {
            if (update_spell)
                affect_join_fspell_object(obj, af + i);
            else
                affect_join_object(obj, af + i, accum_duration, FALSE, accum_affect, FALSE);
        }
    }

    if (IS_EVILS(ch))
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
    else
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_divine_power)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(SECS_PER_MUD_HOUR, (level / 20) * SECS_PER_MUD_HOUR);
    af[0].location = APPLY_DAMROLL;
    af[0].modifier = MAX(1, level / 25);
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);



    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_power_word_stune)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE, result = FALSE;
    int i;

    if (ch == NULL || IN_ROOM(ch) == NOWHERE || victim == NULL)
        return;

    if (!may_kill_here(ch, victim))
        return;

    i = MAX(1, level - affected_by_spell(victim, SPELL_FREE_MOVES));
    if (general_savingthrow_3(victim, SAV_REFL, i)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }
    //1..75%
    if (level > 0 && !general_savingthrow_3(victim, SAV_WILL, level / 2)) {
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            af[i].type = spellnum;
            af[i].bitvector = 0;
            af[i].modifier = 0;
            af[i].battleflag = 0;
            af[i].level = 0;
            af[i].main = 0;
            af[i].location = APPLY_NONE;
        }

        af[0].type = find_spell_num(SPELL_STUNE);
        af[0].duration = MAX(2, level / 20);
        af[0].battleflag = TRUE;
        af[0].bitvector = AFF_STUNE;
        af[0].owner = GET_ID(ch);
        af[0].main = TRUE;

        /*   af[1].type      = spellnum;
           af[1].duration  = af[0].duration;
           af[1].bitvector = AFF_NOTHING;
           af[1].owner     = af[0].owner; */

        if (affected_by_spell_real(victim, spellnum))
            update_spell = TRUE;
        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++)
            if (af[i].bitvector || af[i].location != APPLY_NONE)
                affect_join_char(victim, af + i);
        result = TRUE;
    }

    if (!result)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_power_word_blind)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE, result = FALSE;
    int i;

    if (ch == NULL || IN_ROOM(ch) == NOWHERE || victim == NULL)
        return;

//1..50%
    if (level > 0 && !general_savingthrow_3(victim, SAV_WILL, level / 3)) {
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            af[i].type = spellnum;
            af[i].bitvector = 0;
            af[i].modifier = 0;
            af[i].battleflag = 0;
            af[i].level = 0;
            af[i].main = 0;
            af[i].location = APPLY_NONE;
        }

        af[0].type = find_spell_num(SPELL_BLIND);
        af[0].location = APPLY_NONE;
        af[0].duration = MAX(1, level / 10);
        af[0].modifier = level;
        af[0].battleflag = TRUE;
        af[0].bitvector = AFF_BLIND;
        af[0].owner = GET_ID(ch);
        if (!IS_AFFECTED(victim, AFF_BLIND))
            af[0].main = TRUE;

        /*   af[1].type      = spellnum;
           af[1].duration  = af[0].duration;
           af[1].bitvector = AFF_NOTHING;
           af[1].owner     = af[0].owner; */

        if (affected_by_spell_real(victim, spellnum))
            update_spell = TRUE;
        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++)
            if (af[i].bitvector || af[i].location != APPLY_NONE) {
                result = TRUE;
                affect_join_char(victim, af + i);
            }
    }

    if (!result)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_power_word_kill)
{
    int dam = 0, x, y, z, w;

    if (ch == NULL || IN_ROOM(ch) == NOWHERE || victim == NULL)
        return;


    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        //1%
        if (level > 0 && !general_savingthrow_3(victim, SAV_WILL, 1))
            dam = GET_MAX_HIT(victim) * 2;
        else
            dam = SPLDAMAGE;
    }

    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);

    return;
}

ASPELL(spell_protect_from_good)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i, dam = 0;


    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Проверка на доброту
    if (IS_GOODS(victim) && ch != victim) {
        if (!may_kill_here(ch, victim))
            return;
        dam = 1;
        mag_damage(spellnum, dam, ch, victim, show_mess,
                   Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);
        return;
    }
    //само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(SECS_PER_MUD_HOUR, (level / 20) * SECS_PER_MUD_HOUR);
    af[0].bitvector = AFF_PROTECT_GOOD;
    af[0].modifier = level / 8;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_desecrate)
{
    struct room_affect_data af[MAX_SPELL_AFFECTS];
    bool accum_affect = FALSE, accum_duration = FALSE, success = TRUE;
    bool update_spell = FALSE, result = FALSE;
    int in_room = IN_ROOM(ch), i, perc = 100;
    struct room_data *room;

    if (ch == NULL || in_room == NOWHERE)
        return;

    room = &world[in_room];

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].owner = 0;
        af[i].duration = 0;
    }

    af[0].bitvector = ROOM_AFF_DESECRATE;
    af[0].duration = MAX(SECS_PER_MUD_HOUR, (level / 25) * SECS_PER_MUD_HOUR);
    af[0].modifier = MAX(1, level / 30);
    af[0].owner = GET_ID(ch);

    int mod = 0;

    if ((mod = affected_room_by_bitvector(room, ROOM_AFF_SANCTUARY)))
        perc += mod;

    if (number(1, perc) < level) {
        if (affected_room_by_spell_real(room, spellnum) && success)
            update_spell = TRUE;
        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            if (af[i].bitvector) {
                if (update_spell)
                    affect_join_fspell_room(room, af + i);
                else
                    affect_join_room(room, af + i, accum_duration, FALSE, accum_affect, FALSE);
            }
        }
        result = TRUE;
    }

    if (!result)
        act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_FAIL);
    else
        act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_HIT);

}

/* Процедура создания нежити */
ASPELL(spell_create_undead)
{
    int mob_num, mob_rnum, i, mob_type, min_level, max_level, min_hp, max_hp, rnum, mlevel;
    struct char_data *mob;
    struct affected_type af;

    if (obj == NULL)
        return;

//log("ТТТ0");

    if (!IS_CORPSE(obj)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_GOD);
        return;
    }

    if (affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_SANCTUARY)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }
//Животное уже есть
    if (get_followers_num(ch, FLW_UNDEAD) > 0)
        level = 0;

//Читаем данные
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d=%d+%d/%d+%d", &mob_type,
               &min_level, &max_level, &min_hp, &max_hp);
    } else {
        send_to_charf(ch, "Неправильный файл конфигурации\r\n");
        return;
    }

    mob_num = GET_OBJ_VAL(obj, 2);
    mob_rnum = real_mobile(mob_num);

    if (mob_rnum == -1) {
        send_to_charf(ch, "Неизвествный монстр прародитель.\r\n");
        return;
    }

    if (mob_num > 0) {
        if (GET_LEVEL(mob_proto + mob_rnum) <= min_level)
            level = 0;
    } else {                    //труп игрока
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_GOD);
        //extract_obj(obj);
        return;
    }

    if (IS_CONSTRUCTION(mob_proto + mob_rnum) || IS_UNDEAD(mob_proto + mob_rnum)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        extract_obj(obj);
        return;
    }
//log("ТТТ1");
    i = level / 15;

//1..10%
    if (general_savingthrow_3(mob_proto + mob_rnum, SAV_NEGATIVE, i) || i < 1) {
        //профейлили
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        extract_obj(obj);
        return;
    }

    if (!(mob = read_mobile(mob_type, VIRTUAL))) {
        send_to_charf(ch, "ОШИБКА.\r\n");
        return;
    }
//log("ТТТ2");

//log("TTT2.1 %s",GET_NAME(mob));
//log("TTT2.2 %s",GET_OBJ_PNAME(obj,0));

//log("ТТТ2.3");
    char_to_room(mob, IN_ROOM(ch));
//log("ТТТ2.4");
    act_object_mess(spellnum, mob, obj, show_mess, TYPE_MESS_HIT);

//log("ТТТ2.5");
    extract_obj(obj);

    rnum = real_mobile(mob_num);

    i = GET_REAL_CON(mob_proto + rnum);
    GET_CON(mob) = i - 2;

    i = GET_SIZE(mob_proto + rnum);
    GET_SIZE(mob) = i;

    i = GET_SIZE_ADD(mob_proto + rnum);
    GET_SIZE_ADD(mob) = i;

    i = GET_LEVEL(mob_proto + rnum) - 2;
    mlevel = GET_LEVEL(mob) = MAX(min_level, MIN(i, max_level));
    add_class(mob, 0, 0, 0);
    add_class(mob, CLASS_WARRIOR, mlevel, 1);

    i = GET_WEIGHT(mob_proto + rnum);
    GET_WEIGHT(mob) = i;

    i = GET_WEIGHT_ADD(mob_proto + rnum);
    GET_WEIGHT_ADD(mob) = i;

    mob->player.description = NULL;

    mob->npc()->specials.damage = MAX(mlevel / 2, level / 5);
    mob->npc()->specials.damage2 = MAX(mlevel / 2, level / 5);

    recalc_params(mob);

//Указать что нежить
    GET_MOB_TYPE(mob) = TMOB_UNDEAD;
    GET_ALIGNMENT(mob) = -1000;
    mob->npc()->specials.vnum_corpse = -1;
    mob->npc()->specials.death_script = -1;
    mob->follow_vnum = 0;

    add_follower(mob, ch, FLW_UNDEAD);
    af.type = spellnum;
    af.duration = -1;
    af.modifier = 100;
    af.level = level;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    af.battleflag = 0;
    af.owner = GET_ID(ch);
    affect_to_char(mob, &af);

    GET_HIT(mob) = GET_REAL_MAX_HIT(mob);
    GET_MOVE(mob) = GET_REAL_MAX_MOVE(mob);
    clear_mob_specials(mob);
}

ASPELL(spell_unholy_blight)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int save_val, dam = 0, i, x, y, z, w, r = 0;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if (!may_kill_here(ch, victim))
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }

    if (!IS_UNDEAD(victim)) {
        //чума 1..5% на выносливость
        save_val = MAX(1, level / 30);
        if (save_val > 0 && !general_savingthrow_3(victim, SAV_FORT, save_val) &&
            !AFF_FLAGGED(victim, AFF_PLAGUE)) {
            af[0].type = find_spell_num(SPELL_BLIND);
            af[0].duration = MIN(75, MAX(3, level / 5));
            af[0].main = TRUE;
            af[0].bitvector = AFF_BLIND;
            af[0].battleflag = TRUE;
            af[0].modifier = level;
            af[0].owner = GET_ID(ch);
            r = 1;
        } else {                //отравление 1..30%
            save_val = MAX(1, level / 3);
            if (save_val > 0 && !general_savingthrow_3(victim, SAV_FORT, save_val) &&
                !AFF_FLAGGED(victim, AFF_POISON)) {
                af[0].type = find_spell_num(SPELL_POISON);
                af[0].duration = MIN(75, MAX(1, level / 5));
                af[0].bitvector = AFF_POISON;
                af[0].main = TRUE;
                af[0].modifier = level;
                af[0].battleflag = TRUE;
                af[0].owner = GET_ID(ch);
                r = 1;
            }
        }
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
    if (r == 1) {
        af[1].type = spellnum;
        af[1].duration = af[0].duration;
        af[1].location = APPLY_DEX;
        af[1].modifier = -MAX(2, af[0].modifier / 25);
        af[1].battleflag = af[0].battleflag;
        af[1].owner = af[0].owner;
        af[2].type = spellnum;
        af[2].duration = af[0].duration;
        af[2].location = APPLY_CON;
        af[2].modifier = af[1].modifier;
        af[2].owner = af[0].owner;
        af[2].battleflag = af[0].battleflag;
        af[3].type = spellnum;
        af[3].duration = af[0].duration;
        af[3].location = APPLY_INT;
        af[3].modifier = af[1].modifier;
        af[3].owner = af[0].owner;
        af[3].battleflag = af[0].battleflag;
        af[4].type = spellnum;
        af[4].duration = af[0].duration;
        af[4].location = APPLY_WIS;
        af[4].modifier = af[1].modifier;
        af[4].owner = af[0].owner;
        af[4].battleflag = af[0].battleflag;
        af[5].type = spellnum;
        af[5].duration = af[0].duration;
        af[5].location = APPLY_CHA;
        af[5].modifier = af[1].modifier;
        af[5].owner = af[0].owner;
        af[5].battleflag = af[0].battleflag;
        af[6].type = spellnum;
        af[6].duration = af[0].duration;
        af[6].location = APPLY_STR;
        af[6].modifier = af[1].modifier;
        af[6].owner = af[0].owner;
        af[6].battleflag = af[0].battleflag;
    }
//накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

// mag_damage(spellnum, dam, ch, victim, show_mess, Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);

}

ASPELL(spell_dispel_good)
{
    int dam = 0, x, y, z, w;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

//сами из себя зло не выгоняем
    if (ch == victim)
        return;

    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }

    if (IS_EVILS(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
        dam /= 2;
        GET_HIT(victim) = MIN(GET_HIT(victim) + dam, GET_REAL_MAX_HIT(victim));
    } else if (IS_NEUTRALS(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    } else {
        if (!may_kill_here(ch, victim))
            return;
        mag_damage(spellnum, dam, ch, victim, show_mess,
                   Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);
    }
}

ASPELL(spell_blasphemy)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i, dam = 0, x, y, z, w;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

//Жертва не может услышать слово
    /*if (GET_POS(victim) <= POS_SLEEPING || AFF_FLAGGED(victim,AFF_DEAFNESS))
       {
       if (show_mess)
       act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
       return;
       } */

    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }
    //1..5
    if (!general_savingthrow_3(victim, SAV_NEGATIVE, level / 30)) {
        //паралич,слабость,слепота
        //send_to_charf(ch,"паралич\r\n");
        af[0].type = find_spell_num(SPELL_HOLD);
        af[0].duration = MAX(1, level / 30);
        af[0].battleflag = TRUE;
        af[0].bitvector = AFF_HOLD;
        af[0].owner = GET_ID(ch);

        af[1].type = find_spell_num(SPELL_BLIND);
        af[1].duration = af[0].duration;
        af[1].modifier = level;
        af[1].battleflag = TRUE;
        af[1].bitvector = AFF_BLIND;
        af[1].owner = af[0].owner;

        af[2].type = spellnum;
        af[2].location = APPLY_STR;
        af[2].duration = af[0].duration;
        af[2].modifier = -MAX(1, level / 20);
        af[2].battleflag = TRUE;
        af[2].main = TRUE;
        af[2].owner = af[0].owner;
    } else                      //1..10%
    if (!general_savingthrow_3(victim, SAV_NEGATIVE, level / 15)) {     //жертва пропустила уровень/4 (37% маск) слабость,слепота
        //send_to_charf(ch,"слепота\r\n");
        af[0].type = find_spell_num(SPELL_BLIND);
        af[0].modifier = level;
        af[0].duration = MAX(1, level / 30) * PULSE_VIOLENCE;
        af[0].bitvector = AFF_BLIND;
        af[0].owner = GET_ID(ch);

        af[1].type = spellnum;
        af[1].location = APPLY_STR;
        af[1].duration = af[0].duration;
        af[1].modifier = -MAX(1, level / 20);
        af[1].main = TRUE;
        af[1].owner = af[0].owner;
    }                           //1..15%
    else if (!general_savingthrow_3(victim, SAV_NEGATIVE, level / 10)) {        //иначе слабость полюбому
        //send_to_charf(ch,"слабость\r\n");
        af[0].type = spellnum;
        af[0].duration = MAX(1, level / 30) * PULSE_VIOLENCE;
        af[0].location = APPLY_STR;
        af[0].modifier = -MAX(1, level / 20);
        af[0].main = TRUE;
        af[0].owner = GET_ID(ch);
    }

    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_unholy_aura)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if (IS_GOODS(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(2 * SECS_PER_MUD_TICK, (level / 6) * SECS_PER_MUD_TICK);
    af[0].bitvector = AFF_UNHOLYAURA;
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;
    af[1].type = af[0].type;
    af[1].duration = af[0].duration;
    af[1].location = APPLY_AC;
    af[1].modifier = MAX(1, level / 10);
    af[1].owner = af[0].owner;

    //если нейтрал то поправляем чуток
    if (IS_NEUTRALS(victim))
        af[1].modifier /= 3;

    //выводим сообщения
    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    //новый каст или апдейт

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_bull_strengh)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(SECS_PER_MUD_HOUR, (level / 20) * SECS_PER_MUD_HOUR);
    af[0].location = APPLY_STR;
    af[0].modifier = MAX(1, level / 45);
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);



    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_magic_immunity)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 10) * SECS_PER_MUD_TICK;
    af[0].location = APPLY_AC;
    af[0].modifier = 1;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);



    //выводим сообщения
    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }
}

ASPELL(spell_righteous_might)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(SECS_PER_MUD_HOUR, (level / 20) * SECS_PER_MUD_HOUR);
    af[0].location = APPLY_SIZE;
    af[0].modifier = MAX(5, (level / 4));
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);

    af[1].type = spellnum;
    af[1].duration = af[0].duration;
    af[1].location = APPLY_CHAR_WEIGHT;
    af[1].modifier = MAX(10, (level / 2));
    af[1].owner = GET_ID(ch);



    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }
}

ASPELL(spell_stone_skin)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(SECS_PER_MUD_HOUR, (level / 25) * SECS_PER_MUD_HOUR);
    af[0].bitvector = AFF_STONE_SKIN;
    af[0].modifier = MAX(3, (level / 8));
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);


    //выводим сообщения
    //act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_grasping_hand)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;


    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    i = MAX(5, level / 5);
    af[0].type = find_spell_num(SPELL_GRASP);
    af[0].duration = MAX(2, (level / 10));
    af[0].battleflag = TRUE;
    af[0].location = APPLY_AC;
    af[0].modifier = -i;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);

    i = MAX(10, level / 3);
    af[1].type = spellnum;
    af[1].duration = af[0].duration;
    af[1].battleflag = TRUE;
    af[1].location = APPLY_ARMOUR0;
    af[1].modifier = -i;
    af[1].owner = af[0].owner;

    af[2].type = spellnum;
    af[2].duration = af[0].duration;
    af[2].battleflag = TRUE;
    af[2].location = APPLY_ARMOUR1;
    af[2].modifier = af[1].modifier;
    af[2].owner = af[0].owner;

    af[3].type = spellnum;
    af[3].battleflag = TRUE;
    af[3].duration = af[0].duration;
    af[3].location = APPLY_ARMOUR2;
    af[3].modifier = af[1].modifier;
    af[3].owner = af[0].owner;


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    //выводим сообщения
    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_clenched_fist)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i, dam = 0, x, y, z, w;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(2, (level / 10));
    af[0].bitvector = AFF_NOTHING;
    af[0].battleflag = TRUE;
    af[0].main = TRUE;
    af[0].modifier = 1;
    af[0].owner = GET_ID(ch);



    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    //выводим сообщения
    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }

    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);
}


ASPELL(spell_crushing_fist)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i, dam = 0, x, y, z, w;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    i = MAX(3, level / 10);
    af[0].type = find_spell_num(SPELL_GRASP);
    af[0].duration = MAX(2, (level / 12));
    af[0].battleflag = TRUE;
    af[0].location = APPLY_AC;
    af[0].modifier = -i;
    af[0].owner = GET_ID(ch);

    i = MAX(5, level / 6);
    af[1].type = spellnum;
    af[1].duration = af[0].duration;
    af[1].location = APPLY_ARMOUR0;
    af[1].battleflag = af[0].battleflag;
    af[1].modifier = -i;
    af[1].main = FALSE;
    af[1].owner = af[0].owner;

    af[2].type = spellnum;
    af[2].duration = af[0].duration;
    af[2].location = APPLY_ARMOUR1;
    af[2].modifier = af[1].modifier;
    af[2].owner = af[0].owner;
    af[2].battleflag = af[0].battleflag;

    af[3].type = spellnum;
    af[3].duration = af[0].duration;
    af[3].location = APPLY_ARMOUR2;
    af[3].modifier = af[1].modifier;
    af[3].owner = af[0].owner;
    af[3].battleflag = af[0].battleflag;

    af[4].type = spellnum;
    af[4].duration = af[0].duration;
    af[4].bitvector = AFF_NOTHING;
    af[4].modifier = 1;
    af[4].main = TRUE;
    af[4].battleflag = af[0].battleflag;
    af[4].owner = GET_ID(ch);

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    //выводим сообщения
    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);


    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }

    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);
}


ASPELL(spell_fast_move)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 10) * SECS_PER_MUD_HOUR;
    af[0].bitvector = AFF_FASTER;
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    //выводим сообщения
    if (update_spell)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_locate_object)
{
    struct obj_data *fobj = NULL;
    char name[MAX_INPUT_LENGTH];
    int froom = -1, len, dist = MAX(100, level * 10);
    CPathRetDir dir;


    if (GET_OBJ_LOCATE(ch)) {
        act("Вы потеряли нить до @1р.", "Мп", ch, GET_OBJ_LOCATE(ch));
        GET_OBJ_LOCATE(ch) = 0;
        ch->char_specials.locate_step = 0;
    } else if (GET_CHAR_LOCATE(ch)) {
        act("Вы потеряли нить до $2р.", "Мм", ch, GET_CHAR_LOCATE(ch));
        GET_CHAR_LOCATE(ch) = 0;
        ch->char_specials.locate_step = 0;
    }

    strcpy(name, fname(cast_argument));

    ClearPathLabel();

    for (fobj = object_list; fobj; fobj = fobj->next) {
        if (fobj->carried_by
            && zone_table[world[IN_ROOM(ch)].zone].plane !=
            zone_table[world[IN_ROOM(fobj->carried_by)].zone].plane) {
//  send_to_charf(ch,"Предмет carried не в той зоне %d != %d\r\n",zone_table[world[IN_ROOM(ch)].zone].plane, zone_table[world[IN_ROOM(fobj->carried_by)].zone].plane);
//  log("*");
            continue;
        } else
            if (fobj->worn_by
                && zone_table[world[IN_ROOM(ch)].zone].plane !=
                zone_table[world[IN_ROOM(fobj->worn_by)].zone].plane) {
//  send_to_charf(ch,"Предмет worn не в той зоне %d != %d\r\n",zone_table[world[IN_ROOM(ch)].zone].plane, zone_table[world[IN_ROOM(fobj->worn_by)].zone].plane);
//  log("+");
            continue;
        } else
            if (!fobj->worn_by && !fobj->carried_by
                && zone_table[world[IN_ROOM(ch)].zone].plane !=
                zone_table[world[IN_ROOM(fobj)].zone].plane) {
//  send_to_charf(ch,"Предмет не в той зоне %d != %d\r\n",zone_table[world[IN_ROOM(ch)].zone].plane, zone_table[world[IN_ROOM(fobj)].zone].plane);
//  log("-");
            continue;
        }

        if (IN_ROOM(fobj) == NOWHERE)
            continue;

        if (SECT(IN_ROOM(fobj)) == SECT_SECRET)
            continue;

        if (!CAN_SEE_OBJ(ch, fobj))
            continue;

        if (IS_CORPSE(fobj))
            continue;

        //send_to_charf(ch,"Ищем %s == %s \r\n",name,fobj->name);

        /*нашли */
        if (isname(name, get_name_pad(fobj->names, PAD_IMN, PAD_OBJECT))) {
            if (fobj->carried_by && fobj->carried_by != ch) {
                if (general_savingthrow_3(fobj->carried_by, SAV_WILL, MAX(1, level / 3))) {
                    act("Перед Вами возник образ 1р, но тут же рассеялся.", "Мм", ch, fobj);
                    act("Вы почувствовали, что кто-то ищет @1в принадлежащ@1(ий,ию.ие,ии) Вам.",
                        "Мп", fobj->carried_by, fobj);
                    return;
                }
                if (affected_by_spell(fobj->carried_by, SPELL_MIND_BLANK) > level - number(1, 20)) {
                    act("Перед Вами возник образ 1р, но тут же рассеялся.", "Мм", ch, fobj);
                    act("Вы почувствовали, что кто-то ищет @1в принадлежащ@1(ий,ию.ие,ии) Вам.",
                        "Мп", fobj->carried_by, fobj);
                    return;
                }
                act("@1и находится у 2р.", "Ммп", ch, fobj->carried_by, fobj);
                return;
            } else if (fobj->worn_by && fobj->worn_by != ch) {
                if (general_savingthrow_3(fobj->worn_by, SAV_WILL, MAX(1, level / 3))) {
                    act("Перед Вами возник образ 1р, но тут же рассеялся.", "Мм", ch, fobj);
                    act("Вы почувствовали, что кто-то ищет @1в принадлежащ@1(ий,ию.ие,ии) Вам.",
                        "Мп", fobj->worn_by, fobj);
                    return;
                }
                if (affected_by_spell(fobj->worn_by, SPELL_MIND_BLANK) > level - number(1, 20)) {
                    act("Перед Вами возник образ 1р, но тут же рассеялся.", "Мм", ch, fobj);
                    act("Вы почувствовали, что кто-то ищет @1в принадлежащ@1(ий,ию.ие,ии) Вам.",
                        "Мп", fobj->worn_by, fobj);
                    return;
                }
                act("@1и одет@1(,а,о,ы) на 2п.", "Ммп", ch, fobj->worn_by, fobj);
                return;
            } else {
                SetPathLabel(IN_ROOM(fobj));
            }
        }
    }

    len = TracePath(IN_ROOM(ch), froom, 0, &dir, dist);
    if (len == -1) {
        send_to_charf(ch, "Возникающие образы перед Вашими глазами были слишком нечеткими.\r\n");
        return;
    } else {
        for (fobj = world[froom].contents; fobj; fobj = fobj->next_content)
            if (isname(name, get_name_pad(fobj->names, PAD_IMN, PAD_OBJECT))) {
                act("Едва видимая призрачная нить протянулась от Вас к @1д.", "Мп", ch, fobj);
                ch->char_specials.locate_step = MAX(5, level / 3);
                GET_OBJ_LOCATE(ch) = fobj;
                return;
            }
    }


    act("Этот предмет вне Вашей досигаемости.", FALSE, ch, 0, 0, TO_CHAR);
}

ASPELL(spell_levitation)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 20) * SECS_PER_MUD_HOUR;
    af[0].bitvector = AFF_LEVIT;
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    if (update_spell)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    if (GET_POS(victim) > POS_SITTING)
        GET_POS(victim) = POS_FLYING;
}

ASPELL(spell_locate_person)
{
    struct char_data *tch;
    char name[MAX_INPUT_LENGTH];
    int froom = -1, len, dist = MAX(100, level * 10), found = FALSE;
    CPathRetDir dir;

    if (GET_OBJ_LOCATE(ch)) {
        act("Вы потеряли нить до @1р.", "Мп", ch, GET_OBJ_LOCATE(ch));
        GET_OBJ_LOCATE(ch) = 0;
        ch->char_specials.locate_step = 0;
    } else if (GET_CHAR_LOCATE(ch)) {
        act("Вы потеряли нить до $2р.", "Мм", ch, GET_CHAR_LOCATE(ch));
        GET_CHAR_LOCATE(ch) = 0;
        ch->char_specials.locate_step = 0;
    }

    strcpy(name, fname(cast_argument));

    ClearPathLabel();

    for (tch = character_list; tch; tch = tch->next) {
        if (IN_ROOM(tch) == NOWHERE)
            continue;

        if (zone_table[world[IN_ROOM(ch)].zone].plane != zone_table[world[IN_ROOM(tch)].zone].plane)
            continue;

        if (!CAN_SEE(ch, tch))
            continue;

        if (number(1, affected_by_spell(tch, SPELL_MIND_BLANK)) >= level)
            continue;

        if (isname(name, tch->player.PNames[0]))
            SetPathLabel(IN_ROOM(tch));
    }

    len = TracePath(IN_ROOM(ch), froom, 0, &dir, dist);
    if (len == -1) {
        send_to_charf(ch, "Возникающие образы перед Вашими глазами были слишком нечеткими.\r\n");
        return;
    } else {
        for (tch = world[froom].people; tch; tch = tch->next_in_room)
            if (isname(name, tch->player.name)) {
                found = TRUE;
                if (!same_group(ch, tch) && general_savingthrow_3(tch, SAV_WILL, MAX(1, level / 3))) {  //30%
                    act("Перед Вами возник образ 2р, но тут же рассеялся.", "Мм", ch, tch);
                    act("Вы почувствовали, что кто-то хочет узнать Ваше местоположение.", "М", tch);
                    break;
                }

                if (!same_group(ch, tch)
                    && affected_by_spell(tch, SPELL_MIND_BLANK) > level - number(1, 20)) {
                    act("Перед Вами возник образ 2р, но тут же рассеялся.", "Мм", ch, tch);
                    act("Вы почувствовали, что кто-то хочет узнать Ваше местоположение.", "М", tch);
                    break;
                }

                GET_CHAR_LOCATE(ch) = tch;
                act("Едва видимая призрачная нить протянулась от Вас к 2д.", "Ммч", ch, tch, len);
                ch->char_specials.locate_step = MAX(5, level / 3);
                break;
            }
    }

    if (!found) {
        ClearPathLabel();
        for (tch = character_list; tch; tch = tch->next) {
            if (IN_ROOM(tch) == NOWHERE)
                continue;
            if (zone_table[world[IN_ROOM(ch)].zone].plane ==
                zone_table[world[IN_ROOM(tch)].zone].plane)
                continue;
            if (!CAN_SEE(ch, tch))
                continue;
            if (number(1, affected_by_spell(tch, SPELL_MIND_BLANK)) >= level)
                continue;
            if (isname(name, tch->player.PNames[0]))
                SetPathLabel(IN_ROOM(tch));
        }
        len = TracePath(IN_ROOM(ch), froom, 0, &dir, dist);
        if (len == -1) {
            send_to_charf(ch,
                          "Возникающие образы перед Вашими глазами были слишком нечеткими.\r\n");
            return;
        } else {
            for (tch = world[froom].people; tch; tch = tch->next_in_room)
                if (isname(name, tch->player.name)) {
                    found = TRUE;
                    if (general_savingthrow_3(tch, SAV_WILL, MAX(1, level / 3))) {      //30%
                        act("Перед Вами возник образ 2р, но тут же рассеялся.", "Мм", ch, tch);
                        act("Вы почувствовали, что кто-то хочет узнать Ваше местоположение.", "М",
                            tch);
                        break;
                    }
                    if (affected_by_spell(tch, SPELL_MIND_BLANK) > level - number(1, 20)) {
                        act("Перед Вами возник образ 2р, но тут же рассеялся.", "Мм", ch, tch);
                        act("Вы почувствовали, что кто-то хочет узнать Ваше местоположение.", "М",
                            tch);
                        break;
                    }
                    GET_CHAR_LOCATE(ch) = tch;
                    act("Едва видимая призрачная нить протянулась от Вас к 2д.", "Ммч", ch, tch,
                        len);
                    ch->char_specials.locate_step = MAX(5, level / 3);
                    break;
                }
        }
    }

    if (!found)
        send_to_charf(ch, "Возникающие образы перед Вашими глазами были слишком нечеткими.\r\n");
}

ASPELL(spell_fly)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 4) * SECS_PER_MUD_TICK;
    af[0].bitvector = AFF_FLY;
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    if (update_spell)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    GET_POS(victim) = POS_FLYING;
}

ASPELL(spell_teleport)
{
    int loc_vnum = 0, r_num = -1, fnd_room = -1, i, inroom = IN_ROOM(ch);
    int percent = 0, groups = 0;
    char arg[MAX_INPUT_LENGTH], *argument;
    struct char_data *k;
    struct follow_type *f, *l;

    strcpy(arg, cast_argument);
    argument = arg;

    skip_spaces(&argument);

    if (!*argument || !argument) {
        send_to_charf(ch, "Укажите место куда Вы хотите переместится.\r\n");
        return;
    }

    if (!is_positive_number(arg)) {
        for (i = 0; i <= GET_MAXMEM(ch); i++) {
            r_num = real_room(GET_MEMORY(ch, i));
            if (r_num != NOWHERE && GET_TIMEMEM(ch, i)) {
                if (GET_DESCMEM(ch, i) && isname(argument, GET_DESCMEM(ch, i))) {
                    loc_vnum = GET_MEMORY(ch, i);
                    percent = GET_TIMEMEM(ch, i);
                    break;
                } else if (isname(argument, get_name_pad(world[r_num].name, 0, PAD_OBJECT))) {
                    loc_vnum = GET_MEMORY(ch, i);
                    percent = GET_TIMEMEM(ch, i);
                    break;
                }
            }
        }
    } else {
        i = atoi(argument);
        r_num = real_room(GET_MEMORY(ch, i));
        if (r_num != NOWHERE && GET_TIMEMEM(ch, i)) {
            loc_vnum = GET_MEMORY(ch, i);
            percent = GET_TIMEMEM(ch, i);
        }
    }

    if (!loc_vnum || r_num == NOWHERE) {
        send_to_charf(ch, "Это место присутствует только в Вашем воображении.\r\n");
        return;
    }

    k = (ch->master ? ch->master : ch);
    for (f = k->followers; f; f = f->next) {
        if (IS_AFFECTED(f->follower, AFF_CHARM) && IS_AFFECTED(f->follower, AFF_HELPER))
            groups += 10;
    }

    k = (ch->party_leader ? ch->party_leader : ch);
    for (f = k->party; f; f = f->next) {
        if (!same_group(ch, f->follower))
            continue;

        if (!IS_AFFECTED(f->follower, AFF_GROUP))
            continue;

        groups += 10;
    }

    percent = (100 * percent) / GET_MAXTIME(ch);
    percent = (percent + MAX(50, level)) / 2;

    percent -= groups;

//send_to_charf(ch,"Вас переместят в локацию %d процент %d %d\r\n",loc_vnum,percent,level);
    if (percent < 15) {
        act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_FAIL);
        return;
    } else if (percent < 45) {
        for (i = 0; i < MAX_SUMMON_TRIES; i++) {
            fnd_room = number(0, top_of_world);
            if (world[r_num].zone == world[fnd_room].zone &&
                SECT(fnd_room) != SECT_SECRET &&
                !ROOM_FLAGGED(fnd_room, ROOM_DEATH | ROOM_TUNNEL) &&
                !ROOM_FLAGGED(fnd_room, ROOM_NOTELEPORT) &&
                !ROOM_FLAGGED(fnd_room, ROOM_FLYDEATH) &&
                (!ROOM_FLAGGED(fnd_room, ROOM_GODROOM) || IS_IMMORTAL(victim)) &&
                (!ROOM_FLAGGED(fnd_room, ROOM_PRIVATE)))
                break;
        }
        if (i >= MAX_SUMMON_TRIES) {
            act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_FAIL);
            return;
        }
        r_num = fnd_room;
    }

    act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_HIT);
    char_from_room(ch);
    char_to_room(ch, r_num);
    check_horse(ch);
    look_at_room(ch, TRUE);
    act_affect_mess(spellnum, ch, ch, show_mess, TYPE_MESS_GOD);

    k = (ch->master ? ch->master : ch);


    if (k != ch && inroom == IN_ROOM(k) && same_group(ch, k)) {
        act_affect_mess(spellnum, k, 0, show_mess, TYPE_MESS_HIT);
        char_from_room(k);
        char_to_room(k, r_num);
        check_horse(k);
        look_at_room(k, TRUE);
        act_affect_mess(spellnum, ch, k, show_mess, TYPE_MESS_GOD);
    }


    for (f = k->followers; f; f = f->next) {
        if (inroom != IN_ROOM(f->follower))
            continue;

        if (IS_AFFECTED(f->follower, AFF_CHARM) || f->type == FLW_CHARM
            || IS_AFFECTED(f->follower, AFF_HELPER)) {
            act_affect_mess(spellnum, f->follower, 0, show_mess, TYPE_MESS_HIT);
            char_from_room(f->follower);
            char_to_room(f->follower, r_num);
            check_horse(f->follower);
            look_at_room(f->follower, 0);
            act_affect_mess(spellnum, ch, f->follower, show_mess, TYPE_MESS_GOD);
            for (l = f->follower->followers; l; l = l->next) {
                if (inroom != IN_ROOM(l->follower))
                    continue;

                if (!AFF_FLAGGED(l->follower, AFF_GROUP) ||
                    ch == l->follower || (IS_NPC(l->follower) &&
                                          !AFF_FLAGGED(l->follower, AFF_CHARM)))
                    continue;
                act_affect_mess(spellnum, l->follower, 0, show_mess, TYPE_MESS_HIT);

                char_from_room(l->follower);
                char_to_room(l->follower, r_num);
                check_horse(l->follower);
                look_at_room(l->follower, TRUE);
                act_affect_mess(spellnum, ch, l->follower, show_mess, TYPE_MESS_GOD);
            }
        }
    }

    k = (ch->party_leader ? ch->party_leader : ch);


    if (k != ch && inroom == IN_ROOM(k) && same_group(ch, k)) {
        act_affect_mess(spellnum, k, 0, show_mess, TYPE_MESS_HIT);
        char_from_room(k);
        char_to_room(k, r_num);
        check_horse(k);
        look_at_room(k, TRUE);
        act_affect_mess(spellnum, ch, k, show_mess, TYPE_MESS_GOD);
    }


    for (f = k->party; f; f = f->next) {
        if (inroom != IN_ROOM(f->follower))
            continue;

        if (!same_group(ch, f->follower))
            continue;

        if (IS_AFFECTED(f->follower, AFF_GROUP) || f->type == FLW_CHARM
            || IS_AFFECTED(f->follower, AFF_HELPER)) {
            act_affect_mess(spellnum, f->follower, 0, show_mess, TYPE_MESS_HIT);
            char_from_room(f->follower);
            char_to_room(f->follower, r_num);
            check_horse(f->follower);
            look_at_room(f->follower, TRUE);
            act_affect_mess(spellnum, ch, f->follower, show_mess, TYPE_MESS_GOD);
            for (l = f->follower->followers; l; l = l->next) {
                if (inroom != IN_ROOM(l->follower))
                    continue;

                if (!AFF_FLAGGED(l->follower, AFF_GROUP) ||
                    ch == l->follower || (IS_NPC(l->follower) &&
                                          !AFF_FLAGGED(l->follower, AFF_CHARM)))
                    continue;
                act_affect_mess(spellnum, l->follower, 0, show_mess, TYPE_MESS_HIT);
                char_from_room(l->follower);
                char_to_room(l->follower, r_num);
                check_horse(l->follower);
                look_at_room(l->follower, TRUE);
                act_affect_mess(spellnum, ch, l->follower, show_mess, TYPE_MESS_GOD);
            }
        }
    }


}

ASPELL(spell_dimension_door)
{
    int numb = 0, dir, i, room = IN_ROOM(ch), room_dir = NOWHERE, maxdir = MAX(2, level / 10);
    bool fnd = TRUE;
    char *arg = NULL, *argument;
    char argbuf[MAX_STRING_LENGTH];

    strcpy(argbuf, cast_argument);
    argument = argbuf;

    skip_spaces(&argument);

    if (!*argument || !argument) {
        send_to_charf(ch, "Укажите куда Вы хотите переместится.\r\n");
        return;
    }

    /* Выделяем часть для числа и направления */
    if ((arg = strchr(argument, '.')) != NULL) {
        *(arg++) = '\0';
        while (a_isspace(*arg))
            arg++;
    } else {
        send_to_charf(ch, "Укажите количество шагов в нужном направлении.\r\n");
        return;
    }

    if (!is_positive_number(argument)) {
        send_to_charf(ch, "Укажите количество шагов в нужном направлении.\r\n");
        return;
    }

    numb = atoi(argument);
    if ((dir = search_block(arg, dirs, FALSE)) == -1 &&
        (dir = search_block(arg, DirIs, FALSE)) == -1) {
        send_to_charf(ch, "Неизвествное направление.\r\n");
        return;
    }



    if (EXITDATA(room, dir) && EXITDATA(room, dir)->to_room != NOWHERE) {
        room_dir = room;
        for (i = 1; i <= numb && fnd; i++) {
            room_dir = EXITDATA(room_dir, dir)->to_room;
            /* Перебор */
            if ((!EXITDATA(room_dir, dir) || EXITDATA(room_dir, dir)->to_room == NOWHERE)
                && i != numb) {
                //send_to_charf(ch,"Аварийный выход %d != %d\r\n",i,numb);
                fnd = FALSE;
                break;
            }
            if (i >= maxdir)
                break;
        }

        if (!fnd || room_dir == NOWHERE) {
            act_affect_mess(spellnum, ch, ch, show_mess, TYPE_MESS_FAIL);
            return;
        }

        act_affect_mess(spellnum, ch, ch, show_mess, TYPE_MESS_HIT);
        char_from_room(ch);

        act_affect_mess(spellnum, ch, ch, show_mess, TYPE_MESS_GOD);
        char_to_room(ch, room_dir);
        look_at_room(ch, FALSE);
    } else {
        send_to_charf(ch, "Чтобы переместиться, Вы должны видеть место.\r\n");
        act("Быстро заморгав, $n дернулся как от удара.", TRUE, ch, 0, 0, TO_ROOM);
    }
}


ASPELL(spell_astral_project)
{
    ACMD(do_sleep);
    struct char_data *mob = NULL;
    mob_vnum mob_num = 199;
    char buf2[MAX_STRING_LENGTH];

    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE))
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d", &mob_num);


    if (!(mob = read_mobile(-mob_num, VIRTUAL))) {
        send_to_charf(ch, "Ошибка #sp7801. Срочно сообщите Богам.\r\n", ch);
        return;
    }

    sprintf(buf2, "%s", GET_NAME(ch));
    mob->player.name = str_dup(buf2);

    sprintf(buf2, "%s", GET_PAD(ch, 0));
    mob->player.short_descr = str_dup(buf2);

    mob->player.long_descr = NULL;
    sprintf(buf2, "%s", GET_PAD(ch, 0));
    GET_PAD(mob, 0) = str_dup(buf2);
    sprintf(buf2, "%s", GET_PAD(ch, 1));
    GET_PAD(mob, 1) = str_dup(buf2);
    sprintf(buf2, "%s", GET_PAD(ch, 2));
    GET_PAD(mob, 2) = str_dup(buf2);
    sprintf(buf2, "%s", GET_PAD(ch, 3));
    GET_PAD(mob, 3) = str_dup(buf2);
    sprintf(buf2, "%s", GET_PAD(ch, 4));
    GET_PAD(mob, 4) = str_dup(buf2);
    sprintf(buf2, "%s", GET_PAD(ch, 5));
    GET_PAD(mob, 5) = str_dup(buf2);

    GET_STR(mob) = GET_STR(ch);
    GET_INT(mob) = GET_INT(ch);
    GET_WIS(mob) = GET_WIS(ch);
    GET_DEX(mob) = GET_DEX(ch);
    GET_CON(mob) = GET_CON(ch);
    GET_CHA(mob) = GET_CHA(ch);

    GET_LEVEL(mob) = GET_LEVEL(ch);
    GET_HR(mob) = GET_HR(ch);
    GET_AC(mob) = GET_AC(ch);
    GET_DR(mob) = GET_DR(ch);

    GET_MAX_HIT(mob) = MAX(1, GET_MAX_HIT(ch) / 2);
    GET_HIT(mob) = MAX(1, GET_HIT(ch) / 2);
    mob->npc()->specials.damnodice = 0;
    mob->npc()->specials.damsizedice = 0;
    GET_GOLD(mob) = 0;
    GET_GOLD_NoDs(mob) = 0;
    GET_GOLD_SiDs(mob) = 0;
    GET_EXP(mob) = 0;

    GET_POS(mob) = POS_STANDING;
    GET_DEFAULT_POS(mob) = POS_STANDING;
    GET_SEX(mob) = GET_SEX(ch);

    GET_WEIGHT(mob) = GET_WEIGHT(ch);
    GET_HEIGHT(mob) = GET_HEIGHT(ch);
    GET_SIZE(mob) = GET_SIZE(ch);
    SET_BIT(MOB_FLAGS(mob, MOB_CLONE), MOB_CLONE);
    SET_BIT(MOB_FLAGS(mob, MOB_WALKER), MOB_WALKER);
    mob->npc()->specials.vnum_corpse = -1;
    mob->npc()->specials.death_script = -1;
    EXTRACT_TIMER(mob) = level * SECS_PER_MUD_DAY;

    char_to_room(mob, ch->in_room);

    act_affect_mess(spellnum, ch, mob, show_mess, TYPE_MESS_HIT);
    do_sleep(ch, 0, 0, 0, 1);
#if 0
    if (IS_SET(PRF_FLAGS(ch, PRF_BRIEF), PRF_BRIEF))
        SET_BIT(PRF_FLAGS(mob, PRF_BRIEF), PRF_BRIEF);
    if (IS_SET(PRF_FLAGS(ch, PRF_COMPACT), PRF_COMPACT))
        SET_BIT(PRF_FLAGS(mob, PRF_COMPACT), PRF_COMPACT);
    if (IS_SET(PRF_FLAGS(ch, PRF_NOHOLLER), PRF_NOHOLLER))
        SET_BIT(PRF_FLAGS(mob, PRF_NOHOLLER), PRF_NOHOLLER);
    if (IS_SET(PRF_FLAGS(ch, PRF_NOTELL), PRF_NOTELL))
        SET_BIT(PRF_FLAGS(mob, PRF_NOTELL), PRF_NOTELL);
    if (IS_SET(PRF_FLAGS(ch, PRF_DISPHP), PRF_DISPHP))
        SET_BIT(PRF_FLAGS(mob, PRF_DISPHP), PRF_DISPHP);
    if (IS_SET(PRF_FLAGS(ch, PRF_DISPMANA), PRF_DISPMANA))
        SET_BIT(PRF_FLAGS(mob, PRF_DISPMANA), PRF_DISPMANA);
    if (IS_SET(PRF_FLAGS(ch, PRF_DISPMOVE), PRF_DISPMOVE))
        SET_BIT(PRF_FLAGS(mob, PRF_DISPMOVE), PRF_DISPMOVE);
    if (IS_SET(PRF_FLAGS(ch, PRF_AUTOEXIT), PRF_AUTOEXIT))
        SET_BIT(PRF_FLAGS(mob, PRF_AUTOEXIT), PRF_AUTOEXIT);
    if (IS_SET(PRF_FLAGS(ch, PRF_NOREPEAT), PRF_NOREPEAT))
        SET_BIT(PRF_FLAGS(mob, PRF_NOREPEAT), PRF_NOREPEAT);
    if (IS_SET(PRF_FLAGS(ch, PRF_HOLYLIGHT), PRF_HOLYLIGHT))
        SET_BIT(PRF_FLAGS(mob, PRF_HOLYLIGHT), PRF_HOLYLIGHT);
    if (IS_SET(PRF_FLAGS(ch, PRF_COLOR_1), PRF_COLOR_1))
        SET_BIT(PRF_FLAGS(mob, PRF_COLOR_1), PRF_COLOR_1);
    if (IS_SET(PRF_FLAGS(ch, PRF_COLOR_2), PRF_COLOR_2))
        SET_BIT(PRF_FLAGS(mob, PRF_COLOR_2), PRF_COLOR_2);
    if (IS_SET(PRF_FLAGS(ch, PRF_CURSES), PRF_CURSES))
        SET_BIT(PRF_FLAGS(mob, PRF_CURSES), PRF_CURSES);
    if (IS_SET(PRF_FLAGS(ch, PRF_NOGOSS), PRF_NOGOSS))
        SET_BIT(PRF_FLAGS(mob, PRF_NOGOSS), PRF_NOGOSS);
    if (IS_SET(PRF_FLAGS(ch, PRF_NOGRATZ), PRF_NOGRATZ))
        SET_BIT(PRF_FLAGS(mob, PRF_NOGRATZ), PRF_NOGRATZ);
//if (IS_SET(PRF_FLAGS(ch,PRF_ROOMFLAGS),PRF_ROOMFLAGS))
    REMOVE_BIT(PRF_FLAGS(mob, PRF_ROOMFLAGS), PRF_ROOMFLAGS);
    if (IS_SET(PRF_FLAGS(ch, PRF_DISPEXP), PRF_DISPEXP))
        SET_BIT(PRF_FLAGS(mob, PRF_DISPEXP), PRF_DISPEXP);
    if (IS_SET(PRF_FLAGS(ch, PRF_DISPEXITS), PRF_DISPEXITS))
        SET_BIT(PRF_FLAGS(mob, PRF_DISPEXITS), PRF_DISPEXITS);
    if (IS_SET(PRF_FLAGS(ch, PRF_DISPLEVEL), PRF_DISPLEVEL))
        SET_BIT(PRF_FLAGS(mob, PRF_DISPLEVEL), PRF_DISPLEVEL);
    if (IS_SET(PRF_FLAGS(ch, PRF_DISPGOLD), PRF_DISPGOLD))
        SET_BIT(PRF_FLAGS(mob, PRF_DISPGOLD), PRF_DISPGOLD);
    if (IS_SET(PRF_FLAGS(ch, PRF_NOSHOUT), PRF_NOSHOUT))
        SET_BIT(PRF_FLAGS(mob, PRF_NOSHOUT), PRF_NOSHOUT);
    if (IS_SET(PRF_FLAGS(ch, PRF_AUTOFRM), PRF_AUTOFRM))
        SET_BIT(PRF_FLAGS(mob, PRF_AUTOFRM), PRF_AUTOFRM);
    if (IS_SET(PRF_FLAGS(ch, PRF_MOBILES), PRF_MOBILES))
        SET_BIT(PRF_FLAGS(mob, PRF_MOBILES), PRF_MOBILES);
    if (IS_SET(PRF_FLAGS(ch, PRF_OBJECTS), PRF_OBJECTS))
        SET_BIT(PRF_FLAGS(mob, PRF_OBJECTS), PRF_OBJECTS);
    if (IS_SET(PRF_FLAGS(ch, PRF_DISPBOI), PRF_DISPBOI))
        SET_BIT(PRF_FLAGS(mob, PRF_DISPBOI), PRF_DISPBOI);
    if (IS_SET(PRF_FLAGS(ch, PRF_SHOWKILL), PRF_SHOWKILL))
        SET_BIT(PRF_FLAGS(mob, PRF_SHOWKILL), PRF_SHOWKILL);
    if (IS_SET(PRF_FLAGS(ch, PRF_SHOWMEGA), PRF_SHOWMEGA))
        SET_BIT(PRF_FLAGS(mob, PRF_SHOWMEGA), PRF_SHOWMEGA);
    if (IS_SET(PRF_FLAGS(ch, PRF_THEME), PRF_THEME))
        SET_BIT(PRF_FLAGS(mob, PRF_THEME), PRF_THEME);
#endif

    strcpy(mob->divd, ch->divd);
    strcpy(mob->divr, ch->divr);

    ch->desc->character = mob;
    ch->desc->original = ch;

    mob->desc = ch->desc;
//ch->desc = NULL;

}

ASPELL(spell_free_moves)
{

    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if (affected_by_spell(victim, SPELL_STUNE) ||
        affected_by_spell(victim, SPELL_HOLD) || IS_AFFECTED(victim, AFF_HOLD))
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 4) * SECS_PER_MUD_TICK;
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = MAX(1, level / 2);
    af[0].main = TRUE;

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    if (update_spell)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

}


ASPELL(spell_sanctuary)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 8) * SECS_PER_MUD_TICK;
    af[0].battleflag = FALSE;
    af[0].bitvector = AFF_SANCTUARY;
    af[0].owner = GET_ID(ch);
    af[0].modifier = level;
    af[0].main = TRUE;


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    if (update_spell)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}


ASPELL(spell_oshield)
{

    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 20) * SECS_PER_MUD_TICK;
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].location = APPLY_SAVING_REFL;
    af[0].modifier = MAX(1, level / 10);
    af[0].main = TRUE;

    af[1].type = spellnum;
    af[1].duration = af[0].duration;
    af[1].owner = af[0].owner;
    af[1].location = APPLY_AC;
    af[1].modifier = MAX(2, level / 12);


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    if (update_spell)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_imun_elements)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    if (what_sky == SAV_FIRE) {
        af[0].type = find_spell_num(SPELL_IMM_FIRE);
        af[0].duration = MAX(1, level / 20) * SECS_PER_MUD_TICK;
        af[0].location = APPLY_SAVING_FIRE;
        af[0].modifier = 101;
        af[0].main = TRUE;
        af[0].owner = GET_ID(ch);
    } else if (what_sky == SAV_COLD) {
        af[0].type = find_spell_num(SPELL_IMM_COLD);
        af[0].duration = MAX(1, level / 20) * SECS_PER_MUD_TICK;
        af[0].location = APPLY_SAVING_COLD;
        af[0].modifier = 101;
        af[0].main = TRUE;
        af[0].owner = GET_ID(ch);
    } else if (what_sky == SAV_ELECTRO) {
        af[0].type = find_spell_num(SPELL_IMM_ELEC);
        af[0].duration = MAX(1, level / 20) * SECS_PER_MUD_TICK;
        af[0].location = APPLY_SAVING_ELECTRO;
        af[0].modifier = 101;
        af[0].main = TRUE;
        af[0].owner = GET_ID(ch);
    } else if (what_sky == SAV_ACID) {
        af[0].type = find_spell_num(SPELL_IMM_ACID);
        af[0].duration = MAX(1, level / 20) * SECS_PER_MUD_TICK;
        af[0].location = APPLY_SAVING_ACID;
        af[0].modifier = 101;
        af[0].main = TRUE;
        af[0].owner = GET_ID(ch);
    }

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }


}

ASPELL(spell_save_will)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 12) * SECS_PER_MUD_TICK;
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].location = APPLY_SAVING_WILL;
    af[0].modifier = MAX(1, level / 9);
    af[0].main = TRUE;


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    if (update_spell)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}


ASPELL(spell_nomagic_field)
{
    struct room_affect_data af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    bool accum_affect = FALSE, accum_duration = FALSE;
    int in_room = IN_ROOM(ch), i;
    struct room_data *room;

    if (ch == NULL || in_room == NOWHERE)
        return;

    room = &world[in_room];

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].owner = 0;
        af[i].duration = 0;
    }

    af[0].bitvector = ROOM_AFF_NOMAGIC;
    af[0].duration = MAX(1, level / 10) * SECS_PER_MUD_ROUND;
    af[0].modifier = level;
    af[0].owner = GET_ID(ch);

    if (affected_room_by_spell_real(room, spellnum))
        update_spell = TRUE;
    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
        if (af[i].bitvector) {
            if (update_spell)
                affect_join_fspell_room(room, af + i);
            else
                affect_join_room(room, af + i, accum_duration, FALSE, accum_affect, FALSE);
        }


    act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_repulsion)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 5) * SECS_PER_MUD_TICK;
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = level;
    af[0].main = TRUE;


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}


ASPELL(spell_mind_blank)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if (!same_group(ch, victim)) {
        send_to_char(ONLYSAME, ch);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(10, level / 3) * SECS_PER_MUD_TICK;
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = level;
    af[0].main = TRUE;
    af[1].type = af[0].type;
    af[1].location = APPLY_INT;
    af[1].modifier = -1;
    af[1].owner = af[0].owner;
    af[2].type = af[0].type;
    af[2].location = APPLY_WIS;
    af[2].modifier = -1;
    af[2].owner = af[0].owner;

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_prisma_sphere)
{
    struct room_affect_data af[MAX_SPELL_AFFECTS];
    int in_room = IN_ROOM(ch), i;
    struct room_data *room;
    struct char_data *tch;

    if (ch == NULL || in_room == NOWHERE)
        return;

    room = &world[in_room];

    if (affected_room_by_spell_real(room, spellnum)) {
        act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_FAIL);
        return;
    }
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].owner = 0;
        af[i].duration = 0;
    }

    af[0].bitvector = ROOM_AFF_PSPHERE;
    af[0].duration = MAX(3, level / 8) * SECS_PER_MUD_ROUND;
    af[0].modifier = level * 3;
    af[0].owner = GET_ID(ch);

    af[1].bitvector = ROOM_AFF_PRISMA_HITS;
    af[1].duration = af[0].duration;
    af[1].modifier = af[0].modifier * 3;
    af[1].owner = af[0].owner;

    af[2].bitvector = ROOM_AFF_PRISMA_FIRE;
    af[2].duration = af[0].duration;
    af[2].modifier = af[0].modifier;
    af[2].owner = af[0].owner;

    af[3].bitvector = ROOM_AFF_PRISMA_COLD;
    af[3].duration = af[0].duration;
    af[3].modifier = af[0].modifier;
    af[3].owner = af[0].owner;

    af[4].bitvector = ROOM_AFF_PRISMA_ELEC;
    af[4].duration = af[0].duration;
    af[4].modifier = af[0].modifier;
    af[4].owner = af[0].owner;

    af[5].bitvector = ROOM_AFF_PRISMA_ACID;
    af[5].duration = af[0].duration;
    af[5].modifier = af[0].modifier;
    af[5].owner = af[0].owner;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
        if (af[i].bitvector)
            affect_join_fspell_room(room, af + i);
    for (tch = world[in_room].people; tch; tch = tch->next_in_room)
        if (same_group(tch, ch) || tch == ch)
            add_psphere_char(tch, room);

    act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_entropic_shield)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 20) * SECS_PER_MUD_HOUR;
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = level;
    af[0].main = TRUE;


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    if (update_spell)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_fpantacle)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;


    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(2, level / 4) * SECS_PER_MUD_TICK;
    af[0].location = APPLY_LACKY;
    af[0].owner = GET_ID(ch);
    af[0].modifier = MAX(2, level / 4);
    af[0].main = TRUE;
    af[1].type = af[0].type;
    af[1].location = APPLY_DEX;
    af[1].duration = af[0].duration;
    af[1].modifier = MAX(1, level / 25);
    af[1].owner = af[0].owner;


    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_magic_parry)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(3, level / 5) * SECS_PER_MUD_TICK;
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = level;
    af[0].main = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}


ASPELL(spell_expand_skill)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 25) * SECS_PER_MUD_HOUR;
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = what_sky;
    af[0].main = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_evil_fate)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(4, level / 8);
    af[0].battleflag = TRUE;
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = level;
    af[0].main = TRUE;
    /*  af[1].type   = af[0].type;
       af[1].duration  = af[0].duration;
       af[1].owner     = af[0].owner;
       af[1].location  = APPLY_LACKY;
       af[1].modifier  = -MAX(1,level/25); */

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_fast_learn)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 15) * SECS_PER_MUD_HOUR;
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = level;
    af[0].main = TRUE;


    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_mislead)
{
    struct char_data *mob = NULL;
    mob_vnum mob_num = 198;
    char buf2[MAX_STRING_LENGTH];

    /*if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE))
       sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(),"%d",&mob_num); */

    if (!(mob = read_mobile(-mob_num, VIRTUAL))) {
        send_to_charf(ch, "Ошибка #sp7801. Срочно сообщите Богам.\r\n", ch);
        return;
    }

    sprintf(buf2, "%s", GET_NAME(ch));
    mob->player.name = str_dup(buf2);
    sprintf(buf2, "%s", GET_PAD(ch, 0));
    mob->player.short_descr = str_dup(buf2);
    mob->player.long_descr = NULL;

    sprintf(buf2, "%s", GET_PAD(ch, 0));
    GET_PAD(mob, 0) = str_dup(buf2);
    sprintf(buf2, "%s", GET_PAD(ch, 1));
    GET_PAD(mob, 1) = str_dup(buf2);
    sprintf(buf2, "%s", GET_PAD(ch, 2));
    GET_PAD(mob, 2) = str_dup(buf2);
    sprintf(buf2, "%s", GET_PAD(ch, 3));
    GET_PAD(mob, 3) = str_dup(buf2);
    sprintf(buf2, "%s", GET_PAD(ch, 4));
    GET_PAD(mob, 4) = str_dup(buf2);
    sprintf(buf2, "%s", GET_PAD(ch, 5));
    GET_PAD(mob, 5) = str_dup(buf2);
    mob->player.description = NULL;
    if (GET_TITLE(mob))
        free(GET_TITLE(mob));
    GET_TITLE(mob) = str_dup(GET_TITLE(ch));

    GET_STR(mob) = GET_STR(ch);
    GET_INT(mob) = GET_INT(ch);
    GET_WIS(mob) = GET_WIS(ch);
    GET_DEX(mob) = GET_DEX(ch);
    GET_CON(mob) = GET_CON(ch);
    GET_CHA(mob) = GET_CHA(ch);

    GET_LEVEL(mob) = GET_LEVEL(ch);
    GET_HR(mob) = GET_HR(ch);
    GET_AC(mob) = GET_AC(ch);
    GET_DR(mob) = GET_DR(ch);

    GET_MAX_HIT(mob) = MAX(1, GET_MAX_HIT(ch) / 2);
    GET_HIT(mob) = MAX(1, GET_HIT(ch) / 2);
    mob->npc()->specials.damnodice = 0;
    mob->npc()->specials.damsizedice = 0;
    GET_GOLD(mob) = 0;
    GET_GOLD_NoDs(mob) = 0;
    GET_GOLD_SiDs(mob) = 0;
    GET_EXP(mob) = 0;

    GET_POS(mob) = POS_STANDING;
    GET_DEFAULT_POS(mob) = POS_STANDING;
    GET_SEX(mob) = GET_SEX(ch);
    GET_RACE(mob) = GET_RACE(ch);
    GET_EYES(mob) = GET_EYES(ch);

    GET_WEIGHT(mob) = GET_WEIGHT(ch);
    GET_HEIGHT(mob) = GET_HEIGHT(ch);
    GET_SIZE(mob) = GET_SIZE(ch);
    SET_BIT(NPC_FLAGS(mob, NPC_MISLEAD), NPC_MISLEAD);
    SET_BIT(MOB_FLAGS(mob, MOB_WALKER), MOB_WALKER);
    mob->npc()->specials.vnum_corpse = -1;
    mob->npc()->specials.death_script = -1;
    EXTRACT_TIMER(mob) = MAX(1, level / 5) * SECS_PER_MUD_DAY;

    char_to_room(mob, ch->in_room);

    act_affect_mess(spellnum, ch, mob, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_miracle)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    struct char_data *m;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = SECS_PER_MUD_DAY;
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = level;
    af[0].main = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    if (level > 130 && ch == victim) {
        struct follow_type *f;

        m = (ch->master) ? ch->master : ch;
        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            if (af[i].bitvector || af[i].location != APPLY_NONE)
                affect_join_char(m, af + i);
        }
        act_affect_mess(spellnum, ch, m, show_mess, TYPE_MESS_HIT);

        for (f = m->followers; f; f = f->next) {
            if ((AFF_FLAGGED(f->follower, AFF_GROUP) || AFF_FLAGGED(f->follower, AFF_CHARM))
                && (f->follower != ch)) {
                //накладываем эффекты
                for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
                    if (af[i].bitvector || af[i].location != APPLY_NONE)
                        affect_join_char(f->follower, af + i);
                }
                act_affect_mess(spellnum, ch, f->follower, show_mess, TYPE_MESS_HIT);
            }
        }

        m = (ch->party_leader) ? ch->party_leader : ch;
        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            if (af[i].bitvector || af[i].location != APPLY_NONE)
                affect_join_char(m, af + i);
        }
        act_affect_mess(spellnum, ch, m, show_mess, TYPE_MESS_HIT);
        for (f = m->party; f; f = f->next) {
            if ((AFF_FLAGGED(f->follower, AFF_GROUP) || AFF_FLAGGED(f->follower, AFF_CHARM))
                && (f->follower != ch)) {
                //накладываем эффекты
                for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
                    if (af[i].bitvector || af[i].location != APPLY_NONE)
                        affect_join_char(f->follower, af + i);
                }
                act_affect_mess(spellnum, ch, f->follower, show_mess, TYPE_MESS_HIT);
            }
        }

    }
}

ASPELL(spell_spirit_weapon)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(SECS_PER_MUD_HOUR, (level / 20) * SECS_PER_MUD_HOUR);
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = level;
    af[0].main = TRUE;


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_consecrate)
{
    struct room_affect_data af[MAX_SPELL_AFFECTS];
    bool accum_affect = FALSE, accum_duration = FALSE, success = TRUE;
    bool update_spell = FALSE, result = FALSE;
    int in_room = IN_ROOM(ch), i, perc = 100;
    struct room_data *room;

    if (ch == NULL || in_room == NOWHERE)
        return;

    room = &world[in_room];

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].owner = 0;
        af[i].duration = 0;
    }

    af[0].bitvector = ROOM_AFF_SANCTUARY;
    af[0].duration = MAX(SECS_PER_MUD_HOUR, (level / 25) * SECS_PER_MUD_HOUR);
    af[0].modifier = MAX(1, level / 30);
    af[0].owner = GET_ID(ch);

    int mod = 0;

    if ((mod = affected_room_by_bitvector(room, ROOM_AFF_DESECRATE)))
        perc += mod;

    if (number(1, perc) < level) {
        if (affected_room_by_spell_real(room, spellnum) && success)
            update_spell = TRUE;
        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            if (af[i].bitvector) {
                if (update_spell)
                    affect_join_fspell_room(room, af + i);
                else
                    affect_join_room(room, af + i, accum_duration, FALSE, accum_affect, FALSE);
            }
        }
        result = TRUE;
    }

    if (!result)
        act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_FAIL);
    else
        act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_aspect_god)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;


    if (GET_GODS(ch) != GET_GODS(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(2 * SECS_PER_MUD_TICK, (level / 10) * SECS_PER_MUD_TICK);
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = level;
    af[0].main = TRUE;

    af[1].type = af[0].type;
    af[1].duration = af[0].duration;
    af[1].location = APPLY_SAVING_FIRE;
    af[1].owner = af[0].owner;
    af[1].modifier = MAX(10, level / 5);
    af[2].type = af[0].type;
    af[2].duration = af[0].duration;
    af[2].location = APPLY_SAVING_COLD;
    af[2].owner = af[0].owner;
    af[2].modifier = MAX(10, level / 5);
    af[3].type = af[0].type;
    af[3].duration = af[0].duration;
    af[3].location = APPLY_SAVING_ELECTRO;
    af[3].owner = af[0].owner;
    af[3].modifier = MAX(10, level / 5);
    af[4].type = af[0].type;
    af[4].duration = af[0].duration;
    af[4].location = APPLY_SAVING_ACID;
    af[4].owner = af[0].owner;
    af[4].modifier = MAX(10, level / 5);
    af[5].type = af[0].type;
    af[5].duration = af[0].duration;
    af[5].location = APPLY_ARMOUR0;
    af[5].owner = af[0].owner;
    af[5].modifier = MIN(9, MAX(3, level / 12));
    af[6].type = af[0].type;
    af[6].duration = af[0].duration;
    af[6].location = APPLY_ARMOUR1;
    af[6].owner = af[0].owner;
    af[6].modifier = MIN(9, MAX(3, level / 15));
    af[7].type = af[0].type;
    af[7].duration = af[0].duration;
    af[7].location = APPLY_ARMOUR2;
    af[7].owner = af[0].owner;
    af[7].modifier = MIN(9, MAX(3, level / 12));
    af[8].type = af[0].type;
    af[8].duration = af[0].duration;
    af[8].location = APPLY_CHA;
    af[8].owner = af[0].owner;
    af[8].modifier = MAX(1, level / 20);
    af[9].type = af[0].type;
    af[9].duration = af[0].duration;
    af[9].bitvector = AFF_HOLYLIGHT;
    af[9].owner = af[0].owner;
    af[9].modifier = MAX(1, level / 25);
    af[10].type = af[0].type;
    af[10].duration = af[0].duration;
    af[10].location = APPLY_POWER;
    af[10].owner = af[0].owner;
    af[10].modifier = MAX(1, level / 40);


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}


ASPELL(spell_broth_weapon)
{
    int i;
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if (!same_group(ch, victim) && ch != victim) {
        send_to_charf(ch, ONLYSAME);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(3 * SECS_PER_MUD_TICK, (level / 5) * SECS_PER_MUD_TICK);
    af[0].bitvector = AFF_NOTHING;
    af[0].owner = GET_ID(ch);
    af[0].modifier = level;
    af[0].main = TRUE;


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_de_mind)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int alevel = -MAX(1, level / 10), i;

    if (ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = 60;
    af[0].bitvector = AFF_NOTHING;
    af[0].location = APPLY_LEVEL;
    af[0].owner = GET_ID(ch);
    af[0].modifier = alevel;
    af[0].main = TRUE;


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    if (GET_HIT(victim) > GET_REAL_MAX_HIT(victim))
        GET_HIT(victim) = GET_REAL_MAX_HIT(victim);
}

AEVENT(event_animate_animal)
{
    int mob_num, i, mob_level, spellnum = find_spell_num(SPELL_ANIMATE_ANIMAL), rnum;
    bool show_mess = TRUE;
    struct char_data *mob;
    char buf[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    struct affected_type af;
    struct obj_data *obj = params->object;
    struct char_data *ch = params->actor;
    int level = params->narg[0];

    if (affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_SANCTUARY)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (get_followers_num(ch, FLW_UNDEAD) != 0) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    mob_num = GET_OBJ_VAL(obj, 2);
    rnum = real_mobile(mob_num);
    if (mob_num > 0) {
        if (!IS_ANIMAL(mob_proto + rnum) || GET_POWER(mob_proto + rnum)) {
            act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
            return;
        }
        mob_level = GET_LEVEL(mob_proto + rnum);
    } else {                    //труп игрока
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (mob_level > GET_LEVEL(ch) + 2) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (!(mob = read_mobile(mob_num, VIRTUAL))) {
        send_to_charf(ch, "ОШИБКА.\r\n");
        return;
    }


    SET_BIT(MOB_FLAGS(mob, MOB_ISNPC), MOB_ISNPC);

    if (GET_OBJ_VAL(obj, 3) == 4) {
        GET_MOB_VID(mob) = VMOB_SKELET;

        sprintf(name, "скелет() %s", get_name_pad(mob->player.names, PAD_ROD, PAD_MONSTER));

        for (i = 0; i < NUM_PADS; i++)
            GET_PAD(mob, i) = str_dup(get_name_pad(name, i, PAD_MONSTER));

        GET_NAME(mob) = str_dup(get_name_pad(name, PAD_IMN, PAD_MONSTER));
        mob->player.name = str_dup(get_name_pad(name, PAD_IMN, PAD_MONSTER));
        sprintf(buf, "%s ожидает приказа своего хозяина.",
                get_name_pad(name, PAD_IMN, PAD_MONSTER));
        mob->player.long_descr = str_dup(buf);

        GET_LEVEL(mob) = 0;
        GET_GOLD(mob) = 0;

        for (int icls = 0; icls < NUM_CLASSES; icls++)
            if (ch->classes[icls]) {
                ch->classes[icls] /= 2;
                GET_LEVEL(mob) += ch->classes[icls];
            }

        sprintf(buf, "%s ожидает приказа своего хозяина.",
                get_name_pad(name, PAD_IMN, PAD_MONSTER));
        mob->player.charm_descr = str_dup(buf);

        sprintf(buf, "%s ожидает Вашего приказа.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
        mob->player.charm_descr_me = str_dup(buf);

    } else {
        GET_MOB_VID(mob) = VMOB_ZOMBIE;

        if (GET_SEX(mob) == SEX_MALE)
            sprintf(name, "мертв(ый) %s", mob->player.names);
        else if (GET_SEX(mob) == SEX_FEMALE)
            sprintf(name, "мертв(ая) %s", mob->player.names);
        else if (GET_SEX(mob) == SEX_NEUTRAL)
            sprintf(name, "мертв(ое) %s", mob->player.names);
        else
            sprintf(name, "мертв(ые) %s", mob->player.names);

        for (i = 0; i < NUM_PADS; i++) {
            GET_PAD(mob, i) = str_dup(get_name_pad(name, i, PAD_MONSTER));
        }

        GET_NAME(mob) = str_dup(get_name_pad(name, PAD_IMN, PAD_MONSTER));
        mob->player.name = str_dup(get_name_pad(name, PAD_IMN, PAD_MONSTER));
        sprintf(buf, "%s ожидает приказа своего хозяина.",
                get_name_pad(name, PAD_IMN, PAD_MONSTER));
        mob->player.long_descr = str_dup(buf);

        GET_LEVEL(mob) = 0;
        for (int icls = 0; icls < NUM_CLASSES; icls++)
            if (mob->classes[icls]) {
                mob->classes[icls] = (mob->classes[icls] * 3) / 4;
                GET_LEVEL(mob) += mob->classes[icls];
            }

        sprintf(buf, "%s ожидает приказа своего хозяина.",
                get_name_pad(name, PAD_IMN, PAD_MONSTER));
        mob->player.charm_descr = str_dup(buf);

        sprintf(buf, "%s ожидает Вашего приказа.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
        mob->player.charm_descr_me = str_dup(buf);
    }

    mob->player.names = str_dup(name);
    mob->player.description = NULL;
    clear_mob_specials(mob);
//Пересчитываем параметры
    recalc_params(mob);
    GET_HIT(mob) = GET_REAL_MAX_HIT(mob);

    char_to_room(mob, IN_ROOM(ch));
//act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_HIT);
    act("@1и медленно встал1(,а,о,и).", "КМп", ch, obj);
    act("2и начал2(,а,о,и) следовать за Вами.", "Мм", ch, mob);
    act("2+и начал2(,а,о,и) следовать за 1+т.", "Кмм", ch, mob);
    GET_OBJ_TIMER(obj) = -1;
//указать что нежить
    GET_MOB_TYPE(mob) = TMOB_UNDEAD;
    GET_ALIGNMENT(mob) = -1000;
    mob->follow_vnum = 0;
    mob->npc()->specials.vnum_corpse = -1;
    mob->npc()->specials.death_script = -1;
    add_follower(mob, ch, FLW_UNDEAD);
// GET_SKILL(mob,SKILL_GUARD) = 1;
    stop_guarding(mob);
    /* if (mob->master)
       GUARDING(mob) = mob->master; */
    af.type = spellnum;
    af.duration = MAX(2, level / 5) * SECS_PER_MUD_HOUR;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    af.battleflag = 0;
    af.owner = GET_ID(ch);
    affect_to_char(mob, &af);
}


ASPELL(spell_animate)
{
    char buf1[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    char buf3[MAX_INPUT_LENGTH];
    struct event_param_data params;

    init_event_param(&params);

    if (obj == NULL)
        return;

    if (!IS_CORPSE(obj)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_GOD);
        return;
    }

    if (get_followers_num(ch, FLW_UNDEAD) > 0)
        level = 0;


    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.actor = ch;
    params.narg[0] = level;
    params.object = obj;

    sprintf(buf1, "Вы оживляете %s", GET_OBJ_PNAME(obj, 0));
    sprintf(buf2, "оживляет %s", GET_OBJ_PNAME(obj, 0));
    sprintf(buf3, "Вы прекратили процесс оживления %s", GET_OBJ_PNAME(obj, 1));

    params.status = buf1;
    params.rstatus = buf2;
    params.bto_actor = buf3;

    add_event(30, 0, event_animate_animal, &params);
    act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_protect_undead)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 20) * SECS_PER_MUD_HOUR;
    af[0].location = APPLY_NONE;
    af[0].bitvector = AFF_PROT_UNDEAD;
    af[0].modifier = MAX(1, level / 4);
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);



    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_cause_fear)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int ll, i = 0;
    bool ok = FALSE;

    if (!ch || !victim || IN_ROOM(victim) == NOWHERE)
        return;


    if (may_pkill(ch, victim) != PC_REVENGE_PC &&
        (!IS_NPC(victim) || (IS_NPC(victim) && MOB_FLAGGED(victim, MOB_MEMORY))))
        inc_pkill_group(victim, ch, 1, 0);

    ll = MAX(1, level / 8);     //(от 1 до 18%)
    if ((ll > 0 && general_savingthrow_3(victim, SAV_WILL, ll)) || IS_UNDEAD(victim) ||
        MOB_FLAGGED(victim, MOB_SENTINEL) || MOB_FLAGGED(victim, MOB_NOFIGHT) ||
        !may_kill_here(ch, victim, FALSE)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
    stop_fighting(victim, FALSE);

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 5);
    af[0].battleflag = TRUE;
    af[0].location = APPLY_NONE;
    af[0].bitvector = AFF_NOTHING;
    af[0].modifier = level;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);


    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

    for (i = 0; i < 6; i++) {
        int rnd = number(0, NUM_OF_DIRS - 1);

        if (CAN_GO(victim, rnd) && MAY_MOVE2(victim)) {
            if (SECT(EXIT(victim, rnd)->to_room) == SECT_FLYING && GET_POS(victim) != POS_FLYING)
                continue;
            ok = do_simple_move(victim, rnd | 0x80, TRUE, TRUE);
        }
    }

    if (!ok)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_GOD);
}


ASPELL(spell_detect_undead)
{

    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 20) * SECS_PER_MUD_HOUR;
    af[0].location = APPLY_NONE;
    af[0].bitvector = AFF_DETECT_UNDEAD;
    af[0].modifier = level;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);



    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }
}

ASPELL(spell_skull_snare)
{
    bool fail = FALSE;
    struct char_data *k;
    struct follow_type *f;
    struct obj_data *trap;

    if (ch->in_room == NOWHERE)
        return;

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL))
        fail = TRUE;
    if (!fail) {
        trap = create_obj();
        trap->item_number = NOTHING;
        trap->in_room = NOWHERE;
        GET_OBJ_SEX(trap) = SEX_MALE;

        trap->description = str_dup("Призрачный череп парит здесь сверкая глазами.");


        trap->short_description = str_dup("призрачный череп");
        trap->names = str_dup("призрачн(ый) череп()");
        trap->name = str_dup("призрачный череп");
        for (int i = 0; i < NUM_PADS; i++)
            trap->PNames[i] = str_dup((const char *) get_name_pad(trap->names, i, PAD_OBJECT));

        GET_OBJ_TYPE(trap) = ITEM_SPARE;
        GET_OBJ_EXTRA(trap, ITEM_NODONATE) |= ITEM_NODONATE;
        GET_OBJ_EXTRA(trap, ITEM_NODROP) |= ITEM_NODROP;
        GET_OBJ_EXTRA(trap, ITEM_NOSELL) |= ITEM_NOSELL;
        GET_OBJ_EXTRA(trap, ITEM_NOSELL) |= ITEM_NORENT;

        GET_OBJ_QUALITY(trap) = 5;
        GET_OBJ_WEIGHT(trap) = 1;       //10кг вес
        GET_OBJ_CUR(trap) = GET_OBJ_MAX(trap) = 100;
        GET_OBJ_TEMP(trap) = 0;
        GET_OBJ_TIMER(trap) = MAX(2, level / 15);

        GET_OBJ_VAL(trap, 0) = level;
        GET_OBJ_VAL(trap, 1) = TRUE;

        k = (ch->master ? ch->master : ch);
        for (f = k->followers; f; f = f->next) {
            if (!AFF_FLAGGED(f->follower, AFF_GROUP) || ch == f->follower)
                continue;
            add_obj_visible(trap, f->follower);
        }

        k = (ch->party_leader ? ch->party_leader : ch);
        for (f = k->party; f; f = f->next) {
            if (!AFF_FLAGGED(f->follower, AFF_GROUP) || ch == f->follower)
                continue;
            add_obj_visible(trap, f->follower);
        }

        add_obj_visible(trap, ch);

        obj_to_room(trap, ch->in_room);
        act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_HIT);
    } else
        act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_FAIL);
}


AEVENT(event_animate_skl)
{
    int mob_num, i, mob_level, spellnum = find_spell_num(SPELL_ANIMATE_SKELET);
    bool show_mess = TRUE;
    struct char_data *mob;
    char buf[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    struct affected_type af;
    struct obj_data *obj = params->object;
    struct char_data *ch = params->actor;
    int level = params->narg[0];

    if (!IS_CORPSE(obj)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_GOD);
        return;
    }

    if (GET_OBJ_VAL(obj, 3) != 4) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_GOD);
        return;
    }

    if (affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_SANCTUARY)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (get_followers_num(ch, FLW_UNDEAD) > 0)
        level = 0;

    mob_num = GET_OBJ_VAL(obj, 2);

    if (mob_num > 0) {
        mob_level = GET_LEVEL(mob_proto + real_mobile(mob_num));
    } else {                    //труп игрока
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (mob_level > GET_LEVEL(ch) + 2) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (!(mob = read_mobile(mob_num, VIRTUAL))) {
        send_to_charf(ch, "ОШИБКА.\r\n");
        return;
    }

    SET_BIT(MOB_FLAGS(mob, MOB_ISNPC), MOB_ISNPC);

    sprintf(name, "скелет() %s", GET_SEX(mob) == SEX_FEMALE ?
            get_name_pad((char *) race_name_pad_female[(int) GET_RACE(mob)], PAD_ROD, PAD_MONSTER) :
            get_name_pad((char *) race_name_pad_male[(int) GET_RACE(mob)], PAD_ROD, PAD_MONSTER));

    //mob->player.names = str_dup(name);

    for (i = 0; i < NUM_PADS; i++)
        GET_PAD(mob, i) = str_dup(get_name_pad(name, i, PAD_MONSTER));

    GET_NAME(mob) = str_dup(get_name_pad(name, PAD_IMN, PAD_MONSTER));
    mob->player.name = str_dup(get_name_pad(name, PAD_IMN, PAD_MONSTER));
    sprintf(buf, "%s ожидает приказа своего хозяина.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    mob->player.long_descr = str_dup(buf);

    sprintf(buf, "%s ожидает приказа своего хозяина.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    mob->player.charm_descr = str_dup(buf);

    sprintf(buf, "%s ожидает Вашего приказа.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    mob->player.charm_descr_me = str_dup(buf);

    mob->player.names = str_dup(name);
    mob->player.description = NULL;

    char_to_room(mob, IN_ROOM(ch));
    GET_RACE(mob) = RACE_SKELET;
    GET_SEX(mob) = SEX_MALE;
    GET_DEX(mob) = 10;
    GET_STR(mob) = 16;
    GET_CON(mob) = 18;
    GET_INT(mob) = 8;
    GET_WIS(mob) = 8;
    GET_CHA(mob) = 3;

//указать что нежить
    GET_MOB_TYPE(mob) = TMOB_UNDEAD;
    GET_ALIGNMENT(mob) = -1000;
    GET_MOB_VID(mob) = VMOB_SKELET;
    mob->follow_vnum = 0;
    SET_BIT(AFF_FLAGS(mob, AFF_DARKVISION), AFF_DARKVISION);
    clear_mob_specials(mob);

    recalc_params(mob);
    GET_HIT(mob) = GET_REAL_MAX_HIT(mob);
    mob->npc()->specials.vnum_corpse = -1;
    mob->npc()->specials.death_script = -1;
    mob->npc()->specials.armor[0] = (mob->npc()->specials.armor[0] * 4) / 5;
    mob->npc()->specials.armor[1] *= 2;
    mob->npc()->specials.armor[2] = (mob->npc()->specials.armor[2] * 2) / 3;

    add_follower(mob, ch, FLW_UNDEAD);

    act("Вы создали 2в из груды костей.", "Мм", ch, mob);
    act("1и создал1(,а,о,и) 2в из груды костей.", "Кмм", ch, mob);
    act("2и начал2(,а,о,и) следовать за Вами.", "Мм", ch, mob);
    act("2+и начал2(,а,о,и) следовать за 1+т.", "Кмм", ch, mob);
    GET_OBJ_TIMER(obj) = -1;

    SET_SKILL(mob, SKILL_GUARD) = 1;
    stop_guarding(mob);

    if (mob->master)
        GUARDING(mob) = mob->master;

    af.type = spellnum;
    af.duration = MAX(2, level) * SECS_PER_MUD_TICK;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    af.battleflag = 0;
    af.owner = GET_ID(ch);
    affect_to_char(mob, &af);
    mob->npc()->specials.AlrNeed = TRUE;

}

ASPELL(spell_animate_skl)
{
    char buf1[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    char buf3[MAX_INPUT_LENGTH];
    struct event_param_data params;

    init_event_param(&params);


    if (obj == NULL)
        return;

    if (!IS_CORPSE(obj)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_GOD);
        return;
    }

    if (get_followers_num(ch, FLW_UNDEAD) > 0)
        level = 0;


    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.actor = ch;
    params.narg[0] = level;
    params.object = obj;


    sprintf(buf1, "Вы создаете скелет из груды костей");
    sprintf(buf2, "создает скелет из груды костей");
    sprintf(buf3, "Вы прекратили процесс создания скелета");

    params.status = buf1;
    params.rstatus = buf2;
    params.bto_actor = buf3;

    add_event(30, 0, event_animate_skl, &params);
    act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_HIT);

}

ASPELL(spell_make_skelet)
{
    void corpse_desc_skelet(struct obj_data *obj);

    if (obj == NULL)
        return;

    if (!IS_CORPSE(obj)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (GET_OBJ_VAL(obj, 3) == 4) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    corpse_desc_skelet(obj);
}

ASPELL(spell_freeze_blood)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i, dam = 0, x, y, z, w;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }

    if (IS_CONSTRUCTION(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_GOD);
        return;
    }

    if (IS_UNDEAD(victim) || IS_REPTILE(victim))
        dam = 0;
//холод 1%
    if (!general_savingthrow_3(victim, SAV_COLD, 1) && dam) {
        af[0].type = spellnum;
        af[0].duration = PULSE_VIOLENCE + 10;
        af[0].bitvector = AFF_HOLD;
        af[0].main = TRUE;
        af[0].owner = GET_ID(ch);
    }

    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);

//накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_shadow_death)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i, dam = 0, x, y, z, w;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }


    if (IS_UNDEAD(victim) || IS_CONSTRUCTION(victim))
        dam = 0;

    if (!general_savingthrow_3(victim, SAV_FORT, level / 5) && dam) {
        af[0].type = spellnum;
        af[0].duration = PULSE_VIOLENCE * MIN(10, MAX(2, level / 10));  //не меньше 2х не больше 10ти раундов
        af[0].bitvector = AFF_HAEMORRAGIA;
        af[0].main = TRUE;
        af[0].modifier = level;
        af[0].owner = GET_ID(ch);
    }

    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);

//накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}


AEVENT(event_wakeup_dead)
{
    int mob_num, i, mob_level, is_mum = FALSE, rnum;
    int spellnum = find_spell_num(SPELL_WAKEUP_DEAD);
    bool show_mess = TRUE;
    struct char_data *mob;
    struct affected_type af;
    char buf[MAX_INPUT_LENGTH];
    char name[MAX_INPUT_LENGTH];
    int level = params->narg[0];
    struct obj_data *obj = params->object;
    struct char_data *ch = params->actor;

    if (obj == NULL)
        return;

    if (affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_SANCTUARY)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    mob_num = GET_OBJ_VAL(obj, 2);

    if (get_followers_num(ch, FLW_UNDEAD) > 0)
        level = 0;

    if (GET_OBJ_VAL(obj, 3) == 5)       //забальзамированое
        is_mum = TRUE;
    else
        is_mum = FALSE;

    rnum = real_mobile(mob_num);

    if (mob_num > 0) {
        if (IS_ANIMAL(mob_proto + rnum)) {
            act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
            return;
        }
        mob_level = GET_LEVEL(mob_proto + rnum);
    } else {                    //труп игрока
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (mob_level * 2 > level || GET_POWER(mob_proto + rnum)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (!(mob = read_mobile(mob_num, VIRTUAL))) {
        send_to_charf(ch, "ОШИБКА.\r\n");
        return;
    }

    GET_GOLD(mob) = 0;
    GET_SEX(mob) = get_sex_infra(mob);

    if (is_mum) {
        sprintf(name, "мум(ия) %s", GET_SEX(mob) == SEX_FEMALE ?
                get_name_pad((char *) race_name_pad_female[(int) GET_RACE(mob)], PAD_ROD,
                             PAD_MONSTER) : get_name_pad((char *) race_name_pad_male[(int)
                                                                                     GET_RACE(mob)],
                                                         PAD_ROD, PAD_MONSTER));
        GET_MOB_VID(mob) = VMOB_MUMIE;
    } else {
        if (GET_SEX(mob) == SEX_MALE)
            sprintf(name, "мертв(ый) %s", race_name_pad_male[(int) GET_RACE(mob)]);
        else if (GET_SEX(mob) == SEX_FEMALE)
            sprintf(name, "мертв(ая) %s", race_name_pad_female[(int) GET_RACE(mob)]);
        else if (GET_SEX(mob) == SEX_NEUTRAL)
            sprintf(name, "мертв(ое) %s", race_name_pad_male[(int) GET_RACE(mob)]);
        else
            sprintf(name, "мертв(ые) %s", race_name_pad_male[(int) GET_RACE(mob)]);
        GET_MOB_VID(mob) = VMOB_ZOMBIE;
    }
    for (i = 0; i < NUM_PADS; i++)
        GET_PAD(mob, i) = str_dup(get_name_pad(name, i, PAD_MONSTER));

    GET_NAME(mob) = str_dup(get_name_pad(name, PAD_IMN, PAD_MONSTER));
    mob->player.name = str_dup(get_name_pad(name, PAD_IMN, PAD_MONSTER));
    sprintf(buf, "%s ожидает приказа своего хозяина.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    mob->player.long_descr = str_dup(buf);

    sprintf(buf, "%s ожидает приказа своего хозяина.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    mob->player.charm_descr = str_dup(buf);

    sprintf(buf, "%s ожидает Вашего приказа.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    mob->player.charm_descr_me = str_dup(buf);

    mob->player.names = str_dup(name);
    mob->player.description = NULL;

    recalc_mob(mob);

    char_to_room(mob, IN_ROOM(ch));
    //SET_BIT(NPC_FLAGS(mob, NPC_DIRTY), NPC_DIRTY);
    //указать что нежить
    GET_MOB_TYPE(mob) = TMOB_UNDEAD;
    GET_ALIGNMENT(mob) = -1000;
    mob->follow_vnum = 0;
    SET_BIT(AFF_FLAGS(mob, AFF_DARKVISION), AFF_DARKVISION);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_AGGRESSIVE), MOB_AGGRESSIVE);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_SPEC), MOB_SPEC);
    mob->npc()->specials.vnum_corpse = -1;
    mob->npc()->specials.death_script = -1;
    add_follower(mob, ch, FLW_UNDEAD);
    af.type = spellnum;
    af.duration = MAX(2, level) * SECS_PER_MUD_TICK;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    af.battleflag = 0;
    af.owner = GET_ID(ch);
    affect_to_char(mob, &af);
    clear_mob_specials(mob);
    act("@1и медленно встал1(,а,о,и).", "КМп", ch, obj);
    act("2и начал2(,а,о,и) следовать за Вами.", "Мм", ch, mob);
    act("2+и начал2(,а,о,и) следовать за 1+т.", "Кмм", ch, mob);
    GET_OBJ_TIMER(obj) = -1;
    GET_HIT(mob) = GET_REAL_MAX_HIT(mob);
    //extract_obj(obj);
}

ASPELL(spell_wakeup_dead)
{
    char buf1[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    char buf3[MAX_INPUT_LENGTH];
    struct event_param_data params;

    init_event_param(&params);


    if (obj == NULL)
        return;

    if (!IS_CORPSE(obj)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_GOD);
        return;
    }

    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.actor = ch;
    params.narg[0] = level;
    params.object = obj;

    sprintf(buf1, "Вы оживляете %s", GET_OBJ_PNAME(obj, 0));
    sprintf(buf2, "оживляет %s", GET_OBJ_PNAME(obj, 0));
    sprintf(buf3, "Вы прекратили процесс оживления %s", GET_OBJ_PNAME(obj, 1));

    params.status = buf1;
    params.rstatus = buf2;
    params.bto_actor = buf3;

    add_event(30, 0, event_wakeup_dead, &params);
    act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_or)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    if (IS_UNDEAD(victim) || IS_CONSTRUCTION(victim) || PRF_FLAGGED(victim, PRF_NOHASSLE)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_GOD);
        return;
    }
//выносливость 1..25.%
    if (!general_savingthrow_3(victim, SAV_FORT, level / 6)) {
        af[0].type = find_spell_num(SPELL_HAEMORRAGIA);
        af[0].duration = MAX(1, level / 16) * SECS_PER_MUD_TICK;
        af[0].bitvector = AFF_HAEMORRAGIA;
        af[0].main = TRUE;
        af[0].modifier = MAX(15, level);
        af[0].owner = GET_ID(ch);

        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            if (af[i].bitvector || af[i].location != APPLY_NONE)
                affect_join_char(victim, af + i);
        }
    } else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
}

ASPELL(spell_steel_bones)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    char name[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    if (!IS_SKELET(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (!IS_AFFECTED(victim, AFF_CHARM) && victim->master != ch) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_GOD);
        return;
    }

    af[0].type = spellnum;
    af[0].duration = -1;
    af[0].location = APPLY_ARMOUR0;
    af[0].main = TRUE;
    af[0].modifier = MAX(5, level / 5);
    af[0].owner = GET_ID(ch);


    af[1].type = af[0].type;
    af[1].duration = af[0].duration;
    af[1].location = APPLY_ARMOUR2;
    af[1].main = FALSE;
    af[1].modifier = af[0].modifier;
    af[1].owner = af[0].owner;

    act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    sprintf(name, "стальн(ой) скелет() %s",
            get_name_pad(victim->player.names, PAD_ROD, PAD_MONSTER));

    for (i = 0; i < NUM_PADS; i++)
        MOB_STR_REASSIGN(victim, player.PNames[i], get_name_pad(name, i, PAD_MONSTER));

    MOB_STR_REASSIGN(victim, player.short_descr, get_name_pad(name, PAD_IMN, PAD_MONSTER));
    MOB_STR_REASSIGN(victim, player.name, get_name_pad(name, PAD_IMN, PAD_MONSTER));
    sprintf(buf, "%s ожидает приказа своего хозяина.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    MOB_STR_REASSIGN(victim, player.long_descr, buf);

    sprintf(buf, "%s ожидает приказа своего хозяина.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    MOB_STR_REASSIGN(victim, player.charm_descr, buf);

    sprintf(buf, "%s ожидает Вашего приказа.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    MOB_STR_REASSIGN(victim, player.charm_descr_me, buf);

    MOB_STR_REASSIGN(victim, player.names, name);

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_energy_undead)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    if (!IS_UNDEAD(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_GOD);
        return;
    }

    if (GET_LEVEL(victim) < MAX(5, level / 3)) {
        af[0].type = spellnum;
        af[0].duration = MAX(2, level / 3) * SECS_PER_MUD_TICK;
        af[0].location = APPLY_ARMOUR0;
        af[0].main = TRUE;
        af[0].modifier = MAX(5, level / 5);
        af[0].owner = GET_ID(ch);

        af[1].type = af[0].type;
        af[1].duration = af[0].duration;
        af[1].location = APPLY_ARMOUR1;
        af[1].main = FALSE;
        af[1].modifier = af[0].modifier;
        af[1].owner = af[0].owner;

        af[2].type = af[0].type;
        af[2].duration = af[0].duration;
        af[2].location = APPLY_ARMOUR2;
        af[2].main = FALSE;
        af[2].modifier = af[0].modifier;
        af[2].owner = af[0].owner;

        af[3].type = af[0].type;
        af[3].duration = af[0].duration;
        af[3].location = APPLY_SPEED;
        af[3].main = FALSE;
        af[3].modifier = level * 3;
        af[3].owner = af[0].owner;

        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            if (af[i].bitvector || af[i].location != APPLY_NONE)
                affect_join_char(victim, af + i);
        }
    } else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
}

/*****************************************************************************/

ASPELL(spell_ghost_fear)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i, dam = 0, x, y, z, w, fnd = FALSE;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    /* Расчет повреждений */
    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
    }


    if (IS_UNDEAD(victim) || IS_CONSTRUCTION(victim))
        dam = 0;

//воля 1..2%
    if (!general_savingthrow_3(victim, SAV_WILL, level / 70) && dam) {
        af[0].type = spellnum;
        af[0].duration = SECS_PER_MUD_ROUND;
        af[0].bitvector = AFF_HOLD;
        af[0].main = TRUE;
        af[0].modifier = level;
        af[0].owner = GET_ID(ch);
        af[1].type = spellnum;
        af[1].duration = SECS_PER_MUD_ROUND * 3;
        af[1].bitvector = AFF_NOTHING;
        af[1].main = FALSE;
        af[1].modifier = af[0].modifier;
        af[1].owner = af[0].owner;
        fnd = TRUE;
    }

    mag_damage(spellnum, dam, ch, victim, show_mess,
               Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);

    if (fnd && GET_POS(victim) >= POS_RESTING)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

//накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

/****************************************************************************/

ASPELL(spell_slaved_shadow)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    if (IS_UNDEAD(victim) || IS_CONSTRUCTION(victim) || IS_DARK(IN_ROOM(victim))) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }
//воля 1...50%
    if (!general_savingthrow_3(victim, SAV_WILL, level / 3)) {
        af[0].type = spellnum;
        af[0].duration = MAX(3, level / 3) * SECS_PER_MUD_ROUND;
        af[0].location = APPLY_SAVING_NEGATIVE;
        af[0].main = TRUE;
        af[0].modifier = -MAX(5, level / 2);
        af[0].owner = GET_ID(ch);


        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            if (af[i].bitvector || af[i].location != APPLY_NONE)
                affect_join_char(victim, af + i);
        }
    } else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
}

/*****************************************************************************/

AEVENT(event_make_ghola_undead)
{
    char buf[MAX_STRING_LENGTH];
    char name[MAX_STRING_LENGTH];
    struct char_data *ch = params->actor;
    int i;

    act("У Вас увеличились зубы и выросли длинные когти.", FALSE, ch, 0, 0, TO_CHAR);
    act("У $n1 увеличились зубы и выросли длинные когти.", FALSE, ch, 0, 0, TO_ROOM);

    SET_BIT(MOB_FLAGS(ch, MOB_ISNPC), MOB_ISNPC);

    if (GET_SEX(ch) == SEX_FEMALE)
        sprintf(name, "гхол(а) %s",
                get_name_pad((char *) race_name_pad_female[(int) GET_RACE(ch)], PAD_ROD,
                             PAD_MONSTER));
    else
        sprintf(name, "гхол(а) %s",
                get_name_pad((char *) race_name_pad_male[(int) GET_RACE(ch)], PAD_ROD,
                             PAD_MONSTER));

    for (i = 0; i < NUM_PADS; i++)
        MOB_STR_REASSIGN(ch, player.PNames[i], get_name_pad(name, i, PAD_MONSTER));

    MOB_STR_REASSIGN(ch, player.short_descr, get_name_pad(name, PAD_IMN, PAD_MONSTER));
    MOB_STR_REASSIGN(ch, player.name, get_name_pad(name, PAD_IMN, PAD_MONSTER));
    sprintf(buf, "%s бесцельно бродит по окресностям.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    MOB_STR_REASSIGN(ch, player.long_descr, buf);
    sprintf(buf, "%s ожидает приказов своего хозяина.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    MOB_STR_REASSIGN(ch, player.charm_descr, buf);

    sprintf(buf, "%s ожидает Ваших приказов.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    MOB_STR_REASSIGN(ch, player.charm_descr_me, buf);

    MOB_STR_REASSIGN(ch, player.names, name);
    ch->player.description = NULL;
    recalc_mob(ch);
    GET_HIT(ch) = GET_REAL_MAX_HIT(ch);

//GET_SEX(ch) = SEX_FEMALE;

    GET_MAX_HIT(ch) = (GET_MAX_HIT(ch) * 2) / 3;
    if (GET_HIT(ch) > GET_REAL_MAX_HIT(ch))
        GET_HIT(ch) = GET_REAL_MAX_HIT(ch);

    if (ch->npc()->specials.ExtraAttack)
        ch->npc()->specials.attack_type = 4;    //Укусить

    if (ch->npc()->specials.ExtraAttack2)
        ch->npc()->specials.attack_type2 = 8;   //Оцарапать

    GET_MOVE_TYPE(ch) = 8;      //ковыляет

    SET_SKILL(ch, SKILL_GUARD) = 1;
    stop_guarding(ch);

    if (ch->master)
        GUARDING(ch) = ch->master;

//указать что нежить
    GET_MOB_TYPE(ch) = TMOB_UNDEAD;
    GET_ALIGNMENT(ch) = -1000;
    GET_MOB_VID(ch) = VMOB_GHOLA;
    ch->follow_vnum = 0;
    SET_BIT(AFF_FLAGS(ch, AFF_DARKVISION), AFF_DARKVISION);
    clear_mob_specials(ch);
}


ASPELL(spell_make_ghola)
{

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if (IS_AFFECTED(victim, AFF_CHARM) && victim->master == ch && IS_UNDEAD(victim)) {
        struct event_param_data params;

        init_event_param(&params);

        params.stopflag = STOP_NONE;
        params.actor = victim;

        params.status = "У Вас начался процесс превращения в гхолу.";
        params.rstatus = "превращается в гхолу";
        params.bto_actor = "Процесс превращения в гхолу прекратился";

        add_event(SECS_PER_MUD_HOUR, 0, event_make_ghola_undead, &params);
        act_affect_mess(find_spell_num(SPELL_MAKE_GHOLA), ch, victim, TRUE, TYPE_MESS_HIT);
    } else {
        make_ghola(victim, 1, level);
        if (may_pkill(ch, victim) != PC_REVENGE_PC &&
            (!IS_NPC(victim) || (IS_NPC(victim) && MOB_FLAGGED(victim, MOB_MEMORY))))
            inc_pkill_group(victim, ch, 1, 0);

        if (CAN_SEE(victim, ch) && MAY_ATTACK(victim) && !IS_NPC(ch) &&
            IN_ROOM(victim) == IN_ROOM(ch) && IS_NPC(victim))
            attack_best(victim, ch);
    }
}


ASPELL(spell_implant_weapon)
{
    char name[MAX_INPUT_LENGTH], name2[MAX_INPUT_LENGTH];
    struct obj_data *weapon;
    bool result = TRUE;


    two_arguments(cast_argument, name2, name);

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if (!obj) {
        send_to_charf(ch, "Что Вы хотите вживить?\r\n");
        return;
    }

    if (!IS_UNDEAD(victim)) {
        send_to_charf(ch, "Вживить можно только в нежить.\r\n");
        return;
    }

    if (GET_OBJ_TYPE(obj) != ITEM_WEAPON) {
        send_to_charf(ch, "Вживить можно только оружие.\r\n");
        return;
    }

    weapon = obj;

    if (!IS_AFFECTED(victim, AFF_CHARM) && victim->master != ch) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_GOD);
        return;
    }

    if (IS_ANIMAL(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
        return;
    }

    if (!CAN_WEAR(weapon, ITEM_WEAR_WIELD) &&
        !CAN_WEAR(weapon, ITEM_WEAR_BOTHS) &&
        !OK_WIELD(victim, weapon) &&
        invalid_anti_class(victim, weapon) && invalid_no_class(victim, weapon))
        result = FALSE;
    else if (GET_EQ(victim, WEAR_WIELD))
        result = FALSE;
    else {
        obj_from_char(weapon);
        if (CAN_WEAR(weapon, ITEM_WEAR_BOTHS)) {
            GET_EQ(victim, WEAR_BOTHS) = weapon;
            weapon->worn_on = WEAR_BOTHS;
        } else {
            GET_EQ(victim, WEAR_WIELD) = weapon;
            weapon->worn_on = WEAR_WIELD;
        }
        weapon->worn_by = ch;
        weapon->next_content = NULL;
        IS_WEARING_W(ch) += GET_OBJ_WEIGHT(weapon);
    }

    if (!result)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
    else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    SET_BIT(AFF_FLAGS(victim, AFF_IMPLANT_WEAPON), AFF_IMPLANT_WEAPON);
}


ASPELL(spell_is_undead)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 3) * SECS_PER_MUD_HOUR;
    af[0].location = APPLY_NONE;
    af[0].bitvector = AFF_IS_UNDEAD;
    af[0].modifier = level;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);
    af[1].type = af[0].type;
    af[1].duration = af[0].duration;
    af[1].location = APPLY_NONE;
    af[1].bitvector = AFF_DARKVISION;
    af[1].modifier = level;
    af[1].main = FALSE;
    af[1].owner = af[0].owner;
    af[2].type = af[0].type;
    af[2].duration = af[0].duration;
    af[2].location = APPLY_CHA;
    af[2].bitvector = AFF_NOTHING;
    af[2].modifier = -15;
    af[2].main = FALSE;
    af[2].owner = af[0].owner;



    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}


ASPELL(spell_bones_wall)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    struct char_data *mob = NULL, *db_data_cnt = NULL;
    struct obj_data *tobj = NULL;
    bool update_spell = FALSE;
    int i, corpses = 1, cnt_in_room = 0;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL)
        return;

    for (tobj = world[IN_ROOM(ch)].contents; tobj; tobj = tobj->next_content)
        if (IS_CORPSE(tobj) && GET_OBJ_VAL(tobj, 3) == 4)
            corpses += 1;

    if (what_sky == -1) {       //ставим стену вокруг себя
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            af[i].type = spellnum;
            af[i].bitvector = 0;
            af[i].modifier = 0;
            af[i].battleflag = 0;
            af[i].level = 0;
            af[i].main = 0;
            af[i].location = APPLY_NONE;
        }

        //Само заклинание
        af[0].type = spellnum;
        af[0].duration = MAX(3, level / 4) * SECS_PER_MUD_ROUND;
        af[0].location = APPLY_NONE;
        af[0].bitvector = AFF_BONES_WALL;
        af[0].modifier = MAX(5, level / 10) * (corpses + 1);
        af[0].main = TRUE;
        af[0].owner = GET_ID(ch);

        //новый каст или апдейт
        if (affected_by_spell_real(ch, spellnum))
            update_spell = TRUE;

        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            if (af[i].bitvector || af[i].location != APPLY_NONE)
                affect_join_char(ch, af + i);
        }
    } else {                    //ставим стену на направление
        int vnum, rnum;

        //struct mob_spechit_data *h = NULL;
        char buf[MAX_INPUT_LENGTH];

        if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE))
            sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d", &vnum);

        rnum = real_mobile(vnum);

        if (rnum == -1) {
            send_to_charf(ch, "Ошибка #sp1801. Срочно сообщите Богам.\r\n", ch);
            return;
        }

        if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)) {
            act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_FAIL);
            return;
        }

        for (db_data_cnt = world[IN_ROOM(ch)].people; db_data_cnt;
             db_data_cnt = db_data_cnt->next_in_room)
            if (GET_MOB_RNUM(db_data_cnt) == rnum)
                cnt_in_room++;

        if (cnt_in_room) {
            send_to_charf(ch, "В этой комнате уже установлена костяная стена.\r\n");
            return;
        }

        mob = read_mobile(rnum, REAL);

        SET_BIT(MOB_FLAGS(mob, MOB_ISNPC), MOB_ISNPC);
        sprintf(buf, "%s %s.", mob->player.long_descr, DirIs[what_sky]);
        mob->player.long_descr = str_dup(buf);

        GET_HIT(mob) = GET_MAX_HIT(mob) = MAX(50, level * 3 * corpses);
#if 0
        /*'bone wall' mobile already has required number of spec attacks with carefully
         * chosen chances. and this stuff just introduces memory leak
         */
        if (mob->spec_hit) {
            int type = mob->spec_hit->type;
            int hit = mob->spec_hit->hit;
            int spell = mob->spec_hit->spell;
            int damnodice = mob->spec_hit->damnodice;
            int damsizedice = mob->spec_hit->damsizedice;
            int damage = mob->spec_hit->damage;
            int percent = mob->spec_hit->percent;
            int power = mob->spec_hit->power;
            char *to_victim = mob->spec_hit->to_victim;
            char *to_room = mob->spec_hit->to_room;

            for (i = 1; i < MAX(1, level / 20); i++) {
                add_spechit(mob, type, hit, spell, 0,
                            damnodice, damsizedice, damage,
                            percent, power, to_victim, to_room, 0, NULL);
            }
        }
#endif
        SET_BIT(NPC_FLAGS(mob, 1 << what_sky), 1 << what_sky);
        char_to_room(mob, IN_ROOM(ch));

        act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_HIT);
    }

}


ASPELL(spell_bones_pick)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    if (!IS_NPC(ch) && !same_group(ch, victim) && ch != victim) {
        send_to_charf(ch, ONLYSAME);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }
    //Само заклинание
    af[0].type = spellnum;
    af[0].duration = MAX(2, level / 2) * SECS_PER_MUD_TICK;
    af[0].location = APPLY_NONE;
    af[0].bitvector = AFF_BONES_PICK;
    af[0].modifier = level;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_thr_death)
{
    struct room_affect_data af[MAX_SPELL_AFFECTS];
    bool accum_affect = FALSE, accum_duration = FALSE, success = TRUE;
    bool update_spell = FALSE;
    int in_room = IN_ROOM(ch), i;
    struct room_data *room;

    if (ch == NULL || in_room == NOWHERE)
        return;

    room = &world[in_room];

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].owner = 0;
        af[i].duration = 0;
    }

    af[0].bitvector = ROOM_AFF_THRDEATH;
    af[0].duration = level * 6;
    af[0].modifier = level;
    af[0].owner = GET_ID(ch);

    if (affected_room_by_spell_real(room, spellnum) && success)
        update_spell = TRUE;
    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector) {
            if (update_spell)
                affect_join_fspell_room(room, af + i);
            else
                affect_join_room(room, af + i, accum_duration, FALSE, accum_affect, FALSE);
        }
    }

    act_affect_mess(spellnum, ch, 0, show_mess, TYPE_MESS_HIT);
}


ASPELL(spell_foul_flesh)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    if (IS_UNDEAD(victim) || IS_CONSTRUCTION(victim)) {
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_GOD);
        return;
    }
//рефлекс 1...50%
    if (!general_savingthrow_3(victim, SAV_REFL, level / 3)) {
        af[0].type = spellnum;
        af[0].duration = SECS_PER_MUD_TICK;
        af[0].bitvector = AFF_NOTHING;
        af[0].main = TRUE;
        af[0].modifier = MAX(3, level / 10);
        af[0].owner = GET_ID(ch);

        af[1].type = af[0].type;
        af[1].duration = af[0].duration;
        af[1].location = APPLY_LEVEL;
        af[1].modifier = MAX(1, level / 15);
        af[1].owner = af[0].owner;

        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);
        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            if (af[i].bitvector || af[i].location != APPLY_NONE)
                affect_join_char(victim, af + i);
        }
        if (GET_HIT(victim) > GET_REAL_MAX_HIT(victim))
            GET_HIT(victim) = GET_REAL_MAX_HIT(victim);
    } else
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_FAIL);
}

ASPELL(spell_prismatic_skin)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    /* Только на согрупника */
    if (!IS_NPC(ch) && !same_group(ch, victim)) {
        send_to_charf(ch, ONLYSAME);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(2 * SECS_PER_MUD_TICK, (level / 6) * SECS_PER_MUD_TICK);
    af[0].bitvector = AFF_PRISMA_SKIN;
    af[0].modifier = level;
    af[0].level = level;
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;


    //выводим сообщения
    if (show_mess)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}

ASPELL(spell_remove_curse)
{
    int i;
    struct mess_p_data *k;
    struct obj_data *tobj, *next_obj;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    /* Только на согрупника */
    if (!IS_NPC(ch) && !same_group(ch, victim)) {
        send_to_charf(ch, ONLYSAME);
        return;
    }

    for (tobj = victim->carrying; tobj; tobj = next_obj) {
        next_obj = tobj->next_content;
        if (IS_OBJ_STAT(tobj, ITEM_NODROP)) {
            obj_from_char(tobj);
            act_object_mess(spellnum, victim, tobj, show_mess, TYPE_MESS_HIT);
            obj_to_room(tobj, IN_ROOM(victim));
        } else
            for (k = tobj->mess_data; k; k = k->next)
                if (k->command == CMD_REMOVE && k->stoping) {
                    obj_from_char(tobj);
                    act_object_mess(spellnum, victim, tobj, show_mess, TYPE_MESS_HIT);
                    obj_to_room(tobj, IN_ROOM(victim));
                }
    }

    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(victim, i) && IS_OBJ_STAT(GET_EQ(victim, i), ITEM_NODROP)) {
            tobj = unequip_char(victim, i);
            act_object_mess(spellnum, victim, tobj, show_mess, TYPE_MESS_HIT);
            obj_to_room(tobj, IN_ROOM(victim));
        } else if (GET_EQ(victim, i)) {
            for (k = GET_EQ(victim, i)->mess_data; k; k = k->next)
                if (k->command == CMD_REMOVE && k->stoping) {
                    tobj = unequip_char(victim, i);
                    act_object_mess(spellnum, victim, tobj, show_mess, TYPE_MESS_HIT);
                    obj_to_room(tobj, IN_ROOM(victim));
                    break;
                }
        }
    }

    if (affected_by_spell(victim, SPELL_MAKE_GHOLA_S1))
        affect_from_char(victim, SPELL_MAKE_GHOLA_S1);

    if (affected_by_spell(victim, SPELL_MAKE_GHOLA_S2))
        affect_from_char(victim, SPELL_MAKE_GHOLA_S2);

    if (affected_by_spell(victim, SPELL_MAKE_GHOLA_S3))
        affect_from_char(victim, SPELL_MAKE_GHOLA_S3);

    if (affected_by_spell(victim, SPELL_MAKE_GHOLA_S4))
        affect_from_char(victim, SPELL_MAKE_GHOLA_S4);

    if (affected_by_spell(victim, SPELL_MAKE_GHOLA_S5))
        affect_from_char(victim, SPELL_MAKE_GHOLA_S5);

    //выводим сообщения
    if (show_mess)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

}


ASPELL(spell_death_arrows)
{
    int i;
    bool update_spell = FALSE;
    struct C_obj_affected_type af[MAX_SPELL_AFFECTS];
    bool accum_affect = FALSE, accum_duration = FALSE, success = TRUE;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || obj == NULL)
        return;

    if (GET_OBJ_TYPE(obj) != ITEM_MISSILE) {
        act("Темное облако окутало $o3 и быстро расстворилось.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].duration = 0;
        af[i].location = APPLY_NONE;
        af[i].extra = 0;
        af[i].no_flag = 0;
        af[i].anti_flag = 0;
        af[i].main = 0;
    }

    af[0].type = spellnum;
    af[0].bitvector = AFF_NOTHING;
    af[0].duration = MAX(2 * SECS_PER_MUD_TICK, (level / 4) * SECS_PER_MUD_TICK);
    af[0].modifier = MAX(1, level);
    af[0].owner = GET_ID(ch);
    af[0].extra = ITEM_DEATHAURA;
    af[0].main = TRUE;

    if (affected_object_by_spell(obj, spellnum) && success)
        update_spell = TRUE;

//накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE) {
            if (update_spell)
                affect_join_fspell_object(obj, af + i);
            else
                affect_join_object(obj, af + i, accum_duration, FALSE, accum_affect, FALSE);
        }
    }

    act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_HIT);
}


ASPELL(spell_summon_horse)
{

    struct char_data *mob = NULL;

//struct   affected_type af;
    mob_vnum mob_num = 0;

//int l,mlevel,i;

    if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
        if (!(mob_num = atoi(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString())))
            mob_num = 6503;
    }

    if (!(mob = read_mobile(-mob_num, VIRTUAL))) {
        send_to_charf(ch, "Ошибка #sp1902. Срочно сообщите Богам.\r\n", ch);
        return;
    }

    char_to_room(mob, ch->in_room);
    if (show_mess)
        act_affect_mess(spellnum, ch, mob, show_mess, TYPE_MESS_HIT);
    add_follower(mob, ch, FLW_HORSE);
    make_horse(mob, ch);

}

ASPELL(spell_shadow_protect)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    bool update_spell = FALSE;
    int i;

    if (victim == NULL || IN_ROOM(victim) == NOWHERE || ch == NULL)
        return;

    /* Только на согрупника */
    if (!IS_NPC(ch) && !same_group(ch, victim)) {
        send_to_charf(ch, ONLYSAME);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, level / 20) * SECS_PER_MUD_HOUR;
    af[0].location = APPLY_SAVING_NEGATIVE;
    af[0].modifier = MAX(1, level / 2);
    af[0].level = level;
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;

    //выводим сообщения
    if (show_mess)
        act_affect_mess(spellnum, ch, victim, show_mess, TYPE_MESS_HIT);

    //новый каст или апдейт
    if (affected_by_spell_real(victim, spellnum))
        update_spell = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }
}

ASPELL(spell_freedom_undead)
{
    struct char_data *tch, *next_tch, *master;
    bool found = FALSE;

    if (IN_ROOM(ch) == NOWHERE)
        return;

    for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
        master = NULL;
        next_tch = tch->next_in_room;
        if (IS_UNDEAD(tch) && tch->master && IS_AFFECTED(tch, AFF_CHARM)
            && IN_ROOM(tch->master) == IN_ROOM(tch)) {
            master = tch->master;
            if (ch == master || same_group(ch, master)
                || general_savingthrow_3(master, SAV_WILL, level / 2))
                continue;
            act_affect_mess(spellnum, master, tch, show_mess, TYPE_MESS_HIT);
            stop_follower(tch, TRUE);
            recalc_mob(tch);
            set_fighting(tch, master);
            found = TRUE;
        }
    }

    if (!found)
        act_affect_mess(spellnum, ch, ch, show_mess, TYPE_MESS_FAIL);
}


ASPELL(spell_mass_fear)
{

    bool update_spell = FALSE;
    struct char_data *tch, *next_tch;
    int ll, i = 0, spellfear = find_spell_num(SPELL_CAUSE_FEAR);
    bool ok = FALSE;

    if (!ch || IN_ROOM(ch) == NOWHERE)
        return;


    for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
        next_tch = tch->next_in_room;

        if (same_group(ch, tch) || (tch->master && same_group(ch, tch->master)))
            continue;

        if (may_pkill(ch, tch) != PC_REVENGE_PC &&
            (!IS_NPC(tch) || (IS_NPC(tch) && MOB_FLAGGED(tch, MOB_MEMORY))))
            inc_pkill_group(tch, ch, 1, 0);

        ll = MAX(1, level / 4); //(от 1 до 37%)
        if ((ll > 0 && general_savingthrow_3(tch, SAV_WILL, ll)) || IS_UNDEAD(tch) ||
            MOB_FLAGGED(tch, MOB_SENTINEL) || MOB_FLAGGED(tch, MOB_NOFIGHT) ||
            !may_kill_here(ch, tch, FALSE)) {
            act_affect_mess(spellnum, ch, tch, show_mess, TYPE_MESS_FAIL);
            continue;
        }
        act_affect_mess(spellnum, ch, tch, show_mess, TYPE_MESS_HIT);
        stop_fighting(tch, FALSE);
        struct affected_type af;

        af.type = spellfear;
        af.bitvector = 0;
        af.modifier = 0;
        af.battleflag = 0;
        af.level = 0;
        af.main = 0;
        af.location = APPLY_NONE;


        //Само заклинание
        af.type = spellfear;
        af.duration = MAX(1, level / 5);
        af.battleflag = TRUE;
        af.location = APPLY_NONE;
        af.bitvector = AFF_NOTHING;
        af.modifier = level;
        af.main = TRUE;
        af.owner = GET_ID(ch);



        //новый каст или апдейт
        if (affected_by_spell_real(tch, spellnum))
            update_spell = TRUE;

        //накладываем эффекты
        for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
            if (af.bitvector || af.location != APPLY_NONE)
                affect_join_char(tch, &af);
        }

        for (i = 0; i < 6; i++) {
            int rnd = number(0, NUM_OF_DIRS - 1);

            if (CAN_GO(tch, rnd) && MAY_MOVE2(tch)) {
                if (SECT(EXIT(tch, rnd)->to_room) == SECT_FLYING && GET_POS(tch) != POS_FLYING)
                    continue;

                ok = do_simple_move(tch, rnd | 0x80, TRUE, TRUE);
            }
        }
        if (!ok)
            act_affect_mess(spellnum, ch, tch, show_mess, TYPE_MESS_GOD);
    }
}


ASPELL(spell_death_weapon)
{
    int i;
    bool update_spell = FALSE;
    struct C_obj_affected_type af[MAX_SPELL_AFFECTS];
    bool accum_affect = FALSE, accum_duration = FALSE, success = TRUE;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL || obj == NULL)
        return;

    if (GET_OBJ_TYPE(obj) != ITEM_WEAPON || GET_OBJ_SKILL(obj) == SKILL_BOWS
        || GET_OBJ_SKILL(obj) == SKILL_CROSSBOWS) {
        act("Темное облако окутало $o3 и быстро расстворилось.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].duration = 0;
        af[i].location = APPLY_NONE;
        af[i].extra = 0;
        af[i].no_flag = 0;
        af[i].anti_flag = 0;
        af[i].main = 0;
    }

    af[0].type = spellnum;
    af[0].bitvector = AFF_NOTHING;
    af[0].duration = MAX(2 * SECS_PER_MUD_TICK, (level / 4) * SECS_PER_MUD_TICK);
    af[0].modifier = MAX(1, level);
    af[0].owner = GET_ID(ch);
    af[0].extra = ITEM_DEATHAURA;
    af[0].main = TRUE;

    if (affected_object_by_spell(obj, spellnum) && success)
        update_spell = TRUE;

//накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE) {
            if (update_spell)
                affect_join_fspell_object(obj, af + i);
            else
                affect_join_object(obj, af + i, accum_duration, FALSE, accum_affect, FALSE);
        }
    }

    act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_HIT);
}

ASPELL(spell_bury)
{
    struct char_data *tch, *next_tch, *master;
    bool found = FALSE;

    if (IN_ROOM(ch) == NOWHERE)
        return;

    for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
        master = NULL;
        next_tch = tch->next_in_room;
        if (IS_UNDEAD(tch) && tch->master && IS_AFFECTED(tch, AFF_CHARM)
            && IN_ROOM(tch->master) == IN_ROOM(tch)) {
            master = tch->master;
            if (ch == master || same_group(ch, master)
                || general_savingthrow_3(master, SAV_WILL, level / 2))
                continue;
            act_affect_mess(spellnum, master, tch, show_mess, TYPE_MESS_HIT);
            die(tch, ch);
            found = TRUE;
        } else
            if (IS_UNDEAD(tch) && IS_NPC(tch) && !general_savingthrow_3(tch, SAV_WILL, level / 3)) {
            act_affect_mess(spellnum, tch, tch, show_mess, TYPE_MESS_HIT);
            die(tch, ch);
            found = TRUE;
        }
    }

    if (!found)
        act_affect_mess(spellnum, ch, ch, show_mess, TYPE_MESS_FAIL);
}


ASPELL(spell_poison_fog)
{
    int i, in_room = 0;
    struct room_affect_data af[MAX_SPELL_AFFECTS];
    bool accum_affect = FALSE, accum_duration = FALSE;
    bool update_spell = FALSE;
    struct room_data *room;
    struct affected_type caf;

    if (IN_ROOM(ch) == NOWHERE || ch == NULL)
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = spellnum;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].owner = 0;
        af[i].duration = 0;
    }
    in_room = IN_ROOM(ch);

    room = &world[in_room];

    af[0].bitvector = ROOM_AFF_POISON_FOG;
    af[0].duration = MAX(SECS_PER_MUD_ROUND, (level / 3) * SECS_PER_MUD_ROUND);
    af[0].modifier = level;
    af[0].level = level;
    af[0].owner = GET_ID(ch);

    if (affected_room_by_spell_real(room, spellnum))
        update_spell = TRUE;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++)
        if (af[i].bitvector) {
            if (update_spell)
                affect_join_fspell_room(room, af + i);
            else
                affect_join_room(room, af + i, accum_duration, FALSE, accum_affect, FALSE);
        }

    caf.type = spellnum;
    caf.bitvector = AFF_NOTHING;
    caf.duration = af[0].duration;
    caf.modifier = level;
    caf.battleflag = 0;
    caf.level = level;
    caf.main = TRUE;
    caf.owner = GET_ID(ch);
    caf.location = APPLY_NONE;

    affect_join_char(ch, &caf);

}

ASPELL(spell_ressurect_necro)
{
    struct event_param_data params;
    struct char_data *tch, *next_ch, *vict = NULL;

    if (obj == NULL) {
        act_object_mess(spellnum, ch, NULL, TRUE, TYPE_MESS_HIT);
        return;
    }

    if (!IS_CORPSE(obj)) {
        act_object_mess(spellnum, ch, obj, show_mess, TYPE_MESS_GOD);
        return;
    }

    for (tch = world[IN_ROOM(obj)].people; tch; tch = next_ch) {
        next_ch = tch->next_in_room;
        if (!IS_SOUL(tch))
            continue;
        if (GET_OBJ_VAL(obj, 1) == GET_ID(tch)) {
            vict = tch;
            break;
        }
    }

    if (!vict) {
        act_object_mess(spellnum, ch, NULL, TRUE, TYPE_MESS_HIT);
        return;
    }

    if (IS_GOODS(vict)) {
        act_object_mess(spellnum, ch, NULL, TRUE, TYPE_MESS_HIT);
        return;
    }

    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char buf4[MAX_STRING_LENGTH];
    char buf5[MAX_STRING_LENGTH];
    char buf6[MAX_STRING_LENGTH];

    sprintf(buf1, "Вы начали процесс воскрешения %s.", GET_OBJ_PNAME(obj, 1));
    sprintf(buf6, "Вы совершаете магические пасы над %s", GET_OBJ_PNAME(obj, 4));
    sprintf(buf2, "1+и начал1(,а,о,и) совершать магические пасы над %s.", GET_OBJ_PNAME(obj, 4));
    sprintf(buf3, "совершает магические пасы над %s", GET_OBJ_PNAME(obj, 4));
    sprintf(buf4, "Вы прекратили магический обряд.");
    sprintf(buf5, "1+и прекратил1(,а,о,и) магический обряд.");

    init_event_param(&params);

    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.actor = ch;
    params.victim = vict;
    params.narg[0] = TRUE;
    params.sto_actor = buf1;
    params.sto_room = buf2;
    params.status = buf6;
    params.rstatus = buf3;
    params.bto_actor = buf4;
    params.bto_room = buf5;

    add_event(60, 0, event_ressurect, &params);
}

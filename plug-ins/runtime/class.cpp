/* ************************************************************************
*   File: class.c                                       Part of CircleMUD *
*  Usage: Source file for class-specific code                             *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
 * This file attempts to concentrate most of the code which must be changed
 * in order for new classes to be added.  If you're adding a new class,
 * you should go through this entire file from beginning to end and add
 * the appropriate new special cases for your new class.
 */



#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "utils.h"
#include "comm.h"
#include "spells.h"
#include "interpreter.h"
#include "handler.h"
#include "constants.h"
#include "xbody.h"


/* Names first */

const char *race_menu =
    "\r\n"
    "Ваша раса:\r\n"
    "1) Человек (Рекомендуется для новичков)\r\n"
    "2) Орк\r\n"
    "3) Гном\r\n"
    "4) Эльф\r\n"
    "5) Полуэльф\r\n"
    "6) Бариаур\r\n"
    "7) Тифлинг\r\n"
    "8) Эйзимар\r\n"
    "9) Дроу\r\n"
    "Для получения справки напишите: справка <раса>, например: справка тифлинг\r\n";

#define ALIGN_ANY 100



#define Y   TRUE
#define N   FALSE

int min_roll [NUM_RACES][7] = {
    /*               Str, Con, Dex, Int, Wis, Cha*/
    /* Human    */ {   9,   9,   9,   9,   9,  9},
    /* Orc      */ {  12,  12,  10,   7,   7,  6},
    /* Gnome    */ {  11,  11,   6,   9,   9,  8},
    /* Elves    */ {   6,   6,  10,  10,  10, 12},
    /* HElves   */ {   8,   8,   9,   9,  10, 10},
    /* Bariaur  */ {  12,  12,   6,   8,   8,  8},
    /* Tiefling */ {  10,   8,  11,   8,   8,  9},
    /* Aasimar  */ {   8,   8,   8,   9,   9, 12},
    /* Drou     */ {   6,   6,  10,  10,  10, 12}
};

int max_roll [NUM_RACES][7] = {
    /*               Str, Con, Dex, Int, Wis, Cha*/
    /* Human    */ {  13,  13,  13,  13, 13,  13},
    /* Orc      */ {  15,  15,  12,  10, 10,  10},
    /* Gnome    */ {  14,  14,  10,  10, 12,  12},
    /* Elves    */ {   9,   9,  14,  14, 12,  14},
    /* HElves   */ {  12,  10,  12,  11, 14,  13},
    /* Bariaur  */ {  15,  15,   8,  11, 11,  12},
    /* Tiefling */ {  12,  10,  15,  12, 11,  12},
    /* Aasimar  */ {  11,  10,  12,  12, 12,  15},
    /* Drou     */ {   9,   9,  14,  14, 12,  14}
};

//int race_roll_3 [NUM_RACES][6] = {
/*               Str, Con, Dex, Int, Wis, Cha*/
/* Human     {  15,  15,  15,  10,  10, 10}, */
/* Orc       {  20,  20,  10,   5,  10,  5}, */
/* Gnome     {  20,  20,  10,  10,  10,  5}, */
/* Elves     {  10,   5,  15,  20,  10, 15}, */
/* HElves    {  15,  10,  20,   5,   5, 20}, */
/* Bariaur   {  20,  20,  10,  10,   5, 10}, */
/* Tiefling  {  10,   5,  20,  15,  10, 15}, */
/* Aasimar   {  10,  10,  10,   5,  20, 20} */
/* Drou      {  10,   5,  15,  20,  10, 15}, */
//};

int race_ability [NUM_RACES][6] = {
    /*               Str, Con, Dex, Int, Wis, Cha*/
    /* Human    */ {  12,  12,  12,  10,  10, 10},
    /* Orc      */ {  13,  13,  12,   8,   9,  8},
    /* Gnome    */ {  12,  13,  12,   9,   9,  8},
    /* Elves    */ {   8,   7,  12,  12,  11, 13},
    /* HElves   */ {  10,   9,  12,  10,  10, 12},
    /* Bariaur  */ {  13,  14,   8,   9,   9, 10},
    /* Tiefling */ {  11,   9,  13,  10,  10, 10},
    /* Aasimar  */ {  10,   9,  10,  10,  10, 14},
    /* Drou     */ {   8,   7,  12,  12,  11, 13}
};

int align_ok_race[NUM_RACES][NUM_ALIGN] = {
    /*               GA, GG, GN, NG, NN, NE, EN, EE, EA */
    /* Human    */  { N,  Y,  Y,  Y,  Y,  Y,  Y,  Y,  N},
    /* Orc      */  { N,  N,  N,  Y,  Y,  Y,  Y,  Y,  Y},
    /* Gnome    */  { N,  Y,  Y,  Y,  Y,  Y,  Y,  Y,  N},
    /* Elves    */  { Y,  Y,  Y,  Y,  Y,  Y,  Y,  N,  N},
    /* HElves   */  { Y,  Y,  Y,  Y,  Y,  Y,  Y,  Y,  N},
    /* Bariaur  */  { Y,  Y,  Y,  Y,  Y,  Y,  Y,  Y,  Y},
    /* Tiefling */  { N,  N,  N,  N,  Y,  Y,  Y,  Y,  Y},
    /* Aasimar  */  { Y,  Y,  Y,  N,  N,  N,  N,  N,  N},
    /* Drou     */  { N,  N,  N,  N,  Y,  Y,  Y,  Y,  Y}
};

void display_align(struct char_data *ch) {
    int x, i = 1;
    char buf2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    send_to_charf(ch, "Ваша наклоность:\r\n");
    for (x = 0; x < NUM_ALIGN; x++)
        if (align_ok_race[(int)GET_RACE(ch)][x]) {
            sprintf(buf2, "%s", align_name[x]);
            PHRASE(buf2);
            sprintf(buf, "%d) %s\r\n", i, buf2);
            send_to_char(buf, ch);
            i++;
        }
    send_to_charf(ch,
                  "Для получения справки напишите: справка <натура>, например: справка злой\r\n"
                  "Если вы впервые в этом мире, то рекомендуем выбрать нейтральную натуру\r\n"
                 );

}


//int gods_ok_race[NUM_GODS][NUM_RACES] = {
/*                 Hm, Or, Gn, El, He, Br, Ti, Aa */
/* NULL        {  N,  N,  N,  N,  N,  N,  N,  N },*/
/* prPelor     {  Y,  N,  Y,  Y,  Y,  Y,  N,  Y },*/
/* prElona     {  Y,  N,  N,  Y,  Y,  Y,  N,  Y },*/
/* prKL        {  N,  N,  N,  Y,  Y,  N,  N,  N }, */
/* prFarlan    {  Y,  Y,  Y,  Y,  Y,  Y,  Y,  Y }, */
/* prOlidam    {  Y,  N,  Y,  Y,  Y,  Y,  Y,  N }, */
/* prGubert    {  Y,  N,  Y,  N,  N,  N,  N,  Y }, */
/* prGruumsh   {  N,  Y,  N,  N,  N,  N,  N,  N }, */
/* prNerull    {  Y,  Y,  N,  N,  N,  N,  Y,  N }, */
/* prHextor    {  Y,  Y,  N,  N,  N,  N,  Y,  N }  */
//};

//Временная таблица
int gods_ok_race[NUM_GODS][NUM_RACES] = {
    /*                 Hm, Or, Gn, El, He, Br, Ti, Aa, Dr */
    /* NULL       */ {  N,  N,  N,  N,  N,  N,  N,  N, N },
    /* prPelor    */ {  Y,  N,  Y,  Y,  Y,  Y,  N,  Y, N },
    /* prElona    */ {  N,  N,  N,  N,  N,  N,  N,  N, N },
    /* prKL       */ {  N,  N,  N,  N,  N,  N,  N,  N, N },
    /* prFarlan   */ {  Y,  Y,  Y,  Y,  Y,  Y,  Y,  Y, Y },
    /* prOlidam   */ {  N,  N,  N,  N,  N,  N,  N,  N, N },
    /* prGubert   */ {  N,  N,  N,  N,  N,  N,  N,  N, N },
    /* prGruumsh  */ {  Y,  Y,  Y,  N,  Y,  Y,  Y,  N, Y },
    /* prNerull   */ {  N,  N,  N,  N,  N,  N,  N,  N, N },
    /* prHextor   */ {  N,  N,  N,  N,  N,  N,  N,  N, N }
};

void display_gods(struct char_data *ch) {
    int x, i = 1;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    send_to_charf(ch, "Ваш Бог:\r\n");
    for (x = 1; x < NUM_GODS; x++)
        if (gods_ok_race[x][(int)GET_RACE(ch)]) {
            sprintf(buf2, "%s", gods_name[x]);
            PHRASE(buf2);
            sprintf(buf, "%d) %s\r\n", i, buf2);
            send_to_char(buf, ch);
            i++;
        }
    send_to_charf(ch,
                  "Для получения справки напишите: справка <бог>, например: справка нерулл\r\n"
                 );

}


/*
 * The code to interpret a class letter -- used in interpreter.c when a
 * new character is selecting a class and by 'set class' in act.wizard.c.
 */

int parse_race(char arg) {
    arg = LOWER(arg);

    switch (arg) {
        case '1':
            return RACE_HUMAN;
        case '2':
            return RACE_ORC;
        case '3':
            return RACE_GNOME;
        case '4':
            return RACE_ELVES;
        case '5':
            return RACE_HALFELVES;
        case '6':
            return RACE_BARIAUR;
        case '7':
            return RACE_TIEFLING;
        case '8':
            return RACE_AASIMAR;
        case '9':
            return RACE_DROU;
        default:
            return RACE_UNDEFINED;
    }
}


int parse_align(struct char_data *ch, int arg) {
    int align = -1, x, i = 0;

    for (x = 0; x < NUM_ALIGN; x++) {
        if (align_ok_race[(int)GET_RACE(ch)][x]) {
            i++;
            if (i == arg) {
                align = x;
                break;
            }
        }
    }
    return (align);
}

int parse_gods(struct char_data *ch, int arg) {
    int align = -1, x, i = 0;

    for (x = 1; x < NUM_GODS; x++) {
        if (gods_ok_race[x][(int)GET_RACE(ch)]) {
            i++;
            if (i == arg) {
                align = x;
                break;
            }
        }
    }
    return (align);
}

/*
 * bitvectors (i.e., powers of two) for each class, mainly for use in
 * do_who and do_users.  Add new classes at the end so that all classes
 * use sequential powers of two (1 << 0, 1 << 1, 1 << 2, 1 << 3, 1 << 4,
 * 1 << 5, etc.
 */

/* Возвращает цвет глаз персонажа */

int generate_eyes(struct char_data *ch) {
    int eyes = 0;

    if (IS_EVILS(ch)) {
        do
            (eyes = eyes_race_evil[(int)GET_RACE(ch)][number(1,NUM_EYES)]);
        while (eyes == 0);
    } else
        if (IS_NEUTRALS(ch)) {
            do
                (eyes = eyes_race_neut[(int)GET_RACE(ch)][number(1,NUM_EYES)]);
            while (eyes == 0);
        } else {
            do
                (eyes = eyes_race_good[(int)GET_RACE(ch)][number(1,NUM_EYES)]);
            while (eyes == 0);
        }

    return eyes;
}


// возвращает значение сейва (100 минимальная защита, 0 максимальная)
int saving_throws_3(struct char_data *ch, int type) {
    int saving = 0, sField;

    if (type == SAV_REFL || type == SAV_FORT || type == SAV_WILL) {
        if (IS_NPC(ch) && ch->npc()->specials.saved[type] != 0)
            saving += ch->npc()->specials.saved[type];
        else {
            switch (type) {
                case SAV_FORT:
                    sField = TCL_KFORT;
                    saving += GET_REAL_CON(ch) / 2; //бонус на выносливость за тело
                    break;
                case SAV_REFL:
                    sField = TCL_KREFL;
                    saving += GET_REAL_DEX(ch) / 2; //бонус на рефлекс за ловкость
                    break;
                case SAV_WILL:
                    sField = TCL_KWILL;
                    saving += GET_REAL_INT(ch) / 2; //бонус на волю за разум
                    break;
                default:
                    return (0);
                    break;
            }
            for (int icls = 0;icls < NUM_CLASSES;icls++)
                if (ch->classes[icls])
                    saving += (tProf.FindItem(icls)->GetItem(sField)->GetInt() * ch->classes[icls]) / 100;
        }
    } else {
        if (IS_NPC(ch) && ch->npc()->specials.saved[type] != 0)
            saving += ch->npc()->specials.saved[type];
        else { //Расчитываем резисты
            switch (type) {
                case  SAV_FIRE:
                    sField = TBD_FIRE;
                    break;
                case  SAV_COLD:
                    sField = TBD_COLD;
                    break;
                case  SAV_ELECTRO:
                    sField = TBD_ELECTRO;
                    break;
                case  SAV_ACID:
                    sField = TBD_ACID;
                    break;
                case  SAV_XAOS:
                    sField = TBD_XAOS;
                    break;
                case  SAV_ORDER:
                    sField = TBD_ORDER;
                    break;
                case  SAV_NEGATIVE:
                    sField = TBD_NEGATIVE;
                    break;
                case  SAV_POSITIVE:
                    sField = TBD_POSITIVE;
                    break;
                case  SAV_POISON:
                    sField = TBD_POISON;
                    break;
                default:
                    log("Неизвестный типа резиста %d в функции saving_throws_3()", type);
                    return (0);
                    break;
            }
            saving = (tBody.FindItem(GET_RACE(ch))->GetItem(sField)->GetInt() * GET_LEVEL(ch)) / 100;
        }
    }
    return (saving);
}

int speedy(struct char_data * ch) {
    int koef, resul = 0;
    int class_num = -1, level = 0;

    for (int icls = 0;icls < NUM_CLASSES;icls++)
        if (ch->classes[icls]) {
            class_num = icls;
            level = ch->classes[icls];
            switch (class_num) {
                case CLASS_THIEF:
                    koef = 20;
                    break;
                case CLASS_RANGER:
                    koef = 15;
                    break;
                case CLASS_MAGIC_USER:
                case CLASS_NECRO:
                    koef = 10;
                    break;
                case CLASS_WARRIOR:
                    koef = 12;
                    break;
                case CLASS_PRIEST:
                    koef = 10;
                    break;
                default:
                    koef = 5;
                    break;
            }
            resul += (level * koef);
        }

    if (ch->trap_object)
        resul /= 2;
    return (resul);
};



/* Роллинг статов */
void roll_stat(struct char_data * ch) {
    char buf[MAX_STRING_LENGTH];
//send_to_charf(ch,"max %d\r\n",get_max_class(ch));

    sprintf(buf, "Пересчитывается персонаж %s.", GET_NAME(ch));
    mudlog(buf, CMP, LVL_HIGOD, TRUE);

    GET_STR(ch) = number(min_roll[(int)GET_RACE(ch)][0], max_roll[(int)GET_RACE(ch)][0]);
    GET_CON(ch) = number(min_roll[(int)GET_RACE(ch)][1], max_roll[(int)GET_RACE(ch)][1]);
    GET_DEX(ch) = number(min_roll[(int)GET_RACE(ch)][2], max_roll[(int)GET_RACE(ch)][2]);
    GET_INT(ch) = number(min_roll[(int)GET_RACE(ch)][3], max_roll[(int)GET_RACE(ch)][3]);
    GET_WIS(ch) = number(min_roll[(int)GET_RACE(ch)][4], max_roll[(int)GET_RACE(ch)][4]);
    GET_CHA(ch) = number(min_roll[(int)GET_RACE(ch)][5], max_roll[(int)GET_RACE(ch)][5]);

}

/* Инициализация параметров игрока */
void init_stats(struct char_data * ch) {
//начальные статы
    /*ch->real_abils.str   = race_ability[(int) GET_RACE(ch)][0];
    ch->real_abils.con   = race_ability[(int) GET_RACE(ch)][1];
    ch->real_abils.dex   = race_ability[(int) GET_RACE(ch)][2];
    ch->real_abils.intel = race_ability[(int) GET_RACE(ch)][3];
    ch->real_abils.wis   = race_ability[(int) GET_RACE(ch)][4];
    ch->real_abils.cha   = race_ability[(int) GET_RACE(ch)][5];*/
    ch->real_abils.str   = max_roll[(int) GET_RACE(ch)][0];
    ch->real_abils.con   = max_roll[(int) GET_RACE(ch)][1];
    ch->real_abils.dex   = max_roll[(int) GET_RACE(ch)][2];
    ch->real_abils.intel = max_roll[(int) GET_RACE(ch)][3];
    ch->real_abils.wis   = max_roll[(int) GET_RACE(ch)][4];
    ch->real_abils.cha   = max_roll[(int) GET_RACE(ch)][5];
    ch->real_abils.lck   = 5;

    switch (GET_RACE(ch)) {
        case RACE_HUMAN:
            GET_HEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 160 + number(0, 15) : 165 + number(0, 20);
            GET_WEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 45 + number(0, 15) : 55 + number(0, 25);
            GET_SIZE(ch)   = 38 + number(2, 10);
            GET_INIT_HIT(ch) = 18;
            GET_MAX_MOVE(ch) = 70;
            GET_INIT_MANA(ch) = 20;
            break;
        case RACE_ORC:
            GET_HEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 185 + number(0, 15) : 195 + number(0, 20);
            GET_WEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 80 + number(0, 15) : 85 + number(0, 25);
            GET_SIZE(ch)   = 46 + number(5, 15);
            GET_INIT_HIT(ch) = 20;
            GET_MAX_MOVE(ch) = 75;
            GET_INIT_MANA(ch) = 10;
            break;
        case RACE_GNOME:
            GET_HEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 110 + number(0, 10) : 115 + number(0, 15);
            GET_WEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 90 + number(0, 10) : 105 + number(0, 25);
            GET_SIZE(ch)   = 26 + number(1, 8);
            GET_INIT_HIT(ch) = 22;
            GET_MAX_MOVE(ch) = 65;
            GET_INIT_MANA(ch) = 15;
            break;
        case RACE_ELVES:
        case RACE_DROU:
            GET_HEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 180 + number(0, 10) : 180 + number(0, 25);
            GET_WEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 50 + number(0, 10) : 55 + number(0, 20);
            GET_SIZE(ch)   = 40 + number(1, 5);
            GET_INIT_HIT(ch) = 15;
            GET_MAX_MOVE(ch) = 80;
            GET_INIT_MANA(ch) = 25;
            break;
        case RACE_HALFELVES:
            GET_HEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 165 + number(0, 20) : 165 + number(0, 30);
            GET_WEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 45 + number(0, 15) : 55 + number(0, 25);
            GET_SIZE(ch)   = 38 + number(2, 10);
            GET_INIT_HIT(ch) = 16;
            GET_MAX_MOVE(ch) = 75;
            GET_INIT_MANA(ch) = 18;
            break;
        case RACE_BARIAUR:
            GET_HEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 215 + number(0, 20) : 220 + number(0, 25);
            GET_WEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 290 + number(0, 20) : 305 + number(0, 25);
            GET_SIZE(ch)   = 60 + number(2, 10);
            GET_INIT_HIT(ch) = 25;
            GET_MAX_MOVE(ch) = 100;
            GET_INIT_MANA(ch) = 10;
            break;
        case RACE_TIEFLING:
            GET_HEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 150 + number(0, 15) : 155 + number(0, 15);
            GET_WEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 50 + number(0, 8) : 55 + number(0, 10);
            GET_SIZE(ch)   = 45 + number(2, 10);
            GET_INIT_HIT(ch) = 16;
            GET_MAX_MOVE(ch) = 75;
            GET_INIT_MANA(ch) = 10;
            break;
        case RACE_AASIMAR:
            GET_HEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 175 + number(0, 10) : 175 + number(0, 15);
            GET_WEIGHT(ch) = GET_SEX(ch) == SEX_FEMALE ? 55 + number(0, 10) : 55 + number(0, 15);
            GET_SIZE(ch)   = 40 + number(1, 10);
            GET_INIT_HIT(ch) = 18;
            GET_MAX_MOVE(ch) = 70;
            GET_INIT_MANA(ch) = 35;
            break;
    }
}

void check_stats(struct char_data * ch) {
    switch (GET_RACE(ch)) {
        case RACE_HUMAN:
            GET_INIT_HIT(ch) = 18;
            GET_INIT_MANA(ch) = 20;
            break;
        case RACE_ORC:
            GET_INIT_HIT(ch) = 20;
            GET_INIT_MANA(ch) = 10;
            break;
        case RACE_GNOME:
            GET_INIT_HIT(ch) = 22;
            GET_INIT_MANA(ch) = 15;
            break;
        case RACE_ELVES:
        case RACE_DROU:
            GET_INIT_HIT(ch) = 15;
            GET_INIT_MANA(ch) = 25;
            break;
        case RACE_HALFELVES:
            GET_INIT_HIT(ch) = 16;
            GET_INIT_MANA(ch) = 18;
            break;
        case RACE_BARIAUR:
            GET_INIT_HIT(ch) = 25;
            GET_INIT_MANA(ch) = 10;
            break;
        case RACE_TIEFLING:
            GET_INIT_HIT(ch) = 16;
            GET_INIT_MANA(ch) = 10;
            break;
        case RACE_AASIMAR:
            GET_INIT_HIT(ch) = 18;
            GET_INIT_MANA(ch) = 35;
            break;
    }

    affect_total(ch);
}

void do_newbie(struct char_data *ch) {
    struct obj_data *obj;

// тут выдается начальная экипировка
    /* Для всех */
    if (!GET_EQ(ch, WEAR_ARMS))
        NEW_EQUIP(ch, WEAR_ARMS, 113);
    if (!IS_BARIAUR(ch) && !GET_EQ(ch, WEAR_LEGS))
        NEW_EQUIP(ch, WEAR_LEGS, 111);

    if (GET_LEVEL(ch) > 0) {
        int flevel = GET_HLEVEL(ch, 1);
        if (GET_RACE(ch) == RACE_GNOME) {
            if (!GET_EQ(ch, WEAR_HOLD) && !GET_EQ(ch, WEAR_LIGHT) && !GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_SHIELD) && !GET_EQ(ch, WEAR_BOTHS))
                NEW_EQUIP(ch, WEAR_BOTHS, 106);
        } else {
            switch (flevel) {
                case CLASS_PRIEST:
                case CLASS_MAGIC_USER:
                case CLASS_NECRO:
                    if (!GET_EQ(ch, WEAR_HOLD) && !GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_LIGHT) && !GET_EQ(ch, WEAR_SHIELD) && !GET_EQ(ch, WEAR_BOTHS))
                        NEW_EQUIP(ch, WEAR_BOTHS, 104);
                    break;
                case CLASS_WARRIOR:
                    if (!GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_BOTHS))
                        NEW_EQUIP(ch, WEAR_WIELD, 102);
                    if (!GET_EQ(ch, WEAR_HOLD) && !GET_EQ(ch, WEAR_SHIELD) && !GET_EQ(ch, WEAR_BOTHS))
                        NEW_EQUIP(ch, WEAR_SHIELD, 123);
                    break;
                case CLASS_THIEF:
                    if (!GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_BOTHS))
                        NEW_EQUIP(ch, WEAR_WIELD, 100);
                    if (!GET_EQ(ch, WEAR_HOLD) && !GET_EQ(ch, WEAR_LIGHT) && !GET_EQ(ch, WEAR_SHIELD) && !GET_EQ(ch, WEAR_BOTHS))
                        NEW_EQUIP(ch, WEAR_HOLD, 100);
                    break;
                case CLASS_RANGER:
                    if (!GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_BOTHS))
                        NEW_EQUIP(ch, WEAR_WIELD, 100);
                    NEW_INV(ch, 103);
                    break;
            }
        }
    } else {
        if (!GET_EQ(ch, WEAR_WIELD) && !GET_EQ(ch, WEAR_BOTHS))
            NEW_EQUIP(ch, WEAR_WIELD, 102);
        if (!GET_EQ(ch, WEAR_HOLD) && !GET_EQ(ch, WEAR_SHIELD) && !GET_EQ(ch, WEAR_BOTHS))
            NEW_EQUIP(ch, WEAR_SHIELD, 123);
    }
    if (!GET_EQ(ch, WEAR_BODY))
        NEW_EQUIP(ch, WEAR_BODY, 112);
    if (!IS_BARIAUR(ch) && !GET_EQ(ch, WEAR_ABOUT))
        NEW_EQUIP(ch, WEAR_ABOUT, 119);
    if (!IS_BARIAUR(ch) && !GET_EQ(ch, WEAR_HEAD))
        NEW_EQUIP(ch, WEAR_HEAD, 114);
    if (!IS_BARIAUR(ch) && !GET_EQ(ch, WEAR_FEET))
        NEW_EQUIP(ch, WEAR_FEET, 109);

}

/* Some initializations for characters, including initial skills */
void do_start(struct char_data * ch, int newbie) {
    struct obj_data *obj;
    int i;

    set_title(ch, NULL);

    if (newbie) {
        init_stats(ch);
        GET_EYES(ch) = generate_eyes(ch);
        /* Удаляем значения в ExtraSkill
        а то все рождаются с уровнем S_BASIC */
        for (i = 0; i < MAX_SKILLS; i++) {
            SET_SKILL_LEVEL(ch, i, 0);
        }

        for (i = 1; i <= 60; i++)
            GET_HLEVEL(ch, i) = CLASS_UNDEFINED;

        /*     GET_POINT_SKILL(ch,SKILL_TYPEWAR)   = 350;
             GET_POINT_SKILL(ch,SKILL_TYPEBASIC) = 500;
             GET_POINT_SKILL(ch,SKILL_TYPEADDON) = 250;
             GET_POINT_SKILL(ch,SKILL_TYPEMAGIC) = 350; */

        GET_GOLD(ch) = 500;
        NEW_INV(ch, 125); //лампа
        NEW_INV(ch, 124); //мех
        NEW_INV(ch, 126); //хлеб
        NEW_INV(ch, 126);
        NEW_INV(ch, 126);
        NEW_INV(ch, 126);
        NEW_INV(ch, 150); //учебник
        do_newbie(ch);
        SET_BIT(PRF_FLAGS(ch, PRF_NONEW), PRF_NONEW);
    };

    /*
    switch (GET_RACE(ch))
    {
    case RACE_HUMAN:
      SPEAKING(ch) = SKILL_LANG_HUMAN;
    break;
    case RACE_ORC:
      SPEAKING(ch) = SKILL_LANG_ORC;
    break;
    case RACE_ELVES:
      SPEAKING(ch) = SKILL_LANG_ELF;
    break;
    case RACE_HALFELVES:
      SPEAKING(ch) = SKILL_LANG_HUMAN;
    break;
    case RACE_GNOME:
      SPEAKING(ch) = SKILL_LANG_DWARN;
    break;
    } */
    SPEAKING(ch) = SKILL_LANG_COMMON;

    /* ВРЕМЕННО ВСЕ МОГУ ГОВОРИТЬ И ПОНИМАТЬ ВСЕ ЯЗЫКИ */
    SET_SKILL(ch, SKILL_LANG_COMMON) =  100;
    switch (GET_RACE(ch)) {
        case RACE_ELVES:
        case RACE_DROU:
            SET_SKILL(ch, SKILL_LANG_ELF) =  100;
            break;
        case RACE_HALFELVES:
            SET_SKILL(ch, SKILL_LANG_ELF) = 100;
            SET_SKILL(ch, SKILL_LANG_HUMAN) =  100;
            break;
        case RACE_HUMAN:
            SET_SKILL(ch, SKILL_LANG_HUMAN) = 100;
            break;
        case RACE_GNOME:
            SET_SKILL(ch, SKILL_LANG_DWARN) = 100;
            break;
        case RACE_ORC:
            SET_SKILL(ch, SKILL_LANG_ORC) = 100;
            break;
        case RACE_AASIMAR:
            SET_SKILL(ch, SKILL_LANG_AASIMAR) = 100;
            break;
        case RACE_TIEFLING:
            SET_SKILL(ch, SKILL_LANG_TIEFLING) = 100;
            break;
        case RACE_BARIAUR:
            SET_SKILL(ch, SKILL_LANG_BARIAUR) = 100;
            break;
    };
    ///////////////////////////////

    //if (!GET_LEVEL(ch) == 1) advance_level(ch);
    GET_DR(ch) += 5;
    GET_HR(ch) += 5;
    GET_HIT(ch)  = GET_REAL_MAX_HIT(ch);
    GET_MOVE(ch) = GET_REAL_MAX_MOVE(ch);
    GET_MANA(ch) = GET_REAL_MAX_MANA(ch);
    GET_HLT(ch)  = 100; //Здоровье 100%

    GET_COND(ch, THIRST) = 48 * SECS_PER_MUD_TICK;
    GET_COND(ch, FULL)   = 48 * SECS_PER_MUD_TICK;
    GET_COND(ch, SLEEP)  = 48 * SECS_PER_MUD_TICK;
    GET_COND(ch, DRUNK)  = 0;

    ch->player.time.played = 0;
    ch->player.time.logon  = time(0);

    if (siteok_everyone)
        SET_BIT(PLR_FLAGS(ch, PLR_SITEOK), PLR_SITEOK);
}



/*
 * This function controls the change to maxmove, maxmana, and maxhp for
 * each class every time they gain a level.
 */

void advance_level(struct char_data * ch) {
    int i;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "%s получил%s уровень %d", GET_NAME(ch), GET_CH_SUF_1(ch), GET_LEVEL(ch));
    mudlog(buf, BRF, LVL_IMMORT, TRUE);

    if (IS_IMMORTAL(ch))
        for (i = 0; i < 3; i++)
            GET_COND(ch, i) = (char) - 1;

    save_char(ch, NOWHERE);
}

void decrease_level(struct char_data * ch) {
    int level = GET_LEVEL(ch);
    int cls = GET_HLEVEL(ch, level);
    log("Вычитаю уровень");

    if (check_class(ch, cls) - 1 <= 0) {
        log("Параметры %d", cls);
        del_class(ch, cls);
    } else {
        log("Параметры %d %d", cls, check_class(ch, cls) - 1);
        add_class(ch, cls, check_class(ch, cls) - 1, 0);
    }
    if (check_class(ch, cls) == 0)
        del_class(ch, cls);

    if (GET_TITLE(ch) && level <= 25)
        set_title(ch, "");

    GET_WIMP_LEV(ch)     = MAX(0, MIN(GET_WIMP_LEV(ch), GET_REAL_MAX_HIT(ch) / 2));
    if (!IS_IMMORTAL(ch))
        REMOVE_BIT(PRF_FLAGS(ch, PRF_HOLYLIGHT), PRF_HOLYLIGHT);
}



/*
 * invalid_class is used by handler.c to determine if a piece of equipment is
 * usable by a particular class, based on the ITEM_ANTI_{class} bitvectors.
 */

int invalid_unique(struct char_data *ch, struct obj_data *obj) {
    struct obj_data *object;
    
    if (IS_NPC(ch))
        return (FALSE);

    if (PRF_FLAGGED(ch, PRF_NOHASSLE))
        return (FALSE);

    if (!IS_CORPSE(obj))
        for (object = obj->contains; object; object = object->next_content)
            if (invalid_unique(ch, object))
                return (TRUE);
    if (!ch             ||
            !obj            ||
            IS_IMMORTAL(ch) ||
            obj->obj_flags.Obj_owner == 0 ||
            obj->obj_flags.Obj_owner == GET_UNIQUE(ch))
        return (FALSE);
    return (TRUE);
}

int invalid_anti_class(struct char_data *ch, struct obj_data *obj) {
    struct obj_data *object;

    if (IS_NPC(ch))
        return (FALSE);

    if (PRF_FLAGGED(ch, PRF_NOHASSLE))
        return (FALSE);

    if (!IS_CORPSE(obj))
        for (object = obj->contains; object; object = object->next_content)
            if (invalid_anti_class(ch, object))
                return (TRUE);

    if ((IS_OBJ_ANTI(obj, ITEM_AN_GOOD)       && IS_GOODS(ch)) ||
            (IS_OBJ_ANTI(obj, ITEM_AN_EVIL)       && IS_EVILS(ch)) ||
            (IS_OBJ_ANTI(obj, ITEM_AN_NEUTRAL)    && IS_NEUTRALS(ch)) ||
            (IS_OBJ_ANTI(obj, ITEM_AN_MAGIC_USER) && IS_MAGIC_USER(ch)) ||
            (IS_OBJ_ANTI(obj, ITEM_AN_THIEF)      && IS_THIEF(ch))      ||
            (IS_OBJ_ANTI(obj, ITEM_AN_WARRIOR)    && IS_WARRIOR(ch))    ||
            (IS_OBJ_ANTI(obj, ITEM_AN_RANGER)     && IS_RANGER(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_PRIEST)     && IS_PRIEST(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_HUMAN)      && IS_HUMAN(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_ORC)        && IS_ORC(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_GNOME)      && IS_GNOME(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_ELVES)      && IS_ELVES(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_HALFELVES)  && IS_HALFELVES(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_BARIAUR)    && IS_BARIAUR(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_TIEFLING)   && IS_TIEFLING(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_AASIMAR)    && IS_AASIMAR(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_ANIMAL)     && IS_ANIMAL(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_DRAGON)     && IS_DRAGON(ch))     ||
            (IS_OBJ_ANTI(obj, ITEM_AN_HUMANOID)   && IS_HUMANOID(ch))   ||
            (IS_OBJ_ANTI(obj, ITEM_AN_GENASI)     && IS_GENASI(ch))
       )
        return (TRUE);
    return (FALSE);
}

int invalid_no_class(struct char_data *ch, struct obj_data *obj) {
    if (IS_NPC(ch))
        return (FALSE);

    if (PRF_FLAGGED(ch, PRF_NOHASSLE))
        return (FALSE);


    if (!IS_CORPSE(obj))
        if ((IS_OBJ_NO(obj, ITEM_NO_GOOD)       && IS_GOODS(ch))       ||
                (IS_OBJ_NO(obj, ITEM_NO_EVIL)       && IS_EVILS(ch))       ||
                (IS_OBJ_NO(obj, ITEM_NO_NEUTRAL)    && IS_NEUTRALS(ch))    ||
                (IS_OBJ_NO(obj, ITEM_NO_MAGIC_USER) && IS_MAGIC_USER(ch))  ||
                (IS_OBJ_NO(obj, ITEM_NO_THIEF)      && IS_THIEF(ch))       ||
                (IS_OBJ_NO(obj, ITEM_NO_WARRIOR)    && IS_WARRIOR(ch))     ||
                (IS_OBJ_NO(obj, ITEM_NO_RANGER)     && IS_RANGER(ch))      ||
                (IS_OBJ_NO(obj, ITEM_NO_PRIEST)     && IS_PRIEST(ch))     ||
                (IS_OBJ_NO(obj, ITEM_NO_HUMAN)      && IS_HUMAN(ch))       ||
                (IS_OBJ_NO(obj, ITEM_NO_ORC)        && IS_ORC(ch))         ||
                (IS_OBJ_NO(obj, ITEM_NO_GNOME)      && IS_GNOME(ch))       ||
                (IS_OBJ_NO(obj, ITEM_NO_ELVES)      && IS_ELVES(ch))       ||
                (IS_OBJ_NO(obj, ITEM_NO_HALFELVES)  && IS_HALFELVES(ch))   ||
                (IS_OBJ_NO(obj, ITEM_NO_BARIAUR)    && IS_BARIAUR(ch))     ||
                (IS_OBJ_NO(obj, ITEM_NO_TIEFLING)   && IS_TIEFLING(ch))    ||
                (IS_OBJ_NO(obj, ITEM_NO_AASIMAR)    && IS_AASIMAR(ch))     ||
                (IS_OBJ_NO(obj, ITEM_NO_ANIMAL)     && IS_ANIMAL(ch))     ||
                (IS_OBJ_NO(obj, ITEM_NO_DRAGON)     && IS_DRAGON(ch))     ||
                (IS_OBJ_NO(obj, ITEM_NO_HUMANOID)   && IS_HUMANOID(ch))     ||
                (IS_OBJ_NO(obj, ITEM_NO_GENASI)     && IS_GENASI(ch))
           )
            return (TRUE);
    return (FALSE);
}

/* Function to return the exp required for each class/level */
/*int level_exp(int level)
{

   if (level > LVL_IMMORT)
      {return EXP_IMPL - ((LVL_IMPL - level) * 1000);
      }

   switch (level)
   {
    case 0:  return 0;
    case 1:  return 1;
    case 2:  return 1500;
    case 3:  return 4500;
    case 4:  return 10000;
    case 5:  return 23000;
    case 6:  return 50000;
    case 7:  return 100000;
    case 8:  return 160000;
    case 9:  return 230000;
    case 10: return 310000;
    case 11: return 400000;
    case 12: return 500000;
    case 13: return 610000;
    case 14: return 730000;
    case 15: return 860000;
    case 16: return 1000000;
    case 17: return 1600000;
    case 18: return 2300000;
    case 19: return 3100000;
    case 20: return 4000000;
    case 21: return 5000000;
    case 22: return 6250000;
    case 23: return 7700000;
    case 24: return 10000000;
    case 25: return 16000000;
    case 26: return 24000000;
    case 27: return 34000000;
    case 28: return 46000000;
    case 29: return 61000000;
    case 30: return 79000000;
    case LVL_IMMORT: return 100000000;
   }

 return (0);

}
*/


int Diff(int level) {
    if (level <= 28)
        return 0;
    if (level == 29)
        return 1;
    if (level == 30)
        return 2;
    if (level == 31)
        return 4;
    if (level >= 32)
        return (level -30)*5;
    return 0;
}

int MXP(int level) {
    return (45 + 15*level);
}


/*int level_exp(int level)
{
 int xp;

 if (level == -1) return 0;
 if (level == 0) return 0;
 if (level == 1) return 1;

 xp = (5 * level + Diff(level)) * MXP(level);

 return (level_exp(level-1) + xp);
}*/


int level_exp(struct char_data *ch, int level) {
    int xpc[] = {0, 5, 7, 10, 10, 12, 15, 17, 20, 22, 25, 25, 27, 27, 27, 30, 30, 32, 32, 32, 35, 37, 37, 40, 40, 45, 45, 47, 47, 47, 50,
                 50, 52, 52, 55, 55, 55, 57, 57, 57, 60, 60, 62, 62, 62, 65, 67, 70, 72, 75, 77, 80, 82, 85, 87, 90, 92,
                 95, 97, 97, 100
                };
    int xp, base = 100;
    xp = base * xpc[level-1];

    if (level <= 0)
        return 0;

    else
        return (level_exp(ch, level -1) + xp * (1 + GET_REMORT(ch)*0.1));
}

int level_exp_mob(int level) {
     int xpc[] = {0, 5, 7, 10, 10, 12, 15, 17, 20, 22, 25, 25, 27, 27, 27, 30, 30, 32, 32, 32, 35, 37, 37, 40, 40, 45, 45, 47, 47, 47, 50,
                  50, 52, 52, 55, 55, 55, 57, 57, 57, 60, 60, 62, 62, 62, 65, 67, 70, 72, 75, 77, 80, 82, 85, 87, 90, 92,
                  95, 97, 97, 100
                 };
     int xp, base = 100;
     xp = base * xpc[level-1];

     if (level <= 0)
         return 0;

     else
         return (level_exp_mob(level -1) + xp);
 }




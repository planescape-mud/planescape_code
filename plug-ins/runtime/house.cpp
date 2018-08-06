/* ************************************************************************
*   File: house.c                                       Part of CircleMUD *
*  Usage: Handling of player houses                                       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "structs.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "interpreter.h"
#include "utils.h"
#include "house.h"
#include "constants.h"
#include "screen.h"
#include "planescape.h"

ACMD(do_fractions);


const char *fullname_fraction[NUM_FRACTS] = {
    "Не Знающие",
    "Атхары",
    "Церковь Основы",
    "Черные Кабалисты",
    "Стражи Судьбы",
    "Пыльники",
    "Судьбоносцы",
    "Вселенские Закон",
    "Свободная Лига",
    "Грамония",
    "Стражи Закона",
    "Лига Революционеров",
    "Экзистенциалисты",
    "Общество Впечатлений",
    "Невидимый порядок",
    "Хаосцисты"
};

const char *name_fraction[NUM_FRACTS] = {
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "11",
    "12",
    "13",
    "14",
    "15"
};

ACMD(do_fractions)
{
    int i;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "Философские Фракции:\r\n");
    sprintf(buf + strlen(buf), "Название                Полное название\r\n"
            "-----------------------------------------------------------\r\n");

    for (i = 1; i < NUM_FRACTS; i++)
        sprintf(buf + strlen(buf), "%-24s%s\r\n", name_fraction[i], fullname_fraction[i]);

    send_to_char(buf, ch);
}

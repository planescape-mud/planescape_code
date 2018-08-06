/* ************************************************************************
*   File: weather.c                                     Part of CircleMUD *
*  Usage: functions handling time and the weather                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"


#include "structs.h"
#include "spells.h"
#include "utils.h"
#include "comm.h"
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "screen.h"

/* local functions */
void another_hour(int mode);
void weather_change(void);
void update_day(void);


void weather_and_time(int mode)
{
    another_hour(mode);
    if (mode)
        weather_change();
}

const int sunrise[][2] = { {8, 18},
{7, 19},
{7, 20},
{6, 20},
{5, 21},
{4, 21},
{4, 22},
{4, 22},
{5, 21},
{6, 21},
{6, 20},
{7, 20},
{7, 19},
{8, 19}
};

/* погода давление(1000мм) и влажность(100%) */
const int weather[][2] = {
    {950, 95},                  /* Sigil */
    {1000, 80}                  /* Elysium */
};

void another_hour(int mode)
{
    int i;
    int time_hours;

//  time_info.hours++;

    for (i = 0; i <= top_of_zone_table; ++i) {
        zone_table[i].time_info.hours++;
        if (mode) {
            time_hours = zone_table[i].time_info.hours;

            if (time_hours == sunrise[time_info.month][0]) {
                zone_table[i].weather_info.sunlight = SUN_RISE;
                send_to_weather(0, i);
            } else if (time_hours == sunrise[time_info.month][0] + 1) {
                zone_table[i].weather_info.sunlight = SUN_LIGHT;
                send_to_weather(1, i);
            } else if (time_hours == sunrise[time_info.month][1]) {
                zone_table[i].weather_info.sunlight = SUN_SET;
                send_to_weather(2, i);
            } else if (time_hours == sunrise[time_info.month][1] + 1) {
                zone_table[i].weather_info.sunlight = SUN_DARK;
                send_to_weather(3, i);
            }
        }

        if (zone_table[i].time_info.hours >= HOURS_PER_DAY) {   /* Changed by HHS due to bug ??? */
            zone_table[i].time_info.hours = 0;
            zone_table[i].time_info.day++;

            zone_table[i].weather_info.week_day++;
            if (zone_table[i].weather_info.week_day >= WEEK_CYCLE)
                zone_table[i].weather_info.week_day = 0;

            if (zone_table[i].time_info.day >= DAYS_PER_MONTH) {
                zone_table[i].time_info.day = 0;
                zone_table[i].time_info.month++;
                if (zone_table[i].time_info.month >= MONTHS_PER_YEAR) {
                    zone_table[i].time_info.month = 0;
                    zone_table[i].time_info.year++;
                }
            }
        }
    }


}

void weather_change(void)
{
    int i, time, sky = 255;

    for (i = 0; i <= top_of_zone_table; ++i) {
        if (zone_table[i].plane == PLAN_DUNGEON)
            continue;

        time = zone_table[i].time_info.hours;
        switch (time) {
            case 6:
                sky = SKY_CLOUDLESS;
                break;
            case 8:
                if (dice(1, 20) == 1)
                    sky = SKY_LIGHTNING;
                else
                    sky = SKY_CLOUDLESS;
                break;
            case 12:
                if (dice(1, 20) < 10)
                    sky = SKY_CLOUDLESS;
                else
                    sky = SKY_CLOUDY;
                break;
            case 17:
                if (dice(1, 20) < 15)
                    sky = SKY_CLOUDY;
                else
                    sky = SKY_RAINING;
                break;
            case 23:
                switch (number(1, 4)) {
                    case 1:
                        sky = SKY_LIGHTNING;
                        break;
                    case 2:
                        sky = SKY_CLOUDLESS;
                        break;
                    case 3:
                        sky = SKY_CLOUDY;
                        break;
                    case 4:
                        sky = SKY_RAINING;
                        break;
                }
                break;
        }
        if (sky != 255) {
            send_to_weather(sky + 4, i);
            zone_table[i].weather_info.sky = sky;
        }
    }

    update_day();
}

int get_room_sky(int rnum)
{
    return GET_ROOM_SKY(rnum);
}


void update_day(void)
{
    int hour, i;

    for (i = 0; i <= top_of_world; i++) {
        hour = zone_table[world[i].zone].time_info.hours;

        if (ROOM_FLAGGED(i, ROOM_NOWEATHER) || SECT(i) == SECT_INSIDE) {
            world[i].weather.sky = SKY_LIGHTNING;
            world[i].weather.light = 100;
            world[i].weather.fog = 0;
            world[i].weather.rain = 0;
        } else {
            world[i].weather.sky = zone_table[world[i].zone].weather_info.sky;
            if (hour >= 23 && hour < 5)
                world[i].weather.light = 10;
            else if (hour >= 5 && hour < 7)
                world[i].weather.light = 20;
            else if (hour >= 7 && hour < 8)
                world[i].weather.light = 30;
            else if (hour >= 8 && hour < 10)
                world[i].weather.light = 50;
            else if (hour >= 10 && hour < 12)
                world[i].weather.light = 80;
            else if (hour >= 12 && hour < 16)
                world[i].weather.light = 100;
            else if (hour >= 16 && hour < 18)
                world[i].weather.light = 90;
            else if (hour >= 18 && hour < 20)
                world[i].weather.light = 60;
            else if (hour >= 20 && hour < 21)
                world[i].weather.light = 40;
            else if (hour >= 21 && hour < 23)
                world[i].weather.light = 20;
            world[i].weather.fog = 0;
            world[i].weather.rain = 0;
        }
    }
}

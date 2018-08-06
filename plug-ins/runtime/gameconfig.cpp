/* ************************************************************************
*   File: config.c                                      Part of CircleMUD *
*  Usage: Configuration of various aspects of CircleMUD operation         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "structs.h"
#include "interpreter.h"        /* alias_data definition for structs.h */
#include "utils.h"

#define YES 1
#define FALSE 0
#define NO 0

/*
 * Below are several constants which you can change to alter certain aspects
 * of the way CircleMUD acts.  Since this is a .c file, all you have to do
 * to change one of the constants (assuming you keep your object files around)
 * is change the constant in this file and type 'make'.  Make will recompile
 * this file and relink; you don't have to wait for the whole thing to
 * recompile as you do if you change a header file.
 *
 * I realize that it would be slightly more efficient to have lots of
 * #defines strewn about, so that, for example, the autowiz code isn't
 * compiled at all if you don't want to use autowiz.  However, the actual
 * code for the various options is quite small, as is the computational time
 * in checking the option you've selected at run-time, so I've decided the
 * convenience of having all your options in this one file outweighs the
 * efficency of doing it the other way.
 *
 */

/****************************************************************************/
/****************************************************************************/

/* GAME PLAY OPTIONS */


/* number of movement points it costs to holler */

/* How many ticks before a player is sent to the void or idle-rented. */
int idle_void = 5;
int idle_rent_time = 30;

/* This level and up is immune to idling, LVL_IMPL+1 will disable it. */
int idle_max_level = LVL_GOD;

/* should items in death traps automatically be junked? */

/*
 * Whether you want items that immortals load to appear on the ground or not.
 * It is most likely best to set this to 'YES' so that something else doesn't
 * grab the item before the immortal does, but that also means people will be
 * able to carry around things like boards.  That's not necessarily a bad
 * thing, but this will be left at a default of 'NO' for historic reasons.
 */
int load_into_inventory = YES;

/* "okay" etc. */
const char *OK = "Хорошо.\r\n";
const char *NOPERSON = "Нет такого создания в этом мире.\r\n";
const char *NOEFFECT = "Ваши потуги оказались напрасными.\r\n";
const char *ONLYSAME = "Только на себя или согруппника.\r\n";

/*
 * You can define or not define TRACK_THOUGH_DOORS, depending on whether
 * or not you want track to find paths which lead through closed or
 * hidden doors. A setting of 'NO' means to not go through the doors
 * while 'YES' will pass through doors to find the target.
 */
int track_through_doors = YES;

/****************************************************************************/
/****************************************************************************/


/* RENT/CRASHSAVE OPTIONS */

/*
 * Should the MUD allow you to 'rent' for free?  (i.e. if you just quit,
 * your objects are saved at no cost, as in Merc-type MUDs.)
 */
int free_rent = NO;

/*
 * Should the game automatically save people?  (i.e., save player data
 * every 4 kills (on average), and Crash-save as defined below.  This
 * option has an added meaning past bpl13.  If auto_save is YES, then
 * the 'save' command will be disabled to prevent item duplication via
 * game crashes.
 */
int auto_save = YES;

/*
 * if auto_save (above) is yes, how often (in minutes) should the MUD
 * Crash-save people's objects?   Also, this number indicates how often
 * the MUD will Crash-save players' houses.
 */
int autosave_time = 1;

/* The period of free rent after crash or forced-rent in hours*/
int free_crashrent_period = 2;
int free_rebootrent_period = 48;

/****************************************************************************/
/****************************************************************************/

/* ROOM NUMBERS */

/* virtual number of room that mortals should enter at */

room_vnum mortal_start_room = 5603;

//room_vnum mortal_start_room = 100;

/* virtual number of room that immorts should enter at by default */
room_vnum immort_start_room = 100;      /* place  in castle */

/* virtual number of room that frozen players should enter at */
room_vnum frozen_start_room = 119;      /* something in castle */

/* virtual number of room that helled players should enter at */
room_vnum helled_start_room = 119;      /* something in castle */

room_vnum jail_start_room = 198;        //тюрма

room_vnum named_start_room = 5603;

//room_vnum named_start_room  = 100;

room_vnum unreg_start_room = 119;


/****************************************************************************/
/****************************************************************************/


/* GAME OPERATION OPTIONS */

/*
 * IP address to which the MUD should bind.  This is only useful if
 * you're running Circle on a host that host more than one IP interface,
 * and you only want to bind to *one* of them instead of all of them.
 * Setting this to NULL (the default) causes Circle to bind to all
 * interfaces on the host.  Otherwise, specify a numeric IP address in
 * dotted quad format, and Circle will only bind to that IP address.  (Of
 * course, that IP address must be one of your host's interfaces, or it
 * won't work.)
 */
#ifdef TEST_CODE
const char *DFLT_IP = "127.0.0.1";
#else
const char *DFLT_IP = NULL;     /* bind to all interfaces */
#endif

/* const char *DFLT_IP = "192.168.1.1";  -- bind only to one interface */


/* maximum number of players allowed before game starts to turn people away */
#ifdef TEST_CODE
int max_playing = 4;
#else
int max_playing = 300;
#endif

/* maximum number of password attempts before disconnection */
int max_bad_pws = 3;

/*
 * Rationale for enabling this, as explained by naved@bird.taponline.com.
 *
 * Usually, when you select ban a site, it is because one or two people are
 * causing troubles while there are still many people from that site who you
 * want to still log on.  Right now if I want to add a new select ban, I need
 * to first add the ban, then SITEOK all the players from that site except for
 * the one or two who I don't want logging on.  Wouldn't it be more convenient
 * to just have to remove the SITEOK flags from those people I want to ban
 * rather than what is currently done?
 */
int siteok_everyone = TRUE;

/*
 * Some nameservers are very slow and cause the game to lag terribly every
 * time someone logs in.  The lag is caused by the gethostbyaddr() function
 * which is responsible for resolving numeric IP addresses to alphabetic names.
 * Sometimes, nameservers can be so slow that the incredible lag caused by
 * gethostbyaddr() isn't worth the luxury of having names instead of numbers
 * for players' sitenames.
 *
 * If your nameserver is fast, set the variable below to NO.  If your
 * nameserver is slow, of it you would simply prefer to have numbers
 * instead of names for some other reason, set the variable to YES.
 *
 * You can experiment with the setting of nameserver_is_slow on-line using
 * the SLOWNS command from within the MUD.
 */

int nameserver_is_slow = YES;


const char *MENU =
    "\r\n"
    "&4&Y Добро пожаловать в планы! &w\r\n"
    "---------------------------\r\n"
    " 0) Отсоединиться.\r\n"
    " &R1) Начать игру.&w\r\n"
    " 2) Ввести описание Вашего персонажа.\r\n"
    " 3) Получить тренировочную экипировку.\r\n"
    " 4) Статистика игры.\r\n"
    " 8) Изменить пароль.\r\n" " 9) Удалить персонажа.\r\n" "\r\n" "Ваш выбор: ";



const char *WELC_MESSG = "\r\n" "Добро пожаловать в МПМ \"Грани Мира\".\r\n" "\r\n";

const char *START_MESSG = "\r\n";

/****************************************************************************/
/****************************************************************************/


int calc_loadroom(struct char_data *ch)
{
    if (IS_IMMORTAL(ch))
        return (immort_start_room);
    else if (PLR_FLAGGED(ch, PLR_FROZEN))
        return (frozen_start_room);
    else
        return GET_LOADROOM(ch);

//  return (mortal_start_room);
}

/* ************************************************************************
*   File: act.wizard.c                                  Part of CircleMUD *
*  Usage: Player-level god commands and other goodies                     *
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
#include "ban.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "house.h"
#include "screen.h"
#include "constants.h"
#include "mobmax.h"
#include "pk.h"
#include "xspells.h"
#include "xobj.h"
#include "case.h"
#include "xsave.h"
#include "xtempl.h"
#include "xboot.h"
#include "xscripts.h"
#include "events.h"
#include "planescape.h"
#include "registration.h"
#include "mudstats.h"
#include "dlfileop.h"
#include "mudfile.h"

/* local functions */
void show_char_rent(struct char_data *ch, char *name);
void show_obj_location(struct char_data *ch, int vnum);
int perform_set(struct char_data *ch, struct char_data *vict, int mode, char *val_arg);
void perform_immort_invis(struct char_data *ch, int level);
room_rnum find_target_room(struct char_data *ch, char *rawroomstr);
void do_stat_room(struct char_data *ch);
void do_stat_object(struct char_data *ch, struct obj_data *j);
void do_stat_character(struct char_data *ch, struct char_data *k);
void stop_snooping(struct char_data *ch);
void print_zone_to_buf(char *bufptr, zone_rnum zone);

#define MAX_TIME 0x7fffffff


ACMD(do_noplayer)
{
    int i, zn, rn = 0;

    if (!*argument) {
        send_to_char("Формат: безигроков <номер зоны>\r\n", ch);
        return;
    }

    /* if (!is_positive_number(argument))
       {
       send_to_char("Номер зоны должен быть числом.\r\n",ch);
       return;
       } */

    i = atoi(argument);

    for (zn = 0; zn <= top_of_zone_table; zn++)
        if (zone_table[zn].number == i)
            rn = zn;



    if (!zone_table[rn].noplayer) {
        zone_table[rn].noplayer = 1;
        send_to_charf(ch, "Установили флаг 'без игроков' для зоны #%d\r\n", i);
    } else {
        zone_table[rn].noplayer = 0;
        send_to_charf(ch, "Сняли флаг 'без игроков' для зоны #%d\r\n", i);
    }

}

ACMD(do_mtitle)
{
    struct descriptor_data *d;
    struct char_data *vict;
    char arg[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "%s", CCICYN(ch, C_NRM));
        send_to_charf(ch, "Титул запросили:\r\n");
        send_to_charf(ch, "%s", CCNRM(ch, C_NRM));
        for (d = descriptor_list; d; d = d->next) {
            if (STATE(d) != CON_PLAYING)
                continue;

            if (d->original)
                vict = d->original;
            else if (!(vict = d->character))
                continue;

            if (GET_RTITLE(vict))
                send_to_charf(ch, "%s%s : %s%s\r\n", CCCYN(ch, C_NRM), GET_NAME(vict),
                              GET_RTITLE(vict), CCNRM(ch, C_NRM));
        }
        send_to_charf(ch, "\r\nФормат: утитул [одобрить|отклонить] имя_песонажа\r\n");
        return;
    }

    if (isname(arg, "одобрить")) {

        one_argument(argument, arg);

        if (!(vict = get_char_vis(ch, arg, FIND_CHAR_WORLD))) {
            send_to_char("Кому одобрить тутул?\r\n", ch);
            return;
        };

        set_title(vict, GET_RTITLE(vict));
        GET_RTITLE(vict) = NULL;
        send_to_charf(ch, "Вы доборили тутул &C%s&n для &C%s&n\r\n", GET_TITLE(vict),
                      GET_PAD(vict, 2));
        return;
    }

    if (isname(arg, "отклонить")) {

        one_argument(argument, arg);

        if (!(vict = get_char_vis(ch, arg, FIND_CHAR_WORLD))) {
            send_to_char("Кому отклонить тутул?\r\n", ch);
            return;
        };

        send_to_charf(ch, "Вы отклонили тутул &C%s&n для &C%s&n\r\n", GET_RTITLE(vict),
                      GET_PAD(vict, 2));
        GET_RTITLE(vict) = NULL;
        send_to_charf(vict, "Боги отклонили Ваш титул.\r\n");

        return;
    }


}

ACMD(do_whoall)
{
    int i;
    struct char_data *k;
    char tempname[MAX_INPUT_LENGTH];
    char wabuf[MAX_EXTEND_LENGTH];

    *wabuf = '\0';

    sprintf(wabuf, "&CСписок всех персонажей мада:\r\n");
    sprintf(wabuf + strlen(wabuf),
            "Имя                                  Уров E-mail                        Последний IP&n\r\n");

    for (i = 0; i <= top_of_p_table; i++) {
        if (player_table[i].unique < 0)
            continue;

        strcpy(tempname, player_table[i].name);

        k = new Player();
        if (load_char(tempname, k) > -1) {
            sprintf(wabuf + strlen(wabuf), "%s", CCCYN(ch, C_NRM));
            sprintf(wabuf + strlen(wabuf), "%-35s %s %2d %-27s %-16s\r\n",
                    race_or_title2(k),
                    (IS_SET(PLR_FLAGS(k, PLR_DELETED), PLR_DELETED)) ? "-" : " ",
                    GET_LEVEL(k), GET_EMAIL(k), GET_LASTIP(k));
        } else {
            sprintf(wabuf + strlen(wabuf), "Отсутствует файл данных на %s\r\n", tempname);
        }
        delete k;

        sprintf(wabuf + strlen(wabuf), "%s", CCNRM(ch, C_NRM));
    }
    page_string(ch->desc, wabuf, 1);
}

ACMD(do_delete)
{
    int ptnum;
    bool miss = FALSE, full = FALSE;
    struct char_data *victim = NULL;
    char tempname[MAX_INPUT_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (!*argument) {
        send_to_char("Формат: удалить <полное имя игрока>\r\n", ch);
        return;
    }

    argument = one_argument(argument, arg);

    if (isname(arg, "полностью"))
        full = TRUE;

    if (full && !*argument) {
        send_to_charf(ch, "Кого Вы хотите удалить полностью?\r\n");
        return;
    } else if (!*argument)
        argument = arg;

    if ((ptnum = get_ptable_by_name(argument)) < 0) {
        send_to_char("Такого персонажа нет.\r\n", ch);
        return;
    }

    strcpy(tempname, player_table[ptnum].name);
    if ((victim = get_player_vis(ch, tempname, FIND_CHAR_WORLD)) != NULL) {

        act("$n удален$g за нарушение правил.", FALSE, victim, 0, 0, TO_ROOM);
        send_to_char("&RВы удалены за нарушение правил.&n\r\n\r\n", victim);

        SET_BIT(PLR_FLAGS(victim, PLR_DELETED), PLR_DELETED);
        strcpy(victim->delete_name, GET_PAD(ch, 4));
        victim->delete_level = GET_LEVEL(ch);
        //save_char(victim,NOWHERE);

        if (STATE(victim->desc) == CON_PLAYING)
            STATE(victim->desc) = CON_DISCONNECT;
        else
            STATE(victim->desc) = CON_CLOSE;

        if (full)
            delete_char(tempname);
    } else if (full)
        delete_char(tempname);
    else {
        victim = new Player();
        if (load_char(tempname, victim) > -1) {
            if (PLR_FLAGGED(victim, PLR_DELETED)) {
                if (victim->delete_level > GET_LEVEL(ch))
                    send_to_char("У Вас недостаточно прав.\r\n", ch);
                else {
                    REMOVE_BIT(PLR_FLAGS(victim, PLR_DELETED), PLR_DELETED);
                    save_char(victim, NOWHERE);
                }
                miss = TRUE;
            } else if (GET_LEVEL(victim) > GET_LEVEL(ch) && !GET_COMMSTATE(ch))
                send_to_char("У Вас недостаточно прав.\r\n", ch);
            else {
                SET_BIT(PLR_FLAGS(victim, PLR_DELETED), PLR_DELETED);
                strcpy(victim->delete_name, GET_PAD(ch, 4));
                victim->delete_level = GET_LEVEL(ch);
                save_char(victim, NOWHERE);
            }
        }
        delete victim;
    }

    if (!miss) {
        send_to_charf(ch, "Вы %sудалили персонажа '%s'\r\n", full ? "полностью " : "", tempname);
        sprintf(buf, "%s %sудалил%s персонажа '%s'", GET_NAME(ch), full ? "полностью " : "",
                GET_CH_SUF_1(ch), tempname);
    } else {
        send_to_charf(ch, "Вы отменили удаление персонажа '%s'\r\n", tempname);
        sprintf(buf, "%s отменил%s удаление персонажа '%s'", GET_NAME(ch), GET_CH_SUF_1(ch),
                tempname);
    }
    mudlog(buf, CMP, LVL_IMPL, TRUE);

}

ACMD(do_peace)
{
    struct char_data *vict, *next_v;
    char buf[MAX_STRING_LENGTH];

    act("Все бои по Вашей воле прекращены.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n сделал$g величественный взмах рукой и в комнате воцарился мир.", FALSE, ch, 0, 0,
        TO_ROOM);


    for (vict = world[ch->in_room].people; vict; vict = next_v) {
        next_v = vict->next_in_room;
        if (FIGHTING(vict)) {
            if (FIGHTING(FIGHTING(vict)) == vict)
                stop_fighting(FIGHTING(vict), FALSE);
            stop_fighting(vict, FALSE);
        }
    }

    sprintf(buf, "%s установил%s мир в #%d", GET_NAME(ch), GET_CH_SUF_1(ch),
            GET_ROOM_VNUM(IN_ROOM(ch)));
    mudlog(buf, CMP, LVL_IMPL, TRUE);
}

ACMD(do_settime)
{
    int hours, day, month;
    sh_int year;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg);
    if (!*arg) {
        send_to_char("Способ использования:\r\n", ch);
        send_to_char("\tсвремя <час> <день> <месяц> <год>\r\n", ch);
        sprintf(buf, "Где час от 0 до %i, день от 0 до %i, месяц от 0 до %i\r\n",
                HOURS_PER_DAY - 1, DAYS_PER_MONTH - 1, MONTHS_PER_YEAR - 1);
        send_to_char(buf, ch);
        return;
    }

    hours = atoi(arg);
    if (hours < 0 || hours > HOURS_PER_DAY - 1) {
        send_to_char("Способ использования:\r\n", ch);
        send_to_char("\tсвремя <час> <день> <месяц> <год>\r\n", ch);
        sprintf(buf, "Где час от 0 до %i, день от 0 до %i, месяц от 0 до %i\r\n",
                HOURS_PER_DAY - 1, DAYS_PER_MONTH - 1, MONTHS_PER_YEAR - 1);
        send_to_char(buf, ch);
        return;
    }
    argument = one_argument(argument, arg);
    day = atoi(arg);
    if (!*arg || day < 0 || day > DAYS_PER_MONTH - 1) {
        send_to_char("Способ использования:\r\n", ch);
        send_to_char("\tсвремя <час> <день> <месяц> <год>\r\n", ch);
        sprintf(buf, "Где час от 0 до %i, день от 0 до %i, месяц от 0 до %i\r\n",
                HOURS_PER_DAY - 1, DAYS_PER_MONTH - 1, MONTHS_PER_YEAR - 1);
        send_to_char(buf, ch);
        return;
    }

    argument = one_argument(argument, arg);
    month = atoi(arg);
    if (!*arg || month < 0 || month > MONTHS_PER_YEAR - 1) {
        send_to_char("Способ использования:\r\n", ch);
        send_to_char("\tсвремя <час> <день> <месяц> <год>\r\n", ch);
        sprintf(buf, "Где час от 0 до %i, день от 0 до %i, месяц от 0 до %i\r\n",
                HOURS_PER_DAY - 1, DAYS_PER_MONTH - 1, MONTHS_PER_YEAR - 1);
        send_to_char(buf, ch);
        return;
    }

    argument = one_argument(argument, arg);
    year = atoi(arg);
    if (!*arg || year < 1) {
        send_to_char("Способ использования:\r\n", ch);
        send_to_char("\tсвремя <час> <день> <месяц> <год>\r\n", ch);
        sprintf(buf, "Где час от 0 до %i, день от 0 до %i, месяц от 0 до %i\r\n",
                HOURS_PER_DAY - 1, DAYS_PER_MONTH - 1, MONTHS_PER_YEAR - 1);
        send_to_char(buf, ch);
        return;
    }

    time_info.hours = hours;
    time_info.day = day;
    time_info.month = month;
    time_info.year = year;
    set_time(time_info);

    if (IS_NPC(ch))
        return;
    sprintf(buf, "Сейчас ");
    switch (time_info.hours % 24) {
        case 0:
            sprintf(buf + strlen(buf), "полночь, ");
            break;
        case 1:
            sprintf(buf + strlen(buf), "1 час ночи, ");
            break;
        case 2:
        case 3:
        case 4:
            sprintf(buf + strlen(buf), "%d часа ночи, ", time_info.hours);
            break;
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
            sprintf(buf + strlen(buf), "%d часов утра, ", time_info.hours);
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
            sprintf(buf + strlen(buf), "%d часа пополудни, ", time_info.hours - 12);
            break;
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
            sprintf(buf + strlen(buf), "%d часов вечера, ", time_info.hours - 12);
            break;
    }

    strcat(buf, weekdays[weather_info.week_day]);
    switch (weather_info.sunlight) {
        case SUN_DARK:
            strcat(buf, ", темно");
            break;
        case SUN_SET:
            strcat(buf, ", закат");
            break;
        case SUN_LIGHT:
            strcat(buf, ", день");
            break;
        case SUN_RISE:
            strcat(buf, ", рассвет");
            break;
    }
    strcat(buf, ".\r\n");
    send_to_char(buf, ch);

    day = time_info.day + 1;    /* day in [1..35] */
    *buf = '\0';
    sprintf(buf + strlen(buf), "%s, %dй День, Год %d",
            month_name[(int) time_info.month], day, time_info.year);
    switch (weather_info.season) {
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

ACMD(do_xname)
{
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, buf);

    if (!*buf) {
        send_to_char("Запретить какое имя?\r\n", ch);
        return;
    }

    DLFileAppend file(MudFile(mud->xnameFile));

    if (!file.open()) {
        log("ОШИБКА: Не могу открыть файл плохих имен %s", file.getCPath());
        return;
    }

    DLString name(buf);

    name.toLower();

    file.printf("%s\n", name.c_str());

    send_to_charf(ch, "%s добавлено в xname!\r\n", name.c_str());
    Read_Invalid_List();

    mudlogf(CMP, LVL_IMPL, TRUE, "%s запретил%s имя '%s'",
            GET_NAME(ch), GET_CH_SUF_1(ch), name.c_str());
}

ACMD(do_zname)
{
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, buf);

    if (!*buf) {
        send_to_char("Одобрить какое имя?\r\n", ch);
        return;
    }

    DLFileAppend file(MudFile(mud->znameFile));

    if (!file.open()) {
        log("ОШИБКА: Не могу открыть файл хороших имен %s", file.getCPath());
        return;
    }

    DLString name(buf);

    name.toLower();

    file.printf("%s\n", name.c_str());

    send_to_charf(ch, "%s добавлено в zname!\r\n", name.c_str());
    Read_Valid_List();

    mudlogf(CMP, LVL_IMPL, TRUE, "%s одобрил%s имя '%s'",
            GET_NAME(ch), GET_CH_SUF_1(ch), name.c_str());
}

ACMD(do_echo)
{
    char buf[MAX_STRING_LENGTH];

    skip_spaces(&argument);

    if (!*argument)
        send_to_char("И что Вы хотите выразить столь красочно ?\r\n", ch);
    else {
        if (subcmd == SCMD_EMOTE) {
            if (IS_NPC(ch) && !AFF_FLAGGED(ch, AFF_CHARM))
                sprintf(buf, "$n %s.&n", argument);
            else
                //sprintf(buf, "*$n %s.&n", argument);
                sprintf(buf, "$n %s.&n", argument);
        } else
            strcpy(buf, argument);
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
    }
}


ACMD(do_send)
{
    struct char_data *vict;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    half_chop(argument, arg, buf);

    if (!*arg) {
        send_to_char("Послать что и кому (не путать с куда и кого:)\r\n", ch);
        return;
    }
    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_WORLD))) {
        send_to_char(NOPERSON, ch);
        return;
    }
    send_to_charf(vict, "%s\r\n", buf);
    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char("Послано.\r\n", ch);
    else {
        sprintf(buf2, "Вы послали '%s' %s.\r\n", buf, GET_PAD(vict, 2));
        send_to_char(buf2, ch);
    }
}



/* take a string, and return an rnum.. used for goto, at, etc.  -je 4/6/93 */
room_rnum find_target_room(struct char_data *ch, char *rawroomstr)
{
    room_vnum tmp;
    room_rnum location;
    struct char_data *target_mob;
    struct obj_data *target_obj;
    char roomstr[MAX_INPUT_LENGTH];

    one_argument(rawroomstr, roomstr);

    if (!*roomstr) {
        send_to_char("Укажите номер или название комнаты.\r\n", ch);
        return (NOWHERE);
    }
    if (IS_DIGIT(*roomstr) && !strchr(roomstr, '.')) {
        tmp = atoi(roomstr);
        if ((location = real_room(tmp)) < 0) {
            send_to_char("Нет комнаты с таким номером.\r\n", ch);
            return (NOWHERE);
        }
    } else if ((target_mob = get_char_vis(ch, roomstr, FIND_CHAR_WORLD)) != NULL)
        location = target_mob->in_room;
    else if ((target_obj = get_obj_vis(ch, roomstr)) != NULL) {
        if (target_obj->in_room != NOWHERE)
            location = target_obj->in_room;
        else {
            send_to_char("Этот объект Вам недоступен.\r\n", ch);
            return (NOWHERE);
        }
    } else {
        send_to_char("В округе нет похожего предмета или создания.\r\n", ch);
        return (NOWHERE);
    }

    /* a location has been found -- if you're < GRGOD, check restrictions. */
    if (!IS_GOD(ch)) {
        if (ROOM_FLAGGED(location, ROOM_GODROOM) && GET_LEVEL(ch) < LVL_GOD) {
            send_to_char("Вы не столь божественны, чтобы получить доступ в это комнату!\r\n", ch);
            return (NOWHERE);
        }
        if (ROOM_FLAGGED(location, ROOM_PRIVATE) &&
            world[location].people && world[location].people->next_in_room) {
            send_to_char("Что-то интимное вершится в этой комнате - третий явно лишний.\r\n", ch);
            return (NOWHERE);
        }
        if (ROOM_FLAGGED(location, ROOM_HOUSE)) {
            send_to_char("Частная собственность - посторонним в ней делать нечего!\r\n", ch);
            return (NOWHERE);
        }
    }
    return (location);
}



ACMD(do_at)
{
    char command[MAX_INPUT_LENGTH];
    room_rnum location, original_loc;
    char buf[MAX_STRING_LENGTH];

    half_chop(argument, buf, command);
    if (!*buf) {
        send_to_char("Необходимо указать номер или название комнаты.\r\n", ch);
        return;
    }

    if (!*command) {
        send_to_char("Что Вы собираетесь там делать?\r\n", ch);
        return;
    }

    if ((location = find_target_room(ch, buf)) < 0)
        return;

    /* a location has been found. */
    original_loc = ch->in_room;
    char_from_room(ch);
    char_to_room(ch, location);
    command_interpreter(ch, command);

    sprintf(buf, "%s выполнил%s '%s' в #%d", GET_NAME(ch), GET_CH_SUF_1(ch),
            command, GET_ROOM_VNUM(location));
    mudlog(buf, CMP, GET_LEVEL(ch), TRUE);


    /* check if the char is still there */
    if (ch->in_room == location) {
        char_from_room(ch);
        char_to_room(ch, original_loc);
    }
    check_horse(ch);
}


ACMD(do_goto)
{
    char buf[MAX_STRING_LENGTH];
    room_rnum location;

    if ((location = find_target_room(ch, argument)) < 0)
        return;

    if (!GET_COMMSTATE(ch)) {
        if (POOFOUT(ch))
            sprintf(buf, "%s", POOFOUT(ch));
        else
            strcpy(buf, "$n растворил$u в клубах дыма.");
    } else
        strcpy(buf, "$n взял$g свиток возврата в левую рукую.\r\n"
               "$n зачитал$g свиток возврата.\r\n" "$n исчез$q.");

    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);

    if (get_horse_on(ch))
        char_from_room(get_horse_on(ch));

    char_to_room(ch, location);

    if (get_horse_on(ch))
        char_to_room(get_horse_on(ch), location);

    //check_horse(ch);


    if (!GET_COMMSTATE(ch) && POOFIN(ch))
        sprintf(buf, "%s", POOFIN(ch));
    else
        strcpy(buf, "$n возник$q посреди комнаты.");

    act(buf, TRUE, ch, 0, 0, TO_ROOM);
    look_at_room(ch, 0);


    sprintf(buf, "%s переместил%s в #%d", GET_NAME(ch), GET_CH_SUF_2(ch), GET_ROOM_VNUM(location));
    mudlog(buf, CMP, LVL_IMPL, TRUE);

}



ACMD(do_trans)
{
    struct descriptor_data *i;
    struct char_data *victim;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, buf);
    if (!*buf)
        send_to_char("Кого Вы хотите переместить ?\r\n", ch);
    else if (str_cmp("all", buf) && str_cmp("все", buf)) {
        if (!(victim = find_char(ch, buf)))
            send_to_char(NOPERSON, ch);
        else if (victim == ch)
            send_to_char("Себя?\r\n", ch);
        else {
            if ((GET_LEVEL(ch) < GET_LEVEL(victim)) && !IS_NPC(victim) && !GET_COMMSTATE(ch)) {
                send_to_char("Бога рангом выше Вашего?\r\n", ch);
                return;
            }
            act("$n растворил$u в клубах дыма.", FALSE, victim, 0, 0, TO_ROOM);
            char_from_room(victim);
            char_to_room(victim, ch->in_room);
            check_horse(victim);
            act("$n появил$u, окутанн$w розовым туманом.", FALSE, victim, 0, 0, TO_ROOM);
            act("$n призвал$g Вас!", FALSE, ch, 0, victim, TO_VICT);
            look_at_room(victim, 0);
            sprintf(buf, "%s призвал%s к себе %s", GET_NAME(ch), GET_CH_SUF_1(ch),
                    GET_PAD(victim, 3));
            mudlog(buf, CMP, LVL_IMPL, TRUE);
        }
    } else {                    /* Trans All */
        if (!IS_HIGOD(ch)) {
            send_to_char("У Вас не хватит силы переместить всех сразу.\r\n", ch);
            return;
        }

        for (i = descriptor_list; i; i = i->next)
            if (STATE(i) == CON_PLAYING && i->character && i->character != ch) {
                victim = i->character;
                if (GET_LEVEL(victim) >= GET_LEVEL(ch) && !GET_COMMSTATE(ch))
                    continue;
                act("$n растворил$u в клубах дыма.", FALSE, victim, 0, 0, TO_ROOM);
                char_from_room(victim);
                char_to_room(victim, ch->in_room);
                check_horse(victim);
                act("$n появил$u, окутанн$w розовым туманом.", FALSE, victim, 0, 0, TO_ROOM);
                act("$n призвал$g Вас!", FALSE, ch, 0, victim, TO_VICT);
                look_at_room(victim, 0);
                sprintf(buf, "%s призвал%s к себе %s", GET_NAME(ch), GET_CH_SUF_1(ch),
                        GET_PAD(victim, 3));
                mudlog(buf, CMP, LVL_IMPL, TRUE);
            }
        send_to_char(OK, ch);
    }
}


ACMD(do_teleport)
{
    struct char_data *victim;
    room_rnum target;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    two_arguments(argument, buf, buf2);

    if (!*buf)
        send_to_char("Кого Вы хотите переместить?\r\n", ch);
    else if (!(victim = get_char_vis(ch, buf, FIND_CHAR_WORLD)))
        send_to_char(NOPERSON, ch);
    else if (victim == ch)
        send_to_char("Используйте &Cидти&n для собственного перемещения.\r\n", ch);
    else if (GET_LEVEL(victim) >= GET_LEVEL(ch) && !GET_COMMSTATE(ch))
        send_to_char("У Вас не хватит сил.\r\n", ch);
    else if (!*buf2)
        act("Куда Вы хотите $S переместить?", FALSE, ch, 0, victim, TO_CHAR);
    else if ((target = find_target_room(ch, buf2)) >= 0) {
        send_to_char(OK, ch);
        act("$n растворил$u в клубах дыма.", FALSE, victim, 0, 0, TO_ROOM);
        char_from_room(victim);
        char_to_room(victim, target);
        check_horse(victim);
        act("$n появил$u, окутанн$w розовым туманом.", FALSE, victim, 0, 0, TO_ROOM);
        act("$n переместил$g Вас!", FALSE, ch, 0, (char *) victim, TO_VICT);
        look_at_room(victim, 0);
        sprintf(buf, "%s переместил%s %s в #%d", GET_NAME(ch), GET_CH_SUF_1(ch),
                GET_PAD(victim, 3), GET_ROOM_VNUM(target));
        mudlog(buf, CMP, LVL_IMPL, TRUE);
    }
}

ACMD(do_vnum)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    half_chop(argument, buf, buf2);

    if (!*buf || !*buf2 || (!is_abbrev(buf, "mob") && !is_abbrev(buf, "obj") &&
                            !is_abbrev(buf, "монстр") && !is_abbrev(buf, "предмет"))) {
        send_to_char("Формат: вномер [ предмет | монстр ] <имя>\r\n", ch);
        return;
    }
    if (is_abbrev(buf, "mob") || is_abbrev(buf, "монстр"))
        if (!vnum_mobile(buf2, ch))
            send_to_char("Нет монстра с таким именем.\r\n", ch);

    if (is_abbrev(buf, "obj") || is_abbrev(buf, "предмет"))
        if (!vnum_object(buf2, ch))
            send_to_char("Нет предмета с таким названием.\r\n", ch);
}



void do_stat_room(struct char_data *ch)
{
    struct extra_descr_data *desc;
    struct room_data *rm = &world[ch->in_room];
    struct room_affect_data *af;
    int i, found;
    struct obj_data *j;
    struct char_data *k;
    struct db_load_data *db_data;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    sprintf(buf, "Комната: %s%s%s\r\n", CCCYN(ch, C_NRM), rm->name, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);

    sprinttype(rm->sector_type, sector_types, buf2);
    sprintf(buf, "Зона: [%3d], VNum: [%s%5d%s], RNum: [%5d], Тип  сектора: %s\r\n",
            zone_table[rm->zone].number,
            CCGRN(ch, C_NRM), rm->number, CCNRM(ch, C_NRM), ch->in_room, buf2);
    send_to_char(buf, ch);

    sprintbits(rm->room_flags, room_bits, buf2, ",");
    sprintf(buf, "СпецПроцедура: %s, Флаги: %s\r\n",
            (rm->func == NULL) ? "(нет)" : rm->func_name, buf2);
    send_to_char(buf, ch);

    send_to_char("Описание:\r\n", ch);
    if (rm->description)
        send_to_charf(ch, "%s\r\n", rm->description);
    else
        send_to_char("  Нет.\r\n", ch);

    if (rm->ex_description) {
        sprintf(buf, "Доп. описание:%s", CCCYN(ch, C_NRM));
        for (desc = rm->ex_description; desc; desc = desc->next) {
            strcat(buf, " ");
            strcat(buf, desc->keyword);
        }
        strcat(buf, CCNRM(ch, C_NRM));
        send_to_char(strcat(buf, "\r\n"), ch);
    }
    sprintf(buf, "Живые существа:%s", CCYEL(ch, C_NRM));
    for (found = 0, k = rm->people; k; k = k->next_in_room) {
        if (!CAN_SEE(ch, k))
            continue;
        sprintf(buf2, "%s %s(%s)", found++ ? "," : "",
                GET_NAME(k), (!IS_NPC(k) ? "PC" : (!IS_MOB(k) ? "NPC" : "MOB")));
        strcat(buf, buf2);
        if (strlen(buf) >= 62) {
            if (k->next_in_room)
                send_to_char(strcat(buf, ",\r\n"), ch);
            else
                send_to_char(strcat(buf, "\r\n"), ch);
            *buf = found = 0;
        }
    }

    if (*buf)
        send_to_char(strcat(buf, "\r\n"), ch);
    send_to_char(CCNRM(ch, C_NRM), ch);

    if (rm->contents) {
        sprintf(buf, "Предметы:%s", CCGRN(ch, C_NRM));
        for (found = 0, j = rm->contents; j; j = j->next_content) {
            if (!CAN_SEE_OBJ(ch, j))
                continue;
            sprintf(buf2, "%s %s", found++ ? "," : "", j->short_description);
            strcat(buf, buf2);
            if (strlen(buf) >= 62) {
                if (j->next_content)
                    send_to_char(strcat(buf, ",\r\n"), ch);
                else
                    send_to_char(strcat(buf, "\r\n"), ch);
                *buf = found = 0;
            }
        }

        if (*buf)
            send_to_char(strcat(buf, "\r\n"), ch);
        send_to_char(CCNRM(ch, C_NRM), ch);
    }
    for (i = 0; i < NUM_OF_DIRS; i++) {
        if (rm->dir_option[i]) {
            if (rm->dir_option[i]->to_room == NOWHERE)
                sprintf(buf1, " %sNONE%s", CCCYN(ch, C_NRM), CCNRM(ch, C_NRM));
            else
                sprintf(buf1, "%s%5d%s", CCCYN(ch, C_NRM),
                        GET_ROOM_VNUM(rm->dir_option[i]->to_room), CCNRM(ch, C_NRM));
            sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2, ",");
            sprintf(buf,
                    "Выход %s%-5s%s:  Ведет в: [%s], Ключ: [%5d], Шифр: [&C%s&n], Название: %s, Тип: %s\r\n ",
                    CCCYN(ch, C_NRM), dirs[i], CCNRM(ch, C_NRM), buf1, rm->dir_option[i]->key,
                    rm->dir_option[i]->lock_code,
                    rm->dir_option[i]->keyword ? rm->dir_option[i]->keyword : "Нет(дверь)", buf2);
            send_to_char(buf, ch);
            if (rm->dir_option[i]->general_description)
                sprintf(buf, "  %s.\r\n", rm->dir_option[i]->general_description);
            else
                strcpy(buf, "  Нет описания выхода.\r\n");
            send_to_char(buf, ch);
        }
    }
    /*Эффекты */
    strcpy(buf, "Эффекты:\r\n");
    for (af = rm->affects; af; af = af->next)
        if (af->bitvector) {
            sprintbit(af->bitvector, room_affects, buf2, ",");
            sprintf(buf + strlen(buf), " &C%s&n время &G%d&n модиф &G%d&n", buf2, af->duration,
                    af->modifier);
        }
    strcat(buf, "\r\n");
    send_to_charf(ch, buf);

    /*Загрузки */
    strcpy(buf, "Загружается:\r\n");
    for (db_data = rm->mobiles; db_data; db_data = db_data->next)
        sprintf(buf + strlen(buf), "Монстр VNUM: %d Вероятность: %d%%\r\n",
                db_data->vnum, db_data->chance);
    for (db_data = rm->objects; db_data; db_data = db_data->next)
        sprintf(buf + strlen(buf), "Предмет VNUM: %d Вероятность: %d%%\r\n",
                db_data->vnum, db_data->chance);

    send_to_charf(ch, "%s\r\n", buf);
    /* check the room for a script */
}



void do_stat_object(struct char_data *ch, struct obj_data *j)
{
    int found;
    obj_vnum rnum, vnum;
    struct obj_data *j2;
    struct extra_descr_data *desc;
    struct C_obj_affected_type *af;
    struct material_data_list *m;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    vnum = GET_OBJ_VNUM(j);
    rnum = GET_OBJ_RNUM(j);
    sprintf(buf, "Название: &C%s&n Синонимы: &C%s&n\r\n",
            ((j->short_description) ? j->short_description : "нет"), j->name);
    send_to_char(buf, ch);
    sprinttype(GET_OBJ_TYPE(j), item_types, buf1);
    if (GET_OBJ_RNUM(j) >= 0 && obj_index[GET_OBJ_RNUM(j)].func)
        strcpy(buf2, obj_index[GET_OBJ_RNUM(j)].func_name);
    else
        strcpy(buf2, "(нет)");
    sprintf(buf,
            "ВНомер: [&G%d&n] РНомер: [&G%d&n] Зона: [&G%d&n] Тип: &C%s&n СпецПроцедура: &C%s&n\r\n",
            vnum, GET_OBJ_RNUM(j), GET_OBJ_ZONE(j), buf1, buf2);
    send_to_char(buf, ch);

    if (GET_OBJ_OWNER(j)) {
        sprintf(buf, "Владелец: [&G%d&n]", GET_OBJ_OWNER(j));
        send_to_char(buf, ch);
    }

    sprintf(buf, "Длинное описание: &C%s&n\r\n", ((j->description) ? j->description : "Нет"));
    send_to_char(buf, ch);

    if (j->ex_description) {
        sprintf(buf, "Экстра описание: &C");
        for (desc = j->ex_description; desc; desc = desc->next) {
            strcat(buf, " ");
            strcat(buf, desc->keyword);
        }
        strcat(buf, "&n");
        send_to_char(strcat(buf, "\r\n"), ch);
    }


    if (j->materials) {
        send_to_charf(ch, "Материалы:\r\n");
        for (m = j->materials; m; m = m->next) {
            send_to_charf(ch, "Название: &C%s&n Количество: &G%d&n%s\r\n",
                          get_name_pad(get_material_param(m->number_mat)->name, PAD_IMN,
                                       PAD_OBJECT), m->value, (m->main ? " основной" : ""));
        }
    }
    send_to_charf(ch, "Качество: [&G%d&n] Род: [&G%d&n]\r\n", j->obj_flags.quality, GET_OBJ_SEX(j));

    send_to_char("Может быть одет: &C", ch);
    sprintbit(j->obj_flags.wear_flags, wear_bits, buf, ",");
    strcat(buf, "&n\r\n");
    send_to_char(buf, ch);

    send_to_char("Неудобства: &C", ch);
    sprintbits(j->obj_flags.no_flag, no_bits, buf, ",");
    strcat(buf, "&n\r\n");
    send_to_char(buf, ch);

    send_to_char("Запреты: &C", ch);
    sprintbits(j->obj_flags.anti_flag, anti_bits, buf, ",");
    strcat(buf, "&n\r\n");
    send_to_char(buf, ch);
    send_to_char("Устанавливает аффекты: &C", ch);

    //sprintbits(j->affects, affected_bits, buf, ",");
    for (af = j->C_affected; af; af = af->next)
        if (af->bitvector) {
            sprintbit(af->bitvector, affected_bits, buf2, ",");
            sprintf(buf + strlen(buf), " &C%s&n (&G%d&n)", buf2, af->duration);
        }
    strcat(buf, "&n\r\n");
    send_to_char(buf, ch);

    send_to_char("Дополнительные флаги: &C", ch);
    sprintbits(j->obj_flags.extra_flags, extra_bits, buf, ",");
    strcat(buf, "&n\r\n");
    send_to_char(buf, ch);
    sprintf(buf, "Температура: [&G%d/%d&n] Вес: [&G%d&n] Цена: [&G%d&n]\r\n",
            GET_OBJ_TEMP(j), GET_OBJ_TEMP_ADD(j), GET_OBJ_WEIGHT(j), GET_OBJ_COST(j));
    send_to_char(buf, ch);

    send_to_charf(ch, "Таймер: [&G%d/%d&n] Состояние: [&G%d/%d&n] Время создания: &C%s&n\r\n",
                  GET_OBJ_TIMER(j), GET_OBJ_DESTROY(j),
                  GET_OBJ_CUR(j), GET_OBJ_MAX(j), ascii_time(GET_OBJ_TIMELOAD(j)));

    strcpy(buf, "Находится: ");
    if (j->in_room == NOWHERE)
        strcat(buf, "&Cнигде&n");
    else {
        sprintf(buf2, "&G%d&n", GET_ROOM_VNUM(IN_ROOM(j)));
        strcat(buf, buf2);
    }
    /*
     * NOTE: In order to make it this far, we must already be able to see the
     *       character holding the object. Therefore, we do not need CAN_SEE().
     */
    strcat(buf, " В контейнере: &C");
    strcat(buf, j->in_obj ? j->in_obj->short_description : "Нет");
    strcat(buf, "&n");
    strcat(buf, " В инвентаре: &C");
    strcat(buf, j->carried_by ? GET_NAME(j->carried_by) : "Нет");
    strcat(buf, "&n");
    strcat(buf, " Одет: &C");
    strcat(buf, j->worn_by ? GET_NAME(j->worn_by) : "Нет");
    strcat(buf, "&n");
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    sprintf(buf, "Значения 0-3: [&G%d&n] [&G%d&n] [&G%d&n] [&G%d&n]",
            GET_OBJ_VAL(j, 0), GET_OBJ_VAL(j, 1), GET_OBJ_VAL(j, 2), GET_OBJ_VAL(j, 3));
    send_to_char(strcat(buf, "\r\n"), ch);

    struct weapon_damage_data *n;

    if (j->weapon && j->weapon->damages) {
        sprintf(buf, "Усиление &G%d&n, Наносимые повреждения:\r\n", j->powered);
        for (n = j->weapon->damages; n; n = n->next)
            sprintf(buf + strlen(buf),
                    " &C%s&n: минимальное &G%d&n, максимальное &G%d&n, среднее: &G%d&n\r\n",
                    damage_name[n->type_damage], n->min_damage, n->max_damage,
                    (n->min_damage + n->max_damage) / 2);
        send_to_char(buf, ch);
    }

    if (*j->lock_code)
        send_to_charf(ch, "Шифр: [&G%s&n]\r\n", j->lock_code);

    if (j->missile)
        send_to_charf(ch, "Снарядов: &C%s&n &G%d&n из &G%d&n\r\n",
                      get_name_pad((char *) missiles_name[j->obj_flags.value[0]], PAD_IMN,
                                   PAD_OBJECT), j->obj_flags.value[2], j->obj_flags.value[1]);
    /*
     * I deleted the "equipment status" code from here because it seemed
     * more or less useless and just takes up valuable screen space.
     */

    if (j->contains) {
        sprintf(buf, "\r\nСодержит:%s", CCGRN(ch, C_NRM));
        for (found = 0, j2 = j->contains; j2; j2 = j2->next_content) {
            sprintf(buf2, "%s %s", found++ ? "," : "", j2->short_description);
            strcat(buf, buf2);
            if (strlen(buf) >= 62) {
                if (j2->next_content)
                    send_to_char(strcat(buf, ",\r\n"), ch);
                else
                    send_to_char(strcat(buf, "\r\n"), ch);
                *buf = found = 0;
            }
        }

        if (*buf)
            send_to_charf(ch, "%s\r\n", buf);
        send_to_char(CCNRM(ch, C_NRM), ch);
    }
    found = 0;
    send_to_charf(ch, "Всего предметов: %d\r\n", GET_OBJ_COUNTITEMS(j));
    send_to_char("Модификаторы:", ch);
    for (af = j->C_affected; af; af = af->next)
        if (af->location && af->modifier) {
            sprinttype(af->location, apply_types, buf2);
            sprintf(buf, "%s &C%s&n &G%+d&n время &G%d&n |", found++ ? " " : "", buf2, af->modifier,
                    af->duration);
            send_to_char(buf, ch);
        }
    if (!found)
        send_to_char(" &CНет&n", ch);

    send_to_char("\r\n", ch);
    sprintf(buf, "Кол-во в мире: [&G%d(%d)&n] Предел: [&G%d&n]\r\n",
            rnum >= 0 ? obj_index[rnum].number : -1,
            rnum >= 0 ? obj_index[rnum].stored : -1, GET_OBJ_LIMIT(j));
    send_to_char(buf, ch);
    /* check the object for a script */
}


void do_stat_character(struct char_data *ch, struct char_data *k)
{
    int i, i2, found = 0;
    struct obj_data *j;
    struct follow_type *fol;
    struct affected_type *aff;
    char *get_addr_ip(char *arg);
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    //sprinttype(GET_SEX(k), genders, buf);

    sprintf(buf, "%s &C%s&n IDномер: &G[%ld]&n Индефикатор: &G[%ld]&n В комнате &G[%d]&n\r\n",
            (!IS_NPC(k) ? "Персонаж: " : (!IS_MOB(k) ? "NPC: " : "Монстр: ")),
            (IS_NPC(k) ? GET_NAMES(k) : GET_NAME(k)), GET_IDNUM(k), GET_ID(k),
            GET_ROOM_VNUM(IN_ROOM(k)));
    send_to_char(buf, ch);

    if (IS_MOB(k))
        sprintf(buf, "Синонимы: &C%s[%s]&n Вномер: &G[%d]&n Рномер: &G[%d]&n",
                k->player.name, k->player.names, GET_MOB_VNUM(k), GET_MOB_RNUM(k));
    else
        sprintf(buf, "Падежи: &C(%s,%s,%s,%s,%s,%s)&n ",
                GET_PAD(k, 0), GET_PAD(k, 1), GET_PAD(k, 2), GET_PAD(k, 3), GET_PAD(k, 4),
                GET_PAD(k, 5));

    send_to_char(buf, ch);


    if (!IS_NPC(k)) {

        if (!NAME_GOD(k)) {
            sprintf(buf, "Имя не одобрено.\r\n");
            send_to_char(buf, ch);
        } else if (NAME_GOD(k) < 1000) {
            sprintf(buf, "Имя запрещено: &C%s&n\r\n", get_name_by_id(NAME_ID_GOD(k)));
            send_to_char(buf, ch);
        } else {
            sprintf(buf, "Имя одобрено: &C%s&n\r\n", get_name_by_id(NAME_ID_GOD(k)));
            send_to_char(buf, ch);
        }

        if (k->desc) {
            sprintf(buf, "IP: &G%s&n, Местоположение: &C%s&n\r\n", k->desc->host,
                    get_addr_ip(k->desc->host));
            send_to_charf(ch, buf);
        }

        sprintf(buf, "E-mail: &C%s&n "
                "Unique: &G[%d]&n "
                "Регкод: &C%s&n ",
                GET_EMAIL(k), GET_UNIQUE(k), ch->registry_code ? ch->registry_code : "Нет");
        send_to_char(buf, ch);
        sprintf(buf, "Титул: &C%s&n\r\n", (k->player.title ? k->player.title : "Нет"));
        send_to_char(buf, ch);

        if (PLR_FLAGGED(k, PLR_HELLED) && HELL_DURATION(k)) {
            sprintf(buf, "Находиться в темнице: &G%ld&n час.\r\n",
                    (HELL_DURATION(k) - time(NULL)) / 3600);
            send_to_char(buf, ch);
        }
        if (PLR_FLAGGED(k, PLR_NAMED) && NAME_DURATION(k)) {
            sprintf(buf, "Находиться в комнате имени: &G%ld&n час.\r\n",
                    (NAME_DURATION(k) - time(NULL)) / 3600);
            send_to_char(buf, ch);
        }
        if (PLR_FLAGGED(k, PLR_MUTE) && MUTE_DURATION(k)) {
            sprintf(buf, "Будет охриплен: &G%ld&n час.\r\n",
                    (MUTE_DURATION(k) - time(NULL)) / 3600);
            send_to_char(buf, ch);
        }
        if (PLR_FLAGGED(k, PLR_DUMB) && DUMB_DURATION(k)) {
            sprintf(buf, "Будет молчать: &G%ld&n мин.\r\n", (DUMB_DURATION(k) - time(NULL)) / 60);
            send_to_char(buf, ch);
        }
        if (GET_GOD_FLAG(k, GF_GODSLIKE) && GODS_DURATION(k)) {
            sprintf(buf, "Любимчик богов: &G%ld&n час.\r\n",
                    (GODS_DURATION(k) - time(NULL)) / 3600);
            send_to_char(buf, ch);
        }
        if (GET_GOD_FLAG(k, GF_GODSCURSE) && GODS_DURATION(k)) {
            sprintf(buf, "Проклят богами: &G%ld&n час.\r\n",
                    (GODS_DURATION(k) - time(NULL)) / 3600);
            send_to_char(buf, ch);
        }
    } else
        send_to_char("\r\n", ch);

    sprintf(buf, "Дополнительное описание: &C%s&n",
            (k->player.long_descr ? k->player.long_descr : "Нет"));
    strcat(buf, "\r\n");
    send_to_char(buf, ch);

    strcpy(buf, "Профессии:&C");
    for (int icls = 0; icls < NUM_CLASSES; icls++)
        if (k->classes[icls])
            sprintf(buf + strlen(buf), " %s(%d)", class_name[icls], k->classes[icls]);
    strcat(buf, "&n.\r\n");
    send_to_char(buf, ch);
    *buf = '\0';

    strcat(buf, "&n Раса: &C");
    strcat(buf, race_name[(int) GET_RACE(k)][(int) GET_SEX(k)]);
    strcat(buf, "&n ");
    strcat(buf, "&n Глаза: &C");
    strcat(buf, eyes_color[(int) GET_EYES(k)]);
    strcat(buf, "&n ");

    sprintf(buf2, "Уровень: [&G%d&n] Опыт: [&G%ld&n] Наклонности: [&G%d&n]\r\n",
            GET_LEVEL(k), GET_EXP(k), GET_ALIGNMENT(k));
    strcat(buf, buf2);
    send_to_char(buf, ch);

    strcpy(buf, "История получения уровней:\r\n");
    for (i = 1; i < LVL_MAX; i++)
        if (GET_HLEVEL(k, i) != -1)
            sprintf(buf + strlen(buf), "%2d:[&G%2d&n]%s", i, GET_HLEVEL(k, i),
                    i % 10 < 1 ? "\r\n" : " ");
    send_to_char(buf, ch);

    if (!IS_NPC(k)) {
        strcpy(buf1, ascii_time(k->player.time.birth));
        strcpy(buf2, ascii_time(LAST_LOGON(k)));

        int ticks = GET_TICKS(k);
        int tick_day = ticks / 1440;
        int tick_hours = (ticks - (tick_day * 1440)) / 60;

        sprintf(buf,
                "Создан: [&C%s&n] Последний вход: [&C%s&n] Играл: [&G%d&n д. &G%d&n ч.] Возраст: [&G%d&n]\r\n",
                buf1, buf2, tick_day, tick_hours, age_old(k)->year);
        send_to_char(buf, ch);

        sprintf(buf, "Комната постоя: [&G%d&n] Говорил: [&G%d&n,&G%d&n,&G%d&n] ",
                k->player.hometown, GET_TALK(k, 0), GET_TALK(k, 1), GET_TALK(k, 2));
        strcat(buf, "\r\n");
        send_to_char(buf, ch);
    }
    sprintf(buf, "Сила:[&G%d/%d(%d)&n] Интел:[&G%d/%d(%d)&n] Мудр:[&G%d/%d(%d)&n] "
            "Ловк:[&G%d/%d(%d)&n] Тело:[&G%d/%d(%d)&n] Обаян:[&G%d/%d(%d)&n]\r\n"
            "Воплощений:[&G%d&n] Удача:[&G%d(%d)&n] Скорость:[&G%d(%d)&n]\r\n",
            GET_STR_ROLL(k), GET_STR(k), GET_REAL_STR(k),
            GET_INT_ROLL(k), GET_INT(k), GET_REAL_INT(k),
            GET_WIS_ROLL(k), GET_WIS(k), GET_REAL_WIS(k),
            GET_DEX_ROLL(k), GET_DEX(k), GET_REAL_DEX(k),
            GET_CON_ROLL(k), GET_CON(k), GET_REAL_CON(k),
            GET_CHA_ROLL(k), GET_CHA(k), GET_REAL_CHA(k),
            GET_REMORT(k), GET_LCK(k), GET_REAL_LCK(k),
            speedy(k), GET_SPEED_ADD(k));
    send_to_char(buf, ch);

    sprintf(buf,
            "Размер: [&G%d(%d)&n] Вес: [&G%d(%d)&n] Рост: [&G%d(%d)&n] Здоровье: [&G%d(%d)&n]\r\n",
            GET_SIZE(k), GET_REAL_SIZE(k), GET_WEIGHT(k), GET_REAL_WEIGHT(k), GET_HEIGHT(k),
            GET_REAL_HEIGHT(k), GET_HLT(k), GET_REAL_HLT(k));
    send_to_char(buf, ch);
    sprintf(buf, "Жизнь: [&G%d(%d)+%d&n] Бодрость: [&G%d(%d)+%d&n]",
            GET_HIT(k), GET_REAL_MAX_HIT(k), hit_gain(k),
            GET_MOVE(k), GET_REAL_MAX_MOVE(k), move_gain(k));
    send_to_char(buf, ch);

    if (IS_MANA_CASTER(k)) {
        sprintf(buf, " Мана :[&G%d(%d)+%d&n]\r\n", GET_MANA(k), GET_REAL_MAX_MANA(k), mana_gain(k));
    } else {
        sprintf(buf, "\r\n");
    }
    send_to_char(buf, ch);

    /*  if (!IS_NPC(k))
       {
       sprintf(buf,"Очки умений: оруженые: &G%d&n основные: &G%d&n дополнительные: &G%d&n магические: &G%d&n\r\n",
       GET_POINT_SKILL(k,SKILL_TYPEWAR),
       GET_POINT_SKILL(k,SKILL_TYPEBASIC),
       GET_POINT_SKILL(k,SKILL_TYPEADDON),
       GET_POINT_SKILL(k,SKILL_TYPEMAGIC));
       send_to_char(buf,ch);
       }
     */
    sprintf(buf, "Наличные: [&G%d&n] Банк: [&G%ld&n] Всего: [&G%ld&n]\r\n",
            GET_GOLD(k), GET_BANK_GOLD(k), GET_GOLD(k) + GET_BANK_GOLD(k));
    send_to_char(buf, ch);

    sprintf(buf,
            "Защита: [&G%d+%d+%d+%d=%d&n] Броня: [&G%d|%d|%d&n] Атака: [&G%d|%d&n] Вред: [&G%d+%d=%d&n]\r\n",
            GET_AC(k), GET_AC_WEAR(k), GET_AC_ADD(k), GET_AC_RT(k), GET_REAL_AC(k),
            (IS_NPC(k) ? k->npc()->specials.armor[0] : 0) + GET_ARMOUR(k, 0),
            (IS_NPC(k) ? k->npc()->specials.armor[1] : 0) + GET_ARMOUR(k, 1),
            (IS_NPC(k) ? k->npc()->specials.armor[2] : 0) + GET_ARMOUR(k, 2), GET_HR(k),
            GET_REAL_HR(k), GET_DR(k), GET_DR_ADD(k), GET_REAL_DR(k));
    send_to_char(buf, ch);

    sprintf(buf, "СпасБроски 3ed:\r\n"
            " Рефлекс: [&G%2d&n] Выносливость: [&G%2d&n] Воля: [&G%2d&n]\r\n"
            " Хаос   : [&G%2d&n] Порядок     : [&G%2d&n] None: [&G%2d&n]\r\n"
            " Огонь  : [&G%2d&n] Электричест : [&G%2d&n] Кисл: [&G%2d&n]\r\n"
            " Холод  : [&G%2d&n] Позитив     : [&G%2d&n] Негт: [&G%2d&n]\r\n"
            " Отравл : [&G%2d&n]\r\n",
            GET_SAVE3(k, SAV_REFL) + saving_throws_3(k, SAV_REFL),
            GET_SAVE3(k, SAV_FORT) + saving_throws_3(k, SAV_FORT),
            GET_SAVE3(k, SAV_WILL) + saving_throws_3(k, SAV_WILL),
            GET_SAVE3(k, SAV_XAOS) + saving_throws_3(k, SAV_XAOS),
            GET_SAVE3(k, SAV_ORDER) + saving_throws_3(k, SAV_ORDER),
            GET_SAVE3(k, SAV_NONE) + saving_throws_3(k, SAV_NONE),
            GET_SAVE3(k, SAV_FIRE) + saving_throws_3(k, SAV_FIRE),
            GET_SAVE3(k, SAV_ELECTRO) + saving_throws_3(k, SAV_ELECTRO),
            GET_SAVE3(k, SAV_ACID) + saving_throws_3(k, SAV_ACID),
            GET_SAVE3(k, SAV_COLD) + saving_throws_3(k, SAV_COLD),
            GET_SAVE3(k, SAV_POSITIVE) + saving_throws_3(k, SAV_POSITIVE),
            GET_SAVE3(k, SAV_NEGATIVE) + saving_throws_3(k, SAV_NEGATIVE),
            GET_SAVE3(k, SAV_POISON) + saving_throws_3(k, SAV_POISON));

    send_to_char(buf, ch);


    if (!IS_NPC(k)) {
        sprintf(buf, "Коэфф. прокачки умений:\r\n");
        for (int icls = 0; icls < NUM_CLASSES; icls++)
            if (k->classes[icls])
                sprintf(buf + strlen(buf), "%10s:[&G%d&n] ", class_name[icls],
                        calc_need_improove(k, k->classes[icls]));
        sprintf(buf + strlen(buf), "\r\n");
        send_to_char(buf, ch);
    }

    if (IS_MOB(k) && GET_SPEC_TYPE(k)) {
        sprintf(buf, "СпецУдар: &C%s&n ", (GET_SPEC_TYPE(k) == 1 ? "Заклинание" : "Удар"));

        if (GET_SPEC_TYPE(k) == 1)
            sprintf(buf + strlen(buf), "Тип: &C%s&n", spell_name(GET_SPEC_SKILL(k)));
        else
            sprintf(buf + strlen(buf), "Вред: &G%dd%d+%d&n", GET_SPEC_DAMDICE(k),
                    GET_SPEC_DAMSIZE(k), GET_SPEC_DAMROLL(k));

        sprintf(buf + strlen(buf), "\r\n");
        sprintf(buf + strlen(buf), "Сообщение жертве: &C%s&n\r\n", GET_SPEC_TOVICT(k));
        sprintf(buf + strlen(buf), "Сообщение в комнату: &C%s&n\r\n", GET_SPEC_TOROOM(k));
        send_to_char(buf, ch);
    };

    if (IS_NPC(k)) {
        if (k->npc()->specials.ExtraAttack)
            sprintf(buf, "Атака1: &G%dd%d+%d&n Ср: &G%d&n Тип &C%s&n", k->npc()->specials.damnodice,
                    k->npc()->specials.damsizedice, k->npc()->specials.damage,
                    (((k->npc()->specials.damsizedice + 1) * k->npc()->specials.damnodice) / 2) +
                    k->npc()->specials.damage, attack_name[k->npc()->specials.attack_type]);
        if (k->npc()->specials.ExtraAttack2)
            sprintf(buf + strlen(buf), "  Атака2: &G%dd%d+%d&n Ср: &G%d&n Тип &C%s&n\r\n",
                    k->npc()->specials.damnodice2, k->npc()->specials.damsizedice2,
                    k->npc()->specials.damage2,
                    (((k->npc()->specials.damsizedice2 + 1) * k->npc()->specials.damnodice2) / 2) +
                    k->npc()->specials.damage2, attack_name[k->npc()->specials.attack_type2]);
        else
            sprintf(buf + strlen(buf), "\r\n");

        sprintf(buf + strlen(buf), "Внум трупа: &G%d&n\n", k->npc()->specials.vnum_corpse);
        send_to_charf(ch, buf);
    }
    sprinttype(GET_POS(k), position_types, buf2);
    sprintf(buf, "Положение: &C%s&n Противник: &C%s&n", buf2,
            (FIGHTING(k) ? GET_NAME(FIGHTING(k)) : "Нет"));

    sprintf(buf + strlen(buf), " Трусость: [&G%d&n]", GET_WIMP_LEV(k));

    //log ("SC8");

    if (k->desc) {
        sprinttype(STATE(k->desc), connected_types, buf2);
        strcat(buf, " Соединение: &C");
        strcat(buf, buf2);
        strcat(buf, "&n");
        send_to_char(buf, ch);
        sprintf(buf, " Постой: [&G%d&n]\r\n", k->char_specials.timer);
        send_to_char(buf, ch);
    } else {
        send_to_char(buf, ch);
        send_to_char("\r\n", ch);
    }

    if (IS_MOB(k)) {
        sprintf(buf, "Позиция по умолчанию: &C");
        sprinttype((k->npc()->specials.default_pos), position_types, buf2);
        strcat(buf, buf2);
        strcat(buf, "&n ");
        if (k->npc()->specials.move_to != NOWHERE)
            sprintf(buf + strlen(buf), "Следует: %d [%s]", k->npc()->specials.move_to,
                    world[k->npc()->specials.move_to].name);
        send_to_char(buf, ch);
        sprintbits(k->char_specials.saved.act, action_bits, buf2, ",");
        sprintf(buf, "\r\nСвойства: &C%s&n\r\n", buf2);
        send_to_charf(ch, buf);
        sprintbits(k->npc()->specials.npc_flags, function_bits, buf2, ",");
        sprintf(buf, "Дополнительно: &C%s&n\r\n", buf2);
        send_to_charf(ch, buf);
    } else {
        sprintbits(k->char_specials.saved.act, player_bits, buf2, ",");
        sprintf(buf, "PLR: &C%s&n\r\n", buf2);
        send_to_char(buf, ch);
        sprintbits(k->pc()->specials.saved.pref, preference_bits, buf2, ",");
        sprintf(buf, "PRF: &C%s&n\r\n", buf2);
        send_to_char(buf, ch);
    }

    if (IS_MOB(k)) {
        sprintf(buf, " СпецПроц: &C%s&n, Таймер: [&G%d&n] Предел: [&G%d&n]\r\n",
                (mob_index[GET_MOB_RNUM(k)].func
                 ? mob_index[GET_MOB_RNUM(k)].func_name : "(нет)"),
                EXTRACT_TIMER(k), GET_MAX_FACTOR(k));
        send_to_char(buf, ch);
    }
    sprintf(buf, "Предметы: Вес: [&G%d&n] Кол-во: [&G%d&n] ", IS_CARRYING_W(k), IS_CARRYING_N(k));

    for (i = 0, j = k->carrying; j; j = j->next_content, i++);
    sprintf(buf + strlen(buf), "В инвентаре: [&G%d&n] ", i);

    for (i = 0, i2 = 0; i < NUM_WEARS; i++)
        if (GET_EQ(k, i))
            i2++;
    sprintf(buf2, "В экипировке: [&G%d&n] Вес: [&G%d&n]\r\n", i2, IS_WEARING_W(k));
    strcat(buf, buf2);
    send_to_char(buf, ch);

    if (!IS_NPC(k)) {
        sprintf(buf, "Голод: [&G%d&n] Жажда: [&G%d&n] Опьянение: [&G%d&n] Сон: [&G%d&n]\r\n",
                GET_COND(k, FULL), GET_COND(k, THIRST), GET_COND(k, DRUNK), GET_COND(k, SLEEP));
        send_to_char(buf, ch);
    }

    sprintf(buf, "Лидер: &C%s&n Последователи: &C", ((k->master) ? GET_NAME(k->master) : "нет"));

    for (fol = k->followers; fol; fol = fol->next) {
        sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch, 0));
        strcat(buf, buf2);
        if (strlen(buf) >= 60) {
            if (fol->next)
                send_to_char(strcat(buf, ",\r\n"), ch);
            else
                send_to_char(strcat(buf, "\r\n"), ch);
            *buf = found = 0;
        }
    }
    sprintf(buf + strlen(buf), "&n Охраняет: &C%s", GUARDING(k) ? GET_NAME(GUARDING(k)) : "никого");

    if (*buf)
        send_to_char(strcat(buf, "&n\r\n"), ch);

    sprintf(buf, "Лидер: &C%s&n Группа: &C",
            ((k->party_leader) ? GET_NAME(k->party_leader) : "нет"));

    for (fol = k->followers; fol; fol = fol->next) {
        sprintf(buf2, "%s %s", found++ ? "," : "", PERS(fol->follower, ch, 0));
        strcat(buf, buf2);
        if (strlen(buf) >= 60) {
            if (fol->next)
                send_to_char(strcat(buf, ",\r\n"), ch);
            else
                send_to_char(strcat(buf, "\r\n"), ch);
            *buf = found = 0;
        }
    }

    if (*buf)
        send_to_char(strcat(buf, "&n\r\n"), ch);

    log("SC11");
    /* Showing the bitvector */
    sprintbits(k->char_specials.saved.affected_by, affected_bits, buf2, ",");
    sprintf(buf, "Эффекты: %s%s%s\r\n", CCIYEL(ch, C_NRM), buf2, CCNRM(ch, C_NRM));
    send_to_char(buf, ch);

    log("SC12");
    /* Routine to show what spells a char is affected by */
    if (k->affected)
        for (aff = k->affected; aff; aff = aff->next) {
            if (aff)
                log("test1 %d", aff->type);
            log("SC12.1");
            {
                *buf2 = '\0';
                if (aff)
                    log("test2 %d own %s", aff->type, aff->owner ? "yes" : "no");
                sprintf(buf,
                        "Воздействие: (&G%d%s или %dтиков&n) &C%s %-18s&n Владелец: &C%-10s[%ld]%s&n",
                        aff->duration, aff->battleflag ? "раунд" : "мин", (aff->duration + 1) / 60,
                        CCICYN(ch, C_NRM), SPELL_NAME(aff->type),
                        (aff->owner ? get_name_by_id(aff->owner) : "нет"), aff->owner, CCNRM(ch,
                                                                                             C_NRM));
                log("SC12.3");
                if (aff->modifier) {
                    log("SC12.4");
                    sprintf(buf2, "&C%s&n &G%d&n", apply_types[(int) aff->location],
                            //(aff->modifier < 0) ? "-":"+",
                            aff->modifier);
                    strcat(buf, buf2);
                    log("SC12.5");
                }
                /*if (aff->bitvector)
                   {

                   if (*buf2)
                   strcat(buf, "уст ");
                   else
                   strcat(buf, "уст ");
                   sprintbit(aff->bitvector, affected_bits, buf2,",");
                   strcat(buf, buf2);
                   } */
                log("SC12.6");
                send_to_char(strcat(buf, "\r\n"), ch);
                log("SC12.7");
            }
            log("SC12.8");
        }

    log("SC13");
    /* check mobiles for a script */


    if (IS_NPC(k)) {
        pk_list_sprintf(k, buf);
        send_to_char(buf, ch);
    } else {                    /* this is a PC, display their global variables */
        if (!k->saved_vars.empty()) {
            //log("GVARS");
            //log("GVARS %d",k->saved_vars.size());
            send_to_char("Глобальные переменные:\r\n", ch);

            for (vars_storage::iterator v = k->saved_vars.begin(); v != k->saved_vars.end(); v++) {
                sprintf(buf, "%20s: %s [%d]\r\n",
                        v->first.c_str(), v->second.value.c_str(), v->second.time);
            }
            //log("GV: end");
        }
        if (k->Questing.count) {
            send_to_char("Выполнил квесты NN: \r\n", ch);
            *buf = '\0';
            for (i = 0; i < k->Questing.count && strlen(buf) + 80 < MAX_STRING_LENGTH; i++)
                sprintf(buf + strlen(buf), "%-8d", k->Questing.quests[i]);
            strcat(buf, "\r\n");
            send_to_char(buf, ch);
        }
        pk_list_sprintf(k, buf);
        send_to_char(buf, ch);
    }
}


ACMD(do_stat)
{
    struct char_data *victim;
    struct obj_data *object;
    int tmp;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    half_chop(argument, buf1, buf2);

    if (!*buf1) {
        send_to_char("Формат: статистика [команата | предмет | монстр | персонаж ] <имя>\r\n", ch);
        return;
    } else if (is_abbrev(buf1, "комната") || is_abbrev(buf1, "room")) {
        do_stat_room(ch);
    } else if (is_abbrev(buf1, "монстр") || is_abbrev(buf1, "mob")) {
        if (!*buf2)
            send_to_char("Имя монстра?\r\n", ch);
        else {
            if ((victim = get_char_vis(ch, buf2, FIND_CHAR_WORLD)) != NULL)
                do_stat_character(ch, victim);
            else
                send_to_char("Монстр не найден.\r\n", ch);
        }
    } else if (is_abbrev(buf1, "персонаж") || is_abbrev(buf1, "player")) {
        if (!*buf2) {
            send_to_char("Состояние какого персонажа?\r\n", ch);
        } else {
            if ((victim = get_player_vis(ch, buf2, FIND_CHAR_WORLD)) != NULL)
                do_stat_character(ch, victim);
            else
                send_to_char("Этого персонажа сейчас нет в игре.\r\n", ch);
        }
    } else if (is_abbrev(buf1, "файл") || is_abbrev(buf1, "file")) {
        if (!*buf2) {
            send_to_char("Состояние какого игрока(из файла)?\r\n", ch);
        } else {
            victim = new Player();
            if (load_char(buf2, victim) > -1) {
                if (GET_LEVEL(victim) > GET_LEVEL(ch) && !GET_COMMSTATE(ch))
                    send_to_char("Извините, Вам это еще рано.\r\n", ch);
                else
                    do_stat_character(ch, victim);
            } else {
                send_to_char("Такого игрока нет ВООБЩЕ.\r\n", ch);
            }
            delete victim;
        }
    } else if (is_abbrev(buf1, "предмет") || is_abbrev(buf1, "object")) {
        if (!*buf2)
            send_to_char("Состояние какого предмета?\r\n", ch);
        else {
            if ((object = get_obj_vis(ch, buf2)) != NULL)
                do_stat_object(ch, object);
            else
                send_to_char("Нет такого предмета в игре.\r\n", ch);
        }
    } else {
        if ((object = get_object_in_equip_vis(ch, buf1, ch->equipment, &tmp)) != NULL)
            do_stat_object(ch, object);
        else if ((object = get_obj_in_list_vis(ch, buf1, ch->carrying)) != NULL)
            do_stat_object(ch, object);
        else if ((victim = get_char_vis(ch, buf1, FIND_CHAR_ROOM)) != NULL)
            do_stat_character(ch, victim);
        else if ((object = get_obj_in_list_vis(ch, buf1, world[ch->in_room].contents)) != NULL)
            do_stat_object(ch, object);
        else if ((victim = get_char_vis(ch, buf1, FIND_CHAR_WORLD)) != NULL)
            do_stat_character(ch, victim);
        else if ((object = get_obj_vis(ch, buf1)) != NULL)
            do_stat_object(ch, object);
        else
            send_to_char("Ничего похожего с этим именем нет.\r\n", ch);
    }
}


ACMD(do_shutdown)
{
    int times = 0;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    if (subcmd != SCMD_SHUTDOWN) {
        send_to_char("Если Вы хотите SHUT что-то DOWN, так и скажите!\r\n", ch);
        return;
    }
    two_arguments(argument, arg, buf);
    shutdown_time = 0;

    if (*buf && (times = atoi(buf)) > 0) {
        shutdown_time = time(NULL) + times;
    }

    if (!*arg) {
        log("(GC) Shutdown by %s.", GET_NAME(ch));
        if (!times)
            send_to_all("&WСИСТЕМА: ОСТАНОВКА.&n\r\n");
        else {
            sprintf(buf, "&WСИСТЕМА: ОСТАНОВКА через %d %s.&n\r\n", times,
                    desc_count(times, WHAT_SEC));
            send_to_all(buf);
        };
        circle_shutdown = 1;
    } else if (!str_cmp(arg, "reboot")) {
        log("(GC) Reboot by %s.", GET_NAME(ch));
        if (!times)
            send_to_all("&WСИСТЕМА: ПЕРЕЗАГРУЗКА.&n\r\n");
        circle_shutdown = circle_reboot = 1;
    } else if (!str_cmp(arg, "now")) {
        sprintf(buf, "(GC) Shutdown NOW by %s.", GET_NAME(ch));
        log(buf);
        send_to_all("&WСИСТЕМА: ПЕРЕЗАГРУЗКА.. Вернетесь через пару минут.&n\r\n");
        shutdown_time = 0;
        circle_shutdown = 1;
        circle_reboot = 2;
    } else if (!str_cmp(arg, "die")) {
        log("(GC) Shutdown by %s.", GET_NAME(ch));
        send_to_all("&WСИСТЕМА: ОСТАНОВКА.&n\r\n");
        if (!times)
            send_to_all("&WСИСТЕМА: ОСТАНОВКА.&n\r\n");
        else {
            sprintf(buf, "&WСИСТЕМА: ОСТАНОВКА через %d %s.&n\r\n", times,
                    desc_count(times, WHAT_SEC));
            send_to_all(buf);
        };
        MudFile(mud->killScriptFile).touch();
        circle_shutdown = 1;
    } else if (!str_cmp(arg, "pause")) {
        log("(GC) Shutdown by %s.", GET_NAME(ch));
        if (!times)
            send_to_all("&WСИСТЕМА: ОСТАНОВКА.&n\r\n");
        else {
            sprintf(buf, "&WСИСТЕМА: ОСТАНОВКА через %d %s.&n\r\n", times,
                    desc_count(times, WHAT_SEC));
            send_to_all(buf);
        };
        MudFile(mud->pauseFile).touch();
        circle_shutdown = 1;
    } else
        send_to_char("Использование: shutdown [now | reboot | die | pause] [time].\r\n", ch);
}


void stop_snooping(struct char_data *ch)
{
    char buf[MAX_STRING_LENGTH];

    if (!ch->desc->snooping)
        send_to_char("Вы не шпионите.\r\n", ch);
    else {
        send_to_char("Вы прекратили шпионить.\r\n", ch);
        sprintf(buf, "%s прекратил%s шпионить", GET_NAME(ch), GET_CH_SUF_1(ch));
        mudlog(buf, CMP, LVL_IMPL, TRUE);

        del_snooper(ch);
        ch->desc->snooping = NULL;
    }
}


ACMD(do_snoop)
{
    struct char_data *victim, *tch;
    char arg[MAX_STRING_LENGTH];

    if (!ch->desc)
        return;

    one_argument(argument, arg);

    if (!*arg) {
        stop_snooping(ch);
        return;
    }

    if (!(victim = get_player_vis(ch, arg, FALSE))) {
        send_to_char("Нет такого создания в игре.\r\n", ch);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("Личная жизнь мобов Вас не касается! :)\r\n", ch);
        return;
    }

    if (!victim->desc)
        act("Вы не можете $S шпионить - он$G потерял$G связь..\r\n", FALSE, ch, 0, victim, TO_CHAR);
    else if (victim == ch)
        stop_snooping(ch);
    else if (ch->desc->snooping)
        send_to_char("Вы уже шпионите.\r\n", ch);
    else {
        if (victim->desc->original)
            tch = victim->desc->original;
        else
            tch = victim;

        if (GET_LEVEL(tch) >= GET_LEVEL(ch) && !GET_COMMSTATE(ch)) {
            send_to_char("Вы не можете.\r\n", ch);
            return;
        }
        send_to_char(OK, ch);

        if (ch->desc->snooping)
            ch->desc->snooping->snoop_by = NULL;

        ch->desc->snooping = victim->desc;

        add_snooper(ch);
    }
}



ACMD(do_switch)
{
    struct char_data *victim;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (ch->desc->original)
        send_to_char("Вы уже в чьем-то теле.\r\n", ch);
    else if (!*arg)
        send_to_char("Воплотиться в кого?\r\n", ch);
    else if (!(victim = get_char_vis(ch, arg, FIND_CHAR_WORLD)))
        send_to_char("Нет такого создания.\r\n", ch);
    else if (ch == victim)
        send_to_char("Вы и так им являетесь.\r\n", ch);
    else if (victim->desc)
        send_to_char("Это тело уже под контролем.\r\n", ch);
    else if (!IS_IMPL(ch) && !IS_NPC(victim))
        send_to_char("Вы не столь могущественны, чтобы контроолировать тело игрока.\r\n", ch);
    else if (GET_LEVEL(ch) < LVL_GRGOD && ROOM_FLAGGED(IN_ROOM(victim), ROOM_GODROOM))
        send_to_char("Вы не можете находиться в той комнате.\r\n", ch);
    else if (!IS_GRGOD(ch) && ROOM_FLAGGED(IN_ROOM(victim), ROOM_HOUSE))
        send_to_char("Вы не сможете проникнуть на частную территорию.\r\n", ch);
    else {
        send_to_char(OK, ch);

        if (IS_NPC(victim)) {
#if 0
            /* 
             * We cannot just set player-specific flags onto NPC,
             * must find another way to preserve original flags
             */
            if (IS_SET(PRF_FLAGS(ch, PRF_BRIEF), PRF_BRIEF))
                SET_BIT(PRF_FLAGS(victim, PRF_BRIEF), PRF_BRIEF);
            if (IS_SET(PRF_FLAGS(ch, PRF_COMPACT), PRF_COMPACT))
                SET_BIT(PRF_FLAGS(victim, PRF_COMPACT), PRF_COMPACT);
            if (IS_SET(PRF_FLAGS(ch, PRF_NOHOLLER), PRF_NOHOLLER))
                SET_BIT(PRF_FLAGS(victim, PRF_NOHOLLER), PRF_NOHOLLER);
            if (IS_SET(PRF_FLAGS(ch, PRF_NOTELL), PRF_NOTELL))
                SET_BIT(PRF_FLAGS(victim, PRF_NOTELL), PRF_NOTELL);
            if (IS_SET(PRF_FLAGS(ch, PRF_DISPHP), PRF_DISPHP))
                SET_BIT(PRF_FLAGS(victim, PRF_DISPHP), PRF_DISPHP);
            if (IS_SET(PRF_FLAGS(ch, PRF_DISPMANA), PRF_DISPMANA))
                SET_BIT(PRF_FLAGS(victim, PRF_DISPMANA), PRF_DISPMANA);
            if (IS_SET(PRF_FLAGS(ch, PRF_DISPMOVE), PRF_DISPMOVE))
                SET_BIT(PRF_FLAGS(victim, PRF_DISPMOVE), PRF_DISPMOVE);
            if (IS_SET(PRF_FLAGS(ch, PRF_AUTOEXIT), PRF_AUTOEXIT))
                SET_BIT(PRF_FLAGS(victim, PRF_AUTOEXIT), PRF_AUTOEXIT);
            if (IS_SET(PRF_FLAGS(ch, PRF_NOREPEAT), PRF_NOREPEAT))
                SET_BIT(PRF_FLAGS(victim, PRF_NOREPEAT), PRF_NOREPEAT);
            if (IS_SET(PRF_FLAGS(ch, PRF_HOLYLIGHT), PRF_HOLYLIGHT))
                SET_BIT(PRF_FLAGS(victim, PRF_HOLYLIGHT), PRF_HOLYLIGHT);
            if (IS_SET(PRF_FLAGS(ch, PRF_COLOR_1), PRF_COLOR_1))
                SET_BIT(PRF_FLAGS(victim, PRF_COLOR_1), PRF_COLOR_1);
            if (IS_SET(PRF_FLAGS(ch, PRF_COLOR_2), PRF_COLOR_2))
                SET_BIT(PRF_FLAGS(victim, PRF_COLOR_2), PRF_COLOR_2);
            if (IS_SET(PRF_FLAGS(ch, PRF_CURSES), PRF_CURSES))
                SET_BIT(PRF_FLAGS(victim, PRF_CURSES), PRF_CURSES);
            if (IS_SET(PRF_FLAGS(ch, PRF_NOGOSS), PRF_NOGOSS))
                SET_BIT(PRF_FLAGS(victim, PRF_NOGOSS), PRF_NOGOSS);
            if (IS_SET(PRF_FLAGS(ch, PRF_NOGRATZ), PRF_NOGRATZ))
                SET_BIT(PRF_FLAGS(victim, PRF_NOGRATZ), PRF_NOGRATZ);
            if (IS_SET(PRF_FLAGS(ch, PRF_ROOMFLAGS), PRF_ROOMFLAGS))
                SET_BIT(PRF_FLAGS(victim, PRF_ROOMFLAGS), PRF_ROOMFLAGS);
            if (IS_SET(PRF_FLAGS(ch, PRF_DISPEXP), PRF_DISPEXP))
                SET_BIT(PRF_FLAGS(victim, PRF_DISPEXP), PRF_DISPEXP);
            if (IS_SET(PRF_FLAGS(ch, PRF_DISPEXITS), PRF_DISPEXITS))
                SET_BIT(PRF_FLAGS(victim, PRF_DISPEXITS), PRF_DISPEXITS);
            if (IS_SET(PRF_FLAGS(ch, PRF_DISPLEVEL), PRF_DISPLEVEL))
                SET_BIT(PRF_FLAGS(victim, PRF_DISPLEVEL), PRF_DISPLEVEL);
            if (IS_SET(PRF_FLAGS(ch, PRF_DISPGOLD), PRF_DISPGOLD))
                SET_BIT(PRF_FLAGS(victim, PRF_DISPGOLD), PRF_DISPGOLD);
            if (IS_SET(PRF_FLAGS(ch, PRF_NOSHOUT), PRF_NOSHOUT))
                SET_BIT(PRF_FLAGS(victim, PRF_NOSHOUT), PRF_NOSHOUT);
            if (IS_SET(PRF_FLAGS(ch, PRF_AUTOFRM), PRF_AUTOFRM))
                SET_BIT(PRF_FLAGS(victim, PRF_AUTOFRM), PRF_AUTOFRM);
            if (IS_SET(PRF_FLAGS(ch, PRF_MOBILES), PRF_MOBILES))
                SET_BIT(PRF_FLAGS(victim, PRF_MOBILES), PRF_MOBILES);
            if (IS_SET(PRF_FLAGS(ch, PRF_OBJECTS), PRF_OBJECTS))
                SET_BIT(PRF_FLAGS(victim, PRF_OBJECTS), PRF_OBJECTS);
            if (IS_SET(PRF_FLAGS(ch, PRF_DISPBOI), PRF_DISPBOI))
                SET_BIT(PRF_FLAGS(victim, PRF_DISPBOI), PRF_DISPBOI);
            if (IS_SET(PRF_FLAGS(ch, PRF_SHOWKILL), PRF_SHOWKILL))
                SET_BIT(PRF_FLAGS(victim, PRF_SHOWKILL), PRF_SHOWKILL);
            if (IS_SET(PRF_FLAGS(ch, PRF_SHOWMEGA), PRF_SHOWMEGA))
                SET_BIT(PRF_FLAGS(victim, PRF_SHOWMEGA), PRF_SHOWMEGA);
            if (IS_SET(PRF_FLAGS(ch, PRF_THEME), PRF_THEME))
                SET_BIT(PRF_FLAGS(victim, PRF_THEME), PRF_THEME);
#endif
        }


        strcpy(victim->divd, ch->divd);
        strcpy(victim->divr, ch->divr);
        ch->desc->character = victim;
        ch->desc->original = ch;

        victim->desc = ch->desc;
        ch->desc = NULL;

        sprintf(buf, "%s переключил%s в %s", GET_NAME(ch), GET_CH_SUF_2(ch), GET_PAD(victim, 3));
        mudlog(buf, CMP, LVL_IMPL, TRUE);

    }
}


ACMD(do_return)
{
    bool is_extract = FALSE;
    char buf[MAX_STRING_LENGTH];

    ACMD(do_wake);

    if (IS_NPC(ch) && MOB_FLAGGED(ch, MOB_CLONE) && ch->desc && ch->desc->original) {
        if (FIGHTING(ch)) {
            send_to_charf(ch, "Вернуться в свое тело не получится.\r\n");
            return;
        }
        if (!subcmd) {
            send_to_charf(ch, "Ваше сознание вернулось в Ваше тело.\r\n");
            do_wake(ch->desc->original, 0, 0, 0, 1);
        }
        is_extract = TRUE;
        ch->desc->character = ch->desc->original;
        ch->desc->original = NULL;
        ch->desc->character->desc = ch->desc;
        ch->desc = NULL;
    } else if (ch->desc && ch->desc->original) {
        send_to_char("Вы вернулись в свое тело.\r\n", ch);
        sprintf(buf, "%s вернул%s в свое тело", GET_NAME(ch->desc->original),
                GET_CH_SUF_2(ch->desc->original));
        //send_to_charf(ch,buf);
        mudlog(buf, CMP, LVL_IMPL, TRUE);


        /*
         * If someone switched into your original body, disconnect them.
         *   - JE 2/22/95
         *
         * Zmey: here we put someone switched in our body to disconnect state
         * but we must also NULL his pointer to our character, otherwise
         * close_socket() will damage our character's pointer to our descriptor
         * (which is assigned below in this function). 12/17/99
         */
        if (ch->desc->original->desc) {
            ch->desc->original->desc->character = NULL;
            STATE(ch->desc->original->desc) = CON_DISCONNECT;
        }
        ch->desc->character = ch->desc->original;
        ch->desc->original = NULL;

        ch->desc->character->desc = ch->desc;
        ch->desc = NULL;
    }

    if (is_extract)
        extract_char(ch, FALSE);
}



ACMD(do_load)
{
    struct char_data *mob;
    struct obj_data *obj;
    mob_vnum number;
    mob_rnum r_num;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    two_arguments(argument, buf, buf2);

    if (!*buf || !*buf2 || !IS_DIGIT(*buf2)) {
        send_to_char("Формат: загрузить [ предмет | монстра ] <номер>\r\n", ch);
        return;
    }
    if ((number = atoi(buf2)) < 0) {
        send_to_char("Номер отрицательный бывает только у трупов.\r\n", ch);
        return;
    }
    if (is_abbrev(buf, "монстр") || is_abbrev(buf, "mobile")) {
        if ((r_num = real_mobile(number)) < 0) {
            send_to_char("Нет такого моба.\r\n", ch);
            return;
        }
        mob = read_mobile(r_num, REAL);
        char_to_room(mob, ch->in_room);
        if (!GET_COMMSTATE(ch)) {
            act("$n взмахнул$g рукой и неожиданно материализовал$U $N.", TRUE, ch, NULL, mob,
                TO_ROOM);
        }
        sprintf(buf, "%s создал%s монстра '%s' в #%d", GET_NAME(ch), GET_CH_SUF_1(ch),
                GET_NAME(mob), GET_ROOM_VNUM(IN_ROOM(mob)));
        mudlog(buf, CMP, LVL_IMPL, TRUE);
        act("Вы создали $N3.", FALSE, ch, 0, mob, TO_CHAR);
    } else if (is_abbrev(buf, "предмет") || is_abbrev(buf, "object")) {
        if ((r_num = real_object(number)) < 0) {
            send_to_char("Нет такого предмета.\r\n", ch);
            return;
        }
        obj = read_object(r_num, REAL, TRUE);
        if (load_into_inventory)
            obj_to_char(obj, ch);
        else
            obj_to_room(obj, ch->in_room);

        GET_OBJ_ZONE(obj) = world[IN_ROOM(ch)].zone;
        if (!GET_COMMSTATE(ch)) {
            act("$n взмахнул$g рукой и неожиданно материализовал$U $o.", TRUE, ch, obj, NULL,
                TO_ROOM);
        }
        sprintf(buf, "%s создал%s предмет '%s' в #%d", GET_NAME(ch), GET_CH_SUF_1(ch),
                GET_OBJ_PNAME(obj, 0), GET_ROOM_VNUM(IN_ROOM(ch)));
        mudlog(buf, CMP, LVL_IMPL, TRUE);

        act("Вы создали $o3.", FALSE, ch, obj, 0, TO_CHAR);
        obj_decay(obj);
    } else
        send_to_char("Формат: загрузить [ предмет | монстра ] <номер>\r\n", ch);
}



ACMD(do_vstat)
{
    struct char_data *mob;
    struct obj_data *obj;
    mob_vnum number;            /* or obj_vnum ... */
    mob_rnum r_num;             /* or obj_rnum ... */
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    two_arguments(argument, buf, buf2);

    if (!*buf || !*buf2 || !IS_DIGIT(*buf2)) {
        send_to_char("Формат: сведения [ предмет | монстр ] <номер>\r\n", ch);
        return;
    }
    if ((number = atoi(buf2)) < 0) {
        send_to_char("Отрицательный номер?\r\n", ch);
        return;
    }
    if (is_abbrev(buf, "mob") || is_abbrev(buf, "монстр")) {
        if ((r_num = real_mobile(number)) < 0) {
            send_to_char("Монстра с таким номером нет.\r\n", ch);
            return;
        }
        mob = read_mobile(r_num, REAL);
        char_to_room(mob, 1);
        mort_show_char_values(mob, ch, 150);
        extract_char(mob, FALSE);
    } else if (is_abbrev(buf, "obj") || is_abbrev(buf, "предмет")) {
        if ((r_num = real_object(number)) < 0) {
            send_to_char("Предмета с таким номером нет.\r\n", ch);
            return;
        }
        obj = read_object(r_num, REAL, TRUE);
        mort_show_obj_values(obj, ch, 150);
        extract_obj(obj);
    } else
        send_to_char("Формат: сведения [ предмет | монстр ] <номер>\r\n", ch);
}




/* clean a room of all mobiles and objects */
ACMD(do_purge)
{
    struct char_data *vict, *next_v;
    struct obj_data *obj, *next_o;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, buf);

    if (*buf) {                 /* argument supplied. destroy single object
                                 * or char */
        if ((vict = get_char_vis(ch, buf, FIND_CHAR_ROOM)) != NULL) {
            if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && !GET_COMMSTATE(ch)) {
                send_to_char("У Вас не хватит силы.\r\n", ch);
                return;
            }

            if (!GET_COMMSTATE(ch))
                act("$n обратил$g в прах $N3.", FALSE, ch, 0, vict, TO_NOTVICT);
            if (!IS_NPC(vict)) {
                if (vict->desc) {
                    STATE(vict->desc) = CON_CLOSE;
                    vict->desc->character = NULL;
                    vict->desc = NULL;
                }
            }
            sprintf(buf, "%s уничтожил%s %s в #%d", GET_NAME(ch), GET_CH_SUF_1(ch),
                    GET_PAD(vict, 3), GET_ROOM_VNUM(IN_ROOM(vict)));
            mudlog(buf, CMP, LVL_IMPL, TRUE);

            die(vict, ch);
            //extract_char(vict,TRUE);
        } else if ((obj = get_obj_in_list_vis(ch, buf, world[ch->in_room].contents)) != NULL) {
            if (!GET_COMMSTATE(ch))
                act("$n транглитировал$g $o3 в атомы.", TRUE, ch, obj, 0, TO_ROOM);

            sprintf(buf, "%s уничтожил%s предмет '%s' в #%d", GET_NAME(ch), GET_CH_SUF_1(ch),
                    GET_OBJ_PNAME(obj, 0), GET_ROOM_VNUM(IN_ROOM(ch)));
            mudlog(buf, CMP, LVL_IMPL, TRUE);

            extract_obj(obj);
        } else {
            send_to_char("Ничего похожего с таким именем нет.\r\n", ch);
            return;
        }
        send_to_char(OK, ch);
    } else {                    /* no argument. clean out the room */
        if (!GET_COMMSTATE(ch)) {
            act("$n произнес$q: 'Angiest uns Diergos Kherst!!!'", FALSE, ch, 0, 0, TO_ROOM);
            act("Это было последнее, что Вы услышали.", FALSE, ch, 0, 0, TO_ROOM);
            send_to_room("Хорошая работа.\r\n", ch->in_room, FALSE);
        }

        sprintf(buf, "%s уничтожил%s ВСЕ в #%d", GET_NAME(ch), GET_CH_SUF_1(ch),
                GET_ROOM_VNUM(IN_ROOM(ch)));
        mudlog(buf, CMP, LVL_IMPL, TRUE);

        for (vict = world[ch->in_room].people; vict; vict = next_v) {
            next_v = vict->next_in_room;
            if (IS_NPC(vict))
                extract_char(vict, FALSE);
        }

        for (obj = world[ch->in_room].contents; obj; obj = next_o) {
            next_o = obj->next_content;
            extract_obj(obj);
        }
    }
}



const char *logtypes[] = {
    "нет", "краткий", "нормальный", "полный", "\n"
};

ACMD(do_syslog)
{
    int tp;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!*arg) {
        tp = ((PRF_FLAGGED(ch, PRF_LOG1) ? 1 : 0) + (PRF_FLAGGED(ch, PRF_LOG2) ? 2 : 0));
        sprintf(buf, "Тип Вашего системного лога сейчас %s.\r\n", logtypes[tp]);
        send_to_char(buf, ch);
        return;
    }
    if (isname(arg, "разговор")) {
        TOGGLE_BIT(PRF_FLAGS(ch, PRF_SLOG), PRF_SLOG);
        if (IS_SET(PRF_FLAGS(ch, PRF_SLOG), PRF_SLOG))
            send_to_charf(ch, "Включено.\r\n");
        else
            send_to_charf(ch, "Выключено.\r\n");
        return;
    }

    if (((tp = search_block(arg, logtypes, FALSE)) == -1)) {
        send_to_char("Формат: журнал [ нет | краткий | нормальный | полный ]\r\n", ch);
        return;
    }
    REMOVE_BIT(PRF_FLAGS(ch, PRF_LOG2), PRF_LOG2);
    REMOVE_BIT(PRF_FLAGS(ch, PRF_LOG2), PRF_LOG2);
    SET_BIT(PRF_FLAGS(ch, PRF_LOG1), (PRF_LOG1 * (tp & 1)));
    SET_BIT(PRF_FLAGS(ch, PRF_LOG2), (PRF_LOG2 * (tp & 2) >> 1));

    sprintf(buf, "Тип Вашего системного лога сейчас %s.\r\n", logtypes[tp]);
    send_to_char(buf, ch);
}



ACMD(do_advance)
{
    char name[MAX_STRING_LENGTH];
    char level[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct char_data *victim;
    int newlevel, oldlevel;

    two_arguments(argument, name, level);

    if (*name) {
        if (!(victim = get_char_vis(ch, name, FIND_CHAR_WORLD))) {
            send_to_char("Не найду такого игрока.\r\n", ch);
            return;
        }
    } else {
        send_to_char("Повысить кого ?\r\n", ch);
        return;
    }

    if (GET_LEVEL(ch) <= GET_LEVEL(victim) && !GET_COMMSTATE(ch)) {
        send_to_char("Нелогично.\r\n", ch);
        return;
    }
    if (IS_NPC(victim)) {
        send_to_char("Нет! Только не моба!\r\n", ch);
        return;
    }
    if (!*level || (newlevel = atoi(level)) <= 0) {
        send_to_char("Это не похоже на уровень.\r\n", ch);
        return;
    }
    if (newlevel > LVL_IMPL) {
        sprintf(buf, "%d - максимальный возможный уровень.\r\n", LVL_IMPL);
        send_to_char(buf, ch);
        return;
    }

    if (newlevel < LVL_IMMORT) {
        sprintf(buf, "Повышать можно только до бессмертия.\r\n");
        send_to_char(buf, ch);
        return;
    }

    if (newlevel > GET_LEVEL(ch) && !GET_COMMSTATE(ch)) {
        send_to_char("Вы не можете установить уровень выше собственного.\r\n", ch);
        return;
    }
    if (newlevel == GET_LEVEL(victim)) {
        act("$E и так этого уровня.", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }
    oldlevel = GET_LEVEL(victim);
    if (newlevel < GET_LEVEL(victim)) {
        do_start(victim, FALSE);
        GET_LEVEL(victim) = newlevel;
        send_to_char("Вас окутало облако тьмы.\r\n"
                     "Вы почувствовали себя лишенным чего-то.\r\n", victim);
    } else {
        send_to_char("Вам показалось, будто неземное тепло разлилось по каждой клеточке\r\n"
                     "Вашего тела, наполняя его доселе невиданными Вами ощущениями.\r\n", victim);

        GET_LEVEL(victim) = newlevel;

        if (newlevel > LVL_IMMORT) {
            // Инициализируем бога
            GET_HIT(victim) = 500;
            GET_INIT_HIT(victim) = 500;
            GET_STR(victim) = 25;
            GET_CON(victim) = 25;
            GET_DEX(victim) = 25;
            GET_INT(victim) = 25;
            GET_WIS(victim) = 25;
            GET_CHA(victim) = 25;
        }
    }

    send_to_char(OK, ch);
    if (newlevel < oldlevel)
        log("(GC) %s demoted %s from level %d to %d.",
            GET_NAME(ch), GET_NAME(victim), oldlevel, newlevel);
    else
        log("(GC) %s has advanced %s to level %d (from %d)",
            GET_NAME(ch), GET_NAME(victim), newlevel, oldlevel);

    save_char(victim, NOWHERE);
}



ACMD(do_restore)
{
    int i;
    struct char_data *vict;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, buf);
    if (!*buf)
        send_to_char("Кого Вы хотите восстановить ?\r\n", ch);
    else if (!(vict = get_char_vis(ch, buf, FIND_CHAR_WORLD)))
        send_to_char(NOPERSON, ch);
    else {
        GET_HIT(vict) = GET_REAL_MAX_HIT(vict);
        GET_MOVE(vict) = GET_REAL_MAX_MOVE(vict);
        GET_MANA(vict) = GET_REAL_MAX_MANA(vict);

        if (IS_GOD(vict)) {
            vict->real_abils.intel = 15;
            vict->real_abils.wis = 15;
            vict->real_abils.dex = 15;
            vict->real_abils.str = 15;
            vict->real_abils.con = 15;
            vict->real_abils.cha = 15;
            vict->real_abils.lck = 15;

            for (i = 1; i <= MAX_SKILLS; i++) {
                SET_SKILL(ch, i) = MAX(GET_SKILL(ch, i), 100);
                GET_SKILL_LEVEL(ch, i) = 5;
            }

            for (i = 0; i <= MAX_SPELLS; i++)
                GET_SPELL_TYPE(ch, i) = GET_SPELL_TYPE(ch, i) |
                    SPELL_ITEMS |
                    SPELL_KNOW | SPELL_RUNES | SPELL_SCROLL | SPELL_POTION | SPELL_WAND;
        }

        if (!IS_NPC(vict)) {
            GET_COND(vict, FULL) = 48 * SECS_PER_MUD_TICK;
            GET_COND(vict, THIRST) = 48 * SECS_PER_MUD_TICK;
            GET_COND(vict, SLEEP) = 48 * SECS_PER_MUD_TICK;
        }

        update_pos(vict);
        send_to_char(OK, ch);
        sprintf(buf, "%s восстановил %s.", GET_NAME(ch), GET_NAME(vict));
        mudlog(buf, CMP, LVL_HIGOD, TRUE);
        if (!GET_COMMSTATE(ch))
            act("Вы были полностью восстановлены $N4!", FALSE, vict, 0, ch, TO_CHAR);
    }
}


void perform_immort_vis(struct char_data *ch)
{
    if (GET_INVIS_LEV(ch) == 0 &&
        !AFF_FLAGGED(ch, AFF_HIDE) &&
        !AFF_FLAGGED(ch, AFF_INVISIBLE) && !AFF_FLAGGED(ch, AFF_CAMOUFLAGE)) {
        send_to_char("Вы уже сняли невидимость.\r\n", ch);
        return;
    }

    ch->pc()->specials.saved.invis_level = 0;
    appear(ch);
    send_to_char("Вы теперь полностью видны.\r\n", ch);
}


void perform_immort_invis(struct char_data *ch, int level)
{
    struct char_data *tch;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
        if (tch == ch)
            continue;
        if (GET_LEVEL(tch) >= GET_INVIS_LEV(ch) && GET_LEVEL(tch) < level)
            act("Вы вздрогнули, когда $n растворил$u на Ваших глазах.", FALSE, ch, 0, tch, TO_VICT);
        if (GET_LEVEL(tch) < GET_INVIS_LEV(ch) && GET_LEVEL(tch) >= level)
            act("Вы почувствовали что-то странное, когда $n пропал$g.", FALSE, ch, 0, tch, TO_VICT);
    }

    ch->pc()->specials.saved.invis_level = level;
    sprintf(buf, "Ваш уровень невидимости: %d.\r\n", level);
    send_to_char(buf, ch);
}


ACMD(do_invis)
{
    int level;
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch)) {
        send_to_char("Вы не можете сделать этого.\r\n", ch);
        return;
    }

    one_argument(argument, arg);
    if (!*arg) {
        if (GET_INVIS_LEV(ch) > 0)
            perform_immort_vis(ch);
        else
            perform_immort_invis(ch, GET_LEVEL(ch));
    } else {
        level = atoi(arg);
        if (level > GET_LEVEL(ch) && !GET_COMMSTATE(ch))
            send_to_char("Вы не можете достичь невидимости выше Вашего уровня.\r\n", ch);
        else if (level < 1)
            perform_immort_vis(ch);
        else
            perform_immort_invis(ch, level);
    }
}


ACMD(do_gecho)
{
    struct descriptor_data *pt;
    char buf[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    delete_doubledollar(argument);

    if (!*argument)
        send_to_char("Это, пожалуй, ошибка...\r\n", ch);
    else {
        sprintf(buf, "%s\r\n", argument);
        for (pt = descriptor_list; pt; pt = pt->next)
            if (STATE(pt) == CON_PLAYING && pt->character && pt->character != ch)
                send_to_char(buf, pt->character);
        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else
            send_to_char(buf, ch);
    }
}

ACMD(do_gsystem)
{
    struct descriptor_data *pt;
    char buf[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    delete_doubledollar(argument);

    if (!*argument)
        send_to_char("Это, пожалуй, ошибка...\r\n", ch);
    else {
        sprintf(buf, "&WСИСТЕМА: %s&n\r\n", argument);
        for (pt = descriptor_list; pt; pt = pt->next)
            if (STATE(pt) == CON_PLAYING && pt->character && pt->character != ch)
                send_to_char(buf, pt->character);
        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else
            send_to_char(buf, ch);
    }
}


ACMD(do_poofset)
{
    char **msg;

    switch (subcmd) {
        case SCMD_POOFIN:
            msg = &(POOFIN(ch));
            break;
        case SCMD_POOFOUT:
            msg = &(POOFOUT(ch));
            break;
        default:
            return;
    }

    skip_spaces(&argument);

    if (*msg)
        free(*msg);

    if (!*argument)
        *msg = NULL;
    else
        *msg = str_dup(argument);

    send_to_char(OK, ch);
}



ACMD(do_dc)
{
    struct descriptor_data *d;
    int num_to_dc;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, arg);
    if (!(num_to_dc = atoi(arg))) {
        send_to_char("Usage: DC <user number> (type USERS for a list)\r\n", ch);
        return;
    }
    for (d = descriptor_list; d && d->desc_num != num_to_dc; d = d->next);

    if (!d) {
        send_to_char("Нет такого соединения.\r\n", ch);
        return;
    }
    if (d->character && GET_LEVEL(d->character) >= GET_LEVEL(ch) && !GET_COMMSTATE(ch)) {
        if (!CAN_SEE(ch, d->character))
            send_to_char("Нет такого соединения.\r\n", ch);
        else
            send_to_char("Вы уверены.\r\n", ch);
        return;
    }

    /* We used to just close the socket here using close_socket(), but
     * various people pointed out this could cause a crash if you're
     * closing the person below you on the descriptor list.  Just setting
     * to CON_CLOSE leaves things in a massively inconsistent state so I
     * had to add this new flag to the descriptor.
     *
     * It is a much more logical extension for a CON_DISCONNECT to be used
     * for in-game socket closes and CON_CLOSE for out of game closings.
     * This will retain the stability of the close_me hack while being
     * neater in appearance. -gg 12/1/97
     */
    if (STATE(d) == CON_DISCONNECT || STATE(d) == CON_CLOSE)
        send_to_char("Соединение уже разорвано.\r\n", ch);
    else {
        /*
         * Remember that we can disconnect people not in the game and
         * that rather confuses the code when it expected there to be
         * a character context.
         */
        if (STATE(d) == CON_PLAYING)
            STATE(d) = CON_DISCONNECT;
        else
            STATE(d) = CON_CLOSE;

        sprintf(buf, "Соединение #%d закрыто.\r\n", num_to_dc);
        send_to_char(buf, ch);
        log("(GC) Connection closed by %s.", GET_NAME(ch));
    }

    // STATE(d) = CON_DISCONNECT;
    sprintf(buf, "Соединение #%d закрыто.\r\n", num_to_dc);
    send_to_char(buf, ch);
    log("(GC) %s закрыл соединение.", GET_NAME(ch));
}



ACMD(do_wizlock)
{
    int value;
    const char *when;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);
    if (*arg) {
        value = atoi(arg);
        if (value < 0 || (value > GET_LEVEL(ch) && !GET_COMMSTATE(ch))) {
            send_to_char("Неверное значение для блокировки.\r\n", ch);
            return;
        }
        mud->modRestrict.override(value);
        when = "теперь";
    } else {
        when = "в настоящее время";
        value = mud->modRestrict.getValue();
    }

    switch (value) {
        case 0:
            sprintf(buf, "Игра %s полностью открыта.\r\n", when);
            break;
        case 1:
            sprintf(buf, "Игра %s закрыта для новых игроков.\r\n", when);
            break;
        default:
            sprintf(buf, "Только игроки %d %s и выше могут %s войти в игру.\r\n",
                    value, desc_count(value, WHAT_LEVEL), when);
            break;
    }
    send_to_char(buf, ch);
}


ACMD(do_date)
{
    time_t mytime;
    int d, h, m, s;
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "Тип перезагрузки : ");
    switch (mud->getStats()->getRebootLevel()) {
        case 1:
            strcat(buf, "Креш сервера.\r\n");
            break;
        case 2:
            strcat(buf, "Холодная перезагрузка.\r\n");
            break;
        case 3:
            strcat(buf, "Горячая перезагрузка.\r\n");
            break;
        case 4:
            strcat(buf, "Системный сброс.\r\n");
            break;
        default:
            strcat(buf, "Первоначальная загрузка.\r\n");
            break;
    }

    mytime = time(0);

    sprintf(buf + strlen(buf), "Время сервера    : %s\r\n", ascii_time(mytime));

    mytime = boot_time;

    sprintf(buf + strlen(buf), "Игра запущена    : %s\r\n", ascii_time(mytime));

    mytime = time(0) - boot_time;
    d = mytime / 86400;
    h = (mytime / 3600) % 24;
    m = (mytime / 60) % 60;
    s = mytime % 60;

    sprintf(buf + strlen(buf),
            "Время работы     : %d сут%s, %d:%02d.%02d\r\n", d, ((d == 1) ? "ки" : "ок"), h, m, s);
    sprintf(buf + strlen(buf),
            "Пиковая нагрузка      : %d %s.\r\n",
            mud->getStats()->getHigh(), desc_count(mud->getStats()->getHigh(), WHAT_PERSONA));
    sprintf(buf + strlen(buf),
            "Максимальная нагрузка : %d %s.\r\n",
            mud->getStats()->getPeak(), desc_count(mud->getStats()->getPeak(), WHAT_PERSONA));

    if (circle_shutdown) {
        int wait = shutdown_time - time(NULL);

        if (circle_reboot)
            sprintf(buf + strlen(buf), "Перезагрузка системы ");
        else
            sprintf(buf + strlen(buf), "Остановка системы ");
        if (wait < 60)
            sprintf(buf + strlen(buf), "через: %d %s.\r\n", wait, desc_count(wait, WHAT_SEC));
        else
            sprintf(buf + strlen(buf), "менее чем через: %d %s.\r\n", (wait / 60) + 1,
                    desc_count((wait / 60) + 1, WHAT_MINu));
    }
    if (circle_copyover) {
        int wait = copyover_time - time(NULL);

        sprintf(buf + strlen(buf), "Горячая перезагрузка системы ");
        if (wait < 60)
            sprintf(buf + strlen(buf), "через: %d %s.\r\n", wait, desc_count(wait, WHAT_SEC));
        else
            sprintf(buf + strlen(buf), "менее чем через: %d %s.\r\n", (wait / 60) + 1,
                    desc_count((wait / 60) + 1, WHAT_MINu));
    }


    send_to_char(buf, ch);
}



ACMD(do_last)
{
    struct char_data *chdata;
    int pos, last = 0;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);
    if (!*arg) {
        send_to_char("Кого Вы хотите найти ?\r\n", ch);
        return;
    }
    chdata = new Player();
    if (load_char(arg, chdata) < 0) {
        send_to_char("Нет такого игрока.\r\n", ch);
        delete chdata;

        return;
    }

    if ((pos = get_ptable_by_name(GET_NAME(chdata))) != -1)
        last = player_table[pos].last_logon;

    if (GET_LEVEL(chdata) > GET_LEVEL(ch) && !IS_IMPL(ch)) {
        send_to_char("Конфидециальная информация.\r\n", ch);
    } else {
        sprintf(buf, "[%5ld] [%2d] %-12s : %-18s : %-20s\r\n",
                GET_IDNUM(chdata), (int) GET_LEVEL(chdata),
                GET_NAME(chdata),
                GET_LASTIP(chdata)[0] ? GET_LASTIP(chdata) : "Неизвестный",
                ascii_time(LAST_LOGON(chdata)));
        send_to_char(buf, ch);
    }

    delete chdata;
}


ACMD(do_force)
{
    struct descriptor_data *i, *next_desc;
    struct char_data *vict, *next_force;
    char to_force[MAX_INPUT_LENGTH + 2];
    char arg[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    half_chop(argument, arg, to_force);

    sprintf(buf1, "$n принудил$g Вас '%s'.", to_force);

    if (!*arg || !*to_force)
        send_to_char("Кого и что Вы хотите принудить сделать ?\r\n", ch);
    else if (!IS_GRGOD(ch) ||
             (str_cmp("all", arg) && str_cmp("room", arg) && str_cmp("все", arg)
              && str_cmp("здесь", arg))) {
        if (!(vict = find_char(ch, arg)))
            //if (!(vict = get_player_vis(ch, arg, FALSE)))
            send_to_char(NOPERSON, ch);
        else if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && !GET_COMMSTATE(ch))
            send_to_char("Нельзя принудить выполнить команду у персонажа старшего уровня.\r\n", ch);
        else {
            send_to_char(OK, ch);
            if (!GET_COMMSTATE(ch))
                act(buf1, TRUE, ch, NULL, vict, TO_VICT);
            sprintf(buf, "%s принудил%s %s к '%s'", GET_NAME(ch), GET_CH_SUF_1(ch), GET_NAME(vict),
                    to_force);
            mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
            command_interpreter(vict, to_force);
        }
    } else if (!str_cmp("room", arg) || !str_cmp("здесь", arg) || !str_cmp("локация", arg)) {
        send_to_char(OK, ch);
        sprintf(buf, "(GC) %s принудил%s всех в #%d к '%s'",
                GET_NAME(ch), GET_CH_SUF_1(ch), GET_ROOM_VNUM(IN_ROOM(ch)), to_force);
        mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

        for (vict = world[ch->in_room].people; vict; vict = next_force) {
            next_force = vict->next_in_room;
            if (!IS_NPC(vict) && GET_LEVEL(vict) >= GET_LEVEL(ch) && !GET_COMMSTATE(ch))
                continue;
            if (!GET_COMMSTATE(ch))
                act(buf1, TRUE, ch, NULL, vict, TO_VICT);
            command_interpreter(vict, to_force);
        }
    } else {                    /* force all */
        send_to_char(OK, ch);
        sprintf(buf, "(GC) %s forced all to %s", GET_NAME(ch), to_force);
        mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

        for (i = descriptor_list; i; i = next_desc) {
            next_desc = i->next;

            if (STATE(i) != CON_PLAYING || !(vict = i->character) ||
                (!IS_NPC(vict) && GET_LEVEL(vict) >= GET_LEVEL(ch) && !GET_COMMSTATE(ch)))
                continue;
            if (!GET_COMMSTATE(ch))
                act(buf1, TRUE, ch, NULL, vict, TO_VICT);
            command_interpreter(vict, to_force);
        }
    }
}



ACMD(do_wiznet)
{
    struct descriptor_data *d;
    char any = FALSE;
    int level = LVL_GOD;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    delete_doubledollar(argument);

    if (!*argument) {
        send_to_char("Формат: wiznet <text> | #<level> <text> | \r\n "
                     "        wiznet @<level> | wiz @\r\n", ch);
        return;
    }
    switch (*argument) {
        case '#':
            one_argument(argument + 1, buf1);
            if (is_positive_number(buf1)) {
                half_chop(argument + 1, buf1, argument);
                level = MAX(atoi(buf1), LVL_IMMORT);
                if (level > GET_LEVEL(ch) && !GET_COMMSTATE(ch)) {
                    send_to_char("Вы не можете изрекать выше Вашего уровня.\r\n", ch);
                    return;
                }
            }
            break;
        case '@':
            for (d = descriptor_list; d; d = d->next) {
                if (STATE(d) == CON_PLAYING &&
                    IS_IMMORTAL(d->character) &&
                    !PRF_FLAGGED(d->character, PRF_NOWIZ) &&
                    (CAN_SEE(ch, d->character) || IS_IMPL(ch))) {
                    if (!any) {
                        strcpy(buf1, "Боги в игре:\r\n");
                        any = TRUE;
                    }
                    sprintf(buf1 + strlen(buf1), "  %s", GET_NAME(d->character));
                    if (PLR_FLAGGED(d->character, PLR_WRITING))
                        strcat(buf1, " (пишет)\r\n");
                    else if (PLR_FLAGGED(d->character, PLR_SCRIPTING))
                        strcat(buf1, " (пишет сценарий)\r\n");
                    else if (PLR_FLAGGED(d->character, PLR_MAILING))
                        strcat(buf1, " (пишет письмо)\r\n");
                    else
                        strcat(buf1, "\r\n");

                }
            }
            any = FALSE;
            for (d = descriptor_list; d; d = d->next) {
                if (STATE(d) == CON_PLAYING &&
                    IS_IMMORTAL(d->character) &&
                    PRF_FLAGGED(d->character, PRF_NOWIZ) && CAN_SEE(ch, d->character)) {
                    if (!any) {
                        strcat(buf1, "Боги вне игры:\r\n");
                        any = TRUE;
                    }
                    sprintf(buf1 + strlen(buf1), "  %s\r\n", GET_NAME(d->character));
                }
            }
            send_to_char(buf1, ch);
            return;
        case '\\':
            ++argument;
            break;
        default:
            break;
    }
    if (PRF_FLAGGED(ch, PRF_NOWIZ)) {
        send_to_char("Вы вне игры!\r\n", ch);
        return;
    }
    skip_spaces(&argument);

    if (!*argument) {
        send_to_char("Не думаю, что Боги одобрят это.\r\n", ch);
        return;
    }
    if (level > LVL_GOD) {
        sprintf(buf1, "%s: <%d> %s\r\n", GET_NAME(ch), level, argument);
        sprintf(buf2, "Кто-то: <%d> %s\r\n", level, argument);
    } else {
        sprintf(buf1, "%s: %s\r\n", GET_NAME(ch), argument);
        sprintf(buf2, "Кто-то: %s\r\n", argument);
    }

    for (d = descriptor_list; d; d = d->next) {
        if ((STATE(d) == CON_PLAYING) &&
            (GET_LEVEL(d->character) >= level) &&
            (!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
            (!PLR_FLAGGED(d->character, PLR_WRITING)) &&
            (!PLR_FLAGGED(d->character, PLR_MAILING)) &&
            (!PLR_FLAGGED(d->character, PLR_SCRIPTING)) &&
            (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
            send_to_char(CCCYN(d->character, C_NRM), d->character);
            if (CAN_SEE(d->character, ch))
                send_to_char(buf1, d->character);
            else
                send_to_char(buf2, d->character);
            send_to_char(CCNRM(d->character, C_NRM), d->character);
        }
    }

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char(OK, ch);
}



ACMD(do_zreset)
{
    zone_rnum i;
    zone_vnum j;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);
    if (!*arg) {
        send_to_char("Укажите зону.\r\n", ch);
        return;
    }
    if (*arg == '*') {
        for (i = 0; i <= top_of_zone_table; i++)
            new_reset_zone(i);
        send_to_char("Перезагружаю мир.\r\n", ch);
        sprintf(buf, "(GC) %s перегрузил мир.", GET_NAME(ch));
        mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
        return;
    } else if (*arg == '.')
        i = world[ch->in_room].zone;
    else {
        j = atoi(arg);
        for (i = 0; i <= top_of_zone_table; i++)
            if (zone_table[i].number == j)
                break;
    }
    if (i >= 0 && i <= top_of_zone_table) {
        new_reset_zone(i);
        sprintf(buf, "Перегружаю зону %d (#%d): %s.\r\n", i, zone_table[i].number,
                zone_table[i].name);
        send_to_char(buf, ch);
        sprintf(buf, "(GC) %s сбросил %d (%s)", GET_NAME(ch), i, zone_table[i].name);
        mudlog(buf, NRM, MAX(LVL_GRGOD, GET_INVIS_LEV(ch)), TRUE);
    } else
        send_to_char("Нет такой зоны.\r\n", ch);

}


/*
 *  General fn for wizcommands of the sort: cmd <player>
 */

ACMD(do_wizutil)
{
    struct char_data *vict;
    long result;
    int times;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!*arg)
        send_to_char("Для кого ?\r\n", ch);
    else if (!(vict = get_char_vis(ch, arg, FIND_CHAR_WORLD)))
        send_to_char("Нет такого игрока.\r\n", ch);
    else if (IS_NPC(vict))
        send_to_char("Невозможно для моба.\r\n", ch);
    else if (GET_LEVEL(vict) > GET_LEVEL(ch) && !GET_COMMSTATE(ch))
        send_to_char("А он ведь старше Вас....\r\n", ch);
    else {
        switch (subcmd) {
            case SCMD_REROLL:
                send_to_char("Перегенерирую...\r\n", ch);
                roll_stat(vict);
                sprintf(buf, "Новые параметры: Str %d, Con %d, Dex %d, Int %d, Wis %d, Cha %d\r\n",
                        GET_STR(vict), GET_CON(vict), GET_DEX(vict),
                        GET_INT(vict), GET_WIS(vict), GET_CHA(vict));
                send_to_char(buf, ch);
                sprintf(buf, "%s перегенирировал%s %s", GET_NAME(ch), GET_CH_SUF_1(ch),
                        GET_PAD(vict, 3));
                mudlog(buf, CMP, LVL_IMPL, TRUE);

                break;
            case SCMD_NOTITLE:
                result = PLR_TOG_CHK(vict, PLR_NOTITLE);
                sprintf(buf, "(GC) %s установил 'без титула: %s' для %s.", GET_NAME(ch),
                        ONOFF(result), GET_NAME(vict));
                mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
                strcat(buf, "\r\n");
                send_to_char(buf, ch);
                break;
            case SCMD_SQUELCH:
                break;
            case SCMD_MUTE:
                if (sscanf(argument, "%s %d", arg, &times) > 1) {
                    MUTE_DURATION(vict) = (times > 0) ? time(NULL) + times * 60 * 60 : MAX_TIME;
                    SET_BIT(PLR_FLAGS(vict, PLR_MUTE), PLR_MUTE);
                } else if (!PLR_FLAGGED(vict, PLR_MUTE)) {
                    send_to_char("Ваша жертва и так может кричать.\r\n", ch);
                    return;
                } else {
                    MUTE_DURATION(vict) = 0;
                    REMOVE_BIT(PLR_FLAGS(vict, PLR_MUTE), PLR_MUTE);
                }
                result = PLR_FLAGGED(vict, PLR_MUTE);
                if (result)
                    sprintf(buf, "Холодный ветер прошелся по Вашему телу.\r\n" "Вы охрипли.");
                else
                    sprintf(buf, "Ваше горло прошло.");

                act(buf, FALSE, vict, 0, ch, TO_CHAR);
                sprintf(buf, "(GC) GET_NAME(ch) 'простуда %s' для %s на %dч.", GET_NAME(vict),
                        ONOFF(result), times);
                mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
                strcat(buf, "\r\n");
                send_to_char(buf, ch);
                break;
            case SCMD_DUMB:
                if (sscanf(argument, "%s %d", arg, &times) > 1) {
                    DUMB_DURATION(vict) = (times > 0) ? time(NULL) + times * 60 : MAX_TIME;
                    SET_BIT(PLR_FLAGS(vict, PLR_DUMB), PLR_DUMB);
                } else if (!PLR_FLAGGED(vict, PLR_DUMB)) {
                    send_to_char("Ваша жертва и так может говорить.\r\n", ch);
                    return;
                } else {
                    DUMB_DURATION(vict) = 0;
                    REMOVE_BIT(PLR_FLAGS(vict, PLR_DUMB), PLR_DUMB);
                }
                result = PLR_FLAGGED(vict, PLR_DUMB);
                if (result)
                    sprintf(buf, "Ваш рот растворился.");
                else
                    sprintf(buf, "У Вас образовался рот.");

                act(buf, FALSE, vict, 0, ch, TO_CHAR);
                sprintf(buf, "(GC) %s 'молчание %s' для %s на %dм.", GET_NAME(ch), ONOFF(result),
                        GET_NAME(vict), times);
                mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
                strcat(buf, "\r\n");
                send_to_char(buf, ch);
                break;
            case SCMD_FREEZE:
                if (ch == vict) {
                    send_to_char("Это слишком жестоко...\r\n", ch);
                    return;
                }
                if (PLR_FLAGGED(vict, PLR_FROZEN)) {
                    send_to_char("Ваша жертва и так заморожена.\r\n", ch);
                    return;
                }
                if (sscanf(argument, "%s %d", arg, &times) > 0)
                    FREEZE_DURATION(vict) = (times > 0) ? time(NULL) + times * 60 * 60 : MAX_TIME;
                else
                    FREEZE_DURATION(vict) = 0;
                SET_BIT(PLR_FLAGS(vict, PLR_FROZEN), PLR_FROZEN);
                GET_FREEZE_LEV(vict) = GET_LEVEL(ch);
                /*sprintf(buf,"%s$N заморозил$G Вас.%s",
                   CCIRED(vict,C_NRM),CCNRM(vict,C_NRM));
                   act(buf,FALSE,vict,0,ch,TO_CHAR); */
                send_to_char("Адский холод сковал Ваше тело ледяным панцирем.\r\n", vict);
                send_to_char("Заморожен.\r\n", ch);
                act("Ледяной панцирь покрыл тело $n1!", FALSE, vict, 0, 0, TO_ROOM);
                sprintf(buf, "(GC) %s frozen by %s(%dh).", GET_NAME(vict), GET_NAME(ch), times);
                mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
                break;
            case SCMD_HELL:
                if (sscanf(argument, "%s %d", arg, &times) > 1)
                    HELL_DURATION(vict) = (times > 0) ? time(NULL) + times * 60 * 60 : MAX_TIME;
                else
                    HELL_DURATION(vict) = 0;

                if (HELL_DURATION(vict)) {
                    if (PLR_FLAGGED(vict, PLR_HELLED)) {
                        send_to_char("Ваша жертва и так в лабиринтах.\r\n", ch);
                        return;
                    }
                    go_jail(ch, vict, times);
                } else {
                    if (!PLR_FLAGGED(vict, PLR_HELLED)) {
                        send_to_char("Ваша жертва и так на свободе.\r\n", ch);
                        return;
                    }
                    REMOVE_BIT(PLR_FLAGS(vict, PLR_HELLED), PLR_HELLED);
                    send_to_char("Вас выпустили из лабиринтов.\r\n", vict);
                    send_to_char("Выпустили.\r\n", ch);
                    if ((result = GET_LOADROOM(vict)) == NOWHERE)
                        result = calc_loadroom(vict);
                    result = real_room(result);
                    if (result == NOWHERE) {
                        if (GET_LEVEL(vict) >= LVL_IMMORT)
                            result = r_immort_start_room;
                        else
                            result = r_mortal_start_room;
                    }
                    act("$n выпущен$a из лабиринтов!", FALSE, vict, 0, 0, TO_ROOM);
                    char_from_room(vict);
                    char_to_room(vict, result);
                    look_at_room(vict, result);
                    act("$n выпущен$a из лабиринтов!", FALSE, vict, 0, 0, TO_ROOM);
                    sprintf(buf, "(GC) %s выпустил %s из лабиринта.", GET_NAME(vict), GET_NAME(ch));
                    mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
                }
                break;

            case SCMD_NAME:
                if (sscanf(argument, "%s %d", arg, &times) > 1)
                    NAME_DURATION(vict) = (times > 0) ? time(NULL) + times * 60 * 60 : MAX_TIME;
                else
                    NAME_DURATION(vict) = 0;

                if (NAME_DURATION(vict)) {
                    if (PLR_FLAGGED(vict, PLR_NAMED)) {
                        send_to_char("Ваша жертва и так в комнате имени.\r\n", ch);
                        return;
                    }
                    SET_BIT(PLR_FLAGS(vict, PLR_NAMED), PLR_NAMED);
                    send_to_char("Перемещен.\r\n", ch);
                    act("$n водворен$a в КОМНАТУ ИМЕНИ!", FALSE, vict, 0, 0, TO_ROOM);
                    char_from_room(vict);
                    char_to_room(vict, r_named_start_room);
                    sprintf(buf, "%s$N поместил$G Вас в комнату имен.%s",
                            CCIRED(vict, C_NRM), CCNRM(vict, C_NRM));
                    act(buf, FALSE, vict, 0, ch, TO_CHAR);
                    look_at_room(vict, r_named_start_room);
                    act("$n водворен$a в КОМНАТУ ИМЕНИ!", FALSE, vict, 0, 0, TO_ROOM);
                    sprintf(buf, "(GC) %s removed to NAMES ROOM by %s(%dh).", GET_NAME(vict),
                            GET_NAME(ch), times);
                    mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
                } else {
                    if (!PLR_FLAGGED(vict, PLR_NAMED)) {
                        send_to_char("Вашей жертвы там нет.\r\n", ch);
                        return;
                    }
                    REMOVE_BIT(PLR_FLAGS(vict, PLR_NAMED), PLR_NAMED);
                    send_to_char("Вас выпустили из КОМНАТЫ ИМЕНИ.\r\n", vict);
                    send_to_char("Выпустили.\r\n", ch);
                    if ((result = GET_LOADROOM(vict)) == NOWHERE)
                        result = calc_loadroom(vict);
                    result = real_room(result);
                    if (result == NOWHERE) {
                        if (GET_LEVEL(vict) >= LVL_IMMORT)
                            result = r_immort_start_room;
                        else
                            result = r_mortal_start_room;
                    }
                    act("$n выпущен$a из КОМНАТЫ ИМЕНИ!", FALSE, vict, 0, 0, TO_ROOM);
                    char_from_room(vict);
                    char_to_room(vict, result);
                    look_at_room(vict, result);
                    act("$n выпущен$a из КОМНАТЫ ИМЕНИ!", FALSE, vict, 0, 0, TO_ROOM);
                    sprintf(buf, "(GC) %s removed from NAMES ROOM by %s(%ds).", GET_NAME(vict),
                            GET_NAME(ch), times);
                    mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
                }
                break;

            case SCMD_REGISTER:
                if (ch == vict) {
                    send_to_char("Господи, не чуди...\r\n", ch);
                    return;
                }
                if (PLR_FLAGGED(vict, PLR_REGISTERED)) {
                    send_to_char("Респондент уже зарегистрирован.\r\n", ch);
                    return;
                }
                SET_BIT(PLR_FLAGS(vict, PLR_REGISTERED), PLR_REGISTERED);
                send_to_char("Вас зарегистрировали.\r\n", vict);
                send_to_char("Зарегистрирован.\r\n", ch);
                if (IN_ROOM(vict) == r_unreg_start_room) {
                    if ((result = GET_LOADROOM(vict)) == NOWHERE)
                        result = calc_loadroom(vict);
                    result = real_room(result);
                    if (result == NOWHERE) {
                        if (GET_LEVEL(vict) >= LVL_IMMORT)
                            result = r_immort_start_room;
                        else
                            result = r_mortal_start_room;
                    }
                    char_from_room(vict);
                    char_to_room(vict, result);
                    look_at_room(vict, result);
                    act("$n появил$u в центре комнаты, с гордостью показывая всем штампик регистрации!", FALSE, vict, 0, 0, TO_ROOM);
                }
                sprintf(buf, "(GC) %s registered by %s(%ds).", GET_NAME(vict), GET_NAME(ch), times);
                mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
                break;


            case SCMD_THAW:
                if (!PLR_FLAGGED(vict, PLR_FROZEN)) {
                    send_to_char("Ваша цель и так не заморожена.\r\n", ch);
                    return;
                }
                if (GET_FREEZE_LEV(vict) > GET_LEVEL(ch)) {
                    sprintf(buf, "Извините, %s заморожен(а) старшими богами...\r\n",
                            GET_NAME(vict));
                    send_to_char(buf, ch);
                    return;
                }
                sprintf(buf, "(GC) %s растопил %s.", GET_NAME(ch), GET_NAME(vict));
                mudlog(buf, BRF, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
                REMOVE_BIT(PLR_FLAGS(vict, PLR_FROZEN), PLR_FROZEN);
                send_to_char("Ваш ледяной панцирь расстаял.\r\n", vict);
                send_to_char("Разморожен.\r\n", ch);
                act("Ледяной панцрь $n1 расстаял.", FALSE, vict, 0, 0, TO_ROOM);
                break;
            case SCMD_UNAFFECT:
                if (vict->affected) {
                    while (vict->affected)
                        affect_remove(vict, vict->affected);
                    send_to_char("Яркая вспышка осветила Вас!\r\n"
                                 "Вы почувствовали себя немного иначе.\r\n", vict);
                    send_to_char("Все афекты сняты.\r\n", ch);
                } else {
                    send_to_char("Аффектов не было изначально.\r\n", ch);
                    return;
                }
                break;
            default:
                log("SYSERR: Unknown subcmd %d passed to do_wizutil (%s)", subcmd, __FILE__);
                break;
        }
        save_char(vict, NOWHERE);
    }
}


/* single zone printing fn used by "show zone" so it's not repeated in the
   code 3 times ... -je, 4/6/93 */

void print_zone_to_buf(char *bufptr, zone_rnum zone)
{
    sprintf(bufptr, "%s%3d %-30.30s Age: %3d; Reset: %3d (%1d); Top: %5d\r\n",
            bufptr, zone_table[zone].number, zone_table[zone].name,
            zone_table[zone].age, zone_table[zone].lifespan,
            zone_table[zone].reset_mode, zone_table[zone].top);
}


ACMD(do_show_imm)
{
    int i, j, k, l, con;        /* i, j, k to specifics? */
    zone_rnum zrn;
    zone_vnum zvn;
    char self = 0;
    struct char_data *vict;
    struct obj_data *obj;
    struct descriptor_data *d;
    char field[MAX_INPUT_LENGTH], value[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    struct show_struct {
        const char *cmd;
        const char *alias;
        const char level;
    } fields[] = {
        {
        "ничего", "nothing", 0},        /* 0 */
        {
        "зоны", "zones", LVL_GOD},      /* 1 */
        {
        "персонажа", "player", LVL_GOD}, {
        "постой", "rent", LVL_GOD}, {
        "статистику", "stats", LVL_IMMORT}, {
        "ошибки", "errors", LVL_IMPL},  /* 5 */
        {
        "смерть", "death", LVL_GOD}, {
        "эфир", "godrooms", LVL_GOD}, {
        "магазин", "shops", LVL_IMMORT}, {
        "переменные", "vars", LVL_HIGOD}, {
        "шпионаж", "snoop", LVL_GRGOD}, /* 10 */
        {
        "связь", "linkdrop", LVL_GRGOD},        /* 11 */
        {
        "предмет", "object", LVL_GRGOD},        /* 11 */
        {
        "\n", "\n", 0}
    };

    skip_spaces(&argument);

    if (!*argument) {
        strcpy(buf, "Опции для показа:\r\n");
        for (j = 0, i = 1; fields[i].level; i++)
            if (fields[i].level <= GET_LEVEL(ch) || GET_COMMSTATE(ch))
                sprintf(buf + strlen(buf), "%-15s%s", fields[i].cmd, (!(++j % 5) ? "\r\n" : ""));
        strcat(buf, "\r\n");
        send_to_char(buf, ch);
        return;
    }

    strcpy(arg, two_arguments(argument, field, value));

    for (l = 0; *(fields[l].cmd) != '\n'; l++)
        if (!strncmp(field, fields[l].cmd, strlen(field)))
            break;

    if (GET_LEVEL(ch) < fields[l].level && !GET_COMMSTATE(ch)) {
        send_to_char("Вы не столь могущественны, чтобы узнать это.\r\n", ch);
        return;
    }
    if (!strcmp(value, "."))
        self = 1;
    buf[0] = '\0';
    switch (l) {
        case 1:                /* zone */
            /* tightened up by JE 4/6/93 */
            if (self)
                print_zone_to_buf(buf, world[ch->in_room].zone);
            else if (*value && is_positive_number(value)) {
                for (zvn = atoi(value), zrn = 0;
                     zone_table[zrn].number != zvn && zrn <= top_of_zone_table; zrn++);
                if (zrn <= top_of_zone_table)
                    print_zone_to_buf(buf, zrn);
                else {
                    send_to_char("Нет такой зоны.\r\n", ch);
                    return;
                }
            } else
                for (zrn = 0; zrn <= top_of_zone_table; zrn++)
                    print_zone_to_buf(buf, zrn);
            page_string(ch->desc, buf, TRUE);
            break;
        case 2:
            break;
        case 3:
            if (!*value) {
                send_to_char("Уточните имя.\r\n", ch);
                return;
            }
            show_char_rent(ch, value);
            break;
        case 4:
            i = 0;
            j = 0;
            k = 0;
            con = 0;
            for (vict = character_list; vict; vict = vict->next) {
                if (IS_NPC(vict))
                    j++;
                else if (CAN_SEE(ch, vict)) {
                    i++;
                    if (vict->desc)
                        con++;
                }
            }
            for (obj = object_list; obj; obj = obj->next)
                k++;
            strcpy(buf, "Текущее состояние:\r\n");
            sprintf(buf + strlen(buf), "  Игроков в игре - %5d, соединений - %5d\r\n", i, con);
            sprintf(buf + strlen(buf), "  Всего игровых зон - %d\r\n", top_of_zone_table + 1);
            sprintf(buf + strlen(buf), "  Всего зарегистрировано игроков - %5d\r\n",
                    top_of_p_table + 1);
            sprintf(buf + strlen(buf), "  Мобов - %5d,  прообразов мобов - %5d\r\n",
                    j, top_of_mobt + 1);
            sprintf(buf + strlen(buf), "  Предметов - %5d, прообразов предметов - %5d\r\n",
                    k, top_of_objt + 1);
            sprintf(buf + strlen(buf), "  Комнат - %5d\r\n", top_of_world + 1);
            sprintf(buf + strlen(buf), "  Шаблоны доспехов - %5d\r\n", ArmTmp.GetNumberItem());
            sprintf(buf + strlen(buf), "  Шаблоны оружия   - %5d\r\n", WeapTmp.GetNumberItem());
            sprintf(buf + strlen(buf), "  Шаблоны стрел    - %5d\r\n", MissTmp.GetNumberItem());
            sprintf(buf + strlen(buf), "  Действия         - %5d\r\n", Scr.GetNumberItem());
            sprintf(buf + strlen(buf), "  Активных событий - %5d\r\n", event_count());



#ifdef OS_FREEBSD
            sprintf(buf + strlen(buf), "  ОС FreeBSD\r\n");
#endif
            send_to_char(buf, ch);
            break;
        case 5:
            strcpy(buf, "Пустых выходов\r\n" "--------------\r\n");
            for (i = 0, k = 0; i <= top_of_world; i++)
                for (j = 0; j < NUM_OF_DIRS; j++)
                    if (world[i].dir_option[j] && world[i].dir_option[j]->to_room == 0)
                        sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n", ++k, GET_ROOM_VNUM(i),
                                world[i].name);
            page_string(ch->desc, buf, TRUE);
            break;
        case 6:
            strcpy(buf, "Смертельных выходов\r\n" "-------------------\r\n");
            for (i = 0, j = 0; i <= top_of_world; i++)
                if (ROOM_FLAGGED(i, ROOM_DEATH))
                    sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n", ++j,
                            GET_ROOM_VNUM(i), world[i].name);
            if (ROOM_FLAGGED(i, ROOM_FLYDEATH))
                sprintf(buf + strlen(buf), "%2d: [%5d] %s (!полет)\r\n", ++j,
                        GET_ROOM_VNUM(i), world[i].name);

            page_string(ch->desc, buf, TRUE);
            break;
        case 7:
            strcpy(buf, "Комнаты для богов\r\n" "-----------------\r\n");
            for (i = 0, j = 0; i <= top_of_world; i++)
                if (ROOM_FLAGGED(i, ROOM_GODROOM))
                    sprintf(buf + strlen(buf), "%2d: [%5d] %s\r\n",
                            ++j, GET_ROOM_VNUM(i), world[i].name);
            page_string(ch->desc, buf, TRUE);
            break;
        case 8:
            send_to_charf(ch, "Показ магазинов пока отключен.\r\n");
            break;
        case 9:
            send_to_charf(ch, "Глобальные переменные\r\n");
            send_to_charf(ch, "------------------------------------\r\n");
            for (vars_storage::iterator v = global_vars.begin(); v != global_vars.end(); v++) {
                send_to_charf(ch, "%20s %5s %5d\r\n",
                              v->first.c_str(), v->second.value.c_str(), v->second.time);
            }
            break;
        case 10:
            *buf = '\0';
            send_to_char("Система негласного контроля:\r\n", ch);
            send_to_char("----------------------------\r\n", ch);
            for (d = descriptor_list; d; d = d->next) {
                if (d->snooping == NULL || d->character == NULL)
                    continue;
                if (STATE(d) != CON_PLAYING ||
                    (GET_LEVEL(ch) < GET_LEVEL(d->character) && !GET_COMMSTATE(ch)))
                    continue;
                if (!CAN_SEE(ch, d->character) || IN_ROOM(d->character) == NOWHERE)
                    continue;
                sprintf(buf + strlen(buf), "%-10s - подслушивается %s.\r\n",
                        GET_NAME(d->snooping->character), GET_PAD(d->character, 3));
            }
            send_to_char(*buf ? buf : "Никто не подслушивается.\r\n", ch);
            break;              /* snoop */

        case 11:               // show linkdrop
            send_to_char("&CСписок игроков в состоянии 'link drop'&n\r\n", ch);
            sprintf(buf, "&c%-25s%s <%-25s> %s&n\r\n", "Имя", "Комната", "e-mail", "Таймер (тики)");
            send_to_char(buf, ch);
            for (i = 0, vict = character_list; vict; vict = vict->next) {
                if (IS_NPC(vict) || vict->desc != NULL || IN_ROOM(vict) == NOWHERE)
                    continue;
                ++i;
                sprintf(buf, "%-25s[% 5d] <%-25s> %d\r\n", GET_NAME(vict),
                        GET_ROOM_VNUM(IN_ROOM(vict)), GET_EMAIL(vict), vict->char_specials.timer);
                send_to_char(buf, ch);
            }
            sprintf(buf, "Всего - %d\r\n", i);
            send_to_char(buf, ch);
            break;
        case 12:
            if (!*value) {
                send_to_charf(ch, "Укажите виртуальный номер предмета.\r\n");
                return;
            }
            if (atoi(value) < 0) {
                send_to_charf(ch, "Виртуальный номер может быть только положительным числом.\r\n");
                return;
            }
            show_obj_location(ch, atoi(value));
            break;
        default:
            send_to_char("Извините, неверная команда.\r\n", ch);
            break;
    }
}


/***************** The do_set function ***********************************/

#define PC   1
#define NPC  2
#define BOTH 3

#define MISC  0
#define BINARY  1
#define NUMBER  2

#define SET_OR_REMOVE(flagset, flags) { \
        if (on) SET_BIT(flagset, flags); \
        else \
            if (off) REMOVE_BIT(flagset, flags);}

#define RANGE(low, high) (value = MAX((low), MIN((high), (value))))


/* The set options available */
struct set_struct {
    const char *cmd;
    const char level;
    const char pcnpc;
    const char type;
} set_fields[] = {
    {
    "brief", LVL_GOD, PC, BINARY},      /* 0 */
    {
    "invstart", LVL_GOD, PC, BINARY},   /* 1 */
    {
    "титул", LVL_GOD, PC, MISC}, {
    "nosummon", LVL_GRGOD, PC, BINARY}, {
    "жизнь", LVL_IMPL, BOTH, NUMBER}, {
    "мана", LVL_IMPL, BOTH, NUMBER},    /* 5 */
    {
    "бодрость", LVL_IMPL, BOTH, NUMBER}, {
    "текжизнь", LVL_GRGOD, BOTH, NUMBER}, {
    "текмана", LVL_GRGOD, BOTH, NUMBER}, {
    "текбодрость", LVL_GRGOD, BOTH, NUMBER}, {
    "праса_поменять", LVL_IMPL, BOTH, MISC},    /* 10 */
    {
    "сила", LVL_IMPL, BOTH, NUMBER}, {
    "размер", LVL_IMPL, BOTH, NUMBER}, {
    "интелект", LVL_IMPL, BOTH, NUMBER}, {
    "мудрость", LVL_IMPL, BOTH, NUMBER}, {
    "ловкость", LVL_IMPL, BOTH, NUMBER},        /* 15 */
    {
    "телосложение", LVL_IMPL, BOTH, NUMBER}, {
    "обаяние", LVL_IMPL, BOTH, NUMBER}, {
    "защита", LVL_GRGOD, BOTH, NUMBER}, {
    "наличные", LVL_GRGOD, BOTH, NUMBER}, {
    "банк", LVL_IMPL, PC, NUMBER},      /* 20 */
    {
    "опыт", LVL_GRGOD, BOTH, NUMBER}, {
    "hitroll", LVL_IMPL, BOTH, NUMBER}, {
    "damroll", LVL_IMPL, BOTH, NUMBER}, {
    "невидимость", LVL_GOD, PC, NUMBER}, {
    "nohassle", LVL_GRGOD, PC, BINARY}, /* 25 */
    {
    "заморожен", LVL_GOD, PC, BINARY}, {
    "practices", LVL_GRGOD, PC, NUMBER}, {
    "lessons", LVL_GRGOD, PC, NUMBER}, {
    "опьянение", LVL_GOD, PC, MISC}, {
    "голод", LVL_GOD, PC, MISC},        /* 30 */
    {
    "жажда", LVL_GOD, PC, MISC}, {
    "killer", LVL_GOD, PC, BINARY}, {
    "thief", LVL_GOD, PC, BINARY}, {
    "уровень", LVL_IMPL, BOTH, NUMBER}, {
    "room", LVL_IMPL, BOTH, NUMBER},    /* 35 */
    {
    "roomflag", LVL_GRGOD, PC, BINARY}, {
    "siteok", LVL_GRGOD, PC, BINARY}, {
    "deleted", LVL_IMPL, PC, BINARY}, {
    "класс", LVL_IMPL, BOTH, MISC}, {
    "nowizlist", LVL_GOD, PC, BINARY},  /* 40 */
    {
    "quest", LVL_GOD, PC, BINARY}, {
    "loadroom", LVL_GRGOD, PC, MISC}, {
    "color", LVL_GOD, PC, BINARY}, {
    "idnum", LVL_IMPL, PC, NUMBER}, {
    "пароль", LVL_IMPL, PC, MISC},      /* 45 */
    {
    "nodelete", LVL_IMPL, PC, BINARY}, {
    "пол", LVL_GRGOD, BOTH, MISC}, {
    "возраст", LVL_GRGOD, BOTH, NUMBER}, {
    "height", LVL_GRGOD, BOTH, NUMBER}, {
    "weight", LVL_GRGOD, BOTH, NUMBER}, /* 50 */
    {
    "godslike", LVL_IMPL, PC, BINARY}, {
    "godscurse", LVL_IMPL, PC, BINARY}, {
    "olc", LVL_IMPL, PC, NUMBER}, {
    "имя", LVL_GRGOD, PC, MISC}, {
    "trgquest", LVL_IMPL, PC, MISC},    /* 55 */
    {
    "mkill", LVL_IMPL, PC, MISC}, {
    "тестер", LVL_GOD, PC, BINARY}, {
    "слава", LVL_IMPL, PC, MISC}, {
    "remort", LVL_GRGOD, PC, BINARY}, {
    "hell", LVL_GOD, PC, MISC}, /* 60 */
    {
    "email", LVL_GRGOD, PC, MISC}, {
    "раса", LVL_GRGOD, PC, MISC}, {
    "alignment", LVL_GRGOD, PC, NUMBER}, {
    "templ", LVL_GRGOD, PC, NUMBER}, {
    "протокол", LVL_GOD, PC, BINARY}, {
    "мультинг", LVL_GRGOD, PC, BINARY}, {
    "оружейные", LVL_GOD, PC, NUMBER}, {
    "основные", LVL_GOD, PC, NUMBER}, {
    "дополнительные", LVL_GOD, PC, NUMBER}, {
    "магические", LVL_GOD, PC, NUMBER}, {
    "удача", LVL_IMPL, BOTH, NUMBER}, {
    "перевоплощения", LVL_IMPL, BOTH, NUMBER}, {
    "\n", 0, BOTH, MISC}
};

int perform_set(struct char_data *ch, struct char_data *vict, int mode, char *val_arg)
{
    int i, j, c, on = 0, off = 0, value = 0, return_code = 1, ptnum, times, result;
    char npad[NUM_PADS][256];
    char saltpass[MAX_NAME_LENGTH + 10];
    room_rnum rnum;
    room_vnum rvnum;
    char output[MAX_STRING_LENGTH];
    int rod;
    char buf[MAX_STRING_LENGTH];

    /* Check to make sure all the levels are correct */
    if (!IS_IMPL(ch)) {
        if (!IS_NPC(vict) && GET_LEVEL(ch) <= GET_LEVEL(vict) && !GET_COMMSTATE(ch) && vict != ch) {
            send_to_char("Это не так просто, как Вам кажется...\r\n", ch);
            return (0);
        }
    }
    if (GET_LEVEL(ch) < set_fields[mode].level && !GET_COMMSTATE(ch)) {
        send_to_char("Кем Вы себя возомнили ?\r\n", ch);
        return (0);
    }

    /* Make sure the PC/NPC is correct */
    if (IS_NPC(vict) && !(set_fields[mode].pcnpc & NPC)) {
        send_to_char("Эта тварь недостойна такой чести!\r\n", ch);
        return (0);
    } else if (!IS_NPC(vict) && !(set_fields[mode].pcnpc & PC)) {
        act("Вы оскорбляете $S - $E ведь не моб!", FALSE, ch, 0, vict, TO_CHAR);
        return (0);
    }

    /* Find the value of the argument */
    if (set_fields[mode].type == BINARY) {
        if (!strn_cmp(val_arg, "on", 2) ||
            !strn_cmp(val_arg, "yes", 3) || !strn_cmp(val_arg, "вкл", 3))
            on = 1;
        else if (!strn_cmp(val_arg, "off", 3) ||
                 !strn_cmp(val_arg, "no", 2) || !strn_cmp(val_arg, "выкл", 4))
            off = 1;
        if (!(on || off)) {
            send_to_char("Значение может быть 'вкл' или 'выкл'.\r\n", ch);
            return (0);
        }
        sprintf(output, "%s %s для %s.", set_fields[mode].cmd, ONOFF(on), GET_PAD(vict, 1));
    } else if (set_fields[mode].type == NUMBER) {
        value = atoi(val_arg);
        sprintf(output, "У %s %s установлено в %d.", GET_PAD(vict, 1), set_fields[mode].cmd, value);
    } else {
        strcpy(output, "Хорошо.");
    }
    switch (mode) {
        case 0:
            SET_OR_REMOVE(PRF_FLAGS(vict, PRF_BRIEF), PRF_BRIEF);
            break;
        case 1:
            SET_OR_REMOVE(PLR_FLAGS(vict, PLR_INVSTART), PLR_INVSTART);
            break;
        case 2:
            set_title(vict, val_arg);
            sprintf(output, "Титул %s изменен: %s",
                    GET_PAD(vict, 1), GET_TITLE(vict) ? only_title(vict) : "убран");
            break;
        case 3:
            SET_OR_REMOVE(PRF_FLAGS(vict, PRF_SUMMONABLE), PRF_SUMMONABLE);
            sprintf(output, "Возможность призыва %s для %s.\r\n", ONOFF(!on), GET_PAD(vict, 1));
            break;
        case 4:
            vict->points.max_hit = RANGE(1, 5000);
            affect_total(vict);
            break;
        case 5:
            GET_MAX_MANA(vict) = RANGE(0, 5000);
            break;
        case 6:
            vict->points.max_move = RANGE(1, 5000);
            affect_total(vict);
            break;
        case 7:
            vict->points.hit = RANGE(-9, 5000);
            affect_total(vict);
            update_pos(vict);
            break;
        case 8:
            GET_MANA(vict) = RANGE(0, 5000);
            break;
        case 9:
            GET_MOVE(vict) = RANGE(0, 5000);
            break;
        case 10:
            send_to_charf(ch, "Некуй тут ничего делать.\r\n");
            break;
        case 11:
            RANGE(3, 35);
            vict->real_abils.str = value;
            affect_total(vict);
            break;
        case 12:
            vict->real_abils.size = RANGE(1, 100);
            affect_total(vict);
            break;
        case 13:
            RANGE(3, 35);
            vict->real_abils.intel = value;
            affect_total(vict);
            break;
        case 14:
            RANGE(3, 35);
            vict->real_abils.wis = value;
            affect_total(vict);
            break;
        case 15:
            RANGE(3, 35);
            vict->real_abils.dex = value;
            affect_total(vict);
            break;
        case 16:
            RANGE(3, 35);
            vict->real_abils.con = value;
            affect_total(vict);
            break;
        case 17:
            RANGE(3, 35);
            vict->real_abils.cha = value;
            affect_total(vict);
            break;
        case 18:
            vict->real_abils.armor = RANGE(-100, 100);
            affect_total(vict);
            break;
        case 19:
            GET_GOLD(vict) = RANGE(0, 100000000);
            break;
        case 20:
            GET_BANK_GOLD(vict) = RANGE(0, 100000000);
            break;
        case 21:
            vict->points.exp = RANGE(0, 999999999);
            break;
        case 22:
            vict->real_abils.hitroll = RANGE(-20, 20);
            affect_total(vict);
            break;
        case 23:
            vict->real_abils.damroll = RANGE(-20, 20);
            affect_total(vict);
            break;
        case 24:
            if (!IS_IMPL(ch) && ch != vict) {
                send_to_char("Вы не столь Божественны, как Вам кажется!\r\n", ch);
                return (0);
            }
            vict->pc()->specials.saved.invis_level = RANGE(0, GET_LEVEL(vict));
            break;
        case 25:
            if (!IS_IMPL(ch) && ch != vict) {
                send_to_char("Вы не столь Божественны, как Вам кажется!\r\n", ch);
                return (0);
            }
            SET_OR_REMOVE(PRF_FLAGS(vict, PRF_NOHASSLE), PRF_NOHASSLE);
            break;
        case 26:
            if (ch == vict && on) {
                send_to_char("Лучше не стоит - Вас ожидает долгая зима:)\r\n", ch);
                return (0);
            }
            SET_OR_REMOVE(PLR_FLAGS(vict, PLR_FROZEN), PLR_FROZEN);
            if (on) {
                if (sscanf(val_arg, "%d", &i) != 0)
                    FREEZE_DURATION(vict) = (i > 0) ? time(NULL) + i * 60 * 60 : MAX_TIME;
                else
                    FREEZE_DURATION(vict) = 0;
            }
            break;
        case 27:
        case 28:
            return_code = 0;
            break;
        case 29:
        case 30:
        case 31:
            if (!str_cmp(val_arg, "off") || !str_cmp(val_arg, "выкл")) {
                GET_COND(vict, (mode - 29)) = (char) -1;        /* warning: magic number here */
                sprintf(output, "Для %s %s сейчас отключен.", GET_PAD(vict, 1),
                        set_fields[mode].cmd);
            } else if (is_positive_number(val_arg)) {
                value = atoi(val_arg);
                RANGE(0, 24);
                GET_COND(vict, (mode - 29)) = (char) value;     /* and here too */
                sprintf(output, "Для %s %s установлен в %d.", GET_PAD(vict, 1),
                        set_fields[mode].cmd, value);
            } else {
                send_to_char("Должно быть 'off' или значение от 0 до 24.\r\n", ch);
                return (0);
            }
            break;
        case 32:
            //SET_OR_REMOVE(PLR_FLAGS(vict, PLR_KILLER), PLR_KILLER);
            break;
        case 34:
            if (!GET_COMMSTATE(ch) &&
                (value > GET_LEVEL(ch) || value > LVL_IMPL || GET_LEVEL(vict) > GET_LEVEL(ch))) {
                send_to_char("Вы не можете установить уровень игрока выше собственного.\r\n", ch);
                return (0);
            }
            RANGE(0, LVL_IMPL);
            vict->player.level = (byte) value;
            break;
        case 35:
            if ((rnum = real_room(value)) < 0) {
                send_to_char("Поищите другой МУД. В этом МУДе нет такой комнаты.\r\n", ch);
                return (0);
            }
            if (IN_ROOM(vict) != NOWHERE)       /* Another Eric Green special. */
                char_from_room(vict);
            char_to_room(vict, rnum);
            check_horse(vict);
            break;
        case 36:
            SET_OR_REMOVE(PRF_FLAGS(vict, PRF_ROOMFLAGS), PRF_ROOMFLAGS);
            break;
        case 37:
            SET_OR_REMOVE(PLR_FLAGS(vict, PLR_SITEOK), PLR_SITEOK);
            break;
        case 38:
            SET_OR_REMOVE(PLR_FLAGS(vict, PLR_DELETED), PLR_DELETED);
            break;
        case 39:
            if ((i = sscanf(val_arg, "%s %s %s", npad[0], npad[1], npad[2])) != 3) {
                sprintf(buf, "Формат: set жертва mclass id.class level favorite\r\n");
                send_to_char(buf, ch);
                return (0);
            }
            if (atoi(npad[1]) != 0) {
                add_class(vict, atoi(npad[0]), atoi(npad[1]), atoi(npad[2]));
                sprintf(output, "%s класс %s уровень %s любимый %s.", GET_NAME(vict),
                        class_name[(int) atoi(npad[0])], npad[1], npad[2]);
            } else {
                del_class(vict, atoi(npad[0]));
                sprintf(output, "%s убран класс %s", GET_NAME(vict),
                        class_name[(int) atoi(npad[0])]);
            }
            break;
        case 40:
            SET_OR_REMOVE(PLR_FLAGS(vict, PLR_NOWIZLIST), PLR_NOWIZLIST);
            break;
        case 41:
            SET_OR_REMOVE(PRF_FLAGS(vict, PRF_QUEST), PRF_QUEST);
            break;
        case 42:
            if (!str_cmp(val_arg, "off") || !str_cmp(val_arg, "выкл")) {
                REMOVE_BIT(PLR_FLAGS(vict, PLR_LOADROOM), PLR_LOADROOM);
            } else if (is_positive_number(val_arg)) {
                rvnum = atoi(val_arg);
                if (real_room(rvnum) != NOWHERE) {
                    SET_BIT(PLR_FLAGS(vict, PLR_LOADROOM), PLR_LOADROOM);
                    GET_LOADROOM(vict) = rvnum;
                    sprintf(output, "%s будет входить в игру из комнаты #%d.", GET_NAME(vict),
                            GET_LOADROOM(vict));
                } else {
                    send_to_char
                        ("Прежде чем кого-то куда-то поместить, надо это КУДА-ТО создать.\r\n"
                         "Скажите Стрибогу - пусть зоны рисует, а не пьянствует.\r\n", ch);
                    return (0);
                }
            } else {
                send_to_char("Должно быть 'off' или виртуальный номер комнаты.\r\n", ch);
                return (0);
            }
            break;
        case 43:
            SET_OR_REMOVE(PRF_FLAGS(vict, PRF_COLOR_1), PRF_COLOR_1);
            SET_OR_REMOVE(PRF_FLAGS(vict, PRF_COLOR_2), PRF_COLOR_2);
            break;
        case 44:
            if (!IS_IMPL(ch) || !IS_NPC(vict))
                return (0);
            GET_IDNUM(vict) = value;
            break;
        case 45:
            if (!IS_IMPL(ch) && !GET_COMMSTATE(ch) && ch != vict) {
                send_to_char("Давайте не будем экспериментировать.\r\n", ch);
                return (0);
            }
            if (IS_IMPL(vict) && ch != vict && !GET_COMMSTATE(ch)
                ) {
                send_to_char("Вы не можете ЭТО изменить.\r\n", ch);
                return (0);
            }

            sprintf(saltpass, "$1$%s$", GET_NAME(vict));
            strcpy(GET_PASSWD(vict), CRYPT(val_arg, saltpass));
            sprintf(output, "Пароль изменен на '%s'.", val_arg);
            break;
        case 46:
            SET_OR_REMOVE(PLR_FLAGS(vict, PLR_NODELETE), PLR_NODELETE);
            break;
        case 47:
            if ((i = search_block(val_arg, genders, FALSE)) < 0) {
                send_to_char
                    ("Может быть 'мужчина', 'женщина', или 'бесполое'(а вот это я еще не оценил:).\r\n",
                     ch);
                return (0);
            }
            GET_SEX(vict) = i;
            break;
        case 48:               /* set age */
            if (value < 2 || value > 200) {     /* Arbitrary limits. */
                send_to_char("Поддерживаются возрасты от 2 до 200.\r\n", ch);
                return (0);
            }
            /*
             * NOTE: May not display the exact age specified due to the integer
             * division used elsewhere in the code.  Seems to only happen for
             * some values below the starting age (17) anyway. -gg 5/27/98
             */
            vict->player.time.birth = time(0) - ((value - 17) * SECS_PER_MUD_YEAR);
            break;

        case 49:               /* Blame/Thank Rick Glover.:) */
            GET_HEIGHT(vict) = value;
            affect_total(vict);
            break;

        case 50:
            GET_WEIGHT(vict) = value;
            affect_total(vict);
            break;

        case 51:
            if (on) {
                SET_GOD_FLAG(vict, GF_GODSLIKE);
                if (sscanf(val_arg, "%s %d", npad[0], &i) != 0)
                    GODS_DURATION(vict) = (i > 0) ? time(NULL) + i * 60 * 60 : MAX_TIME;
                else
                    GODS_DURATION(vict) = 0;
            } else if (off)
                CLR_GOD_FLAG(vict, GF_GODSLIKE);
            break;
        case 52:
            if (on) {
                SET_GOD_FLAG(vict, GF_GODSCURSE);
                if (sscanf(val_arg, "%s %d", npad[0], &i) != 0)
                    GODS_DURATION(vict) = (i > 0) ? time(NULL) + i * 60 * 60 : MAX_TIME;
                else
                    GODS_DURATION(vict) = 0;
            } else if (off)
                CLR_GOD_FLAG(vict, GF_GODSCURSE);
            break;
        case 53:
            break;
        case 54:
            /* изменение имени!!! */

            if ((i =
                 sscanf(val_arg, "%s %s %s %s %s %s", npad[0], npad[1], npad[2], npad[3], npad[4],
                        npad[5])) != 6) {
                sprintf(buf, "Требуется указать 6 падежей, найдено %d\r\n", i);
                send_to_char(buf, ch);
                return (0);
            }

            if (*npad[0] == '*') {      // Only change pads
                for (i = 1; i < NUM_PADS; i++)
                    if (!_parse_name(npad[i], npad[i])) {
                        if (GET_PAD(vict, i))
                            free(GET_PAD(vict, i));
                        CREATE(GET_PAD(vict, i), char, strlen(npad[i]) + 1);

                        strcpy(GET_PAD(vict, i), npad[i]);
                    }
                sprintf(buf, "Произведена замена падежей.\r\n");
                send_to_char(buf, ch);
            } else {
                if (_parse_name(npad[0], npad[0]) ||
                    strlen(npad[0]) < MIN_NAME_LENGTH ||
                    strlen(npad[0]) > MAX_NAME_LENGTH ||
                    !Valid_Name(npad[0]) || reserved_word(npad[0]) || fill_word(npad[0])
                    ) {
                    send_to_char("Некорректное имя.\r\n", ch);
                    return (0);
                }
                /*
                   if ((ptnum = cmp_ptable_by_name(npad[0],MIN_NAME_LENGTH)) >= 0 &&
                   player_table[ptnum].unique != GET_UNIQUE(vict)
                   )
                   {send_to_char("Первые 4 символа этого имени совпадают еще у одного персонажа.\r\n"
                   "Для исключения различного рода недоразумений имя отклонено.\r\n", ch);
                   return (0);
                   }
                 */
                ptnum = get_ptable_by_name(GET_NAME(vict));
                if (ptnum < 0)
                    return (0);

                for (i = 0; i < NUM_PADS; i++)
                    if (!_parse_name(npad[i], npad[i])) {
                        if (GET_PAD(vict, i))
                            free(GET_PAD(vict, i));
                        CREATE(GET_PAD(vict, i), char, strlen(npad[i]) + 1);

                        strcpy(GET_PAD(vict, i), npad[i]);
                    }
                if (GET_NAME(vict))
                    free(GET_NAME(vict));
                CREATE(GET_NAME(vict), char, strlen(npad[0]) + 1);

                strcpy(GET_NAME(vict), npad[0]);

                free(player_table[ptnum].name);
                CREATE(player_table[ptnum].name, char, strlen(npad[0]) + 1);

                for (i = 0, player_table[ptnum].name[i] = '\0'; npad[0][i]; i++)
                    player_table[ptnum].name[i] = LOWER(npad[0][i]);
                return_code = 2;
                SET_BIT(PLR_FLAGS(vict, PLR_CRASH), PLR_CRASH);
            }
            break;

        case 55:

            if (sscanf(val_arg, "%d %s", &ptnum, npad[0]) != 2) {
                send_to_char("Формат: set <имя> trgquest <quest_num> <on|off>\r\n", ch);
                return (0);
            }
            if (!str_cmp(npad[0], "off") || !str_cmp(npad[0], "выкл")) {
                for (i = j = 0; j < vict->Questing.count; i++, j++) {
                    if (vict->Questing.quests[i] == ptnum)
                        j++;
                    vict->Questing.quests[i] = vict->Questing.quests[j];
                }
                if (j > i)
                    vict->Questing.count--;
                else {
                    act("$N не выполнял$G этого квеста.", FALSE, ch, 0, vict, TO_CHAR);
                    return (0);
                }
            } else if (!str_cmp(npad[0], "on") || !str_cmp(npad[0], "вкл")) {
                set_quested(vict, ptnum);
            } else {
                send_to_char("Требуется on или off.\r\n", ch);
                return (0);
            }
            break;

        case 56:

            if (sscanf(val_arg, "%d %s", &ptnum, npad[0]) != 2) {
                send_to_char("Формат: set <имя> mkill <mob_vnum> <off|num>\r\n", ch);
                return (0);
            }
            if (!str_cmp(npad[0], "off") || !str_cmp(npad[0], "выкл")) {
                if (!clear_kill_vnum(vict, ptnum)) {
                    act("$N не убивал$G ни одного этого моба.", FALSE, ch, 0, vict, TO_CHAR);
                    return (0);
                }
            } else if ((j = atoi(npad[0])) > 0) {
                if ((c = get_kill_vnum(vict, ptnum)) != j)
                    inc_kill_vnum(vict, ptnum, j - c);
                else {
                    act("$N убил$G именно столько этих мобов.", FALSE, ch, 0, vict, TO_CHAR);
                    return (0);
                }
            } else {
                send_to_char("Требуется off или значение больше 0.\r\n", ch);
                return (0);
            }
            break;

        case 57:
            SET_OR_REMOVE(PRF_FLAGS(vict, PRF_CODERINFO), PRF_CODERINFO);
            break;
        case 58:
            skip_spaces(&val_arg);
            if (!val_arg || !*val_arg || ((j = atoi(val_arg)) == 0 && str_cmp("zerro", val_arg))) {
                sprintf(output, "%s заработал%s %d %s славы.", GET_NAME(vict),
                        GET_CH_SUF_1(vict), GET_GLORY(vict), desc_count(GET_GLORY(vict),
                                                                        WHAT_POINT));
                return_code = 0;
            } else {
                if (*val_arg == '-' || *val_arg == '+')
                    GET_GLORY(vict) = MAX(0, GET_GLORY(vict) + j);
                else
                    GET_GLORY(vict) = j;
                sprintf(output, "Количество славы, которое заработал%s %s установлено в %d %s.",
                        GET_CH_SUF_1(vict), GET_NAME(vict), GET_GLORY(vict),
                        desc_count(GET_GLORY(vict), WHAT_POINT));
            }
            break;
        case 59:
            if (!GET_COMMSTATE(ch)) {
                send_to_char("Coder only!\r\n", ch);
                return (0);
            }
            if (on) {
                SET_GOD_FLAG(vict, GF_REMORT);
                sprintf(output, "%s получил%s право на перевоплощение.",
                        GET_NAME(vict), GET_CH_SUF_1(vict));
            } else if (off) {
                CLR_GOD_FLAG(vict, GF_REMORT);
                sprintf(output, "%s утратил%s право на перевоплощение.",
                        GET_NAME(vict), GET_CH_SUF_1(vict));
            }
            break;

        case 60:
            if (sscanf(val_arg, "%d", &times))
                HELL_DURATION(vict) = (times > 0) ? time(NULL) + times * 60 * 60 : MAX_TIME;
            else
                HELL_DURATION(vict) = 0;

            if (HELL_DURATION(vict)) {
                SET_BIT(PLR_FLAGS(vict, PLR_HELLED), PLR_HELLED);
                if (IN_ROOM(vict) != NOWHERE) {
                    sprintf(buf, "%s$N поместил$G Вас в темницу.%s",
                            CCIRED(vict, C_NRM), CCNRM(vict, C_NRM));
                    act(buf, FALSE, vict, 0, ch, TO_CHAR);
                    send_to_char("Посадили.\r\n", ch);
                    act("$n водворен$a в темницу!", FALSE, vict, 0, 0, TO_ROOM);
                    char_from_room(vict);
                    char_to_room(vict, r_helled_start_room);
                    look_at_room(vict, r_helled_start_room);
                    act("$n водворен$a в темницу!", FALSE, vict, 0, 0, TO_ROOM);
                }
                sprintf(output, "(GC) %s removed to hell by %s(%dh).", GET_NAME(vict), GET_NAME(ch),
                        times);
            } else {
                REMOVE_BIT(PLR_FLAGS(vict, PLR_HELLED), PLR_HELLED);
                if (IN_ROOM(vict) != NOWHERE) {
                    send_to_char("Вас выпустили из темницы.\r\n", vict);
                    send_to_char("Отпустили.\r\n", ch);
                    if ((result = GET_LOADROOM(vict)) == NOWHERE)
                        result = calc_loadroom(vict);
                    result = real_room(result);
                    if (result == NOWHERE) {
                        if (GET_LEVEL(vict) >= LVL_IMMORT)
                            result = r_immort_start_room;
                        else
                            result = r_mortal_start_room;
                    }
                    act("$n выпущен$a из темницы!", FALSE, vict, 0, 0, TO_ROOM);
                    char_from_room(vict);
                    char_to_room(vict, result);
                    look_at_room(vict, result);
                    act("$n выпущен$a из темницы!", FALSE, vict, 0, 0, TO_ROOM);
                }
                sprintf(output, "(GC) %s removed from hell by %s.", GET_NAME(vict), GET_NAME(ch));
            }
            break;
        case 61:
            if (*val_arg) {
                PlayerMail email(val_arg);

                if (!email.goodsize()) {
                    send_to_char("Слишком длинный адрес электронной почты.\r\n", ch);
                    return 0;
                }
                if (!email.valid()) {
                    send_to_char("Некорректный адрес электронной почты.\r\n", ch);
                    return 0;
                }

                email.assign(vict);

            } else {
                send_to_char("Пустой адрес электронной почты.\r\n", ch);
                return 0;
            }
            break;
        case 62:
            /* Выставляется род для РС */
            if (is_positive_number(val_arg)) {
                rod = parse_race(*val_arg);
                if (rod == RACE_UNDEFINED) {
                    send_to_char("Неизвесная расса!\r\n", ch);
                    send_to_char(race_menu, ch);
                    return (0);
                } else {
                    GET_RACE(vict) = rod;
                }
            } else {
                bool fnd = FALSE;

                for (int i = 0; i < NUM_RACES; i++)
                    if (!strcmp(race_name[i][SEX_MALE], val_arg) && !fnd) {
                        GET_RACE(vict) = i;
                        fnd = TRUE;
                    }
                if (!fnd) {
                    send_to_char("Неизвесная расса!\r\n", ch);
                    return (0);
                }
            }
            sprintf(output, "установил%s расу %s в '%s'", GET_CH_SUF_1(ch), GET_PAD(vict, 1),
                    race_name[(int) GET_RACE(vict)][SEX_MALE]);
            affect_total(vict);
            break;
        case 63:
            GET_ALIGNMENT(vict) = RANGE(-1000, 1000);
            break;
        case 64:
            GET_GODS(vict) = RANGE(0, 16);
            send_to_charf(ch, "%s установли религию %s\r\n", GET_NAME(vict),
                          gods_name[(int) GET_GODS(vict)]);
            break;
        case 65:
            if (on) {
                SET_BIT(PRF_FLAGS(vict, PRF_LOGGING), PRF_LOGGING);
                sprintf(output, "Протокол работы для %s включен.", GET_NAME(vict));
            } else if (off) {
                REMOVE_BIT(PRF_FLAGS(vict, PRF_LOGGING), PRF_LOGGING);
                sprintf(output, "Протокол работы для %s выключен.", GET_NAME(vict));
            }
            break;
        case 66:
            if (on) {
                SET_BIT(PLR_FLAGS(vict, PLR_MULTI), PLR_MULTI);
                sprintf(output, "Отколенение мультинга для %s включено.", GET_NAME(vict));
            } else if (off) {
                REMOVE_BIT(PLR_FLAGS(vict, PLR_MULTI), PLR_MULTI);
                sprintf(output, "Отколенение мультинга для %s выключено.", GET_NAME(vict));
            }
            break;
        case 67:
            //GET_POINT_SKILL(vict,SKILL_TYPEWAR) = RANGE(0,1000);
            break;
        case 68:
            //GET_POINT_SKILL(vict,SKILL_TYPEBASIC) = RANGE(0,1000);
            break;
        case 69:
            //GET_POINT_SKILL(vict,SKILL_TYPEADDON) = RANGE(0,1000);
            break;
        case 70:
            //GET_POINT_SKILL(vict,SKILL_TYPEMAGIC) = RANGE(0,1000);
            break;
        case 71:
            RANGE(0, 25);
            vict->real_abils.lck = value;
            affect_total(vict);
            break;
        case 72:
            RANGE(0, 20);
            GET_REMORT(vict) = value;
            affect_total(vict);
            break;

        default:
            send_to_char("Не могу установить это!\r\n", ch);
            return (0);
    }

    send_to_charf(ch, "Ок\r\n");
//  send_to_charf(ch,"%s\r\n",CAP(output));

    sprintf(buf, "%s: %s", GET_NAME(ch), output);
    mudlog(buf, CMP, LVL_IMPL, TRUE);

    return (return_code);
}



ACMD(do_set)
{
    struct char_data *vict = NULL, *cbuf = NULL;
    char field[MAX_INPUT_LENGTH],
        name[MAX_INPUT_LENGTH], val_arg[MAX_INPUT_LENGTH], OName[MAX_INPUT_LENGTH];
    int mode, len, player_i = 0, retval;
    char is_file = 0, is_player = 0;
    char buf[MAX_STRING_LENGTH];

    half_chop(argument, name, buf);

    if (!strcmp(name, "файл")) {
        is_file = 1;
        half_chop(buf, name, buf);
    } else if (!str_cmp(name, "игрок")) {
        is_player = 1;
        half_chop(buf, name, buf);
    } else if (!str_cmp(name, "монстр")) {
        half_chop(buf, name, buf);
    } else
        is_player = 1;

    half_chop(buf, field, buf);
    strcpy(val_arg, buf);

    if (!*name || !*field) {
        send_to_char("Формат: установить [монстр|игрок|файл] <жертва> <поле> <значение>\r\n", ch);
        return;
    }

    /* find the target */
    if (!is_file) {
        if (is_player) {
            if (!(vict = get_player_vis(ch, name, FIND_CHAR_WORLD | FIND_CHAR_DISCONNECTED))) {
                send_to_char("Нет такого игрока.\r\n", ch);
                return;
            }
            if (GET_LEVEL(ch) < GET_LEVEL(vict) && !GET_COMMSTATE(ch)) {
                send_to_char("Вы не можете сделать этого.\r\n", ch);
                return;
            }
        } else {                /* is_mob */
            if (!(vict = get_char_vis(ch, name, FIND_CHAR_WORLD))
                || !IS_NPC(vict)) {
                send_to_char("Нет такой твари Божьей.\r\n", ch);
                return;
            }
        }
    } else if (is_file) {       /* try to load the player off disk */
        cbuf = new Player();
        if ((player_i = load_char(name, cbuf)) > -1) {
            if (GET_LEVEL(cbuf) >= GET_LEVEL(ch) && !GET_COMMSTATE(ch)) {
                delete cbuf;

                send_to_char("Вы не можете сделать этого.\r\n", ch);
                return;
            }
            vict = cbuf;
        } else {
            delete cbuf;

            send_to_char("Нет такого игрока.\r\n", ch);
            return;
        }
    }

    /* find the command in the list */
    len = strlen(field);
    for (mode = 0; *(set_fields[mode].cmd) != '\n'; mode++)
        if (!strncmp(field, set_fields[mode].cmd, len))
            break;

    /* perform the set */
    strcpy(OName, GET_NAME(vict));
    retval = perform_set(ch, vict, mode, val_arg);

    /* save the character if a change was made */
    if (retval && !IS_NPC(vict)) {
        if (retval == 2) {
            rename_char(vict, OName);
        } else {
            if (!is_file && !IS_NPC(vict)) {
                save_char(vict, NOWHERE);
            }
            if (is_file) {
                save_char(vict, GET_LOADROOM(vict));
                send_to_char("Файл сохранен.\r\n", ch);
            }
        }
    }

    /* free the memory if we allocated it earlier */
    if (is_file)
        delete cbuf;
}

ACMD(do_liblist)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int first, last, nr, found = 0;

    two_arguments(argument, buf, buf2);

    if (!*buf || !*buf2) {
        switch (subcmd) {
            case SCMD_RLIST:
                send_to_char("Использование: ксписок <начальный номер> <конечный номер>\r\n", ch);
                break;
            case SCMD_OLIST:
                send_to_char("Использование: осписок <начальный номер> <конечный номер>\r\n", ch);
                break;
            case SCMD_MLIST:
                send_to_char("Использование: мсписок <начальный номер> <конечный номер>\r\n", ch);
                break;
            case SCMD_ZLIST:
                send_to_char("Использование: зсписок <начальный номер> <конечный номер>\r\n", ch);
                break;
            default:
                sprintf(buf, "SYSERR:: invalid SCMD passed to ACMDdo_build_list!");
                mudlog(buf, BRF, LVL_GOD, TRUE);
                break;
        }
        return;
    }

    first = atoi(buf);
    last = atoi(buf2);

    if ((first < 0) || (first > 999999) || (last < 0) || (last > 999999)) {
        send_to_char("Значения должны быть между 0 и 999999.\n\r", ch);
        return;
    }

    if (first >= last) {
        send_to_char("Второе значение должно быть больше первого.\n\r", ch);
        return;
    }

    if (first + 200 < last) {
        send_to_char("Максимальный показываемый промежуток - 200.\n\r", ch);
        return;
    }


    switch (subcmd) {
        case SCMD_RLIST:
            sprintf(buf, "Список комнат от Vnum %d до %d\r\n", first, last);
            for (nr = 0; nr <= top_of_world; nr++) {
                if ((world[nr].number <= last) && world[nr].number >= first) {
                    sprintf(buf, "%s%5d. [%5d] (%3d) %s\r\n", buf, ++found,
                            world[nr].number, world[nr].zone, world[nr].name);
                }
            }
            break;
        case SCMD_OLIST:
            sprintf(buf, "Список объектов Vnum %d до %d\r\n", first, last);
            for (nr = 0; nr <= top_of_objt; nr++) {
                if (obj_index[nr].vnum >= first && obj_index[nr].vnum <= last) {
                    sprintf(buf, "%s%5d. [%5d] %s\r\n", buf, ++found,
                            obj_index[nr].vnum, obj_proto[nr].short_description);
                }
            }
            break;
        case SCMD_MLIST:
            sprintf(buf, "Список мобов от %d до %d\r\n", first, last);
            for (nr = 0; nr <= top_of_mobt; nr++) {
                if (mob_index[nr].vnum >= first && mob_index[nr].vnum <= last) {
                    sprintf(buf, "%s%5d. [%5d] %s\r\n", buf, ++found,
                            mob_index[nr].vnum, mob_proto[nr].player.short_descr);
                }
            }
            break;
        case SCMD_ZLIST:
            sprintf(buf, "Список зон от %d до %d\r\n", first, last);
            for (nr = 0; nr <= top_of_zone_table && (zone_table[nr].number <= last); nr++) {
                if (zone_table[nr].number >= first) {
                    sprintf(buf, "%s%5d. [%5d] (%3d) %s\r\n", buf, ++found,
                            zone_table[nr].number, zone_table[nr].lifespan, zone_table[nr].name);
                }
            }
            break;
        default:
            sprintf(buf, "SYSERR:: invalid SCMD passed to ACMDdo_build_list!");
            mudlog(buf, BRF, LVL_GOD, TRUE);
            return;
    }

    if (!found) {
        switch (subcmd) {
            case SCMD_RLIST:
                send_to_char("Нет комнат в этом промежутке.\r\n", ch);
                break;
            case SCMD_OLIST:
                send_to_char("Нет объектов в этом промежутке.\r\n", ch);
                break;
            case SCMD_MLIST:
                send_to_char("Нет мобов в этом промежутке.\r\n", ch);
                break;
            case SCMD_ZLIST:
                send_to_char("Нет зон в этом промежутке.\r\n", ch);
                break;
            default:
                sprintf(buf, "SYSERR:: invalid SCMD passed to do_build_list!");
                mudlog(buf, BRF, LVL_GOD, TRUE);
                break;
        }
        return;
    }

    page_string(ch->desc, buf, 1);
}

ACMD(do_name)
{
    struct descriptor_data *i;
    struct char_data *vict;
    int lev;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Для кого?\r\n", ch);
        return;
    }

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_WORLD))) {
        send_to_char("Нет такого игрока.\r\n", ch);
        return;
    }

    if (IS_NPC(vict)) {
        send_to_char("Невозможно для моба.\r\n", ch);
        return;
    }

    if (GET_LEVEL(vict) > GET_LEVEL(ch) && !GET_COMMSTATE(ch)) {
        send_to_char("У Вас не хватит сил.\r\n", ch);
        return;
    }

    /* одобряем или нет */
    lev = NAME_GOD(vict);
    if (lev > 1000)
        lev = lev - 1000;
    if (lev > GET_LEVEL(ch) && !GET_COMMSTATE(ch)) {
        send_to_char("Об этом имени уже позаботился бог старше Вас.\r\n", ch);
        return;
    }

    one_argument(argument, arg);
    if (is_abbrev(arg, "одобрить")) {
        NAME_GOD(vict) = GET_LEVEL(ch) + 1000;
        NAME_ID_GOD(vict) = GET_IDNUM(ch);
        send_to_char("Имя одобрено!\r\n", ch);
        send_to_char("Боги одобрили Ваше имя!\r\n", vict);
        sprintf(buf, "zname %s", GET_NAME(vict));
        command_interpreter(ch, buf);

        /*
           act("$n превратил$u в разноцветный дым.", FALSE, vict, 0, 0, TO_ROOM);
           char_from_room(vict);
           char_to_room(vict,r_mortal_start_room);
           act("$n появил$u из разноцветного дыма.", FALSE, vict, 0, 0, TO_ROOM);
           GET_LOADROOM(vict) = mortal_start_room;
         */
        save_char(vict, NOWHERE);
    } else if (is_abbrev(arg, "запретить")) {
        NAME_GOD(vict) = GET_LEVEL(ch);
        NAME_ID_GOD(vict) = GET_IDNUM(ch);
        send_to_char("Имя запрещено!\r\n", ch);
        send_to_charf(vict, "Ваше имя было запрещено Богами как несоответствующее правилам.\r\n");
        sprintf(buf, "xname %s", GET_NAME(vict));
        command_interpreter(ch, buf);
        send_to_char("Прощайте!\r\n", vict);
        act("$n провалил$u в небытие.", FALSE, vict, 0, 0, TO_ROOM);
        for (i = descriptor_list; i; i = i->next)
            if (i->character == vict) {
                if (STATE(i) == CON_PLAYING)
                    STATE(i) = CON_DISCONNECT;
                else
                    STATE(i) = CON_CLOSE;
            }

    } else {
        send_to_char("Можно либо 'одобрить' либо 'запретить'.\r\n", ch);
    }
}



ACMD(do_copyover)
{
    int times = 0;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, arg);
    copyover_time = 0;

    //if (arg && *arg && (times = atoi(arg)) > 0)
    if (*arg && (times = atoi(arg)) > 0) {
        copyover_time = time(NULL) + times;
    }

    log("(GC) Copyover by %s.", GET_NAME(ch));
    sprintf(buf, "%s установил горячую перезагрузку на %d %s", GET_NAME(ch), times,
            desc_count(times, WHAT_SEC));

    /* if (!times)
       send_to_all("&WСИСТЕМА: ГОРЯЧАЯ ПЕРЕЗАГРУЗКА.&n\r\n");
       else
       {
       sprintf(buf,"&WСИСТЕМА: ГОРЯЧАЯ ПЕРЕЗАГРУЗКА через %d %s.&n\r\n",times,desc_count(times,WHAT_SEC));
       send_to_all(buf);
       }; */

    circle_copyover = TRUE;

}

ACMD(do_spell_set)
{
    struct char_data *victim;
    int qend, spellnum;
    bool isdelete = FALSE;
    char arg[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch,
                      "Формат: устзаклинание: <имя_персонажа> <!название_заклинания!> [удалить]\r\n");
        return;
    }

    if (!(victim = get_char_vis(ch, arg, FIND_CHAR_WORLD))) {
        send_to_char(NOPERSON, ch);
        return;
    }
    skip_spaces(&argument);

    if (!*argument) {
        send_to_char("Пропущено название заклинания.\r\n", ch);
        return;
    }
    if (*argument != *ch->divd) {
        send_to_charf(ch, "Заклинание надо заключить в символы: '%s'.\r\n", ch->divd);
        return;
    }

    for (qend = 1; argument[qend] && argument[qend] != *ch->divd; qend++)
        argument[qend] = LOWER(argument[qend]);

    if (argument[qend] != *ch->divd) {
        send_to_charf(ch, "Заклинание должно быть заключено в символы: '%s'.\r\n", ch->divd);
        return;
    }
    strcpy(arg, (argument + 1));
    arg[qend - 1] = '\0';

    spellnum = find_spell_num(arg);
    if (spellnum < 0) {
        send_to_charf(ch, "Неизвестное заклинание.\r\n");
        return;
    }

    argument += qend + 1;       /* skip to next parameter */
    argument = one_argument(argument, arg);

    if (*arg)
        if (isname(arg, "удалить"))
            isdelete = TRUE;

    if (isdelete)
        GET_SPELL_TYPE(victim, SPELL_NO(spellnum)) = FALSE;
    else
        GET_SPELL_TYPE(victim, SPELL_NO(spellnum)) = TRUE;

    send_to_charf(ch, "Вы %s залинание '&C%s&n' у %s.\r\n",
                  (isdelete ? "удалили" : "установили"), SPELL_NAME(spellnum), GET_PAD(victim, 1));

}

ACMD(do_spell_view)
{
    struct char_data *victim;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Чьи заклинания Вы хотите посмотреть?\r\n");
        return;
    }

    if (!(victim = get_char_vis(ch, arg, FIND_CHAR_WORLD))) {
        send_to_char(NOPERSON, ch);
        return;
    }

    if (IS_MOB(victim))
        act("$N - монстр, посмотреть $S заклинания таким способом нельзя.", FALSE, ch, 0, victim,
            TO_CHAR);
    else {
        list_spells(ch, victim, CLASS_MAGIC_USER);
        list_spells(ch, victim, CLASS_NECRO);
        list_prays(ch, victim);
    }
};


ACMD(do_skillview)
{
    int i, flag = 1;
    char help[MAX_STRING_LENGTH], name[MAX_INPUT_LENGTH];
    struct char_data *vict;

    *help = '\0';

    if (!*argument) {
        send_to_char("Формат: смумения <жертва> [все]", ch);
    }
    argument = one_argument(argument, name);

    if (!*name)
        vict = ch;
    else {
        if (!(vict = get_char_vis(ch, name, FIND_CHAR_WORLD))) {
            send_to_char("Персонаж с таким именем не найден.\r\n", ch);
            return;
        }
    }
    argument = one_argument(argument, name);

    if (*name) {
        if (!strcmp(name, "all") || !strcmp(name, "все"))
            flag = 0;
    }
    for (i = 0; i <= TOP_SKILL_DEFINE; i++) {
        if (skill_info[i].name != unused_spellname) {
            if (flag) {
                if (GET_SKILL(vict, i) > 0)
                    sprintf(help + strlen(help), "%18s: %i [%i] \r\n", skill_info[i].name.c_str(),
                            GET_SKILL(vict, i), GET_SKILL_LEVEL(vict, i));
            } else
                sprintf(help + strlen(help), "%18s: %i [%i] \r\n", skill_info[i].name.c_str(),
                        GET_SKILL(vict, i), GET_SKILL_LEVEL(vict, i));
        }
    }
    page_string(ch->desc, help, 1);
}


ACMD(do_skillset)
{
    struct char_data *vict;
    char name[MAX_INPUT_LENGTH], buf2[128];
    char buf[MAX_INPUT_LENGTH], help[MAX_STRING_LENGTH];
    int skill = -1, value, svalue, i, qend;

    if (!argument || !*argument) {
        send_to_char
            ("Формат: устумение <игрок> !<умение/заклинание>! <значение> <значение_специализации>\r\n",
             ch);
        return;
    }

    argument = one_argument(argument, name);
    /*
     * No arguments. print an informative text.
     */
    if (!*name) {               /* no arguments. print an informative text */
        send_to_char
            ("Формат: устумение <игрок> !<умение/заклинание>! <значение> <значение_специализации>\r\n",
             ch);
        strcpy(help, "Возможные умения:\r\n");
        for (qend = 0, i = 0; i <= TOP_SPELL_DEFINE; i++) {
            if (skill_info[i].name == unused_spellname) /* This is valid. */
                continue;
            sprintf(help + strlen(help), "%18s", skill_info[i].name.c_str());
            if (qend++ % 4 == 3) {
                strcat(help, "\r\n");
                send_to_char(help, ch);
                *help = '\0';
            }
        }
        if (*help)
            send_to_char(help, ch);
        send_to_char("\r\n", ch);
        return;
    }

    if (!(vict = get_char_vis(ch, name, FIND_CHAR_WORLD))) {
        send_to_char(NOPERSON, ch);
        return;
    }
    skip_spaces(&argument);

    /* If there is no chars in argument */
    if (!*argument) {
        send_to_char("Пропущено название умения.\r\n", ch);
        return;
    }
    if (*argument != *ch->divd) {
        send_to_charf(ch, "Умение надо заключить в символы: '%s'.\r\n", ch->divd);
        return;
    }
    /* Locate the last quote and lowercase the magic words (if any) */

    for (qend = 1; argument[qend] && argument[qend] != *ch->divd; qend++)
        argument[qend] = LOWER(argument[qend]);

    if (argument[qend] != *ch->divd) {
        send_to_charf(ch, "Умение должно быть заключено в символы: '%s'.\r\n", ch->divd);
        return;
    }
    strcpy(help, (argument + 1));
    help[qend - 1] = '\0';

    if ((skill = find_skill_num(help)) < 0)

        if (skill < 0) {
            send_to_char("Неизвестное умение.\r\n", ch);
            return;
        }
    argument += qend + 1;       /* skip to next parameter */
    argument = one_argument(argument, buf);

    if (!*buf) {
        send_to_char("Пропущен уровень умения.\r\n", ch);
        return;
    }
    value = atoi(buf);
    if (value < 0) {
        send_to_char("Минимальное значение умения 0.\r\n", ch);
        return;
    }
    if (value > 151) {
        send_to_char("Максимальное значение умения 151. \r\n", ch);
        return;
    }

    argument = one_argument(argument, buf);
    if (!*buf) {
        send_to_char("Пропущен уровень специализации.\r\n", ch);
        return;
    }

    svalue = atoi(buf);
    if (svalue < 0) {
        send_to_char("Минимальное значение специализации 0. \r\n", ch);
        return;
    }
    if (svalue > ExtraSkillValuesCol - 1) {
        sprintf(buf, "Максимальное значение специализации %i \r\n", ExtraSkillValuesCol - 1);
        send_to_char(buf, ch);
        return;
    }

    if (value < ExtraSkillValues[svalue]) {
        send_to_char("Уровень умения слишком мал, для данного уровня специализации. \r\n", ch);
        return;
    }

    if (IS_NPC(vict)) {
        send_to_char("Вы не можете добавить умение для мобов.\r\n", ch);
        return;
    }

    sprintf(buf2, "%s изменил умение '%s' %s на %d.", GET_NAME(ch),
            skill_info[skill].name.c_str(), GET_NAME(vict), value);
    mudlog(buf2, BRF, LVL_GOD, TRUE);
    if (skill >= 0 && skill <= MAX_SKILLS) {
        SET_SKILL(vict, skill) = value;
        SET_SKILL_LEVEL(vict, skill, svalue);
    }
    sprintf(buf2, "Вы изменили для %s '%s' на %d [%i]. \r\n", GET_PAD(vict, 1),
            skill_info[skill].name.c_str(), value, GET_SKILL_LEVEL(vict, skill));
    send_to_char(buf2, ch);
}


ACMD(do_ress)
{
    struct char_data *victim = NULL;
    char arg[MAX_INPUT_LENGTH];

    one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Кого Вы хотите воскресить?\r\n");
        return;
    }

    if (!(victim = get_player_soul(ch, arg, FALSE))) {
        send_to_charf(ch, "Души с именем '%s' в мире не видно.\r\n");
        return;
    }

    restore_soul(victim, NULL, FALSE);
}

void show_char_rent(struct char_data *ch, char *name)
{
    char fname[MAX_INPUT_LENGTH];
    char buf[MAX_EXTEND_LENGTH];
    int i, number, c;
    time_t time;

    if ((c = cmp_ptable_by_name(name, 0)) == -1) {
        send_to_charf(ch, "Персонаж '%s' не найден.\r\n", name);
        return;
    }

    strcpy(name, player_table[c].name);

    CObj Wobj;
    CRent Rent;

    if (!Wobj.Initialization() || !Rent.Initialization()) {     //Ошибка инициализации стуркутуры
        send_to_charf(ch, "Ошибка инициализации.\r\n");
        return;
    }
    *buf = '\0';

    sprintf(buf, "Информация о постое персонажа %s:\r\n", CAP(name));
    sprintf(buf + strlen(buf), "-----------------------------\r\n");

    get_filename(name, fname, TIME_CRASH_FILE);
    if (!Rent.ReadConfig(fname)) {
        sprintf(buf + strlen(buf), "Файл ренты отсутствует.\r\n");
        page_string(ch->desc, buf, 1);
        return;
    }

    time = Rent.GetItem(0)->GetItem(RENT_TIME)->GetInt();
    sprintf(buf + strlen(buf), "Ушел на постой: &C%s&n Тип ренты: &G%d&n\r\n\r\n", ascii_time(time),
            Rent.GetItem(0)->GetItem(RENT_TYPE)->GetInt());

    get_filename(name, fname, TEXT_CRASH_FILE);

    if (!Wobj.ReadConfig(fname)) {      //Файл ренты отсутсвует
        sprintf(buf + strlen(buf), "Файл предметов отсутствует.\r\n");
        page_string(ch->desc, buf, 1);
        return;
    }
    number = Wobj.GetNumberItem();

    if (number <= 0) {          //Предметов нет
        sprintf(buf + strlen(buf), "Предметов нет.\r\n");
        page_string(ch->desc, buf, 1);
        return;
    }

    for (i = 0; i < number; i++) {
        int vnum, pos = -1, equip = -1;
        struct new_flag obj_prop = { {0, 0, 0, 0} };

        vnum = Wobj.GetItem(i)->GetItem(OBJ_NUMBER)->GetInt();
        if (Wobj.GetItem(i)->GetItem(OBJ_XSAVE)->GetNumberItem()) {
            equip =
                Wobj.GetItem(i)->GetItem(OBJ_XSAVE)->GetItem(0)->GetItem(OBJ_XSAVE_EQ)->GetInt();
            pos = Wobj.GetItem(i)->GetItem(OBJ_XSAVE)->GetItem(0)->GetItem(OBJ_XSAVE_POS)->GetInt();
        }
        if (pos > 99)
            sprintf(buf + strlen(buf), "   [%6d] %-30s ", vnum,
                    get_name_pad(Wobj.GetItem(i)->GetItem(OBJ_NAME)->GetString(), PAD_IMN,
                                 PAD_OBJECT));
        else
            sprintf(buf + strlen(buf), "[%6d] %-33s ", vnum,
                    get_name_pad(Wobj.GetItem(i)->GetItem(OBJ_NAME)->GetString(), PAD_IMN,
                                 PAD_OBJECT));

        if (equip > -1)
            sprintf(buf + strlen(buf), "%s", where[equip]);
        else if (pos > -1)
            sprintf(buf + strlen(buf), "<в контейнере>");
        else
            sprintf(buf + strlen(buf), "<в инвентаре>");

        sprintf(buf + strlen(buf), " таймер: %d", Wobj.GetItem(i)->GetItem(OBJ_TIMER)->GetInt());

        if (Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString())
            asciiflag_conv((char *) Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString(),
                           &obj_prop);

        if (IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE), ITEM_RENT_DELETE) ||
            IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE2), ITEM_RENT_DELETE2) ||
            IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE3), ITEM_RENT_DELETE3))
            sprintf(buf + strlen(buf), " * удален *");

        sprintf(buf + strlen(buf), "\r\n");
    }

    page_string(ch->desc, buf, 1);
}

void show_obj_location(struct char_data *ch, int vnum)
{
    int rnum = real_object(vnum), c, number, i, counts;
    bool c_online, is_god;
    struct obj_data *obj;
    struct descriptor_data *d;
    struct char_data *tch;
    char fname[MAX_INPUT_LENGTH];
    char name[MAX_NAME_LENGTH];
    char buf[MAX_EXTEND_LENGTH];

    if (rnum == -1) {
        send_to_charf(ch, "Предмет [%d] или не найден в базе данных или является уникальным.\r\n",
                      vnum);
        return;
    }

    sprintf(buf, "Информация по предмету [%d] %s:\r\n", vnum, obj_proto[rnum].name);
    sprintf(buf + strlen(buf), "-------\r\n");
    sprintf(buf + strlen(buf), "Общий лимит предмета:    %d из %d\r\n",
            obj_index[rnum].number + obj_index[rnum].stored, obj_proto[rnum].limit);
    sprintf(buf + strlen(buf), "Кол-во предмета в мире:  %d\r\n", obj_index[rnum].number);
    sprintf(buf + strlen(buf), "Кол-во предмета в ренте: %d\r\n", obj_index[rnum].stored);
    sprintf(buf + strlen(buf), "Информация о ренте предмета:\r\n");
    for (c = 0; c <= top_of_p_table; c++)
        if (player_table[c].unique != -1) {
            c_online = FALSE;
            is_god = player_table[c].level >= 32;
            for (d = descriptor_list; d; d = d->next)   //Если персонаж в игре ставим флаг online
                if ((STATE(d) == CON_PLAYING) && GET_IDNUM(d->character) == player_table[c].id)
                    c_online = TRUE;
            strcpy(name, player_table[c].name);
            CObj Wobj;

            if (!Wobj.Initialization()) //Ошибка инициализации стуркутуры
                break;
            get_filename(name, fname, TEXT_CRASH_FILE);
            if (!Wobj.ReadConfig(fname))        //Файл ренты отсутсвует
                continue;
            number = Wobj.GetNumberItem();
            if (number <= 0)    //Предметов нет
                continue;
            counts = 0;
            for (i = 0; i < number; i++) {
                struct new_flag obj_prop = { {0, 0, 0, 0} };
                if (vnum != Wobj.GetItem(i)->GetItem(OBJ_NUMBER)->GetInt())
                    continue;

                if (Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString())
                    asciiflag_conv((char *) Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString(),
                                   &obj_prop);

                if (!is_god
                    && (IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE), ITEM_RENT_DELETE)
                        || IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE2), ITEM_RENT_DELETE2)
                        || IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE3), ITEM_RENT_DELETE3)))
                    continue;

                counts++;
            }
            if (counts)
                sprintf(buf + strlen(buf), " %-20s [%-6s] : %d шт.\r\n", CAP(name),
                        c_online ? "online" : "offline", counts);
        }

    sprintf(buf + strlen(buf), "Местонахождение предемета в мире:\r\n");

    for (tch = character_list; tch; tch = tch->next) {
        struct mob_shop_data *m;
        struct list_obj_data *i;

        if (!IS_NPC(tch))
            continue;
        m = tch->shop_data;
        if (!m)
            continue;
        for (i = m->obj_list; i; i = i->next)
            if (i->vnum == vnum)
                sprintf(buf + strlen(buf), " %-20s : магазин, кол-во: %d\r\n", CAP(GET_NAME(tch)),
                        i->percent);
    }

    for (obj = object_list; obj; obj = obj->next) {
        if (GET_OBJ_VNUM(obj) != vnum)
            continue;

        if (obj->carried_by)
            sprintf(buf + strlen(buf), " %-20s : инвентарь\r\n", CAP(GET_NAME(obj->carried_by)));
        else if (obj->worn_by)
            sprintf(buf + strlen(buf), " %-20s : экипировка\r\n", CAP(GET_NAME(obj->worn_by)));
        else if (obj->in_room > NOWHERE)
            sprintf(buf + strlen(buf), " %-6d : локация '%s'\r\n", GET_ROOM_VNUM(IN_ROOM(obj)),
                    get_name_pad(world[obj->in_room].name, PAD_IMN, PAD_OBJECT));
        else if (obj->in_obj)
            sprintf(buf + strlen(buf), " контейнер %s [%-6d]\r\n", GET_OBJ_PNAME(obj->in_obj, 0),
                    GET_OBJ_VNUM(obj->in_obj));
        else
            sprintf(buf + strlen(buf), "&R .место положение не зафиксированно.&n\r\n");
    }

    page_string(ch->desc, buf, 1);
}

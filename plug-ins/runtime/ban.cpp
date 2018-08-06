/* ************************************************************************
*   File: ban.c                                         Part of CircleMUD *
*  Usage: banning/unbanning/checking sites and player names               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "structs.h"
#include "screen.h"
#include "utils.h"
#include "comm.h"
#include "ban.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "planescape.h"
#include "dlfileop.h"
#include "mudfile.h"

/* local functions */
void _write_one_node(FILE * fp, struct ban_list_element *node);
void _write_one_proxy(FILE * fp, struct proxy_list_element *node);
void write_ban_list(void);
void write_proxy_list(void);

struct ban_list_element *ban_list = NULL;
struct proxy_list_element *proxy_list = NULL;

const char *ban_types[] = {
    "no",
    "new",
    "select",
    "all",
    "multer",
    "ERROR"
};


void load_proxy(void)
{
    int date, numb = 0;
    char site_name[BANNED_SITE_LENGTH + 1];
    char name[MAX_NAME_LENGTH + 1];
    char misc[MAX_STRING_LENGTH + 1];
    struct proxy_list_element *next_node;

    DLFileRead file(MudFile(mud->proxyFile));

    if (!file.exist()) {
        log("Список прокси пуст");
        return;
    }

    if (!file.open())
        return;

    while (file.scanf(" %s %d %d %s %s\n", site_name, &date, &numb, name, misc) >= 5) {
        CREATE(next_node, struct proxy_list_element, 1);

        strncpy(next_node->site, site_name, BANNED_SITE_LENGTH);
        next_node->site[BANNED_SITE_LENGTH] = '\0';
        strncpy(next_node->name, name, MAX_NAME_LENGTH);
        strncpy(next_node->misc, misc, MAX_STRING_LENGTH);
        next_node->misc[MAX_STRING_LENGTH] = '\0';
        next_node->name[MAX_NAME_LENGTH] = '\0';
        next_node->number = numb;
        next_node->date = date;

//  log("Загружено: %s %ld %d %s",next_node->site, (long) next_node->date, next_node->number, next_node->name);

        next_node->next = proxy_list;
        proxy_list = next_node;
    }
}

void load_banned(void)
{
    int i, date;
    char site_name[BANNED_SITE_LENGTH + 1], ban_type[100], metter[MAX_STRING_LENGTH + 1];
    char name[MAX_NAME_LENGTH + 1];
    struct ban_list_element *next_node;

    DLFileRead file(MudFile(mud->banFile));

    if (!file.exist()) {
        log("Список банов пуст");
        return;
    }

    if (!file.open())
        return;

    while (file.scanf(" %s %s %d %s %s\n", ban_type, site_name, &date, name, metter) >= 4) {
        CREATE(next_node, struct ban_list_element, 1);

        strncpy(next_node->site, site_name, BANNED_SITE_LENGTH);
        next_node->site[BANNED_SITE_LENGTH] = '\0';
        strncpy(next_node->name, name, MAX_NAME_LENGTH);
        next_node->name[MAX_NAME_LENGTH] = '\0';
        strncpy(next_node->metter, metter, BANNED_METTER_LENGTH);
        next_node->metter[BANNED_METTER_LENGTH] = '\0';
        next_node->date = date;

        for (i = BAN_NOT; i <= BAN_MULTER; i++)
            if (!strcmp(ban_type, ban_types[i]))
                next_node->type = i;

        next_node->next = ban_list;
        ban_list = next_node;
    }
}


int isbanned(char *hostname)
{
    int i;
    struct ban_list_element *banned_node;
    char *nextchar;

    if (!hostname || !*hostname)
        return (0);

    i = 0;
    for (nextchar = hostname; *nextchar; nextchar++)
        *nextchar = LOWER(*nextchar);

    for (banned_node = ban_list; banned_node; banned_node = banned_node->next)
        if ((!str_cmp(banned_node->site, "all") || !str_cmp(banned_node->site, "все")) &&
            strlen(banned_node->site) == 3)
            i = MAX(i, banned_node->type);
        else if (strstr(hostname, banned_node->site))   /* if hostname is a substring */
            i = MAX(i, banned_node->type);

    return (i);
}

int is_proxy(char *hostname)
{
    int i;
    struct proxy_list_element *proxy_node;
    char *nextchar;

    if (!hostname || !*hostname)
        return (0);

    i = 0;
    for (nextchar = hostname; *nextchar; nextchar++)
        *nextchar = LOWER(*nextchar);

    for (proxy_node = proxy_list; proxy_node; proxy_node = proxy_node->next)
        if (strstr(hostname, proxy_node->site)) /* if hostname is a substring */
            i = proxy_node->number;

    return (i);
}

void _write_one_node(FILE * fp, struct ban_list_element *node)
{
    if (node) {
        _write_one_node(fp, node->next);
        fprintf(fp, "%s %s %ld %s %s\n", ban_types[node->type],
                node->site, (long) node->date, node->name, node->metter);
    }
}

void _write_one_proxy(FILE * fp, struct proxy_list_element *node)
{
    if (node) {
        _write_one_proxy(fp, node->next);
        fprintf(fp, "%s %ld %d %s %s\n",
                node->site, (long) node->date, node->number, node->name, node->misc);
    }
}



void write_ban_list(void)
{
    DLFileWrite file(MudFile(mud->banFile));

    if (!file.open())
        return;

    _write_one_node(file.getFP(), ban_list);    /* recursively write from end to start */
}

void write_proxy_list(void)
{
    DLFileWrite file(MudFile(mud->proxyFile));

    if (!file.open())
        return;

    _write_one_proxy(file.getFP(), proxy_list); /* recursively write from end to start */
}


ACMD(do_proxy)
{
    char times[256], numb[256], *nextchar, site[MAX_INPUT_LENGTH], misc[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    struct proxy_list_element *proxy_node;

    if (!*argument) {
        if (!proxy_list) {
            send_to_char("Список прокси пустой.\r\n", ch);
            return;
        }
        sprintf(buf, "%s", CCICYN(ch, C_NRM));
        sprintf(buf + strlen(buf), "Список многопользовательских адресов:\r\n");
        sprintf(buf + strlen(buf), "%-16s %-10s %-15s %-25s %s\r\n", "Адрес", "Дата",
                "Кол-во входов", "Добавил", "Комментарий");

        send_to_charf(ch, buf);
        for (proxy_node = proxy_list; proxy_node; proxy_node = proxy_node->next) {
            if (proxy_node->date)
                strcpy(times, ascii_time(proxy_node->date));
            else
                strcpy(times, "Неизвестно");

            if (proxy_node->number != 65535)
                sprintf(numb, "%d", proxy_node->number);
            else
                strcpy(numb, "Без ограничений");
            sprintf(buf, "%s", CCCYN(ch, C_NRM));
            sprintf(buf + strlen(buf), "%-16s %-10.10s %-15s %-25s %s\r\n", proxy_node->site, times,
                    numb, proxy_node->name, proxy_node->misc);
            sprintf(buf + strlen(buf), "%s", CCNRM(ch, C_NRM));
            send_to_charf(ch, buf);

        }

        return;
    }
    argument = one_argument(argument, site);
    if (!*site) {
        send_to_char("Формат: прокси IP-адресс [кол-во точек входа [комментарий]]\r\n", ch);
        return;
    }

    for (proxy_node = proxy_list; proxy_node; proxy_node = proxy_node->next)
        if (!str_cmp(proxy_node->site, site)) {
            send_to_char("Этот прокси уже добавлен в список.\r\n", ch);
            return;
        }

    CREATE(proxy_node, struct proxy_list_element, 1);
    strncpy(proxy_node->site, site, BANNED_SITE_LENGTH);
    for (nextchar = proxy_node->site; *nextchar; nextchar++)
        *nextchar = LOWER(*nextchar);
    proxy_node->site[BANNED_SITE_LENGTH] = '\0';
    strncpy(proxy_node->name, GET_NAME(ch), MAX_NAME_LENGTH);
    proxy_node->name[MAX_NAME_LENGTH] = '\0';
    two_arguments(argument, numb, misc);
    if (*numb) {
        proxy_node->number = atoi(numb);
    } else
        proxy_node->number = 65535;

    if (*misc)
        strcpy(proxy_node->misc, misc);
    else
        strcpy(proxy_node->misc, "-");


    proxy_node->date = time(0);


    proxy_node->next = proxy_list;
    proxy_list = proxy_node;

    sprintf(buf, "%s добавил %s в список прокси на %d входов.", GET_NAME(ch),
            site, proxy_node->number);
    mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
    send_to_char("Прокси добавлен.\r\n", ch);
    write_proxy_list();

}

ACMD(do_ban)
{
    char flag[MAX_INPUT_LENGTH], site[MAX_INPUT_LENGTH],
        format[MAX_INPUT_LENGTH], metter[MAX_INPUT_LENGTH], *nextchar;
    int i;
    struct ban_list_element *ban_node;
    char buf[MAX_STRING_LENGTH];

    if (!*argument) {
        if (!ban_list) {
            send_to_char("No sites are banned.\r\n", ch);
            return;
        }
        strcpy(format, "%-25.25s  %-8.8s  %-10.10s  %-16.16s%s\r\n");
        sprintf(buf, format, "Banned Site Name", "Ban Type", "Banned On", "Banned By", "");

        send_to_char(buf, ch);
        sprintf(buf, format,
                "---------------------------------",
                "---------------------------------",
                "---------------------------------", "---------------------------------", "");
        send_to_char(buf, ch);

        for (ban_node = ban_list; ban_node; ban_node = ban_node->next) {
            if (ban_node->date)
                strcpy(site, ascii_time(ban_node->date));
            else
                strcpy(site, "Unknown");
            strcpy(metter, "\r\n");
            if (*ban_node->metter)
                strcat(metter, ban_node->metter);
            else
                strcat(metter, "Unknown metter");

            sprintf(buf, format, ban_node->site, ban_types[ban_node->type], site,
                    ban_node->name, metter);
            send_to_char(buf, ch);
        }
        return;
    }
    argument = two_arguments(argument, flag, site);
    if (!*site || !*flag) {
        send_to_char("Usage: ban {all | select | new} site_name [metter]\r\n", ch);
        return;
    }
    if (!
        (!str_cmp(flag, "select") || !str_cmp(flag, "all") || !str_cmp(flag, "new")
         || !str_cmp(flag, "multer"))) {
        send_to_char("Flag must be ALL, SELECT, NEW or MULTER.\r\n", ch);
        return;
    }
    for (ban_node = ban_list; ban_node; ban_node = ban_node->next) {
        if (!str_cmp(ban_node->site, site)) {
            send_to_char
                ("That site has already been banned -- unban it to change the ban type.\r\n", ch);
            return;
        }
    }

    CREATE(ban_node, struct ban_list_element, 1);
    strncpy(ban_node->site, site, BANNED_SITE_LENGTH);
    for (nextchar = ban_node->site; *nextchar; nextchar++)
        *nextchar = LOWER(*nextchar);
    ban_node->site[BANNED_SITE_LENGTH] = '\0';
    strncpy(ban_node->name, GET_NAME(ch), MAX_NAME_LENGTH);
    ban_node->name[MAX_NAME_LENGTH] = '\0';
    one_argument(argument, metter);
    if (*metter) {
        strncpy(ban_node->metter, metter, BANNED_METTER_LENGTH);
        ban_node->metter[BANNED_METTER_LENGTH] = '\0';
    } else
        strcpy(ban_node->metter, "Unknown metter");

    ban_node->date = time(0);

    for (i = BAN_NEW; i <= BAN_MULTER; i++)
        if (!str_cmp(flag, ban_types[i]))
            ban_node->type = i;

    ban_node->next = ban_list;
    ban_list = ban_node;

    sprintf(buf, "%s has banned %s for %s players(%s).", GET_NAME(ch), site,
            ban_types[ban_node->type], ban_node->metter);
    mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);
    send_to_char("Site banned.\r\n", ch);
    write_ban_list();
}


ACMD(do_unproxy)
{
    char site[MAX_INPUT_LENGTH];
    struct proxy_list_element *proxy_node, *temp;
    int found = 0;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, site);
    if (!*site) {
        send_to_char("Формат: опрокси IP-адрес.\r\n", ch);
        return;
    }

    proxy_node = proxy_list;
    while (proxy_node && !found) {
        if (!str_cmp(proxy_node->site, site))
            found = 1;
        else
            proxy_node = proxy_node->next;
    }

    if (!found) {
        send_to_char("Этого прокси в списке нет.\r\n", ch);
        return;
    }

    REMOVE_FROM_LIST(proxy_node, proxy_list, next);
    send_to_char("Прокси удален.\r\n", ch);
    sprintf(buf, "%s удалил прокси %s из списка.", GET_NAME(ch), proxy_node->site);
    mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

    free(proxy_node);
    write_proxy_list();
}

ACMD(do_unban)
{
    char site[MAX_INPUT_LENGTH];
    struct ban_list_element *ban_node, *temp;
    int found = 0;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, site);
    if (!*site) {
        send_to_char("A site to unban might help.\r\n", ch);
        return;
    }
    ban_node = ban_list;
    while (ban_node && !found) {
        if (!str_cmp(ban_node->site, site))
            found = 1;
        else
            ban_node = ban_node->next;
    }

    if (!found) {
        send_to_char("That site is not currently banned.\r\n", ch);
        return;
    }
    REMOVE_FROM_LIST(ban_node, ban_list, next);
    send_to_char("Site unbanned.\r\n", ch);
    sprintf(buf, "%s removed the %s-player ban on %s.",
            GET_NAME(ch), ban_types[ban_node->type], ban_node->site);
    mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

    free(ban_node);
    write_ban_list();
}


/**************************************************************************
 *  Code to check for invalid names (i.e., profanity, etc.)    *
 *  Written by Sharon P. Goza        *
 **************************************************************************/

char *invalid_list[MAX_INVALID_NAMES];
char *curses_list[MAX_INVALID_NAMES];
char *valid_list[MAX_INVALID_NAMES];
int num_invalid = 0;
int num_valid = 0;
int num_curses = 0;

int Is_Registry_Name(char *newname)
{
    int i;
    char tempname[MAX_INPUT_LENGTH];

    if (!*valid_list || num_valid < 1)
        return (0);

    /* change to lowercase */
    strcpy(tempname, newname);
    for (i = 0; tempname[i]; i++)
        tempname[i] = LOWER(tempname[i]);

    /* Does the desired name contain a string in the invalid list? */
    for (i = 0; i < num_valid; i++)
        if (strstr(tempname, valid_list[i]))
            return (1);

    return (0);
}

int Is_Valid_Name(char *newname)
{
    int i;
    char tempname[MAX_INPUT_LENGTH];

    if (!*invalid_list || num_invalid < 1)
        return (1);

    /* change to lowercase */
    strcpy(tempname, newname);
    for (i = 0; tempname[i]; i++)
        tempname[i] = LOWER(tempname[i]);

    /* Does the desired name contain a string in the invalid list? */
    for (i = 0; i < num_invalid; i++)
        if (strstr(tempname, invalid_list[i]))
            return (0);

    /* Проверка на имена мобов */
    for (i = 0; i < top_of_mobt; i++) {
        if (isname(tempname, mob_proto[i].player.short_descr))
            return 0;
    }

    return (1);
}


int Is_Valid_Name_no_mob(char *newname)
{
    int i;
    char tempname[MAX_INPUT_LENGTH];

    if (!*invalid_list || num_invalid < 1)
        return (1);

    /* change to lowercase */
    strcpy(tempname, newname);
    for (i = 0; tempname[i]; i++)
        tempname[i] = LOWER(tempname[i]);

    /* Does the desired name contain a string in the invalid list? */
    for (i = 0; i < num_invalid; i++)
        if (strstr(tempname, invalid_list[i]))
            return (0);

    return (1);
}

int Is_Valid_Dc(char *newname)
{
    struct descriptor_data *dt;

    for (dt = descriptor_list; dt; dt = dt->next)
        if (dt->character && GET_NAME(dt->character) && !str_cmp(GET_NAME(dt->character), newname))
            return (STATE(dt) == CON_PLAYING);
    return (1);
}

int Valid_Name(char *newname)
{
    return (Is_Valid_Name(newname) && Is_Valid_Dc(newname));
}


void Read_Curses_List(void)
{
    char temp[256];
    DLFileRead file(ShareFile(mud->cursesFile));

    if (!file.open())
        return;

    num_invalid = 0;
    while (get_line(file.getFP(), temp) && num_curses < MAX_INVALID_NAMES)
        str_reassign(curses_list[num_curses++], temp);

    if (num_curses >= MAX_INVALID_NAMES) {
        log("SYSERR: Too many curses names; change MAX_INVALID_NAMES in ban.c");
        exit(1);
    }
}

void Read_Invalid_List(void)
{
    char temp[256];
    DLFileRead file(MudFile(mud->xnameFile));

    if (!file.open())
        return;

    num_invalid = 0;
    while (get_line(file.getFP(), temp) && num_invalid < MAX_INVALID_NAMES) {
        str_reassign(invalid_list[num_invalid], temp);
        num_invalid++;
    }

    if (num_invalid >= MAX_INVALID_NAMES) {
        log("SYSERR: Too many invalid names; change MAX_INVALID_NAMES in ban.c");
        exit(1);
    }
}

void Read_Valid_List(void)
{
    char temp[256];
    DLFileRead file(MudFile(mud->znameFile));

    if (!file.open())
        return;

    num_valid = 0;
    while (get_line(file.getFP(), temp) && num_valid < MAX_INVALID_NAMES) {
        str_reassign(valid_list[num_valid], temp);
        num_valid++;
    }

    if (num_valid >= MAX_INVALID_NAMES) {
        log("SYSERR: Too many valid names; change MAX_INVALID_NAMES in ban.c");
        exit(1);
    }
}

void ban_init()
{
    /* after world load, near the end */
    log("Загрузка банлиста, списка прокси");
    load_banned();
    load_proxy();

    log("Загрузка списка запрещеных/одобреных имен");
    Read_Invalid_List();
    Read_Valid_List();

    log("Загрузка списка запрещеных слов");
    Read_Curses_List();
}

void ban_destroy()
{
    if (proxy_list) {
        struct proxy_list_element *temp;

        DESTROY_LIST(proxy_list, next, temp);
    }

    if (ban_list) {
        struct ban_list_element *temp;

        DESTROY_LIST(ban_list, next, temp);
    }

    for (int i = 0; i < num_invalid; i++)
        str_free(invalid_list[i]);

    for (int i = 0; i < num_valid; i++)
        str_free(valid_list[i]);

    for (int i = 0; i < num_curses; i++)
        str_free(curses_list[i]);
}

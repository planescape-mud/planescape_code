/* ************************************************************************
*   File: act.social.c                                  Part of CircleMUD *
*  Usage: Functions to handle socials                                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "wrapperbase.h"
#include "register-impl.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "act.social.h"
#include "db.h"
#include "spells.h"
#include "planescape.h"
#include "dlfileop.h"
#include "mudfile.h"


/* local functions */
int find_action(char *cmd);

int top_of_socialm = -1;
int top_of_socialk = -1;
int top_of_adverb = -1;
struct adverb_list_data *adverb_list = NULL;

struct social_messg *soc_mess_list = NULL;
struct social_keyword *soc_keys_list = NULL;


const char *get_soc_keyword(int act_nr)
{
    int top = top_of_socialk, i;

    for (i = 0; i < top; i++)
        if (soc_keys_list[i].social_message == act_nr)
            return soc_keys_list[i].keyword;

    return ("not found");
}

ACMD(do_narlist)
{
    int j, no;
    char buf[MAX_EXTEND_LENGTH];

    sprintf(buf, "Вам доступны следующие наречия:\r\n");
    for (j = 0, no = 1; j < top_of_adverb; j++)
        if (*adverb_list[j].adverb != '!') {
            sprintf(buf + strlen(buf), "%-19s", adverb_list[j].adverb);
            if (!(no % 4))
                strcat(buf, "\r\n");
            no++;
        }
    strcat(buf, "\r\n");
    page_string(ch->desc, buf, 1);
}

int find_action(char *cmd)
{
    int bot, top, mid, len, chk;

    bot = 0;
    top = top_of_socialk;
    len = strlen(cmd);

    if (top < 0 || !len)
        return (-1);

    for (;;) {
        mid = (bot + top) / 2;

        if (bot > top)
            return (-1);
        if (!(chk = strn_cmp(cmd, soc_keys_list[mid].keyword, len))) {
            while (mid > 0 && !strn_cmp(cmd, soc_keys_list[mid - 1].keyword, len))
                mid--;
            return (soc_keys_list[mid].social_message);
        }

        if (chk > 0)
            bot = mid + 1;
        else
            top = mid - 1;
    }
}

char *find_nar(char *nar, struct social_messg *action)
{
    int j, len;

    len = strlen(nar);

    if (!(*nar)) {
        if (action->nars)
            return (action->nars);
        else
            return 0; // prool
    } else {
        for (j = 0; j < top_of_adverb; j++)
            if (!strn_cmp(nar, adverb_list[j].adverb, len))
                return adverb_list[j].adverb;
    }

    return 0; // prool
}

int find_nar1(char *nar, struct social_messg *action)
{
    int j, len;

    len = strlen(nar);

    for (j = 0; j < top_of_adverb; j++)
        if (!strn_cmp(nar, adverb_list[j].adverb, len))
            return j;

    return -1;
}

// Invoke Fenia social triggers for one of the observers.
void mprog_social(struct char_data *ch, struct char_data *actor, struct char_data *vict, const char *social)
{
    FENIA_VOID_CALL( ch, "Social", "CCs", actor, vict, social );
    FENIA_PROTO_VOID_CALL( ch->npc(), "Social", "CCCs", ch, actor, vict, social );

}    

// Invoke social trigger for everyone in the room.
void mprog(struct char_data *actor, struct char_data *vict, const char *social)
{
    struct char_data *rch;

    for (rch = world[actor->in_room].people; rch; rch = rch->next_in_room)
        mprog_social(rch, actor, vict, social);
}

int do_social(struct char_data *ch, char *argument)
{
    int act_nr;
    char social[MAX_INPUT_LENGTH], *nar;
    const char *social_keyword;
    struct social_messg *action;
    struct char_data *vict;
    struct obj_data *fobj;
    char buf[MAX_STRING_LENGTH];

    if (!argument || !*argument)
        return (FALSE);

    argument = one_argument(argument, social);

//социал не найден
    if ((act_nr = find_action(social)) < 0)
        return (FALSE);

    action = &soc_mess_list[act_nr];

    if (AFF_FLAGGED(ch, AFF_SIELENCE) || PLR_FLAGGED(ch, PLR_DUMB)) {
        send_to_charf(ch, "Вы не можете выразить свои эмоции.\r\n");
        return (FALSE);
    }

    if (GET_POS(ch) < action->ch_min_pos || GET_POS(ch) > action->ch_max_pos) {
        send_to_char("В таком положении не удобно это делать.\r\n", ch);
        return (TRUE);
    }

    if (action->char_found && argument)
        argument = one_argument(argument, buf);
    else
        *buf = '\0';

    nar = find_nar(buf, action);
    social_keyword = get_soc_keyword(act_nr);

    // просто ввели социал без параметров и нет наречия
    if ((!*buf || !strcmp(buf, ".")) && !nar) {
        act(action->char_no_arg, FALSE, ch, 0, 0, TO_CHAR);
        act(action->others_no_arg, FALSE, ch, 0, 0, TO_ROOM);
        mprog(ch, NULL, social_keyword);
        return (TRUE);
    }

    generic_find(buf, FIND_CHAR_ROOM, ch, &vict, &fobj);

    if (!vict) {
        if (nar) {
            sprintf(buf, action->char_no_arg, nar);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
            sprintf(buf, action->others_no_arg, nar);
            act(buf, FALSE, ch, 0, 0, TO_ROOM);
        } else {
            send_to_char(action->not_found ? action->not_found :
                         "Поищите кого-нибодь более доступного для этих целей.\r\n", ch);
            send_to_char("\r\n", ch);
            return (TRUE);
        }
    } else if (vict == ch) {
        argument = one_argument(argument, buf);
        nar = find_nar(buf, action);
        //log ("НАРЕЧИЕ[2] %s",nar);
        if (nar) {
            sprintf(buf, action->char_no_arg, nar);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
            sprintf(buf, action->others_no_arg, nar);
            act(buf, FALSE, ch, 0, 0, TO_ROOM);
        } else {
            act(action->char_no_arg, FALSE, ch, 0, 0, TO_CHAR);
            act(action->others_no_arg, FALSE, ch, 0, 0, TO_ROOM);
        }
    } else {
        if (GET_POS(vict) < action->vict_min_pos || GET_POS(vict) > action->vict_max_pos) {
            act("$N2 сейчас, похоже, не до Вас.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
            return (TRUE);
        }
        else {
            argument = one_argument(argument, buf);
            nar = find_nar(buf, action);
            //log ("НАРЕЧИЕ[3] %s",nar);

            //Триггер на социал

            if (nar) {
                sprintf(buf, action->char_found, nar);
                act(buf, 0, ch, 0, vict, TO_CHAR | TO_SLEEP);
                sprintf(buf, action->others_found, nar);
                act(buf, FALSE, ch, 0, vict, TO_NOTVICT);
                sprintf(buf, action->vict_found, nar);
                act(buf, FALSE, ch, 0, vict, TO_VICT);
            } else {
                act(action->char_found, 0, ch, 0, vict, TO_CHAR | TO_SLEEP);
                act(action->others_found, FALSE, ch, 0, vict, TO_NOTVICT);
                act(action->vict_found, FALSE, ch, 0, vict, TO_VICT);
            }
        }
    }

    mprog(ch, vict, social_keyword);
    return (TRUE);
}



char *str_dup_bl(const char *source)
{
    char line[MAX_INPUT_LENGTH];

    line[0] = 0;
    if (source[0])
        strcat(line, source);

    return (str_dup(line));
}

void load_socials(FILE * fl)
{
    char line[MAX_INPUT_LENGTH], *scan, next_key[MAX_INPUT_LENGTH];
    int key = -1, message = -1, c_min_pos, c_max_pos, v_min_pos, v_max_pos, what;

    /* get the first keyword line */
    get_one_line(fl, line);
    while (*line != '$') {
        message++;
        scan = one_word(line, next_key);
        while (*next_key) {
            key++;
            //log("СОЦИАЛ: %d '%s' - message %d",key,next_key,message);
            soc_keys_list[key].keyword = str_dup(next_key);
            soc_keys_list[key].social_message = message;
            scan = one_word(scan, next_key);
        }
        get_one_line(fl, line);
        //грузим дефолтное наречие для социала
        if (*line == '/') {
            scan = one_word(line + 1, next_key);
            soc_mess_list[message].nars = str_dup(next_key);
            get_one_line(fl, line);
        }

        what = 0;

        while (*line != '#') {
            scan = line;
            skip_spaces(&scan);
            if (scan && *scan && *scan != ';') {
                switch (what) {
                    case 0:
                        if (sscanf
                            (scan, " %d %d %d %d \n", &c_min_pos, &c_max_pos, &v_min_pos,
                             &v_max_pos) < 4) {
                            log("SYSERR: format error in %d social file near social '%s' #d #d #d #d\n", message, line);
                            exit(1);
                        }
                        soc_mess_list[message].ch_min_pos = c_min_pos;
                        soc_mess_list[message].ch_max_pos = c_max_pos;
                        soc_mess_list[message].vict_min_pos = v_min_pos;
                        soc_mess_list[message].vict_max_pos = v_max_pos;
                        break;

                    case 1:
                        soc_mess_list[message].char_no_arg = str_dup_bl(scan);
                        break;

                    case 2:
                        soc_mess_list[message].others_no_arg = str_dup_bl(scan);
                        break;

                    case 3:
                        soc_mess_list[message].char_found = str_dup_bl(scan);
                        break;

                    case 4:
                        soc_mess_list[message].others_found = str_dup_bl(scan);
                        break;

                    case 5:
                        soc_mess_list[message].vict_found = str_dup_bl(scan);
                        break;

                    case 6:
                        soc_mess_list[message].not_found = str_dup_bl(scan);
                        break;
                }               //switch
            }                   //if
            if (!scan || *scan != ';')
                what++;
            get_one_line(fl, line);
        }                       //while
        /* get next keyword line (or $) */
        get_one_line(fl, line);

        /*log("\r\nСОЦИАЛ: %s\r\n"
           "наречие: %s\r\n"
           "char_no_arg: %s\r\n"
           "others_no_arg: %s\r\n"
           "#"
           ,
           soc_keys_list[key].keyword,
           soc_mess_list[message].nars,
           soc_mess_list[message].char_no_arg,
           soc_mess_list[message].others_no_arg); */

    }                           //while
}

char *fread_action(FILE * fl, int nr)
{
    char buf[MAX_STRING_LENGTH];

    fgets(buf, MAX_STRING_LENGTH, fl);
    if (feof(fl)) {
        log("SYSERR: fread_action: unexpected EOF near action #%d", nr);
        exit(1);
    }
    if (*buf == '#')
        return (NULL);

    buf[strlen(buf) - 1] = '\0';
    return (str_dup(buf));
}

//ADDED BY HMEPAS
//Функция возвращает наречиие по его номеру
char *get_nar(int num)
{
    if (num > 0)
        return adverb_list[num].adverb;
    log("SYSERR: function get_nar must have arg > 0!");
    return 0;
}


int count_adverb(void)
{
    char line[256];
    DLFileRead file(ShareFile(mud->adverbFile));

    if (!file.open()) {
        log("ОШИБКА: Открытие файла наречий: %s", file.getCPath());
        return top_of_adverb;
    }

    do {
        get_one_line(file.getFP(), line);
        top_of_adverb++;
        if (file.eof()) {
            log("ОШИБКА: Нет указателя ($) конца файла в наречиях");
            break;
        }
    } while (*line != '$');

    return top_of_adverb;
}


void load_adverb(void)
{
    DLFileRead file(ShareFile(mud->adverbFile));
    char line[256];
    int num = -1;

    if (count_adverb() <= 0)
        return;

    CREATE(adverb_list, adverb_list_data, top_of_adverb);

    if (!file.open()) {
        log("ОШИБКА: Открытие файла наречий: %s", file.getCPath());
        return;
    }

    get_one_line(file.getFP(), line);
    do {
        num++;
        if (num > top_of_adverb) {
            log("ОШИБКА: Количество наречий больше чем планировалось.");
            return;
        }
        adverb_list[num].adverb = str_dup(line);
        get_one_line(file.getFP(), line);
    } while (*line != '$' && !file.eof());
}

void socials_init()
{
    /* before world loading */
    log("Загружаю социалы.");
    index_boot(DB_BOOT_SOCIAL);
}

void socials_destroy()
{
    if (soc_mess_list) {
        for (int i = 0; i <= top_of_socialm; i++) {
            if (soc_mess_list[i].char_no_arg)
                free(soc_mess_list[i].char_no_arg);
            if (soc_mess_list[i].others_no_arg)
                free(soc_mess_list[i].others_no_arg);
            if (soc_mess_list[i].char_found)
                free(soc_mess_list[i].char_found);
            if (soc_mess_list[i].others_found)
                free(soc_mess_list[i].others_found);
            if (soc_mess_list[i].vict_found)
                free(soc_mess_list[i].vict_found);
            if (soc_mess_list[i].not_found)
                free(soc_mess_list[i].not_found);
        }
        free(soc_mess_list);
    }
    if (soc_keys_list) {
        for (int i = 0; i <= top_of_socialk; i++)
            if (soc_keys_list[i].keyword)
                free(soc_keys_list[i].keyword);
        free(soc_keys_list);
    }
    top_of_socialm = -1;
    top_of_socialk = -1;
}

void adverb_init()
{
    /* before world loading */
    log("Загружаю наречия.");
    load_adverb();
}

void adverb_destroy()
{
    if (adverb_list) {
        for (int i = 0; i < top_of_adverb; i++)
            if (adverb_list[i].adverb)
                free(adverb_list[i].adverb);
        free(adverb_list);
    }
    top_of_adverb = -1;
}

/* ************************************************************************
*   File: interpreter.c                                 Part of CircleMUD *
*  Usage: parse user commands, search for specials, call ACMD functions   *
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
#include "comm.h"
#include "ban.h"
#include "interpreter.h"
#include "constants.h"
#include "db.h"
#include "utils.h"
#include "spells.h"
#include "handler.h"
#include "act.social.h"
#include "house.h"
#include "mail.h"
#include "screen.h"
#include "pk.h"
#include "commands.h"
#include "xquests.h"
#include "xboot.h"
#include "events.h"
#include "registration.h"
#include "planescape.h"
#include "mudstats.h"
#include "textfileloader.h"

#define multi_play_limit 1

/* local functions */
void do_entergame_(struct descriptor_data *d, int load_room);
void rand_error(struct char_data *ch);
int perform_dupe_check(struct descriptor_data *d);


#define MAX_ERRORS 6

struct error_type {
    const char *text;
};

const struct error_type mistype_table[MAX_ERRORS] = {
    {"Что?"},
    {"А?"},
    {"Хмм."},
    {"НК (Неопознанная Команда)"},      /* revived from DikuMUD! */
    {"Что, что?"},
    {"Хмм!?"}
};

const char *create_name_rules =
    "\r\n"
    "   ПРАВИЛА ИМЕНОВАНИЯ ИГРОВЫХ ПЕРСОНАЖЕЙ\r\n"
    "------------------------------------------\r\n"
    "1. Имя должно быть созвучным и соответствовать фэнтази стилю.\r\n"
    "2. Имя не должно содержать заглавные буквы. (пример: ДартВейдер)\r\n"
    "3. В имя не включаются титулы. (пример: СэрКайл)\r\n"
    "4. Имя не должно быть набором случайных букв. (пример: йцукен)\r\n"
    "5. Имя не должно состоянить из 2х и более имен. (пример: ДжонЛео)\r\n"
    "6. Имя не должно обьединять два и более слова. (пример: красныймеч)\r\n"
    "7. Имя не должно быть именем на *любом* языке. (пример: Энтони)\r\n"
    "8. Имя не должно быть инверсией любого слова. (пример: апож)\r\n"
    "9. Нельзя использовать имена героев фильмов, комиксов и т.п. (пример: Чапаев)\r\n"
    "10.Имя не должно быть заимствовано из мифологии или литературы. (пример: Зевс)\r\n";


const char *fills[] = { "in",
    "from",
    "with",
    "the",
    "on",
    "at",
    "to",
    "\n"
};

const char *reserved[] = { "a",
    "an",
    "self",
    "me",
    "all",
    "room",
    "someone",
    "something",
    "\n"
};



void rand_error(struct char_data *ch)
{
    /*char buf[MAX_STRING_LENGTH];
       int n;

       n = number(1, MAX_ERRORS) - 1;

       sprintf ( buf, "%s\n\r", mistype_table[n].text);
       send_to_char ( buf, ch ); */

    send_to_charf(ch, "Хмм...\r\n");
    return;
}

/*
  проверка команды на прячущие аффекты игрока percent =
  вход:
   -3: снимаються все аффекты
   -2: проверка умения sneak
   -1: снимаются аффекты hide
    0: проверка умения hide
    1: нет проверки умениям
   выход:
   -2: заметили, но не сообщать об этом и не снимать видимость
   -1: заметили но не сообщать об этом
    0: заметили
    1: остались незамечеными
*/

int check_hiding_cmd(struct char_data *ch, int percent)
{
    if (percent == 1)
        return 1;

    if (percent == -3) {
        if (affected_by_spell(ch, SPELL_HIDE))
            affect_from_char(ch, SPELL_HIDE);
        if (affected_by_spell(ch, SPELL_SNEAK))
            affect_from_char(ch, SPELL_SNEAK);
        return -1;
    }

    if (percent == -2) {
        if (affected_by_spell(ch, SPELL_SNEAK)) {
            affect_from_char(ch, SPELL_SNEAK);
            return -2;
        }
        if (affected_by_spell(ch, SPELL_HIDE)) {
            affect_from_char(ch, SPELL_HIDE);
            return 0;
        }
    }

    if (percent == -1) {
        if (affected_by_spell(ch, SPELL_HIDE)) {
            affect_from_char(ch, SPELL_HIDE);
            return 0;
        }
        if (affected_by_spell(ch, SPELL_SNEAK)) {
            affect_from_char(ch, SPELL_SNEAK);
            return -1;
        }
    }

    if (!percent) {
        if (affected_by_spell(ch, SPELL_HIDE)) {
            int prob;
            std::vector < int >vit;
            std::vector < int >vot;

            vit.push_back(GET_REAL_DEX(ch));
            vit.push_back(GET_REAL_INT(ch));
            vot.push_back(number(1, 40));
            prob = calc_like_skill(ch, NULL, SKILL_HIDE, vit, vot);
            if (number(1, 100) > prob) {
                affect_from_char(ch, SPELL_HIDE);
                return 0;
            }
        }
        return 1;
    }
    return 1;
}

void send_hide_mess(struct char_data *ch, int i)
{
    if (!i) {
        act("Вы прекратили прятаться.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n прекратил$g прятаться.", TRUE, ch, 0, 0, TO_ROOM);
    }
    if (i == 0 || i == -1)
        ch->visible_by.clear();
}


// Hook for commands defined in other plugins. Quick solution to 
// add commands such as 'cs', 'eval' etc.
bool(*plugin_command_hook) (struct char_data *, char *, char *) = 0;

/*
 * Invoke Fenia triggers for command interpreter.
 * onCommand:
 *      Called on every character in the room if a command is performed near them. 
 *      Returning true will interrupt normal command processing.
 * Adequate position for executing the command needs to be checked inside the trigger.
 */
static bool mprog_command(struct char_data *actor, const char *cmd, const char *line)
{
    struct char_data *rch, *rch_next;

    for (rch = world[IN_ROOM(actor)].people; rch; rch = rch_next) {
        rch_next = rch->next_in_room;
        FENIA_BOOL_CALL(rch, "Command", "Css", actor, cmd, line);
        FENIA_PROTO_BOOL_CALL(rch->npc(), "Command", "CCss", rch, actor, cmd, line); 
    }

    return false;
}

/*
 * This is the actual command interpreter called from game_loop() in comm.c
 * It makes sure you are the proper level and position to execute the command,
 * then calls the appropriate function.
 */
void command_interpreter(struct char_data *ch, char *cargument)
{
    int cmd, length, social = FALSE, hardcopy = FALSE, i = 1, count = 1;
    char *line, *line_2, *line_old;
    FILE *saved;
    char filename[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    /* make a local copy for the argument, with bigger buffer size. */
    char *argument;
    char argument_buf[MAX_STRING_LENGTH];

    strcpy(argument_buf, cargument);
    argument = argument_buf;

    /* just drop to next line for hitting CR */
    skip_spaces(&argument);

    if (!*argument)
        return;
    if (!IS_NPC(ch))
        log("<%6d> %s [%s]", GET_ROOM_VNUM(IN_ROOM(ch)), GET_NAME(ch), argument);

    if (PRF_FLAGGED(ch, PRF_LOGGING)) {

        /*if (!IS_IMPL(ch))
           {
           sprintf(buf,"%s: %s",GET_NAME(ch),argument);
           mudlog(buf, CMP, LVL_IMPL, TRUE);
           } */

        get_filename(GET_NAME(ch), filename, LOGGER_FILE);
        if (!(saved = fopen(filename, "a"))) {
            log("Не открывается на запись файл лога для %s.", GET_NAME(ch));
        } else {
            fprintf(saved, "[%d] %s\n", GET_ROOM_VNUM(IN_ROOM(ch)), argument);
        }
        fclose(saved);
    }


    /*
     * special case to handle one-character, non-alphanumeric commands;
     * requested by many people so "'hi" or ";godnet test" is possible.
     * Patch sent by Eric Green and Stefan Wasilewski.
     */
    if (!a_isalpha(*argument)) {
        arg[0] = argument[0];
        arg[1] = '\0';
        line = argument + 1;
    } else
        line = any_one_arg(argument, arg);


    /* otherwise, find the command */
    if ((length = strlen(arg)) && length > 1 && *(arg + length - 1) == '!') {
        hardcopy = TRUE;
        *(arg + (--length)) = '\0';
        *(argument + length) = ' ';
    }

    if (is_positive_number(arg) && ch->player.current_quest_mob && ch->player.select_mode) {
        go_show_quest(ch, ch->player.current_quest_mob, atoi(arg), 0);
        return;
    }

    // Call Fenia triggers for any user input.
    if ((!GET_MOB_HOLD(ch) && !AFF_FLAGGED(ch, AFF_STOPFIGHT) && !AFF_FLAGGED(ch, AFF_STUNE))
           || GET_COMMSTATE(ch)) 
    {
        int cont = 0;       /* continue the command checks */

        cont = mprog_command(ch, arg, line);

        if (cont) {
            i = check_hiding_cmd(ch, -1);
            send_hide_mess(ch, i);
            return;         // command trigger took over
        }
    }

    bool fnd = FALSE;

    for (cmd = 0; *cmd_info[cmd].command != '\n'; cmd++)
        if (hardcopy) {
            if (!strcmp(cmd_info[cmd].command, arg) || !strcmp(cmd_info[cmd].command_alias, arg)) {
                if (GET_LEVEL(ch) >= cmd_info[cmd].minimum_level || GET_COMMSTATE(ch)) {
                    fnd = TRUE;
                    break;
                }
            }
        } else {
            if (!strncmp(cmd_info[cmd].command, arg, length) ||
                !strncmp(cmd_info[cmd].command_alias, arg, length)) {
                if (GET_LEVEL(ch) >= cmd_info[cmd].minimum_level || GET_COMMSTATE(ch)) {
                    fnd = TRUE;
                    break;
                }
            }
        }

    if ((PLR_FLAGGED(ch, PLR_SOUL)) && !cmd_info[cmd].hold) {
        send_to_charf(ch, "У Вас нет тела, которое могло бы выполнить данное действие!\r\n");
        return;
    }

    if (*cmd_info[cmd].command == '\n') {
        if (find_action(arg) >= 0)
            social = TRUE;
        else {
            if (!plugin_command_hook || !plugin_command_hook(ch, arg, line))
                rand_error(ch);
            return;
        }
    }

    if (!cmd_info[cmd].hold)
        stop_defence(ch);

    if (!IS_NPC(ch) && !IS_GRGOD(ch) && PLR_FLAGGED(ch, PLR_FROZEN) && !cmd_info[cmd].hold) {
        send_to_char("Сковавший Ваше тело лед мешает это сделать.\r\n", ch);
        return;
    }

    if (!PRF_FLAGGED(ch, PRF_NOHASSLE) && GET_MOB_HOLD(ch) > 0 && !cmd_info[cmd].hold) {
        send_to_char("Ваше тело парализовано, Вы не можете ничего сделать.\r\n", ch);
        return;
    }

    if (!PRF_FLAGGED(ch, PRF_NOHASSLE) && AFF_FLAGGED(ch, AFF_STUNE) && !cmd_info[cmd].hold) {
        send_to_char("Вы контужены настолько, что ничего не можете сделать.\r\n", ch);
        return;
    }

    if (!social && cmd_info[cmd].command_pointer == NULL) {
        send_to_char("Извините, не смог разобрать команду.\r\n", ch);
        return;
    }

    if (!social && cmd_info[cmd].minimum_level < 0 &&
        (!IS_MOB(ch) || (IS_MOB(ch) && AFF_FLAGGED(ch, AFF_CHARM)))) {
        rand_error(ch);
        return;
    }

    if (!social && IS_NPC(ch) && cmd_info[cmd].minimum_level >= LVL_IMMORT) {
        send_to_char("Эта команда только для Богов.\r\n", ch);
        return;
    }

    if (!social && GET_POS(ch) < cmd_info[cmd].minimum_position) {
        switch (GET_POS(ch)) {
            case POS_DEAD:
                send_to_char("Вы мертвы, а трупы не могут что-либо делать.\r\n", ch);
                break;
            case POS_INCAP:
            case POS_MORTALLYW:
                send_to_char("Вы скоро умрете, Вам не до действия!\r\n", ch);
                break;
            case POS_STUNNED:
                send_to_char("Вы не в состоянии что-либо делать!\r\n", ch);
                break;
            case POS_SLEEPING:
                send_to_char("Для этого следует проснуться.\r\n", ch);
                break;
            case POS_RESTING:
                send_to_char("Вы слишком расслаблены.\r\n", ch);
                break;
            case POS_SITTING:
                send_to_char("Вам следует встать на ноги.\r\n", ch);
                log("c3.1");
                break;
            case POS_FIGHTING:
                send_to_char("Вы сражаетесь за свою жизнь!\r\n", ch);
                break;
        }
        return;
    }

    stop_events(ch, cmd_info[cmd].hold);

    if (cmd_info[cmd].unhide_percent < 1 && (ch->dir_pick != -1 || ch->obj_pick)) {
        send_to_char("Вы прекратили свои попытки взломать замок.\r\n", ch);
        if (ch->dir_pick != -1) {
            if (!EXIT(ch, ch->dir_pick))
                send_to_charf(ch, "ОШИБКА: #int023 параметры %d %d (сообщите имморталам)\r\n",
                              world[ch->in_room].number, ch->dir_pick);
            else
                EXIT(ch, ch->dir_pick)->lock_step = 0;
        }
        if (ch->obj_pick)
            ch->obj_pick->lock_step = 0;
        ch->dir_pick = -1;
        ch->obj_pick = NULL;
    }


    if (social) {
        i = check_hiding_cmd(ch, -1);
        send_hide_mess(ch, i);
        do_social(ch, argument);
    } else if (mud->modNoSpecials || !special(ch, cmd, line)) {

        line_old = strdup(line);
        if ((line_2 = strchr(line, '*')) != NULL) {
            *(line_2++) = '\0';
            while (a_isspace(*line_2))
                line_2++;

            count = atoi(line);
            if (count < 1)
                line_2 = line_old;
        } else
            line_2 = line;

        if (count < 1)
            count = 1;

        i = check_hiding_cmd(ch, cmd_info[cmd].unhide_percent);
        send_hide_mess(ch, i);
        (*cmd_info[cmd].command_pointer) (ch, line_2, cmd, cmd_info[cmd].subcmd, count);


        if (line_old)
            free(line_old);
    }
}



/***************************************************************************
 * Various other parsing utilities                                         *
 **************************************************************************/

/*
 * searches an array of strings for a target string.  "exact" can be
 * 0 or non-0, depending on whether or not the match must be exact for
 * it to be returned.  Returns -1 if not found; 0..n otherwise.  Array
 * must be terminated with a '\n' so it knows to stop searching.
 */
int search_block(char *arg, const char **list, int exact)
{
    register int i, l;

    /* Make into lower case, and get length of string */
    for (l = 0; *(arg + l); l++)
        *(arg + l) = LOWER(*(arg + l));

    if (exact) {
        for (i = 0; **(list + i) != '\n'; i++)
            if (!str_cmp(arg, *(list + i)))
                return (i);
    } else {
        if (!l)
            l = 1;              /* Avoid "" to match the first available
                                 * string */
        for (i = 0; **(list + i) != '\n'; i++)
            if (!strn_cmp(arg, *(list + i), l))
                return (i);
    }

    return (-1);
}


/*
 * Function to skip over the leading spaces of a string.
 */
void skip_spaces(char **string)
{
    for (; **string && a_isspace(**string); (*string)++);
}

/*
 * Given a string, change all instances of double dollar signs ($$) to
 * single dollar signs ($).  When strings come in, all $'s are changed
 * to $$'s to avoid having users be able to crash the system if the
 * inputted string is eventually sent to act().  If you are using user
 * input to produce screen output AND YOU ARE SURE IT WILL NOT BE SENT
 * THROUGH THE act() FUNCTION (i.e., do_gecho, do_title, but NOT do_say),
 * you can call delete_doubledollar() to make the output look correct.
 *
 * Modifies the string in-place.
 */
char *delete_doubledollar(char *string)
{
    char *read, *write;

    /* If the string has no dollar signs, return immediately */
    if ((write = strchr(string, '$')) == NULL)
        return (string);

    /* Start from the location of the first dollar sign */
    read = write;


    while (*read)               /* Until we reach the end of the string... */
        if ((*(write++) = *(read++)) == '$')    /* copy one char */
            if (*read == '$')
                read++;         /* skip if we saw 2 $'s in a row */

    *write = '\0';

    return (string);
}


int fill_word(char *argument)
{
    return (search_block(argument, fills, TRUE) >= 0);
}


int reserved_word(char *argument)
{
    return (search_block(argument, reserved, TRUE) >= 0);
}


/*
 * copy the first non-fill-word, space-delimited argument of 'argument'
 * to 'first_arg'; return a pointer to the remainder of the string.
 */
char *one_argument(char *argument, char *first_arg)
{
    char *begin = first_arg;

    if (!argument) {
        *first_arg = '\0';
        return (NULL);
    }

    do {
        skip_spaces(&argument);

        first_arg = begin;
        while (*argument && !a_isspace(*argument)) {
            *(first_arg++) = LOWER(*argument);
            argument++;
        }

        *first_arg = '\0';
    } while (fill_word(begin));

    return (argument);
}


/*
 * one_word is like one_argument, except that words in quotes ("") are
 * considered one word.
 */
char *one_word(char *argument, char *first_arg)
{
    char *begin = first_arg;

    do {
        skip_spaces(&argument);
        first_arg = begin;

        if (*argument == '"') {
            argument++;
            while (*argument && *argument != '"') {
                *(first_arg++) = LOWER(*argument);
                argument++;
            }
            argument++;
        } else {
            while (*argument && !a_isspace(*argument)) {
                *(first_arg++) = LOWER(*argument);
                argument++;
            }
        }
        *first_arg = '\0';
    } while (fill_word(begin));

    return (argument);
}


/* same as one_argument except that it doesn't ignore fill words */
char *any_one_arg(char *argument, char *first_arg)
{
    skip_spaces(&argument);

    while (*argument && !a_isspace(*argument)) {
        *(first_arg++) = LOWER(*argument);
        argument++;
    }

    *first_arg = '\0';

    return (argument);
}


/*
 * Same as one_argument except that it takes two args and returns the rest;
 * ignores fill words
 */
char *two_arguments(char *argument, char *first_arg, char *second_arg)
{
    return (one_argument(one_argument(argument, first_arg), second_arg));       /*:-) */
}



/*
 * determine if a given string is an abbreviation of another
 * (now works symmetrically -- JE 7/25/94)
 *
 * that was dumb.  it shouldn't be symmetrical.  JE 5/1/95
 *
 * returnss 1 if arg1 is an abbreviation of arg2
 */
int is_abbrev(const char *arg1, const char *arg2)
{
    if (!*arg1)
        return (0);

    for (; *arg1 && *arg2; arg1++, arg2++)
        if (LOWER(*arg1) != LOWER(*arg2))
            return (0);

    if (!*arg1)
        return (1);
    else
        return (0);
}



/* return first space-delimited token in arg1; remainder of string in arg2 */
void half_chop(char *string, char *arg1, char *arg2)
{
    char *temp;

    temp = any_one_arg(string, arg1);
    skip_spaces(&temp);
    strcpy(arg2, temp);
}



/* Used in specprocs, mostly.  (Exactly) matches "command" to cmd number */
int find_command(const char *command)
{
    int cmd;

    for (cmd = 0; *cmd_info[cmd].command != '\n'; cmd++) {
        if (!strcmp(cmd_info[cmd].command, command) ||
            !strcmp(cmd_info[cmd].command_alias, command))
            return (cmd);
    }
    return (-1);
}


int special(struct char_data *ch, int cmd, char *arg)
{
    register struct obj_data *i;
    register struct char_data *k;
    int j, n;

    /* special in room? */
    if (GET_ROOM_SPEC(ch->in_room) != NULL)
        if (GET_ROOM_SPEC(ch->in_room) (ch, world + ch->in_room, cmd, arg)) {
            n = check_hiding_cmd(ch, cmd_info[cmd].unhide_percent);
            send_hide_mess(ch, n);
            return (1);
        }

    /* special in equipment list? */
    for (j = 0; j < NUM_WEARS; j++)
        if (GET_EQ(ch, j) && GET_OBJ_SPEC(GET_EQ(ch, j)) != NULL)
            if (GET_OBJ_SPEC(GET_EQ(ch, j)) (ch, GET_EQ(ch, j), cmd, arg)) {
                n = check_hiding_cmd(ch, cmd_info[cmd].unhide_percent);
                send_hide_mess(ch, n);
                return (1);
            }

    /* special in inventory? */
    for (i = ch->carrying; i; i = i->next_content)
        if (GET_OBJ_SPEC(i) != NULL)
            if (GET_OBJ_SPEC(i) (ch, i, cmd, arg)) {
                n = check_hiding_cmd(ch, cmd_info[cmd].unhide_percent);
                send_hide_mess(ch, n);
                return (1);
            }

    /* special in mobile present? */
    for (k = world[ch->in_room].people; k; k = k->next_in_room)
        if (GET_MOB_SPEC(k) != NULL)
            if (GET_MOB_SPEC(k) (ch, k, cmd, arg)) {
                n = check_hiding_cmd(ch, cmd_info[cmd].unhide_percent);
                send_hide_mess(ch, n);
                return (1);
            }

    /* special in object present? */
    for (i = world[ch->in_room].contents; i; i = i->next_content)
        if (GET_OBJ_SPEC(i) != NULL)
            if (GET_OBJ_SPEC(i) (ch, i, cmd, arg)) {
                n = check_hiding_cmd(ch, cmd_info[cmd].unhide_percent);
                send_hide_mess(ch, n);
                return (1);
            }

    return (0);
}



/* *************************************************************************
*  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
************************************************************************* */


/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
    int i;

    for (i = 0; i <= top_of_p_table; i++) {
        if (!str_cmp((player_table + i)->name, name))
            return (i);
    }

    return (-1);
}


int _parse_name(char *arg, char *name)
{
    int i;

    /* skip whitespaces */
    for (i = 0; (*name = (i ? LOWER(*arg) : UPPER(*arg))); arg++, i++, name++)
        if (!a_isalpha(*arg) || *arg > 0)
            return (1);

    if (!i)
        return (1);

    return (0);
}

#define RECON   1
#define USURP   2
#define UNSWITCH  3

/*
 * XXX: Make immortals 'return' instead of being disconnected when switched
 *      into person returns.  This function seems a bit over-extended too.
 */
int perform_dupe_check(struct descriptor_data *d)
{
    struct descriptor_data *k, *next_k;
    struct char_data *target = NULL, *ch, *next_ch;
    int mode = 0;
    char buf[MAX_STRING_LENGTH];

    int id = GET_IDNUM(d->character);

    /*
     * Now that this descriptor has successfully logged in, disconnect all
     * other descriptors controlling a character with the same ID number.
     */

    for (k = descriptor_list; k; k = next_k) {
        next_k = k->next;
        if (k == d)
            continue;

        if (k->original && (GET_IDNUM(k->original) == id)) {    /* switched char */
            if (str_cmp(d->host, k->host)) {
                sprintf(buf, "ПОВТОРНЫЙ ВХОД!!! ID = %ld Персонаж = %s Хост = %s(был %s)",
                        GET_IDNUM(d->character), GET_NAME(d->character), d->host, k->host);
                mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
                //send_to_gods(buf);
            }

            if (k->snooping) {
                del_snooper(k->character);
                k->snooping = NULL;
            }

            SEND_TO_Q("\r\nПопытка повторного входа - отключаемся.\r\n", k);
            STATE(k) = CON_CLOSE;
            if (!target) {
                target = k->original;
                mode = UNSWITCH;
            }
            if (k->character)
                k->character->desc = NULL;
            k->character = NULL;
            k->original = NULL;
        } else if (k->character && (GET_IDNUM(k->character) == id)) {
            if (str_cmp(d->host, k->host)) {
                sprintf(buf, "ПОВТОРНЫЙ ВХОД!!! ID = %ld Name = %s Host = %s(was %s)",
                        GET_IDNUM(d->character), GET_NAME(d->character), d->host, k->host);
                mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
                //send_to_gods(buf);
            }

            if (!target && STATE(k) == CON_PLAYING) {
                SEND_TO_Q("\r\nВаше тело уже кем-то занято!\r\n", k);
                target = k->character;
                mode = USURP;
            }

            if (k->snooping) {
                del_snooper(k->character);
                k->snooping = NULL;
            }

            k->character->desc = NULL;
            k->character = NULL;
            k->original = NULL;
            SEND_TO_Q("\r\nПопытка второго входа - отключаемся.\r\n", k);
            STATE(k) = CON_CLOSE;
        }
    }

    /*
     * now, go through the character list, deleting all characters that
     * are not already marked for deletion from the above step (i.e., in the
     * CON_HANGUP state), and have not already been selected as a target for
     * switching into.  In addition, if we haven't already found a target,
     * choose one if one is available (while still deleting the other
     * duplicates, though theoretically none should be able to exist).
     */

    for (ch = character_list; ch; ch = next_ch) {
        next_ch = ch->next;

        if (IS_NPC(ch))
            continue;
        if (GET_IDNUM(ch) != id)
            continue;

        /* ignore chars with descriptors (already handled by above step) */
        if (ch->desc)
            continue;

        /* don't extract the target char we've found one already */
        if (ch == target)
            continue;

        /* we don't already have a target and found a candidate for switching */
        if (!target) {
            target = ch;
            mode = RECON;
            continue;
        }

        /* we've found a duplicate - blow him away, dumping his eq in limbo. */
        if (ch->in_room != NOWHERE)
            char_from_room(ch);
        char_to_room(ch, STRANGE_ROOM);
        extract_char(ch, FALSE);
    }

    /* no target for swicthing into was found - allow login to continue */
    if (!target)
        return (0);

    /* Okay, we've found a target.  Connect d to target. */

    update_ptable_data(d->character);
    delete(d->character);       /* get rid of the old char */
    d->character = target;
    d->character->desc = d;
    d->original = NULL;
    d->character->char_specials.timer = 0;
    REMOVE_BIT(PLR_FLAGS(d->character, PLR_MAILING), PLR_MAILING);
    REMOVE_BIT(PLR_FLAGS(d->character, PLR_SCRIPTING), PLR_SCRIPTING);
    REMOVE_BIT(PLR_FLAGS(d->character, PLR_WRITING), PLR_WRITING);
    STATE(d) = CON_PLAYING;

    switch (mode) {
        case RECON:
            //toggle_compression(d);
            SEND_TO_Q("Вы восстановили соединение.\r\n", d);
            act("1+и приш1(ел,ла,ло,ли) в себя.", "Км", d->character);
            check_light(d->character, LIGHT_NO, LIGHT_NO, LIGHT_NO, LIGHT_NO, 1);
            sprintf(buf, "%s [%s] восстановил соединение.", GET_NAME(d->character), d->host);
            mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
            load_pets(d->character);
            mud->getStats()->update();
            break;
        case USURP:
            //toggle_compression(d);
            SEND_TO_Q("Вы снова в своем теле!\r\n", d);
            act("Тело $n1 было захвачено новым духом!", TRUE, d->character, 0, 0, TO_ROOM);
            sprintf(buf, "%s пересоединился. Старое соединение потеряно.", GET_NAME(d->character));
            mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
            //load_pets(d->character);
            mud->getStats()->update();
            break;
        case UNSWITCH:
            //toggle_compression(d);
            SEND_TO_Q("Пересоединяемся для перевключения игрока.", d);
            sprintf(buf, "%s [%s] has reconnected.", GET_NAME(d->character), d->host);
            mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
            load_pets(d->character);
            mud->getStats()->update();
            break;
    }

    return (1);
}

int pre_help(struct char_data *ch, char *arg)
{
    char command[MAX_INPUT_LENGTH], topic[MAX_INPUT_LENGTH];

    half_chop(arg, command, topic);

    if (!*command || strlen(command) < 2 || !*topic || strlen(topic) < 2)
        return (0);
    if (isname(command, "помощь") || isname(command, "help") || isname(command, "справка")) {
        do_help(ch, topic, 0, 0, 0);
        //SEND_TO_Q("Для продолжения нажмите ENTER.\r\n",ch->desc);
        return (1);
    }
    return (0);
}


void msg_return(struct char_data *ch, int load_room)
{
    if (world[load_room].hotel && world[load_room].hotel->MessReturn)
        act(world[load_room].hotel->MessReturn, "Км", ch);
    else
        act("$n вступил$g в игру.", TRUE, ch, 0, 0, TO_ROOM);
}

void do_entergame(struct descriptor_data *d)
{
    GET_BAD_PWS(d->character) = 0;
    d->bad_pws = 0;
    do_entergame_(d, -1);
    mud->getStats()->update();
}

void do_entergame_(struct descriptor_data *d, int load_room)
{
    int m_reboot = load_room;
    int neednews = 0, i;
    struct char_data *ch;
    char buf[MAX_STRING_LENGTH];

    read_aliases(d->character);

    check_stats(d->character);

    if ((i = get_ptable_by_name(GET_NAME(d->character))) != -1)
        player_table[i].last_logon = time(0);

    if (PLR_FLAGGED(d->character, PLR_INVSTART))
        d->character->pc()->specials.saved.invis_level = GET_LEVEL(d->character);

    /*
     * We have to place the character in a room before equipping them
     * or equip_char() will gripe about the person in NOWHERE.
     */

    //log("Player %s enter at room %d", GET_NAME(d->character), load_room);
    /* If char was saved with NOWHERE, or real_room above failed... */

    if (m_reboot == -1)
        send_to_char(WELC_MESSG, d->character);

    for (ch = character_list; ch; ch = ch->next)
        if (ch == d->character)
            break;

    if (!ch) {
        d->character->next = character_list;
        character_list = d->character;
    } else {
        REMOVE_BIT(MOB_FLAGS(ch, MOB_DELETE), MOB_DELETE);
        REMOVE_BIT(MOB_FLAGS(ch, MOB_FREE), MOB_FREE);
    }

    if (Is_Registry_Name(GET_NAME(d->character)))
        NAME_GOD(d->character) = 1035;
    else {
        NAME_GOD(d->character) = 0;
        GET_LOADROOM(d->character) = named_start_room;
    }

    if (GET_LOADROOM(d->character) == NOWHERE)
        GET_LOADROOM(d->character) = calc_loadroom(d->character);

    if (load_room == -1)
        load_room = real_room(GET_LOADROOM(d->character));
    else
        load_room = real_room(load_room);

    d->character->linkWrapper();

    char_to_room(d->character, load_room);

    /*  if (PRF_FLAGGED(d->character, PRF_NONEW) || GET_LEVEL(d->character))
       xload_rent(d->character,m_reboot); */

    if (LAST_LOGON(d->character) < mud->getTextFileLoader()->getModifyTime("news"))
        neednews = TRUE;

    /* сбрасываем телы для команды "вспомнить" */
    for (i = 0; i < MAX_REMEMBER_TELLS; i++)
        GET_TELL(d->character, i)[0] = '\0';
    GET_LASTTELL(d->character) = 0;

    /* with the copyover patch, this next line goes in enter_player_game() */
    GET_ID(d->character) = GET_IDNUM(d->character);
    update_ptable_data(d->character);


    if (m_reboot == -1) {
        msg_return(d->character, load_room);
    }

    /* with the copyover patch, this next line goes in enter_player_game() */
    load_quests(d->character);
    load_vars(d->character);

    //send_to_charf(d->character,"Последний раз Вы входили в игру: %s с ip-адерса: %s\r\n\r\n",ascii_time(LAST_LOGON(d->character)),
    //  GET_LASTIP(d->character));

    //strcpy(GET_LASTIP(d->character),d->host);
    LAST_LOGON(d->character) = time(0);


    STATE(d) = CON_PLAYING;
    if (!PRF_FLAGGED(d->character, PRF_NONEW) && !GET_LEVEL(d->character)) {
        do_start(d->character, TRUE);
        save_char(d->character, NOWHERE);
        save_player_index();
        send_to_char(START_MESSG, d->character);
        send_to_char("Воспользуйтесь командой НОВИЧОК для получения вводной информации игроку.\r\n",
                     d->character);
    }

    if (m_reboot == -1)
        sprintf(buf, "%s [%s] - вошел в игру - %d уровень.",
                GET_NAME(d->character), d->host, GET_LEVEL(d->character));
    else
        sprintf(buf, "%s [%s] вернулся с copyover.", GET_NAME(d->character), d->host);
    mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
    //send_to_gods(buf);

    if (m_reboot == -1)
        look_at_room(d->character, TRUE);


    if (PRF_FLAGGED(d->character, PRF_NONEW) || GET_LEVEL(d->character)) {
        if (m_reboot == -1)
            xload_rent(d->character, FALSE);
        else
            xload_rent(d->character, TRUE);
    }

    check_sets(d->character);
    affect_total(d->character);

    if (neednews) {
        send_to_charf(d->character, "%s\r\nВас ожидают игровые новости.%s\r\n",
                      CCWHT(d->character, C_NRM), CCNRM(d->character, C_NRM));
    }

    if (m_reboot == -1 && PRF_FLAGGED(d->character, PRF_TIPS)) {
        send_to_char("\r\n", d->character);
        send_to_char("Совет: ширину экрана можно установить командой: режим ширина\r\n",
                     d->character);
        send_to_char("Совет: полный цвет можно установить командой: режим цвет полный\r\n",
                     d->character);
    }

    if (has_mail(GET_IDNUM(d->character))) {
        sprintf(buf, "%sВам пришло письмо.%s\r\n",
                CCWHT(d->character, C_NRM), CCNRM(d->character, C_NRM));
        send_to_char(buf, d->character);
    }

    if (PRF_FLAGGED(d->character, PRF_EQ)) {
        do_newbie(d->character);
        TOGGLE_BIT(PRF_FLAGS(d->character, PRF_EQ), PRF_EQ);
    }

    /* Убрали компенсацию
       if (!PLR_FLAGGED(d->character,PLR_COMPS) &&
       (d->character->player.time.birth < time(0)-(7*SECS_PER_REAL_DAY)))
       {
       int level = GET_LEVEL(d->character);
       int summa = (int)(sqrt(level)*761.0*(float)level);

       send_to_charf(d->character,"\r\n&WВам выплачена компенсация за утерянные вещи.\r\n");
       send_to_charf(d->character,"Компенсация составила %d %s.\r\n",summa,desc_count(summa, WHAT_MONEYu));
       send_to_charf(d->character,"Деньги переведены на Ваш счет в банке.&n\r\n");

       GET_BANK_GOLD(d->character) += summa;

       SET_BIT(PLR_FLAGS(d->character, PLR_COMPS),PLR_COMPS);
       save_char(d->character, NOWHERE);
       } */

    if (m_reboot == -1) {
        send_to_char("\r\n", d->character);
        do_time(d->character, 0, 0, 0, 0);
    }
    // sprintf(buf,"NEWS %ld %ld(%s)",lastnews,LAST_LOGON(d->character),GET_NAME(d->character));
    // mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
    // sprintf(buf,"У Вас %s.\r\n",d->character->desc ? "есть дескриптор": "нет дескриптора");
    // send_to_char(buf,d->character);
    load_pets(d->character);


    /*if (GET_EXP(d->character) > get_levelexp(d->character,GET_LEVEL(d->character)+1,1)*2 ||
       GET_EXP(d->character) < get_levelexp(d->character,GET_LEVEL(d->character),1))
       {
       send_to_charf(d->character,"Количество Вашего опыта пересчитано.\r\n");
       GET_EXP(d->character) = level_exp(GET_LEVEL(d->character))+1;
       } */

    d->has_prompt = 0;
}


void DoAfterPassword(struct descriptor_data *d)
{
    int load_result;
    char buf[MAX_STRING_LENGTH];

    /* int  connections=0, limit = multi_play_limit;
       struct descriptor_data *k, *next_k; */


    /* Password was correct. */
    load_result = GET_BAD_PWS(d->character);
    GET_BAD_PWS(d->character) = 0;
    d->bad_pws = 0;

    if (isbanned(d->host) == BAN_SELECT && !PLR_FLAGGED(d->character, PLR_SITEOK)) {
        SEND_TO_Q("Извините, Вы не можете выбрать этого игрока с данного IP!\r\n", d);
        STATE(d) = CON_CLOSE;
        sprintf(buf, "Connection attempt for %s denied from %s", GET_NAME(d->character), d->host);
        mudlog(buf, NRM, LVL_HIGOD, TRUE);
        return;
    }

    if (GET_LEVEL(d->character) < mud->modRestrict) {
        SEND_TO_Q("Игра временно приостановлена.. Ждем Вас немного позже.\r\n", d);
        STATE(d) = CON_CLOSE;
        sprintf(buf, "Request for login denied for %s [%s] (wizlock)",
                GET_NAME(d->character), d->host);
        mudlog(buf, NRM, LVL_HIGOD, TRUE);
        return;
    }
    /* Проверка на мультинг */
    /*
       for (k = descriptor_list; k; k = next_k)
       {
       next_k = k->next;

       log ("Проверка на мультинг");

       if (k->character && d->character)
       if ((k->character == d->character) &&
       (STATE(d) != CON_PLAYING ||
       STATE(d) != CON_CLOSE   ||
       STATE(d) != CON_DISCONNECT))
       {
       if (!strcasecmp(k->host, d->host))
       {
       if (STATE(k) !=CON_CLOSE ||
       STATE(k) !=CON_DISCONNECT)  connections++;
       if (is_proxy(d->host)) limit = is_proxy(d->host);

       if (debugz) log("Прибавили conntect %d limit %d",connections,limit);
       if(connections > limit &&
       !PLR_FLAGGED(d->character, PLR_MULTI))
       {
       SEND_TO_Q("С Вашего IP адреса играет другой игрок.\r\n"
       "Если Вы играете из клуба попросите администратора клуба связатся с Богами.\r\n", d);
       STATE(d) = CON_CLOSE;
       sprintf(buf, "Попытка превысить лимит соединений [%s]", d->host);
       mudlog(buf, NRM, LVL_GOD, TRUE);
       return;
       }
       }
       }
       } */

    /* check and make sure no other copies of this player are logged in */
    if (perform_dupe_check(d))
        return;

// if (PRF_FLAGGED(d->character, PRF_AUTOZLIB))
    //toggle_compression(d);

// REM BY SLOWN 02.03.2002
    /* if (GET_LEVEL(d->character) >= LVL_IMMORT)
       SEND_TO_Q(imotd, d);
       else
       if (GET_HOUSE_UID(d->character)) {
       if (!House_news(d))
       SEND_TO_Q(motd, d);
       } else
       SEND_TO_Q(motd, d); */

    sprintf(buf, "%s [%s] has connected.", GET_NAME(d->character), d->host);
    mudlog(buf, BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);

    if (load_result) {
        sprintf(buf, "\r\n\r\n\007\007\007"
                " %sПОПЫТОК НЕСАНКЦИОНИРОВАННОГО ДОСТУПА: &R%d&r.%s\r\n",
                CCRED(d->character, C_SPR), load_result, CCNRM(d->character, C_SPR));
        SEND_TO_Q(buf, d);
        GET_BAD_PWS(d->character) = 0;
    }
    SEND_TO_Q(mud->getTextFileLoader()->get("motd").c_str(), d);
    SEND_TO_Q("\r\nДля продолжения нажмите ENTER ", d);
    STATE(d) = CON_RMENU;
}


#define MSG_UNKNOW_KEY   "\r\nUnknown key table. Retry, please: "
#define MSG_GET_NAME      "\r\nВведите имя персонажа или \"новый\": "
#define MSG_HOW_NAME      "\r\nИ все таки как Вас зовут? "
#define MSG_WHAT_NAME     "\r\nВведите имя персонажа: "
#define MSG_BAD_NAME      "\r\nНекорректное имя!"
#define MSG_NEW_NAME      "\r\nВведите имя нового персонажа: "
#define MSG_NAME_WARRING  "Первые буквы Вашего имени совпадают с уже существующим персонажем.\r\n" \
    "Для исключения разных недоразумений Вам необходимо выбрать другое имя.\r\n"
#define MSG_NEW_IP_DENY   "Создание персонажа невозможно, так как с Вашего IP адреса уже кто-то играет.\r\n" \
    "Если Вы играете из клуба попросите администратора клуба связатся с Богами.\r\n"
#define MSG_NEW_TIME_DENY "С Вашего IP адреса, менее 15 минут назад уже был создан персонаж.\r\n"
#define MSG_NEW_REST_DENY "Извините, Вы не можете создать новый персонаж в настоящий момент.\r\n"
#define MSG_NEW_BAN_DENY  "Извините, создание нового персонажа для Вашего IP - ЗАПРЕЩЕНО!!!\r\n"
#define MSG_CHAR_DELETED  "\r\nЭтот персонаж удален за нарушение правил.\r\n"
#define MSG_CHAR_DELETED2 "\r\nЭтот персонаж удален %s за нарушение правил.\r\n"
#define MSG_CHAT_NOT_FND  "Персонаж не найден.\r\n"
#define MSG_NAME_DENY     "Это имя запрещено для использования.\r\n"
#define MSG_PASSWORD      "Ваш пароль: "
#define MSG_NO_PASS       "Необходимо ввести пароль. Отсоединяемся.\r\n"
#define MSG_CLOSE_PASS    "Неверный пароль. Отсоединяемся.\r\n"
#define MSG_BAD_PASS   "Неправильный пароль!\r\nЕсли Вы забыли пароль введите 'восстановить'\r\n"
#define MSG_BAD_PASS1   "Неправильный пароль!\r\n"
#define MSG_RETYPE_PASS   "\r\nПожалуйста повторите пароль: "
#define MSG_NOCHECK_PASS  "\r\nПароли не соответствуют... повторим.\r\n"
#define MSG_QSEX          "\r\nУбедительная просьба не создавать персонажей с полом, несоответствующим реальному.\r\n" \
    "Персонажи, уличенные в нарушении данного правила, будут немедленно удалены.\r\n" \
    "Ваш пол:\r\n1) Мужской\r\n2) Женский\r\n"
#define MSG_RELIGION      "\r\nОтношение к религии:\r\n1) Атеист\r\n2) Верующий\r\n"
#define MSG_SELECT        "\r\nВаш выбор: "
#define MSG_SELECT_YN     "\r\nОтветьте Да или Нет: "
#define MSG_OK            "\r\nГотово.\r\n"
#define MSG_SAVEPASS     "\r\nПароль сохранен.\r\n"
#define MSG_NAME_NEW      "\r\nВведите имя нового персонажа: "
#define MSG_BAD_SELECT    "\r\nНеправильный выбор!"

#define MSG_NAME2 "\r\nИмя Вашего персонажа в родительном падеже (меч кого?): "
#define MSG_NAME3 "\r\nИмя Вашего персонажа в дательном падеже (отправить кому?): "
#define MSG_NAME4 "\r\nИмя Вашего персонажа в винительном падеже (ударить кого?): "
#define MSG_NAME5 "\r\nИмя Вашего персонажа в творительном падеже (сражаться с кем?): "
#define MSG_NAME6 "\r\nИмя Вашего персонажа в предложном падеже (думать о ком?): "


#define MSG_EMAIL_NOTE "\r\nВведите Ваш адрес электронной почты.\r\n" \
    "По этому адресу будет выслано письмо подтверждающее Вашу регистрацию в маде.\r\n" \
    "Если письмо вернется обратно с сообщением о несуществующем адресе, персонаж\r\n" \
    "зарегистрированный по этому адресу будет УДАЛЕН БЕЗ ПРЕДУПРЕЖДЕНИЯ.\r\n"
#define MSG_EMAIL "\r\nВведите адрес электронной почты: "
#define MSG_BAD_EMAIL "\r\nНекорректный адрес электронной почты!"
#define MSG_ENTER "\r\nДля продолжения нажмите [ENTER]"

/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
    int player_i, load_result = -1;
    char tmp_name[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char saltpass[MAX_NAME_LENGTH + 10];

    /*  int  connections=0, limit = multi_play_limit;
       struct descriptor_data *k, *next_k; */


    skip_spaces(&arg);

    switch (STATE(d)) {
            /* Выбор имени или создание нового персонажа */
        case CON_GET_KEYTABLE:{
                if (strlen(arg) > 0)
                    arg[0] = arg[strlen(arg) - 1];
                if (!*arg || *arg < '0' || *arg >= '0' + KT_LAST) {
                    SEND_TO_Q(MSG_UNKNOW_KEY, d);
                    return;
                };
                d->keytable = (ubyte) * arg - (ubyte) '0';
                SEND_TO_Q(MSG_GET_NAME, d);
                STATE(d) = CON_GET_NAME;
            }
            break;
            /* Ввод имени */
        case CON_GET_NAME:
            if (d->character == NULL) {
                d->character = new Player();
                d->character->desc = d;
            }
            if (!*arg) {
                SEND_TO_Q(MSG_HOW_NAME, d);
                SEND_TO_Q(MSG_WHAT_NAME, d);
                return;
            } else
                // Провека на запреты для создания нового персонажа
            if (isname(arg, "новый")) {
                /* Проверка на мультинг */
                /* Временно убрал
                   for (k = descriptor_list; k; k = next_k)
                   {
                   next_k = k->next;
                   if (!strcasecmp(k->host, d->host))
                   {
                   connections++;
                   if (is_proxy(d->host))
                   limit = is_proxy(d->host);

                   if(connections > limit &&
                   !PLR_FLAGGED(d->character, PLR_MULTI))
                   {
                   SEND_TO_Q(MSG_NEW_IP_DENY, d);
                   STATE(d) = CON_CLOSE;
                   sprintf(buf, "Попытка превысить лимит соединений (новый) [%s]", d->host);
                   mudlog(buf, NRM, LVL_GOD, TRUE);
                   STATE(d) = CON_CLOSE;
                   return;
                   }
                   }
                   } *///Проверка на мультинг
                /* Проверка на скорость создания */
                if (check_ip(d->host)) {
                    SEND_TO_Q(MSG_NEW_TIME_DENY, d);
                    sprintf(buf, "Попытка создания нового персонажа отклонена для [%s] (ip_table)",
                            d->host);
                    mudlog(buf, NRM, LVL_HIGOD, TRUE);
                    STATE(d) = CON_CLOSE;
                    return;
                }
                /* Проверка на запрет создания новых */
                if (mud->modRestrict) {
                    SEND_TO_Q(MSG_NEW_REST_DENY, d);
                    sprintf(buf, "Попытка создания нового персонажа отклонена для [%s] (wizlock)",
                            d->host);
                    mudlog(buf, NRM, LVL_HIGOD, TRUE);
                    STATE(d) = CON_CLOSE;
                    return;
                }
                /* Проверка на тип бана */
                if (isbanned(d->host) >= BAN_NEW) {
                    SEND_TO_Q(MSG_NEW_BAN_DENY, d);
                    sprintf(buf, "Попытка создания нового персонажа отклонена для [%s] (siteban)",
                            d->host);
                    mudlog(buf, NRM, LVL_HIGOD, TRUE);
                    STATE(d) = CON_CLOSE;
                    return;
                }
                // добавляем дефолтную ширину высоту
                d->character->sw = 78;
                d->character->sh = 22;
                strcpy(d->character->divd, "!");
                strcpy(d->character->divr, "#");
                SEND_TO_Q(MSG_QSEX, d);
                SEND_TO_Q(MSG_SELECT, d);
                STATE(d) = CON_QSEX;
            } else
                //Ввели имя
            {
                if (_parse_name(arg, tmp_name) || reserved_word(buf)) {
                    SEND_TO_Q(MSG_CHAT_NOT_FND, d);
                    SEND_TO_Q(MSG_GET_NAME, d);
                    return;
                }
                if ((player_i = load_char(tmp_name, d->character)) > -1) {
                    GET_PFILEPOS(d->character) = player_i;
                    if (!Is_Valid_Name_no_mob(tmp_name)) {
                        delete(d->character);
                        d->character = 0;
                        SEND_TO_Q(MSG_NAME_DENY, d);
                        SEND_TO_Q(MSG_GET_NAME, d);
                        return;
                    } else {
                        REMOVE_BIT(PLR_FLAGS(d->character, PLR_MAILING), PLR_MAILING);
                        REMOVE_BIT(PLR_FLAGS(d->character, PLR_SCRIPTING), PLR_SCRIPTING);
                        REMOVE_BIT(PLR_FLAGS(d->character, PLR_WRITING), PLR_WRITING);
                        REMOVE_BIT(PLR_FLAGS(d->character, PLR_CRYO), PLR_CRYO);
                        SEND_TO_Q(MSG_PASSWORD, d);
                        echo_off(d);
                        d->idle_tics = 0;
                        STATE(d) = CON_PASSWORD;
                    }
                } else {
                    SEND_TO_Q(MSG_CHAT_NOT_FND, d);
                    SEND_TO_Q(MSG_GET_NAME, d);
                    return;
                }
            }
            break;              //end CON_GET_NAME
            /* Ввод пароля */
        case CON_PASSWORD:
#ifdef TEST_CODE
            DoAfterPassword(d);
            return;
#endif
            echo_on(d);
            if (!*arg) {
                SEND_TO_Q(MSG_NO_PASS, d);
                STATE(d) = CON_CLOSE;
            } else {
                if (isname(arg, "восстановить")) {
                    LostPasswordCode().assign(d->character);

                    if (!LostPasswordFile(d->character).write()) {
                        STATE(d) = CON_CLOSE;
                        return;
                    }

                    echo_off(d);

                    sprintf(buf, "Запрос на восстановление пароля '%s (%s)' с адреса %s",
                            GET_NAME(d->character), GET_EMAIL(d->character), d->host);
                    mudlog(buf, BRF, LVL_HIGOD, TRUE);

                    SEND_TO_Q
                        ("На Ваш адрес отправлен код подтверждения. Введите его, чтобы сменить пароль.\r\n"
                         "Код подтверждения:\r\n", d);

                    STATE(d) = CON_REQUST_ANSW;
                    return;
                } else
                    if (strncmp
                        (CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character),
                         MAX_PWD_LENGTH)) {
                    sprintf(buf, "Неправильный пароль для %s с адреса %s", GET_NAME(d->character),
                            d->host);
                    mudlog(buf, BRF, LVL_HIGOD, TRUE);
                    GET_BAD_PWS(d->character)++;
                    save_char(d->character, NOWHERE);
                    if (++(d->bad_pws) >= max_bad_pws) {
                        SEND_TO_Q(MSG_CLOSE_PASS, d);
                        STATE(d) = CON_CLOSE;
                    } else {
                        SEND_TO_Q(MSG_BAD_PASS, d);
                        SEND_TO_Q(MSG_PASSWORD, d);
                        echo_off(d);
                        SEND_TO_Q("\r\n", d);
                    }
                    return;
                }
                if (PLR_FLAGGED(d->character, PLR_DELETED)) {
                    char bz[MAX_STRING_LENGTH];

                    sprintf(buf, "Попытка войти удаленым персонажем '%s' с адреса %s",
                            GET_NAME(d->character), d->host);
                    mudlog(buf, BRF, LVL_HIGOD, TRUE);

                    if (d->character->delete_name)
                        sprintf(bz, MSG_CHAR_DELETED2, d->character->delete_name);
                    else
                        strcpy(bz, MSG_CHAR_DELETED);

                    delete(d->character);
                    d->character = 0;

                    SEND_TO_Q(bz, d);
                    STATE(d) = CON_CLOSE;
                    return;
                }
                if (mud->modRegistration && d->character->registry_code) {
                    SEND_TO_Q("Код регистрации:\r\n", d);
                    STATE(d) = CON_ENTER_REG;
                    return;
                } else
                    DoAfterPassword(d);
            }
            break;
        case CON_ENTER_REG:
            if (strcmp(arg, d->character->registry_code)) {
                SEND_TO_Q("Неверный код регистрации.\r\nКод регистрации:\r\n", d);
                STATE(d) = CON_ENTER_REG;
                return;
            } else {
                if (d->character->registry_code)
                    free(d->character->registry_code);
                d->character->registry_code = NULL;
                save_char(d->character, NOWHERE);
                DoAfterPassword(d);
            }
            break;
        case CON_REQUST_ANSW:
            if (strcmp(arg, d->character->pc()->specials.saved.cAnswer)) {
                SEND_TO_Q("Неверный код подтверждения.\r\nКод подтверждения:\r\n", d);
                STATE(d) = CON_REQUST_ANSW;
                return;
            } else {
                SEND_TO_Q("Корректный код подтверждения.\r\n", d);
                SEND_TO_Q("Введите новый пароль: ", d);
                echo_off(d);
                STATE(d) = CON_CHPWD_GETNEW;
                return;
            }
            break;
            /* Выбор пола */
        case CON_QSEX:
            if (pre_help(d->character, arg)) {
                SEND_TO_Q(MSG_QSEX, d);
                SEND_TO_Q(MSG_SELECT, d);
                STATE(d) = CON_QSEX;
                return;
            }
            switch (LOWER(*arg)) {
                case '1':
                    GET_SEX(d->character) = SEX_MALE;
                    break;
                case '2':
                    GET_SEX(d->character) = SEX_FEMALE;
                    break;
                default:
                    SEND_TO_Q(MSG_BAD_SELECT, d);
                    SEND_TO_Q(MSG_SELECT, d);
                    return;
            }
            SEND_TO_Q(race_menu, d);
            SEND_TO_Q(MSG_SELECT, d);
            STATE(d) = CON_RACE;
            break;
            /* Выбор расы */
        case CON_RACE:
            if (pre_help(d->character, arg)) {
                SEND_TO_Q(race_menu, d);
                SEND_TO_Q(MSG_SELECT, d);
                STATE(d) = CON_RACE;
                return;
            }
            load_result = parse_race(*arg);
            if (load_result == RACE_UNDEFINED) {
                SEND_TO_Q(MSG_BAD_SELECT, d);
                SEND_TO_Q(MSG_SELECT, d);
                return;
            }
            GET_RACE(d->character) = load_result;
            SEND_TO_Q(MSG_NAME_NEW, d);
            STATE(d) = CON_NAME_NEW;
            break;
            /* Выбор имени персонажа */
        case CON_NAME_NEW:
            if (_parse_name(arg, tmp_name) ||
                strlen(tmp_name) < MIN_NAME_LENGTH ||
                strlen(tmp_name) > MAX_NAME_LENGTH ||
                !Is_Valid_Name(tmp_name) ||
                !Valid_Name(tmp_name) || fill_word(strcpy(buf, tmp_name)) || reserved_word(buf)) {
                SEND_TO_Q(MSG_BAD_NAME, d);
                SEND_TO_Q(MSG_NEW_NAME, d);
                return;
            }
            if (cmp_ptable_by_name(tmp_name, MIN_NAME_LENGTH) >= 0) {
                SEND_TO_Q(MSG_NAME_WARRING, d);
                SEND_TO_Q(MSG_NEW_NAME, d);
                return;
            }
            CREATE(d->character->player.name, char, strlen(tmp_name) + 1);
            strcpy(d->character->player.name, CAP(tmp_name));
            CREATE(GET_PAD(d->character, 0), char, strlen(tmp_name) + 1);

            strcpy(GET_PAD(d->character, 0), CAP(tmp_name));
            SEND_TO_Q(create_name_rules, d);
            SEND_TO_Q("\r\nНовый персонаж.\r\n", d);
            sprintf(buf, "Вы действительно выбрали имя %s [Д/Н]? ", tmp_name);
            SEND_TO_Q(buf, d);
            STATE(d) = CON_NAME_CNFRM;
            break;
            /* Подтверждение имени */
        case CON_NAME_CNFRM:
            if (UPPER(*arg) == 'Y' || UPPER(*arg) == 'Д') {
                if (mud->modRestrict) {
                    SEND_TO_Q(MSG_NEW_REST_DENY, d);
                    sprintf(buf,
                            "Попытка создания нового персонажа %s отклонена для [%s] (wizlock)",
                            GET_PC_NAME(d->character), d->host);
                    mudlog(buf, NRM, LVL_HIGOD, TRUE);
                    STATE(d) = CON_CLOSE;
                    return;
                }
                GET_EXP(d->character) = 1;
                SEND_TO_Q(MSG_NAME2, d);
                echo_on(d);
                STATE(d) = CON_NAME2;
            } else if (UPPER(*arg) == 'N' || UPPER(*arg) == 'Н') {
                SEND_TO_Q("Поробуем еще раз?:)\r\n", d);
                SEND_TO_Q(MSG_NEW_NAME, d);
                free(d->character->player.name);
                d->character->player.name = NULL;
                STATE(d) = CON_NAME_NEW;
            } else
                SEND_TO_Q(MSG_SELECT_YN, d);
            break;
        case CON_NAME2:
            skip_spaces(&arg);
            if (!_parse_name(arg, tmp_name) &&
                strlen(tmp_name) >= 4 && strlen(tmp_name) <= MAX_NAME_LENGTH) {
                CREATE(GET_PAD(d->character, 1), char, strlen(tmp_name) + 1);

                strcpy(GET_PAD(d->character, 1), CAP(tmp_name));
                SEND_TO_Q(MSG_NAME3, d);
                echo_on(d);
                STATE(d) = CON_NAME3;
            } else {
                SEND_TO_Q(MSG_BAD_NAME, d);
                SEND_TO_Q(MSG_NAME2, d);
            };
            break;
        case CON_NAME3:
            skip_spaces(&arg);
            if (!_parse_name(arg, tmp_name) &&
                strlen(tmp_name) >= 4 && strlen(tmp_name) <= MAX_NAME_LENGTH) {
                CREATE(GET_PAD(d->character, 2), char, strlen(tmp_name) + 1);

                strcpy(GET_PAD(d->character, 2), CAP(tmp_name));
                SEND_TO_Q(MSG_NAME4, d);
                echo_on(d);
                STATE(d) = CON_NAME4;
            } else {
                SEND_TO_Q(MSG_BAD_NAME, d);
                SEND_TO_Q(MSG_NAME3, d);
            };
            break;
        case CON_NAME4:
            skip_spaces(&arg);
            if (!_parse_name(arg, tmp_name) &&
                strlen(tmp_name) >= 4 && strlen(tmp_name) <= MAX_NAME_LENGTH) {
                CREATE(GET_PAD(d->character, 3), char, strlen(tmp_name) + 1);

                strcpy(GET_PAD(d->character, 3), CAP(tmp_name));
                SEND_TO_Q(MSG_NAME5, d);
                echo_on(d);
                STATE(d) = CON_NAME5;
            } else {
                SEND_TO_Q(MSG_BAD_NAME, d);
                SEND_TO_Q(MSG_NAME4, d);
            };
            break;
        case CON_NAME5:
            skip_spaces(&arg);
            if (!_parse_name(arg, tmp_name) &&
                strlen(tmp_name) >= 4 && strlen(tmp_name) <= MAX_NAME_LENGTH) {
                CREATE(GET_PAD(d->character, 4), char, strlen(tmp_name) + 1);

                strcpy(GET_PAD(d->character, 4), CAP(tmp_name));
                SEND_TO_Q(MSG_NAME6, d);
                echo_on(d);
                STATE(d) = CON_NAME6;
            } else {
                SEND_TO_Q(MSG_BAD_NAME, d);
                SEND_TO_Q(MSG_NAME5, d);
            };
            break;
        case CON_NAME6:
            skip_spaces(&arg);
            if (!_parse_name(arg, tmp_name) &&
                strlen(tmp_name) >= 4 && strlen(tmp_name) <= MAX_NAME_LENGTH) {
                CREATE(GET_PAD(d->character, 5), char, strlen(tmp_name) + 1);

                strcpy(GET_PAD(d->character, 5), CAP(tmp_name));
                sprintf(buf, "\r\nВведите пароль для %s: ", GET_PAD(d->character, 1));
                SEND_TO_Q(buf, d);
                echo_off(d);
                STATE(d) = CON_NEWPASSWD;
            } else {
                SEND_TO_Q(MSG_BAD_NAME, d);
                SEND_TO_Q(MSG_NAME6, d);
            };
            break;

            /* Выбор пароля */
        case CON_NEWPASSWD:
        case CON_CHPWD_GETNEW:
            if (!*arg || strlen(arg) > MAX_PWD_LENGTH || strlen(arg) < 3 ||
                !str_cmp(arg, GET_PC_NAME(d->character))) {
                SEND_TO_Q(MSG_BAD_PASS1, d);
                SEND_TO_Q(MSG_PASSWORD, d);
                return;
            }

            sprintf(saltpass, "$1$%s$", GET_PC_NAME(d->character));
            strcpy(GET_PASSWD(d->character), CRYPT(arg, saltpass));

            SEND_TO_Q(MSG_RETYPE_PASS, d);
            if (STATE(d) == CON_NEWPASSWD)
                STATE(d) = CON_CNFPASSWD;
            else
                STATE(d) = CON_CHPWD_VRFY;
            break;
            /* Проверка пароля */
        case CON_CNFPASSWD:
        case CON_CHPWD_VRFY:
            if (strncmp(CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character),
                        MAX_PWD_LENGTH)) {
                SEND_TO_Q(MSG_NOCHECK_PASS, d);
                SEND_TO_Q(MSG_PASSWORD, d);
                if (STATE(d) == CON_CNFPASSWD)
                    STATE(d) = CON_NEWPASSWD;
                else
                    STATE(d) = CON_CHPWD_GETNEW;
                return;
            }
            echo_on(d);

            if (STATE(d) == CON_CNFPASSWD) {
                SEND_TO_Q(MSG_RELIGION, d);
                SEND_TO_Q(MSG_SELECT, d);
                STATE(d) = CON_RELIGION;
            } else {
                save_char(d->character, NOWHERE);
                echo_on(d);
                SEND_TO_Q(MSG_SAVEPASS, d);
                SEND_TO_Q(MENU, d);
                STATE(d) = CON_MENU;
            }
            break;
            /* Выбор религии */
        case CON_RELIGION:
            if (pre_help(d->character, arg)) {
                SEND_TO_Q(MSG_RELIGION, d);
                SEND_TO_Q(MSG_SELECT, d);
                STATE(d) = CON_RELIGION;
                return;
            }
            switch (LOWER(*arg)) {
                case '1':
                    display_align(d->character);
                    SEND_TO_Q(MSG_SELECT, d);
                    STATE(d) = CON_ALIGN;
                    break;
                case '2':
                    display_gods(d->character);
                    SEND_TO_Q(MSG_SELECT, d);
                    STATE(d) = CON_GODS;
                    break;
                default:
                    SEND_TO_Q(MSG_BAD_SELECT, d);
                    SEND_TO_Q(MSG_SELECT, d);
                    return;
            }
            break;
            /* Выбор натуры */
        case CON_ALIGN:
            if (pre_help(d->character, arg)) {
                display_align(d->character);
                SEND_TO_Q(MSG_SELECT, d);
                STATE(d) = CON_ALIGN;
                return;
            }
            load_result = parse_align(d->character, atoi(arg));
            if (load_result == -1) {
                SEND_TO_Q(MSG_BAD_SELECT, d);
                SEND_TO_Q(MSG_SELECT, d);
                return;
            }
            GET_GODS(d->character) = GOD_UNDEFINED;
            switch (load_result) {
                case 0:
                    GET_ALIGNMENT(d->character) = 800;  //GA
                    break;
                case 1:
                    GET_ALIGNMENT(d->character) = 601;  //GG
                    break;
                case 2:
                    GET_ALIGNMENT(d->character) = 401;  //GN
                    break;
                case 3:
                    GET_ALIGNMENT(d->character) = 201;  //NG
                    break;
                case 4:
                    GET_ALIGNMENT(d->character) = 0;    //NN
                    break;
                case 5:
                    GET_ALIGNMENT(d->character) = -201; //NE
                    break;
                case 6:
                    GET_ALIGNMENT(d->character) = -401; //EN;
                    break;
                case 7:
                    GET_ALIGNMENT(d->character) = -601; //EE
                    break;
                case 8:
                    GET_ALIGNMENT(d->character) = -800; //EA
                    break;
                default:
                    GET_ALIGNMENT(d->character) = 0;
                    break;
            }

            SEND_TO_Q(MSG_EMAIL_NOTE, d);
            SEND_TO_Q(MSG_EMAIL, d);
            STATE(d) = CON_GET_EMAIL;
            break;

        case CON_GODS:
            if (pre_help(d->character, arg)) {
                display_gods(d->character);
                SEND_TO_Q(MSG_SELECT, d);
                STATE(d) = CON_GODS;
                return;
            }
            load_result = parse_gods(d->character, atoi(arg));
            if (load_result == -1) {
                SEND_TO_Q(MSG_BAD_SELECT, d);
                SEND_TO_Q(MSG_SELECT, d);
                return;
            }
            GET_GODS(d->character) = load_result;

            switch (load_result) {
                case 1:
                    GET_ALIGNMENT(d->character) = 800;  //GA
                    break;
                case 2:
                    GET_ALIGNMENT(d->character) = 800;  //GG
                    break;
                case 3:
                    GET_ALIGNMENT(d->character) = 800;  //GN
                    break;
                case 4:
                    GET_ALIGNMENT(d->character) = 200;  //NG
                    break;
                case 5:
                    GET_ALIGNMENT(d->character) = 0;    //NN
                    break;
                case 6:
                    GET_ALIGNMENT(d->character) = -200; //NE
                    break;
                case 7:
                    GET_ALIGNMENT(d->character) = -800; //EN;
                    break;
                case 8:
                    GET_ALIGNMENT(d->character) = -800; //EE
                    break;
                case 9:
                    GET_ALIGNMENT(d->character) = -800; //EA
                    break;
                default:
                    GET_ALIGNMENT(d->character) = 0;
                    break;
            }

            SEND_TO_Q(MSG_EMAIL_NOTE, d);
            SEND_TO_Q(MSG_EMAIL, d);
            STATE(d) = CON_GET_EMAIL;
            break;

        case CON_GET_EMAIL:
            if (!*arg) {
                SEND_TO_Q(MSG_EMAIL, d);
                return;
            } else {
                for (int i = 0; arg[i]; i++)
                    arg[i] = LOWER(arg[i]);

                PlayerMail email(arg);

                if (!email.valid()) {
                    SEND_TO_Q(MSG_BAD_EMAIL, d);
                    SEND_TO_Q(MSG_EMAIL, d);
                    return;
                }

                if (!email.goodsize()) {
                    SEND_TO_Q("Слишком длинный адрес электронный почты.", d);
                    SEND_TO_Q(MSG_EMAIL, d);
                    return;
                }

                if (GET_PFILEPOS(d->character) < 0)
                    GET_PFILEPOS(d->character) = create_entry(GET_PC_NAME(d->character));

                init_char(d->character);
                email.assign(d->character);
                if (mud->modRegistration) 
                    RegistrationCode().assign(d->character);
                save_char(d->character, NOWHERE);
                save_player_index();
            }

            if (mud->modRegistration)
                if (!RegistrationFile(d->character).write()) {
                    STATE(d) = CON_CLOSE;
                    return;
                }

            sprintf(buf, "%s [%s] - Новый игрок,  %s.\r\nE-mail: %s",
                    GET_NAME(d->character), d->host,
                    race_name[(int) GET_RACE(d->character)][(int) GET_SEX(d->character)],
                    GET_EMAIL(d->character));
            mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
            SEND_TO_Q(mud->getTextFileLoader()->get("motd").c_str(), d);
            SEND_TO_Q(MSG_ENTER, d);
            add_ip_table(d->host, time(0));
            STATE(d) = CON_RMENU;
            break;

        case CON_RMENU:
            if (Is_Registry_Name(GET_NAME(d->character)))
                NAME_GOD(d->character) = 1035;
            SEND_TO_Q(MENU, d);
            STATE(d) = CON_MENU;
            break;

        case CON_MENU:
            switch (*arg) {
                case '0':
                    SEND_TO_Q("Надоело играть? Ну, тогда, пока!.\r\n", d);
                    STATE(d) = CON_CLOSE;
                    break;
                case '1':
                    load_char(d->character->player.name, d->character);
                    do_entergame(d);
                    break;
                case '2':
                    if (d->character->player.description) {
                        SEND_TO_Q
                            ("Ширина строки не более 80ти символов, кол-во строк не более 15.\r\n",
                             d);
                        SEND_TO_Q("Ваше ТЕКУЩЕЕ описание:\r\n", d);
                        SEND_TO_Q(d->character->player.description, d);
                        if (d->backstr)
                            free(d->backstr);
                        d->backstr = str_dup(d->character->player.description);
                    }
                    SEND_TO_Q
                        ("Введите описание Вашего героя, которое будет выводиться по команде <осмотреть>.\r\n",
                         d);
                    SEND_TO_Q("(/s сохранить /h помощь)\r\n", d);
                    d->str = &d->character->player.description;
                    d->max_str = EXDSCR_LENGTH;
                    STATE(d) = CON_EXDESC;
                    break;
                case '3':
                    send_to_char("Вашему персонажу выдана тренировочная экипировка.\r\n",
                                 d->character);
                    TOGGLE_BIT(PRF_FLAGS(d->character, PRF_EQ), PRF_EQ);
                    SEND_TO_Q(MENU, d);
                    break;
                case '4':
                    char buf[MAX_STRING_LENGTH];

                    sprintf(buf, "Статистика на %s\r\n", ascii_time(time(0)));
                    sprintf(buf + strlen(buf), "------------------------------------\r\n");
                    sprintf(buf + strlen(buf), "Зарегистрированных игроков: %6d\r\n",
                            top_of_p_table + 1);
                    sprintf(buf + strlen(buf), "Уникальных монстров:        %6d\r\n",
                            top_of_mobt + 1);
                    sprintf(buf + strlen(buf), "Уникальных предметов:       %6d\r\n",
                            top_of_objt + 1);
                    sprintf(buf + strlen(buf), "Игровых локаций:            %6d\r\n",
                            top_of_world + 1);
                    sprintf(buf + strlen(buf), "Социалов:                   %6d\r\n",
                            top_of_socialk + 1);
                    sprintf(buf + strlen(buf), "Игровых зон:                %6d\r\n",
                            top_of_zone_table + 1);
                    sprintf(buf + strlen(buf), " Служебных:                 %6d\r\n",
                            count_zones[0] + 1);
                    sprintf(buf + strlen(buf), " Начальных:                 %6d\r\n",
                            count_zones[1] + 1);
                    sprintf(buf + strlen(buf), " Младших:                   %6d\r\n",
                            count_zones[2] + 1);
                    sprintf(buf + strlen(buf), " Средних:                   %6d\r\n",
                            count_zones[3] + 1);
                    sprintf(buf + strlen(buf), " Высоких:                   %6d\r\n",
                            count_zones[4] + 1);
                    sprintf(buf + strlen(buf), " Элитных:                   %6d\r\n",
                            count_zones[5] + 1);
                    sprintf(buf + strlen(buf), " Для всех:                  %6d\r\n",
                            count_zones[6] + 1);

                    SEND_TO_Q(buf, d);
                    SEND_TO_Q(MSG_ENTER, d);
                    STATE(d) = CON_RMENU;
                    break;
                case '8':
                    SEND_TO_Q("\r\nВведите СТАРЫЙ пароль: ", d);
                    echo_off(d);
                    STATE(d) = CON_CHPWD_GETOLD;
                    break;
                case '9':
                    SEND_TO_Q("\r\nДля подтверждения введите свой пароль: ", d);
                    echo_off(d);
                    STATE(d) = CON_DELCNF1;
                    break;
                default:
                    SEND_TO_Q("\r\nЭто не правильный ответ!\r\n", d);
                    SEND_TO_Q(MENU, d);
                    break;
            }
            break;

        case CON_CHPWD_GETOLD:
            if (strncmp
                (CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
                echo_on(d);
                SEND_TO_Q(MSG_BAD_PASS1, d);
                SEND_TO_Q(MENU, d);
                STATE(d) = CON_MENU;
            } else {
                SEND_TO_Q("\r\nВведите НОВЫЙ пароль: ", d);
                STATE(d) = CON_CHPWD_GETNEW;
            }
            return;
            break;
        case CON_DELCNF1:
            echo_on(d);
            if (strncmp
                (CRYPT(arg, GET_PASSWD(d->character)), GET_PASSWD(d->character), MAX_PWD_LENGTH)) {
                SEND_TO_Q(MSG_BAD_PASS1, d);
                SEND_TO_Q(MENU, d);
                STATE(d) = CON_MENU;
            } else {
                SEND_TO_Q("\r\n!!! ВАШ ПЕРСОНАЖ БУДЕТ УДАЛЕН!!!\r\n"
                          "Вы АБСОЛЮТНО В ЭТОМ УВЕРЕНЫ ?\r\n\r\n"
                          "Наберите \"YES / ДА\" для подтверждения: ", d);
                STATE(d) = CON_DELCNF2;
            }
            break;
        case CON_DELCNF2:
            if (!strcmp(arg, "yes") || !strcmp(arg, "YES") ||
                !strcmp(arg, "да") || !strcmp(arg, "ДА")) {
                if (PLR_FLAGGED(d->character, PLR_FROZEN)) {
                    SEND_TO_Q("Вы решились на суицид, но Боги запретили Вам.\r\n", d);
                    SEND_TO_Q("Персонаж не удален.\r\n", d);
                    STATE(d) = CON_CLOSE;
                    return;
                }
                if (GET_LEVEL(d->character) >= LVL_GRGOD)
                    return;
                if (!delete_char(GET_NAME(d->character))) {
                    sprintf(buf, "Произошла ошибка удаления персонажа %s\r\n"
                            "Сообщите об этого имморталам мада.\r\n", GET_NAME(d->character));
                    SEND_TO_Q(buf, d);
                    SEND_TO_Q(MENU, d);
                    STATE(d) = CON_MENU;
                    return;
                }
                sprintf(buf, "Персонаж '%s' удален!\r\n"
                        "До свидания.\r\n", GET_NAME(d->character));
                SEND_TO_Q(buf, d);
                sprintf(buf, "%s (lev %d) has self-deleted.", GET_NAME(d->character),
                        GET_LEVEL(d->character));
                mudlog(buf, NRM, LVL_HIGOD, TRUE);
                STATE(d) = CON_CLOSE;
                return;
            } else {
                SEND_TO_Q("\r\nПерсонаж не удален.\r\n", d);
                SEND_TO_Q(MENU, d);
                STATE(d) = CON_MENU;
            }
            break;


        case CON_CLOSE:
            break;
    }                           //end switch
}

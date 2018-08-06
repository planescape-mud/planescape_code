/* ***********************************************************************
*  File: alias.c    A utility to CircleMUD                               *
* Usage: writing/reading player's aliases.                               *
*                                                                        *
* Code done by Jeremy Hess and Chad Thompson                             *
* Modifed by George Greer for inclusion into CircleMUD bpl15.            *
*                                                                        *
* Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
* CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
*********************************************************************** */

#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "db.h"
#include "comm.h"

void write_aliases(struct char_data *ch)
{
    FILE *file;
    char fn[MAX_STRING_LENGTH];
    struct alias_data *temp;

    log("Write alias %s", GET_NAME(ch));
    get_filename(GET_NAME(ch), fn, ALIAS_FILE);
    remove(fn);

    if (GET_ALIASES(ch) == NULL)
        return;


    if ((file = fopen(fn, "w")) == NULL) {
        syserr("SYSERR: Couldn't save aliases for %s in '%s'.", GET_NAME(ch), fn);
        return;
    }

    for (temp = GET_ALIASES(ch); temp; temp = temp->next) {
        int aliaslen = strlen(temp->alias);
        int repllen = strlen(temp->replacement) - 1;

        fprintf(file, "%d\n%s\n"        /* Alias */
                "%d\n%s\n"      /* Replacement */
                "%d\n",         /* Type */
                aliaslen, temp->alias, repllen, temp->replacement + 1, temp->type);
    }

    fclose(file);
}

void read_aliases(struct char_data *ch)
{
    FILE *file;
    char xbuf[MAX_STRING_LENGTH];
    struct alias_data *t2;
    int length;

    //log("Read alias %s",GET_NAME(ch));
    get_filename(GET_NAME(ch), xbuf, ALIAS_FILE);

    if ((file = fopen(xbuf, "r")) == NULL) {
        if (errno != ENOENT) {
            syserr("SYSERR: Couldn't open alias file '%s' for %s.", xbuf, GET_NAME(ch));
        }
        return;
    }

    CREATE(GET_ALIASES(ch), struct alias_data, 1);
    t2 = GET_ALIASES(ch);

    for (;;) {                  /* Read the aliased command. */
        fscanf(file, "%d\n", &length);
        fgets(xbuf, length + 1, file);
        t2->alias = str_dup(xbuf);

        /* Build the replacement. */
        fscanf(file, "%d\n", &length);
        *xbuf = ' ';            /* Doesn't need terminated, fgets() will. */
        fgets(xbuf + 1, length + 1, file);
        t2->replacement = str_dup(xbuf);

        /* Figure out the alias type. */
        fscanf(file, "%d\n", &length);
        t2->type = length;

        if (feof(file))
            break;

        CREATE(t2->next, struct alias_data, 1);

        t2 = t2->next;
    };

    fclose(file);
}

/**************************************************************************
 * Routines to handle aliasing                                             *
  **************************************************************************/

struct alias_data *find_alias(struct alias_data *alias_list, char *str)
{
    while (alias_list != NULL) {
        if (*str == *alias_list->alias) /* hey, every little bit counts:-) */
            if (!strcmp(str, alias_list->alias))
                return (alias_list);

        alias_list = alias_list->next;
    }

    return (NULL);
}


void free_alias(struct alias_data *a)
{
    if (a->alias)
        free(a->alias);
    if (a->replacement)
        free(a->replacement);
    free(a);
}


ACMD(do_alias)
{
    char *repl;
    struct alias_data *a, *temp;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    repl = any_one_arg(argument, arg);

    if (!*arg) {                /* no argument specified -- list currently defined aliases */
        send_to_char("Определены следующие синонимы:\r\n", ch);
        if ((a = GET_ALIASES(ch)) == NULL)
            send_to_char(" Нет синонимов.\r\n", ch);
        else {
            while (a != NULL) {
                sprintf(buf, "%-15s %s\r\n", a->alias, a->replacement);
                send_to_char(buf, ch);
                a = a->next;
            }
        }
    } else {                    /* otherwise, add or remove aliases */
        /* is this an alias we've already defined? */
        if ((a = find_alias(GET_ALIASES(ch), arg)) != NULL) {
            REMOVE_FROM_LIST(a, GET_ALIASES(ch), next);
            free_alias(a);
        }
        /* if no replacement string is specified, assume we want to delete */
        if (!*repl) {
            if (a == NULL)
                send_to_char("Такой синоним уже определен.\r\n", ch);
            else
                send_to_char("Синоним успешно удален.\r\n", ch);
        } else {                /* otherwise, either add or redefine an alias */
            if (!str_cmp(arg, "alias") || !str_cmp(arg, "синоним")) {
                send_to_char("Вы не можете определить синоним 'alias' или 'синоним'.\r\n", ch);
                return;
            }
            CREATE(a, struct alias_data, 1);
            a->alias = str_dup(arg);
            delete_doubledollar(repl);
            a->replacement = str_dup(repl);
            if (strchr(repl, ALIAS_SEP_CHAR) || strchr(repl, ALIAS_VAR_CHAR))
                a->type = ALIAS_COMPLEX;
            else
                a->type = ALIAS_SIMPLE;
            a->next = GET_ALIASES(ch);
            GET_ALIASES(ch) = a;
            send_to_char("Синоним успешно добавлен.\r\n", ch);
        }
    }
}

/*
 * Valid numeric replacements are only $1 .. $9 (makes parsing a little
 * easier, and it's not that much of a limitation anyway.)  Also valid
 * is "$*", which stands for the entire original line after the alias.
 * ";" is used to delimit commands.
 */
#define NUM_TOKENS       9

void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias_data *a)
{
    struct txt_q temp_queue;
    char *tokens[NUM_TOKENS], *temp, *write_point;
    int num_of_tokens = 0, num;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    /* First, parse the original string */
    temp = strtok(strcpy(buf2, orig), " ");
    while (temp != NULL && num_of_tokens < NUM_TOKENS) {
        tokens[num_of_tokens++] = temp;
        temp = strtok(NULL, " ");
    }

    /* initialize */
    write_point = buf;
    temp_queue.head = temp_queue.tail = NULL;

    /* now parse the alias */
    for (temp = a->replacement; *temp; temp++) {
        if (*temp == ALIAS_SEP_CHAR) {
            *write_point = '\0';
            buf[MAX_INPUT_LENGTH - 1] = '\0';
            write_to_q(buf, &temp_queue, 1);
            write_point = buf;
        } else if (*temp == ALIAS_VAR_CHAR) {
            temp++;
            if ((num = *temp - '1') < num_of_tokens && num >= 0) {
                strcpy(write_point, tokens[num]);
                write_point += strlen(tokens[num]);
            } else if (*temp == ALIAS_GLOB_CHAR) {
                strcpy(write_point, orig);
                write_point += strlen(orig);
            } else if ((*(write_point++) = *temp) == '$')       /* redouble $ for act safety */
                *(write_point++) = '$';
        } else
            *(write_point++) = *temp;
    }

    *write_point = '\0';
    buf[MAX_INPUT_LENGTH - 1] = '\0';
    write_to_q(buf, &temp_queue, 1);

    /* push our temp_queue on to the _front_ of the input queue */
    if (input_q->head == NULL)
        *input_q = temp_queue;
    else {
        temp_queue.tail->next = input_q->head;
        input_q->head = temp_queue.head;
    }
}


/*
 * Given a character and a string, perform alias replacement on it.
 *
 * Return values:
 *   0: String was modified in place; call command_interpreter immediately.
 *   1: String was _not_ modified in place; rather, the expanded aliases
 *      have been placed at the front of the character's input queue.
 */
int perform_alias(struct descriptor_data *d, char *orig)
{
    char first_arg[MAX_INPUT_LENGTH], *ptr;
    struct alias_data *a, *tmp;

    /* Mobs don't have alaises. */
    if (IS_NPC(d->character))
        return (0);

    /* bail out immediately if the guy doesn't have any aliases */
    if ((tmp = GET_ALIASES(d->character)) == NULL)
        return (0);

    /* find the alias we're supposed to match */
    ptr = any_one_arg(orig, first_arg);

    /* bail out if it's null */
    if (!*first_arg)
        return (0);

    /* if the first arg is not an alias, return without doing anything */
    if ((a = find_alias(tmp, first_arg)) == NULL)
        return (0);

    if (a->type == ALIAS_SIMPLE) {
        strcpy(orig, a->replacement);
        return (0);
    } else {
        perform_complex_alias(&d->input, ptr, a);
        return (1);
    }
}

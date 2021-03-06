/* ************************************************************************
*   File: modify.c                                      Part of CircleMUD *
*  Usage: Run-time modification of game variables                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "spells.h"
#include "mail.h"
#include "boards.h"
#include "screen.h"
#include "constants.h"
#include "scriptboard.h"

#define PARSE_FORMAT      0
#define PARSE_REPLACE     1
#define PARSE_HELP        2
#define PARSE_DELETE      3
#define PARSE_INSERT      4
#define PARSE_LIST_NORM   5
#define PARSE_LIST_NUM    6
#define PARSE_EDIT        7


/* local functions */
void smash_tilde(char *str);
char *next_page(char *str, struct char_data *ch);
int count_pages(char *str, struct char_data *ch);
void paginate_string(char *str, struct descriptor_data *d);



/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */

/*
 * Put '#if 1' here to erase ~, or roll your own method.  A common idea
 * is smash/show tilde to convert the tilde to another innocuous character
 * to save and then back to display it. Whatever you do, at least keep the
 * function around because other MUD packages use it, like mudFTP.
 *   -gg 9/9/98
 */
void smash_tilde(char *str)
{
#if 0
    /*
     * Erase any ~'s inserted by people in the editor.  This prevents anyone
     * using online creation from causing parse errors in the world files.
     * Derived from an idea by Sammy <samedi@dhc.net> (who happens to like
     * his tildes thank you very much.), -gg 2/20/98
     */
    while ((str = strchr(str, '~')) != NULL)
        *str = ' ';
#endif
}

/*
 * Basic API function to start writing somewhere.
 *
 * 'data' isn't used in stock CircleMUD but you can use it to pass whatever
 * else you may want through it.  The improved editor patch when updated
 * could use it to pass the old text buffer, for instance.
 */
void string_write(struct descriptor_data *d, char **writeto, size_t len, long mailto, void *data)
{
    if (d->character && !IS_NPC(d->character))
        SET_BIT(PLR_FLAGS(d->character, PLR_WRITING), PLR_WRITING);

    if (data)
        mudlog("SYSERR: string_write: I don't understand special data.", BRF, LVL_IMMORT, TRUE);

    d->str = writeto;
    d->max_str = len;
    d->mail_to = mailto;
}


/*
 * Handle some editor commands.
 */
void parse_action(int command, char *string, struct descriptor_data *d)
{
    int indent = 0, rep_all = 0, flags = 0, total_len, replaced;
    register int j = 0;
    int i, line_low, line_high;
    char *s, *t, temp;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    //log("[PA] Start %d(%s)", command, string);
    switch (command) {
        case PARSE_HELP:
            sprintf(buf,
                    "������ ������ ���������: /<letter>\r\n\r\n"
                    "/a         -  ���������� ��������������\r\n"
                    "/c         -  �������� �����\r\n"
                    "/d#,       -  ������� ������ #\r\n"
                    "/e# <text> -  �������� ������ # �� ����� <text>\r\n"
                    "/f         -  ������������� �����\r\n"
                    "/fi        -  indented formatting of text\r\n"
                    "/h         -  ���������� ������ ������ (������)\r\n"
                    "/i# <text> -  �������� <text> ����� ������ #\r\n"
                    "/l         -  ���������� �����\r\n"
                    "/n         -  ���������� ����� � �������� �����\r\n"
                    "/r 'a' 'b' -  �������� ������ ��������� ������ <a> � ������ �� ����� <b>\r\n"
                    "/ra 'a' 'b'-  �������� ��� ��������� ������ <a> � ������ �� ����� <b>\r\n"
                    "              ������: /r[a] '������' '��_���_������'\r\n"
                    "/s         -  ��������� �����\r\n");
            SEND_TO_Q(buf, d);
            break;
        case PARSE_FORMAT:
            while (isalpha(string[j]) && j < 2) {
                switch (string[j]) {
                    case 'i':
                        if (!indent) {
                            indent = TRUE;
                            flags += FORMAT_INDENT;
                        }
                        break;
                    default:
                        break;
                }
                j++;
            }
            format_text(d->str, flags, d, d->max_str);
            sprintf(buf, "����� �������������� %s.\r\n", (indent ? "WITH INDENT" : ""));
            SEND_TO_Q(buf, d);
            break;
        case PARSE_REPLACE:
            while (isalpha(string[j]) && j < 2) {
                switch (string[j]) {
                    case 'a':
                        if (!indent)
                            rep_all = 1;
                        break;
                    default:
                        break;
                }
                j++;
            }
            if ((s = strtok(string, "'")) == NULL) {
                SEND_TO_Q("�������� ������.\r\n", d);
                return;
            } else if ((s = strtok(NULL, "'")) == NULL) {
                SEND_TO_Q("������ ������ ���� �������� � ���������.\r\n", d);
                return;
            } else if ((t = strtok(NULL, "'")) == NULL) {
                SEND_TO_Q("No replacement string.\r\n", d);
                return;
            } else if ((t = strtok(NULL, "'")) == NULL) {
                SEND_TO_Q("���������� ������ ������ ���� ��������� � ���������.\r\n", d);
                return;
            } else
                if ((total_len =
                     (int) ((strlen(t) - strlen(s)) + strlen(*d->str))) <= (int) d->max_str) {
                if ((replaced = replace_str(d->str, s, t, rep_all, d->max_str)) > 0) {
                    sprintf(buf, "�������� ��������� '%s' �� '%s' - %d.\r\n", s, t, replaced);
                    SEND_TO_Q(buf, d);
                } else if (replaced == 0) {
                    sprintf(buf, "������ '%s' �� ������.\r\n", s);
                    SEND_TO_Q(buf, d);
                } else
                    SEND_TO_Q("������: ��� ������� ������ ����� ���������� - ��������.\r\n", d);
            } else
                SEND_TO_Q("��� ���������� ����� ��� ���������� �������.\r\n", d);
            break;
        case PARSE_DELETE:
            switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
                case 0:
                    SEND_TO_Q("�� ������ ������� ����� ������ ��� �������� ��� ��������.\r\n", d);
                    return;
                case 1:
                    line_high = line_low;
                    break;
                case 2:
                    if (line_high < line_low) {
                        SEND_TO_Q("�������� ��������.\r\n", d);
                        return;
                    }
                    break;
            }

            i = 1;
            total_len = 1;
            if ((s = *d->str) == NULL) {
                SEND_TO_Q("����� ����.\r\n", d);
                return;
            } else if (line_low > 0) {
                while (s && (i < line_low))
                    if ((s = strchr(s, '\n')) != NULL) {
                        i++;
                        s++;
                    }
                if ((i < line_low) || (s == NULL)) {
                    SEND_TO_Q("������(�) ��� ��������� - ���������������.\r\n", d);
                    return;
                }
                t = s;
                while (s && (i < line_high))
                    if ((s = strchr(s, '\n')) != NULL) {
                        i++;
                        total_len++;
                        s++;
                    }
                if ((s) && ((s = strchr(s, '\n')) != NULL)) {
                    s++;
                    while (*s != '\0')
                        *(t++) = *(s++);
                } else
                    total_len--;
                *t = '\0';
                RECREATE(*d->str, char, strlen(*d->str) + 3);

                sprintf(buf, "%d line%sdeleted.\r\n", total_len, ((total_len != 1) ? "s " : " "));
                SEND_TO_Q(buf, d);
            } else {
                SEND_TO_Q("������������� ��� ������� ����� ������ ��� ��������.\r\n", d);
                return;
            }
            break;
        case PARSE_LIST_NORM:
            /*
             * Note: Rv's buf, buf1, buf2, and arg variables are defined to 32k so
             * they are probly ok for what to do here.
             */
            *buf = '\0';
            if (*string != '\0')
                switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
                    case 0:
                        line_low = 1;
                        line_high = 999999;
                        break;
                    case 1:
                        line_high = line_low;
                        break;
            } else {
                line_low = 1;
                line_high = 999999;
            }

            if (line_low < 1) {
                SEND_TO_Q("��������� ����� ������������� ��� 0.\r\n", d);
                return;
            } else if (line_high < line_low) {
                SEND_TO_Q("�������� ��������.\r\n", d);
                return;
            }
            *buf = '\0';
            if ((line_high < 999999) || (line_low > 1))
                sprintf(buf, "������� �������� [%d - %d]:\r\n", line_low, line_high);
            i = 1;
            total_len = 0;
            s = *d->str;
            while (s && (i < line_low))
                if ((s = strchr(s, '\n')) != NULL) {
                    i++;
                    s++;
                }
            if ((i < line_low) || (s == NULL)) {
                SEND_TO_Q("������(�) ��� ��������� - ���������������.\r\n", d);
                return;
            }
            t = s;
            while (s && (i <= line_high))
                if ((s = strchr(s, '\n')) != NULL) {
                    i++;
                    total_len++;
                    s++;
                }
            if (s) {
                temp = *s;
                *s = '\0';
                strcat(buf, t);
                *s = temp;
            } else
                strcat(buf, t);
            /*
             * This is kind of annoying...but some people like it.
             */
#if 0
            sprintf(buf, "%s\r\n�������� ����� - %d.\r\n", buf, total_len);
#endif
            page_string(d, buf, TRUE);
            break;
        case PARSE_LIST_NUM:
            /*
             * Note: Rv's buf, buf1, buf2, and arg variables are defined to 32k so
             * they are probly ok for what to do here.
             */
            *buf = '\0';
            if (*string != '\0')
                switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
                    case 0:
                        line_low = 1;
                        line_high = 999999;
                        break;
                    case 1:
                        line_high = line_low;
                        break;
            } else {
                line_low = 1;
                line_high = 999999;
            }

            if (line_low < 1) {
                SEND_TO_Q("����� ������ ������ ���� ������ 0.\r\n", d);
                return;
            }
            if (line_high < line_low) {
                SEND_TO_Q("�������� ��������.\r\n", d);
                return;
            }
            *buf = '\0';
            i = 1;
            total_len = 0;
            s = *d->str;
            while (s && (i < line_low))
                if ((s = strchr(s, '\n')) != NULL) {
                    i++;
                    s++;
                }
            if ((i < line_low) || (s == NULL)) {
                SEND_TO_Q("������(�) ��� ��������� - ���������������.\r\n", d);
                return;
            }
            t = s;
            while (s && (i <= line_high))
                if ((s = strchr(s, '\n')) != NULL) {
                    i++;
                    total_len++;
                    s++;
                    temp = *s;
                    *s = '\0';
                    sprintf(buf, "%s%4d:\r\n", buf, (i - 1));
                    strcat(buf, t);
                    *s = temp;
                    t = s;
                }
            if (s && t) {
                temp = *s;
                *s = '\0';
                strcat(buf, t);
                *s = temp;
            } else if (t)
                strcat(buf, t);
            /*
             * This is kind of annoying .. seeing as the lines are numbered.
             */
#if 0
            sprintf(buf, "%s\r\n����������� ����� - %d.\r\n", buf, total_len);
#endif
            page_string(d, buf, TRUE);
            break;

        case PARSE_INSERT:
            half_chop(string, buf, buf2);
            if (*buf == '\0') {
                SEND_TO_Q("�� ������ ������� ����� ������, ����� ������� �������� �����.\r\n", d);
                return;
            }
            line_low = atoi(buf);
            strcat(buf2, "\r\n");

            i = 1;
            *buf = '\0';
            if ((s = *d->str) == NULL) {
                SEND_TO_Q("����� ���� - ������ �� ���������.\r\n", d);
                return;
            }
            if (line_low > 0) {
                while (s && (i < line_low))
                    if ((s = strchr(s, '\n')) != NULL) {
                        i++;
                        s++;
                    }
                if ((i < line_low) || (s == NULL)) {
                    SEND_TO_Q("����� ������ ��� ��������� - ��������.\r\n", d);
                    return;
                }
                temp = *s;
                *s = '\0';
                if ((strlen(*d->str) + strlen(buf2) + strlen(s + 1) + 3) > d->max_str) {
                    *s = temp;
                    SEND_TO_Q("���������� �������� ������ - ��������.\r\n", d);
                    return;
                }
                if (*d->str && (**d->str != '\0'))
                    strcat(buf, *d->str);
                *s = temp;
                strcat(buf, buf2);
                if (s && (*s != '\0'))
                    strcat(buf, s);
                RECREATE(*d->str, char, strlen(buf) + 3);

                strcpy(*d->str, buf);
                SEND_TO_Q("������ ���������.\r\n", d);
            } else {
                SEND_TO_Q("����� ������ ������ ���� ������ 0.\r\n", d);
                return;
            }
            break;

        case PARSE_EDIT:
            half_chop(string, buf, buf2);
            if (*buf == '\0') {
                SEND_TO_Q("�� ������ ������� ����� ������ ����������� ������.\r\n", d);
                return;
            }
            line_low = atoi(buf);
            strcat(buf2, "\r\n");

            i = 1;
            *buf = '\0';
            if ((s = *d->str) == NULL) {
                SEND_TO_Q("����� ���� - ��������� �� ���������.\r\n", d);
                return;
            }
            if (line_low > 0) { /*
                                 * Loop through the text counting \\n characters until we get to the line/
                                 */
                while (s && (i < line_low))
                    if ((s = strchr(s, '\n')) != NULL) {
                        i++;
                        s++;
                    }
                /*
                 * Make sure that there was a THAT line in the text.
                 */
                if ((i < line_low) || (s == NULL)) {
                    SEND_TO_Q("������ ��� ��������� - ��������.\r\n", d);
                    return;
                }
                /*
                 * If s is the same as *d->str that means I'm at the beginning of the
                 * message text and I don't need to put that into the changed buffer.
                 */
                if (s != *d->str) {     /*
                                         * First things first .. we get this part into the buffer.
                                         */
                    temp = *s;
                    *s = '\0';
                    /*
                     * Put the first 'good' half of the text into storage.
                     */
                    strcat(buf, *d->str);
                    *s = temp;
                }
                /*
                 * Put the new 'good' line into place.
                 */
                strcat(buf, buf2);
                if ((s = strchr(s, '\n')) != NULL) {    /*
                                                         * This means that we are at the END of the line, we want out of
                                                         * there, but we want s to point to the beginning of the line
                                                         * AFTER the line we want edited
                                                         */
                    s++;
                    /*
                     * Now put the last 'good' half of buffer into storage.
                     */
                    strcat(buf, s);
                }
                /*
                 * Check for buffer overflow.
                 */
                if (strlen(buf) > d->max_str) {
                    SEND_TO_Q("���������� ������������� ������� ������ - ��������.\r\n", d);
                    return;
                }
                /*
                 * Change the size of the REAL buffer to fit the new text.
                 */
                RECREATE(*d->str, char, strlen(buf) + 3);
                strcpy(*d->str, buf);
                SEND_TO_Q("������ ��������.\r\n", d);
            } else {
                SEND_TO_Q("����� ������ ������ ���� ������ 0.\r\n", d);
                return;
            }
            break;
        default:
            SEND_TO_Q("�������� �����.\r\n", d);
            mudlog("SYSERR: invalid command passed to parse_action", BRF, LVL_IMPL, TRUE);
            return;
    }
    //log("[PA] Stop");
}




/* Add user input to the 'current' string (as defined by d->str) */
void string_add(struct descriptor_data *d, char *str)
{
    int terminator = 0, action = 0;
    register int i = 2, j = 0;
    char actions[MAX_INPUT_LENGTH];

    /* determine if this is the terminal string, and truncate if so */
    /* changed to only accept '@' at the beginning of line - J. Elson 1/17/94 */
    /* Changed to accept '/<letter>' style editing commands - instead */
    /*   of solitary '@' to end. - M. Scott 10/15/96 */

    delete_doubledollar(str);

#if 0
    /* Removed old handling of '@' character, put #if 1 to re-enable it. */
    if ((terminator = (*str == '@')))
        *str = '\0';
#endif

    //log("[SA] Start<%s>", str);
    smash_tilde(str);
    if (!d->str)
        return;

    if (!IS_NPC(d->character))
        log("<%6d> %s editor [%s]",
            GET_ROOM_VNUM(IN_ROOM(d->character)), GET_NAME(d->character), str);

    if ((action = (*str == '/'))) {
        while (str[i] != '\0') {
            actions[j] = str[i];
            i++;
            j++;
        }
        actions[j] = '\0';
        *str = '\0';
        switch (str[1]) {
            case 'a':
                terminator = 2; /* Working on an abort message, */
                break;
            case 'c':
                if (*(d->str)) {
                    free(*d->str);
                    *(d->str) = NULL;
                    SEND_TO_Q("����� ������.\r\n", d);
                } else
                    SEND_TO_Q("����� ����.\r\n", d);
                break;
            case 'd':
                parse_action(PARSE_DELETE, actions, d);
                break;
            case 'e':
                parse_action(PARSE_EDIT, actions, d);
                break;
            case 'f':
                if (*(d->str))
                    parse_action(PARSE_FORMAT, actions, d);
                else
                    SEND_TO_Q("����� ����.\r\n", d);
                break;
            case 'i':
                if (*(d->str))
                    parse_action(PARSE_INSERT, actions, d);
                else
                    SEND_TO_Q("����� ����.\r\n", d);
                break;
            case 'h':
                parse_action(PARSE_HELP, actions, d);
                break;
            case 'l':
                if (*(d->str))
                    parse_action(PARSE_LIST_NORM, actions, d);
                else
                    SEND_TO_Q("����� ����.\r\n", d);
                break;
            case 'n':
                if (*(d->str))
                    parse_action(PARSE_LIST_NUM, actions, d);
                else
                    SEND_TO_Q("����� ����.\r\n", d);
                break;
            case 'r':
                parse_action(PARSE_REPLACE, actions, d);
                break;
            case 's':
                terminator = 1;
                *str = '\0';
                break;
            default:
                SEND_TO_Q("���������� �����. � � ��� �������...\r\n", d);
                break;
        }
    }

    if (!(*d->str)) {           //log("[SA] No str s");
        if (strlen(str) + 3 > d->max_str) {     /* \r\n\0 */
            send_to_char("������� ������� ������ - �������.\r\n", d->character);
            strcpy(&str[d->max_str - 3], "\r\n");
            CREATE(*d->str, char, d->max_str);

            strcpy(*d->str, str);

            /* Changed this to NOT abort out.. just give warning. */
            /* terminator = 1;                                    */
        } else {
            CREATE(*d->str, char, strlen(str) + 3);

            strcpy(*d->str, str);
        }
        //log("[SA] No str f");
    } else {                    //log("[SA] 1s");
        if (strlen(str) + strlen(*d->str) + 3 > d->max_str) {   /* \r\n\0 */
            //log("[SA] 1.1");
            send_to_char("������� ������� ��������. ��������� ������ ��������������.\r\n",
                         d->character);
            action = TRUE;
            /* terminator = 1; */
        } else {                //log("[SA] 1.2");
            RECREATE(*d->str, char, strlen(*d->str) + strlen(str) + 3); /* \r\n\0 */

            strcat(*d->str, str);
        }
        //log("[SA] 1f");
    }

    if (terminator) {
        /*
         * Here we check for the abort option and reset the pointers.
         */
        if (terminator == 2 && STATE(d) == CON_EXDESC) {        //log("[SA] 2s");
            if (*(d->str))
                free(*d->str);
            if (d->backstr) {
                *d->str = d->backstr;
            } else
                *d->str = NULL;
            d->backstr = NULL;
            d->str = NULL;
            //log("[SA] 2f");
        }
        /*
         * This fix causes the editor to NULL out empty messages -- M. Scott
         * Fixed to fix the fix for empty fixed messages. -- gg
         */
        else if ((d->str) && (*d->str) && (**d->str == '\0')) { //log("[SA] 3s");
            free(*d->str);
            *d->str = str_dup("\r\n");
            //log("[SA] 3f");
        }


        if (!d->connected
            && (PLR_FLAGGED(d->character, PLR_MAILING)
                || PLR_FLAGGED(d->character, PLR_SCRIPTING))) {
            if (PLR_FLAGGED(d->character, PLR_MAILING)) {
                if ((terminator == 1) && *d->str) {
                    store_mail(d->mail_to, GET_IDNUM(d->character), *d->str);
                    SEND_TO_Q("� �������� ���� ������ ��������!\r\n", d);
                } else
                    SEND_TO_Q("������ �������.\r\n", d);
                d->mail_to = 0;
            }
            if (PLR_FLAGGED(d->character, PLR_SCRIPTING)) {
                if ((terminator == 1) && *d->str) {
                    cs_post(d->character, *d->str);
                } else {
                    cs_cancel(d->character);
                }
            }

            if (*(d->str))
                free(*d->str);
            if (d->str)
                free(d->str);
        } else if (d->mail_to >= BOARD_MAGIC) { //log("[SA] 6s");
            Board_save_board(d->mail_to - BOARD_MAGIC);
            SEND_TO_Q("�������� ���������. ����������� �������� <����� ��������>.\r\n", d);
            d->mail_to = 0;
            //log("[SA] 6f");
        } else if (STATE(d) == CON_EXDESC) {    //log("[SA] 7s");
            if (terminator != 1)
                SEND_TO_Q("�������� �������� ��������.\r\n", d);
            else
                save_char(d->character, GET_LOADROOM(d->character));
            SEND_TO_Q(MENU, d);
            d->connected = CON_MENU;
            //log("[SA] 7f");
        } else if (!d->connected && d->character && !IS_NPC(d->character)) {
            if (terminator == 1) {      //log("[SA] 8s");
                if (*(d->str) && strlen(*d->str) == 0) {
                    free(*d->str);
                    *d->str = NULL;
                }
                //log("[SA] 8f");
            } else {            //log("[SA] 9s");
                if (*(d->str))
                    free(*d->str);
                if (d->backstr)
                    *d->str = d->backstr;
                else
                    *d->str = NULL;
                d->backstr = NULL;
                SEND_TO_Q("��������� ��������.\r\n", d);
                //log("[SA] 9f");
            }
        }
        //log("[SA] 10s");
        if (d->character && !IS_NPC(d->character)) {
            REMOVE_BIT(PLR_FLAGS(d->character, PLR_WRITING), PLR_WRITING);
            REMOVE_BIT(PLR_FLAGS(d->character, PLR_SCRIPTING), PLR_SCRIPTING);
            REMOVE_BIT(PLR_FLAGS(d->character, PLR_MAILING), PLR_MAILING);
        }
        if (d->backstr)
            free(d->backstr);
        d->backstr = NULL;
        d->str = NULL;
        //log("[SA] 10f");
    } else if (!action)
        strcat(*d->str, "\r\n");
    //log("[SA] Stop");
}




/*********************************************************************
* New Pagination Code
* Michael Buselli submitted the following code for an enhanced pager
* for CircleMUD.  All functions below are his.  --JE 8 Mar 96
*
*********************************************************************/

#define PAGE_LENGTH     (ch->sh)
#define PAGE_WIDTH      (ch->sw)

/* Traverse down the string until the begining of the next page has been
 * reached.  Return NULL if this is the last page of the string.
 */
char *next_page(char *str, struct char_data *ch)
{
    int col = 1, line = 1, spec_code = FALSE;
    const char *color;

    for (;; str++) {            /* If end of string, return NULL. */
        if (*str == '\0')
            return (NULL);

        else if (line > PAGE_LENGTH)
            return (str);
        /* Check for the begining of an ANSI color code block. */
        else if (*str == '$' && !strncmp(str + 1, "COLOR", 5)) {
            if (!ch)
                color = KNRM;
            else
                switch (*(str + 6)) {
                    case 'N':
                        color = (char *) (clr((ch), (C_NRM)) ? KIDRK : KNRM);
                        break;
                    case 'n':
                        color = (char *) (clr((ch), (C_NRM)) ? KNRM : KNRM);
                        break;
                    case 'R':
                        color = (char *) (clr((ch), (C_NRM)) ? KIRED : KNRM);
                        break;
                    case 'r':
                        color = (char *) (clr((ch), (C_NRM)) ? KRED : KNRM);
                        break;
                    case 'G':
                        color = (char *) (clr((ch), (C_NRM)) ? KIGRN : KNRM);
                        break;
                    case 'g':
                        color = (char *) (clr((ch), (C_NRM)) ? KGRN : KNRM);
                        break;
                    case 'Y':
                        color = (char *) (clr((ch), (C_NRM)) ? KIYEL : KNRM);
                        break;
                    case 'y':
                        color = (char *) (clr((ch), (C_NRM)) ? KYEL : KNRM);
                        break;
                    case 'B':
                        color = (char *) (clr((ch), (C_NRM)) ? KIBLU : KNRM);
                        break;
                    case 'b':
                        color = (char *) (clr((ch), (C_NRM)) ? KBLU : KNRM);
                        break;
                    case 'M':
                        color = (char *) (clr((ch), (C_NRM)) ? KIMAG : KNRM);
                        break;
                    case 'm':
                        color = (char *) (clr((ch), (C_NRM)) ? KMAG : KNRM);
                        break;
                    case 'C':
                        color = (char *) (clr((ch), (C_NRM)) ? KICYN : KNRM);
                        break;
                    case 'c':
                        color = (char *) (clr((ch), (C_NRM)) ? KCYN : KNRM);
                        break;
                    case 'W':
                        color = (char *) (clr((ch), (C_NRM)) ? KIDRK : KNRM);
                        break;
                    case 'w':
                        color = (char *) (clr((ch), (C_NRM)) ? KWHT : KNRM);
                        break;
                    default:
                        color = (char *) KNRM;
                }
            strncpy(str, color, strlen(color));
            str += (strlen(color) - 1);
        } else if (*str == '\x1B' && !spec_code)
            spec_code = TRUE;

        /* Check for the end of an ANSI color code block. */
        else if (*str == 'm' && spec_code)
            spec_code = FALSE;

        /* Check for everything else. */
        else if (!spec_code) {  /* Carriage return puts us in column one. */
            if (*str == '\r')
                col = 1;
            /* Newline puts us on the next line. */
            else if (*str == '\n')
                line++;

            /* We need to check here and see if we are over the page width,
             * and if so, compensate by going to the begining of the next line.
             */
            /*           else
               if (col++ > PAGE_LENGTH)
               {col = 1;
               line++;
               } */
        }
        /* If we're at the start of the next page, return this fact. */
    }
}


/* Function that returns the number of pages in the string. */
int count_pages(char *str, struct char_data *ch)
{
    int pages;

    for (pages = 1; (str = next_page(str, ch)); pages++);
    return (pages);
}


/* This function assigns all the pointers for showstr_vector for the
 * page_string function, after showstr_vector has been allocated and
 * showstr_count set.
 */
void paginate_string(char *str, struct descriptor_data *d)
{
    int i;


    if (d->showstr_count)
        *(d->showstr_vector) = str;

    for (i = 1; i < d->showstr_count && str; i++)
        str = d->showstr_vector[i] = next_page(str, d->character);
    d->showstr_page = 0;
}


/* The call that gets the paging ball rolling... */
void page_string(struct descriptor_data *d, const char *str, int keep_internal)
{
    char actbuf[MAX_INPUT_LENGTH] = "";
    static char strz[MAX_EXTEND_LENGTH];

    strcpy(strz, str);

    if (!d)
        return;

    if (!*strz) {
        send_to_char("", d->character);
        return;
    }
    d->showstr_count = count_pages(strz, d->character);
    CREATE(d->showstr_vector, char *, d->showstr_count);

    d->showstr_head = str_dup(strz);
    paginate_string(d->showstr_head, d);

    show_string(d, actbuf);
}


/* The call that displays the next page. */
void show_string(struct descriptor_data *d, char *input)
{
    char buffer[MAX_EXTEND_LENGTH];
    int diff;
    char buf[MAX_EXTEND_LENGTH];

    *buf = '\0';
    *buffer = '\0';

    any_one_arg(input, buf);

    /* Q is for quit.:) */
    if (LOWER(*buf) == 'q' || LOWER(*buf) == '�') {
        free(d->showstr_vector);
        d->showstr_count = 0;
        if (d->showstr_head) {
            free(d->showstr_head);
            d->showstr_head = NULL;
        }
        return;
    }
    /* R is for refresh, so back up one page internally so we can display
     * it again.
     */
    else if (LOWER(*buf) == 'r' || LOWER(*buf) == '�')
        d->showstr_page = MAX(0, d->showstr_page - 1);

    /* B is for back, so back up two pages internally so we can display the
     * correct page here.
     */
    else if (LOWER(*buf) == 'b' || LOWER(*buf) == '�')
        d->showstr_page = MAX(0, d->showstr_page - 2);

    /* Feature to 'goto' a page.  Just type the number of the page and you
     * are there!
     */
    else if (IS_DIGIT(*buf))
        d->showstr_page = MAX(0, MIN(atoi(buf) - 1, d->showstr_count - 1));

    else if (*buf) {
        send_to_char
            ("&n���������� �������: [enter] ������, [�] �����, [�] ���������, ��� NN ��������.\r\n",
             d->character);
        return;
    }
    /* If we're displaying the last page, just send it to the character, and
     * then free up the space we used.
     */
    if (d->showstr_page + 1 >= d->showstr_count) {
        send_to_char(d->showstr_vector[d->showstr_page], d->character);
        free(d->showstr_vector);
        d->showstr_count = 0;
        if (d->showstr_head) {
            free(d->showstr_head);
            d->showstr_head = NULL;
        }
    }
    /* Or if we have more to show.... */
    else {
        diff = d->showstr_vector[d->showstr_page + 1] - d->showstr_vector[d->showstr_page];
        if (diff >= MAX_STRING_LENGTH)
            diff = MAX_STRING_LENGTH - 1;
        strncpy(buffer, d->showstr_vector[d->showstr_page], diff);
        buffer[diff] = '\0';
        send_to_char(buffer, d->character);
        d->showstr_page++;
    }
}

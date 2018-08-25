/* ************************************************************************
*   File: act.comm.c                                    Part of CircleMUD *
*  Usage: Player-level communication commands                             *
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
#include "ban.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "screen.h"
#include "spells.h"
#include "case.h"
#include "xboot.h"

/* local functions */
void perform_tell(struct char_data *ch, struct char_data *vict, char *arg);
int is_tell_ok(struct char_data *ch, struct char_data *vict);
int do_social(struct char_data *ch, char *argument);

ACMD(do_languages);
ACMD(do_say);
ACMD(do_gsay);
ACMD(do_tell);
ACMD(do_reply);
ACMD(do_remem);
ACMD(do_spec_comm);
ACMD(do_write);
ACMD(do_page);
ACMD(do_gen_comm);
ACMD(do_qcomm);
ACMD(do_remembertell);
ACMD(do_appeal);
ACMD(do_listen);

#define SIELENCE ("��� ��� �����!.\r\n")

const char *languages[] = {
    "��������",
    "�������",
    "������",
    "����������",
    "������������",
    "�����",
    "�������",
    "������",
    "\n"
};

const char *languages_o[] = {
    "�� ��������",
    "�� �������",
    "�� �������",
    "�� ����������",
    "�� �����������",
    "�� �����",
    "�� ��������",
    "�� �������",
    "\n"
};

const char *languages_d[] = {
    "�� �������� �����",
    "�� ������� �����",
    "�� ������� �����",
    "�� ���������� �����",
    "�� ������������ �����",
    "�� �����",
    "�� ��������",
    "�� �������",
    "\n"
};

void list_languages(struct char_data *ch)
{
    int a = 0, i;
    char buf[MAX_STRING_LENGTH];

    if (SPEAKING(ch) <= MIN_LANGUAGES) {
        SET_SKILL(ch, MIN_LANGUAGES) = 100;
        SPEAKING(ch) = MIN_LANGUAGES;
    }


    /* Slight hack to avoid core dumping cause no one speaks the
     * language after implementation. */

    sprintf(buf, "�� ������ ��������� �����:\r\n");
    for (i = MIN_LANGUAGES; i <= MAX_LANGUAGES; i++) {
        if (GET_SKILL(ch, i) > 1) {
            strcat(buf, SPEAKING(ch) == i ? "&B" : "&n");
            sprintf(buf, "%s %-20s", buf, languages[i - MIN_LANGUAGES]);
            sprintf(buf, "%s �� %d ���������.", buf, GET_SKILL(ch, i));
            strcat(buf, "\r\n");
            a = 1;
        }
    }
    if (a == 0)
        send_to_char("�� �� ������ �� ������ �����.\r\n", ch);

    send_to_char(buf, ch);
}

ACMD(do_languages)
{
    int i, found = FALSE;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    one_argument(argument, arg);
    if (!*arg)
        list_languages(ch);
    else {
        for (i = MIN_LANGUAGES; i <= MAX_LANGUAGES; i++) {
            if (isname(arg, languages[i - MIN_LANGUAGES]) && GET_SKILL(ch, i) > 80) {
                SPEAKING(ch) = i;
                sprintf(buf, "�� ������ �������� %s.\r\n", languages_o[i - MIN_LANGUAGES]);
                send_to_char(buf, ch);
                found = TRUE;
                break;
            }
        }
        if (!found)
            list_languages(ch);
    }
}



void garble_text(char *string, int percent, int lang)
{
    char letters[33] = "";
    int i, s = 0, need = 1;


    switch (lang) {
        case SKILL_LANG_HUMAN:
            strcpy(letters, "��������������������������������'");
            s = 32;
            break;
        case SKILL_LANG_ORC:
        case SKILL_LANG_BARIAUR:
        case SKILL_LANG_DWARN:
            strcpy(letters, "���������`");
            s = 9;
            break;
        case SKILL_LANG_AASIMAR:
            strcpy(letters, "���������'");
            s = 9;
            break;
        case SKILL_LANG_ELF:
            strcpy(letters, "�����������������������'");
            s = 24;
            break;
        case SKILL_LANG_TIEFLING:
            strcpy(letters, "����������.");
            s = 10;
            break;
        default:
            need = 0;
            break;
    }

    if (need == 1)
        for (i = 0; i < (int) strlen(string); ++i)
            if (!(string[i] == ' ') && (number(0, 100) > percent))
                string[i] = letters[number(0, s)];
}

/* Invoke Fenia speech progs for all non-deaf listeners in the same room.
 * Output can be garbled for PCs. 
 * Invoked immediately after all output has been done.
 */
typedef std::map<struct char_data *, string> HeardMessages; 

static void mprog_speech(struct char_data *talker, const HeardMessages &messages)
{
    for (HeardMessages::const_iterator m = messages.begin( ); m != messages.end( ); m++) {
        struct char_data *listener = m->first;
        const char *msg = m->second.c_str( );
        FENIA_VOID_CALL(listener, "Speech", "Cs", talker, msg);
        FENIA_PROTO_VOID_CALL(listener->npc(), "Speech", "CCs", listener, talker, msg);
    }    
}


ACMD(do_say)
{
    char ibuf[MAX_RAW_INPUT_LENGTH];
    char obuf[MAX_RAW_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct mess_p_data *k, *found = NULL;

    struct char_data *tch;
    int speak_ch = GET_SPEAKING(ch);

    skip_spaces(&argument);

    if (GET_RACE(ch) == RACE_ANIMAL)
        return;

    if (AFF_FLAGGED(ch, AFF_SIELENCE)) {
        send_to_char(SIELENCE, ch);
        return;
    }

    if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DUMB)) {
        send_to_char("� ��� ��� ���!\r\n", ch);
        return;
    }


    if (!*argument) {
        send_to_char("��� �� ������ �������?\r\n", ch);
        return;
    };


    strcpy(ibuf, argument);     /* ��������� ������������ ����� */

    /*  if (!IS_NPC(ch))
       garble_text(ibuf, GET_SKILL(ch, SPEAKING(ch)), SPEAKING(ch)); */

    // Keep track of all messages sent to every person in the room,
    // to use in Fenia triggers. 
    HeardMessages messages;

    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
        if (tch != ch && AWAKE(tch) && (tch->desc || tch->mess_data || tch->hasWrapper())) {
            for (k = tch->mess_data; !found && k; k = k->next) {
                if (k->command == CMD_SAY) {
                    found = k;
                    break;
                }
            }
            strcpy(obuf, ibuf); /* preserve the first garble */

            if (!IS_NPC(ch) && !IS_NPC(tch) && !IS_GOD(ch) && !IS_GOD(tch))
                garble_text(obuf, GET_SKILL(tch, speak_ch), speak_ch);

            if (!IS_NPC(tch) && !PRF_FLAGGED(tch, PRF_CURSES))
                curses_check(obuf);

            if (IS_GOD(ch)) 
                act("1� ��������1(,��,��,��): '&W%1&n'", "���", ch, tch, obuf);
            else if (IS_AFFECTED(tch, AFF_DEAFNESS)) {
                act("1� ������� ������.", "��", ch, tch);
                continue;
            }
            else if (found && found->mess_to_vict)
                act(found->mess_to_vict, "���", ch, tch, obuf);
            else {
                act("1� ��������1(,��,��,��): '&c%1&n'", "���", ch, tch, obuf);
            }

            messages[tch] = obuf;
        }
    }

    if (found && found->mess_to_char)
        act(found->mess_to_char, "��", ch, argument);
    else if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char(OK, ch);
    else {
        delete_doubledollar(argument);
        sprintf(buf, "�� ����������: '&c%s&n'\r\n", argument);
        send_to_char(buf, ch);
    }

    if (found && found->script) {
        if (found->sarg && !strncmp(found->sarg, argument, strlen(found->sarg))) {
            go_script(found->script, ch, argument);
        } else if (!found->sarg) {
            go_script(found->script, ch, argument);
        }
    }
    
    mprog_speech(ch, messages);
}

ACMD(do_appeal)
{
    struct descriptor_data *d;
    int level = LVL_GOD;
    char buf[MAX_STRING_LENGTH];

    if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
        return;

    if (PLR_FLAGGED(ch, PLR_DUMB) || PLR_FLAGGED(ch, PLR_MUTE)) {
        send_to_charf(ch, "������� �� �������� ������� ���.\r\n");
        return;
    }

    skip_spaces(&argument);
    if (!*argument) {
        send_to_charf(ch, "� ����� �������� �� ������ ��������?\r\n");
        return;
    }
    delete_doubledollar(argument);

    sprintf(buf, "%s �������%s: '%s'\r\n", GET_NAME(ch), GET_CH_SUF_1(ch), argument);
    send_to_charf(ch, "��� ����� ������ � ��������� �������!\r\n");

    for (d = descriptor_list; d; d = d->next) {
        if ((STATE(d) == CON_PLAYING) &&
            (GET_LEVEL(d->character) >= level) &&
            (!PRF_FLAGGED(d->character, PRF_NOWIZ)) &&
            (!PLR_FLAGGED(d->character, PLR_WRITING)) &&
            (!PLR_FLAGGED(d->character, PLR_MAILING)) &&
            (!PLR_FLAGGED(d->character, PLR_SCRIPTING)) &&
            (d != ch->desc || !(PRF_FLAGGED(d->character, PRF_NOREPEAT)))) {
            send_to_char(CCCYN(d->character, C_NRM), d->character);
            send_to_char(buf, d->character);
            send_to_char(CCNRM(d->character, C_NRM), d->character);
        }
    }

}

ACMD(do_gsay)
{
    char ibuf[MAX_RAW_INPUT_LENGTH];
    char obuf[MAX_RAW_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int speak_ch = GET_SPEAKING(ch);
    struct char_data *k;
    struct follow_type *f;

    if (GET_RACE(ch) == RACE_ANIMAL)
        return;

    if (AFF_FLAGGED(ch, AFF_SIELENCE)) {
        send_to_char(SIELENCE, ch);
        return;
    }

    if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DUMB)) {
        send_to_char("� ��� ��� ���!\r\n", ch);
        return;
    }

    skip_spaces(&argument);

    if (!AFF_FLAGGED(ch, AFF_GROUP)) {
        send_to_char("�� �� ��������� ������ ������!\r\n", ch);
        return;
    }
    if (!*argument)
        send_to_char("� ��� �� ������ ������� ������?\r\n", ch);
    else {
        if (ch->party_leader)
            k = ch->party_leader;
        else
            k = ch;

        strcpy(ibuf, argument); /* ��������� ������������ ����� */



        sprintf(buf, "%s ������%s ������ %s: %s", GET_NAME(ch), GET_CH_SUF_1(ch), GET_PAD(k, 1),
                argument);
        if (!IS_MOB(ch))
            speaklog(buf, LVL_IMPL, TRUE);

        //��������� ������
        if (AFF_FLAGGED(k, AFF_GROUP) && (k != ch)) {

            strcpy(obuf, ibuf);
            garble_text(obuf, GET_SKILL(k, speak_ch), speak_ch);
            if (!IS_NPC(k) && !PRF_FLAGGED(k, PRF_CURSES))
                curses_check(obuf);

            send_to_charf(k, CCIGRN(k, C_NRM));
            act("1� ������1(,�,�,�) ������: '%1'.&n", "�!��", ch, k, obuf);
            send_to_charf(k, CCNRM(k, C_NRM));
        }
        //����������
        for (f = k->party; f; f = f->next)
            if (AFF_FLAGGED(f->follower, AFF_GROUP) && (f->follower != ch)) {
                strcpy(obuf, ibuf);     /* preserve the first garble */
                if (!IS_NPC(ch) && !IS_NPC(f->follower) && !IS_GOD(ch) && !IS_GOD(f->follower))
                    garble_text(obuf, GET_SKILL(f->follower, speak_ch), speak_ch);
                if (!IS_NPC(f->follower) && !PRF_FLAGGED(f->follower, PRF_CURSES))
                    curses_check(obuf);

                if (IS_AFFECTED(f->follower, AFF_DEAFNESS)) {
                    sprintf(buf, "$n ���-�� ������$g ������, �� �� ������ �� ��������.");
                } else {
                    send_to_charf(f->follower, CCIGRN(f->follower, C_NRM));
                    sprintf(buf, "$n ������$g ������: '%s'.", obuf);
                    act(buf, FALSE, ch, 0, f->follower, TO_VICT | TO_SLEEP);
                    send_to_charf(f->follower, CCNRM(f->follower, C_NRM));
                }
            }

        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else
            send_to_charf(ch, "%s�� ������� ������: '%s'%s\r\n",
                          CCIGRN(ch, C_NRM), argument, CCNRM(ch, C_NRM));
    }
}


void perform_tell(struct char_data *ch, struct char_data *vict, char *arg)
{
    char ibuf[MAX_RAW_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    strcpy(ibuf, arg);          /* ��������� ������������ ����� */

    sprintf(buf, "%s ������%s %s: %s", GET_NAME(ch), GET_CH_SUF_1(ch), GET_PAD(vict, 2), arg);
    if (!IS_MOB(ch))
        speaklog(buf, LVL_GRGOD, TRUE);

    if (!IS_NPC(ch) && !IS_NPC(vict) && !IS_GOD(ch) && !IS_GOD(vict))
        garble_text(ibuf, GET_SKILL(vict, SPEAKING(ch)), SPEAKING(ch));

    if (!IS_NPC(vict) && !PRF_FLAGGED(vict, PRF_CURSES))
        curses_check(ibuf);

    if (IS_NPC(ch))
        act("1� ������1(,�,�,�) ���: '%1'", "�!��", ch, vict, ibuf);
    else
        act("1� ������1(,�,�,�) ���: '%2%1%3'", "�!����", ch, vict, ibuf, CCICYN(vict, C_NRM),
            CCNRM(vict, C_NRM));


    /* ��������� ��� "���������" */
    arg[MAX_INPUT_LENGTH - 1] = 0;

    if (!IS_NPC(vict) && !IS_NPC(ch)) {
        if (CAN_SEE(vict, ch)) {
            sprintf(buf, "%s: '%s'", GET_NAME(ch), ibuf);
        } else {
            sprintf(buf, "���-��: '%s'", ibuf);
        }
        strcpy(GET_TELL(vict, GET_LASTTELL(vict)), buf);
        GET_LASTTELL(vict)++;
        if (GET_LASTTELL(vict) == MAX_REMEMBER_TELLS)
            GET_LASTTELL(vict) = 0;
    }

    if (!IS_NPC(ch) && PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char(OK, ch);
    else {
        sprintf(buf, "�� ������� $N2: '%s%s%s'", CCICYN(ch, C_CMP), arg, CCNRM(ch, C_CMP));
        act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    }

    if (!IS_NPC(vict) && !IS_NPC(ch))
        GET_LAST_TELL(vict) = GET_IDNUM(ch);

}

int is_tell_ok(struct char_data *ch, struct char_data *vict)
{

    if (GET_RACE(ch) == RACE_ANIMAL)
        return (FALSE);


    if (ch == vict) {
        send_to_char("���� ��� � ����� �� ������ ������.\r\n", ch);
        return (FALSE);
    }
    if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DUMB)) {
        send_to_char("� ��� ��� ���!\r\n", ch);
        return (FALSE);
    }
    if (!IS_NPC(vict) && !vict->desc) { /* linkless */
        act("$N ��������$U � �����, �������� ������������ ����������������.", TRUE, ch, 0, vict,
            TO_CHAR | TO_SLEEP);
        return (FALSE);
    }
    if (PLR_FLAGGED(vict, PLR_WRITING)) {
        act("$N ����� ��������� - ��������� �������.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
        return (FALSE);
    }

    if (IS_GOD(ch))
        return (TRUE);

    if (((!IS_NPC(vict) && PRF_FLAGGED(vict, PRF_NOTELL) && !IS_GOD(ch)) ||
         ROOM_FLAGGED(vict->in_room, ROOM_SOUNDPROOF)) && !IS_GOD(vict)) {
        act("$N ��� �� �������.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
        return (FALSE);
    }

    if (!IS_NPC(ch) && !NAME_GOD(ch) && !IS_IMMORTAL(vict)) {
        act("��������� � ��������������������� ������� ����� �������� ������ � ������������.",
            FALSE, ch, 0, vict, TO_CHAR);
        return (FALSE);
    } else if (!IS_NPC(vict) && !NAME_GOD(vict) && !IS_IMMORTAL(ch)) {
        send_to_char(NOPERSON, ch);
        return (FALSE);
    }



    if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF) && !IS_GOD(vict))
        send_to_char("����� ��������� ���� �����.\r\n", ch);
    else if (GET_POS(vict) < POS_RESTING)
        act("$N ��� �� �������.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
    else
        return (TRUE);

    return (FALSE);
}

/*
 * Yes, do_tell probably could be combined with whisper and ask, but
 * called frequently, and should IMHO be kept as tight as possible.
 */
ACMD(do_tell)
{
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    struct char_data *vict = NULL;

    if (AFF_FLAGGED(ch, AFF_SIELENCE)) {
        send_to_char(SIELENCE, ch);
        return;
    }

    if (GET_RACE(ch) == RACE_ANIMAL)
        return;


    *buf = '\0';
    *buf2 = '\0';

    half_chop(argument, buf, buf2);

    if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DUMB)) {
        send_to_char("� ��� ��� ���!\r\n", ch);
        return;
    }

    if (!*buf || !*buf2) {
        send_to_char("��� � ���� �� ������ �������?\r\n", ch);
    } else if (!IS_IMMORTAL(ch) && !(vict = get_player_vis(ch, buf, FIND_CHAR_WORLD))) {
        send_to_char(NOPERSON, ch);
    } else if (IS_IMMORTAL(ch) && !(vict = get_char_vis(ch, buf, FIND_CHAR_WORLD))) {
        send_to_char(NOPERSON, ch);
    } else if (is_tell_ok(ch, vict)) {
        if (PRF_FLAGGED(ch, PRF_NOTELL))
            send_to_char("�������� ��� �� ������!\r\n", ch);
        perform_tell(ch, vict, buf2);
    }
}


ACMD(do_reply)
{
    struct char_data *tch = character_list;

    if (IS_NPC(ch))
        return;

    if (AFF_FLAGGED(ch, AFF_SIELENCE)) {
        send_to_char(SIELENCE, ch);
        return;
    }

    if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DUMB)) {
        send_to_char("� ��� ��� ���!\r\n", ch);
        return;
    }

    skip_spaces(&argument);

    if (GET_LAST_TELL(ch) == NOBODY)
        send_to_char("��� ������ ��������!\r\n", ch);
    else if (!*argument)
        send_to_char("��� �� ����������� ��������?\r\n", ch);
    else {                      /*
                                 * Make sure the person you're replying to is still playing by searching
                                 * for them.  Note, now last tell is stored as player IDnum instead of
                                 * a pointer, which is much better because it's safer, plus will still
                                 * work if someone logs out and back in again.
                                 */

        /*
         * XXX: A descriptor list based search would be faster although
         *      we could not find link dead people.  Not that they can
         *      hear tells anyway.:) -gg 2/24/98
         */
        while (tch != NULL && ((IS_NPC(tch) || !tch->desc) || GET_IDNUM(tch) != GET_LAST_TELL(ch)))
            tch = tch->next;

        if (tch == NULL)
            send_to_char("�������� ���� � ����� � �� ������������ ����������������.\r\n", ch);
        else if (IS_GOD(tch) && GET_INVIS_LEV(tch) > GET_LEVEL(ch))
            send_to_char("��� �������� �� �������� �������� � ����.\r\n", ch);
        else if (is_tell_ok(ch, tch))
            perform_tell(ch, tch, argument);
    }
}


void ask_mob(struct char_data *ch, struct char_data *vict)
{
    char buf[MAX_STRING_LENGTH], sub_buf[MAX_STRING_LENGTH];

    act("�� ������ $N2 ������.", TRUE, ch, 0, vict, TO_CHAR);
    act("$n ����� $N2 ������.", TRUE, ch, 0, vict, TO_ROOM);

    if (CAN_SEE(vict, ch)) {
        sprintf(buf, "������ %s", GET_NAME(ch));
        do_social(vict, buf);
    } else {
        strcpy(sub_buf, "������ ���������");
        do_social(vict, sub_buf);
    }
}


ACMD(do_spec_comm)
{
    struct char_data *vict;
    const char *action_sing, *action_plur, *action_others, *vict1, *vict2;
    char vict3[MAX_INPUT_LENGTH];
    char ibuf[MAX_RAW_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    struct mess_p_data *k = NULL;
    bool found = FALSE;


    if (AFF_FLAGGED(ch, AFF_SIELENCE)) {
        send_to_char(SIELENCE, ch);
        return;
    }

    if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DUMB)) {
        send_to_char("��� ��������� ���������� � ������ �������!\r\n", ch);
        return;
    }

    if (subcmd == SCMD_WHISPER) {
        action_sing = "�������";
        vict1 = "����";
        vict2 = "���";
        action_plur = "���������";
        action_others = "$n ���-�� ���������$g $N2.";
    } else {
        action_sing = "��������";
        vict1 = "� ����";
        vict2 = "� ���";
        action_plur = "�������";
        action_others = "$n �����$g $N2 ������.";
    }

    half_chop(argument, buf, buf2);

    if (!*buf || !*buf2) {
        sprintf(buf, "��� �� ������ %s.. � %s?\r\n", action_sing, vict1);
        send_to_char(buf, ch);
    } else if (!(vict = get_char_vis(ch, buf, FIND_CHAR_ROOM)))
        send_to_char(NOPERSON, ch);
    else if (vict == ch)
        send_to_char("������ ����?\r\n", ch);
    else {
        if (subcmd == SCMD_WHISPER)
            sprintf(vict3, "%s", GET_PAD(vict, 2));
        else {
            if (IS_NPC(vict)) {
                ask_mob(ch, vict);
                return;
            }
        }


        if (subcmd == SCMD_WHISPER) {   //�������� ��� �������
            for (k = vict->mess_data; !found && k; k = k->next) {
                if (k->command == CMD_WHISPER) {
                    found = TRUE;
                    break;
                }
            }
        }

        strcpy(ibuf, buf2);     /* ��������� ������������ ����� */
        if (!IS_NPC(ch) && !IS_NPC(vict) && !IS_GOD(ch) && !IS_GOD(vict))
            garble_text(ibuf, GET_SKILL(vict, SPEAKING(ch)), SPEAKING(ch));
        if (!IS_NPC(vict) && !PRF_FLAGGED(vict, PRF_CURSES))
            curses_check(ibuf);

        if (subcmd == SCMD_WHISPER)
            sprintf(vict3, "%s", GET_PAD(vict, 2));
        else
            sprintf(vict3, "� %s", GET_PAD(vict, 1));

        sprintf(buf, "$n %s$g %s: '&K%s&n'", action_plur, vict2, ibuf);

        if (found && k && k->mess_to_vict)
            act(k->mess_to_vict, "���", ch, vict, buf);
        else
            act(buf, FALSE, ch, 0, vict, TO_VICT);

        if (found && k && k->mess_to_char)
            act(k->mess_to_char, "��", ch, argument);
        else if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else {
            sprintf(buf, "�� %s� %s: '&K%s&n'\r\n", action_plur, vict3, buf2);
            send_to_char(buf, ch);
        }

        act(action_others, FALSE, ch, 0, vict, TO_NOTVICT);


        if (found && k && k->script) {
            if (k->sarg && !strncmp(k->sarg, argument, strlen(k->sarg))) {
                go_script(k->script, ch, argument);
            } else if (!k->sarg) {
                go_script(k->script, ch, argument);
            }
        }

    }
}



#define MAX_NOTE_LENGTH 1000    /* arbitrary */

ACMD(do_write)
{
    struct obj_data *paper, *pen = NULL;
    char *papername, *penname;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    papername = buf1;
    penname = buf2;

    two_arguments(argument, papername, penname);

    if (!ch->desc)
        return;

    if (!*papername) {          /* nothing was delivered */
        send_to_char("��������?  ���?  � �� ���?\r\n", ch);
        return;
    }
    if (*penname) {             /* there were two arguments */
        if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
            sprintf(buf, "� ��� ��� %s.\r\n", papername);
            send_to_char(buf, ch);
            return;
        }
        if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
            sprintf(buf, "� ��� ��� %s.\r\n", penname);
            send_to_char(buf, ch);
            return;
        }
    } else {                    /* there was one arg.. let's see what we can find */
        if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
            sprintf(buf, "�� �� ������ %s � ���������.\r\n", papername);
            send_to_char(buf, ch);
            return;
        }
        if (GET_OBJ_TYPE(paper) == ITEM_PEN) {  /* oops, a pen.. */
            pen = paper;
            paper = NULL;
        } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
            send_to_char("�� ���� ������ ������.\r\n", ch);
            return;
        }
        /* One object was found.. now for the other one. */
        if (!GET_EQ(ch, WEAR_HOLD)) {
            sprintf(buf, "��� ����� ������!\r\n");
            send_to_char(buf, ch);
            return;
        }
        if (!CAN_SEE_OBJ(ch, GET_EQ(ch, WEAR_HOLD))) {
            send_to_char("���� ������ ������!\r\n", ch);
            return;
        }
        if (pen)
            paper = GET_EQ(ch, WEAR_HOLD);
        else
            pen = GET_EQ(ch, WEAR_HOLD);
    }


    /* ok.. now let's see what kind of stuff we've found */
    if (GET_OBJ_TYPE(pen) != ITEM_PEN)
        act("�� �� ������� ������ $o4.", FALSE, ch, pen, 0, TO_CHAR);
    else if (GET_OBJ_TYPE(paper) != ITEM_NOTE)
        act("�� �� ������ ������ �� $o5.", FALSE, ch, paper, 0, TO_CHAR);
    else if (paper->action_description)
        send_to_char("��� ��� ���-�� ��������.\r\n", ch);
    else {                      /* we can write - hooray! */
        /* this is the PERFECT code example of how to set up:
         * a) the text editor with a message already loaed
         * b) the abort buffer if the player aborts the message
         */
        ch->desc->backstr = NULL;
        send_to_char("������ ������.  (/s ��������� ������  /h ������)\r\n", ch);
        /* ok, here we check for a message ALREADY on the paper */
        if (paper->action_description) {        /* we str_dup the original text to the descriptors->backstr */
            if (ch->desc->backstr)
                free(ch->desc->backstr);
            ch->desc->backstr = str_dup(paper->action_description);
            /* send to the player what was on the paper (cause this is already */
            /* loaded into the editor) */
            send_to_char(paper->action_description, ch);
        }
        act("$n �����$g ������.", TRUE, ch, 0, 0, TO_ROOM);
        /* assign the descriptor's->str the value of the pointer to the text */
        /* pointer so that we can reallocate as needed (hopefully that made */
        /* sense:>) */
        string_write(ch->desc, &paper->action_description, MAX_NOTE_LENGTH, 0, NULL);
    }
}

ACMD(do_pager)
{
    struct descriptor_data *d;
    struct char_data *vict;
    char buf2[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    half_chop(argument, arg, buf2);

    if (IS_NPC(ch))
        send_to_char("������� �� ����� ��� �������.\r\n", ch);
    else if (!*arg)
        send_to_char("���� �����������?\r\n", ch);
    else {
        sprintf(buf, "\007\007*$n* %s", buf2);
        if (!str_cmp(arg, "all") || !str_cmp(arg, "���")) {
            for (d = descriptor_list; d; d = d->next)
                if (STATE(d) == CON_PLAYING && d->character)
                    act(buf, FALSE, ch, 0, d->character, TO_VICT);
        }
        if ((vict = get_char_vis(ch, arg, FIND_CHAR_WORLD)) != NULL) {
            act(buf, FALSE, ch, 0, vict, TO_VICT);
            if (PRF_FLAGGED(ch, PRF_NOREPEAT))
                send_to_char(OK, ch);
            else
                act(buf, FALSE, ch, 0, vict, TO_CHAR);
        } else
            send_to_char("����� ����� �����������!\r\n", ch);
    }
}


/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

struct communication_type {
    const char *muted_msg;
    const char *action;
    const char *no_channel;
    const char *color;
    const char *you_action;
    const char *hi_action;
    int min_lev;
    int move_cost;
    int noflag;
};

ACMD(do_gen_comm)
{
    struct descriptor_data *i;
    char color_on[24];
    char ibuf[MAX_INPUT_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    /*
     * com_msgs: Message if you can't perform the action because of mute
     *           name of the action
     *           message if you're not on the channel
     *           a color string.
     *           �� ....
     *           ��(�) ....
     *           min access level.
     *           mov cost.
     */

    struct communication_type com_msgs[] = {
        {"�� ������� � �� ������ �������.\r\n", /* shout */
         "�������",
         "�� ��� ��������� ������.\r\n",
         KIYEL,
         "��������",
         "�������$g",
         LVL_ROLL,
         10,
         PRF_NOHOLLER},

        {"���� ����� ����� � �� �� ������ �����.\r\n",  /* holler */
         "�����",
         "�� ��� ��������� ������.",
         KIYEL,
         "�������",
         "������$g",
         LVL_ROLL,
         5,
         PRF_NOSHOUT},

        {"�� �������, ��� �� �� ��������.\r\n", /* gossip */
         "�������",
         "�� ��� ��������� ������.\r\n",
         KMAG,
         "��������",
         "�������$g",
         (LVL_ROLL + 2),
         3,
         PRF_NOGOSS},

        {"���� ������������ �� � ����.\r\n",    /* congratulate */
         "�����������",
         "�� ��� ��������� ������.\r\n",
         KIBLU,
         "����������",
         "���������$g",
         LVL_ROLL,
         5,
         PRF_NOGRATZ}
    };

    /* to keep pets, etc from being ordered to shout */
    /*  if (!ch->desc)
       return; */

    if (IS_ANIMAL(ch))
        return;

    if (AFF_FLAGGED(ch, AFF_CHARM)) {
        act("$n ���-�� �������� ����������$g.", TRUE, ch, 0, 0, TO_ROOM);
        return;
    }

    if (AFF_FLAGGED(ch, AFF_SIELENCE)) {
        send_to_char(SIELENCE, ch);
        return;
    }

    if (PLR_FLAGGED(ch, PLR_DUMB)) {
        send_to_char("� ��� ��� ���!\r\n", ch);
        return;
    }

    if (PLR_FLAGGED(ch, PLR_MUTE)) {
        send_to_char(com_msgs[subcmd].muted_msg, ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF)) {
        send_to_char("����� ��������� ���� �����.\r\n", ch);
        return;
    }

    /*if (GET_LEVEL(ch) < com_msgs[subcmd].min_lev && GET_REMORT(ch) == 0) {
        sprintf(buf1, "��� ����� ������� ���� �� %d ������, ����� �� ����� %s.\r\n",
                com_msgs[subcmd].min_lev, com_msgs[subcmd].action);
        send_to_char(buf1, ch);
        return;
    }*/

    /* make sure the char is on the channel */
    if (PRF_FLAGGED(ch, com_msgs[subcmd].noflag)) {
        send_to_char(com_msgs[subcmd].no_channel, ch);
        return;
    }

    /* skip leading spaces */
    skip_spaces(&argument);

    /* make sure that there is something there to say! */
    if (!*argument && subcmd != SCMD_AUCTION) {
        sprintf(buf1, "%s ����� %s?\r\n",
                subcmd == SCMD_GRATZ ? "����" : "���", com_msgs[subcmd].action);
        send_to_char(buf1, ch);
        return;
    }

    if (!check_moves(ch, com_msgs[subcmd].move_cost))
        return;

    /* set up the color on code */
    strcpy(color_on, com_msgs[subcmd].color);

    /* first, set up strings to be given to the communicator */

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char(OK, ch);
    else {
        if (COLOR_LEV(ch) >= C_CMP)
            sprintf(buf1, "�� %s: '%s%s%s'", com_msgs[subcmd].you_action, color_on, argument, KNRM);
        else
            sprintf(buf1, "�� %s: '%s'", com_msgs[subcmd].you_action, argument);
        act(buf1, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
    }


    /* now send all the strings out */
    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) == CON_PLAYING && i != ch->desc && i->character &&
            !PRF_FLAGGED(i->character, com_msgs[subcmd].noflag) &&
            !PLR_FLAGGED(i->character, PLR_WRITING) &&
            !ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF) &&
            GET_POS(i->character) > POS_SLEEPING) {
            if (subcmd == SCMD_SHOUT &&
                ((world[ch->in_room].zone != world[i->character->in_room].zone) ||
                 !AWAKE(i->character)))
                continue;

            strcpy(ibuf, argument);     /* ��������� ������������ ����� */

            if (!IS_NPC(ch) && !IS_NPC(i->character) && !IS_GOD(ch) && !IS_GOD(i->character))
                garble_text(ibuf, GET_SKILL(i->character, SPEAKING(ch)), SPEAKING(ch));
            if (!IS_NPC(i->character) && !PRF_FLAGGED(i->character, PRF_CURSES))
                curses_check(ibuf);

            sprintf(buf, "$n %s: '%s%s%s'", com_msgs[subcmd].hi_action,
                    (COLOR_LEV(i->character) >= C_NRM) ? color_on : "", ibuf,
                    (COLOR_LEV(i->character) >= C_NRM) ? KNRM : "");
            act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);

        }
    }
}


ACMD(do_mobshout)
{
    struct descriptor_data *i;
    char buf[MAX_STRING_LENGTH];

    /* to keep pets, etc from being ordered to shout */
    if (!IS_NPC(ch))
        return;
    if (AFF_FLAGGED(ch, AFF_CHARM))
        return;
    sprintf(buf, "$n ������$g: '%s'", argument);

    /* now send all the strings out */
    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) == CON_PLAYING && i->character &&
            !PLR_FLAGGED(i->character, PLR_WRITING) && GET_POS(i->character) > POS_SLEEPING) {
            if (COLOR_LEV(i->character) >= C_NRM)
                send_to_char(KIYEL, i->character);
            act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
            if (COLOR_LEV(i->character) >= C_NRM)
                send_to_char(KNRM, i->character);
        }
    }
}

ACMD(do_qcomm)
{
    struct descriptor_data *i;
    char buf[MAX_STRING_LENGTH];

    if (!PRF_FLAGGED(ch, PRF_QUEST)) {
        send_to_char("� ��� ��� ������� �������!\r\n", ch);
        return;
    }
    skip_spaces(&argument);

    if (!IS_NPC(ch) && PLR_FLAGGED(ch, PLR_DUMB)) {
        send_to_char("��� ��������� ���������� � ������ �������!\r\n", ch);
        return;
    }

    if (!*argument) {
        sprintf(buf, "�����! �� ��� %s ?\r\n", CMD_NAME);
        send_to_char(buf, ch);
    } else {
        if (PRF_FLAGGED(ch, PRF_NOREPEAT))
            send_to_char(OK, ch);
        else {
            if (subcmd == SCMD_QSAY)
                sprintf(buf, "�� ��������: '%s'", argument);
            else
                strcpy(buf, argument);
            act(buf, FALSE, ch, 0, argument, TO_CHAR);
        }

        if (subcmd == SCMD_QSAY)
            sprintf(buf, "$n �������$g: '%s'", argument);
        else
            strcpy(buf, argument);

        for (i = descriptor_list; i; i = i->next)
            if (STATE(i) == CON_PLAYING && i != ch->desc && PRF_FLAGGED(i->character, PRF_QUEST))
                act(buf, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
    }
}

ACMD(do_remembertell)
{
    int i, j = 0, k = 0;

    if (IS_NPC(ch))
        return;

    send_to_char("�� ���������:\r\n", ch);

    for (i = 0; i < MAX_REMEMBER_TELLS; i++) {
        j = GET_LASTTELL(ch) + i;
        if (j >= MAX_REMEMBER_TELLS)
            j = j - MAX_REMEMBER_TELLS;
        if (GET_TELL(ch, j)[0] != '\0') {
            if (k == 0)
                send_to_char("&C", ch);
            k = 1;
            send_to_char(GET_TELL(ch, j), ch);
            send_to_char("\r\n", ch);
        }
    }

    if (!k)
        send_to_char("...��� ���� ������ �����.\r\n", ch);
    else
        send_to_char("&n", ch);
}


bool prep(char t)
{
    return (t == ' ' || t == ',' || t == '!' || t == '?' || t == '.' || t == ':' || t == ';');
}

void curses_check(char *words)
{
    int iCurCheck = 0;
    int iClearWord = 0;
    char *pcLastPos;
    char tbuf[MAX_STRING_LENGTH];
    char *poi;

    while (curses_list[iCurCheck] && iCurCheck < num_curses) {
        iClearWord = 0;
        while ((pcLastPos = str_str(words + iClearWord, curses_list[iCurCheck]))) {
            while (pcLastPos > words + iClearWord && !prep(*pcLastPos))
                pcLastPos--;
            if (prep(*pcLastPos))
                pcLastPos++;
            poi = tbuf;
            for (; !prep(*pcLastPos) && *pcLastPos != '\0'; pcLastPos++) {
                *(poi++) = *pcLastPos;
            }
            *poi = '\0';

            if (pcLastPos > words)
                pcLastPos--;
            while (pcLastPos > words && !prep(*pcLastPos))
                pcLastPos--;
            if (prep(*pcLastPos))
                pcLastPos++;
            for (; !prep(*pcLastPos) && *pcLastPos != '\0'; pcLastPos++)
                *pcLastPos = '*';

        }
        iCurCheck++;
    }

}

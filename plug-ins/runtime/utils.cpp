/**************************************************************************
*   File: utils.c                                       Part of CircleMUD *
*  Usage: various internal functions of a utility nature                  *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "comm.h"
#include "screen.h"
#include "spells.h"
#include "handler.h"
#include "interpreter.h"
#include "constants.h"
#include "pk.h"
#include "case.h"
#include "events.h"
#include "xspells.h"
#include "xbody.h"
#include "xtempl.h"
#include "xboot.h"
#include "strlib.h"
#include "expr.h"
#include "planescape.h"
#include "mudfile.h"

/* extern functions */
ACMD(do_stand);
SPECIAL(shoper);

/* local functions */
int get_max_ac_wear_obj(struct char_data *ch, struct obj_data *obj);


char AltToKoi[] = {
    "АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмноп░▒▓│┤╡╢╖╕╣║╗╝╜╛┐└┴┬├─┼╞╟╚╔╩╦╠═╬╧╨╤╥╙╘╒╓╫╪┘┌█▄▌▐▀рстуфхцчшщъыьэюяЁё≥≤⌠⌡÷≈°·∙√ ²■©"
};

char KoiToAlt[] = {
    "дЁз©юыц╢баеъэшщч╟╠╡ТЧЗШВСРЭУЬЩЫЖм╨уЯжи╦╥╩тсх╬╫╪фгл╣П╤╧яркопйьвнЪН═║Ф╓╔ДёЕ╗╘╙╚╛╜╝╞ОЮАБЦ╕╒ЛК╖ХМИГЙ·─│√└┘■┐∙┬┴┼▀▄█▌▐÷░▒▓⌠├┌°⌡┤≤²≥≈ "
};

char WinToKoi[] = {
    "©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©©АБВГДЕЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдежзийклмнопрстуфхцчшщъыьэюя"
};

char KoiToWin[] = {
    "-|+++++++++.....................-|+.+++++++++++++++.+++++++++++.ЧЮАЖДЕТЦУХИЙКЛМНОЪПЯРСФБЭШГЬЩЫВЗчюаждетцухийклмноъпярсфбэшгьщывз"
};

char KoiToWin2[] = {
    "-|+++++++++.....................-|+.+++++++++++++++.+++++++++++.ЧЮАЖДЕТЦУХИЙКЛМНОzПЯРСФБЭШГЬЩЫВЗчюаждетцухийклмноъпярсфбэшгьщывз"
};

char AltToLat[] = {
    "─│┌┐└┘├┤┬┴┼▀▄█▌▐░▒▓⌠■∙√≈≤≥ ⌡°²·÷═║╒ё╓╔╕╖╗╘╙╚╛╜╝╞╟╠╡Ё╢╣╤╥╦╧╨╩╪╫╬©0abcdefghijklmnopqrstY1v23z456780ABCDEFGHIJKLMNOPQRSTY1V23Z45678"
};

/************************************************************
 * return number of obj|mib in world by vnum
 ************************************************************/
int count_mob_vnum(long n)
{
    int count = 0;
    struct char_data *ch;

    for (ch = character_list; ch; ch = ch->next)
        if (n == GET_MOB_VNUM(ch))
            count++;

    return (count);
}

int gcount_obj_vnum(long n)
{
    int count = 0;
    obj_data *i;

    for (i = object_list; i; i = i->next)
        if (n == GET_OBJ_VNUM(i))
            count++;

    return (count);
}

int count_obj_vnum(long n)
{
    int i;

    if ((i = real_object(n)) < 0)
        return 0;
    return (obj_index[i].number + obj_index[i].stored);
}

/* показываем алинг в кодовом обозначении*/
int calc_alignment(struct char_data *ch)
{
    if IS_GOODA
        (ch)
            return (0);
    else if IS_GOOD
        (ch)
            return (1);
    else if IS_GOODN
        (ch)
            return (2);
    else if IS_NEUTRALG
        (ch)
            return (3);
    else if IS_NEUTRAL
        (ch)
            return (4);
    else if IS_NEUTRALE
        (ch)
            return (5);
    else if IS_EVILN
        (ch)
            return (6);
    else if IS_EVIL
        (ch)
            return (7);
    else if IS_EVILA
        (ch)
            return (8);
    else
        return (-1);
}


int calc_need_improove(struct char_data *ch, int level)
{
    float koef;
    float fk = 1 + (((GET_REAL_WIS(ch) / 16.0) - 1.0) / 2.0);

//foat fk = 2.0+(((float)GET_REAL_WIS(ch)-10.0)/10.0);
    int xcl = 0;

//Если докачали класс до 30го уровня, то ограничиваем только мудростью причем с лучшими свойствами
    if (level >= LVL_MAX / 2) {
        return (int) (fk * 100);
    }

    if ((xcl = get_extra_class(ch)) > 1) {
        int max_level = 1;

        for (int icls = 0; icls < NUM_CLASSES; icls++)
            if (ch->classes[icls] > max_level)
                max_level = ch->classes[icls];

        koef = GET_LEVEL(ch) - max_level;
        koef = koef / (xcl + 1);
        koef = level + koef;
        koef = fk * progress_skill[MIN(30, (int) koef)];
    } else
        koef = fk * progress_skill[MIN(30, level)];

    return (int) koef;
}

int get_flee_koef(struct char_data *ch)
{
    int flee_k = 0, level = GET_LEVEL(ch);

    for (int icls = 0; icls < NUM_CLASSES; icls++)
        if (ch->classes[icls])
            flee_k += (flee_koef[icls] * ch->classes[icls]) / level;

    return flee_k;
}


/* верояность повышения умения */
int calc_improove_skill(struct char_data *ch, int skill_no, struct char_data *victim, int tlevel)
{
    int skill_koef = 0, out = 0;
    int k_improove = 0, c_level = 0, level = GET_LEVEL(ch);

    for (int icls = 0; icls < NUM_CLASSES; icls++)
        if (ch->classes[icls])
            if (skill_info[skill_no].k_improove[icls][(int) GET_GODS(ch)] > 0) {
                int t_skill = 0;

                k_improove = skill_info[skill_no].k_improove[icls][(int) GET_GODS(ch)];
                c_level = ch->classes[icls];

                if (PRF_FLAGGED(ch, PRF_CODERINFO))
                    send_to_charf(ch,
                                  "&RТест %s skill_koef %d k_improove %d c_level %d level %d&n\r\n",
                                  class_name[icls], skill_koef, k_improove, c_level, level);

                t_skill = (k_improove * c_level) / level;
                if (k_improove > t_skill)
                    t_skill = k_improove;
                skill_koef += t_skill;
            }
    if (IS_GNOME(ch) && skill_no == SKILL_AXES)
        skill_koef = 20;
    //send_to_charf(ch,"Общий коэф %d\r\n",skill_koef);

    if (victim)
        out =
            (int) (skill_koef *
                   (float) ((float) GET_LEVEL(ch) / (float) GET_LEVEL(victim) *
                            (float) (18 / (float) GET_REAL_INT(ch))));
    else if (tlevel)
        out =
            (int) (skill_koef *
                   (float) ((float) GET_LEVEL(ch) / (float) tlevel *
                            (float) (18 / (float) GET_REAL_INT(ch))));
    else
        out =
            (int) (skill_koef *
                   (float) ((float) GET_LEVEL(ch) / 2.0 * (float) (18 / (float) GET_REAL_INT(ch))));

    //send_to_charf(ch,"коэф до %d ",out);

    //Поправка на уровень игрока тренирующего скилл
    out +=
        (int) ((float) out *
               ((float) GET_SKILL(ch, skill_no) / (float) (skill_info[skill_no].max_percent)));
    //send_to_charf(ch,"после %d уровень %3.2f\r\n",out, (float) out * ((float)GET_SKILL(ch,skill_no)/(float)100));

    if (victim) {
        if (IS_AFFECTED(victim, AFF_SLEEP) ||
            IS_AFFECTED(victim, AFF_BLIND) || GET_POS(victim) <= POS_SLEEPING)
            out *= 4;
    }
    //Поправка на заклинание
    int mod = affected_by_spell(ch, SPELL_FAST_LEARN);

    if (mod > 0)
        //out = (out*2)/3;
        out /= 2;

    if (victim) {
        if (IS_AFFECTED(victim, AFF_HOLD)
            || ROOM_FLAGGED(IN_ROOM(ch), ROOM_PEACEFUL)
            || IS_AFFECTED(victim, AFF_STOPFIGHT)
            || GET_POS(victim) <= POS_SLEEPING
            || (IS_NPC(victim) && NPC_FLAGGED(victim, NPC_MISLEAD))
            || (IS_AFFECTED(victim, AFF_STOPLEFT) && IS_AFFECTED(victim, AFF_STOPRIGHT)))
            out = 65535;
    }

    return out;
}

int calc_like_skill(struct char_data *ch, struct char_data *victim, int skill_no, int prob)
{

    if (!IS_NPC(ch)) {
        int c_koef = get_skill_abil(ch, skill_no);

        if (GET_SKILL(ch, skill_no))
            prob += GET_SKILL_ADD(ch, skill_no);

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KПоправка на классы: умение %s до %d ",
                          skill_info[skill_no].name.c_str(), prob);
        prob = (prob * c_koef) / 100;
        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "после %d коэф %d.", prob, c_koef);
    }

    if (victim) {               //Если есть жертва
        if (IS_AFFECTED(victim, AFF_HOLD) || GET_MOB_HOLD(victim))
            prob *= 6;
        if (IS_AFFECTED(victim, AFF_STOPFIGHT))
            prob *= 3;
        if (victim->trap_object)
            prob *= 2;

        prob = (prob * (100 - GET_REAL_PRT_SKILL(victim))) / 100;

        if (GET_POS(victim) < POS_FIGHTING)
            prob =
                (int) ((float) prob *
                       (float) (1.0 +
                                (float) ((float) POS_FIGHTING - (float) GET_POS(victim)) / 3.0));
        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "= %d", prob);
    }

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&n\r\n");


    /* ШТРАФЫ ЗА ДОСПЕХИ */
    switch (skill_no) {
        case SKILL_SNEAK:
        case SKILL_HIDE:
            if (GET_EQ(ch, WEAR_BODY) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_BODY)) == ITEM_ARMOR
                && GET_ARM_TYPE(GET_EQ(ch, WEAR_BODY)) == TARM_MEDIUM)
                prob = (prob * 3) / 4;  //25%
            if (GET_EQ(ch, WEAR_BODY) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_BODY)) == ITEM_ARMOR
                && GET_ARM_TYPE(GET_EQ(ch, WEAR_BODY)) == TARM_HARD)
                prob = prob / 2;        //50%
            break;
        case SKILL_BACKSTAB:
        case SKILL_DEVIATE:
            if (GET_EQ(ch, WEAR_BODY) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_BODY)) == ITEM_ARMOR
                && GET_ARM_TYPE(GET_EQ(ch, WEAR_BODY)) == TARM_MEDIUM)
                prob = (prob * 4) / 5;  //20%
            if (GET_EQ(ch, WEAR_BODY) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_BODY)) == ITEM_ARMOR
                && GET_ARM_TYPE(GET_EQ(ch, WEAR_BODY)) == TARM_HARD)
                prob = prob / 2;        //50%
            break;
        case SKILL_CIRCLESTAB:
        case SKILL_BLOODLET:
            if (GET_EQ(ch, WEAR_ARMS) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_ARMS)) == ITEM_ARMOR
                && GET_ARM_TYPE(GET_EQ(ch, WEAR_ARMS)) == TARM_MEDIUM)
                prob = (prob * 3) / 5;  //40%
            if (GET_EQ(ch, WEAR_ARMS) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_ARMS)) == ITEM_ARMOR
                && GET_ARM_TYPE(GET_EQ(ch, WEAR_ARMS)) == TARM_HARD)
                prob = prob / 3;        //66%
            break;
        case SKILL_PICK_LOCK:
        case SKILL_SAPPER:
            if (GET_EQ(ch, WEAR_HANDS) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_HANDS)) == ITEM_ARMOR
                && GET_ARM_TYPE(GET_EQ(ch, WEAR_HANDS)) == TARM_MEDIUM)
                prob = prob / 2;        //50%
            if (GET_EQ(ch, WEAR_HANDS) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_HANDS)) == ITEM_ARMOR
                && GET_ARM_TYPE(GET_EQ(ch, WEAR_HANDS)) == TARM_HARD)
                prob = prob / 4;        //75%
            break;
    }

    if (victim)
        if (GET_POS(victim) < POS_SLEEPING)
            prob = 200;

    return (prob);
};

//Вероятность удачного применения умения (число от 1 до 100)
int calc_like_skill(struct char_data *ch, struct char_data *victim, int skill_no,
                    std::vector < int >&cit, std::vector < int >&cot)
{
    int prob = 0;
    unsigned int i;
    float z = 0;


    if (IS_MOB(ch))
        z = (float) GET_SKILL_MOB(ch, skill_no);
    else
        z = (float) GET_SKILL(ch, skill_no);

    for (i = 0; i < cit.size(); i++)
        z *= (float) cit[i];

    for (i = 0; i < cot.size(); i++)
        z /= (float) cot[i];


    prob = (int) z;

    if (prob <= 0)
        return (0);

    if (!IS_NPC(ch)) {
        int c_koef = get_skill_abil(ch, skill_no);

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KПоправка на классы: умение %s до %d ",
                          skill_info[skill_no].name.c_str(), prob);
        prob = (prob * c_koef) / 100;
        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "после %d коэф %d.&n\r\n", prob, c_koef);
    }

    if (victim) {               //Если есть жертва
        if (IS_AFFECTED(victim, AFF_HOLD) || GET_MOB_HOLD(victim))
            prob *= 6;
        if (IS_AFFECTED(victim, AFF_STOPFIGHT))
            prob *= 3;
        if (victim->trap_object)
            prob *= 2;
        if (GET_POS(victim) < POS_FIGHTING)
            prob =
                (int) ((float) prob *
                       (float) (1.0 +
                                (float) ((float) POS_FIGHTING - (float) GET_POS(victim)) / 3.0));
    }

    if (((IS_THIEF(ch) || IS_RANGER(ch)) && equip_in_metall(ch) > 20) &&
        (skill_no == SKILL_CAMOUFLAGE ||
         skill_no == SKILL_SNEAK ||
         skill_no == SKILL_HIDE ||
         skill_no == SKILL_LOOK_HIDE || skill_no == SKILL_BACKSTAB || skill_no == SKILL_DEVIATE)) {
        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "Убрали (50%%) от умения %s за металические доспехи: %d %d\r\n",
                          skill_info[skill_no].name.c_str(), prob, prob / 2);
        prob /= 2;
    }

    return (prob);
};

//Aункция считающая максимальный процент владения
int get_skill_abil(struct char_data *ch, int skill_no)
{
    int ablt, max = 0, result = 100;

    for (int icls = 0; icls < NUM_CLASSES; icls++)
        if (ch->classes[icls]) {
            if (skill_info[skill_no].ability[icls][(int) GET_GODS(ch)] > 0)
                ablt = skill_info[skill_no].ability[icls][(int) GET_GODS(ch)];
            else
                ablt = 50;

            if (ablt * ch->classes[icls] > max) {       //Если условный коэф умения больше
                result = ablt;
                max = ablt * ch->classes[icls];
            }
        }

    return (result);
}

/* Делаем первый символ большим, учитывая &X цвета и пробелы */
void PHRASE(char *txt)
{
    for (; *txt; txt++) {
        if (a_isspace(*txt))
            continue;

        if (*txt == '&')
            if (isalpha(*(txt + 1)) || *(txt + 1) == '&') {
                txt++;
                continue;
            }

        break;
    }

    *txt = UPPER(*txt);
}

char *CAP(const char *txt)
{
    static char bufcap[MAX_STRING_LENGTH];

    strcpy(bufcap, txt);

    *bufcap = UPPER(*bufcap);

    return (bufcap);
}

char *DAP(const char *txt)
{
    static char bufcap[MAX_STRING_LENGTH];

    strcpy(bufcap, txt);

    *bufcap = LOWER(*bufcap);

    return (bufcap);
}


/*
 * Strips \r\n from end of string.
 */
void prune_crlf(char *txt)
{
    int i = strlen(txt) - 1;

    while (txt[i] == '\n' || txt[i] == '\r')
        txt[i--] = '\0';
}


/*
 * mudlog -- log mud messages to a file & to online imm's syslogs
 * based on syslog by Fen Jul 3, 1992
 */
void mudlog(const char *str, int type, int level, int file)
{
    char buf[MAX_STRING_LENGTH], tp;
    struct descriptor_data *i;

    if (str == NULL)
        return;                 /* eh, oh well. */
    if (file)
        log(str);
    if (level < 0)
        return;

    sprintf(buf, "[ %s ]\r\n", str);

    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING || IS_NPC(i->character))    /* switch */
            continue;
        if (GET_LEVEL(i->character) < level)
            continue;
        if (PLR_FLAGGED(i->character, PLR_WRITING))
            continue;
        tp = ((PRF_FLAGGED(i->character, PRF_LOG1) ? 1 : 0) +
              (PRF_FLAGGED(i->character, PRF_LOG2) ? 2 : 0));
        if (tp < type)
            continue;

        send_to_char(CCGRN(i->character, C_NRM), i->character);
        send_to_char(buf, i->character);
        send_to_char(CCNRM(i->character, C_NRM), i->character);
    }
}

void mudlogf(int type, int level, int file, const char *fmt, ...)
{
    char buf[MAX_STRING_LENGTH];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    mudlog(buf, type, level, file);
}


void speaklog(const char *str, int level, int file)
{
    char buf[MAX_STRING_LENGTH];
    struct descriptor_data *i;

    if (str == NULL)
        return;                 /* eh, oh well. */
    if (file)
        log(str);
    if (level < 0)
        return;

    sprintf(buf, "-> %s\r\n", str);

    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING || IS_NPC(i->character))    /* switch */
            continue;
        if (GET_LEVEL(i->character) < level)
            continue;
        if (PLR_FLAGGED(i->character, PLR_WRITING))
            continue;
        if (PRF_FLAGGED(i->character, PRF_SLOG)) {
            send_to_char(CCGRN(i->character, C_NRM), i->character);
            send_to_char(buf, i->character);
            send_to_char(CCNRM(i->character, C_NRM), i->character);
        }
    }
}

/*
 * If you don't have a 'const' array, just cast it as such.  It's safer
 * to cast a non-const array as const than to cast a const one as non-const.
 * Doesn't really matter since this function doesn't change the array though.
 */
const char *empty_string = "ничего";
int sprintbit(bitvector_t bitvector, const char *names[], char *result, const char *div)
{
    long nr = 0, fail = 0, divider = FALSE;

    *result = '\0';

    if ((unsigned long) bitvector < (unsigned long) INT_ONE);
    else if ((unsigned long) bitvector < (unsigned long) INT_TWO)
        fail = 1;
    else if ((unsigned long) bitvector < (unsigned long) INT_THREE)
        fail = 2;
    else
        fail = 3;
    bitvector &= 0x3FFFFFFF;
    while (fail) {
        if (*names[nr] == '\n')
            fail--;
        nr++;
    }


    for (; bitvector; bitvector >>= 1) {
        if (IS_SET(bitvector, 1)) {
            if (*names[nr] != '\n') {
                strcat(result, names[nr]);
                strcat(result, div);
                divider = TRUE;
            } else {
                strcat(result, "UNDEF");
                strcat(result, div);
                divider = TRUE;
            }
        }
        if (*names[nr] != '\n')
            nr++;
    }

    if (!*result) {
        strcat(result, empty_string);
        return FALSE;
    } else if (divider)
        *(result + strlen(result) - 1) = '\0';
    return TRUE;
}

void sprintbits(struct new_flag flags, const char *names[], char *result, const char *div)
{
    char buffer[MAX_STRING_LENGTH];
    int i;

    *result = '\0';
    for (i = 0; i < 4; i++) {
        if (sprintbit(flags.flags[i] | (i << 30), names, buffer, div)) {
            if (strlen(result)) {
                strcat(result, div);
            }
            strcat(result, buffer);
        }
    }

    if (!strlen(result))
        strcat(result, buffer);
}



void sprinttype(int type, const char *names[], char *result)
{
    int nr = 0;

    while (type && *names[nr] != '\n') {
        type--;
        nr++;
    }

    if (*names[nr] != '\n')
        strcpy(result, names[nr]);
    else
        strcpy(result, "UNDEF");
}


/* Calculate the REAL time passed over the last t2-t1 centuries (secs) */
struct time_info_data *real_time_passed(time_t t2, time_t t1)
{
    long secs;
    static struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs / SECS_PER_REAL_HOUR) % 24;       /* 0..23 hours */
    secs -= SECS_PER_REAL_HOUR * now.hours;

    now.day = (secs / SECS_PER_REAL_DAY);       /* 0..34 days  */
    /* secs -= SECS_PER_REAL_DAY * now.day; - Not used. */

    now.month = -1;
    now.year = -1;

    return (&now);
}



/* Calculate the MUD time passed over the last t2-t1 centuries (secs) */
struct time_info_data *mud_time_passed(time_t t2, time_t t1)
{
    long secs;
    static struct time_info_data now;

    secs = (long) (t2 - t1);

    now.hours = (secs / (SECS_PER_MUD_TICK * TIME_KOEFF)) % HOURS_PER_DAY;      /* 0..23 hours */
    secs -= SECS_PER_MUD_TICK * TIME_KOEFF * now.hours;

    now.day = (secs / (SECS_PER_MUD_DAY * TIME_KOEFF)) % DAYS_PER_MONTH;        /* 0..29 days  */
    secs -= SECS_PER_MUD_DAY * TIME_KOEFF * now.day;

    now.month = (secs / (SECS_PER_MUD_MONTH * TIME_KOEFF)) % MONTHS_PER_YEAR;   /* 0..11 months */
    secs -= SECS_PER_MUD_MONTH * TIME_KOEFF * now.month;

    now.year = (secs / (SECS_PER_MUD_YEAR * TIME_KOEFF));       /* 0..XX? years */

    return (&now);
}


//функция считает возраст по проведеному времени в игре
int age(struct char_data *ch)
{
    int ticks, tick_year;

    ticks = GET_TICKS(ch);
    tick_year = ((ticks / HOURS_PER_DAY) / (DAYS_PER_MONTH * MONTHS_PER_YEAR));
    tick_year += 17;
    return (tick_year);
}


//функция считает возраст с момента регистрации
struct time_info_data *age_old(struct char_data *ch)
{
    static struct time_info_data player_age;

    player_age = *mud_time_passed(time(0), ch->player.time.birth);

    player_age.year += 17;

    return (&player_age);
}


/* Check if making CH follow VICTIM will create an illegal */
/* Follow "Loop/circle"                                    */
bool circle_follow(struct char_data * ch, struct char_data * victim)
{
    struct char_data *k;

    for (k = victim; k; k = k->master) {
        if (k->master == k) {
            k->master = NULL;
            return (FALSE);
        }
        if (k == ch)
            return (TRUE);
    }

    return (FALSE);
}

void make_horse(struct char_data *horse, struct char_data *ch)
{
    SET_BIT(AFF_FLAGS(horse, AFF_HORSE_BUY), AFF_HORSE_BUY);
    add_follower(horse, ch, FLW_HORSE);
    horse->npc()->specials.wimp_level = 0;
    REMOVE_BIT(MOB_FLAGS(horse, MOB_SENTINEL), MOB_SENTINEL);
    REMOVE_BIT(MOB_FLAGS(horse, MOB_HELPER), MOB_HELPER);
    REMOVE_BIT(MOB_FLAGS(horse, MOB_AGGRESSIVE), MOB_AGGRESSIVE);
    //REMOVE_BIT(MOB_FLAGS(horse,MOB_MOUNTING),MOB_MOUNTING);
    REMOVE_BIT(AFF_FLAGS(horse, AFF_TETHERED), AFF_TETHERED);
}

int on_horse(struct char_data *ch)
{
    return (AFF_FLAGGED(ch, AFF_HORSE) && has_horse(ch, TRUE));
}

int has_horse(struct char_data *ch, int same_room)
{
    struct follow_type *f;

    /*  if (IS_NPC(ch))
       return (FALSE); */

    for (f = ch->followers; f; f = f->next) {
        if (IS_NPC(f->follower) && AFF_FLAGGED(f->follower, AFF_HORSE_BUY) &&
            (!same_room || IN_ROOM(ch) == IN_ROOM(f->follower)))
            return (TRUE);
    }
    return (FALSE);
}

struct char_data *get_horse(struct char_data *ch)
{
    struct follow_type *f;

    /*  if (IS_NPC(ch))
       return (NULL); */

    for (f = ch->followers; f; f = f->next) {
        if (IS_NPC(f->follower) && AFF_FLAGGED(f->follower, AFF_HORSE_BUY))
            return (f->follower);
    }
    return (NULL);
}

struct char_data *get_horse_on(struct char_data *ch)
{
    struct follow_type *f;

    /*  if (IS_NPC(ch))
       return (NULL); */

    for (f = ch->followers; f; f = f->next) {
        if (IS_NPC(f->follower) && AFF_FLAGGED(f->follower, AFF_HORSE_BUY)
            && AFF_FLAGGED(f->follower, AFF_HORSE))
            return (f->follower);
    }
    return (NULL);
}

void horse_drop(struct char_data *ch)
{
    if (ch->master) {
        act("$N сбросил$G Вас со своей спины.", FALSE, ch->master, 0, ch, TO_CHAR);
        act("$N сбросил$G $n3 со своей спины.", FALSE, ch->master, 0, ch, TO_NOTVICT);
        REMOVE_BIT(AFF_FLAGS(ch->master, AFF_HORSE), AFF_HORSE);
        WAIT_STATE(ch->master, 1 * PULSE_VIOLENCE);
        if (GET_POS(ch->master) > POS_SITTING)
            GET_POS(ch->master) = POS_SITTING;
    }
}

void check_horse(struct char_data *ch)
{
    if (!IS_NPC(ch) && !has_horse(ch, TRUE))
        REMOVE_BIT(AFF_FLAGS(ch, AFF_HORSE), AFF_HORSE);
}


/* Called when stop following persons, or stopping charm */
/* This will NOT do if a character quits/dies!!          */
int stop_follower(struct char_data *ch, int mode)
{
    struct char_data *master;
    struct follow_type *j, *k;
    char buf[MAX_STRING_LENGTH];
    int i;

    ACMD(do_look);
    ACMD(do_say);

    //log("[Stop follower] Start function(%s->%s)",ch ? GET_NAME(ch): "none",
    //      ch->master ? GET_NAME(ch->master): "none");

    if (!ch->master) {
        log("SYSERR: stop_follower(%s) without master", GET_NAME(ch));
        // core_dump();
        return (FALSE);
    }

    //log("[Stop follower] Stop horse");
    if (get_horse_on(ch->master) == ch && on_horse(ch->master))
        horse_drop(ch);
    else {
        act("$n прекратил$g следовать за Вами.", TRUE, ch, 0, ch->master, TO_VICT);
        act("Вы прекратили следовать за $N4.", FALSE, ch, 0, ch->master, TO_CHAR);
        if (IN_ROOM(ch) == IN_ROOM(ch->master))
            act("$n прекратил$g следовать за $N4.", TRUE, ch, 0, ch->master, TO_NOTVICT);
    }

    //log("[Stop follower] Remove from followers list");
    if (!ch->master->followers)
        log("[Stop follower] SYSERR: Followers absent for %s (master %s).", GET_NAME(ch),
            GET_NAME(ch->master));
    else if (ch->master->followers->follower == ch) {   /* Head of follower-list? */
        k = ch->master->followers;
        ch->master->followers = k->next;
        free(k);
    } else {                    /* locate follower who is not head of list */
        for (k = ch->master->followers; k->next && k->next->follower != ch; k = k->next);
        if (!k->next)
            log("[Stop follower] SYSERR: Undefined %s in %s followers list.", GET_NAME(ch),
                GET_NAME(ch->master));
        else {
            j = k->next;
            k->next = j->next;
            free(j);
        }
    }
    master = ch->master;
    ch->master = NULL;

    if (IS_NPC(ch) && AFF_FLAGGED(ch, AFF_HORSE))
        REMOVE_BIT(AFF_FLAGS(ch, AFF_HORSE), AFF_HORSE);

    //log("[Stop follower] Free charmee");
    if (AFF_FLAGGED(ch, AFF_CHARM) || AFF_FLAGGED(ch, AFF_HELPER) || IS_SET(mode, SF_CHARMLOST)
        ) {
        if (affected_by_spell(ch, SPELL_CHARM))
            affect_from_char(ch, SPELL_CHARM);
        EXTRACT_TIMER(ch) = 15;
        REMOVE_BIT(AFF_FLAGS(ch, AFF_CHARM), AFF_CHARM);
        // log("[Stop follower] Stop fight charmee");
        if (FIGHTING(ch))
            stop_fighting(ch, TRUE);
        /*
           log("[Stop follower] Stop fight charmee opponee");
           for (vict = world[IN_ROOM(ch)].people; vict; vict = vict->next)
           {if (FIGHTING(vict) &&
           FIGHTING(vict) == ch &&
           FIGHTING(ch) != vict)
           stop_fighting(vict);
           }
         */
        //log("[Stop follower] Charmee MOB reaction");
        if (IS_NPC(ch)) {
            if (MOB_FLAGGED(ch, MOB_ANGEL)) {
                act("$n с мягким хлопком растворил$u в воздухе.", TRUE, ch, 0, 0, TO_ROOM);
                GET_LASTROOM(ch) = IN_ROOM(ch);
                extract_char(ch, FALSE);
                return (TRUE);
            }
            if (MOB_FLAGGED(ch, MOB_DAEMON)) {
                act("$n неистовым воплем растворил$u в воздухе.", TRUE, ch, 0, 0, TO_ROOM);
                GET_LASTROOM(ch) = IN_ROOM(ch);
                extract_char(ch, FALSE);
                return (TRUE);
            } else if (AFF_FLAGGED(ch, AFF_HELPER))
                REMOVE_BIT(AFF_FLAGS(ch, AFF_HELPER), AFF_HELPER);
            else if (IS_UNDEAD(ch)) {
                //Действия для нежити
                if (GET_MOB_VID(ch) == VMOB_SKELET) {
                    act("1и рассыпал1(ся,ась,ось,ись) в прах.", "Км", ch);
                    extract_char(ch, FALSE);
                }
                return (TRUE);
            } else {
                if (master &&
                    IS_SET(mode, SF_MASTERDIE) &&
                    IN_ROOM(ch) == IN_ROOM(master) && CAN_SEE(ch, master)) {
                    if (may_pkill(master, ch) != PC_REVENGE_PC)
                        inc_pkill_group(ch, master, 1, 0);

                    if (dice(1, 20) > GET_REAL_LCK(ch)) {
                        if (GET_RACE(ch) == RACE_ANIMAL) {
                            act("$N с диким ревом набросился на Вас.", FALSE, master, 0, ch,
                                TO_CHAR);
                            act("$N с диким ревом набросился на $n3.", FALSE, master, 0, ch,
                                TO_ROOM);
                        } else {
                            do_look(ch, GET_NAME(master), 0, 0, 0);
                            strcpy(buf, "Ты мной командовал, за это ты умрешь.");
                            do_say(ch, buf, 0, 0, 0);
                        }
                        check_position(ch);
                        set_fighting(ch, master);
                    }
                } else
                    if (master &&
                        (!IS_SET(mode, SF_MASTERDIE) || AFF_FLAGGED(ch, AFF_CHARM)) &&
                        CAN_SEE(ch, master) && MOB_FLAGGED(ch, MOB_MEMORY)) {
                    if (may_pkill(master, ch) != PC_REVENGE_PC)
                        inc_pkill_group(ch, master, 1, 0);
                }
            }
        }
    }
    //log("[Stop follower] Restore mob flags");
    if (IS_NPC(ch) && (i = GET_MOB_RNUM(ch)) >= 0) {
        MOB_FLAGS(ch, INT_ZERRO) = MOB_FLAGS(mob_proto + i, INT_ZERRO);
        MOB_FLAGS(ch, INT_ONE) = MOB_FLAGS(mob_proto + i, INT_ONE);
        MOB_FLAGS(ch, INT_TWO) = MOB_FLAGS(mob_proto + i, INT_TWO);
        MOB_FLAGS(ch, INT_THREE) = MOB_FLAGS(mob_proto + i, INT_THREE);
        GET_LEVEL(ch) = GET_LEVEL(mob_proto + i);
        GET_EXP(ch) = GET_EXP(mob_proto + i);

        for (int c = 0; c < NUM_CLASSES; c++)
            ch->init_classes[c] = mob_proto[i].init_classes[c];

    }
    //log("[Stop follower] Stop function");
    return (FALSE);
}



/* Called when a character that follows/is followed dies */
void die_follower(struct char_data *ch)
{
    struct follow_type *j, *k = ch->followers;

    if (ch->master)
        stop_follower(ch, SF_FOLLOWERDIE);

    if (on_horse(ch))
        REMOVE_BIT(AFF_FLAGS(ch, AFF_HORSE), AFF_HORSE);

    for (k = ch->followers; k; k = j) {
        j = k->next;
        stop_follower(k->follower, SF_FOLLOWERDIE);
    }
}

/* возвращает уровень класса или 0 если класс не определен */
int check_class(struct char_data *ch, int nclass)
{
    if (nclass < 0 || nclass > NUM_CLASSES)
        return (0);

    return (ch->classes[nclass]);
}

//возвращает маскимальную профессию
int get_max_class(struct char_data *ch)
{
    int level = 0, nclass = 0;

    for (int icls = 0; icls < NUM_CLASSES; icls++)
        if (ch->classes[icls] > level) {
            nclass = icls;
            level = ch->classes[icls];
        }

    return (nclass);
}

//возвращает уровень в максимальной професии
int get_max_levelclass(struct char_data *ch)
{
    int level = 0, nclass = 0;

    for (int icls = 0; icls < NUM_CLASSES; icls++)
        if (ch->classes[icls] > level) {
            nclass = icls;
            level = ch->classes[icls];
        }

    return (level);
}

//возвращает кол-во профессий
int get_extra_class(struct char_data *ch)
{
    int i = 0;

    for (int icls = 0; icls < NUM_CLASSES; icls++)
        if (ch->classes[icls])
            i++;

    return (i);
}

/* параметры vnum_obj номер ключа, rnum_room комната
   если ключ есть в комнате у игрока то возвращает номер
   комнаты телепортации иначе 0 */

int check_portal(int vnum_obj, int rnum_room)
{
    int i, vnum_r;
    struct obj_data *obj = NULL;
    struct char_data *ch;

    for (ch = world[rnum_room].people; ch; ch = ch->next_in_room)
        for (i = 0; i < NUM_OF_DIRS; i++)
            if (world[rnum_room].dir_option[i] && world[rnum_room].dir_option[i]->type_port) {
                vnum_r = world[rnum_room].dir_option[i]->room_port;
                if (GET_EQ(ch, WEAR_HOLD))
                    obj = GET_EQ(ch, WEAR_HOLD);
                if (obj && GET_OBJ_VNUM(obj) == vnum_obj)
                    return (vnum_r);
            }

    return (0);
}

int check_ip(char *ip)
{
    struct ip_table_type *k;

    for (k = ip_table; k; k = k->next) {
        if (!strcmp(ip, k->ip) && (time(0) < k->timer + 15 * 60))
            return 1;
    }
    return 0;
}

void add_ip_table(char *ip, long timer)
{
    struct ip_table_type *k;

    CREATE(k, struct ip_table_type, strlen(ip) + 1);

    memcpy(k->ip, ip, strlen(ip) + 1);
    k->timer = timer;
    k->next = ip_table;

    ip_table = k;

}

/* добавляет/изменяет класс/уровень игроку */
void add_class(struct char_data *ch, int nclass, int level, int favorite)
{
    if (nclass < 0 || nclass > NUM_CLASSES)
        return;

    /* Заносим в таблицу */
    ch->init_classes[nclass] = level;
}

/* удаляет класс из игрока */
void del_class(struct char_data *ch, int nclass)
{
    if (nclass < 0 || nclass > NUM_CLASSES)
        return;

    if (ch->init_classes[nclass])
        ch->init_classes[nclass] = 0;
}


/* Добавляет сообщения в перехватчик */
/* Монстрам */
void add_message(struct char_data *ch, int no_command, int stopflag, int script,
                 char *sarg,
                 char *mess_to_char, char *mess_to_vict, char *mess_to_other, char *mess_to_room)
{
    struct mess_p_data *k;


    CREATE(k, struct mess_p_data, 1);

    k->command = no_command;
    k->stoping = stopflag;
    k->script = script;

    if (sarg)
        k->sarg = str_dup(sarg);

    if (mess_to_char)
        k->mess_to_char = str_dup(mess_to_char);
    else
        k->mess_to_char = NULL;

    if (mess_to_vict)
        k->mess_to_vict = str_dup(mess_to_vict);
    else
        k->mess_to_vict = NULL;

    if (mess_to_other)
        k->mess_to_other = str_dup(mess_to_other);
    else
        k->mess_to_other = NULL;

    if (mess_to_room)
        k->mess_to_room = str_dup(mess_to_room);
    else
        k->mess_to_room = NULL;

    k->next = ch->mess_data;
    ch->mess_data = k;
}

/* В комнаты */
void add_message(struct room_data *room, int no_command, int stopflag, int script,
                 char *mess_to_char, char *mess_to_vict, char *mess_to_other, char *mess_to_room)
{
    struct mess_p_data *k;


    CREATE(k, struct mess_p_data, 1);

    k->command = no_command;
    k->stoping = stopflag;
    k->script = script;

    k->mess_to_char = str_dup(mess_to_char);

    if (mess_to_vict)
        k->mess_to_vict = str_dup(mess_to_vict);

    if (mess_to_other)
        k->mess_to_other = str_dup(mess_to_other);

    if (mess_to_room)
        k->mess_to_room = str_dup(mess_to_room);

    k->next = room->mess_data;
    room->mess_data = k;
}

/* В предметы */
void add_message(struct obj_data *obj, int no_command, int stopflag, int script,
                 char *mess_to_char, char *mess_to_vict, char *mess_to_other, char *mess_to_room)
{
    struct mess_p_data *k;


    CREATE(k, struct mess_p_data, 1);

    k->command = no_command;
    k->script = script;
    k->stoping = stopflag;

    if (mess_to_char)
        k->mess_to_char = str_dup(mess_to_char);
    else
        k->mess_to_char = NULL;

    if (mess_to_vict)
        k->mess_to_vict = str_dup(mess_to_vict);
    else
        k->mess_to_vict = NULL;


    if (mess_to_other)
        k->mess_to_other = str_dup(mess_to_other);
    else
        k->mess_to_other = NULL;


    if (mess_to_room)
        k->mess_to_room = str_dup(mess_to_room);
    else
        k->mess_to_room = NULL;


    k->next = obj->mess_data;
    obj->mess_data = k;
}



/* Do NOT call this before having checked if a circle of followers */
/* will arise. CH will follow leader                               */
void add_follower(struct char_data *ch, struct char_data *leader, int type)
{
    struct follow_type *k;

    if (ch->master) {
        log("SYSERR: add_follower(%s->%s) when master existing(%s)...",
            GET_NAME(ch), leader ? GET_NAME(leader) : "", GET_NAME(ch->master));
        // core_dump();
        return;
    }

    if (ch == leader)
        return;

    ch->master = leader;

    CREATE(k, struct follow_type, 1);

    k->follower = ch;
    k->type = type;
    k->next = leader->followers;
    leader->followers = k;

    if (type == FLW_HORSE || type == FLW_CHARM || type == FLW_GROUP) {
        act("Вы начали следовать за 2+т.", "Мм", ch, leader);
        act("1+и начал1(,а,о,и) следовать за Вами.", "мМ", ch, leader);
        act("1+и начал1(,а,о,и) следовать за 2+т.", "Кмм", ch, leader);
    }
}

/*
 * get_line reads the next non-blank line off of the input stream.
 * The newline character is removed from the input.  Lines which begin
 * with '*' are considered to be comments.
 *
 * Returns the number of lines advanced in the file.
 */
int get_line(FILE * fl, char *buf)
{
    char temp[256];
    int lines = 0;

    do {
        fgets(temp, 256, fl);
        if (feof(fl))
            return (0);
        lines++;
    } while (*temp == '*' || *temp == '\n');

    temp[strlen(temp) - 1] = '\0';
    strcpy(buf, temp);
    return (lines);
}


int get_filename(const char *orig_name, char *filename, int mode)
{
    const char *middle, *suffix;
    MudFile prefix;
    char name[64], *ptr;

    if (orig_name == NULL || *orig_name == '\0' || filename == NULL) {
        log("SYSERR: NULL pointer or empty string passed to get_filename(), %p or %p.",
            orig_name, filename);
        return (0);
    }

    switch (mode) {
        case CRASH_FILE:
            prefix = MudFile(mud->playerObjsDir);
            suffix = SUF_OBJS;
            break;
        case TEXT_CRASH_FILE:
            prefix = MudFile(mud->playerObjsDir);
            suffix = TEXT_SUF_OBJS;
            break;
        case TIME_CRASH_FILE:
            prefix = MudFile(mud->playerObjsDir);
            suffix = TIME_SUF_OBJS;
            break;
        case PET_FILE:
            prefix = MudFile(mud->playerAliasDir);
            suffix = PET_SUF;
            break;
        case ALIAS_FILE:
            prefix = MudFile(mud->playerAliasDir);
            suffix = SUF_ALIAS;
            break;
        case PLAYERS_FILE:
            prefix = MudFile(mud->playerDir);
            suffix = SUF_PLAYER;
            break;
        case PKILLERS_FILE:
            prefix = MudFile(mud->playerDir);
            suffix = SUF_PKILLER;
            break;
        case XPLAYERS_FILE:
            prefix = MudFile(mud->playerDir);
            suffix = SUF_QUESTS;
            break;
        case PMKILL_FILE:
            prefix = MudFile(mud->playerDir);
            suffix = SUF_PMKILL;
            break;
        case ETEXT_FILE:
            prefix = MudFile(mud->playerTextDir);
            suffix = SUF_TEXT;
            break;
        case SCRIPT_VARS_FILE:
            prefix = MudFile(mud->playerVarsDir);
            suffix = SUF_MEM;
            break;
        case QUEST_FILE:
            prefix = MudFile(mud->playerVarsDir);
            suffix = SUF_QST;
            break;
        case LOGGER_FILE:
            prefix = MudFile(mud->playerDir);
            suffix = SUF_LOGGER;
            break;
        default:
            return (0);
    }

    strcpy(name, orig_name);
    for (ptr = name; *ptr; ptr++)
        *ptr = LOWER(AtoL(*ptr));

    switch (LOWER(*name)) {
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
            middle = LIB_A;
            break;
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
            middle = LIB_F;
            break;
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
            middle = LIB_K;
            break;
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
            middle = LIB_P;
            break;
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
            middle = LIB_U;
            break;
        default:
            middle = LIB_Z;
            break;
    }


    sprintf(filename, "%s/%s/%s.%s", prefix.getCPath(), middle, name, suffix);
    return (1);
}


int num_pc_in_room(struct room_data *room)
{
    int i = 0;
    struct char_data *ch;

    for (ch = room->people; ch != NULL; ch = ch->next_in_room)
        if (!IS_NPC(ch) && !IS_SOUL(ch))
            i++;

    return (i);
}

int num_all_in_room(struct room_data *room)
{
    int i = 0;
    struct char_data *ch;

    for (ch = room->people; ch != NULL; ch = ch->next_in_room)
        i++;

    return (i);
}

/*
 * This function (derived from basic fork(); abort(); idea by Erwin S.
 * Andreasen) causes your MUD to dump core (assuming you can) but
 * continue running.  The core dump will allow post-mortem debugging
 * that is less severe than assert();  Don't call this directly as
 * core_dump_unix() but as simply 'core_dump()' so that it will be
 * excluded from systems not supporting them. (e.g. Windows '95).
 *
 * You still want to call abort() or exit(1) for
 * non-recoverable errors, of course...
 *
 * XXX: Wonder if flushing streams includes sockets?
 */
void core_dump_real(const char *who, int line)
{
    log("SYSERR: Assertion failed at %s:%d!", who, line);

#if defined(CIRCLE_UNIX)
    /* These would be duplicated otherwise... */
    fflush(stdout);
    fflush(stderr);

    /*
     * Kill the child so the debugger or script doesn't think the MUD
     * crashed.  The 'autorun' script would otherwise run it again.
     */
    if (fork() == 0)
        abort();
#endif
}

void to_koi(char *str, int from)
{
    switch (from) {
        case KT_ALT:
            for (; *str; *str = AtoK(*str), str++);
            break;
        case KT_WINZ:
        case KT_WIN:
            for (; *str; *str = WtoK(*str), str++);
            break;
    }
}

void from_koi(char *str, int from)
{
    switch (from) {
        case KT_ALT:
            for (; *str; *str = KtoA(*str), str++);
            break;
        case KT_WIN:
            for (; *str; *str = KtoW(*str), str++);
        case KT_WINZ:
            for (; *str; *str = KtoW2(*str), str++);
            break;
    }
}

void koi_to_win(char *str, int size)
{
    for (; size > 0; *str = KtoW(*str), size--, str++);
}

void koi_to_winz(char *str, int size)
{
    for (; size > 0; *str = KtoW2(*str), size--, str++);
}

void koi_to_alt(char *str, int size)
{
    for (; size > 0; *str = KtoA(*str), size--, str++);
}

/* string manipulation fucntion originally by Darren Wilson */
/* (wilson@shark.cc.cc.ca.us) improved and bug fixed by Chris (zero@cnw.com) */
/* completely re-written again by M. Scott 10/15/96 (scottm@workcommn.net), */
/* substitute appearances of 'pattern' with 'replacement' in string */
/* and return the # of replacements */
int replace_str(char **string, char *pattern, char *replacement, int rep_all, int max_size)
{
    char *replace_buffer = NULL;
    char *flow, *jetsam, temp;
    int len, i;

    if (((int) strlen(*string) - (int) strlen(pattern)) + (int) strlen(replacement) > max_size)
        return -1;

    CREATE(replace_buffer, char, max_size);
    i = 0;
    jetsam = *string;
    flow = *string;
    *replace_buffer = '\0';
    if (rep_all) {
        while ((flow = (char *) strstr(flow, pattern)) != NULL) {
            i++;
            temp = *flow;
            *flow = '\0';
            if ((int) (strlen(replace_buffer) + strlen(jetsam) + strlen(replacement)) > max_size) {
                i = -1;
                break;
            }
            strcat(replace_buffer, jetsam);
            strcat(replace_buffer, replacement);
            *flow = temp;
            flow += strlen(pattern);
            jetsam = flow;
        }
        strcat(replace_buffer, jetsam);
    } else {
        if ((flow = (char *) strstr(*string, pattern)) != NULL) {
            i++;
            flow += strlen(pattern);
            len = ((char *) flow - (char *) *string) - strlen(pattern);

            strncpy(replace_buffer, *string, len);
            strcat(replace_buffer, replacement);
            strcat(replace_buffer, flow);
        }
    }
    if (i == 0)
        return 0;
    if (i > 0) {
        RECREATE(*string, char, strlen(replace_buffer) + 3);

        strcpy(*string, replace_buffer);
    }
    free(replace_buffer);
    return i;
}


/* re-formats message type formatted char * */
/* (for strings edited with d->str) (mostly olc and mail)     */
void format_text(char **ptr_string, int mode, struct descriptor_data *d, int maxlen)
{
    int total_chars, cap_next = TRUE, cap_next_next = FALSE;
    char *flow, *start = NULL, temp;

    /* warning: do not edit messages with max_str's of over this value */
    char formated[MAX_STRING_LENGTH];

    flow = *ptr_string;
    if (!flow)
        return;

    if (IS_SET(mode, FORMAT_INDENT)) {
        strcpy(formated, "   ");
        total_chars = 3;
    } else {
        *formated = '\0';
        total_chars = 0;
    }

    while (*flow != '\0') {
        while ((*flow == '\n') ||
               (*flow == '\r') ||
               (*flow == '\f') || (*flow == '\t') || (*flow == '\v') || (*flow == ' '))
            flow++;

        if (*flow != '\0') {
            start = flow++;
            while ((*flow != '\0') &&
                   (*flow != '\n') &&
                   (*flow != '\r') &&
                   (*flow != '\f') &&
                   (*flow != '\t') &&
                   (*flow != '\v') &&
                   (*flow != ' ') && (*flow != '.') && (*flow != '?') && (*flow != '!'))
                flow++;

            if (cap_next_next) {
                cap_next_next = FALSE;
                cap_next = TRUE;
            }

            /* this is so that if we stopped on a sentance .. we move off the sentance delim. */
            while ((*flow == '.') || (*flow == '!') || (*flow == '?')) {
                cap_next_next = TRUE;
                flow++;
            }

            temp = *flow;
            *flow = '\0';

            if ((total_chars + strlen(start) + 1) > 79) {
                strcat(formated, "\r\n");
                total_chars = 0;
            }

            if (!cap_next) {
                if (total_chars > 0) {
                    strcat(formated, " ");
                    total_chars++;
                }
            } else {
                cap_next = FALSE;
                *start = UPPER(*start);
            }

            total_chars += strlen(start);
            strcat(formated, start);

            *flow = temp;
        }

        if (cap_next_next) {
            if ((total_chars + 3) > 79) {
                strcat(formated, "\r\n");
                total_chars = 0;
            } else {
                strcat(formated, "  ");
                total_chars += 2;
            }
        }
    }
    strcat(formated, "\r\n");

    if ((int) strlen(formated) > maxlen)
        formated[maxlen] = '\0';
    RECREATE(*ptr_string, char, MIN((int) maxlen, (int) strlen(formated) + 3));

    strcpy(*ptr_string, formated);
}


const char *some_pads[3][17] = {
    {"дней", "часов", "лет", "очков", "минут", "минут", "монет", "монет",
     "штук", "штук", "уровней", "смертных персонажей", "верст", "единиц",
     "единиц", "секунд", "градусов"},
    {"день", "час", "год", "очко", "минута", "минуту", "монета", "монету",
     "штука", "штуку", "уровень", "смертный персонаж", "версту", "единица",
     "единицу", "секунду", "градус"},
    {"дня", "часа", "года", "очка", "минуты", "минуты", "монеты", "монеты",
     "штуки", "штуки", "уровня", "смертных персонажа", "версты", "единицы",
     "единицы", "секунды", "градуса"}
};

const char *desc_count(int how_many, int of_what)
{
    if (how_many < 0)
        how_many = -how_many;
    if ((how_many % 100 >= 11 && how_many % 100 <= 14) || how_many % 10 >= 5 || how_many % 10 == 0)
        return some_pads[0][of_what];
    if (how_many % 10 == 1)
        return some_pads[1][of_what];
    else
        return some_pads[2][of_what];
}

int check_moves(struct char_data *ch, int how_moves)
{
    if (IS_IMMORTAL(ch) || IS_NPC(ch))
        return (TRUE);
    if (GET_MOVE(ch) < how_moves) {
        send_to_char("Вы слишком устали.\r\n", ch);
        return (FALSE);
    }
    GET_MOVE(ch) -= how_moves;
    return (TRUE);
}

int real_sector(int room)
{
    if (room == NOWHERE)
        return 0;

    return SECT(room);
}


char *only_title(struct char_data *ch)
{
    static char title[MAX_STRING_LENGTH];
    static char clan[MAX_STRING_LENGTH];
    char *pos;

    if (!GET_TITLE(ch) || !*GET_TITLE(ch)) {
        sprintf(title, "%s%s", GET_PAD(ch, 0), clan);
    } else if (!IS_GOD(ch)) {
        if ((pos = strchr(GET_TITLE(ch), ';'))) {
            *(pos++) = '\0';
            sprintf(title, "%s %s%s%s", pos, GET_PAD(ch, 0),
                    *GET_TITLE(ch) ? ", " : " ", GET_TITLE(ch));
            *(--pos) = ';';
        } else
            sprintf(title, "%s %s", GET_PAD(ch, 0), GET_TITLE(ch));
    } else
        sprintf(title, "%s", GET_TITLE(ch));
    return title;
}

char *race_or_title(struct char_data *ch, int pad)
{
    static char title[MAX_STRING_LENGTH];
    static char race_n[MAX_STRING_LENGTH];


    if (!GET_TITLE(ch) || !*GET_TITLE(ch)) {
        if (!pad) {
            sprintf(race_n, "%s", race_name[(int) GET_RACE(ch)][(int) GET_SEX(ch)]);
        } else
            switch (GET_SEX(ch)) {
                case SEX_MALE:
                    strcpy(race_n,
                           get_name_pad((char *) race_name_pad_male[(int) GET_RACE(ch)], pad,
                                        PAD_MONSTER));
                    break;
                case SEX_FEMALE:
                    strcpy(race_n,
                           get_name_pad((char *) race_name_pad_female[(int) GET_RACE(ch)], pad,
                                        PAD_MONSTER));
                    break;
                default:
                    strcpy(race_n, race_name[(int) GET_RACE(ch)][(int) GET_SEX(ch)]);
                    break;
            }
        sprintf(title, "%s %s", race_n, GET_PAD(ch, pad));
    } else
        strcpy(title, only_title(ch));

    return CAP(title);
}


int get_sex_infra(const struct char_data *ch)
{
    switch (GET_RACE(ch)) {
        case RACE_AASIMAR:
            return SEX_MALE;
            break;
        case RACE_ANIMAL:
            return SEX_NEUTRAL;
            break;
        case RACE_DRAGON:
            return SEX_MALE;
            break;
        case RACE_GENASI:
            return SEX_MALE;
            break;
        case RACE_BIRD:
            return SEX_FEMALE;
            break;
        case RACE_WORM:
            return SEX_MALE;
            break;
        case RACE_REPTILE:
            return SEX_FEMALE;
            break;
        case RACE_SNAKE:
            return SEX_FEMALE;
            break;
        case RACE_INSECT:
            return SEX_NEUTRAL;
            break;
        case RACE_GIGANT:
            return SEX_MALE;
            break;
        case RACE_CONSTRUCTION:
            return SEX_FEMALE;
            break;
        case RACE_GHOST:
            return SEX_MALE;
            break;
        case RACE_PLANT:
            return SEX_NEUTRAL;
            break;
        case RACE_SLIME:
            return SEX_FEMALE;
            break;
        case RACE_HUMANOID:
            return SEX_MALE;
            break;
        case RACE_HORSE:
            return SEX_MALE;
            break;
        case RACE_RODENT:
            return SEX_MALE;
            break;
        default:
            return (GET_SEX(ch));
            break;
    }
}

const char *size_m[8][NUM_SEXES] = {
    {"крошечн(ое) ", "крошечн(ый) ", "крошечн(ая) ", "крошечн(ые) "},
    {"малень(кое) ", "малень(кий) ", "малень(кая) ", "малень(кие) "},
    {"небольш(ое) ", "небольш(ой) ", "небольш(ая) ", "небольш(ие) "},
    {"", "", "", ""},           //среднее-нормальное
    {"крупн(ое) ", "крупн(ый) ", "крупн(ая) ", "крупн(ые) "},
    {"огромн(ое) ", "огромн(ый) ", "огромн(ая) ", "огромн(ые) "},
    {"гигантс(кое) ", "гигантс(кий) ", "гигантс(кая) ", "гигантс(кие) "},
    {"титаническ(ое) ", "титаничес(кий) ", "титаническ(ая) ", "титаническ(ие) "}
};

char *get_size_infra(const struct char_data *ch, int pad)
{

    static char buf_size[256];
    int pos = GET_REAL_SIZE(ch);

    if (pos <= 10)
        pos = 0;
    else if (pos > 10 && pos <= 20)
        pos = 1;
    else if (pos > 20 && pos <= 30)
        pos = 2;
    else if (pos > 30 && pos <= 40)
        pos = 3;
    else if (pos > 40 && pos <= 50)
        pos = 4;
    else if (pos > 50 && pos <= 60)
        pos = 5;
    else if (pos > 60 && pos <= 70)
        pos = 6;
    else
        pos = 7;

    strcpy(buf_size, get_name_pad((char *) size_m[pos][get_sex_infra(ch)], pad));

    return (buf_size);
}

char *name_infra(const struct char_data *ch, int pad)
{
    static char title[MAX_STRING_LENGTH];
    char pname[256];

    switch (get_sex_infra(ch)) {
        case SEX_FEMALE:

            strcpy(pname, get_name_pad((char *) race_name_pad_female[(int) GET_RACE(ch)], pad));
            sprintf(title, "%s%s", get_size_infra(ch, pad), pname);
            break;
        default:
            strcpy(pname, get_name_pad((char *) race_name_pad_male[(int) GET_RACE(ch)], pad));
            sprintf(title, "%s%s", get_size_infra(ch, pad), pname);
            break;
    }

    return (title);
}

char *hide_race(struct char_data *ch, int pad)
{
    static char title[MAX_STRING_LENGTH];
    struct obj_data *obj;

    obj = check_incognito(ch);

    if (obj) {
        if (!pad)
            sprintf(title, "%s в %s", race_name[(int) GET_RACE(ch)][(int) GET_SEX(ch)],
                    GET_OBJ_PNAME(obj, 5));
        else
            switch (GET_SEX(ch)) {
                case SEX_MALE:
                    sprintf(title, "%s в %s",
                            get_name_pad((char *) race_name_pad_male[(int) GET_RACE(ch)], pad,
                                         PAD_MONSTER), GET_OBJ_PNAME(obj, 5));
                    break;
                case SEX_FEMALE:
                    sprintf(title, "%s в %s",
                            get_name_pad((char *) race_name_pad_female[(int) GET_RACE(ch)], pad,
                                         PAD_MONSTER), GET_OBJ_PNAME(obj, 5));
                    break;
                default:
                    sprintf(title, "%s в %s", race_name[(int) GET_RACE(ch)][(int) GET_SEX(ch)],
                            GET_OBJ_PNAME(obj, 5));
                    break;
            }
    } else
        sprintf(title, "%s", GET_PAD(ch, pad));

    return (title);
}

char *race_or_title2(struct char_data *ch)
{
    static char title[MAX_STRING_LENGTH];

    /* if (!strcmp(GET_PAD(ch,0),"Груумш"))
       {
       sprintf(title,"Орк Груумш, заведующий Справочным Бюро Сигила");
       return title;
       } */

    if (!GET_TITLE(ch) || !*GET_TITLE(ch))
        sprintf(title, "%s, %s", GET_PAD(ch, 0), race_name[(int) GET_RACE(ch)][(int) GET_SEX(ch)]);
    else
        strcpy(title, only_title(ch));
    return CAP(title);
}

//Проверяем находятся ли victim в одной группе c ch
int same_group(struct char_data *ch, struct char_data *tch)
{
    if (!ch || !tch)
        return (FALSE);

    if (IS_NPC(ch) && ch->party_leader && !IS_NPC(ch->party_leader))
        ch = ch->party_leader;
    if (IS_NPC(tch) && tch->party_leader && !IS_NPC(tch->party_leader))
        tch = tch->party_leader;

// Если мастер и там и там NPC или мастер одинаковый то мы в одной группе
    if ((!IS_AFFECTED(ch, AFF_CHARM) && !IS_AFFECTED(tch, AFF_CHARM) && IS_NPC(ch) && IS_NPC(tch))
        || ch == tch)
        return (TRUE);

    if (MOB_FLAGGED(tch, MOB_ANGEL) && ch == tch->master)
        return (TRUE);

    if (ch->master == tch ||
        tch->master == ch ||
        (ch->master == tch || (ch->master && ch->master == tch->master) || tch->master == ch)
        )
        return (TRUE);

//Если нет флага группа
    if (!AFF_FLAGGED(ch, AFF_GROUP) || !AFF_FLAGGED(tch, AFF_GROUP))
        return (FALSE);

    if (ch->party_leader == tch ||
        tch->party_leader == ch ||
        (ch->party_leader == tch || (ch->party_leader && ch->party_leader == tch->party_leader)
         || tch->party_leader == ch)
        )
        return (TRUE);


    return (FALSE);

}


//ADDED BY SLOWN
#define MAX_FORMAT_BUF 3000

char *string_corrector_load(char *Buffer)
{
    extern int CharIsPoint(char);
    int PtrIn, PtrOut;
    char PrvChar;


    PtrOut = 0;
    PrvChar = 0;
    for (PtrIn = 0; PtrIn < (int) strlen(Buffer); PtrIn++) {
        if (Buffer[PtrIn] == '\t')
            continue;
        if (Buffer[PtrIn] == '"')
            continue;
        if (Buffer[PtrIn] == 0x0A || Buffer[PtrIn] == 0x0D)
            continue;

        if (Buffer[PtrIn] == ' ' && (PrvChar == ' ' || PtrOut == 0))
            // Если пробел стоит после пробела или является первым
            // в строке
            continue;
        PrvChar = Buffer[PtrOut] = Buffer[PtrIn];
        if (PtrOut > 0 && Buffer[PtrOut - 1] == ' ' && CharIsPoint(PrvChar))
            Buffer[PtrOut - 1] = PrvChar;
        else
            PtrOut++;
    }
    Buffer[PtrOut] = 0;

    return Buffer;

}

int get_extralevel(char *name)
{
    int i;

    for (i = 0; i < ExtraSkillValuesCol; i++)
        if (!str_cmp(name, ExtraSkillNames[i]))
            return i;

    return (0);
}

int get_sphere(char *name)
{
    int i;

    for (i = 0; i < NUM_SPHERE; i++)
        if (!str_cmp(name, sphere_name[i]))
            return i;

    return (-1);
}

int get_position(char *name)
{
    int i;

    for (i = 0; i < POS_NUMBER; i++)
        if (!str_cmp(name, position_types_sml[i]))
            return i;

    return (POS_STANDING);
}

int check_spell_mob(struct char_data *ch, int spellnum)
{

    if (IS_SET(GET_SPELL_TYPE(ch, SPELL_NO(spellnum)), SPELL_TEMP | SPELL_KNOW) &&
        GET_MANA(ch) >= GET_MANA_COST(ch, spellnum) && !GET_SPELL_MEM(ch, SPELL_NO(spellnum)))
        return (TRUE);

    return (FALSE);
}

void do_mob_cast(struct char_data *ch, struct char_data *victim, int spellnum)
{
    int casting;

    if (!ch || !victim)
        return;

    if ((casting = cast_spell(ch, victim, NULL, spellnum)) > 0) {

        if (victim && SPELL_VLAG(spellnum) > 0)
            WAIT_STATE(victim, PULSE_VIOLENCE * SPELL_VLAG(spellnum));

        if (casting > 0 && ch && SPELL_MEMORY(spellnum) > 0)
            GET_SPELL_MEM(ch, SPELL_NO(spellnum)) = SPELL_MEMORY(spellnum);

        if (casting > 0 && ch && SPELL_LAG(spellnum) > 0)
            WAIT_STATE(ch, PULSE_VIOLENCE * SPELL_LAG(spellnum));
    }
}

long get_dsu_exp(struct char_data *ch)
{
    return (get_levelexp(ch, GET_LEVEL(ch) + 1, 1) - GET_EXP(ch));
}

#define NEXT_EXP_LVL 50000000
long get_levelexp(struct char_data *ch, int level, int add)
{
    int result;

    if (!IS_MAX_EXP(ch))
        result = level_exp(ch, level);
    else
        result = 0;

    return result;
}


long get_level_mob(int level)
{
    int result;

    if (level < LVL_IMMORT) {
        result = level_exp_mob(level);
    } else {
        result = level_exp_mob(LVL_IMMORT) + (NEXT_EXP_LVL * (level - 30));
    }

    return result;
}


void add_death_obj(struct char_data *mob, int vnum, int perc)
{
    struct list_obj_data *k;

    CREATE(k, struct list_obj_data, 1);

    k->vnum = vnum;
    k->percent = perc;
    k->next = mob->load_death;
    mob->load_death = k;
}

void add_eq_obj(struct char_data *mob, int vnum, int pos)
{
    struct list_obj_data *k;

    CREATE(k, struct list_obj_data, 1);

    k->vnum = vnum;
    k->percent = pos;
    k->next = mob->load_eq;
    mob->load_eq = k;
}

void add_tatoo_obj(struct char_data *mob, int vnum, int pos)
{
    struct list_obj_data *k;

    CREATE(k, struct list_obj_data, 1);

    k->vnum = vnum;
    k->percent = pos;
    k->next = mob->load_tatoo;
    mob->load_tatoo = k;
}

void add_inv_obj(struct char_data *mob, int vnum, int perc)
{
    struct list_obj_data *k;

    CREATE(k, struct list_obj_data, 1);

    k->vnum = vnum;
    k->percent = perc;
    k->next = mob->load_inv;
    mob->load_inv = k;
}

void check_position(struct char_data *ch)
{
    if (!IS_NPC(ch))
        return;

    if (GET_POS(ch) <= POS_SITTING && !GET_MOB_HOLD(ch) && !GET_WAIT(ch))
        do_stand(ch, 0, 0, 0, 0);
}


void tascii(int *pointer, int num_planes, char *ascii)
{
    int i, c, found;

    for (i = 0, found = FALSE; i < num_planes; i++) {
        for (c = 0; c < 31; c++)
            if (*(pointer + i) & (1 << c)) {
                found = TRUE;
                sprintf(ascii + strlen(ascii), "%c%d", c < 26 ? c + 'a' : c - 26 + 'A', i);
            }
    }

    strcat(ascii, "");
}

char *ascii_time(time_t t)
{
    static char tbuf[20];
    struct tm *tb;

    tb = localtime(&t);
    sprintf(tbuf, "%02d.%02d.%02d %02d:%02d.%02d",
            tb->tm_mday, tb->tm_mon + 1, tb->tm_year + 1900, tb->tm_hour, tb->tm_min, tb->tm_sec);
    return tbuf;
}

int max_exp_loss_pc(struct char_data *ch)
{

    int result;

    if (!GET_LEVEL(ch))
        result = 375;
    else
        result = (get_levelexp(ch, GET_LEVEL(ch) + 1, 1) - get_levelexp(ch, GET_LEVEL(ch), 1)) / 4;
    return result;
}

int max_exp_flee_pc(struct char_data *ch)
{
    int result;

    if (!GET_LEVEL(ch))
        result = 0;
    else
        result = (get_levelexp(ch, GET_LEVEL(ch) + 1, 1) - get_levelexp(ch, GET_LEVEL(ch), 1)) / 50;
    return result;
}

int max_exp_gain_pc(struct char_data *ch)
{
    /* int result=0;
       int koef = 10;

       if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM))
       result = 1;
       else
       if (!GET_LEVEL(ch))
       result = 30;
       else
       if (GET_LEVEL(ch) == 1)
       result = get_levelexp(ch,GET_LEVEL(ch)+1,1)/10;
       else
       {
       switch(GET_LEVEL(ch))
       {
       case 0:
       case 1:
       koef = 10;
       break;
       case 2:
       case 3:
       koef = 25;
       break;
       }
       result = (get_levelexp(ch,GET_LEVEL(ch)+1,1) -
       get_levelexp(ch,GET_LEVEL(ch),1))/koef;
       }

       return result;
     */
    return 100000;
}

int max_exp_hit_pc(struct char_data *ch)
{

    return (IS_NPC(ch) ? 1 : (get_levelexp(ch, GET_LEVEL(ch), 1) -
                              get_levelexp(ch, GET_LEVEL(ch) - 1, 1)) / 2000);
}

struct char_data *random_victim(struct char_data *ch, struct char_data *vict, int attacktype)
{
    struct char_data *tch, *vch = NULL;
    char buf[MAX_STRING_LENGTH];

    /* if ((IS_NPC(ch) && !AFF_FLAGGED(ch,AFF_CHARM)) &&
       !IS_NPC(tch) && dice(1,100) < 20)
       return (tch); */

    for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room)
        if ((!IS_NPC(ch) || (IS_NPC(ch) && AFF_FLAGGED(ch, AFF_CHARM))) &&
            IS_NPC(tch) && tch != ch && !AFF_FLAGGED(tch, AFF_CHARM) &&
            !MOB_FLAGGED(tch, MOB_NOFIGHT) && !IS_SHOPKEEPER(tch)
            && (number(1, 60) > attacktype)) {
            vch = tch;
            break;
        }


    if (vch && vch != vict) {
        sprintf(buf, "&R$N промахнул$N по %s, случайно задев Вас.&n", GET_PAD(vict, 2));
        act(buf, FALSE, vch, 0, ch, TO_CHAR);

        sprintf(buf, "&YВы промахнулись по %s, случайно задев $n3.&n", GET_PAD(vict, 2));
        act(buf, FALSE, vch, 0, ch, TO_VICT);

        sprintf(buf, "&R$N промахнулся по Вам, случайно задев %s.&n", GET_PAD(vch, 3));
        act(buf, FALSE, vict, 0, ch, TO_CHAR);

        sprintf(buf, "$N промахнулся по %s, случайно задев $n3.", GET_PAD(vict, 2));
        act(buf, FALSE, vch, 0, ch, TO_NOTVICT);

    } else {
        vch = vict;
    }

    return (vch);
}

struct obj_data *check_incognito(struct char_data *ch)
{
    if (GET_EQ(ch, WEAR_FACE) && OBJ_FLAGGED(GET_EQ(ch, WEAR_FACE), ITEM_INCOGNITO))
        return GET_EQ(ch, WEAR_FACE);
    else if (GET_EQ(ch, WEAR_HEAD) && OBJ_FLAGGED(GET_EQ(ch, WEAR_HEAD), ITEM_INCOGNITO))
        return GET_EQ(ch, WEAR_HEAD);
    else if (GET_EQ(ch, WEAR_ABOUT) && OBJ_FLAGGED(GET_EQ(ch, WEAR_ABOUT), ITEM_INCOGNITO))
        return GET_EQ(ch, WEAR_ABOUT);

    return NULL;
}

/*


-120 - ледяное
 -70 - очень холодное
 -30 - холодное
 -10 - прохладное
   0 - нормальное
  30 - теплое
  60 - горячее
  90 - очень горячее
 120 - раскаленое
*/

int get_const_obj_temp(struct obj_data *obj)
{
    int result;

    if (GET_OBJ_REAL_TEMP(obj) <= -120)
        result = 0;
    else if (GET_OBJ_REAL_TEMP(obj) <= -70)
        result = 1;
    else if (GET_OBJ_REAL_TEMP(obj) <= -30)
        result = 2;
    else if (GET_OBJ_REAL_TEMP(obj) <= -10)
        result = 3;
    else if (GET_OBJ_REAL_TEMP(obj) <= 30)
        result = 4;
    else if (GET_OBJ_REAL_TEMP(obj) <= 60)
        result = 5;
    else if (GET_OBJ_REAL_TEMP(obj) <= 90)
        result = 6;
    else if (GET_OBJ_REAL_TEMP(obj) < 120)
        result = 7;
    else
        result = 8;

    return result;
}

/* Добавлением персонажей кто смог найти ловушку */
void add_tfind_char(struct char_data *ch, struct room_direction_data *exit)
{
    exit->tfind.insert(ch);
}

/* Добавлением персонажей в призматическую сферу */
void add_psphere_char(struct char_data *ch, struct room_data *room)
{
    room->p_sphere.insert(ch);
}

/* Список персонажей что нашел ловушку у предмета */
void add_tfind_char(struct char_data *ch, struct obj_data *obj)
{
    obj->trap->tfind.insert(ch);
}

/* добавляем/изменяем список игроков которые видят этот обьект */
void add_obj_visible(struct obj_data *obj, struct char_data *ch)
{
    obj->visible_by.insert(ch);
}

/* проверка на персонажа на то что он видит ловушку */
bool check_tfind_char(struct char_data *ch, struct room_direction_data *exit)
{
    return exit->tfind.count(ch);
}

/* проверка на персонажа на то что он в призрачной сфере */
bool check_psphere_char(struct char_data * ch, struct room_data * room)
{
    return room->p_sphere.count(ch);
}

/* Проверка персонажа что он видет ловушку на предмете */
bool check_tfind_char(struct char_data * ch, struct obj_data * obj)
{
    return obj->trap->tfind.count(ch);
}

/* проверка на видимость объкта игроку */
bool check_obj_visible(const struct char_data * ch, const struct obj_data * obj)
{
    return obj->visible_by.count(ch);
}

/* добавляем/изменяем список игроков которые видят жертву */
void add_victim_visible(struct char_data *ch, struct char_data *victim)
{
    victim->visible_by.insert(ch);
}

/* проверка на видимость жертвы игроку */
int check_victim_visible(struct char_data *ch, struct char_data *victim)
{
    return victim->visible_by.count(ch);
}


/* добавляем/изменяем список игроков которые могут убить жертву */
void add_victim_may_attack(struct char_data *ch, struct char_data *victim)
{
    ch->may_attack.insert(victim);
}

/* Добавлем/изменяем список дистанции до врага */
void add_distance(struct char_data *ch, struct char_data *victim, int distance, bool show)
{
    distance_list::iterator d = ch->distance.find(victim);

    // для этого врага уже была задана дистанция
    if (d != ch->distance.end()) {
        if (d->second != distance && show) {
            if (distance == DIST_0) {
                act("Вы приблизились на несколько шагов к 2д.", "Мм", ch, victim);
                act("1и приблизил1(ся,аст,ост,ись) к Вам на несколько шагов.", "мМ", ch, victim);
                act("1и приблизил1(ся,аст,ост,ись) на несколько шагов к 2д.", "Кмм", ch, victim);
            } else if (distance == DIST_1) {
                act("Вы удалились на несколько шагов от 2в.", "Мм", ch, victim);
                act("1и удалил1(ся,аст,ост,ись) от Вас на несколько шагов.", "мМ", ch, victim);
                act("1и удалил1(ся,аст,ост,ись) от 2в на несколько шагов.", "Кмм", ch, victim);
            }
        }

        d->second = distance;
        return;
    }
    // новый враг
    ch->distance[victim] = distance;
}

int get_flee(struct char_data *ch, int id)
{
    return ch->flee_by[id] + 1;
}

void add_flee(struct char_data *ch, struct char_data *flee)
{
    ch->flee_by[GET_ID(flee)]++;
}

void clean_flee(struct char_data *ch)
{
    ch->flee_by.clear();
}

void clean_distance(struct char_data *ch)
{
    ch->distance.clear();
    ch->char_specials.chage_distance = 0;
}

/* Возвращаем дистанцию */
int check_distance(struct char_data *ch, struct char_data *victim)
{
    distance_list::iterator d = ch->distance.find(victim);

    if (d != ch->distance.end())
        return d->second;

    return DIST_NOTFOUND;
}


/* добавляем/изменяем список игроков которые могут приблизится */
void add_victim_may_moved(struct char_data *ch, struct char_data *victim)
{
    ch->may_moved.insert(victim);
}

/* проверка на приближение */
bool check_victim_may_moved(struct char_data *ch, struct char_data *victim)
{
    return ch->may_moved.count(victim);
}


/* добавляем/изменяем список игроков которые не могут убить жертву */
void add_victim_not_attack(struct char_data *ch, struct char_data *victim)
{
    ch->not_attack.insert(victim);
}

/* проверка на атаку */
bool check_victim_may_attack(struct char_data *ch, struct char_data *victim)
{
    return ch->may_attack.count(victim);
}

/* проверка на атаку */
bool check_victim_not_attack(struct char_data * ch, struct char_data * victim)
{
    return ch->not_attack.count(victim);
}

/* добавляем/изменяем список игроков которые не могут приблизится */
void add_victim_not_moved(struct char_data *ch, struct char_data *victim)
{
    ch->not_moved.insert(victim);
}

/* проверка на отторжение */
bool check_victim_not_moved(struct char_data *ch, struct char_data *victim)
{
    return ch->not_moved.count(victim);
}

bool check_toroom_repulsion(struct char_data * ch, int in_room)
{
    struct char_data *tch;
    struct follow_type *f;
    bool result = FALSE;

    if (in_room == NOWHERE)
        return (FALSE);

    for (tch = world[in_room].people; tch; tch = tch->next_in_room) {
        if (affected_by_spell(tch, SPELL_REPULSION)) {
            if (same_group(ch, tch))
                continue;

            if (check_victim_not_moved(tch, ch))
                result = TRUE;
            else {              //делаем спас бросок
                int ll = MAX(1, affected_by_spell(tch, SPELL_REPULSION));

                //1..15%
                if (!general_savingthrow_3(ch, SAV_WILL, ll / 10)) {
                    result = TRUE;
                    add_victim_not_moved(tch, ch);
                }
            }
            for (f = tch->followers; f; f = f->next)
                if(same_group(ch, f->follower))
                    result = FALSE;

            if (result)
                break;
        }
    }
    return (result);
}

void horse_master_change(struct char_data *ch, struct char_data *horse)
{
    char buf[MAX_STRING_LENGTH];

    sprintf(buf, "$N увел$G у Вас %s.", GET_PAD(horse, 1));
    act(buf, FALSE, horse->master, 0, ch, TO_CHAR);
    sprintf(buf, "Вы увели %s у $n1.", GET_PAD(horse, 1));
    act(buf, FALSE, horse->master, 0, ch, TO_VICT);
    inc_pk_thiefs(ch, horse->master);
    stop_follower(horse, SF_EMPTY);
    make_horse(horse, ch);
}

int is_spell_type(int spell_no)
{
    int i;

//проверяем защитные спеллы
    for (i = 0; spells_defence[i] != -1; i++)
        if (spell_no == spells_defence[i])
            return (SPL_DEF);

//проверяем групповые защитные спеллы
    /* for(i = 0; spells_defence_group[i]!=-1; i++)
       if (spell_no == spells_defence_group[i]) return (SPL_DEF);
     */

    return (FALSE);
}


int get_skill_class_level(struct char_data *ch, int skill_no)
{
    int clas, level = 0;
    int gods = GET_GODS(ch);

    if (skill_info[skill_no].skill_type == SKILL_TLANG)
        return (100);

    if (skill_no == SKILL_AXES && IS_GNOME(ch))
        return (GET_LEVEL(ch));

    if (IS_GOD(ch))
        return (GET_LEVEL(ch));

    if (SET_SKILL(ch, skill_no))
        for (clas = 0; clas < NUM_CLASSES; clas++) {
            if (skill_info[skill_no].learn_level[clas][gods])
                level += check_class(ch, clas);
        }
    return level;
}

void check_enter_trap(struct char_data *ch, int dir)
{
    int room = IN_ROOM(ch);

    if (world[room].trap_option[dir] && world[room].trap_option[dir]->chance > number(0, 100)) {
        /*act(" ",FALSE,ch,0,0,TO_CHAR);
           act(" ",FALSE,ch,0,0,TO_ROOM);
           act(world[room].trap_option[dir]->trap_active_char,FALSE,ch,0,0,TO_CHAR);
           act(world[room].trap_option[dir]->trap_active_room,FALSE,ch,0,0,TO_ROOM);
           send_to_char(CCIRED(ch, C_CMP), ch);
           act(world[room].trap_option[dir]->damage_mess_char,FALSE,ch,0,0,TO_CHAR);
           send_to_char(CCNRM(ch, C_CMP), ch);
           act(world[room].trap_option[dir]->damage_mess_room,FALSE,ch,0,0,TO_ROOM);
           int dam = dice(world[room].trap_option[dir]->damnodice,world[room].trap_option[dir]->damsizedice)+world[room].trap_option[dir]->damage;
           GET_HIT(ch) -= dam;
         */

        struct P_damage damage;
        struct P_message message;

        GetEnterTrapMessage(world[room].trap_option[dir], message);

        int dam =
            dice(world[room].trap_option[dir]->damnodice,
                 world[room].trap_option[dir]->damsizedice) + world[room].trap_option[dir]->damage;

        damage.valid = true;
        damage.type = world[room].trap_option[dir]->type_hit;
        damage.power = 0;
        damage.check_ac = A_POWER;
        damage.far_min = TRUE;
        damage.armor = FALSE;   //броня учитывается
        damage.weapObj = NULL;
        damage.dam = dam;

        act(" ", FALSE, ch, 0, 0, TO_CHAR);
        act(" ", FALSE, ch, 0, 0, TO_ROOM);
        act(world[room].trap_option[dir]->trap_active_char, FALSE, ch, 0, 0, TO_CHAR);
        act(world[room].trap_option[dir]->trap_active_room, FALSE, ch, 0, 0, TO_ROOM);

        if (_damage(ch, ch, 0, 0, A_POWER, FALSE, damage, message) == RD_KILL) {
            //sprintf(buf,"%s умер от входа в локацию #%d",GET_NAME(ch),world[inroom].number);
            //mudlog(buf,CMP,LVL_GOD,TRUE);
        }

    }
}

void add_shop_type(struct char_data *mob, int type)
{
    struct list_obj_data *k;
    struct mob_shop_data *m;

    m = mob->shop_data;

    CREATE(k, struct list_obj_data, 1);

    k->vnum = type;
    k->next = m->obj_type;
    m->obj_type = k;
}


void add_shop_obj(struct char_data *mob, int vnum, int count)
{
    struct list_obj_data *k;
    struct mob_shop_data *m;
    int found = FALSE;

    m = mob->shop_data;

    for (k = m->obj_list; k; k = k->next)
        if (k->vnum == vnum) {
            if (k->percent != -1) {
                k->percent += count;
                int r_num;

                if ((r_num = real_object(vnum)) > -1)
                    obj_index[r_num].number += count;
            }
            found = TRUE;
        }

    if (!found) {
        CREATE(k, struct list_obj_data, 1);

        //k->vnum = real_object(vnum);
        k->vnum = vnum;
        k->percent = count;
        int r_num;

        if ((r_num = real_object(vnum)) > -1)
            obj_index[r_num].number += count;

        k->next = m->obj_list;
        m->obj_list = k;
//log ("Добавили %d стало %d",vnum,k->percent);
    }


}

void del_shop_obj(struct char_data *mob, int vnum, int count)
{
    struct list_obj_data *temp, *i;
    struct mob_shop_data *m;

    m = mob->shop_data;

//log("vnum %d count %d",vnum,count);
    for (i = m->obj_list; i; i = i->next) {
        //log("test %d=%d && %d",vnum,i->vnum,i->percent);
        if (i->vnum == vnum && i->percent != -1) {
            i->percent = MAX(0, i->percent - count);
            //log("Отняли %d стало %d",count,i->percent);
            if (i->percent == 0) {
                REMOVE_FROM_LIST(i, m->obj_list, next);
            }
            int r_num;

            if ((r_num = real_object(vnum)) > -1)
                obj_index[r_num].number -= count;
        }
    }
}


void add_spechit(struct char_data *mob, byte type, int hit, int spell, int pos[POS_NUMBER],
                 ubyte dnd, ubyte dsd, int dam,
                 ubyte percent, int power,
                 const char *to_victim, const char *to_room, int saves[NUM_SAV], char *property)
{
    struct mob_spechit_data *k;
    int i;

    CREATE(k, struct mob_spechit_data, 1);

    k->type = type;
    k->hit = hit;
    k->percent = percent;
    k->power = power;
    k->spell = spell;
    k->damnodice = dnd;
    k->damsizedice = dsd;
    k->damage = dam;

    if (property)
        asciiflag_conv(property, &k->property);
    else
        k->property = clear_flags;

    if (to_victim) {
        CREATE(k->to_victim, char, strlen(to_victim) + 1);

        strcpy(k->to_victim, to_victim);
        PHRASE(k->to_victim);
    }

    if (to_room) {
        CREATE(k->to_room, char, strlen(to_room) + 1);

        strcpy(k->to_room, to_room);
        PHRASE(k->to_room);
    }

    if (pos)
        for (i = 0; i < POS_NUMBER; i++)
            k->pos[i] = pos[i];

    if (saves)
        for (i = 0; i < NUM_SAV; i++)
            k->saves[i] = saves[i];

    k->next = mob->spec_hit;
    mob->spec_hit = k;

//log ("[%s] ДОБАВИЛИ СПЕЦУДАР ТИП %d dam %d %d %d",GET_NAME(mob),k->type,k->damnodice,k->damsizedice,k->damage);
}

void add_materials_proto(struct obj_data *obj)
{
    if (GET_OBJ_RNUM(obj) < 0)
        return;

    struct obj_data *pobj = &obj_proto[GET_OBJ_RNUM(obj)];
    struct material_data_list *m;

    obj->materials = 0;

    for (m = pobj->materials; m; m = m->next)
        add_material(obj, m->number_mat, m->value, m->main);

}

void add_material(struct obj_data *obj, int type, int value, int main)
{
    struct material_data_list *k;

    CREATE(k, struct material_data_list, 1);

    k->number_mat = type;
    k->value = value;
    k->main = main;

    k->next = obj->materials;
    obj->materials = k;
}

//Добавляем в список повреждения в конце раунда
void add_last_damage(struct char_data *ch, struct char_data *victim, int number,
                     struct P_damage &damage, struct P_message &message)
{
    LastDamageStorage::iterator l = victim->Ldamage.find(number);

    // если данные для этого скила уже были, просто увеличиваем размер повреждения
    if (l != victim->Ldamage.end()) {
        l->second.damage.dam += damage.dam;
    }
    // создаем новую структуру
    else {
        struct P_last_damage ld;

        ld.id_ch = GET_ID(ch);
        // присваивание структур по полям 
        ld.damage = damage;
        ld.message = message;
        victim->Ldamage[number] = ld;
    }
}

char *get_material_name(int type, int pad)
{
    struct material_data *k;

    for (k = material_table; k; k = k->next)
        if (k->number == type)
            return (get_name_pad(k->name, pad, PAD_OBJECT));

    return (NULL);
}

struct material_data *get_material_param(int type)
{
    struct material_data *k;

    for (k = material_table; k; k = k->next)
        if (k->number == type)
            return (k);

    return (NULL);
}

// Проверяет принадлежность к металлу
int is_metall(struct obj_data *obj)
{
    struct material_data_list *m;

    for (m = obj->materials; m; m = m->next)
        if (get_material_param(m->number_mat)->type == TYPE_METALL && m->main == TRUE)
            return (TRUE);

    return (FALSE);
}

int equip_in_metall(struct char_data *ch)
{
    int i, met_wgt = 0, all_wgt = 1;
    struct obj_data *obj;
    struct material_data_list *m;

    if (IS_NPC(ch) && !AFF_FLAGGED(ch, AFF_CHARM))
        return (FALSE);

    for (i = 0; i < NUM_WEARS; i++) {
        if (!GET_EQ(ch, i))
            continue;

        obj = GET_EQ(ch, i);

        //штраф идет только за доспехи
        if (GET_OBJ_TYPE(obj) != ITEM_ARMOR)
            continue;

        if (!obj->materials)
            continue;

        for (m = obj->materials; m; m = m->next) {
            if (get_material_param(m->number_mat)->type == TYPE_METALL && m->main)
                met_wgt += m->value * get_material_param(m->number_mat)->weight;
            all_wgt += m->value * get_material_param(m->number_mat)->weight;
        }
    }

    return ((100 * met_wgt) / all_wgt);
}

int get_durab_obj(struct obj_data *obj)
{
    int perc = 0;
    long wgt = 1;
    struct material_data_list *m;

    if (!obj->materials)
        return (FALSE);

    for (m = obj->materials; m; m = m->next) {
        wgt += ((m->value * get_material_param(m->number_mat)->weight)) *
            get_material_param(m->number_mat)->durab;
        perc += m->value * get_material_param(m->number_mat)->weight;
    }

    if (perc)
        return (wgt / perc);
    else
        return (1);

}

int get_save_obj(struct obj_data *obj, int save)
{

    int perc = 0;
    long wgt = 1;
    struct material_data_list *m;

    if (!obj || !obj->materials)
        return (FALSE);

    for (m = obj->materials; m; m = m->next) {
        wgt += ((m->value * get_material_param(m->number_mat)->weight)) *
            get_material_param(m->number_mat)->save[save];
        perc += m->value * get_material_param(m->number_mat)->weight;
    }

    return (int) ((float) wgt / (float) perc);
}

void add_container_obj(struct obj_data *obj, int vnum, int count)
{
    struct list_obj_data *k;

    CREATE(k, struct list_obj_data, 1);

    //k->vnum = real_object(vnum);
    k->vnum = vnum;
    k->percent = count;
    k->next = obj->load_obj;
    obj->load_obj = k;
    //log ("Добавили %d стало %d",vnum,k->percent);
}


int check_fight_command(struct char_data *ch)
{
    if (GET_POS(ch) == POS_FIGHTING || FIGHTING(ch)) {
        send_to_char("Вы сражаетесь за свою жизнь!\r\n", ch);
        return (FALSE);
    }

    return (TRUE);
}

int check_transport_command(struct char_data *ch)
{
    if (ch->is_transpt) {
        act("Необходимо выбраться из $o1.", FALSE, ch, ch->is_transpt, 0, TO_CHAR);
        return (FALSE);
    }

    return (TRUE);
}


int check_wld_damage(int bits, struct char_data *victim)
{
    int result = FALSE;

    if (!bits)
        return (TRUE);

    if (IS_SET(bits, DAMAGE_ALL))
        result = TRUE;

    if (IS_SET(bits, DAMAGE_PC)
        && (!IS_MOB(victim) || (IS_MOB(victim) && AFF_FLAGGED(victim, AFF_CHARM))))
        result = TRUE;

    if (IS_SET(bits, DAMAGE_NPC) && IS_MOB(victim) && !AFF_FLAGGED(victim, AFF_CHARM))
        result = TRUE;

    if (IS_SET(bits, DAMAGE_STEP) && !AFF_FLAGGED(victim, AFF_LEVIT)
        && !AFF_FLAGGED(victim, AFF_FLY))
        result = TRUE;

    if (IS_SET(bits, DAMAGE_LEVIT) && AFF_FLAGGED(victim, AFF_LEVIT)
        && GET_POS(victim) == POS_FLYING)
        result = TRUE;

    if (IS_SET(bits, DAMAGE_FLY) && AFF_FLAGGED(victim, AFF_FLY) && GET_POS(victim) == POS_FLYING)
        result = TRUE;

    if (result && IS_SET(bits, DAMAGE_ONES) && IN_ROOM(victim) != victim->last_room_dmg)
        result = TRUE;
    else
        result = FALSE;

    return (result);
}

//IS_SET(world[count].damage->type, FIND_OBJ_INV)


int get_main_material_type(struct obj_data *obj)
{
    struct material_data_list *m;

    for (m = obj->materials; m; m = m->next) {
        if (m->main)
            return (get_material_param(m->number_mat)->type);
    }

    return (0);
}

void create_fragments(struct obj_data *object)
{
    struct obj_data *corpse;
    char buf2[MAX_STRING_LENGTH];

    if (GET_OBJ_TYPE(object) == ITEM_FRAGMENT)
        return;

    corpse = create_obj();
    corpse->item_number = NOTHING;
    corpse->in_room = NOWHERE;

    sprintf(buf2, "обломки %s", object->names);
    corpse->names = str_dup(buf2);

    sprintf(buf2, "обломки %s лежат здесь.", GET_OBJ_PNAME(object, 1));
    corpse->description = str_dup(buf2);
    sprintf(buf2, "обломки %s", GET_OBJ_PNAME(object, 1));
    corpse->short_description = str_dup(buf2);
    sprintf(buf2, "обломки %s", GET_OBJ_PNAME(object, 1));
    corpse->PNames[0] = str_dup(buf2);
    corpse->name = str_dup(buf2);
    sprintf(buf2, "обломоков %s", GET_OBJ_PNAME(object, 1));
    corpse->PNames[1] = str_dup(buf2);
    sprintf(buf2, "обломкам %s", GET_OBJ_PNAME(object, 1));
    corpse->PNames[2] = str_dup(buf2);
    sprintf(buf2, "обломки %s", GET_OBJ_PNAME(object, 1));
    corpse->PNames[3] = str_dup(buf2);
    sprintf(buf2, "обломками %s", GET_OBJ_PNAME(object, 1));
    corpse->PNames[4] = str_dup(buf2);
    sprintf(buf2, "обломках %s", GET_OBJ_PNAME(object, 1));
    corpse->PNames[5] = str_dup(buf2);
    GET_OBJ_SEX(corpse) = SEX_POLY;

    GET_OBJ_TYPE(corpse) = ITEM_FRAGMENT;
    GET_OBJ_WEAR(corpse) = ITEM_WEAR_TAKE;
    GET_OBJ_EXTRA(corpse, ITEM_NODONATE) |= ITEM_NODONATE;
    GET_OBJ_EXTRA(corpse, ITEM_NOSELL) |= ITEM_NOSELL;
    GET_OBJ_EXTRA(corpse, ITEM_NOSELL) |= ITEM_NORENT;
    GET_OBJ_WEIGHT(corpse) = MAX(1, GET_OBJ_WEIGHT(object) / 2);
    GET_OBJ_MAX(corpse) = 100;
    GET_OBJ_CUR(corpse) = 10;


    GET_OBJ_TIMER(corpse) = 5;
    struct material_data_list *m;

    for (m = object->materials; m; m = m->next)
        add_material(corpse, m->number_mat, m->value, m->main);

    obj_to_room(corpse, IN_ROOM(object));
}

///////////////////////////////////////////////////////////////////////////////
// Функции вырезания лишних символов
//                                                  ИЗМЕНЕНА 14.09.2003 (Viper)
// Вне особых зон иничтожаются лишние пробелы и символы табуляции
// В спец. зонах оставляет неизменное количество пробелов,
// однако табуляция уничтожаются
//
// Изменение: Переводы строк больше не уничтожаются
char *string_corrector(char *Buffer)
{
    extern int CharIsPoint(char);
    int PtrIn, PtrOut;
    char PrvChar;
    bool SpZone = false;        // Текущее состояние проверки (02.09.2003)

    // Первый проход. Считаем количество символов $
    int NumSp = 0;

    for (PtrIn = 0; PtrIn < (int) strlen(Buffer); PtrIn++)
        if (Buffer[PtrIn] == '$')
            NumSp++;
    // Если их количество нечетно, то убиваем один из них (последний)
    if (NumSp % 2 != 0) {
        PtrIn = strlen(Buffer) - 1;
        while (Buffer[PtrIn] != '$')
            PtrIn--;
        Buffer[PtrIn] = ' ';
    }
    // Второй проход
    for (PtrIn = 0; PtrIn < (int) strlen(Buffer); PtrIn++) {
        if (Buffer[PtrIn] == '$') {
            if (SpZone)
                SpZone = false;
            else
                SpZone = true;
            continue;
        }
        if (Buffer[PtrIn] == '\t' && SpZone)
            Buffer[PtrIn] = ' ';
        // Переводы строк больше не уничтожаем        (14.09.2003 - Viper)
        //if( (Buffer[PtrIn] == 0x0A || Buffer[PtrIn] == 0x0D) && SpZone )
        // Buffer[PtrIn] = ' ';
    }

    PtrOut = 0;
    PrvChar = 0;
    SpZone = false;             // Третий проход, сброс признака
    for (PtrIn = 0; PtrIn < (int) strlen(Buffer); PtrIn++) {
        if (Buffer[PtrIn] == '$') {
            if (SpZone)
                SpZone = false;
            else
                SpZone = true;
            if (SpZone && PrvChar == ' ')
                // Если перед началом особой зоны стоит пробел,
                // то он нам нах.й не нужен
                PtrOut--;
            if (!SpZone) {
                // Пробелы в конце особой зоны никому не нужны
                while (Buffer[PtrOut - 1] == ' ')
                    PtrOut--;
            }
            // Сохраняем символ особой зоны
            PrvChar = Buffer[PtrOut] = Buffer[PtrIn];
            PtrOut++;
            continue;
        }
        //if(Buffer[PtrIn] == '\t' || Buffer[PtrIn] == 0x0A ||
        // Buffer[PtrIn] == 0x0D)
        // // Эти символы могут быть только внутри особой зоны
        // // Убиваем без разборок
        // continue;
        if (Buffer[PtrIn] == 0x0A) {
            // Удаляем все пробелы перед переводом строки   (14.09.2003)
            while (PtrOut > 0 && Buffer[PtrOut - 1] == ' ')
                PtrOut--;
        }
        if (Buffer[PtrIn] == ' ' && (PrvChar == ' ' || PrvChar == '$' ||
                                     PtrOut == 0 || PrvChar == 0x0A) && !SpZone)
            // Если пробел стоит после пробела, или после особой зоны,
            // или является первым в строке, или после перевода строки
            continue;
        PrvChar = Buffer[PtrOut] = Buffer[PtrIn];
        if (SpZone) {
            PtrOut++;
            continue;
        }
        if (PtrOut > 0 && Buffer[PtrOut - 1] == ' ' && CharIsPoint(PrvChar))
            Buffer[PtrOut - 1] = PrvChar;
        else
            PtrOut++;
    }
    Buffer[PtrOut] = 0;

    return Buffer;
}

///////////////////////////////////////////////////////////////////////////////
int CharIsPoint(char s)
{
    char mp[] = ".,?!';";
    int i;

    for (i = 0; i < (int) strlen(mp); i++)
        if (mp[i] == s)
            return 1;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Форматирование текста по ширине
//                                                  Изменена 14.09.2003 (Viper)
void FLineV(char *, int *, int, int, int);

void myStrWideFormat(const char *BufferIn, char *BufferOut, int Wide, int FVir)
{
    extern int GetWordLen(const char *, int &);
    int PtrIn = 0, PtrOut = 0, LenLine = 0, RealLine = 0;
    bool SpZone = false;

    BufferOut[0] = 0;

    while (PtrIn < (int) strlen(BufferIn)) {
        if (BufferIn[PtrIn] == '$') {
            if (SpZone)
                SpZone = false;
            else
                SpZone = true;
            PtrIn++;
            BufferOut[PtrOut] = '\r';
            PtrOut++;
            BufferOut[PtrOut] = '\n';
            PtrOut++;
            LenLine = 0;
            RealLine = 0;
            continue;
        }
        if (BufferIn[PtrIn] == 0x0A) {
            // Добавлено 14.09.2003   (Viper)
            PtrIn++;
            BufferOut[PtrOut] = '\r';
            PtrOut++;
            BufferOut[PtrOut] = '\n';
            PtrOut++;
            LenLine = 0;
            RealLine = 0;
            continue;
        }
        int RealLen;
        int sz = GetWordLen(&BufferIn[PtrIn], RealLen);

        if (LenLine + sz >= Wide) {
            while (PtrOut > 0 && BufferOut[PtrOut - 1] == ' ') {
                PtrOut--;
                LenLine--;
                RealLine--;
            }
            if (FVir && !SpZone)
                FLineV(BufferOut, &PtrOut, LenLine, Wide, RealLine);
            BufferOut[PtrOut] = '\r';
            PtrOut++;
            BufferOut[PtrOut] = '\n';
            PtrOut++;
            LenLine = 0;
            RealLine = 0;
        }
        memcpy(&BufferOut[PtrOut], &BufferIn[PtrIn], RealLen);
        PtrOut += RealLen;
        PtrIn += RealLen;
        LenLine += sz;
        RealLine += RealLen;
        while (BufferIn[PtrIn] == ' ') {
            BufferOut[PtrOut] = ' ';
            PtrIn++;
            PtrOut++;
            LenLine++;
            RealLine++;
        }
    }
    while (PtrOut > 0 && BufferOut[PtrOut - 1] == ' ')
        PtrOut--;
    if (PtrOut > 0 && (BufferOut[PtrOut - 1] == 0x0A || BufferOut[PtrOut - 1] == 0x0D)) {
        // Пустая строка никому не нужна
        BufferOut[PtrOut] = 0;
        return;
    }
    BufferOut[PtrOut] = '\r';
    PtrOut++;
    BufferOut[PtrOut] = '\n';
    PtrOut++;
    BufferOut[PtrOut] = 0;
}

///////////////////////////////////////////////////////////////////////////////
void FLineV(char *mBuf, int *mPtr, int mLineLen, int Wide, int RealLine)
{
    extern int GetNumWords(char *, int);
    int nWord, nw, pOut, nSpace, ns, i;
    char bb[120], s;

    if (mLineLen >= Wide)
        return;
    nSpace = Wide - mLineLen;
    //*mPtr -= mLineLen;
    *mPtr -= RealLine;
    // Получить количество слов
    nWord = GetNumWords(&mBuf[*mPtr], RealLine);
    if (nWord <= 1)
        // Ну бля...
        return;
    memset(bb, 0, sizeof(bb));
    nw = 0;
    pOut = 0;
    //for(i = 0; i < mLineLen; i ++)
    for (i = 0; i < RealLine; i++) {
        s = mBuf[*mPtr + i];
        bb[pOut] = s;
        pOut++;
        // Слово не началось и пробел
        if (!nw && s == ' ')
            continue;
        // Начало слова
        if (!nw && s != ' ') {
            nw = 1;             // Слово
            nWord--;
            continue;
        }
        // Продолжаем просматривать то же слово
        if (nw && s != ' ')
            continue;
        // Слово кончилось.
        nw = 0;

        ns = nSpace / nWord;
        nSpace -= ns;
        memset(&bb[pOut], ' ', ns);
        pOut += ns;
    }
    memcpy(&mBuf[*mPtr], bb, pOut);
    *mPtr += pOut;
}

///////////////////////////////////////////////////////////////////////////////
int GetNumWords(char *Buf, int Len)
{
    int nw = 0, i;
    int Number = 0;
    char s;

    for (i = 0; i < Len; i++) {
        s = Buf[i];
        // Слово не началось и пробел
        if (!nw && s == ' ')
            continue;
        // Начало слова
        if (!nw && s != ' ') {
            nw = 1;             // Слово
            Number++;
            continue;
        }
        // Продолжаем просматривать то же слово
        if (nw && s != ' ')
            continue;
        // Слово кончилось.
        nw = 0;
    }
    return Number;
}

///////////////////////////////////////////////////////////////////////////////
char *strbraker(const char *BufferIn, int Wide, int FVir)
{
    static char BufferOut[MAX_FORMAT_BUF];

    myStrWideFormat(BufferIn, BufferOut, Wide, FVir);
    return BufferOut;
}

///////////////////////////////////////////////////////////////////////////////
//                                                  Изменена 14.09.2003 (Viper)
int GetWordLen(const char *Str, int &RealLen)
{
    int i = 0;
    int len = 0;

    while (i < (int) strlen(Str)) {
        if (Str[i] == ' ' || Str[i] == '$' || Str[i] == 0x0A)
            break;
        if (Str[i] == '&') {
            if (Str[i + 1] != ' ' && Str[i + 1] != '$' && Str[i + 1] != 0x0A) {
                i += 2;
                continue;
            }
        }
        len++;
        i++;
    }
    RealLen = i;
    return len;
}

///////////////////////////////////////////////////////////////////////////////

// Форматирование текста в рамку. Ширина текста задается с учетом рамки,
// т.е. текст уже на четыре символа (рамка и пробел с каждой стороны).
// В конце текста ставится перевод строки.
//
// ВНИМАНИЕ! В тексте не должно быть символов специальных зон $
// Появление этих символов в тексте может привести к фатальной ошибке.
// Слова также не должны превишать длину width-4
char *strformat(char *text, int width, int FVir, char topl, char top, char topr, char left,
                char right, char bottoml, char bottom, char bottomr)
{
    static char *retbuf = NULL;

    // Удаляем старый буфер (если был)
    if (retbuf != NULL)
        free(retbuf);
    retbuf = NULL;
    //  Форматируем текст по ширине (исходная ширина - 4)
    char *ret = strbraker(text, width - 4, FVir);

    // Подсчет количества строк в тексте
    int numln = 0;

    for (char *pb = ret; *pb != 0; pb++)
        if (*pb == '\n')
            numln++;
    numln++;                    // Текст заканчивается не на перевод строки
    // Вычисляем требуемый размер буфера
    int bsize = (numln + 2) * (width + 2) + 1;

    // Рождаем выходной буфер
    CREATE(retbuf, char, bsize);

    // Заполняем пробелами (для удобства)
    memset(retbuf, ' ', bsize);
    // Верхняя рамка
    retbuf[0] = topl;
    int ptr = 1;

    memset(&retbuf[ptr], top, width - 2);
    ptr += width - 2;
    retbuf[ptr] = topr;
    ptr++;
    retbuf[ptr] = '\r';
    ptr++;
    retbuf[ptr] = '\n';
    ptr++;

    // Начинаем разбирать отформатированный текст
    int read_ptr = 0;
    int len;

    while (1) {
        if (ret[read_ptr] == 0)
            break;
        // Левая рамка
        retbuf[ptr] = left;
        ptr += 2;
        len = 2;
        // Выковыриваем строку
        while (ret[read_ptr] != '\n' && ret[read_ptr] != 0) {
            if (ret[read_ptr] == '\r') {        // Игнорируем
                read_ptr++;
                continue;
            }
            // Копируем
            retbuf[ptr] = ret[read_ptr];
            ptr++;
            len++;
            read_ptr++;
        }
        // Если не конец текста, то смещаем ползунок
        if (ret[read_ptr] != 0)
            read_ptr++;
        // Эти символы нахуй
        while (ret[read_ptr] == '\r')
            read_ptr++;
        // Строка скопирована
        // Закрываем рамку справа
        ptr += width - len - 1;
        retbuf[ptr] = right;
        ptr++;
        retbuf[ptr] = '\r';
        ptr++;
        retbuf[ptr] = '\n';
        ptr++;
    }
    // Закончили с текстом
    // Закрываем рамку снизу
    retbuf[ptr] = bottoml;
    ptr++;
    memset(&retbuf[ptr], bottom, width - 2);
    ptr += width - 2;
    retbuf[ptr] = bottomr;
    ptr++;
    retbuf[ptr] = '\r';
    ptr++;
    retbuf[ptr] = '\n';
    ptr++;
    retbuf[ptr] = 0;
    //printf("bsize = %d, ptr = %d\n", bsize, ptr+1);
    return retbuf;
}

unsigned char rc_buf[] = {
    0, 0, 0, 107, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    132, 101, 102, 124, 105, 106, 122, 104, 123, 110, 111, 112, 113, 114, 115, 116,
    117, 133, 118, 119, 120, 121, 108, 103, 130, 129, 109, 126, 131, 127, 125, 128,
    32, 1, 2, 24, 5, 6, 22, 4, 23, 10, 11, 12, 13, 14, 15, 16,
    17, 33, 18, 19, 20, 21, 8, 3, 30, 29, 9, 26, 31, 27, 25, 28
};

int cmpchars(char s1, char s2)
{
    if (s1 >= 0 && s2 >= 0)
        return s1 - s2;
    if (s1 >= 0)
        return -1;
    if (s2 >= 0)
        return 1;
    // Оба символа < 0 (русские буквы)
    unsigned char rs1, rs2;

    rs1 = (unsigned char) s1;
    rs2 = (unsigned char) s2;
    if (rs1 < 0xA0 && rs2 < 0xA0)
        // Ёбаная графика
        return rs1 - rs2;
    if (rs1 < 0xA0)
        return -1;
    if (rs2 < 0xA0)
        return 1;
    rs1 = rc_buf[rs1 - 0xA0];
    rs2 = rc_buf[rs2 - 0xA0];
    return rs1 - rs2;
}

int ruscmp(char *str1, char *str2)
{
    int i = 0;

    while (1) {
        int ret = cmpchars(str1[i], str2[i]);

        if (ret != 0)
            return ret;
        if (str1[i] == 0)
            return 0;
        i++;
    }
}

/* Сортировка */
void QuickSort(struct obj_data **A, int L, int R, int type);

struct obj_data *SortObjects(struct obj_data *obj, int type)
{
    if (obj == NULL)
        // Список пуст
        return NULL;
    struct obj_data *list = obj;
    int num = 0;

    while (list != NULL) {
        num++;
        list = list->next_content;
    }
    if (num == 1)
        // Всего один элемент
        return obj;
    struct obj_data **numobj = (struct obj_data **) malloc(sizeof(struct obj_data *) * num);
    int i;

    for (i = 0; i < num; i++) {
        numobj[i] = obj;
        obj = obj->next_content;
    }
    // Сделали нумерованный массив
    QuickSort(numobj, 0, num - 1, type);
    for (i = 0; i < num - 1; i++)
        numobj[i]->next_content = numobj[i + 1];
    numobj[num - 1]->next_content = NULL;
    obj = numobj[0];
    free(numobj);
    return obj;
}

void QuickSort(struct obj_data **A, int L, int R, int type)
{
    // Алгоритм быстрой сортировки Хоара
    int i = L;
    int j = R;
    char *x_name = A[(L + R) / 2]->name;
    int x_number = A[(L + R) / 2]->item_number;

    do {
        if (type == SORT_NAME) {
            while (ruscmp(A[i]->name, x_name) < 0)
                i++;
            while (ruscmp(x_name, A[j]->name) < 0)
                j--;
        }
        if (type == SORT_NUMBER) {
            while (A[i]->item_number < x_number)
                i++;
            while (x_number < A[j]->item_number)
                j--;
        }
        if (i <= j) {
            struct obj_data *tmp = A[i];

            A[i] = A[j];
            A[j] = tmp;
            i++;
            j--;
        }
    } while (i < j);
    if (L < j)
        QuickSort(A, L, j, type);
    if (i < R)
        QuickSort(A, i, R, type);
}


void stop_guarding(struct char_data *ch)
{

    if (GUARDING(ch)) {
        act("Вы прекратили охранять $N3.", FALSE, ch, 0, GUARDING(ch), TO_CHAR);
        act("$n прекратил$g охранять Вас.", FALSE, ch, 0, GUARDING(ch), TO_VICT);
        GUARDING(ch) = 0;
    }
}

/* Проверяет существование локации в памяти */
/* -1 если не найдено */
int check_memory(struct char_data *ch, int vnum_room)
{
    int i;
    int found = -1;

    for (i = 0; i <= GET_MAXMEM(ch); i++)
        if (GET_MEMORY(ch, i) && GET_MEMORY(ch, i) == vnum_room)
            found = i;

    return (found);
}

/* Добавляет локацию в память */
bool add_memory(struct char_data * ch, int vnum_room, long time, const char *desc)
{
    if (GET_LASTMEM(ch) > GET_MAXMEM(ch)) {
        log("Возврат %d vs %d", GET_LASTMEM(ch), GET_MAXMEM(ch));
        return (FALSE);
    }
    GET_MEMORY(ch, GET_LASTMEM(ch)) = vnum_room;
    GET_TIMEMEM(ch, GET_LASTMEM(ch)) = time;
    if (!desc || !*desc)
        GET_DESCMEM(ch, GET_LASTMEM(ch)) = 0;
    else
        GET_DESCMEM(ch, GET_LASTMEM(ch)) = str_dup(desc);

    GET_LASTMEM(ch)++;
    log("%s lastmem %d", GET_NAME(ch), GET_LASTMEM(ch));
    return (TRUE);
}

/* Удаляет локацию из памяти */
bool del_memory(struct char_data * ch, int pos)
{
    int i;

    GET_MEMORY(ch, pos) = 0;
    if (GET_DESCMEM(ch, pos)) {
        free(GET_DESCMEM(ch, pos));
        GET_DESCMEM(ch, pos) = 0;
    }

    if (!GET_LASTMEM(ch))
        return (FALSE);

    for (i = 0; i <= GET_MAXMEM(ch); i++)
        if (GET_MEMORY(ch, i) == 0) {
            GET_MEMORY(ch, i) = GET_MEMORY(ch, i + 1);
            GET_MEMORY(ch, i + 1) = 0;

            if (GET_DESCMEM(ch, i + 1)) {
                GET_DESCMEM(ch, i) = GET_DESCMEM(ch, i + 1);
                GET_DESCMEM(ch, i + 1) = 0;
            }
        }

    GET_LASTMEM(ch)--;

    return (TRUE);
}


/* Создает скелета из трупа */
#define MOB_SKELET = 117;
int create_skelet(struct obj_data *corpse)
{
    struct char_data *mob = NULL, *tcrp = NULL, *victim = NULL;
    int in_room = IN_ROOM(corpse);
    bool fnd = FALSE;

    if (in_room == NOWHERE)
        return (FALSE);

    if (!(IS_CORPSE(corpse) && GET_OBJ_VAL(corpse, 3) == 4))
        return (FALSE);

    if (!(mob = read_mobile(117, VIRTUAL)))
        return (FALSE);

    if (corpse->killer) {
        victim = get_char_by_id(corpse->killer);
        if (victim) {
            inc_pkill_group(mob, victim, 1, 0);
            //SET_BIT(NPC_FLAGS(mob,NPC_TRACK),NPC_TRACK);
            //SET_BIT(NPC_FLAGS(mob,NPC_DIRTY),NPC_DIRTY);
            //указать что нежить
            SET_BIT(MOB_FLAGS(mob, MOB_MEMORY), MOB_MEMORY);
            fnd = TRUE;
        }
    }
    if (!fnd)
        SET_BIT(MOB_FLAGS(mob, MOB_XENO), MOB_XENO);

    /* параметры монстра который был прообразом скелета */
    if (GET_OBJ_VAL(corpse, 2) && real_mobile(GET_OBJ_VAL(corpse, 2)) >= 0) {
        tcrp = (mob_proto + real_mobile(GET_OBJ_VAL(corpse, 2)));
        GET_LEVEL(mob) = MAX(1, GET_LEVEL(tcrp) / 2);
        mob->npc()->specials.damnodice = tcrp->npc()->specials.damnodice;
        mob->npc()->specials.damsizedice = tcrp->npc()->specials.damsizedice;
        mob->npc()->specials.damage = tcrp->npc()->specials.damage;
        mob->npc()->specials.damnodice2 = tcrp->npc()->specials.damnodice2;
        mob->npc()->specials.damsizedice2 = tcrp->npc()->specials.damsizedice2;
        mob->npc()->specials.damage2 = tcrp->npc()->specials.damage2;
    }


    /* Труп в скелета */
    GET_EXP(mob) = 1;
    EXTRACT_TIMER(mob) = 30;

    char_to_room(mob, in_room);
    obj_from_room(corpse);
    obj_to_char(corpse, mob);
    extract_obj(corpse);

    return (TRUE);
}

void mob_alarm(struct char_data *victim, struct char_data *ch)
{
    struct char_data *tch;
    room_rnum room = IN_ROOM(victim);
    int zone = world[room].zone;

    for (tch = character_list; tch; tch = tch->next) {
        if (world[IN_ROOM(tch)].zone != zone)
            continue;

        if (!IS_NPC(tch) ||
            AFF_FLAGGED(tch, AFF_CHARM) ||
            AFF_FLAGGED(tch, AFF_HOLD) ||
            victim->npc()->specials.alr_helper.count(GET_MOB_VNUM(tch)) == 0 ||
            GET_WAIT(tch) > 0 || FIGHTING(tch))
            continue;

        inc_pkill_group(tch, ch, 1, 0);
        tch->npc()->specials.move_to = room;
    }
}


struct char_data *get_char_by_id(long id)
{
    struct char_data *tch;

    for (tch = character_list; tch; tch = tch->next)
        if (GET_ID(tch) == id)
            return (tch);

    return (NULL);
}


void preplague_to_char(struct char_data *ch, struct char_data *victim, int modifier)
{
    struct affected_type af[1];
    int spellnum = find_spell_num(SPELL_PREPLAGUE);

    if (affected_by_spell(victim, SPELL_IMM_PLAGUE))
        return;

    af[0].type = -1;
    af[0].bitvector = 0;
    af[0].modifier = 0;
    af[0].battleflag = 0;
    af[0].main = 0;
    af[0].location = APPLY_NONE;

    af[0].type = spellnum;
    af[0].duration = 24 * SECS_PER_MUD_TICK;
    af[0].bitvector = AFF_NOTHING;
    af[0].modifier = modifier;
    af[0].main = TRUE;
    af[0].owner = GET_ID(ch);

//накладываем эффекты
    affect_join_char(victim, af);

}

void plague_to_char(struct char_data *ch, struct char_data *victim, int modifier)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i, spellnum = find_spell_num(SPELL_PLAGUE);

    if (affected_by_spell(victim, SPELL_IMM_PLAGUE))
        return;

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = -1;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = MAX(1, GET_LEVEL(victim) * 2) * SECS_PER_MUD_TICK;
    af[0].bitvector = AFF_NOTHING;
    af[0].modifier = modifier;
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(victim, af + i);
    }

}


void immplague_to_char(struct char_data *ch)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int i, spellnum = find_spell_num(SPELL_IMM_PLAGUE);

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = -1;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    af[0].type = spellnum;
    af[0].duration = 12 * SECS_PER_MUD_HOUR;
    af[0].battleflag = AF_DEADKEEP;
    af[0].modifier = 1;
    af[0].owner = GET_ID(ch);
    af[0].main = TRUE;

    //накладываем эффекты
    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        if (af[i].bitvector || af[i].location != APPLY_NONE)
            affect_join_char(ch, af + i);
    }

}



bool distract_psphere(struct room_data *room, long bitvector, int damage)
{
    struct room_affect_data *af, *next;
    bool result = TRUE;

    for (af = room->affects; af; af = next) {
        next = af->next;
        if (af->bitvector == bitvector) {
            af->modifier -= damage;
            if (af->modifier <= 0) {
                affect_room_removed(room, af);
                result = FALSE;
            }
        }
    }

    return (result);
}


bool distract_magic_parry(struct char_data * ch, int damage)
{
    int spellnum = find_spell_num(SPELL_MAGIC_PARRY);
    struct affected_type *hjp, *next;


    for (hjp = ch->affected; hjp; hjp = next) {
        next = hjp->next;
        if (hjp->type == spellnum) {
            hjp->modifier -= damage;
            if (hjp->modifier <= 0) {
                if (hjp->main)
                    show_spell_off(hjp->type, ch, NULL);
                affect_remove(ch, hjp);
            }
        }
    }

    return (TRUE);
}

bool distract_bones_wall(struct char_data * ch, int damage)
{
    int spellnum = find_spell_num(SPELL_BONES_WALL);
    struct affected_type *hjp, *next;

    for (hjp = ch->affected; hjp; hjp = next) {
        next = hjp->next;
        if (hjp->type == spellnum) {
            hjp->modifier -= damage;
            if (hjp->modifier <= 0) {
                //  if (hjp->main)
                //   show_spell_off(hjp->type, ch, NULL);
                affect_remove(ch, hjp);
            }
        }
    }

    return (TRUE);
}


//Знаком ли ch с victim
bool check_know(struct char_data * ch, const struct char_data * victim)
{

    if (IS_NPC(victim))
        return (TRUE);
    else
        return (FALSE);
}


char *meter_bar(struct char_data *ch, int current, int max, ubyte pos)
{
    int i, i_percent = 0, chars = 0, rows = 10;
    float percent = 0;
    static char buf2[MAX_STRING_LENGTH];


    percent = ((float) current / (float) max);

    i_percent = (int) ((float) percent * 100.0);

    chars = (int) ((float) rows * (float) percent);

    if (chars > rows)
        chars = rows;

    for (i = 0; i < chars; ++i)
        *(buf2 + i) = *ch->divr;
    *(buf2 + i) = '\0';

    if (pos > 2) {
        sprintf(buf2 + i, "&n");
        i = i + 2;
    } else if (pos == 2) {
        sprintf(buf2 + i, "&r");
        i = i + 2;
    } else {
        sprintf(buf2 + i, "&K");
        i = i + 2;
    }



    for (; i < rows + 2; ++i)
        *(buf2 + i) = '.';
    *(buf2 + i) = '\0';

    return (buf2);

}

char *meter_barf(struct char_data *ch, int current, int max)
{
    int i, chars = 0, rows = 10;
    float percent = 0;
    static char buf2[MAX_STRING_LENGTH];



    percent = ((float) current / (float) max);  //90/100 = 0.9

    chars = (int) ((float) rows * (float) percent);     //9
    chars = rows - chars;       //10-9=1

    if (chars > rows)
        chars = rows;

    for (i = 0; i < chars; ++i)
        *(buf2 + i) = *ch->divr;
    *(buf2 + i) = '\0';

    for (; i < rows + 1; ++i)
        *(buf2 + i) = '.';
    *(buf2 + i) = '\0';

    return (buf2);

}

bool check_spells_attacked(struct char_data * ch, struct char_data * victim, int stopflag, int afar)
{
    int level = 0;
    int spell_sanc = find_spell_num(SPELL_SANCTUARY);
    int spell_repl = find_spell_num(SPELL_REPULSION);

    /* Проверка на отторжение */
    if ((level = affected_by_spell_real(victim, spell_repl))) {
        //Проверяем если ли в списке "атака" или стреляем из лука-магии
        if (check_victim_may_moved(victim, ch) || afar)
            return (FALSE);

        //Проверяем если нет в списке "приближение"
        if (!check_victim_not_moved(victim, ch)) {
            if (!general_savingthrow_3(ch, SAV_WILL, level / 12)) {     //Если провалили сейв
                act("2и не смог2(,ла,о,ли) приблизится к Вам на расстояние атаки.", "Мм", victim,
                    ch);
                act("Вы не смогли приблизиться к 2д на расстояние атаки.", "мМ", victim, ch);
                act("2+и не смог2(,ла,о,ли) приблизится к 2+д на расстояние атаки.", "Кмм", victim,
                    ch);
                add_victim_not_moved(victim, ch);
                if (stopflag) {
                    stop_fighting(ch, FALSE);
                    if (FIGHTING(ch) == victim)
                        stop_fighting(victim, FALSE);
                }
                return (TRUE);
            } else {
                act("2и проигнорировал2(,а,о,и) Ваш защитный круг отторжения.", "Мм", victim, ch);
                act("Вы проигнорировали защитый круг отторжения вокруг 1р.", "мМ", victim, ch);
                act("2и проигнорировал2(,а,о,и) защитый круг отторжения вокруг 1р.", "Кмм", victim,
                    ch);
                add_victim_may_moved(victim, ch);
                return (FALSE);
            }
        } else
            return (TRUE);
    }

    /* Проверка на святилище */
    if ((level = affected_by_spell_real(victim, spell_sanc))) {
        //Проверяем если ли в списке "атака"
        if (check_victim_may_attack(victim, ch))
            return (FALSE);

        //Проверяем есть ли в списке  "не атака"
        if (!check_victim_not_attack(victim, ch)) {
            if (!general_savingthrow_3(ch, SAV_WILL, level / 10)) {     //Если провалили сейв
                act("Яркая аура вокруг Вас мешает атаковать 2д.", "Мм", victim, ch);
                act("Яркая аура вокруг 1р мешает Вам атаковать 1ер.", "мМ", victim, ch);
                act("Яркая аура вокруг 1+р мешает 2+д атаковать 1ер.", "Кмм", victim, ch);
                add_victim_not_attack(victim, ch);
                return (TRUE);
            } else {            //Иначе добавляем в список кто может атаковать
                act("Яркая аура вокруг Вас не подействовала на 2в.", "Мм", victim, ch);
                act("Яркая аура вокруг 1р не подействовала на Вас.", "мМ", victim, ch);
                act("Яркая аура вокруг 1+р не подействовала на 2+в.", "Кмм", victim, ch);
                add_victim_may_attack(victim, ch);
                return (FALSE);
            }
        } else {                //Те кто есть уже в списке "не атака", сразу нах... посылаем
            if (!FIGHTING(victim)) {
                act("Яркая аура вокруг Вас мешает атаковать 2д.", "Мм", victim, ch);
                act("Яркая аура вокруг 1р мешает Вам атаковать 1ер.", "мМ", victim, ch);
                act("Яркая аура вокруг 1+р мешает 2+д атаковать 1ер.", "Кмм", victim, ch);
            }
            return (TRUE);
        }
    }

    return (FALSE);
}


void ShowMessage(struct char_data *ch,
                 struct char_data *victim,
                 struct obj_data *weapObj,
                 struct P_message &message, int dam, int hit, int type, int location)
{
    struct P_message emptyAddMessage;

    ShowMessage(ch, victim, weapObj, message, dam, hit, type, location, emptyAddMessage, 0);
}

void ShowMessage(struct char_data *ch,
                 struct char_data *victim,
                 struct obj_data *weapObj,
                 struct P_message &message,
                 int dam, int hit, int type, int location, struct P_message &addMessage, int adam)
{
    char *rstring(int dam, const char *str, int hTyp, const char *wear_mess, const char *wear_mess2,
                  const char *wname);
    char *to_char = 0, *to_vict = 0, *to_room = 0, *buf = 0, tmp[256], tmp2[256]; // prool
    char *m_to_char = 0, *m_to_vict = 0, *m_to_room = 0, wname[256]; // prool
    int mdam, atype;
    struct char_data *tch;


    if (!message.valid)
        return;

    *tmp = '\0';
    *tmp2 = '\0';
    *wname = '\0';

    if (weapObj)
        sprintf(wname, " %s", GET_OBJ_PNAME(weapObj, 4));


    /*log("--00--");
       log("char-name: %s",GET_NAME(ch));
       log("victim-name: %s",GET_NAME(victim));
       log("type: %d, location: %d",type,location);
       log("message-hChar: %s",message.hChar); */

//провекра на оружие по умолчанию

    if (dam == 0)
        mdam = 0;
    else if (dam <= 4)
        mdam = 1;
    else if (dam <= 8)
        mdam = 2;
    else if (dam <= 14)
        mdam = 3;
    else if (dam <= 20)
        mdam = 4;
    else if (dam <= 26)
        mdam = 5;
    else if (dam <= 32)
        mdam = 6;
    else if (dam <= 46)
        mdam = 7;
    else if (dam <= 52)
        mdam = 8;
    else if (dam <= 62)
        mdam = 9;
    else
        mdam = 10;

    if (adam == RD_ARMOR || adam == RD_NONE)
        atype = M_NONE;
    else
        atype = M_HIT;



    if (addMessage.valid) {
// to_char = message.tChar;
// to_vict = message.tVict;
// to_room = message.tRoom;
        if (adam > 0) {
            m_to_char = addMessage.hChar;
            m_to_vict = addMessage.hVict;
            m_to_room = addMessage.hRoom;
        } else
            switch (atype) {
                case M_KILL:
                    m_to_char = addMessage.kChar;
                    m_to_vict = addMessage.kVict;
                    m_to_room = addMessage.kRoom;
                    break;
                case M_HIT:
                case M_ARM:
                    m_to_char = addMessage.hChar;
                    m_to_vict = addMessage.hVict;
                    m_to_room = addMessage.hRoom;
                    break;
                case M_NONE:
                    m_to_char = addMessage.mChar;
                    m_to_vict = addMessage.mVict;
                    m_to_room = addMessage.mRoom;
                    break;
            }
    }
// else
    if (mdam == 0 && type == M_HIT) {
        to_char = message.mChar;
        to_vict = message.mVict;
        to_room = message.mRoom;
    } else
        switch (type) {
            case M_HIT:
                to_char = message.hChar;
                to_vict = message.hVict;
                to_room = message.hRoom;
                break;
            case M_NONE:
                to_char = message.mChar;
                to_vict = message.mVict;
                to_room = message.mRoom;
                break;
            case M_KILL:
                to_char = message.kChar;
                to_vict = message.kVict;
                to_room = message.kRoom;
                break;
            case M_ARM:
                int hLoc = 0;

                if (victim->body)
                    hLoc = wpos_to_wear[victim->body[location].wear_position];
                if (hLoc && GET_EQ(victim, hLoc)) {
                    weapObj = GET_EQ(victim, hLoc);
                    to_char = message.aChar;
                    to_vict = message.aVict;
                    to_room = message.aRoom;
                } else {
                    to_char = message.bChar;
                    to_vict = message.bVict;
                    to_room = message.bRoom;
                }
                /* switch (GET_RACE(victim))
                   {
                   case RACE_ANIMAL: case RACE_DRAGON: case RACE_BIRD:
                   case RACE_WORM: case RACE_REPTILE: case RACE_SNAKE:
                   case RACE_INSECT: case RACE_CONSTRUCTION: case RACE_SLIME:
                   to_char = message.bChar;
                   to_vict = message.bVict;
                   to_room = message.bRoom;
                   break;
                   default:
                   to_char = message.aChar;
                   to_vict = message.aVict;
                   to_room = message.aRoom;
                   break;
                   } */
                break;
        }

//send_to_charf(ch,"ch Показываю type %d\r\n",type);
//send_to_charf(victim,"vict Показываю type %d\r\n",type);

    if (victim->body && victim->body[location].name.size() != 0) {
        switch (hit) {
            case 0:            //ударив по руке
                // sprintf(tmp," по %s",get_name_pad(victim->body[location].name.c_str(),PAD_DAT,PAD_OBJECT));
                //break;
            case 2:            //хлестнув по руке
            case 3:            //рубанув по руке
            case 5:            //огрев по руке
            case 6:            //сокрушив по руке
            case 7:            //резанув по руке
            case 17:           //стукнув по руке
                sprintf(tmp, " по %s",
                        get_name_pad(victim->body[location].name.c_str(), PAD_DAT, PAD_OBJECT));
                break;
            case 1:            //ободрав руку
            case 4:            //укусив руку
            case 8:            //оцарапав руку
                sprintf(tmp, " %s",
                        get_name_pad(victim->body[location].name.c_str(), PAD_VIN, PAD_OBJECT));
                break;
            case 9:            //подстрелив в руку
            case 10:           //пырнув в руку
            case 11:           //уколов в руку
            case 12:           //ткнув в руку
            case 13:           //лягнув в руку
            case 14:           //боднув в руку
            case 15:           //клюнув в руку
            case 16:
                sprintf(tmp, " в %s",
                        get_name_pad(victim->body[location].name.c_str(), PAD_VIN, PAD_OBJECT));
                break;
        }
        sprintf(tmp2, " %s",
                get_name_pad(victim->body[location].name.c_str(), PAD_VIN, PAD_OBJECT));
    }

    char toc[8192];
    char tov[8192];
    char tor[8192];

    *toc = '\0';
    *tov = '\0';
    *tor = '\0';


    if (m_to_char) {
        char dd[3];

        sprintf(dd, " [%d хп]", adam);
        sprintf(toc, "%s%s, %s", m_to_char, PRF_FLAGGED(ch, PRF_CODERINFO) ? dd : "", to_char);
    } else if (to_char)
        sprintf(toc, "%s", to_char);

    if (*toc) {
        buf = rstring(mdam, toc, hit, tmp, tmp2, wname);
        if (ch != victim) {
            if (type != M_ARM)
            {
                if (dam < 1)
                    send_to_char(CCYEL(ch, C_CMP), ch);
                else
                    send_to_char(CCIYEL(ch, C_CMP), ch);
            }
            if (PRF_FLAGGED(ch, PRF_CODERINFO))
                sprintf(buf + strlen(buf), " [#1 хп]");
            del_spaces(buf);
            act(buf, "Ммпч", ch, victim, weapObj, dam);
            send_to_char(CCNRM(ch, C_CMP), ch);
        }
    }

    if (m_to_vict) {
        char dd[3];

        sprintf(dd, " [%d хп]", adam);
        sprintf(tov, "%s%s, %s", m_to_vict, PRF_FLAGGED(victim, PRF_CODERINFO) ? dd : "", to_vict);
    } else if (to_vict)
        sprintf(tov, "%s", to_vict);

    if (*tov) {
        buf = rstring(mdam, tov, hit, tmp, tmp2, wname);
        if (type != M_ARM)
        {
            if (dam < 1)
                send_to_char(CCRED(victim, C_CMP), victim);
            else
                send_to_char(CCIRED(victim, C_CMP), victim);
        }            
        if (PRF_FLAGGED(victim, PRF_CODERINFO))
            sprintf(buf + strlen(buf), " [#1 хп]");
        del_spaces(buf);
        act(buf, "м!Мпч", ch, victim, weapObj, dam);
        send_to_char(CCNRM(victim, C_CMP), victim);
    }
//if (m_to_room)
// sprintf(to_room,"%s %s",m_to_room,to_room);
    if (m_to_room)
        sprintf(tor, "%s %s", m_to_room, to_room);
    else if (to_room)
        sprintf(tor, "%s", to_room);

    if (*tor && IN_ROOM(ch) != NOWHERE) {
        buf = rstring(mdam, tor, hit, tmp, tmp2, wname);
        del_spaces(buf);
        //act(buf,"Кммп",ch,victim,weapObj);

        for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room)
            if (!PRF_FLAGGED(tch, PRF_SELFMESS) && !IS_NPC(tch) && tch != ch && tch != victim) {
                act(buf, "ммМп", ch, victim, tch, weapObj);
            }
    }
}


void GetExitTrapMessage(struct room_direction_data *exit, struct P_message &pMess)
{
    pMess.valid = false;

    if (!exit)
        return;

    pMess.valid = true;

    if (exit->trap_kill_char) {
        pMess.kChar = exit->trap_kill_char;
        pMess.kVict = exit->trap_kill_char;
    } else {
        pMess.kChar = NULL;
        pMess.kVict = NULL;
    }
    if (exit->trap_kill_room)
        pMess.kRoom = exit->trap_kill_room;
    else
        pMess.kRoom = NULL;

    if (exit->trap_damage_char) {
        pMess.hChar = exit->trap_damage_char;
        pMess.hVict = exit->trap_damage_char;
    } else {
        pMess.hChar = NULL;
        pMess.hVict = NULL;
    }
    if (exit->trap_damage_room)
        pMess.hRoom = exit->trap_damage_room;
    else
        pMess.hRoom = NULL;

    if (exit->trap_nodamage_char) {
        pMess.mChar = exit->trap_nodamage_char;
        pMess.mVict = exit->trap_nodamage_char;
    } else {
        pMess.mChar = NULL;
        pMess.mVict = NULL;
    }
    if (exit->trap_nodamage_room)
        pMess.mRoom = exit->trap_nodamage_room;
    else
        pMess.mRoom = NULL;

    if (exit->trap_nodamage_char) {
        pMess.pChar = exit->trap_nodamage_char;
        pMess.pVict = exit->trap_nodamage_char;
    } else {
        pMess.pChar = NULL;
        pMess.pVict = NULL;
    }
    if (exit->trap_nodamage_room)
        pMess.pRoom = exit->trap_nodamage_room;
    else
        pMess.pRoom = NULL;

    if (exit->trap_nodamage_char) {
        pMess.aChar = exit->trap_nodamage_char;
        pMess.aVict = exit->trap_nodamage_char;
    } else {
        pMess.aChar = NULL;
        pMess.aVict = NULL;
    }
    if (exit->trap_nodamage_room)
        pMess.aRoom = exit->trap_nodamage_room;
    else
        pMess.aRoom = NULL;

    if (exit->trap_nodamage_char) {
        pMess.bChar = exit->trap_nodamage_char;
        pMess.bVict = exit->trap_nodamage_char;
    } else {
        pMess.bChar = NULL;
        pMess.bVict = NULL;
    }
    if (exit->trap_nodamage_room)
        pMess.bRoom = exit->trap_nodamage_room;
    else
        pMess.bRoom = NULL;
}

void GetObjTrapMessage(struct obj_trap_data *trap, struct P_message &pMess)
{
    pMess.valid = false;

    if (!trap)
        return;

    pMess.valid = true;

    if (trap->trap_kill_char) {
        pMess.kChar = trap->trap_kill_char;
        pMess.kVict = trap->trap_kill_char;
    } else {
        pMess.kChar = NULL;
        pMess.kVict = NULL;
    }
    if (trap->trap_kill_room)
        pMess.kRoom = trap->trap_kill_room;
    else
        pMess.kRoom = NULL;

    if (trap->trap_damage_char) {
        pMess.hChar = trap->trap_damage_char;
        pMess.hVict = trap->trap_damage_char;
    } else {
        pMess.hChar = NULL;
        pMess.hVict = NULL;
    }
    if (trap->trap_damage_room)
        pMess.hRoom = trap->trap_damage_room;
    else
        pMess.hRoom = NULL;

    if (trap->trap_nodamage_char) {
        pMess.mChar = trap->trap_nodamage_char;
        pMess.mVict = trap->trap_nodamage_char;
    } else {
        pMess.mChar = NULL;
        pMess.mVict = NULL;
    }
    if (trap->trap_nodamage_room)
        pMess.mRoom = trap->trap_nodamage_room;
    else
        pMess.mRoom = NULL;

    if (trap->trap_nodamage_char) {
        pMess.pChar = trap->trap_nodamage_char;
        pMess.pVict = trap->trap_nodamage_char;
    } else {
        pMess.pChar = NULL;
        pMess.pVict = NULL;
    }
    if (trap->trap_nodamage_room)
        pMess.pRoom = trap->trap_nodamage_room;
    else
        pMess.pRoom = NULL;

    if (trap->trap_nodamage_char) {
        pMess.aChar = trap->trap_nodamage_char;
        pMess.aVict = trap->trap_nodamage_char;
    } else {
        pMess.aChar = NULL;
        pMess.aVict = NULL;
    }
    if (trap->trap_nodamage_room)
        pMess.aRoom = trap->trap_nodamage_room;
    else
        pMess.aRoom = NULL;

    if (trap->trap_nodamage_char) {
        pMess.bChar = trap->trap_nodamage_char;
        pMess.bVict = trap->trap_nodamage_char;
    } else {
        pMess.bChar = NULL;
        pMess.bVict = NULL;
    }
    if (trap->trap_nodamage_room)
        pMess.bRoom = trap->trap_nodamage_room;
    else
        pMess.bRoom = NULL;
}

void GetEnterTrapMessage(struct room_trap_data *trap, struct P_message &pMess)
{
    pMess.valid = false;

    if (!trap)
        return;

    pMess.valid = true;

    if (trap->kill_mess_char) {
        pMess.kChar = trap->kill_mess_char;
        pMess.kVict = trap->damage_mess_char;
    } else if (trap->damage_mess_char) {
        pMess.kChar = trap->damage_mess_char;
        pMess.kVict = trap->damage_mess_char;
    } else {
        pMess.kChar = NULL;
        pMess.kVict = NULL;
    }

    if (trap->kill_mess_room) {
        pMess.kRoom = trap->kill_mess_room;
    } else if (trap->damage_mess_char) {
        pMess.kRoom = trap->damage_mess_room;
    } else {
        pMess.kRoom = NULL;
    }



    if (trap->damage_mess_char) {
        pMess.kChar = trap->damage_mess_char;
        pMess.kVict = trap->damage_mess_char;
        pMess.hChar = trap->damage_mess_char;
        pMess.hVict = trap->damage_mess_char;
    } else {
        pMess.kChar = NULL;
        pMess.kVict = NULL;
        pMess.hChar = NULL;
        pMess.hVict = NULL;
    }

    if (trap->damage_mess_room) {
        pMess.kRoom = trap->damage_mess_room;
        pMess.hRoom = trap->damage_mess_room;
    } else {
        pMess.kRoom = NULL;
        pMess.hRoom = NULL;
    }

    if (trap->nodamage_mess_char) {
        pMess.mChar = trap->nodamage_mess_char;
        pMess.mVict = trap->nodamage_mess_char;
    } else {
        pMess.mChar = NULL;
        pMess.mVict = NULL;
    }
    if (trap->nodamage_mess_room)
        pMess.mRoom = trap->nodamage_mess_room;
    else
        pMess.mRoom = NULL;

    if (trap->nodamage_mess_char) {
        pMess.pChar = trap->nodamage_mess_char;
        pMess.pVict = trap->nodamage_mess_char;
    } else {
        pMess.pChar = NULL;
        pMess.pVict = NULL;
    }
    if (trap->nodamage_mess_room)
        pMess.pRoom = trap->nodamage_mess_room;
    else
        pMess.pRoom = NULL;

    if (trap->nodamage_mess_char) {
        pMess.aChar = trap->nodamage_mess_char;
        pMess.aVict = trap->nodamage_mess_char;
    } else {
        pMess.aChar = NULL;
        pMess.aVict = NULL;
    }
    if (trap->nodamage_mess_room)
        pMess.aRoom = trap->nodamage_mess_room;
    else
        pMess.aRoom = NULL;

    if (trap->nodamage_mess_char) {
        pMess.bChar = trap->nodamage_mess_char;
        pMess.bVict = trap->nodamage_mess_char;
    } else {
        pMess.bChar = NULL;
        pMess.bVict = NULL;
    }
    if (trap->nodamage_mess_room)
        pMess.bRoom = trap->nodamage_mess_room;
    else
        pMess.bRoom = NULL;
}

void del_spaces(char *str)
{
    int write_ptr = -1;
    char prv_sym = ' ';

    for (int read_ptr = 0; str[read_ptr] != 0; read_ptr++) {
        if (str[read_ptr] == ' ' && prv_sym == ' ')
            continue;
        write_ptr++;
        prv_sym = str[read_ptr];
        if (read_ptr == write_ptr)
            continue;
        str[write_ptr] = str[read_ptr];
    }
    str[write_ptr + 1] = 0;
    while (write_ptr >= 0 && str[write_ptr] == ' ') {
        str[write_ptr] = 0;
        write_ptr--;
    }
}

/* Снятие с ch эффекта невидимости и показ сообщения в локацию */

void appear(struct char_data *ch)
{
    struct char_data *victim;
    bool is_hide = FALSE, is_invis = FALSE;
    int in_room = IN_ROOM(ch);

    if (in_room == NOWHERE)
        return;

    if (affected_by_spell(ch, SPELL_HIDE) || IS_AFFECTED(ch, AFF_HIDE))
        is_hide = TRUE;

    if (affected_by_spell(ch, SPELL_INVISIBLE) || IS_AFFECTED(ch, AFF_INVISIBLE))
        is_invis = TRUE;

//Удаление ненужных эффектов
    if (affected_by_spell(ch, SPELL_HIDE))
        affect_from_char(ch, SPELL_HIDE);

    if (affected_by_spell(ch, SPELL_INVISIBLE))
        affect_from_char(ch, SPELL_INVISIBLE);

    if (affected_by_spell(ch, SPELL_CAMOUFLAGE))
        affect_from_char(ch, SPELL_CAMOUFLAGE);

    REMOVE_BIT(AFF_FLAGS(ch, AFF_INVISIBLE), AFF_INVISIBLE);
    REMOVE_BIT(AFF_FLAGS(ch, AFF_HIDE), AFF_HIDE);
    REMOVE_BIT(AFF_FLAGS(ch, AFF_CAMOUFLAGE), AFF_CAMOUFLAGE);

//Проверяем всех в локации
    for (victim = world[in_room].people; victim; victim = victim->next_in_room) {
        if (victim == ch)
            continue;

        if (is_hide && !check_victim_visible(victim, ch))       //проверка на прятки
            act("Из ничем не примечательной тени появил$u $n.", FALSE, ch, 0, victim, TO_VICT);

        if (is_invis && !IS_AFFECTED(victim, AFF_DETECT_INVIS)) //проверка на невидимость
            act("Словно из ниоткуда рядом с Вами появил$u $n.", FALSE, ch, 0, victim, TO_VICT);
    }

}

void restore_soul(struct char_data *soul, struct obj_data *corpse, bool exp)
{
    int dec_exp = 0;

    if (corpse) {
        struct char_data *tch, *next_ch;

        if (IN_ROOM(corpse) == NOWHERE)
            return;
        for (tch = world[IN_ROOM(corpse)].people; tch; tch = next_ch) {
            next_ch = tch->next_in_room;
            if (!IS_SOUL(tch))
                continue;
            if (GET_OBJ_VAL(corpse, 1) == GET_ID(tch)) {
                obj_from_room(corpse);
                obj_to_char(corpse, tch);
                //extract_obj(corpse);
                REMOVE_BIT(PLR_FLAGS(tch, PLR_SOUL), PLR_SOUL);
                GET_POS(tch) = POS_STANDING;
                act("Хрипло вдохнув, $n постепенно приш$y в себя.", FALSE, tch, 0, 0, TO_ROOM);
                send_to_charf(tch,
                              "&WМощная сила повлекла Вас назад, в стремительно восстанавливающееся тело.\r\nОбретя второе дыхание, Вы постепенно пришли в себя.&n\r\n");
                if (exp) {
                    dec_exp = get_levelexp(tch, GET_LEVEL(tch) + 1, 1);
                    dec_exp = MIN(max_exp_loss_pc(tch), dec_exp);
                    dec_exp = MAX(1, (dec_exp * 20) / 100);
                    if (dec_exp)
                        gain_exp(tch, -dec_exp, TRUE);
                } else {
                    GET_HIT(tch) = GET_REAL_MAX_HIT(tch);
                    GET_MANA(tch) = GET_REAL_MAX_MANA(tch);
                    GET_MOVE(tch) = GET_REAL_MAX_MOVE(tch);
                }
                return;
            }
        }
    }
    if (soul) {
        struct obj_data *tobj, *next_obj;

        if (IN_ROOM(soul) == NOWHERE)
            return;
        for (tobj = world[IN_ROOM(soul)].contents; tobj; tobj = next_obj) {
            next_obj = tobj->next_content;
            if (!IS_CORPSE(tobj))
                continue;
            if (GET_OBJ_VAL(tobj, 1) == GET_ID(soul)) { //Нашли труп
                obj_from_room(tobj);
                obj_to_char(tobj, soul);
                extract_obj(tobj);
                REMOVE_BIT(PLR_FLAGS(soul, PLR_SOUL), PLR_SOUL);
                GET_POS(soul) = POS_STANDING;
                act("Хрипло вдохнув, $n постепенно приш$y в себя.", FALSE, soul, 0, 0, TO_ROOM);
                send_to_charf(soul,
                              "&WМощная сила повлекла Вас назад, в стремительно восстанавливающееся тело.\r\nОбретя второе дыхание, Вы постепенно пришли в себя.&n\r\n");
                if (exp) {
                    dec_exp = get_levelexp(soul, GET_LEVEL(soul) + 1, 1);
                    dec_exp = MIN(max_exp_loss_pc(soul), dec_exp);
                    dec_exp = MAX(1, (dec_exp * 20) / 100);
                    if (dec_exp)
                        gain_exp(soul, -dec_exp, TRUE);
                } else {
                    GET_HIT(soul) = GET_REAL_MAX_HIT(soul);
                    GET_MANA(soul) = GET_REAL_MAX_MANA(soul);
                    GET_MOVE(soul) = GET_REAL_MAX_MOVE(soul);
                }
                return;
            }
        }
    }
}


void GetForceDirMessage(struct room_forcedir_data *fd, struct P_message &pMess)
{
    pMess.valid = false;

    if (!fd)
        return;

    pMess.valid = true;

    if (fd->mess_fail_char)
        pMess.mChar = fd->mess_fail_char;
    if (fd->mess_fail_room)
        pMess.mRoom = fd->mess_fail_room;

    if (fd->mess_exit_char)
        pMess.hChar = fd->mess_exit_char;
    if (fd->mess_exit_room)
        pMess.hRoom = fd->mess_exit_room;

    if (fd->mess_kill_char)
        pMess.kChar = fd->mess_kill_char;
    if (fd->mess_kill_room)
        pMess.kRoom = fd->mess_kill_room;

    if (fd->mess_fail_char)
        pMess.aChar = fd->mess_fail_char;
    if (fd->mess_fail_room)
        pMess.aRoom = fd->mess_fail_room;


}

void forcedir(struct char_data *ch, int rnum, struct room_forcedir_data *fd)
{
    int to_room = NOWHERE;
    bool dam = FALSE;

    if (fd->vnum)
        to_room = real_room(fd->vnum);
    else if (world[rnum].dir_option[(int) fd->dir])
        to_room = world[rnum].dir_option[(int) fd->dir]->to_room;

    if (to_room == NOWHERE) {
        send_to_charf(ch,
                      "Течение в этой локации ссылается на несуществующую комнату.\r\nСообщите богам об этой ошибке.\r\n");
        return;
    }

    if (fd->save)
        if (general_savingthrow_3(ch, fd->save_type, fd->save)) {
            if (fd->mess_fail_char)
                send_to_charf(ch, "%s\r\n", fd->mess_fail_char);
            if (fd->mess_fail_room)
                act(fd->mess_fail_room, FALSE, ch, 0, 0, TO_ROOM);
            return;
        }

    if (fd->damage_type) {
        struct P_damage damage;
        struct P_message message;

        damage.valid = true;
        damage.type = fd->damage_type;
        damage.power = 0;
        damage.far_min = TRUE;
        damage.armor = FALSE;
        damage.weapObj = NULL;
        damage.check_ac = A_POWER;
        damage.dam = dice(fd->damnodice, fd->damsizedice) + fd->damage;

        GetForceDirMessage(fd, message);

        char name[MAX_INPUT_LENGTH];

        strcpy(name, GET_NAME(ch));
        if (_damage(ch, ch, 0, 0, A_POWER, FALSE, damage, message) == RD_KILL) {
            char buf[MAX_STRING_LENGTH];

            sprintf(buf, "Персонаж %s погиб от течения в локации [%d]%s.", name, world[rnum].number,
                    world[rnum].name);
            mudlog(buf, BRF, LVL_IMMORT, TRUE);
        }
        if (!ch)
            return;
        dam = TRUE;
    }
// if (!dam)
// {
    if (fd->mess_exit_char)
        send_to_charf(ch, "\r\n%s\r\n\r\n", fd->mess_exit_char);
    if (fd->mess_exit_room)
        act(fd->mess_exit_room, FALSE, ch, 0, 0, TO_ROOM);
// }

    char_from_room(ch);
    char_to_room(ch, to_room);

    look_at_room(ch, TRUE);

    if (fd->mess_enter_char)
        send_to_charf(ch, "\r\n%s\r\n", fd->mess_enter_char);

    if (fd->mess_enter_room)
        act(fd->mess_enter_room, FALSE, ch, 0, 0, TO_ROOM);
}

int get_followers_num(struct char_data *ch, int type)
{
    int result = 0;
    struct follow_type *k, *k_next;

    for (k = ch->followers; k; k = k_next) {
        k_next = k->next;
        if (k->type == type)
            result++;
    }

    return result;
}


const char *get_pers_name(struct char_data *ch, struct char_data *vict, int pad)
{
    /* static char buf[MAX_INPUT_LENGTH];
     *buf='\0';*/

    if (IS_SOUL(vict))
        return name_infra(ch, pad);

    if (!CAN_SEE(vict, ch))     //Не видим вообще
        return GET_PAD_PERS(pad);       //Возвращаем кто-то

//в локациях темно и не можем видеть в темноте
    if (IS_DARK(IN_ROOM(ch)) && IS_DARK(IN_ROOM(vict)) && !CAN_SEE_IN_DARK_ALL(vict)) {
        if (IS_AFFECTED(vict, AFF_INFRAVISION))
            return name_infra(ch, pad);
        else
            return GET_PAD_PERS(pad);
    }
//светло или можем видеть в темноте
    else {
        if (IS_NPC(vict))       //На мобов не действует инкогнито
            return GET_PAD(ch, pad);

        if (check_incognito(ch))
            return hide_race(ch, pad);
        else
            return GET_PAD(ch, pad);
    }

//return (GET_PAD(ch,pad));
}

void recalc_mob(struct char_data *mob)
{
    int icls;

    GET_MAX_HIT(mob) = 0;
    GET_MAX_MANA(mob) = 0;

    GET_DR(mob) = (GET_REAL_STR(mob) - 10) / 2;
    GET_HR(mob) = (int) (((float) GET_FACT_LEVEL(mob) * 0.04) * (float) GET_REAL_DEX(mob));
    GET_AC_WEAR(mob) = GET_AC(mob) =
        (int) ((float) (0.95 + ((float) GET_FACT_LEVEL(mob) * 0.05)) * (float) GET_REAL_DEX(mob));

    mob->npc()->specials.armor[0] =
        (int) ((float) GET_FACT_LEVEL(mob) *
               tBody.FindItem(mob->player.race)->GetItem(TBD_ARM0)->GetFloat());
    mob->npc()->specials.armor[1] =
        (int) ((float) GET_FACT_LEVEL(mob) *
               tBody.FindItem(mob->player.race)->GetItem(TBD_ARM1)->GetFloat());
    mob->npc()->specials.armor[2] =
        (int) ((float) GET_FACT_LEVEL(mob) *
               tBody.FindItem(mob->player.race)->GetItem(TBD_ARM2)->GetFloat());
    for (icls = 0; icls < NUM_CLASSES; icls++)
        if (mob->classes[icls] > 0) {
            //ЖИЗНЬ
            if (IS_AFFECTED(mob, AFF_CHARM)) {
                GET_MAX_HIT(mob) +=
                    (int) ((add_classes[icls][6] +
                            (float) ((GET_REAL_CON(mob) -
                                      10) / 2)) * (float) mob->classes[icls] *
                           (float) (sqrt(mob->real_abils.size) / 8.0));
                GET_MAX_MANA(mob) +=
                    (int) (add_classes[icls][7] * GET_REAL_WIS(mob) * mob->classes[icls]) / 8;
            } else {
                GET_MAX_HIT(mob) +=
                    (int) ((add_classes[icls][6] +
                            (float) ((GET_REAL_CON(mob) -
                                      10) / 2)) * (float) mob->classes[icls] *
                           (float) (sqrt(mob->real_abils.size) / 5.0));
                GET_MAX_MANA(mob) +=
                    (int) (add_classes[icls][7] * GET_REAL_WIS(mob) * mob->classes[icls]) / 5;
            }
            mob->npc()->specials.armor[0] +=
                (int) ((add_classes[icls][10]) * (float) mob->classes[icls]);
            mob->npc()->specials.armor[1] +=
                (int) ((add_classes[icls][11]) * (float) mob->classes[icls]);
            mob->npc()->specials.armor[2] +=
                (int) ((add_classes[icls][12]) * (float) mob->classes[icls]);
        }
//ЖИЗНЬ на коэф расы.
    GET_MAX_HIT(mob) =
        (int) ((float) GET_MAX_HIT(mob) *
               (float) tBody.FindItem(mob->player.race)->GetItem(TBD_HEALTH)->GetFloat());

//ВРЕД1
    if (mob->npc()->specials.ExtraAttack) {
        int dam = (int) ((float) ((float) GET_REAL_STR(mob) *
                                  (float) GET_FACT_LEVEL(mob) *
                                  (float) sqrt((float) GET_REAL_SIZE(mob))
                         ) / 100.0);

        mob->npc()->specials.damnodice = 2;
        mob->npc()->specials.damsizedice = MAX(1, dam / 2);
        mob->npc()->specials.damage = 0;
    }
//ВРЕД2
    if (mob->npc()->specials.ExtraAttack2) {
        int dam = (int) ((float) ((float) GET_REAL_STR(mob) *
                                  (float) GET_FACT_LEVEL(mob) *
                                  (float) sqrt((float) GET_REAL_SIZE(mob))
                         ) / 120.0);

        mob->npc()->specials.damnodice2 = 2;
        mob->npc()->specials.damsizedice2 = MAX(1, dam / 2);
        mob->npc()->specials.damage2 = 0;
    }

}

void recalc_char(struct char_data *ch)
{
    int icls, i;
    struct obj_data *obj;

    GET_MAX_HIT(ch) = GET_INIT_HIT(ch);
    GET_MAX_MANA(ch) = GET_INIT_MANA(ch);

    GET_HR(ch) = (int) (((float) GET_FACT_LEVEL(ch) * 0.04) * (float) GET_REAL_DEX(ch));
    GET_AC(ch) =
        (int) ((float) (0.95 + ((float) GET_FACT_LEVEL(ch) * 0.05)) * (float) GET_REAL_DEX(ch));
    GET_DR(ch) = (GET_REAL_STR(ch) - 10) / 2;

    for (icls = 0; icls < NUM_CLASSES; icls++)
        if (ch->classes[icls]) {
            //ЖИЗНЬ
            GET_MAX_HIT(ch) +=
                (((int) add_classes[icls][6] +
                  (((GET_CON(ch) + GET_CON_ROLL(ch)) - 10) / 2)) * ch->classes[icls] +
                 ((GET_CON_ADD(ch) * (int) add_classes[icls][6] * ch->classes[icls]) / 10));
            GET_MAX_MANA(ch) +=
                (((int) add_classes[icls][7] +
                  (((GET_WIS(ch) + GET_WIS_ROLL(ch)) - 10) / 2)) * ch->classes[icls] +
                 ((GET_WIS_ADD(ch) * (int) add_classes[icls][7] * ch->classes[icls]) / 10));
            //АТАКА
            //GET_HR(ch) += (int)(add_classes[icls][8]*(float)ch->classes[icls]);
            //ЗАЩИТА
            //GET_AC(ch) += (int)(add_classes[icls][9]*(float)ch->classes[icls]);
        }

    /*БОНУСЫ К ЗАЩИТЕ ОТ ДОСПЕХОВ */
    for (i = 0; i < NUM_WEARS; i++) {
        /* Обрабатываються позиции:
           тело, шлем, наручи, перчатки, поножи, сапоги, плащь */
        if (i == WEAR_BODY || i == WEAR_HEAD || i == WEAR_ARMS || i == WEAR_HANDS || i == WEAR_LEGS
            || i == WEAR_FEET || i == WEAR_ABOUT) {
            if ((obj = GET_EQ(ch, i))) {
                if (GET_OBJ_TYPE(obj) == ITEM_ARMOR)
                    GET_AC_WEAR(ch) += get_max_ac_wear_obj(ch, obj);
            } else
                GET_AC_WEAR(ch) += GET_AC(ch);
        }
    }

    GET_AC_WEAR(ch) = GET_AC_WEAR(ch) / 7;
    /* БОНУС К ЗАЩИТЕ ОТ ЩИТА
       if (GET_EQ(ch,WEAR_SHIELD))
       GET_AC_WEAR(ch) += MAX(1,GET_OBJ_VAL(GET_EQ(ch,WEAR_SHIELD),0)/2); */

}

void recalc_params(struct char_data *ch)
{
    if (IS_MOB(ch))
        recalc_mob(ch);
    else
        recalc_char(ch);
}

void recalc_realtime(struct char_data *ch)
{
    struct affected_type *aff;

    memset((char *) &ch->realtime_abils, 0, sizeof(struct char_realtime_data));

//Положение
    if (GET_POS(ch) < POS_FIGHTING)
        GET_AC_RT(ch) =
            -(GET_REAL_AC(ch) -
              (int) ((float) GET_REAL_AC(ch) /
                     (2.0 + ((float) (POS_FIGHTING - GET_POS(ch)) / 3.0))));

//Эффекты
    for (aff = ch->affected; aff; aff = aff->next)
        switch (SPELL_NO(aff->type)) {
            case SPELL_HOLD:
                GET_AC_RT(ch) -= 100;
                break;
            case SPELL_GRASP:
                GET_AC_RT(ch) -= 30;
                break;
        }

}

AEVENT(event_die)
{
    int spl = find_spell_num(SPELL_MAKE_GHOLA_S5);
    struct char_data *ch = params->actor;

    act_affect_mess(spl, ch, ch, TRUE, TYPE_MESS_HIT);
    mag_damage(spl, GET_REAL_MAX_HIT(ch) * 3, ch, ch, FALSE, 8, TRUE);
}

void make_ghola(struct char_data *ch, int type, int level)
{
    struct affected_type af[MAX_SPELL_AFFECTS];
    int spl[6], i;
    int time = SECS_PER_MUD_HOUR;

    if (general_savingthrow_3(ch, SAV_FORT, level / 5) && type != 1) {
        send_to_charf(ch, "К Вам вренулся обычный облик.\r\n");
        act("К 1д вернулся обычный облик.", "Км", ch);
        return;
    }

    if (MOB_FLAGGED(ch, MOB_NOFIGHT) || IS_SHOPKEEPER(ch)) {
        act_affect_mess(find_spell_num(SPELL_MAKE_GHOLA), ch, ch, TRUE, TYPE_MESS_FAIL);
        return;
    }

    for (i = 0; i < MAX_SPELL_AFFECTS; i++) {
        af[i].type = 0;
        af[i].bitvector = 0;
        af[i].modifier = 0;
        af[i].battleflag = 0;
        af[i].level = 0;
        af[i].main = 0;
        af[i].location = APPLY_NONE;
    }

    spl[1] = find_spell_num(SPELL_MAKE_GHOLA_S1);
    spl[2] = find_spell_num(SPELL_MAKE_GHOLA_S2);
    spl[3] = find_spell_num(SPELL_MAKE_GHOLA_S3);
    spl[4] = find_spell_num(SPELL_MAKE_GHOLA_S4);

    switch (type) {
        case 1:
            af[0].type = spl[type];
            af[0].battleflag = 0;
            af[0].level = level;
            af[0].main = TRUE;
            af[0].location = APPLY_CON;
            af[0].modifier = -2;
            af[0].bitvector = 0;
            af[0].level = level;
            af[0].duration = time;
            af[0].owner = GET_ID(ch);
            for (i = 0; i < MAX_SPELL_AFFECTS; i++)
                if (af[i].bitvector || af[i].location != APPLY_NONE)
                    affect_join_char(ch, af + i);

            act_affect_mess(spl[type], ch, ch, TRUE, TYPE_MESS_HIT);
            break;
        case 2:
            af[0].type = spl[type];
            af[0].battleflag = 0;
            af[0].level = level;
            af[0].main = TRUE;
            af[0].location = APPLY_CON;
            af[0].modifier = -2;
            af[0].bitvector = 0;
            af[0].level = level;
            af[0].duration = time;
            af[0].owner = GET_ID(ch);
            af[1].type = spl[type];
            af[1].battleflag = 0;
            af[1].level = level;
            af[1].main = FALSE;
            af[1].location = APPLY_CHA;
            af[1].modifier = -5;
            af[1].bitvector = 0;
            af[1].level = level;
            af[1].duration = time;
            af[1].owner = GET_ID(ch);
            for (i = 0; i < MAX_SPELL_AFFECTS; i++)
                if (af[i].bitvector || af[i].location != APPLY_NONE)
                    affect_join_char(ch, af + i);

            act_affect_mess(spl[type], ch, ch, TRUE, TYPE_MESS_HIT);
            break;
        case 3:
            af[0].type = spl[type];
            af[0].battleflag = 0;
            af[0].level = level;
            af[0].main = TRUE;
            af[0].location = APPLY_CON;
            af[0].modifier = -2;
            af[0].bitvector = 0;
            af[0].level = level;
            af[0].duration = time;
            af[0].owner = GET_ID(ch);
            af[1].type = spl[type];
            af[1].battleflag = 0;
            af[1].level = level;
            af[1].main = FALSE;
            af[1].location = APPLY_CHA;
            af[1].modifier = -5;
            af[1].bitvector = 0;
            af[1].level = level;
            af[1].duration = time;
            af[1].owner = GET_ID(ch);
            af[2].type = spl[type];
            af[2].battleflag = 0;
            af[2].level = level;
            af[2].main = FALSE;
            af[2].location = APPLY_CON;
            af[2].modifier = -2;
            af[2].bitvector = 0;
            af[2].level = level;
            af[2].duration = time;
            af[2].owner = GET_ID(ch);
            for (i = 0; i < MAX_SPELL_AFFECTS; i++)
                if (af[i].bitvector || af[i].location != APPLY_NONE)
                    affect_join_char(ch, af + i);

            act_affect_mess(spl[type], ch, ch, TRUE, TYPE_MESS_HIT);
            break;
        case 4:
            af[0].type = spl[type];
            af[0].battleflag = 0;
            af[0].level = level;
            af[0].main = TRUE;
            af[0].location = APPLY_CON;
            af[0].modifier = -2;
            af[0].bitvector = 0;
            af[0].level = level;
            af[0].duration = time;
            af[0].owner = GET_ID(ch);
            af[1].type = spl[type];
            af[1].battleflag = 0;
            af[1].level = level;
            af[1].main = FALSE;
            af[1].location = APPLY_CHA;
            af[1].modifier = -5;
            af[1].bitvector = 0;
            af[1].level = level;
            af[1].duration = time;
            af[1].owner = GET_ID(ch);
            af[2].type = spl[type];
            af[2].battleflag = 0;
            af[2].level = level;
            af[2].main = FALSE;
            af[2].location = APPLY_CON;
            af[2].modifier = -2;
            af[2].bitvector = 0;
            af[2].level = level;
            af[2].duration = time;
            af[2].owner = GET_ID(ch);
            af[3].type = spl[type];
            af[3].battleflag = 0;
            af[3].level = level;
            af[3].main = FALSE;
            af[3].location = APPLY_WIS;
            af[3].modifier = -2;
            af[3].bitvector = 0;
            af[3].level = level;
            af[3].duration = time;
            af[3].owner = GET_ID(ch);
            af[4].type = spl[type];
            af[4].battleflag = 0;
            af[4].level = level;
            af[4].main = FALSE;
            af[4].location = APPLY_INT;
            af[4].modifier = -2;
            af[4].bitvector = 0;
            af[4].level = level;
            af[4].duration = time;
            af[4].owner = GET_ID(ch);
            for (i = 0; i < MAX_SPELL_AFFECTS; i++)
                if (af[i].bitvector || af[i].location != APPLY_NONE)
                    affect_join_char(ch, af + i);
            act_affect_mess(spl[type], ch, ch, TRUE, TYPE_MESS_HIT);
            break;
        case 5:
            struct event_param_data params;

            init_event_param(&params);
            params.actor = ch;
            add_event(0, 0, event_die, &params);
            break;
    }
}

struct obj_data *have_lockpick(struct char_data *ch)
{
    struct obj_data *o = NULL;

    if ((o = GET_EQ(ch, WEAR_HOLD)) != NULL)
        if (IS_LOCKPICK(o))
            return (o);

    for (o = ch->carrying; o; o = o->next_content)
        if (IS_LOCKPICK(o))
            return (o);


    return (NULL);
}


int get_max_ac_wear_obj(struct char_data *ch, struct obj_data *obj)
{
    if (GET_OBJ_TYPE(obj) != ITEM_ARMOR)
        return 0;
    float level = MIN(30, GET_LEVEL(ch)), koef, result = 0;

    koef = 3.45 - (0.95 + (level * 0.05));
    result = (float) GET_OBJ_VAL(obj, 0) / koef;

    return (int) (result);
}

void add_mob_command(struct char_data *mob, int no_cmd, int obj_vnum, char *arg, int mob_vnum,
                     char *err, int script, int extract, int tool, int lroom, int linv,
                     char *active, char *active_room, char *to_char, char *to_room, int xscript)
{
    struct item_op_data *k;

    CREATE(k, struct item_op_data, 1);

    k->command = no_cmd;
    k->tool = tool;
    k->obj_vnum = obj_vnum;
    k->mob_vnum = mob_vnum;
    k->load_room = lroom;
    k->load_char = linv;
    k->script_vnum = script;
    k->extract = extract;
    k->script = xscript;

    if (arg) {
        CREATE(k->arg, char, strlen(arg) + 1);

        memcpy(k->arg, arg, strlen(arg) + 1);
    }
    if (err) {
        CREATE(k->error, char, strlen(err) + 1);

        memcpy(k->error, err, strlen(err) + 1);
    }

    if (active) {
        CREATE(k->active, char, strlen(active) + 1);

        memcpy(k->active, active, strlen(active) + 1);
    }

    if (active_room) {
        CREATE(k->active_room, char, strlen(active_room) + 1);

        memcpy(k->active_room, active_room, strlen(active_room) + 1);
    }

    if (to_char) {
        CREATE(k->mess_load_char, char, strlen(to_char) + 1);

        memcpy(k->mess_load_char, to_char, strlen(to_char) + 1);
    }

    if (to_room) {
        CREATE(k->mess_load_room, char, strlen(to_room) + 1);

        memcpy(k->mess_load_room, to_room, strlen(to_room) + 1);
    }

    k->next = mob->operations;
    mob->operations = k;
}

/////////////////////////////

void add_weapon_damage(struct weapon_data *weapon, int type, int min, int max)
{
    struct weapon_damage_data *k;

    CREATE(k, struct weapon_damage_data, 1);

    k->type_damage = type;
    k->min_damage = min;
    k->max_damage = max;

    k->next = weapon->damages;
    weapon->damages = k;
}

void add_missile_damage(struct missile_data *missile, int typ, int min, int max)
{
    struct weapon_damage_data *k;

    CREATE(k, struct weapon_damage_data, 1);

    k->type_damage = typ;
    k->min_damage = min;
    k->max_damage = max;

    k->next = missile->damages;
    missile->damages = k;
}


struct obj_data *get_missile(struct char_data *ch, struct obj_data *weapObj)
{
    struct obj_data *quiver = GET_EQ(ch, WEAR_QUIVER), *c, *c_next;
    int type_missile;

    if (quiver == NULL)         //Не одет колчан значит нет и стрел.
        return (NULL);

    switch (GET_OBJ_SKILL(weapObj)) {
        case SKILL_BOWS:
            type_missile = MISSILE_ARROW;
            break;
        case SKILL_CROSSBOWS:
            type_missile = MISSILE_BOLT;
            break;
        default:
            type_missile = MISSILE_NONE;
            break;
    }

    for (c = quiver->contains; c; c = c_next) {
        c_next = c->next_content;
        if (c->missile && c->obj_flags.value[0] == type_missile) {
            if (c->obj_flags.value[2] <= 0)
                extract_obj(c);
            else
                return (c);
        }
    }
    return (NULL);
}


void world_period_update(int rnum)
{
    struct room_data *room = &world[rnum];
    int zone = room->zone, rmob = -1, robj = -1;
    struct room_period_data *p = room->period;
    struct obj_data *obj = NULL;
    struct char_data *mob = NULL;

    if (!p)
        return;

    if (zone_table[zone].time_info.hours == p->start) {
        if (p->object) {
            obj = read_object(p->object, VIRTUAL, TRUE);
            if (!obj)
                return;
            obj_to_room(obj, rnum);
        }

        if (p->monster) {
            mob = read_mobile(p->monster, VIRTUAL);
            if (!mob)
                return;
            char_to_room(mob, rnum);
        }

        if (p->start_room && room->people)
            act(p->start_room, "КМмп", room->people, mob, obj);

        return;
    }

    if (zone_table[zone].time_info.hours == p->stop) {
        if (p->object)
            for (obj = object_list; obj; obj = obj->next)
                if (GET_OBJ_VNUM(obj) == p->object) {
                    robj = GET_OBJ_RNUM(obj);
                    extract_obj(obj);
                    break;
                }
        if (p->monster)
            for (mob = character_list; mob; mob = mob->next)
                if (GET_MOB_VNUM(mob) == p->monster) {
                    rmob = GET_MOB_RNUM(mob);
                    extract_char(mob, FALSE);
                    break;
                }
        if (p->stop_room && room->people) {
            if (robj != -1 && rmob != -1) {
                act(p->stop_room, "КМмп", room->people, mob_proto + rmob, obj_proto + robj);
            } else {
                if (robj != -1)
                    act(p->stop_room, "КМп", room->people, obj_proto + robj);
                if (rmob != -1)
                    act(p->stop_room, "КМм", room->people, mob_proto + rmob);
            }
        }
    }
}

void char_period_update(struct char_data *ch, int pulse)
{
    struct char_data *tch, *victim = NULL;
    int in_room = IN_ROOM(ch);

    if (IN_ROOM(ch) == NOWHERE)
        return;
    struct room_data *room = &world[in_room];
    struct mob_period_data *period = NULL;


    if (!IS_NPC(ch))
        return;

    if (GET_DEFAULT_POS(ch) != GET_POS(ch) || FIGHTING(ch))
        return;

    for (period = ch->period; period; period = period->next) {
        if (!(pulse % period->period)) {
            if (!period->target) {
                victim = room->people;

                if (period->to_char && victim)
                    act(period->to_char, "мМ", ch, victim);

                if (period->to_room)
                    act(period->to_room, "Кмм", ch, victim);
            } else if (period->target == 1) {
                for (tch = room->people; tch && !victim; tch = tch->next_in_room) {
                    if (tch == ch)
                        continue;
                    if ((IS_NPC(tch) && number(1, 5) == 3) || (!IS_NPC(tch) && number(1, 3) == 2))
                        victim = tch;
                }
                if (!victim)
                    victim = room->people;

                if (period->to_char && victim)
                    act(period->to_char, "мМ", ch, victim);

                if (period->to_room)
                    act(period->to_room, "Кмм", ch, victim);
            } else if (period->target == 2) {
                for (tch = room->people; tch; tch = tch->next_in_room)
                    if (!IS_NPC(tch) && period->to_char)
                        act(period->to_char, "мМ", ch, tch);
            }
            break;
        }
    }

}


void check_sets(struct char_data *ch)
{
    int i, eq[NUM_WEARS], j, l, variante = 0, set = -1;

    for (i = 0; i < NUM_WEARS; i++)
        eq[i] = -1;

    if (!ch)
        return;

    for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i))
            eq[i] = GET_OBJ_VNUM(GET_EQ(ch, i));

    for (j = 0; j < top_of_sets; j++) {
        if (!set_table[j].list_objects)
            continue;
        for (l = 0; l < (int) set_table[j].list_objects->size(); l++) {
            for (i = 0; i < NUM_WEARS; i++) {
                std::vector < int >lo = *set_table[j].list_objects;

                if (eq[i] == -1)
                    continue;
                if (eq[i] == lo[l]) {
                    variante++;
                    set = j;
                }
            }
        }
    }

    /* if (set != -1 && variante)
       send_to_charf(ch,"set %d | variante %d\r\n",set,variante); */

    int max = 0, old_set = ch->set_number, old_var = ch->set_variante;

    ch->set_number = -1;
    ch->set_variante = -1;
    if (set != -1)
        for (l = 0; l < (int) set_table[set].variante->size(); l++) {
            std::vector < struct set_variante_data >vc = *set_table[set].variante;
            struct set_variante_data varz = vc[l];

            if (varz.count_objects <= variante) {
                if (varz.count_objects >= max) {
                    ch->set_number = set;
                    ch->set_variante = l;
                    max = varz.count_objects;
                }
            }
        }

    if (old_set != ch->set_number || old_var != ch->set_variante) {
        if (old_var < ch->set_variante) {
            ch->set_change = 1;
            ch->set_message_num = ch->set_number;
            ch->set_message_var = ch->set_variante;
        } else if (old_var > ch->set_variante) {
            ch->set_change = -1;
            if (old_set != -1)
                ch->set_message_num = old_set;
            if (old_var != -1)
                ch->set_message_var = old_var;
        }
    }

    /* send_to_charf(ch,"Вариант set [%d]%d | var [%d]%d | mess %d | change %d\r\n",
       old_set, ch->set_number, old_var, ch->set_variante, ch->set_message_var, ch->set_change
       ); */

}

void clear_mob_specials(struct char_data *mob)
{
    int i;

    mob->npc()->specials.AlrNeed = TRUE;
    mob->npc()->specials.alr_helper.clear();
    mob->period = NULL;
    mob->spec_hit = NULL;
    mob->npc()->specials.move_to = NOWHERE;
    mob->npc()->specials.CMessChar = NULL;
    mob->npc()->specials.CMessRoom = NULL;
    mob->npc()->specials.death_flag = 0;
    mob->shop_data = NULL;
// mob->func = NULL;

    mob->npc()->specials.powered = 0;

    for (i = 1; i <= MAX_SKILLS; i++)
        SET_SKILL(mob, i) = 0;

    for (i = 1; i <= MAX_SPELLS; i++)
        GET_SPELL_TYPE(mob, i) = 0;


    REMOVE_BIT(MOB_FLAGS(mob, MOB_HELPER), MOB_HELPER);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_GUARD), MOB_GUARD);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_LOOTER), MOB_LOOTER);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_AGGRESSIVE), MOB_AGGRESSIVE);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_AGGRGOOD), MOB_AGGRGOOD);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_AGGREVIL), MOB_AGGREVIL);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_AGGRNEUTRAL), MOB_AGGRNEUTRAL);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_AGGR_SLEEP), MOB_AGGR_SLEEP);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_AGGR_DAY), MOB_AGGR_DAY);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_AGGR_NIGHT), MOB_AGGR_NIGHT);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_XENO), MOB_XENO);
    REMOVE_BIT(MOB_FLAGS(mob, MOB_SPEC), MOB_SPEC);

    REMOVE_BIT(NPC_FLAGS(mob, NPC_NORTH), NPC_NORTH);
    REMOVE_BIT(NPC_FLAGS(mob, NPC_EAST), NPC_EAST);
    REMOVE_BIT(NPC_FLAGS(mob, NPC_WEST), NPC_WEST);
    REMOVE_BIT(NPC_FLAGS(mob, NPC_SOUTH), NPC_SOUTH);
    REMOVE_BIT(NPC_FLAGS(mob, NPC_UP), NPC_UP);
    REMOVE_BIT(NPC_FLAGS(mob, NPC_DOWN), NPC_DOWN);
    REMOVE_BIT(NPC_FLAGS(mob, NPC_INVIS), NPC_INVIS);
    REMOVE_BIT(NPC_FLAGS(mob, NPC_REPOPKILL), NPC_REPOPKILL);
    REMOVE_BIT(NPC_FLAGS(mob, NPC_HELPED), NPC_HELPED);
}

void stop_defence(struct char_data *ch)
{

    if (GET_AF_BATTLE(ch, EAF_BLOCK)) {
        CLR_AF_BATTLE(ch, EAF_BLOCK);
        act("Вы прекратили блокировать удары противника.", FALSE, ch, 0, 0, TO_CHAR);
    }

    if (GET_AF_BATTLE(ch, EAF_DEVIATE)) {
        CLR_AF_BATTLE(ch, EAF_DEVIATE);
        act("Вы прекратили уклоняться от ударов противника.", FALSE, ch, 0, 0, TO_CHAR);
    }

    if (GET_AF_BATTLE(ch, EAF_PARRY)) {
        CLR_AF_BATTLE(ch, EAF_PARRY);
        act("Вы прекратили парировать удары противника.", FALSE, ch, 0, 0, TO_CHAR);
    }

    if (GET_AF_BATTLE(ch, EAF_MULTYPARRY)) {
        CLR_AF_BATTLE(ch, EAF_MULTYPARRY);
        act("Вы прекратили отражать удары противника.", FALSE, ch, 0, 0, TO_CHAR);
    }

}


bool check_killer_zone(struct char_data *ch)
{
    struct char_data *zch = NULL;
    int in_zone = -1;

    if (IN_ROOM(ch) == NOWHERE)
        return (FALSE);

    in_zone = world[IN_ROOM(ch)].zone;

    for (zch = character_list; zch; zch = zch->next)
        if (!IS_NPC(zch) && IN_ROOM(zch) != NOWHERE && world[IN_ROOM(zch)].zone == in_zone) {
            if (ch->pk_list.count(GET_ID(zch)))
                return (TRUE);
        }

    return (FALSE);
}


const char *dam_mess[][4] = {
    {"никак", "", "", ""},      //0
    {"слегка", "", "", ""},     //1
    {"очень легко", "", "", ""},        //2
    {"легко", "", "", ""},      //3
    {"", "", "", ""},           //4
    {"тяжело", "", "", ""},     //5
    {"очень тяжело", "", "", ""},       //6
    {"смертельно", "", "", ""}, //7
    {"калечаще", "", "", ""},   //8
    {"уничтожительно", "", "", ""},     //9
    {"УНИЧТОЖИТЕЛЬНО", "", "", ""}      //10
};

void rstring(const char *str, int dam, char *lbuf)
{
    const char *i = NULL;
    char *buf;
    int stopbyte, padis, mdam;

    if (dam == 0)
        mdam = 0;
    else if (dam <= 4)
        mdam = 1;
    else if (dam <= 8)
        mdam = 2;
    else if (dam <= 14)
        mdam = 3;
    else if (dam <= 20)
        mdam = 4;
    else if (dam <= 26)
        mdam = 5;
    else if (dam <= 32)
        mdam = 6;
    else if (dam <= 46)
        mdam = 7;
    else if (dam <= 52)
        mdam = 8;
    else if (dam <= 62)
        mdam = 9;
    else
        mdam = 10;

    buf = lbuf;
    for (stopbyte = 0; stopbyte < MAX_STRING_LENGTH; stopbyte++) {
        if (*str == '#') {
            switch (*(++str)) {
                case '0':
                    i = dam_mess[mdam][0];
                    break;
                case '1':
                    i = dam_mess[mdam][1];
                    break;
                case '2':
                    i = dam_mess[mdam][2];
                    break;
                case '3':
                    i = dam_mess[mdam][3];
                    break;
                case 'K':
                    if (*(str + 1) < '0' || *(str + 1) > '3') {
                        i = "";
                    } else {
                        padis = *(++str) - '0';
                        i = dam_mess[mdam][padis];
                    }
                    break;
                default:
                    i = "";
                    break;
            }
            while ((*buf = *(i++)))
                buf++;
            str++;
        } else if (*str == '\\') {
            if (*(str + 1) == 'r') {
                *(buf++) = '\r';
                str += 2;
            } else if (*(str + 1) == 'n') {
                *(buf++) = '\n';
                str += 2;
            } else
                *(buf++) = *(str++);
        } else if (!(*(buf++) = *(str++)))
            break;
    }

    *(++buf) = '\0';
}

void ShowHitMessage(struct char_data *ch, struct char_data *victim, struct obj_data *weapObj,
                    struct obj_data *missObj, int dam, int hit, int type, int location)
{
    char buf_char[MAX_STRING_LENGTH], buf_vict[MAX_STRING_LENGTH], buf_room[MAX_STRING_LENGTH];
    char *to_char, *to_vict, *to_room;
    struct obj_data *body = NULL;
    struct char_data *tch = NULL;
    CItem *ms = NULL;
    int i, hl;

// hit = 0; //пока все ударить
    CItem *message = HitMess.FindItem(hit);

    if (!message) {
        send_to_charf(ch, "Сообщение удара не найдено!\r\n");
        return;
    }

    if (dam == 0)
        type = M_NONE;
    if (type == M_SKIN && victim->body) {
        hl = wpos_to_wear[victim->body[location].wear_position];
        if (hl && GET_EQ(victim, hl)) {
            body = GET_EQ(victim, hl);
            type = M_ARM;
        }
    }

    if (!weapObj)
        switch (type) {
            case M_NONE:
                ms = message->GetItem(MS_MISS_NONE)->GetItem(0);
                break;
            case M_HIT:
                ms = message->GetItem(MS_HIT_NONE)->GetItem(0);
                break;
            case M_KILL:
                ms = message->GetItem(MS_KILL_NONE)->GetItem(0);
                break;
            case M_ARM:
                ms = message->GetItem(MS_ARM_NONE)->GetItem(0);
                break;
            case M_SKIN:
                ms = message->GetItem(MS_SKIN_NONE)->GetItem(0);
                break;
    } else
        switch (type) {
            case M_NONE:
                ms = message->GetItem(MS_MISS_WEAP)->GetItem(0);
                break;
            case M_HIT:
                ms = message->GetItem(MS_HIT_WEAP)->GetItem(0);
                break;
            case M_KILL:
                ms = message->GetItem(MS_KILL_WEAP)->GetItem(0);
                break;
            case M_ARM:
                ms = message->GetItem(MS_ARM_WEAP)->GetItem(0);
                break;
            case M_SKIN:
                ms = message->GetItem(MS_SKIN_WEAP)->GetItem(0);
                break;
        }

    if (!ms) {
        send_to_charf(ch, "Не найдено сообщение удара по виду!\r\n");
        return;
    }

    to_char = ms->GetItem(MS_SUB_CHAR)->GetString();
    to_vict = ms->GetItem(MS_SUB_VICT)->GetString();
    to_room = ms->GetItem(MS_SUB_ROOM)->GetString();

//Таблица повреждений
    rstring(to_char, dam, buf_char);
    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        sprintf(buf_char + strlen(buf_char), " [#1 хп]");
    to_char = buf_char;

    rstring(to_vict, dam, buf_vict);
    if (PRF_FLAGGED(victim, PRF_CODERINFO))
        sprintf(buf_vict + strlen(buf_vict), " [#1 хп]");
    to_vict = buf_vict;

    rstring(to_room, dam, buf_room);
    to_room = buf_room;

    if (victim->body && victim->body[location].name.size() != 0 && !body) {
        body = create_obj();
        body->item_number = NOTHING;
        body->in_room = NOWHERE;
        GET_OBJ_SEX(body) = SEX_NEUTRAL;

//  get_name_pad(victim->body[location].name.c_str(),PAD_VIN,PAD_OBJECT);

        body->short_description =
            str_dup(get_name_pad(victim->body[location].name.c_str(), PAD_IMN, PAD_OBJECT));
        body->names = str_dup(victim->body[location].name.c_str());
        body->name = str_dup(body->short_description);
        for (i = 0; i < 6; i++)
            body->PNames[i] =
                str_dup(get_name_pad(victim->body[location].name.c_str(), i, PAD_OBJECT));
    }

    if (weapObj == NULL)
        weapObj = body;
    if (missObj == NULL)
        missObj = body;
    if (dam < 1)
        send_to_char(CCYEL(ch, C_CMP), ch);    
    else
        send_to_char(CCIYEL(ch, C_CMP), ch);    
    act(to_char, "Ммпппч", ch, victim, weapObj, missObj, body, dam);
    send_to_char(CCNRM(ch, C_CMP), ch);
   
    if (dam < 1)
        send_to_char(CCRED(victim, C_CMP), victim);      
    else
        send_to_char(CCIRED(victim, C_CMP), victim);

    act(to_vict, "мМпппч", ch, victim, weapObj, missObj, body, dam);
    send_to_char(CCNRM(victim, C_CMP), victim);

    if (IN_ROOM(ch) != NOWHERE) {
        for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room)
            if (!PRF_FLAGGED(tch, PRF_SELFMESS) && !IS_NPC(tch) && tch != ch && tch != victim) {
                act(to_room, "ммМппп", ch, victim, tch, weapObj, missObj, body);
            }
    }

    if (body)
        extract_obj(body);
}

void change_pet_name(struct char_data *ch, struct char_data *cvict)
{
    char buf[256], name[256];

    strcpy(name, cvict->player.names);

// sprintf(buf,"%s ожидает приказа своего хозяина.",get_name_pad(name,PAD_IMN,PAD_MONSTER));
// cvict->player.long_descr = str_dup(buf);

    sprintf(buf, "%s ожидает приказа своего хозяина.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    str_reassign(cvict->player.charm_descr, buf);

    sprintf(buf, "%s ожидает Вашего приказа.", get_name_pad(name, PAD_IMN, PAD_MONSTER));
    str_reassign(cvict->player.charm_descr_me, buf);
}

/* Увеличиваем количество выполненых квестов на incvalue */
void inc_exp_script_num(struct char_data *ch, int vnum, int incvalue)
{
    int i;

    if (IS_NPC(ch) || vnum <= 0)
        return;

    if (ch->ScriptExp.vnum) {
        for (i = 0; i < ch->ScriptExp.count; i++)
            if (*(ch->ScriptExp.vnum + i) == vnum) {
                *(ch->ScriptExp.howmany + i) += incvalue;
                return;
            }

        if (!(ch->ScriptExp.count % 10L)) {
            RECREATE(ch->ScriptExp.howmany, int, (ch->ScriptExp.count / 10L + 1) * 10L);
            RECREATE(ch->ScriptExp.vnum, int, (ch->ScriptExp.count / 10L + 1) * 10L);
        }
    } else {
        ch->ScriptExp.count = 0;
        CREATE(ch->ScriptExp.vnum, int, 10);
        CREATE(ch->ScriptExp.howmany, int, 10);
    }

    *(ch->ScriptExp.vnum + ch->ScriptExp.count) = vnum;
    *(ch->ScriptExp.howmany + ch->ScriptExp.count++) = incvalue;

}

/* возвращет количество выполненых vnum квестов */
int get_exp_script_vnum(struct char_data *ch, int vnum)
{
    int i;

    if (IS_NPC(ch) || vnum <= 0)
        return (0);

    if (ch->ScriptExp.vnum) {
        for (i = 0; i < ch->ScriptExp.count; i++)
            if (*(ch->ScriptExp.vnum + i) == vnum)
                return (*(ch->ScriptExp.howmany + i));
    }
    return (0);
}


int get_extend_exp(int exp, struct char_data *ch, int type)
{
    float koefr = 0;

    if (IS_NPC(ch))
        return (exp);

//коэф на алигнмент
    koefr = (exp_align[calc_alignment(ch)][type]);
    exp = (int) ((float) exp * (float) ((float) koefr / 10.0));
//send_to_charf(ch,"Поправка на наклоности %d\r\n",exp);

    return (exp);
}

void gexp(struct char_data *tch, int exp, int type, int maxlevel, int total_killers, int noexp)
{
    int level, dec_level, exs_num;
    float koefr;

    if (GET_LEVEL(tch))
        level = GET_LEVEL(tch);
    else
        level = 1;

    if (!maxlevel)
        maxlevel = 1;

    koefr = (float) level / maxlevel;
    dec_level = maxlevel - level;

    if (dec_level < 5)
        koefr = koefr;
    else if (dec_level < 8)
        koefr = koefr * 0.8;
    else if (dec_level < 10)
        koefr = koefr * 0.5;
    else
        koefr = koefr * 0.25;

    exp =
        (int) ((float) ((float) exp / (float) total_killers) /
               (float) ((float) maxlevel / (float) level));

    //send_to_charf(tch,"Стартовый опыт %d ",exp);

    /* Замакс опыта */
    exs_num = get_exp_script_vnum(tch, noexp);
    if (exs_num && exp > 0) {
        int texp = (exp * exs_num) / 20;

        exp = MAX(1, exp - texp);
    }

    exp = get_extend_exp(exp, tch, type);

    //send_to_charf(tch,"Опыт до изменения %d ",exp);

    exp = MIN(max_exp_gain_pc(tch), exp);
    //send_to_charf(tch,"Опыт после изменения %d\r\n",exp);

    gain_exp(tch, exp, TRUE);

    inc_exp_script_num(tch, noexp, 1);
}

void main_gexp(struct char_data *actor, int exp, int type, int noexp, int group)
{
    int maxlevel = GET_LEVEL(actor), total_killers = 1;
    struct char_data *master = NULL;
    struct follow_type *f;

    /* Подсчет состава группы */
    if (AFF_FLAGGED(actor, AFF_GROUP) && group) {
        master = (actor->master ? actor->master : actor);
        for (f = master->followers; f; f = f->next) {
            if (!AFF_FLAGGED(f->follower, AFF_GROUP))
                continue;
            if (IN_ROOM(f->follower) != IN_ROOM(master))        //экспа только тем
                continue;       //кто в одной локации
            maxlevel = (maxlevel > GET_LEVEL(f->follower)) ? maxlevel : GET_LEVEL(f->follower);
            total_killers++;
        }

        for (f = master->followers; f; f = f->next) {
            if (!AFF_FLAGGED(f->follower, AFF_GROUP))
                continue;
            if (IN_ROOM(f->follower) != IN_ROOM(master))        //экспа только тем
                continue;       //кто в одной локации

            gexp(f->follower, exp, type, maxlevel, total_killers, noexp);
        }

        if (master == actor)
            gexp(master, exp, type, maxlevel, total_killers, noexp);
    } else
        gexp(actor, exp, type, maxlevel, total_killers, noexp);
}

void go_jail(struct char_data *ch, struct char_data *vict, int time)
{
    char buf[MAX_STRING_LENGTH];

    GET_LOADROOM(vict) = jail_start_room;
    HELL_DURATION(vict) = time * SECS_PER_MUD_TICK;
    SET_BIT(PLR_FLAGS(vict, PLR_HELLED), PLR_HELLED);
    act("1и посажен1(,а,о,ы) в тюрму!", "!Км", vict);
    act("Вас посадили в тюрму за нарушение правил игры.", "М", vict);
    char_from_room(vict);
    char_to_room(vict, r_jail_start_room);
    look_at_room(vict, r_jail_start_room);
    sprintf(buf, "Отправили в тюрму %s, время %d", GET_NAME(vict), time);
    mudlog(buf, BRF, LVL_GOD, TRUE);

}

bool check_abuse(int id, int owner, int count)
{
    bool result = FALSE, add = TRUE;
    int i;

    for (i = 0; i < (int) bug_abuse.size(); i++) {
        if (bug_abuse[i].no != id)
            continue;
        if (bug_abuse[i].owner == owner) {
            bug_abuse[i].owner++;
            add = FALSE;
            if (bug_abuse[i].count > count)
                result = TRUE;
        }
    }

    if (add) {
        struct bug_abuse_data abuse;

        abuse.no = id;
        abuse.owner = owner;
        abuse.count = 0;
        bug_abuse.push_back(abuse);
    }

    return (result);
}


void add_missed(struct char_data *ch, struct char_data *victim)
{
    struct follow_type *k, *n;
    int fnd = FALSE;


    for (n = ch->missed_magic; n; n = n->next)
        if (n->follower == victim) {
            n->type++;
            fnd = TRUE;
        }

    if (!fnd) {
        CREATE(k, struct follow_type, 1);

        k->follower = ch;
        k->type = 1;
        k->next = ch->missed_magic;
        ch->missed_magic = k;
    }

}

int get_missed(struct char_data *ch, struct char_data *victim)
{
    struct follow_type *n;
    int result = 0;

    for (n = ch->missed_magic; n; n = n->next)
        if (n->follower == victim)
            result++;


    return (result);
}


/*
Возвращает уровень видимости игрока
0 - Не видно
1 - виден силует
2 - виден большой (размер) силует
3 - виден большой (размер) орк (раса)
4 - полная видимость

Алгоритм проверки:

1. Сравнивается освещение в локации: выставляется оценка
2. Сравнивается hide эффекты и детект на них
3. Cравнивает invisible эффекты и детект на них
4. Сравнивается immortal invisible

Выставляется общая оценка
*/

int can_see(struct char_data *ch, struct char_data *victim)
{

    return 0;
}


void join_party(struct char_data *leader, struct char_data *ch)
{
    struct follow_type *k;

    if (ch->party_leader) {
        send_to_charf(ch, "Вы уже в группе.\r\n");
        return;
    }

    SET_BIT(AFF_FLAGS(ch, AFF_GROUP), AFF_GROUP);
    SET_BIT(AFF_FLAGS(leader, AFF_GROUP), AFF_GROUP);

    if (ch == leader)
        return;

    ch->party_leader = leader;

    CREATE(k, struct follow_type, 1);

    k->follower = ch;
    k->type = 0;
    k->next = leader->party;
    leader->party = k;

}

void leave_party(struct char_data *ch)
{
    struct follow_type *j, *k;

    if (!ch->party_leader)
        return;

    if (!ch->party_leader->party)
        log("ОШИБКА: попытка покинуть группу %s при отсуствии последователей у лидера %s.",
            GET_NAME(ch), GET_NAME(ch->party_leader));
    else if (ch->party_leader->party->follower == ch) {
        k = ch->party_leader->party;
        if (!(ch->party_leader->party = k->next) && !ch->party_leader->party_leader)
            REMOVE_BIT(AFF_FLAGS(ch->party_leader, AFF_GROUP), AFF_GROUP);
        free(k);
    } else {
        for (k = ch->party_leader->party; k->next && k->next->follower != ch; k = k->next);
        if (!k->next)
            log("[leave party] SYSERR: Undefined %s in %s followers list.", GET_NAME(ch),
                GET_NAME(ch->party_leader));
        else {
            j = k->next;
            k->next = j->next;
            free(j);
        }
    }
//  leader     = ch->party_leader;
    ch->party_leader = NULL;
    REMOVE_BIT(AFF_FLAGS(ch, AFF_GROUP), AFF_GROUP);
}


void die_party(struct char_data *ch)
{
    struct follow_type *j, *k;

    for (k = ch->party; k; k = j) {
        j = k->next;
        send_to_charf(k->follower, "Ваша группа расформирована.\r\n");
        leave_party(k->follower);
    }

    if (ch->party_leader) {
        send_to_charf(ch, "Ваша группа расформирована.\r\n");
        leave_party(ch);
    }

}

int perform_group(struct char_data *ch, struct char_data *vict)
{
    if (AFF_FLAGGED(vict, AFF_GROUP) ||
        AFF_FLAGGED(vict, AFF_CHARM) || AFF_FLAGGED(vict, AFF_HELPER) || IS_HORSE(vict)
        )
        return (FALSE);

    SET_BIT(AFF_FLAGS(vict, AFF_GROUP), AFF_GROUP);
    if (ch != vict) {
        act("$N принят$A в Вашу группу.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n принял$g Вас в свою группу.", FALSE, ch, 0, vict, TO_VICT);
        act("$N принят$A в группу $n1.", FALSE, ch, 0, vict, TO_NOTVICT);
    }
    return (TRUE);
}

void change_leader(struct char_data *oleader, struct char_data *nleader, int lead)
{
    struct char_data *tch;
    struct follow_type *k, *next;

    if (oleader == NULL || nleader == NULL)
        return;

    if (!AFF_FLAGGED(oleader, AFF_GROUP)) {
        log("ОШИБКА: Лидер %s не состоит в группе. (change_leader)", GET_NAME(oleader));
        return;
    }


    for (k = oleader->party; k && k->follower->party_leader; k = next) {
        next = k->next;
        tch = k->follower;
        if (IS_MOB(tch))
            continue;
        leave_party(tch);
        join_party(nleader, tch);
        if (nleader != tch)
            act("Новый лидер группы 2и.", "Мм", tch, nleader);
        else
            send_to_charf(nleader, "Вы стали лидером группы.\r\n");
    }

//теперь сам старый лидер
    if (lead) {
        leave_party(oleader);
        join_party(nleader, oleader);
    }

    act("Вы передали управление группой 2д.", "Мм", oleader, nleader);
}


#define BASE_HONOR 100.0
void inc_honor(struct char_data *killer, struct char_data *victim, int total_killers, int maxlevel)
{
    int koef;
    int result;

    if (IN_ROOM(killer) == NOWHERE)
        return;

    if (!ROOM_FLAGGED(IN_ROOM(killer), ROOM_ARENA))
        return;

    koef = (GET_LEVEL(killer) - GET_LEVEL(victim)) * 10;

    if (koef > 100)
        koef = 100;

    result = (int) ((float) (BASE_HONOR / (float) total_killers) - (float) koef);

    send_to_charf(killer, "Вы получили %d %s чести.\r\n", result, desc_count(result, WHAT_POINT));

    GET_HONOR(killer) += result;

}

void event_mob(struct char_data *ch, struct char_data *victim, int event_type, int arg)
{
    struct char_data *tch, *tch_next;
    struct event_mob_data *e;

    if (IN_ROOM(ch) == NOWHERE)
        return;

    switch (event_type) {
        case EVNT_ENTER:       //ВОШЕЛ
            for (tch = world[IN_ROOM(ch)].people; tch; tch = tch_next) {
                tch_next = tch->next_in_room;
                for (e = tch->event_mob; e; e = e->next)
                    if (e->command == event_type) {
                        if (e->argument != -1 && rev_dir[arg] == e->argument)
                            go_script(e->script, ch, tch);
                        else if (e->argument == -1)
                            go_script(e->script, ch, tch);
                    }
            }
            break;
    }

}

// Mob XP Functions (including Con Colors)
// Colors will be numbers:
//  {grey = 0, green = 1, yellow = 2, orange = 3, red = 4, skull = 5}
// NOTE: skull = red when working with anything OTHER than mobs!

double getConColor(int playerlvl, int moblvl)
{
    if (playerlvl + 5 <= moblvl) {
        if (playerlvl + 10 <= moblvl) {
            return 5;
        } else {
            return 4;
        }
    } else {
        switch (moblvl - playerlvl) {
            case 4:
            case 3:
                return 3;
                break;
            case 2:
            case 1:
            case 0:
            case -1:
            case -2:
                return 2;
                break;
            default:
                // More adv formula for grey/green lvls:
                if (playerlvl <= 5) {
                    return 1;   //All others are green.
                } else {
                    if (playerlvl <= 39) {
                        if (moblvl <= (playerlvl - 5 - floor(playerlvl / 10))) {
                            // Its below or equal to the 'grey level':
                            return 0;
                        } else {
                            return 1;
                        }
                    } else {
                        //player over lvl 39:
                        if (moblvl <= (playerlvl - 1 - floor(playerlvl / 5))) {
                            return 0;
                        } else {
                            return 1;
                        }
                    }
                }
        }
    }
}

double getZD(int lvl)
{
    if (lvl <= 7) {
        return 5;
    } else {
        if (lvl <= 9) {
            return 6;
        } else {
            if (lvl <= 11) {
                return 7;
            } else {
                if (lvl <= 15) {
                    return 8;
                } else {
                    if (lvl <= 19) {
                        return 9;
                    } else {
                        if (lvl <= 29) {
                            return 11;
                        } else {
                            if (lvl <= 39) {
                                return 12;
                            } else {
                                if (lvl <= 44) {
                                    return 13;
                                } else {
                                    if (lvl <= 49) {
                                        return 14;
                                    } else {
                                        if (lvl <= 54) {
                                            return 15;
                                        } else {
                                            if (lvl <= 59) {
                                                return 16;
                                            } else {
                                                return 17;      // Approx.
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

/*double getMobXP(int playerlvl, int moblvl)
{
if(moblvl >= playerlvl) {
double temp = ((playerlvl * 5) + 45) * (1 + (0.05 * (moblvl - playerlvl)));
double tempcap = ((playerlvl * 5) + 45) * 1.2;
if(temp > tempcap) {
return floor(tempcap);
}
else {
return floor(temp);
}
}
else {
if(getConColor(playerlvl, moblvl) == 0) {
return 0;
}
else {
return floor((playerlvl * 5) + 45) * (1 - (playerlvl -  moblvl)/getZD(playerlvl));
}
}
}*/


double getMobXP(int playerlvl, int moblvl)
{
    int base = 100, xp = 0;

    if (playerlvl > moblvl) {
        xp = base - (1 - (moblvl - playerlvl)) * 10;
        if (xp < 0)
            xp = 0;
    } else {
        xp = base + ((moblvl - playerlvl) * 10);
    }

    return (double) xp;
}

double getEliteMobXP(int playerlvl, int moblvl)
{
    return getMobXP(playerlvl, moblvl) * 2;
}

// Rested Bonus:
// Restedness is double XP, but if we only have part restedness we must split the XP:

double getMobXPFull(int playerlvl, int moblvl, bool elite, int rest)
{
// rest = how much XP is left before restedness wears off:
    double temp = 0;

    if (elite) {
        temp = getEliteMobXP(playerlvl, moblvl);
    } else {
        temp = getMobXP(playerlvl, moblvl);
    }
// Now to apply restedness.  temp = actual XP.
// If restedness is 0...
    if (rest == 0) {
        return temp;
    } else {
        if (rest >= temp) {
            return temp * 2;
        } else {
//Restedness is partially covering the XP gained.
// XP = rest + (AXP - (rest / 2))
            return rest + (temp - (rest / 2));
        }
    }
}

// Party Mob XP:
double getPartyMobXPFull(int playerlvl, int highestlvl, int sumlvls, int moblvl, bool elite,
                         int rest)
{
    double temp = getMobXPFull(highestlvl, moblvl, elite, 0);

    /*if (playerlvl != sumlvls)   //игрок в группе
        temp = 1.5 * temp * playerlvl / sumlvls;*/

    return temp;
}

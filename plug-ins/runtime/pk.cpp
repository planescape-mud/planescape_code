/* ************************************************************************
*   File: mobmax.c                                      Part of CircleMUD *
*  Usage: ПК система                                                      *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "screen.h"

#include "pk.h"


#define KILLER_UNRENTABLE  30
#define REVENGE_UNRENTABLE 5
#define THIEF_UNRENTABLE   10
// Период за который идет проверка на спаммПК в минутах
#define SPAM_PK_TIME       30
// Время в мин за которое считается недопустимым получать 2й флаг на жертву
#define SPAM_PK_TIME_ONE_CHAR 10
// Время на которое вешается наказание - час
#define TIME_GODS_CURSE    96
// Максимальное допустимое количество флагов за период
#define MAX_PKILL_FOR_PERIOD 3
// Если разница по времени получении флагов TIME_PK_GROUP или менее - считаем
// что быто нападение на группу
#define TIME_PK_GROUP      5

struct pkiller_file_u {
    int unique;
    int pkills;
};

ACMD(do_revenge);


int dec_pk_values(struct char_data *killer, struct char_data *victim, int pkills, int prevenge)
{
    PK_Memory_type::iterator pk;


    log("СНИМАЕМ ФЛАГУБИЙЦЫ: Убийца %s : Жертва %s", GET_NAME(killer), GET_NAME(victim));

    pk = victim->pk_list.find(GET_ID(killer));

    if (pk == victim->pk_list.end())
        return (0);

    victim->pk_list.erase(pk);
    /*    if (!IS_NPC(killer) && !IS_NPC(victim))
       {
       act("Вы использовали право мести для $N2!", FALSE, victim, 0, killer, TO_CHAR);
       act("$n использовал$G свое право мести!", FALSE, victim, 0, killer, TO_VICT);
       } */
    return (1);
}

int inc_pk_values(struct char_data *killer, struct char_data *victim, int pkills, int prevenge)
{

    if (!killer) {
        log("Не найде killer в inc_pk_values");
        return (PK_OK);
    }

    if (PRF_FLAGGED(killer, PRF_NOHASSLE))
        return (PK_OK);

    if (killer->pk_list.count(GET_ID(victim)))
        return (PK_OK);

    if (!victim->pk_list.count(GET_ID(killer))) {
        /* if (pkills <= 0)
           return (PK_OK); */


        /* if (!IS_NPC(killer) && !IS_NPC(victim))
           {
           act("Вы получили право отомстить $n2!", FALSE, killer, 0, victim, TO_VICT);
           act("$N получил$G право отомстить Вам!", FALSE, killer, 0, victim, TO_CHAR);
           } */
    }

    victim->pk_list[GET_ID(killer)] = KILLER_UNRENTABLE * SECS_PER_MUD_TICK;

    if (!IS_NPC(killer) && !IS_NPC(victim)) {
        RENTABLE(killer) = MAX(RENTABLE(killer), time(NULL) + KILLER_UNRENTABLE * 60);
        RENTABLE(victim) = MAX(RENTABLE(victim), time(NULL) + REVENGE_UNRENTABLE * 60);
    }

    return (PK_OK);
}

int inc_pk_thiefs(struct char_data *ch, struct char_data *victim)
{

    log("ВОРУЕТ: Вор %s : Жертва %s", GET_NAME(ch), GET_NAME(victim));

    if (PRF_FLAGGED(ch, PRF_NOHASSLE))
        return (PK_OK);

    if (ch->pk_list.count(GET_ID(victim)))
        return (PK_OK);

    if (!victim->pk_list.count(GET_ID(ch))) {
        /* if (!IS_NPC(ch) && !IS_NPC(victim))
           {
           act("Вы получили право отомстить $n2!", FALSE, ch, 0, victim, TO_VICT);
           act("$N получил$G право отомстить Вам!", FALSE, ch, 0, victim, TO_CHAR);
           } */
    }

    victim->pk_list[GET_ID(ch)] = THIEF_UNRENTABLE * SECS_PER_MUD_TICK;
    /* XXX why RENTABLE not updated here as in inc_pk_values above? */

    return (PK_OK);
}

int KILLER(struct char_data *killer)
{
    struct char_data *tch;

    if (!FIGHTING(killer))
        return (0);

    for (tch = character_list; tch; tch = tch->next) {
        if (FIGHTING(killer) != tch)
            continue;

        if (tch->pk_list.count(GET_ID(killer)))
            return (1);
    }

    return (0);
}

ACMD(do_revenge)
{
    struct char_data *tch;

    PK_Memory_type::iterator pk;
    int found = FALSE;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];
    int time;

    if (IS_NPC(ch))
        return;

    one_argument(argument, arg);

    sprintf(buf, "Вы можете отомстить:\r\n");
    for (tch = character_list; tch; tch = tch->next) {
        if (IS_NPC(tch))
            continue;

        if (!ch->pk_list.count(GET_ID(tch)))
            continue;

        time = ch->pk_list[GET_ID(tch)];
        sprintf(buf + strlen(buf), "%-25s осталось времени: %d\r\n", GET_NAME(tch),
                (time / SECS_PER_MUD_TICK) + 1);
        found = TRUE;
    }

    if (!found)
        sprintf(buf, "Вам некому мстить.\r\n");

    send_to_char(buf, ch);

}


int may_pkill(struct char_data *revenger, struct char_data *killer)
{
    if (revenger->pk_list.count(GET_ID(killer)))
        return (PC_REVENGE_PC);

    return (PC_KILL_PC);
}


int inc_pkill(struct char_data *victim, struct char_data *killer, int pkills, int prevenge)
{
    if (victim == killer)
        return (0);

    /* act("$N получает флаг на $n",FALSE,killer,0,victim,TO_NOTVICT);
       act("$N получает флаг на Вас",FALSE,killer,0,victim,TO_CHAR);
       act("Вы получает флаг на $n",FALSE,killer,0,victim,TO_VICT); */

    inc_pk_values(killer, victim, pkills, 0);

    return (1);
}

int dec_pkill(struct char_data *victim, struct char_data *killer)
{
    int result;

    if (victim == killer)
        return (0);

    if (IS_NPC(killer)) {
        if (!AFF_FLAGGED(killer, AFF_CHARM))
            return (0);
        else if (!(killer = killer->party_leader) || IS_NPC(killer))
            return (0);
    }

    if (IS_NPC(victim)) {
        if (!AFF_FLAGGED(victim, AFF_CHARM) && !IS_HORSE(victim))
            return (0);
        else if (!(victim = victim->party_leader) || IS_NPC(victim))
            return (0);
    }

    if (victim == killer || ROOM_FLAGGED(IN_ROOM(victim), ROOM_ARENA))
        return (0);

    if (may_pkill(victim, killer) != PC_REVENGE_PC)
        return (0);

    result = dec_pk_values(killer, victim, 1, 0);
    save_char(killer, NOWHERE);
    return (1);
}

void inc_pkill_group(struct char_data *victim, struct char_data *killer, int pkills, int prevenge)
{
    struct char_data *tch = NULL;
    struct follow_type *f;

    if (same_group(victim, killer))
        return;


    if (IN_ROOM(victim) != NOWHERE && IN_ROOM(killer) != NOWHERE
        && (ROOM_FLAGGED(IN_ROOM(victim), ROOM_ARENA) || ROOM_FLAGGED(IN_ROOM(killer), ROOM_ARENA)))
        return;
    inc_pkill(victim, killer, pkills, prevenge);

//Флаг для последователей жертвы
    tch = victim->master ? victim->master : victim;
    inc_pkill(victim, killer, pkills, prevenge);
    for (f = tch->followers; f; f = f->next)
        if (f->follower && IS_AFFECTED(f->follower, AFF_CHARM))
            inc_pkill(f->follower, killer, pkills, prevenge);

//Флаг для группы жертвы
    tch = victim->party_leader ? victim->party_leader : victim;
    inc_pkill(victim, killer, pkills, prevenge);
    for (f = tch->party; f; f = f->next)
        if (f->follower && IS_AFFECTED(f->follower, AFF_GROUP))
            inc_pkill(f->follower, killer, pkills, prevenge);

    if (!IS_AFFECTED(killer, AFF_GROUP))
        return;

// Флаг на хозяина чармиса
    if (IS_NPC(killer)) {
        if ((AFF_FLAGGED(killer, AFF_CHARM) || IS_HORSE(killer)) && killer->master) {
            inc_pkill(victim, killer->master, pkills, prevenge);
            tch = killer->master;

            // Флаг на группу хозяина чармиса
            if (AFF_FLAGGED(tch, AFF_GROUP)) {
                tch = tch->party_leader;
                inc_pkill(victim, tch, pkills, prevenge);
                for (f = tch->party; f; f = f->next)
                    if (f->follower && IS_AFFECTED(f->follower, AFF_GROUP) && !IS_NPC(f->follower))
                        inc_pkill(victim, f->follower, pkills, prevenge);
            }
        }
        return;
    }
// Флаг на последователей
    tch = killer->master ? killer->master : killer;
    inc_pkill(victim, tch, pkills, prevenge);
    for (f = tch->followers; f; f = f->next)
        if (f->follower && IS_AFFECTED(f->follower, AFF_CHARM) && !IS_NPC(f->follower))
            inc_pkill(victim, f->follower, pkills, prevenge);

// Флаг на группу
    tch = killer->party_leader ? killer->party_leader : killer;
    inc_pkill(victim, tch, pkills, prevenge);
    for (f = tch->party; f; f = f->next)
        if (f->follower && IS_AFFECTED(f->follower, AFF_GROUP) && !IS_NPC(f->follower))
            inc_pkill(victim, f->follower, pkills, prevenge);
}

int dec_pkill_group(struct char_data *victim, struct char_data *killer)
{
    struct char_data *tch;
    struct follow_type *f;

    if (same_group(victim, killer))
        return (0);

    /* revenge victim */
    if (dec_pkill(victim, killer))
        return (1);

    /* revenge nothing */
    if (IS_NPC(victim)) {
        if (victim->master && !IS_NPC(victim->master))
            victim = victim->master;
        else
            return (0);
    }
    if (!AFF_FLAGGED(victim, AFF_GROUP))
        return (0);

    /* revenge group-members   */
    tch = victim->party_leader ? victim->party_leader : victim;
    for (f = tch->party; f; f = f->next) {
        if (f->follower && AFF_FLAGGED(f->follower, AFF_GROUP) && FIGHTING(f->follower) == killer)
            if (dec_pkill(f->follower, killer))
                return (1);
    };

    /* revenge nothing */
    return (0);
}

/* Печать списка пк */
void pk_list_sprintf(struct char_data *ch, char *buff)
{
    PK_Memory_type::iterator pk;
    const char *name;

    *buff = '\0';
    strcat(buff, "ПК список:\r\n");
    for (pk = ch->pk_list.begin(); pk != ch->pk_list.end(); pk++) {
        name = get_name_by_id(pk->first);

        sprintf(buff + strlen(buff), "Имя: %-25s Время осталось: [&G%d&n]\r\n",
                name ? name : "(не найдено)", pk->second);
    }
}

/* ************************************************************************
*   File: mobmax.c                                      Part of CircleMUD *
*  Usage: Расчет замакса по мобам                                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "db.h"

#include "mobmax.h"

#define MIN_MOB_IN_MOBKILL 2    /* Минимальное количество мобов одного уровня */
                               /*  в файле замакса */
#define MAX_MOB_IN_MOBKILL 100  /* Максимальное количество мобов одного уровня */
                               /*  в файле замакса */
#define MOBKILL_KOEFF 0.5       /* Во сколько раз надо убить мобов меньше */
                                 /* чем  их есть в мире, чтобы начать */
                                 /*  размаксивать */


/* Размакс для мобов уровня level */
void drop_old_mobs(struct char_data *ch, int level)
{
    int i, lev_count, r_num;

    if (level > MAX_MOB_LEVEL || level < 0)
        return;

    i = ch->MobKill.count - 1;
    lev_count = 0;
    while (i >= 0) {
        r_num = real_mobile(ch->MobKill.vnum[i]);
        if (r_num < 0 || (ch->MobKill.howmany[i]) < 1) {
            clear_kill_vnum(ch, (ch->MobKill.vnum[i]));
        } else if (GET_LEVEL(mob_proto + r_num) == level) {
            if (lev_count > num_mob_lev[level]) {
                clear_kill_vnum(ch, (ch->MobKill.vnum[i]));
            } else {
                lev_count++;
            }
        }
        i--;
    }
}


/* снимаем замакс по мобу vnum. возвращает true если сняли замакс и false
   если его и не было */
int clear_kill_vnum(struct char_data *vict, int vnum)
{
    int i, j;

    for (i = j = 0; j < vict->MobKill.count; i++, j++) {
        if (vict->MobKill.vnum[i] == vnum)
            j++;
        vict->MobKill.vnum[i] = vict->MobKill.vnum[j];
        vict->MobKill.howmany[i] = vict->MobKill.howmany[j];
    }
    if (j > i) {
        vict->MobKill.count--;
        return (TRUE);
    } else {
        return (FALSE);
    }
}

/* Увеличиваем количество убитых vnum мобов на incvalue */
void inc_kill_vnum(struct char_data *ch, int vnum, int incvalue)
{

    return;

    int i;

    if (IS_NPC(ch) || IS_IMMORTAL(ch) || vnum < 0)
        return;

    if (ch->MobKill.vnum) {
        for (i = 0; i < ch->MobKill.count; i++)
            if (*(ch->MobKill.vnum + i) == vnum) {
                *(ch->MobKill.howmany + i) += incvalue;
                return;
            }
        if (!(ch->MobKill.count % 10L)) {
            RECREATE(ch->MobKill.howmany, int, (ch->MobKill.count / 10L + 1) * 10L);
            RECREATE(ch->MobKill.vnum, int, (ch->MobKill.count / 10L + 1) * 10L);
        }
    } else {
        ch->MobKill.count = 0;
        CREATE(ch->MobKill.vnum, int, 10);
        CREATE(ch->MobKill.howmany, int, 10);
    }

    *(ch->MobKill.vnum + ch->MobKill.count) = vnum;
    *(ch->MobKill.howmany + ch->MobKill.count++) = incvalue;

    /* Проводим размакс */
    i = real_mobile(vnum);
    if (i >= 0) {
        drop_old_mobs(ch, GET_LEVEL(mob_proto + i));
    }
}

/* возвращет количество убитых vnum мобов */
int get_kill_vnum(struct char_data *ch, int vnum)
{
    int i;

    if (IS_NPC(ch) || IS_IMMORTAL(ch) || vnum < 0)
        return (0);
    if (ch->MobKill.vnum) {
        for (i = 0; i < ch->MobKill.count; i++)
            if (*(ch->MobKill.vnum + i) == vnum)
                return (*(ch->MobKill.howmany + i));
    }
    return (0);
}

/* сохраняет в аски файле замакс */
void save_mkill(struct char_data *ch, FILE * saved)
{
    int i;
    mob_rnum r_num;

    /*  if (IS_NPC(ch) || IS_IMMORTAL(ch))
       return; */
    if (IS_NPC(ch))
        return;

    if (ch->MobKill.vnum) {
        for (i = 0; i < ch->MobKill.count; i++)
            if ((r_num = real_mobile(*(ch->MobKill.vnum + i))) > -1) {
                fprintf(saved, "Mob : %d %d %s\n", *(ch->MobKill.vnum + i),
                        *(ch->MobKill.howmany + i), mob_proto[r_num].player.short_descr);
            }
    }

    if (ch->ScriptExp.vnum) {
        for (i = 0; i < ch->ScriptExp.count; i++)
            if ((r_num = real_mobile(*(ch->ScriptExp.vnum + i))) > -1) {
                fprintf(saved, "SExp: %d %d\n", *(ch->ScriptExp.vnum + i),
                        *(ch->ScriptExp.howmany + i));
            }
    }

}



/* снимает замакс или освобждает память под него */
void free_mkill(struct char_data *ch)
{
    if (ch->MobKill.howmany)
        free(ch->MobKill.howmany);

    if (ch->MobKill.vnum)
        free(ch->MobKill.vnum);

    ch->MobKill.count = 0;
    ch->MobKill.vnum = NULL;
    ch->MobKill.howmany = NULL;

    //
    if (ch->ScriptExp.howmany)
        free(ch->ScriptExp.howmany);

    if (ch->ScriptExp.vnum)
        free(ch->ScriptExp.vnum);

    ch->ScriptExp.count = 0;
    ch->ScriptExp.vnum = NULL;
    ch->ScriptExp.howmany = NULL;

}

/* пересчет количества типов мобов каждого уровня в мире и вычмсление
   максимального их количества в файле замакса */
void mob_lev_count(void)
{
    int nr;

    for (nr = 0; nr <= MAX_MOB_LEVEL; nr++)
        num_mob_lev[nr] = 0;

    for (nr = 0; nr <= top_of_mobt; nr++) {
        if (GET_LEVEL(mob_proto + nr) > MAX_MOB_LEVEL)
            log("Внимание! Монстр > Максимального уровня!");
        else if (GET_LEVEL(mob_proto + nr) < 0)
            log("Внимание! Монстр < Минимального уровня!");
        else
            num_mob_lev[(int) GET_LEVEL(mob_proto + nr)]++;
    }

    for (nr = 0; nr <= MAX_MOB_LEVEL; nr++) {
        //log("Монстр уровень %d. Количество %d", nr, num_mob_lev[nr]);
        num_mob_lev[nr] = (int) (num_mob_lev[nr] / MOBKILL_KOEFF);
        if (num_mob_lev[nr] < MIN_MOB_IN_MOBKILL)
            num_mob_lev[nr] = MIN_MOB_IN_MOBKILL;
        if (num_mob_lev[nr] > MAX_MOB_IN_MOBKILL)
            num_mob_lev[nr] = MAX_MOB_IN_MOBKILL;
        //log("Монстр уровень %d. Замаксено %d", nr, num_mob_lev[nr]);

    }
}

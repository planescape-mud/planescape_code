/* ************************************************************************
*   File: handler.c                                     Part of CircleMUD *
*  Usage: internal funcs: moving and finding chars/objs                   *
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
#include "constants.h"
#include "utils.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "interpreter.h"
#include "spells.h"
#include "xspells.h"
#include "screen.h"
#include "events.h"
#include "case.h"
#include "xquests.h"
#include "xboot.h"

/* external functions */
ACMD(do_return);

/* local functions */
void update_object(struct obj_data *obj, int use);
void update_char_objects(struct char_data *ch);
void unsset_affects(struct char_data *ch, struct set_variante_data *vrnt);
void sset_affects(struct char_data *ch, struct set_variante_data *vrnt);

#define GET_ALIAS(mob) ((mob)->player.name)
#define GET_SDESC(mob) ((mob)->player.short_descr)
#define GET_LDESC(mob) ((mob)->player.long_descr)
#define GET_DDESC(mob) ((mob)->player.description)

char *fname(const char *namelist)
{
    static char holder[30];
    register char *point;

    for (point = holder; a_isalpha(*namelist); namelist++, point++)
        *point = *namelist;

    *point = '\0';

    return (holder);
}


int isfullname(const char *str, const char *namelist)
{
    int j, fnd;
    char tname[256];
    const char *curname, *curstr;

    if (!namelist)
        return 0;

    fnd = FALSE;
    for (j = 0; j < NUM_PADS; j++) {
        strcpy(tname, get_name_pad((char *) namelist, j));

        //curname = namelist;
        curname = tname;
        for (;;) {
            for (curstr = str;; curstr++, curname++) {
                if (!*curstr && !a_isalpha(*curname))
                    return (1);

                if (!*curname)
                    break;
                //return (0);

                if (!*curstr || *curname == ' ')
                    break;

                if (LOWER(*curstr) != LOWER(*curname))
                    break;
            }

            /* skip to next name */

            for (; a_isalpha(*curname); curname++);
            if (!*curname)
                break;
            //return (0);
            curname++;          /* first char of new name */
        }
    }
    return (0);
}

int isname(const char *str, const char *namelist)
{
    int once_ok = FALSE;
    const char *curname, *curstr, *laststr;

    if (!namelist || !*namelist)
        return (FALSE);

    curname = namelist;
    curstr = laststr = str;
    for (;;) {
        once_ok = FALSE;
        for (;; curstr++, curname++) {
            if (!*curstr)
                return (once_ok);
            if (curstr != laststr && *curstr == '!')
                if (a_isalnum(*curname)) {
                    curstr = laststr;
                    break;
                }
            if (!a_isalnum(*curstr)) {
                for (; !a_isalnum(*curstr); curstr++) {
                    if (!*curstr)
                        return (once_ok);
                }
                laststr = curstr;
                break;
            }
            if (!*curname)
                return (FALSE);
            if (!a_isalnum(*curname)) {
                curstr = laststr;
                break;
            }
            if (LOWER(*curstr) != LOWER(*curname)) {
                curstr = laststr;
                break;
            } else
                once_ok = TRUE;
        }
        /* skip to next name */
        for (; a_isalnum(*curname); curname++);
        for (; !a_isalnum(*curname); curname++) {
            if (!*curname)
                return (FALSE);
        }
    }
}

void check_light(struct char_data *ch,
                 int was_equip, int was_single, int was_holylight, int was_holydark, int koef)
{
    int light_equip = FALSE, i;

    if (!IS_NPC(ch) && !ch->desc)
        return;

    if (IN_ROOM(ch) == NOWHERE)
        return;

    for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i)) {
            if ((GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_LIGHT && GET_LIGHT_VAL(GET_EQ(ch, i)) && GET_LIGHT_ON(GET_EQ(ch, i))) || (GET_OBJ_TYPE(GET_EQ(ch, i)) != ITEM_LIGHT && GET_LIGHT_VAL(GET_EQ(ch, i)))) {    //send_to_char("Light OK!\r\n",ch);
                light_equip = TRUE;
            }
        }
    // In equipment
    if (light_equip) {
        if (was_equip == LIGHT_NO)
            world[ch->in_room].light = MAX(0, world[ch->in_room].light + koef);
    } else {
        if (was_equip == LIGHT_YES)
            world[ch->in_room].light = MAX(0, world[ch->in_room].light - koef);
    }
    // Singleligt affect
    if (AFF_FLAGGED(ch, AFF_SINGLELIGHT)) {
        if (was_single == LIGHT_NO)
            world[ch->in_room].light = MAX(0, world[ch->in_room].light + koef);
    } else {
        if (was_single == LIGHT_YES)
            world[ch->in_room].light = MAX(0, world[ch->in_room].light - koef);
    }
    // Holyligh affect
    if (AFF_FLAGGED(ch, AFF_HOLYLIGHT)) {
        if (was_holylight == LIGHT_NO)
            world[ch->in_room].glight = MAX(0, world[ch->in_room].glight + koef);
    } else {
        if (was_holylight == LIGHT_YES)
            world[ch->in_room].glight = MAX(0, world[ch->in_room].glight - koef);
    }
    // Holydark affect
    // if (IS_IMMORTAL(ch))
    //    {sprintf(buf,"holydark was %d\r\n",was_holydark);
    //     send_to_char(buf,ch);
    //    }
    if (AFF_FLAGGED(ch, AFF_HOLYDARK)) {        // if (IS_IMMORTAL(ch))
        //    send_to_char("holydark on\r\n",ch);
        if (was_holydark == LIGHT_NO)
            world[ch->in_room].gdark = MAX(0, world[ch->in_room].gdark + koef);
    } else {                    // if (IS_IMMORTAL(ch))
        //   send_to_char("HOLYDARK OFF\r\n",ch);
        if (was_holydark == LIGHT_YES)
            world[ch->in_room].gdark = MAX(0, world[ch->in_room].gdark - koef);
    }
    //if (IS_IMMORTAL(ch))
    //   {sprintf(buf,"%d %d %d (%d)\r\n",world[IN_ROOM(ch)].light,world[IN_ROOM(ch)].glight,world[IN_ROOM(ch)].gdark,koef);
    //    send_to_char(buf,ch);
    //   }
}

void skill_modify(struct char_data *ch, int loc, int mod)
{
    if (loc)
        GET_SKILL_ADD(ch, loc) += mod;
}

void affect_modify(struct char_data *ch, byte loc, int mod, bitvector_t bitv, bool add)
{
    if (add) {
        SET_BIT(AFF_FLAGS(ch, bitv), bitv);
    } else {
        REMOVE_BIT(AFF_FLAGS(ch, bitv), bitv);
        mod = -mod;
    }
    switch (loc) {
        case APPLY_NONE:
            break;
        case APPLY_STR:
            GET_STR_ADD(ch) += mod;
            break;
        case APPLY_DEX:
            GET_DEX_ADD(ch) += mod;
            break;
        case APPLY_INT:
            GET_INT_ADD(ch) += mod;
            break;
        case APPLY_WIS:
            GET_WIS_ADD(ch) += mod;
            break;
        case APPLY_CON:
            GET_CON_ADD(ch) += mod;
            break;
        case APPLY_CHA:
            GET_CHA_ADD(ch) += mod;
            break;
        case APPLY_LACKY:
            GET_LCK_ADD(ch) += mod;
            break;
        case APPLY_CLASS:
            break;
        case APPLY_POWER:
            GET_POWER_ADD(ch) += mod;
            break;

            /*
             * My personal thoughts on these two would be to set the person to the
             * value of the apply.  That way you won't have to worry about people
             * making +1 level things to be imp (you restrict anything that gives
             * immortal level of course).  It also makes more sense to set someone
             * to a class rather than adding to the class number. -gg
             */

        case APPLY_LEVEL:
            GET_LEVEL_ADD(ch) -= mod;
            break;
        case APPLY_AGE:
            GET_AGE_ADD(ch) += mod;
            break;
        case APPLY_CHAR_WEIGHT:
            GET_WEIGHT_ADD(ch) += mod;
            break;
        case APPLY_CHAR_HEIGHT:
            GET_HEIGHT_ADD(ch) += mod;
            break;
        case APPLY_MANAREG:
            GET_MANAREG(ch) += mod;
            break;
        case APPLY_HIT:
            GET_HIT_ADD(ch) += mod;
            break;
        case APPLY_MOVE:
            GET_MOVE_ADD(ch) += mod;
            break;
        case APPLY_GOLD:
            break;
        case APPLY_EXP:
            break;
        case APPLY_AC:
            GET_AC_ADD(ch) += mod;
            break;
        case APPLY_HITROLL:
            GET_HR_ADD(ch) += mod;
            break;
        case APPLY_DAMROLL:
            GET_DR_ADD(ch) += mod;
            break;
        case APPLY_HITREG:
            GET_HITREG(ch) += mod;
            break;
        case APPLY_MOVEREG:
            GET_MOVEREG(ch) += mod;
            break;
        case APPLY_SIZE:
            GET_SIZE_ADD(ch) += mod;
            break;
        case APPLY_ARMOUR0:
            GET_ARMOUR(ch, 0) += mod;
            break;
        case APPLY_ARMOUR1:
            GET_ARMOUR(ch, 1) += mod;
            break;
        case APPLY_ARMOUR2:
            GET_ARMOUR(ch, 2) += mod;
            break;
        case APPLY_POISON:
            GET_POISON(ch) += mod;
            break;
        case APPLY_CAST_SUCCESS:
            GET_CAST_SUCCESS(ch) += mod;
            break;
        case APPLY_REMEMORY:
            GET_REMEMORY(ch) += mod;
            break;
        case APPLY_SPEED:
            GET_SPEED_ADD(ch) += mod;
            break;
            // saving 3ed
        case APPLY_SAVING_REFL:
            GET_SAVE3(ch, SAV_REFL) += mod;
            break;
        case APPLY_SAVING_FORT:
            GET_SAVE3(ch, SAV_FORT) += mod;
            break;
        case APPLY_SAVING_WILL:
            GET_SAVE3(ch, SAV_WILL) += mod;
            break;
        case APPLY_SAVING_FIRE:
            GET_SAVE3(ch, SAV_FIRE) += mod;
            break;
        case APPLY_SAVING_COLD:
            GET_SAVE3(ch, SAV_COLD) += mod;
            break;
        case APPLY_SAVING_ELECTRO:
            GET_SAVE3(ch, SAV_ELECTRO) += mod;
            break;
        case APPLY_SAVING_ACID:
            GET_SAVE3(ch, SAV_ACID) += mod;
            break;
        case APPLY_SAVING_NEGATIVE:
            GET_SAVE3(ch, SAV_NEGATIVE) += mod;
            break;
        case APPLY_SAVING_POSITIVE:
            GET_SAVE3(ch, SAV_POSITIVE) += mod;
            break;
        case APPLY_SAVING_XAOS:
            GET_SAVE3(ch, SAV_XAOS) += mod;
            break;
        case APPLY_SAVING_ORDER:
            GET_SAVE3(ch, SAV_ORDER) += mod;
            break;
        case APPLY_SAVING_POISON:
            GET_SAVE3(ch, SAV_POISON) += mod;
            break;
        case APPLY_MANA:
            GET_MANA_ADD(ch) += mod;
            break;
        case APPLY_DB:
            break;
            //увеличение вреда
        case APPLY_INC_HEAL:   //исцеляющая магия
            GET_INC_MAGIC(ch, 0) += mod;
            break;
        case APPLY_INC_MAGIC:  //вся магия
            GET_INC_MAGIC(ch, 1) += mod;
            break;
        case APPLY_INC_FIRE:   //только огонь
            GET_INC_MAGIC(ch, 2) += mod;
            break;
        case APPLY_INC_COLD:   //только холод
            GET_INC_MAGIC(ch, 3) += mod;
            break;
        case APPLY_INC_ELECTRO:        //только электро
            GET_INC_MAGIC(ch, 4) += mod;
            break;
        case APPLY_INC_ACID:   //только кислота
            GET_INC_MAGIC(ch, 5) += mod;
            break;
        case APPLY_INC_NEGATIVE:       //только негатив
            GET_INC_MAGIC(ch, 6) += mod;
            break;
        case APPLY_INC_POSITIVE:       //только позитив
            GET_INC_MAGIC(ch, 7) += mod;
            break;

        default:
            log("SYSERR: Неизвестный модификатор %d для %s в (%s, affect_modify).", loc,
                GET_NAME(ch), __FILE__);
            break;

    }                           /* switch */
}



int char_saved_aff[] = { AFF_GROUP,
    AFF_HORSE,
    AFF_ORENT,
    0
};

int char_stealth_aff[] = { AFF_HIDE,
    AFF_SNEAK,
    AFF_CAMOUFLAGE,
    0
};

/* Перерасчет параметров персонажа */
void affect_total(struct char_data *ch)
{
    struct affected_type *af;
    struct C_obj_affected_type *af_obj;
    struct extra_affects_type *extra_affect = NULL;
    struct obj_affected_type *extra_modifier = NULL;
    struct skill_modify_type *skill_modifier = NULL;
    int nomagic = 0, icls, i, j, age;
    bool isdead = FALSE;

//send_to_charf(ch,"[ *** Affect total *** ]\r\n");
    /*Определение уровня "поле антимагии" */
    if (IN_ROOM(ch) != NOWHERE)
        nomagic = affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_NOMAGIC);
    /*Очистка временных параметров персонажа */
    memset((char *) &ch->add_abils, 0, sizeof(struct char_played_ability_data));
    for (icls = 0; icls < NUM_CLASSES; icls++)
        ch->add_classes[icls] = 0;

//Уставнока наборов предмета
    if (ch->set_message_num != -1 && ch->set_message_var != -1) {
        std::vector < struct set_variante_data >vc = *set_table[ch->set_message_num].variante;
        struct set_variante_data vrnt = vc[ch->set_message_var];

        if (ch->set_change) {   //Флаг вывод сообщения об изменениях набора
            if (ch->set_change == 1) {  //Начало
                if (vrnt.start_to_char)
                    send_to_charf(ch, "&W%s&n\r\n", vrnt.start_to_char);
                if (vrnt.start_to_room)
                    act(vrnt.start_to_room, "Км", ch);
                sset_affects(ch, &vrnt);
            } else if (ch->set_change == -1) {  //Конец
                if (vrnt.stop_to_char)
                    send_to_charf(ch, "&W%s&n\r\n", vrnt.stop_to_char);
                if (vrnt.stop_to_room)
                    act(vrnt.stop_to_room, "Км", ch);
                unsset_affects(ch, &vrnt);
            }

            ch->set_change = 0;
            ch->set_message_var = -1;
            ch->set_message_num = -1;
        }
    }

    /*Установка эффектов на персонаже */
    for (af = ch->affected; af; af = af->next) {
        if (af->location <= 0)
            continue;
        //if (nomagic < af->level) //если уровнь эффекта больше уровня антимагии, то все ок.
        affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
    }
    /* Модификаторы от татуировок */
    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_TATOO(ch, i)) {
            /* Бонус умения от татуировки */
            for (j = 0; j < MAX_SKILLS; j++)
                skill_modify(ch, j, GET_TATOO(ch, i)->skill_modify[j]);
            /* Эффекты от татуировки */
            for (af_obj = GET_TATOO(ch, i)->C_affected; af_obj; af_obj = af_obj->next)
                affect_modify(ch, af_obj->location, af_obj->modifier, af_obj->bitvector, TRUE);
        }
    }
    /* Модификаторы от экипировки */
    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i)) {
            /* Добавление модификторов скиллов */
            for (j = 0; j < MAX_SKILLS; j++)
                skill_modify(ch, j, GET_EQ(ch, i)->skill_modify[j]);
            /* Обновление модификаторов предмета */
            for (af_obj = GET_EQ(ch, i)->C_affected; af_obj; af_obj = af_obj->next)
                affect_modify(ch, af_obj->location, af_obj->modifier, af_obj->bitvector, TRUE);
        }
    }

    /* Модифаторы негативных уровней */
    if (GET_LEVEL_ADD(ch) < 0) {
        //log("Негативные уровни у %s (%d)",GET_NAME(ch),GET_LEVEL_ADD(ch));
        for (icls = 0; icls > GET_LEVEL_ADD(ch); icls--) {
            //log("Обратока %d Результат %d",icls,GET_LEVEL(ch) + icls);
            if ((GET_LEVEL(ch) + icls) <= 1) {
                //log("Достигли конца %d",icls);
                break;          //Ниже 1го уровня опустить нельзя
            }
            ch->add_classes[(int) GET_HLEVEL(ch, GET_LEVEL(ch) + icls)]--;
        }
    }
    /* Инициализация уровней */
    for (icls = 0; icls < NUM_CLASSES; icls++) {
        ch->classes[icls] = ch->init_classes[icls] + ch->add_classes[icls];
        GET_FACT_LEVEL(ch) += ch->classes[icls];
    }

    if (GET_FACT_LEVEL(ch) <= 0)
        isdead = TRUE;

    /*Установка характеристик у персонажа */
// if (!IS_NPC(ch) && GET_LEVEL(ch) >= 3)
    if (GET_LEVEL(ch) >= 3) {
        for (icls = 0; icls < NUM_CLASSES; icls++)
            if (ch->classes[icls]) {
                GET_STR_ROLL(ch) += (int) (add_classes[icls][0] * ch->classes[icls]);
                GET_CON_ROLL(ch) += (int) (add_classes[icls][1] * ch->classes[icls]);
                GET_DEX_ROLL(ch) += (int) (add_classes[icls][2] * ch->classes[icls]);
                GET_INT_ROLL(ch) += (int) (add_classes[icls][3] * ch->classes[icls]);
                GET_WIS_ROLL(ch) += (int) (add_classes[icls][4] * ch->classes[icls]);
                GET_CHA_ROLL(ch) += (int) (add_classes[icls][5] * ch->classes[icls]);
            }
    }
    /*Штрафы и бонусы за возраст */
    if (!IS_NPC(ch)) {
        age = GET_REAL_AGE(ch);
        if (age > 20 && age <= 30) {
            GET_STR_ADD(ch) += 1;
            GET_CON_ADD(ch) += 1;
        } else if (age > 30 && age <= 45) {
            GET_WIS_ADD(ch) += 1;
            GET_DEX_ADD(ch) -= 1;
        } else if (age > 45 && age <= 60) {
            GET_DEX_ADD(ch) -= 1;
            GET_STR_ADD(ch) -= 1;
            GET_WIS_ADD(ch) += 1;
            GET_INT_ADD(ch) += 1;
        } else if (age > 60 && age <= 80) {
            GET_DEX_ADD(ch) -= 2;
            GET_STR_ADD(ch) -= 1;
            GET_CON_ADD(ch) -= 1;
            GET_WIS_ADD(ch) += 2;
            GET_INT_ADD(ch) += 1;
        } else if (age > 80 && age <= 110) {
            GET_DEX_ADD(ch) -= 2;
            GET_STR_ADD(ch) -= 2;
            GET_CON_ADD(ch) -= 2;
            GET_WIS_ADD(ch) += 2;
            GET_INT_ADD(ch) += 2;
        } else if (age > 110) {
            GET_DEX_ADD(ch) -= 3;
            GET_STR_ADD(ch) -= 3;
            GET_CON_ADD(ch) -= 3;
            GET_WIS_ADD(ch) += 3;
            GET_INT_ADD(ch) += 3;
        }
    }

    /* Лимитация умений */
    /* for (j=0;j<MAX_SKILLS;j++)
       if (get_skill_class_level(ch,j))
       GET_SKILL(ch,j) = MIN(GET_SKILL(ch,j), calc_need_improove(ch,get_skill_class_level(ch,j))); */

    /* Рассовые бонусы */
    if (GET_RACE(ch) < NUM_RACES) {
        extra_affect = race_app[(int) GET_RACE(ch)].extra_affects;
        extra_modifier = race_app[(int) GET_RACE(ch)].extra_modifiers;
        skill_modifier = race_app[(int) GET_RACE(ch)].extra_skills;
        for (i = 0; extra_affect && (extra_affect + i)->affect != -1; i++)
            affect_modify(ch, APPLY_NONE, 0, (extra_affect + i)->affect,
                          (extra_affect + i)->set_or_clear);

        for (i = 0; extra_modifier && (extra_modifier + i)->location != -1; i++)
            affect_modify(ch, (extra_modifier + i)->location, (extra_modifier + i)->modifier, 0,
                          TRUE);

        for (i = 0; skill_modifier && (skill_modifier + i)->skill_no != -1; i++)
            skill_modify(ch, (skill_modifier + i)->skill_no, (skill_modifier + i)->mod);
    }

    /* if (!IS_NPC(ch) && IN_ROOM(ch) != NOWHERE && ch->desc)
       for (j=0;j<MAX_SKILLS;j++)
       if (get_skill_class_level(ch,j))
       {
       GET_SKILL(ch,j) = MIN(GET_SKILL(ch,j), calc_need_improove(ch,get_skill_class_level(ch,j)));
       } */

    /* Расчет параметров maxhit, damroll, ac */
    recalc_params(ch);

}



/* Insert an affect_type in a char_data structure
   Automatically sets apropriate bits and apply's */
void affect_to_char(struct char_data *ch, struct affected_type *af)
{
    long was_lgt = AFF_FLAGGED(ch, AFF_SINGLELIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hlgt = AFF_FLAGGED(ch, AFF_HOLYLIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hdrk = AFF_FLAGGED(ch, AFF_HOLYDARK) ? LIGHT_YES : LIGHT_NO;
    struct affected_type *affected_alloc;

    CREATE(affected_alloc, struct affected_type, 1);

    *affected_alloc = *af;
    affected_alloc->next = ch->affected;
    ch->affected = affected_alloc;

    affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
    affect_total(ch);
    check_light(ch, LIGHT_UNDEF, was_lgt, was_hlgt, was_hdrk, 1);
}



/*
 * Remove an affected_type structure from a char (called when duration
 * reaches zero). Pointer *af must never be NIL!  Frees mem and calls
 * affect_location_apply
 */

void affect_remove(struct char_data *ch, struct affected_type *af)
{
    int was_lgt = AFF_FLAGGED(ch, AFF_SINGLELIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hlgt = AFF_FLAGGED(ch, AFF_HOLYLIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hdrk = AFF_FLAGGED(ch, AFF_HOLYDARK) ? LIGHT_YES : LIGHT_NO;

    struct affected_type *temp;
    int change = 0;

    // if (IS_IMMORTAL(ch))
    //   {sprintf(buf,"<%d>\r\n",was_hdrk);
    //    send_to_char(buf,ch);
    //   }

    if (ch->affected == NULL) {
        log("SYSERR: affect_remove(%s) when no affects...", GET_NAME(ch));
        // core_dump();
        return;
    }

    affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);
    if (af->type == find_spell_num(SPELL_SANCTUARY) && !ch->not_attack.empty()) {
        ch->not_attack.clear();
        ch->may_attack.clear();
    }

    if (af->type == find_spell_num(SPELL_REPULSION) && !ch->not_moved.empty()) {
        ch->not_moved.clear();
        ch->may_moved.clear();
    }

    if (af->type == find_spell_num(SPELL_HIDE) && !ch->visible_by.empty())
        ch->visible_by.clear();

    if (af->type == find_spell_num(SPELL_PREPLAGUE) && (!IS_SOUL(ch) && GET_POS(ch) > POS_DEAD))
        plague_to_char(ch, ch, af->modifier);

    if (af->type == find_spell_num(SPELL_MAKE_GHOLA_S1) && af->main)
        make_ghola(ch, 2, af->level);
    if (af->type == find_spell_num(SPELL_MAKE_GHOLA_S2) && af->main)
        make_ghola(ch, 3, af->level);
    if (af->type == find_spell_num(SPELL_MAKE_GHOLA_S3) && af->main)
        make_ghola(ch, 4, af->level);
    if (af->type == find_spell_num(SPELL_MAKE_GHOLA_S4) && af->main)
        make_ghola(ch, 5, af->level);

    if (af->type == find_spell_num(SPELL_PLAGUE))
        immplague_to_char(ch);

    if (af->bitvector == AFF_BONES_PICK && (!IS_UNDEAD(ch) || !IS_AFFECTED(ch, AFF_IS_UNDEAD))) {
        act_affect_mess(find_spell_num(SPELL_BONES_PICK), ch, 0, TRUE, TYPE_MESS_KILL);
        GET_HIT(ch) = -(GET_REAL_MAX_HIT(ch) / 10);
        update_pos(ch);
    }

    if (af->bitvector == AFF_MEDITATION) {
        //send_to_charf(ch,"Вы полностью восстановили ману и прекратили свой медитативный сон.\r\n");
        act("$n проснул$u и сел$g отдыхать.", FALSE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
    }

    if ((af->bitvector == AFF_FLY || af->bitvector == AFF_LEVIT) && GET_POS(ch) == POS_FLYING)
        GET_POS(ch) = POS_STANDING;

    if (change)
        affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);
    else {
        REMOVE_FROM_LIST(af, ch->affected, next);
        free(af);
    }
    //log("[AFFECT_REMOVE->AFFECT_TOTAL] Start");
    affect_total(ch);
    //log("[AFFECT_TO_CHAR->AFFECT_TOTAL] Stop");
    check_light(ch, LIGHT_UNDEF, was_lgt, was_hlgt, was_hdrk, 1);
}



/* Call affect_remove with every spell of spelltype "skill" */
void affect_from_char(struct char_data *ch, int type)
{
    struct affected_type *hjp, *next;

    type = find_spell_num(type);

    for (hjp = ch->affected; hjp; hjp = next) {
        next = hjp->next;
        if (hjp->type == type) {
            if (hjp->main)
                show_spell_off(hjp->type, ch, NULL);
            affect_remove(ch, hjp);
        }
    }

    if (IS_NPC(ch) && type == SPELL_CHARM)
        EXTRACT_TIMER(ch) = 5;
}

/*
 * Return TRUE if a char is affected by a spell (SPELL_XXX),
 * FALSE indicates not affected.
 */
int affected_by_spell(struct obj_data *obj, int type)
{
    type = find_spell_num(type);
    return (affected_by_spell_real(obj, type));
}

int affected_by_spell_real(struct obj_data *obj, int type)
{
    struct C_obj_affected_type *hjp, *result = NULL;
    int min = -1;

    for (hjp = obj->C_affected; hjp; hjp = hjp->next) {
        if (hjp->type == type && hjp->main) {
            if (hjp->modifier > min)
                result = hjp;
        }
    }

    return (result ? result->modifier : 0);
}

int affected_by_spell(struct char_data *ch, int type)
{
    type = find_spell_num(type);
    return (affected_by_spell_real(ch, type));
}

int affected_by_spell_real(struct char_data *ch, int type)
{
    struct affected_type *hjp, *result = NULL;
    int min = -1;

    for (hjp = ch->affected; hjp; hjp = hjp->next) {
        if (hjp->type == type && hjp->main) {
            if (hjp->modifier > min)
                result = hjp;
        }
    }

    return (result ? result->modifier : 0);
}

struct affected_type *get_affect_by_vector(struct char_data *ch, int vector)
{
    struct affected_type *hjp, *result = NULL;

//int min = -1;// warning: unused variable

    for (hjp = ch->affected; hjp; hjp = hjp->next) {
        if (hjp->bitvector == vector && hjp->main)
            result = hjp;
    }

    return (result);
}

/* Новая функция affect_join_char()
   Входящий параметры:
   struct char_data *ch - персонаж на которого накладывается эффект
   struct affected_type *aff - структура описания эффекта
   Принцип наложения:
   1.  Проверка такого же эффекта по полю type
   2.  Эффект найден:
   2.1 Если owner одинаковый то обновляем
   2.2 Если owner разный то:
   2.3 Если modifiy больше то обновляем
   2.4 Если нет modify, но есть bitvector
   2.5 Если duration больше, то обновляем
   3.  Эффект не найден, добавляем
   Возвращаемые значения:
   TRUE - Эффект добавлен.
   FALSE - Эффект не добавлен.
*/
int affect_join_char(struct char_data *ch, struct affected_type *aff)
{
    struct affected_type *hjp;
    bool found = FALSE, result = FALSE, abuse = FALSE;

    for (hjp = ch->affected; !found && hjp; hjp = hjp->next) {
        if (hjp->type == aff->type && hjp->location != aff->location) {
            result = FALSE;
            continue;
        }
        if (hjp->type == aff->type && hjp->location == aff->location && !aff->bitvector) {
            if (aff->owner == hjp->owner) {
                hjp->modifier = aff->modifier;
                hjp->duration = aff->duration;
                hjp->owner = aff->owner;
                affect_total(ch);
                found = TRUE;
                result = TRUE;
            } else if (aff->modifier >= hjp->modifier) {
                hjp->modifier = aff->modifier;
                hjp->duration = aff->duration;
                hjp->owner = aff->owner;
                affect_total(ch);
                found = TRUE;
                result = TRUE;
            } else if (aff->modifier < hjp->modifier) {
                found = TRUE;   //что бы не обновляли низкими эффектами
                abuse = TRUE;
            }
        } else if (hjp->type == aff->type && hjp->bitvector == aff->bitvector && !aff->location) {
            if (aff->owner == hjp->owner) {
                hjp->duration = aff->duration;
                hjp->owner = aff->owner;
                affect_total(ch);
                found = TRUE;
                result = TRUE;
            } else if (aff->duration >= hjp->duration) {
                hjp->duration = aff->duration;
                hjp->owner = aff->owner;
                affect_total(ch);
                found = TRUE;
                result = TRUE;
            } else if (aff->duration < hjp->duration) {
                found = TRUE;   //что бы не обновляли низкими эффектами
                abuse = TRUE;
            }
        } else
            if (hjp->type == aff->type && hjp->bitvector == aff->bitvector
                && hjp->location == aff->location) {
            if (aff->owner == hjp->owner) {
                hjp->duration = aff->duration;
                hjp->owner = aff->owner;
                affect_total(ch);
                found = TRUE;
                result = TRUE;
            } else if (aff->duration >= hjp->duration) {
                hjp->duration = aff->duration;
                hjp->owner = aff->owner;
                affect_total(ch);
                found = TRUE;
                result = TRUE;
            } else if (aff->modifier >= hjp->modifier) {
                hjp->modifier = aff->modifier;
                hjp->duration = aff->duration;
                hjp->owner = aff->owner;
                affect_total(ch);
                found = TRUE;
                result = TRUE;
            } else if (aff->duration < hjp->duration || aff->modifier < hjp->modifier) {
                found = TRUE;   //что бы не обновляли низкими эффектами
                abuse = TRUE;
            }
        }
    }

    if (aff->main)
        show_spell_on(aff->type, ch);

    if (!found) {
        affect_to_char(ch, aff);
        result = TRUE;
    }

    if (abuse && !IS_NPC(ch) && check_abuse(aff->type, GET_ID(ch), 1))
        go_jail(NULL, ch, 1440);

    return (result);
}

/* Insert an timed_type in a char_data structure */
void timed_to_char(struct char_data *ch, struct timed_type *timed)
{
    struct timed_type *timed_alloc;

    CREATE(timed_alloc, struct timed_type, 1);

    *timed_alloc = *timed;
    timed_alloc->next = ch->timed;
    ch->timed = timed_alloc;
}

void timed_from_char(struct char_data *ch, struct timed_type *timed)
{
    struct timed_type *temp;

    if (ch->timed == NULL) {
        log("SYSERR: timed_from_char(%s) when no timed...", GET_NAME(ch));
        // core_dump();
        return;
    }

    REMOVE_FROM_LIST(timed, ch->timed, next);

    /*  switch (GET_SKILL(ch, ch->timed->skill))
       {
       case SKILL_AID:
       case SKILL_HOLYLIGHT:
       case SKILL_PRAY:
       case SKILL_LOOKING:
       case SKILL_PICK:
       case SKILL_IDENTIFY:
       case SKILL_POISONED:
       case SKILL_COURAGE:
       case SKILL_HIDETRACK:
       case SKILL_FIND:
       sprintf(buf, "Ваши силы востановились и Вы снова можете использовать умение \"%s\".\r\n", skill_info[ch->timed->skill].name.c_str());
       send_to_char(buf, ch);
       break;
       }
     */
    free(timed);
}

int timed_by_skill(struct char_data *ch, int skill)
{
    struct timed_type *hjp;

    for (hjp = ch->timed; hjp; hjp = hjp->next)
        if (hjp->skill == skill)
            return (hjp->time);

    return (0);
}


/* move a player out of a room */
void char_from_room(struct char_data *ch)
{
    struct char_data *temp;

    if (ch == NULL || ch->in_room == NOWHERE) {
        log("SYSERR: NULL character or NOWHERE in %s, char_from_room", __FILE__);
        return;
    }

    ch->last_room_dmg = 0;

    /* Убираем флаги проверенных локаций */
    for (int vdir = 0; vdir < NUM_OF_DIRS; vdir++)
        ch->vdir[vdir] = FALSE;
    if (AFF_FLAGGED(ch, AFF_ORENT))
        REMOVE_BIT(AFF_FLAGS(ch, AFF_ORENT), AFF_ORENT);

    if (FIGHTING(ch) != NULL)
        stop_fighting(ch, TRUE);

    if (ch->trap_object) {
        ch->trap_object->trap_victim = NULL;
        ch->trap_object = NULL;
    }

    if (affected_by_spell(ch, SPELL_BLADE_BARRIER)) {
        affect_from_char(ch, SPELL_BLADE_BARRIER);
        REMOVE_BIT(AFF_FLAGS(ch, AFF_BLADES), AFF_BLADES);
        //act("Стена лезвий осыпалась после изчезновения $n1.",FALSE,ch,0,0,TO_ROOM);
    }

    ch->not_moved.clear();
    ch->may_moved.clear();

    ch->player.current_quest_mob = NULL;
    ch->player.select_mode = 0;

    if (GET_FICTION(ch)) {
        if (!CAN_WEAR(GET_FICTION(ch), ITEM_WEAR_TAKE))
            go_close_fiction(ch, GET_FICTION(ch));
    }

    check_light(ch, LIGHT_NO, LIGHT_NO, LIGHT_NO, LIGHT_NO, -1);
    REMOVE_FROM_LIST(ch, world[ch->in_room].people, next_in_room);
    ch->in_room = NOWHERE;
    ch->next_in_room = NULL;
}


/* place a character in a room */
void char_to_room(struct char_data *ch, room_rnum room)
{
    int check_hide(struct char_data *ch, struct char_data *tch);
    struct char_data *tch;

    if (ch == NULL || room < 0 || room > top_of_world)
        log("SYSERR: Illegal value(s) passed to char_to_room. (Room: %d/%d Ch: %p",
            room, top_of_world, ch);
    else {
        ch->next_in_room = world[room].people;
        world[room].people = ch;
        ch->in_room = room;
        check_light(ch, LIGHT_NO, LIGHT_NO, LIGHT_NO, LIGHT_NO, 1);
        REMOVE_BIT(EXTRA_FLAGS(ch, EXTRA_FAILHIDE), EXTRA_FAILHIDE);
        REMOVE_BIT(EXTRA_FLAGS(ch, EXTRA_FAILSNEAK), EXTRA_FAILSNEAK);
        REMOVE_BIT(EXTRA_FLAGS(ch, EXTRA_FAILCAMOUFLAGE), EXTRA_FAILCAMOUFLAGE);
        /*      if (IS_GRGOD(ch) && PRF_FLAGGED(ch,PRF_CODERINFO))
           {sprintf(buf,"%sКомната=%s%d %sСвет=%s%d %sОсвещ=%s%d %sКостер=%s%d \r\n"
           "%sТьма=%s%d %sЗапрет=%s%d %sСолнце=%s%d %sНебо=%s%d %sЛуна=%s%d.\r\n",
           CCNRM(ch,C_NRM), CCINRM(ch, C_NRM), room,
           CCRED(ch,C_NRM), CCIRED(ch, C_NRM), world[room].light,
           CCGRN(ch,C_NRM), CCIGRN(ch, C_NRM), world[room].glight,
           CCYEL(ch,C_NRM), CCIYEL(ch, C_NRM), world[room].fires,
           CCBLU(ch,C_NRM), CCIBLU(ch, C_NRM), world[room].gdark,
           CCMAG(ch,C_NRM), CCIMAG(ch, C_NRM), world[room].forbidden,
           CCCYN(ch,C_NRM), CCICYN(ch, C_NRM), weather_info.sky,
           CCWHT(ch,C_NRM), CCIWHT(ch, C_NRM), weather_info.sunlight,
           CCYEL(ch,C_NRM), CCIYEL(ch, C_NRM), weather_info.moon_day
           );
           send_to_char(buf,ch);
           } */

        /* Stop fighting now, if we left. */
        if (FIGHTING(ch) && IN_ROOM(ch) != IN_ROOM(FIGHTING(ch))) {
            stop_fighting(FIGHTING(ch), FALSE);
            stop_fighting(ch, TRUE);
        }
        //Проверку на видимость других персонажей
        if (IN_ROOM(ch) != NOWHERE)
            for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
                if (ch != tch && affected_by_spell(tch, SPELL_HIDE))
                    check_hide(tch, ch);
    }
}

void restore_object(struct obj_data *obj, struct char_data *ch)
{
    int i, j;

// struct C_obj_affected_type *af;

    if ((i = GET_OBJ_RNUM(obj)) < 0)
        return;
    if (GET_OBJ_OWNER(obj) &&
        OBJ_FLAGGED(obj, ITEM_NODONATE) &&
        (!ch || IS_NPC(ch) || GET_UNIQUE(ch) != GET_OBJ_OWNER(obj))
        ) {
        GET_OBJ_VAL(obj, 0) = GET_OBJ_VAL(obj_proto + i, 0);
        GET_OBJ_VAL(obj, 1) = GET_OBJ_VAL(obj_proto + i, 1);
        GET_OBJ_VAL(obj, 2) = GET_OBJ_VAL(obj_proto + i, 2);
        GET_OBJ_VAL(obj, 3) = GET_OBJ_VAL(obj_proto + i, 3);
        //GET_OBJ_MATER(obj) = GET_OBJ_MATER(obj_proto+i);
        GET_OBJ_MAX(obj) = GET_OBJ_MAX(obj_proto + i);
        GET_OBJ_CUR(obj) = 1;
        GET_OBJ_WEIGHT(obj) = GET_OBJ_WEIGHT(obj_proto + i);
        GET_OBJ_TIMER(obj) = GET_OBJ_TIMER(obj_proto + i);
        obj->obj_flags.extra_flags = (obj_proto + i)->obj_flags.extra_flags;
        obj->obj_flags.affects = (obj_proto + i)->obj_flags.affects;
        GET_OBJ_WEAR(obj) = GET_OBJ_WEAR(obj_proto + i);
        GET_OBJ_OWNER(obj) = 0;

        /* for (af = (obj_proto+i)->C_affected;af;af=af->next)
           {


           } */
        for (j = 0; j < MAX_OBJ_AFFECT; j++)
            obj->affected[j] = (obj_proto + i)->affected[j];
    }
}


/* give an object to a char   */
void obj_to_char(struct obj_data *object, struct char_data *ch)
{
    int may_carry = TRUE;

    if (object && ch) {
        restore_object(object, ch);
        if (invalid_no_class(ch, object) || invalid_unique(ch, object))
            may_carry = FALSE;

        if (!may_carry) {
            act("Электрический разряд заставил Вас выпустить $o3 из рук.", FALSE, ch, object, 0,
                TO_CHAR);
            act("$n3 ударило током и $e выронил$g $o3.", FALSE, ch, object, 0, TO_ROOM);
            obj_to_room(object, IN_ROOM(ch));
            return;
        }

        if (!IS_NPC(ch) && OBJ_FLAGGED(object, ITEM_SOULBIND) && !IS_GOD(ch)) {
            if (object->owner && object->owner != GET_ID(ch)) {
                act("Вы тут же выронили @1в едва взяв @1ер в руки.", "Мп", ch, object);
                act("1и выронил1(,а,о,и) @1в едва взяв @1ер в руки.", "Кмп", ch, object);
                obj_to_room(object, IN_ROOM(ch));
                return;
            } else if (!object->owner) {
                object->owner = GET_ID(ch);
                act("Вы почувствовали, что @1и стал@1(,а,о,и) частью Вашей души.", "Мп", ch,
                    object);
            }
        }

        if (!IS_NPC(ch) && GET_OBJ_TYPE(object) != ITEM_TATOO)
            SET_BIT(GET_OBJ_EXTRA(object, ITEM_TICKTIMER), ITEM_TICKTIMER);

        if (IS_NPC(ch) && !AFF_FLAGGED(ch, AFF_CHARM))
            REMOVE_BIT(GET_OBJ_EXTRA(object, ITEM_TICKTIMER), ITEM_TICKTIMER);

        object->next_content = ch->carrying;
        ch->carrying = object;
        object->carried_by = ch;
        object->in_room = NOWHERE;
        IS_CARRYING_W(ch) += GET_OBJ_WEIGHT(object);
        IS_CARRYING_N(ch)++;

        set_obj_quest(ch, GET_OBJ_VNUM(object));
        /* set flag for crash-save system, but not on mobs! */
        if (!IS_NPC(ch))
            SET_BIT(PLR_FLAGS(ch, PLR_CRASH), PLR_CRASH);
    } else
        log("SYSERR: NULL obj (%p) or char (%p) passed to obj_to_char.", object, ch);
}


/* take an object from a char */
void obj_from_char(struct obj_data *object)
{
    struct obj_data *temp;

    if (object == NULL) {
        log("SYSERR: NULL object passed to obj_from_char.");
        return;
    }

    if (object->carried_by == NULL) {
        log("ОШИБКА: Премет %d уже изъят у персонажа", GET_OBJ_VNUM(object));
        if (object->worn_by != NULL)
            log("ОШИБКА: но есть в экипировке %s", GET_NAME(object->worn_by));
        return;
    }

    if (object == GET_FICTION(object->carried_by)) {
        GET_FICTION(object->carried_by) = NULL;
        SET_BIT(OBJVAL_FLAGS(object, EXIT_CLOSED), EXIT_CLOSED);
        object->page = 0;
    }

    IS_CARRYING_W(object->carried_by) -= GET_OBJ_WEIGHT(object);
    IS_CARRYING_N(object->carried_by)--;

    unset_obj_quest(object->carried_by, GET_OBJ_VNUM(object));
    REMOVE_FROM_LIST(object, object->carried_by->carrying, next_content);

    /* set flag for crash-save system, but not on mobs! */
    if (!IS_NPC(object->carried_by))
        SET_BIT(PLR_FLAGS(object->carried_by, PLR_CRASH), PLR_CRASH);

    object->carried_by = NULL;
    object->next_content = NULL;
}

int preequip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
    if (pos < 0 || pos >= NUM_WEARS) {
        log("SYSERR: preequip(%s,%d) in unknown pos...", GET_NAME(ch), pos);
        // core_dump();
        return (FALSE);
    }

    if (GET_EQ(ch, pos)) {
        log("SYSERR: Char is already equipped: %s, %s", GET_NAME(ch), obj->short_description);
        return (FALSE);
    }
    /*
       if (obj->carried_by)
       {log("SYSERR: PREEQUIP: %s - Obj is carried_by when equip.", OBJN(obj,ch,0));
       return(FALSE);
       } */
    if (obj->in_room != NOWHERE) {
        log("SYSERR: PREEQUIP: %s - Obj is in_room when equip.", OBJN(obj, ch, 0));
        return (FALSE);
    }

    if (IS_BARIAUR(ch) && pos == WEAR_FEET) {
        act("$o0 не предназначен$A для Вас.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n попытал$u использовать $o3, но у н$s ничего не получилось.", FALSE, ch, obj, 0,
            TO_ROOM);
        return (FALSE);
    }

    if (!IS_TIEFLING(ch) && pos == WEAR_TAIL) {
        act("$o0 не предназначен$A для Вас.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n попытал$u использовать $o3, но у н$s ничего не получилось.", FALSE, ch, obj, 0,
            TO_ROOM);
        return (FALSE);
    }

    if (invalid_no_class(ch, obj)) {
        act("$o0 не предназначен$A для Вас.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n попытал$u использовать $o3, но у н$s ничего не получилось.", FALSE, ch, obj, 0,
            TO_ROOM);
        //obj_to_char(obj, ch);
        return (FALSE);
    }

    if (!IS_NPC(ch) && invalid_anti_class(ch, obj)) {
        act("Вас обожгло при попытке надеть $o3.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n попытал$u использовать $o3 - и чудом не обгорел$g.", FALSE, ch, obj, 0, TO_ROOM);
        obj_from_char(obj);
        obj_to_room(obj, IN_ROOM(ch));
        obj_decay(obj);
        return (FALSE);
    }

    return (TRUE);
}

void equiped_char(struct char_data *ch, struct obj_data *obj, int pos)
{
    int was_lgt = AFF_FLAGGED(ch, AFF_SINGLELIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hlgt = AFF_FLAGGED(ch, AFF_HOLYLIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hdrk = AFF_FLAGGED(ch, AFF_HOLYDARK) ? LIGHT_YES : LIGHT_NO, was_lamp = FALSE, i;

    for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i)) {
            if ((GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_LIGHT && GET_LIGHT_VAL(GET_EQ(ch, i)) && GET_LIGHT_ON(GET_EQ(ch, i))) || (GET_OBJ_TYPE(GET_EQ(ch, i)) != ITEM_LIGHT && GET_LIGHT_VAL(GET_EQ(ch, i)))) {    //send_to_char("Light OK!\r\n",ch);
                was_lamp = TRUE;
            }
        }


    GET_EQ(ch, pos) = obj;
    obj->worn_by = ch;
    obj->worn_on = pos;
    obj->next_content = NULL;
    IS_WEARING_W(ch) += GET_OBJ_WEIGHT(obj);

    if (ch->in_room == NOWHERE)
        log("SYSERR: ch->in_room = NOWHERE when equipping char %s.", GET_NAME(ch));
    /*  else
       IN_ROOM(obj) = IN_ROOM(ch);   */
    check_sets(ch);
    //log("[PREEQUIP_CHAR(%s)->AFFECT_TOTAL] Start",GET_NAME(ch));
    affect_total(ch);
    //log("[PREEQUIP_CHAR(%s)->AFFECT_TOTAL] Stop",GET_NAME(ch));
    check_light(ch, was_lamp, was_lgt, was_hlgt, was_hdrk, 1);

}

void postequip_char(struct char_data *ch, struct obj_data *obj)
{

// непонятно для чего пока
}




void equip_char(struct char_data *ch, struct obj_data *obj, int pos)
{
    int was_lgt = AFF_FLAGGED(ch, AFF_SINGLELIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hlgt = AFF_FLAGGED(ch, AFF_HOLYLIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hdrk = AFF_FLAGGED(ch, AFF_HOLYDARK) ? LIGHT_YES : LIGHT_NO, was_lamp = FALSE, i;
    int skip_total = IS_SET(pos, 0x80);
    struct C_obj_affected_type *af;

    REMOVE_BIT(pos, (0x80 | 0x40));

    if (pos < 0 || pos >= NUM_WEARS) {
        log("SYSERR: equip_char(%s,%d) in unknown pos...", GET_NAME(ch), pos);
        // core_dump();
        return;
    }

    if (GET_EQ(ch, pos)) {
        log("SYSERR: Char is already equipped: %s, %s", GET_NAME(ch), obj->short_description);
        return;
    }
    if (obj->carried_by) {
        log("SYSERR: EQUIP: %s - Obj is carried_by when equip.", OBJN(obj, ch, 0));
        return;
    }
    if (obj->in_room != NOWHERE) {
        log("SYSERR: EQUIP: %s - Obj is in_room when equip.", OBJN(obj, ch, 0));
        return;
    }

    if (invalid_anti_class(ch, obj)) {
        act("Вас обожгло при попытке надеть $o3.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n попытал$u надеть $o3 - и чудом не обгорел$g.", FALSE, ch, obj, 0, TO_ROOM);
        obj_to_room(obj, IN_ROOM(ch));
        obj_decay(obj);
        return;
    }

    if (!IS_NPC(ch) && invalid_no_class(ch, obj)) {
        act("$o3 явно не предназначен$A для Вас.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n попытал$u надеть $o3, но у н$s ничего не получилось.", FALSE, ch, obj, 0, TO_ROOM);
        obj_to_char(obj, ch);
        return;
    }

    for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i)) {
            if ((GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_LIGHT && GET_LIGHT_VAL(GET_EQ(ch, i)) && GET_LIGHT_ON(GET_EQ(ch, i))) || (GET_OBJ_TYPE(GET_EQ(ch, i)) != ITEM_LIGHT && GET_LIGHT_VAL(GET_EQ(ch, i)))) {    //send_to_char("Light OK!\r\n",ch);
                was_lamp = TRUE;
            }
        }


    GET_EQ(ch, pos) = obj;
    obj->worn_by = ch;
    obj->worn_on = pos;
    obj->next_content = NULL;
    IS_WEARING_W(ch) += GET_OBJ_WEIGHT(obj);

    if (ch->in_room == NOWHERE)
        log("SYSERR: ch->in_room = NOWHERE when equipping char %s.", GET_NAME(ch));
    /*  else
       IN_ROOM(obj) = IN_ROOM(ch); */

    for (af = obj->C_affected; af; af = af->next)
        affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);


    if (IN_ROOM(ch) != NOWHERE) {
        if (!skip_total) {      //log("[EQUIP_CHAR(%s)->AFFECT_TOTAL] Start",GET_NAME(ch));
            check_sets(ch);
            affect_total(ch);
            //log("[EQUIP_CHAR(%s)->AFFECT_TOTAL] Stop",GET_NAME(ch));
            check_light(ch, was_lamp, was_lgt, was_hlgt, was_hdrk, 1);
        }
    }
}



struct obj_data *unequip_char(struct char_data *ch, int pos)
{
    int was_lgt = AFF_FLAGGED(ch, AFF_SINGLELIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hlgt = AFF_FLAGGED(ch, AFF_HOLYLIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hdrk = AFF_FLAGGED(ch, AFF_HOLYDARK) ? LIGHT_YES : LIGHT_NO, was_lamp = FALSE, i;
    struct C_obj_affected_type *af;
    int skip_total = IS_SET(pos, 0x80);
    struct obj_data *obj;

    //struct affected_type *af, *next;

    REMOVE_BIT(pos, (0x80 | 0x40));

    if ((pos < 0 || pos >= NUM_WEARS) || GET_EQ(ch, pos) == NULL) {
        log("SYSERR: unequip_char(%s,%d) - unused pos or no equip...", GET_NAME(ch), pos);
        // core_dump();
        return (NULL);
    }

    for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i)) {
            if ((GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_LIGHT && GET_LIGHT_VAL(GET_EQ(ch, i)) && GET_LIGHT_ON(GET_EQ(ch, i))) || (GET_OBJ_TYPE(GET_EQ(ch, i)) != ITEM_LIGHT && GET_LIGHT_VAL(GET_EQ(ch, i)))) {    //send_to_char("Light OK!\r\n",ch);
                was_lamp = TRUE;
            }
        }


    obj = GET_EQ(ch, pos);
    obj->carried_by = NULL;
    obj->worn_by = NULL;
    obj->next_content = NULL;
    obj->worn_on = -1;
    GET_EQ(ch, pos) = NULL;
    IS_WEARING_W(ch) -= GET_OBJ_WEIGHT(obj);

    if (ch->in_room == NOWHERE)
        log("SYSERR: ch->in_room = NOWHERE when unequipping char %s.", GET_NAME(ch));


    for (af = obj->C_affected; af; af = af->next)
        affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);


    if (IN_ROOM(ch) != NOWHERE)
        if (!skip_total) {      //log("[UNEQUIP_CHAR->AFFECT_TOTAL] Start");
            check_sets(ch);
            affect_total(ch);
            //log("[UNEQUIP_CHAR->AFFECT_TOTAL] Stop");
            check_light(ch, was_lamp, was_lgt, was_hlgt, was_hdrk, 1);
        }

    IN_ROOM(obj) = NOWHERE;
    return (obj);
}


int get_number(char **name)
{
    int i;
    char *ppos;
    char number[MAX_INPUT_LENGTH];

    *number = '\0';

    if ((ppos = strchr(*name, '.')) != NULL) {
        *ppos = '\0';
        strcpy(number, *name);
        for (i = 0; *(number + i); i++) {
            if (!IS_DIGIT(*(number + i))) {
                *ppos = '.';
                return (1);
            }
        }
        strcpy(*name, ppos + 1);
        return (atoi(number));
    }
    return (1);
}



/* Search a given list for an object number, and return a ptr to that obj */
struct obj_data *get_obj_in_list_num(int num, struct obj_data *list)
{
    struct obj_data *i;

    for (i = list; i; i = i->next_content)
        if (GET_OBJ_RNUM(i) == num)
            return (i);

    return (NULL);
}



/* search the entire world for an object number, and return a pointer  */
struct obj_data *get_obj_num(obj_rnum nr)
{
    struct obj_data *i;

    for (i = object_list; i; i = i->next)
        if (GET_OBJ_RNUM(i) == nr)
            return (i);

    return (NULL);
}



/* search a room for a char, and return a pointer if found..  */
struct char_data *get_char_room(char *name, room_rnum room)
{
    struct char_data *i;
    int j = 0, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp = tmpname;

    strcpy(tmp, name);
    if (!(number = get_number(&tmp)))
        return (NULL);

    for (i = world[room].people; i && (j <= number); i = i->next_in_room)
        if (isname(tmp, i->player.name))
            if (++j == number)
                return (i);

    return (NULL);
}



/* search all over the world for a char num, and return a pointer if found */
struct char_data *get_char_num(mob_rnum nr)
{
    struct char_data *i;

    for (i = character_list; i; i = i->next)
        if (GET_MOB_RNUM(i) == nr)
            return (i);

    return (NULL);
}


const int money_destroy_timer = 45;
const int death_destroy_timer = 5;
const int room_destroy_timer = 30;
const int room_nodestroy_timer = -1;
const int script_destroy_timer = 1; /**!!! Never set less than ONE **/

/* put an object in a room */
void obj_to_room(struct obj_data *object, room_rnum room)
{
    int sect = 0;

    if (!object || room < 0 || room > top_of_world) {
        log("SYSERR: Illegal value(s) passed to obj_to_room. (Room #%d/%d, obj %p)",
            room, top_of_world, object);
        if (object)
            extract_obj(object);
    } else {
        restore_object(object, 0);
        object->next_content = world[room].contents;
        world[room].contents = object;
        object->in_room = room;
        object->carried_by = NULL;
        object->worn_by = NULL;
        sect = real_sector(room);
        if (GET_OBJ_TYPE(object) == ITEM_LIGHT
            && (GET_LIGHT_VAL(object) == -1 || GET_LIGHT_ON(object)))
            world[room].light += 1;
        if (ROOM_FLAGGED(room, ROOM_HOUSE))
            SET_BIT(ROOM_FLAGS(room, ROOM_HOUSE_CRASH), ROOM_HOUSE_CRASH);
        if (OBJ_FLAGGED(object, ITEM_NODECAY))
            GET_OBJ_DESTROY(object) = room_nodestroy_timer;
        else if (GET_OBJ_TYPE(object) == ITEM_MONEY)
            GET_OBJ_DESTROY(object) = money_destroy_timer;
        else if (ROOM_FLAGGED(room, ROOM_DEATH))
            GET_OBJ_DESTROY(object) = death_destroy_timer;
        else if (ROOM_FLAGGED(room, ROOM_NODECAY))
            GET_OBJ_DESTROY(object) = GET_OBJ_TIMER(object);
        else
            GET_OBJ_DESTROY(object) = room_destroy_timer;

    }
}

/* Функция для удаления обьектов после лоада в комнату
   результат работы - 1 если посыпался, 0 - если остался */
int obj_decay(struct obj_data *object)
{
    int room, sect;

    room = object->in_room;
    sect = real_sector(room);

    if (room == NOWHERE)
        return (0);

    if ((OBJ_FLAGGED(object, ITEM_DECAY) && !object->carried_by && !object->worn_by) ||
        (OBJ_FLAGGED(object, ITEM_ZONEDECAY) &&
         GET_OBJ_ZONE(object) != NOWHERE && GET_OBJ_ZONE(object) != world[room].zone)) {

        log("**** Рассыпается %s", GET_OBJ_PNAME(object, 0));
        log("Условия carr %s, worn %s", object->carried_by ? "да" : "нет",
            object->worn_by ? "да" : "нет");
        act("$o0 рассыпал$U в мелкую пыль, которую развеял ветер.", FALSE, world[room].people,
            object, 0, TO_ROOM);
        act("$o0 рассыпал$U в мелкую пыль, которую развеял ветер.", FALSE, world[room].people,
            object, 0, TO_CHAR);
        extract_obj(object);
        return (1);
    }
//падают и тонут
    if (object->worn_by || object->carried_by)
        return (0);


    if (((sect == SECT_WATER_SWIM || sect == SECT_WATER_NOSWIM || sect == SECT_UNDERWATER) &&
         !OBJ_FLAGGED(object, ITEM_SWIMMING))) {
        obj_swim_down(object, room);
    }

    if (((sect == SECT_FLYING) && !OBJ_FLAGGED(object, ITEM_FLYING))) {
        obj_drop_down(object, room);
    }


    if (ROOM_FLAGGED(room, ROOM_DOWN) && !OBJ_FLAGGED(object, ITEM_BURIED)) {
        act("$o0 утонул$G в куче мусора.", FALSE, world[room].people, object, 0, TO_ROOM);
        act("$o0 утонул$G в куче мусора.", FALSE, world[room].people, object, 0, TO_CHAR);

        SET_BIT(GET_OBJ_EXTRA(object, ITEM_BURIED), ITEM_BURIED);
    }

    return (0);
}

/* Take an object from a room */
void obj_from_room(struct obj_data *object)
{
    struct obj_data *temp;

    if (!object) {
        log("SYSERR: NULL object passed to obj_from_room");
        return;
    }

    if (object->trap_victim) {
        object->trap_victim->trap_object = NULL;
        object->trap_victim = NULL;
    }

    if (object->in_room == NOWHERE) {
        log("SYSERR: obj '%s' not in a room (%d) passed to obj_from_room", object->name,
            object->in_room);
        return;
    }

    if (GET_OBJ_TYPE(object) == ITEM_LIGHT && world[object->in_room].light
        && (GET_LIGHT_VAL(object) == -1 || GET_LIGHT_ON(object)))
        world[object->in_room].light -= 1;

    REMOVE_FROM_LIST(object, world[object->in_room].contents, next_content);

    if (ROOM_FLAGGED(object->in_room, ROOM_HOUSE))
        SET_BIT(ROOM_FLAGS(object->in_room, ROOM_HOUSE_CRASH), ROOM_HOUSE_CRASH);
    object->in_room = NOWHERE;
    object->next_content = NULL;
}


/* put an object in an object (quaint)  */
void obj_to_obj(struct obj_data *obj, struct obj_data *obj_to)
{
    struct obj_data *tmp_obj;

    if (!obj || !obj_to || obj == obj_to) {
        log("SYSERR: NULL object (%p) or same source (%p) and target (%p) obj passed to obj_to_obj.", obj, obj, obj_to);
        return;
    }

    /*  if (GET_OBJ_COUNTITEMS(obj_to) >= GET_OBJ_VAL(obj_to, 0)/100)
       {
       log("ОШИБКА: Попытка положить в контейнер предметов больше чем он может вместить");
       return;
       } */

    obj->next_content = obj_to->contains;
    obj_to->contains = obj;
    obj->in_obj = obj_to;

    for (tmp_obj = obj->in_obj; tmp_obj->in_obj; tmp_obj = tmp_obj->in_obj)
        GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);

    /* top level object.  Subtract weight from inventory if necessary. */
    GET_OBJ_WEIGHT(tmp_obj) += GET_OBJ_WEIGHT(obj);
    if (tmp_obj->carried_by)
        IS_CARRYING_W(tmp_obj->carried_by) +=
            (GET_OBJ_WEIGHT(obj) * (100 - GET_OBJ_VAL(obj_to, 3))) / 100;

    GET_OBJ_COUNTITEMS(obj_to)++;

}


/* remove an object from an object */
void obj_from_obj(struct obj_data *obj)
{
    struct obj_data *temp, *obj_from;

    if (obj->in_obj == NULL) {
        log("SYSERR: (%s): trying to illegally extract obj %s from obj.", __FILE__,
            GET_OBJ_PNAME(obj, 0));
        return;
    }
    obj_from = obj->in_obj;
    REMOVE_FROM_LIST(obj, obj_from->contains, next_content);

    GET_OBJ_COUNTITEMS(obj_from)--;

    /* Subtract weight from containers container */
    for (temp = obj->in_obj; temp->in_obj; temp = temp->in_obj)
        GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);

    /* Subtract weight from char that carries the object */
    GET_OBJ_WEIGHT(temp) -= GET_OBJ_WEIGHT(obj);
    if (temp->carried_by)
        IS_CARRYING_W(temp->carried_by) -=
            (GET_OBJ_WEIGHT(obj) * (100 - GET_OBJ_VAL(obj_from, 3))) / 100;

    obj->in_obj = NULL;
    obj->next_content = NULL;

}


/* Set all carried_by to point to new owner */
void object_list_new_owner(struct obj_data *list, struct char_data *ch)
{

    if (list) {
        object_list_new_owner(list->contains, ch);
        object_list_new_owner(list->next_content, ch);
        if (GET_OBJ_TYPE(list) == ITEM_TATOO)
            return;
        list->carried_by = ch;
    }
}

void extract_obj(struct obj_data *obj)
{
    extract_obj(obj, TRUE);
}

/* Extract an object from the world */
void extract_obj(struct obj_data *obj, int decay)
{
    struct obj_data *temp;
    struct char_data *tch;

    if (decay)
        extract_event_from_object(obj);

    if (obj->trap_victim)
        obj->trap_victim->trap_object = NULL;

    for (tch = character_list; tch; tch = tch->next) {
        if (IS_NPC(tch))
            continue;
        if (tch->obj_pick == obj)
            tch->obj_pick = NULL;
    }

    /* Если телега */
    if (obj->transpt && (obj->transpt->driver || obj->transpt->people.size())) {
        for (int i = 0; i < (int) obj->transpt->people.size(); i++) {
            tch = obj->transpt->people[i];
            act("Вы принудительно покинули @1в.", "Мп", tch, obj);
            act("1и принудительно покинул1(,а,о,и) @1в.", "Кмп", tch, obj);
            remove_char_from_obj(tch, obj);
            i--;
        }
    }

    /* Обработка содержимого контейнера при его уничтожении */
    while (obj->contains) {
        temp = obj->contains;
        obj_from_obj(temp);

        if (obj->carried_by) {
            if (IS_NPC(obj->carried_by) ||
                (IS_CARRYING_N(obj->carried_by) >= CAN_CARRY_N(obj->carried_by))) {
                obj_to_room(temp, IN_ROOM(obj->carried_by));
                obj_decay(temp);
            } else {
                obj_to_char(temp, obj->carried_by);
            }
        } else if (obj->worn_by != NULL) {
            if (IS_NPC(obj->worn_by) || (IS_CARRYING_N(obj->worn_by) >= CAN_CARRY_N(obj->worn_by))) {
                obj_to_room(temp, IN_ROOM(obj->worn_by));
                obj_decay(temp);
            } else {
                obj_to_char(temp, obj->worn_by);
            }
        } else if (obj->in_room != NOWHERE) {
            obj_to_room(temp, obj->in_room);
            obj_decay(temp);
        } else if (obj->in_obj) {
            extract_obj(temp);
        } else
            extract_obj(temp);
    }
    /* Содержимое контейнера удалено */

    if (obj->worn_by != NULL) {
        if (GET_OBJ_TYPE(obj) == ITEM_TATOO) {
            if (unequip_tatoo(obj->worn_by, obj->worn_on) != obj)
                log("SYSERR: Inconsistent worn_by and worn_on pointers!!");
        } else {
            if (unequip_char(obj->worn_by, obj->worn_on) != obj)
                log("SYSERR: Inconsistent worn_by and worn_on pointers!!");
        }
    }

    if (obj->worn_by != NULL)
        if (unequip_char(obj->worn_by, obj->worn_on) != obj)
            log("SYSERR: Inconsistent worn_by and worn_on pointers!!");
    if (obj->in_room != NOWHERE)
        obj_from_room(obj);
    else if (obj->carried_by)
        obj_from_char(obj);
    else if (obj->in_obj)
        obj_from_obj(obj);


    if (GET_OBJ_RNUM(obj) >= 0 && decay)
        (obj_index[GET_OBJ_RNUM(obj)].number)--;

    REMOVE_FROM_LIST(obj, object_list, next);

    delete obj;
}



void update_object(struct obj_data *obj, int use)
{
    /* dont update objects with a timer trigger */
    if (GET_OBJ_TIMER(obj) > 0 && OBJ_FLAGGED(obj, ITEM_TICKTIMER)
        )
        GET_OBJ_TIMER(obj) -= use;
    if (obj->contains)
        update_object(obj->contains, use);
    if (obj->next_content)
        update_object(obj->next_content, use);

}


void update_char_objects(struct char_data *ch)
{
    int i;

    for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i))
            update_object(GET_EQ(ch, i), 1);

    if (ch->carrying)
        update_object(ch->carrying, 1);
}

void change_fighting(struct char_data *ch, int need_stop)
{
    struct char_data *k, *j, *temp;
    int in_room = IN_ROOM(ch);

//log ("[CHANGE FIGHTING - START %s]",GET_NAME(ch));
//for (k = character_list; k; k = temp)
    for (k = world[in_room].people; k; k = temp) {
        temp = k->next_in_room;
        if (FIGHTING(k) != ch || IN_ROOM(k) == NOWHERE)
            continue;
        j = NULL;

        for (j = world[in_room].people; j; j = j->next_in_room) {
            if (j == ch)
                continue;
            if (!FIGHTING(j))
                continue;
            if (FIGHTING(j) == k) {
                act("Вы переключили внимание на $N3.", FALSE, k, 0, j, TO_CHAR);
                act("$n переключил$u на Вас!", FALSE, k, 0, j, TO_VICT);
                FIGHTING(k) = j;
                break;
            }
        }
        if (!j && need_stop)
            stop_fighting(k, FALSE);
    }
//log ("[CHANGE FIGHTING - END]");
}

static void mprog_extract(struct char_data *ch)
{
    bool fTotal = IS_NPC(ch);

    FENIA_VOID_CALL(ch, "Extract", "i", fTotal);
    FENIA_PROTO_VOID_CALL(ch->npc(), "Extract", "Ci", ch, fTotal);

    ch->extractWrapper(fTotal);
}

//флаг clear_obj дань старой моде, потом можно убрать будет
void extract_char(struct char_data *ch, int clear_objs)
{
    int cyc_count, i;
    struct descriptor_data *t_desc = NULL;
    struct descriptor_data *d;
    struct obj_data *obj = NULL, *next_obj = NULL;
    struct char_data *tch = NULL, *next_tch = NULL;
    struct affected_type *aff = NULL, *next_aff = NULL;

//Прекращаем все бои
    if (FIGHTING(ch))
        stop_fighting(ch, TRUE);

//Прекращаем охранные действия и прочие связи
    for (tch = character_list; tch; tch = next_tch) {
        next_tch = tch->next;
        if (GUARDING(tch) == ch)
            stop_guarding(tch);

        if (tch->invite_char == ch) {
            act("1и отозвал1(,а,о,и) свое приглашение в группу.", "Мм", ch, tch);
            tch->invite_char = NULL;
            tch->invite_time = 0;
        }

        if (tch->player.current_quest_mob == ch)
            tch->player.current_quest_mob = NULL;
    }

//Останавливаем все события
    stop_events(ch, STOP_ALL);
//Особождаем ссылки на события в которых принимал участие персонаж
    extract_event_from_char(ch);
//Очищаем память задействованную при побеге
    clean_flee(ch);
//Очищаем память от последних атак
    remove_last_attack(ch);
//Удаляем последователей
    if (ch->followers || ch->master)
        die_follower(ch);
//Очишаем группу
    if (ch->party || ch->party_leader)
        die_party(ch);

    if (ch->missed_magic) {
        struct follow_type *temp;

        DESTROY_LIST(ch->missed_magic, next, temp);
    }

//Убираем ловушки
    if (ch->trap_object)
        ch->trap_object->trap_victim = NULL;

    {                           //Удаляем ПК-список
        ch->pk_list.clear();
    }

    {                           //Удаляем список убийц
        ch->killer_list.clear();
    }

//Очищаем эффекты
    while (ch->affected)
        affect_remove(ch, ch->affected);

//Убираем персонажа из локации
    if (!IS_MOB(ch))
        save_char(ch, NOWHERE);

//Удаляем временные задержки на умения
    while (ch->timed)
        timed_from_char(ch, ch->timed);

//Убираем снупперов
    if (ch->desc) {
        if (ch->desc->snooping) {
            del_snooper(ch);
            ch->desc->snooping = NULL;
        }
        if (ch->desc->snoop_by_col) {
            for (cyc_count = 0; cyc_count < ch->desc->snoop_by_col; cyc_count++) {
                SEND_TO_Q("Ваша жертва теперь недоступна.\r\n", ch->desc->snoop_by[cyc_count]);
                ch->desc->snoop_by[cyc_count]->snooping = NULL;
            }
            ch->desc->snoop_by_col = 0;
            ch->desc->snoop_by = NULL;
        }
    }

    if (IS_NPC(ch) && IN_ROOM(ch) != NOWHERE) {
        int inroom = IN_ROOM(ch);

        for (obj = ch->carrying; obj; obj = next_obj) {
            next_obj = obj->next_content;
            obj_from_char(obj);
            obj_to_room(obj, inroom);
        }
        // в том числе и из экипировки
        for (i = 0; i < NUM_WEARS; i++)
            if (GET_EQ(ch, i)) {
                obj = unequip_char(ch, i);
                obj_to_room(obj, inroom);
            }
    } else {
        //Уничтожаем все предметы которые были у персонажа (кто не записался, я не виноват)
        for (obj = ch->carrying; obj; obj = next_obj) {
            next_obj = obj->next_content;
            obj_from_char(obj);
            extract_obj(obj);
        }
        // в том числе и из экипировки
        for (i = 0; i < NUM_WEARS; i++)
            if (GET_EQ(ch, i)) {
                obj = unequip_char(ch, i);
                extract_obj(obj);
            }
    }
//и татушки
    for (i = 0; i < NUM_WEARS; i++)
        if (GET_TATOO(ch, i)) {
            obj = unequip_tatoo(ch, i);
            extract_obj(obj);
        }

    char_from_room(ch);

    if (!IS_NPC(ch) && !ch->desc)
        for (t_desc = descriptor_list; t_desc; t_desc = t_desc->next)
            if (t_desc->original == ch) {
                if (t_desc->character) {
                    do_return(t_desc->character, NULL, 0, 1, 0);
                } else
                    t_desc->original = NULL;
            }

    if (ch->desc) {
        if (ch->desc->original)
            do_return(ch, NULL, 0, 1, 0);
        else {
            for (d = descriptor_list; d; d = d->next) {
                if (d == ch->desc)
                    continue;
                if (d->character && GET_IDNUM(ch) == GET_IDNUM(d->character))
                    STATE(d) = CON_CLOSE;
            }
            // STATE(ch->desc) = CON_MENU;
            //SEND_TO_Q(MENU, ch->desc);
        }
    }

//Если это монстр то выкладываем предметы, что остались в локацию

//Удаляем эффекты
    for (aff = ch->affected; aff; aff = next_aff) {
        next_aff = aff->next;
        affect_remove(ch, aff);
    }

// Дистанция до врага
    {
        ch->distance.clear();
    }

// Кто не может атаковать персонажа (санка)
    {
        ch->not_attack.clear();
    }
// Кто может атаковать персонажа (санка)
    {
        ch->may_attack.clear();
    }

// Кто не может приблизится (отторжение)
    {
        ch->not_moved.clear();
    }

// Кто уже приблизился (отторжение)
    {
        ch->may_moved.clear();
    }

    mprog_extract(ch);

//Удаляем специфичные для монстров структуры
    if (IS_NPC(ch)) {
        if (GET_MOB_RNUM(ch) > -1)
            mob_index[GET_MOB_RNUM(ch)].number--;

        //Устанавливаем признак того что моба надо удалить
        SET_BIT(MOB_FLAGS(ch, MOB_DELETE), MOB_DELETE);
        SET_BIT(MOB_FLAGS(ch, MOB_FREE), MOB_FREE);
    }
//Если это игрок имеющий соединение
    if (ch->desc != NULL) {
        STATE(ch->desc) = CON_MENU;
        SEND_TO_Q(MENU, ch->desc);
    }
}

/* ***********************************************************************
* Here follows high-level versions of some earlier routines, ie functions*
* which incorporate the actual player-data                               *.
*********************************************************************** */


struct char_data *get_player_soul(struct char_data *ch, char *name, int inroom)
{
    struct char_data *i;

    for (i = character_list; i; i = i->next) {
        if (IS_NPC(i) || (!i->desc && !(inroom & FIND_CHAR_DISCONNECTED)))
            continue;
        if ((inroom & FIND_CHAR_ROOM) && i->in_room != ch->in_room)
            continue;

        if (!IS_SOUL(i))
            continue;

        if (!isname(name, i->player.name) && !isfullname(name, i->player.names))
            continue;
        return (i);
    }

    return (NULL);
}

struct char_data *get_player_vis(struct char_data *ch, char *name, int inroom)
{
    struct char_data *i;

    for (i = character_list; i; i = i->next) {
        if (IS_NPC(i) || (!i->desc && !(inroom & FIND_CHAR_DISCONNECTED)))
            continue;
        if ((inroom & FIND_CHAR_ROOM) && i->in_room != ch->in_room)
            continue;
        if (!CAN_SEE(ch, i))
            continue;
        if (IS_SOUL(i))
            continue;
        //if (!isfullname(name,i->player.name)) полное имя
        if (!isname(name, i->player.name) && !isfullname(name, i->player.names))
            continue;
        return (i);
    }

    return (NULL);
}


struct char_data *get_char_room_vis2(struct char_data *ch, char *name, int *number)
{
    struct char_data *i;
    int num;

    if (!number) {
        number = &num;
        num = get_number(&name);
    }

    /* JE 7/18/94 :-) :-) */
    if (!str_cmp(name, "self") || !str_cmp(name, "me") ||
        !str_cmp(name, "я") || !str_cmp(name, "меня") || !str_cmp(name, "себя"))
        return (ch);

    /* 0.<name> means PC with name */
    if (*number == 0)
        return (get_player_vis(ch, name, FIND_CHAR_ROOM));

    //сначала среди монстров
    for (i = world[IN_ROOM(ch)].people; i && *number; i = i->next_in_room) {
        if (!IS_NPC(i) || NPC_FLAGGED(i, NPC_HIDDEN))
            continue;

        if ((isname(name, i->player.name) || isfullname(name, i->player.names))
            || (IS_DARK(i->in_room)
                && isname(name, race_name[(int) GET_RACE(i)][(int) GET_SEX(i)])))
            if (CAN_SEE(ch, i) || check_victim_visible(ch, i))
                if (--(*number) == 0)
                    return (i);
    }

    //теперь среди игроков
    for (i = world[IN_ROOM(ch)].people; i && *number; i = i->next_in_room) {
        if (IS_NPC(i))
            continue;
        if (IS_SOUL(i))
            continue;
        if (isname(name, i->player.name) || isfullname(name, i->player.names) ||
            ((check_incognito(i) || IS_DARK(i->in_room)) &&
             isname(name, race_name[(int) GET_RACE(i)][(int) GET_SEX(i)])))
            if (CAN_SEE(ch, i) || check_victim_visible(ch, i))
                if (--(*number) == 0)
                    return (i);
    }

    return (NULL);
}

struct char_data *get_char_room_vis(struct char_data *ch, char *name)
{
    struct char_data *i;
    int j = 0, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp = tmpname;

    /* JE 7/18/94:-):-) */
    if (!str_cmp(name, "self") || !str_cmp(name, "me") ||
        !str_cmp(name, "я") || !str_cmp(name, "меня") || !str_cmp(name, "себя"))
        return (ch);

    /* 0.<name> means PC with name */
    strcpy(tmp, name);
    if (!(number = get_number(&tmp)))
        return (get_player_vis(ch, tmp, FIND_CHAR_ROOM));

    for (i = world[ch->in_room].people; i && j <= number; i = i->next_in_room) {

        if ((IS_NPC(i) || i->desc) && CAN_SEE(ch, i)
            && (isname(tmp, i->player.name) || isfullname(tmp, i->player.names)))
            if (++j == number)
                return (i);
    }
    return (NULL);
}


struct char_data *find_char(struct char_data *ch, char *name)
{
    struct char_data *i;
    int j = 0, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp = tmpname;


    strcpy(tmp, name);
    if (!(number = get_number(&tmp)))
        number = 1;

    if (IN_ROOM(ch) != NOWHERE)
        for (i = world[ch->in_room].people; i && j <= number; i = i->next_in_room) {
            if (IS_NPC(i) && NPC_FLAGGED(i, NPC_HIDDEN))
                continue;
            if ((IS_NPC(i) || i->desc) && CAN_SEE(ch, i)
                && (isname(tmp, i->player.name) || isfullname(tmp, i->player.names)))
                if (++j == number)
                    return (i);
        }

    for (i = character_list; i && (j <= number); i = i->next) {
        if (IS_NPC(i) && NPC_FLAGGED(i, NPC_HIDDEN))
            continue;
        if (isname(tmp, i->player.name) || isfullname(tmp, i->player.names))
            if (++j == number)
                return (i);
    }
    return (NULL);
}

struct char_data *get_char_vis(struct char_data *ch, char *name, int where)
{
    struct char_data *i;
    int j = 0, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp = tmpname;

    /* check the room first */
    if (where == FIND_CHAR_ROOM)
        return get_char_room_vis2(ch, name, 0);
    else if (where == FIND_CHAR_WORLD) {
        if ((i = get_char_room_vis(ch, name)) != NULL)
            return (i);

        strcpy(tmp, name);
        if (!(number = get_number(&tmp)))
            return get_player_vis(ch, tmp, 0);

        for (i = character_list; i && (j <= number); i = i->next)
            if ((IS_NPC(i) || i->desc)
                && CAN_SEE(ch, i)
                && !IS_SOUL(i)
                && !(IS_NPC(i) && NPC_FLAGGED(i, NPC_HIDDEN))
                && (isname(tmp, i->player.name) || isfullname(tmp, i->player.names)))
                if (++j == number)
                    return (i);
    }

    return (NULL);
}



struct obj_data *get_obj_in_list_vis2(struct char_data *ch, char *name, int *number,
                                      struct obj_data *list)
{
    struct obj_data *i;
    int num;

    if (!number) {
        number = &num;
        num = get_number(&name);
    }

    if (*number == 0)
        return (NULL);

    for (i = list; i && *number; i = i->next_content)
        if (isname(name, i->name) || isfullname(name, i->names))
            if (CAN_SEE_OBJ(ch, i))
                if (--(*number) == 0)
                    return (i);

    return (NULL);
}

struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name, struct obj_data *list)
{
    struct obj_data *i;
    int j = 0, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp = tmpname;

    strcpy(tmp, name);
    if (!(number = get_number(&tmp)))
        return (NULL);

    for (i = list; i && (j <= number); i = i->next_content)
        if (isname(tmp, i->name) || isfullname(tmp, i->names))
            if (CAN_SEE_OBJ(ch, i))
                if (++j == number) {    //* sprintf(buf,"Show obj %d %s %x ", number, i->name, i);
                    //* send_to_char(buf,ch);
                    return (i);
                }

    return (NULL);
}




/* search the entire world for an object, and return a pointer  */
struct obj_data *get_obj_vis(struct char_data *ch, char *name)
{
    struct obj_data *i;
    int j = 0, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp = tmpname;

    log("get_obj_vis");
    /* scan items carried */
    if ((i = get_obj_in_list_vis(ch, name, ch->carrying)) != NULL)
        return (i);

    /* scan room */
    if ((i = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)) != NULL)
        return (i);

    strcpy(tmp, name);
    if ((number = get_number(&tmp)) == 0)
        return (NULL);

    /* ok.. no luck yet. scan the entire obj list   */
    for (i = object_list; i && (j <= number); i = i->next)
        if (isname(tmp, i->name) || isfullname(tmp, i->names))
            if (CAN_SEE_OBJ(ch, i))
                if (++j == number)
                    return (i);

    return (NULL);
}



struct obj_data *get_object_in_equip_vis(struct char_data *ch,
                                         char *arg, struct obj_data *equipment[], int *j)
{
    int l, number;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp = tmpname;

    strcpy(tmp, arg);

    if ((number = get_number(&tmp)) == 0)
        return (NULL);

    for ((*j) = 0, l = 0; (*j) < NUM_WEARS; (*j)++)
        if (equipment[(*j)])
            if (CAN_SEE_OBJ(ch, equipment[(*j)]))
                if (isname(arg, equipment[(*j)]->name) || isfullname(arg, equipment[(*j)]->names))
                    if (++l == number)
                        return (equipment[(*j)]);

    return (NULL);
}

int get_obj_pos_in_equip_vis(struct char_data *ch, char *arg, int *number,
                             struct obj_data *equipment[])
{
    int j, num;

    if (!number) {
        number = &num;
        num = get_number(&arg);
    }

    if (*number == 0)
        return (-1);

    for (j = 0; j < NUM_WEARS; j++)
        if (equipment[j] && CAN_SEE_OBJ(ch, equipment[j])
            && (isname(arg, equipment[j]->name) || isfullname(arg, equipment[j]->names)))
            if (--(*number) == 0)
                return (j);
    return (-1);
}

struct obj_data *get_obj_in_eq_vis(struct char_data *ch, char *arg)
{
    int l, number, j;
    char tmpname[MAX_INPUT_LENGTH];
    char *tmp = tmpname;

    strcpy(tmp, arg);
    if ((number = get_number(&tmp)) == 0)
        return (NULL);

    for (j = 0, l = 0; j < NUM_WEARS; j++)
        if (GET_EQ(ch, j))
            if (CAN_SEE_OBJ(ch, GET_EQ(ch, j)))
                if (isname(arg, GET_EQ(ch, j)->name) || isfullname(arg, GET_EQ(ch, j)->names))
                    if (++l == number)
                        return (GET_EQ(ch, j));

    return (NULL);
}


char *money_desc(int amount, int padis)
{
    static char buf[128];
    const char *single[6][2] = { {"а", "а"},
    {"ой", "ы"},
    {"ой", "е"},
    {"у", "у"},
    {"ой", "ой"},
    {"ой", "е"}
    }, *plural[6][3] = { {
    "ая", "а", "а"}, {
    "ой", "и", "ы"}, {
    "ой", "е", "е"}, {
    "ую", "у", "у"}, {
    "ой", "ой", "ой"}, {
    "ой", "е", "е"}
    };

    if (amount <= 0) {
        log("SYSERR: Try to create negative or 0 money (%d).", amount);
        return (NULL);
    }
    if (amount == 1) {
        sprintf(buf, "одн%s монет%s", single[padis][0], single[padis][1]);
    } else if (amount <= 5)
        sprintf(buf, "малюсеньк%s горстк%s монет", plural[padis][0], plural[padis][1]);
    else if (amount <= 10)
        sprintf(buf, "маленьк%s горстк%s монет", plural[padis][0], plural[padis][1]);
    else if (amount <= 20)
        sprintf(buf, "небольш%s горстк%s монет", plural[padis][0], plural[padis][1]);
    else if (amount <= 50)
        sprintf(buf, "маленьк%s кучк%s монет", plural[padis][0], plural[padis][1]);
    else if (amount <= 75)
        sprintf(buf, "небольш%s кучк%s монет", plural[padis][0], plural[padis][1]);
    else if (amount <= 100)
        sprintf(buf, "кучк%s монет", plural[padis][1]);
    else if (amount <= 150)
        sprintf(buf, "больш%s кучк%s монет", plural[padis][0], plural[padis][1]);
    else if (amount <= 250)
        sprintf(buf, "груд%s монет", plural[padis][2]);
    else if (amount <= 500)
        sprintf(buf, "больш%s груд%s монет", plural[padis][0], plural[padis][2]);
    else if (amount <= 750)
        sprintf(buf, "горк%s монет", plural[padis][1]);
    else if (amount <= 1000)
        sprintf(buf, "гор%s монет", plural[padis][2]);
    else
        sprintf(buf, "огромн%s гор%s монет", plural[padis][0], plural[padis][2]);

    return (buf);
}

char *money_xdesc(int amount)
{
    static char buf[128];

    if (amount <= 0) {
        log("SYSERR: Try to create negative or 0 money (%d).", amount);
        return (NULL);
    }
    if (amount == 1)
        sprintf(buf, "одн(а,ой,ой,у,ой,ой) монет(а,ы,е,у,ой,е)");
    else if (amount <= 5)
        sprintf(buf, "малюсеньк(ая) горстк(а) монет");
    else if (amount <= 10)
        sprintf(buf, "маленьк(ая,ую,ой,ую,ой,ой) горстк(а,у,е,у,ой,е) монет");
    else if (amount <= 20)
        sprintf(buf, "небольш(ая,ую,ой,ую,ой,ой) горстк(а,у,е,у,ой,е) монет");
    else if (amount <= 50)
        sprintf(buf, "маленьк(ая,ую,ой,ую,ой,ой) кучк(а,у,е,у,ой,е) монет");
    else if (amount <= 75)
        sprintf(buf, "небольш(ая,ую,ой,ую,ой,ой) кучк(а,у,е,у,ой,е) монет");
    else if (amount <= 100)
        sprintf(buf, "кучк(а,у,е,у,ой,е) монет");
    else if (amount <= 150)
        sprintf(buf, "больш(ая) кучк(а,у,е,у,ой,е) монет");
    else if (amount <= 250)
        sprintf(buf, "груд(а) монет");
    else if (amount <= 500)
        sprintf(buf, "больш(ая) груд(а) монет");
    else if (amount <= 750)
        sprintf(buf, "горк(а) монет");
    else if (amount <= 1000)
        sprintf(buf, "гор(а) монет");
    else
        sprintf(buf, "огром(ая) гор(а) монет");

    return (buf);
}


struct obj_data *create_money(int amount)
{
    int i;
    struct obj_data *obj;
    struct extra_descr_data *new_descr;
    char buf[200];

    if (amount <= 0) {
        log("SYSERR: Try to create negative or 0 money. (%d)", amount);
        return (NULL);
    }
    obj = create_obj();
    CREATE(new_descr, struct extra_descr_data, 1);

    if (amount == 1) {
        sprintf(buf, "coin звенелка монета");
        obj->name = str_dup(buf);
        obj->names = str_dup("монет(а)");
        obj->short_description = str_dup("монета");
        obj->description = str_dup("Одна монета лежит здесь.");
        new_descr->keyword = str_dup("coin gold монет кун денег");
        new_descr->description = str_dup("Всего лишь одна монета.");
        for (i = 0; i < NUM_PADS; i++)
            obj->PNames[i] = str_dup(money_desc(amount, i));
    } else {
        sprintf(buf, "coins звенелок звенелки монеты монет %s", money_desc(amount, 0));
        obj->name = str_dup(buf);
        obj->names = str_dup(money_xdesc(amount));
        obj->short_description = str_dup(money_desc(amount, 0));
        for (i = 0; i < NUM_PADS; i++)
            obj->PNames[i] = str_dup(money_desc(amount, i));

        sprintf(buf, "Здесь лежит %s.", money_desc(amount, 0));
        obj->description = str_dup(CAP(buf));

        new_descr->keyword = str_dup("coins gold монет денег");
    }

    new_descr->next = NULL;
    obj->ex_description = new_descr;

    GET_OBJ_TYPE(obj) = ITEM_MONEY;
    GET_OBJ_WEAR(obj) = ITEM_WEAR_TAKE;
    GET_OBJ_SEX(obj) = SEX_FEMALE;
    GET_OBJ_VAL(obj, 0) = amount;
    GET_OBJ_COST(obj) = amount;
    GET_OBJ_MAX(obj) = 100;
    GET_OBJ_CUR(obj) = 100;
    GET_OBJ_TIMER(obj) = 43200;
    GET_OBJ_WEIGHT(obj) = MAX(1, amount / 100);
    GET_OBJ_EXTRA(obj, ITEM_NODONATE) |= ITEM_NODONATE;
    GET_OBJ_EXTRA(obj, ITEM_NOSELL) |= ITEM_NOSELL;

    obj->item_number = NOTHING;

    return (obj);
}


/* Generic Find, designed to find any object/character
 *
 * Calling:
 *  *arg     is the pointer containing the string to be searched for.
 *           This string doesn't have to be a single word, the routine
 *           extracts the next word itself.
 *  bitv..   All those bits that you want to "search through".
 *           Bit found will be result of the function
 *  *ch      This is the person that is trying to "find"
 *  **tar_ch Will be NULL if no character was found, otherwise points
 * **tar_obj Will be NULL if no object was found, otherwise points
 *
 * The routine used to return a pointer to the next word in *arg (just
 * like the one_argument routine), but now it returns an integer that
 * describes what it filled in.
 */
int generic_find(char *arg, bitvector_t bitvector, struct char_data *ch,
                 struct char_data **tar_ch, struct obj_data **tar_obj)
{
    int i, found, number;
    char name_val[MAX_INPUT_LENGTH];
    char *name = name_val;

    *tar_ch = NULL;
    *tar_obj = NULL;

    one_argument(arg, name);

    if (!*name)
        return (0);
    if (!(number = get_number(&name)))
        return (0);

    if (IS_SET(bitvector, FIND_CHAR_ROOM)) {    /* Find person in room */
        if ((*tar_ch = get_char_room_vis2(ch, name, &number)) != NULL)
            return (FIND_CHAR_ROOM);
    }

    /*  if (IS_SET(bitvector, FIND_CHAR_ROOM))
       {// Find person in room
       if ((*tar_ch = get_char_vis(ch, name, FIND_CHAR_ROOM)) != NULL)
       return (FIND_CHAR_ROOM);
       } */

    //////////////////////////////////////
    if (IS_SET(bitvector, FIND_CHAR_WORLD)) {
        if ((*tar_ch = get_char_vis(ch, name, FIND_CHAR_WORLD)) != NULL)
            return (FIND_CHAR_WORLD);
    }
    /////////////////////////////////////
    if (IS_SET(bitvector, FIND_OBJ_EQUIP)) {
        for (found = FALSE, i = 0; i < NUM_WEARS && !found; i++)
            if (GET_EQ(ch, i) && isname(name, GET_EQ(ch, i)->name) && --number == 0) {
                *tar_obj = GET_EQ(ch, i);
                found = TRUE;
            }
        if (found)
            return (FIND_OBJ_EQUIP);
    }

    if (IS_SET(bitvector, FIND_OBJ_TATOO)) {
        for (found = FALSE, i = 0; i < NUM_WEARS && !found; i++)
            if (GET_TATOO(ch, i) && isname(name, GET_TATOO(ch, i)->name) && --number == 0) {
                *tar_obj = GET_TATOO(ch, i);
                found = TRUE;
            }
        if (found)
            return (FIND_OBJ_EQUIP);
    }

    if (IS_SET(bitvector, FIND_OBJ_INV)) {
        if ((*tar_obj = get_obj_in_list_vis2(ch, name, &number, ch->carrying)) != NULL)
            return (FIND_OBJ_INV);
    }

    if (IS_SET(bitvector, FIND_OBJ_ROOM)) {
        if ((*tar_obj =
             get_obj_in_list_vis2(ch, name, &number, world[IN_ROOM(ch)].contents)) != NULL)
            return (FIND_OBJ_ROOM);
    }

    /*if (IS_SET(bitvector, FIND_OBJ_INV))
       {if ((*tar_obj = get_obj_in_list_vis(ch, name, ch->carrying)) != NULL)
       return (FIND_OBJ_INV);
       }

       if (IS_SET(bitvector, FIND_OBJ_ROOM))
       {if ((*tar_obj = get_obj_in_list_vis(ch, name, world[ch->in_room].contents)) != NULL)
       return (FIND_OBJ_ROOM);
       } */

    if (IS_SET(bitvector, FIND_OBJ_WORLD)) {
        if ((*tar_obj = get_obj_vis(ch, name)))
            return (FIND_OBJ_WORLD);
    }
    return (0);
}


/* a function to scan for "all" or "all.x" */
int find_all_dots(char *arg)
{
    if (!str_cmp(arg, "all") || !str_cmp(arg, "все"))
        return (FIND_ALL);
    else if (!strn_cmp(arg, "all.", 4) || !strn_cmp(arg, "все.", 4)) {
        strcpy(arg, arg + 4);
        return (FIND_ALLDOT);
    } else
        return (FIND_INDIV);
}


int preequip_tatoo(struct char_data *ch, struct obj_data *obj, int pos)
{
    int was_lgt = AFF_FLAGGED(ch, AFF_SINGLELIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hlgt = AFF_FLAGGED(ch, AFF_HOLYLIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hdrk = AFF_FLAGGED(ch, AFF_HOLYDARK) ? LIGHT_YES : LIGHT_NO, was_lamp = FALSE;

    if (pos < 0 || pos >= NUM_WEARS) {
        log("ОШИБКА: татуировка(%s,%d) в неивестной позиции", GET_NAME(ch), pos);
        return (FALSE);
    }

    if (GET_TATOO(ch, pos)) {
        log("ОШИБКА: Персонаж уже с татуировкой: %s, %s", GET_NAME(ch), obj->short_description);
        return (FALSE);
    }
    if (obj->carried_by) {
        log("ОШИБКА: Татуировка: %s уже наколота.", OBJN(obj, ch, 0));
        return (FALSE);
    }
    if (obj->in_room != NOWHERE) {
        log("ОШИБКА: Татуировка: %s IN_ROOM неизвестен.", OBJN(obj, ch, 0));
        return (FALSE);
    }

    if (invalid_anti_class(ch, obj) || invalid_no_class(ch, obj)) {
        act("$o не подходит для Вас.", FALSE, ch, obj, 0, TO_CHAR);
        obj_to_char(obj, ch);
        return (FALSE);
    }

    GET_TATOO(ch, pos) = obj;
    obj->worn_by = ch;
    obj->worn_on = pos;
    obj->next_content = NULL;

    if (ch->in_room == NOWHERE)
        log("SYSERR: ch->in_room = NOWHERE when equipping char %s.", GET_NAME(ch));
    /*  else
       IN_ROOM(obj) = IN_ROOM(ch);   */

    affect_total(ch);
    check_light(ch, was_lamp, was_lgt, was_hlgt, was_hdrk, 1);
    return (TRUE);
}

struct obj_data *unequip_tatoo(struct char_data *ch, int pos)
{
    int was_lgt = AFF_FLAGGED(ch, AFF_SINGLELIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hlgt = AFF_FLAGGED(ch, AFF_HOLYLIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hdrk = AFF_FLAGGED(ch, AFF_HOLYDARK) ? LIGHT_YES : LIGHT_NO, was_lamp = FALSE;

    int skip_total = IS_SET(pos, 0x80);
    struct obj_data *obj;
    struct C_obj_affected_type *af;

    //struct affected_type *af, *next;
    REMOVE_BIT(pos, (0x80 | 0x40));

    if ((pos < 0 || pos >= NUM_WEARS) || GET_TATOO(ch, pos) == NULL) {
        log("SYSERR: unequip_tatoo(%s,%d) - unused pos or no equip...", GET_NAME(ch), pos);
        // core_dump();
        return (NULL);
    }

    obj = GET_TATOO(ch, pos);
    obj->worn_by = NULL;
    obj->next_content = NULL;
    obj->worn_on = -1;
    GET_TATOO(ch, pos) = NULL;

    log("ОТЛАДКА: UEQ_TATTO ch->in_room %d", ch->in_room);

    if (ch->in_room == NOWHERE)
        log("SYSERR: ch->in_room = NOWHERE when unequipping_tatoo char %s.", GET_NAME(ch));

    for (af = obj->C_affected; af; af = af->next)
        affect_modify(ch, af->location, af->modifier, af->bitvector, FALSE);


    if (IN_ROOM(ch) != NOWHERE)

        if (!skip_total) {      //log("[UNEQUIP_CHAR->AFFECT_TOTAL] Start");
            affect_total(ch);
            //log("[UNEQUIP_CHAR->AFFECT_TOTAL] Stop");
            check_light(ch, was_lamp, was_lgt, was_hlgt, was_hdrk, 1);
        }

    IN_ROOM(obj) = NOWHERE;
    return (obj);
}

void equip_tatoo(struct char_data *ch, struct obj_data *obj, int pos)
{
    int was_lgt = AFF_FLAGGED(ch, AFF_SINGLELIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hlgt = AFF_FLAGGED(ch, AFF_HOLYLIGHT) ? LIGHT_YES : LIGHT_NO,
        was_hdrk = AFF_FLAGGED(ch, AFF_HOLYDARK) ? LIGHT_YES : LIGHT_NO, was_lamp = FALSE;
    int skip_total = IS_SET(pos, 0x80);
    struct C_obj_affected_type *af;

    REMOVE_BIT(pos, (0x80 | 0x40));

    if (pos < 0 || pos >= NUM_WEARS) {
        log("SYSERR: equip_char(%s,%d) in unknown pos...", GET_NAME(ch), pos);
        // core_dump();
        return;
    }

    if (GET_TATOO(ch, pos)) {
        log("SYSERR: Char is already equipped: %s, %s", GET_NAME(ch), obj->short_description);
        return;
    }
    if (obj->carried_by) {
        log("SYSERR: EQUIPTAT: %s - Obj is carried_by when equip.", OBJN(obj, ch, 0));
        return;
    }
    if (obj->in_room != NOWHERE) {
        log("SYSERR: EQUIPTAT: %s - Obj is in_room when equip.", OBJN(obj, ch, 0));
        return;
    }

    if (!IS_NPC(ch) && (invalid_no_class(ch, obj) || invalid_anti_class(ch, obj))) {
        act("$o3 явно не предназначен$A для Вас.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n попытал$u надеть $o3, но у н$s ничего не получилось.", FALSE, ch, obj, 0, TO_ROOM);
        obj_to_char(obj, ch);
        return;
    }
    GET_TATOO(ch, pos) = obj;
    obj->worn_by = ch;
    obj->worn_on = pos;
    obj->next_content = NULL;

    if (ch->in_room == NOWHERE)
        log("SYSERR: ch->in_room = NOWHERE when equipping tatoo char %s.", GET_NAME(ch));
    /*  else
       IN_ROOM(obj) = IN_ROOM(ch); */

    for (af = obj->C_affected; af; af = af->next)
        affect_modify(ch, af->location, af->modifier, af->bitvector, TRUE);


    if (IN_ROOM(ch) != NOWHERE)

        if (!skip_total) {      //log("[EQUIP_CHAR(%s)->AFFECT_TOTAL] Start",GET_NAME(ch));
            affect_total(ch);
            //log("[EQUIP_CHAR(%s)->AFFECT_TOTAL] Stop",GET_NAME(ch));
            check_light(ch, was_lamp, was_lgt, was_hlgt, was_hdrk, 1);
        }
}


void sset_affects(struct char_data *ch, struct set_variante_data *vrnt)
{
    int i;

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_BLIND), AFF_BLIND))  //СЛЕПОТА
        affect_modify(ch, APPLY_NONE, 0, AFF_BLIND, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_INVISIBLE), AFF_INVISIBLE))  //НЕВИДИМОСТЬ
        affect_modify(ch, APPLY_NONE, 0, AFF_INVISIBLE, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DETECT_ALIGN), AFF_DETECT_ALIGN))    //ЗНАНИЕ_НАКЛОНОСТЕЙ
        affect_modify(ch, APPLY_NONE, 0, AFF_DETECT_ALIGN, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DETECT_INVIS), AFF_DETECT_INVIS))    //ВИДЕТЬ_НЕВИДИМОЕ
        affect_modify(ch, APPLY_NONE, 0, AFF_DETECT_INVIS, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DETECT_MAGIC), AFF_DETECT_MAGIC))    //ВИДЕТЬ_МАГИЮ
        affect_modify(ch, APPLY_NONE, 0, AFF_DETECT_MAGIC, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_SENSE_LIFE), AFF_SENSE_LIFE))        //ЧУВСТВОВАТЬ_ЖИЗНЬ
        affect_modify(ch, APPLY_NONE, 0, AFF_SENSE_LIFE, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_LEVIT), AFF_LEVIT))  //ЛЕВИТАЦИЯ
        affect_modify(ch, APPLY_NONE, 0, AFF_LEVIT, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_WATERWALK), AFF_WATERWALK))  //ХОЖДЕНИЕ_ПО_ВОДЕ
        affect_modify(ch, APPLY_NONE, 0, AFF_WATERWALK, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_INFRAVISION), AFF_INFRAVISION))      //ИНФРАЗРЕНИЕ
        affect_modify(ch, APPLY_NONE, 0, AFF_INFRAVISION, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_PROTECT_EVIL), AFF_PROTECT_EVIL))    //ЗАЩИТА_ОТ_ЗЛА
        affect_modify(ch, APPLY_NONE, 0, AFF_PROTECT_EVIL, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_PROTECT_GOOD), AFF_PROTECT_GOOD))    //ЗАЩИТА_ОТ_ДОБРА
        affect_modify(ch, APPLY_NONE, 0, AFF_PROTECT_GOOD, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_NOTRACK), AFF_NOTRACK))      //НЕ_ВЫСЛЕДИТЬ
        affect_modify(ch, APPLY_NONE, 0, AFF_NOTRACK, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_FLY), AFF_FLY))      //ПОЛЕТ
        affect_modify(ch, APPLY_NONE, 0, AFF_FLY, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_SIELENCE), AFF_SIELENCE))    //НЕМОЙ
        affect_modify(ch, APPLY_NONE, 0, AFF_SIELENCE, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_AWARNESS), AFF_AWARNESS))    //НАСТОРОЖЕН
        affect_modify(ch, APPLY_NONE, 0, AFF_AWARNESS, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_HOLYLIGHT), AFF_HOLYLIGHT))  //СВЕТ
        affect_modify(ch, APPLY_NONE, 0, AFF_HOLYLIGHT, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_HOLYDARK), AFF_HOLYDARK))    //ТЬМА
        affect_modify(ch, APPLY_NONE, 0, AFF_HOLYDARK, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DETECT_POISON), AFF_DETECT_POISON))  //ОПРЕДЕЛЕНИЕ_ЯДА
        affect_modify(ch, APPLY_NONE, 0, AFF_DETECT_POISON, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_WATERBREATH), AFF_WATERBREATH))      //ДЫХАНИЕ_ВОДОЙ
        affect_modify(ch, APPLY_NONE, 0, AFF_WATERBREATH, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DARKVISION), AFF_DARKVISION))        //НОЧНОЕ_ЗРЕНИЕ
        affect_modify(ch, APPLY_NONE, 0, AFF_DARKVISION, TRUE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DEAFNESS), AFF_DEAFNESS))    //ГЛУХОТА
        affect_modify(ch, APPLY_NONE, 0, AFF_DEAFNESS, TRUE);

    for (i = 0; i < (int) vrnt->addons.size(); i++)
        affect_modify(ch, vrnt->addons[i].location, vrnt->addons[i].modifier, 0, TRUE);

    for (i = 0; i < (int) vrnt->skills.size(); i++)
        skill_modify(ch, vrnt->skills[i].location, vrnt->skills[i].modifier);

}

void unsset_affects(struct char_data *ch, struct set_variante_data *vrnt)
{
    if (IS_SET(GET_FLAG(vrnt->affects, AFF_LEVIT), AFF_LEVIT))  //ЛЕВИТАЦИЯ
        affect_modify(ch, APPLY_NONE, 0, AFF_LEVIT, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_BLIND), AFF_BLIND))  //СЛЕПОТА
        affect_modify(ch, APPLY_NONE, 0, AFF_BLIND, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_INVISIBLE), AFF_INVISIBLE))  //НЕВИДИМОСТЬ
        affect_modify(ch, APPLY_NONE, 0, AFF_INVISIBLE, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DETECT_ALIGN), AFF_DETECT_ALIGN))    //ЗНАНИЕ_НАКЛОНОСТЕЙ
        affect_modify(ch, APPLY_NONE, 0, AFF_DETECT_ALIGN, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DETECT_INVIS), AFF_DETECT_INVIS))    //ВИДЕТЬ_НЕВИДИМОЕ
        affect_modify(ch, APPLY_NONE, 0, AFF_DETECT_INVIS, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DETECT_MAGIC), AFF_DETECT_MAGIC))    //ВИДЕТЬ_МАГИЮ
        affect_modify(ch, APPLY_NONE, 0, AFF_DETECT_MAGIC, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_SENSE_LIFE), AFF_SENSE_LIFE))        //ЧУВСТВОВАТЬ_ЖИЗНЬ
        affect_modify(ch, APPLY_NONE, 0, AFF_SENSE_LIFE, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_LEVIT), AFF_LEVIT))  //ЛЕВИТАЦИЯ
        affect_modify(ch, APPLY_NONE, 0, AFF_LEVIT, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_WATERWALK), AFF_WATERWALK))  //ХОЖДЕНИЕ_ПО_ВОДЕ
        affect_modify(ch, APPLY_NONE, 0, AFF_WATERWALK, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_INFRAVISION), AFF_INFRAVISION))      //ИНФРАЗРЕНИЕ
        affect_modify(ch, APPLY_NONE, 0, AFF_INFRAVISION, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_PROTECT_EVIL), AFF_PROTECT_EVIL))    //ЗАЩИТА_ОТ_ЗЛА
        affect_modify(ch, APPLY_NONE, 0, AFF_PROTECT_EVIL, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_PROTECT_GOOD), AFF_PROTECT_GOOD))    //ЗАЩИТА_ОТ_ДОБРА
        affect_modify(ch, APPLY_NONE, 0, AFF_PROTECT_GOOD, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_NOTRACK), AFF_NOTRACK))      //НЕ_ВЫСЛЕДИТЬ
        affect_modify(ch, APPLY_NONE, 0, AFF_NOTRACK, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_FLY), AFF_FLY))      //ПОЛЕТ
        affect_modify(ch, APPLY_NONE, 0, AFF_FLY, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_SIELENCE), AFF_SIELENCE))    //НЕМОЙ
        affect_modify(ch, APPLY_NONE, 0, AFF_SIELENCE, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_AWARNESS), AFF_AWARNESS))    //НАСТОРОЖЕН
        affect_modify(ch, APPLY_NONE, 0, AFF_AWARNESS, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_HOLYLIGHT), AFF_HOLYLIGHT))  //СВЕТ
        affect_modify(ch, APPLY_NONE, 0, AFF_HOLYLIGHT, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_HOLYDARK), AFF_HOLYDARK))    //ТЬМА
        affect_modify(ch, APPLY_NONE, 0, AFF_HOLYDARK, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DETECT_POISON), AFF_DETECT_POISON))  //ОПРЕДЕЛЕНИЕ_ЯДА
        affect_modify(ch, APPLY_NONE, 0, AFF_DETECT_POISON, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_WATERBREATH), AFF_WATERBREATH))      //ДЫХАНИЕ_ВОДОЙ
        affect_modify(ch, APPLY_NONE, 0, AFF_WATERBREATH, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DARKVISION), AFF_DARKVISION))        //НОЧНОЕ_ЗРЕНИЕ
        affect_modify(ch, APPLY_NONE, 0, AFF_DARKVISION, FALSE);

    if (IS_SET(GET_FLAG(vrnt->affects, AFF_DEAFNESS), AFF_DEAFNESS))    //ГЛУХОТА
        affect_modify(ch, APPLY_NONE, 0, AFF_DEAFNESS, FALSE);
}

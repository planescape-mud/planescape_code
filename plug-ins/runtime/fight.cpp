/* ************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
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
#include "handler.h"
#include "interpreter.h"
#include "db.h"
#include "spells.h"
#include "screen.h"
#include "constants.h"
#include "mobmax.h"
#include "pk.h"
#include "xspells.h"
#include "sp_define.h"
#include "ai.h"
#include "xskills.h"
#include "events.h"
#include "xquests.h"
#include "xwld.h"
#include "xboot.h"
#include "fight.h"

/* External procedures */
bool check_victim_may_moved(struct char_data *ch, struct char_data *victim);
void add_victim_may_attack(struct char_data *ch, struct char_data *victim);
void add_victim_may_moved(struct char_data *ch, struct char_data *victim);

int go_guard(struct char_data *ch, struct char_data *vict, struct char_data *tmp_ch);

ACMD(do_flee);
ACMD(do_stand);
ACMD(do_wake);

/* local functions */
void spec_weapon(struct char_data *ch, struct char_data *victim, struct obj_data *weapObj);
void magic_damage_from_char(struct char_data *ch);
void dam_message(int dam, struct char_data *ch, struct char_data *victim, int w_type, int hitloc);
void make_corpse(struct char_data *ch, struct char_data *killer);
void change_alignment(struct char_data *ch, struct char_data *victim);
void death_cry(struct char_data *ch, struct char_data *victim);
void kill_gain(struct char_data *ch, struct char_data *victim, int koef, int maxlevel);
void perform_violence(void);
void end_battle_round(void);
int compute_ac_wear(struct char_data *ch);
int compute_ac_magic(struct char_data *ch, struct char_data *victim);
void go_switch_victim(struct char_data *ch, struct char_data *vict);
int check_block(struct char_data *ch, struct char_data *victim, int dam, int type, int rTyp);
int check_parry(struct char_data *ch, struct char_data *victim, int dam, int type);
int check_deviate(struct char_data *ch, struct char_data *victim, int dam, int type);


#define BL_NONE  (1 << 0)
#define BL_FAR  (1 << 1)
#define BL_DUAL  (1 << 2)
#define BL_IGNORE       (1 << 3)
#define BL_NODEVIATE    (1 << 4)



void go_ret(struct char_data *ch)
{
    struct char_data *victim = NULL;
    bool found = FALSE;

    for (victim = combat_list; victim; victim = victim->next_fighting)
        if (FIGHTING(victim) == ch && check_distance(ch, victim) == DIST_0) {
            found = TRUE;
            act("1и отсутпил1(,а,о,и) от Вас на несколько шагов.", "мМ", ch, victim);
            add_distance(ch, victim, DIST_1, FALSE);
            add_distance(victim, ch, DIST_1, FALSE);
            //GET_WAIT_STATE(victim) -= PULSE_VIOLENCE;
        }

    if (!found)
        send_to_charf(ch, "Рядом с Вами нет противников ближнего боя.\r\n");
    else {
        act("Вы отсупили на несколько шагов от своих противников.", "М", ch);
        act("1и отсутпил1(,а,о,и) на несколько шагов от своих противников.", "Км", ch);
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    }
};

void go_ats(struct char_data *ch, struct char_data *victim)
{

    if (check_distance(ch, victim) == DIST_0) {
        send_to_charf(ch, "Вы уже в ближнем бою со своим противником.\r\n");
        return;
    }

    if (GET_POS(ch) == POS_FLYING ||
        (IS_NPC(ch) && (GET_MOVE_TYPE(ch) == MOVE_FLY || NPC_FLAGGED(ch, NPC_MOVEFLY)))) {
        act("Вы подлетели ближе к 2д.", "Мм", ch, victim);
        act("1и подлетел1(,а,о,и) к Вам ближе.", "мМ", ch, victim);
        act("1и подлетел1(,а,о,и) ближе к 2д.", "Кмм", ch, victim);
    } else {
        act("Вы подошли ближе к 2д.", "Мм", ch, victim);
        act("1и подош1(ел,ла,ло,ли) к Вам ближе.", "мМ", ch, victim);
        act("1и подош1(ел,ла,ло,ли) ближе к 2д.", "Кмм", ch, victim);
    }
    add_distance(ch, victim, DIST_0, FALSE);
    add_distance(victim, ch, DIST_0, FALSE);
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
// GET_WAIT_STATE(victim) -= PULSE_VIOLENCE;
};

void clear_battle_affect(struct char_data *ch)
{
    CLR_AF_BATTLE(ch, EAF_PARRY);
    CLR_AF_BATTLE(ch, EAF_BLOCK);
    CLR_AF_BATTLE(ch, EAF_DEVIATE);
    CLR_AF_BATTLE(ch, EAF_MULTYPARRY);
};

void remove_last_attack(struct char_data *ch)
{
    ch->Ldamage.clear();
}

// проверка тренировок на игроках
int check_need_train(struct char_data *vict, int skill_no)
{
    int result;

    if (GET_POS(vict) <= POS_STUNNED && IS_NPC(vict))
        return FALSE;

    if (IS_AFFECTED(vict, AFF_HOLD) || IS_AFFECTED(vict, AFF_STOPFIGHT))
        return FALSE;

    if (IS_NPC(vict))
        return TRUE;
    if (skill_info[skill_no].skill_type == SKILL_TMAGIC)
        return TRUE;
    if (skill_info[skill_no].skill_type == SKILL_TPRIEST)
        return TRUE;

    switch (skill_no) {
        case SKILL_PICK_LOCK:
        case SKILL_RESCUE:
        case SKILL_GUARD:
        case SKILL_FIRE:
        case SKILL_MAKETRAP:
        case SKILL_FIND:
        case SKILL_AID:
        case SKILL_RDANCE:
        case SKILL_HOLYLIGHT:
        case SKILL_TRACKON:
        case SKILL_TRACK:
        case SKILL_COURAGE:
        case SKILL_PRAY:
        case SKILL_HIDETRACK:
        case SKILL_CRASHDOOR:
        case SKILL_SP_ENCHANTMENT:
        case SKILL_SP_DIVINITION:
        case SKILL_SP_NECROMANCE:
        case SKILL_SP_ALTERATION:
        case SKILL_SP_INVOCATION:
        case SKILL_SP_CONJURATION:
        case SKILL_SP_ILLUSION:
        case SKILL_SP_ABJURATION:
        case SKILL_SP_KINDNESS:
        case SKILL_SP_HEALING:
        case SKILL_SP_LIGHT:
        case SKILL_SP_WAR:
        case SKILL_SP_PROTECTION:
        case SKILL_SP_NATURE:
        case SKILL_SP_SUCCESS:
        case SKILL_SP_TRAVEL:
        case SKILL_SP_FORCE:
        case SKILL_SP_MAGIC:
        case SKILL_SP_EVIL:
        case SKILL_SP_DEATH:
        case SKILL_SP_DESTRUCTION:
            result = TRUE;
            break;
        default:
            result = FALSE;
            break;
    }

    return result;
}

//тренировка умений по новым формулам
//возврашает 1 если была тренировка, 0 если нет
int improove_skill(struct char_data *ch, struct char_data *victim, int tlevel, int skill_no)
{
    int ii, ni = 1;
    char buf[MAX_STRING_LENGTH];

    if (IS_MOB(ch))
        return (FALSE);

    if (GET_SKILL(ch, skill_no) <= 0)
        return (FALSE);

    if (victim) {
        if (!check_need_train(victim, skill_no))
            return (FALSE);
    }

    if (GET_SKILL(ch, skill_no) > skill_info[skill_no].max_percent)
        return (FALSE);

    ni = calc_improove_skill(ch, skill_no, victim, tlevel);
    ii = dice(1, ni);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&KТренировка '%s' шанс 1 к %d выпало %d должно %d флаг %s&n\r\n",
                      skill_info[skill_no].name.c_str(), ni, ii, ni / 2, GET_IMPROOVE(ch,
                                                                                      skill_no) ?
                      "-" : "+");
//  send_to_charf(ch,"Нид %d vs %d\r\n",GET_SKILL(ch,skill_no) , calc_need_improove(ch,get_skill_class_level(ch,skill_no)));
    if (ii == ni / 2
        && GET_SKILL(ch, skill_no) < calc_need_improove(ch, get_skill_class_level(ch, skill_no))
        && !GET_IMPROOVE(ch, skill_no)) {
        GET_IMPROOVE(ch, skill_no) = TRUE;
        if (fmod(GET_SKILL(ch, skill_no), 10)) {
            sprintf(buf, "%sВы повысили навык владения умением \"%s\".%s\r\n", CCICYN(ch, C_NRM),
                    skill_name(skill_no), CCNRM(ch, C_NRM));
            send_to_char(buf, ch);
            SET_SKILL(ch, skill_no)++;
            if (!fmod(GET_SKILL(ch, skill_no), 10)) {
                sprintf(buf,
                        "%sВам следует посетить гильдию для дальнейшего изучения умения \"%s\".%s\r\n",
                        CCIGRN(ch, C_NRM), skill_name(skill_no), CCNRM(ch, C_NRM));
                send_to_char(buf, ch);
            }
        }
        return (TRUE);
    }

    return (FALSE);
}

/* The Fight related routines */
int compute_armour_wear(struct char_data *ch, int type, int hitloc)
{
    int armour = 0, arm = 0;
    struct obj_data *obj;

//type=type;//броня всегда от режущего потом поменять когда будет нормальная броня
    type--;
    switch (type) {
        case ARM_SLASH:
            armour += GET_ARMOUR(ch, ARM_SLASH);
            break;
        case ARM_PICK:
            armour += GET_ARMOUR(ch, ARM_PICK);
            break;
        case ARM_BLOW:
            armour += GET_ARMOUR(ch, ARM_BLOW);
            break;
    }

    if ((GET_EQ(ch, hitloc) != NULL) && (GET_OBJ_TYPE(GET_EQ(ch, hitloc)) == ITEM_ARMOR)) {
        obj = GET_EQ(ch, hitloc);

        arm = (GET_OBJ_VAL(obj, type + 1) * GET_OBJ_CUR(obj) / MAX(1, GET_OBJ_MAX(obj)));
        //armour += MIN(arm,max_armor[(int)GET_LEVEL(ch)]);
        //send_to_charf(ch,"arm %d armour %d %s\r\n",arm,armour,GET_OBJ_PNAME(obj,0));
    }

//Добавляем щит
    /* Убрали так как поглощение щитом будем считать через блок
       if (GET_EQ(ch,WEAR_SHIELD) != NULL && number(1,20) > 10)
       {
       obj = GET_EQ(ch, WEAR_SHIELD);
       switch (hitloc)
       {
       case WEAR_BODY:
       arm = (GET_OBJ_VAL(obj,type+1) * GET_OBJ_CUR(obj)/MAX(1,GET_OBJ_MAX(obj)));
       break;
       case WEAR_ARMS:
       if (GET_OBJ_WEIGHT(obj) > 10)
       arm = (GET_OBJ_VAL(obj,type+1) * GET_OBJ_CUR(obj)/MAX(1,GET_OBJ_MAX(obj)));
       break;
       case WEAR_LEGS:
       if (GET_OBJ_WEIGHT(obj) > 18)
       arm = (GET_OBJ_VAL(obj,type+1) * GET_OBJ_CUR(obj)/MAX(1,GET_OBJ_MAX(obj)));
       break;
       case WEAR_WAIST:
       if (GET_OBJ_WEIGHT(obj) > 30)
       arm = (GET_OBJ_VAL(obj,type+1) * GET_OBJ_CUR(obj)/MAX(1,GET_OBJ_MAX(obj)));
       break;
       case WEAR_HEAD:
       if (GET_OBJ_WEIGHT(obj) > 45)
       arm = (GET_OBJ_VAL(obj,type+1) * GET_OBJ_CUR(obj)/MAX(1,GET_OBJ_MAX(obj)));
       break;
       }


       }
     */

    armour += MIN(arm, max_armor[(int) GET_LEVEL(ch)]);
//Проверка на шок
    /*
       if (affected_by_spell(ch, SPELL_GRASP))
       {
       arm = get_spell_value(ch,SPELL_GRASP);
       armour = MAX(0,armour-arm);

       if (PRF_FLAGGED(ch,PRF_CODERINFO))
       send_to_charf(ch,"&mШтраф за шок: %d Броня %d &n\r\n",arm,armour);
       }  */

    return armour;
}

int compute_ac_magic(struct char_data *ch, struct char_data *victim)
{
    struct affected_type *aff;
    int armorclass = 0;
    int type;

    if (victim->affected)
        for (aff = victim->affected; aff; aff = aff->next) {
            type = SPELL_NO(aff->type);
            switch (type) {
                case SPELL_PROTECT_FROM_EVIL:
                    if (IS_EVILS(ch))
                        armorclass += aff->modifier;
                    break;
                case SPELL_PROTECT_FROM_GOOD:
                    if (IS_GOODS(ch))
                        armorclass += aff->modifier;
                    break;
                case SPELL_PROTECT_UNDEAD:
                    if (IS_UNDEAD(ch))
                        armorclass += aff->modifier;
                    break;
            }
        }
    return (armorclass);
}


//AC на броне (0 нет, 100 маскимум)

int compute_ac_wear(struct char_data *ch)
{
    int armorclass = GET_REAL_AC(ch);

    return (MIN(armorclass, 100));
}



int update_pos(struct char_data *victim)
{
    if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
        GET_POS(victim) = GET_POS(victim);
    else if (GET_HIT(victim) > 0) {
        if (GET_POS(victim) == POS_STUNNED) {
            act("Вы пришли в себя.", FALSE, victim, 0, 0, TO_CHAR);
            act("$n приш$y в себя.", FALSE, victim, 0, 0, TO_ROOM);
            GET_POS(victim) = POS_SITTING;
        } else
            GET_POS(victim) = POS_STANDING;
    } else if (miracle_action(victim, MIRC_DIE)) {
        return (FALSE);
    } else if (IS_UNDEAD(victim) && GET_REAL_MAX_HIT(victim) <= 0) {
        GET_POS(victim) = POS_DEAD;
    } else if (GET_HIT(victim) <= -(GET_REAL_MAX_HIT(victim) / 7))
        GET_POS(victim) = POS_DEAD;
    else if (GET_HIT(victim) <= -(GET_REAL_MAX_HIT(victim) / 9))
        GET_POS(victim) = POS_MORTALLYW;
    else if (GET_HIT(victim) <= -(GET_REAL_MAX_HIT(victim) / 10))
        GET_POS(victim) = POS_INCAP;
    else
        GET_POS(victim) = POS_STUNNED;

    /*  if (AFF_FLAGGED(victim,AFF_SLEEP) &&
       GET_POS(victim) != POS_SLEEPING
       )
       affect_from_char(victim,SPELL_SLEEP); */

    if (on_horse(victim) && GET_POS(victim) < POS_SITTING)
        horse_drop(get_horse(victim));

    if (IS_HORSE(victim) && GET_POS(victim) < POS_SITTING && on_horse(victim->master))
        horse_drop(victim);

    return (TRUE);
}


void set_battle_pos(struct char_data *ch)
{

    if (GET_WAIT(ch) <= 0)
        switch (GET_POS(ch)) {
            case POS_STANDING:
                GET_POS(ch) = POS_FIGHTING;
                break;
            case POS_RESTING:
            case POS_SITTING:
            case POS_SLEEPING:
                if (!GET_MOB_HOLD(ch) && !AFF_FLAGGED(ch, AFF_SLEEP))
                    //&& !AFF_FLAGGED(ch, AFF_CHARM))

                {
                    if (AFF_FLAGGED(ch, AFF_MEDITATION)) {
                        send_to_charf(ch, "От боли Вы прекратили медитировать.\r\n");
                        act("$n прекратил медитацию от боли.", FALSE, ch, 0, 0, TO_ROOM);
                        affect_from_char(ch, SPELL_MEDITATION);
                        GET_POS(ch) = POS_SITTING;
                    } else if (IS_NPC(ch)) {
                        if (GET_POS(ch) == POS_SLEEPING) {
                            act("$n проснул$u от боли.", FALSE, ch, 0, 0, TO_ROOM);
                            //do_wake(ch,0,SCMD_WAKE,0,0);
                            GET_POS(ch) = POS_SITTING;
                        } else
                            do_stand(ch, 0, 0, 0, 0);
                    }

                    if (!IS_NPC(ch) && GET_POS(ch) == POS_SLEEPING) {
                        do_wake(ch, 0, SCMD_WAKE, 0, 0);
                    }
                }
                break;
        }
}

void restore_battle_pos(struct char_data *ch)
{
    switch (GET_POS(ch)) {
        case POS_FIGHTING:
            GET_POS(ch) = POS_STANDING;
            break;
        case POS_RESTING:
        case POS_SITTING:
        case POS_SLEEPING:
            if (IS_NPC(ch) &&
                GET_WAIT(ch) <= 0 &&
                !GET_MOB_HOLD(ch) && !AFF_FLAGGED(ch, AFF_SLEEP) && !AFF_FLAGGED(ch, AFF_CHARM)) {
                act("$n встал$g на ноги.", FALSE, ch, 0, 0, TO_ROOM);
                GET_POS(ch) = POS_STANDING;
            }
            break;
    }
    if (AFF_FLAGGED(ch, AFF_SLEEP))
        GET_POS(ch) = POS_SLEEPING;
}

/* start one char fighting another (yes, it is horrible, I know... )  */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
    if (ch == vict)
        return;

    if (FIGHTING(ch)) {
        log("SYSERR: set_fighting(%s->%s) when already fighting(%s)...",
            GET_NAME(ch), GET_NAME(vict), GET_NAME(FIGHTING(ch)));
        // core_dump();
        return;
    }

    if ((IS_NPC(ch) && MOB_FLAGGED(ch, MOB_NOFIGHT)) ||
        (IS_NPC(vict) && MOB_FLAGGED(ch, MOB_NOFIGHT)))
        return;

    // if (AFF_FLAGGED(ch,AFF_STOPFIGHT))
    //    return;

    ch->next_fighting = combat_list;
    combat_list = ch;

    /*  if (AFF_FLAGGED(ch, AFF_SLEEP))
       affect_from_char(ch, SPELL_SLEEP); */
    FIGHTING(ch) = vict;
    NUL_AF_BATTLE(ch);
    BATTLECNTR(ch) = 0;
    GET_MISSED(ch) = 0;
    *HUNT_NAME(ch) = '\0';
    HUNT_STEP(ch) = 0;
    SET_EXTRA(ch, 0, NULL);

    add_distance(ch, vict, DIST_NOTFOUND, FALSE);
    add_distance(vict, ch, DIST_NOTFOUND, FALSE);

    /*  if (GET_POS(ch) == POS_SLEEPING || GET_POS(ch) == POS_STANDING)
       set_battle_pos(ch);
     */
}

/* remove a char from the list of fighting chars */
void stop_fighting(struct char_data *ch, int switch_others)
{
    int i;
    struct char_data *temp, *found;

    if (ch == next_combat_list)
        next_combat_list = ch->next_fighting;

    REMOVE_FROM_LIST(ch, combat_list, next_fighting);
    ch->next_fighting = NULL;
    FIGHTING(ch) = NULL;
    BATTLECNTR(ch) = 0;
    GET_MISSED(ch) = 0;
    ch->damage_rnd = 0;
    ch->doh = 0;
    clean_distance(ch);

    //снимаем ограничения на изучения умения в бою
    if (!IS_NPC(ch))
        for (i = 1; i <= MAX_SKILLS; i++)
            GET_IMPROOVE(ch, i) = FALSE;


    /* Санбаем снимаем после окончания боя
       if (affected_by_spell(ch, SPELL_SUNBEAM))
       affect_from_char(ch, SPELL_SUNBEAM); */
    SET_EXTRA(ch, 0, NULL);
    SET_CAST(ch, 0, NULL, NULL);
    restore_battle_pos(ch);
    NUL_AF_BATTLE(ch);
// sprintf(buf,"[Stop fighting] %s - %s\r\n",GET_NAME(ch),switch_others ? "switching": "no switching");
// send_to_gods(buf);
    /**** switch others *****/

    for (temp = combat_list; temp; temp = temp->next_fighting) {
        if (GET_EXTRA_VICTIM(temp) == ch)
            SET_EXTRA(temp, 0, NULL);
        if (GET_CAST_CHAR(temp) == ch)
            SET_CAST(temp, 0, NULL, NULL);
        if (FIGHTING(temp) == ch && switch_others) {
            for (found = combat_list; found; found = found->next_fighting)
                if (found != ch && FIGHTING(found) == temp) {
                    act("Вы переключили свое внимание на $N3.", FALSE, temp, 0, found, TO_CHAR);
                    FIGHTING(temp) = found;
                    break;
                }
            if (!found)
                stop_fighting(temp, FALSE);
        }
    }
    update_pos(ch);
}


void make_corpse(struct char_data *ch, struct char_data *killer)
{
    struct obj_data *corpse, *o, *on;
    struct obj_data *money;
    struct list_obj_data *k;
    bool load_in_room = TRUE;
    int l;
    bool arena = FALSE;
    char buf2[MAX_STRING_LENGTH];
    std::vector < int >wloot;

#define MAT_BODY 325

    int i, vcorpse = (IS_NPC(ch) ? ch->npc()->specials.vnum_corpse : 0);

    if (IN_ROOM(ch) != NOWHERE)
        arena = ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA);

    if (IS_NPC(ch) && vcorpse == -1) {
        /*Загружаем предметы после смерти в труп */
        for (k = ch->load_death; k; k = k->next) {
            if (k->percent < number(1, 100))
                continue;
            o = read_object(k->vnum, VIRTUAL, TRUE);
            GET_OBJ_ZONE(o) = world[IN_ROOM(ch)].zone;
            if (IN_ROOM(ch) != NOWHERE)
                obj_to_room(o, IN_ROOM(ch));
            else
                extract_obj(o);
        }

        /* перемещаем экипировку в труп */
        for (i = 0; i < NUM_WEARS; i++)
            if (GET_EQ(ch, i)) {
                o = unequip_char(ch, i);

                if (i == WEAR_WIELD || i == WEAR_HOLD || i == WEAR_BOTHS)
                    if (IS_AFFECTED(ch, AFF_IMPLANT_WEAPON)) {
                        extract_obj(o);
                        continue;
                    }
                if (IN_ROOM(ch) != NOWHERE)
                    obj_to_room(o, IN_ROOM(ch));
                else
                    extract_obj(o);
            }

        /* перемещаем инвентарь в труп */
        for (o = ch->carrying; o != NULL; o = on) {
            on = o->next_content;
            obj_from_char(o);
            if (IN_ROOM(ch) != NOWHERE)
                obj_to_room(o, IN_ROOM(ch));
            else
                extract_obj(o);
        }


        /* transfer gold */
        if (GET_GOLD(ch) > 0) { /* following 'if' clause added to fix gold duplication loophole */
            if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
                money = create_money(GET_GOLD(ch));
                if (IN_ROOM(ch) != NOWHERE)
                    obj_to_room(money, IN_ROOM(ch));
                else
                    extract_obj(money);
            }
            GET_GOLD(ch) = 0;
        }

        ch->carrying = NULL;
        IS_CARRYING_N(ch) = 0;
        IS_CARRYING_W(ch) = 0;

        return;
    }

    if (!vcorpse || !(corpse = read_object(vcorpse, VIRTUAL, TRUE))) {
        corpse = create_obj();
        corpse->item_number = NOTHING;
        corpse->in_room = NOWHERE;

        if (GET_RACE(ch) == RACE_CONSTRUCTION) {
            sprintf(buf2, "обломки %s лежат здесь.", GET_PAD(ch, 1));
            corpse->description = str_dup(buf2);
            sprintf(buf2, "обломки %s", GET_PAD(ch, 1));
            corpse->short_description = str_dup(buf2);
            corpse->names = str_dup(buf2);
            sprintf(buf2, "обломки %s", GET_PAD(ch, 1));
            corpse->PNames[0] = str_dup(buf2);
            corpse->name = str_dup(buf2);
            sprintf(buf2, "обломоков %s", GET_PAD(ch, 1));
            corpse->PNames[1] = str_dup(buf2);
            sprintf(buf2, "обломкам %s", GET_PAD(ch, 1));
            corpse->PNames[2] = str_dup(buf2);
            sprintf(buf2, "обломки %s", GET_PAD(ch, 1));
            corpse->PNames[3] = str_dup(buf2);
            sprintf(buf2, "обломками %s", GET_PAD(ch, 1));
            corpse->PNames[4] = str_dup(buf2);
            sprintf(buf2, "обломках %s", GET_PAD(ch, 1));
            corpse->PNames[5] = str_dup(buf2);
            GET_OBJ_SEX(corpse) = SEX_POLY;
        } else if (GET_RACE(ch) == RACE_SKELET) {
            sprintf(buf2, "кучка костей %s рассыпана здесь.", GET_PAD(ch, 1));
            corpse->description = str_dup(buf2);
            sprintf(buf2, "кучка костей %s", GET_PAD(ch, 1));
            corpse->short_description = str_dup(buf2);
            corpse->names = str_dup(buf2);
            sprintf(buf2, "кучка костей %s", GET_PAD(ch, 1));
            corpse->PNames[0] = str_dup(buf2);
            corpse->name = str_dup(buf2);
            sprintf(buf2, "кучки костей %s", GET_PAD(ch, 1));
            corpse->PNames[1] = str_dup(buf2);
            sprintf(buf2, "кучке костей %s", GET_PAD(ch, 1));
            corpse->PNames[2] = str_dup(buf2);
            sprintf(buf2, "кучку костей %s", GET_PAD(ch, 1));
            corpse->PNames[3] = str_dup(buf2);
            sprintf(buf2, "кучками костей %s", GET_PAD(ch, 1));
            corpse->PNames[4] = str_dup(buf2);
            sprintf(buf2, "кучках костей %s", GET_PAD(ch, 1));
            corpse->PNames[5] = str_dup(buf2);
            GET_OBJ_SEX(corpse) = SEX_POLY;
        } else {
            sprintf(buf2, "труп %s лежит здесь.", GET_PAD(ch, 1));
            corpse->description = str_dup(buf2);
            sprintf(buf2, "труп %s", GET_PAD(ch, 1));
            corpse->short_description = str_dup(buf2);
            corpse->names = str_dup(buf2);
            sprintf(buf2, "труп %s", GET_PAD(ch, 1));
            corpse->PNames[0] = str_dup(buf2);
            corpse->name = str_dup(buf2);
            sprintf(buf2, "трупа %s", GET_PAD(ch, 1));
            corpse->PNames[1] = str_dup(buf2);
            sprintf(buf2, "трупу %s", GET_PAD(ch, 1));
            corpse->PNames[2] = str_dup(buf2);
            sprintf(buf2, "труп %s", GET_PAD(ch, 1));
            corpse->PNames[3] = str_dup(buf2);
            sprintf(buf2, "трупом %s", GET_PAD(ch, 1));
            corpse->PNames[4] = str_dup(buf2);
            sprintf(buf2, "трупе %s", GET_PAD(ch, 1));
            corpse->PNames[5] = str_dup(buf2);
            GET_OBJ_SEX(corpse) = SEX_MALE;
        }
    }

    GET_OBJ_TYPE(corpse) = ITEM_CORPSE;
    if (killer != NULL)
        corpse->killer = GET_ID(killer);
    GET_OBJ_VAL(corpse, 1) = GET_ID(ch);


    if (GET_OBJ_TYPE(corpse) == ITEM_CORPSE) {
        GET_OBJ_VAL(corpse, 2) = IS_NPC(ch) ? GET_MOB_VNUM(ch) : -1;
        GET_OBJ_WEAR(corpse) = ITEM_WEAR_TAKE;
        GET_OBJ_EXTRA(corpse, ITEM_NODONATE) |= ITEM_NODONATE;
        GET_OBJ_EXTRA(corpse, ITEM_NOSELL) |= ITEM_NOSELL;
        GET_OBJ_EXTRA(corpse, ITEM_NOSELL) |= ITEM_NORENT;
        GET_OBJ_VAL(corpse, 0) = GET_WEIGHT(ch);        /* You can't store stuff in a corpse */
        if (GET_RACE(ch) == RACE_SKELET)
            GET_OBJ_VAL(corpse, 3) = 4; //признак скелета
        else
            GET_OBJ_VAL(corpse, 3) = 1; /* corpse identifier */
        GET_OBJ_WEIGHT(corpse) = (GET_WEIGHT(ch) * 1000) + IS_CARRYING_W(ch);
        GET_OBJ_CUR(corpse) = GET_OBJ_MAX(corpse) = 100;
        int weight = MAX(1, (GET_WEIGHT(ch) * 1000) / get_material_param(MAT_BODY)->weight);

        add_materials_proto(corpse);
        add_material(corpse, MAT_BODY, weight, TRUE);
        /* Чумной труп */
        int pp;

        if ((pp = affected_by_spell(ch, SPELL_PLAGUE)))
            set_affect_obj(corpse, AFF_PLAGUE, pp);

        if (IS_NPC(ch) && NPC_FLAGGED(ch, NPC_PLAGUE))
            set_affect_obj(corpse, AFF_PLAGUE, GET_LEVEL(ch) * 3);

        load_in_room = FALSE;
    }

    if (IS_NPC(ch))
        GET_OBJ_TIMER(corpse) = 16;
    else
        GET_OBJ_TIMER(corpse) = 30;

    GET_OBJ_ZONE(corpse) = world[IN_ROOM(ch)].zone;

    /*Загружаем предметы после смерти в труп */
    for (k = ch->load_death; k; k = k->next) {
        if (k->percent < number(1, 100))
            continue;

        o = read_object(k->vnum, VIRTUAL, TRUE);

        if (killer)
            for (int q = 0; q < (int) o->quests.size(); q++) {
                if (check_quest(killer, o->quests[q])) {
                    GET_OBJ_ZONE(o) = world[IN_ROOM(ch)].zone;
                    if (load_in_room && IN_ROOM(ch) != NOWHERE)
                        obj_to_room(o, IN_ROOM(ch));
                    else
                        obj_to_char(o, ch);
                } else
                    extract_obj(o);
            }

        if (o && !o->quests.size()) {
            GET_OBJ_ZONE(o) = world[IN_ROOM(ch)].zone;
            if (load_in_room && IN_ROOM(ch) != NOWHERE)
                obj_to_room(o, IN_ROOM(ch));
            else
                obj_to_char(o, ch);
        }
    }

//Мировой лут
    wloot = check_world_loot(GET_MOB_VNUM(ch));
    for (l = 0; l < (int) wloot.size(); l++) {
        o = read_object(wloot[l], VIRTUAL, TRUE);
        log("WL: %d = %d", l, wloot[l]);

        if (o) {
            if (killer)
                for (int q = 0; q < (int) o->quests.size(); q++) {
                    if (check_quest(killer, o->quests[q])) {
                        GET_OBJ_ZONE(o) = world[IN_ROOM(ch)].zone;
                        if (load_in_room && IN_ROOM(ch) != NOWHERE)
                            obj_to_room(o, IN_ROOM(ch));
                        else
                            obj_to_char(o, ch);
                    } else
                        extract_obj(o);
                }


            if (!o->quests.size()) {
                GET_OBJ_ZONE(o) = world[IN_ROOM(ch)].zone;
                if (load_in_room && IN_ROOM(ch) != NOWHERE)
                    obj_to_room(o, IN_ROOM(ch));
                else
                    obj_to_char(o, ch);
            }
        }
    }


    /* перемещаем экипировку в труп */
    if (!arena)
        for (i = 0; i < NUM_WEARS; i++)
            if (GET_EQ(ch, i)) {
                o = unequip_char(ch, i);

                if (i == WEAR_WIELD || i == WEAR_HOLD || i == WEAR_BOTHS)
                    if (IS_AFFECTED(ch, AFF_IMPLANT_WEAPON)) {
                        extract_obj(o);
                        continue;
                    }
                if (load_in_room && IN_ROOM(ch) != NOWHERE)
                    obj_to_room(o, IN_ROOM(ch));
                else
                    obj_to_char(o, ch);
            }

    /* перемещаем инвентарь в труп */
    if (!arena) {
        if (load_in_room && IN_ROOM(ch) != NOWHERE) {
            for (o = ch->carrying; o != NULL; o = o->next_content) {
                if (o->carried_by == NULL)
                    continue;
                obj_from_char(o);
                obj_to_room(o, IN_ROOM(ch));
            }
        } else {
            corpse->contains = ch->carrying;
            for (o = corpse->contains; o != NULL; o = o->next_content) {
                o->in_obj = corpse;
            }

            object_list_new_owner(corpse, NULL);
        }
    }
    /* transfer gold */
    if (!arena) {
        if (GET_GOLD(ch) > 0) { /* following 'if' clause added to fix gold duplication loophole */
            if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
                money = create_money(GET_GOLD(ch));
                obj_to_obj(money, corpse);
            }
            GET_GOLD(ch) = 0;
        }
        ch->carrying = NULL;
        IS_CARRYING_N(ch) = 0;
        IS_CARRYING_W(ch) = 0;
    }


    obj_to_room(corpse, ch->in_room);

    if (IS_NPC(ch) && ch->npc()->specials.death_script > 0)
        go_script(ch->npc()->specials.death_script, killer, ch);

    /*save obj to file */
    xsave_rent(ch, RENT_NORMAL, FALSE);
}


/* When ch kills victim */
void change_alignment(struct char_data *ch, struct char_data *victim)
{
    int align_vict, align_ch;
    float koef = 0, change;

    align_vict = GET_ALIGNMENT(victim);
    align_ch = GET_ALIGNMENT(ch);

    koef = (change_align[calc_alignment(ch)][calc_alignment(victim)]);

    change = ((-align_vict / 20) * koef);
    GET_ALIGNMENT(ch) += (int) change;
}


void mob_death_hit(struct char_data *ch, struct char_data *victim)
{
    int dam;
    struct char_data *tch = NULL, *next_tch;

    if (IN_ROOM(ch) == NOWHERE)
        return;

    if (!IS_NPC(ch))
        return;

    if (ch->npc()->specials.death_flag) {
        for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
            next_tch = tch->next_in_room;
            if (PRF_FLAGGED(tch, PRF_NOHASSLE))
                continue;
            if (same_group(ch, tch))
                continue;
            if (!may_kill_here(ch, tch))
                continue;
            if (ch == tch)
                continue;

            //Достается случайному
            if (dice(1, 20) > 10 && ch->npc()->specials.death_flag == 1) {
                dam =
                    dice(ch->npc()->specials.DamageDamDice,
                         ch->npc()->specials.DamageDamSize) + ch->npc()->specials.DamageDamRoll;
                if (dam) {
                    if (ch->npc()->specials.DMessChar)
                        act(ch->npc()->specials.DMessChar, "мМ", ch, tch);
                    if (ch->npc()->specials.DMessRoom)
                        act(ch->npc()->specials.DMessRoom, "Кмм", ch, tch);

                    GET_HIT(tch) -= dam;
                    tch->damage_rnd += dam;
                    update_pos(tch);
                    if (GET_POS(tch) == POS_DEAD) {
                        death_cry(tch, NULL);
                        die(tch, NULL);
                    }
                }
                break;
            } else {            //Достается всем
                dam =
                    dice(ch->npc()->specials.DamageDamDice,
                         ch->npc()->specials.DamageDamSize) + ch->npc()->specials.DamageDamRoll;
                if (dam) {
                    if (ch->npc()->specials.DMessChar)
                        act(ch->npc()->specials.DMessChar, "мМ", ch, tch);
                    if (ch->npc()->specials.DMessRoom)
                        act(ch->npc()->specials.DMessRoom, "Кмм", ch, tch);
                    GET_HIT(tch) -= dam;
                    tch->damage_rnd += dam;
                    update_pos(tch);
                    if (GET_POS(tch) == POS_DEAD) {
                        death_cry(tch, NULL);
                        die(tch, NULL);
                    }
                }
                continue;
            }
        }
    } else if (victim) {        //достается только игроку
        if (PRF_FLAGGED(victim, PRF_NOHASSLE))
            return;
        if (!may_kill_here(ch, victim))
            return;
        dam =
            dice(ch->npc()->specials.DamageDamDice,
                 ch->npc()->specials.DamageDamSize) + ch->npc()->specials.DamageDamRoll;
        if (dam) {
            //log("DMessC %s, DMessR %s",ch->npc()->specials.DMessChar,ch->npc()->specials.DMessRoom);

            if (ch->npc()->specials.DMessChar)
                act(ch->npc()->specials.DMessChar, "мМ", ch, victim);

            if (ch->npc()->specials.DMessRoom)
                act(ch->npc()->specials.DMessRoom, "Кмм", ch, victim);

            GET_HIT(victim) -= dam;
            victim->damage_rnd += dam;
            update_pos(victim);
            if (GET_POS(victim) == POS_DEAD) {
                death_cry(victim, NULL);
                die(tch, NULL);
            }
        }
    }

    if (ch->npc()->specials.death_script && victim) {
        go_script(ch->npc()->specials.death_script, victim, ch);
    }

}

void death_cry(struct char_data *ch, struct char_data *victim)
{
    int door, in_room = IN_ROOM(ch);
    bool dd = FALSE;

    //log("[ FIGHT ] death_cry %s [%d]",GET_NAME(ch),world[in_room].number);

    GET_POS(ch) = POS_STANDING;
    GET_POS(ch) = POS_DEAD;

    if (IS_NPC(ch) && ch->npc()->specials.CMessChar)
        act(ch->npc()->specials.CMessChar, victim ? "мМ" : "м", ch, victim);
    else
        dd = TRUE;

    if (IS_NPC(ch) && ch->npc()->specials.CMessRoom)
        act(ch->npc()->specials.CMessRoom, victim ? "КМм" : "КМ", ch, victim);

    //Повреждения при смерти
    if (IS_NPC(ch) && ch->npc()->specials.d_type_hit && victim && ch != victim)
        mob_death_hit(ch, victim);

    if (dd) {
        if (IS_UNDEAD(ch) && ch->npc()->specials.vnum_corpse == -1)
            act("1и развалил1(ся,ась,ось,ись) на мелкие куски.", "Км", ch);
        else
            act("1и умер1(,ла,ло,ли).", "Км", ch);
    }


    if (in_room == NOWHERE)
        return;

    //это умер лидер группы
    if (AFF_FLAGGED(ch, AFF_GROUP) && !ch->party_leader) {
        struct follow_type *k, *next;
        struct char_data *tch = NULL;

        //поиск нового лидера
        for (k = ch->party; k && k->follower->party_leader; k = next) {
            next = k->next;
            tch = k->follower;
            if (!IS_MOB(k->follower)) {
                tch = k->follower;
                break;
            }
        }
        if (tch)
            change_leader(ch, tch, FALSE);
    }

    for (door = 0; door < NUM_OF_DIRS; door++)
        if (CAN_GO_DATA(in_room, door))
            send_to_room("Ваша кровь застыла в жилах от чьего-то предсмертного крика.\r\n",
                         world[in_room].dir_option[door]->to_room, TRUE);

//log("[ FIGHT ] Death cry - Stop");
}




//ADD BY SLOWN
/* увеличиваем счетчик крови */
void increase_blood(int rm, int blood)
{
    if (real_sector(rm) == SECT_WATER_SWIM ||
        real_sector(rm) == SECT_WATER_NOSWIM ||
        real_sector(rm) == SECT_UNDERWATER || real_sector(rm) == SECT_FLYING)
        return;

    RM_BLOOD(rm) = MIN(RM_BLOOD(rm) + blood, 10);
}


void death(struct char_data *ch)
{
    int dec_exp;

    send_to_charf(ch, "Связывавшие душу и тело нити рассеялись, освободив Вас.\r\n");

    if (IS_SOUL(ch))
        REMOVE_BIT(PLR_FLAGS(ch, PLR_SOUL), PLR_SOUL);

    if (IN_ROOM(ch) != NOWHERE && !ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA))
        if (!IS_NPC(ch)) {
            if (!(IS_NPC(ch) || IS_IMMORTAL(ch))) {
                dec_exp = get_levelexp(ch, GET_LEVEL(ch) + 1, 1);
                dec_exp = MIN(max_exp_loss_pc(ch), dec_exp);

                if (dec_exp)
                    gain_exp(ch, -dec_exp, TRUE);
            }
        }

    if (ch->followers || ch->master)
        die_follower(ch);

    save_pets(ch);

    if (IN_ROOM(ch) != NOWHERE)
        extract_char(ch, FALSE);
}

static void mprog_death(struct char_data *ch, struct char_data *killer)
{
    FENIA_VOID_CALL(ch, "Death", "C", killer);
    FENIA_PROTO_VOID_CALL(ch->npc(), "Death", "CC", ch, killer);
}

static void mprog_kill(struct char_data *ch, struct char_data *victim)
{
    FENIA_VOID_CALL(ch, "Kill", "C", victim);
    FENIA_PROTO_VOID_CALL(ch->npc(), "Kill", "CC", ch, victim);
}

//Процедура смерти персонажа
void die(struct char_data *ch, struct char_data *killer)
{
    struct char_data *tch;
    struct affected_type *af, *naf;
    int is_pk = 0, i;


    stop_fighting(ch, TRUE);

    GET_POS(ch) = POS_DEAD;

    if (!IS_NPC(ch))
        send_to_charf(ch, "Вы умерли!!!\r\n");


    if (ch->followers || ch->master)
        die_follower(ch);

    if (IN_ROOM(ch) != NOWHERE)
        make_corpse(ch, killer);

    if (!IS_NPC(ch)) {
        for (i = 1; i <= MAX_SPELLS; i++)
            GET_SPELL_MEM(ch, i) = 0;
        for (af = ch->affected; af; af = naf) {
            naf = af->next;
            if (!IS_SET(af->battleflag, AF_DEADKEEP))
                affect_remove(ch, af);
        }
        asciiflag_conv("", &AFF_FLAGS(ch, 0));
        affect_total(ch);
    }

    if (!MOB_FLAGGED(ch, MOB_CLONE) && !IS_UNDEAD(ch) && !IS_CONSTRUCTION(ch))
        increase_blood(ch->in_room, (GET_SIZE(ch) * GET_LEVEL(ch)) / 100);      // счетчик крови +

    if (!IS_NPC(ch) && killer)
        is_pk = dec_pkill_group(killer, ch);

    GET_HIT(ch) = 1;
    if (!IS_NPC(ch)) {
        if (GET_COND(ch, FULL) != -1)
            GET_COND(ch, FULL) = 24 * SECS_PER_MUD_TICK;
        if (GET_COND(ch, THIRST) != -1)
            GET_COND(ch, THIRST) = 24 * SECS_PER_MUD_TICK;
        if (GET_COND(ch, SLEEP) != -1)
            GET_COND(ch, SLEEP) = 24 * SECS_PER_MUD_TICK;
    }

    change_fighting(ch, TRUE);
    GET_FICTION(ch) = NULL;

    /* Убираем флаг-убийца у всех на кого нападал этот моб */
    if (IS_NPC(ch) && killer) {
        for (tch = character_list; tch; tch = tch->next)
            tch->pk_list.erase(GET_ID(ch));
    } else                      //Если это игрок и он умер от руки моба, то очишаем ПК-список
        // у всех мобов на этого игрока
    if (killer && IS_NPC(killer) && !IS_NPC(ch)) {
        for (tch = character_list; tch; tch = tch->next) {
            if (!IS_NPC(tch))
                continue;
            tch->pk_list.erase(GET_ID(ch));
        }
    }

    if (IN_ROOM(ch) != NOWHERE) {
        if (!IS_NPC(ch))
            FORGET_ALL(ch);

        if (!IS_NPC(ch))
            RENTABLE(ch) = 0;
    }

    if (killer)
        mprog_kill(killer, ch);

    mprog_death(ch, killer);

    if (!IS_NPC(ch) && !IS_GOD(ch)) {

        if (IN_ROOM(ch) != NOWHERE && ROOM_FLAGGED(IN_ROOM(ch), ROOM_ARENA)) {
            death(ch);
            return;
        }

        SET_BIT(PLR_FLAGS(ch, PLR_SOUL), PLR_SOUL);
        GET_POS(ch) = POS_FLYING;
        send_to_charf(ch, "Мир поблек и затих, когда Ваша душа воспарила над телом.\r\n");
        GET_WAIT(ch) = 0;
        EXTRACT_TIMER(ch) = MAX(1, GET_LEVEL(ch) / 3);
    } else
        death(ch);

}

int get_extend_exp(int exp, struct char_data *ch, struct char_data *victim)
{
    float koefr = 0;

    if (!IS_NPC(victim) || IS_NPC(ch))
        return (exp);

    //send_to_charf(ch,"Поправка на разницу %d\r\n",exp);
    //коэф на алигнмент
    koefr = exp_align[calc_alignment(ch)][calc_alignment(victim)];
    exp = (int) ((float) exp * (float) ((float) koefr / 10.0));

    return (exp);
}

void kill_gain(struct char_data *ch, struct char_data *victim, int maxlevel, int sumlevels)
{
    int exp, level;
    double getMobXP(int playerlvl, int moblvl);

    if (GET_LEVEL(ch))
        level = GET_LEVEL(ch);
    else
        level = 1;


    if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master
        && GET_LEVEL(ch) >= GET_LEVEL(ch->master))
        return;

    exp =
        (int) getPartyMobXPFull(GET_LEVEL(ch), maxlevel, sumlevels, GET_LEVEL(victim), FALSE,
                                FALSE);
//expm = getMobXP(GET_LEVEL(ch),GET_LEVEL(victim));
//exp = (int) expf;
//send_to_charf(ch,"Тест: %.2f (%.2f) | %d %d\r\n",expf,expm,maxlevel, sumlevels);
    //Вставляем обработку на квесты
    if (IS_NPC(victim))
        set_mob_quest(ch, GET_MOB_VNUM(victim));

    if (!OK_GAIN_EXP(ch, victim)) {     //send_to_char("Не стоило убивать.\r\n",ch);
        send_to_char("Ваш опыт не изменился.\r\n", ch);
        return;
    }

    exp = get_extend_exp(exp, ch, victim);
    //send_to_charf(ch,"Опыт до изменения %d ",exp);

    //exp = MIN(max_exp_gain_pc(ch), exp);
    //send_to_charf(ch,"Опыт после изменения %d\r\n",exp);

    if (exp)
        exp = exp / get_flee(victim, GET_ID(ch));

    gain_exp(ch, exp, TRUE);
}



char *rstring(int dam, const char *str, int hTyp, const char *wear_mess, const char *wear_mess2,
              const char *wname)
{
    static char lbuf[MAX_STRING_LENGTH];
    const char *i = NULL;
    char *buf;
    int stopbyte, padis;

    const char *dam_mess[][4] = {
        {"никак ", "никак", "никак", "никак"},
        {"слегка задели ", "слегка задел1(,а,о,и) ", "слегка ", "слабые "},
        {"очень легко ранили ", "очень легко ранил1(,а,о,и) ", "очень легко ", "очень легкие "},
        {"легко ранили ", "легко ранил1(,а,о,и) ", "легко ", "легкие "},
        {"ранили ", "ранил1(,а,о,и) ", "", ""},
        {"тяжело ранили ", "тяжело ранил1(,а,о,и) ", "тяжело ", "тяжелые "},
        {"очень тяжело ранили ", "очень тяжело ранил1(,а,о,и) ", "очень тяжело ", "очень тяжелые "},
        {"смертельно ранили ", "смертельно ранил1(,а,о,и) ", "смертельно ", "смертельные "},
        {"покалечили ", "покалечил1(,а,о,и) ", "покалечено ", "калечащие "},
        {"уничтожили ", "уничтожил1(,а,о,и) ", "уничтожительно ", "уничтожительные "},
        {"обратили в прах ", "обратил1(,а,о,и) в прах ", "УНИЧТОЖИТЕЛЬНО ", "обращающие в прах "}
    };

    const char *attack_hit_mess[][3] = {
        {"ударив ", "", "ударить "},
        {"ободрав ", "обдирающий ", "ободрать "},
        {"хлестнув ", "хлестающий ", "хлестнуть "},
        {"рубанув ", "рубящий ", "рубануть "},
        {"укусив ", "кусающий ", "укусить "},
        {"огрев ", "огревающий ", "огреть "},
        {"сокрушив ", "сокрушающий ", "сокрушить "},
        {"резанув ", "режущий ", "резануть "},
        {"оцарапав ", "царапающий ", "царапнуть "},
        {"подстрелив ", "стреляющий ", "подстрельнуть "},
        {"пырнув ", "пыряющий ", "пырнуть "},   /* 10 */
        {"уколов ", "колющий ", "уколоть "},
        {"ткнув ", "тыкающий ", "ткнуть "},
        {"лягнув ", "лягающий ", "лягнуть "},
        {"боднув ", "бодающий ", "боднуть "},
        {"клюнув ", "клюющийся ", "клюнуть "},
        {"ужалив", "жалящий", "ужалить "},
        {"стукнув", "стучащий", "стукнуть "},
        {"*", "*", "*"},
        {"*", "*" "*"}
    };

    buf = lbuf;
    for (stopbyte = 0; stopbyte < MAX_STRING_LENGTH; stopbyte++) {
        if (*str == '#') {
            switch (*(++str)) {
                case 'W':
                    if (*(str + 1) < '0' || *(str + 1) > '2') {
                        i = "";
                    } else {
                        padis = *(++str) - '0';
                        i = attack_hit_mess[hTyp][padis];
                    }
                    break;
                case '1':
                    i = dam_mess[dam][3];
                    break;
                case '0':
                    i = dam_mess[dam][2];
                    break;
                case 'K':
                    if (*(str + 1) < '0' || *(str + 1) > '3') {
                        i = "";
                    } else {
                        padis = *(++str) - '0';
                        i = dam_mess[dam][padis];
                    }
                    break;
                case 'P':
                    if (*(str + 1) < '0' || *(str + 1) > '1') {
                        i = "";
                    } else {
                        padis = *(++str) - '0';
                        if (padis == 0)
                            i = wear_mess;
                        else if (padis == 1)
                            i = wear_mess2;
                        else
                            i = wear_mess;
                    }
                    break;
                case 'I':
                    if (wear_mess2)
                        i = wear_mess2;
                    else
                        i = "";
                    break;
                case 'Q':
                    i = wname;
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

    return (lbuf);
}


/*
 * message for doing damage with a spell or skill
 *  C3.0: Also used for weapon damage on miss and death blows
 */
#define DUMMY_KNIGHT 990300
#define DUMMY_SHIELD 990000
#define DUMMY_WEAPON 392



void temp_alt_armor(struct char_data *victim, struct char_data *ch, struct obj_data *obj, int dam,
                    int tSave)
{
    int save = 0;

    const char *damage_tt[] = {
        "",
        "ударом",
        "огнем",
        "холодом",
        "электричеством",
        "кислотой" "ядом",
        "негативном",
        "позитивом"
    };

    save = GET_OBJ_SAVE(obj, tSave);
    dam = ((MAX(1, dam / 9)) * MAX(1, 100 - save)) / 100;
    GET_OBJ_CUR(obj) -= dam;
    if (GET_OBJ_CUR(obj) <= 0) {
        if (ch && tSave == TYPE_HITS)
            act("Ваш@1(,а,о,и) @1и развалил@1(ся,ась,ось,ись) на части от удара 2р.", "Ммпт",
                victim, ch, obj, damage_tt[tSave]);
        else
            act("Ваш@1(,а,о,и) @1и развалил@1(ся,ась,ось,ись) на части от повреждений %1.", "Мпт",
                victim, obj, damage_tt[tSave]);
        extract_obj(obj);
    }

}

//Повреждения доспехов
void alt_armor(struct char_data *victim, struct char_data *ch, int dam, int hLoc, int rTyp)
{
    int tSave;

//Преобразование rTyp в save материала
    if (rTyp == HIT_NONE)
        return;

    switch (rTyp) {
        case HIT_SLASH:
        case HIT_BLOW:
        case HIT_PICK:
        default:
            tSave = TYPE_HITS;
            break;
        case HIT_FIRE:
            tSave = TYPE_FIRE;
            break;
        case HIT_COLD:
            tSave = TYPE_COLD;
            break;
        case HIT_ELECTRO:
            tSave = TYPE_ELECTRO;
            break;
        case HIT_ACID:
            tSave = TYPE_ACID;
            break;
    }

    if (hLoc == -1 && dam > 0) {        //Наносим по всем передметам
        //send_to_charf(ch,"По всему телу\r\n");
        int i;
        struct obj_data *obj, *next_obj;

        for (i = 0; i < NUM_WEARS; i++)
            if (GET_EQ(victim, i))
                temp_alt_armor(victim, ch, GET_EQ(victim, i), dam, tSave);
        for (obj = victim->carrying; obj; obj = next_obj) {
            next_obj = obj->next_content;
            temp_alt_armor(victim, ch, obj, dam, tSave);
        }
    } else {
        if (hLoc <= 0 || hLoc > NUM_WEARS || !GET_EQ(victim, hLoc) || dam < 0)
            return;

        if (hLoc == WEAR_ARMS) {
            if (GET_EQ(victim, WEAR_WRIST_R))
                temp_alt_armor(victim, ch, GET_EQ(victim, WEAR_WRIST_R), dam / 4, tSave);
            if (GET_EQ(victim, WEAR_WRIST_L))
                temp_alt_armor(victim, ch, GET_EQ(victim, WEAR_WRIST_L), dam / 4, tSave);
        }

        if ((hLoc != WEAR_HEAD || hLoc != WEAR_FACE) && GET_EQ(victim, WEAR_ABOUT))
            temp_alt_armor(victim, ch, GET_EQ(victim, WEAR_ABOUT), dam / 10, tSave);

        temp_alt_armor(victim, ch, GET_EQ(victim, hLoc), dam, tSave);
    }
    return;
}

/* Alterate equipment
 *
 */
void alterate_object(struct obj_data *obj, int dam, int chance, int typehit)
{
    int dec;

    if (!obj)
        return;

//log("FIGHT: alterate object");

    dec = (100 - (obj->materials ? GET_OBJ_SAVE(obj, typehit) : 90));
    dam = number(0, (dam * dec) / 1000);


    if (dam > 0 && chance >= number(0, 100)) {
        if ((GET_OBJ_CUR(obj) -= dam) <= 0) {
            if (obj->worn_by)
                act("$o рассыпал$U, не выдержав повреждений.", FALSE, obj->worn_by, obj, 0,
                    TO_CHAR);
            else if (obj->carried_by)
                act("$o рассыпал$U, не выдержав повреждений.", FALSE, obj->carried_by, obj, 0,
                    TO_CHAR);
            extract_obj(obj);
        }
    }
}



/*  Global variables for critical damage */
int was_critic = FALSE;
int dam_critic = 0;


int compute_critical(struct char_data *ch, struct char_data *victim, int dam, int hitloc)
{
//log("[C_CRITICAL] Start");

    act("Вы нанесли удачный удар по $N2.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n нанес$q удачный удар по Вам.", FALSE, ch, 0, victim, TO_VICT);
    act("$n нанес$q удачный удар по $N2.", FALSE, ch, 0, victim, TO_NOTVICT);

//50% к удару
    dam += MAX(2, (dam * 50) / 100);

//log("[C_CRITICAL] End");
    return dam;
}

void poison_victim(struct char_data *ch, struct char_data *vict, int modifier,
                   struct obj_data *wield)
{
    struct affected_type af[1];

    //log("[EXTDAMAGE] Poisoned");
    /* change strength */
    af[0].type = find_spell_num(SPELL_POISON);
    af[0].location = APPLY_NONE;
    af[0].duration = MAX(2, modifier / 10);
    af[0].modifier = modifier;
    af[0].bitvector = AFF_POISON;
    af[0].battleflag = TRUE;
    af[0].owner = GET_ID(ch);

    if (wield) {
        act("Вы отравили $N3 своим $o4.", FALSE, ch, wield, vict, TO_CHAR);
        act("$n отравил$g Вас своим $o4.", FALSE, ch, wield, vict, TO_VICT);
    } else {
        act("Вы отравили $N3.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n отравил$g Вас.", FALSE, ch, 0, vict, TO_VICT);
    }
    affect_join_char(vict, af);
    vict->Poisoner = GET_ID(ch);
//log("[EXTDAMAGE] Poisoned ***");
}

void char_pos_message(struct char_data *ch)
{

    if (!IS_NPC(ch) && (GET_HIT(ch) < (GET_REAL_MAX_HIT(ch) / 5)) && FIGHTING(ch)) {
        send_to_charf(ch, "%sВы желаете, чтобы Ваши раны не кровоточили так сильно!%s\r\n",
                      CCRED(ch, C_SPR), CCNRM(ch, C_SPR));
    }

    switch (GET_POS(ch)) {
        case POS_STUNNED:
            act("$n без сознания, но вероятно скоро придет в себя.", TRUE, ch, 0, 0, TO_ROOM);
            send_to_char("Вы потеряли сознание, но возможно скоро придете в себя.\r\n", ch);
            break;
        case POS_INCAP:
            act("$n смертельно ранен$g и нуждается в помощи.", TRUE, ch, 0, 0, TO_ROOM);
            send_to_char("Вы смертельно ранены и нуждаетесь в помощи.\r\n", ch);
            break;
        case POS_MORTALLYW:
            act("$n смертельно ранен$a и умрет, если $m не помогут.", TRUE, ch, 0, 0, TO_ROOM);
            send_to_char("Вы смертельно ранены и умрете, если Вам не помогут.\r\n", ch);
            break;
        case POS_DEAD:
            act("$n мертв$g, $s душа медленно подымается в небеса.", FALSE, ch, 0, 0, TO_ROOM);
            send_to_char("Вы мертвы!\r\n", ch);
            break;
    }
}

void char_dam_message(int dam, struct char_data *ch, struct char_data *victim, int attacktype)
{
    if (GET_POS(victim) > POS_MORTALLYW) {
        if (dam > (GET_REAL_MAX_HIT(victim) / 4))
            send_to_char("Это действительно БОЛЬНО!\r\n", victim);
    }
}


int get_atack_type(int attacktype)
{
    int result = 0;

    switch (attacktype) {
        case 400:
        case 401:
        case 402:
        case 404:
        case 405:
        case 406:
        case 408:
        case 413:
        case 414:
            result = ARM_BLOW;
            break;
        case 403:
        case 407:
            result = ARM_SLASH;
            break;
        case 409:
        case 410:
        case 411:
        case 412:
            result = ARM_PICK;
            break;
    }

    return result;
}

int real_attack_type(int type)
{
    int result = 0;

    switch (type) {
        case 0:
        case 1:
        case 2:
        case 4:
        case 5:
        case 6:
        case 8:
        case 13:
        case 14:
            result = HIT_BLOW;
            break;
        case 3:
        case 7:
            result = HIT_SLASH;
            break;
        case 9:
        case 10:
        case 11:
        case 12:
            result = HIT_PICK;
            break;
    }

    return result;
}


void add_one_killer(struct char_data *victim, struct char_data *killer)
{
    victim->killer_list.insert(killer);
}

void add_killers(struct char_data *victim, struct char_data *killer)
{
    struct char_data *temp, *tch, *next;
    struct follow_type *f;
    int in_room = IN_ROOM(victim);

    if (in_room == NOWHERE) {
        log("ОШИБКА: Вызов add_killers c In_room = NOWHERE");
        return;
    }

    for (temp = world[in_room].people; temp; temp = next) {
        next = temp->next_in_room;

        // Если чармленый монстр или ездовое животное
        if (IS_NPC(temp) && (FIGHTING(temp) == victim || temp == killer)
            && ((AFF_FLAGGED(temp, AFF_CHARM) || AFF_FLAGGED(temp, AFF_HELPER) || IS_HORSE(temp))
                && temp->master)) {
            //Убийца хозяин
            tch = temp->master;

            add_one_killer(victim, tch);        //флаг на мастера


            //Убийцы чармисы хозяина
            tch = tch->master ? tch->master : tch;
            add_one_killer(victim, tch);
            for (f = tch->followers; f; f = f->next) {
                if (IS_AFFECTED(f->follower, AFF_CHARM) && !IS_NPC(f->follower))
                    add_one_killer(victim, f->follower);
                if (IS_NPC(f->follower) && f->type == FLW_CHARM)
                    add_one_killer(victim, f->follower);
            }

            //Убийцы последователи лидера
            tch = tch->party_leader ? tch->party_leader : tch;
            add_one_killer(victim, tch);
            for (f = tch->party; f; f = f->next) {
                if (IS_AFFECTED(f->follower, AFF_GROUP) && !IS_NPC(f->follower))
                    add_one_killer(victim, f->follower);
                if (IS_NPC(f->follower) && f->type == FLW_CHARM)
                    add_one_killer(victim, f->follower);
            }
            tch = tch->party_leader ? tch->party_leader : tch;
        }

        if (!IS_NPC(temp) && (FIGHTING(temp) == victim || temp == killer)) {
            //if (IS_AFFECTED(temp,AFF_GROUP))
            {
                tch = temp->master ? temp->master : temp;

                add_one_killer(victim, tch);

                for (f = tch->followers; f; f = f->next) {
                    if (IS_AFFECTED(f->follower, AFF_GROUP) && !IS_NPC(f->follower))
                        add_one_killer(victim, f->follower);
                    if (IS_NPC(f->follower) && f->type == FLW_CHARM)
                        add_one_killer(victim, f->follower);
                }
                //Убийцы последователи лидера
                tch = tch->party_leader ? tch->party_leader : tch;
                add_one_killer(victim, tch);
                for (f = tch->party; f; f = f->next) {
                    if (IS_AFFECTED(f->follower, AFF_GROUP) && !IS_NPC(f->follower))
                        add_one_killer(victim, f->follower);
                    if (IS_NPC(f->follower) && f->type == FLW_CHARM)
                        add_one_killer(victim, f->follower);
                }

            }
            //else
            add_one_killer(victim, temp);
        }
    }

}

int damage_obj(struct char_data *victim, struct obj_data *obj, int dam, int mess_no, int show_mess)
{
    int hTyp, in_room = IN_ROOM(victim);
    struct P_damage damage;
    struct P_message message;
    char buf[MAX_STRING_LENGTH];

    if (dam < 0)
        return (0);

    if (!Spl.GetItem(mess_no) && show_mess)
        return (0);

    if (mess_no)
        hTyp = Spl.GetItem(mess_no)->GetItem(SPL_TDAMAGE)->GetInt();
    else
        hTyp = HIT_SLASH;


    damage.valid = true;
    damage.type = hTyp;
    damage.dam = dam;
    damage.power = GET_OBJ_POWER(obj);
    damage.far_min = TRUE;
    damage.armor = FALSE;
    damage.critic = FALSE;
    damage.weapObj = obj;
    damage.check_ac = A_POWER;

    if (show_mess) {
        mess_no = find_spell_num(mess_no);
        GetSpellMessage(mess_no, message);
    }

    if ((_damage(victim, victim, 0, 0, A_POWER, FALSE, damage, message) == RD_KILL)) {
        sprintf(buf, "%s '%s' умер%s от поражения предметом '[%d]%s' в локации %d.",
                IS_MOB(victim) ? "Моб" : "Игрок", GET_NAME(victim), GET_CH_SUF_4(victim),
                GET_OBJ_VNUM(obj), GET_OBJ_PNAME(obj, 0), world[in_room].number);
        mudlog(buf, CMP, LVL_GOD, TRUE);
    }

    return (dam);
}

////////////////////////////////////////////////////////////////////////////////

int damage_wld(struct char_data *victim, room_rnum count, int show_mess)
{
    int hTyp = world[count].damage->type_hit, in_room = -1;
    int dam =
        dice(world[count].damage->damnodice,
             world[count].damage->damsizedice) + world[count].damage->damage;
    struct P_damage damage;
    struct P_message message;
    char buf[MAX_STRING_LENGTH];

    if (!victim)
        return (0);

    if (dam < 0)
        return (0);

    in_room = IN_ROOM(victim);

    victim->last_room_dmg = in_room;

    damage.valid = true;
    damage.type = hTyp;
    damage.dam = dam;
    damage.power = 0;
    damage.far_min = TRUE;
    damage.armor = FALSE;
    damage.critic = FALSE;
    damage.weapObj = NULL;
    damage.check_ac = A_POWER;

    if (show_mess)
        GetWldMessage(count, message);

    if ((_damage(victim, victim, 0, 0, A_POWER, FALSE, damage, message) == RD_KILL)) {
        sprintf(buf, "%s '%s' умер%s в локации %d.", IS_MOB(victim) ? "Моб" : "Игрок",
                GET_NAME(victim), GET_CH_SUF_4(victim), world[in_room].number);
        mudlog(buf, CMP, LVL_GOD, TRUE);
    }

    return (dam);
}

int get_hit_body(struct char_data *ch, struct char_data *victim)
{
    int j, i, cnt_vict = 0;
    float ch_size = GET_REAL_SIZE(ch), vict_size = GET_REAL_SIZE(victim), no_position;
    int nums = 0, pos[20];

    if (!victim->body) {
        return (0);
    }

    for (i = 0; i < 20; i++)
        pos[i] = -1;
    for (i = 0; i < victim->ibody; i++)
        cnt_vict++;

    if (IS_FLY(ch))             //Летающие мобы почти не имееют ограничений
        ch_size = 255;
    else
        ch_size = (ch_size * 15) / 10;

    vict_size /= cnt_vict;

    no_position = MAX(0.0, (float) ch_size / vict_size) + 1.0;

    for (i = 0; i < victim->ibody; i++)
        if (victim->body[i].chance <= (int) no_position) {
            pos[nums] = i;
            nums++;
        }
    j = number(0, nums - 1);

    i = pos[j];

    if (i == -1) {
//  log("Ситуация когда не нашли тело");
        i = 0;
    }
    return i;
}



/**************************/
int calc_affect_attack(struct char_data *ch, struct char_data *victim)
{
    int attack = 0;

//Если не видим противника, то получаем -10 к атаке
    if (!CAN_SEE(ch, victim))
        attack /= 2;

    //Если хотим спать, то получаем -10 к атаке
    /* 29.09.2018 Убираем штрафы
    if (!IS_NPC(ch) && GET_LEVEL(ch) > 15 && GET_COND(ch, SLEEP) == 0)
        attack -= 10;*/

    return attack;
}

/**************************/
/*                        */
/**************************/


//Вычисляем шанс попасть по жертве (victim).
//Чем выше число на выходе тем меньше у нас шансов попасть.


int calc_attack(struct char_data *ch, struct char_data *victim, int skill, int weapon)
{
    int attack = 0;

    attack = GET_REAL_HR(ch);
    attack += calc_affect_attack(ch, victim);

    if (attack && !IS_MOB(ch))
        switch (GET_RACE(ch)) {
            case RACE_HUMAN:
                if (skill == SKILL_SWORDS)
                    attack += 2;
                if (skill == SKILL_CROSSBOWS)
                    attack += 2;
                break;
            case RACE_ORC:
                if (skill == SKILL_AXES)
                    attack += 4;
                break;
            case RACE_GNOME:
                if (skill == SKILL_AXES)
                    attack += 4;
                break;
            case RACE_ELVES:
                if (skill == SKILL_BOWS)
                    attack += 2;
                if (skill == SKILL_STAFFS)
                    attack += 2;
                break;
            case RACE_HALFELVES:
                if (skill == SKILL_BOWS)
                    attack += 2;
                if (skill == SKILL_DAGGERS)
                    attack += 2;
                break;
            case RACE_BARIAUR:
                if (skill == SKILL_SPAEKS)
                    attack += 4;
                break;
            case RACE_TIEFLING:
                if (skill == SKILL_DAGGERS)
                    attack += 4;
                break;
            case RACE_AASIMAR:
                if (skill == SKILL_SWORDS)
                    attack += 2;
                if (skill == SKILL_DAGGERS)
                    attack += 2;
                break;
        }
    // Модификаторы к уровню прокачки скилла оружия
    if (attack && !IS_MOB(ch))
        attack += weapon / 20;

    if (!IS_NPC(ch) && GET_LEVEL(ch) < 10)
        attack += 5;

    return attack;
}

//Расчет попадания от удара

int get_attack_hit(struct char_data *ch, struct char_data *victim,
                   int skill, int victim_ac, int weapon)
{
    int result = FALSE, diceroll, c_attack, rnd;

//Расчет брони жертвы
// if (GET_POS(victim) > POS_STUNNED)
    victim_ac = compute_ac_wear(victim);

    victim_ac += compute_ac_magic(ch, victim);

//Расчет атаки
    c_attack = calc_attack(ch, victim, skill, weapon);

//Если в ловушке сложнее атаковать
    if (ch->trap_object)
        c_attack -= 20;

    diceroll = dice(1, 10);
//rnd = number(1,MIN(101,GET_LEVEL(ch)*4));
    rnd = number(1, (GET_LEVEL(ch) * 3));

    if (number(1, 100) <= 5)    //5% попадания
        rnd = 101;

    if (MAX(0, victim_ac - c_attack - diceroll) > rnd)
        result = FALSE;
    else
        result = TRUE;

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&KВы: шанс:[%d-%d-%d]:%d vs %d = %s&n\r\n",
                      victim_ac, c_attack, diceroll, MAX(0, victim_ac - c_attack - diceroll), rnd,
                      result ? "попадание" : "промах");

    if (PRF_FLAGGED(victim, PRF_CODERINFO))
        send_to_charf(victim, "&K%s: шанс:[%d-%d-%d]:%d vs %d = %s&n\r\n", GET_NAME(ch),
                      victim_ac, c_attack, diceroll, MAX(0, victim_ac - c_attack - diceroll), rnd,
                      result ? "попадание" : "промах");

    return (result);
}


/* Пишем правильную функцию hit */
void hit(struct char_data *ch, struct char_data *victim, int type, int weapon)
{
//Пустая функция
}



struct char_data *find_best_victim(struct char_data *ch)
{
    struct char_data *vict, *victim = NULL, *use_light = NULL, *min_hp = NULL,
        *min_lvl = NULL, *caster = NULL, *best = NULL;

    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
        if (IS_MOB(vict))
            continue;

        if (!victim)
            victim = vict;

        if (IS_DEFAULTDARK(IN_ROOM(ch)) &&
            ((GET_EQ(vict, ITEM_LIGHT) && GET_OBJ_VAL(GET_EQ(vict, ITEM_LIGHT), 2)) ||
             ((AFF_FLAGGED(vict, AFF_SINGLELIGHT) ||
               AFF_FLAGGED(vict, AFF_HOLYLIGHT)) &&
              !AFF_FLAGGED(vict, AFF_HOLYDARK))) &&
            (!use_light || GET_REAL_CHA(use_light) > GET_REAL_CHA(vict)))
            use_light = vict;

        if (!min_hp ||
            GET_HIT(vict) + GET_REAL_CHA(vict) * 10 < GET_HIT(min_hp) + GET_REAL_CHA(min_hp) * 10)
            min_hp = vict;

        if (!min_lvl ||
            GET_LEVEL(vict) + number(1, GET_REAL_CHA(vict)) <
            GET_LEVEL(min_lvl) + number(1, GET_REAL_CHA(min_lvl)))
            min_lvl = vict;

    }

    if (GET_LEVEL(ch) < 5 + number(1, 6))
        best = victim;
    else if (GET_LEVEL(ch) < 10 + number(1, 6))
        best = use_light ? use_light : victim;
    else if (GET_LEVEL(ch) < 15 + number(1, 6))
        best = min_lvl ? min_lvl : (use_light ? use_light : victim);
    else if (GET_LEVEL(ch) < 20 + number(1, 6))
        best = caster ? caster : (min_lvl ? min_lvl : (use_light ? use_light : victim));
    else
        best =
            min_hp ? min_hp : (caster ? caster
                               : (min_lvl ? min_lvl : (use_light ? use_light : victim)));


    return best;
}

void mob_switch(struct char_data *ch)
{
    struct char_data *tch, *vict = NULL;

    if (!MAY_LIKES(ch) || AFF_FLAGGED(ch, AFF_CHARM))
        return;

    for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room)
        if (IS_NPC(tch) &&
            AFF_FLAGGED(tch, AFF_CHARM) &&
            tch->master && !same_group(ch, tch->master) && IN_ROOM(tch->master) == IN_ROOM(ch)) {
            vict = tch->master; // нападаем на владельца чармиса
            //act("&B$n: нападение на владельца чармиса $N.&n",FALSE,ch,0,tch,TO_ROOM);
        } else {
            if (FIGHTING(tch) == ch && !IS_NPC(tch))
                if (vict == NULL || GET_HIT(tch) < GET_HIT(vict))
                    vict = tch;
        }

    if (vict != NULL && FIGHTING(ch) != vict && CAN_SEE(ch, vict) && !same_group(ch, vict)) {
        //act("&B$n выбрал$g цель для переключения $N.&n",FALSE,ch,0,vict,TO_ROOM);
        go_switch_victim(ch, vict);
    }

}

void perform_violence_magic(void)
{
    struct char_data *ch, *ch_next;

    for (ch = character_list; ch; ch = ch_next) {
        ch_next = ch->next;
        if (ch->in_room == NOWHERE || GET_POS(ch) < POS_SLEEPING ||
            GET_POS(ch) == POS_FIGHTING || FIGHTING(ch))
            continue;
        magic_damage_from_char(ch);
    }
}

void magic_damage_from_char(struct char_data *ch)
{
    int level = 0, spellnum;

    if (IN_ROOM(ch) != NOWHERE && affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_NOMAGIC))
        return;

//Огненная корона
    spellnum = find_spell_num(SPELL_FIRE_CROWN);
    if ((level = affected_by_spell_real(ch, spellnum))) {
        struct char_data *tch, *next_tch;
        int x, y, z, w, dam = 0, hittype, afar;

        hittype = Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt();
        afar = TRUE;
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        for (tch = world[IN_ROOM(ch)].people; tch; tch = next_tch) {
            next_tch = tch->next_in_room;
            if (tch == ch)
                continue;
            if (IS_SOUL(tch) || !IS_NPC(tch))
                continue;
            if (!IS_EVILS(tch) || same_group(ch, tch) || MOB_FLAGGED(tch, MOB_NOFIGHT))
                continue;

            if (tch->shop_data)
                continue;

            if ((ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) &&
                 (may_pkill(ch, tch) == PC_REVENGE_PC ||
                  (IS_NPC(ch) && ch->nr == real_mobile(CASTER_PROXY)) || IS_KILLER(tch)))
                || !ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
                dam = SPLDAMAGE;
                mag_damage(spellnum, dam, ch, tch, TRUE, hittype, TRUE);
            }
        }
    }
    level = 0;
//призрачный клинок
    spellnum = find_spell_num(SPELL_SPIRIT_WEAPON);
    if ((level = affected_by_spell_real(ch, spellnum)) && FIGHTING(ch)) {
        int x, y, z, w, dam = 0, hittype, afar;

        hittype = Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt();
        afar = TRUE;
        sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
               &w);
        dam = SPLDAMAGE;
        mag_damage(spellnum, dam, ch, FIGHTING(ch), TRUE, hittype, FALSE);
    }

}

void magic_damage_to_char(struct char_data *ch)
{
    long id;
    struct char_data *owner;
    int dam = 0;
    int x, y, z, w, level, spellnum;

    if (IN_ROOM(ch) != NOWHERE && affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_NOMAGIC))
        return;

    //Бьем spell_clenched_fist
    spellnum = find_spell_num(SPELL_CLENCHED_FIST);
    if (affected_by_spell_real(ch, spellnum)
        && !affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_NOMAGIC)) {
        id = get_spell_owner(ch, SPELL_CLENCHED_FIST);
        owner = get_char_by_id(id);

        if (!owner)
            return;

        level = GET_SKILL(owner, SPELL_SPHERE(spellnum) + TYPE_SPHERE_SKILL);
        if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
            sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y,
                   &z, &w);
            dam = SPLDAMAGE;
        }
        //if (!get_attack_hit(owner,ch, WEAR_BODY, SKILL_PUNCH, 0, GET_SKILL(owner,SKILL_PUNCH)))
        // mag_damage(spellnum, 0, owner, ch, TRUE, Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE, TRUE);
        //else
        mag_damage(spellnum, dam, owner, ch, TRUE,
                   Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), FALSE);
    }
    //бьем crushing fist
    spellnum = find_spell_num(SPELL_CRUSHING_FIST);
    if (affected_by_spell_real(ch, spellnum)) {

        id = get_spell_owner(ch, SPELL_CRUSHING_FIST);
        owner = get_char_by_id(id);

        if (!owner)
            return;

        dam = 0;
        level = GET_SKILL(owner, SPELL_SPHERE(spellnum) + TYPE_SPHERE_SKILL);
        if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
            sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y,
                   &z, &w);
            dam = SPLDAMAGE;
        }
        //if (!get_attack_hit(owner,ch, WEAR_BODY, SKILL_PUNCH, 0, GET_SKILL(owner,SKILL_PUNCH)))
        // mag_damage(spellnum, 0, owner, ch, TRUE, Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE, TRUE);
        //else
        mag_damage(spellnum, dam, owner, ch, TRUE,
                   Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), FALSE);
    }

    dam = 0;
    if (!IS_UNDEAD(ch) && !IS_CONSTRUCTION(ch)) {
        spellnum = find_spell_num(SPELL_HAEMORRAGIA);
        if ((level = affected_by_spell_real(ch, spellnum))) {
            struct P_damage damage;
            struct P_message message;

            if (Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)) {
                sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x,
                       &y, &z, &w);
                dam = SPLDAMAGE;
            }

            damage.valid = true;
            damage.type = Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt();
            damage.dam = dam;
            damage.check_ac = A_POWER;
            damage.armor = TRUE;
            GetSpellMessage(spellnum, message);
            long id = get_spell_owner_real(ch, spellnum);
            struct char_data *own = get_char_by_id(id);

            if (own)
                _damage(own, ch, 0, 0, A_POWER, FALSE, damage, message);
            else
                _damage(ch, ch, 0, 0, A_POWER, FALSE, damage, message);
        }
    }

    for (LastDamageStorage::iterator l = ch->Ldamage.begin(); l != ch->Ldamage.end(); l++) {
        struct P_message emptyMessage;

        owner = NULL;
        owner = get_char_by_id(l->second.id_ch);
        if (!owner)
            continue;
        //_damage(owner,ch,0,0,A_POWER,l->second.damage,l->second.message);
        _damage(owner, ch, 0, 0, A_POWER, FALSE, l->second.damage, emptyMessage);
    }
}

void mob_round_act(struct char_data *ch)
{
    if (!MAY_LIKES(ch) || AFF_FLAGGED(ch, AFF_CHARM))
        return;

    if (AFF_FLAGGED(ch, MOB_SCAVENGER) && npc_battle_scavenge(ch)) {
        // Моб вооружается
        if (NPC_FLAGGED(ch, NPC_WIELDING))
            npc_wield(ch);

        // Моб одевается
        if (NPC_FLAGGED(ch, NPC_ARMORING))
            npc_armor(ch);
    }
}

//Возвращает TRUE если исцеление прошло удачно
int heal_spec_hit(struct char_data *ch, struct char_data *victim, struct mob_spechit_data *h)
{
    int heal = dice(h->damnodice, h->damsizedice) + h->damage;

    if (GET_REAL_MAX_HIT(victim) <= GET_HIT(victim))
        return (FALSE);

    GET_HIT(victim) += heal;
    update_pos(victim);

    if (h->to_victim)
        act(h->to_victim, "мМ", ch, victim);

    if (h->to_room)
        act(h->to_room, "Кмм", ch, victim);

    return (TRUE);
}


void damage_one_spec_hit(struct char_data *ch, struct char_data *victim, struct mob_spechit_data *h)
{
    int spellnum, level, i;
    bool mvd = FALSE;
    struct P_damage damage;
    struct P_message message;

// log("*** %s %s",GET_NAME(ch),GET_NAME(victim));

    for (i = 0; i < NUM_SAV; i++)
        if (h->saves[i])
            if (general_savingthrow_3(victim, i, h->saves[i]))
                return;

    spellnum = find_spell_num(h->spell);
    level = h->power;

    damage.valid = true;
    damage.type = h->hit;
    damage.dam = dice(h->damnodice, h->damsizedice) + h->damage;
    damage.power = GET_POWER(ch);
    damage.far_min = TRUE;
    damage.check_ac = A_POWER;
    damage.armor = FALSE;
    damage.weapObj = NULL;
    if (damage.dam == 0)
        damage.dam = 1;
    if (damage.dam || spellnum != -1) {
        message.valid = true;
        message.hVict = h->to_victim;
        message.hRoom = h->to_room;
        message.kVict = h->to_victim;
        message.kRoom = h->to_room;
        message.aVict = h->to_victim;
        message.aRoom = h->to_room;
        message.bVict = h->to_victim;
        message.bRoom = h->to_room;
        message.mVict = h->to_victim;
        message.mRoom = h->to_room;
        _damage(ch, victim, 0, 0, C_POWER, TRUE, damage, message);
    }
    if (spellnum >= 0) {
        int i = 0;

        for (i = 0; spell_functions[i].name; i++) {
            if (strcmp(SPELL_PROCEDURE(spellnum), spell_functions[i].name) == 0)
                spell_functions[i].func(spellnum, level, ch, victim, 0, FALSE);
        }
    }

    if (IS_SET(GET_SPEC_PROPERTY(h, MSPEC_MOVED), MSPEC_MOVED) && GET_POS(victim) >= POS_FIGHTING) {
        int i, dir;

        mvd = FALSE;
        for (i = 0; i < 6; i++) {
            dir = number(0, NUM_OF_DIRS - 1);
            if (CAN_GO(ch, dir))
                if (do_simple_move(victim, dir | 0x40, TRUE, TRUE)) {
                    mvd = TRUE;
                    act("Удар $1р отшвырнул Вас куда-то %1.", "мМт", ch, victim, DirsTo[dir]);
                    act("Удар $1р отшвырнул $2в куда-то %1.", "Кммт", ch, victim, DirsTo[dir]);
                    break;
                }
        }
        if (!mvd)
            for (i = 0; i < NUM_OF_DIRS; i++) {
                if (CAN_GO(ch, i))
                    if (do_simple_move(victim, i | 0x40, TRUE, TRUE)) {
                        mvd = TRUE;
                        act("Удар $1р отшвырнул Вас куда-то %1.", "мМт", ch, victim, DirsTo[i]);
                        act("Удар $1р отшвырнул $2в куда-то %1.", "Кммт", ch, victim, DirsTo[i]);
                        break;
                    }
            }
    }

    if (mvd)
        GET_WAIT(victim) += PULSE_VIOLENCE * 2;

    if (IS_SET(GET_SPEC_PROPERTY(h, MSPEC_BASH), MSPEC_BASH) && GET_POS(victim) >= POS_FIGHTING) {
        act("Отлетев в сторону, Вы упали.", "мМ", ch, victim);
        act("Отлетев в сторону, 1и упал1(,а,о,и).", "Кмм", victim, ch);
        GET_POS(victim) = POS_SITTING;
        GET_WAIT(victim) += PULSE_VIOLENCE;
    }
}


void mob_spec_hit(struct char_data *ch)
{
    struct mob_spechit_data *h;
    struct char_data *victim = NULL, *tch, *tch_next;

    if (GET_POS(ch) <= POS_SLEEPING)
        return;

    for (h = ch->spec_hit; h; h = h->next) {
        if (h->percent < number(0, 100))
            continue;

        if (!h->pos[(int) GET_POS(ch)])
            continue;

        if (IS_SET(GET_SPEC_PROPERTY(h, MSPEC_HEAL), MSPEC_HEAL)) {
            if (IS_SET(GET_SPEC_PROPERTY(h, MSPEC_ENEMY), MSPEC_ENEMY)) {
                for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room)
                    if (same_group(ch, tch) || FIGHTING(tch) == FIGHTING(ch)) {
                        if (heal_spec_hit(ch, tch, h))
                            return;
                        else
                            continue;
                    }
            } else if (heal_spec_hit(ch, ch, h))
                return;
            else
                continue;
        } else {
            victim = FIGHTING(ch);
            if (victim == NULL)
                continue;

            if (IS_SET(GET_SPEC_PROPERTY(h, MSPEC_ALL), MSPEC_ALL))
                for (tch = world[IN_ROOM(ch)].people; tch; tch = tch_next) {
                    tch_next = tch->next_in_room;
                    if (tch == ch)
                        continue;
                    damage_one_spec_hit(ch, tch, h);
            } else if (IS_SET(GET_SPEC_PROPERTY(h, MSPEC_ENEMY), MSPEC_ENEMY))
                for (tch = world[IN_ROOM(ch)].people; tch; tch = tch_next) {
                    tch_next = tch->next_in_room;
                    if (tch == victim || same_group(victim, tch) || FIGHTING(tch) == ch)
                        damage_one_spec_hit(ch, tch, h);
            } else
                damage_one_spec_hit(ch, victim, h);
            return;
        }

        // for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room)

        // проверка все ли еще тут
        // if (!FIGHTING(ch) || IN_ROOM(ch) != IN_ROOM(FIGHTING(ch)) || !ch)
        //    return;
        // ok = TRUE;
        // victim = FIGHTING(ch);
        // damage_one_spec_hit(ch, victim);
    }

}

/* control the fights going on.  Called every 3 seconds from comm.c. */
void perform_violence(void)
{
    struct char_data *ch;
    int i, speed = 0, min_speed = 0, max_speed = 0;

//Расчитываем скорости
    for (ch = combat_list; ch; ch = next_combat_list) {
        next_combat_list = ch->next_fighting;
        speed = GET_REAL_SPEED(ch);
        //speed = speedy(ch);
        max_speed = MAX(max_speed, speed);
        min_speed = MIN(min_speed, speed);
    }

    for (speed = max_speed; speed >= min_speed; speed--)
        for (ch = combat_list; ch; ch = next_combat_list) {
            next_combat_list = ch->next_fighting;

            if (GET_REAL_SPEED(ch) != speed)
                continue;

            BATTLECNTR(ch) = 0;
            SET_AF_BATTLE(ch, EAF_STAND);

            /*  if (affected_by_spell(ch,SPELL_SLEEP))
               SET_AF_BATTLE(ch,EAF_SLEEP); */

            // Extract battler if no opponent
            if (FIGHTING(ch) == NULL ||
                IN_ROOM(ch) != IN_ROOM(FIGHTING(ch)) || IN_ROOM(ch) == NOWHERE) {
                stop_fighting(ch, TRUE);
                continue;
            }
            //персонаж не учавствует в битве;
            if (IN_ROOM(ch) == NOWHERE)
                continue;

            // спит, без сознания, умирает и etc

            // Устанавливаем позицию битвы
            if (GET_POS(ch) > POS_STUNNED)
                set_battle_pos(ch);
            if (GET_POS(ch) == POS_SLEEPING)
                continue;
            //если это монстр
            if (IS_NPC(ch)) {
                if (GET_POS(ch) > POS_STUNNED) {
                    //групповой бой
                    npc_groupbattle(ch);
                    // проверка все ли еще тут
                    if (!FIGHTING(ch) || IN_ROOM(ch) != IN_ROOM(FIGHTING(ch)))
                        continue;
                    //log("[ROUND] Битва моба");
                    /***** удар первой атакой */
                    if (!AFF_FLAGGED(ch, AFF_STOPRIGHT) && !AFF_FLAGGED(ch, AFF_STOPFIGHT) &&
                        !GET_AF_BATTLE(ch, EAF_USEDRIGHT) && GET_MISSED(ch) <= 0 &&
                        !GET_MOB_HOLD(ch) && !AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
                        //log("[ROUND] Первая атака");
                        int count = 0;

                        if (affected_by_spell(ch, SPELL_SUNBEAM)
                            || GET_AF_BATTLE(ch, EAF_USEDRIGHT))
                            count = 1;
                        else if (GET_AF_BATTLE(ch, EAF_ADDSHOT)) {
                            count = MAX(1, number(1, GET_SKILL(ch, SKILL_ADDSHOT) / 20));
                            count += GET_REAL_DEX(ch) / 6;
                        } else
                            count = ch->npc()->specials.ExtraAttack;

                        for (i = 0; i < count; i++) {
                            if (GET_MISSED(ch))
                                continue;
                            _damage(ch, NULL, WEAP_RIGHT, 0, C_POWER, TRUE);
                        }
                    }
                    /***** удар второй атакой */
                    if (!AFF_FLAGGED(ch, AFF_STOPLEFT) && !AFF_FLAGGED(ch, AFF_STOPFIGHT) &&
                        !GET_EQ(ch, WEAR_SHIELD) && !GET_EQ(ch, WEAR_BOTHS) && GET_MISSED(ch) <= 0
                        && !GET_AF_BATTLE(ch, EAF_USEDLEFT) && !GET_MOB_HOLD(ch)
                        && !AFF_FLAGGED(ch, AFF_STOPFIGHT) && ch->npc()->specials.ExtraAttack2) {
                        //log("[ROUND] Вторая атака");
                        int count = 0;

                        if (affected_by_spell(ch, SPELL_SUNBEAM))
                            count = 0;
                        else
                            count = ch->npc()->specials.ExtraAttack2;

                        for (i = 0; i < count; i++) {
                            if (GET_MISSED(ch))
                                continue;
                            _damage(ch, NULL, WEAP_LEFT, 0, C_POWER, TRUE);
                        }
                    }
                    //log("[ROUND] СпецУдары");
                    //СпецУдары
                    if (ch->spec_hit && !GET_WAIT(ch) && GET_MISSED(ch) <= 0 &&
                        !IS_AFFECTED(ch, AFF_STOPFIGHT) && !IS_AFFECTED(ch, AFF_STUNE) &&
                        !IS_AFFECTED(ch, AFF_HOLD))
                        mob_spec_hit(ch);
                    /* КОНЕЦ УДАРОВ МОБА */
                }
                if (GET_AF_BATTLE(ch, EAF_CRITIC))
                    CLR_AF_BATTLE(ch, EAF_CRITIC);
                //удары от магических эффектов
                //log("[ROUND] Магические эффекты");
                magic_damage_to_char(ch);
                if (!ch)
                    continue;
                magic_damage_from_char(ch);
                //remove_last_attack(ch);
                //log("[ROUND] Конец боя монстра");
                if (GET_AF_BATTLE(ch, EAF_CACT_RIGHT))
                    CLR_AF_BATTLE(ch, EAF_CACT_RIGHT);
            }                   // конец боя монстра
            else
                //если это игрок
            {
                if (GET_POS(ch) > POS_STUNNED) {
                    //log("[ROUND] Начало боя игрока");
                    // Если круговая защита то удары не наносим
                    if (GET_AF_BATTLE(ch, EAF_MULTYPARRY))
                        continue;
                    if (PLR_FLAGGED(ch, PLR_FROZEN))
                        continue;
                    //проверка вдруг что-то случилось с жертвой
                    if (!FIGHTING(ch) || IN_ROOM(ch) != IN_ROOM(FIGHTING(ch)))
                        continue;
                    if (IS_SOUL(ch))
                        continue;
                    /* УДАРЫ ИГРОКА */
                    // удар правой рукой
                    if (!AFF_FLAGGED(ch, AFF_STOPRIGHT) && GET_MISSED(ch) <= 0 &&
                        !GET_MOB_HOLD(ch) && !AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
                        //send_to_charf(ch,"[ROUND] Удар правой рукой");
                        int count = 1, sun = affected_by_spell(ch, SPELL_SUNBEAM);

                        if (GET_AF_BATTLE(ch, EAF_ADDSHOT) && !sun) {
                            improove_skill(ch, FIGHTING(ch), 0, SKILL_ADDSHOT);
                            if (get_max_class(ch) == CLASS_RANGER
                                && get_max_levelclass(ch) > MAX(1, GET_LEVEL(ch) / 2)) {
                                count = MAX(1, number(1, GET_SKILL(ch, SKILL_ADDSHOT) / 25));
                                count += GET_REAL_DEX(ch) / 6;
                            } else {
                                count = MAX(1, number(1, GET_SKILL(ch, SKILL_ADDSHOT) / 35));
                                count += GET_REAL_DEX(ch) / 8;
                            }
                        }
                        if (GET_AF_BATTLE(ch, EAF_ADDSLASH) && !sun) {
                            improove_skill(ch, FIGHTING(ch), 0, SKILL_SATTACK);
                            if (get_max_class(ch) == CLASS_WARRIOR
                                && get_max_levelclass(ch) >= MAX(1, GET_LEVEL(ch) / 2)) {
                                count = MAX(1, number(1, GET_SKILL(ch, SKILL_SATTACK) / 30));
                                count += GET_REAL_DEX(ch) / 8;
                            } else {
                                count = MAX(1, number(1, GET_SKILL(ch, SKILL_SATTACK) / 45));
                                count += GET_REAL_DEX(ch) / 12;
                            }
                        }

                        if (!GET_AF_BATTLE(ch, EAF_USEDRIGHT))
                            for (i = 0; i < count; i++) {
                                if (GET_MISSED(ch))
                                    continue;
                                if ((_damage(ch, NULL, WEAP_RIGHT, 0, C_POWER, TRUE) == RD_NOARROW))
                                    i = count;
                            }
                    }
                    CLR_AF_BATTLE(ch, EAF_USEDRIGHT);
                    //проверка вдруг что-то случилось с жертвой
                    if (!FIGHTING(ch) || IN_ROOM(ch) != IN_ROOM(FIGHTING(ch)))
                        continue;
                    if (IS_SOUL(ch))
                        continue;
                    // удар второй рукой
                    if (!GET_EQ(ch, WEAR_LIGHT) && GET_MISSED(ch) <= 0 &&
                        !GET_EQ(ch, WEAR_SHIELD) && !GET_EQ(ch, WEAR_BOTHS) &&
                        !AFF_FLAGGED(ch, AFF_STOPLEFT) &&
                        !GET_AF_BATTLE(ch, EAF_USEDLEFT) &&
                        !GET_MOB_HOLD(ch) && !AFF_FLAGGED(ch, AFF_STOPFIGHT) &&
                        (!GET_EQ(ch, WEAR_HOLD)
                         || (GET_EQ(ch, WEAR_HOLD)
                             && GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_WEAPON))
                        && !affected_by_spell(ch, SPELL_SUNBEAM)) {
                        //send_to_charf(ch,"[ROUND] Удар левой2 рукой");
                        _damage(ch, NULL, WEAP_LEFT, 0, C_POWER, TRUE);
                        CLR_AF_BATTLE(ch, EAF_SECOND);
                    }
                    if (GET_AF_BATTLE(ch, EAF_CRITIC))
                        CLR_AF_BATTLE(ch, EAF_CRITIC);
                }
                if (IS_SOUL(ch))
                    continue;
                //удары от магических эффектов
                magic_damage_to_char(ch);
                if (!ch)
                    continue;

                magic_damage_from_char(ch);
                //log("[ROUND] Магические эффекты3");
                //remove_last_attack(ch);
                //log("[ROUND]  Сообщение");
                if (GET_POS(ch) > POS_STUNNED &&
                    GET_POS(ch) < POS_FIGHTING &&
                    GET_WAIT(ch) <= 0 && GET_AF_BATTLE(ch, EAF_STAND)) {
                    send_to_charf(ch, "Вам лучше встать на ноги.\r\n");
                    CLR_AF_BATTLE(ch, EAF_STAND);
                }
                //log("[ROUND]  Конец боя игрока");
                if (GET_AF_BATTLE(ch, EAF_CACT_RIGHT))
                    CLR_AF_BATTLE(ch, EAF_CACT_RIGHT);
            }                   //конец боя игрока
            //log("[ROUND]  Конец цикла боя");
        }                       //конец цикла боя

// обработка действий монстра в конце раунда (берет вещи, лутит и т.п.)
// if (IS_NPC(ch)) mob_round_act(ch);

//подтирание штанов
//log("[ROUND]  Подтирание штанов");
    for (ch = combat_list; ch; ch = ch->next_fighting) {
        if (IN_ROOM(ch) == NOWHERE)
            continue;

        // Обработка итогов боя
        CLR_AF_BATTLE(ch, EAF_FIRST);
        CLR_AF_BATTLE(ch, EAF_SECOND);

//  if (GET_AF_BATTLE(ch, EAF_SLEEP)) affect_from_char(ch, SPELL_SLEEP);
//  battle_affect_update(ch);
        if (affected_by_spell(ch, SPELL_BROTH_WEAPON) && ch->damage_rnd) {
            broth_weapon(ch, ch->damage_rnd);
        }

    }

//log("[ROUND] Конец");
}

void end_battle_round(void)
{
    struct char_data *ch;
    struct affected_type *af, *next;
    int i, mod = 0;

//Подлечиваем
    ch = NULL;

    mod = 0;
//log("[EBR] Начало");
    for (ch = combat_list; ch; ch = ch->next_fighting) {
        if (GET_MISSED(ch) > 0)
            GET_MISSED(ch)--;

        if (IN_ROOM(ch) == NOWHERE)
            continue;

//Начинаем орать

        if (!GET_WAIT(ch) &&
            !IS_AFFECTED(ch, AFF_STOPFIGHT) && !IS_AFFECTED(ch, AFF_STUNE) &&
            !IS_AFFECTED(ch, AFF_HOLD))
            if (IS_NPC(ch) && !ch->npc()->specials.alr_helper.empty()
                && !GET_MOB_HOLD(ch) && !IS_AFFECTED(ch, AFF_STOPFIGHT) && !CHECK_WAIT(ch)
                && (GET_HIT(ch) * 100 / GET_REAL_MAX_HIT(ch) <= ch->npc()->specials.AlrLevel)
                && !ch->npc()->specials.AlrNeed) {
                if (ch->npc()->specials.AlrMessRoom) {
                    if (ch->npc()->specials.AlrMessChar)
                        act(ch->npc()->specials.AlrMessRoom, "Кмм", ch, FIGHTING(ch));
                    else
                        act(ch->npc()->specials.AlrMessRoom, "КмМ", ch, FIGHTING(ch));
                }
                if (ch->npc()->specials.AlrMessChar)
                    act(ch->npc()->specials.AlrMessChar, "мМ", ch, FIGHTING(ch));

                mob_alarm(ch, FIGHTING(ch));
                ch->npc()->specials.AlrNeed = TRUE;
            }

        if ((affected_by_spell(ch, SPELL_EVIL_FATE)) && GET_POS(ch) > POS_SITTING
            && !affected_by_spell(ch, SPELL_FPANTACLE)) {
            if (mod == 3) {
                act("Вы оступились и подвернули лодыжку.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n оступил$u и подвернул$g лодыжку.", FALSE, ch, 0, 0, TO_ROOM);
                WAIT_STATE(ch, PULSE_VIOLENCE * 2);
                GET_POS(ch) = POS_SITTING;
            }
        }
        //Обнуляем счетчик повреждений в раунд
        ch->damage_rnd = 0;
        ch->doh = 0;
        remove_last_attack(ch);

        //Снимаем время с HOLDа
        int hold = find_spell_num(SPELL_HOLD);
        int repl = find_spell_num(SPELL_REPULSION);

        for (af = ch->affected; af; af = next) {
            next = af->next;
            if ((af->type != hold && af->type != repl && af->bitvector != AFF_HOLD)
                || af->duration == -1)
                continue;

            if (af->duration >= 1)
                af->duration = MAX(0, af->duration - SECS_PER_MUD_TICK);
            else {
                if (af->type <= 0)
                    show_spell_off(af->type, ch, NULL);
                affect_remove(ch, af);
            }
        }

        // устанавливаем лаг от скилла, а затем очищаем его
        for (i = 1; i <= MAX_SKILLS; i++) {
            if (!CHECK_WAIT(ch) && GET_SKILL_LAGR(ch, i))
                WAIT_STATE(ch, PULSE_VIOLENCE * GET_SKILL_LAGR(ch, i));
            GET_SKILL_LAGR(ch, i) = 0;
        }
        /* очищяем боевые эффекты */
//        if (GET_AF_BATTLE(ch, EAF_BLOCK) && GET_FSKILL(ch) == SKILL_BLOCK) {CLR_AF_BATTLE(ch, EAF_BLOCK); GET_FSKILL(ch) = 0;}
//        if (GET_AF_BATTLE(ch, EAF_PARRY) && GET_FSKILL(ch) == SKILL_PARRY) {CLR_AF_BATTLE(ch, EAF_PARRY); GET_FSKILL(ch) = 0;}
//        if (GET_AF_BATTLE(ch, EAF_DEVIATE) && GET_FSKILL(ch) == SKILL_DEVIATE) {CLR_AF_BATTLE(ch, EAF_DEVIATE); GET_FSKILL(ch) = 0;}
        if (GET_AF_BATTLE(ch, EAF_NOCRITIC))
            CLR_AF_BATTLE(ch, EAF_NOCRITIC);
        if (GET_FSKILL(ch) == SKILL_MULTYPARRY)
            GET_FSKILL(ch) = 0;
        if (GET_AF_BATTLE(ch, EAF_USEDRIGHT))
            CLR_AF_BATTLE(ch, EAF_USEDRIGHT);
        if (GET_AF_BATTLE(ch, EAF_USEDLEFT))
            CLR_AF_BATTLE(ch, EAF_USEDLEFT);
        if (GET_AF_BATTLE(ch, EAF_ADDSHOT))
            CLR_AF_BATTLE(ch, EAF_ADDSHOT);
        if (GET_AF_BATTLE(ch, EAF_CRITIC))
            CLR_AF_BATTLE(ch, EAF_CRITIC);
        if (GET_AF_BATTLE(ch, EAF_ADDSLASH))
            CLR_AF_BATTLE(ch, EAF_ADDSLASH);

        char_pos_message(ch);

        if (MAY_MOVE2(ch) && ch->char_specials.chage_distance && FIGHTING(ch)) {
            struct char_data *victim = FIGHTING(ch);

            if (ch->char_specials.chage_distance > 0 && !check_victim_not_moved(victim, ch)) {
                go_ats(ch, victim);
                ch->char_specials.chage_distance = 0;
            } else if (ch->char_specials.chage_distance < 0) {
                go_ret(ch);
                ch->char_specials.chage_distance = 0;
            }
        }
        //Пытаемся сбежать
        struct char_data *victim = FIGHTING(ch);
        int wimp_lev = GET_WIMP_LEV(ch);

        if (ch && victim && victim != ch && HERE(ch) && wimp_lev && !IS_SOUL(ch) &&
            ((int) ((float) GET_HIT(ch) * 100 / (float) GET_REAL_MAX_HIT(ch)) < wimp_lev) &&
            GET_POS(ch) > POS_SITTING && !GET_MOB_HOLD(ch) && !GET_WAIT(ch)) {
            send_to_char("Вы запаниковали и попытались убежать!\r\n", ch);
            do_flee(ch, NULL, 0, 0, 0);
            WAIT_STATE(ch, PULSE_VIOLENCE / 2);
        }
    }
//log("[EBR] Конец");
}



/* Это временная функция */
int calc_damage_resist(struct char_data *ch, struct char_data *victim, int dam, int hitloc,
                       int hittype, int attacktype, int dam_critic, int afar)
{
    return dam;
}

int calc_magic_decrease(struct char_data *ch, struct char_data *victim, int dam, int type,
                        int hitloc, int critic, int cfar)
{
    int decrease = 0;
    struct room_data *room;

    if (IN_ROOM(ch) != NOWHERE && affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_NOMAGIC))
        return (dam);

//Защита призрачной сферы
    if (IN_ROOM(victim) != NOWHERE && ch && dam) {
        room = &world[IN_ROOM(victim)];
        if (affected_room_by_spell(room, SPELL_PRISMA_SPHERE))
            if (check_psphere_char(victim, room)) {
                //физические атаки
                if ((type == HIT_SLASH || type == HIT_PICK || type == HIT_BLOW)
                    && affected_room_by_bitvector(room, ROOM_AFF_PRISMA_HITS)) {
                    act("Сфера полностью поглотила Ваши повреждения.", FALSE, ch, 0, victim,
                        TO_CHAR);
                    act("Сфера полностью поглотила повреждения $n1.", FALSE, ch, 0, victim,
                        TO_VICT);
                    act("Сфера полностью поглотила повреждения $n1.", FALSE, ch, 0, victim,
                        TO_NOTVICT);
                    if (!distract_psphere(room, ROOM_AFF_PRISMA_HITS, dam)) {
                        act("Белая составляющая сферы уничтожена.", FALSE, ch, 0, victim, TO_CHAR);
                        act("Белая составляющая сферы уничтожена.", FALSE, ch, 0, victim, TO_ROOM);
                    }
                    dam = RD_MAGIC;
                }
                //огненная атака
                if (type == HIT_FIRE && affected_room_by_bitvector(room, ROOM_AFF_PRISMA_FIRE)) {
                    act("Сфера полностью поглотила Ваши повреждения.", FALSE, ch, 0, victim,
                        TO_CHAR);
                    act("Сфера полностью поглотила повреждения $n1.", FALSE, ch, 0, victim,
                        TO_VICT);
                    act("Сфера полностью поглотила повреждения $n1.", FALSE, ch, 0, victim,
                        TO_NOTVICT);
                    if (!distract_psphere(room, ROOM_AFF_PRISMA_HITS, dam)) {
                        act("Красная составляющая сферы уничтожена.", FALSE, ch, 0, victim,
                            TO_CHAR);
                        act("Красная составляющая сферы уничтожена.", FALSE, ch, 0, victim,
                            TO_ROOM);
                    }
                    dam = RD_MAGIC;
                }
                //холодная атака
                if (type == HIT_COLD && affected_room_by_bitvector(room, ROOM_AFF_PRISMA_COLD)) {
                    act("Сфера полностью поглотила Ваши повреждения.", FALSE, ch, 0, victim,
                        TO_CHAR);
                    act("Сфера полностью поглотила повреждения $n1.", FALSE, ch, 0, victim,
                        TO_VICT);
                    act("Сфера полностью поглотила повреждения $n1.", FALSE, ch, 0, victim,
                        TO_NOTVICT);
                    if (!distract_psphere(room, ROOM_AFF_PRISMA_HITS, dam)) {
                        act("Синня составляющая сферы уничтожена.", FALSE, ch, 0, victim, TO_CHAR);
                        act("Синня составляющая сферы уничтожена.", FALSE, ch, 0, victim, TO_ROOM);
                    }
                    dam = RD_MAGIC;
                }
                //электрическая атака
                if (type == HIT_ELECTRO && affected_room_by_bitvector(room, ROOM_AFF_PRISMA_ELEC)) {
                    act("Сфера полностью поглотила Ваши повреждения.", FALSE, ch, 0, victim,
                        TO_CHAR);
                    act("Сфера полностью поглотила повреждения $n1.", FALSE, ch, 0, victim,
                        TO_VICT);
                    act("Сфера полностью поглотила повреждения $n1.", FALSE, ch, 0, victim,
                        TO_NOTVICT);
                    if (!distract_psphere(room, ROOM_AFF_PRISMA_HITS, dam)) {
                        act("Желтая составляющая сферы уничтожена.", FALSE, ch, 0, victim, TO_CHAR);
                        act("Желтая составляющая сферы уничтожена.", FALSE, ch, 0, victim, TO_ROOM);
                    }
                    dam = RD_MAGIC;
                }
                //кислотная атака
                if (type == HIT_ACID && affected_room_by_bitvector(room, ROOM_AFF_PRISMA_ACID)) {
                    act("Сфера полностью поглотила Ваши повреждения.", FALSE, ch, 0, victim,
                        TO_CHAR);
                    act("Сфера полностью поглотила повреждения $n1.", FALSE, ch, 0, victim,
                        TO_VICT);
                    act("Сфера полностью поглотила повреждения $n1.", FALSE, ch, 0, victim,
                        TO_NOTVICT);
                    if (!distract_psphere(room, ROOM_AFF_PRISMA_HITS, dam)) {
                        act("Зеленая составляющая сферы уничтожена.", FALSE, ch, 0, victim,
                            TO_CHAR);
                        act("Зеленая составляющая сферы уничтожена.", FALSE, ch, 0, victim,
                            TO_ROOM);
                    }
                    dam = RD_MAGIC;
                }

                if (!affected_room_by_bitvector(room, ROOM_AFF_PRISMA_HITS) &&
                    !affected_room_by_bitvector(room, ROOM_AFF_PRISMA_FIRE) &&
                    !affected_room_by_bitvector(room, ROOM_AFF_PRISMA_COLD) &&
                    !affected_room_by_bitvector(room, ROOM_AFF_PRISMA_ELEC) &&
                    !affected_room_by_bitvector(room, ROOM_AFF_PRISMA_ACID))
                    affect_from_room(room, SPELL_PRISMA_SPHERE);
            }
    }
//Защита щита энтропии
    if (ch != victim && (type == HIT_SLASH || type == HIT_PICK || type == HIT_BLOW)
        && (decrease = affected_by_spell(victim, SPELL_ENTROPIC_SHIELD)) && dam && cfar) {
        dam = (dam * (100 - (decrease * 2) / 3)) / 100;
        dam -= MAX(0, (decrease / 10));
        if (dam <= 0 || decrease / 3 >= number(1, 100)) {
            act("Радужный щит $N1 отклонил Вашу атаку.", FALSE, ch, 0, victim, TO_CHAR);
            act("Ваш радужный щит отклонил атаку $n1.", FALSE, ch, 0, victim, TO_VICT);
            act("Радужный щит $N1 отклонил атаку $n1.", FALSE, ch, 0, victim, TO_NOTVICT);
            dam = RD_MAGIC;
        }
    }
//поглощение stone_skin
    if (affected_by_spell(victim, SPELL_STONE_SKIN)) {
        decrease = number(1, get_spell_value(victim, SPELL_STONE_SKIN));
        if (PRF_FLAGGED(victim, PRF_CODERINFO))
            send_to_charf(victim, "&bУдар предмета %d кожа %d", dam, decrease);
        dam -= decrease;
        if (PRF_FLAGGED(victim, PRF_CODERINFO))
            send_to_charf(victim, "=%d&n\r\n", dam);
        if (dam <= 0) {
            if (ch != victim) {
                act("Ваша кожа полностью поглотила удар $n1.&n", FALSE, ch, 0, victim, TO_VICT);
                act("Кожа $N1 полностью поглотила Ваш удар.&n", FALSE, ch, 0, victim, TO_CHAR);
                act("Кожа $N1 полностью поглотила удар $n1.", TRUE, ch, 0, victim, TO_NOTVICT);
            }
            dam = RD_MAGIC;
        }
    }


    return (dam);
}

int calc_armor_decrease(struct char_data *ch, struct char_data *victim, int dam, int type,
                        int hitloc, int critic, int cfar, int sLevel)
{
    int decrease = 0, arms = 0, cover = 0, dd = number(1, 100);

//Повреждения брони
//alt_armor(victim,ch,dam,hitloc,type);

//log("ch %s dam %d hittype %d",GET_NAME(ch),dam,hittype);
    /* резист от обычного удара */

    if (IS_NPC(victim))
        cover = 100;
    else if (GET_EQ(victim, hitloc))
        cover = GET_OBJ_COV(GET_EQ(victim, hitloc), type - 1);

    if (PRF_FLAGGED(victim, PRF_CODERINFO) && cover)
        send_to_charf(victim, "&KПо вам: покрытие %d против %d\r\n", dd, (sLevel * 10 / cover));

    if (cover && (dd >= (sLevel * 10 / cover)))
        if (!critic && type >= HIT_SLASH && type <= HIT_BLOW) {
            // считаем в зависимости от точки удара и затем вычисляем %
            if (IS_NPC(victim) && type >= HIT_SLASH && type <= HIT_BLOW)
                arms = victim->npc()->specials.armor[type - 1];
            else if (type >= HIT_SLASH && type <= HIT_BLOW)
                arms = compute_armour_wear(victim, type, hitloc);

            if (GET_EQ(victim, WEAR_SHIELD) != NULL)
                temp_alt_armor(victim, ch, GET_EQ(victim, WEAR_SHIELD), dam / 10, type);

            //decrease = number(0,arms);
            decrease = arms;

            if (PRF_FLAGGED(victim, PRF_CODERINFO))
                send_to_charf(victim, "&KПо вам:удар %s %d броня %d[%3.2f]", damage_name[type], dam,
                              decrease, (float) (((float) 100 - (float) decrease) / (float) 100));

            if (PRF_FLAGGED(ch, PRF_CODERINFO))
                send_to_charf(ch, "&KВы: поглощение %s %d броня %d[%3.2f]", damage_name[type], dam,
                              decrease, (float) (((float) 100 - (float) decrease) / (float) 100));

            //Снимаем в процентах

            dam = (dam * (100 - arms)) / 100;

            if (PRF_FLAGGED(victim, PRF_CODERINFO))
                send_to_charf(victim, " =(%d)", dam);

            if (PRF_FLAGGED(ch, PRF_CODERINFO))
                send_to_charf(ch, " =(%d)", dam);

            //Снимаем абсолютное значение
            decrease = MAX(0, arms / 20);
            dam -= decrease;

            if (PRF_FLAGGED(victim, PRF_CODERINFO))
                send_to_charf(victim, " [%d]&n\r\n", dam);

            if (PRF_FLAGGED(ch, PRF_CODERINFO))
                send_to_charf(ch, " [%d]&n\r\n", dam);

            if (dam <= 0)
                dam = RD_ARMOR;
        }

    /* резист от ударов стихиями */
    if (dam > 0 && type > HIT_BLOW) {

        if (PRF_FLAGGED(victim, PRF_CODERINFO))
            send_to_charf(victim, "&KПо Вам: резист %s [%d] ", damage_name[type], dam);
        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KВы: резист %s [%d] ", damage_name[type], dam);

        switch (type) {
            case HIT_FIRE:
                decrease = MAX(0, 100 - get_saves3(victim, SAV_FIRE));
                dam = (dam * decrease) / 100;
                break;
            case HIT_COLD:
                decrease = MAX(0, 100 - get_saves3(victim, SAV_COLD));
                dam = (dam * decrease) / 100;
                break;
            case HIT_ELECTRO:
                decrease = MAX(0, 100 - get_saves3(victim, SAV_ELECTRO));
                dam = (dam * decrease) / 100;
                break;
            case HIT_ACID:
                decrease = MAX(0, 100 - get_saves3(victim, SAV_ACID));
                dam = (dam * decrease) / 100;
                break;
            case HIT_POISON:
                decrease = MAX(0, 100 - get_saves3(victim, SAV_POISON));
                dam = (dam * decrease) / 100;
                break;
            case HIT_POSITIVE:
                decrease = MAX(0, 100 - get_saves3(victim, SAV_POSITIVE));
                dam = (dam * decrease) / 100;
                break;
            case HIT_NEGATIVE:
                decrease = MAX(0, 100 - get_saves3(victim, SAV_NEGATIVE));
                dam = (dam * decrease) / 100;
                break;
            default:
                log("ОШИБКА: damage() вызвано с параметром type > HIT_XXX");
                break;
        }

        if (PRF_FLAGGED(victim, PRF_CODERINFO))
            send_to_charf(victim, "%d%%= %d&n\r\n", decrease, dam);

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "%d%% = %d&n\r\n", decrease, dam);

    }

    alt_armor(victim, ch, dam, hitloc, type);   // повреждения брони

    return (dam);
}

/* Магические повреждения от аур и стены */
void mag_aura_damage(struct char_data *ch, struct char_data *victim, int dam, int afar, int &adam,
                     int weapon, struct P_message &message)
{
    struct P_damage mDamage;
    int no_dam = 0, mTyp = 0;
    int sp_prism_skin = find_spell_num(SPELL_PRISMATIC_SKIN);

    if (IN_ROOM(ch) != NOWHERE && affected_room_by_bitvector(&world[IN_ROOM(ch)], ROOM_AFF_NOMAGIC))
        return;


    if (victim != ch) {
        int level = 0, decl = 0;

        //Призрачная кожа
        if (dam && !afar && !IS_UNDEAD(ch) && !IS_CONSTRUCTION(ch) && (BATTLECNTR(ch) == 1) &&
            (level = affected_by_spell_real(victim, sp_prism_skin)) &&
            !PRF_FLAGGED(ch, PRF_NOHASSLE)) {

            decl = affected_by_spell_real(ch, sp_prism_skin);

            struct affected_type af[1];

            af[0].type = 0;
            af[0].bitvector = 0;
            af[0].modifier = 0;
            af[0].duration = 0;
            af[0].battleflag = 0;
            af[0].main = 0;
            af[0].location = APPLY_NONE;

            af[0].type = sp_prism_skin;
            af[0].duration = MAX(3, level / 20);
            af[0].location = APPLY_LEVEL;
            af[0].modifier = decl + MAX(1, level / 25);
            af[0].battleflag = TRUE;
            af[0].main = TRUE;
            af[0].level = level;
            af[0].owner = GET_ID(victim);

            send_to_char(CCIGRN(ch, C_CMP), ch);
            act("Вы почувствовали огромную слабость прикоснувшись к коже $N1.", FALSE, ch, 0,
                victim, TO_CHAR);
            send_to_char(CCNRM(ch, C_CMP), ch);
            send_to_char(CCIGRN(victim, C_CMP), victim);
            act("$n резко ослаб прикоснувшись к Вашей коже.", FALSE, ch, 0, victim, TO_VICT);
            send_to_char(CCNRM(victim, C_CMP), victim);
            act("$n резко ослаб прикоснувшись к коже $N1.", FALSE, ch, 0, victim, TO_NOTVICT);

            affect_join_char(ch, af + 0);
        }
        //Святая аура
        if (dam && !afar && IS_UNDEAD(ch) && affected_by_spell(victim, SPELL_HOLY_AURA) &&
            !PRF_FLAGGED(ch, PRF_NOHASSLE) && !affected_by_spell(ch, SPELL_BLIND) &&
            //проидее вместо кубика нужно поставить резист, но пока оставим так
            number(1, 5) == 4) {
            struct affected_type af[1];

            af[0].type = 0;
            af[0].bitvector = 0;
            af[0].modifier = 0;
            af[0].duration = 0;
            af[0].battleflag = 0;
            af[0].main = 0;
            af[0].location = APPLY_NONE;

            af[0].type = find_spell_num(SPELL_BLIND);
            af[0].duration = PULSE_VIOLENCE;
            af[0].modifier = 50;
            af[0].bitvector = AFF_BLIND;
            af[0].owner = GET_ID(victim);

            send_to_char(CCIRED(ch, C_CMP), ch);
            act("Вы ослепли от прикосновения к сверкающей ауре окружающей $N3.", FALSE, ch, 0,
                victim, TO_CHAR);
            send_to_char(CCNRM(ch, C_CMP), ch);
            send_to_char(CCIYEL(victim, C_CMP), victim);
            act("$n ослеп$q от прикосновения к сверкающей ауре окружающей Вас.", FALSE, ch, 0,
                victim, TO_VICT);
            send_to_char(CCNRM(ch, C_CMP), ch);
            act("$n ослеп$q от прикосновения к сверкающей ауре окружающей $N3.", FALSE, ch, 0,
                victim, TO_NOTVICT);

            affect_join_char(ch, af + 0);
        }
        //Аура скверны
        if (dam && !afar && IS_GOODS(ch) && affected_by_spell(victim, SPELL_UNHOLY_AURA) &&
            !PRF_FLAGGED(ch, PRF_NOHASSLE) && !affected_by_spell(ch, SPELL_UNHOLY) &&
            //проидее вместо кубика нужно поставить резист, но пока оставим так
            dice(1, 20) > 10) {
            struct affected_type af[1];

            af[0].type = 0;
            af[0].bitvector = 0;
            af[0].modifier = 0;
            af[0].duration = 0;
            af[0].battleflag = 0;
            af[0].main = 0;
            af[0].location = APPLY_NONE;

            af[0].type = find_spell_num(SPELL_UNHOLY);
            af[0].duration = PULSE_VIOLENCE;
            af[0].location = APPLY_STR;
            af[0].modifier = -dice(1, 6);
            af[0].owner = GET_ID(victim);


            send_to_char(CCIRED(ch, C_CMP), ch);
            act("Вы ослабли от прикосновения к мерцающей ауре окружающей $N3.", FALSE, ch, 0,
                victim, TO_CHAR);
            send_to_char(CCNRM(ch, C_CMP), ch);
            send_to_char(CCIYEL(victim, C_CMP), victim);
            act("$n ослаб$q от прикосновения к мерцающей ауре окружающей Вас.", FALSE, ch, 0,
                victim, TO_VICT);
            send_to_char(CCNRM(victim, C_CMP), victim);
            act("$n вздрогнул$g от прикосновения к мерцающей ауре окружающей $N3.", FALSE, ch, 0,
                victim, TO_NOTVICT);

            affect_join_char(ch, af + 0);
        }
        //log("[DAMAGE] fire blade");
        if (dam && !afar &&
            (affected_by_spell(victim, SPELL_FIRE_BLADES) ||
             (affected_by_spell(victim, SPELL_FIRE_SHIELD) &&
              affected_by_spell(victim, SPELL_BLADE_BARRIER)))) {
            int x, y, z, w, level, spell;

            level = affected_by_spell(victim, SPELL_FIRE_BLADES);
            no_dam = SPELL_FIRE_BLADES;
            mTyp = spell = find_spell_num(SPELL_FIRE_BLADES);
            sscanf(Spl.GetItem(spell)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
                   &w);

            mDamage.valid = true;
            mDamage.type = Spl.GetItem(spell)->GetItem(SPL_TDAMAGE)->GetInt();
            adam = SPLDAMAGE;
            mDamage.armor = TRUE;
            mDamage.magic = TRUE;
            adam = calc_armor_decrease(ch, victim, adam, mDamage.type, WEAR_BODY, FALSE, FALSE, 0);
            mDamage.dam = adam;
            mDamage.power = A_POWER;
            mDamage.far_min = TRUE;
            mDamage.check_ac = A_POWER;
            /*if (weapon)
               GetSpellMessage(find_spell_num(SPELL_FIRE_BLADES_WEAP), message);
               else      */
            GetSpellMessage(spell, message);
        } else
            //Огненый щит
        if (dam && !was_critic && !afar && affected_by_spell(victim, SPELL_FIRE_SHIELD)) {
            int x, y, z, w, spell, level;

            level = affected_by_spell(victim, SPELL_FIRE_SHIELD);
            no_dam = SPELL_FIRE_SHIELD;
            mTyp = spell = find_spell_num(SPELL_FIRE_SHIELD);
            sscanf(Spl.GetItem(spell)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
                   &w);

            mDamage.valid = true;
            mDamage.type = Spl.GetItem(spell)->GetItem(SPL_TDAMAGE)->GetInt();
            adam = SPLDAMAGE;
            mDamage.armor = TRUE;
            mDamage.magic = TRUE;
            adam = calc_armor_decrease(victim, ch, adam, mDamage.type, WEAR_BODY, FALSE, FALSE, 0);
            mDamage.dam = adam;
            mDamage.power = A_POWER;
            mDamage.far_min = TRUE;
            mDamage.check_ac = A_POWER;
            /*if (weapon)
               GetSpellMessage(find_spell_num(SPELL_FIRE_SHIELD_WEAP), message);
               else */
            GetSpellMessage(spell, message);
        } else
            //Вращаюшиеся лезвия
        if (affected_by_spell(victim, SPELL_BLADE_BARRIER) && !afar && dam > 0) {
            int x, y, z, w, level, spell;

            no_dam = SPELL_BLADE_BARRIER;
            level = affected_by_spell(victim, SPELL_BLADE_BARRIER);
            mTyp = spell = find_spell_num(SPELL_BLADE_BARRIER);
            sscanf(Spl.GetItem(spell)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
                   &w);

            mDamage.valid = true;
            mDamage.type = Spl.GetItem(spell)->GetItem(SPL_TDAMAGE)->GetInt();
            adam = SPLDAMAGE;
            mDamage.armor = TRUE;
            mDamage.magic = TRUE;
            adam = calc_armor_decrease(ch, victim, adam, mDamage.type, WEAR_BODY, FALSE, FALSE, 0);
            mDamage.dam = adam;
            mDamage.power = A_POWER;
            mDamage.far_min = TRUE;
            mDamage.check_ac = A_POWER;
            /*if (weapon)
               GetSpellMessage(find_spell_num(SPELL_BLADE_BARRIER_WEAP), message);
               else */
            GetSpellMessage(spell, message);
        } else
            //Костяные шипы
        if (dam && !was_critic && !afar && affected_by_spell(victim, SPELL_BONES_PICK)) {
            int x, y, z, w, spell, level;

            level = affected_by_spell(victim, SPELL_BONES_PICK);
            no_dam = SPELL_BONES_PICK;
            mTyp = spell = find_spell_num(SPELL_BONES_PICK);
            sscanf(Spl.GetItem(spell)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y, &z,
                   &w);

            mDamage.valid = true;
            mDamage.type = Spl.GetItem(spell)->GetItem(SPL_TDAMAGE)->GetInt();
            adam = SPLDAMAGE;
            mDamage.armor = TRUE;
            mDamage.magic = TRUE;
            adam = calc_armor_decrease(victim, ch, adam, mDamage.type, WEAR_BODY, FALSE, FALSE, 0);
            mDamage.dam = adam;
            mDamage.power = A_POWER;
            mDamage.far_min = TRUE;
            mDamage.check_ac = A_POWER;
            /*if (weapon)
               GetSpellMessage(find_spell_num(SPELL_FIRE_SHIELD_WEAP), message);
               else */
            GetSpellMessage(spell, message);
        }



        if (no_dam) {           //Наносим ответные повреждения если они есть
            //if (!weapon)
            _damage(victim, ch, 0, 0, A_POWER, FALSE, mDamage, message);
            //else
            // add_last_damage(victim, ch, no_dam, mDamage, message);
        }

    }
}

bool is_critic(struct char_data *ch, int koef, int slevel)
{
    int percent = number(1, 100);

    koef = MAX(1, koef);
    if (dice(1, 40) == 20 / koef && GET_WAIT(ch) <= 0 && slevel + GET_REAL_LCK(ch) >= percent)
        return (TRUE);
    else
        return (FALSE);
}


int _damage(struct char_data *ch, struct char_data *victim, int weapon, int style, int power,
            int set)
{
    struct P_message emptyMessage;
    struct P_damage emptyDamage;

    return _damage(ch, victim, weapon, style, power, set, emptyDamage, emptyMessage);
}

/* Функция нанесения повреждений по персонажу */
int _damage(struct char_data *ch,
            struct char_data *victim,
            int weapon,
            int style, int power, int set, struct P_damage &damage, struct P_message &message)
{
    int dam = 0, adam = 0, skill = 0, addskill = 0, tdam = 0, dtype = 0, location, slevel =
        0, afar = FALSE, isCritic = FALSE, vfar = FALSE;
    int pVictim, weap_power = 0, dmess = 0, bLoc = WPOS_BODY;
    int bl_bits = BL_NONE;
    bool okDam, in_use = FALSE, isArm = FALSE, ignore = FALSE;
    int maxlevel = 0, sumlevels = 0, tMes = M_HIT;

    Killer_list_type::iterator k;
    struct char_data *killer = NULL, *vict = NULL;
    struct obj_data *weapObj = NULL, *missileObj = NULL;
    struct weapon_damage_data *damages;
    int spell_repl = find_spell_num(SPELL_REPULSION);
    char buf[MAX_STRING_LENGTH];

    struct P_message addMessage;

    if (!ch)
        return RD_NONE;

    if (!victim && !FIGHTING(ch))
        return RD_NONE;

    if (!victim)
        victim = FIGHTING(ch);

    stop_events(ch, STOP_ALL);
    stop_events(victim, STOP_ALL);

    if (IS_SOUL(victim))
        return (RD_NONE);

//Если игрок под святилищем и начал бой, то снимаем эффект святилища на жертву и ее группу.
    if (affected_by_spell(ch, SPELL_SANCTUARY)) {
        struct char_data *master = victim->master ? victim->master : victim;
        struct char_data *leader = victim->party_leader ? victim->party_leader : victim;
        struct follow_type *f;

        add_victim_may_attack(ch, victim);
        for (f = master->followers; f; f = f->next)
            if (same_group(victim, f->follower) && f->follower != victim)
                add_victim_may_attack(ch, f->follower);

        for (f = leader->party; f; f = f->next)
            if (same_group(victim, f->follower) && f->follower != victim)
                add_victim_may_attack(ch, f->follower);

    }

    /* Делаем проверку вдруг кто-то уже убежал */
    if (ch->in_room != victim->in_room || ch->in_room == NOWHERE) {
        if (FIGHTING(ch) && FIGHTING(ch) == victim)
            stop_fighting(ch, TRUE);
        return RD_NONE;
    }
//Прекращаем охранные действия
    if (GUARDING(ch) == victim)
        stop_guarding(ch);

    if (GUARDING(victim) == ch)
        stop_guarding(victim);

//Охрана
    for (vict = world[IN_ROOM(victim)].people; vict; vict = vict->next_in_room) {
        if (GUARDING(vict) == victim &&
            !AFF_FLAGGED(vict, AFF_STOPFIGHT) &&
            !AFF_FLAGGED(vict, AFF_STUNE) &&
            CAN_SEE(vict, ch) &&
            GET_WAIT(vict) <= 0 &&
            !GET_MOB_HOLD(vict) && ch != victim && GET_POS(vict) == POS_STANDING) {
            if (go_guard(vict, victim, ch))
                victim = vict;
            break;
        }
    }

//Эффект от луча света всегда только правой рукой
    if (affected_by_spell(ch, SPELL_SUNBEAM) && !damage.valid) {
        if (weapon == WEAP_RIGHT) {
            int x, y, z, w, level, spellnum;

            level = affected_by_spell(ch, SPELL_SUNBEAM);
            spellnum = find_spell_num(SPELL_SUNBEAM);
            sscanf(Spl.GetItem(spellnum)->GetItem(SPL_DAMAGE)->GetString(), "%d+%d+%d+%d", &x, &y,
                   &z, &w);
            dam = SPLDAMAGE;
            if (IS_UNDEAD(victim))
                dam = (dam * 3) / 2;
            else if (IS_CONSTRUCTION(victim))
                dam = (dam * 2) / 3;

            if (check_distance(ch, victim) == DIST_0)
                dam = MAX(1, dam / 2);

            mag_damage(spellnum, dam, ch, victim, TRUE,
                       Spl.GetItem(spellnum)->GetItem(SPL_TDAMAGE)->GetInt(), TRUE);
            if (check_spells_attacked(ch, victim, TRUE, TRUE))
                return (RD_NONE);
        }
        return RD_NORMAL;
    }

    pVictim = GET_POWER(victim);

    if (weapon == 0 && damage.valid) {  //удар без оружия, но с использованием damage
        dam = damage.dam;
        afar = damage.far_min;
        dmess = 0;              //Не имеет занчения
        dtype = damage.type;
        weap_power = damage.power;
        weapObj = damage.weapObj;
        slevel = 100;
        if (!damage.hLoc) {
            bLoc = get_hit_body(ch, victim);
            location = wpos_to_wear[victim->body[bLoc].wear_position];
        } else {
            location = damage.hLoc;
            for (int b = 0; b < victim->ibody; b++) {
                //log("Сравнение %d == %d", wear_to_wpos[location],victim->body[b].wear_position);
                if (wear_to_wpos[location] == victim->body[b].wear_position) {
                    bLoc = b;
                    break;
                }
            }
            //log("location %d  bLoc %d",location,bLoc);
        }


        if (damage.magic == TRUE || damage.armor == FALSE)
            SET_BIT(bl_bits, BL_IGNORE);

        if (damage.deviate == TRUE)
            SET_BIT(bl_bits, BL_NODEVIATE);

        isArm = damage.armor;

        if (PRF_FLAGGED(victim, PRF_CODERINFO))
            send_to_charf(victim, "&KПо Вам: начальное повреждение %s %d check_ac %d\r\n",
                          damage_name[dtype], dam, damage.check_ac);
        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KВы: начальное повреждение %s %d check_ac %d\r\n",
                          damage_name[dtype], dam, damage.check_ac);

        if (!isArm)
            if ((dam =
                 calc_armor_decrease(ch, victim, dam, dtype, location, FALSE, afar,
                                     slevel)) == RD_ARMOR) {
                dam = 0;
                tMes = M_SKIN;
            }
    } else if (weapon) {
        bool dual = FALSE;

        //Удары с оружием у персонажа
//  if (!IS_NPC(ch) && ((GET_EQ(ch,weapon+18) && GET_EQ(ch,weapon+18)->weapon) || //проверка позиции на оружие
        if ((GET_EQ(ch, weapon + 18) && GET_EQ(ch, weapon + 18)->weapon) ||     //проверка позиции на оружие
            (weapon == WEAP_RIGHT && GET_EQ(ch, WEAR_BOTHS) && GET_EQ(ch, WEAR_BOTHS)->weapon)) {
            if (GET_EQ(ch, WEAR_BOTHS)) {
                weapObj = GET_EQ(ch, WEAR_BOTHS);
                dual = TRUE;
            } else
                weapObj = GET_EQ(ch, weapon + 18);

            skill = GET_OBJ_SKILL(weapObj);
            slevel = GET_SKILL(ch, skill);
            dmess = weapObj->weapon->message;
            weap_power = GET_OBJ_POWER(weapObj);

            if (dual && skill != SKILL_BOWS && skill != SKILL_CROSSBOWS) {
                addskill = SKILL_BOTHHANDS;
                slevel = (slevel + GET_SKILL(ch, addskill)) / 2;
                SET_BIT(bl_bits, BL_DUAL);
            }

            if (weapon == WEAP_LEFT) {
                slevel = (slevel + GET_SKILL(ch, SKILL_SHIT)) / 2;
                addskill = SKILL_SHIT;
            }

            if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NARROW))
                victim = random_victim(ch, victim, slevel);

            if (skill == SKILL_BOWS || skill == SKILL_CROSSBOWS) {
                missileObj = get_missile(ch, weapObj);
                if (missileObj == NULL) {
                    act("У Вас закончились стрелы для @1р.", "Мп", ch, weapObj);
                    return RD_NOARROW;
                }
            }

            bLoc = get_hit_body(ch, victim);
            location = wpos_to_wear[victim->body[bLoc].wear_position];
            if (skill == SKILL_BOWS || skill == SKILL_CROSSBOWS) {      //стрелковое
                afar = TRUE;
                SET_BIT(bl_bits, BL_FAR);
            }
            isCritic = is_critic(ch, weapObj->weapon->critics, slevel);

            for (damages = weapObj->weapon->damages; damages; damages = damages->next) {
                dtype = damages->type_damage;
                tdam = number(damages->min_damage, damages->max_damage);
                //if (tdam == 0) ignore = TRUE;
                if (dual && !missileObj)
                    tdam += (GET_REAL_DR(ch) * 3) / 2;
                else if (missileObj)
                    tdam += (GET_REAL_DEX(ch) - 10) / 2;
                else
                    tdam += GET_REAL_DR(ch);
                if (!isCritic)
                    if ((tdam =
                         calc_armor_decrease(ch, victim, tdam, dtype, location, FALSE, afar,
                                             slevel)) == RD_ARMOR)
                        tdam = 0;
                dam += tdam;
            }

            if (IS_OBJ_STAT(weapObj, ITEM_DEATHAURA)) { //с призрачной аурой
                int dlevel = affected_by_spell(weapObj, SPELL_DEATH_WEAPON);

                if (number(1, 100) < MAX(5, dlevel / 5)) {
                    dtype = HIT_NEGATIVE;
                    tdam = MAX(1, dlevel / 5) + 5;
                    if (!isCritic)
                        if ((tdam =
                             calc_armor_decrease(ch, victim, tdam, dtype, location, FALSE, afar,
                                                 dlevel)) == RD_ARMOR)
                            tdam = 0;
                    dam += tdam;
                    act("Призрачная аура вспыхнула вокруг Ваш@1(его,ей,его,их) @1р.", "Мп", ch,
                        weapObj);
                    act("Призрачная аура вспыхнула вокруг @1р 1+р.", "Кмп", ch, weapObj);
                }
            }

            if (missileObj != NULL) {
                for (damages = missileObj->missile->damages; damages; damages = damages->next) {
                    dtype = damages->type_damage;
                    tdam = number(damages->min_damage, damages->max_damage);
                    if (!isCritic)
                        if ((tdam =
                             calc_armor_decrease(ch, victim, tdam, dtype, location, FALSE, afar,
                                                 slevel)) == RD_ARMOR)
                            tdam = 0;
                    dam += tdam;
                }
                if (IS_OBJ_STAT(missileObj, ITEM_DEATHAURA)) {  //с призрачной аурой
                    int dlevel = affected_by_spell(missileObj, SPELL_DEATH_ARROWS);

                    if (number(1, 100) < MAX(5, dlevel / 10)) {
                        dtype = HIT_NEGATIVE;
                        tdam = MAX(1, dlevel / 10) + 5;
                        if (!isCritic)
                            if ((tdam =
                                 calc_armor_decrease(ch, victim, tdam, dtype, location, FALSE, afar,
                                                     dlevel)) == RD_ARMOR)
                                tdam = 0;
                        dam += tdam;
                        act("Призрачная аура вспыхнула вокруг Ваш@1(его,ей,его,их) @1р.", "Мп", ch,
                            missileObj);
                        act("Призрачная аура вспыхнула вокруг @1р 1+р.", "Кмп", ch, missileObj);
                    }
                }
                if (dam == 0)
                    tMes = M_SKIN;
                weap_power = MAX(weap_power, GET_OBJ_POWER(missileObj));
            }
            dam = MAX(1, MIN(dam, dam * GET_OBJ_CUR(weapObj) / MAX(1, GET_OBJ_MAX(weapObj))));
            weap_power = MAX(weap_power, GET_OBJ_POWER(weapObj));
            weap_power += GET_POWER(ch);
        } else {                //Удары без оружия или удары монстра
            weap_power = GET_POWER(ch);
            if (IS_NPC(ch)) {
                if (weapon == WEAP_RIGHT && ch->npc()->specials.ExtraAttack)
                    dmess = dtype = ch->npc()->specials.attack_type;
                else if (weapon == WEAP_LEFT && ch->npc()->specials.ExtraAttack2)
                    dmess = dtype = ch->npc()->specials.attack_type2;
                else
                    dtype = 0;
                if (dtype == 9) {       //стрелкое у моба
                    afar = TRUE;
                    SET_BIT(bl_bits, BL_FAR);
                }
                dtype = real_attack_type(dtype);
                slevel = calc_need_improove(ch, GET_LEVEL(ch));
                dam =
                    dice(ch->npc()->specials.damnodice,
                         ch->npc()->specials.damsizedice) + (ch->npc()->specials.damage);
            } else {            //У персонажа всегда тип удар
                skill = SKILL_PUNCH;
                dmess = 0;
                if (weapon == WEAP_RIGHT)
                    slevel = GET_SKILL(ch, skill);
                if (weapon == WEAP_LEFT) {
                    slevel = (GET_SKILL(ch, skill) + GET_SKILL(ch, SKILL_SHIT)) / 2;
                    addskill = SKILL_SHIT;
                }

                if (ROOM_FLAGGED(IN_ROOM(ch), ROOM_NARROW))
                    victim = random_victim(ch, victim, slevel);

                dtype = HIT_BLOW;
                dam = GET_REAL_DR(ch);
            }
            bLoc = get_hit_body(ch, victim);
            location = wpos_to_wear[victim->body[bLoc].wear_position];
            isCritic = is_critic(ch, 1, slevel);
            if (!isCritic)
                if ((dam =
                     calc_armor_decrease(ch, victim, dam, dtype, location, FALSE, afar,
                                         slevel)) == RD_ARMOR) {
                    dam = 0;
                    tMes = M_SKIN;
                }
        }

        if (!IS_NPC(ch)) {
            int c_koef = (MIN(100, get_skill_abil(ch, skill)) * GET_SKILL(ch, skill)) / 500;
            int max_dam = max_damage[(int) MAX(0, c_koef)];

            if (PRF_FLAGGED(ch, PRF_CODERINFO))
                send_to_charf(ch, "&KПоправка на %% умения %d[max:%d]=%d\r\n", dam, max_dam,
                              MIN(dam, max_dam));
            dam = MIN(dam, max_dam);
        }
        GetSkillMessage(0, dmess, message);
    } else {                    //Не оружейный
        return RD_ERROR;
    }

    if (weapon && IS_ANIMAL(victim) && IS_AFFECTED(ch, AFF_DANCE)) {
        int lvl = affected_by_spell(ch, SPELL_DANCE);

        dam = dam + ((dam * lvl) / 500);
    }
//Дальность оружия соперника
    if (ch != victim) {
        struct obj_data *victObj = NULL;

        if (GET_EQ(victim, WEAR_BOTHS) && GET_OBJ_TYPE(GET_EQ(victim, WEAR_BOTHS)) == ITEM_WEAPON) {    //проверка позиции на оружие
            victObj = GET_EQ(victim, WEAR_BOTHS);
            int vskill = GET_OBJ_SKILL(victObj);

            if (vskill == SKILL_BOWS || vskill == SKILL_CROSSBOWS)      //стрелковое
                vfar = TRUE;
            else
                vfar = FALSE;   //расстояние оружия
        } else if (GET_EQ(victim, WEAR_WIELD) && GET_OBJ_TYPE(GET_EQ(victim, WEAR_WIELD)) == ITEM_WEAPON) {     //проверка позиции на оружие
            victObj = GET_EQ(victim, WEAR_WIELD);
            int vskill = GET_OBJ_SKILL(victObj);

            if (vskill == SKILL_BOWS || vskill == SKILL_CROSSBOWS)      //стрелковое
                vfar = TRUE;
            else
                vfar = FALSE;   //расстояние оружия
        }
    }
//Проверка аур фарлангуна
    if (check_spells_attacked(ch, victim, TRUE, afar))
        return (RD_NONE);

//Устанавливаем боевые позиции
    if (GET_POS(ch) > POS_STUNNED && (FIGHTING(ch) == NULL)) {
        if (may_pkill(ch, victim) != PC_REVENGE_PC &&
            (!IS_NPC(victim) || (IS_NPC(victim) && MOB_FLAGGED(victim, MOB_MEMORY))))
            inc_pkill_group(victim, ch, 1, 0);
        if (set)
            set_fighting(ch, victim);
    }

    if (GET_POS(victim) > POS_STUNNED && (FIGHTING(victim) == NULL) && set)
        set_fighting(victim, ch);

//Устанавливает дистанцию боя
    int cDist = check_distance(ch, victim);
    int vDist = check_distance(victim, ch);

//send_to_charf(ch,"ДО: Дистанция боя Вы %d, Противник %d\r\n",check_distance(ch, victim),check_distance(victim, ch));


    if (cDist == DIST_NOTFOUND)
        add_distance(ch, victim, afar ? DIST_1 : DIST_0, FALSE);
    if (vDist == DIST_NOTFOUND)
        add_distance(victim, ch, afar ? DIST_1 : DIST_0, FALSE);


//Если противник не может приблизится то меняем дистанцию на дальнию
    if (check_victim_not_moved(ch, victim) && cDist == DIST_1) {
        add_distance(ch, victim, DIST_1, FALSE);
        add_distance(victim, ch, DIST_1, FALSE);
    }

    cDist = check_distance(ch, victim);
    vDist = check_distance(victim, ch);

//Если игрок под отторжением и начал боя с ближнего оружия то
//Устанавливаем ближнию дистанцию
    if (affected_by_spell_real(ch, spell_repl) && cDist == DIST_0) {
        add_victim_may_moved(ch, victim);
        cDist = DIST_0;
        vDist = DIST_0;
    }
//send_to_charf(ch,"ПО: Дистанция боя Вы %d, Противник %d\r\n",check_distance(ch, victim),check_distance(victim, ch));


    if ((weapon == 2) && !IS_NPC(ch))
        okDam = MAX(5, GET_SKILL(ch, SKILL_SHIT)) >= number(1, 100)
            && get_attack_hit(ch, victim, skill, 0, slevel);
    else if ((weapon) || (!weapon && damage.check_ac == C_POWER))
        okDam = get_attack_hit(ch, victim, skill, 0, slevel);
    else if (damage.check_ac == A_POWER)
        okDam = true;
    else
        okDam = false;

    if (GET_AF_BATTLE(ch, EAF_CRITIC)) {
        int percent = number(1, 100);
        int prob = 0, weap_i;
        int skill_c =
            IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_CIRCLESTAB) : GET_SKILL(ch, SKILL_CIRCLESTAB);
        GetSkillMessage(SKILL_CIRCLESTAB, dtype, message);

        prob = GET_REAL_DEX(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) + skill_c + RNDSKILL;
        percent = GET_REAL_DEX(victim) + GET_REAL_DEX(victim) + GET_REAL_INT(victim) +
            ((GET_SAVE3(victim, SAV_REFL) + saving_throws_3(victim, SAV_REFL)) * 3) + RNDSKILL;

        if (weapon == WEAP_RIGHT)
            improove_skill(ch, victim, 0, SKILL_CIRCLESTAB);

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);

        if (!weapObj && ((weap_i = real_object(OBJ_KNIF)) >= 0))
            weapObj = (obj_proto + weap_i);

        if (prob >= percent) {
            if (weapon == WEAP_RIGHT)   //Если у нас проходит умение
            {                   //то правой всегда промах
                message.valid = false;
                okDam = false;
                dam /= 2;
            } else if (weapon == WEAP_LEFT) {   //А левой всегда усиленный удар
                dam = (dam + (dam * skill_c) / 100);
                isArm = TRUE;
                okDam = true;
            }
        } else {
            okDam = false;
            if (weapon == WEAP_RIGHT)
                message.valid = false;
            //Пропускаем раундбоя
            if (weapon == WEAP_LEFT) {
                SET_AF_BATTLE(ch, EAF_NOCRITIC);
                SET_AF_BATTLE(ch, EAF_USEDLEFT);
                SET_AF_BATTLE(ch, EAF_USEDRIGHT);
                GET_MISSED(ch)++;
            }
        }
    }
//Если были скрыты появляемся
    appear(ch);

//Установка флагов мести и боевых позиций
    if (victim != ch) {
        if (victim->master == ch) {
            //лидер атакует своего последователя - выгнать последователя
            if (stop_follower(victim, SF_EMPTY))
                return (-1);
        } else if (ch->master == victim) {
            //последователь атакует своего лидера - выгнать последователя
            if (stop_follower(ch, SF_EMPTY))
                return (-1);
        } else if (ch->master && ch->master == victim->master) {
            //один последователь атакует другого последователя
            if (stop_follower(AFF_FLAGGED(victim, AFF_GROUP) ? ch : victim, SF_EMPTY))
                return (RD_NONE);
        }
        //send_to_charf(ch,"ДО: Дистанция боя Вы %d, Противник %d\r\n",check_distance(ch, victim),check_distance(victim, ch));

        if (weapon && vDist == DIST_1 && !afar) {
            act("&nВаше оружие не достает до 2р.", "Мм", ch, victim);
            act("&nОружие 1р не достает до Вас.", "мМ", ch, victim);
            act("Оружие 1р не достает до 2р.", "Кмм", ch, victim);
            if (IS_MOB(ch))
                ch->char_specials.chage_distance = 1;
            return (RD_FAR);
        }

        if (weapon && vDist == DIST_0 && afar) {
            act("&nВаше оружие не эффективно в ближнем бою.", "М", ch);
            act("&nОружие 1р не эффективно в ближнем бою.", "Км", ch);
            if (IS_MOB(ch))
                ch->char_specials.chage_distance = -1;
            return (RD_FAR);
        }

        /*if (dam>=0 && damage_mtrigger(ch,victim))
           return (RD_NONE); */

        if (!afar) {
            if (IS_AFFECTED(victim, AFF_BONES_WALL)) {
                act("Стена с костяными руками мешает $n2 атаковать Вас.", FALSE, ch, 0, victim,
                    TO_VICT);
                act("Стена с костяными руками мешает Вам атаковать $N3.", FALSE, ch, 0, victim,
                    TO_CHAR);
                act("Стена с костяными руками мешает $n2 атаковать $N3.", FALSE, ch, 0, victim,
                    TO_NOTVICT);
                if (distract_bones_wall(victim, dam)) {
                    act("Ваша костянная стена рассыпалась в прах от удара $1р.", "мМ", ch, victim);
                    act("Костянная стена округ $2р рассыпалась в прах от Вашего удара.", "Мм", ch,
                        victim);
                    act("Костянная стена округ $2р рассыпалась в прах от удара $1р.", "Кмм", ch,
                        victim);
                }
                return (RD_NONE);
            }
        }
    }


    if (okDam) {
        //проверка астрала
        if (IS_NPC(victim) && NPC_FLAGGED(victim, NPC_MISLEAD)) {
            act("Изображение $n1 расплылось в воздухе и медленно иcчезло.", FALSE, victim, 0, 0,
                TO_ROOM);
            extract_char(victim, FALSE);
            //die(victim,NULL);
            return (RD_NONE);
        }
        //Расчет ответных ударов (огненный щит, стена лезвий)
        if (!afar && !ch->doh) {
            mag_aura_damage(ch, victim, dam, FALSE, adam, weapon, addMessage);
            addMessage.valid = false;   // в текущей реализации addMessage игнорируется 
            ch->doh = TRUE;
        }
        //Расчет усиления
        if (power != A_POWER
            && ((weap_power + GET_POWER_ADD(ch) < GET_POWER(victim) && power == C_POWER)
                || power == N_POWER || PRF_FLAGGED(victim, PRF_NOHASSLE))) {
            if (message.valid) {
                ShowMessage(ch, victim, weapObj, message, dam, dmess, M_NONE, bLoc);
            } else if (skill == SKILL_PUNCH || !weapObj) {
                act("Ваш удар не эффективен против $N1.", FALSE, ch, 0, victim, TO_CHAR);
                act("Удар $n1 не способен поразить Вас.", FALSE, ch, 0, victim, TO_VICT);
                act("Удар $n1 не эффективен против $N1.", FALSE, ch, 0, victim, TO_NOTVICT);
            } else {
                act("Ваше оружие не эффективно против $N1.", FALSE, ch, 0, victim, TO_CHAR);
                act("Оружие $n1 не способно поразить Вас.", FALSE, ch, 0, victim, TO_VICT);
                act("Оружие $n1 не эффективно против $N1.", FALSE, ch, 0, victim, TO_NOTVICT);
            }
            return (RD_POWER);
        }
        //Увеличиваем кол-во опыта
        if (IS_NPC(victim) && dam > 0 && MAY_EXP(victim)) {
            int exp = 0;

            exp = GET_EXP(victim) / 500;
            exp = get_extend_exp(exp, ch, victim);
            exp = MIN(max_exp_hit_pc(ch), exp);
            if (exp > 0)
                gain_exp(ch, exp, FALSE);
        }
        //Расчитываем критический удар
        int mod = 0;

        if ((mod = affected_by_spell(ch, SPELL_EVIL_FATE)) && !damage.valid) {
            if (number(1, MAX(5, mod / 10)) >= GET_REAL_LCK(ch) && FIGHTING(ch)) {
                act("Вы нанесли неудачный удар по $N2.", FALSE, ch, 0, victim, TO_CHAR);
                act("$n нанес$q по Вам неудачный удар.", FALSE, ch, 0, victim, TO_VICT);
                act("$n нанес$q по $N2 неудачный удар.", FALSE, ch, 0, victim, TO_NOTVICT);
                SET_AF_BATTLE(ch, EAF_NOCRITIC);
                SET_AF_BATTLE(ch, EAF_USEDLEFT);
                SET_AF_BATTLE(ch, EAF_USEDRIGHT);
                GET_MISSED(ch)++;
                WAIT_STATE(ch, PULSE_VIOLENCE);
                if (FIGHTING(ch) == NULL)
                    set_fighting(ch, victim);
                return (RD_FAIL);
            }
        } else if (dice(1, 40) == 20 && GET_WAIT(ch) <= 0 && !damage.valid && !isCritic) {
            if (!GET_AF_BATTLE(ch, EAF_NOCRITIC) && number(1, 3) == 2 && FIGHTING(ch)) {
                act("Вы нанесли неудачный удар по $N2.", FALSE, ch, 0, victim, TO_CHAR);
                act("$n нанес$q по Вам неудачный удар.", FALSE, ch, 0, victim, TO_VICT);
                act("$n нанес$q по $N2 неудачный удар.", FALSE, ch, 0, victim, TO_NOTVICT);
                SET_AF_BATTLE(ch, EAF_NOCRITIC);
                SET_AF_BATTLE(ch, EAF_USEDLEFT);
                SET_AF_BATTLE(ch, EAF_USEDRIGHT);
                GET_MISSED(ch)++;
                WAIT_STATE(ch, PULSE_VIOLENCE);
                if (FIGHTING(ch) == NULL)
                    set_fighting(ch, victim);
                return (RD_FAIL);
            }
        }

        if (weapon && !GET_AF_BATTLE(ch, EAF_CRITIC)) {
            improove_skill(ch, victim, 0, skill);
            if (addskill)
                improove_skill(ch, victim, 0, addskill);
        }
        //Расчет дополнительных увеличивающих/уменьшающих эффектов
        /* 29.09.2018 Убираем штрафы
        if (!IS_NPC(ch) && weapon && ((GET_COND(ch, FULL) == 0 || GET_COND(ch, THIRST) == 0 || GET_COND(ch, SLEEP) == 0)) && GET_LEVEL(ch) >= 15)       //Хотим есть
            dam /= 2;
            */

        if (GET_POS(victim) < POS_FIGHTING && IS_SET(bl_bits, BL_IGNORE))
            dam =
                (int) ((float) dam *
                       (float) (1.0 +
                                (float) ((float) POS_FIGHTING - (float) GET_POS(victim)) / 3.0));

        if (GET_MOB_HOLD(victim) && bl_bits != BL_IGNORE)
            dam += (dam >> 1);

        //send_to_charf(ch,"Dam %d\r\n",dam);

        BATTLECNTR(victim)++;

        //Критические удары
        if (isCritic)
            dam = compute_critical(ch, victim, dam, location);

        //Расчет уклонений, парирований и блоков
        /* обработка уклона */
        if (GET_POS(ch) > POS_SLEEPING && ch != victim && weapon) {
            if (!in_use)
                if ((dam = check_parry(ch, victim, dam, bl_bits)) == -1)
                    return RD_DEVIATE;
            if (!in_use)
                if ((dam = check_block(ch, victim, dam, bl_bits, dtype)) == -1)
                    return RD_DEVIATE;
            if (!in_use)
                if ((dam = check_deviate(ch, victim, dam, bl_bits)) == -1)
                    return RD_DEVIATE;
        }

        if ((dam = calc_magic_decrease(ch, victim, dam, dtype, location, FALSE, afar)) == RD_MAGIC)
            return (RD_MAGIC);

        //щит боли
        if (affected_by_spell(victim, SPELL_SHIELD_OTHER))
            if (GET_ID(victim) != get_spell_owner(victim, SPELL_SHIELD_OTHER)) {
                struct char_data *owner =
                    get_char_by_id(get_spell_owner(victim, SPELL_SHIELD_OTHER));
                if (owner && (GET_HIT(owner) * 100 / GET_REAL_MAX_HIT(owner)) > 10) {
                    dam = dam / 2;
                    if (dam > 0) {
                        act("&RВы разделили боль с $N4.&n", FALSE, owner, 0, victim, TO_CHAR);
                        GET_HIT(owner) -= dam;
                        //owner->damage_rnd += dam;
                        update_pos(owner);
                    }
                }
            }
        //Наносим повреждения
        GET_HIT(victim) -= dam;
        victim->damage_rnd += dam;
        if (!update_pos(victim))
            return (RD_NONE);

        if (GET_POS(victim) == POS_DEAD) {      //Жертва умерла
            if (weapon && !GET_AF_BATTLE(ch, EAF_CRITIC))
                ShowHitMessage(ch, victim, weapObj, missileObj, dam, dmess, M_KILL, bLoc);
            else
                ShowMessage(ch, victim, weapObj, message, dam, dmess, M_KILL, bLoc, addMessage,
                            adam);
            death_cry(victim, ch);
            //Вычитаем стрелу
            if (missileObj != NULL && missileObj->missile) {
                missileObj->obj_flags.value[2]--;
                if (missileObj->obj_flags.value[2] <= 0)
                    extract_obj(missileObj);
            }
            //Добавляем экспу и все такое
            if (IS_NPC(victim) || victim->desc) {
                if (ch != victim)
                    killer = ch;

                if (killer) {
                    add_killers(victim, killer);        //добавляем в список киллеров
                    sumlevels = 0;
                    maxlevel = 0;
                    for (k = victim->killer_list.begin(); k != victim->killer_list.end(); k++) {
                        if ((!IS_NPC(*k) && IN_ROOM(*k) == IN_ROOM(victim)) /*||
                            (IS_NPC(*k) && IS_AFFECTED(*k, AFF_CHARM) && IS_ANIMAL(*k))*/) {
                            maxlevel = (maxlevel > GET_LEVEL(*k)) ? maxlevel : GET_LEVEL(*k);
                            sumlevels += GET_LEVEL(*k);
                        }
                    }

                    for (k = victim->killer_list.begin(); k != victim->killer_list.end(); k++) {
                        if (PRF_FLAGGED(*k, PRF_CODERINFO))
                            send_to_charf(*k, "Сумма %d Макс %d\r\n", sumlevels, maxlevel);

                        //act("1и убийца","Км",k->killer);
                        if (!IS_NPC(*k) && (IN_ROOM(*k) == IN_ROOM(victim))) {
                            kill_gain(*k, victim, maxlevel, sumlevels);
                            inc_honor(*k, victim, maxlevel, sumlevels);
                        }
                        if (IS_NPC(*k) && IS_AFFECTED(*k, AFF_CHARM) && IS_ANIMAL(*k))
                            kill_gain(*k, victim, GET_LEVEL(*k), GET_LEVEL(*k));
                    }
                }
            }
            if (!IS_NPC(victim)) {
                sprintf(buf, "%s убит %s на %s", GET_NAME(victim), GET_NAME(ch),
                        IN_ROOM(victim) != NOWHERE ? world[victim->in_room].name : "NOWHERE");
                mudlog(buf, BRF, LVL_IMMORT, TRUE);

                if (IS_NPC(ch) &&
                    (AFF_FLAGGED(ch, AFF_CHARM) || IS_HORSE(ch)) &&
                    ch->master && !IS_NPC(ch->master)) {
                    sprintf(buf, "%s подчиняется %s.", GET_NAME(ch), GET_PAD(ch->master, 2));
                    mudlog(buf, BRF, LVL_IMMORT, TRUE);
                }
            }
            die(victim, ch);
            return (RD_KILL);
        } else {
            if (!ignore) {
                if (weapon && !GET_AF_BATTLE(ch, EAF_CRITIC))
                    ShowHitMessage(ch, victim, weapObj, missileObj, dam, dmess, tMes, bLoc);
                else
                    ShowMessage(ch, victim, weapObj, message, dam, dmess, M_HIT, bLoc, addMessage,
                                adam);
            }
            if (weapon && weapObj && weapObj->spec_weapon)
                spec_weapon(ch, victim, weapObj);
            //Вычитаем стрелу
            if (missileObj != NULL && missileObj->missile) {
                missileObj->obj_flags.value[2]--;
                if (missileObj->obj_flags.value[2] <= 0)
                    extract_obj(missileObj);
            }
        }
        // Пытаемся отравить оружием
        if (dam && weapObj && weapon) {
            struct C_obj_affected_type *af;
            int mod;

            for (af = weapObj->C_affected; af; af = af->next)
                if (af->location == APPLY_POISON && !AFF_FLAGGED(victim, AFF_POISON)
                    && !IS_GOD(victim)) {
                    mod = MAX(3, af->modifier / 10);
                    improove_skill(ch, vict, 0, SKILL_POISONED);
                    if (!general_savingthrow_3(victim, SAV_FORT, mod))
                        poison_victim(ch, victim, af->modifier, weapObj);
                    af->modifier = af->modifier - 5;
                }
        } else
            if (dam && weapon && ch != victim &&
                IS_NPC(ch) &&
                NPC_FLAGGED(ch, NPC_POISON) &&
                !AFF_FLAGGED(ch, AFF_CHARM) &&
                GET_WAIT(ch) <= 0 &&
                !AFF_FLAGGED(victim, AFF_POISON) &&
                !general_savingthrow_3(victim, SAV_FORT, GET_LEVEL(ch))) {
            poison_victim(ch, victim, MAX(1, GET_LEVEL(ch) * 3), NULL);
        }

        char_dam_message(dam, ch, victim, dtype);
        //Если остались живы, начинам кричать alarm

        return (dam);
    }

    dam = 0;

    if (!ignore) {
        if (weapon && !GET_AF_BATTLE(ch, EAF_CRITIC))
            ShowHitMessage(ch, victim, weapObj, missileObj, dam, dmess, M_NONE, bLoc);
        else
            ShowMessage(ch, victim, weapObj, message, dam, dmess, M_NONE, bLoc, addMessage, adam);
    }
//Вычитаем стрелу
    if (missileObj != NULL && missileObj->missile) {
        missileObj->obj_flags.value[2]--;
        if (missileObj->obj_flags.value[2] <= 0)
            extract_obj(missileObj);
    }

    return RD_NONE;
}

/************************ УКЛОНЕНИЕ АТАК ************************************/
int check_deviate(struct char_data *ch, struct char_data *victim, int dam, int type)
{
    bool on = GET_AF_BATTLE(victim, EAF_DEVIATE);
    int skill =
        IS_MOB(victim) ? GET_SKILL_MOB(victim, SKILL_DEVIATE) : GET_SKILL(victim, SKILL_DEVIATE);
    int percent, prob;
    int bcount = MIN(3, MAX(1, skill / 32));

    if (GET_POS(victim) <= POS_SITTING)
        return (dam);

    if (!skill)
        return (dam);


    if (!on && IS_SET(type, BL_NODEVIATE))
        return (dam);

    if (!on && skill) {         //Не включено парирование
        skill = (skill * 7) / 10;
        bcount = 1;
        //return (dam);
    }

    if (dam <= 0 || GET_AF_BATTLE(victim, EAF_PARRY) || GET_AF_BATTLE(victim, EAF_BLOCK) || IS_SET(type, BL_IGNORE) ||  //удары с флагом игнорируются
        AFF_FLAGGED(victim, AFF_STOPFIGHT) ||
        AFF_FLAGGED(victim, AFF_STUNE) ||
        GET_WAIT(victim) || GET_MOB_HOLD(victim) || BATTLECNTR(victim) > bcount)
        return (dam);

    prob = GET_REAL_DEX(victim) + GET_REAL_DEX(victim) + GET_REAL_INT(victim) + skill + RNDSKILL;
    percent = GET_REAL_DEX(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) +
        ((GET_SAVE3(ch, SAV_REFL) + saving_throws_3(ch, SAV_REFL)) * 3) + RNDSKILL;

    if (IS_SET(type, BL_DUAL))
        percent += (percent * 25) / 100;

    if (PRF_FLAGGED(victim, PRF_CODERINFO))
        send_to_charf(victim, "&KПримерение умения 'уклонение' %d >= %d\r\n&n", prob, percent);

    if (BATTLECNTR(victim) == 1 && on)
        improove_skill(victim, ch, 0, SKILL_DEVIATE);

    if ((float) prob / (float) percent >= 1.5) {
        in_use = 1;
        act("Вы %2уклонились от %1 2+р.", "Ммтт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрела" : "удара", on ? "" : "рефлекторно ");
        act("1+и %2уклонил1(ся,ась,ось,ись) от Вашего %1.", "мМтт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрела" : "удара", on ? "" : "рефлекторно ");
        act("1+и %2уклонил1(ся,ась,ось,ись) от %1 2+р.", "Кммтт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрела" : "удара", on ? "" : "рефлекторно ");
        if (on) {
            GET_FSKILL(victim) = SKILL_DEVIATE;
            GET_SKILL_LAGR(victim, SKILL_DEVIATE) = 0;
            if (IS_MOB(victim))
                CLR_AF_BATTLE(victim, EAF_DEVIATE);
        }
        dam = -1;
    } else if ((float) prob / (float) percent >= 1) {
        in_use = 1;
        act("Вы %2уклонились от %1 2+р.", "Ммтт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрела" : "удара", on ? "частично " : "почти ");
        act("1+и %2уклонил1(ся,ась,ось,ись) от Вашего %1.", "мМтт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрела" : "удара", on ? "частично " : "почти ");
        act("1+и %2уклонил1(ся,ась,ось,ись) от %1 2+р.", "Кммтт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрела" : "удара", on ? "частично " : "почти ");
        dam = MAX(1, (int) ((dam * (100 - prob)) / 100));
        if (on) {
            GET_FSKILL(victim) = SKILL_DEVIATE;
            GET_SKILL_LAGR(victim, SKILL_DEVIATE) = 0;
            if (IS_MOB(victim))
                CLR_AF_BATTLE(victim, EAF_DEVIATE);
        }
    } else if (on) {
        act("Вы не смогли уклониться от %1 2+р.", "Ммт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрела" : "удара");
        act("1+и не сумел1(,а,о,и) уклониться Вашего %1.", "мМт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрела" : "удара");
        act("1+и не сумел1(,а,о,и) уклониться от %1 2+р.", "Кммт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрела" : "удара");
        stop_defence(victim);
        GET_SKILL_LAGR(victim, SKILL_DEVIATE) = 1;
    }

    return (dam);
}

/************************ ПАРИРОВАНИЕ АТАК ************************************/
int check_parry(struct char_data *ch, struct char_data *victim, int dam, int type)
{
    bool on = GET_AF_BATTLE(victim, EAF_PARRY);
    int skill =
        IS_MOB(victim) ? GET_SKILL_MOB(victim, SKILL_PARRY) : GET_SKILL(victim, SKILL_PARRY);
    int percent, prob;
    int bcount = MIN(4, MAX(1, skill / 25));
    struct obj_data *weapon =
        GET_EQ(victim, WEAR_WIELD) ? GET_EQ(victim, WEAR_WIELD) : GET_EQ(victim, WEAR_BOTHS);

    if (GET_POS(victim) <= POS_SITTING)
        return (dam);

    if (!skill)
        return (dam);

    if (!on && IS_SET(type, BL_NODEVIATE))
        return (dam);

    if (!on && skill) {         //Не включено парирование
        skill = skill * 7 / 10;
        bcount = 1;
//  return (dam);
    }


    if (dam <= 0 || GET_AF_BATTLE(victim, EAF_DEVIATE) || GET_AF_BATTLE(victim, EAF_BLOCK) || IS_SET(type, BL_IGNORE) ||        //удары с флагом игнорируются
        IS_SET(type, BL_FAR) || //стрелы парировать тоже не можем
        AFF_FLAGGED(victim, AFF_STOPFIGHT) ||
        AFF_FLAGGED(victim, AFF_STOPRIGHT) ||
        AFF_FLAGGED(victim, AFF_STUNE) ||
        GET_WAIT(victim) || GET_MOB_HOLD(victim) || BATTLECNTR(victim) > bcount)
        return (dam);

    if ((!weapon || GET_OBJ_TYPE(weapon) != ITEM_WEAPON ||
         (GET_OBJ_SKILL(weapon) == SKILL_BOWS ||
          GET_OBJ_SKILL(weapon) == SKILL_CROSSBOWS)) && !IS_NPC(victim)) {
        if (on) {
            act("Вам нечем парировать удар противника.", "М", victim);
            stop_defence(victim);
        }
        return (dam);
    }

    prob = GET_REAL_STR(victim) + GET_REAL_DEX(victim) + GET_REAL_INT(victim) + skill + RNDSKILL;
    percent =
        GET_REAL_STR(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) +
        ((GET_SAVE3(ch, SAV_REFL) + saving_throws_3(ch, SAV_REFL)) * 3) + RNDSKILL;

    if (GET_EQ(victim, WEAR_BOTHS))     //Если двуручник то получаем хороший бонус на парирование
        prob += GET_OBJ_WEIGHT(weapon) / 1000;

    if (IS_SET(type, BL_DUAL))
        percent += (percent * 30) / 100;


    if (PRF_FLAGGED(victim, PRF_CODERINFO))
        send_to_charf(victim, "&KПримерение умения 'парирование' %d >= %d\r\n&n", prob, percent);

    if (BATTLECNTR(victim) == 1 && on)
        improove_skill(victim, ch, 0, SKILL_PARRY);

    if ((float) prob / (float) percent >= 1.5) {
        in_use = 1;
        if (on) {
            act("Вы парировали удар 2+р.", "Ммт", victim, ch);
            act("1+и парировал1(,а,о,и) Ваш удар.", "мМт", victim, ch);
            act("1+и парировал1(,а,о,и) удар 2+р.", "Кммт", victim, ch);
            GET_SKILL_LAGR(victim, SKILL_PARRY) = 0;
            GET_FSKILL(victim) = SKILL_PARRY;
            if (IS_MOB(victim))
                CLR_AF_BATTLE(victim, EAF_PARRY);
        } else {
            act("Вы рефлекторно отвели удар 2+р.", "Ммт", victim, ch);
            act("1+и рефлекторно отвел1(,а,о,и) Ваш удар.", "мМт", victim, ch);
            act("1+и рефлекторно отвел1(,а,о,и) удар 2+р.", "Кммт", victim, ch);
        }
        dam = -1;
    } else if ((float) prob / (float) percent >= 0.8) {
        in_use = 1;
        dam = MAX(1, (int) ((dam * (100 - prob)) / 100));
        if (on) {
            act("Вы частично парировали атаку 2+р.", "Мм", victim, ch);
            act("1+и частично парировал1(,а,о,и) Вашу атаку.", "мМ", victim, ch);
            act("1+и частично парировал1(,а,о,и) атаку 2+р.", "Кмм", victim, ch);
            GET_SKILL_LAGR(victim, SKILL_PARRY) = 0;
            GET_FSKILL(victim) = SKILL_PARRY;
            if (IS_MOB(victim))
                CLR_AF_BATTLE(victim, EAF_PARRY);
        } else {
            act("Вы немного отвели удар 2+р.", "Ммт", victim, ch);
            act("1+и немного отвел1(,а,о,и) Ваш удар.", "мМт", victim, ch);
            act("1+и немного отвел1(,а,о,и) удар 2+р.", "Кммт", victim, ch);
        }
    } else if (on) {
        act("Вы не смогли парировать удар 2+р.", "Мм", victim, ch);
        act("1+и не сумел1(,а,о,и) парировать Ваш удар.", "мМ", victim, ch);
        act("1+и не сумел1(,а,о,и) парировать удар 2+р.", "Кмм", victim, ch);
        stop_defence(victim);
        GET_SKILL_LAGR(victim, SKILL_PARRY) = 1;
    }

    return (dam);
}

/************************ БЛОКИРОВКА АТАК ************************************/
int check_block(struct char_data *ch, struct char_data *victim, int dam, int type, int rTyp)
{
    bool on = GET_AF_BATTLE(victim, EAF_BLOCK);
    int skill =
        IS_MOB(victim) ? GET_SKILL_MOB(victim, SKILL_BLOCK) : GET_SKILL(victim, SKILL_BLOCK);
    struct obj_data *shield = GET_EQ(victim, WEAR_SHIELD);
    int percent, prob;
    int bcount = MIN(8, MAX(1, skill / 15));

    if (GET_POS(victim) <= POS_SITTING)
        return (dam);

    if (!skill)
        return (dam);

    if (!on && IS_SET(type, BL_NODEVIATE))
        return (dam);

    if (!on && skill) {         //Не включен блок
        skill = skill * 6 / 10;
        bcount = 2;
        //return (dam);
    }

    if (dam <= 0 || GET_AF_BATTLE(victim, EAF_DEVIATE) || GET_AF_BATTLE(victim, EAF_PARRY) || IS_SET(type, BL_IGNORE) ||        //удары с флагом игнорируются
        AFF_FLAGGED(victim, AFF_STOPFIGHT) ||
        AFF_FLAGGED(victim, AFF_STOPRIGHT) ||
        AFF_FLAGGED(victim, AFF_STUNE) ||
        GET_WAIT(victim) || GET_MOB_HOLD(victim) || BATTLECNTR(victim) > bcount)
        return (dam);

    if (!shield && !IS_NPC(victim)) {
        if (on)
            act("Вам нечем прикрыться от удара противника.", "М", victim);
        stop_defence(victim);
        return (dam);
    }

    prob = GET_REAL_STR(victim) + GET_REAL_DEX(victim) + GET_REAL_INT(victim) + skill + RNDSKILL;
    percent = GET_REAL_STR(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) +
        ((GET_SAVE3(ch, SAV_REFL) + saving_throws_3(ch, SAV_REFL)) * 3) + RNDSKILL;

    if (IS_SET(type, BL_DUAL))
        percent += (percent * 35) / 100;

    if (shield) {
        prob += GET_OBJ_WEIGHT(shield) / 1000;
        switch (GET_ARM_TYPE(shield)) {
            case TARM_NONE:
            case TARM_LIGHT:
            case TARM_WEAR:
            case TARM_JEWS:    //легкий щит
                percent += (percent * 5) / 100;
                break;
            case TARM_MEDIUM:
                percent += (percent * 2) / 100;
                break;
            case TARM_HARD:
                percent -= (percent * 15) / 100;
                break;
        }
    }

    if (PRF_FLAGGED(victim, PRF_CODERINFO))
        send_to_charf(victim, "&KПримерение умения 'блок щитом' %.1f vs 1.5|1|&n\r\n",
                      (float) prob / (float) percent);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&KПримерение умения 'блок щитом' против Вас %.1f vs 1.5|1|&n\r\n",
                      (float) prob / (float) percent);

    if (BATTLECNTR(victim) == 1)
        improove_skill(victim, ch, 0, SKILL_BLOCK);

    if ((float) prob / (float) percent >= 1.5) {
        in_use = 1;
        if (!shield) {
            act("Вы полностью блокировали %1 2+р.", "Ммт", victim, ch,
                IS_SET(type, BL_FAR) ? "выстрел" : "удар");
            act("1+и полностью блокировал1(,а,о,и) Ваш %1.", "мМт", victim, ch,
                IS_SET(type, BL_FAR) ? "выстрел" : "удар");
            act("1+и полностью блокировал1(,а,о,и) %1 2+р.", "Кммт", victim, ch,
                IS_SET(type, BL_FAR) ? "выстрел" : "удар");
        } else {
            act("Вы %2прикрылись @1т от %1 2+р.", "Ммптт", victim, ch, shield,
                IS_SET(type, BL_FAR) ? "выстрела" : "удара", on ? "" : "рефлекторно ");
            act("1+и %2прикрыл1(ся,ась,ось,ись) @1т от Вашего %1.", "мМптт", victim, ch, shield,
                IS_SET(type, BL_FAR) ? "выстрела" : "удара", on ? "" : "рефлекторно ");
            act("1+и %2прикрыл1(ся,ась,ось,ись) @1т от %1 2+р.", "Кммптт", victim, ch, shield,
                IS_SET(type, BL_FAR) ? "выстрела" : "удара", on ? "" : "рефлекторно ");
        }
        GET_FSKILL(victim) = SKILL_BLOCK;
        GET_SKILL_LAGR(victim, SKILL_BLOCK) = 0;
        alt_armor(victim, ch, MAX(1, dam / 3), WEAR_SHIELD, rTyp);
        if (IS_MOB(victim))
            CLR_AF_BATTLE(victim, EAF_BLOCK);
        dam = -1;
    } else if ((float) prob / (float) percent >= 1) {
        in_use = 1;
        if (!shield) {
            act("Вы частично блокировали %1 2+р.", "Ммт", victim, ch,
                IS_SET(type, BL_FAR) ? "выстрел" : "удар");
            act("1+и частично блокировал1(,а,о,и) Ваш %1.", "мМт", victim, ch,
                IS_SET(type, BL_FAR) ? "выстрел" : "удар");
            act("1+и частично блокировал1(,а,о,и) %1 2+р.", "Кммт", victim, ch,
                IS_SET(type, BL_FAR) ? "выстрел" : "удар");
        } else {
            act("Вы частично прикрылись @1т от %1 2+р.", "Ммпт", victim, ch, shield,
                IS_SET(type, BL_FAR) ? "выстрела" : "удара");
            act("1+и частично прикрыл1(ся,ась,ось,ись) @1т от Вашего %1.", "мМпт", victim, ch,
                shield, IS_SET(type, BL_FAR) ? "выстрела" : "удара");
            act("1+и частично прикрыл1(ся,ась,ось,ись) @1т от %1 2+р.", "Кммпт", victim, ch, shield,
                IS_SET(type, BL_FAR) ? "выстрела" : "удара");
        }
        GET_FSKILL(victim) = SKILL_BLOCK;
        alt_armor(victim, ch, MAX(1, dam / 5), WEAR_SHIELD, rTyp);
        dam = MAX(1, (int) ((dam * (100 - prob)) / 100));
        GET_SKILL_LAGR(victim, SKILL_BLOCK) = 0;
        if (IS_MOB(victim))
            CLR_AF_BATTLE(victim, EAF_BLOCK);
    } else if (on) {
        act("Вы не смогли блокировать %1 2+р.", "Ммт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрел" : "удар");
        act("1+и не сумел1(,а,о,и) блокировать Ваш %1.", "мМт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрел" : "удар");
        act("1+и не сумел1(,а,о,и) блокировать %1 2+р.", "Кммт", victim, ch,
            IS_SET(type, BL_FAR) ? "выстрел" : "удар");
        stop_defence(victim);
        //SET_AF_BATTLE(victim, EAF_USEDRIGHT);
        GET_SKILL_LAGR(victim, SKILL_BLOCK) = 1;
    }

    return (dam);
}

void spec_weapon(struct char_data *ch, struct char_data *victim, struct obj_data *weapObj)
{
    struct spec_weapon_data *k;
    int fnd, i;

    for (k = weapObj->spec_weapon; k; k = k->next)
        if (k->shance >= number(1, 100)) {
            fnd = FALSE;
            for (i = 0; i < NUM_RACES; i++)
                if (k->zrace[i])
                    fnd = TRUE;
            if (!fnd && !k->zrace[(int) GET_RACE(victim)]) {
                if (k->xrace[(int) GET_RACE(victim)])
                    return;
            } else if (fnd && !k->zrace[(int) GET_RACE(victim)])
                return;

            fnd = FALSE;
            for (i = 0; i < NUM_TMOBS; i++)
                if (k->zmtype[i])
                    fnd = TRUE;
            if (IS_NPC(victim)) {
                if (!fnd && !k->zmtype[(int) GET_MOB_TYPE(victim)]) {
                    if (k->xmtype[(int) GET_MOB_TYPE(victim)])
                        return;
                } else if (fnd && !k->zmtype[(int) GET_MOB_TYPE(victim)])
                    return;
            }

            if (k->type_damage) {
                struct P_damage damage;
                struct P_message message;

                damage.valid = true;
                damage.type = k->type_damage;
                damage.power = GET_OBJ_POWER(weapObj);
                damage.dam = number(k->min_damage, k->max_damage);
                damage.check_ac = A_POWER;
                damage.far_min = FALSE;
                damage.magic = TRUE;
                damage.armor = FALSE;   //броня учитывается

                message.valid = true;
                if (k->to_char) {
                    message.kChar = k->to_char;
                    message.hChar = k->to_char;
                    message.mChar = k->to_char;
                    message.aChar = k->to_char;
                    message.bChar = k->to_char;
                }
                if (k->to_vict) {
                    message.kVict = k->to_vict;
                    message.hVict = k->to_vict;
                    message.mVict = k->to_vict;
                    message.aVict = k->to_vict;
                    message.bVict = k->to_vict;
                }
                if (k->to_room) {
                    message.kRoom = k->to_room;
                    message.hRoom = k->to_room;
                    message.mRoom = k->to_room;
                    message.aRoom = k->to_room;
                    message.bRoom = k->to_room;
                }
                _damage(ch, victim, 0, 0, C_POWER, TRUE, damage, message);
            }
            if (k->level && k->spell) {
                if (k->to_char) {
                    send_to_char(CCIYEL(ch, C_CMP), ch);
                    act(k->to_char, "Ммп", ch, victim, weapObj);
                    send_to_char(CCNRM(ch, C_CMP), ch);
                }
                if (k->to_vict) {
                    send_to_char(CCIRED(victim, C_CMP), victim);
                    act(k->to_vict, "мМп", ch, victim, weapObj);
                    send_to_char(CCNRM(victim, C_CMP), victim);
                }
                if (k->to_room)
                    act(k->to_room, "Кммп", ch, victim, weapObj);
            }
            int fspl = find_spell_num(k->spell);

            if (fspl != -1 && k->level)
                call_magic(ch, victim, weapObj, find_spell_num(k->spell), k->level, CAST_WEAPON,
                           FALSE);
        }
}


int get_weapon_damage(struct char_data *ch, struct char_data *victim,
                      struct obj_data *weapObj, struct obj_data *missileObj,
                      int isCritic, int location, int afar, int slevel, int dual)
{
    struct weapon_damage_data *damages;
    int dam = 0, tdam = 0, dtype;


    for (damages = weapObj->weapon->damages; damages; damages = damages->next) {
        dtype = damages->type_damage;
        tdam = number(damages->min_damage, damages->max_damage);
        if (dual)
            tdam += (GET_REAL_DR(ch) * 3) / 2;
        else
            tdam += GET_REAL_DR(ch);
        if (!isCritic)
            if ((tdam =
                 calc_armor_decrease(ch, victim, tdam, dtype, location, FALSE, afar,
                                     slevel)) == RD_ARMOR)
                tdam = 0;
        dam += tdam;
    }
    if (missileObj != NULL) {
        for (damages = missileObj->missile->damages; damages; damages = damages->next) {
            dtype = damages->type_damage;
            tdam = number(damages->min_damage, damages->max_damage);
            if (!isCritic)
                if ((tdam =
                     calc_armor_decrease(ch, victim, tdam, dtype, location, FALSE, afar,
                                         slevel)) == RD_ARMOR)
                    tdam = 0;
            dam += tdam;
        }
    }
    dam = MAX(1, MIN(dam, dam * GET_OBJ_CUR(weapObj) / MAX(1, GET_OBJ_MAX(weapObj))));

    return dam;
}
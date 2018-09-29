/* ************************************************************************
*   File: act.offensive.c                               Part of CircleMUD *
*  Usage: player-level commands of an offensive nature                    *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "constants.h"
#include "screen.h"
#include "spells.h"
#include "xskills.h"
#include "xboot.h"
#include "pk.h"
#include "case.h"

/* extern functions */
ACMD(do_stand);

/* prevent accidental pkill */
#define CHECK_PKILL(ch,opponent) (!IS_NPC(opponent) ||\
                                  (opponent->master &&!IS_NPC(opponent->master) && opponent->master != ch))


bool check_ground(struct char_data * ch)
{
    if (!IS_NPC(ch) && (((affected_by_spell(ch, SPELL_FLY) || IS_AFFECTED(ch, AFF_FLY) ||
                          IS_AFFECTED(ch, AFF_LEVIT)) && GET_POS(ch) == POS_FLYING) ||
                        real_sector(IN_ROOM(ch)) == SECT_FLYING ||
                        real_sector(IN_ROOM(ch)) == SECT_UNDERWATER ||
                        real_sector(IN_ROOM(ch)) == SECT_WATER_NOSWIM ||
                        real_sector(IN_ROOM(ch)) == SECT_WATER_SWIM)) {
        send_to_charf(ch, "�� ������ �������� ������ �� �����.\r\n");
        return (FALSE);
    }

    return (TRUE);
}

int check_pkill(struct char_data *ch, struct char_data *opponent, char *arg)
{
    char *pos;

    if (!*arg)
        return (FALSE);

    /* if (!CHECK_PKILL(ch,opponent))
       return (FALSE); */

    if (FIGHTING(ch) == opponent || FIGHTING(opponent) == ch)
        return (FALSE);

    while (*arg && (*arg == '.' || (*arg >= '0' && *arg <= '9')))
        arg++;

    if ((pos = strchr(arg, '!')))
        *pos = '\0';

    if ((!IS_NPC(opponent) || (IS_NPC(opponent) && IS_AFFECTED(opponent, AFF_CHARM)))
        && str_cmp(arg, GET_NAME(opponent)))
        /*    if (IS_NPC(opponent) &&
           IS_AFFECTED(opponent,AFF_CHARM) &&
           str_cmp(arg, GET_NAME(opponent))) */
    {
        send_to_char("��� ���������� ������������� ������� ��� ���������.\r\n", ch);
        if (pos)
            *pos = '!';
        return (TRUE);
    }
    if (pos)
        *pos = '!';
    return (FALSE);
}

int have_mind(struct char_data *ch)
{
    if (!AFF_FLAGGED(ch, AFF_CHARM) && !IS_HORSE(ch))
        return (TRUE);
    return (FALSE);
}

void set_wait(struct char_data *ch, int waittime, int victim_in_room)
{
    if ((!victim_in_room || (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
        )
        )
        WAIT_STATE(ch, waittime * PULSE_VIOLENCE);
};

int check_hit(struct char_data *ch, struct char_data *victim)
{
    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return (FALSE);
    }
    if (FIGHTING(ch) || GET_MOB_HOLD(ch)) {
        return (FALSE);
    }

    return (TRUE);
}

int set_hit(struct char_data *ch, struct char_data *victim)
{
    if (!check_hit(ch, victim))
        return (FALSE);
    if (affected_by_spell(ch, SPELL_CAUSE_FEAR)) {
        send_to_charf(ch, "�� ������� ��������, ��� �� ������� �� ����-������.\r\n");
        return (FALSE);
    }
    if (IS_NPC(ch))
        do_stand(ch, 0, 0, 0, 0);
    _damage(ch, victim, WEAP_RIGHT, 0, TRUE, C_POWER);
    set_wait(ch, 1, TRUE);
    return (TRUE);
};

int onhorse(struct char_data *ch)
{
    if (on_horse(ch)) {
        act("��� ������ $N.", FALSE, ch, 0, get_horse(ch), TO_CHAR);
        return (TRUE);
    }
    return (FALSE);
};

int used_attack(struct char_data *ch)
{
    const char *message = NULL;

    if (GET_AF_BATTLE(ch, EAF_BLOCK))
        message = "�� ��������� ����������� �����.";
    else if (GET_AF_BATTLE(ch, EAF_PARRY))
        message = "�� ��������� ���������� �����.";
    else if (GET_AF_BATTLE(ch, EAF_MULTYPARRY))
        message = "�� ������������� �� �������� ������.";
    else if (GET_AF_BATTLE(ch, EAF_DEVIATE))
        message = "�� ��������� ����������.";
    if (!GET_EXTRA_VICTIM(ch))
        return (FALSE);
    else
        switch (GET_EXTRA_SKILL(ch)) {
            case SKILL_BASH:
                message = "�� ��������� ����� $N3.";
                break;
            case SKILL_KICK:
                message = "�� ��������� ����� $N3.";
                break;
            case SKILL_CHOPOFF:
                message = "�� ��������� ������� $N3.";
                break;
            case SKILL_DISARM:
                message = "�� ��������� ����������� $N3.";
                break;
            case SKILL_THROW:
                message = "�� ��������� ������� ������ � $N3.";
                break;
            default:
                return (FALSE);
        }
    if (message)
        act(message, FALSE, ch, 0, GET_EXTRA_VICTIM(ch), TO_CHAR);
    return (TRUE);
}

ACMD(do_assist)
{
    struct char_data *helpee, *opponent;
    char arg[MAX_STRING_LENGTH];

    if (FIGHTING(ch)) {
        send_to_char("�� ���������� ����.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!*arg) {
        for (helpee = world[ch->in_room].people; helpee; helpee = helpee->next_in_room)
            if (FIGHTING(helpee) && FIGHTING(helpee) != ch && helpee != ch &&
                (same_group(ch, helpee) ||
                 ((IS_AFFECTED(ch, AFF_CHARM) || IS_AFFECTED(helpee, AFF_CHARM))
                  && (ch->master == helpee || (ch->master && ch->master == helpee->master)
                      || helpee->master == ch)
                 )))
                break;

        if (!helpee) {
            send_to_char("���� �� ������ ������?\r\n", ch);
            return;
        }
    } else if (!(helpee = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_char(NOPERSON, ch);
        return;
    } else if (helpee == ch) {
        send_to_char("��� ����� ������ ������ ����!\r\n", ch);
        return;
    }
    /*
     * Hit the same enemy the person you're helping is.
     */

    if (FIGHTING(helpee))
        opponent = FIGHTING(helpee);
    else
        for (opponent = world[ch->in_room].people;
             opponent && (FIGHTING(opponent) != helpee); opponent = opponent->next_in_room);

    if (!opponent)
        act("�� ����� �� ��������� � $N4!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!CAN_SEE(ch, opponent))
        act("�� �� ������ ���������� $N1!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (opponent == ch)
        act("$E ��������� � ����!", FALSE, ch, 0, helpee, TO_CHAR);
    else if (!may_kill_here(ch, opponent))
        return;
    else {
        if (check_hit(ch, opponent)) {
            act("�� �������������� � �����, ������� $N2!", FALSE, ch, 0, helpee, TO_CHAR);
            act("$N �����$G ������ ��� � �����!", 0, helpee, 0, ch, TO_CHAR);
            act("$n �������$g � ��� �� ������� $N1.", FALSE, ch, 0, helpee, TO_NOTVICT);
            set_fighting(ch, opponent);
            //_damage(ch,opponent,WEAP_RIGHT,0,C_POWER);
            set_wait(ch, 1, TRUE);
        }
    }
}


ACMD(do_hit)
{
    struct char_data *vict;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (FIGHTING(ch)) {
        send_to_char("������-�� �� ��� ���������� � ���.\r\n", ch);
        return;
    }
    if (!*arg)
    {
        for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room)
        {
            if (!vict or !IS_NPC(vict))
                continue;
            strcpy(arg, vict->player.name);
            break;
        }
    }
    
    if (!*arg)
        send_to_char("�� ���� �������?\r\n", ch);
    else if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
        send_to_char("�� �� ������ ����.\r\n", ch);
    else if (vict == ch) {
        send_to_char("�� ������� ����!\r\n", ch);
        act("$n ������$g ����, � �������$g �� ����.", FALSE, ch, 0, vict, TO_ROOM);
    } else if (!may_kill_here(ch, vict))
        return;
    else if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master == vict))
        act("$N ������� ����� ��� ���, ����� ���� $S.", FALSE, ch, 0, vict, TO_CHAR);
    else {
        if (subcmd != SCMD_MURDER && check_pkill(ch, vict, arg))
            return;
        if (FIGHTING(ch)) {
            if (vict == FIGHTING(ch)) {
                act("�� ��� ���������� � $N4.", FALSE, ch, 0, vict, TO_CHAR);
                return;
            }
            if (!FIGHTING(vict)) {
                act("$N �� ��������� � ����, �� �������� $S.", FALSE, ch, 0, vict, TO_CHAR);
                return;
            }
            stop_fighting(ch, FALSE);
            set_fighting(ch, vict);
            set_wait(ch, 1, TRUE);
        } else if ((GET_POS(ch) >= POS_STANDING) && (vict != FIGHTING(ch))) {
            set_hit(ch, vict);
        } else
            send_to_char("��� ���� �� �� ���!\r\n", ch);
    }
}



ACMD(do_kill)
{
    struct char_data *vict;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);


    if (!*arg) {
        send_to_char("��� ����������!\r\n", ch);
    } else {
        if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)))
            send_to_char("����?\r\n", ch);
        else if (ch == vict)
            send_to_char("������� ������?\r\n", ch);
        else if (IS_GOD(vict) && !GET_COMMSTATE(ch))
            send_to_char("������� �� ����?! �����������!\r\n", ch);
        else {
            act("�� �������� $N3 � ����! ��������! �����!", FALSE, ch, 0, vict, TO_CHAR);
            act("$N �������$g ��� � ���� ����� ����������� ������!", FALSE, vict, 0, ch, TO_CHAR);
            act("$n ������ ���������$g �������� $N3!", FALSE, ch, 0, vict, TO_NOTVICT);
            die(vict, ch);
        }
    }
}

/************************ BACKSTAB VICTIM */
void go_backstab(struct char_data *ch, struct char_data *vict)
{
    int percent, prob, hTyp, weap_i, dam;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_BACKSTAB) : GET_SKILL(ch, SKILL_BACKSTAB);
    struct P_damage damage;
    struct P_message message;
    struct obj_data *weapObj = NULL;
    struct weapon_damage_data *damages;

    if (onhorse(ch))
        return;

    if (!ok_damage_shopkeeper(ch, vict))
        return;

    if (check_spells_attacked(ch, vict, FALSE, FALSE))
        return;

    if (may_pkill(ch, vict) != PC_REVENGE_PC)
        inc_pkill_group(vict, ch, 1, 0);


    if (FIGHTING(vict)) {
        send_to_char("���� ���� ������� ������ �������������!\r\n", ch);
        return;
    }

    if (FIGHTING(ch)) {
        send_to_char("�� �� ������ �������� �� ���!\r\n", ch);
        return;
    }

    if (IS_UNDEAD(vict) || IS_CONSTRUCTION(vict)) {
        //���� ������ �� ������ ���� ��� �����
        act("�� �� ����� ��������� ����� � $N1.", FALSE, ch, 0, vict, TO_CHAR);
        set_hit(ch, vict);
        return;
    }

    if ((MOB_FLAGGED(vict, MOB_AWARE) && AWAKE(vict)) || FIGHTING(vict)) {
        act("�� ��������, ��� $N �������$y ��� ��������!", FALSE, vict, 0, ch, TO_CHAR);
        act("$n �������$g ���� ������� �������� $s!", FALSE, vict, 0, ch, TO_VICT);
        act("$n �������$g ������� $N1 �������� $s!", FALSE, vict, 0, ch, TO_NOTVICT);
        set_hit(vict, ch);
        return;
    }

    if (GET_EQ(ch, WEAR_WIELD) && GET_OBJ_SKILL(GET_EQ(ch, WEAR_WIELD)) == SKILL_DAGGERS) {
        weapObj = GET_EQ(ch, WEAR_WIELD);
        hTyp = GET_OBJ_VAL(weapObj, 3);
    } else
        hTyp = 11;              //�������

    if (!weapObj && ((weap_i = real_object(OBJ_KNIF)) >= 0))
        weapObj = (obj_proto + weap_i);

    damage.valid = true;

    dam = 0;
    if (weapObj) {
        for (damages = weapObj->weapon->damages; damages; damages = damages->next)
            if (damages->type_damage == HIT_PICK || damages->type_damage >= HIT_FIRE) {
                damage.type = damages->type_damage;
                dam += number(damages->min_damage, damages->max_damage);
            }

        skill = (GET_SKILL(ch, SKILL_BACKSTAB) + GET_SKILL(ch, SKILL_DAGGERS)) / 2;
        dam = MAX(1, MIN(dam, dam * GET_OBJ_CUR(weapObj) / MAX(1, GET_OBJ_MAX(weapObj))));
        //dam = (int)((float)dam*(float)MAX((float)2.0,(float)((float)skill/20.0)));
        dam = dam + (dam * skill) / 15;
        damage.dam = dam;
    } else if (IS_NPC(ch)) {
        //� ���� �� ������ ����� ���� ������ �����
        dam =
            dice(ch->npc()->specials.damnodice,
                 ch->npc()->specials.damsizedice) + (ch->npc()->specials.damage);
        skill = GET_SKILL_MOB(ch, SKILL_BACKSTAB) + GET_SKILL_MOB(ch, SKILL_DAGGERS);
        dam = dam + (dam * skill) / 15;
        damage.dam = dam;
    } else                      //�� ���� ������ ���� �� �����, �� ��� ��
        damage.dam = 100;

    damage.power = GET_POWER(ch);
    damage.power += weapObj ? GET_OBJ_POWER(weapObj) : 0;
    damage.far_min = FALSE;
    damage.deviate = TRUE;
    damage.armor = TRUE;
    damage.weapObj = weapObj ? weapObj : NULL;

    GetSkillMessage(SKILL_BACKSTAB, hTyp, message);
    percent = number(1, 100);

    prob =
        GET_REAL_DEX(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) + GET_SKILL(ch,
                                                                           SKILL_BACKSTAB) +
        RNDSKILL;
    percent =
        GET_REAL_DEX(vict) + GET_REAL_DEX(vict) + GET_REAL_INT(vict) + GET_REAL_AC(vict) +
        ((GET_SAVE3(vict, SAV_REFL) + saving_throws_3(vict, SAV_REFL)) * 3) + RNDSKILL;

    if (!CAN_SEE(vict, ch))
        prob *= 2;

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (percent > prob)
        damage.check_ac = N_POWER;      //������
    else
        damage.check_ac = A_POWER;      //������

    if (!IS_NPC(ch) && !IS_NPC(vict))
        damage.dam = (damage.dam * 8) / 10;     //PvP ���� ������ �� 20%

    _damage(ch, vict, 0, 0, C_POWER, TRUE, damage, message);

    improove_skill(ch, vict, 0, SKILL_BACKSTAB);

    set_wait(ch, 1, TRUE);
}

void go_runup(struct char_data *ch, struct char_data *vict)
{
    int percent, prob, dam = 0;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_RUNUP) : GET_SKILL(ch, SKILL_RUNUP);
    struct P_damage damage;
    struct P_message message;

    if (!on_horse(ch) && !IS_BARIAUR(ch)) {
        send_to_charf(ch, "��� ����� �� ������ ���� ������.\r\n");
        return;
    }

    if (!check_ground(ch))
        return;

    if (may_pkill(ch, vict) != PC_REVENGE_PC)
        inc_pkill_group(vict, ch, 1, 0);

    if (check_spells_attacked(ch, vict, FALSE, FALSE))
        return;

    if (IS_NPC(ch)) {
        int dam;

        dam =
            dice(ch->npc()->specials.damnodice,
                 ch->npc()->specials.damsizedice) + (ch->npc()->specials.damage);
        dam = dam + (dam * skill) / 50;
    } else if (IS_BARIAUR(ch))
        if (GET_EQ(ch, WEAR_HEAD) && GET_OBJ_TYPE(GET_EQ(ch, WEAR_HEAD)) == ITEM_ARMOR)
            dam += GET_OBJ_WEIGHT(GET_EQ(ch, WEAR_HEAD)) / 250;

    dam += GET_REAL_DR(ch);
    dam = dam + ((dam / 3) * skill) / 60;

    damage.valid = true;
    damage.type = real_attack_type(14); //������
    damage.power = GET_POWER(ch);
    damage.far_min = FALSE;
    damage.weapObj = NULL;
    damage.armor = FALSE;
    damage.dam = dam;

    prob =
        GET_REAL_STR(ch) + GET_REAL_STR(ch) + GET_REAL_DEX(ch) + GET_REAL_WEIGHT(ch) + skill +
        RNDSKILL;
    percent =
        GET_REAL_STR(vict) + GET_REAL_DEX(vict) + GET_REAL_DEX(vict) + GET_REAL_WEIGHT(vict) +
        ((GET_SAVE3(vict, SAV_REFL) + saving_throws_3(vict, SAV_REFL)) * 3) + RNDSKILL;

    improove_skill(ch, vict, 0, SKILL_RUNUP);

    if (IS_BARIAUR(ch)) {
        GetSkillMessage(SKILL_RUNUPB, 14, message);
        prob = (prob * 135) / 100;      //��������� 35% ���������
    } else
        GetSkillMessage(SKILL_RUNUP, 10, message);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (percent > prob) {
        damage.check_ac = N_POWER;      //������
        _damage(ch, vict, 0, 0, C_POWER, TRUE, damage, message);
        if (dice(1, MAX(1, skill + GET_REAL_LCK(ch))) <= 9 && get_horse_on(ch)) {

            act("�� ����� � $N1.", FALSE, ch, 0, get_horse(ch), TO_CHAR);
            act("$n ����$g � $N1.", TRUE, ch, 0, get_horse(ch), TO_ROOM);
            GET_POS(ch) = POS_SITTING;
            REMOVE_BIT(AFF_FLAGS(ch, AFF_HORSE), AFF_HORSE);
            if (IS_HORSE(ch) && on_horse(ch->master))
                horse_drop(ch);
        }
        WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
    } else {
        damage.check_ac = A_POWER;      //������
        _damage(ch, vict, 0, 0, C_POWER, TRUE, damage, message);
        if (GET_POS(vict) == POS_STANDING && prob / 2 > percent && !MOB_FLAGGED(vict, MOB_NOBASH)) {
            act("�� ������ ����� $N �������$U �� �����.", FALSE, ch, 0, vict, TO_CHAR);
            act("�� ����� $n1 �� �������� ���������� � �����.", FALSE, ch, 0, vict, TO_VICT);
            act("�� ����� $n1 $N �������$G ���������� � ����$G.", FALSE, ch, 0, vict, TO_NOTVICT);
            set_wait(vict, 1, TRUE);
            GET_POS(vict) = POS_SITTING;
        }
    }
}

ACMD(do_runup)
{
    struct char_data *vict;
    char arg[MAX_STRING_LENGTH];

    if (!GET_SKILL(ch, SKILL_RUNUP)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (!check_ground(ch)) {
        send_to_charf(ch, "���� ���� ������ ������ �� ������� �����.\r\n");
        return;
    }
    one_argument(argument, arg);

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_char("���� �� ������ ������� � �������?\r\n", ch);
        return;
    }

    if (vict == ch) {
        send_to_char("������ ���, ��� ����������!\r\n", ch);
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (FIGHTING(ch)) {
        send_to_char("�� ������ ����!\r\n", ch);
        return;
    }

    if (check_pkill(ch, vict, arg))
        return;

    go_runup(ch, vict);
}


ACMD(do_backstab)
{
    struct char_data *vict;
    char arg[MAX_STRING_LENGTH];

    if (!GET_SKILL(ch, SKILL_BACKSTAB)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (onhorse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_char("���� �� ������ �������� ?\r\n", ch);
        return;
    }

    if (vict == ch) {
        send_to_char("��! ��� ������!\r\n", ch);
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (!GET_EQ(ch, WEAR_WIELD)) {
        send_to_char("��������� ������� ������ � ������ ����.\r\n", ch);
        return;
    }

    if (GET_OBJ_SKILL(GET_EQ(ch, WEAR_WIELD)) != SKILL_DAGGERS) {
        send_to_char("�������� ����� ������ ������� ������ '�������'.\r\n", ch);
        return;
    }

    if (AFF_FLAGGED(ch, AFF_STOPRIGHT) || AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }

    if (FIGHTING(vict)) {
        send_to_char("���� ���� ������� ������ �������������!\r\n", ch);
        return;
    }

    if (FIGHTING(ch)) {
        send_to_char("�� �� ������ �������� �� ���!\r\n", ch);
        return;
    }

    if (check_pkill(ch, vict, arg))
        return;

    go_backstab(ch, vict);
}


/******************* CHARM ORDERS PROCEDURES */
ACMD(do_order)
{
    char name[MAX_INPUT_LENGTH], message[MAX_INPUT_LENGTH];
    bool found = FALSE;
    room_rnum org_room;
    struct follow_type *k, *k_next;

    half_chop(argument, name, message);

    if (AFF_FLAGGED(ch, AFF_SIELENCE)) {
        send_to_char("�� �� � ��������� ����������� ������.\r\n", ch);
        return;
    }

    if (!*name || !*message) {
        send_to_char("��������� ���?\r\n", ch);
        return;
    }

    if (is_abbrev(name, "����")) {      /* This is order "followers" */
        act("$n �����$g ������.", FALSE, ch, 0, 0, TO_ROOM | CHECK_DEAF);
        send_to_charf(ch, "�� ���������: '%s'.\r\n", message);
        org_room = ch->in_room;
        for (k = ch->followers; k; k = k_next) {
            k_next = k->next;
            if (k->type != FLW_CHARM && k->type != FLW_UNDEAD)
                continue;
            if (org_room == k->follower->in_room)
                if (AFF_FLAGGED(k->follower, AFF_CHARM) && !AFF_FLAGGED(k->follower, AFF_DEAFNESS)) {
                    found = TRUE;
                    if (GET_WAIT_STATE(k->follower) <= 0 && GET_POS(k->follower) >= POS_SLEEPING)
                        command_interpreter(k->follower, message);
                    else {
                        act("$N ��������������$G ��� ������.", FALSE, ch, 0, k->follower, TO_CHAR);
                        act("$N ��������������$G ������ $n1.", FALSE, ch, 0, k->follower,
                            TO_NOTVICT);
                        act("�� ��������������$G ������ $n1.", FALSE, ch, 0, k->follower, TO_VICT);
                    }
                }
        }
    } else {
        org_room = ch->in_room;
        for (k = ch->followers; k; k = k_next) {
            k_next = k->next;
            if (k->type != FLW_CHARM && k->type != FLW_UNDEAD)
                continue;
            if (isname(name, k->follower->player.name)
                || isfullname(name, k->follower->player.names)) {
                act("$n �����$g ������.", FALSE, ch, 0, k->follower, TO_ROOM | CHECK_DEAF);
                act("�� ��������� 2�: '%1'.", "���", ch, k->follower, message);
                if (org_room == k->follower->in_room && AFF_FLAGGED(k->follower, AFF_CHARM)
                    && !AFF_FLAGGED(k->follower, AFF_DEAFNESS)) {
                    found = TRUE;
                    if (GET_WAIT_STATE(k->follower) <= 0 && GET_POS(k->follower) >= POS_SLEEPING)
                        command_interpreter(k->follower, message);
                    else {
                        act("$N ��������������$G ��� ������.", FALSE, ch, 0, k->follower, TO_CHAR);
                        act("$N ��������������$G ������ $n1.", FALSE, ch, 0, k->follower,
                            TO_NOTVICT);
                        act("�� ��������������$G ������ $n1.", FALSE, ch, 0, k->follower, TO_VICT);
                    }
                }
                return;
            }
        }
    }

    if (!found)
        send_to_char("����� ��� �� ��� ����������������?\r\n", ch);

}


/********************** FLEE PROCEDURE */
void go_flee(struct char_data *ch)
{
    int i, attempt, res;
    struct char_data *was_fighting;

    if (GET_POS(ch) < POS_FIGHTING) {
        send_to_char("�� �� ������ ������� �� ����� ���������.\r\n", ch);
        return;
    }

    if (!MAY_MOVE(ch))
        return;

    if (affected_by_spell(ch, SPELL_GRASP)) {
        send_to_char("� ��������� ���� �� �� ������� �������.\r\n", ch);
        return;
    }
    if (affected_by_spell(ch, SPELL_COURAGE)) {
        send_to_charf(ch, "�� ������� �������� ����, ����� �������.\r\n");
        return;
    }


    for (i = 0; i < 6; i++) {
        attempt = number(0, NUM_OF_DIRS - 1);   /* Select a random direction */
        if (CAN_GO(ch, attempt) && !ROOM_FLAGGED(EXIT(ch, attempt)->to_room, ROOM_DEATH)) {

            if (SECT(EXIT(ch, attempt)->to_room) == SECT_FLYING && GET_POS(ch) != POS_FLYING)
                continue;

            was_fighting = FIGHTING(ch);
            if (was_fighting && !IS_NPC(ch) && IS_NPC(was_fighting))
                add_flee(was_fighting, ch);


            res = do_simple_move(ch, attempt | 0x80, TRUE, TRUE);
            if (res) {
                send_to_char("\r\n�� ������ ������� �� ���.\r\n", ch);
                return;
            }
        }
    }

    act("$n �����������$g � �������$u �������, �� �� ����$q!", TRUE, ch, 0, 0, TO_ROOM);
    act("�� ������������ � �� ������ �������!", FALSE, ch, 0, 0, TO_CHAR);
    WAIT_STATE(ch, 10);         //1 �������
}

const char *FleeDirs[] = { "�����",
    "������",
    "��",
    "�����",
    "�����",
    "����",
    "\n"
};

void go_dir_flee(struct char_data *ch, int direction)
{
    int percent = 0, prob = 0;
    struct char_data *vict = NULL;

    if (GET_MOB_HOLD(ch))
        return;

    if (affected_by_spell(ch, SPELL_GRASP)) {
        send_to_char("� ��������� ���� �� �� ������� �������.\r\n", ch);
        return;
    }

    if (GET_WAIT(ch) > 0)
        return;

    if (!FIGHTING(ch)) {
        send_to_charf(ch, "� ��� ������ ���� ���������!\r\n");
        return;
    }

    vict = FIGHTING(ch);

    if (GET_POS(ch) < POS_FIGHTING) {
        send_to_char("�� �� ������� ��������� ������ �� ����� ���������.\r\n", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_COURAGE)) {
        send_to_charf(ch, "�� ������� �������� ����, ����� ��������� ������.\r\n");
        return;
    }

    if (!(IS_GOD(ch) || (!IS_NPC(ch) && GET_GOD_FLAG(ch, GF_GODSLIKE))))
        WAIT_STATE(ch, 1 * PULSE_VIOLENCE);

    if (!legal_dir(ch, direction, TRUE, FALSE)) {
        send_to_charf(ch, "� ���� ����������� ���������� ��������� ������.\r\n");
        return;
    }

    act("$n �������$u ��������� �������� ������.", FALSE, ch, 0, 0, TO_ROOM);

    prob =
        GET_REAL_DEX(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) + GET_SKILL(ch,
                                                                           SKILL_DIRFLEE) +
        RNDSKILL;
    percent =
        GET_REAL_DEX(vict) + GET_REAL_INT(vict) + GET_REAL_WIS(vict) + GET_SAVE3(vict,
                                                                                 SAV_REFL) +
        saving_throws_3(vict, SAV_REFL) + RNDSKILL;

    improove_skill(ch, vict, 0, SKILL_DIRFLEE);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (prob >= percent) {
        if (do_simple_move(ch, direction | 0x80, TRUE, TRUE))
            send_to_char("�� ��������� �������� ������.\r\n", ch);
    } else
        send_to_char("�� �� ������ ��������� �������� ������.\r\n", ch);

    WAIT_STATE(ch, 10);
}


ACMD(do_dir_flee)
{
    char arg[MAX_STRING_LENGTH];
    int direction = -1;

    if (!FIGHTING(ch)) {
        send_to_char("�� �� ���� �� � ��� �� ����������!\r\n", ch);
        return;
    }

    if (!GET_SKILL(ch, SKILL_DIRFLEE)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    one_argument(argument, arg);
    if ((direction = search_block(arg, dirs, FALSE)) >= 0 ||
        (direction = search_block(arg, FleeDirs, FALSE)) >= 0) {
        go_dir_flee(ch, direction);
        return;
    } else {
        send_to_char("�������� ����������� �������.\r\n", ch);
    }
}

ACMD(do_flee)
{
    if (!FIGHTING(ch)) {
        send_to_char("�� �� ���� �� � ��� �� ����������!\r\n", ch);
        return;
    }

    if (AFF_FLAGGED(ch, AFF_HOLD)) {
        send_to_char("���� ���� ������������, �� �� ������ ������ �������.\r\n", ch);
        return;
    }

    go_flee(ch);
}

/************************** BASH PROCEDURES */
void go_bash(struct char_data *ch, struct char_data *vict)
{
    int prob, percent, ch_wait, vict_wait, retval;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_BASH) : GET_SKILL(ch, SKILL_BASH);
    int save = GET_SAVE3(ch, SAV_REFL) + saving_throws_3(ch, SAV_REFL);
    struct P_damage damage;
    struct P_message message;
    struct obj_data *weapObj = NULL;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || AFF_FLAGGED(ch, AFF_STOPLEFT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }

    if (onhorse(ch))
        return;

    if (!check_ground(ch))
        return;

    if (!(IS_NPC(ch) ||         // ���
          GET_EQ(ch, WEAR_SHIELD) ||    // ���� ���
          IS_IMMORTAL(ch))) {   // �����������
        send_to_char("�� �� ������ ����� ��� ����.\r\n", ch);
        return;
    };

    if (GET_POS(ch) < POS_FIGHTING) {
        send_to_char("��� ������� ������ �� ����.\r\n", ch);
        return;
    }

    if (!ok_damage_shopkeeper(ch, vict)) {
        return;
    }

    if (check_spells_attacked(ch, vict, FALSE, FALSE))
        return;

    if (check_distance(ch, vict) == DIST_1) {
        send_to_charf(ch, "����� ���������� ����� ������ � ������� ���.\r\n");
        return;
    }

    if (GET_POS(vict) <= POS_SITTING)
        GetSkillMessage(SKILL_ABASH, 0, message);
    else
        GetSkillMessage(SKILL_BASH, 0, message);

    if (GET_EQ(ch, WEAR_SHIELD))
        weapObj = GET_EQ(ch, WEAR_SHIELD);
    else {
        weapObj = NULL;
        GetSkillMessage(SKILL_NBASH, 0, message);
    }

    /*percent = number(1,100);
       prob = (skill - (GET_REAL_WEIGHT(vict)/3) + (GET_REAL_WEIGHT(ch)/3) - GET_REAL_CON(vict) + GET_REAL_CON(ch));
       prob = (prob * GET_REAL_SIZE(ch)) / GET_REAL_SIZE(vict);
       if (!get_attack_hit(ch,vict,SKILL_BASH,0,skill))
       prob /=2;
       prob =  calc_like_skill(ch, vict, SKILL_BASH, prob); */
//������� ������ �� ����� �������
    prob =
        GET_REAL_STR(ch) + GET_REAL_STR(ch) + GET_REAL_DEX(ch) + (GET_REAL_WEIGHT(ch) / 3) + skill +
        RNDSKILL;
    percent =
        GET_REAL_STR(vict) + GET_REAL_DEX(vict) + GET_REAL_DEX(vict) + (GET_REAL_WEIGHT(ch) / 3) +
        GET_SAVE3(vict, SAV_FORT) + saving_throws_3(vict, SAV_FORT) + RNDSKILL;



    improove_skill(ch, vict, 0, SKILL_BASH);

    damage.valid = true;
    damage.type = HIT_BLOW;
    damage.power = A_POWER;
    damage.far_min = FALSE;
    damage.armor = TRUE;
    damage.weapObj = weapObj ? weapObj : NULL;
    damage.dam = MAX(1, (GET_REAL_DR(ch) * skill) / 200);

//��������� � ���� ��� ���� 1% �� ������ 3.5�� ����
    if (weapObj) {
        prob += GET_OBJ_WEIGHT(weapObj) / 3500;
        switch (GET_ARM_TYPE(weapObj)) {
            case TARM_MEDIUM:
                percent += 15;
                break;
            case TARM_LIGHT:
                percent += 5;
                break;
            case TARM_WEAR:
            case TARM_JEWS:
                percent -= 5;
                break;
            case TARM_HARD:
                percent -= 15;
                break;
        }
    }
    if (IS_BARIAUR(vict) && GET_POS(vict) != POS_FLYING)
        prob = MAX(1, prob / 4);

    if ((IS_AFFECTED(vict, AFF_FLY) && GET_POS(vict) == POS_FLYING))
        prob = MAX(1, (prob * 100) / 35);

    if (GET_POS(vict) < POS_FIGHTING)
        prob /= 4;

    /*  if (percent <= 5)
       prob = 100;
       else if (percent >= 95)
       prob = 0;             */

    if (MOB_FLAGGED(vict, MOB_NOBASH))
        prob = 0;

    if (PRF_FLAGGED(vict, PRF_NOHASSLE))
        prob = 0;

//prob =  calc_like_skill(ch, vict, SKILL_BASH, prob);


    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (PRF_FLAGGED(vict, PRF_CODERINFO))
        send_to_charf(vict, "&K���������� ������ ������ ��� %d >= %d\r\n&n", prob, percent);

    if (percent > prob) {       //������
        damage.check_ac = N_POWER;      //������
        damage.dam = 0;
        _damage(ch, vict, 0, 0, A_POWER, TRUE, damage, message);
        GET_POS(ch) = POS_SITTING;
        ch_wait = 2;
        vict_wait = 0;
    } else {                    //������
        damage.check_ac = A_POWER;      //������
        ch_wait = 1;
        vict_wait = 3;

        retval = _damage(ch, vict, 0, 0, A_POWER, TRUE, damage, message);

        if (retval == RD_ARMOR || retval == RD_MAGIC)
            return;

        if (retval == RD_DEVIATE) {
            act("�� �� �������� ���������� � �����.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n �� ������� ����������, ����$g.", FALSE, ch, 0, vict, TO_ROOM);
            GET_POS(ch) = POS_SITTING;
            return;
        }

        GET_POS(vict) = POS_SITTING;
        if (get_horse_on(vict)) {
            act("�� ����� � $N1.", FALSE, vict, 0, get_horse(vict), TO_CHAR);
            act("$n ����$g � $N1.", TRUE, vict, 0, get_horse(vict), TO_ROOM);
            REMOVE_BIT(AFF_FLAGS(vict, AFF_HORSE), AFF_HORSE);
        }
        if (IS_HORSE(vict) && on_horse(vict->master))
            horse_drop(vict);

        int ll = dice(1, skill) + GET_REAL_LCK(ch) + GET_REAL_DEX(ch);

        if (save > ll && !IS_BARIAUR(ch)) {
            act("�� �� �������� ���������� � ����� ����� � $N4.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n �� ������� ����������, ����$g ����� � ����.", FALSE, ch, 0, vict, TO_VICT);
            act("$n �� ������� ����������, ����$g ����� � $N4.", FALSE, ch, 0, vict, TO_NOTVICT);
            GET_POS(ch) = POS_SITTING;
        }
    }

    if (ch_wait)
        GET_WAIT(ch) = (PULSE_VIOLENCE * ch_wait);
    if (vict_wait)
        GET_WAIT(vict) = (PULSE_VIOLENCE * vict_wait);
}


ACMD(do_bash)
{
    struct char_data *vict = NULL;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_BASH)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (onhorse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
            vict = FIGHTING(ch);
        else {
            send_to_char("���� �� ������ �����?\r\n", ch);
            return;
        }
    }

    if (vict == ch) {
        send_to_char("�� ������ ���� �����. ��� �����?\r\n", ch);
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (check_pkill(ch, vict, arg))
        return;

    if (!used_attack(ch))
        go_bash(ch, vict);

    /* if (!FIGHTING(ch))
       go_bash(ch, vict);
       else if (!used_attack(ch))
       SET_EXTRA(ch,SKILL_BASH,vict); */
}


/******************** RESCUE PROCEDURES */
int go_rescue(struct char_data *ch, struct char_data *vict, struct char_data *tmp_ch)
{
    int percent, prob;
    int wait_ch, wait_victim;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_RESCUE) : GET_SKILL(ch, SKILL_RESCUE);
    struct char_data *tch, *first;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return (FALSE);
    }

    if (SECT(IN_ROOM(ch) == SECT_UNDERWATER)) {
        send_to_charf(ch, "�������� ��� ����� ����������.\r\n");
        return (FALSE);
    }
    //��������� ��� ����� tmp_ch
    prob = GET_REAL_STR(ch) + GET_REAL_STR(ch) + GET_REAL_DEX(ch) + skill + RNDSKILL;
    percent = GET_REAL_WEIGHT(vict) + RNDSKILL;

    improove_skill(ch, tmp_ch, 0, SKILL_RESCUE);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (percent > prob) {
        act("�� ���������� �������� ������ $N3.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n ���������� �����$u ������ ���.", FALSE, ch, 0, vict, TO_VICT);
        act("$n ���������� �����$u ������ $N3.", FALSE, ch, 0, vict, TO_NOTVICT);
        wait_ch = 1;
        WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);
        return (FALSE);
    }

    act("������, �� ����� ���������� $N3 � �������.", FALSE, ch, 0, vict, TO_CHAR);
    act("������, $n ����� ���������$g ��� � �������.", FALSE, ch, 0, vict, TO_VICT);
    act("������, $n ����� ���������$g $N3 � �������.", TRUE, ch, 0, vict, TO_NOTVICT);
    wait_victim = 1;
    wait_ch = 1;
    //������ ������
    if (10 > (dice(1, GET_REAL_DEX(vict)) + GET_REAL_LCK(vict)) && GET_POS(vict) > POS_SITTING) {
        if (IS_HORSE(vict) && on_horse(vict->master))
            horse_drop(vict);

        act("�� ������� ����������, $N ����$G!", FALSE, ch, 0, vict, TO_CHAR);
        act("�� �� �������� ���������� � �����!", FALSE, ch, 0, vict, TO_VICT);
        act("�� ������� ����������, $N ����$G!", FALSE, ch, 0, vict, TO_NOTVICT);
        GET_POS(vict) = POS_SITTING;
        WAIT_STATE(vict, PULSE_VIOLENCE * 1);
    } else if (on_horse(vict) && get_horse_on(vict)) {
        REMOVE_BIT(AFF_FLAGS(vict, AFF_HORSE), AFF_HORSE);
        REMOVE_BIT(AFF_FLAGS(get_horse_on(vict), AFF_HORSE), AFF_HORSE);
    }

    first = tmp_ch;
    for (tch = world[IN_ROOM(vict)].people; tch; tch = tch->next_in_room) {
        if (tch == first)
            continue;
        if (FIGHTING(tch) == vict)
            tmp_ch = tch;

        if (FIGHTING(vict) == tmp_ch)
            stop_fighting(vict, FALSE);

        if (!IS_NPC(ch))
            if (may_pkill(ch, tmp_ch) != PC_REVENGE_PC)
                inc_pkill_group(tmp_ch, ch, 1, 0);

        if (FIGHTING(ch))
            FIGHTING(ch) = tmp_ch;
        else
            set_fighting(ch, tmp_ch);
        if (FIGHTING(tmp_ch))
            FIGHTING(tmp_ch) = ch;
        else
            set_fighting(tmp_ch, ch);
    }
    if (FIGHTING(ch))
        FIGHTING(ch) = first;
    else
        set_fighting(ch, first);
    if (FIGHTING(first))
        FIGHTING(first) = ch;
    else
        set_fighting(first, ch);

    WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);
    WAIT_STATE(vict, PULSE_VIOLENCE * wait_victim);

    return (TRUE);
}

ACMD(do_rescue)
{
    struct char_data *vict, *tmp_ch;
    char arg[MAX_STRING_LENGTH];

    if (!GET_SKILL(ch, SKILL_RESCUE)) {
        send_to_char("�� �� �� ������ ���.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_char("���� �� ������ ������?\r\n", ch);
        return;
    }

    if (vict == ch) {
        send_to_char("���� �������� �� ������ �������� ������ �����.\r\n", ch);
        return;
    }
    if (FIGHTING(ch) == vict) {
        send_to_char("������ ������ �����? �� ������ ���� ����� ���.\r\n", ch);
        return;
    }

    for (tmp_ch = world[ch->in_room].people;
         tmp_ch && (FIGHTING(tmp_ch) != vict); tmp_ch = tmp_ch->next_in_room);

    if (!tmp_ch) {
        act("�� ����� �� ��������� � $N4!", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }

    if (!may_kill_here(ch, tmp_ch))
        return;

    go_rescue(ch, vict, tmp_ch);
}

/*******************  KICK PROCEDURES */
void go_kick(struct char_data *ch, struct char_data *vict)
{
    int percent, prob, wait = 0, weap_i, dam = 0;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_KICK) : GET_SKILL(ch, SKILL_KICK);
    struct P_damage damage;
    struct P_message message;
    struct obj_data *weapObj = NULL;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }

    if (GET_POS(ch) < POS_FIGHTING) {
        send_to_charf(ch, "�� ������ ������, ��� �� ����� ����-������.\r\n");
        return;
    }

    if (onhorse(ch))
        return;

    if (!ok_damage_shopkeeper(ch, vict))
        return;

    if (check_spells_attacked(ch, vict, FALSE, FALSE))
        return;

    if (check_distance(ch, vict) == DIST_1) {
        send_to_charf(ch, "����� ���������� ����� ������ � ������� ���.\r\n");
        return;
    }

    prob = GET_REAL_STR(ch) + GET_REAL_STR(ch) + GET_REAL_DEX(ch) + skill + RNDSKILL;
    percent =
        (GET_REAL_DEX(vict) * 3) +
        ((GET_SAVE3(vict, SAV_REFL) + saving_throws_3(vict, SAV_REFL)) * 3) + RNDSKILL;

    improove_skill(ch, vict, 0, SKILL_KICK);

    GetSkillMessage(SKILL_KICK, 0, message);

    if (GET_EQ(ch, WEAR_FEET))
        weapObj = GET_EQ(ch, WEAR_FEET);
    else if (IS_BARIAUR(ch) && ((weap_i = real_object(OBJ_FOOT_BAR)) >= 0))
        weapObj = (obj_proto + weap_i);
    else if ((weap_i = real_object(OBJ_FOOT)) >= 0)
        weapObj = (obj_proto + weap_i);


    dam = GET_REAL_DR(ch);
    if (weapObj)
        dam += GET_OBJ_WEIGHT(weapObj) / 500;

    damage.valid = true;
    damage.type = HIT_BLOW;
    damage.power = GET_POWER(ch);
    damage.far_min = FALSE;
    damage.armor = FALSE;
    damage.weapObj = weapObj ? weapObj : NULL;
    damage.dam = dam;


    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (percent > prob) {
        damage.check_ac = N_POWER;      //������
        wait = 2;
    } else {
        damage.check_ac = A_POWER;      //������
        wait = 1;
    }

    int krlt;

    krlt = _damage(ch, vict, 0, 0, C_POWER, TRUE, damage, message);

    if (krlt == RD_NONE) {
        if (!IS_BARIAUR(ch) && !general_savingthrow_3(ch, SAV_REFL, MAX(1, 20 - skill)))
            if (!(AFF_FLAGGED(ch, AFF_FLY) && GET_POS(ch) == POS_FLYING) &&
                !MOB_FLAGGED(ch, MOB_FLYING)) {
                act("�� �������� ���������� � �����.", FALSE, ch, 0, 0, TO_CHAR);
                act("$n �������$g ���������� � ����$g.", TRUE, ch, 0, 0, TO_ROOM);
                GET_POS(ch) = POS_SITTING;
            }
    } else if (krlt == RD_NORMAL && vict) {
        if (!IS_BARIAUR(vict) && skill > number(50, GET_REAL_SIZE(vict)))
            if (!(AFF_FLAGGED(vict, AFF_FLY) && GET_POS(vict) == POS_FLYING) &&
                !MOB_FLAGGED(vict, MOB_FLYING)) {
                act("�� ������ ����� $N �������$G ���������� � ����$G.", FALSE, ch, 0, vict,
                    TO_CHAR);
                act("�� ����� $n1 �� �������� ���������� � �����.", FALSE, ch, 0, vict, TO_VICT);
                act("�� ����� $n1 $N �������$G ���������� � ����$G.", FALSE, ch, 0, vict,
                    TO_NOTVICT);
                GET_POS(vict) = POS_SITTING;
                WAIT_STATE(vict, PULSE_VIOLENCE);
            }
    }

    if (wait)
        WAIT_STATE(ch, PULSE_VIOLENCE * wait);
}

ACMD(do_kick)
{
    struct char_data *vict = NULL;
    char arg[MAX_STRING_LENGTH];

    if (!GET_SKILL(ch, SKILL_KICK)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (onhorse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }

    one_argument(argument, arg);
    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
            vict = FIGHTING(ch);
        else {
            send_to_char("���� ������?\r\n", ch);
            return;
        }
    }
    if (vict == ch) {
        send_to_char("�� ����� ����...\r\n", ch);
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (check_pkill(ch, vict, arg))
        return;

    if (!used_attack(ch))
        go_kick(ch, vict);

    /* if (!FIGHTING(ch))
       go_kick(ch, vict);
       else
       if (!used_attack(ch)) SET_EXTRA(ch, SKILL_KICK, vict); */

}

/******************** BLOCK PROCEDURES */
void go_block(struct char_data *ch)
{
    struct obj_data *shield = GET_EQ(ch, WEAR_SHIELD);

    if (AFF_FLAGGED(ch, AFF_STOPLEFT)) {
        send_to_char("���� ���� ������������.\r\n", ch);
        return;
    }

    if (!shield && !IS_NPC(ch)) {
        act("��� ����� ���������� �� ����� ����������.", "�", ch);
        CLR_AF_BATTLE(ch, EAF_BLOCK);
        return;
    }


    act("�� ����������� ����������� ����� ����������.", FALSE, ch, 0, 0, TO_CHAR);
    SET_AF_BATTLE(ch, EAF_BLOCK);
}

ACMD(do_block)
{
    if (!GET_SKILL(ch, SKILL_BLOCK)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }
    if (!FIGHTING(ch)) {
        send_to_char("�� �� � ��� �� ����������.\r\n", ch);
        return;
    };
    if (!(IS_NPC(ch) ||         // ���
          GET_EQ(ch, WEAR_SHIELD) ||    // ���� ���
          IS_GOD(ch) ||         // �����������
          GET_GOD_FLAG(ch, GF_GODSLIKE) // ��������
        )) {
        send_to_char("�� �� ������ ������� ��� ��� ����.\r\n", ch);
        return;
    }

    if (GET_AF_BATTLE(ch, EAF_BLOCK)) {
        send_to_charf(ch, "�� ��� ���������� ����� ����������.\r\n");
        return;
    }
    go_block(ch);

}

/***************** MULTYPARRY PROCEDURES */
void go_multyparry(struct char_data *ch)
{
    if (AFF_FLAGGED(ch, AFF_STOPRIGHT) ||
        AFF_FLAGGED(ch, AFF_STOPLEFT) || AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }

    send_to_char("�� ������ �������� ����� ����������.\r\n", ch);
    SET_AF_BATTLE(ch, EAF_MULTYPARRY);
}

ACMD(do_multyparry)
{
    struct obj_data *dubl = GET_EQ(ch, WEAR_BOTHS);

    if (!GET_SKILL(ch, SKILL_MULTYPARRY)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }
    if (!FIGHTING(ch)) {
        send_to_char("�� �� �� � ��� �� ���������� ?\r\n", ch);
        return;
    }
    if (!(IS_NPC(ch) ||         // ���
          (dubl && GET_OBJ_TYPE(dubl) == ITEM_WEAPON) || IS_GOD(ch) ||  // �����������
          GET_GOD_FLAG(ch, GF_GODSLIKE) // ��������
        )) {
        send_to_char("�� ������ �������� ����� ������ ��������� �������.\r\n", ch);
        return;
    }
    go_multyparry(ch);
}




/***************** PARRY PROCEDURES */
void go_parry(struct char_data *ch)
{
    if (AFF_FLAGGED(ch, AFF_STOPRIGHT) ||
        AFF_FLAGGED(ch, AFF_STOPLEFT) || AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }

    act("�� ������ ���������� ����� ����������.", FALSE, ch, 0, 0, TO_CHAR);
    SET_AF_BATTLE(ch, EAF_PARRY);

}

ACMD(do_parry)
{
    struct obj_data *primary = GET_EQ(ch, WEAR_WIELD);
    struct obj_data *dubl = GET_EQ(ch, WEAR_BOTHS);

    if (!GET_SKILL(ch, SKILL_PARRY)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }
    if (!FIGHTING(ch)) {
        send_to_char("�� �� �� � ��� �� ���������� ?\r\n", ch);
        return;
    }
    if (!(IS_NPC(ch) ||         // ���
          (primary && GET_OBJ_TYPE(primary) == ITEM_WEAPON) ||  // ������
          (dubl && GET_OBJ_TYPE(dubl) == ITEM_WEAPON)   // ������
        )) {
        send_to_char("�� �� ������ ��������� ����� ����������.\r\n", ch);
        return;
    }

    if (!IS_NPC(ch) &&
        ((primary
          && (GET_OBJ_SKILL(primary) == SKILL_BOWS || GET_OBJ_SKILL(primary) == SKILL_CROSSBOWS))
         || (dubl
             && (GET_OBJ_SKILL(dubl) == SKILL_BOWS || GET_OBJ_SKILL(dubl) == SKILL_CROSSBOWS)))) {
        send_to_charf(ch, "���������� ������� ���������� �� ���������.\r\n");
        return;
    }

    go_parry(ch);
}


/************** TOUCH PROCEDURES */
void go_touch(struct char_data *ch, struct char_data *victim)
{
    struct timed_type timed;
    int prob, percent, wait_ch = 0, wait_victim = 0;

    if (!FIGHTING(victim)) {
        act("$N, ����, �� ���!", FALSE, ch, 0, victim, TO_CHAR);
        act("$n ����������$a ���, ����� ����-�� �����?", FALSE, ch, 0, victim, TO_VICT);
        act("�������� $n �������$a $N3!", FALSE, ch, 0, victim, TO_NOTVICT);
        return;
    }

    if (!GET_SKILL(ch, SKILL_TOUCH)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (timed_by_skill(ch, SKILL_TOUCH)) {
        send_to_char("� ��� �� ������ ���.\r\n", ch);
        return;
    }

    if (!ok_damage_shopkeeper(ch, victim))
        return;

    if (check_distance(ch, victim) == DIST_1) {
        send_to_charf(ch, "�������� ���������� ����� ������ � ������� ���.\r\n");
        return;
    }

    prob =
        GET_REAL_STR(ch) + GET_REAL_STR(ch) + GET_REAL_DEX(ch) + GET_REAL_WEIGHT(ch) + GET_SKILL(ch,
                                                                                                 SKILL_TOUCH)
        + RNDSKILL;
    percent =
        GET_REAL_STR(victim) + GET_REAL_DEX(victim) + GET_REAL_DEX(victim) + GET_REAL_WEIGHT(ch) +
        ((GET_SAVE3(victim, SAV_REFL) + saving_throws_3(victim, SAV_REFL)) * 3) + RNDSKILL;

    improove_skill(ch, victim, 0, SKILL_TOUCH);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (prob >= percent) {
        act("�� ���������� $N3 �� ���.", FALSE, ch, 0, victim, TO_CHAR);
        act("$n ���������$g ��� �� ���.", FALSE, ch, 0, victim, TO_VICT);
        act("$n ���������$g $N3 �� ���.", FALSE, ch, 0, victim, TO_NOTVICT);
        stop_fighting(victim, TRUE);

        //������ ������
        if (10 > (dice(1, GET_REAL_DEX(victim)) + GET_REAL_LCK(victim))
            && GET_POS(victim) > POS_SITTING) {
            act("$N �� ��������$G ���������� � ����$G!", FALSE, ch, 0, victim, TO_CHAR);
            act("�� �� ��������� ���������� � �����!", FALSE, ch, 0, victim, TO_VICT);
            act("$N �� ��������$G ���������� � ����$G!", FALSE, ch, 0, victim, TO_NOTVICT);
            GET_POS(victim) = POS_SITTING;
        }

        wait_ch = 1;
        wait_victim = 2;
        timed.skill = SKILL_TOUCH;
        timed.time  = SECS_PER_MUD_TICK / 2;
        timed_to_char(ch, &timed);
    } else {
        act("�� �� ������ ���������� $N3 �� ���.", FALSE, ch, 0, victim, TO_CHAR);
        act("$n �����$g �������� ���, �� ���������$u.", FALSE, ch, 0, victim, TO_VICT);
        act("$n �����$g �������� $N3, �� ���������$u.", FALSE, ch, 0, victim, TO_NOTVICT);
        wait_ch = 2;
        timed.skill = SKILL_TOUCH;
        timed.time  = SECS_PER_MUD_TICK;
        timed_to_char(ch, &timed);
    }

    WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);
    WAIT_STATE(victim, PULSE_VIOLENCE * wait_victim);

}

ACMD(do_touch)
{
    struct char_data *vict = NULL;
    char arg[MAX_STRING_LENGTH];

    if (onhorse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
            vict = FIGHTING(ch);
        else {
            act("����-� ���������, ��?", FALSE, ch, 0, 0, TO_CHAR);
            //$n �����$a ������� �������� �������.
            return;
        }
    }

    if (ch == vict) {
        act("�� �������� �������� ���� � ���.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

    if (check_pkill(ch, vict, arg))
        return;

    if (!used_attack(ch))
        go_touch(ch, vict);
}

/************** DEVIATE PROCEDURES */
void go_deviate(struct char_data *ch)
{
    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }
    if (onhorse(ch)) {
        send_to_char("��� ����� ���������� ���������.\r\n", ch);
        return;
    }

    send_to_charf(ch, "�� ����������� ��������� �� ����� ����������.\r\n");
    SET_AF_BATTLE(ch, EAF_DEVIATE);
}

ACMD(do_deviate)
{
    if (!GET_SKILL(ch, SKILL_DEVIATE)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (!(FIGHTING(ch))) {
        send_to_char("�� �� ���� �� � ��� �� ����������!\r\n", ch);
        return;
    }

    if (onhorse(ch)) {
        send_to_char("��� ����� ���������� ���������.\r\n", ch);
        return;
    }

    if (GET_AF_BATTLE(ch, EAF_DEVIATE)) {
        send_to_charf(ch, "�� �� ��� ���������� �� �����.\r\n");
        return;
    }

    go_deviate(ch);
}

/************** DISARM PROCEDURES */
void go_disarm(struct char_data *ch, struct char_data *vict)
{
    int percent, prob, pos = 0;
    int wait_ch = 0, wait_victim = 0;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_DISARM) : GET_SKILL(ch, SKILL_DISARM);
    struct obj_data *weapon = GET_EQ(vict, WEAR_WIELD) ? GET_EQ(vict, WEAR_WIELD) :
        (GET_EQ(vict, WEAR_HOLD) ? GET_EQ(vict, WEAR_HOLD) : (GET_EQ(vict, WEAR_BOTHS)));

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }

    if (check_spells_attacked(ch, vict, FALSE, FALSE))
        return;

    if (check_distance(ch, vict) == DIST_1) {
        send_to_charf(ch, "����������� ���������� ����� ������ � ������� ���.\r\n");
        return;
    }

    if (weapon == NULL || (weapon && GET_OBJ_TYPE(weapon) != ITEM_WEAPON)) {
        act("$N �� ��������$G.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }

    pos = weapon->worn_on;

    prob = GET_REAL_STR(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) + skill + RNDSKILL;
    percent = GET_REAL_DEX(vict) + GET_REAL_DEX(vict) + GET_REAL_INT(vict) +
        ((GET_SAVE3(vict, SAV_REFL) + saving_throws_3(vict, SAV_REFL)) * 3) + RNDSKILL;

    if (IS_AFFECTED(vict, AFF_IMPLANT_WEAPON))
        prob = 0;
    else
        improove_skill(ch, vict, 0, SKILL_DISARM);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (percent > prob || OBJ_FLAGGED(GET_EQ(vict, pos), ITEM_NODISARM)
        || IS_AFFECTED(vict, AFF_IMPLANT_WEAPON)) {
        act("� ��� �� ���������� ����������� $N3.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n �����$g ����������� ���.", FALSE, ch, 0, vict, TO_VICT);
        act("$n �������$g ������ ������ �� ��� $N1.", FALSE, ch, 0, vict, TO_NOTVICT);
        wait_ch = 1;
        wait_victim = 0;
    } else {
        weapon = unequip_char(vict, pos);
        act("������ ������ �� ������ $o3 �� ��� $N1.", FALSE, ch, weapon, vict, TO_CHAR);
        act("$n �����$g $o3 �� ����� ���.", FALSE, ch, weapon, vict, TO_VICT);
        act("$n �����$g $o3 �� ��� $N1.", TRUE, ch, weapon, vict, TO_NOTVICT);
        wait_ch = 1;
        wait_victim = 1;
        check_sets(ch);
        affect_total(ch);
        obj_to_room(weapon, IN_ROOM(vict));
        obj_decay(weapon);
    }

    if (may_pkill(ch, vict) != PC_REVENGE_PC)
        inc_pkill_group(vict, ch, 1, 0);

    appear(ch);

    if (IS_NPC(vict) && dice(1, 20) > GET_REAL_LCK(ch) && GET_WAIT(ch) <= 0)
        set_hit(vict, ch);

    if (wait_ch)
        WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);

    if (wait_victim)
        WAIT_STATE(vict, PULSE_VIOLENCE * wait_victim);
}

ACMD(do_disarm)
{
    struct char_data *vict = NULL;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_DISARM)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
            vict = FIGHTING(ch);
        else {
            send_to_char("���� �������������?\r\n", ch);
            return;
        }
    };

    if (ch == vict) {
        send_to_char("�������������� �������� '����� <�������>'.\r\n", ch);
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (check_pkill(ch, vict, arg))
        return;

    go_disarm(ch, vict);
}

/************************** CHOPOFF PROCEDURES */
void go_chopoff(struct char_data *ch, struct char_data *vict)
{
    int percent, prob, attack = 0;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }

    if (on_horse(ch)) {
        send_to_char("��� ����� ����� ��������.\r\n", ch);
        return;
    }

    if (FIGHTING(ch)) {
        send_to_charf(ch, "�� ����� ���?\r\n");
        return;
    }

    if (FIGHTING(vict)) {
        send_to_charf(ch, "���� ���� ������� ������ �������������.\r\n");
        return;
    }

    if (!ok_damage_shopkeeper(ch, vict)) {
        return;
    }

    if (check_spells_attacked(ch, vict, FALSE, FALSE)) {
        act("���� ������� ������� 2+� ���������.", "��", ch, vict);
        act("1+� �������1(��,���,���,���) ������� ���, �� �� ����1(,��,��,��).", "��", ch, vict);
        act("1+� �������1(��,���,���,���) ������� ���, �� �� ����1(,��,��,��).", "���", ch, vict);
        return;
    }

    prob =
        GET_REAL_DEX(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) + GET_SKILL(ch,
                                                                           SKILL_CHOPOFF) +
        RNDSKILL;
    percent =
        GET_REAL_DEX(vict) + GET_REAL_DEX(vict) + GET_REAL_CON(vict) +
        ((GET_SAVE3(vict, SAV_REFL) + saving_throws_3(vict, SAV_REFL)) * 3) + RNDSKILL;

    improove_skill(ch, vict, 0, SKILL_CHOPOFF);

    if (GET_POS(vict) < POS_FIGHTING || MOB_FLAGGED(vict, MOB_NOBASH))
        prob /= 2;

    if (IS_BARIAUR(vict) || IS_BARIAUR(ch))
        prob -= (prob * 25) / 100;

    if ((AFF_FLAGGED(vict, AFF_FLY) && GET_POS(vict) == POS_FLYING) ||
        MOB_FLAGGED(vict, MOB_FLYING))
        prob = 0;

    if (!CAN_SEE(vict, ch))
        prob *= 2;


    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);
    if (GET_POS(vict) == POS_SLEEPING) {
        act("�� ���������� ������� $N3, �� ������ ��������� $S.", FALSE, ch, 0, vict, TO_CHAR);
        act("������ ��� �� �������������, ��� $n ������ ���.", FALSE, ch, 0, vict, TO_VICT);
        act("$n �������$u ������� $N3, �� ������ ��������$g $S.", FALSE, ch, 0, vict, TO_NOTVICT);
        GET_POS(vict) = POS_SITTING;
        GET_POS(ch) = POS_SITTING;
        prob = 1;
        attack = 1;
    } else {
        if (percent > prob || (GET_POS(vict) <= POS_SITTING)) {
            if ((AFF_FLAGGED(ch, AFF_FLY) && GET_POS(ch) == POS_FLYING) ||
                MOB_FLAGGED(ch, MOB_FLYING)) {
                act("�� ���������� ������� $N3, �� �������� �����������.", FALSE, ch, 0, vict,
                    TO_CHAR);
                act("$n �������$u ������� ���, �� �������$g �����������.", FALSE, ch, 0, vict,
                    TO_VICT);
                act("$n �������$u ������� $N3, �� �������$g �����������.", FALSE, ch, 0, vict,
                    TO_NOTVICT);
            } else {
                act("�� ���������� ������� $N3, �� ����� ����.", FALSE, ch, 0, vict, TO_CHAR);
                act("$n �������$u ������� ���, �� ����$g ���$g.", FALSE, ch, 0, vict, TO_VICT);
                act("$n �������$u ������� $N3, �� ����$g ���$g.", FALSE, ch, 0, vict, TO_NOTVICT);
                GET_POS(ch) = POS_SITTING;
            }
            prob = 2;
            attack = 1;
        } else {
            act("�� ����� �������� $N3, ������ $S �� �����.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n ����� ������$q ���, ������ �� �����.", FALSE, ch, 0, vict, TO_VICT);
            act("$n ����� ������$q $N3, ������ $S �� �����.", FALSE, ch, 0, vict, TO_NOTVICT);
            set_wait(vict, 3, FALSE);
            attack = 0;
            prob = 1;
            if (IN_ROOM(ch) == IN_ROOM(vict))
                GET_POS(vict) = POS_SITTING;
            if (IS_HORSE(vict) && on_horse(vict->master))
                horse_drop(vict);
        }
    }

    if (may_pkill(ch, vict) != PC_REVENGE_PC)
        inc_pkill_group(vict, ch, 1, 0);

    appear(ch);

    if (IS_NPC(vict) && attack && dice(1, 20) > GET_REAL_LCK(ch))
        set_hit(vict, ch);

    if (prob)
        set_wait(ch, prob, FALSE);

}


ACMD(do_chopoff)
{
    struct char_data *vict = NULL;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_CHOPOFF)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (on_horse(ch)) {
        send_to_char("��� ����� ����� ��������.\r\n", ch);
        return;
    }

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
            vict = FIGHTING(ch);
        else {
            send_to_char("���� �� ����������� ������� ?\r\n", ch);
            return;
        }
    }

    if (vict == ch) {
        act("���� ���� ��������� � �� �����.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n ������$g ������������� �� � ����$g.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (check_pkill(ch, vict, arg))
        return;

    if (!used_attack(ch))
        go_chopoff(ch, vict);
    /*  if (IS_IMPL(ch) ||!FIGHTING(ch))
       go_chopoff(ch, vict);
       else
       if (!used_attack(ch))
       {//act("�� ����������� ������� $N3.",FALSE,ch,0,vict,TO_CHAR);
       SET_EXTRA(ch, SKILL_CHOPOFF, vict);
       } */
}

/************************** STUPOR PROCEDURES */
void go_stupor(struct char_data *ch, struct char_data *victim)
{
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_STUPOR) : GET_SKILL(ch, SKILL_STUPOR);
    int percent, prob, wait_ch = 0, wait_victim = 0, weap_i;
    struct P_message message;
    struct P_damage damage;
    struct obj_data *weapObj = NULL;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        act("�� �������� �� � ��������� ���������.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

    if (!ok_damage_shopkeeper(ch, victim))
        return;

    if (check_spells_attacked(ch, victim, FALSE, FALSE))
        return;

    if (check_distance(ch, victim) == DIST_1) {
        send_to_charf(ch, "�������� ���������� ����� ������ � ������� ���.\r\n");
        return;
    }

    prob = GET_REAL_STR(ch) + GET_REAL_DEX(ch) + skill + RNDSKILL;
    percent = GET_REAL_CON(victim) + RNDSKILL +
        ((GET_SAVE3(victim, SAV_REFL) + saving_throws_3(victim, SAV_REFL)) * 9);

    improove_skill(ch, victim, 0, SKILL_STUPOR);


    if (GET_EQ(ch, WEAR_BOTHS)) {
        prob += GET_OBJ_WEIGHT(GET_EQ(ch, WEAR_BOTHS)) / 8000;
        weapObj = GET_EQ(ch, WEAR_BOTHS);
    } else if (GET_EQ(ch, WEAR_WIELD)) {
        prob += GET_OBJ_WEIGHT(GET_EQ(ch, WEAR_WIELD)) / 10000;
        weapObj = GET_EQ(ch, WEAR_WIELD);
    } else if ((weap_i = real_object(OBJ_HITR)) >= 0)
        weapObj = (obj_proto + weap_i);

    if (PRF_FLAGGED(victim, PRF_NOHASSLE))
        prob = 0;

    GetSkillMessage(SKILL_STUPOR, 0, message);

    damage.valid = true;
    damage.power = C_POWER;
    damage.far_min = FALSE;
    damage.weapObj = weapObj ? weapObj : NULL;
    damage.type = HIT_BLOW;
    damage.armor = TRUE;
    damage.dam = GET_DR(ch) + MAX(1, skill / 10);

//������ ������� ��������
    int i;

    i = MAX(1, skill - affected_by_spell(victim, SPELL_FREE_MOVES));
    if (general_savingthrow_3(victim, SAV_REFL, i))
        prob = 0;


    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (prob >= percent) {
        struct affected_type af;

        af.duration = 1;
        af.type = find_spell_num(SPELL_STUNE);
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_STOPFIGHT;
        af.battleflag = TRUE;
        af.owner = GET_ID(ch);
        af.main = TRUE;
        affect_join_char(victim, &af);
        wait_ch = 1;
        wait_victim = 1;
        damage.check_ac = A_POWER;
    } else {
        wait_ch = 2;
        wait_victim = 0;
        damage.check_ac = N_POWER;
    }

    if (FIGHTING(ch) == NULL)
        set_fighting(ch, victim);

    _damage(ch, victim, 0, 0, A_POWER, TRUE, damage, message);

    if (wait_ch)
        WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);

    if (wait_victim)
        WAIT_STATE(victim, PULSE_VIOLENCE * wait_victim);

    GET_MISSED(ch)++;
}

ACMD(do_stupor)
{
    struct char_data *vict = NULL;
    struct obj_data *obj = NULL;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_STUPOR)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
            vict = FIGHTING(ch);
        else {
            send_to_char("���� �� ������ ��������?\r\n", ch);
            return;
        }
    }

    if (vict == ch) {
        send_to_char("�� ������ ��������� ��������.\r\n", ch);
        return;
    }

    if (!(obj = GET_EQ(ch, WEAR_BOTHS)))
        obj = GET_EQ(ch, WEAR_WIELD);

    if (!obj && !IS_MOB(ch)) {
        send_to_charf(ch, "�������� ����� ����� ������ � ������ ��� � ����� �����.\r\n");
        return;
    } else if (obj && (GET_OBJ_SKILL(obj) == SKILL_BOWS || GET_OBJ_SKILL(obj) == SKILL_CROSSBOWS)) {
        send_to_charf(ch, "���� ������� �������� ������.\r\n");
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (check_pkill(ch, vict, arg))
        return;

    go_stupor(ch, vict);
}

/************************** MIGHTHIT PROCEDURES */
void go_mighthit(struct char_data *ch, struct char_data *victim)
{
    int prob, percent, dam = 0;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_MIGHTHIT) : GET_SKILL(ch, SKILL_MIGHTHIT);
    int wait_ch, wait_victim;
    struct P_damage damage;
    struct P_message message;
    struct timed_type timed;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }

    if (check_spells_attacked(ch, victim, FALSE, FALSE))
        return;

//!IS_GOD(ch) &&
    if (timed_by_skill(ch, SKILL_MIGHTHIT)) {
        send_to_char("� ��� �� ������ ���.\r\n", ch);
        return;
    }

    if (!IS_NPC(ch) && GET_EQ(ch, WEAR_BOTHS) && GET_EQ(ch, WEAR_WIELD) &&
        GET_EQ(ch, WEAR_HOLD) && GET_EQ(ch, WEAR_LIGHT)) {
        send_to_charf(ch, "���� ���� ����� ������� ������ ������ ������.\r\n");
        return;
    }

    if (check_distance(ch, victim) == DIST_1) {
        send_to_charf(ch, "������� ���������� ����� ������ � ������� ���.\r\n");
        return;
    }

    damage.valid = true;
    damage.type = real_attack_type(0);  //�������
    damage.power = GET_POWER(ch);
    damage.far_min = FALSE;
    damage.weapObj = NULL;
    damage.armor = TRUE;

    GetSkillMessage(SKILL_MIGHTHIT, 0, message);

    if ((IS_NPC(ch) || IS_GOD(ch)) ||
        !(GET_EQ(ch, WEAR_BOTHS) || GET_EQ(ch, WEAR_WIELD) ||
          GET_EQ(ch, WEAR_HOLD) || GET_EQ(ch, WEAR_LIGHT))) {
        prob = (GET_REAL_STR(ch) * 3) + skill + RNDSKILL;
        percent = (GET_REAL_DEX(victim) * 2) + GET_REAL_CON(victim) +
            ((GET_SAVE3(victim, SAV_REFL) + saving_throws_3(victim, SAV_REFL)) * 3) + RNDSKILL;

        improove_skill(ch, victim, 0, SKILL_MIGHTHIT);

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

        if (prob >= percent) {
            dam += GET_REAL_DR(ch);
            if (affected_by_spell(ch, SPELL_COURAGE))
                dam += ((GET_SKILL(ch, SKILL_COURAGE) + 19) / 20);

            if (IS_NPC(ch))
                dam +=
                    dice(ch->npc()->specials.damnodice,
                         ch->npc()->specials.damsizedice) + (ch->npc()->specials.damage);
            else
                dam += (GET_SKILL(ch, SKILL_PUNCH) + 9) / 10;
            damage.dam = dam * dice(1, MAX(1, skill / 20));
            damage.check_ac = A_POWER;
            wait_ch = 1;
            wait_victim = 2;
        } else {
            dam = 1;
            damage.check_ac = N_POWER;
            wait_ch = 2;
            wait_victim = 0;
        }

        _damage(ch, victim, 0, 0, C_POWER, TRUE, damage, message);

        if (wait_ch)
            WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);

        if (wait_victim)
            WAIT_STATE(victim, PULSE_VIOLENCE * wait_victim);

        timed.skill = SKILL_MIGHTHIT;
        timed.time = 2 * SECS_PER_MUD_TICK;
        timed_to_char(ch, &timed);
    }
}

ACMD(do_mighthit)
{
    struct char_data *vict = NULL;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_MIGHTHIT)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
            vict = FIGHTING(ch);
        else {
            send_to_char("���� �� ������ ������ �������?\r\n", ch);
            return;
        }
    }

    if (vict == ch) {
        send_to_char("�� ������ ������� ����.\r\n", ch);
        return;
    }

    if (!(IS_NPC(ch) || IS_GOD(ch)) &&
        (GET_EQ(ch, WEAR_BOTHS) || GET_EQ(ch, WEAR_WIELD) ||
         GET_EQ(ch, WEAR_HOLD) || GET_EQ(ch, WEAR_SHIELD) || GET_EQ(ch, WEAR_LIGHT))) {
        send_to_char("���� ���������� ������ ��� ������� ����.\r\n", ch);
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (check_pkill(ch, vict, arg))
        return;

    go_mighthit(ch, vict);
}

/****************** STOPFIGHT ************************************/
ACMD(do_stopfight)
{
    struct char_data *tmp_ch;

    if (!FIGHTING(ch)) {
        send_to_char("�� �� �� �� � ��� �� ����������.\r\n", ch);
        return;
    }

    if (GET_POS(ch) < POS_FIGHTING) {
        send_to_char("�� ����� ��������� ��������� ����������.\r\n", ch);
        return;
    }

    for (tmp_ch = world[IN_ROOM(ch)].people; tmp_ch; tmp_ch = tmp_ch->next_in_room)
        if (FIGHTING(tmp_ch) == ch && CAN_SEE(tmp_ch, ch))
            break;

    if (tmp_ch && !(IS_AFFECTED(tmp_ch, AFF_HOLD) || IS_AFFECTED(tmp_ch, AFF_STOPFIGHT))) {
        send_to_char("�� ���������� �� ���� �����.\r\n", ch);
        return;
    } else {
        stop_fighting(ch, TRUE);
        if (!(IS_IMMORTAL(ch) || (!IS_NPC(ch) && GET_GOD_FLAG(ch, GF_GODSLIKE))))
            WAIT_STATE(ch, PULSE_VIOLENCE);
        send_to_char("�� ��������� �� �����.\r\n", ch);
        act("$n �����$g �� �����.", FALSE, ch, 0, 0, TO_ROOM);
    }
}

ACMD(do_charm)
{
    struct char_data *cvict = NULL;
    struct obj_data *ovict = NULL;
    struct affected_type af;
    int prob = 0, percent = 0, k;

    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch) || GET_SKILL(ch, SKILL_CHARM) <= 0) {
        send_to_char("�� �� �� ������ ���.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    k = generic_find(arg, FIND_CHAR_ROOM, ch, &cvict, &ovict);

    if (!k) {
        send_to_char("���� �� ������ ���������?\r\n", ch);
        return;
    }


    if (get_followers_num(ch, FLW_CHARM) != 0) {
        if (cvict->master && cvict->master == ch && low_charm(cvict)) {
            //������ ����������
        } else {
            send_to_char("������ ��� ��������? ������ ������� �������?\r\n", ch);
            return;
        }
    }

    if (cvict == ch) {
        send_to_char("�� ���� ��� ���������.\r\n", ch);
        return;
    }

    if (!IS_NPC(cvict)) {
        send_to_char("�� �� ������ ��������� ��� ��������!\r\n", ch);
        return;
    }

    if (GET_RACE(cvict) != RACE_ANIMAL && GET_RACE(cvict) != RACE_RODENT &&
        GET_RACE(cvict) != RACE_HORSE) {
        send_to_char("��������� ����� ������ ��������.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL)) {
        send_to_char("�� �� ������ ����� ��������� ��������.\r\n", ch);
        return;
    }

    if (!IS_GOD(ch) && MOB_FLAGGED(cvict, MOB_NOCHARM)) {
        send_to_char("���� ������ ��������� � �����!\r\n", ch);
        return;
    }

    if (AFF_FLAGGED(ch, AFF_CHARM)) {
        send_to_char("�� ���� ��������� ���-�� � �� ������ ����� ��������������.\r\n", ch);
        return;
    }

    if ((AFF_FLAGGED(cvict, AFF_CHARM) && cvict->master != ch) ||
        MOB_FLAGGED(cvict, MOB_AGGRESSIVE) ||
        MOB_FLAGGED(cvict, MOB_AGGRGOOD) ||
        MOB_FLAGGED(cvict, MOB_AGGREVIL) ||
        MOB_FLAGGED(cvict, MOB_AGGRNEUTRAL) ||
        MOB_FLAGGED(cvict, MOB_AGGR_DAY) || MOB_FLAGGED(cvict, MOB_AGGR_NIGHT))
        /*     MOB_FLAGGED(cvict, MOB_AGGR_WINTER) ||
           MOB_FLAGGED(cvict, MOB_AGGR_SPRING)   ||
           MOB_FLAGGED(cvict, MOB_AGGR_SUMMER)   ||
           MOB_FLAGGED(cvict, MOB_AGGR_AUTUMN) */
    {
        send_to_char("������ ��������� ��� ��������.\r\n", ch);
        return;
    }

    if (((cvict)->char_specials.fighting) || GET_POS(cvict) < POS_RESTING) {
        act("$N2 ������ �� �� ������ ��������.", TRUE, ch, 0, cvict, TO_CHAR);
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;
    }

    if (IS_HORSE(cvict)) {
        send_to_char("��� ������ ������, ��� ��������� ������.\r\n", ch);
        return;
    }

    act("�� �������� � ���� $N3 ������� $S ���������.", FALSE, ch, 0, cvict, TO_CHAR);
    act("$n �������$g � ���� $N3 ������� $S ���������.", TRUE, ch, 0, cvict, TO_ROOM);

    if (affected_by_spell(cvict, SPELL_STUNE) ||
        affected_by_spell(cvict, SPELL_HOLD) || IS_AFFECTED(cvict, AFF_HOLD)) {
        act("$N ��������������$G ���� �������� � �$M.", FALSE, ch, 0, cvict, TO_CHAR);
        act("�������������$G �������� $n1.", FALSE, ch, 0, cvict, TO_NOTVICT);
        WAIT_STATE(ch, PULSE_VIOLENCE);
        return;
    }

    percent = GET_REAL_INT(cvict) + GET_REAL_WIS(cvict) + RNDSKILL +
        ((GET_SAVE3(cvict, SAV_WILL) + saving_throws_3(cvict, SAV_WILL)) * 3);
    prob = GET_REAL_CHA(ch) + GET_REAL_CHA(ch) + RNDSKILL + GET_SKILL(ch, SKILL_CHARM);

    if ((GET_HIT(cvict) >= GET_REAL_MAX_HIT(cvict)) && (ch->LastCharm != GET_ID(cvict)))
        improove_skill(ch, cvict, 0, SKILL_CHARM);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    ch->LastCharm = GET_ID(cvict);

    if (percent > prob
        || (GET_LEVEL(cvict) > 5
            && ((ch->classes[CLASS_RANGER] * 3) / 2 + ((GET_REAL_CHA(ch) - 10) / 2) <
                GET_LEVEL(cvict)))) {
        act("��� �� ������� ��������� $N3 � $E ������$U �� ���.", FALSE, ch, 0, cvict, TO_CHAR);
        act("$N ������$U �� $n3.", FALSE, ch, 0, cvict, TO_NOTVICT);
        WAIT_STATE(ch, PULSE_VIOLENCE);

        if (may_pkill(ch, cvict) != PC_REVENGE_PC)
            inc_pkill(cvict, ch, 1, 0);

        check_position(cvict);
        set_hit(cvict, ch);
        return;
    }

    if (cvict->master) {
        if (stop_follower(cvict, SF_MASTERDIE))
            return;
    }

    affect_from_char(cvict, SKILL_CHARM);
    add_follower(cvict, ch, FLW_CHARM);
    clear_mob_specials(cvict);
    GET_EXP(cvict) = level_exp(cvict, GET_LEVEL(cvict) + 1);

//af.duration = GET_SKILL(ch,SKILL_CHARM) * 2 * SECS_PER_MUD_TICK;
    af.duration = -1;
    af.type = find_spell_num(SPELL_CHARM);
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_CHARM;
    af.battleflag = 0;
    af.main = TRUE;
    af.owner = GET_ID(ch);
    affect_join_char(cvict, &af);

    act("$n �������$g ���� ������ ���������, ��� �� ������ �� ��� ���� �$s.", TRUE, ch, 0, cvict,
        TO_VICT);

    REMOVE_BIT(MOB_FLAGS(cvict, MOB_AGGRESSIVE), MOB_AGGRESSIVE);
    REMOVE_BIT(MOB_FLAGS(cvict, MOB_SPEC), MOB_SPEC);

    change_pet_name(ch, cvict);
}

void go_switch_victim(struct char_data *ch, struct char_data *vict)
{
    int prob, percent, wait_ch = 0, wait_victim = 0;
    struct char_data *tmp_ch = NULL;

    if (GET_POS(ch) <= POS_SITTING) {
        send_to_charf(ch, "�� ����� ��������� ��� ������ ������������� �� ������� ����������.\r\n");
        return;
    }

    if (!FIGHTING(ch)) {
        send_to_charf(ch, "��� �� ����������� ��������� �  ��� ������ ���� ���������.\r\n");
        return;
    }

    tmp_ch = FIGHTING(ch);

    if (!may_kill_here(ch, vict))
        return;

    if (!ok_damage_shopkeeper(ch, vict)) {
        return;
    }

    if (check_spells_attacked(ch, vict, FALSE, FALSE))
        return;

    if (check_distance(ch, vict) != DIST_0) {
        send_to_charf(ch, "������������� ����� ������ �� ���������� �������� ���.\r\n");
        return;
    }

    prob =
        GET_REAL_DEX(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) + GET_SKILL(ch,
                                                                           SKILL_SWITCH) + RNDSKILL;
    percent =
        GET_REAL_STR(tmp_ch) + GET_REAL_DEX(tmp_ch) + GET_REAL_INT(tmp_ch) +
        ((GET_SAVE3(tmp_ch, SAV_REFL) + saving_throws_3(tmp_ch, SAV_REFL)) * 3) + RNDSKILL;


    improove_skill(ch, tmp_ch, 0, SKILL_SWITCH);


    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (prob > percent) {
        stop_fighting(ch, FALSE);
        act("�� ����������� ���� �������� �� $N3.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n ����������$g ���� �������� �� $N3.", FALSE, ch, 0, vict, TO_NOTVICT);
        act("$n ����������$g ���� �������� �� ���.", FALSE, ch, 0, vict, TO_VICT);
        set_fighting(ch, vict);
        wait_victim = 1;
        wait_ch = 1;
    } else {
        act("��� �� ������� ����������� ���� �������� �� $N3.", FALSE, ch, 0, vict, TO_CHAR);
        wait_ch = 2;
    }

    if (wait_ch)
        WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);

    if (wait_victim)
        WAIT_STATE(vict, PULSE_VIOLENCE * wait_victim);

}

ACMD(do_switch_victim)
{
    struct char_data *vict = NULL;
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_SWITCH)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (onhorse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }

    if (!FIGHTING(ch)) {
        send_to_charf(ch, "�� �� � ��� �� ����������.\r\n");
        return;
    }

    one_argument(argument, arg);

//if (!*arg || !arg)
    if (!*arg) {
        send_to_char("�� ���� �� ������ ����������� ���� ��������?\r\n", ch);
        return;
    }

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_char("���� �� �������.\r\n", ch);
        return;
    }

    if (vict == FIGHTING(ch)) {
        send_to_charf(ch, "�� ��� ���������� � �%s.\r\n", GET_CH_SUF_7(vict));
        return;
    }
    //send_to_charf(ch,"���������� ���� �������� �� %s\r\n",GET_PAD(vict,3));

    go_switch_victim(ch, vict);
}

/* ��������������� */
void go_counteract(struct char_data *ch, struct char_data *vict)
{
    int percent, prob;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ������������������.\r\n", ch);
        return;
    }

    if (onhorse(ch))
        return;

    prob =
        GET_REAL_DEX(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) + GET_SKILL(ch,
                                                                           SKILL_COUNTERACT) +
        RNDSKILL;
    percent =
        GET_REAL_DEX(vict) + GET_REAL_INT(vict) + GET_REAL_WIS(vict) +
        ((GET_SAVE3(vict, SAV_REFL) + saving_throws_3(vict, SAV_REFL)) * 3) + RNDSKILL;

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    act("�� ������ ������������������ ������ $N1.", FALSE, ch, 0, vict, TO_CHAR);
// send_to_charf(vict,"Prob %d Percent %d %d\r\n",prob,percent,GET_SKILL_MOB(ch,SKILL_COUNTERACT));
    if (prob >= percent)
        SET_AF_BATTLE(ch, EAF_CACT_RIGHT);

    GET_WAIT(ch) += PULSE_VIOLENCE;

}

ACMD(do_counteract)
{
    struct char_data *vict = NULL;
    char arg[MAX_STRING_LENGTH];

    if (!GET_SKILL(ch, SKILL_COUNTERACT)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (onhorse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }

    if (FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
        vict = FIGHTING(ch);

    if (!vict) {
        send_to_charf(ch, "�� �� � ��� �� ����������.\r\n");
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (check_pkill(ch, vict, arg))
        return;

    if (!used_attack(ch))
        go_counteract(ch, vict);

}

void go_critichit(struct char_data *ch, struct char_data *victim, int pos)
{
    int percent, prob, wait_ch = 0, wait_victim = 0, dam = 0, rsl = 0;
    struct timed_type timed;

//int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch,SKILL_CRITIC) : GET_SKILL(ch,SKILL_CRITIC);
    int skill = GET_SKILL(ch, SKILL_CRITIC);
    struct P_damage damage;
    struct P_message message;
    struct obj_data *weapObj = NULL;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || timed_by_skill(ch, SKILL_CRITIC)) {
        send_to_char("�� �������� �� � ��������� ������� ������ ����.\r\n", ch);
        return;
    }

    if (onhorse(ch))
        return;

    if (!ok_damage_shopkeeper(ch, victim))
        return;

    if (check_spells_attacked(ch, victim, FALSE, FALSE))
        return;

    if (check_distance(ch, victim) == DIST_1) {
        send_to_charf(ch, "������� ������ ���� ����� ������ ���������� �������� ���.\r\n");
        return;
    }

    prob = GET_REAL_STR(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) + skill + RNDSKILL;
    percent = (GET_REAL_DEX(victim) * 3) +
        ((GET_SAVE3(victim, SAV_REFL) + saving_throws_3(victim, SAV_REFL)) * 3) + RNDSKILL;

    improove_skill(ch, victim, 0, SKILL_CRITIC);

//��������� ������� ������
    if (IS_NPC(ch))
        dam =
            dice(ch->npc()->specials.damnodice,
                 ch->npc()->specials.damsizedice) + (ch->npc()->specials.damage);
    else
        dam = GET_REAL_DR(ch);

//��������� ���� ������
    if (!(weapObj = GET_EQ(ch, WEAR_WIELD)))
        weapObj = GET_EQ(ch, WEAR_BOTHS);

    if (weapObj)
        dam += get_weapon_damage(ch, victim, weapObj, NULL, TRUE, pos, TRUE, skill, FALSE);

    damage.valid = true;
    damage.type = HIT_SLASH;
    damage.power = GET_POWER(ch);
    damage.power += weapObj ? GET_OBJ_POWER(weapObj) : 0;
    damage.far_min = FALSE;
    damage.armor = TRUE;
    damage.weapObj = weapObj ? weapObj : NULL;
    damage.dam = dam + (dam * skill) / 25;
    damage.hLoc = pos;

    if (!IS_NPC(ch) && !IS_NPC(victim))
        damage.dam = (damage.dam * 8) / 10;     //�� ������� ����� �� 20% ������

    GetSkillMessage(SKILL_CRITIC, 0, message);


    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= 30[60]\r\n&n", prob);

    if ((float) prob / (float) percent < 1.0) {
        damage.check_ac = N_POWER;      //������
        rsl = _damage(ch, victim, 0, 0, C_POWER, TRUE, damage, message);
        wait_ch = 2;
    } else {
        damage.check_ac = A_POWER;      //������
        rsl = _damage(ch, victim, 0, 0, C_POWER, TRUE, damage, message);
        wait_ch = 1;
        wait_victim = 2;
    }

    if (is_critic(ch, 1, skill)) {
        update_pos(victim);
        if (GET_POS(victim) > POS_DEAD && rsl > 0)
            switch (pos) {
                case WEAR_BODY:
                    act("��� ������ ���� ��� ��������� �������, ��� � $N1 �������� ������������.",
                        FALSE, ch, 0, victim, TO_CHAR);
                    act("������ ���� $n1 ��� ��������� �������, ��� � ��� �������� ������������.",
                        FALSE, ch, 0, victim, TO_VICT);
                    act("������ ���� $n1 ��� ��������� �������, ��� � $N1 �������� ������������.",
                        FALSE, ch, 0, victim, TO_NOTVICT);
                    if (!IS_UNDEAD(victim) && !IS_CONSTRUCTION(victim)) {
                        struct affected_type af;

                        af.duration = 7;
                        af.type = find_spell_num(SPELL_HAEMORRAGIA);
                        af.modifier = skill;
                        af.location = 0;
                        af.bitvector = AFF_HAEMORRAGIA;
                        af.battleflag = 0;
                        af.level = skill;
                        af.owner = GET_ID(ch);
                        affect_join_char(victim, &af);
                    }
                    break;
                case WEAR_LEGS:
                    act("��� ������ ���� ��� ��������� �������, ��� � $N1 ����������� ����.", FALSE,
                        ch, 0, victim, TO_CHAR);
                    act("������ ���� $n1 ��� ��������� �������, ��� � ��� ����������� ����.", FALSE,
                        ch, 0, victim, TO_VICT);
                    act("������ ���� $n1 ��� ��������� �������, ��� � $N1 ����������� ����.", FALSE,
                        ch, 0, victim, TO_NOTVICT);
                    GET_POS(victim) = POS_SITTING;
                    break;
                case WEAR_ARMS:{
                        int pos =
                            GET_EQ(victim,
                                   WEAR_WIELD) ? WEAR_WIELD : (GET_EQ(victim,
                                                                      WEAR_BOTHS) ? WEAR_BOTHS :
                                                               WEAR_HOLD);
                        struct obj_data *weap = NULL;

                        if ((weap = unequip_char(victim, pos)) != NULL) {
                            act("��� ������ ���� ��� ��������� �������, ��� $N �������$G $o3.",
                                FALSE, ch, weap, victim, TO_CHAR);
                            act("������ ���� $n1 ��� ��������� �������, ��� �� �������� $o3.",
                                FALSE, ch, weap, victim, TO_VICT);
                            act("������ ���� $n1 ��� ��������� �������, ��� $N �������$G $o3.",
                                FALSE, ch, weap, victim, TO_NOTVICT);
                            check_sets(ch);
                            affect_total(ch);
                            obj_to_room(weap, IN_ROOM(victim));
                            obj_decay(weap);
                        }
                        break;
                    }
                case WEAR_HEAD:{
                        act("��� ������ ���� ��� ��������� �������, ��� $N3 ��������.", FALSE, ch,
                            0, victim, TO_CHAR);
                        act("������ ���� $n1 ��� ��������� �������, ��� ��� ��������.", FALSE, ch,
                            0, victim, TO_VICT);
                        act("������ ���� $n1 ��� ��������� �������, ��� $N3 ��������.", FALSE, ch,
                            0, victim, TO_NOTVICT);

                        struct affected_type af;

                        af.duration = 4;
                        af.type = find_spell_num(SPELL_STUNE);
                        af.modifier = 0;
                        af.location = 0;
                        af.bitvector = AFF_STOPFIGHT;
                        af.battleflag = 0;
                        af.owner = GET_ID(ch);
                        affect_join_char(victim, &af);
                    }
                    break;
            }
    }

    timed.skill = SKILL_CRITIC;
    timed.time = 6 * SECS_PER_MUD_ROUND;
    timed_to_char(ch, &timed);

    if (wait_ch)
        WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);
    if (wait_victim)
        WAIT_STATE(victim, PULSE_VIOLENCE * wait_victim);
}


ACMD(do_critichit)
{
    char part[256];
    char tname[256];
    int pos = -1;
    struct char_data *vict = NULL;
    char arg[MAX_STRING_LENGTH];

    if (!GET_SKILL(ch, SKILL_CRITIC)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    *part = '\0';

    two_arguments(argument, part, arg);

    if (!*part) {
        send_to_charf(ch, "������� ����� ����, ���� �� ������ �������.\r\n");
        return;
    }


    if (onhorse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
            vict = FIGHTING(ch);
        else {
            send_to_charf(ch, "���� �� ������ �������� ����?\r\n");
            return;
        }
    }

    for (int i = 0; pos == -1 && i < vict->ibody; i++)
        if (vict->body[i].name.size() != 0)
            for (int j = 0; j < NUM_PADS; j++) {
                strcpy(tname, get_name_pad(vict->body[i].name.c_str(), j));
                if (!strncmp(part, tname, strlen(part))) {
                    pos = wpos_to_wear[vict->body[i].wear_position];
                    goto go1;
                }
            }

  go1:
    if (pos == -1) {
        send_to_charf(ch, "�������� ����� ����.\r\n");
        return;
    }

    if (ch == vict) {
        act("�� �������� ����.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

    if (FIGHTING(ch) && FIGHTING(ch) != vict) {
        send_to_charf(ch,
                      "������� ����������� ����������� ����� ������ �� ���������� ����������.\r\n");
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (check_pkill(ch, vict, arg))
        return;

    log("������� ������� %d", pos);
    if (!used_attack(ch))
        go_critichit(ch, vict, pos);
}

void go_addshot(struct char_data *ch, struct char_data *victim)
{
    int prob, percent, i;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_ADDSHOT) : GET_SKILL(ch, SKILL_ADDSHOT);
    void stop_fighting(struct char_data *ch, int switch_others);
    std::vector < int >vit;
    std::vector < int >vot;


    if (IS_AFFECTED(ch, AFF_BLIND)) {
        send_to_charf(ch, "�� �� �� ������ ���� ����!\r\n");
        return;
    }

    prob = 0;
    percent = number(1, 10);

//��������� ��� �����
    vit.push_back(GET_REAL_DEX(ch));
    vit.push_back(GET_REAL_HR(ch) + dice(1, 10));
//��������� ��� ������
    vot.push_back(GET_REAL_DEX(victim));
    vot.push_back(GET_REAL_AC(victim) + compute_ac_magic(ch, victim));

    prob = calc_like_skill(ch, victim, SKILL_ADDSHOT, vit, vot);
    improove_skill(ch, victim, 0, SKILL_ADDSHOT);


    if (percent <= 5)
        i = 2;
    else if (percent >= 95)
        i = 0;
    else {
        i = dice(1, number(1, skill / 20));
        i += GET_REAL_DEX(ch) / 12;
    }

    /* if (PRF_FLAGGED(ch,PRF_CODERINFO))
       send_to_charf(ch,"&K���������� ������ ���-�� ��������� %d\r\n&n",i); */

    for (percent = 1; percent <= i; percent++) {
        if ((_damage(ch, victim, WEAP_RIGHT, 0, TRUE, C_POWER) == RD_NOARROW))
            percent = i + 1;

        if (percent < i)
            stop_fighting(victim, FALSE);
    }

    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
}

ACMD(do_addshot)
{
    struct obj_data *wield = NULL;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_ADDSHOT)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        send_to_charf(ch, "�� �� �� ������ ���� ����!\r\n");
        return;
    }

    if (onhorse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }

    wield = GET_EQ(ch, WEAR_WIELD) ? GET_EQ(ch, WEAR_WIELD) : GET_EQ(ch, WEAR_BOTHS);

    if (!wield) {
        send_to_charf(ch, "�� ������ ���� ��������� �����.\r\n");
        return;
    }

    if (GET_OBJ_SKILL(wield) != SKILL_BOWS) {
        act("$o �� �������� �����.", FALSE, ch, wield, 0, TO_CHAR);
        return;
    }

    if (*arg && !FIGHTING(ch)) {
        struct char_data *vict;

        if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
            send_to_char("� ���� ��������?\r\n", ch);
            return;
        }

        if (vict == ch) {
            send_to_char("�� ���������� ����.\r\n", ch);
            return;
        }

        if (!may_kill_here(ch, vict))
            return;

        if (check_pkill(ch, vict, arg))
            return;

        go_addshot(ch, vict);
        return;
    }

    if (!FIGHTING(ch)) {
        send_to_charf(ch, "�� �� �� � ��� �� ���������.\r\n");
        return;
    }

    if (check_distance(ch, FIGHTING(ch)) == DIST_0) {
        send_to_charf(ch, "������� ��������� ��������� �� ������ ������ �� �������� ���.\r\n");
        return;
    }

    act("�� ������������� � ��������������� �������� �� $o1.", FALSE, ch, wield, 0, TO_CHAR);
    act("1� ����������1(��,���,���,���) � ��������������� �������� �� @1�.", "���", ch, wield);
    SET_AF_BATTLE(ch, EAF_ADDSHOT);
    WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_addslash)
{
    int prob, percent;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_SATTACK) : GET_SKILL(ch, SKILL_SATTACK);
    struct char_data *victim = NULL;
    struct obj_data *wield = NULL;
    struct obj_data *hold = NULL;

    if (!GET_SKILL(ch, SKILL_SATTACK)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (onhorse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }

    if (!FIGHTING(ch)) {
        send_to_charf(ch, "�� �� �� � ��� �� ���������.\r\n");
        return;
    } else
        victim = FIGHTING(ch);

    wield = GET_EQ(ch, WEAR_WIELD) ? GET_EQ(ch, WEAR_WIELD) : GET_EQ(ch, WEAR_BOTHS);
    hold = GET_EQ(ch, WEAR_HOLD);

    if (!wield) {
        send_to_charf(ch, "�� ������ ����������� ������� �������.\r\n");
        return;
    }

    if (GET_OBJ_SKILL(wield) != SKILL_AXES) {
        act("$o �� ����������� ������ '������'.", FALSE, ch, wield, 0, TO_CHAR);
        return;
    }

    if (check_distance(ch, FIGHTING(ch)) == DIST_1) {
        send_to_charf(ch, "������� ��������� ������� ������ �� ������ ������ � ������� ���.\r\n");
        return;
    }

    percent = number(1, 100);
    prob = skill + GET_REAL_DEX(ch) + GET_REAL_HR(ch) + dice(1, 20);
    prob = prob - GET_REAL_DEX(victim) - GET_REAL_AC(victim) - compute_ac_magic(ch, victim);
    prob = calc_like_skill(ch, victim, SKILL_SATTACK, prob);
    improove_skill(ch, victim, 0, SKILL_SATTACK);

    act("�� ������������� � ��������������� ����� $o4.", FALSE, ch, wield, 0, TO_CHAR);


    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (prob >= percent)
        SET_AF_BATTLE(ch, EAF_ADDSLASH);

    if (hold && GET_OBJ_TYPE(hold) == ITEM_WEAPON)
        if (GET_OBJ_SKILL(hold) == SKILL_AXES) {
            prob /= 2;
            if (prob >= percent)
                SET_AF_BATTLE(ch, EAF_ADDSLASH_LEFT);
        }

    WAIT_STATE(ch, PULSE_VIOLENCE);
}

ACMD(do_guard)
{
    struct char_data *vict;

    if (!GET_SKILL(ch, SKILL_GUARD)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    skip_spaces(&argument);

    if (!*argument) {
        stop_guarding(ch);
        return;
    }

    if (!(vict = give_find_vict(ch, argument)))
        return;

    if (vict == ch) {
        send_to_charf(ch, "�� ��� ��������� ����.\r\n");
        return;
    }

    stop_guarding(ch);

    act("�� ������ ��������� $N3.", TRUE, ch, 0, vict, TO_CHAR);
    act("$n �����$g �������� ���.", TRUE, ch, 0, vict, TO_VICT);
    act("$n �����$g �������� $N3.", TRUE, ch, 0, vict, TO_NOTVICT);

    GUARDING(ch) = vict;
}

int go_guard(struct char_data *ch, struct char_data *vict, struct char_data *tmp_ch)
{
    int percent, prob;
    int wait_ch, wait_victim;
    struct char_data *tch, *first;
    std::vector < int >vit;
    std::vector < int >vot;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return (FALSE);
    }

    if (SECT(IN_ROOM(ch) == SECT_UNDERWATER)) {
        send_to_charf(ch, "�������� ��� ����� ����������.\r\n");
        return (FALSE);
    }

    percent = number(1, 100);
    //��������� ��� �����
    vit.push_back(GET_REAL_DEX(ch));
    vit.push_back(GET_REAL_CON(ch));
    //��������� ��� ������
    vot.push_back(GET_REAL_DEX(vict));
    vot.push_back(GET_REAL_CON(vict));

    prob = calc_like_skill(ch, vict, SKILL_GUARD, vit, vot);
    improove_skill(ch, vict, 0, SKILL_GUARD);

    if (percent <= 5)
        prob = 100;
    else if (percent >= 95)
        prob = 0;

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (percent > prob) {
        act("�� ���������� �������� ������ $N3.", FALSE, ch, 0, vict, TO_CHAR);
        act("$n ���������� �����$u ������ ���.", FALSE, ch, 0, vict, TO_VICT);
        act("$n ���������� �����$u ������ $N3.", FALSE, ch, 0, vict, TO_NOTVICT);
        GUARDING(ch) = 0;
        wait_ch = 2;
        WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);
        return (FALSE);
    }

    act("������, �� ����� ���������� $N3 � �������.", FALSE, ch, 0, vict, TO_CHAR);
    act("������, $n ����� ���������$g ��� � �������.", FALSE, ch, 0, vict, TO_VICT);
    act("������, $n ����� ���������$g $N3 � �������.", TRUE, ch, 0, vict, TO_NOTVICT);
    GUARDING(ch) = 0;
    wait_victim = 1;
    wait_ch = 1;

    //������ ������
    if (14 > (dice(1, GET_REAL_DEX(vict)) + GET_REAL_LCK(vict)) && GET_POS(vict) > POS_SITTING) {
        if (IS_HORSE(vict) && on_horse(vict->master))
            horse_drop(vict);

        act("�� ������� ����������, $N ����$G!", FALSE, ch, 0, vict, TO_CHAR);
        act("�� �� �������� ���������� � �����!", FALSE, ch, 0, vict, TO_VICT);
        act("�� ������� ����������, $N ����$G!", FALSE, ch, 0, vict, TO_NOTVICT);
        GET_POS(vict) = POS_SITTING;
        WAIT_STATE(vict, PULSE_VIOLENCE * 1);
    } else if (on_horse(vict) && get_horse_on(vict)) {
        REMOVE_BIT(AFF_FLAGS(vict, AFF_HORSE), AFF_HORSE);
        REMOVE_BIT(AFF_FLAGS(get_horse_on(vict), AFF_HORSE), AFF_HORSE);
    }

    first = tmp_ch;
    for (tch = world[IN_ROOM(vict)].people; tch; tch = tch->next_in_room) {
        if (tch == first)
            continue;
        if (FIGHTING(tch) == vict)
            tmp_ch = tch;

        if (FIGHTING(vict) == tmp_ch)
            stop_fighting(vict, FALSE);

        if (!IS_NPC(ch))
            if (may_pkill(ch, tmp_ch) != PC_REVENGE_PC)
                inc_pkill_group(tmp_ch, ch, 1, 0);

        if (FIGHTING(ch))
            FIGHTING(ch) = tmp_ch;
        else
            set_fighting(ch, tmp_ch);
        if (FIGHTING(tmp_ch))
            FIGHTING(tmp_ch) = ch;
        else
            set_fighting(tmp_ch, ch);
    }
    if (FIGHTING(ch))
        FIGHTING(ch) = first;
    else
        set_fighting(ch, first);
    if (FIGHTING(first))
        FIGHTING(first) = ch;
    else
        set_fighting(first, ch);

    WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);
    WAIT_STATE(vict, PULSE_VIOLENCE * wait_victim);

    return (TRUE);
}

void go_bloodlet(struct char_data *ch, struct char_data *victim)
{
    int prob, percent, dam = 0, hTyp = HIT_SLASH;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_BLOODLET) : GET_SKILL(ch, SKILL_BLOODLET);
    struct P_damage damage;
    struct P_message message;
    struct weapon_damage_data *damages;
    struct obj_data *weapObj = NULL;
    struct timed_type timed;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }

    if (onhorse(ch))
        return;

    if (!ok_damage_shopkeeper(ch, victim))
        return;

    if (check_spells_attacked(ch, victim, FALSE, FALSE))
        return;

    if (check_distance(ch, victim) == DIST_1) {
        send_to_charf(ch, "������� ����� ����� ������ ���������� �������� ���.\r\n");
        return;
    }
//!IS_GOD(ch) &&
    if (timed_by_skill(ch, SKILL_BLOODLET)) {
        send_to_char("� ��� �� ������ ���.\r\n", ch);
        return;
    }

    if (IS_UNDEAD(victim) || IS_CONSTRUCTION(victim)) {
        act("�� �� ����� ��������� ����� � 2�.", "��", ch, victim);
        return;
    }

    if (GET_EQ(ch, WEAR_WIELD) &&
        (GET_OBJ_SKILL(GET_EQ(ch, WEAR_WIELD)) == SKILL_DAGGERS ||
         (GET_OBJ_SKILL(GET_EQ(ch, WEAR_WIELD)) == SKILL_SWORDS)))
        weapObj = GET_EQ(ch, WEAR_WIELD);

    damage.valid = true;
    damage.type = real_attack_type(hTyp);

    if (IS_NPC(ch)) {
        int dam;

        //� ���� �� ������ ����� ���� ������ �����
        dam =
            dice(ch->npc()->specials.damnodice,
                 ch->npc()->specials.damsizedice) + (ch->npc()->specials.damage);
        dam = dam * MAX(1, (((skill * 100) / 25) / 100));
    }

    if (weapObj) {
        int percent, damz;

        percent = dice(GET_OBJ_VAL(weapObj, 1), GET_OBJ_VAL(weapObj, 2)) + GET_OBJ_VAL(weapObj, 0);
        for (damages = weapObj->weapon->damages; damages; damages = damages->next)
            if (damages->type_damage == HIT_PICK || damages->type_damage >= HIT_FIRE) {
                damage.type = damages->type_damage;
                percent += number(damages->min_damage, damages->max_damage);
            }
        percent = MIN(percent, percent * GET_OBJ_CUR(weapObj) / MAX(1, GET_OBJ_MAX(weapObj)));
        damz = MAX(1, percent);
        dam = damz * MAX(1, (((skill * 100) / 15) / 100));
    } else
        hTyp = 7;               //��������

    dam += GET_REAL_DR(ch);

    damage.dam = dam;
    damage.power = GET_POWER(ch);
    damage.power += weapObj ? GET_OBJ_POWER(weapObj) : 0;
    damage.far_min = FALSE;
    damage.armor = TRUE;
    damage.weapObj = weapObj ? weapObj : NULL;

    GetSkillMessage(SKILL_BLOODLET, hTyp, message);

    prob = GET_REAL_DEX(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) + skill + RNDSKILL;
    percent = GET_REAL_DEX(victim) + GET_REAL_DEX(victim) + GET_REAL_CON(victim) +
        GET_SAVE3(victim, SAV_FORT) + saving_throws_3(victim, SAV_FORT) + RNDSKILL;

    improove_skill(ch, victim, 0, SKILL_BLOODLET);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d > %d\r\n&n", prob, percent);

    if (percent > prob)
        damage.check_ac = N_POWER;      //������
    else {
        damage.check_ac = A_POWER;      //������
        struct affected_type af;

        af.duration = MAX(4, skill / 5);
        af.type = find_spell_num(SPELL_HAEMORRAGIA);
        af.modifier = skill;
        af.location = 0;
        af.bitvector = AFF_HAEMORRAGIA;
        af.battleflag = TRUE;
        af.main = TRUE;
        af.owner = GET_ID(ch);
        affect_join_char(victim, &af);
    }
//send_to_charf(ch,"dam %d\r\n",damage.dam);
    _damage(ch, victim, 0, 0, C_POWER, TRUE, damage, message);


    WAIT_STATE(ch, PULSE_VIOLENCE);
    timed.skill = SKILL_BLOODLET;
    timed.time = 6 * SECS_PER_MUD_ROUND;
    timed_to_char(ch, &timed);
}

ACMD(do_bloodlet)
{
    struct char_data *vict = NULL;
    char arg[MAX_STRING_LENGTH];

    *arg = '\0';
    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_BLOODLET)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }

    if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
        vict = FIGHTING(ch);
    else if (*arg)
        vict = get_char_vis(ch, arg, FIND_CHAR_ROOM);

    if (!vict) {
        send_to_char("���� �� ������ ������� �����?\r\n", ch);
        return;
    }

    if (!IS_MOB(ch) && !GET_EQ(ch, WEAR_WIELD)) {
        send_to_charf(ch, "���������� ������� ������ � ������ ����.\r\n");
        return;
    }

    if (!IS_MOB(ch) && GET_OBJ_SKILL(GET_EQ(ch, WEAR_WIELD)) != SKILL_DAGGERS &&
        GET_OBJ_SKILL(GET_EQ(ch, WEAR_WIELD)) != SKILL_SWORDS) {
        send_to_char("������� ����� ����� ������ ������� ������ '�������' ��� '����'.\r\n", ch);
        return;
    }

    if (vict == ch) {
        send_to_char("�� ��������� ���� ����.\r\n", ch);
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (check_pkill(ch, vict, arg))
        return;

    go_bloodlet(ch, vict);
}


void go_shieldhit(struct char_data *ch, struct char_data *victim)
{
    int prob, percent, dam = 0, weap_i;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_SHIELDHIT) : GET_SKILL(ch, SKILL_SHIELDHIT);
    struct P_damage damage;
    struct P_message message;
    struct obj_data *weapObj = NULL;
    struct timed_type timed;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }

    if (onhorse(ch))
        return;

    if (!ok_damage_shopkeeper(ch, victim))
        return;

    if (check_distance(ch, victim) == DIST_1) {
        send_to_charf(ch, "���� ����� ����� ������� ������ �� ���������� �������� ���.\r\n");
        return;
    }

    if (check_spells_attacked(ch, victim, FALSE, FALSE))
        return;

//!IS_GOD(ch) &&
    if (timed_by_skill(ch, SKILL_SHIELDHIT)) {
        send_to_char("� ��� �� ������ ���.\r\n", ch);
        return;
    }

    if (GET_EQ(ch, WEAR_SHIELD))
        weapObj = GET_EQ(ch, WEAR_SHIELD);
    else if ((weap_i = real_object(OBJ_SHIELD)) >= 0)
        weapObj = (obj_proto + weap_i);


    if (IS_NPC(ch))             //� ���� �� ������ ����� ���� ������ �����
        dam =
            dice(ch->npc()->specials.damnodice,
                 ch->npc()->specials.damsizedice) + (ch->npc()->specials.damage);
    else
        dam = GET_REAL_DR(ch);

    if (weapObj) {
        int percent;

        percent = dice(1, GET_OBJ_WEIGHT(weapObj) / 2500);
        percent = MIN(percent, percent * GET_OBJ_CUR(weapObj) / MAX(1, GET_OBJ_MAX(weapObj)));
        dam += MAX(1, percent);
    }

    dam = dam + (dam * skill) / 80;

    damage.valid = true;
    damage.type = HIT_BLOW;
    damage.dam = dam;
    damage.power = GET_POWER(ch);
    damage.power += weapObj ? GET_OBJ_POWER(weapObj) : 0;
    damage.far_min = FALSE;
    damage.armor = FALSE;
    damage.weapObj = weapObj ? weapObj : NULL;

    GetSkillMessage(SKILL_SHIELDHIT, 0, message);
    prob =
        GET_REAL_STR(ch) + GET_REAL_STR(ch) + GET_REAL_DEX(ch) + GET_REAL_WEIGHT(ch) + skill +
        RNDSKILL;
    percent =
        GET_REAL_STR(victim) + GET_REAL_DEX(victim) + GET_REAL_DEX(victim) +
        GET_REAL_WEIGHT(victim) +
        ((GET_SAVE3(victim, SAV_REFL) + saving_throws_3(victim, SAV_REFL)) * 3) + RNDSKILL;

    improove_skill(ch, victim, 0, SKILL_SHIELDHIT);

    switch (GET_ARM_TYPE(weapObj)) {
        case TARM_NONE:
        case TARM_LIGHT:
        case TARM_WEAR:
        case TARM_JEWS:        //������ ���
            percent -= (percent * 15) / 100;
            break;
        case TARM_MEDIUM:
            percent += (percent * 15) / 100;
            break;
        case TARM_HARD:
            percent += (percent * 25) / 100;
            break;
    }

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (percent > prob)
        damage.check_ac = N_POWER;      //������
    else
        damage.check_ac = A_POWER;      //������

//send_to_charf(ch,"dam %d\r\n",damage.dam);
    _damage(ch, victim, 0, 0, C_POWER, TRUE, damage, message);

    WAIT_STATE(ch, PULSE_VIOLENCE);

    timed.skill = SKILL_SHIELDHIT;
    timed.time = 3 * SECS_PER_MUD_ROUND;
    timed_to_char(ch, &timed);
}

ACMD(do_shieldhit)
{
    struct char_data *victim = NULL;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!GET_SKILL(ch, SKILL_SHIELDHIT)) {
        send_to_char("�� �� ������� � ���� �������.\r\n", ch);
        return;
    }

    if (!(victim = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
            victim = FIGHTING(ch);
        else {
            send_to_char("���� �� ������ ������� �����?\r\n", ch);
            return;
        }
    }

    if (!IS_MOB(ch) && !GET_EQ(ch, WEAR_SHIELD)) {
        send_to_charf(ch, "��� ���������� ����� ����� ���������� ������� ���.\r\n");
        return;
    }

    if (victim == ch) {
        send_to_char("�� ������� � ��� ����� ������� ����.\r\n", ch);
        return;
    }

    if (!may_kill_here(ch, victim))
        return;

    if (check_pkill(ch, victim, arg))
        return;

    go_shieldhit(ch, victim);
}


void go_circlestab(struct char_data *ch, struct char_data *victim)
{
    if (AFF_FLAGGED(ch, AFF_STOPFIGHT)) {
        send_to_char("�� �������� �� � ��������� ���������.\r\n", ch);
        return;
    }

    if (onhorse(ch))
        return;

    if (!ok_damage_shopkeeper(ch, victim))
        return;

    if (check_spells_attacked(ch, victim, FALSE, FALSE))
        return;

    if (check_distance(ch, victim) == DIST_1) {
        send_to_charf(ch, "������ ����� ����� ������� ������ ��� ���������� �������� ���.\r\n");
        return;
    }

    SET_AF_BATTLE(ch, EAF_CRITIC);
    send_to_charf(ch, "�� ������������� ��������� ������ �����.\r\n");
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);

}

ACMD(do_circlestab)
{
    struct char_data *victim = NULL;

    if (!GET_SKILL(ch, SKILL_CIRCLESTAB)) {
        send_to_char("�� �� ������� � ���� �������.\r\n", ch);
        return;
    }

    if (!FIGHTING(ch)) {
        send_to_charf(ch, "���������� ���� � ���, ����� ������������ ��� ������.\r\n");
        return;
    }

    victim = FIGHTING(ch);

    if (!victim)
        return;

    if (!IS_MOB(ch)
        && (!GET_EQ(ch, WEAR_WIELD) || !GET_EQ(ch, WEAR_WIELD)->weapon
            || GET_OBJ_SKILL(GET_EQ(ch, WEAR_WIELD)) != SKILL_DAGGERS)) {
        send_to_charf(ch, "���������� ������� ������ � ������ ����.\r\n");
        return;
    }

    if (!IS_MOB(ch)
        && (!GET_EQ(ch, WEAR_HOLD) || !GET_EQ(ch, WEAR_HOLD)->weapon
            || GET_OBJ_SKILL(GET_EQ(ch, WEAR_HOLD)) != SKILL_DAGGERS)) {
        send_to_charf(ch, "���������� ������� ������ � ����� ����.\r\n");
        return;
    }
    go_circlestab(ch, victim);
}

void go_throw(struct char_data *ch, struct char_data *victim, struct obj_data *object)
{
    struct obj_data *tObj = NULL;
    struct P_damage damage;
    struct P_message message;
    int weight, dam = 0, percent = 0, prob = 0;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_THROW) : GET_SKILL(ch, SKILL_THROW);

    if (!object)
        tObj = ch->carrying;
    else
        tObj = object;

    if (!tObj) {
        send_to_charf(ch, "��� ����� ������� � ����������.\r\n");
        return;
    }

    weight = GET_OBJ_WEIGHT(tObj);

    if (weight / 1000 > GET_REAL_STR(ch)) {
        act("$O ������� �����$d, ��� �� �� ����� $l �������.", FALSE, ch, tObj, 0, TO_CHAR);
        return;
    }

    if (check_distance(ch, victim) == DIST_0) {
        send_to_charf(ch, "������� ������� � ���������� �������� ��� ������.\r\n");
        return;
    }

    dam = GET_OBJ_WEIGHT(tObj) / 500;
    if (GET_OBJ_TYPE(tObj) == ITEM_WEAPON) {
        int percent;

        percent = dice(GET_OBJ_VAL(tObj, 1), GET_OBJ_VAL(tObj, 2)) + GET_OBJ_VAL(tObj, 0);
        percent = MIN(percent, percent * GET_OBJ_CUR(tObj) / MAX(1, GET_OBJ_MAX(tObj)));
        dam += MAX(1, percent);
        damage.type = real_attack_type(GET_OBJ_VAL(tObj, 3));
    } else {
        damage.type = HIT_BLOW;
        dam += GET_REAL_DR(ch) / 2;
    }

    damage.valid = true;
    dam = MAX(dam, (((skill * dam * 100) / 20) / 100));
    damage.dam = dam;
    damage.power = tObj ? GET_OBJ_POWER(tObj) : 0;
    damage.far_min = TRUE;
    damage.armor = FALSE;
    damage.weapObj = tObj ? tObj : NULL;

    GetSkillMessage(SKILL_THROW, 0, message);

    percent = number(1, 100);
    prob =
        skill + GET_REAL_DEX(ch) + GET_REAL_STR(ch) - GET_REAL_DEX(victim) - GET_REAL_STR(victim);

    prob = calc_like_skill(ch, victim, SKILL_THROW, prob);
    improove_skill(ch, victim, 0, SKILL_THROW);

    if (percent <= 5)
        prob = 100;
    else if (percent >= 95)
        prob = 0;

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d >= %d\r\n&n", prob, percent);

    if (percent > prob)
        damage.check_ac = N_POWER;      //������
    else
        damage.check_ac = A_POWER;      //������

    _damage(ch, victim, 0, 0, C_POWER, TRUE, damage, message);

    set_wait(ch, 1, TRUE);

    obj_from_char(tObj);
    obj_to_char(tObj, victim);

}

ACMD(do_throw)
{
    struct char_data *victim = NULL;
    struct obj_data *object = NULL;
    char arg[MAX_STRING_LENGTH];
    char objn[MAX_STRING_LENGTH];

    *objn = '\0';

    if (!GET_SKILL(ch, SKILL_THROW)) {
        send_to_char("�� �� ������� � ���� �������.\r\n", ch);
        return;
    }

    two_arguments(argument, arg, objn);

    if (*objn) {
        generic_find(objn, FIND_OBJ_INV, ch, &victim, &object);
        if (!object) {
            send_to_charf(ch, "� ��� ��� '%s'.\r\n", objn);
            return;
        }
    }

    victim = NULL;
    if (!(victim = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
            victim = FIGHTING(ch);
        else {
            send_to_char("� ���� �� ������ �������?\r\n", ch);
            return;
        }
    }

    if (victim == ch) {
        send_to_char("������� � ����? �����������.\r\n", ch);
        return;
    }

    if (!may_kill_here(ch, victim))
        return;

    if (check_pkill(ch, victim, arg))
        return;

    go_throw(ch, victim, object);
}

void go_snapshot(struct char_data *ch, struct char_data *victim, int pos)
{
    int percent, prob, wait_ch = 0, wait_victim = 0, dam = 0, rsl = 0;
    struct timed_type timed;
    int skill = GET_SKILL(ch, SKILL_SNAPSHOT);
    struct P_damage damage;
    struct P_message message;
    struct obj_data *weapObj = NULL, *missileObj = NULL;

    if (AFF_FLAGGED(ch, AFF_STOPFIGHT) || timed_by_skill(ch, SKILL_SNAPSHOT)) {
        send_to_char("�� �������� �� � ��������� ���������� ������ �������.\r\n", ch);
        return;
    }

    if (onhorse(ch))
        return;

    if (!ok_damage_shopkeeper(ch, victim))
        return;

    if (check_spells_attacked(ch, victim, FALSE, FALSE))
        return;

//��������� ���� ������
    if (!(weapObj = GET_EQ(ch, WEAR_WIELD)))
        weapObj = GET_EQ(ch, WEAR_BOTHS);

    if (weapObj) {
        missileObj = get_missile(ch, weapObj);
        if (missileObj == NULL) {
            act("� ��� ����������� ������ ��� @1�.", "��", ch, weapObj);
            return;
        }
    }
//��������� ������� ������
    if (IS_NPC(ch))
        dam =
            dice(ch->npc()->specials.damnodice,
                 ch->npc()->specials.damsizedice) + (ch->npc()->specials.damage);
    else
        dam = (GET_REAL_DEX(ch) - 10) / 2;

    if (weapObj && missileObj) {
        dam += get_weapon_damage(ch, victim, weapObj, missileObj, TRUE, pos, TRUE, skill, FALSE);
    }

    if (missileObj != NULL && missileObj->missile) {
        missileObj->obj_flags.value[2]--;
        if (missileObj->obj_flags.value[2] <= 0)
            extract_obj(missileObj);
    }

    prob = GET_REAL_STR(ch) + GET_REAL_DEX(ch) + GET_REAL_INT(ch) + skill + RNDSKILL;
    percent = (GET_REAL_DEX(victim) * 3) +
        ((GET_SAVE3(victim, SAV_REFL) + saving_throws_3(victim, SAV_REFL)) * 3) + RNDSKILL;

    improove_skill(ch, victim, 0, SKILL_SNAPSHOT);

    damage.valid = true;
    damage.type = HIT_PICK;
    damage.power = GET_POWER(ch);
    damage.power += weapObj ? GET_OBJ_POWER(weapObj) : 0;
    damage.far_min = TRUE;
    damage.armor = TRUE;
    damage.weapObj = weapObj ? weapObj : NULL;
    damage.dam = dam + (dam * skill) / 25;
    damage.hLoc = pos;

    GetSkillMessage(SKILL_SNAPSHOT, HIT_PICK, message);


    switch (pos) {              //������������ �� ���������
        case WEAR_FACE:
            prob -= 40;
            break;
        case WEAR_HEAD:
            prob -= 20;
            break;
        case WEAR_ARMS:
            prob -= 15;
            break;
        case WEAR_LEGS:
            prob -= 10;
            break;
    }

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&K���������� ������ %d vs %d\r\n&n", prob, percent);

    if ((float) prob / (float) percent < 1.2) { //��� ���������� ������
        switch (pos) {
            case WEAR_FACE:
                damage.dam *= 2;
                break;
            case WEAR_HEAD:
                damage.dam = (damage.dam * 15) / 10;
                break;
        }
    }

//send_to_charf(ch,"������2 %s\r\n",GET_NAME(victim));

    if (!IS_NPC(ch) && !IS_NPC(victim))
        damage.dam = (damage.dam * 8) / 10;     //�� ������� ������� �� 20% ������

    if ((float) prob / (float) percent < 1.0) {
        damage.check_ac = N_POWER;      //������
        rsl = _damage(ch, victim, 0, 0, C_POWER, TRUE, damage, message);
        wait_ch = 2;
    } else {
        damage.check_ac = A_POWER;      //������
        rsl = _damage(ch, victim, 0, 0, C_POWER, TRUE, damage, message);
        wait_ch = 1;
        wait_victim = 2;
    }

    if (is_critic(ch, 1, skill)) {
        rsl = _damage(ch, victim, 0, 0, C_POWER, TRUE, damage, message);
        update_pos(victim);
        if (GET_POS(victim) > POS_DEAD && rsl > 0)
            switch (pos) {
                case WEAR_BODY:
                    act("��������� ������ ��������.", FALSE, ch, 0, victim, TO_CHAR);
                    act("��������� ��� ��������.", FALSE, ch, 0, victim, TO_VICT);
                    act("��������� ������ ��������.", FALSE, ch, 0, victim, TO_NOTVICT);
                    struct affected_type af;

                    af.duration = MAX(1, skill / 5);
                    af.type = find_spell_num(SPELL_UNHOLY);
                    af.modifier = -2;
                    af.location = APPLY_DEX;
                    af.bitvector = AFF_NOTHING;
                    af.battleflag = TRUE;
                    af.owner = GET_ID(ch);
                    af.main = TRUE;
                    affect_join_char(victim, &af);
                    break;
                case WEAR_LEGS:
                    act("����������� �� �����.", FALSE, ch, 0, victim, TO_CHAR);
                    act("����������� �� �����.", FALSE, ch, 0, victim, TO_VICT);
                    act("����������� �� �����.", FALSE, ch, 0, victim, TO_NOTVICT);
                    GET_POS(victim) = POS_SITTING;
                    break;
                case WEAR_ARMS:{
                        act("����������� �� ����.", FALSE, ch, 0, victim, TO_CHAR);
                        act("����������� �� ����.", FALSE, ch, 0, victim, TO_VICT);
                        act("����������� �� ����.", FALSE, ch, 0, victim, TO_NOTVICT);
                        struct affected_type af;

                        af.duration = MAX(1, skill / 5);
                        af.type = find_spell_num(SPELL_UNHOLY);
                        af.modifier = -20;
                        af.location = APPLY_HITROLL;
                        af.bitvector = AFF_NOTHING;
                        af.battleflag = TRUE;
                        af.owner = GET_ID(ch);
                        af.main = TRUE;
                        affect_join_char(victim, &af);
                    }
                    break;
                case WEAR_HEAD:{
                        act("������ ��������.", FALSE, ch, 0, victim, TO_CHAR);
                        act("��� ��������.", FALSE, ch, 0, victim, TO_VICT);
                        act("$N3 ��������.", FALSE, ch, 0, victim, TO_NOTVICT);
                        struct affected_type af;

                        af.duration = 2;
                        af.type = find_spell_num(SPELL_UNHOLY);
                        af.modifier = skill;
                        af.location = 0;
                        af.bitvector = AFF_STOPFIGHT;
                        af.battleflag = TRUE;
                        af.owner = GET_ID(ch);
                        af.main = TRUE;
                        affect_join_char(victim, &af);
                    }
                    break;
                case WEAR_FACE:{
                        if (!IS_UNDEAD(victim) && !IS_CONSTRUCTION(victim)) {
                            act("������ ��������.", FALSE, ch, 0, victim, TO_CHAR);
                            act("��� ��������.", FALSE, ch, 0, victim, TO_VICT);
                            act("$N3 ��������.", FALSE, ch, 0, victim, TO_NOTVICT);
                            struct affected_type af;

                            af.duration = MAX(1, skill / 10);
                            af.type = find_spell_num(SPELL_BLIND);
                            af.modifier = skill;
                            af.location = 0;
                            af.bitvector = AFF_BLIND;
                            af.battleflag = TRUE;
                            af.owner = GET_ID(ch);
                            af.main = TRUE;
                            affect_join_char(victim, &af);
                        }
                    }
                    break;
            }
    }

    timed.skill = SKILL_SNAPSHOT;
    timed.time = 6 * SECS_PER_MUD_ROUND;
    timed_to_char(ch, &timed);

    if (wait_ch)
        WAIT_STATE(ch, PULSE_VIOLENCE * wait_ch);
    if (wait_victim)
        WAIT_STATE(victim, PULSE_VIOLENCE * wait_victim);

}

ACMD(do_snapshot)
{
    char part[256];
    char tname[256];
    int pos = -1;
    struct char_data *vict = NULL;
    struct obj_data *wield;
    char arg[MAX_STRING_LENGTH];

    if (!GET_SKILL(ch, SKILL_SNAPSHOT)) {
        send_to_char("�� �� ������ ���.\r\n", ch);
        return;
    }
    *arg = '\0';
    *part = '\0';

    two_arguments(argument, part, arg);

    if (!*part) {
        send_to_charf(ch, "������� ����� ����, ���� �� ������ ����������.\r\n");
        return;
    }


    if (onhorse(ch)) {
        send_to_char("��� ����� ����� ���������.\r\n", ch);
        return;
    }


    wield = GET_EQ(ch, WEAR_WIELD) ? GET_EQ(ch, WEAR_WIELD) : GET_EQ(ch, WEAR_BOTHS);

    if (!wield) {
        send_to_charf(ch, "�� ������ ���� ��������� ���������� �������.\r\n");
        return;
    }

    if (GET_OBJ_SKILL(wield) != SKILL_BOWS && GET_OBJ_SKILL(wield) != SKILL_CROSSBOWS) {
        act("$o �� �������� ���������� �������.", FALSE, ch, wield, 0, TO_CHAR);
        return;
    }
    if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
        vict = FIGHTING(ch);
    else if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_charf(ch, "� ���� �� ������ ���������� �������?\r\n");
        return;
    }

    for (int i = 0; pos == -1 && i < vict->ibody; i++)
        if (vict->body[i].name.size() != 0)
            for (int j = 0; j < NUM_PADS; j++) {
                strcpy(tname, get_name_pad(vict->body[i].name.c_str(), j));
                if (!strncmp(part, tname, strlen(part))) {
                    pos = wpos_to_wear[vict->body[i].wear_position];
                    goto go2;
                }
            }

  go2:
    if (pos == -1) {
        send_to_charf(ch, "�������� ����� ����.\r\n");
        return;
    }

    if (ch == vict) {
        act("�� ��������� ����.", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

//send_to_charf(ch,"������ %s\r\n",GET_NAME(vict));


    if (check_distance(ch, vict) == DIST_0) {
        send_to_charf(ch, "������� ������ ������� ����� ������ � ������� �������� ���.\r\n");
        return;
    }

    if (!may_kill_here(ch, vict))
        return;

    if (check_pkill(ch, vict, arg))
        return;

    if (!used_attack(ch))
        go_snapshot(ch, vict, pos);

}

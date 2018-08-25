/* ************************************************************************
*   File: act.movement.c                                Part of CircleMUD *
*  Usage: movement commands, door handling, & sleep/rest/etc state        *
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
#include "interpreter.h"
#include "handler.h"
#include "act.social.h"
#include "db.h"
#include "spells.h"
#include "house.h"
#include "constants.h"
#include "pk.h"
#include "case.h"
#include "events.h"
#include "ai.h"
#include "xboot.h"

/* local functions */
void check_ranger_trap(struct char_data *ch);
void check_necro_spare(struct char_data *ch);
void check_ice(int room);
int has_boat(struct char_data *ch);
void train_sneak(struct char_data *ch);
int check_sneak(struct char_data *ch, struct char_data *tch);
int add_char_to_obj(struct char_data *ch, struct obj_data *obj);
int remove_char_from_obj(struct char_data *ch, struct obj_data *obj);
int go_transport_move(struct char_data *ch, int dir);
void go_change_driver(struct char_data *ch, char *argument);


#define PULSE_MOVE 5
#define PULSE_LOCATE 15

const int Reverse[NUM_OF_DIRS] = { 2, 3, 0, 1, 5, 4 };

void stop_all_fighting(struct char_data *ch, int flag)
{
    struct char_data *vict;
    int room = IN_ROOM(ch);


    change_fighting(ch, FALSE);

    for (vict = world[room].people; vict; vict = vict->next_in_room)
        if (FIGHTING(vict) == ch)
            stop_fighting(vict, flag);


    stop_fighting(ch, flag);

}

/* check some death situation */
int check_death_trap(struct char_data *ch)
{
    return (FALSE);

    int i = IN_ROOM(ch);

    if (real_sector(i) == SECT_FLYING) {
        if (!(IS_AFFECTED(ch, AFF_FLY) && GET_POS(ch) == POS_FLYING) && !MOB_FLAGGED(ch, MOB_FLYING)
            && !ON_HORSE_FLY(ch) && !PRF_FLAGGED(ch, PRF_NOHASSLE))
            ch_drop_down(ch);

    } else {
        if (real_sector(i) == SECT_WATER_NOSWIM || real_sector(i) == SECT_UNDERWATER)
            if (!(IS_AFFECTED(ch, AFF_FLY) && GET_POS(ch) == POS_FLYING)
                && !IS_AFFECTED(ch, AFF_WATERWALK) && !MOB_FLAGGED(ch, MOB_FLYING)
                && !MOB_FLAGGED(ch, MOB_SWIMMING) && !GET_MOVE(ch) && !PRF_FLAGGED(ch, PRF_NOHASSLE)
                && !has_boat(ch) && !ON_HORSE_FLY(ch) && !ON_HORSE_SWIM(ch))
                ch_swim_down(ch);
    }

    if (!ch)
        return (TRUE);

    return (FALSE);
}


/* simple function to determine if char can walk on water */
int has_boat(struct char_data *ch)
{
    struct obj_data *obj;
    int i;

    /*
       if (ROOM_IDENTITY(ch->in_room) == DEAD_SEA)
       return (1);
     */

    if (IS_IMMORTAL(ch))
        return (TRUE);

    if (AFF_FLAGGED(ch, AFF_WATERWALK))
        return (TRUE);

    if (AFF_FLAGGED(ch, AFF_WATERBREATH))
        return (TRUE);

    if (AFF_FLAGGED(ch, AFF_FLY) && GET_POS(ch) == POS_FLYING)
        return (TRUE);

    /* non-wearable boats in inventory will do it */
    for (obj = ch->carrying; obj; obj = obj->next_content)
        if (GET_OBJ_TYPE(obj) == ITEM_BOAT && (find_eq_pos(ch, obj, NULL) < 0))
            return (TRUE);

    /* and any boat you're wearing will do it too */
    for (i = 0; i < NUM_WEARS; i++)
        if (GET_EQ(ch, i) && GET_OBJ_TYPE(GET_EQ(ch, i)) == ITEM_BOAT)
            return (TRUE);

    return (FALSE);
}

void make_visible(struct char_data *ch, int affect)
{
    char to_room[MAX_STRING_LENGTH], to_char[MAX_STRING_LENGTH];

    *to_room = *to_char = 0;

    switch (affect) {
        case AFF_HIDE:
            strcpy(to_char, "Вы прекратили прятаться.\r\n");
            strcpy(to_room, "$n прекратил$g прятаться.\r\n");
            break;
        case AFF_CAMOUFLAGE:
            strcpy(to_char, "Вы прекратили маскироваться.");
            strcpy(to_room, "$n прекратил$g маскироваться.");
            break;
    }
    REMOVE_BIT(AFF_FLAGS(ch, affect), affect);
    ch->visible_by.clear();

    if (*to_char)
        send_to_char(to_char, ch);
    if (*to_room)
        act(to_room, FALSE, ch, 0, 0, TO_ROOM);
}


int skip_camouflage(struct char_data *ch, struct char_data *vict)
{
    int percent, prob;

    if (MAY_SEE(vict, ch) &&
        (AFF_FLAGGED(ch, AFF_CAMOUFLAGE) || affected_by_spell(ch, SPELL_CAMOUFLAGE))) {
        if (awake_camouflage(ch)) {     //if (affected_by_spell(ch,SPELL_CAMOUFLAGE))
            //send_to_char("Вы попытались замаскироваться, но Ваша экипировка выдала Вас.\r\n",ch);
            affect_from_char(ch, SPELL_CAMOUFLAGE);
            make_visible(ch, AFF_CAMOUFLAGE);
            SET_BIT(EXTRA_FLAGS(ch, EXTRA_FAILCAMOUFLAGE), EXTRA_FAILCAMOUFLAGE);
        } else if (affected_by_spell(ch, SPELL_CAMOUFLAGE)) {
            percent = number(1, 100);
            std::vector < int >vit;
            std::vector < int >vot;

            //Параметры для атаки
            vit.push_back(GET_REAL_DEX(ch));
            vit.push_back(GET_REAL_WIS(ch));
            //Параметры для защиты
            vot.push_back(GET_REAL_DEX(vict));
            vot.push_back(GET_REAL_CON(vict));
            prob = calc_like_skill(ch, vict, SKILL_CAMOUFLAGE, vit, vot);
            improove_skill(ch, vict, 0, SKILL_CAMOUFLAGE);

            if (PRF_FLAGGED(ch, PRF_CODERINFO))
                send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);
            if (percent > prob) {
                affect_from_char(ch, SPELL_CAMOUFLAGE);
            } else
                return (TRUE);
        }
    }
    return (FALSE);
}


int skip_sneaking(struct char_data *ch, struct char_data *vict)
{
    int percent, prob;

    percent = number(1, 100);
    std::vector < int >vit;
    std::vector < int >vot;

    //Параметры для атаки
    vit.push_back(GET_REAL_DEX(ch));
    vit.push_back(GET_REAL_INT(ch) / 2);
//Параметры для защиты
    vot.push_back(GET_REAL_DEX(vict));
    vot.push_back(GET_REAL_INT(vict) / 2);
    prob = calc_like_skill(ch, vict, SKILL_SNEAK, vit, vot);
    improove_skill(ch, vict, 0, SKILL_SNEAK);

    if (MAY_SEE(vict, ch) && (AFF_FLAGGED(ch, AFF_SNEAK) || affected_by_spell(ch, SPELL_SNEAK))) {
        if (awake_sneak(ch)) {
            affect_from_char(ch, SPELL_SNEAK);
            if (affected_by_spell(ch, SPELL_HIDE))
                affect_from_char(ch, SPELL_HIDE);
            make_visible(ch, AFF_SNEAK);
            SET_BIT(EXTRA_FLAGS(ch, EXTRA_FAILSNEAK), EXTRA_FAILSNEAK);
        } else if (affected_by_spell(ch, SPELL_SNEAK)) {
            if (PRF_FLAGGED(ch, PRF_CODERINFO))
                send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);
            if (percent > prob) {
                affect_from_char(ch, SPELL_SNEAK);
                if (affected_by_spell(ch, SPELL_HIDE))
                    affect_from_char(ch, SPELL_HIDE);
            }
            return (TRUE);
        }
    }

    return (FALSE);
}

/* do_simple_move assumes
 *    1. That there is no master and no followers.
 *    2. That the direction exists.
 *
 *   Returns:
 *   1: If succes.
 *   0: If fail
 */
/*
 * Check for special routines (North is 1 in command list, but 0 here) Note
 * -- only check if following; this avoids 'double spec-proc' bug
 */


int legal_dir(struct char_data *ch, int dir, int need_specials_check, int show_msg)
{
    int need_movement = 0, sp_ghost_fear = find_spell_num(SPELL_GHOST_FEAR);
    struct char_data *tch;
    char buf[MAX_STRING_LENGTH], sub_buf[MAX_STRING_LENGTH];

    //act("$n вот и я",FALSE,ch,0,0,TO_ROOM);
    strcpy(sub_buf, "");
    if (need_specials_check && special(ch, dir + 1, sub_buf))
        return (FALSE);

    /* XXX: GET_DEST check commented out because null rooms 
     * aren't handled properly later in this function */
    if (!CAN_GO(ch, dir)
        /* && (!IS_NPC(ch) || (IS_NPC(ch) && GET_DEST(ch) == NOWHERE && ch->npc()->specials.move_to == NOWHERE)) */
        )
        return (FALSE);

    if (IS_NPC(ch) && affected_by_spell(ch, SPELL_BLADE_BARRIER))
        return (FALSE);

    if (ch->trap_object) {
        if (show_msg) {
            send_to_charf(ch, "Вы не сможете перемещатся пока Вас держит %s.\r\n",
                          GET_OBJ_PNAME(ch->trap_object, 0));
            act("$n дернул$u, но $o помешал$G $m сдвинуться с места.", TRUE, ch, ch->trap_object, 0,
                TO_ROOM);
        }
        return (FALSE);
    }

    if (get_horse_on(ch) && get_horse_on(ch)->trap_object) {
        act("Ваш$G $N не может перемещатся пока $E в $o.", FALSE, ch, get_horse_on(ch)->trap_object,
            get_horse_on(ch), TO_CHAR);
        return (FALSE);
    }

    if (affected_by_spell_real(ch, sp_ghost_fear)) {
        act("Вы так напуганы, что не в силах шевельнуть даже пальцем.", "М", ch);
        return (FALSE);
    }

    if (get_horse_on(ch) && affected_by_spell_real(get_horse_on(ch), sp_ghost_fear)) {
        act("Ваш2(,а,о,и) 2+и застыл2(,а,о,и) на месте от страха.", "Мм", ch, get_horse_on(ch));
        return (FALSE);
    }

    if (!OK_MOVE_W(ch)) {
        send_to_charf(ch, "Вы перегруженны так, что не способны сдвинуться с места.\r\n");
        return (FALSE);
    }

    if (zone_table[world[EXIT(ch, dir)->to_room].zone].noplayer && !IS_MOB(ch) && !IS_GOD(ch)) {
        if (show_msg)
            send_to_char("Великая магическая сила мешает Вам пройти.\r\n", ch);
        return (FALSE);
    }

    if (!EXIT(ch, dir)->timer &&
        ((ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_NOHUMAN) && IS_HUMAN(ch)) ||
         (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_NOORC) && IS_ORC(ch)) ||
         (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_NOGNOME) && IS_GNOME(ch)) ||
         (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_NOELVES) && IS_ELVES(ch)) ||
         (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_NOHALFELVES) && IS_HALFELVES(ch)) ||
         (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_NOBARIAUR) && IS_BARIAUR(ch)) ||
         (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_NOTIEFLING) && IS_TIEFLING(ch)) ||
         (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_NOAASIMAR) && IS_AASIMAR(ch)) ||
         (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_NOPLAYER) && !IS_NPC(ch)))
        ) {
        if (show_msg) {
            act("Особенности вашего телосложения не позволяют Вам туда пройти.", FALSE, ch, 0, 0,
                TO_CHAR);
            sprintf(buf, "$n хотел$g пойти %s, но не смог$q из-за своего телосложения.",
                    DirsTo[dir]);
            act(buf, TRUE, ch, 0, 0, TO_ROOM);
        }
        return (FALSE);
    }



    /* не пускать в ванрумы после пк */
    if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_TUNNEL) && !IS_NPC(ch) && RENTABLE(ch)) {
        if (show_msg)
            send_to_char("В связи с боевыми действиями эвакуация временно прекращена.\r\n", ch);
        return (FALSE);
    }

    /* charmed */
    if (AFF_FLAGGED(ch, AFF_CHARM) && !AFF_FLAGGED(ch, AFF_HORSE) &&
        ch->master && ch->in_room == ch->master->in_room) {
        if (show_msg) {
            send_to_char("Вы не можете покинуть свой идеал.\r\n", ch);
            act("$N попытал$U покинуть Вас.", FALSE, ch->master, 0, ch, TO_CHAR);
        }
        return (FALSE);
    }

    if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_PRIVATE) &&
        (num_pc_in_room(&(world[EXIT(ch, dir)->to_room])) >= 2 || on_horse(ch))) {
        if (show_msg)
            send_to_char("Что-то интимное вершится в этой комнате - третий явно лишний.\r\n", ch);
        return (FALSE);
    }


    /* check NPC's */
    if (IS_NPC(ch)) {
        ///act("$n проверяют мои передвижения",FALSE,ch,0,0,TO_CHAR);
        //if (GET_DEST(ch) != NOWHERE)
        //{
        //act("$n могу идти",FALSE,ch,0,0,TO_ROOM);
        //return (TRUE);
        //}

        if (DOOR_FLAGGED(EXIT(ch, dir), EXIT_CLOSED) && GET_INT(ch) <= 5)
            return (FALSE);
        /*sprintf(buf,"Мой GET_DEST %d",GET_DEST(ch));
           act(buf,FALSE,ch,0,0,TO_ROOM); */


        if (world[EXIT(ch, dir)->to_room].damage && !IS_MOUNT(ch))
            return (FALSE);
        if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_TUNNEL) &&
            (num_pc_in_room(&(world[EXIT(ch, dir)->to_room])) > 0))
            return (FALSE);
        //  if this room or the one we're going to needs a boat, check for one */
        if (!MOB_FLAGGED(ch, MOB_SWIMMING) &&
            !MOB_FLAGGED(ch, MOB_FLYING) &&
            !(AFF_FLAGGED(ch, AFF_FLY) && GET_POS(ch) == POS_FLYING) &&
            !ON_HORSE_FLY(ch) &&
            (real_sector(ch->in_room) == SECT_WATER_NOSWIM ||
             real_sector(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM)) {
            if (!has_boat(ch))
                return (FALSE);
        }
        if (!MOB_FLAGGED(ch, MOB_FLYING) &&
            !(AFF_FLAGGED(ch, AFF_FLY) && GET_POS(ch) == POS_FLYING) &&
            real_sector(EXIT(ch, dir)->to_room) == SECT_FLYING)
            return (FALSE);
        if (MOB_FLAGGED(ch, MOB_ONLYSWIMMING) &&
            !(real_sector(EXIT(ch, dir)->to_room) == SECT_WATER_SWIM ||
              real_sector(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM ||
              real_sector(EXIT(ch, dir)->to_room) == SECT_UNDERWATER))
            return (FALSE);
        if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_NOMOB) &&
            !IS_MOUNT(ch) && !MOB_FLAGGED(ch, MOB_WALKER) && !AFF_FLAGGED(ch, AFF_CHARM)
            && !AFF_FLAGGED(ch, AFF_HELPER))
            return (FALSE);

        if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_DEATH) && !IS_MOUNT(ch))
            return (FALSE);

        if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_FLYDEATH) && !IS_AFFECTED(ch, AFF_FLY))
            return (FALSE);


        if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_GODROOM))
            return (FALSE);

        if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_NOHORSE) && IS_MOUNT(ch))
            return (FALSE);

        if (IN_ROOM(ch) == NOWHERE)
            return (FALSE);
        if (IS_AFFECTED(ch, AFF_CHARM)) {
            need_movement =
                (movement_loss[real_sector(ch->in_room)] +
                 movement_loss[real_sector(EXIT(ch, dir)->to_room)]) / 2;
            if (IS_FLY(ch))
                need_movement = 1;
            else
                need_movement =
                    (movement_loss[real_sector(ch->in_room)] +
                     movement_loss[real_sector(EXIT(ch, dir)->to_room)]) / 2;

            need_movement = MAX(1, need_movement);

            if (GET_MOVE(ch) < need_movement) {
                send_to_charf(ch, "Монстр слишком устал чтобы ходить.\r\n");
                return (FALSE);
            }

        } else if (MOB_FLAGGED(ch, MOB_CLONE)) {
            need_movement =
                (movement_loss[real_sector(ch->in_room)] +
                 movement_loss[real_sector(EXIT(ch, dir)->to_room)]) / 8;
            need_movement = MAX(1, need_movement);

            if (GET_MOVE(ch) < need_movement) {
                if (miracle_action(ch, MIRC_MOVE)) {

                } else {
                    send_to_charf(ch, "Вы слишком устали.\r\n");
                    return (FALSE);
                }
            }
        }
        //проверка на отторжение
        if (check_toroom_repulsion(ch, EXIT(ch, dir)->to_room)) {
            send_to_charf(ch, "Что-то невидимое мешает Вам туда пройти.\r\n");
            return (FALSE);
        }
    } else
        if (!MOB_FLAGGED(ch, MOB_FLYING) &&
            !(AFF_FLAGGED(ch, AFF_FLY) && GET_POS(ch) == POS_FLYING) &&
            !ON_HORSE_FLY(ch) && SECT(EXIT(ch, dir)->to_room) == SECT_FLYING && dir == UP) {
        if (show_msg)
            send_to_char("Вы должны летать, чтобы попасть туда.\r\n", ch);
        return (FALSE);
    } else if (OFF_HORSE_FLY(ch) && SECT(EXIT(ch, dir)->to_room) == SECT_FLYING) {
        if (show_msg)
            act("Ваш $N должен уметь летать.", FALSE, ch, 0, get_horse_on(ch), TO_CHAR);
        return (FALSE);
    } else
        if (real_sector(EXIT(ch, dir)->to_room) == SECT_WATER_NOSWIM &&
            (OFF_HORSE_SWIM(ch) || OFF_HORSE_FLY(ch))) {
        if (show_msg)
            act("Ваш $N должен уметь летать или плавать.", FALSE, ch, 0, get_horse_on(ch), TO_CHAR);
        return (FALSE);
    } else if (real_sector(EXIT(ch, dir)->to_room) == SECT_UNDERWATER && OFF_HORSE_SWIM(ch)) {
        if (show_msg)
            act("Ваш $N должен уметь плавать.", FALSE, ch, 0, get_horse_on(ch), TO_CHAR);
        return (FALSE);
    } else {
        // move points needed is avg. move loss for src and destination sect type
        if (!EXIT(ch, dir)->timer) {
            if (IS_FLY(ch))
                need_movement = 1;
            else
                need_movement =
                    (movement_loss[real_sector(ch->in_room)] +
                     movement_loss[real_sector(EXIT(ch, dir)->to_room)]) / 2;

            need_movement = MAX(1, need_movement);
        } else
            need_movement = 1;

        if (IS_IMMORTAL(ch))
            need_movement = 0;

        if (affected_by_spell(ch, SPELL_CAMOUFLAGE))
            need_movement += CAMOUFLAGE_MOVES;

        if (affected_by_spell(ch, SPELL_SNEAK))
            need_movement += SNEAK_MOVES;

        if (IS_AFFECTED(ch, AFF_FASTER) && !IS_IMMORTAL(ch))
            need_movement = MAX(1, need_movement / 2);

        if (GET_MOVE(ch) < need_movement) {
            if (number(0, 3) == 2 && miracle_action(ch, MIRC_MOVE)) {

            } else {
                if (need_specials_check && ch->master) {
                    if (show_msg)
                        send_to_char("Вы слишком устали, чтобы следовать туда.\r\n", ch);
                } else {
                    if (show_msg)
                        send_to_char("Вы слишком устали.\r\n", ch);
                }
                return (FALSE);
            }
        }

        if (ROOM_FLAGGED(ch->in_room, ROOM_ATRIUM)) {
            // фракции
        }

        if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_TUNNEL) &&
            (num_pc_in_room(&(world[EXIT(ch, dir)->to_room])) > 0 || on_horse(ch))) {
            if (show_msg)
                send_to_char("Слишком мало места.\r\n", ch);
            return (FALSE);
        }

        if (on_horse(ch) && !legal_dir(get_horse_on(ch), dir, need_specials_check, FALSE)) {
            if (show_msg)
                act("Ваш$G $N отказывается туда идти.", FALSE, ch, 0, get_horse_on(ch), TO_CHAR);
            return (FALSE);
        }

        if (on_horse(ch) && GET_MOVE(get_horse_on(ch)) <= 0) {
            if (show_msg)
                act("Ваш$G $N загнан$G настолько, что не может нести Вас на себе.", FALSE, ch, 0,
                    get_horse(ch), TO_CHAR);
            return (FALSE);
        }

        if (ROOM_FLAGGED(EXIT(ch, dir)->to_room, ROOM_GODROOM) && !IS_GOD(ch)) {
            if (show_msg)
                send_to_char("Вы не столь Божественны, как Вам кажется!\r\n", ch);
            return (FALSE);
        }


        /*if (EXIT(ch, dir)->to_room != NOWHERE && !enter_wtrigger(&world[EXIT(ch, dir)->to_room], ch, dir))
           return (FALSE); */

        if (IN_ROOM(ch) == NOWHERE)
            return (FALSE);

        if (IN_ROOM(ch) != NOWHERE) {
            for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
                if (!IS_NPC(tch))
                    continue;
                if (NPC_FLAGGED(tch, 1 << dir) &&
                    AWAKE(tch) &&
                    GET_POS(tch) > POS_SLEEPING &&
                    CAN_SEE(tch, ch) && !AFF_FLAGGED(tch, AFF_CHARM)) {
                    if (show_msg)
                        act("$N преградил$G Вам путь.", FALSE, ch, 0, tch, TO_CHAR);
                    return (FALSE);
                }
            }
        }
        //проверка на отторжение
        /*if (check_toroom_repulsion(ch,EXIT(ch, dir)->to_room))
           {
           send_to_charf(ch,"Что-то невидимое мешает Вам туда пройти.\r\n");
           return(FALSE);
           } */
    }
    return (need_movement ? need_movement : 1);
}

#define MOB_AGGR_TO_ALIGN (MOB_AGGR_EVIL | MOB_AGGR_NEUTRAL | MOB_AGGR_GOOD)

#define MAX_DRUNK_SONG 6
const char *drunk_songs[MAX_DRUNK_SONG] = { "\"Шумел камыш, и-к-к..., деревья гнулися\"",
    "\"Куда ты, тропинка, меня завела\"",
    "\"Бабам, пара пабабам\"",
    "\"А мне любое море по колено\"",
    "\"Не жди меня мама, хорошего сына\"",
    "\"Бываааали дниии, весеееелыя\"",
};

#define MAX_DRUNK_VOICE 5
const char *drunk_voice[MAX_DRUNK_VOICE] = { " - затянул$g $n",
    " - запел$g $n.",
    " - прохрипел$g $n.",
    " - зычно заревел$g $n.",
    " - разухабисто протянул$g $n.",
};



int do_simple_move(struct char_data *ch, int dir, int need_specials_check, bool show)
{
    struct track_data *track;
    room_rnum was_in, go_to;
    int need_movement, i, ndir = -1, nm, invis = 0, use_horse = 0, is_horse = 0, direction = 0;
    int IsFlee = dir & 0x80, mob_rnum = -1, odir, train = 0;
    int IsMoved = dir & 0x40;
    int IsFly = FALSE, IsWater = FALSE;
    struct char_data *vict, *horse = NULL;
    struct mess_p_data *k = NULL;
    char buf_p[256];
    char buf[MAX_STRING_LENGTH], buf1[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH],
        sub_buf[MAX_STRING_LENGTH];
    bool found = 0;


    if (IsMoved)
        odir = dir = dir & 0xbf;
    else
        odir = dir = dir & 0x7f;

    stop_events(ch, STOP_ALL);

    if (!(need_movement = legal_dir(ch, dir, need_specials_check, TRUE)))
        return (FALSE);


    /* Mortally drunked - it is loss direction */
    if (!IS_NPC(ch)
        && GET_COND(ch, DRUNK) >= CHAR_MORTALLY_DRUNKED && !on_horse(ch)
        && GET_COND(ch, DRUNK) >= number(CHAR_DRUNKED, 50))
        for (i = 0; i < NUM_OF_DIRS && ndir < 0; i++) {
            ndir = number(0, 5);
            if (!EXIT(ch, ndir) ||
                (EXIT(ch, ndir)->to_room == NOWHERE &&
                 !EXIT(ch, ndir)->timer) ||
                DOOR_FLAGGED(EXIT(ch, ndir), EXIT_CLOSED) ||
                !(nm = legal_dir(ch, ndir, need_specials_check, TRUE)))
                ndir = -1;
            else {
                if (dir != ndir) {
                    strcpy(sub_buf, "икнуть");
                    do_social(ch, sub_buf);
                }
                if (!FIGHTING(ch) && number(10, 24) < GET_COND(ch, DRUNK)) {
                    affect_from_char(ch, SPELL_SNEAK);
                    affect_from_char(ch, SPELL_CAMOUFLAGE);
                };
                dir = ndir;
                need_movement = nm;
            }
        }

    /* Now we know we're allow to go into the room. */


    if (!IS_GOD(ch)
        &&
        ((IS_NPC(ch) && (IS_AFFECTED(ch, AFF_CHARM) || MOB_FLAGGED(ch, MOB_CLONE) || IS_HORSE(ch)))
         || !IS_NPC(ch)))
        GET_MOVE(ch) -= need_movement;

    was_in = ch->in_room;

    if ((ROOM_FLAGGED(was_in, ROOM_FCIRCLE) ||
         (ROOM_FLAGGED(was_in, ROOM_CIRCLE) && dir < UP)) && !IS_AFFECTED(ch, AFF_ORENT)) {
        int vdir;

        do {
            vdir = number(0, 5);
        } while (!legal_dir(ch, vdir, TRUE, FALSE) || ch->vdir[dir]);
        ch->vdir[vdir] = TRUE;
        go_to = world[was_in].dir_option[vdir]->to_room;
    } else
        go_to = world[was_in].dir_option[dir]->to_room;

    go_to = world[was_in].dir_option[dir]->to_room;
    direction = dir + 1;
    use_horse = AFF_FLAGGED(ch, AFF_HORSE) && has_horse(ch, FALSE) &&
        (IN_ROOM(get_horse_on(ch)) == was_in || IN_ROOM(get_horse_on(ch)) == go_to);
    is_horse = IS_HORSE(ch) && (ch->master) && on_horse(ch->master)
        && !AFF_FLAGGED(ch->master, AFF_INVISIBLE) && (IN_ROOM(ch->master) == was_in
                                                       || IN_ROOM(ch->master) == go_to);
    if (on_horse(ch) /* || has_horse(ch, TRUE) */ )
        horse = get_horse_on(ch);

    if (affected_by_spell(ch, SPELL_BLADE_BARRIER)) {
        affect_from_char(ch, SPELL_BLADE_BARRIER);
        REMOVE_BIT(AFF_FLAGS(ch, AFF_BLADES), AFF_BLADES);
    }

    if (ch->is_transpt)
        return (go_transport_move(ch, dir));

    /* НАЧАЛО ОБРАБОТКИ ПОРТАЛА */
    if (world[was_in].dir_option[dir]->timer) {
        act(world[was_in].dir_option[dir]->mess_char_enter, "М", ch);
        act(world[was_in].dir_option[dir]->mess_room_enter, "Км", ch);

        go_to = world[was_in].dir_option[dir]->active_room;

        /* перемещаем чувака */
        char_from_room(ch);
        char_to_room(ch, go_to);
        if (horse) {
            GET_HORSESTATE(horse) -= 1;
            char_from_room(horse);
            char_to_room(horse, go_to);
        }
        /* */

        sprintf(buf, "$n появил$u из портала.");
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        if (GET_OBJ_LOCATE(ch) || GET_CHAR_LOCATE(ch))
            WAIT_STATE(ch, PULSE_LOCATE);
        else
            WAIT_STATE(ch, PULSE_MOVE);

        /*if (ch->desc != NULL && show)
           look_at_room(ch, 0); */

        if (check_death_trap(ch)) {
            if (horse)
                extract_char(horse, FALSE);
            return (FALSE);
        }

    }
    /* КОНЕЦ ОБРАБОТКИ ПОРТАЛОВ */

    /* Проверка на перехват сообщений */
    if (IN_ROOM(ch) != NOWHERE)
        for (k = world[IN_ROOM(ch)].mess_data; k; k = k->next)
            if (k->command == dir + 1) {
                found = TRUE;
                break;
            }


    /* char income, go mobs action */
    vict = NULL;
    train = 0;
    for (vict = world[go_to].people; vict; vict = vict->next_in_room) {
        if (!IS_NPC(vict))
            continue;

        if (MOB_FLAGGED(vict, MOB_XENO) || MOB_FLAGGED(vict, MOB_AGGRESSIVE) ||
            (MOB_FLAGGED(vict, MOB_AGGRGOOD) && IS_GOODS(ch)) ||
            (MOB_FLAGGED(vict, MOB_AGGREVIL) && IS_EVILS(ch)) ||
            (MOB_FLAGGED(vict, MOB_AGGRNEUTRAL) && IS_NEUTRALS(ch)))

            train = TRUE;
        break;
    }

    if (AFF_FLAGGED(ch, AFF_CAMOUFLAGE) && !IsFlee && train && vict) {
        int prob;
        std::vector < int >vit;
        std::vector < int >vot;

        //Параметры для атаки
        vit.push_back(GET_REAL_DEX(ch));
        vit.push_back(GET_REAL_INT(ch) / 2);
        //Параметры для защиты
        vot.push_back(GET_REAL_DEX(vict));
        vot.push_back(GET_REAL_INT(vict) / 2);

        prob = calc_like_skill(ch, vict, SKILL_CAMOUFLAGE, vit, vot);

        if (number(1, 100) <= prob)
            invis = 2;
        else {
            affect_from_char(ch, SPELL_CAMOUFLAGE);
            invis = 0;
        }
    }
    //Проверка на то видят наш уход там откуда мы уходим
    if (AFF_FLAGGED(ch, AFF_SNEAK) && !IsFlee) {
        if (affected_by_spell(ch, SPELL_SNEAK))
            invis = 1;
        else
            invis = 0;
    }

    if (!invis && !is_horse) {
        //log("%s move_type %d=%d",GET_NAME(ch),GET_MOVE_TYPE(ch),MOVE_RUN);
        if (IS_NPC(ch) && (NPC_FLAGGED(ch, NPC_MOVERUN) || GET_MOVE_TYPE(ch) == MOVE_RUN))
            strcpy(buf1, "убежал1(,а,о,и)");
        else if (AFF_FLAGGED(ch, AFF_FLY) ||
                 AFF_FLAGGED(ch, AFF_LEVIT) ||
                 (IS_NPC(ch) && (NPC_FLAGGED(ch, NPC_MOVEFLY) || GET_MOVE_TYPE(ch) == MOVE_FLY))) {
            IsFly = TRUE;
            strcpy(buf1, "улетел1(,а,о,и)");
        } else if (IS_NPC(ch) && (NPC_FLAGGED(ch, NPC_MOVESWIM) || (GET_MOVE_TYPE(ch) == MOVE_SWIM)) && (       //real_sector(was_in) == SECT_WATER_SWIM   ||
                                                                                                            (real_sector(was_in) == SECT_WATER_NOSWIM) || (real_sector(go_to) == SECT_WATER_NOSWIM) || (real_sector(was_in) == SECT_UNDERWATER))) {
            strcpy(buf1, "уплыл1(,а,о,и)");
            IsWater = TRUE;
        } else if (             //real_sector(was_in) == SECT_WATER_SWIM   ||
                      real_sector(was_in) == SECT_WATER_NOSWIM ||
                      real_sector(go_to) == SECT_WATER_NOSWIM ||
                      real_sector(was_in) == SECT_UNDERWATER) {
            strcpy(buf1, "уплыл1(,а,о,и)");
            IsWater = TRUE;
        } else if (IS_NPC(ch) && (NPC_FLAGGED(ch, NPC_MOVEJUMP) || GET_MOVE_TYPE(ch) == MOVE_JUMP))
            strcpy(buf1, "упрыгал1(,а,о,и)");
        else if (IS_NPC(ch) && (NPC_FLAGGED(ch, NPC_MOVECREEP) || GET_MOVE_TYPE(ch) == MOVE_CROUCH))
            strcpy(buf1, "уполз1(,ла,ло,ли)");
        else if (IS_NPC(ch) && GET_MOVE_TYPE(ch) == MOVE_GALOP)
            strcpy(buf1, "ускакал1(,а,о,и)");
        else if (IS_NPC(ch) && GET_MOVE_TYPE(ch) == MOVE_PKAT)
            strcpy(buf1, "укатил1(ся,ась,ось,ись)");
        else if (IS_NPC(ch) && GET_MOVE_TYPE(ch) == MOVE_PKOV)
            strcpy(buf1, "уковылял1(,а,о,и)");
        else if (use_horse) {
            if (AFF_FLAGGED(horse, AFF_FLY) ||
                AFF_FLAGGED(horse, AFF_LEVIT) ||
                (IS_NPC(horse) &&
                 (NPC_FLAGGED(horse, NPC_MOVEFLY) ||
                  GET_MOVE_TYPE(horse) == MOVE_FLY ||
                  MOB_FLAGGED(horse, MOB_FLYING) || MOB_FLAGGED(horse, MOB_LEVITING)))) {
                IsFly = TRUE;
                strcpy(buf1, "улетел1(,а,о,и)");
            } else if (IS_NPC(horse) && GET_MOVE_TYPE(horse) == MOVE_GALOP)
                strcpy(buf1, "ускакал1(,а,о,и)");
            else
                strcpy(buf1, "уехал1(,а,о,и)");
        } else if (IsFlee)
            strcpy(buf1, "убежал1(,а,о,и)");
        else
            strcpy(buf1, "уш1(ел,ла,ло,ли)");

        if (MOB_FLAGGED(ch, MOB_CLONE))
            IsFly = 1;

        /* А вот и повреждения на выход */
        if (EXIT(ch, dir)->shance && !check_tfind_char(ch, EXIT(ch, dir))
            && !DOOR_FLAGGED(EXIT(ch, dir), EXIT_CLOSE)) {
            exit_trap_active(ch, dir, FALSE);
            /* Жив ли он еще */
            if (!ch || (ch && (GET_POS(ch) < POS_STANDING))) {
                log("И креш");
                return (FALSE);
            }
        }

        if (IsMoved) {
            //
        } else if (found) {
            if (!invis) {
                if (k->mess_to_char) {
                    sprintf(buf_p, k->mess_to_char, buf1);
                    act(buf_p, "М", ch);
                }
                if (k->mess_to_room) {
                    sprintf(buf_p, k->mess_to_room, buf1);
                    act(buf_p, "Км", ch);
                }
            }
            go_script(k->script, ch);
            if (k->stoping)
                return (FALSE);
        }

        if (!found || (found && !k->mess_to_room)) {
            if (use_horse) {
                sprintf(buf2, "1и верхом на 2п %s %s.", buf1, DirsTo[dir]);
                act(buf2, "Кмм", ch, horse);
            } else if (IsFlee) {
                sprintf(buf2, "1и, спасаясь бегством, %s %s.", buf1, DirsTo[dir]);
                act(buf2, "Км", ch);
            } else if (IS_NPC(ch) && GET_MOVE_STR(ch) != NULL) {
                if (*GET_MOVE_STR(ch) == ',')
                    sprintf(buf2, "1и%s %s %s.", GET_MOVE_STR(ch), buf1, DirsTo[dir]);
                else
                    sprintf(buf2, "1и %s %s %s.", GET_MOVE_STR(ch), buf1, DirsTo[dir]);
                act(buf2, "Км", ch);
            } else {
                sprintf(buf2, "1и %s %s.", buf1, DirsTo[dir]);
                act(buf2, "Км", ch);
            }
            if (IsWater)
                send_to_charf(ch, "Вы поплыли на %s.\r\n", DirsTo[dir]);
        }
    } else if (invis == 1 && !is_horse) {
        struct char_data *tch;

        for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
            if (ch != tch && check_sneak(ch, tch)) {
                act("1+и прокрал1(ся,ась,ось,ись) %1.", "Кмт", ch, DirsTo[dir]);
            }
        }
    }

    if (invis == 2 && !is_horse) {
        act("1+и тихо удалился отсюда.", "Км", ch);
    }


    /* Если сбежали, и по противнику никто не бьет, то убираем с него аттаку */
    if (IsFlee)
        stop_all_fighting(ch, FALSE);


    char_from_room(ch);
    char_to_room(ch, go_to);
    if (GET_OBJ_LOCATE(ch) || GET_CHAR_LOCATE(ch))
        WAIT_STATE(ch, PULSE_LOCATE);
    //else
        //WAIT_STATE(ch, PULSE_MOVE);

    if (horse) {
        int move_horse = IS_FLY(horse) ? 1 :
            MAX(1,
                (movement_loss[real_sector(IN_ROOM(horse))] +
                 movement_loss[real_sector(EXIT(horse, dir)->to_room)]) / 4);

        GET_MOVE(horse) -= move_horse;
        //GET_HORSESTATE(horse) -= 1;
        char_from_room(horse);
        char_to_room(horse, go_to);
    }

    invis = 0;

    /* Проверка и прокачка умения "красться" по локации куда мы пришли
       результат 1 прошли незаметно, 0 нас заметили
     */
    IsFly = FALSE;

    if (affected_by_spell(ch, SPELL_SNEAK))
        invis = 1;
    else
        invis = 0;

    if (!invis && !is_horse) {
        if (IS_NPC(ch) && (NPC_FLAGGED(ch, NPC_MOVERUN) || GET_MOVE_TYPE(ch) == MOVE_RUN))
            strcpy(buf1, "прибежал1(,а,о,и)");
        else if (GET_POS(ch) == POS_FLYING || (IS_NPC(ch) && GET_MOVE_TYPE(ch) == MOVE_FLY)) {
            IsFly = TRUE;
            strcpy(buf1, "прилетел1(,а,о,и)");
        } else
            if (IS_NPC(ch) && (NPC_FLAGGED(ch, NPC_MOVESWIM) ||
                               GET_MOVE_TYPE(ch) == MOVE_SWIM) &&
                (real_sector(go_to) == SECT_WATER_SWIM ||
                 real_sector(go_to) == SECT_WATER_NOSWIM || real_sector(go_to) == SECT_UNDERWATER))
            strcpy(buf1, "приплыл1(,а,о,и)");
        else if (real_sector(go_to) == SECT_WATER_SWIM ||
                 real_sector(go_to) == SECT_WATER_NOSWIM || real_sector(go_to) == SECT_UNDERWATER)
            strcpy(buf1, "приплыл1(,а,о,и)");
        else if (IS_NPC(ch) && (NPC_FLAGGED(ch, NPC_MOVEJUMP) || GET_MOVE_TYPE(ch) == MOVE_JUMP))
            strcpy(buf1, "припрыгал1(,а,о,и)");
        else if (IS_NPC(ch) && (NPC_FLAGGED(ch, NPC_MOVECREEP) || GET_MOVE_TYPE(ch) == MOVE_CROUCH))
            strcpy(buf1, "приполз1(,ла,ло,ли)");
        else if (IS_NPC(ch) && GET_MOVE_TYPE(ch) == MOVE_GALOP)
            strcpy(buf1, "прискакал1(,а,о,и)");
        else if (IS_NPC(ch) && GET_MOVE_TYPE(ch) == MOVE_PKAT)
            strcpy(buf1, "прикатил1(ся,ась,ось,ись)");
        else if (IS_NPC(ch) && GET_MOVE_TYPE(ch) == MOVE_PKOV)
            strcpy(buf1, "приковылял1(,а,о,и)");
        else if (use_horse) {
            if (AFF_FLAGGED(horse, AFF_FLY) ||
                AFF_FLAGGED(horse, AFF_LEVIT) ||
                (IS_NPC(horse) &&
                 (NPC_FLAGGED(horse, NPC_MOVEFLY) ||
                  GET_MOVE_TYPE(horse) == MOVE_FLY ||
                  MOB_FLAGGED(horse, MOB_FLYING) || MOB_FLAGGED(horse, MOB_LEVITING)
                 ))) {
                IsFly = TRUE;
                strcpy(buf1, "прилетел1(,а,о,и)");
            } else if (IS_NPC(horse) && GET_MOVE_TYPE(horse) == MOVE_GALOP)
                strcpy(buf1, "прискакал1(,а,о,и)");
            else
                strcpy(buf1, "приехал1(,а,о,и)");
        } else if (IsFlee)
            strcpy(buf1, "прибежал1(,а,о,и)");
        else
            strcpy(buf1, "приш1(ел,ла,ло,ли)");

        if (MOB_FLAGGED(ch, MOB_CLONE))
            IsFly = 1;


        if (IsMoved) {
            act("Кувыркаясь, 1и прилетел1(,а,о,и) %1", "Кмт", ch, DirsFrom[dir]);
        } else if (found) {
            if (!invis) {
                if (k->mess_to_vict) {
                    sprintf(buf_p, k->mess_to_char, buf1);
                    act(buf_p, "М", ch);
                }
                if (k->mess_to_other) {
                    sprintf(buf_p, k->mess_to_other, buf1);
                    act(buf_p, "Км", ch);
                }
            }
        }
        if (!found || (found && !k->mess_to_other)) {
            if (use_horse) {
                sprintf(buf2, "1и верхом на 2п %s %s.", buf1, DirsFrom[dir]);
                act(buf2, "Кмм", ch, horse);
            } else if (IsFlee) {
                sprintf(buf2, "1и, спасаясь бегством, %s %s.", buf1, DirsFrom[dir]);
                act(buf2, "Км", ch);
            } else if (IS_NPC(ch) && GET_MOVE_STR(ch) != NULL) {
                if (*GET_MOVE_STR(ch) == ',')
                    sprintf(buf2, "1и%s %s %s.", GET_MOVE_STR(ch), buf1, DirsFrom[dir]);
                else
                    sprintf(buf2, "1и %s %s %s.", GET_MOVE_STR(ch), buf1, DirsFrom[dir]);
                act(buf2, "Км", ch);
            } else {
                sprintf(buf2, "1и %s %s.", buf1, DirsFrom[dir]);
                act(buf2, "Км", ch);
            }
        }
    } else if (invis == 1 && !is_horse) {
        struct char_data *tch;

        train_sneak(ch);
        for (tch = world[ch->in_room].people; tch; tch = tch->next_in_room) {
            if (ch != tch && check_sneak(ch, tch)) {
                act("1+и крадучись приш1(ел,ла,ло,ли) %1.", "мМт", ch, tch, DirsFrom[dir]);
            }
        }
    }

    if (invis == 2 && !is_horse) {
        act("1+и тихо подкрался сюда.", "Км", ch);
    }

    if (IS_NPC(ch)) {
        check_ranger_trap(ch);
        check_necro_spare(ch);
    }

    if (ch->desc != NULL) {
        if (GET_OBJ_LOCATE(ch) && ch->locate_dirs && !IS_BITS(ch->locate_dirs, dir)) {
            send_to_charf(ch, "Путеводная нить плавно рассеялась в воздухе.\r\n");
            GET_OBJ_LOCATE(ch) = 0;
            ch->locate_dirs = 0;
        }

        if (GET_OBJ_LOCATE(ch) && (--ch->char_specials.locate_step <= 0)) {
            ch->char_specials.locate_step = 0;
            send_to_charf(ch, "Путеводная нить плавно рассеялась в воздухе.\r\n");
            GET_OBJ_LOCATE(ch) = 0;
        }
        if (GET_CHAR_LOCATE(ch) && ch->locate_dirs && !IS_BITS(ch->locate_dirs, dir)) {
            send_to_charf(ch, "Путеводная нить плавно рассеялась в воздухе.\r\n");
            GET_CHAR_LOCATE(ch) = 0;
            ch->locate_dirs = 0;
        }
        if (GET_CHAR_LOCATE(ch) && (--ch->char_specials.locate_step <= 0)) {
            ch->char_specials.locate_step = 0;
            send_to_charf(ch, "Путеводная нить плавно рассеялась в воздухе.\r\n");
            GET_CHAR_LOCATE(ch) = 0;
        }

        ch->locate_dirs = 0;

        if (show)
            look_at_room(ch, 0);

        check_enter_trap(ch, Reverse[dir]);
        check_ranger_trap(ch);
        check_necro_spare(ch);
    }
    if (check_death_trap(ch)) {
        if (horse)
            extract_char(horse, FALSE);
        return (FALSE);
    }


    /*  char_from_room(ch);
       char_to_room(ch, was_in);
       if (horse)
       {char_from_room(horse);
       char_to_room(horse,was_in);
       }
       look_at_room(ch, 0);
       check_enter_trap(ch,Reverse[dir]);
       check_ranger_trap(ch);
       check_necro_spare(ch); */

    /* add track info */
    if (!AFF_FLAGGED(ch, AFF_NOTRACK) && !IsFly &&
        (!IS_NPC(ch) || (mob_rnum = GET_MOB_RNUM(ch)) >= 0)
        ) {
        for (track = world[go_to].track; track; track = track->next)
            if ((IS_NPC(ch) && IS_SET(track->track_info, TRACK_NPC) && track->who == mob_rnum) ||
                (!IS_NPC(ch) && !IS_SET(track->track_info, TRACK_NPC) && track->who == GET_ID(ch)))
                break;

        if (!track && !ROOM_FLAGGED(go_to, ROOM_NOTRACK)) {
            CREATE(track, struct track_data, 1);

            track->track_info = IS_NPC(ch) ? TRACK_NPC : 0;
            track->who = IS_NPC(ch) ? mob_rnum : GET_ID(ch);
            track->next = world[go_to].track;
            world[go_to].track = track;
        }

        if (track) {
            SET_BIT(track->time_income[Reverse[dir]], 1);
            REMOVE_BIT(track->track_info, TRACK_HIDE);
        }

        for (track = world[was_in].track; track; track = track->next)
            if ((IS_NPC(ch) && IS_SET(track->track_info, TRACK_NPC) && track->who == mob_rnum) ||
                (!IS_NPC(ch) && !IS_SET(track->track_info, TRACK_NPC) && track->who == GET_ID(ch)))
                break;

        if (!track && !ROOM_FLAGGED(was_in, ROOM_NOTRACK)) {
            CREATE(track, struct track_data, 1);

            track->track_info = IS_NPC(ch) ? TRACK_NPC : 0;
            track->who = IS_NPC(ch) ? mob_rnum : GET_ID(ch);
            track->next = world[was_in].track;
            world[was_in].track = track;
        }
        if (track) {
            SET_BIT(track->time_outgone[dir], 1);
            REMOVE_BIT(track->track_info, TRACK_HIDE);
        }
    }

    if (!IS_NPC(ch) && *HUNT_NAME(ch)) {
        _do_track(ch, HUNT_NAME(ch));
        if (IS_BITS(ch->track_dirs, dir))
            improove_skill(ch, 0, 0, SKILL_TRACK);
    }

    ch->track_dirs = 0;

    return (direction);
}

/*
 * Invoke Fenia triggers for movement.
 * onEntry,postEntry:
 *      Called for each party member immediately after they entered the room.
 * onGreet,postGreet:
 *      Called on every mobile in the room, for each party member immediately after their onEntry.
 */
static void mprog_greet(struct char_data *rch, struct char_data *walker, const char *from_dirname)
{
    FENIA_VOID_CALL(rch, "Greet", "Cs", walker, from_dirname);
    FENIA_PROTO_VOID_CALL(rch->npc(), "Greet", "CCs", rch, walker, from_dirname);
}

static void mprog_entry(struct char_data *walker)
{
    FENIA_VOID_CALL(walker, "Entry", "");
    FENIA_PROTO_VOID_CALL(walker->npc(), "Entry", "");
}

// Stop triggers if walker is dead or transferred somewhere else.
static bool still_here(struct char_data *walker, room_rnum was_in)
{
    if (!walker || GET_POS(walker) <= POS_STUNNED)
        return false;
    if (walker->in_room != was_in)
        return false;    
    return true;
}

static void prog_movement(struct char_data *walker, int to_dir)
{
    struct char_data *rch, *rch_next;
    room_rnum was_in = walker->in_room;
    const char *from_dirname = dirs[rev_dir[to_dir]];
        
    mprog_entry(walker);
    if (!still_here(walker, was_in))
        return;
                
    for (rch = world[IN_ROOM(walker)].people; rch; rch = rch_next) {
        rch_next = rch->next_in_room;
        mprog_greet(rch, walker, from_dirname);
        if (!still_here(walker, was_in))
            return;
    }
}

int perform_move(struct char_data *ch, int dir, int need_specials_check, int checkmob)
{
    room_rnum was_in;
    struct follow_type *k, *next;

    if (ch == NULL || dir < 0 || dir >= NUM_OF_DIRS || FIGHTING(ch))
        return (0);
    else if (!EXIT(ch, dir) || (EXIT(ch, dir)->to_room == NOWHERE && !EXIT(ch, dir)->timer))
        send_to_charf(ch, "Вы не сможете туда пройти.\r\n", dir);
    else if (DOOR_FLAGGED(EXIT(ch, dir), EXIT_CLOSED)) {
        if (EXIT(ch, dir)->exit_name) {
            if (EXIT(ch, dir)->sex == SEX_POLY)
                send_to_charf(ch, "Проход закрывают %s.\r\n",
                              get_name_pad(EXIT(ch, dir)->exit_name, PAD_IMN, PAD_OBJECT));
            else
                send_to_charf(ch, "Проход закрывает %s.\r\n",
                              get_name_pad(EXIT(ch, dir)->exit_name, PAD_IMN, PAD_OBJECT));
        } else
            send_to_charf(ch,
                          "Закрыто. Билдер поленился добавить название двери. Сообщите об этом имморталам.\r\n");
    } else {
        if (affected_by_spell(ch, SPELL_HIDE))
            affect_from_char(ch, SPELL_HIDE);

        if (!ch->followers) {
            if (!do_simple_move(ch, dir, need_specials_check, TRUE))
                return (FALSE);
        } else {
            was_in = ch->in_room;
            // When leader mortally drunked - he change direction
            // So returned value set to FALSE or DIR + 1
            if (!(dir = do_simple_move(ch, dir, need_specials_check, FALSE)))
                return (FALSE);
            dir--;

            for (k = ch->followers; k && k->follower->master; k = next) {
                next = k->next;
                if (k->follower->in_room == was_in &&
                    !FIGHTING(k->follower) &&
                    !(IS_NPC(k->follower) && k->follower->npc()->specials.transpt) &&
                    HERE(k->follower) &&
                    !GET_MOB_HOLD(k->follower) &&
                    AWAKE(k->follower) &&
                    (IS_NPC(k->follower) ||
                     (!PLR_FLAGGED(k->follower, PLR_MAILING) &&
                      !PLR_FLAGGED(k->follower, PLR_SCRIPTING) &&
                      !PLR_FLAGGED(k->follower, PLR_WRITING)
                     )
                    ) && (!IS_MOUNT(k->follower) || !AFF_FLAGGED(k->follower, AFF_TETHERED)
                    )
                    ) {
                    if (GET_POS(k->follower) < POS_STANDING) {
                        if (IS_NPC(k->follower) &&
                            IS_NPC(k->follower->master) && GET_POS(k->follower) > POS_SLEEPING) {
                            act("$n поднял$u.", FALSE, k->follower, 0, 0, TO_ROOM);
                            GET_POS(k->follower) = POS_STANDING;
                        } else
                            continue;
                    }

                    if (affected_by_spell(k->follower, SPELL_BLADE_BARRIER))
                        continue;

                    act("Вы последовали за $N4.", FALSE, k->follower, 0, ch, TO_CHAR);
                    perform_move(k->follower, dir, 1, FALSE);

                }
            }
            if (ch->desc != NULL)
                look_at_room(ch, 0);
            event_mob(ch, NULL, EVNT_ENTER, dir);
        }

        prog_movement(ch, dir);
        return (TRUE);
    }
    return (FALSE);
}

//ADD BY SLOWN

ACMD(do_speedwalk)
{
    int dir, r;

    if (!*argument) {
        send_to_char("Куда вы собрались идти?\r\n", ch);
    }

    for (r = 1; *argument && r; argument++) {
        while (*argument == ' ')
            ++argument;

        switch (UPPER(*argument)) {
            case 'N':
            case 'С':
                dir = NORTH;
                break;
            case 'E':
            case 'В':
                dir = EAST;
                break;
            case 'S':
            case 'Ю':
                dir = SOUTH;
                break;
            case 'W':
            case 'З':
                dir = WEST;
                break;
            case 'U':
            case 'П':
                dir = UP;
                break;
            case 'D':
            case 'О':
                dir = DOWN;
                break;
            default:
                send_to_char("Формат команды: С(евер)Ю(г)В(осток)З(апад)П(вверх)О(вниз)\r\n", ch);
                return;
                break;
        }

        r = perform_move(ch, dir, 1, TRUE);
        if (r && *(argument + 1))
            send_to_char("\r\n", ch);
    }
}


ACMD(do_move)
{
    /*
     * This is basically a mapping of cmd numbers to perform_move indices.
     * It cannot be done in perform_move because perform_move is called
     * by other functions which do not require the remapping.
     */
    int nr_times, loop;

    if (*argument) {
        if (atoi(argument) <= 0)
            nr_times = 1;
        else
            nr_times = atoi(argument);
    } else
        nr_times = 1;


    if (nr_times <= 15) {
        for (loop = 0; loop < nr_times; loop++)
            perform_move(ch, subcmd - 1, 0, TRUE);
    } else
        send_to_char("Не больше 15ти шагов в одном направлении.\r\n", ch);

}

ACMD(do_hidemove)
{
    int dir = 0;
    struct affected_type af;

    skip_spaces(&argument);
    if (!GET_SKILL(ch, SKILL_SNEAK)) {
        send_to_char("Вы не знаете как.\r\n", ch);
        return;
    }

    if (!*argument) {
        send_to_char("И куда это Вы направляетесь?\r\n", ch);
        return;
    }

    if ((dir = search_block(argument, dirs, FALSE)) < 0 &&
        (dir = search_block(argument, DirIs, FALSE)) < 0) {
        send_to_char("Неизвестное направление.\r\n", ch);
        return;
    }
    if (on_horse(ch)) {
        send_to_char("Это этого нужно спешится.\r\n", ch);
        return;
    }

    affect_from_char(ch, SPELL_SNEAK);
    af.type = find_spell_num(SPELL_SNEAK);
    af.location = 0;
    af.modifier = GET_SKILL(ch, SKILL_SNEAK);
    af.duration = 1;            //Даем 1 секунду
    af.bitvector = AFF_SNEAK;
    af.battleflag = 0;
    af.owner = GET_ID(ch);
    af.main = TRUE;
    affect_join_char(ch, &af);
    perform_move(ch, dir, 0, TRUE);
    //affect_from_char(ch,SPELL_SNEAK);
}


int has_key(struct char_data *ch, obj_vnum key)
{
    struct obj_data *o;

    for (o = ch->carrying; o; o = o->next_content)
        if (GET_OBJ_VNUM(o) == key && GET_OBJ_TYPE(o) == ITEM_KEY)
            return (TRUE);

    if (GET_EQ(ch, WEAR_HOLD))
        if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == key &&
            GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_KEY)
            return (TRUE);

    return (FALSE);
}

void go_enter(struct char_data *ch, struct obj_data *obj)
{
    char buf[MAX_STRING_LENGTH], bufz[MAX_STRING_LENGTH], bufz1[MAX_STRING_LENGTH],
        buf2[MAX_STRING_LENGTH];

    if (obj) {
        if (GET_OBJ_TYPE(obj) != ITEM_TRANSPORT) {
            sprintf(buf, "%s не является транспортным средством.\r\n", GET_OBJ_PNAME(obj, 0));
            send_to_charf(ch, "%s", CAP(buf));
            return;
        }

        if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM)) {
            act("1и отказал1(ся,ась,ось,ись) погрузиться в @1в.", "Кмп", ch, obj);
            return;
        }

        if (ch->is_transpt) {
            send_to_charf(ch, "Вы уже сидите в транспортном средстве.\r\n");
            return;
        }

        if (add_char_to_obj(ch, obj)) {
            if (obj->transpt->driver == ch) {
                if (!TRANSP_FLAGGED(obj, TRANS_ENGINE)) {
                    sprintf(bufz, "взяли поводья в руки");
                    sprintf(bufz1, "взял$g поводья в руки");
                } else {
                    sprintf(bufz, "сели за руль");
                    sprintf(bufz1, "сел$g за руль");
                }

                sprintf(buf, "Вы вошли в $o3 и %s.", bufz);
                sprintf(buf2, "$n погрузил$u в $o3 и %s.", bufz1);
                act(buf, FALSE, ch, obj, 0, TO_CHAR);
                act(buf2, FALSE, ch, obj, 0, TO_ROOM);
            } else {
                act("Вы вошли в $o3.", FALSE, ch, obj, 0, TO_CHAR);
                act("$n погрузил$u в $o3.", FALSE, ch, obj, 0, TO_ROOM);
            }
        }
    } else
        send_to_charf(ch, "Здесь нет '%s'.\r\n", buf);

}


ACMD(do_leave)
{
    struct obj_data *obj;

    if (ch->is_transpt) {
        obj = ch->is_transpt;
        remove_char_from_obj(ch, obj);
        act("Вы вышли из $o1.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n выгрузил$u из $o1.", FALSE, ch, obj, 0, TO_ROOM);

        if (obj->transpt->driver == NULL && obj->transpt->people.size()) {
            obj->transpt->driver = obj->transpt->people[0];
            act("Управление $o5 перешло Вам.", FALSE, obj->transpt->people[0], obj, 0, TO_CHAR);
            act("Управление $o5 перешло $n2.", FALSE, obj->transpt->people[0], obj, 0, TO_ROOM);
        }
        if (!obj->transpt->driver)
            IN_ROOM(obj) = IN_ROOM(ch);
    } else
        send_to_charf(ch, "Вы уже покинули транспортное средство.\r\n");

}


ACMD(do_stand)
{
    struct mess_p_data *k = NULL;
    int found = 0, count = 0;

    /* Проверка на перехват сообщений */
    if (IN_ROOM(ch) != NOWHERE)
        for (k = world[IN_ROOM(ch)].mess_data; k; k = k->next, count++)
            if (k->command == CMD_STAND) {
                found = 1;
                break;
            }

    if (on_horse(ch)) {
        act("Прежде всего, Вам стоит слезть с $N1.", FALSE, ch, 0, get_horse(ch), TO_CHAR);
        return;
    }
    switch (GET_POS(ch)) {
        case POS_STANDING:
        case POS_FLYING:
            send_to_char("Вы уже стоите.\r\n", ch);
            break;
        case POS_SITTING:
            send_to_char("Вы встали.\r\n", ch);
            act("$n встал$g.", TRUE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = FIGHTING(ch) ? POS_FIGHTING : POS_STANDING;
            break;
        case POS_RESTING:
            if (found) {
                if (k->mess_to_char)
                    act(k->mess_to_char, "М", ch);
                if (k->mess_to_room)
                    act(k->mess_to_room, "Км", ch);
                go_script(k->script, ch);
                if (k->stoping)
                    return;
            } else {
                send_to_char("Вы прекратили отдыхать и встали.\r\n", ch);
                act("$n прекратил$g отдых и поднял$u.", TRUE, ch, 0, 0, TO_ROOM);
            }
            GET_POS(ch) = POS_STANDING;
            break;
        case POS_SLEEPING:
            send_to_char("Пожалуй, сначала стоит проснуться!\r\n", ch);
            break;
        case POS_FIGHTING:
            send_to_char("А Вы уже стоите.\r\n", ch);
            break;
        default:
            //send_to_char("Вы прекратили летать и опустились на землю.\r\n", ch);
            //act("$n опустил$u на землю.",TRUE, ch, 0, 0, TO_ROOM);
            //GET_POS(ch) = POS_STANDING;
            break;
    }
}


ACMD(do_sit)
{
    struct mess_p_data *k = NULL;
    int found = 0, count = 0;

    /* Проверка на перехват сообщений */
    if (IN_ROOM(ch) != NOWHERE)
        for (k = world[IN_ROOM(ch)].mess_data; k; k = k->next, count++)
            if (k->command == CMD_SIT) {
                found = 1;
                break;
            }

    if (on_horse(ch)) {
        act("Прежде всего, Вам стоит слезть с $N1.", FALSE, ch, 0, get_horse(ch), TO_CHAR);
        return;
    }

    if (FIGHTING(ch)) {
        send_to_char("Сесть? Во время боя?\r\n", ch);
        return;
    }

    if (real_sector(IN_ROOM(ch)) == SECT_FLYING) {
        send_to_charf(ch, "И как Вы собираетесь присесть в воздухе?\r\n");
        return;
    }

    if ((real_sector(IN_ROOM(ch)) == SECT_WATER_SWIM ||
         real_sector(IN_ROOM(ch)) == SECT_WATER_NOSWIM ||
         real_sector(IN_ROOM(ch)) == SECT_UNDERWATER) && !IS_AFFECTED(ch, AFF_WATERBREATH)) {
        send_to_charf(ch, "Вы рискуете захлебнуться если присядите.\r\n");
        return;
    }

    switch (GET_POS(ch)) {
        case POS_STANDING:
            if (found) {
                if (k->mess_to_char)
                    act(k->mess_to_char, "М", ch);
                if (k->mess_to_room)
                    act(k->mess_to_room, "Км", ch);
                go_script(k->script, ch);
                if (k->stoping)
                    return;
            } else {
                send_to_char("Вы сели.\r\n", ch);
                act("$n сел$g.", FALSE, ch, 0, 0, TO_ROOM);
            }
            GET_POS(ch) = POS_SITTING;
            break;
        case POS_SITTING:
            send_to_char("А Вы и так сидите.\r\n", ch);
            break;
        case POS_RESTING:
            if (found) {
                if (k->mess_to_char)
                    act(k->mess_to_char, "М", ch);
                if (k->mess_to_room)
                    act(k->mess_to_room, "Км", ch);
                go_script(k->script, ch);
                if (k->stoping)
                    return;
            } else {
                send_to_char("Вы прекратили отдыхать и сели.\r\n", ch);
                act("$n прервал$g отдых и сел$g.", TRUE, ch, 0, 0, TO_ROOM);
            }
            GET_POS(ch) = POS_SITTING;
            break;
        case POS_SLEEPING:
            send_to_char("Вам стоит проснуться.\r\n", ch);
            break;
        case POS_FIGHTING:
            send_to_char("Сесть? Во время боя? Вы явно не в себе.\r\n", ch);
            break;
        default:
            send_to_char("Вы прекратили свой полет и сели.\r\n", ch);
            act("$n прекратил$g свой полет и сел$g.", TRUE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = POS_SITTING;
            break;
    }
}


ACMD(do_rest)
{
    struct mess_p_data *k = NULL;
    int found = 0, count = 0;

    if (IS_UNDEAD(ch)) {
        if (ch->master)
            act("2и не нуждается в отдыхе.", "Мм", ch->master, ch);
        send_to_charf(ch, "Мертвые не нуждаются в отдыхе.\r\n");
        return;
    }
    /* Проверка на перехват сообщений */
    if (IN_ROOM(ch) != NOWHERE)
        for (k = world[IN_ROOM(ch)].mess_data; k; k = k->next, count++)
            if (k->command == CMD_REST) {
                found = 1;
                break;
            }


    if (on_horse(ch)) {
        act("Прежде всего, Вам стоит слезть с $N1.", FALSE, ch, 0, get_horse(ch), TO_CHAR);
        return;
    }

    if (FIGHTING(ch)) {
        send_to_char("Отдыха в бою Вам не будет!\r\n", ch);
        return;
    }

    if (real_sector(IN_ROOM(ch)) == SECT_FLYING) {
        send_to_charf(ch, "Отдыхать в воздухе невозможно.\r\n");
        return;
    }

    if ((real_sector(IN_ROOM(ch)) == SECT_WATER_SWIM ||
         real_sector(IN_ROOM(ch)) == SECT_WATER_NOSWIM ||
         real_sector(IN_ROOM(ch)) == SECT_UNDERWATER) && !IS_AFFECTED(ch, AFF_WATERBREATH)) {
        send_to_charf(ch, "Здесь Вы не сможете отодхнуть.\r\n");
        return;
    }

    switch (GET_POS(ch)) {
        case POS_STANDING:
            if (found) {
                if (k->mess_to_char)
                    act(k->mess_to_char, "М", ch);
                if (k->mess_to_room)
                    act(k->mess_to_room, "Км", ch);
                go_script(k->script, ch);
                if (k->stoping)
                    return;
            } else {
                send_to_char("Вы присели отдохнуть.\r\n", ch);
                act("$n присел$g отдохнуть.", TRUE, ch, 0, 0, TO_ROOM);
            }
            GET_POS(ch) = POS_RESTING;
            break;
        case POS_SITTING:
            send_to_char("Вы пристроились поудобнее для отдыха.\r\n", ch);
            act("$n пристроил$u поудобнее для отдыха.", TRUE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = POS_RESTING;
            break;
        case POS_RESTING:
            send_to_char("Вы и так отдыхаете.\r\n", ch);
            break;
        case POS_SLEEPING:
            send_to_char("Вам лучше сначала проснуться.\r\n", ch);
            break;
        case POS_FIGHTING:
            send_to_char("Отдыха в бою Вам не будет!\r\n", ch);
            break;
        default:
            send_to_char("Вы прекратили полет и присели отдохнуть.\r\n", ch);
            act("$n прекратил$g полет и пристроил$u поудобнее для отдыха.", FALSE, ch, 0, 0,
                TO_ROOM);
            GET_POS(ch) = POS_SITTING;
            break;
    }

}


ACMD(do_sleep)
{
    struct mess_p_data *k = NULL;
    int found = 0, count = 0;

    if (IS_UNDEAD(ch)) {
        if (ch->master)
            act("2и не нуждается в сне.", "Мм", ch->master, ch);
        send_to_charf(ch, "Мертвые не нуждаются в сне.\r\n");
        return;
    }

    /* Проверка на перехват сообщений */
    if (IN_ROOM(ch) != NOWHERE)
        for (k = world[IN_ROOM(ch)].mess_data; k; k = k->next, count++)
            if (k->command == CMD_SLEEP) {
                found = 1;
                break;
            }

    if (on_horse(ch)) {
        act("Прежде всего, Вам стоит слезть с $N1.", FALSE, ch, 0, get_horse(ch), TO_CHAR);
        return;
    }


    if (FIGHTING(ch)) {
        act("Во время боя спать нельзя!", FALSE, ch, 0, 0, TO_CHAR);
        return;
    }

    if (real_sector(IN_ROOM(ch)) == SECT_FLYING) {
        send_to_charf(ch, "Спать в воздухе невозможно.\r\n");
        return;
    }

    if ((real_sector(IN_ROOM(ch)) == SECT_WATER_SWIM ||
         real_sector(IN_ROOM(ch)) == SECT_WATER_NOSWIM ||
         real_sector(IN_ROOM(ch)) == SECT_UNDERWATER) && !IS_AFFECTED(ch, AFF_WATERBREATH)) {
        send_to_charf(ch, "Здесь Вы не сможете уснуть.\r\n");
        return;
    }

    switch (GET_POS(ch)) {
        case POS_STANDING:
        case POS_SITTING:
        case POS_RESTING:
            if (found) {
                if (k->mess_to_char)
                    act(k->mess_to_char, "М", ch);
                if (k->mess_to_room)
                    act(k->mess_to_room, "Км", ch);
                go_script(k->script, ch);
                if (k->stoping)
                    return;
            } else {
                send_to_char("Вы заснули.\r\n", ch);
                act("$n уснул$g.", TRUE, ch, 0, 0, TO_ROOM);
            }
            GET_POS(ch) = POS_SLEEPING;
            break;
        case POS_SLEEPING:
            send_to_char("А Вы и так спите.\r\n", ch);
            break;
        case POS_FIGHTING:
            send_to_char("Вам нужно сражаться!\r\n", ch);
            break;
        default:
            send_to_char("Вы прекратили свой полет и отошли ко сну.\r\n", ch);
            act("$n прекратил$g летать и нагло заснул$g.", TRUE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = POS_SLEEPING;
            break;
    }
}

ACMD(do_horseon)
{
    struct char_data *horse;
    char arg[MAX_STRING_LENGTH];

    /*  if (IS_NPC(ch))
       return; */

    /*  if (!get_horse(ch))
       {send_to_char("У Вас нет скакуна.\r\n",ch);
       return;
       } */

    if (on_horse(ch)) {
        send_to_char("Не пытайтесь усидеть на двух стульях.\r\n", ch);
        return;
    }

    if (GET_RACE(ch) == RACE_BARIAUR) {
        send_to_charf(ch, "Бариаурам крайне сложно ездить верхом.\r\n");
        return;
    }

    one_argument(argument, arg);
    if (*arg) {
        if (has_horse(ch, FALSE)) {
            horse = get_horse(ch);
            REMOVE_BIT(AFF_FLAGS(horse, AFF_HORSE), AFF_HORSE);
        }
        horse = get_char_vis(ch, arg, FIND_CHAR_ROOM);
    } else
        horse = get_horse(ch);

    if (horse == NULL) {
        send_to_char(NOPERSON, ch);
        return;
    }

    if (IN_ROOM(horse) != IN_ROOM(ch)) {
        send_to_char("Ваш скакун далеко от Вас.\r\n", ch);
        return;
    }

    if (!MOB_FLAGGED(horse, MOB_MOUNTING) || (MOB_FLAGGED(horse, MOB_MOUNTING) && !IS_MOUNT(horse))) {
        send_to_charf(ch, "Необходимо оседлать %s.\r\n", GET_PAD(horse, 3));
        return;
    }

    if (horse->master && horse->master != ch)
        horse_master_change(ch, horse);

    if (GET_POS(horse) < POS_FIGHTING)
        act("$N не сможет Вас нести в таком состоянии.", FALSE, ch, 0, horse, TO_CHAR);
    else if (AFF_FLAGGED(horse, AFF_TETHERED))
        act("Вам стоит отвязать $N3.", FALSE, ch, 0, horse, TO_CHAR);
    else if (GET_RACE(ch) == RACE_BARIAUR) {
        act("Да Вы с ума сошли! Вы же раздавите $N3", FALSE, ch, 0, horse, TO_CHAR);
        return;
    } else if (GET_REAL_DEX(ch) < number(0, 21)) {
        act("Вы хотели взобраться на спину $N1, но подскользнулись и упали.", FALSE, ch, 0, horse,
            TO_CHAR);
        act("$n хотел$g взобраться на спину $N1, но подскользнул$u и упал$g.", FALSE, ch, 0, horse,
            TO_ROOM);
        GET_POS(ch) = POS_SITTING;
    } else {
        if (affected_by_spell(ch, SPELL_SNEAK))
            affect_from_char(ch, SPELL_SNEAK);
        if (affected_by_spell(ch, SPELL_CAMOUFLAGE))
            affect_from_char(ch, SPELL_CAMOUFLAGE);
        if (affected_by_spell(ch, SPELL_FLY))
            affect_from_char(ch, SPELL_FLY);
        act("Вы взобрались на спину $N1.", FALSE, ch, 0, horse, TO_CHAR);
        act("$n взобрал$u на спину $N1.", FALSE, ch, 0, horse, TO_ROOM);
        if (!AFF_FLAGGED(horse, AFF_HORSE_BUY))
            make_horse(horse, ch);
        SET_BIT(AFF_FLAGS(horse, AFF_HORSE), AFF_HORSE);
        SET_BIT(AFF_FLAGS(ch, AFF_HORSE), AFF_HORSE);
    }
}

ACMD(do_horseoff)
{
    struct char_data *horse;

    if (IS_NPC(ch))
        return;
    if (!(horse = get_horse_on(ch))) {
        send_to_char("У Вас нет скакуна.\r\n", ch);
        return;
    }

    if (!on_horse(ch)) {
        send_to_char("Чтобы соскочить с кого-то, надо взабраться на кого-то.\r\n", ch);
        return;
    }

    act("Вы слезли со спины $N1.", FALSE, ch, 0, horse, TO_CHAR);
    act("$n соскочил$g с $N1.", FALSE, ch, 0, horse, TO_ROOM);
    REMOVE_BIT(AFF_FLAGS(ch, AFF_HORSE), AFF_HORSE);
    REMOVE_BIT(AFF_FLAGS(horse, AFF_HORSE), AFF_HORSE);
}

ACMD(do_horseget)
{
    struct char_data *horse;
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    if (on_horse(ch)) {
        send_to_char("Вы уже сидите на скакуне.\r\n", ch);
        return;
    }

    one_argument(argument, arg);
    if (*arg)
        horse = get_char_vis(ch, arg, FIND_CHAR_ROOM);
    else
        horse = get_horse_on(ch);

    if (horse == NULL)
        send_to_char(NOPERSON, ch);
    else if (IN_ROOM(horse) != IN_ROOM(ch))
        send_to_char("Ваш скакун далеко от Вас.\r\n", ch);
    else if (!IS_MOUNT(horse))
        send_to_char("Это не скакун.\r\n", ch);
    else
        /*if (horse->master != ch)
           send_to_char("Это не Ваш скакун.\r\n",ch);
           else */
    if (!AFF_FLAGGED(horse, AFF_TETHERED))
        act("А $N и не привязан$A.", FALSE, ch, 0, horse, TO_CHAR);
    else {
        act("Вы отвязали $N3.", FALSE, ch, 0, horse, TO_CHAR);
        act("$n отвязал$g $N3.", FALSE, ch, 0, horse, TO_ROOM);
        REMOVE_BIT(AFF_FLAGS(horse, AFF_TETHERED), AFF_TETHERED);
    }
}


ACMD(do_horseput)
{
    struct char_data *horse;
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    if (on_horse(ch)) {
        send_to_char("Вам стоит слезть со скакуна.\r\n", ch);
        return;
    }

    one_argument(argument, arg);
    if (*arg)
        horse = get_char_vis(ch, arg, FIND_CHAR_ROOM);
    else
        horse = get_horse(ch);
    if (horse == NULL)
        send_to_char(NOPERSON, ch);
    else if (IN_ROOM(horse) != IN_ROOM(ch))
        send_to_char("Ваш скакун далеко от Вас.\r\n", ch);
    else if (!IS_MOUNT(horse))
        send_to_char("Это не скакун.\r\n", ch);
    else
        /*  if (horse->master != ch)
           send_to_char("Это не Ваш скакун.\r\n",ch);
           else */
    if (AFF_FLAGGED(horse, AFF_TETHERED))
        act("А $N уже и так привязан$A.", FALSE, ch, 0, horse, TO_CHAR);
    else {
        act("Вы привязали $N3.", FALSE, ch, 0, horse, TO_CHAR);
        act("$n привязал$g $N3.", FALSE, ch, 0, horse, TO_ROOM);
        SET_BIT(AFF_FLAGS(horse, AFF_TETHERED), AFF_TETHERED);
    }
}


ACMD(do_horsetake)
{
    struct char_data *horse = NULL;
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    if (on_horse(ch)) {
        send_to_char("Для этого необходимо спешится.\r\n", ch);
        return;
    }

    one_argument(argument, arg);

    if (*arg)
        horse = get_char_vis(ch, arg, FIND_CHAR_ROOM);

    if (horse == NULL) {
        send_to_char(NOPERSON, ch);
        return;
    } else if (!IS_NPC(horse)) {
        send_to_char("Господи, не чуди...\r\n", ch);
        return;
    } else if (!MOB_FLAGGED(horse, MOB_MOUNTING)) {
        act("Вы не сможете оседлать $N3.", FALSE, ch, 0, horse, TO_CHAR);
        return;
    } else if (AFF_FLAGGED(horse, AFF_CHARM)) {
        send_to_charf(ch, "Прирученых животных не нужно оседлывать.\r\n");
        return;
    } else if (IS_MOUNT(horse)) {
        act("$N уже под седлом.", FALSE, ch, 0, horse, TO_CHAR);
        return;
    } else if (GET_POS(horse) < POS_STANDING) {
        act("$N не сможет стать Вашим скакуном.", FALSE, ch, 0, horse, TO_CHAR);
        return;
    } else {
        if (dice(1, GET_LEVEL(horse)) > GET_REAL_LCK(ch)) {
            act("Вы хотели оседлать $N3, но только разозлили $S.", FALSE, ch, 0, horse, TO_CHAR);
            act("Вы хотел$g оседлать $N3, но только разозлил$g $S.", FALSE, ch, 0, horse, TO_ROOM);
            inc_pk_thiefs(ch, horse);
            _damage(horse, ch, WEAP_RIGHT, 0, TRUE, C_POWER);
            return;
        }
        act("Вы оседлали $N3.", FALSE, ch, 0, horse, TO_CHAR);
        act("$n оседлал$g $N3.", FALSE, ch, 0, horse, TO_ROOM);
        add_follower(horse, ch, FLW_HORSE);
        make_horse(horse, ch);
    }
}

ACMD(do_givehorse)
{
    struct char_data *horse, *victim;
    char arg[MAX_STRING_LENGTH];

    if (ch->is_transpt) {
        go_change_driver(ch, argument);
        return;
    }

    if (IS_NPC(ch))
        return;

    if (!(horse = get_horse(ch))) {
        send_to_char("Да нету у Вас скакуна.\r\n", ch);
        return;
    }
    if (!has_horse(ch, TRUE)) {
        send_to_char("Ваш скакун далеко от Вас.\r\n", ch);
        return;
    }
    one_argument(argument, arg);
    if (!*arg) {
        send_to_char("Кому Вы хотите передать скакуна ?\r\n", ch);
        return;
    }
    if (!(victim = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_char("Вам некому передать скакуна.\r\n", ch);
        return;
    } else if (IS_NPC(victim)) {
        send_to_char("Он и без этого обойдется.\r\n", ch);
        return;
    }
    /*  if (get_horse(victim))
       {act("У $N1 уже есть скакун.\r\n",FALSE,ch,0,victim,TO_CHAR);
       return;
       } */
    if (on_horse(ch)) {
        send_to_char("Вам стоит слезть со скакуна.\r\n", ch);
        return;
    }
    if (AFF_FLAGGED(horse, AFF_TETHERED)) {
        send_to_char("Вам стоит прежде отвязать своего скакуна.\r\n", ch);
        return;
    }
    stop_follower(horse, SF_EMPTY);
    act("Вы передали своего скакуна $N2.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n передал$g Вам своего скакуна.", FALSE, ch, 0, victim, TO_VICT);
    act("$n передал$g своего скакуна $N2.", TRUE, ch, 0, victim, TO_NOTVICT);
    make_horse(horse, victim);
}

ACMD(do_stophorse)
{
    struct char_data *horse;
    char arg[MAX_STRING_LENGTH];

    if (IS_NPC(ch))
        return;

    one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Кого Вы хотите отпустить?\r\n");
        return;
    }

    if (!(horse = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_charf(ch, "Вы не видите '%s'.\r\n", arg);
        return;
    }

    if (horse->master != ch) {
        act("2и не следует за Вами.", "Мм", ch, horse);
        return;
    }

    if (IS_AFFECTED(horse, AFF_GROUP)) {
        REMOVE_BIT(AFF_FLAGS(horse, AFF_GROUP), AFF_GROUP);
        act("2и исключен2(,а,о,ы) из состава Вашей группы.", "Мм", ch, horse);
        act("Вы исключены из группы 1р.", "мМ", ch, horse);
        act("2и был2(,а,о,и) исключен2(,а,о,ы) из группы 1р.", "Кмм", ch, horse);
    } else {
        act("Вы отпустили 2в.", "Мм", ch, horse);
        act("1и отпустил1(,а,о,и) 2в.", "Кмм", ch, horse);
        REMOVE_BIT(AFF_FLAGS(horse, AFF_CHARM), AFF_CHARM);
        affect_from_char(horse, SPELL_CHARM);
    }
    stop_follower(horse, SF_EMPTY);

}





ACMD(do_wake)
{
    ACMD(do_return);
    struct char_data *vict;
    int self = 0;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (subcmd == SCMD_WAKEUP) {
        if (!(*arg)) {
            send_to_char("Кого разбудить?\r\n", ch);
            return;
        }
    } else {
        *arg = 0;
    }


    if (*arg) {
        if (GET_POS(ch) == POS_SLEEPING)
            send_to_char("Может быть Вам лучше проснуться?\r\n", ch);
        else if ((vict = get_char_vis(ch, arg, FIND_CHAR_ROOM)) == NULL)
            send_to_char(NOPERSON, ch);
        else if (vict == ch)
            self = 1;
        else if (AWAKE(vict))
            act("$E и не спал$G.", FALSE, ch, 0, vict, TO_CHAR);
        else if (AFF_FLAGGED(vict, AFF_SLEEP))
            act("Вы не можете $S разбудить!", FALSE, ch, 0, vict, TO_CHAR);
        else if (AFF_FLAGGED(vict, AFF_MEDITATION))
            act("Вы не можете прервать медитативный сон $N1.", FALSE, ch, 0, vict, TO_CHAR);
        else if (GET_POS(vict) < POS_SLEEPING)
            act("$M так плохо! Оставьте $S в покое!", FALSE, ch, 0, vict, TO_CHAR);
        else {
            if (vict->desc && vict->desc->character) {
                send_to_charf(vict->desc->character, "Ваше сознание вернулось в Ваше тело.\r\n");
                do_return(vict->desc->character, 0, 0, 1, 1);
            }
            act("Вы разбудили $N3.", FALSE, ch, 0, vict, TO_CHAR);
            act("$n разбудил$g Вас.", FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);
            GET_POS(vict) = POS_SITTING;
        }
        if (!self)
            return;
    }
    if (AFF_FLAGGED(ch, AFF_MEDITATION))
        send_to_charf(ch, "Вы не можете прервать медитативный сон самостоятельно.\r\n");
    else if (AFF_FLAGGED(ch, AFF_SLEEP))
        send_to_char("Вы не можете проснуться!\r\n", ch);
    else if (GET_POS(ch) > POS_SLEEPING)
        send_to_char("А Вы и не спали...\r\n", ch);
    else {
        /*       if (ch->desc->character)
           {
           send_to_charf(ch->desc->character,"Ваше сознание вернулось в Ваше тело.\r\n");
           do_return(ch->desc->character,0,0,1,1);
           } */
        send_to_char("Вы проснулись и сели.\r\n", ch);
        act("$n проснул$u и сел$g.", TRUE, ch, 0, 0, TO_ROOM);
        GET_POS(ch) = POS_SITTING;
    }
}


ACMD(do_follow)
{
    struct char_data *leader;
    char buf[MAX_STRING_LENGTH];

    one_argument(argument, buf);

    if (IS_NPC(ch) && AFF_FLAGGED(ch, AFF_CHARM) && FIGHTING(ch))
        return;


    if (*buf) {
        if (!str_cmp(buf, "я") || !str_cmp(buf, "self") || !str_cmp(buf, "me")) {
            if (!ch->master)
                send_to_char("Но вы ведь ни за кем не следуете...\r\n", ch);
            else
                stop_follower(ch, SF_EMPTY);
            return;
        }
        if (!(leader = get_char_vis(ch, buf, FIND_CHAR_ROOM))) {
            send_to_char(NOPERSON, ch);
            return;
        }
    } else {
        send_to_char("За кем Вы хотите следовать ?\r\n", ch);
        return;
    }

    if (!IS_NPC(ch) && PRF_FLAGGED(leader, PRF_NOFOLLOW)) {
        act("$N не разрешил$G следовать за собой.", FALSE, ch, 0, leader, TO_CHAR);
        act("$n хотел$g поседовать за Вами.", FALSE, ch, 0, leader, TO_VICT);
        return;
    }

    if (ch->master == leader) {
        act("Вы уже следуете за $N4.", FALSE, ch, 0, leader, TO_CHAR);
        return;
    }
    if (AFF_FLAGGED(ch, AFF_CHARM) && (ch->master)) {
        act("Но Вы можете следовать только за $N4!", FALSE, ch, 0, ch->master, TO_CHAR);
    } else {                    /* Not Charmed follow person */
        if (leader == ch) {
            if (!ch->master) {
                send_to_char("Вы уже следуете за собой.\r\n", ch);
                return;
            }
            stop_follower(ch, SF_EMPTY);
        } else {                //log("[Follow] Check circle...");
            if (circle_follow(ch, leader)) {
                send_to_char("Так у Вас целый хоровод получится.\r\n", ch);
                return;
            }
            //log("[Follow] Stop last follow...");
            if (ch->master)
                stop_follower(ch, SF_EMPTY);
            //log("[Follow] Start new follow...");
            add_follower(ch, leader, FLW_GROUP);
            //log("[Follow] Stop function...");
        }
    }
}

int check_sneak(struct char_data *ch, struct char_data *tch)
{
    struct follow_type *f = NULL;
    struct char_data *m = NULL;
    int percent = 0, prob = 0;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_SNEAK) : GET_SKILL(ch, SKILL_SNEAK);

//char buf[8912];


    if (AFF_FLAGGED(tch, AFF_SENSE_LIFE)) {
        add_victim_visible(tch, ch);
        return (TRUE);
    }
//Если мы уже знаем что он тут
    if (check_victim_visible(tch, ch))
        return (TRUE);

    prob = GET_REAL_INT(ch) + GET_REAL_DEX(ch) + GET_REAL_DEX(ch) + skill + RNDSKILL;
    percent = GET_REAL_INT(tch) + GET_REAL_WIS(tch) + GET_REAL_WIS(tch) +
        GET_SAVE3(tch, SAV_REFL) + saving_throws_3(tch, SAV_REFL) + RNDSKILL;

    if (!IS_NPC(tch) && may_pkill(tch, ch) == PC_REVENGE_PC)
        percent += (percent * 5) / 10;  //добавляем шанс 50% ПКиллерам

    /*sprintf(buf,"%s крадется умения %d | %d > %d",skill,percent,prob);
       act(buf,FALSE,ch,0,tch,TO_VICT); */

    if (percent > prob) {
        add_victim_visible(tch, ch);

        if (AFF_FLAGGED(tch, AFF_GROUP)) {
            m = (tch->party_leader ? tch->party_leader : tch);
            for (f = m->followers; f; f = f->next)
                if (AFF_FLAGGED(f->follower, AFF_GROUP))
                    add_victim_visible(f->follower, ch);
        }
        m = (tch->master ? tch->master : tch);
        for (f = m->followers; f; f = f->next)
            if (AFF_FLAGGED(f->follower, AFF_CHARM))
                add_victim_visible(f->follower, ch);

        return (TRUE);
    }

    return (FALSE);
}

int check_hide(struct char_data *ch, struct char_data *tch)
{
    struct follow_type *f = NULL;
    struct char_data *m = NULL;
    int percent, prob;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_HIDE) : GET_SKILL(ch, SKILL_HIDE);

//Если мы уже знаем что он тут

    if (AFF_FLAGGED(tch, AFF_SENSE_LIFE)) {
        add_victim_visible(tch, ch);
        return (TRUE);
    }

    if (check_victim_visible(tch, ch))
        return (TRUE);

//Не знаем
    prob = GET_REAL_DEX(ch) + GET_REAL_INT(ch) + GET_REAL_WIS(ch) + skill + RNDSKILL;
    percent = GET_REAL_INT(tch) + GET_REAL_WIS(tch) + GET_REAL_WIS(tch) +
        GET_SAVE3(tch, SAV_REFL) + saving_throws_3(tch, SAV_REFL) + RNDSKILL;


    if (!IS_NPC(tch) && may_pkill(tch, ch) == PC_REVENGE_PC)
        percent += (percent * 5) / 10;  //добавляем шанс 50% ПКиллерам

    if (percent > prob) {
        add_victim_visible(tch, ch);

        if (AFF_FLAGGED(tch, AFF_GROUP)) {
            m = (tch->party_leader ? tch->party_leader : tch);

            for (f = m->followers; f; f = f->next)
                if (AFF_FLAGGED(f->follower, AFF_GROUP))
                    add_victim_visible(f->follower, ch);
        }
        m = (tch->master ? tch->master : tch);
        for (f = m->followers; f; f = f->next)
            if (AFF_FLAGGED(f->follower, AFF_CHARM))
                add_victim_visible(f->follower, ch);

        return (TRUE);
    }

    return (FALSE);
}

void train_sneak(struct char_data *ch)
{
    int max_level = 0;
    struct char_data *tch = NULL, *vict = NULL;

// Сначала ищем монстра
    for (vict = world[ch->in_room].people; vict; vict = vict->next_in_room) {
        if (vict == ch || !IS_NPC(vict))
            continue;
        if (GET_LEVEL(vict) > max_level &&
            ((extra_aggressive(vict, ch)) ||
             (MOB_FLAGGED(vict, MOB_MEMORY) && may_pkill(vict, ch) == PC_REVENGE_PC))) {
            tch = vict;
            max_level = GET_LEVEL(vict);
        }
    }

    if (tch)
        improove_skill(ch, tch, 0, SKILL_SNEAK);
}

int add_char_to_obj(struct char_data *ch, struct obj_data *obj)
{
    struct transpt_data_obj *transp;

    if (!obj->transpt)
        obj->transpt = new transpt_data_obj();

    transp = obj->transpt;

    if (!transp->driver)
        transp->driver = ch;

    transp->people.push_back(ch);

    ch->is_transpt = obj;

    return (TRUE);
}

int remove_char_from_obj(struct char_data *ch, struct obj_data *obj)
{
    int i;
    struct transpt_data_obj *transp;

    if (!obj->transpt)
        return (FALSE);

    transp = obj->transpt;
    for (i = 0; i < (int) transp->people.size(); i++)
        if (transp->people[i] == ch) {
            transp->people.erase(transp->people.begin() + i);
            break;
        }

    if (obj->transpt->driver == ch)
        obj->transpt->driver = NULL;
    ch->is_transpt = NULL;

    return (TRUE);
}


int go_transport_move(struct char_data *ch, int dir)
{
    int direction = FALSE;
    room_rnum was_in, go_to;
    struct char_data *tch;
    struct obj_data *tobj;
    struct transpt_data_obj *transp;
    char buf[MAX_STRING_LENGTH];

    tobj = ch->is_transpt;
    transp = ch->is_transpt->transpt;

    if (!transp) {
        log("ОШИБКА: Карета пустая");
        return (FALSE);
    }
//Не хозяин не может перемещаться внутри кареты
    if (transp->driver != ch) {
        send_to_charf(ch, "Вы не управляете %s.\r\n", GET_OBJ_PNAME(tobj, 4));
        return (direction);
    }

    if (!TRANSP_FLAGGED(tobj, TRANS_ENGINE) && !transp->engine)
//if (!transp->engine)
    {
        act("Для передвижения $o должн$d быть запряженна.", FALSE, ch, tobj, 0, TO_CHAR);
        return (direction);
    }

    was_in = ch->in_room;
    go_to = world[was_in].dir_option[dir]->to_room;
    direction = dir + 1;

//Перемещаем всех кто сидит в карете

    for (int i = 0; i < (int) transp->people.size(); i++) {
        tch = transp->people[i];
        sprintf(buf, "Вы уехали %s в %s.\r\n", DirsTo[dir], GET_OBJ_PNAME(tobj, 5));
        if (!PRF_FLAGGED(tch, PRF_BRIEF))
            sprintf(buf + strlen(buf), "\r\n");
        send_to_charf(tch, buf);
        char_from_room(tch);
        char_to_room(tch, go_to);
        if (tch->desc != NULL && tch != transp->driver)
            look_at_room(tch, 0);
    }

//Перемещаем engine
    if (transp->engine) {
        char_from_room(transp->engine);
        char_to_room(transp->engine, go_to);
    }
//Перемещаем карету
    sprintf(buf, "$o с %s уехал$G %s.", GET_PAD(transp->driver, 4), DirsTo[dir]);
    act(buf, FALSE, world[was_in].people, tobj, 0, TO_ROOM);
    act(buf, FALSE, world[was_in].people, tobj, 0, TO_CHAR);

    obj_from_room(tobj);
    obj_to_room(tobj, go_to);

    sprintf(buf, "$o с %s приехал$G %s.", GET_PAD(transp->driver, 4), DirsFrom[dir]);
    act(buf, FALSE, ch, tobj, 0, TO_ROOM);

    return (direction);
}

ACMD(do_harness)
{
    struct obj_data *obj;
    struct char_data *vict;
    char arg[MAX_STRING_LENGTH];

    if (!check_fight_command(ch))
        return;

    if (!check_transport_command(ch))
        return;

    argument = one_argument(argument, arg);

    skip_spaces(&argument);

    if (!*arg) {
        send_to_charf(ch, "Кого Вы хотите запрячь?\r\n");
        return;
    }

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_charf(ch, "Здесь нет '%s'.\r\n", arg);
        return;
    }

    if (!*argument) {
        act("Куда Вы хотите запрячь $N3.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, argument, world[ch->in_room].contents))) {
        send_to_charf(ch, "Здесь нет '%s'.\r\n", argument);
        return;
    }

    if (GET_OBJ_TYPE(obj) != ITEM_TRANSPORT) {
        act("Запрячь $N3 можно только в транспортное средство.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }

    if (get_horse(ch) != vict && vict->master != ch) {
        act("$N не подчиняется Вам.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    } else if (!MOB_FLAGGED(vict, MOB_MOUNTING)) {
        act("Вы не сможете запрячь $N3.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }


    if (TRANSP_FLAGGED(obj, TRANS_ENGINE)) {
        act("В $o3 никого запрягать не нужно.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (obj->transpt && obj->transpt->engine) {
        act("$o уже запряжен$d $N4.", FALSE, ch, obj, obj->transpt->engine, TO_CHAR);
        return;
    }

    if (!obj->transpt)
        obj->transpt = new transpt_data_obj();

    act("Вы запрягли $N в $o3.", FALSE, ch, obj, vict, TO_CHAR);
    act("$n запряг$g Вас в $o3.", FALSE, ch, obj, vict, TO_VICT);
    act("$n запряг$g $N в $o3.", FALSE, ch, obj, vict, TO_NOTVICT);

    obj->transpt->engine = vict;
    vict->npc()->specials.transpt = obj;
}

ACMD(do_unharness)
{

}

void go_change_driver(struct char_data *ch, char *argument)
{
    struct char_data *vict;
    struct obj_data *obj;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Кому Вы хотите передать управление?\r\n", ch);
        return;
    }

    if (!ch->is_transpt) {
        send_to_charf(ch, "Но Вы даже не в транспорте.\r\n");
        return;
    }

    obj = ch->is_transpt;

    if (obj->transpt->driver != ch) {
        act("Но Вы не управляете $o4.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_charf(ch, "Здесь нет '%s'.\r\n", arg);
        return;
    }


    obj->transpt->driver = vict;
    act("Вы передали управление $o4 $N2.", FALSE, ch, obj, vict, TO_CHAR);
    act("$n передал$g Вам управление $o4.", FALSE, ch, obj, vict, TO_VICT);
    act("$n передал$g управление $o4 $N2.", FALSE, ch, obj, vict, TO_NOTVICT);

}


ACMD(do_flyon)
{
    int sector = 0;

    if (IN_ROOM(ch) == NOWHERE)
        return;

    if (!IS_AFFECTED(ch, AFF_FLY) && !IS_AFFECTED(ch, AFF_LEVIT)) {
        send_to_charf(ch, "Вы должны уметь летать или парить, что бы подняться в воздух.\r\n");
        return;
    }

    if (GET_POS(ch) == POS_FLYING) {
        send_to_charf(ch, "Выше взлететь не получится.\r\n");
        return;
    }

    sector = real_sector(IN_ROOM(ch));
    if (sector == SECT_UNDERWATER) {
        send_to_charf(ch, "Подводой невозможно летать.\r\n");
        return;
    }

    act("Вы плавно поднялись в воздух.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n плавно поднял$u в воздух.", FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_FLYING;
}

ACMD(do_flyoff)
{
    int sector = 0;
    char buf[MAX_STRING_LENGTH];

    if (IN_ROOM(ch) == NOWHERE)
        return;

    if (GET_POS(ch) < POS_FLYING) {
        send_to_charf(ch, "Вы не в полете.\r\n");
        return;
    }

    sector = real_sector(IN_ROOM(ch));

    sprintf(buf, "$n плавно опустил$u %s.",
            (sector == SECT_WATER_SWIM || sector == SECT_WATER_NOSWIM) ? "в воду" : "на землю");
    send_to_charf(ch, "Вы плавно опустились %s.\r\n",
                  (sector == SECT_WATER_SWIM
                   || sector == SECT_WATER_NOSWIM) ? "в воду" : "на землю");
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    GET_POS(ch) = POS_STANDING;
}


void check_ranger_trap(struct char_data *ch)
{
    struct obj_data *obj;
    int val = 0;

    if (IN_ROOM(ch) == NOWHERE)
        return;

    if (IS_FLY(ch))
        return;

    if (ch->trap_object)
        return;

    for (obj = world[IN_ROOM(ch)].contents; obj; obj = obj->next_content) {

        if (GET_OBJ_TYPE(obj) != ITEM_TRAP)
            continue;

        if (check_obj_visible(ch, obj))
            continue;

        if (!GET_OBJ_VAL(obj, 1))
            continue;

        val = GET_OBJ_VAL(obj, 0);

        if (val > number(1, 150) + GET_REAL_LCK(ch)) {
            REMOVE_BIT(GET_OBJ_EXTRA(obj, ITEM_HIDDEN), ITEM_HIDDEN);
            GET_OBJ_VAL(obj, 1) = FALSE;
            act("Вы угодили в установленный здесь $o.", FALSE, ch, obj, 0, TO_CHAR);
            act("$n угодил$g в $o и забил$u, пытаясь освободиться.", FALSE, ch, obj, 0, TO_ROOM);
            obj->trap_victim = ch;
            ch->trap_object = obj;
        }

    }

}

void check_necro_spare(struct char_data *ch)
{
    struct obj_data *obj;
    int val = 0;

    if (IN_ROOM(ch) == NOWHERE)
        return;

    if (ch->trap_object)
        return;

    for (obj = world[IN_ROOM(ch)].contents; obj; obj = obj->next_content) {

        if (GET_OBJ_TYPE(obj) != ITEM_SPARE)
            continue;

        if (check_obj_visible(ch, obj))
            continue;

        if (!GET_OBJ_VAL(obj, 1))
            continue;

        val = GET_OBJ_VAL(obj, 0);

        //резист рефлекс 1..15%
        if (!general_savingthrow_3(ch, SAV_WILL, val / 10)) {
            //REMOVE_BIT(GET_OBJ_EXTRA(obj,ITEM_HIDDEN),ITEM_HIDDEN);
            GET_OBJ_VAL(obj, 1) = FALSE;
            act("$o впился Вам в голову.", FALSE, ch, obj, 0, TO_CHAR);
            act("$o впился в голову $n1.", FALSE, ch, obj, 0, TO_ROOM);
            obj->trap_victim = ch;
            ch->trap_object = obj;
        }
    }

}

ACMD(do_take)
{
    struct char_data *victim = NULL;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!(victim = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (!*arg && FIGHTING(ch) && IN_ROOM(ch) == IN_ROOM(FIGHTING(ch)))
            victim = FIGHTING(ch);
        else {
            send_to_char("К кому Вы хотите прикоснуться.\r\n", ch);
            return;
        }
    }

    act("Вы прикоснулись рукой к $N2.", FALSE, ch, 0, victim, TO_CHAR);
    act("$n легко дотронул$u до Вас рукой.", FALSE, ch, 0, victim, TO_VICT);
    act("$n легко дотронул$u до $N1 рукой.", FALSE, ch, 0, victim, TO_NOTVICT);
}



//Приблизиться
ACMD(do_move_up)
{
    if (!FIGHTING(ch)) {
        send_to_charf(ch, "Но Вы ни с кем не сражаетесь!\r\n");
        return;
    }

    if (affected_by_spell(ch, SPELL_GRASP)) {
        send_to_char("В состоянии шока Вы не перемещаться.\r\n", ch);
        return;
    }

    if (!MAY_MOVE2(ch)) {
        send_to_charf(ch, "Ваше предвижение в данный момент затрудненно!\r\n");
        return;
    }

    ch->char_specials.chage_distance = 1;
}

//Отступить
ACMD(do_move_retr)
{
    if (!FIGHTING(ch)) {
        send_to_charf(ch, "Но Вы ни с кем не сражаетесь!\r\n");
        return;
    }

    if (affected_by_spell(ch, SPELL_GRASP)) {
        send_to_char("В состоянии шока Вы не перемещаться.\r\n", ch);
        return;
    }

    if (!MAY_MOVE(ch)) {
        send_to_charf(ch, "Ваше предвижение в данный момент затрудненно!\r\n");
        return;
    }

    ch->char_specials.chage_distance = -1;
}

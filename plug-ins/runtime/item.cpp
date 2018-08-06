/********************************/
/* МПМ Грани Мира (с) 2002-2005 */
/********************************/

#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "constants.h"
#include "case.h"
#include "screen.h"
#include "events.h"
#include "expr.h"
#include "xboot.h"


#define TURN_RIGHT 1
#define TURN_LEFT 2

struct item_op_data *find_operations(struct obj_data *obj, struct char_data *actor, int no_cmd,
                                     int obj_arg)
{
    struct item_op_data *k;

    for (k = obj->operations; k; k = k->next)
        if ((k->command == no_cmd && !obj_arg) ||
            (k->command == no_cmd && obj_arg && obj_arg == k->obj_vnum)) {
            if (!EXPR(k->expr)->Expr(actor, NULL, NULL))
                continue;
            return (k);
        }
    return (NULL);
}

struct item_op_data *find_operations(struct obj_data *obj, struct char_data *actor, int no_cmd)
{
    return find_operations(obj, actor, no_cmd, 0);
}

struct item_op_data *find_operations(struct obj_data *obj, struct char_data *actor, int no_cmd,
                                     int obj_arg, int mob_arg)
{
    struct item_op_data *k;

    for (k = obj->operations; k; k = k->next)
        if ((k->command == no_cmd && !mob_arg) ||
            (k->command == no_cmd && mob_arg && mob_arg == k->mob_vnum)) {
            if (!EXPR(k->expr)->Expr(actor, NULL, NULL))
                continue;
            return (k);
        }

    return (NULL);
}

struct item_op_data *find_operations(struct char_data *mob, int no_cmd, int obj_arg)
{
    struct item_op_data *k;

    for (k = mob->operations; k; k = k->next)
        if ((k->command == no_cmd && !obj_arg) ||
            (k->command == no_cmd && obj_arg && obj_arg == k->obj_vnum))
            return (k);

    return (NULL);
}

struct item_op_data *find_operations(struct char_data *mob, int no_cmd)
{
    return find_operations(mob, no_cmd, 0);
}

struct item_op_data *find_operations(struct char_data *mob, int no_cmd, int obj_arg, int mob_arg)
{
    struct item_op_data *k;

    for (k = mob->operations; k; k = k->next)
        if ((k->command == no_cmd && !mob_arg) ||
            (k->command == no_cmd && mob_arg && mob_arg == k->mob_vnum))
            return (k);

    return (NULL);
}

///////////////////////////////////////////

void exe_script(struct char_data *actor, struct obj_data *obj, int nr, struct obj_data *tobj)
{
//Пусто все действие перенеслось в post_exeс
}

void post_exec(struct char_data *ch, struct obj_data *obj, struct obj_data *tobj,
               struct item_op_data *k)
{
    struct obj_data *robj = NULL;
    bool show_mess = FALSE;
    int in_room = IN_ROOM(ch);
    struct room_data *room = NULL;

    if (in_room != NOWHERE)
        room = &world[in_room];

    if (k->load_room) {
        robj = read_object(k->load_room, VIRTUAL, TRUE);
        if (robj) {
            if (IN_ROOM(ch) != NOWHERE) {
                int rnum = GET_OBJ_RNUM(robj);

                if (GET_OBJ_LIMIT(robj) >= obj_index[rnum].number + obj_index[rnum].stored) {
                    obj_to_room(robj, IN_ROOM(ch));
                    show_mess = TRUE;
                } else {
                    send_to_charf(ch, "Но ничего не произошло.\r\n");
                    extract_obj(robj);
                }
            } else
                extract_obj(robj);
        }
    }

    if (k->load_char) {
        robj = read_object(k->load_char, VIRTUAL, TRUE);
        if (robj) {
            int rnum = GET_OBJ_RNUM(robj);

            if (GET_OBJ_LIMIT(robj) >= obj_index[rnum].number + obj_index[rnum].stored) {
                obj_to_char(robj, ch);
                show_mess = TRUE;
            } else {
                send_to_charf(ch, "Но ничего не произошло.\r\n");
                extract_obj(robj);
            }
        }
    }

    if (show_mess) {
        if (k->mess_load_char)
            act(k->mess_load_char, "Мппп", ch, obj, tobj, robj);
        if (k->mess_load_room)
            act(k->mess_load_room, "Кмппп", ch, obj, tobj, robj);
    }

    if (k->script)
        go_script(k->script, ch, obj);

    if (k->extract >= 1)
        extract_obj(obj);
    if (tobj && k->extract >= 2)
        extract_obj(tobj);
    return;
}

void post_exec_victim(struct char_data *ch, struct obj_data *obj, struct char_data *victim,
                      struct item_op_data *k)
{
    int in_room = IN_ROOM(ch);
    struct obj_data *robj = NULL;
    bool show_mess = FALSE;
    struct room_data *room = NULL;

    if (in_room != NOWHERE)
        room = &world[in_room];

    if (k->load_room) {
        robj = read_object(k->load_room, VIRTUAL, TRUE);
        if (robj) {
            if (IN_ROOM(ch) != NOWHERE) {
                int rnum = GET_OBJ_RNUM(robj);

                if (GET_OBJ_LIMIT(robj) >= obj_index[rnum].number + obj_index[rnum].stored) {
                    obj_to_room(robj, IN_ROOM(ch));
                    show_mess = TRUE;
                } else {
                    send_to_charf(ch, "Но ничего не произошло.\r\n");
                    extract_obj(robj);
                }
            } else
                extract_obj(robj);
        }
    }

    if (k->load_char) {
        robj = read_object(k->load_char, VIRTUAL, TRUE);
        if (robj) {
            int rnum = GET_OBJ_RNUM(robj);

            if (GET_OBJ_LIMIT(robj) >= obj_index[rnum].number + obj_index[rnum].stored) {
                obj_to_char(robj, ch);
                show_mess = TRUE;
            } else {
                send_to_charf(ch, "Но ничего не произошло.\r\n");
                extract_obj(robj);
            }
        }
    }

    if (show_mess) {
        if (k->mess_load_char)
            act(k->mess_load_char, "Мпмп", ch, obj, victim, robj);
        if (k->mess_load_room)
            act(k->mess_load_room, "Кмпмп", ch, obj, victim, robj);
    }

    if (k->script)
        go_script(k->script, ch, obj);

    if (k->extract >= 1)
        extract_obj(obj);
    if (victim && k->extract >= 2)
        extract_char(victim, FALSE);
    return;
}


void turn_lockpick(struct char_data *ch, struct obj_data *obj, char *turn)
{
    int trn = 0;

    if (!IS_LOCKPICK(obj)) {
        act("Повернуть @1в будет слишком трудно, даже невозможно.", "Мп", ch, obj);
        return;
    }

    if (ch->dir_pick == -1 && ch->obj_pick == NULL) {
        act("Прежде, чем поворачивать @1в, начните что-нибудь взламывать.", "Мп", ch, obj);
        return;
    }

    if (!*turn) {
        act("Куда Вы хотите повернуть @1в, вправо или влево?", "Мп", ch, obj);
        return;
    }

    if (isname(turn, "вправо"))
        trn = TURN_RIGHT;
    else if (isname(turn, "влево"))
        trn = TURN_LEFT;

    if (!trn) {
        send_to_charf(ch, "Неизвестное направление для поворота.\r\n");
        return;
    }

    char *lcode;
    int *pstep;

    if (ch->dir_pick != -1) {
        lcode = EXITDATA(IN_ROOM(ch), ch->dir_pick)->lock_code;
        pstep = &(EXITDATA(IN_ROOM(ch), ch->dir_pick)->lock_step);
    } else if (ch->obj_pick) {
        lcode = ch->obj_pick->lock_code;
        pstep = &(ch->obj_pick->lock_step);
    } else
        return;

    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_PICK_LOCK) : GET_SKILL(ch, SKILL_PICK_LOCK);
    int pick_quality = GET_OBJ_QUALITY(obj) * 3;
    int dir_pick = ch->dir_pick;
    struct obj_data *obj_pick = ch->obj_pick;

    act("Вы осторожно повернули @1в %1.", "Мпт", ch, obj, trn == TURN_LEFT ? "налево" : "направо");
    act("1и осторожно повернул1(,а,о,и) @1в %1.", "Кмпт", ch, obj,
        trn == TURN_LEFT ? "налево" : "направо");
    if ((trn == TURN_RIGHT && lcode[*pstep] == '>') || (trn == TURN_LEFT && lcode[*pstep] == '<')) {
        (*pstep)++;
        if (!lcode[*pstep]) {
            act("Мелодично щелкнув, замок уступил Вашим усилиям.", "М", ch);
            if (dir_pick != -1) {
                get_pick_door(ch, dir_pick);
                EXIT(ch, dir_pick)->lock_step = 0;
                pstep = 0;

            } else if (obj_pick) {
                get_pick_cont(ch, obj_pick);
                obj_pick->lock_step = 0;
                pstep = 0;
            }
            ch->dir_pick = -1;
            ch->obj_pick = NULL;
            return;
        } else {
            switch (number(0, 2)) {
                case 0:
                    act("Замок отозвался тихим щелчком.", "М", ch);
                    break;
                case 1:
                    act("В замке что-то тихо щелкнуло.", "М", ch);
                    break;
                case 2:
                    act("Вы уловили едва слышный щелчок.", "М", ch);
                    break;
            }
        }
    } else {
        if (dir_pick != -1) {
            act("Не скрывая разачарования, $1и оторвал1(ся,ась,ось,ись) от %1.", "Кмт", ch,
                get_name_pad(EXIT(ch, dir_pick)->exit_name, PAD_VIN, PAD_OBJECT));
            ch->dir_pick = -1;
            ch->obj_pick = NULL;
            EXIT(ch, dir_pick)->lock_step = 0;
            pstep = 0;
        } else if (obj_pick) {
            act("Не скрывая разачарования, $1и оторвал1(ся,ась,ось,ись) от @1р.", "Кмп", ch,
                obj_pick);
            ch->dir_pick = -1;
            ch->obj_pick = NULL;
            obj->lock_step = 0;
            pstep = 0;
        }
        if (50 + pick_quality + (skill / 4) < number(0, 100)) {
            switch (number(0, 2)) {
                case 0:
                    act("В замке что-то скрежетнуло и @1и сломал@1(ся,ась,ось,ись).", "Мп", ch,
                        obj);
                    break;
                case 1:
                    act("В замке что-то резко хрустнуло и @1и сломал@1(ся,ась,ось,ись).", "Мп", ch,
                        obj);
                    break;
                case 2:
                    act("Замок отозвался пронзительным скрежетом и @1и сломал@1(ся,ась,ось,ись).",
                        "Мп", ch, obj);
                    break;
            }
            extract_obj(obj);
            if (number(1, 4) == 2) {
                char lock_code[32];

                *lock_code = '\0';
                if (dir_pick != -1) {
                    set_door_code(lock_code,
                                  (world[IN_ROOM(ch)].dir_option[dir_pick]->quality * 15) / 100);
                    strcpy(world[IN_ROOM(ch)].dir_option[dir_pick]->lock_code, lock_code);
                    world[IN_ROOM(ch)].dir_option[dir_pick]->lock_step = 0;
                } else if (obj_pick) {
                    set_door_code(lock_code, (GET_OBJ_QUALITY(obj_pick) * 15) / 10);
                    strcpy(obj_pick->lock_code, lock_code);
                    obj_pick->lock_step = 0;
                }
                act("Вы услышали как шестеренки в замке пришли в движение.", "М", ch);
            }
            act("Похоже, придется начинать все сначала.", "М", ch);
            return;
        } else {
            switch (number(0, 2)) {
                case 0:
                    act("В замке что-то неприятно скрежетнуло.", "М", ch);
                    break;
                case 1:
                    act("В замке что-то резко хрустнуло.", "М", ch);
                    break;
                case 2:
                    act("Замок отозвался пронзительным скрежетом.", "М", ch);
                    break;
            }
        }
        act("Похоже, придется начинать все сначала.", "М", ch);
    }

}

ACMD(do_item_turn)
{
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH], turn[MAX_STRING_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *found_char = NULL;

    skip_spaces(&argument);
    argument = two_arguments(argument, arg, turn);

    if (!*arg) {
        send_to_charf(ch, "Что именно Вы хотите повернуть?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_EQUIP | FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if (is_abbrev(turn, "на") || is_abbrev(turn, "to"))
        strcpy(turn, argument);

    if (IS_LOCKPICK(obj))
        turn_lockpick(ch, obj, turn);
    else {
        if ((k = find_operations(obj, ch, CMD_TURN))) {
            if (*turn && k->arg && !isname(k->arg, turn)) {
                act("Вы попытались повернуть @1в, но ничего не добились.", "Мп", ch, obj);
                act("$1и попытал1(ся,лась,лось,лись) повернуть @1в, но ничего не добил1(ся,ась,ось,ись).", "Кмп", ch, obj);
                return;
            }

            if (k->active)
                act(k->active, "Мп", ch, obj);
            else
                act("Вы медленно повернули @1в.", "Мп", ch, obj);

            if (k->active_room)
                act(k->active_room, "Кмп", ch, obj);
            else
                act("$1и медленно повернул1(,а,о,и) @1в.", "Кмп", ch, obj);

            post_exec(ch, obj, NULL, k);
            return;
        } else {
            act("Повернуть @1в? В этом нет никакого смысла.", "Мп", ch, obj);
            act("$1и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, obj);
        }
    }
}


ACMD(do_item_press)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (is_abbrev(arg, "на") || is_abbrev(arg, "to"))
        strcpy(arg, argument);

    if (!*arg) {
        send_to_charf(ch, "Что именно Вы хотели нажать?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_PRESS))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Вы осторожно нажали на @1в.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("$1и осторожно нажал$1(,а,о,и) на @1в.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Нажать на @1в? Даже и не пытайтесь.", "Мп", ch, obj);
        act("$1и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, obj);
    }
}


ACMD(do_item_insert)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL, *tobj = NULL;
    struct item_op_data *k = NULL;
    char name[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];


    if (!*argument) {
        send_to_charf(ch, "Что именно и куда Вы хотите вставить?\r\n");
        return;
    }

    skip_spaces(&argument);
    argument = two_arguments(argument, name, arg);


    if (is_abbrev(arg, "в") || is_abbrev(arg, "in"))
        strcpy(arg, argument);

    if (!*name) {
        send_to_charf(ch, "Что Вы хотите вставить?\r\n");
        return;
    }

    generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", name);
        return;
    }

    if (!*arg) {
        act("Куда Вы хотите вставить @1в?", "Мп", ch, obj);
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &tobj);


    if (!tobj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_INSERT, GET_OBJ_VNUM(tobj)))) {
        if (k->obj_vnum != GET_OBJ_VNUM(tobj)) {
            act("Вы попытались вставить @1в в @2в, но ничего не вышло.", "Мпп", ch, obj, tobj);
            act("$1+и приставил1(,а,о,и) @1в к @2д, но ничего не добил1(ся,ась,ось,ись).", "Мпп",
                ch, obj, tobj);
            return;
        }

        if (k->active)
            act(k->active, "Мпп", ch, obj, tobj);
        else
            act("Вы вставили @1в в @2в.", "Мпп", ch, obj, tobj);

        if (k->active_room)
            act(k->active_room, "Кмпп", ch, obj, tobj);
        else
            act("$1и вставил1(,а,о,и) @1в в @2в.", "Кмпп", ch, obj, tobj);

        post_exec(ch, obj, tobj, k);
        return;
    } else {
        act("Даже не пытайтесь, вставить @1в в @2в Вам не удастся.", "Мпп", ch, obj, tobj);
        act("$1+и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, obj);
    }
}

ACMD(do_item_join)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL, *tobj = NULL;
    struct item_op_data *k = NULL;
    char name[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];


    if (!*argument) {
        send_to_charf(ch, "Что именно и с чем Вы хотите соединить?\r\n");
        return;
    }

    skip_spaces(&argument);
    argument = two_arguments(argument, name, arg);

    if (is_abbrev(arg, "и") || is_abbrev(arg, "and"))
        strcpy(arg, argument);

    if (!*name) {
        send_to_charf(ch, "Что Вы хотите соединить?\r\n");
        return;
    }

    generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", name);
        return;
    }

    if (!*arg) {
        act("С чем Вы хотите соединить @1в?", "Мп", ch, obj);
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &tobj);


    if (!tobj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_JOIN, GET_OBJ_VNUM(tobj)))) {
        if (k->obj_vnum != GET_OBJ_VNUM(tobj)) {
            act("Вы попытались соединить @1в и @2в, но ничего не вышло.", "Мпп", ch, obj, tobj);
            act("$1и попытал1(ся,ась,ось,ись) соединить @1в и @2в, но ничего не добил1(ся,ась,ось,ись).", "Мпп", ch, obj, tobj);
            return;
        }

        if (k->active)
            act(k->active, "Мпп", ch, obj, tobj);
        else
            act("Вы соединили @1в и @2в.", "Мпп", ch, obj, tobj);

        if (k->active_room)
            act(k->active_room, "Кмпп", ch, obj, tobj);
        else
            act("$1и соединил1(,а,о,и) @1в и @2в.", "Кмпп", ch, obj, tobj);

        post_exec(ch, obj, tobj, k);
        return;
    } else {
        act("Даже не пытайтесь, соединить @1в в @2в Вам не удастся.", "Мпп", ch, obj, tobj);
        act("$1+и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, obj);
    }
}


ACMD(do_item_push)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Что именно Вы хотели сдвинуть с места?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_PUSH))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Собравшись с силами, Вы сдвинули с места @1в.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("Собравшись с силами, $1и сдвинул1(,а,о,и) с места @1в.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Даже и не пытайтесь, сдвинуть с места @1в все равно не удастся.", "Мп", ch, obj);
        act("$1и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, obj);
    }
}


ACMD(do_item_twist)
{
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH], turn[MAX_STRING_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *found_char = NULL;

    skip_spaces(&argument);
    argument = two_arguments(argument, arg, turn);

    if (!*arg) {
        send_to_charf(ch, "Что именно Вы хотили покрутить?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if (is_abbrev(turn, "на") || is_abbrev(turn, "to"))
        strcpy(turn, argument);

    if (IS_LOCKPICK(obj))
        turn_lockpick(ch, obj, turn);
    else {
        if ((k = find_operations(obj, ch, CMD_TWIST))) {
            if (*turn && k->arg && !isname(k->arg, turn)) {
                act("Вы попытались крутить @1в, но ничего не вышло.", "Мп", ch, obj);
                act("$1и попытал1(ся,ась,ось,ись) крутить @1в, но у н1ер ничего не вышло.", "Кмп",
                    ch, obj);
                return;
            }

            if (k->active)
                act(k->active, "Мп", ch, obj);
            else
                act("Вы осторожно покрутили @1в.", "Мп", ch, obj);

            if (k->active_room)
                act(k->active_room, "Кмп", ch, obj);
            else
                act("$1и осторожно покрутил1(,а,о,и) @1в.", "Кмп", ch, obj);

            post_exec(ch, obj, NULL, k);
            return;
        } else {
            act("Крутить @1в? Даже и не пытайтесь.", "Мп", ch, obj);
            act("$1и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, obj);
        }
    }
}

ACMD(do_item_breach)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL, *tobj = NULL;
    struct item_op_data *k = NULL;
    char name[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];


    if (!*argument) {
        send_to_charf(ch, "Что именно и чем Вы хотели бы пробить?\r\n");
        return;
    }

    skip_spaces(&argument);
    argument = two_arguments(argument, name, arg);

    if (!*name) {
        send_to_charf(ch, "В чем Вы хотите пробить?\r\n");
        return;
    }

    generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", name);
        return;
    }

    if (!*arg) {
        act("Куда Вы хотите вставить @1в?", "Мп", ch, obj);
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &tobj);


    if (!tobj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_BREACH, GET_OBJ_VNUM(tobj)))) {
        if (k->obj_vnum != GET_OBJ_VNUM(tobj)) {
            act("Вы попытались пробить @1в @2т, но ничего не вышло.", "Мпп", ch, obj, tobj);
            act("$1и попытал1(ся,ась,ось,ись) пробить @1в @2т, но у н1ер ничего не вышло.", "Мпп",
                ch, obj, tobj);
            return;
        }

        if (k->active)
            act(k->active, "Мпп", ch, obj, tobj);
        else
            act("Тщательно прицелившись, Вы пробили @1в @2т.", "Мпп", ch, obj, tobj);

        if (k->active_room)
            act(k->active_room, "Кмпп", ch, obj, tobj);
        else
            act("Тщательно прицелившись, $1и пробил1(,а,о,и) @1в @2т.", "Кмпп", ch, obj, tobj);

        post_exec(ch, obj, tobj, k);
        return;
    } else {
        act("Пробить @1в? Да в своем ли Вы уме?", "Мпп", ch, obj, tobj);
        act("$1+и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, obj);
    }
}


ACMD(do_item_nailup)
{
    struct char_data *found_char = NULL;
    struct obj_data *hammer =
        GET_EQ(ch, WEAR_WIELD) ? GET_EQ(ch,
                                        WEAR_WIELD) : (GET_EQ(ch, WEAR_BOTHS) ? GET_EQ(ch,
                                                                                       WEAR_BOTHS) :
                                                       NULL);
    struct obj_data *obj = NULL, *tobj = NULL;
    struct item_op_data *k = NULL;
    char name[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];


    if (!*argument) {
        send_to_charf(ch, "Что именно и чем Вы хотели бы забить?\r\n");
        return;
    }

    skip_spaces(&argument);
    argument = two_arguments(argument, name, arg);

    if (!*name) {
        send_to_charf(ch, "Что именно и куда Вы хотели бы забить?\r\n");
        return;
    }

    generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", name);
        return;
    }

    if (!*arg) {
        act("Куда именно Вы хотели бы забить @1в?", "Мп", ch, obj);
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &tobj);


    if (!tobj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if (!hammer || (hammer && !IS_HAMMER(hammer))) {
        send_to_charf(ch, "Прежде чем начнете что-то забивать, возьмите в руку молоток.\r\n");
        return;
    }

    if ((k = find_operations(obj, ch, CMD_NAILUP, GET_OBJ_VNUM(tobj)))) {
        if (k->obj_vnum != GET_OBJ_VNUM(tobj)) {
            act("Вы взмахнули @1т, но забить @2в в @3в не удалось.", "Мппп", ch, hammer, obj, tobj);
            act("$1и взмахнул1(,а,о,и) @1т, но пробить @2в @3т, 1ед не удалось.", "Мппп", ch,
                hammer, obj, tobj);
            return;
        }

        if (k->active)
            act(k->active, "Мппп", ch, obj, tobj);
        else
            act("Умело ударив @1т, Вы забили @2в в @3в.", "Мппп", ch, hammer, obj, tobj);

        if (k->active_room)
            act(k->active_room, "Кмппп", ch, obj, tobj);
        else
            act("$1и взмахнул1(,а,о,и) @1т, но пробить @2в @3т, 1ед не удалось.", "Кмппп", ch,
                hammer, obj, tobj);

        post_exec(ch, obj, tobj, k);
        return;
    } else {
        act("Забить @1в в @2в? Только не здесь!", "Мпп", ch, obj, tobj);
        act("$1+и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, tobj);
    }
}


ACMD(do_item_shatter)
{
    struct char_data *found_char = NULL;
    struct obj_data *weapon =
        GET_EQ(ch, WEAR_WIELD) ? GET_EQ(ch,
                                        WEAR_WIELD) : (GET_EQ(ch, WEAR_BOTHS) ? GET_EQ(ch,
                                                                                       WEAR_BOTHS) :
                                                       NULL);
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];


    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Что именно Вы хотели бы разбить?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }



    if ((k = find_operations(obj, ch, CMD_SHATTER))) {
        if (k->tool) {
            if (!GET_EQ(ch, WEAR_BOTHS) || GET_OBJ_TOOL(GET_EQ(ch, WEAR_BOTHS)) != k->tool) {
                act("Что бы разбить @1в Вы должны держать в руках %1.", "Мпт", ch, obj,
                    get_name_pad((char *) tool_name[k->tool], 3, PAD_OBJECT));
                return;
            }
        } else
            if (!weapon ||
                (weapon &&
                 (GET_OBJ_TYPE(weapon) != ITEM_WEAPON ||
                  (GET_OBJ_TYPE(weapon) == ITEM_WEAPON &&
                   real_attack_type(GET_OBJ_VAL(weapon, 3)) != HIT_BLOW)))) {
            send_to_charf(ch,
                          "Прежде чем начнете что-то забивать, возьмите в руку ударное оружие.\r\n");
            return;
        }

        if (k->active)
            act(k->active, "Мпп", ch, obj, weapon);
        else
            act("Вы разбили @1в, ударив по н@1ед @2т.", "Мпп", ch, obj, weapon);

        if (k->active_room)
            act(k->active_room, "Кмпп", ch, obj, weapon);
        else
            act("$1и разбил1(,а,о,и) @1в, ударив н@1ед @2т.", "Кмпп", ch, obj, weapon);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Разбить @1в? Ни за что!", "Мп", ch, obj);
        act("$1+и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, obj);
    }
}

ACMD(do_item_enter)
{
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH], turn[MAX_STRING_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *found_char = NULL;

    skip_spaces(&argument);
    argument = two_arguments(argument, arg, turn);

    if (!*arg) {
        send_to_charf(ch, "Куда именно Вы хотели войти?\r\n");
        return;
    }

    if (is_abbrev(arg, "в") || is_abbrev(arg, "in")) {
        strcpy(arg, turn);
        strcpy(turn, argument);
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }


    if (GET_OBJ_TYPE(obj) == ITEM_TRANSPORT)
        go_enter(ch, obj);
    else {
        if ((k = find_operations(obj, ch, CMD_ENTER))) {
            if (*turn && k->arg && !isname(k->arg, turn)) {
                send_to_charf(ch, "turn %s arg %s\r\n", turn, k->arg);
                act("Вы попытались войти в @1в, но не смогли.", "Мп", ch, obj);
                act("$1и попытал1(ся,ась,ось,ись) войти в @1в, но не смог1(,ла,ло,ли).", "Кмп", ch,
                    obj);
                return;
            }

            if (k->active)
                act(k->active, "Мп", ch, obj);
            else
                act("Вы вошли в @1в.", "Мп", ch, obj);

            if (k->active_room)
                act(k->active_room, "Кмп", ch, obj);
            else
                act("$1и вош1(ел,ла,ло,ли) в @1в.", "Кмп", ch, obj);

            post_exec(ch, obj, NULL, k);
            return;
        } else {
            act("Войти в @1в? Вряд ли это возможно.", "Мп", ch, obj);
            act("$1и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, obj);
        }
    }
}

ACMD(do_item_dig)
{
    struct char_data *found_char = NULL;
    struct obj_data *shovel =
        GET_EQ(ch, WEAR_WIELD) ? GET_EQ(ch,
                                        WEAR_WIELD) : (GET_EQ(ch, WEAR_BOTHS) ? GET_EQ(ch,
                                                                                       WEAR_BOTHS) :
                                                       NULL);
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];


    if (!shovel || (shovel && !IS_SHOVEL(shovel))) {
        send_to_charf(ch, "Прежде чем начнете копать, возьмите в руки лопату.\r\n");
        return;
    }

    if (!*argument) {
        go_dig(ch);
        return;
    }

    skip_spaces(&argument);
    argument = one_argument(argument, arg);


    if (!*arg) {
        send_to_charf(ch, "Что именно Вы хотели выкопать?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не можете выкопать здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_DIG, GET_OBJ_VNUM(shovel)))) {
        if (k->obj_vnum != GET_OBJ_VNUM(shovel)) {
            act("Вы попытались копать, но ничего не вышло.", "Мпп", ch, shovel, obj);
            act("$1и попытал1(ся,ась,ось,ись) копать, но у н1ер ничего не вышло.", "Мпп", ch,
                shovel, obj);
            return;
        }

        if (k->active)
            act(k->active, "Мпп", ch, shovel, obj);
        else
            act("Энергично засучив рукава, Вы принялись копать.", "Мпп", ch, shovel, obj);

        if (k->active_room)
            act(k->active_room, "Кмпп", ch, shovel, obj);
        else
            act("Энергично засучив рукава, $1и принял1(ся,ась,ось,ись) копать.", "Кмпп", ch, shovel,
                obj);

        post_exec(ch, obj, shovel, k);
        return;
    } else {
        act("Копать здесь? Да в своем ли Вы уме?", "М", ch);
        act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Км", ch);
    }
}

ACMD(do_item_pull)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Что именно Вы хотели потянуть?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_PULL))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Вы слегка потянули @1в на себя.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("$1и слегка потянул1(,а,о,и) @1в на себя.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Даже и не пытайтесь тянуть @1в, ничего из этого не выйдет.", "Мп", ch, obj);
        act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Кмп", ch, obj);
    }
}


ACMD(do_item_dive)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (is_abbrev(arg, "в") || is_abbrev(arg, "in"))
        strcpy(arg, argument);

    if (!*arg) {
        send_to_charf(ch, "Куда именно Вы хотели нырнуть?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_DIVE))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Вы нырнули в @1в.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("$1и нырннул1(,а,о,и) в @1в.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Нырнуть в @1в? Вряд ли это возможно.", "Мп", ch, obj);
        act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Кмп", ch, obj);
    }
}

ACMD(do_item_ring)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (is_abbrev(arg, "в") || is_abbrev(arg, "in"))
        strcpy(arg, argument);

    if (!*arg) {
        send_to_charf(ch, "Во что именно Вы хотели звонить?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_RING))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Вы энергично позвонили в @1в.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("$1и энергично позвонил1(,а,о,и) в @1в.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Даже и не пытайтесь звонить в @1в, ничего из этого не выйдет.", "Мп", ch, obj);
        act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Кмп", ch, obj);
    }
}

ACMD(do_item_climb)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (is_abbrev(arg, "в") || is_abbrev(arg, "in") || is_abbrev(arg, "на") || is_abbrev(arg, "at"))
        strcpy(arg, argument);

    if (!*arg) {
        send_to_charf(ch, "Куда именно Вы хотели залезть?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if (IS_BARIAUR(ch)) {
        send_to_charf(ch, "Что? Лезть? Вы же не обезьяна, а бариур!\r\n");
        return;
    }

    if ((k = find_operations(obj, ch, CMD_CLIMB))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else if (IS_OBJ_STAT(obj, ITEM_EXTERN))
            act("Вы залезли в @1в.", "Мп", ch, obj);
        else
            act("Вы полезли по @1д.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else if (IS_OBJ_STAT(obj, ITEM_EXTERN))
            act("$1и залез1(,ла,ло,ли) в @1в.", "Кмп", ch, obj);
        else
            act("$1и залез1(,ла,ло,ли) по @1д.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Залезть в @1в? Вряд ли это возможно.", "Мп", ch, obj);
        act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Кмп", ch, obj);
    }
}


ACMD(do_item_jumpdown)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (is_abbrev(arg, "c") || is_abbrev(arg, "with"))
        strcpy(arg, argument);

    if (!*arg) {
        send_to_charf(ch, "Откуда Вы хотели бы спрыгнуть?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_JUMPDOWN))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Вы спрыгнули с @1р.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("$1и спрыгнул1(,а,о,и) с @1р.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Спрыгнуть с @1р? Вряд ли это возможно.", "Мп", ch, obj);
        act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Кмп", ch, obj);
    }
}

ACMD(do_item_jump)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (is_abbrev(arg, "на") || is_abbrev(arg, "to"))
        strcpy(arg, argument);

    if (!*arg) {
        send_to_charf(ch, "Куда Вы хотели бы запрыгнуть?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_JUMP))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Вы запрыгнули на @1в.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("$1и запрыгнул1(,а,о,и) на @1в.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Запрыгнуть на @1в? Вряд ли это возможно.", "Мп", ch, obj);
        act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Кмп", ch, obj);
    }
}

ACMD(do_item_break)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Что именно Вы хотели сломать?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_BREAK))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Вы сломали @1в.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("$1и сломал1(,а,о,и) @1в.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else
        go_break(ch, obj);
}



ACMD(do_item_crack)
{
    struct char_data *found_char = NULL;
    struct obj_data *weapon =
        GET_EQ(ch, WEAR_WIELD) ? GET_EQ(ch,
                                        WEAR_WIELD) : (GET_EQ(ch, WEAR_BOTHS) ? GET_EQ(ch,
                                                                                       WEAR_BOTHS) :
                                                       NULL);
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];


    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Что именно Вы хотели бы расколоть?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }


    if (!weapon ||
        (weapon &&
         (GET_OBJ_TYPE(weapon) != ITEM_WEAPON ||
          (GET_OBJ_TYPE(weapon) == ITEM_WEAPON &&
           real_attack_type(GET_OBJ_VAL(weapon, 3)) != HIT_SLASH)))) {
        send_to_charf(ch,
                      "Прежде, чем попытаться что-нибудь расколоть, вооружитесь рубящим оружием.\r\n");
        return;
    }

    if ((k = find_operations(obj, ch, CMD_CRACK))) {

        if (k->active)
            act(k->active, "Мпп", ch, obj, weapon);
        else
            act("Вы раскололи @1в, рубанув @ев @2т.", "Мпп", ch, obj, weapon);

        if (k->active_room)
            act(k->active_room, "Кмпп", ch, obj, weapon);
        else
            act("$1и расколол1(,а,о,и) @1в, рубанув @ев @2т.", "Кмпп", ch, obj, weapon);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Расколоть @1в? Ни за что!", "Мп", ch, obj);
        act("$1+и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, obj);
    }
}

ACMD(do_item_clear)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Что именно Вы хотели бы расчистить?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_CLEAR))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Вы осторожно расчистили поверхность @1р.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("$1и осторожно расчистил1(,а,о,и) поверхность @1р.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Расчистить поверхность @1р? Но зачем?", "Мп", ch, obj);
        act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Кмп", ch, obj);
    }
}

ACMD(do_item_erase)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Что именно Вы хотели бы стереть?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_ERASE))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Вы осторожно стерли @1р.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("$1и осторожно стерли @1р.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Стереть @1р? Но зачем?", "Мп", ch, obj);
        act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Кмп", ch, obj);
    }
}

ACMD(do_item_burrow)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (is_abbrev(arg, "в") || is_abbrev(arg, "in"))
        strcpy(arg, argument);

    if (!*arg) {
        send_to_charf(ch, "Где именно Вы собирались рыться?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_BURROW))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Вы принялись рыться в @1п.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("$1и принял1(ся,ась,ось,ись) рыться в @1п.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Рыться в @1п? Но зачем?", "Мп", ch, obj);
        act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Кмп", ch, obj);
    }
}


ACMD(do_item_knock)
{

    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL, *tobj = NULL;
    struct item_op_data *k = NULL;
    char name[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];


    if (!*argument) {
        send_to_charf(ch, "Куда Вы хотите постучать?\r\n");
        return;
    }

    skip_spaces(&argument);
    argument = two_arguments(argument, name, arg);

    if (!*name) {
        send_to_charf(ch, "Постучать куда?\r\n");
        return;
    }

    if (is_abbrev(name, "по") || is_abbrev(name, "to"))
        strcpy(name, arg);

    generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", name);
        return;
    }

    if (*arg) {
        if (is_abbrev(arg, "по") || is_abbrev(arg, "to"))
            strcpy(arg, argument);

        generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &tobj);
        if (!tobj) {
            send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
            return;
        }

        if ((k = find_operations(obj, ch, CMD_KNOCK, GET_OBJ_VNUM(tobj)))) {
            if (k->obj_vnum != GET_OBJ_VNUM(tobj)) {
                act("Вы постучали @1т по @2д, но ничего не произошло.", "Мпп", ch, obj, tobj);
                act("$1и постучал1(,а,о,и) @1т по @2д, но ничего не произошло.", "Мпп", ch, obj,
                    tobj);
                return;
            }
            if (k->active)
                act(k->active, "Мпп", ch, obj, tobj);
            else
                act("Вы постучали @1т по @2д.", "Мпп", ch, obj, tobj);
            if (k->active_room)
                act(k->active_room, "Кмпп", ch, obj, tobj);
            else
                act("$1и постучал1(,а,о,и) @1т по @2д.", "Кмпп", ch, obj, tobj);

            post_exec(ch, obj, tobj, k);
            return;
        } else {
            act("Постучать @1т по @2д? Забудьте об этом!", "Мпп", ch, obj, tobj);
            act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Кмп", ch, obj);
        }
    } else {
        if ((k = find_operations(obj, ch, CMD_KNOCK))) {
            if (k->active)
                act(k->active, "Мпп", ch, obj, tobj);
            else
                act("Вы постучали по @1д.", "Мпп", ch, obj, tobj);
            if (k->active_room)
                act(k->active_room, "Кмпп", ch, obj, tobj);
            else
                act("$1и постучал1(,а,о,и) по @1д.", "Кмпп", ch, obj, tobj);

            post_exec(ch, obj, tobj, k);
            return;
        } else {
            act("Постучать по @1д? Забудьте об этом!", "Мпп", ch, obj);
            act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Км", ch);
        }
    }
}

ACMD(do_item_catchon)
{
    struct char_data *found_char = NULL;
    struct obj_data *obj = NULL;
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (is_abbrev(arg, "за") || is_abbrev(arg, "to"))
        strcpy(arg, argument);

    if (!*arg) {
        send_to_charf(ch, "За какой предмет Вы хотели бы зацепиться?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if ((k = find_operations(obj, ch, CMD_PRESS))) {
        if (k->active)
            act(k->active, "Мп", ch, obj);
        else
            act("Вы зацепились за @1в.", "Мп", ch, obj);

        if (k->active_room)
            act(k->active_room, "Кмп", ch, obj);
        else
            act("$1и зацепил1(,а,о,и) за @1в.", "Кмп", ch, obj);

        post_exec(ch, obj, NULL, k);
        return;
    } else {
        act("Зацепиться за @1в? Вряд ли это возможно.", "Мп", ch, obj);
        act("$1и смерил1(,а,о,и) взглядом @1в и тяжело вздохнул1(,а,о,и).", "Кмп", ch, obj);
    }

}

ACMD(do_item_sprinkle)
{
    struct char_data *victim = NULL;
    struct obj_data *obj = NULL, *tobj = NULL;
    struct item_op_data *k = NULL;
    char name[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];


    if (!*argument) {
        send_to_charf(ch, "Что именно и чем Вы хотели бы окропить?\r\n");
        return;
    }

    skip_spaces(&argument);
    argument = two_arguments(argument, name, arg);

    if (!*name) {
        send_to_charf(ch, "Чем Вы хотите окропить?\r\n");
        return;
    }

    generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", name);
        return;
    }

    if (!*arg) {
        act("Что именно Вы хотели бы окропить?", "Мп", ch, obj);
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM, ch, &victim, &tobj);


    if (!tobj && !victim) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if (tobj) {
        if ((k = find_operations(obj, ch, CMD_SPRINKLE, GET_OBJ_VNUM(tobj)))) {
            if (k->obj_vnum != GET_OBJ_VNUM(tobj)) {
                act("Вы окропили @2в жидкостью из @2р, но ничего не произошло.", "Мпп", ch, obj,
                    tobj);
                act("$1и попытал1(ся,ась,ись,ось) окропить @2в жидкостью из @1р.", "Кмпп", ch, obj,
                    tobj);
                return;
            }
            if (k->active)
                act(k->active, "Мпп", ch, obj, tobj);
            else
                act("Вы окропили @2в жидкостью из @1р.", "Мпп", ch, obj, tobj);
            if (k->active_room)
                act(k->active_room, "Кмпп", ch, obj, tobj);
            else
                act("$1и окропил1(,а,о,и) @2в жидкостью из @1р.", "Кмпп", ch, obj, tobj);

            post_exec(ch, obj, tobj, k);
            return;
        } else {
            act("Окропить @2в? Вряд ли это возможно.", "Мпп", ch, obj, tobj);
            act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Км", ch);
        }
    } else {
        if ((k = find_operations(obj, ch, CMD_SPRINKLE, 0, GET_MOB_VNUM(victim)))) {
            if (k->mob_vnum != GET_MOB_VNUM(victim)) {
                act("Вы попытались окропить $2в жидкостью из @1р, но не смогли.", "Ммп", ch, victim,
                    obj);
                act("$1и попытал1(ся,ась,ись,ось) окропить $2в жидкостью из @1р.", "Кммп", ch,
                    victim, obj);
                return;
            }

            if (k->active)
                act(k->active, "Ммп", ch, victim, obj);
            else
                act("Вы окропили $1в жидкостью из @1р.", "Ммп", ch, victim, obj);

            if (k->active_room)
                act(k->active_room, "Кммп", ch, victim, obj);
            else
                act("$1и окропил1(,а,о,и) $1в жидкостью из @1р.", "Кммп", ch, victim, obj);

            post_exec_victim(ch, obj, victim, k);
            return;
        } else {
            act("Окропить $2в? Вряд ли это возможно.", "Ммп", ch, victim, obj);
            act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Км", ch);
        }
    }

}


ACMD(do_item_smear)
{
    struct char_data *victim = NULL;
    struct obj_data *obj = NULL, *tobj = NULL;
    struct item_op_data *k = NULL;
    char name[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];


    if (!*argument) {
        send_to_charf(ch, "Что именно и чем Вы хотели бы намазать?\r\n");
        return;
    }

    skip_spaces(&argument);
    argument = two_arguments(argument, name, arg);

    if (!*name) {
        send_to_charf(ch, "Чем Вы хотите намазать?\r\n");
        return;
    }

    generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", name);
        return;
    }

    if (!*arg) {
        act("Что именно Вы хотели бы намазать?", "Мп", ch, obj);
        return;
    }

    generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_CHAR_ROOM, ch, &victim, &tobj);

    if (!tobj && !victim) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if (tobj) {
        if ((k = find_operations(obj, ch, CMD_SPRINKLE, GET_OBJ_VNUM(tobj)))) {
            if (k->obj_vnum != GET_OBJ_VNUM(tobj)) {
                act("Вы намазали @2в составом из @2р, но ничего не произошло.", "Мпп", ch, obj,
                    tobj);
                act("$1и попытал1(ся,ась,ись,ось) намазать @2в составом из @1р.", "Кмпп", ch, obj,
                    tobj);
                return;
            }
            if (k->active)
                act(k->active, "Мпп", ch, obj, tobj);
            else
                act("Вы намазали @2в составом из @1р.", "Мпп", ch, obj, tobj);
            if (k->active_room)
                act(k->active_room, "Кмпп", ch, obj, tobj);
            else
                act("$1и намазал1(,а,о,и) @2в составом из @1р.", "Кмпп", ch, obj, tobj);

            post_exec(ch, obj, tobj, k);
            return;
        } else {
            act("Намазать @2в? Вряд ли это возможно.", "Мпп", ch, obj, tobj);
            act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Км", ch);
        }
    } else {
        if ((k = find_operations(obj, ch, CMD_SPRINKLE, 0, GET_MOB_VNUM(victim)))) {
            if (k->mob_vnum != GET_MOB_VNUM(victim)) {
                act("Вы попытались намазать $2в составом из @1р, но не смогли.", "Ммп", ch, victim,
                    obj);
                act("$1и попытал1(ся,ась,ись,ось) намазать $2в составом из @1р.", "Кммп", ch,
                    victim, obj);
                return;
            }

            if (k->active)
                act(k->active, "Ммп", ch, victim, obj);
            else
                act("Вы намазали $1в составом из @1р.", "Ммп", ch, victim, obj);

            if (k->active_room)
                act(k->active_room, "Кммп", ch, victim, obj);
            else
                act("$1и намазал1(,а,о,и) $1в составом из @1р.", "Кммп", ch, victim, obj);

            post_exec_victim(ch, obj, victim, k);
            return;
        } else {
            act("Намазать $2в? Вряд ли это возможно.", "Ммп", ch, victim, obj);
            act("$1и о чем-то глубоко задумал1(ся,ась,ось,ись).", "Км", ch);
        }
    }

}

ACMD(do_action)
{
    struct item_op_data *k = NULL;
    char arg[MAX_STRING_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *found_char = NULL;

    skip_spaces(&argument);
    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Действия с каким предметом Вы хотите посмотреть?\r\n");
        return;
    }

    generic_find(arg, FIND_OBJ_EQUIP | FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &found_char, &obj);

    if (!obj) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if (obj->operations) {
        send_to_charf(ch, "Вы можете выполнить следующие действия:\r\n");
        for (k = obj->operations; k; k = k->next) {
            int num_cmd = k->command - 100;

            send_to_charf(ch, " %s\r\n", action_name[num_cmd]);
        }
    }

}

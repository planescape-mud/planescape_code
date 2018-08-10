/* ************************************************************************
*   File: act.item.c                                    Part of CircleMUD *
*  Usage: object handling routines -- get/drop and container handling     *
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
#include "db.h"
#include "spells.h"
#include "constants.h"
#include "case.h"
#include "screen.h"
#include "events.h"
#include "xenchant.h"
#include "xboot.h"

/* local functions */
bool find_enchant_components(struct char_data *ch, int num);
void remove_enchant_components(struct char_data *ch, int num);
int _find_door(struct char_data *ch, const char *type, char *dir);
int a_find_door(struct char_data *ch, const char *type, char *dir);
int can_take_obj(struct char_data *ch, struct obj_data *obj);
int perform_get_from_room(struct char_data *ch, struct obj_data *obj);
void get_from_room(struct char_data *ch, char *arg, int amount);
void perform_give_gold(struct char_data *ch, struct char_data *vict, int amount);
void perform_give(struct char_data *ch, struct char_data *vict, struct obj_data *obj);
int perform_drop(struct char_data *ch, struct obj_data *obj);
void perform_drop_gold(struct char_data *ch, int amount);
struct char_data *give_find_vict(struct char_data *ch, char *arg);
void weight_change_object(struct obj_data *obj, int weight);
void perform_put(struct char_data *ch, struct obj_data *obj, struct obj_data *cont);
void get_from_container(struct char_data *ch, struct obj_data *cont, char *arg, int mode,
                        int amount);
void wear_message(struct char_data *ch, struct obj_data *obj, int where);
void perform_wear(struct char_data *ch, struct obj_data *obj, int where);
int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg);
void perform_get_from_container(struct char_data *ch, struct obj_data *obj, struct obj_data *cont,
                                int mode);
void perform_remove(struct char_data *ch, int pos);


void perform_put(struct char_data *ch, struct obj_data *obj, struct obj_data *cont)
{
    if (GET_OBJ_COUNTITEMS(cont) >= GET_OBJ_VAL(cont, 0) / 100)
        act("В @1п нет свободного места.", "Мп", ch, cont);
    else if (GET_OBJ_WEIGHT(cont) + GET_OBJ_WEIGHT(obj) > GET_OBJ_VAL(cont, 0))
        act("@1и не помеща@1(ется,ется,ется,ются) в @2в.", "Мпп", ch, obj, cont);
    else if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER)
        act("Невозможно положить @1в в @2в.", "Мпп", ch, obj, cont);
    else if (IS_OBJ_STAT(obj, ITEM_NODROP))
        act("Неведомая сила помешала Вам положить @1д в @2в.", "Мпп", ch, obj, cont);
    else if (cont->obj_flags.bnotfit[(int) GET_OBJ_TYPE(obj)])
        act("@1и не помещается в @2в.", "Мпп", ch, obj, cont);
    else {
        if (IS_OBJ_STAT(cont, ITEM_EXTERN))
            act("1+и положил1(,а,о,и) @1+в на @2+в.", "Кмпп", ch, obj, cont);
        else
            act("1+и положил1(,а,о,и) @1+в в @2+в.", "Кмпп", ch, obj, cont);


        /* Yes, I realize this is strange until we have auto-equip on rent. -gg */
        if (IS_OBJ_STAT(obj, ITEM_NODROP) && !IS_OBJ_STAT(cont, ITEM_NODROP)) {
            SET_BIT(GET_OBJ_EXTRA(cont, ITEM_NODROP), ITEM_NODROP);
            act("Вы почувствовали что-то странное, когда положили @1в в @2в.", "Мпп", ch, obj,
                cont);
        } else {
            if (IS_OBJ_STAT(cont, ITEM_EXTERN))
                act("Вы положили @1в на @2в.", "Мпп", ch, obj, cont);
            else
                act("Вы положили @1в в @2в.", "Мпп", ch, obj, cont);
        }
        obj_from_char(obj);
        obj_to_obj(obj, cont);
        cont->contains = SortObjects(cont->contains, SORT_NAME);
    }
}


/* The following put modes are supported by the code below:

 1) put <object> <container>
 2) put all.<object> <container>
 3) put all <container>

 <container> must be in inventory or on ground.
 all objects to be put into container must be in inventory.
*/

ACMD(do_put)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    struct obj_data *obj, *next_obj, *cont;
    struct char_data *tmp_char;
    int obj_dotmode, cont_dotmode, found = 0, howmany = 1, money_mode = FALSE;
    char *theobj, *thecont, *theplace;
    int where_bits = FIND_OBJ_INV | FIND_OBJ_EQUIP | FIND_OBJ_ROOM;

    argument = two_arguments(argument, arg1, arg2);
    argument = one_argument(argument, arg3);

    if (*arg3 && is_positive_number(arg1)) {
        howmany = atoi(arg1);
        theobj = arg2;
        thecont = arg3;
        theplace = argument;
    } else {
        theobj = arg1;
        thecont = arg2;
        theplace = arg3;
    }

    if (isname(theplace, "земля комната room ground"))
        where_bits = FIND_OBJ_ROOM;
    else if (isname(theplace, "инвентарь inventory"))
        where_bits = FIND_OBJ_INV;
    else if (isname(theplace, "экипировка equipment"))
        where_bits = FIND_OBJ_EQUIP;


    if (theobj &&
        (!strn_cmp("coin", theobj, 4) ||
         !strn_cmp("монет", theobj, 3) || !strn_cmp("зевенел", theobj, 3))) {
        money_mode = TRUE;
        if (howmany <= 0) {
            send_to_char("Следует точную сумму.\r\n", ch);
            return;
        }
        if (GET_GOLD(ch) < howmany) {
            send_to_char("Нет у Вас такой суммы.\r\n", ch);
            return;
        }
        obj_dotmode = FIND_INDIV;
    } else
        obj_dotmode = find_all_dots(theobj);

    cont_dotmode = find_all_dots(thecont);

    if (!*theobj)
        send_to_char("Положить что и куда?\r\n", ch);
    else if (cont_dotmode != FIND_INDIV)
        send_to_char("Вы можете положить вещь только в один контейнер.\r\n", ch);
    else if (!*thecont) {
        send_to_charf(ch, "Куда Вы хотите положить '%s'?\r\n", theobj);
    } else {
        generic_find(thecont, where_bits, ch, &tmp_char, &cont);
        if (!cont) {
            send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", thecont);
        } else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER && GET_OBJ_TYPE(cont) != ITEM_CORPSE)
            act("В @1в нельзя ничего положить.", "Мп", ch, cont);
        else if (OBJVAL_FLAGGED(cont, EXIT_CLOSED))
            act("@1в закрыт@1(,а,о,ы).", "Мп", ch, cont);
        else {
            if (obj_dotmode == FIND_INDIV) {    /* put <obj> <container> */
                if (money_mode) {
                    obj = create_money(howmany);
                    if (!obj)
                        return;
                    obj_to_char(obj, ch);
                    GET_GOLD(ch) -= howmany;
                    perform_put(ch, obj, cont);
                    GET_MISSED(ch)++;
                } else if (!(obj = get_obj_in_list_vis(ch, theobj, ch->carrying))) {
                    send_to_charf(ch, "У Вас нет '%s'.\r\n", theobj);
                } else if (obj == cont)
                    send_to_char("Это невозможно!.\r\n", ch);
                else {
                    struct obj_data *next_obj;

                    while (obj && countcmd--) {
                        next_obj = obj->next_content;
                        perform_put(ch, obj, cont);
                        obj = get_obj_in_list_vis(ch, theobj, next_obj);
                    }
                    GET_MISSED(ch)++;
                }
            } else {
                for (obj = ch->carrying; obj; obj = next_obj) {
                    next_obj = obj->next_content;
                    if (obj != cont && CAN_SEE_OBJ(ch, obj) &&
                        (obj_dotmode == FIND_ALL || isname(theobj, obj->name))) {
                        found = 1;
                        perform_put(ch, obj, cont);
                    }
                }
                if (!found) {
                    if (obj_dotmode == FIND_ALL)
                        send_to_char("А у Вас это есть?\r\n", ch);
                    else {
                        send_to_charf(ch, "Вы не видите ничего похожего на '%s'.\r\n", theobj);
                    }
                } else
                    GET_MISSED(ch)++;
            }
        }
    }
}



int can_take_obj(struct char_data *ch, struct obj_data *obj)
{
    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
        act("Вы не можете удержать в руках @1в.", "Мп", ch, obj);
        return (0);
    } else if ((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) > CAN_CARRY_W(ch)) {
        act("Вам слишком тяжело нести @1в.", "Мп", ch, obj);
        return (0);
    } else if (!(CAN_WEAR(obj, ITEM_WEAR_TAKE))) {
        act("Вы не можете взять @1в.", "Мп", ch, obj);
        return (0);
    }
    return (1);
}


int get_check_money(struct char_data *ch, struct obj_data *obj)
{
    int value = GET_OBJ_VAL(obj, 0);

    if (GET_OBJ_TYPE(obj) != ITEM_MONEY || value <= 0)
        return -1;

    obj_from_char(obj);
    extract_obj(obj);

    GET_GOLD(ch) += value;

    if (value > 1)
        send_to_charf(ch, "Это составило %d %s.\r\n", value, desc_count(value, WHAT_MONEYu));

    return value;
}


void perform_get_from_container(struct char_data *ch, struct obj_data *obj,
                                struct obj_data *cont, int mode)
{
    struct mess_p_data *k;
    bool found = FALSE;
    int count = 0;

    if ((mode == FIND_OBJ_INV || mode == FIND_OBJ_ROOM || mode == FIND_OBJ_EQUIP) &&
        can_take_obj(ch, obj)) {
        if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
            act("Вы не можете удержать в руках @1в.", "Мп", ch, obj);
        else
            // if (get_otrigger(obj, ch))
        {
            if (obj == NULL) {
                log("ВЫХОД");
                return;
            }
            obj_from_obj(obj);
            /* Проверка на перехват сообщений */
            for (k = obj->mess_data; k; k = k->next, count++)
                if (k->command == CMD_GET) {
                    found = TRUE;
                    break;
                }
            if (found) {
                if (k->mess_to_char)
                    act(k->mess_to_char, "Мп", ch, obj);
                if (k->mess_to_room && !obj->worn_by && !obj->carried_by)
                    act(k->mess_to_room, "Кмп", ch, obj);

                if (k->stoping) {
                    if (found && k->script)
                        go_script(k->script, ch, obj);
                    return;
                }
            } else if (IS_OBJ_STAT(cont, ITEM_EXTERN) || IS_CORPSE(cont)) {
                if ((GET_OBJ_TYPE(obj) == ITEM_ARMOR || GET_OBJ_TYPE(obj) == ITEM_WORN ||
                     GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && IS_CORPSE(cont)) {
                    act("Вы сняли @1+в c @2+р.", "Мпп", ch, obj, cont);
                    act("1+и снял1(,а,о,и) @1+в c @2+р.", "Кмпп", ch, obj, cont);
                } else {
                    act("Вы взяли @1+в c @2+р.", "Мпп", ch, obj, cont);
                    act("1+и взял1(,а,о,и) @1+в c @2+р.", "Кмпп", ch, obj, cont);
                }
            } else {
                act("Вы взяли @1+в из @2+р.", "Мпп", ch, obj, cont);
                act("1+и взял1(,а,о,и) @1+в из @2+р.", "Кмпп", ch, obj, cont);
            }
            if (found && k->script)
                go_script(k->script, ch, obj);
            if (obj != NULL) {
                if (OBJ_FLAGGED(obj, ITEM_TAKECRASH)) {
                    extract_obj(obj);
                } else {
                    obj_to_char(obj, ch);
                    GET_WAIT(ch) += MAX(1, GET_OBJ_WEIGHT(obj) / 500);
                }
                if (obj->carried_by == ch)
                    get_check_money(ch, obj);
            }
        }
    }
}


void get_from_container(struct char_data *ch, struct obj_data *cont,
                        char *arg, int mode, int howmany)
{
    struct obj_data *obj, *next_obj;
    int obj_dotmode, found = 0;

    obj_dotmode = find_all_dots(arg);
    if (OBJVAL_FLAGGED(cont, EXIT_CLOSED))
        act("@1 закрыт@1(,а,о,ы).", "Мп", ch, cont);
    else if (obj_dotmode == FIND_INDIV) {
        if (!(obj = get_obj_in_list_vis(ch, arg, cont->contains)))
            act("В @1п нет '%1'.", "Мпт", ch, cont, arg);
        else {
            struct obj_data *obj_next;

            while (obj && howmany--) {
                obj_next = obj->next_content;
                perform_get_from_container(ch, obj, cont, mode);
                obj = get_obj_in_list_vis(ch, arg, obj_next);
            }
        }
    } else {
        if (obj_dotmode == FIND_ALLDOT && !*arg) {
            send_to_char("Взять что \"все\" ?\r\n", ch);
            return;
        }
        for (obj = cont->contains; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (CAN_SEE_OBJ(ch, obj) && !OBJ_FLAGGED(obj, ITEM_NOVISIBLE) &&
                (obj_dotmode == FIND_ALL || isname(arg, obj->name))) {
                found = 1;
                perform_get_from_container(ch, obj, cont, mode);
            }
        }
        if (!found) {
            if (obj_dotmode == FIND_ALL)
                act("В @1п пусто.", "Мп", ch, cont);
            else
                act("В @1п нет '%1'.", "Мпт", ch, cont, arg);
        }
    }
}


int perform_get_from_room(struct char_data *ch, struct obj_data *obj)
{
    struct mess_p_data *k;
    bool found = FALSE;
    int count = 0;

    /* Проверка на перехват сообщений */
    for (k = obj->mess_data; k; k = k->next, count++)
        if (k->command == CMD_GET) {
            found = TRUE;
            break;
        }

    if (found && k->stoping) {
        if (k->mess_to_char)
            act(k->mess_to_char, "Мп", ch, obj);
        if (k->mess_to_room && !obj->worn_by && !obj->carried_by)
            act(k->mess_to_room, "Кмп", ch, obj);

        if (found && k->script)
            go_script(k->script, ch, obj);
        return (0);
    }


    if (can_take_obj(ch, obj)) {
        if (GET_OBJ_TYPE(obj) == ITEM_TRAP) {
            act("Вы поранились при попытке взять @1 в руки.", "Мп", ch, obj);
            act("1+и поранил1(ся,ась,ось,ись) при попытке взять @+1 в руки.", "Кмп", ch, obj);
            int dam = dice(1, 12) + (GET_OBJ_VAL(obj, 0) / 10);

            damage_obj(ch, obj, dam, 0, FALSE);
            return (0);
        }


        obj_from_room(obj);
        if (found) {
            if (k->mess_to_char)
                act(k->mess_to_char, "Мп", ch, obj);
            if (k->mess_to_room && !obj->worn_by && !obj->carried_by)
                act(k->mess_to_room, "Кмп", ch, obj);
        } else {
            act("Вы взяли @1в.", "Мп", ch, obj);
            act("1+и взял1(,а,о,и) @1+в.", "Кмп", ch, obj);
        }

        if (found && k->script)
            go_script(k->script, ch, obj);

        obj_to_char(obj, ch);
        GET_WAIT_STATE(ch) += MAX(1, GET_OBJ_WEIGHT(obj) / 1000);
        if (obj->carried_by == ch) {
            get_check_money(ch, obj);
            return (1);
        }
    }
    return (0);
}


void get_from_room(struct char_data *ch, char *arg, int howmany)
{
    struct obj_data *obj, *next_obj;
    int dotmode, found = 0;

    dotmode = find_all_dots(arg);

    if (dotmode == FIND_INDIV) {
        if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents)))
            send_to_charf(ch, "Здесь нет '%s'.\r\n", arg);
        else {
            struct obj_data *obj_next;

            while (obj && howmany--) {
                obj_next = obj->next_content;
                perform_get_from_room(ch, obj);
                obj = get_obj_in_list_vis(ch, arg, obj_next);
            }
        }
    } else {
        if (dotmode == FIND_ALLDOT && !*arg) {
            send_to_char("Взять что \"все\" ?\r\n", ch);
            return;
        }
        for (obj = world[ch->in_room].contents; obj; obj = next_obj) {
            next_obj = obj->next_content;
            if (CAN_SEE_OBJ(ch, obj) && !OBJ_FLAGGED(obj, ITEM_NOVISIBLE) &&
                (dotmode == FIND_ALL || isname(arg, obj->name))) {
                found = 1;
                perform_get_from_room(ch, obj);
            }
        }
        if (!found) {
            if (dotmode == FIND_ALL)
                send_to_char("Похоже, здесь ничего нет.\r\n", ch);
            else
                send_to_charf(ch, "Здесь нет '%s'.\r\n", arg);
        }
    }
}

ACMD(do_mark)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];

    int cont_dotmode, found = 0, mode;
    struct obj_data *cont;
    struct char_data *tmp_char;

    if (!check_fight_command(ch))
        return;

    argument = two_arguments(argument, arg1, arg2);

    if (!*arg1)
        send_to_char("Что Вы хотите маркировать?\r\n", ch);
    else if (!*arg2 || !is_positive_number(arg2))
        send_to_char("Не указан или неверный маркер.\r\n", ch);
    else {
        cont_dotmode = find_all_dots(arg1);
        if (cont_dotmode == FIND_INDIV) {
            mode =
                generic_find(arg1, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char,
                             &cont);
            if (!cont) {
                send_to_charf(ch, "У Вас нет '%s'.\r\n", arg1);
                return;
            }
            cont->obj_flags.Obj_owner = atoi(arg2);
            act("Вы пометили @1в.", "Мп", ch, cont);
        } else {
            if (cont_dotmode == FIND_ALLDOT && !*arg1) {
                send_to_char("Пометить что \"все\" ?\r\n", ch);
                return;
            }
            for (cont = ch->carrying; cont; cont = cont->next_content)
                if (CAN_SEE_OBJ(ch, cont) && (cont_dotmode == FIND_ALL || isname(arg1, cont->name))
                    ) {
                    cont->obj_flags.Obj_owner = atoi(arg2);
                    act("Вы пометили @1в.", "Мп", ch, cont);
                    found = TRUE;
                }
            for (cont = world[ch->in_room].contents; cont; cont = cont->next_content)
                if (CAN_SEE_OBJ(ch, cont) && (cont_dotmode == FIND_ALL || isname(arg2, cont->name))
                    ) {
                    cont->obj_flags.Obj_owner = atoi(arg2);
                    act("Вы пометили @1в.", "Мп", ch, cont);
                    found = TRUE;
                }
            if (!found) {
                if (cont_dotmode == FIND_ALL)
                    send_to_char("Вы не смогли найти ничего для маркировки.\r\n", ch);
                else
                    send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg1);
            }
        }
    }
}


ACMD(do_get)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char arg3[MAX_INPUT_LENGTH];
    char *theplace;
    int where_bits = FIND_OBJ_ROOM | FIND_OBJ_INV | FIND_OBJ_EQUIP;
    int cont_dotmode, found = 0, mode;
    struct obj_data *cont;
    struct char_data *tmp_char;

    argument = two_arguments(argument, arg1, arg2);
    argument = one_argument(argument, arg3);
    theplace = argument;

    /* COMMENTED BY HMEPAS 04.02.02
       BECAUSE THIS TEST EXIST IN ANOTHER FUNCTION
       if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
       send_to_char("У Вас заняты руки!\r\n", ch);
       else
       END OF CHANGING */

    if (!*arg1)
        send_to_char("Что хотите взять?\r\n", ch);
    else if (!*arg2) {
        for (int i = 1; i <= countcmd; i++)
            get_from_room(ch, arg1, 1);
        GET_MISSED(ch)++;
    } else if (is_positive_number(arg1) && !*arg3) {
        get_from_room(ch, arg2, atoi(arg1));
        GET_MISSED(ch)++;
    } else {
        int amount = 1;

        if (is_positive_number(arg1)) {
            amount = atoi(arg1);
            strcpy(arg1, arg2);
            strcpy(arg2, arg3);
        } else
            theplace = arg3;
        if (isname(theplace, "земля комната room ground"))
            where_bits = FIND_OBJ_ROOM;
        else if (isname(theplace, "инвентарь inventory"))
            where_bits = FIND_OBJ_INV;
        else if (isname(theplace, "экипировка equipment"))
            where_bits = FIND_OBJ_EQUIP;

        cont_dotmode = find_all_dots(arg2);
        if (cont_dotmode == FIND_INDIV) {
            for (int i = 1; i <= countcmd; i++) {
                mode = generic_find(arg2, where_bits, ch, &tmp_char, &cont);
                if (!cont) {
                    send_to_charf(ch, "Вы не видите '%s'.\r\n", arg2);
                    return;
                } else if (GET_OBJ_TYPE(cont) != ITEM_CONTAINER && !IS_CORPSE(cont)) {
                    act("@1 не контейнер.", "Мп", ch, cont);
                    return;
                } else
                    get_from_container(ch, cont, arg1, mode, amount);
            }
        } else {
            if (cont_dotmode == FIND_ALLDOT && !*arg2) {
                send_to_char("Взять все из чего?\r\n", ch);
                return;
            }
            for (cont = ch->carrying;
                 cont && IS_SET(where_bits, FIND_OBJ_INV); cont = cont->next_content)
                if (CAN_SEE_OBJ(ch, cont) && !OBJ_FLAGGED(cont, ITEM_NOVISIBLE) &&
                    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))
                    ) {
                    if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER || IS_CORPSE(cont)) {
                        found = 1;
                        get_from_container(ch, cont, arg1, FIND_OBJ_INV, amount);
                        GET_MISSED(ch)++;
                    } else if (cont_dotmode == FIND_ALLDOT) {
                        found = 1;
                        act("@1 не контейнер.", "Мп", ch, cont);
                    }
                }
            for (cont = world[ch->in_room].contents;
                 cont && IS_SET(where_bits, FIND_OBJ_ROOM); cont = cont->next_content)
                if (CAN_SEE_OBJ(ch, cont) && !OBJ_FLAGGED(cont, ITEM_NOVISIBLE) &&
                    (cont_dotmode == FIND_ALL || isname(arg2, cont->name))
                    ) {
                    if (GET_OBJ_TYPE(cont) == ITEM_CONTAINER || IS_CORPSE(cont)) {
                        get_from_container(ch, cont, arg1, FIND_OBJ_ROOM, amount);
                        GET_MISSED(ch)++;
                        found = 1;
                    } else if (cont_dotmode == FIND_ALLDOT) {
                        act("@1 не контейнер.", "Мп", ch, cont);
                        found = 1;
                    }
                }
            if (!found) {
                if (cont_dotmode == FIND_ALL)
                    send_to_char("Вы не смогли найти ни одного контейнера.\r\n", ch);
                else
                    send_to_charf(ch, "Вы что-то не видите здесь '%s'.\r\n", arg2);
            }
        }
    }
}


void perform_drop_gold(struct char_data *ch, int amount)
{
    struct obj_data *obj;

    if (amount <= 0)
        send_to_char("Это как?\r\n", ch);
    else if (GET_GOLD(ch) < amount)
        send_to_char("У Вас нет такой суммы!\r\n", ch);
    else {
        obj = create_money(amount);
        act("Вы бросили #1 монет#1(у,ы,).", "Мч", ch, amount);
        act("1и бросил1(,а,о,и) #1 монет#1(у,ы,) на землю.", "Кмч", ch, amount);
        obj_to_room(obj, ch->in_room);
        GET_GOLD(ch) -= amount;
    }
}


int perform_drop(struct char_data *ch, struct obj_data *obj)
{
    struct mess_p_data *k;

    for (k = obj->mess_data; k; k = k->next)
        if (k->command == CMD_DROP && k->stoping == TRUE) {
            if (k->mess_to_char)
                act(k->mess_to_char, "Мп", ch, obj);
            if (k->mess_to_room)
                act(k->mess_to_room, "Кмп", ch, obj);
            return 0;
        }

    if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
        act("Что-то помешало Вам бросить @1в.", "Мп", ch, obj);
        return (0);
    }

    obj_from_char(obj);

    if (GET_OBJ_TYPE(obj) == ITEM_FICTION && !OBJVAL_FLAGGED(obj, EXIT_CLOSED)) {
        act("Вы закрыли @1в и выбросили @1ер.", "Мп", ch, obj);
        act("1+и закрыл1(,а,о,и) @1+в и выбросил1(,а,о,и) @1ер.", "Кмп", ch, obj);
    } else {
        act("Вы выбросили @1в.", "Мп", ch, obj);
        act("1+и выбросил1(,а,о,и) @1+в.", "Кмп", ch, obj);
    }

    obj_to_room(obj, ch->in_room);
    if (!EXITDATA(IN_ROOM(ch), DOWN) && SECT(IN_ROOM(ch)) == SECT_WATER_SWIM) {
        act("@1и плавно опустил@1(ся,ася,ось,ись) на дно.", "КМп", ch, obj);
    }
    obj_decay(obj);
    return (0);
}



ACMD(do_drop)
{
    struct obj_data *obj, *next_obj;
    int amount = 0, multi, dotmode;
    char arg[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Что Вы хотите выбросить?\r\n");
        return;
    } else if (is_positive_number(arg)) {
        multi = atoi(arg);
        one_argument(argument, arg);
        if (!str_cmp("coins", arg) || !str_cmp("coin", arg) ||
            !str_cmp("монет", arg) || !str_cmp("звенел", arg)) {
            perform_drop_gold(ch, multi);
            GET_MISSED(ch)++;
        } else if (multi <= 0)
            send_to_char("Не имеет смысла.\r\n", ch);
        else if (!*arg)
            send_to_charf(ch, "Выбросить %d из чего?\r\n", multi);
        else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying)))
            send_to_charf(ch, "У Вас нет ничего похожего на %s.\r\n", arg);
        else {
            GET_MISSED(ch)++;
            do {
                next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
                amount += perform_drop(ch, obj);
                obj = next_obj;
            } while (obj && --multi);
        }
    } else {
        dotmode = find_all_dots(arg);
        if (dotmode == FIND_ALL) {
            if (!ch->carrying)
                send_to_char("А у Вас ничего и нет.\r\n", ch);
            else {
                for (obj = ch->carrying; obj; obj = next_obj) {
                    next_obj = obj->next_content;
                    amount += perform_drop(ch, obj);
                }
                GET_MISSED(ch)++;
            }
        } else if (dotmode == FIND_ALLDOT) {
            if (!*arg) {
                send_to_charf(ch, "Выбросить \"все\" какого типа предметов ?\r\n");
                return;
            }
            if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying)))
                send_to_charf(ch, "У Вас нет ничего похожего на '%s'.\r\n", arg);
            else
                while (obj) {
                    next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
                    amount += perform_drop(ch, obj);
                    obj = next_obj;
                }
            GET_MISSED(ch)++;
        } else {
            for (int i = 1; i <= countcmd; i++) {
                if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
                    send_to_charf(ch, "У Вас нет '%s'.\r\n", arg);
                    return;
                } else
                    amount += perform_drop(ch, obj);
            }
        }
        GET_MISSED(ch)++;
    }

}

static void mprog_bribe(struct char_data *ch, struct char_data *giver, int amount)
{
    FENIA_VOID_CALL(ch, "Bribe", "Ci", giver, amount);
    FENIA_PROTO_VOID_CALL(ch->npc(), "Bribe", "CCi", ch, giver, amount);
}

static void mprog_give(struct char_data *ch, struct char_data *giver, struct obj_data *obj)
{
    FENIA_VOID_CALL(ch, "Give", "CO", giver, obj);
    FENIA_PROTO_VOID_CALL(ch->npc(), "Give", "CCO", ch, giver, obj);
}

void perform_give(struct char_data *ch, struct char_data *vict, struct obj_data *obj)
{
    if (ch == vict) {
        if (GET_OBJ_TYPE(obj) == ITEM_MONEY) {
            act("Вы пересчитали @1в.", "Мп", ch, obj);
            act("1и пересчитал1(,а,о,и) @1в.", "Кмп", ch, obj);
            get_check_money(vict, obj);
        } else {
            act("Вы переложили @1в из руки в руку.", "Мп", ch, obj);
            act("1+и переложил1(,а,о,и) @1+в из руки в руку.", "Кмп", ch, obj);
        }
        return;
    }

    if (IS_ANIMAL(vict) ||
        IS_BIRD(vict) || IS_SLIME(vict) ||
        IS_DRAGON(vict) || IS_SNAKE(vict) ||
        IS_REPTILE(vict) || IS_INSECT(vict) || IS_CONSTRUCTION(vict) || IS_PLANT(vict))
    {
        act("2и не может принять от Вас @1в.", "Ммп", ch, vict, obj);
        get_check_money(ch, obj);
        return;
    }

    if (GET_OBJ_REAL_TEMP(obj) > 30 && !may_kill_here(ch, vict)) {
        act("2и отказал2(ся,ась,ось,ись) принять от Вас горяч@1(ий,ую,ее,ие) @1в.", "Ммп", ch, vict,
            obj);
        act("1и хотел1(,а,о,и) дать Вам горяч@1(ий,ую,ее,ие) @1в.", "мМп", ch, vict, obj);
        return;
    }

    if (PRF_FLAGGED(vict, PRF_NOGIVE) && !IS_NPC(vict)) {
        act("2и отказал2(ся,ась,ось,ись) принять от Вас @1в.", "Ммп", ch, vict, obj);
        act("1и хотел1(,а,о,и) дать Вам @1в.", "мМп", ch, vict, obj);
        get_check_money(ch, obj);
        return;
    }

    if (GET_OBJ_TYPE(obj) != ITEM_MONEY) {
        if (IS_OBJ_STAT(obj, ITEM_NODROP)) {
            act("Вы не можете передать @1в!", "Мп", ch, obj);
            return;
        }
        if (IS_CARRYING_N(vict) >= CAN_CARRY_N(vict)) {
            act("У 2р заняты руки.", "Мм", ch, vict);
            return;
        }
        if ((GET_OBJ_WEIGHT(obj) + IS_CARRYING_W(vict) + IS_WEARING_W(vict)) > MAX_MOVE_W(vict)) {
            act("2и не может нести такой вес.", "Мм", ch, vict);
            return;
        }
    }
    if (GET_OBJ_TYPE(obj) == ITEM_FICTION && !OBJVAL_FLAGGED(obj, EXIT_CLOSED)) {
        act("Вы закрыли @1в и передали @1ев 2д.", "Ммп", ch, vict, obj);
        act("1и закрыл1(,а,о,и) @1в и передал @1ев Вам.", "мМп", ch, vict, obj);
        act("1+и закрыл1(,а,о,и) @1+в и передал1(,а,о,и) @1ев 2+д.", "Кммп", ch, vict, obj);
    } else {
        act("Вы дали @1в 2д.", "Ммп", ch, vict, obj);
        act("1и дал1(,а,о,и) Вам @1в.", "мМп", ch, vict, obj);
        act("1+и дал1(,а,о,и) @1+в 2+д.", "Кммп", ch, vict, obj);
    }

    obj_from_char(obj);
    obj_to_char(obj, vict);

    if (GET_OBJ_TYPE(obj) == ITEM_MONEY) {
        int amount = get_check_money(vict, obj);

        mprog_bribe(vict, ch, amount);
    } else {
        mprog_give(vict, ch, obj);
    }
}

/* utility function for give */
struct char_data *give_find_vict(struct char_data *ch, char *arg)
{
    struct char_data *vict;

    if (!*arg) {
        send_to_char("Кому?\r\n", ch);
        return (NULL);
    } else if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_char(NOPERSON, ch);
        return (NULL);
    } else

        return (vict);
}


void perform_give_gold(struct char_data *ch, struct char_data *vict, int amount)
{
    struct obj_data *money;

    if (amount <= 0) {
        send_to_char("Конечно!\r\n", ch);
        return;
    }
    if (GET_GOLD(ch) < amount && (IS_NPC(ch) || !IS_GOD(ch))
        ) {
        send_to_char("У Вас нет столько денег!\r\n", ch);
        return;
    }
    money = create_money(amount);
    if (ch != vict || !IS_GOD(ch))
        GET_GOLD(ch) -= amount;

    obj_to_char(money, ch);
    perform_give(ch, vict, money);
}


ACMD(do_give)
{
    int amount, dotmode;
    struct char_data *vict;
    struct obj_data *obj, *next_obj;
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg);

    if (!*arg)
        send_to_char("Дать что и кому ?\r\n", ch);
    else if (is_positive_number(arg)) {
        amount = atoi(arg);
        argument = one_argument(argument, arg);
        if (!strn_cmp("coin", arg, 4) || !strn_cmp("монет", arg, 5) || !str_cmp("звенел", arg)) {
            one_argument(argument, arg);
            if ((vict = give_find_vict(ch, arg)) != NULL) {
                perform_give_gold(ch, vict, amount);
                GET_MISSED(ch)++;
            }
            return;
        } else if (!*arg) {     /* Give multiple code. */
            sprintf(buf, "Чего %d Вы хотите дать?\r\n", amount);
            send_to_char(buf, ch);
        } else if (!(vict = give_find_vict(ch, argument))) {
            return;
        } else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
            send_to_charf(ch, "У Вас нет '%s'.\r\n", arg);
        } else {
            while (obj && amount--) {
                next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
                perform_give(ch, vict, obj);
                obj = next_obj;
            }
            GET_MISSED(ch)++;
        }
    } else {
        one_argument(argument, buf1);
        if (!(vict = give_find_vict(ch, buf1)))
            return;
        dotmode = find_all_dots(arg);
        if (dotmode == FIND_INDIV) {
            if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying)))
                send_to_charf(ch, "У Вас нет '%s'.\r\n", arg);
            else {
                perform_give(ch, vict, obj);
                GET_MISSED(ch)++;
            }
        } else {
            if (dotmode == FIND_ALLDOT && !*arg) {
                send_to_char("Дать \"все\" какого типа предметов ?\r\n", ch);
                return;
            }
            if (!ch->carrying)
                send_to_char("У Вас ведь ничего нет.\r\n", ch);
            else {
                bool fnd = FALSE;

                for (obj = ch->carrying; obj; obj = next_obj) {
                    next_obj = obj->next_content;
                    if (CAN_SEE_OBJ(ch, obj) && ((dotmode == FIND_ALL || isname(arg, obj->name)))) {
                        perform_give(ch, vict, obj);
                        fnd = TRUE;
                    }
                }
                if (!fnd)
                    send_to_charf(ch, "У Вас нет '%s'.\r\n", arg);
                else
                    GET_MISSED(ch)++;
            }
        }
    }
}


void weight_change_object(struct obj_data *obj, int weight)
{
    struct obj_data *tmp_obj;
    struct char_data *tmp_ch;

    if (obj->in_room != NOWHERE) {
        GET_OBJ_WEIGHT(obj) += weight;
    } else if ((tmp_ch = obj->carried_by)) {
        obj_from_char(obj);
        GET_OBJ_WEIGHT(obj) += weight;
        obj_to_char(obj, tmp_ch);
    } else if ((tmp_obj = obj->in_obj)) {
        obj_from_obj(obj);
        GET_OBJ_WEIGHT(obj) += weight;
        obj_to_obj(obj, tmp_obj);
    } else {
        log("SYSERR: Unknown attempt to subtract weight from an object.");
    }
}



void name_from_drinkcon(struct obj_data *obj)
{
//int j;

    str_reassign_proto(obj->names, obj_proto[GET_OBJ_RNUM(obj)].names);

    str_reassign_proto(obj->short_description, obj_proto[GET_OBJ_RNUM(obj)].short_description);

// for(j=0;j < NUM_PADS;j++)
//  obj->PNames[j] = str_dup(get_name_pad(obj->names,j,PAD_OBJECT));

    str_reassign_proto(obj->PNames[0], obj_proto[GET_OBJ_RNUM(obj)].PNames[0]);

    return;
}



void name_to_drinkcon(struct obj_data *obj, int type)
{
//int j;
    char new_name[MAX_INPUT_LENGTH];

    sprintf(new_name, "%s c %s", obj_proto[GET_OBJ_RNUM(obj)].names,
            get_name_pad((char *) drinknames[type], PAD_TVR, PAD_OBJECT));

    str_reassign_proto(obj->names, obj_proto[GET_OBJ_RNUM(obj)].names, new_name);

    str_reassign_proto(obj->short_description,
                       obj_proto[GET_OBJ_RNUM(obj)].short_description,
                       get_name_pad(new_name, PAD_IMN, PAD_OBJECT));

// for(j=0;j < NUM_PADS;j++)
//  obj->PNames[j] = str_dup(get_name_pad(obj->names,j,PAD_OBJECT));

    str_reassign_proto(obj->PNames[0],
                       obj_proto[GET_OBJ_RNUM(obj)].PNames[0],
                       get_name_pad(obj->names, PAD_IMN, PAD_OBJECT));

    return;
}

bool check_char_cond(struct char_data * ch, byte type)
{
    int drunk = 0;
    struct affected_type af;
    bool result = FALSE;

    if (IS_NPC(ch))
        return result;

    if ((GET_COND(ch, THIRST) + SECS_PER_MUD_TICK) >= thirst_race[(int) GET_RACE(ch)].max_cond)
        if (type == 2)
            send_to_charf(ch, "Вы полностью утолили свою жажду.\r\n");


    if ((GET_COND(ch, FULL) + SECS_PER_MUD_TICK) >= full_race[(int) GET_RACE(ch)].max_cond) {
        if (type == 1)
            send_to_charf(ch, "Вы набили свой желудок.\r\n");
        result = true;
    }
    if (GET_COND(ch, DRUNK) >= drunk_race[(int) GET_RACE(ch)].max_cond / 2) {
        drunk = 1;
        if ((GET_COND(ch, DRUNK) + SECS_PER_MUD_TICK) >= drunk_race[(int) GET_RACE(ch)].max_cond)
            drunk = 2;
    }

    if (drunk == 1 && !affected_by_spell(ch, SPELL_DRUNKED) && !AFF_FLAGGED(ch, AFF_DRUNKED)) {
        send_to_charf(ch, "Вы почувствовали легкое головокружение.\r\n");
        af.type = find_spell_num(SPELL_DRUNKED);
        af.duration = GET_COND(ch, DRUNK);
        af.bitvector = AFF_DRUNKED;
        af.location = APPLY_HITROLL;
        af.modifier = -10;
        af.owner = GET_ID(ch);
        af.main = FALSE;
        affect_join_char(ch, &af);
    }

    if (drunk == 2) {
        send_to_charf(ch, "Вы пьяны.\r\n");
        af.type = find_spell_num(SPELL_DRUNKED);
        af.duration = GET_COND(ch, DRUNK);
        af.bitvector = AFF_DRUNKED;
        af.location = APPLY_HITROLL;
        af.modifier = -20;
        af.owner = GET_ID(ch);
        af.main = FALSE;
        affect_join_char(ch, &af);
    }

    return (result);
}

void go_drink_item(struct char_data *ch, struct obj_data *obj, int mode, int on_ground)
{
    int drink, value, valdrink, count = 0;;
    bool msg_found = FALSE;
    struct mess_p_data *k;
    struct affected_type af;
    char buf[MAX_STRING_LENGTH];

    if (!obj)
        return;
    if (GET_OBJ_TYPE(obj) != ITEM_DRINKCON && GET_OBJ_TYPE(obj) != ITEM_FOUNTAIN) {
        if (GET_OBJ_TYPE(obj) == ITEM_POTION)
            act("Из @1р пить нельзя. Попробуйте @1ев осушить.", "Мп", ch, obj);
        else
            act("Из @1р пить нельзя.", "Мп", ch, obj);
        return;
    }

    if (!(valdrink = GET_OBJ_VAL(obj, 1))) {
        name_from_drinkcon(obj);
        send_to_charf(ch, "В %s пусто.\r\n", GET_OBJ_PNAME(obj, PAD_PRD));
        return;
    }

    if (!IS_NPC(ch)
        && GET_COND(ch, DRUNK) + SECS_PER_MUD_TICK >= drunk_race[(int) GET_RACE(ch)].max_cond) {
        if (on_ground) {
            act("Вы хотели сделать еще глоток, но неудержали равновесия и упали рядом с @1т.", "Мп",
                ch, obj);
            act("1+и хотел1(,а,о,и) сделать еще глоток, но неудержав равновесия, упал1(,а,о,и) рядом с @1+т.", "Кмп", ch, obj);
            GET_POS(ch) = POS_SITTING;
        } else {
            act("Вы хотели сделать еще глоток, но только выронили @1в из рук.", "Мп", ch, obj);
            act("1+и попытал1(ся,ась,ось,ись) выпить еще, но не удежрал1(,а,о,и) @1+в и выронил1(,а,о,и) @1ев из рук.", "Кмп", ch, obj);
            obj_from_char(obj);
            obj_to_room(obj, IN_ROOM(ch));
        }
        return;
    }
    /* Проверка на перехват сообщений */
    for (k = obj->mess_data; k; k = k->next, count++)
        if (k->command == CMD_DRINK) {
            msg_found = TRUE;
            break;
        }

    if (msg_found && k->stoping) {
        if (k->mess_to_char)
            act(k->mess_to_char, "М", ch);
        if (k->mess_to_room)
            act(k->mess_to_room, "Км", ch);
        return;
    }

    drink = GET_OBJ_VAL(obj, 2);

//значение жидкости сколько вообще в состоянии потребить персонаж
//из фонтана пытаемся напиться полностью

    if (GET_OBJ_TYPE(obj) == ITEM_FOUNTAIN)
        value = thirst_race[(int) GET_RACE(ch)].max_cond;
    else
        value = thirst_race[(int) GET_RACE(ch)].max_cond / 2;
    value = MIN(value, valdrink);
//send_to_charf(ch,"Мой предел на половину %d\r\n",value);

    //значение жидкости сколько в стостоянии выпить сейчас персонаж
    if (!IS_NPC(ch)) {
        value = MIN(value, thirst_race[(int) GET_RACE(ch)].max_cond - GET_COND(ch, THIRST));
        //send_to_charf(ch,"В состоянии %d\r\n",value);

        if (value <= SECS_PER_MUD_TICK && !IS_GOD(ch) && valdrink > SECS_PER_MUD_TICK) {
            send_to_charf(ch, "Ваш желудок не выдержит столько жидкости.\r\n");
            return;
        }

        if (drink_aff[drink][THIRST] && GET_COND(ch, THIRST) > -1)
            GET_COND(ch, THIRST) += (value * drink_aff[drink][THIRST]) / 100;
        if (drink_aff[drink][DRUNK] && GET_COND(ch, DRUNK) > -1)
            GET_COND(ch, DRUNK) += (value * drink_aff[drink][DRUNK]) / 100;
        if (drink_aff[drink][FULL] && GET_COND(ch, FULL) > -1)
            GET_COND(ch, FULL) += (value * drink_aff[drink][FULL]) / 100;
    }

    GET_OBJ_VAL(obj, 1) = MAX(0, GET_OBJ_VAL(obj, 1) - value);

    if (msg_found) {
        if (k->mess_to_char) {
            sprintf(buf, k->mess_to_char,
                    get_name_pad((char *) drinknames[drink], PAD_ROD, PAD_OBJECT));
            act(buf, "Мп", ch, obj);
        }
        if (k->mess_to_room) {
            sprintf(buf, k->mess_to_room,
                    get_name_pad((char *) drinknames[drink], PAD_ROD, PAD_OBJECT));
            act(buf, "Кмп", ch, obj);
        }
    } else {
        act("Вы выпили %1 из @1р.", "Мпт", ch, obj,
            get_name_pad((char *) drinknames[drink], PAD_ROD, PAD_OBJECT));
        act("1+и выпил1(,а,о,и) %1 из @1+р.", "Кмпт", ch, obj,
            get_name_pad((char *) drinknames[drink], PAD_ROD, PAD_OBJECT));
    }

    /* Отравление */
    if (GET_OBJ_VAL(obj, 3)) {  // &&!IS_GOD(ch))
        act("Вкус %1 показался Вам странным.", "Мт", ch,
            get_name_pad((char *) drinknames[GET_OBJ_VAL(obj, 2)], PAD_ROD, PAD_OBJECT));
        act("1и поперхнул1(ся,ась,ось,ись) и закашлял1(,а,о,и).", "Км", ch);
        af.type = find_spell_num(SPELL_POISON);
        af.duration = MAX(3, GET_OBJ_VAL(obj, 3) / 3);
        af.modifier = GET_OBJ_VAL(obj, 3);
        af.bitvector = AFF_POISON;
        af.battleflag = TRUE;
        af.main = FALSE;
        af.owner = GET_ID(ch);

        affect_join_char(ch, &af);
    }

    if (!GET_OBJ_VAL(obj, 1))
        name_from_drinkcon(obj);

    check_char_cond(ch, 2);
}

ACMD(do_drink)
{
    struct obj_data *obj = NULL, *tobj;
    int i, j, drink;
    bool found = FALSE, isdrink = FALSE, on_ground = FALSE;
    char ndrink[128];
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);
    if (!*arg) {
        send_to_char("Пить что или из чего?\r\n", ch);
        return;
    }

    if (FIGHTING(ch)) {
        send_to_charf(ch, "Пить во время боя?\r\n");
        return;
    }

    for (i = 0; **(drinknames + i) != '\n'; i++) {
        for (j = 0; j < NUM_PADS; j++)
            if (isname(arg, get_name_pad((char *) drinknames[i], j, PAD_OBJECT))) {
                isdrink = TRUE;
                strcpy(ndrink, get_name_pad((char *) drinknames[i], PAD_TVR, PAD_OBJECT));
            }
    }

    if (isdrink) {
        //Ищем по названию жидкости
        for (tobj = ch->carrying; tobj; tobj = tobj->next_content) {
            if (GET_OBJ_TYPE(tobj) != ITEM_DRINKCON ||
                (GET_OBJ_TYPE(tobj) == ITEM_DRINKCON && GET_OBJ_VAL(tobj, 2) < 0))
                continue;
            if (!GET_OBJ_VAL(tobj, 1))
                continue;
            for (i = 0; i < NUM_PADS; i++) {
                drink = GET_OBJ_VAL(tobj, 2);
                if (isname(arg, get_name_pad((char *) drinknames[drink], i, PAD_OBJECT))) {
                    obj = tobj;
                    break;
                }
            }
            if (found)
                break;
        }
        if (!obj) {
            send_to_charf(ch, "У Вас в инвентаре нет сосуда с %s.\r\n", ndrink);
            return;
        }
    } else {
        //Находим из чего хотим пить
        if (!obj)
            if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
                if (!(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
                    send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
                    return;
                }
                on_ground = TRUE;
            }
    }

    if (on_ground && GET_OBJ_TYPE(obj) == ITEM_DRINKCON)
        send_to_charf(ch, "Для начала нужно взять в руки %s.\r\n", GET_OBJ_PNAME(obj, 3));
    else {
        go_drink_item(ch, obj, subcmd, on_ground);
        GET_MISSED(ch)++;
    }
}

bool eat_item(struct char_data *ch, struct obj_data *obj)
{
    int value = 0, valeat = 0;
    bool extract = FALSE;
    struct affected_type af;

    if ((IS_AFFECTED(ch, AFF_IS_UNDEAD) ||
         (IS_UNDEAD(ch) && GET_MOB_VID(ch) == VMOB_GHOLA)) &&
        IS_CORPSE(obj) && GET_OBJ_VAL(obj, 3) == 1) {
        //act("1и начал1(,а,о,и) начал жадно поглощать @1в.","Кмп",ch,obj);
        act("Припав к @1д, $1и с чавканьем принял1(ся,ась,ось,ись) поглощать плоть.", "Кмп", ch,
            obj);
        act("Припав к @1д, Вы с чавканьем принялись поглощать плоть.", "Мп", ch, obj);
        GET_HIT(ch) += GET_OBJ_WEIGHT(obj) / 1000;
        GET_HIT(ch) = MIN(GET_HIT(ch), GET_REAL_MAX_HIT(ch)*1.5);
        corpse_desc_skelet(obj);
        return (TRUE);
    }

    if (GET_OBJ_TYPE(obj) != ITEM_FOOD && GET_OBJ_TYPE(obj) != ITEM_NOTE && !IS_HIGOD(ch)) {
        act("@1 несъедобен@1(,а,о,ы)!", "Мп", ch, obj);
        act("1+и попробовал1(,а,о,и) откусить кусочек от @1+р.", "Кмп", ch, obj);
        return (FALSE);
    }

    if (IS_HIGOD(ch)) {
        act("Вы проглотили @1в.", "Мп", ch, obj);
        act("1+и проглотил1(,а,о,и) @1+в.", "Кмп", ch, obj);
        extract_obj(obj);
        return (TRUE);
    }
//сколько еды содержиться в пище
    valeat = value = GET_OBJ_VAL(obj, 0);

//значение еды сколько в состоянии потребить персонаж
    value = MIN(full_race[(int) GET_RACE(ch)].max_cond, value);
//send_to_charf(ch,"Мой предел %d\r\n",value);

//значение еды сколько в состоянии сейчас есть персонаж
    if (!IS_NPC(ch)) {
        value = MIN(value, full_race[(int) GET_RACE(ch)].max_cond - GET_COND(ch, FULL));
        //send_to_charf(ch,"В состоянии %d\r\n",value);

        if (value <= SECS_PER_MUD_TICK && !IS_GOD(ch) && valeat > SECS_PER_MUD_TICK) {
            send_to_charf(ch, "Ваш желудок не выдержит столько еды.\r\n");
            return (FALSE);
        }

        if (GET_COND(ch, FULL) > -1)
            GET_COND(ch, FULL) += value;
    }

    GET_OBJ_VAL(obj, 0) = MAX(0, GET_OBJ_VAL(obj, 0) - value);

    if (GET_OBJ_VAL(obj, 0)) {
        act("Вы немного откусили от @1р.", "Мп", ch, obj);
        act("1+и немного откусил1(,а,о,и) от @1+р.", "Кмп", ch, obj);
    } else {
        act("Вы съели @1в.", "Мп", ch, obj);
        act("1+и съел1(,а,о,и) @1+в.", "Кмп", ch, obj);
        extract = TRUE;
    }

    /* Отравление */
    if (GET_OBJ_VAL(obj, 3) && !IS_GOD(ch)) {
        act("Вкус @1р показался Вам странным.", "Мп", ch, obj);

        act("$n поперхнул$u и закашлял$g.", TRUE, ch, 0, 0, TO_ROOM);

        af.type = find_spell_num(SPELL_POISON);
        af.duration = MAX(3, GET_OBJ_VAL(obj, 3) / 3);
        af.modifier = GET_OBJ_VAL(obj, 3);
        af.bitvector = AFF_POISON;
        af.battleflag = TRUE;
        af.main = FALSE;
        af.owner = GET_ID(ch);

        affect_join_char(ch, &af);
    }

    if (extract)
        extract_obj(obj);

    if (check_char_cond(ch, 1))
        return (FALSE);

    return (TRUE);
}

ACMD(do_eat)
{
    struct obj_data *obj, *next_obj;
    int dotmode;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];


    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_charf(ch, "Что Вы хотите съесть?\r\n");
        return;
    }

    if (FIGHTING(ch)) {
        send_to_charf(ch, "Есть во время боя? Извольте...\r\n");
        return;
    }

    dotmode = find_all_dots(arg);

    if (dotmode == FIND_ALL) {
        if (!ch->carrying)
            send_to_char("У Вас пусто в инвентаре.\r\n", ch);
        else
            for (obj = ch->carrying; obj; obj = next_obj) {
                next_obj = obj->next_content;
                if (GET_OBJ_TYPE(obj) != ITEM_FOOD && !IS_GOD(ch))
                    continue;
                if (!eat_item(ch, obj))
                    return;
            }
    } else if (dotmode == FIND_ALLDOT) {
        if (!*arg) {
            send_to_charf(ch, "Уточните что все Вы хотите съесть?\r\n");
            return;
        }
        if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
            sprintf(buf, "У Вас нет ничего похожего на '%s'.\r\n", arg);
            send_to_char(buf, ch);
        } else {
            while (obj) {
                next_obj = get_obj_in_list_vis(ch, arg, obj->next_content);
                if (!eat_item(ch, obj))
                    return;
                obj = next_obj;
            }
            GET_MISSED(ch)++;
        }
    } else {
        if ((obj = get_obj_in_list_vis(ch, arg, world[IN_ROOM(ch)].contents)) &&
            IS_CORPSE(obj) && ((IS_UNDEAD(ch) && GET_MOB_VID(ch) == VMOB_GHOLA) ||
                               IS_AFFECTED(ch, AFF_IS_UNDEAD))) {
            eat_item(ch, obj);
            return;
        } else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
            sprintf(buf, "У Вас нет '%s'.\r\n", arg);
            send_to_char(buf, ch);
            return;
        } else {
            if (!eat_item(ch, obj))
                return;
            GET_MISSED(ch)++;
        }
    }

}

ACMD(do_pour)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *from_obj = NULL, *to_obj = NULL;
    int amount;

    two_arguments(argument, arg1, arg2);

    if (subcmd == SCMD_POUR || subcmd == SCMD_APOUR) {
        if (!*arg1) {           /* No arguments */
            if (subcmd == SCMD_APOUR)
                send_to_char("Откуда выливаем?\r\n", ch);
            else
                send_to_char("Откуда переливаем?\r\n", ch);
            return;
        }
        if (!(from_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
            send_to_char("У Вас нет этого!\r\n", ch);
            return;
        }
        if (GET_OBJ_TYPE(from_obj) != ITEM_DRINKCON) {
            send_to_char("Вы не можете из этого переливать!\r\n", ch);
            return;
        }
    }
    if (subcmd == SCMD_FILL) {
        if (!*arg1) {           /* no arguments */
            send_to_char("Что и из чего Вы хотели бы наполнить?\r\n", ch);
            return;
        }
        if (!(to_obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
            send_to_char("У Вас этого нет!\r\n", ch);
            return;
        }
        if (GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) {
            act("Вы не можете наполнить @1в!", "Мп", ch, to_obj);
            return;
        }
        if (!*arg2) {           /* no 2nd argument */
            act("Из чего Вы планируете наполнить @1и?", "Мп", ch, to_obj);
            return;
        }
        if (!(from_obj = get_obj_in_list_vis(ch, arg2, world[ch->in_room].contents))) {
            sprintf(buf, "Вы не видите здесь '%s'.\r\n", arg2);
            send_to_char(buf, ch);
            return;
        }
        if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN) {
            act("Вы не сможете ничего наполнить из @1р.", "Мп", ch, from_obj);
            return;
        }
    }
    if (GET_OBJ_VAL(from_obj, 1) == 0) {
        act("В @1+п пусто.", "Мп", ch, from_obj);
        return;
    }

    if (subcmd == SCMD_APOUR) { /* льем на землю */
        act("1+и вылил1(,а,о,и) содержимое @1+р на землю.", "Кмп", ch, from_obj);
        act("Вы опустошили @1в.", "Мп", ch, from_obj);
        //weight_change_object(from_obj, -GET_OBJ_VAL(from_obj, 1)); /* Empty */
        GET_OBJ_VAL(from_obj, 1) = 0;
        GET_OBJ_VAL(from_obj, 2) = 0;
        GET_OBJ_VAL(from_obj, 3) = 0;
        name_from_drinkcon(from_obj);
        GET_MISSED(ch)++;
        return;
    }

    if (subcmd == SCMD_POUR) {
        if (!*arg2) {
            send_to_char("Куда Вы хотите лить?\r\n", ch);
            return;
        }

        if (!(to_obj = get_obj_in_list_vis(ch, arg2, ch->carrying))) {
            send_to_char("Вы не можете этого найти!\r\n", ch);
            return;
        }
        if ((GET_OBJ_TYPE(to_obj) != ITEM_DRINKCON) && (GET_OBJ_TYPE(to_obj) != ITEM_FOUNTAIN)
            ) {
            send_to_char("Вы не сможете в это налить.\r\n", ch);
            return;
        }
    }
    if (to_obj == from_obj) {
        send_to_char("Как это?\r\n", ch);
        return;
    }
    if ((GET_OBJ_VAL(to_obj, 1) != 0) && (GET_OBJ_VAL(to_obj, 2) != GET_OBJ_VAL(from_obj, 2))) {
        send_to_char("Алхимия будет в следующей эпохе.\r\n", ch);
        return;
    }
    if (!(GET_OBJ_VAL(to_obj, 1) < GET_OBJ_VAL(to_obj, 0))) {
        send_to_char("Там нет места.\r\n", ch);
        return;
    }
    if (subcmd == SCMD_POUR) {
        act("1+и перелил1(,а,о,и) %1 из @1+р в @2+в.", "Кмппт", ch, from_obj, to_obj,
            drinks[GET_OBJ_VAL(from_obj, 2)]);
        act("Вы перелили %1 из @1+р в @2+в.", "Мппт", ch, from_obj, to_obj,
            drinks[GET_OBJ_VAL(from_obj, 2)]);
    }
    if (subcmd == SCMD_FILL) {
        act("Вы наполнили @2в из @1р.", "Мпп", ch, from_obj, to_obj);
        act("1+и наполнил1(,а,о,и) @2+в из @1+р.", "Кмпп", ch, from_obj, to_obj);
    }
    /* New alias */
    if (GET_OBJ_VAL(to_obj, 1) == 0)
        name_to_drinkcon(to_obj, GET_OBJ_VAL(from_obj, 2));

    /* First same type liq. */
    GET_OBJ_VAL(to_obj, 2) = GET_OBJ_VAL(from_obj, 2);

    /* Then how much to pour */
    if (GET_OBJ_TYPE(from_obj) != ITEM_FOUNTAIN || GET_OBJ_VAL(from_obj, 1) != 999)
        GET_OBJ_VAL(from_obj, 1) -= (amount = (GET_OBJ_VAL(to_obj, 0) - GET_OBJ_VAL(to_obj, 1)));
    else
        amount = GET_OBJ_VAL(to_obj, 0) - GET_OBJ_VAL(to_obj, 1);

    GET_OBJ_VAL(to_obj, 1) = GET_OBJ_VAL(to_obj, 0);

    /* Then the poison boogie */
    GET_OBJ_VAL(to_obj, 3) = (GET_OBJ_VAL(to_obj, 3) || GET_OBJ_VAL(from_obj, 3));

    if (GET_OBJ_VAL(from_obj, 1) <= 0) {        /* There was too little */
        GET_OBJ_VAL(to_obj, 1) += GET_OBJ_VAL(from_obj, 1);
        amount += GET_OBJ_VAL(from_obj, 1);
        GET_OBJ_VAL(from_obj, 1) = 0;
        GET_OBJ_VAL(from_obj, 2) = 0;
        GET_OBJ_VAL(from_obj, 3) = 0;
        name_from_drinkcon(from_obj);
    }

    GET_MISSED(ch)++;
}



void wear_message(struct char_data *ch, struct obj_data *obj, int where)
{
    const char *wear_messages[][2] = {
        {
         "1+и взял1(,а,о,и) @1+в в левую руку.",
         "Вы взяли @1в в левую руку."},

        {
         "1+и надел1(,а,о,и) @1+в на палец правой руки.",
         "Вы надели @1в на палец правой руки."},

        {
         "1+и надел1(,а,о,и) @1+в на палец левой руки.",
         "Вы надели @1в на палец левой руки."},

        {
         "1+и надел1(,а,о,и) @1+в на лицо.",
         "Вы надели @1в на лицо."},

        {
         "1+и нацепил1(,а,о,и) @1+в на левое ухо.",
         "Вы нацепили @1в на левое ухо."},

        {
         "1+и нацепил1(,а,о,и) @1+в на правое ухо.",
         "Вы нацепили @1в на правое ухо."},

        {
         "1+и повесил1(,а,о,и) @1+в на шею.",
         "Вы повесили @1в на шею."},

        {
         "1+и надел1(,а,о,и) @1+в на грудь.",
         "Вы надели @1в на грудь."},

        {
         "1+и надел1(,а,о,и) @1+в на тело.",
         "Вы надели @1в на тело."},

        {
         "1+и надел1(,а,о,и) @1+в на голову.",
         "Вы надели @1в на голову."},

        {
         "1+и надел1(,а,о,и) @1+в на ноги.",
         "Вы надели @1в на ноги."},

        {
         "1+и обул1(,а,о,и) @1+в.",
         "Вы обули @1в."},

        {
         "1+и надел1(,а,о,и) @1+в на кисти рук.",
         "Вы надели @1в на кисти рук."},

        {
         "1+и надел1(,а,о,и) @1+в на руки.",
         "Вы надели @1в на руки."},

        {
         "1+и надел1(,а,о,и) @1+в на левую руку.",
         "Вы надели @1в на левую руку."},

        {
         "1+и накинул1(,а,о,и) на плечи @1+в.",
         "Вы накинули @1в на плечи."},

        {
         "1+и надел1(,а,о,и) @1+в вокруг талии.",
         "Вы надели @1в вокруг талии."},

        {
         "1+и надел1(,а,о,и) @1+в на правое запястье.",
         "Вы надели @1в на правое запястье."},

        {
         "1+и надел1(,а,о,и) @1+в на левоое запястье.",
         "Вы надели @1в на левое запястье."},

        {
         "1+и вооружил1(ся,ась,ось,ись) @1+т.",
         "Вы вооружились @1т."},

        {
         "1+и дополнительно вооружил1(ся,ась,ось,ись) @1+т.",
         "Вы дополнительно вооружились @1т."},

        {
         "1+и вооружил1(ся,ась,ось,ись) @1+т.",
         "Вы вооружились @1т."},

        {
         "1+и надел1(,а,о,и) @1+в на хвост.",
         "Вы надели @1в на хвост."},

        {
         "1+и нацеп1(ил,ла,ло,ли) @1+в за спину.",
         "Вы нацепили @1в за спину."}

    };

    if (where == WEAR_HOLD && GET_OBJ_TYPE(obj) != ITEM_WEAPON) {
        act("1+и взял1(,а,о,и) @1+в в левую руку.", "Кмп", ch, obj);
        act("Вы взяли @1+в в левую руку.", "Мп", ch, obj);
    } else {
        act(wear_messages[where][0], "Кмп", ch, obj);
        act(wear_messages[where][1], "Мп", ch, obj);
    }
}

void wear_tatoo_message(struct char_data *ch, struct char_data *keeper, struct obj_data *obj,
                        int where)
{
    const char *wear_messages[][2] = {
        {"ОШИБКА 500 код 1",
         "ОШИБКА 500 код 1"},

        {"2и наколол2(,а,о,и) 1р @1в на правый указательный палец.",
         "2и наколол2(,а,о,и) Вам @1в на правый указательный палец."},

        {"2и наколол2(,а,о,и) 1р @1в на левый указательный палец.",
         "2и наколол2(,а,о,и) Вам @1в на левый указательный палец."},

        {"2и наколол2(,а,о,и) 1р @1в на щеку.",
         "2и наколол2(,а,о,и) Вам @1в на щеку."},

        {"2и наколол2(,а,о,и) 1р @1в на левое ухо.",
         "2и наколол2(,а,о,и) Вам @1в на левое ухо."},

        {"2и наколол2(,а,о,и) 1р @1в на правое ухо.",
         "2и наколол2(,а,о,и) Вам @1в на правое ухо."},

        {"2и наколол2(,а,о,и) 1р @1в на шею.",
         "2и наколол2(,а,о,и) Вам @1в на шею."},

        {
         "2и наколол2(,а,о,и) 1р @1в на грудь.",
         "2и наколол2(,а,о,и) Вам @1в на грудь."},

        {
         "2и наколол2(,а,о,и) 1р @1в на грудь.",
         "2и наколол2(,а,о,и) Вам @1в на грудь."},

        {
         "2и наколол2(,а,о,и) 1р @1в на лоб.",
         "2и наколол2(,а,о,и) Вам @1в на лоб."},

        {
         "2и наколол2(,а,о,и) 1р @1в на бедро.",
         "2и наколол2(,а,о,и) Вам @1в на бедро."},

        {
         "2и наколол2(,а,о,и) 1р @1в на икры.",
         "2и наколол2(,а,о,и) Вам @1в на икры."},

        {
         "2и наколол2(,а,о,и) 1р @1в на плечо.",
         "2и наколол2(,а,о,и) Вам @1в на плечо."},

        {
         "2и наколол2(,а,о,и) 1р @1в на ладони.",
         "2и наколол2(,а,о,и) Вам @1в на ладони."},

        {
         "2и не может наклоть @1в в позицию ЩИТ. Сообщите бесмертным!",
         "2и не может наклоть @1в в позицию ЩИТ. Сообщите бесмертным!"},

        {
         "2и наколол2(,а,о,и) 1р @1в на спину.",
         "2и наколол2(,а,о,и) Вам @1в на спину."},

        {
         "2и наколол2(,а,о,и) 1р @1в в область пояса.",
         "2и наколол2(,а,о,и) Вам @1в в область пояса."},

        {
         "2и наколол2(,а,о,и) 1р @1в на правое запястье.",
         "2и наколол2(,а,о,и) Вам @1в на правое запястье."},

        {
         "2и наколол2(,а,о,и) 1р @1в на левое запястье.",
         "2и наколол2(,а,о,и) Вам @1в на левое запястье."},

        {
         "2и не может наклоть @1в в позицию ПРАВАЯ. Сообщите бесмертным!",
         "2и не может наклоть @1в в позицию ПРАВАЯ. Сообщите бесмертным!"},

        {
         "2и не может наклоть @1в в позицию ЛЕВАЯ. Сообщите бесмертным!",
         "2и не может наклоть @1в в позицию ЛЕВАЯ. Сообщите бесмертным!"},

        {
         "2и не может наклоть @1в в позицию ОБЕ. Сообщите бесмертным!",
         "2и не может наклоть @1в в позицию ОБЕ. Сообщите бесмертным!"},

        {
         "2и наколол2(,а,о,и) 1р @1в на хвост.",
         "2и наколол2(,а,о,и) Вам @1в на хвост."},

        {
         "2и не может наклоть @1в в позицию КОЛЧАН. Сообщите бесмертным!",
         "2и не может наклоть @1в в позицию КОЛЧАН. Сообщите бесмертным!"}
    };

    act(wear_messages[where][0], "Кммп", ch, keeper, obj);
    act(wear_messages[where][1], "Ммп", ch, keeper, obj);
}


void perform_wear(struct char_data *ch, struct obj_data *obj, int where)
{
    struct mess_p_data *k;
    int found = 0, count = 0;

    /*
     * ITEM_WEAR_TAKE is used for objects that do not require special bits
     * to be put into that position (e.g. you can hold any object, not just
     * an object with a HOLD bit.)
     */

    const char *already_wearing[] = { "Вы уже используете свет.\r\n",
        "У Вас уже что-то надето на пальцах.\r\n",
        "У Вас уже что-то надето на пальцах.\r\n",
        "У Вас уже что-то одето на лице.\r\n",
        "У Вас уже что-то висит на ушах.\r\n",
        "У Вас уже что-то висит на ушах.\r\n",
        "У Вас уже что-то надето на шею.\r\n",
        "У Вас уже что-то надето на шею.\r\n",
        "У Вас уже что-то надето на туловище.\r\n",
        "У Вас уже что-то надето на голову.\r\n",
        "У Вас уже что-то надето на ноги.\r\n",
        "У Вас уже что-то надето на ступни.\r\n",
        "У Вас уже что-то надето на кисти.\r\n",
        "У Вас уже что-то надето на руки.\r\n",
        "Вы уже используете щит.\r\n",
        "Вы уже облачены во что-то.\r\n",
        "У Вас уже что-то надето на пояс.\r\n",
        "У Вас уже что-то надето на запястья.\r\n",
        "У Вас уже что-то надето на запястья.\r\n",
        "Вы уже что-то держите в правой руке.\r\n",
        "Вы уже что-то держите в левой руке.\r\n",
        "Вы уже что-то держите в обеих руках.\r\n",
        "У Вас уже что-то одето на хвост.\r\n",
        "Вы уже нацепили колчан на спину.\r\n"
    };

    if (GET_OBJ_TYPE(obj) == ITEM_TATOO) {
        act("Татуировки не надеваются, их накалывают на участок кожи.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (where == WEAR_TAIL && !IS_TIEFLING(ch)) {
        act("Вам некуда надеть $o.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    /* first, make sure that the wear position is valid. */
    if (!CAN_WEAR(obj, wear_bitvectors[where]) && where != WEAR_BOTHS) {
        act("Вы не можете надеть $o3 на эту часть тела.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }
    /* for neck, finger, and wrist, try pos 2 if pos 1 is already full */
    if (                        /* не может держать если есть свет или двуручник */
           (where == WEAR_HOLD && (GET_EQ(ch, WEAR_BOTHS) || GET_EQ(ch, WEAR_LIGHT) ||
                                   GET_EQ(ch, WEAR_SHIELD))) ||
           /* не может вооружиться если есть двуручник */
           (where == WEAR_WIELD && GET_EQ(ch, WEAR_BOTHS)) ||
           /* не может держать щит если что-то держит или двуручник */
           (where == WEAR_SHIELD && (GET_EQ(ch, WEAR_HOLD) || GET_EQ(ch, WEAR_BOTHS))) ||
           /* не может двуручник если есть щит, свет, вооружен или держит */
           (where == WEAR_BOTHS && (GET_EQ(ch, WEAR_HOLD) || GET_EQ(ch, WEAR_LIGHT) ||
                                    GET_EQ(ch, WEAR_SHIELD) || GET_EQ(ch, WEAR_WIELD))) ||
           /* не может держать свет если двуручник или держит */
           (where == WEAR_LIGHT && (GET_EQ(ch, WEAR_HOLD) || GET_EQ(ch, WEAR_BOTHS)))
        ) {
        send_to_char("У Вас заняты руки.\r\n", ch);
        return;
    }


    if ((where != WEAR_NECK_1 && where != WEAR_TAIL) && IS_ANIMAL(ch)) {
        act("$n отказал$u надеть $o.", TRUE, ch, obj, 0, TO_ROOM);
        send_to_charf(ch, "Животным это не поможет.\r\n");
        return;
    }

    if ((where == WEAR_FINGER_R) ||
        (where == WEAR_EAR_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R))
        if (GET_EQ(ch, where))
            where++;


    if (GET_EQ(ch, where)) {
        send_to_char(already_wearing[where], ch);
        return;
    }

    /* Проверка на перехват сообщений */
    for (k = obj->mess_data; k; k = k->next, count++)
        if (k->command == CMD_WEAR) {
            found = 1;
            break;
        }


    if (found && k->stoping) {
        if (k->mess_to_char)
            act(k->mess_to_char, "Мп", ch, obj);
        if (k->mess_to_room)
            act(k->mess_to_room, "Кмп", ch, obj);
        return;
    }

    if (preequip_char(ch, obj, where)) {
        if (found) {
            if (k->mess_to_char)
                act(k->mess_to_char, "Мп", ch, obj);
            if (k->mess_to_room)
                act(k->mess_to_room, "Кмп", ch, obj);
        } else {
            wear_message(ch, obj, where);
        }
        if (found && k->script)
            go_script(k->script, ch, obj);

        // Both item and char can be destroyed or relocated after the script.
        if (obj->carried_by != ch) {
            return;
        }

        obj_from_char(obj);
        equiped_char(ch, obj, where);
        postequip_char(ch, obj);
    }
}



int find_eq_pos(struct char_data *ch, struct obj_data *obj, char *arg)
{
    int where = -1;
    char buf[MAX_STRING_LENGTH];

    /* \r to prevent explicit wearing. Don't use \n, it's end-of-array marker. */
    const char *keywords[] = {
        "\r!RESERVED!",
        "правыйпалец",
        "левыйпалец",
        "лицо",
        "правоеухо",
        "левоеухо",             //5
        "шея",
        "грудь",
        "тело",
        "голова",
        "ноги",                 //10
        "ступни",
        "кисти",
        "руки",
        "щит",
        "плечи",                //15
        "пояс",
        "правоезапястье",
        "левоезапястье",
        "правая",
        "левая",
        "обе",                  //20
        "хвост",
        "колчан",
        "\n"
    };

    if (!arg || !*arg) {
        if (CAN_WEAR(obj, ITEM_WEAR_FINGER))
            where = WEAR_FINGER_R;
        if (CAN_WEAR(obj, ITEM_WEAR_NECK))
            where = WEAR_NECK_1;
        if (CAN_WEAR(obj, ITEM_WEAR_BODY))
            where = WEAR_BODY;
        if (CAN_WEAR(obj, ITEM_WEAR_HEAD))
            where = WEAR_HEAD;
        if (CAN_WEAR(obj, ITEM_WEAR_LEGS))
            where = WEAR_LEGS;
        if (CAN_WEAR(obj, ITEM_WEAR_FEET))
            where = WEAR_FEET;
        if (CAN_WEAR(obj, ITEM_WEAR_HANDS))
            where = WEAR_HANDS;
        if (CAN_WEAR(obj, ITEM_WEAR_ARMS))
            where = WEAR_ARMS;
        if (CAN_WEAR(obj, ITEM_WEAR_SHIELD))
            where = WEAR_SHIELD;
        if (CAN_WEAR(obj, ITEM_WEAR_ABOUT))
            where = WEAR_ABOUT;
        if (CAN_WEAR(obj, ITEM_WEAR_WAIST))
            where = WEAR_WAIST;
        if (CAN_WEAR(obj, ITEM_WEAR_WRIST))
            where = WEAR_WRIST_R;
        if (CAN_WEAR(obj, ITEM_WEAR_FACE))
            where = WEAR_FACE;
        if (CAN_WEAR(obj, ITEM_WEAR_TAIL))
            where = WEAR_TAIL;
        if (CAN_WEAR(obj, ITEM_WEAR_EARS))
            where = WEAR_EAR_R;
        if (CAN_WEAR(obj, ITEM_WEAR_QUIVER))
            where = WEAR_QUIVER;
    } else {
        if (((where = search_block(arg, keywords, FALSE)) < 0) || (*arg == '!')) {
            sprintf(buf, "'%s'?Куда?\r\n", arg);
            send_to_char(buf, ch);
            return -1;
        }
    }

    return (where);
}



ACMD(do_wear)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    struct obj_data *obj, *next_obj;
    int where, dotmode, items_worn = 0;

    two_arguments(argument, arg1, arg2);

    if (!*arg1) {
        send_to_char("Что Вы собрались надеть?\r\n", ch);
        return;
    }
    dotmode = find_all_dots(arg1);

    if (*arg2 && (dotmode != FIND_INDIV)) {
        send_to_char("На что Вы хотите это надеть!\r\n", ch);
        return;
    }
    if (dotmode == FIND_ALL) {
        for (obj = ch->carrying; obj && !GET_MOB_HOLD(ch) && GET_POS(ch) > POS_SLEEPING;
             obj = next_obj) {
            next_obj = obj->next_content;
            if (CAN_SEE_OBJ(ch, obj)) {
                items_worn++;
                if (invalid_anti_class(ch, obj)) {
                    act("Вы не смогли надеть $o3.", FALSE, ch, obj, 0, TO_CHAR);
                    act("$n покрутил$g $o3 в руках, но не смог$q никуда надеть.", FALSE, ch, obj, 0,
                        TO_ROOM);
                    continue;
                } else if (!OK_WEAR(ch, obj)) {
                    act("Вам слишком тяжело носить на себе $o3.", FALSE, ch, obj, 0, TO_CHAR);
                } else if ((where = find_eq_pos(ch, obj, 0)) >= 0) {
                    perform_wear(ch, obj, where);
                }
            }
        }
        if (!items_worn)
            send_to_char("Увы, но надеть Вам нечего.\r\n", ch);
    } else if (dotmode == FIND_ALLDOT) {
        if (!*arg1) {
            send_to_char("Надеть \"все\" чего ?\r\n", ch);
            return;
        }
        if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
            sprintf(buf, "У Вас нет ничего похожего на '%s'.\r\n", arg1);
            send_to_char(buf, ch);
        } else
            while (obj && !GET_MOB_HOLD(ch) && GET_POS(ch) > POS_SLEEPING) {
                next_obj = get_obj_in_list_vis(ch, arg1, obj->next_content);
                if (CAN_SEE_OBJ(ch, obj)) {
                    if (invalid_anti_class(ch, obj)) {
                        act("Вы не смогли надеть $o3.", FALSE, ch, obj, 0, TO_CHAR);
                        act("$n покрутил$g $o3 в руках, но не смог$q никуда надеть.", FALSE, ch,
                            obj, 0, TO_ROOM);
                    } else if (!OK_WEAR(ch, obj)) {
                        act("Вам слишком тяжело носить на себе $o3.", FALSE, ch, obj, 0, TO_CHAR);
                    } else {
                        if (GET_OBJ_TYPE(obj) == ITEM_LIGHT)
                            perform_wear(ch, obj, WEAR_LIGHT);
                        else if ((where = find_eq_pos(ch, obj, 0)) >= 0)
                            perform_wear(ch, obj, where);
                        else
                            act("Вы не можете надеть $o3.", FALSE, ch, obj, 0, TO_CHAR);
                    }
                    obj = next_obj;
                }
            }
    } else {
        if (!(obj = get_obj_in_list_vis(ch, arg1, ch->carrying))) {
            sprintf(buf, "У Вас нет ничего похожего на '%s'.\r\n", arg1);
            send_to_char(buf, ch);
        } else if (invalid_anti_class(ch, obj)) {
            act("Вы не смогли надеть $o3.", FALSE, ch, obj, 0, TO_CHAR);
            act("$n покрутил$g $o3 в руках, но не смог$q никуда надеть.", FALSE, ch, obj, 0,
                TO_ROOM);
        } else if (!OK_WEAR(ch, obj)) {
            act("Вам слишком тяжело носить на себе $o3.", FALSE, ch, obj, 0, TO_CHAR);
        } else {
            if (GET_OBJ_TYPE(obj) == ITEM_LIGHT)
                perform_wear(ch, obj, WEAR_LIGHT);
            else if ((where = find_eq_pos(ch, obj, arg2)) >= 0)
                perform_wear(ch, obj, where);
            else if (!*arg2)
                act("Вы не можете надеть $o3.", FALSE, ch, obj, 0, TO_CHAR);
        }
    }
}


ACMD(do_wield)
{
    struct obj_data *obj;
    int wear;
    char arg[MAX_STRING_LENGTH];

    if (IS_ANIMAL(ch))
        return;
    one_argument(argument, arg);

    if (!*arg)
        send_to_char("Чем Вы хотите вооружиться?\r\n", ch);
    else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        send_to_charf(ch, "Вы не видите ничего похожего на '%s'.\r\n", arg);
        return;
    } else {
        if (!CAN_WEAR(obj, ITEM_WEAR_WIELD) && !CAN_WEAR(obj, ITEM_WEAR_BOTHS))
            send_to_char("Вы не можете вооружиться этим.\r\n", ch);
        else if (IS_NPC(ch) && AFF_FLAGGED(ch, AFF_CHARM))
            send_to_char("Справка: ожившие трупы не могут вооружаться.\r\n", ch);
        else if (IS_NPC(ch) && (IS_ANIMAL(ch) || IS_DRAGON(ch)))
            send_to_char("Справка: животные и драконы не могут пользоваться оружием.\r\n", ch);
        else {
            if (CAN_WEAR(obj, ITEM_WEAR_BOTHS)) {
                act("$o предназначен$G для двух рук.\r\nВоспользуйтесь командой 'двуручное'.",
                    FALSE, ch, obj, 0, TO_CHAR);
                return;
            }
            if (CAN_WEAR(obj, ITEM_WEAR_WIELD)) {
                wear = WEAR_WIELD;
                if (!IS_IMMORTAL(ch) && !OK_WIELD(ch, obj)) {
                    act("Вам слишком тяжело держать $o3 в правой руке.", FALSE, ch, obj, 0,
                        TO_CHAR);
                    return;
                }
                perform_wear(ch, obj, wear);
            }
        }
    }
}


ACMD(do_boths)
{
    struct obj_data *obj;
    int wear;
    char arg[MAX_STRING_LENGTH];

    if (IS_ANIMAL(ch))
        return;
    one_argument(argument, arg);

    if (!*arg)
        send_to_char("Что Вы хотите взять в обе руки?\r\n", ch);
    else if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        send_to_charf(ch, "Вы не видите ничего похожего на '%s'.\r\n", arg);
        return;
    } else {
        if (!CAN_WEAR(obj, ITEM_WEAR_WIELD) && !CAN_WEAR(obj, ITEM_WEAR_BOTHS)
            && !CAN_WEAR(obj, ITEM_WEAR_HOLD))
            send_to_char("Вы не можете держать это в руках.\r\n", ch);
        else if (IS_NPC(ch) && AFF_FLAGGED(ch, AFF_CHARM))
            send_to_char("Справка: ожившие трупы не могут вооружаться.\r\n", ch);
        else if (IS_NPC(ch) && (IS_ANIMAL(ch) || IS_DRAGON(ch)))
            send_to_char("Справка: животные и драконы не могут пользоваться оружием.\r\n", ch);
        else {
            wear = WEAR_BOTHS;
            if (!IS_IMMORTAL(ch) && !OK_BOTH(ch, obj)) {
                act("Вам слишком тяжело держать $o3 в руках.", FALSE, ch, obj, 0, TO_CHAR);
                return;
            }
            perform_wear(ch, obj, wear);
        }
    }
}


ACMD(do_hold)
{
    struct obj_data *obj;
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Что именно Вы хотите взять в левую руку?\r\n", ch);
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        send_to_charf(ch, "У Вас нет ничего похожего на '%s'.\r\n", arg);
        return;
    }

    if (IS_NPC(ch) && AFF_FLAGGED(ch, AFF_CHARM)) {
        send_to_char("Ожившие трупы не могут вооружаться.\r\n", ch);
        return;
    }

    if (IS_NPC(ch) && (IS_ANIMAL(ch) || IS_DRAGON(ch))) {
        send_to_char("Справка: животные и драконы не могут пользоваться оружием.\r\n", ch);
        return;
    }

    if (CAN_WEAR(obj, ITEM_WEAR_BOTHS) && !CAN_WEAR(obj, ITEM_WEAR_HOLD)) {
        act("$o предназначен$G для двух рук.\r\nВоспользуйтесь командой 'двуручное'.", FALSE, ch,
            obj, 0, TO_CHAR);
        return;
    }

    if (GET_OBJ_TYPE(obj) == ITEM_LIGHT)
        perform_wear(ch, obj, WEAR_LIGHT);
    else {
        if ((!CAN_WEAR(obj, ITEM_WEAR_HOLD) &&
             GET_OBJ_TYPE(obj) != ITEM_WAND &&
             GET_OBJ_TYPE(obj) != ITEM_STAFF &&
             GET_OBJ_TYPE(obj) != ITEM_SCROLL &&
             GET_OBJ_TYPE(obj) != ITEM_POTION && !OK_HELDN(ch, obj)) || !OK_HELD(ch, obj)) {
            act("Вам слишком тяжело держать $o3 в левой руке.", FALSE, ch, obj, 0, TO_CHAR);
            return;
        }
        perform_wear(ch, obj, WEAR_HOLD);
    }

}


void perform_remove(struct char_data *ch, int pos)
{
    struct mess_p_data *k;
    int found = 0, count = 0;

    struct obj_data *obj;

    if (!(obj = GET_EQ(ch, pos)))
        log("SYSERR: perform_remove: bad pos %d passed.", pos);
    else
        /*
           if (IS_OBJ_STAT(obj, ITEM_NODROP))
           act("Вы не можете снять $o3!", FALSE, ch, obj, 0, TO_CHAR);
           else
         */
    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch) && !IS_GOD(ch))
        act("Вы не можете удержать в руках $o3.", FALSE, ch, obj, 0, TO_CHAR);
    else {
        /* Проверка на перехват сообщений */
        for (k = obj->mess_data; k; k = k->next, count++)
            if (k->command == CMD_REMOVE) {
                found = 1;
                break;
            }

        if (found && k->stoping) {
            if (k->mess_to_char)
                act(k->mess_to_char, "М", ch);
            if (k->mess_to_room)
                act(k->mess_to_room, "Км", ch);
            return;
        }

        obj_to_char(unequip_char(ch, pos | 0x80 | 0x40), ch);
        if (found) {
            if (k->mess_to_char)
                act(k->mess_to_char, "М", ch);
            if (k->mess_to_room)
                act(k->mess_to_room, "Км", ch);
        } else {
            if ((GET_OBJ_TYPE(obj) == ITEM_ARMOR) || (GET_OBJ_TYPE(obj) == ITEM_WORN)) {
                act("Вы сняли $o3.", FALSE, ch, obj, 0, TO_CHAR);
                act("$n снял$g $o3.", TRUE, ch, obj, 0, TO_ROOM);
            } else {
                act("Вы прекратили использовать $o3.", FALSE, ch, obj, 0, TO_CHAR);
                act("$n прекратил$g использовать $o3.", TRUE, ch, obj, 0, TO_ROOM);
            }
        }
        if (found && k->script)
            go_script(k->script, ch, obj);

    }
}



ACMD(do_remove)
{
    int i, dotmode, found;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Снять что ?\r\n", ch);
        return;
    }
    dotmode = find_all_dots(arg);

    if (dotmode == FIND_ALL) {
        found = 0;
        for (i = 0; i < NUM_WEARS; i++)
            if (GET_EQ(ch, i)) {
                perform_remove(ch, i);
                found = 1;
            }
        if (!found)
            send_to_char("На Вас нет такого предмета.\r\n", ch);
    } else if (dotmode == FIND_ALLDOT) {
        if (!*arg)
            send_to_char("Снять все передметы какого типа?\r\n", ch);
        else {
            found = 0;
            for (i = 0; i < NUM_WEARS; i++)
                if (GET_EQ(ch, i) && CAN_SEE_OBJ(ch, GET_EQ(ch, i)) &&
                    isname(arg, GET_EQ(ch, i)->name)) {
                    perform_remove(ch, i);
                    found = 1;
                }
            if (!found) {
                sprintf(buf, "На Вас нет ни одного '%s'.\r\n", arg);
                send_to_char(buf, ch);
            }
        }
    } else {
        if ((i = get_obj_pos_in_equip_vis(ch, arg, NULL, ch->equipment)) < 0) {
            sprintf(buf, "Вы не используете '%s'.\r\n", arg);
            send_to_char(buf, ch);
        } else
            perform_remove(ch, i);
    }
    check_sets(ch);
    affect_total(ch);
}


AEVENT(event_fire)
{
    struct char_data *ch = params->actor;

    if (!ch) {
        log("НЕ найден ch");
        return;
    }

    int skill = GET_SKILL(ch, SKILL_FIRE), percent, prob;


    percent = 20 + RNDSKILL;
    prob = GET_REAL_INT(ch) + GET_REAL_WIS(ch) + skill;


    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);

    improove_skill(ch, ch, 0, SKILL_FIRE);

    if (percent > prob) {
        send_to_char("Вы попытались разжечь костер, но у Вас ничего не вышло.\r\n", ch);
        return;
    } else {
        world[IN_ROOM(ch)].fires = MAX(1, skill / 25);
        send_to_char("Вы разожгли огонь.\n\r", ch);
        act("1+и развел1(,а,о,и) огонь.", "Км", ch);
    }

}

ACMD(do_fire)
{
    struct event_param_data params;

    if (!check_fight_command(ch))
        return;

    if (!GET_SKILL(ch, SKILL_FIRE)) {
        send_to_char("Но Вы не знаете как.\r\n", ch);
        return;
    }

    if (on_horse(ch)) {
        send_to_char("Верхом это будет затруднительно.\r\n", ch);
        return;
    }

    if (AFF_FLAGGED(ch, AFF_BLIND)) {
        send_to_char("Вы ничего не видите!\r\n", ch);
        return;
    }


    if (world[IN_ROOM(ch)].fires) {
        send_to_char("Здесь уже горит огонь.\r\n", ch);
        return;
    }

    if (ROOM_FLAGGED(ch->in_room, ROOM_PEACEFUL) ||
        SECT(IN_ROOM(ch)) == SECT_WATER_SWIM ||
        SECT(IN_ROOM(ch)) == SECT_WATER_NOSWIM ||
        SECT(IN_ROOM(ch)) == SECT_FLYING ||
        SECT(IN_ROOM(ch)) == SECT_UNDERWATER || SECT(IN_ROOM(ch)) == SECT_SECRET) {
        send_to_char("В этой комнате нельзя разжечь костер.\r\n", ch);
        return;
    }

    init_event_param(&params);
    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.actor = ch;
    params.sto_actor = "Вы набрали хвороста и принялись разжигать костер.";
    params.sto_room = "1и набрал1(,а,о,и) хвороста и принял1(ся,ась,ось,ись) разжигать костер.";
    params.status = "Вы разжигаете костер.";
    params.rstatus = "разжигает костер";
    params.bto_actor = "Вы прекратили разжигать костер.";

    add_event(5, 0, event_fire, &params);
}


ACMD(do_firstaid)
{
    struct affected_type af;
    ubyte prob, percent;
    struct char_data *vict;
    struct timed_type timed;
    int spellnum = find_spell_num(SPELL_BANDAGE);
    char arg[MAX_STRING_LENGTH];

    if (!check_fight_command(ch))
        return;

    if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_AID)) {
        send_to_char("Вы не умеете перевязывать раны.\r\n", ch);
        return;
    }

    if (on_horse(ch)) {
        send_to_char("Для этого необходимо спешиться.\r\n", ch);
        return;
    }

    if (AFF_FLAGGED(ch, AFF_BLIND)) {
        send_to_char("Вы слепы и не сможете перевязать раны.\r\n", ch);
        return;
    }

    if (timed_by_skill(ch, SKILL_AID)) {
        send_to_char("Вы не можете так часто перевязывать раны.\r\n", ch);
        return;
    }

    skip_spaces(&argument);
    one_argument(argument, arg);

    if (*arg) {

        if (!(vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
            send_to_charf(ch, "Вы не видите никого похожего на '%s'.\r\n", arg);
            return;
        }
    } else
        vict = ch;


    if (affected_by_spell(ch, SPELL_BLADE_BARRIER)) {
        send_to_charf(ch, "Барьер из лезвий мешает Вам перевязать раны.\r\n");
        return;
    }
    if (ch != vict && affected_by_spell(vict, SPELL_BLADE_BARRIER)) {
        act("Барьер из лезвий вокруг $N1 мешает Вам перевязать $S раны.", FALSE, ch, 0, vict,
            TO_CHAR);
        return;
    }

    if (GET_HIT(vict) >= GET_REAL_MAX_HIT(vict)) {
        if (vict != ch)
            act("У $N1 нет открытых ран для перевязки.", FALSE, ch, 0, vict, TO_CHAR);
        else
            act("У Вас нет открытых ран для перевязки.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }

    if (affected_by_spell(vict, SPELL_BANDAGE) || AFF_FLAGGED(vict, AFF_BANDAGE)) {
        if (vict != ch)
            act("$N уже перевязан$G бинтами.", FALSE, ch, 0, vict, TO_CHAR);
        else
            act("Вас уже перевязали бинтами.", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }

    percent = number(1, 15);
    std::vector < int >vit;
    std::vector < int >vot;

    //Параметры для атаки
    vit.push_back(GET_REAL_DEX(ch));
    vit.push_back(GET_REAL_WIS(ch));
    //Параметры для защиты
    vot.push_back(GET_REAL_DEX(vict));
    vot.push_back(GET_REAL_WIS(vict));

    prob = calc_like_skill(ch, vict, SKILL_AID, vit, vot);
    if (ch != vict)
        improove_skill(ch, vict, 0, SKILL_AID);
    else
        improove_skill(ch, 0, 0, SKILL_AID);


    timed.skill = SKILL_AID;
    timed.time = 6 * SECS_PER_MUD_TICK;
    timed_to_char(ch, &timed);

    if (percent <= 5)
        prob = 100;
    else if (percent >= 95)
        prob = 0;

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);
    if (prob < percent) {
        act_affect_mess(spellnum, ch, vict, TRUE, TYPE_MESS_FAIL);
    } else {
        af.type = spellnum;
        af.duration = MAX(1, GET_SKILL(ch, SKILL_AID) / 15) * SECS_PER_MUD_TICK;
        af.modifier = MAX(5, GET_SKILL(ch, SKILL_AID) / 3);
        af.location = APPLY_NONE;
        af.main = TRUE;
        af.owner = GET_ID(ch);
        af.battleflag = 0;
        af.bitvector = AFF_BANDAGE;
        affect_to_char(vict, &af);
        act_affect_mess(spellnum, ch, vict, TRUE, TYPE_MESS_HIT);
    }

}


ACMD(do_poisoned)
{
    struct obj_data *obj;
    struct timed_type timed;
    struct C_obj_affected_type af[1];
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    if (!GET_SKILL(ch, SKILL_POISONED)) {
        send_to_char("Вы не знаете как.", ch);
        return;
    }

    one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Что вы хотите отравить?\r\n", ch);
        return;
    }

    if (!IS_GOD(ch) && timed_by_skill(ch, SKILL_POISONED)) {
        send_to_char("Вы рискуете отравиться сами, подождите немного.\r\n", ch);
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        sprintf(buf, "У Вас нет \'%s\'.\r\n", arg);
        send_to_char(buf, ch);
        return;
    };

    if (GET_OBJ_TYPE(obj) != ITEM_WEAPON) {
        send_to_char("Вы можете нанести яд только на оружие.\r\n", ch);
        return;
    }


    af[0].type = find_spell_num(SPELL_POISON);
    af[0].duration = -1;
    af[0].modifier = GET_SKILL(ch, SKILL_POISONED);
    af[0].location = APPLY_POISON;
    af[0].bitvector = 0;
    af[0].extra = 0;
    af[0].no_flag = 0;
    af[0].anti_flag = 0;
    affect_to_object(obj, af);

    timed.skill = SKILL_POISONED;
    timed.time = 1 * SECS_PER_MUD_TICK;
    timed_to_char(ch, &timed);

    act("Вы осторожно нанесли яд на $o3.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n осторожно нанес$q яд на $o3.", FALSE, ch, obj, 0, TO_ROOM);
    GET_MISSED(ch)++;
}

void corpse_desc_rotten(struct obj_data *obj)
{
    struct char_data *mob;
    char buf2[MAX_STRING_LENGTH];
    int mobn;

    *buf2 = '\0';

    if (!IS_CORPSE(obj) || (mobn = GET_OBJ_VAL(obj, 2)) < 0)
        return;

    if (real_mobile(mobn) < 0)
        return;

    mob = (mob_proto + real_mobile(mobn));
    if (GET_RACE(mob) == RACE_CONSTRUCTION)
        return;

    if (IN_ROOM(obj) != NOWHERE) {
        act("$o начал$G гнить, источая зловоние.", FALSE, world[IN_ROOM(obj)].people, obj, 0,
            TO_CHAR);
        act("$o начал$G гнить, источая зловоние.", FALSE, world[IN_ROOM(obj)].people, obj, 0,
            TO_ROOM);
    }

    GET_OBJ_VAL(obj, 3) = 3;    //признак гнилости

    sprintf(buf2, "гниющий %s", DAP(obj->description));
    OBJ_STR_REASSIGN(obj, description, buf2);

    sprintf(buf2, "гниющий %s", DAP(obj->short_description));
    OBJ_STR_REASSIGN(obj, short_description, buf2);

    sprintf(buf2, "гнию(щий) %s", DAP(obj->name));
    OBJ_STR_REASSIGN(obj, names, buf2);

    sprintf(buf2, "гниющий %s", DAP(obj->name));
    OBJ_STR_REASSIGN(obj, PNames[0], buf2);
    OBJ_STR_REASSIGN(obj, name, buf2);

    sprintf(buf2, "гниющего %s", DAP(obj->PNames[1]));
    OBJ_STR_REASSIGN(obj, PNames[1], buf2);

    sprintf(buf2, "гниющему %s", DAP(obj->PNames[2]));
    OBJ_STR_REASSIGN(obj, PNames[2], buf2);

    sprintf(buf2, "гниющий %s", DAP(obj->PNames[3]));
    OBJ_STR_REASSIGN(obj, PNames[3], buf2);

    sprintf(buf2, "гниющим %s", DAP(obj->PNames[4]));
    OBJ_STR_REASSIGN(obj, PNames[4], buf2);

    sprintf(buf2, "гниющем %s", DAP(obj->PNames[5]));
    OBJ_STR_REASSIGN(obj, PNames[5], buf2);
}

void corpse_desc_skelet(struct obj_data *obj)
{
    struct char_data *mob;
    char buf2[MAX_STRING_LENGTH];
    int mobn;

    *buf2 = '\0';

    if (!IS_CORPSE(obj) || (mobn = GET_OBJ_VAL(obj, 2)) < 0)
        return;

    if (real_mobile(mobn) < 0)
        return;

    mob = (mob_proto + real_mobile(mobn));
    if (GET_RACE(mob) == RACE_CONSTRUCTION ||
        GET_RACE(mob) == RACE_INSECT || GET_RACE(mob) == RACE_SLIME || GET_RACE(mob) == RACE_PLANT)
        return;

    if (IN_ROOM(obj) != NOWHERE) {
        act("Плоть отвалилась от $o1, обнажив кости.", TRUE, world[IN_ROOM(obj)].people, obj, 0,
            TO_CHAR);
        act("Плоть отвалилась от $o1, обнажив кости.", TRUE, world[IN_ROOM(obj)].people, obj, 0,
            TO_ROOM);
    }

    GET_OBJ_VAL(obj, 3) = 4;    //признак скелета

    sprintf(buf2, "скелет %s лежит здесь.", GET_PAD(mob, 1));
    OBJ_STR_REASSIGN(obj, description, buf2);

    sprintf(buf2, "скелет() %s", GET_PAD(mob, 1));
    OBJ_STR_REASSIGN(obj, names, buf2);

    sprintf(buf2, "скелет %s", GET_PAD(mob, 1));
    OBJ_STR_REASSIGN(obj, short_description, buf2);

    sprintf(buf2, "скелет %s", GET_PAD(mob, 1));
    OBJ_STR_REASSIGN(obj, PNames[0], buf2);
    OBJ_STR_REASSIGN(obj, name, buf2);

    sprintf(buf2, "скелета %s", GET_PAD(mob, 1));
    OBJ_STR_REASSIGN(obj, PNames[1], buf2);

    sprintf(buf2, "скелету %s", GET_PAD(mob, 1));
    OBJ_STR_REASSIGN(obj, PNames[2], buf2);

    sprintf(buf2, "скелет %s", GET_PAD(mob, 1));
    OBJ_STR_REASSIGN(obj, PNames[3], buf2);

    sprintf(buf2, "скелетом %s", GET_PAD(mob, 1));
    OBJ_STR_REASSIGN(obj, PNames[4], buf2);

    sprintf(buf2, "скелете %s", GET_PAD(mob, 1));
    OBJ_STR_REASSIGN(obj, PNames[5], buf2);

    GET_OBJ_WEIGHT(obj) = MAX(1, GET_OBJ_WEIGHT(obj) / 10);
}

void corpse_desc_poor(struct obj_data *obj)
{
    char buf2[MAX_STRING_LENGTH];

    sprintf(buf2, "растерзанный %s", DAP(obj->description));
    OBJ_STR_REASSIGN(obj, description, buf2);

    sprintf(buf2, "растерзанный %s", DAP(obj->short_description));
    OBJ_STR_REASSIGN(obj, short_description, buf2);

    sprintf(buf2, "растерзан(ый) %s", DAP(obj->name));
    OBJ_STR_REASSIGN(obj, names, buf2);

    sprintf(buf2, "растерзанный %s", DAP(obj->name));
    OBJ_STR_REASSIGN(obj, PNames[0], buf2);
    OBJ_STR_REASSIGN(obj, name, buf2);

    sprintf(buf2, "растерзанного %s", DAP(obj->PNames[1]));
    OBJ_STR_REASSIGN(obj, PNames[1], buf2);

    sprintf(buf2, "растерзанному %s", DAP(obj->PNames[2]));
    OBJ_STR_REASSIGN(obj, PNames[2], buf2);

    sprintf(buf2, "растерзанный %s", DAP(obj->PNames[3]));
    OBJ_STR_REASSIGN(obj, PNames[3], buf2);

    sprintf(buf2, "растерзанным %s", DAP(obj->PNames[4]));
    OBJ_STR_REASSIGN(obj, PNames[4], buf2);

    sprintf(buf2, "растерзанном %s", DAP(obj->PNames[5]));
    OBJ_STR_REASSIGN(obj, PNames[5], buf2);
}

void corpse_desc_good(struct obj_data *obj)
{
    char buf2[MAX_STRING_LENGTH];

    *buf2 = '\0';

    sprintf(buf2, "выпотрошенный %s", DAP(obj->description));
    OBJ_STR_REASSIGN(obj, description, buf2);

    sprintf(buf2, "выпотрошенный %s", DAP(obj->short_description));
    OBJ_STR_REASSIGN(obj, short_description, buf2);

    sprintf(buf2, "выпотрошенн(ый) %s", DAP(obj->name));
    OBJ_STR_REASSIGN(obj, names, buf2);

    sprintf(buf2, "выпотрошенный %s", DAP(obj->name));
    OBJ_STR_REASSIGN(obj, PNames[0], buf2);
    OBJ_STR_REASSIGN(obj, name, buf2);

    sprintf(buf2, "выпотрошенного %s", DAP(obj->PNames[1]));
    OBJ_STR_REASSIGN(obj, PNames[1], buf2);

    sprintf(buf2, "выпотрошенному %s", DAP(obj->PNames[2]));
    OBJ_STR_REASSIGN(obj, PNames[2], buf2);

    sprintf(buf2, "выпотрошенный %s", DAP(obj->PNames[3]));
    OBJ_STR_REASSIGN(obj, PNames[3], buf2);

    sprintf(buf2, "выпотрошенным %s", DAP(obj->PNames[4]));
    OBJ_STR_REASSIGN(obj, PNames[4], buf2);

    sprintf(buf2, "выпотрошенном %s", DAP(obj->PNames[5]));
    OBJ_STR_REASSIGN(obj, PNames[5], buf2);
}

void meet_desc_change(struct obj_data *obj, struct char_data *mob, int vnum)
{

    char buf2[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];

    *buf = '\0';
    *buf2 = '\0';

    if (!mob)
        return;


//sprintf(buf2, "%s %s", obj->name,GET_PAD(mob,1));
    switch (GET_SEX(mob)) {
        case SEX_FEMALE:
            sprintf(buf2, "%s %s", obj->name,
                    get_name_pad((char *) race_name_pad_female[(int) GET_RACE(mob)], PAD_ROD,
                                 PAD_MONSTER));
            break;
        default:
            sprintf(buf2, "%s %s", obj->name,
                    get_name_pad((char *) race_name_pad_male[(int) GET_RACE(mob)], PAD_ROD,
                                 PAD_MONSTER));
            break;
    }

    obj->name = str_dup(buf2);


//sprintf(buf2, "%s %s", obj->short_description,GET_PAD(mob,1));
    switch (GET_SEX(mob)) {
        case SEX_FEMALE:
            sprintf(buf2, "%s %s", obj->short_description,
                    get_name_pad((char *) race_name_pad_female[(int) GET_RACE(mob)], PAD_ROD,
                                 PAD_MONSTER));
            break;
        default:
            sprintf(buf2, "%s %s", obj->short_description,
                    get_name_pad((char *) race_name_pad_male[(int) GET_RACE(mob)], PAD_ROD,
                                 PAD_MONSTER));
            break;
    }

    obj->short_description = str_dup(buf2);

    obj->PNames[0] = str_dup(buf2);

    switch (GET_SEX(mob)) {
        case SEX_FEMALE:
            sprintf(buf, "%s",
                    get_name_pad((char *) race_name_pad_female[(int) GET_RACE(mob)], PAD_ROD,
                                 PAD_MONSTER));
            break;
        default:
            sprintf(buf, "%s",
                    get_name_pad((char *) race_name_pad_male[(int) GET_RACE(mob)], PAD_ROD,
                                 PAD_MONSTER));
            break;
    }


    sprintf(buf2, "%s %s", obj->names, buf);
    obj->names = str_dup(buf2);

    sprintf(buf2, "%s %s", obj->PNames[1], buf);
    obj->PNames[1] = str_dup(buf2);

    sprintf(buf2, "%s %s", obj->PNames[2], buf);
    obj->PNames[2] = str_dup(buf2);

    sprintf(buf2, "%s %s", obj->PNames[3], buf);
    obj->PNames[3] = str_dup(buf2);

    sprintf(buf2, "%s %s", obj->PNames[4], buf);
    obj->PNames[4] = str_dup(buf2);

    sprintf(buf2, "%s %s", obj->PNames[5], buf);
    obj->PNames[5] = str_dup(buf2);

    obj->description = str_dup(obj->description);
    obj->action_description = str_dup(obj->action_description);
    obj->ex_description = NULL;

}
const int meet_vnum[] = { 131, 132, 133, 134 };

ACMD(do_makefood)
{
    struct obj_data *obj, *tobj;
    struct char_data *mob;
    int prob, percent = 0, mobn, wgt = 0;
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    if (!check_fight_command(ch))
        return;

    if (!GET_SKILL(ch, SKILL_MAKEFOOD)) {
        send_to_char("Вы не умеете этого.\r\n", ch);
        return;
    }

    one_argument(argument, arg);
    if (!*arg) {
        send_to_char("Что Вы хотите разделать?\r\n", ch);
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying)) &&
        !(obj = get_obj_in_list_vis(ch, arg, world[ch->in_room].contents))) {
        sprintf(buf, "Вы не видите здесь '%s'.\r\n", arg);
        send_to_char(buf, ch);
        return;
    }

    if (!IS_CORPSE(obj) || (mobn = GET_OBJ_VAL(obj, 2)) < 0) {
        act("Вы не сможете разделать $o3.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (real_mobile(mobn) < 0) {
        send_to_char("Этот труп невозможно разделать.\r\n", ch);
        return;
    }

    mob = (mob_proto + real_mobile(mobn));
    if ((GET_RACE(mob) == RACE_GENASI) ||
        (GET_RACE(mob) == RACE_INSECT) ||
        (GET_RACE(mob) == RACE_SLIME) ||
        (GET_RACE(mob) == RACE_PLANT) ||
        (GET_RACE(mob) == RACE_CONSTRUCTION) || (wgt = GET_OBJ_WEIGHT(obj)) < 1) {
        send_to_char("Этот труп невозможно разделать.\r\n", ch);
        return;
    }
    if (GET_OBJ_VAL(obj, 3) == 2) {
        send_to_charf(ch, "Над этим трупом уже кто-то поработал.\r\n");
        return;
    } else if (GET_OBJ_VAL(obj, 3) == 4) {
        send_to_charf(ch, "На этих костях не осталось мяса.\r\n");
        return;
    }


    prob = number(1, 100);
    std::vector < int >vit;
    std::vector < int >vot;

    vit.push_back(GET_REAL_DEX(ch));
    vit.push_back(GET_REAL_WIS(ch));
    vot.push_back(dice(1, 40));

    percent = calc_like_skill(ch, 0, SKILL_MAKEFOOD, vit, vot);
    improove_skill(ch, mob, 0, SKILL_MAKEFOOD);


    if (percent <= 5)
        prob = 100;
    else if (percent >= 95)
        prob = 0;

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", percent, prob);

    int vnum = meet_vnum[number(0, MIN(1, MAX(0, (wgt - 80) / 5)))];

    if (prob > percent || !(tobj = read_object(vnum, VIRTUAL, TRUE))) {
        act("Вы не сумели разделать $o3.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n попытал$u разделать $o3, но только порезал$u.", FALSE, ch, obj, 0, TO_ROOM);
        corpse_desc_poor(obj);
        GET_OBJ_VAL(obj, 3) = 2;
    } else {
        sprintf(buf, "$n умело вырезал$g %s из $o1.", tobj->PNames[3]);
        act(buf, FALSE, ch, obj, 0, TO_ROOM);
        sprintf(buf, "Вы умело вырезали %s из $o1.", tobj->PNames[3]);
        act(buf, FALSE, ch, obj, 0, TO_CHAR);
        if (GET_OBJ_VAL(obj, 3) == 3)
            GET_OBJ_VAL(tobj, 3) = MAX(1, 13 - GET_OBJ_TIMER(obj)) * 10;

        GET_OBJ_VAL(obj, 3) = 2;

        corpse_desc_good(obj);
        meet_desc_change(tobj, mob, vnum);

        if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch)) {
            send_to_char("Вы не можете нести столько предметов.\r\n", ch);
            obj_to_room(tobj, IN_ROOM(ch));
            obj_decay(tobj);
            return;
        } else if (GET_OBJ_WEIGHT(tobj) + IS_CARRYING_W(ch) > CAN_CARRY_W(ch)) {
            send_to_charf(ch, "Вам слишком тяжело нести еще и %s.", tobj->PNames[3]);
            obj_to_room(tobj, IN_ROOM(ch));
//       obj_decay(tobj);
            return;
        } else
            obj_to_char(tobj, ch);
    }
    if (obj->carried_by) {
        obj_from_char(obj);
        obj_to_room(obj, IN_ROOM(ch));
    }
//  extract_obj(obj);
}


#define MAX_ITEMS 9
#define MAX_PROTO 3
#define COAL_PROTO  311
#define WOOD_PROTO      313
#define TETIVA_PROTO    314
#define ONE_DAY     24*60

struct create_item_type {
    int obj_vnum;
    int material_bits;
    int min_weight;
    int max_weight;
    int proto[MAX_PROTO];
    int skill;
    int wear;
};

int ext_search_block(char *arg, const char **list, int exact)
{
    register int i, l, j, o;

    /* Make into lower case, and get length of string */
    for (l = 0; *(arg + l); l++)
        *(arg + l) = LOWER(*(arg + l));

    if (exact) {
        for (i = j = 0, o = 1; j != 1; i++)
            if (**(list + i) == '\n') {
                o = 1;
                switch (j) {
                    case 0:
                        j = INT_ONE;
                        break;
                    case INT_ONE:
                        j = INT_TWO;
                        break;
                    case INT_TWO:
                        j = INT_THREE;
                        break;
                    default:
                        j = 1;
                        break;
                }
            } else if (!str_cmp(arg, *(list + i)))
                return (j | o);
            else
                o <<= 1;
    } else {
        if (!l)
            l = 1;              /* Avoid "" to match the first available
                                 * string */
        for (i = j = 0, o = 1; j != 1; i++)
            if (**(list + i) == '\n') {
                o = 1;
                switch (j) {
                    case 0:
                        j = INT_ONE;
                        break;
                    case INT_ONE:
                        j = INT_TWO;
                        break;
                    case INT_TWO:
                        j = INT_THREE;
                        break;
                    default:
                        j = 1;
                        break;
                }
            } else if (!strn_cmp(arg, *(list + i), l))
                return (j | o);
            else
                o <<= 1;
    }

    return (0);
}


void perform_remove_tatoo(struct char_data *ch, int pos)
{
    struct mess_p_data *k;
    int found = 0, count = 0;

    struct obj_data *obj;

    if (!(obj = GET_TATOO(ch, pos)))
        log("SYSERR: perform_remove: bad pos %d passed.", pos);
    else
        /*
           if (IS_OBJ_STAT(obj, ITEM_NODROP))
           act("Вы не можете снять $o3!", FALSE, ch, obj, 0, TO_CHAR);
           else
         */
    if (IS_CARRYING_N(ch) >= CAN_CARRY_N(ch))
        act("$p: Вы не можете нести столько вещей!", FALSE, ch, obj, 0, TO_CHAR);
    else {
        /* Проверка на перехват сообщений */
        for (k = obj->mess_data; k; k = k->next, count++)
            if (k->command == CMD_REMOVE) {
                found = 1;
                break;
            }

        if (found && k->stoping) {
            act(k->mess_to_char, "Мп", ch, obj);
            act(k->mess_to_room, "Кмп", ch, obj);
            return;
        }

        obj_to_char(unequip_tatoo(ch, pos), ch);
        if (found) {
            act(k->mess_to_char, "Мп", ch, obj);
            act(k->mess_to_room, "Кмп", ch, obj);
        } else {
            act("Вы стерли @1в.", "Мп", ch, obj);
            act("1и стер1(,ла,ло,ли) @1в.", "Кмп", ch, obj);
        }
        extract_obj(obj);
    }
}

int perform_tatoo(struct char_data *ch, struct char_data *keeper, struct obj_data *obj, int where)
{
    struct mess_p_data *k;
    int found = 0, count = 0;

    int wear_bitvectors[] = { ITEM_WEAR_TAKE, ITEM_WEAR_FINGER, ITEM_WEAR_FINGER, ITEM_WEAR_FACE,
        ITEM_WEAR_EARS, ITEM_WEAR_EARS, ITEM_WEAR_NECK,
        ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS,
        ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_SHIELD,
        ITEM_WEAR_ABOUT, ITEM_WEAR_WAIST, ITEM_WEAR_WRIST, ITEM_WEAR_WRIST,
        ITEM_WEAR_WIELD, ITEM_WEAR_TAKE, ITEM_WEAR_BOTHS, ITEM_WEAR_TAIL
    };

    const char *already_wearing[] = { "ОШИБКА 501. Сообщите богам.\r\n",
        "У Вас уже что-то наколото на пальцах.\r\n",
        "У Вас уже что-то наколото на пальцах.\r\n",
        "У Вас уже что-то наколото на лице.\r\n",
        "У Вас уже что-то наколото на ушах.\r\n",
        "У Вас уже что-то наколото на ушах.\r\n",
        "У Вас уже что-то наколото на шее.\r\n",
        "У Вас уже что-то наколото на груди.\r\n",
        "У Вас уже что-то наколото на груди.\r\n",
        "У Вас уже что-то наколото на голове.\r\n",
        "У Вас уже что-то наколото на бедре.\r\n",
        "У Вас уже что-то наколото на икрах.\r\n",
        "У Вас уже что-то наколото на плече.\r\n",
        "У Вас уже что-то наколото на ладонях.\r\n",
        "ОШИБКА 501. Сообщите богам.\r\n",
        "ОШИБКА 502. Сообщите богам.\r\n",
        "ОШИБКА 503. Сообщите богам.\r\n",
        "У Вас уже что-то наколото на запястьях.\r\n",
        "У Вас уже что-то наколото на запястьях.\r\n",
        "ОШИБКА 504. Сообщите богам.\r\n",
        "ОШИБКА 505. Сообщите богам.\r\n",
        "ОШИБКА 506. Сообщите богам.\r\n",
        "У Вас уже что-то наколото на хвосте.\r\n"
    };

    if (!CAN_WEAR(obj, wear_bitvectors[where])) {
        act("Нельзя наколоть $o3 на эту часть тела.", FALSE, ch, obj, 0, TO_CHAR);
        return (FALSE);
    }

    if ((where == WEAR_FINGER_R) ||
        (where == WEAR_EAR_R) || (where == WEAR_NECK_1) || (where == WEAR_WRIST_R))
        if (GET_TATOO(ch, where))
            where++;

    if (GET_TATOO(ch, where)) {
        send_to_char(already_wearing[where], ch);
        return (FALSE);
    }

    for (k = obj->mess_data; k; k = k->next, count++)
        if (k->command == CMD_WEAR) {
            found = 1;
            break;
        }

    if (found && k->stoping) {
        if (k->mess_to_char)
            act(k->mess_to_char, "М", ch);
        if (k->mess_to_room)
            act(k->mess_to_room, "Км", ch);
        return (FALSE);
    }

    obj_from_char(obj);
    if (preequip_tatoo(ch, obj, where) && obj->worn_by == ch) {
        if (found) {
            if (k->mess_to_char)
                act(k->mess_to_char, "М", ch);
            if (k->mess_to_room)
                act(k->mess_to_room, "Км", ch);
        } else {
            wear_tatoo_message(ch, keeper, obj, where);
        }
        postequip_char(ch, obj);
    }

    return (TRUE);
}

int tatoo_to_char(struct char_data *ch, struct char_data *keeper, struct obj_data *obj)
{
    int where = find_eq_pos(ch, obj, 0);

    if (!keeper)
        keeper = ch;

    if (where >= 0) {
        if (!perform_tatoo(ch, keeper, obj, where))
            return (FALSE);
    } else {
        act("Не получилось наколоть $o3.", FALSE, ch, obj, 0, TO_CHAR);
        return (FALSE);
    }

    return (TRUE);
}


ACMD(do_tatoo)
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    struct char_data *victim = NULL;
    struct obj_data *obj;

    two_arguments(argument, arg1, arg2);

    if (!*arg1) {
        send_to_charf(ch, "Формат: наколоть что [кому]\r\n");
        send_to_charf(ch, "Что Вы хотите наколоть?\r\n");
        return;
    } else
        obj = get_obj_in_list_vis(ch, arg1, ch->carrying);

    if (!obj) {
        send_to_charf(ch, "У Вас нет '%s', что бы наколоть.\r\n", arg1);
        return;
    }

    if (GET_OBJ_TYPE(obj) != ITEM_TATOO) {
        send_to_charf(ch, "Наколоть можно только татуировку.\r\n");
        return;
    }

    if (!*arg2)
        victim = ch;
    else
        victim = get_player_vis(ch, arg2, FIND_CHAR_WORLD);

    if (!victim) {
        send_to_charf(ch, "Персонаж с именем '%s' не найден.\r\n", arg2);
        return;
    }

    if (tatoo_to_char(victim, ch, obj)) {
        if (victim != ch)
            act("Вы накололи @1и 2д.", "Ммп", ch, victim, obj);
    } else
        act("Вы не смогли наколоть @1и 2д.", "Ммп", ch, victim, obj);
}

ACMD(do_remove_tatoo)
{
    int i, dotmode, found;
    struct char_data *vict;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    half_chop(argument, buf2, buf);

    if (!*buf) {
        send_to_charf(ch, "Формат: вывести что [с кого]\r\n");
        return;
    }

    if (!(vict = get_player_vis(ch, buf, FIND_CHAR_WORLD))) {
        send_to_char(NOPERSON, ch);
        return;
    }

    one_argument(buf2, arg);

    if (!*arg) {
        send_to_char("Стереть что?\r\n", ch);
        return;
    }
    dotmode = find_all_dots(arg);

    if (dotmode == FIND_ALL) {
        found = 0;
        for (i = 0; i < NUM_WEARS; i++)
            if (GET_TATOO(vict, i)) {
                perform_remove_tatoo(vict, i);
                found = 1;
            }
        if (!found)
            send_to_charf(ch, "На %s нет такого предмета.\r\n", GET_NAME(vict));
    } else if (dotmode == FIND_ALLDOT) {
        if (!*arg)
            send_to_char("Снять все передметы какого типа?\r\n", ch);
        else {
            found = 0;
            for (i = 0; i < NUM_WEARS; i++)
                if (GET_TATOO(vict, i) && CAN_SEE_OBJ(vict, GET_TATOO(vict, i)) &&
                    isname(arg, GET_EQ(vict, i)->name)) {
                    perform_remove_tatoo(vict, i);
                    found = 1;
                }
            if (!found) {
                send_to_charf(ch, "На %s нет ни одного '%s'.\r\n", GET_NAME(vict), arg);
            }
        }
    } else {
        if ((i = get_obj_pos_in_equip_vis(vict, arg, NULL, ch->tatoo)) < 0) {
            sprintf(buf, "%s не использует '%s'.\r\n", GET_NAME(vict), arg);
            send_to_char(buf, ch);
        } else
            perform_remove_tatoo(vict, i);
    }
}

void go_find_all(struct char_data *ch)
{
    int percent, prob, found = FALSE;
    struct obj_data *tobj;
    struct char_data *tch, *k;
    struct follow_type *f;

//struct timed_type timed;
    char buf[MAX_STRING_LENGTH];

    *buf = '\0';

    if (!check_moves(ch, FIND_MOVES))
        return;

    percent = number(1, 100);

    act("Вы принялись внимательно изучать комнату.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n принял$u внимательно изучать комнату.", FALSE, ch, 0, 0, TO_ROOM);

    //поиск предметов
    for (tobj = world[IN_ROOM(ch)].contents; tobj; tobj = tobj->next_content) {

        if (!IS_OBJ_STAT(tobj, ITEM_HIDDEN) ||
            (IS_OBJ_STAT(tobj, ITEM_HIDDEN) && check_obj_visible(ch, tobj)))
            continue;

        std::vector < int >vit;
        std::vector < int >vot;

        vit.push_back(GET_REAL_DEX(ch));
        vit.push_back(GET_REAL_INT(ch));
        vot.push_back(GET_OBJ_QUALITY(tobj) * 6);

        prob = calc_like_skill(ch, 0, SKILL_FIND, vit, vot);
        improove_skill(ch, 0, GET_OBJ_QUALITY(tobj) * 3, SKILL_FIND);


        if (percent <= 5)
            prob = 100;
        else if (percent >= 95)
            prob = 0;

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);

        if (prob >= percent) {
            sprintf(buf + strlen(buf), " %s\r\n", GET_OBJ_PNAME(tobj, 0));
            add_obj_visible(tobj, ch);
            found = TRUE;
            if (AFF_FLAGGED(ch, AFF_GROUP)) {
                k = (ch->party_leader ? ch->party_leader : ch);
                add_obj_visible(tobj, k);
                for (f = k->followers; f; f = f->next) {
                    if (AFF_FLAGGED(f->follower, AFF_GROUP) && GET_POS(f->follower) > POS_SLEEPING)
                        add_obj_visible(tobj, f->follower);
                }
            }
        }
    }
    //поиск людей
    for (tch = world[IN_ROOM(ch)].people; tch; tch = tch->next_in_room) {
        if (tch == ch)
            continue;
        if (!AFF_FLAGGED(tch, AFF_HIDE) ||
            (AFF_FLAGGED(tch, AFF_HIDE)
             && (check_victim_visible(ch, tch) || AFF_FLAGGED(ch, AFF_SENSE_LIFE))))
            continue;

        percent = number(1, 100);
        std::vector < int >vit;
        std::vector < int >vot;

        vit.push_back(GET_REAL_DEX(ch));
        vit.push_back(GET_REAL_INT(ch));
        vot.push_back(GET_REAL_DEX(tch));
        vot.push_back(GET_REAL_INT(tch));
        prob = calc_like_skill(ch, tch, SKILL_FIND, vit, vot);

        if (percent <= 5)
            prob = 100;
        else if (percent >= 95)
            prob = 0;

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);

        if (prob >= percent) {
            found = TRUE;
            sprintf(buf + strlen(buf), " %s\r\n",
                    (check_incognito(tch) ? CAP(hide_race(tch, 3)) : CAP(race_or_title(tch, 3))));

            add_victim_visible(ch, tch);
        }
    }

    if (!found) {
        send_to_charf(ch, "После длительных поисков Вы ничего не нашли.\r\n");
    } else {
        send_to_charf(ch, "После длительных поисков Вы нашли:\r\n%s", buf);
    }

}

void go_find_door(struct char_data *ch, int door)
{
    char door_name[256];
    char buf[MAX_STRING_LENGTH];
    int percent, prob;
    bool found = FALSE;

    if (!check_moves(ch, FIND_MOVES))
        return;

    if (EXIT(ch, (int) door)->exit_name)
        sprintf(door_name, get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
    else
        sprintf(door_name, "проход %s", DirsTo_2[door]);

    if (check_tfind_char(ch, EXIT(ch, (int) door))) {
        if (EXIT(ch, (int) door)->exit_name)
            sprintf(door_name, "на %s",
                    get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_ROD, PAD_OBJECT));
        else
            sprintf(door_name, "в проходе %s", DirsTo_2[door]);

        send_to_charf(ch, "Вы уже нашли ловушку %s.\r\n", door_name);
        return;
    }
    sprintf(buf, "$n принял$u внимательно осматривать %s.", door_name);
    act(buf, TRUE, ch, 0, 0, TO_ROOM);

    if (EXIT(ch, (int) door)->shance && !EXIT(ch, (int) door)->tbreak) {        //ловушка есть
        percent = number(1, 100);
        std::vector < int >vit;
        std::vector < int >vot;

        vit.push_back(GET_REAL_DEX(ch));
        vit.push_back(GET_REAL_INT(ch));
        vot.push_back(EXIT(ch, (int) door)->quality / 2);
        prob = calc_like_skill(ch, 0, SKILL_FIND, vit, vot);
        improove_skill(ch, 0, EXIT(ch, (int) door)->quality / 3, SKILL_FIND);

        if (percent <= 5)
            prob = 100;
        else if (percent >= 95)
            prob = 0;

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);
        if (prob >= percent) {
            found = TRUE;
            add_tfind_char(ch, EXIT(ch, (int) door));
        }
    }

    if (found) {
        send_to_charf(ch, "Внимательно осмотрев %s, Вы обнаружили скрытую ловушку.\r\n", door_name);
        int exp =
            (((get_levelexp(ch, GET_LEVEL(ch), 1) / 200) * EXIT(ch, (int) door)->quality) / 100);
        gain_exp(ch, exp, TRUE);

    } else
        send_to_charf(ch, "Осмотрев %s, Вы не заметили ничего подозрительного.\r\n", door_name);

}

void go_find_container(struct char_data *ch, struct obj_data *obj)
{
    int percent, prob;
    bool found = FALSE;

    if (!check_moves(ch, FIND_MOVES))
        return;

    if (obj->trap && check_tfind_char(ch, obj)) {
        act("Вы уже нашли ловушку в $o5.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    act("$n принял$u внимательно осматривать $o.", TRUE, ch, obj, 0, TO_ROOM);

    if (obj->trap && !obj->trap->tbreak) {      //ловушка есть
        percent = number(1, 100);
        std::vector < int >vit;
        std::vector < int >vot;

        vit.push_back(GET_REAL_DEX(ch));
        vit.push_back(GET_REAL_INT(ch));
        vot.push_back(GET_OBJ_QUALITY(obj) * 6);
        prob = calc_like_skill(ch, 0, SKILL_FIND, vit, vot);
        improove_skill(ch, 0, GET_OBJ_QUALITY(obj) * 3, SKILL_FIND);

        if (percent <= 5)
            prob = 100;
        else if (percent >= 95)
            prob = 0;

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);
        if (prob >= percent) {
            found = TRUE;
            add_tfind_char(ch, obj);
        }
    }

    if (found) {
        act("Внимательно осмотрев $o1, Вы обнаружили скрытую ловушку.", FALSE, ch, obj, 0, TO_CHAR);
        int exp = (((get_levelexp(ch, GET_LEVEL(ch), 1) / 200) * GET_OBJ_QUALITY(obj)) / 10);

        gain_exp(ch, exp, TRUE);
    } else
        act("Осмотрев $o1, Вы не заметили ничего подозрительного.", FALSE, ch, obj, 0, TO_CHAR);

}

ACMD(do_find)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;

    if (!check_fight_command(ch))
        return;

    if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_FIND)) {
        send_to_char("Но Вы не знаете как.\r\n", ch);
        return;
    }

    if (on_horse(ch)) {
        send_to_char("Для этого необходимо спешиться.\r\n", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_BLADE_BARRIER)) {
        send_to_charf(ch, "Барьер из лезвий мешает Вам предпринять поиски.\r\n");
        return;
    }


    if (!*argument)
        go_find_all(ch);
    else {
        two_arguments(argument, type, dir);

        if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
            door = a_find_door(ch, type, dir);

        if (door == -2) {
            send_to_charf(ch, "Вы не видите здесь предмет Ваших поисков.\r\n");
            return;
        }

        if (!obj && door == -2)
            return;

        if (!obj && door >= 0)
            go_find_door(ch, door);
        else if (obj)
            go_find_container(ch, obj);
    }
}

//Возвращает направление в котором находится дверь + поиск по названию выхода
int a_find_door(struct char_data *ch, const char *type, char *dir)
{
    int door;

    if (*dir) {
        if ((door = search_block(dir, dirs, FALSE)) == -1 &&
            (door = search_block(dir, DirIs, FALSE)) == -1) {
            send_to_charf(ch, "Уточните направление.\r\n");
            return (-1);
        }
        if (EXIT(ch, (int) door)) {
            if (isname(type, EXIT(ch, (int) door)->keyword))
                return (door);
            else {
                send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", type);
                return (-1);
            }
        }
    } else {
        if (!*type)
            return (-2);
        for (door = 0; door < NUM_OF_DIRS; door++) {
            if (EXIT(ch, (int) door)) {
                if (EXIT(ch, (int) door)->keyword) {
                    if (isname(type, EXIT(ch, (int) door)->keyword))
                        return (door);
                } else if (isname(type, DirIs[door]))
                    return (door);
            }
        }

        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", type);
        return (-1);
    }

    return (-1);
}

//Возвращает направление в котором находится дверь
int _find_door(struct char_data *ch, const char *type, char *dir)
{
    int door;

    if (*dir) {
        if ((door = search_block(dir, dirs, FALSE)) == -1 &&
            (door = search_block(dir, DirIs, FALSE)) == -1) {
            send_to_charf(ch, "Уточните направление.\r\n");
            return (-1);
        }
        if (EXIT(ch, (int) door)) {
            if (isname(type, EXIT(ch, (int) door)->keyword))
                return (door);
            else {
                send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", type);
                return (-1);
            }
        }
    } else {
        if (!*type)
            return (-2);
        for (door = 0; door < NUM_OF_DIRS; door++) {
            if (EXIT(ch, (int) door)) {
                if (EXIT(ch, (int) door)->keyword) {
                    if (isname(type, EXIT(ch, (int) door)->keyword))
                        return (door);
                }
            }
        }

        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", type);
        return (-1);
    }

    return (-1);
}

void exit_trap_active(struct char_data *ch, int door, bool tbreak)
{
    int dam = 0, type_hit = 0, save = 0;
    struct P_damage damage;
    struct P_message message;

    if (EXIT(ch, (int) door)->shance >= number(1, 100) && !EXIT(ch, (int) door)->tbreak) {
        type_hit = EXIT(ch, (int) door)->type_hit;
        dam =
            dice(EXIT(ch, (int) door)->damnodice, EXIT(ch, (int) door)->damsizedice) + EXIT(ch,
                                                                                            (int)
                                                                                            door)->
            damage;
        save = EXIT(ch, (int) door)->quality / 2;
        save = general_savingthrow_3(ch, SAV_REFL, save);

        GetExitTrapMessage(EXIT(ch, (int) door), message);

        damage.valid = true;
        damage.type = type_hit;
        damage.power = 0;
        damage.far_min = TRUE;
        damage.armor = FALSE;   //броня учитывается
        damage.weapObj = NULL;
        damage.dam = dam;

        if (!save)
            damage.check_ac = A_POWER;
        else
            damage.check_ac = N_POWER;

        if (_damage(ch, ch, 0, 0, A_POWER, FALSE, damage, message) == RD_KILL) {
            //sprintf(buf,"%s умер от ловушки в локации #%d",GET_NAME(ch),world[inroom].number);
            //mudlog(buf,CMP,LVL_GOD,TRUE);
        }
    }

    if (tbreak)
        EXIT(ch, (int) door)->tbreak = TRUE;
}

void container_trap_active(struct char_data *ch, struct obj_data *obj, bool tbreak)
{
    int dam = 0, type_hit = 0, save = 0;
    struct obj_trap_data *trap = obj->trap;
    struct P_damage damage;
    struct P_message message;

    if (trap->shance >= number(1, 100) && !trap->tbreak) {
        type_hit = trap->type_hit;
        dam = dice(trap->damnodice, trap->damsizedice) + trap->damage;
        save = GET_OBJ_QUALITY(obj) * 6;
        save = general_savingthrow_3(ch, SAV_REFL, save);

        GetObjTrapMessage(trap, message);

        damage.valid = true;
        damage.type = type_hit;
        damage.power = 0;
        damage.far_min = TRUE;
        damage.armor = FALSE;   //броня учитывается
        damage.weapObj = obj;
        damage.dam = dam;

        if (!save)
            damage.check_ac = A_POWER;
        else
            damage.check_ac = N_POWER;

        if (_damage(ch, ch, 0, 0, A_POWER, FALSE, damage, message) == RD_KILL) {
            //sprintf(buf,"%s умер от ловушки на предмете %s в локации #%d",GET_NAME(ch),GET_OBJ_PNAME(obj,0),world[inroom].number);
            //mudlog(buf,CMP,LVL_GOD,TRUE);
        }
    }

    if (tbreak)
        trap->tbreak = TRUE;
}

/************************ OPEN *************************************/

void go_open_container(struct char_data *ch, struct obj_data *obj)
{
    bool found = FALSE;
    int count;
    struct mess_p_data *k;

    /* Проверка на перехват сообщений */
    for (k = obj->mess_data; k; k = k->next, count++)
        if (k->command == CMD_OPEN) {
            found = TRUE;
            break;
        }

    if (found && k->stoping) {
        if (k->mess_to_char)
            act(k->mess_to_char, "Мп", ch, obj);
        if (k->mess_to_room && !obj->worn_by && !obj->carried_by)
            act(k->mess_to_room, "Кмп", ch, obj);
        if (k->script)
            go_script(k->script, ch, obj);
        return;
    }

    if (OBJVAL_FLAGGED(obj, EXIT_CRASHED)) {
        act("$o выломан$G.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }


    if (OBJVAL_FLAGGED(obj, EXIT_CLOSED)) {
        if (GET_OBJ_VAL(obj, 2) && OBJVAL_FLAGGED(obj, EXIT_LOCKED)) {
            act("$o заперт$G.", FALSE, ch, obj, 0, TO_CHAR);
            return;
        } else {

            if (found && k->mess_to_char)
                act(k->mess_to_char, "Мп", ch, obj);
            else
                act("Вы открыли @1в.", "Мп", ch, obj);

            if (found && k->mess_to_room && !obj->worn_by && !obj->carried_by)
                act(k->mess_to_room, "Кмп", ch, obj);
            else if (!obj->worn_by && !obj->carried_by)
                act("1и открыл1(,а,о,и) @1в.", "Кмп", ch, obj);

            if (found && k->script)
                go_script(k->script, ch, obj);

            REMOVE_BIT(OBJVAL_FLAGS(obj, EXIT_CLOSED), EXIT_CLOSED);
            if (obj->trap)
                container_trap_active(ch, obj, FALSE);
        }
    } else
        act("$o уже открыт$G.", FALSE, ch, obj, 0, TO_CHAR);

}

void go_open_fiction(struct char_data *ch, struct obj_data *obj)
{
    bool found = FALSE;
    int count;
    struct mess_p_data *k;

    /* Проверка на перехват сообщений */
    for (k = obj->mess_data; k; k = k->next, count++)
        if (k->command == CMD_OPEN) {
            found = TRUE;
            break;
        }

    if (found && k->stoping) {
        if (k->mess_to_char)
            act(k->mess_to_char, "Мп", ch, obj);
        if (k->mess_to_room)
            act(k->mess_to_room, "Кмп", ch, obj);
        return;
    }

    if (CAN_WEAR(obj, ITEM_WEAR_TAKE))
        if (!obj->worn_by && !obj->carried_by) {
            act("Чтобы открыть $o3 необходимо взять $S в руки.", FALSE, ch, obj, 0, TO_CHAR);
            return;
        }

    if (GET_FICTION(ch) && GET_FICTION(ch) != obj) {
        act("Сначала дочитайте $o3.", FALSE, ch, GET_FICTION(ch), 0, TO_CHAR);
        return;
    }

    if (!OBJVAL_FLAGGED(obj, EXIT_CLOSED))
        act("$o уже открыт$G.", FALSE, ch, obj, 0, TO_CHAR);
    else {
        if (found) {
            if (k->mess_to_char)
                act(k->mess_to_char, "Мп", ch, obj);
            if (k->mess_to_room)
                act(k->mess_to_room, "Кмп", ch, obj);
        } else {
            act("Вы открыли $o3.", FALSE, ch, obj, 0, TO_CHAR);
            act("$n открыл$g $o3.", FALSE, ch, obj, 0, TO_ROOM);
        }
        REMOVE_BIT(OBJVAL_FLAGS(obj, EXIT_CLOSED), EXIT_CLOSED);
        GET_FICTION(ch) = obj;
        obj->page = 1;
    }

}

void go_open_door(struct char_data *ch, byte door)
{
    int back_room = NOWHERE;
    int back_dir = -1;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_MOVED)) {
        send_to_charf(ch, "%s нельзя открыть.\r\n",
                      CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT)));
        return;
    }
    //Проверка наличия двери с обратной стороны
    if ((back_room = EXIT(ch, (int) door)->to_room) != NOWHERE) {
        back_dir = rev_dir[(int) door];
        if (!EXITDATA(back_room, back_dir) ||
            (EXITDATA(back_room, back_dir) &&
             (!DOOR_FLAGGED(EXITDATA(back_room, back_dir), EXIT_CLOSED) &&
              !DOOR_FLAGGED(EXITDATA(back_room, back_dir), EXIT_LOCKED))))
            back_room = NOWHERE;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CRASHED)) {
        send_to_charf(ch, "%s выбили.\r\n",
                      CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT)));
        return;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CLOSED)) {
        if (EXIT(ch, (int) door)->key && DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_LOCKED)) {
            send_to_charf(ch, "%s заперт%s.\r\n",
                          CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_IMN, PAD_OBJECT)),
                          DSHR(ch, (int) door));
            return;
        }
        send_to_charf(ch, "Вы открыли %s.\r\n",
                      get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
        sprintf(buf, "$n открыл$g %s.",
                get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        exit_trap_active(ch, door, FALSE);
        REMOVE_BIT(DOOR_FLAGS(EXIT(ch, (int) door), EXIT_CLOSED), EXIT_CLOSED);

        if (back_room != NOWHERE) {
            REMOVE_BIT(DOOR_FLAGS(EXITDATA(back_room, back_dir), EXIT_CLOSED), EXIT_CLOSED);
            if (!EXITDATA(back_room, back_dir)->exit_name) {
                sprintf(buf, "Кто-то открыл %s с другой стороны.",
                        get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
                sprintf(buf2,
                        "Билдер забыл указать название выхода (exit_name) в локации %d направление '%s'",
                        world[back_room].number, DirIs[back_dir]);
                mudlog(buf2, CMP, LVL_IMPL, TRUE);
            } else if (world[back_room].people) {
                sprintf(buf, "Кто-то открыл %s с другой стороны.",
                        get_name_pad(EXITDATA(back_room, back_dir)->exit_name, PAD_VIN,
                                     PAD_OBJECT));
                act(buf, FALSE, world[back_room].people, 0, 0, TO_CHAR);
                act(buf, FALSE, world[back_room].people, 0, 0, TO_ROOM);
            }
        } else
            send_to_charf(ch,
                          "Билдер забыл нарисовать дверь с обратной стороны. Сообщите имморталам.\r\n");

        return;
    } else {
        send_to_charf(ch, "%s уже открыт%s.\r\n",
                      CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_IMN, PAD_OBJECT)),
                      DSHR(ch, (int) door));
        return;
    }
}


ACMD(do_open)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;

    skip_spaces(&argument);

    if (!*argument) {
        send_to_charf(ch, "Открыть что?\r\n");
        return;
    }

    two_arguments(argument, type, dir);

    if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
        door = _find_door(ch, type, dir);

    if (door == -2) {
        send_to_charf(ch, "Что Вы хотите открыть?\r\n");
        return;
    }

    if (!obj && door == -2)
        return;

//обрабатываем двери
    bool fnd = FALSE;

    if (!obj && door >= 0) {
        go_open_door(ch, door);
        fnd = TRUE;
    } else if (obj) {
        if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
            go_open_container(ch, obj);
            fnd = TRUE;
        } else if (GET_OBJ_TYPE(obj) == ITEM_FICTION) {
            go_open_fiction(ch, obj);
            fnd = TRUE;
        }
    }

    if (!fnd)
        send_to_charf(ch, "Что Вы хотите открыть?\r\n");
    else
        GET_MISSED(ch)++;
}



void go_moved_door(struct char_data *ch, byte door)
{
    int back_room = NOWHERE;
    int back_dir = -1;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CLOSE)) {
        send_to_charf(ch, "%s нельзя подвинуть.\r\n",
                      CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT)));
        return;
    }
    //Проверка наличия двери с обратной стороны
    if ((back_room = EXIT(ch, (int) door)->to_room) != NOWHERE) {
        back_dir = rev_dir[(int) door];
        if (!EXITDATA(back_room, back_dir) ||
            (EXITDATA(back_room, back_dir) &&
             (!DOOR_FLAGGED(EXITDATA(back_room, back_dir), EXIT_CLOSED) &&
              !DOOR_FLAGGED(EXITDATA(back_room, back_dir), EXIT_LOCKED))))
            back_room = NOWHERE;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CRASHED)) {
        send_to_charf(ch, "%s выбили.\r\n",
                      CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT)));
        return;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CLOSED)) {
        if (EXIT(ch, (int) door)->key && DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_LOCKED)) {
            send_to_charf(ch, "%s заперт%s.\r\n",
                          CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_IMN, PAD_OBJECT)),
                          DSHR(ch, (int) door));
            return;
        }
        send_to_charf(ch, "Вы подвинули %s.\r\n",
                      get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
        sprintf(buf, "$n подвинул$g %s.",
                get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        exit_trap_active(ch, door, FALSE);
        REMOVE_BIT(DOOR_FLAGS(EXIT(ch, (int) door), EXIT_CLOSED), EXIT_CLOSED);

        if (back_room != NOWHERE) {
            REMOVE_BIT(DOOR_FLAGS(EXITDATA(back_room, back_dir), EXIT_CLOSED), EXIT_CLOSED);
            if (!EXITDATA(back_room, back_dir)->exit_name) {
                sprintf(buf, "Кто-то подвинул %s с другой стороны.",
                        get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
                sprintf(buf2,
                        "Билдер забыл указать название выхода (exit_name) в локации %d направление '%s'",
                        world[back_room].number, DirIs[back_dir]);
                mudlog(buf2, CMP, LVL_IMPL, TRUE);
            } else {
                sprintf(buf, "Кто-то подвинул %s с другой стороны.",
                        get_name_pad(EXITDATA(back_room, back_dir)->exit_name, PAD_VIN,
                                     PAD_OBJECT));
                act(buf, FALSE, world[back_room].people, 0, 0, TO_CHAR);
                act(buf, FALSE, world[back_room].people, 0, 0, TO_ROOM);
            }
        } else
            send_to_charf(ch,
                          "Билдер забыл нарисовать дверь с обратной стороны. Сообщите имморталам.\r\n");

        return;
    } else {                    //Если плита отодвинута закрываем ее
        send_to_charf(ch, "Вы задвинули %s.\r\n",
                      get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
        sprintf(buf, "$n задвинул$g %s.",
                get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
        act(buf, FALSE, ch, 0, 0, TO_ROOM);

        SET_BIT(DOOR_FLAGS(EXIT(ch, (int) door), EXIT_CLOSED), EXIT_CLOSED);

        if (back_room != NOWHERE) {
            SET_BIT(DOOR_FLAGS(EXITDATA(back_room, back_dir), EXIT_CLOSED), EXIT_CLOSED);
            if (!EXITDATA(back_room, back_dir)->exit_name) {
                sprintf(buf, "$N задвинул$G %s с другой стороны.",
                        get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
                sprintf(buf2,
                        "Билдер забыл указать название выхода (exit_name) в локации %d направление '%s'",
                        world[back_room].number, DirIs[back_dir]);
                mudlog(buf2, CMP, LVL_IMPL, TRUE);
            } else {
                sprintf(buf, "$N задвинул$G %s с другой стороны.",
                        get_name_pad(EXITDATA(back_room, back_dir)->exit_name, PAD_VIN,
                                     PAD_OBJECT));
                act(buf, FALSE, world[back_room].people, 0, ch, TO_CHAR);
                act(buf, FALSE, world[back_room].people, 0, ch, TO_ROOM);
            }
        } else
            send_to_charf(ch,
                          "Билдер забыл нарисовать дверь с обратной стороны. Сообщите имморталам.\r\n");
    }
}

ACMD(do_moved)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];

    skip_spaces(&argument);

    if (!*argument) {
        send_to_charf(ch, "Подвинуть что?\r\n");
        return;
    }

    two_arguments(argument, type, dir);

    door = _find_door(ch, type, dir);

    if (door == -2) {
        send_to_charf(ch, "Что Вы хотите подвинуть?\r\n");
        return;
    }
//обрабатываем двери
    bool fnd = FALSE;

    if (door >= 0) {
        go_moved_door(ch, door);
        fnd = TRUE;
    }

    if (!fnd)
        send_to_charf(ch, "Что Вы хотите подвинуть?\r\n");
    else
        GET_MISSED(ch)++;
}

/************************ CLOSE *************************************/

void go_close_container(struct char_data *ch, struct obj_data *obj)
{

    bool found = FALSE;
    int count;
    struct mess_p_data *k;

    /* Проверка на перехват сообщений */
    for (k = obj->mess_data; k; k = k->next, count++)
        if (k->command == CMD_CLOSE) {
            found = TRUE;
            break;
        }

    if (found && k->stoping) {
        if (k->mess_to_char)
            act(k->mess_to_char, "Мп", ch, obj);
        if (k->mess_to_room && !obj->worn_by && !obj->carried_by)
            act(k->mess_to_room, "Кмп", ch, obj);
        return;
    }

    if (OBJVAL_FLAGGED(obj, EXIT_CRASHED)) {
        act("$o выломан$G.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (!OBJVAL_FLAGGED(obj, EXIT_CLOSED)) {
        if (found) {
            if (k->mess_to_char)
                act(k->mess_to_char, "Мп", ch, obj);
            if (k->mess_to_room && !obj->worn_by && !obj->carried_by)
                act(k->mess_to_room, "Кмп", ch, obj);
        } else {
            act("Вы закрыли $o3.", FALSE, ch, obj, 0, TO_CHAR);
            if (!obj->worn_by && !obj->carried_by)
                act("$n закрыл$g $o3.", FALSE, ch, obj, 0, TO_ROOM);
        }
        SET_BIT(OBJVAL_FLAGS(obj, EXIT_CLOSED), EXIT_CLOSED);
    } else
        act("$o уже закрыт$G.", FALSE, ch, obj, 0, TO_CHAR);

}

void go_close_fiction(struct char_data *ch, struct obj_data *obj)
{
    bool found = FALSE;
    int count;
    struct mess_p_data *k;

    /* Проверка на перехват сообщений */
    for (k = obj->mess_data; k; k = k->next, count++)
        if (k->command == CMD_CLOSE) {
            found = TRUE;
            break;
        }

    if (found && k->stoping) {
        if (k->mess_to_char)
            act(k->mess_to_char, "Мп", ch, obj);
        if (k->mess_to_room)
            act(k->mess_to_room, "Кмп", ch, obj);
        return;
    }
    if (CAN_WEAR(obj, ITEM_WEAR_TAKE))
        if (!obj->worn_by && !obj->carried_by) {
            act("Чтобы закрыть $o3 необходимо взять $S в руки.", FALSE, ch, obj, 0, TO_CHAR);
            return;
        }

    if (GET_FICTION(ch) && GET_FICTION(ch) != obj) {
        act("Сначала дочитайте $o3.", FALSE, ch, GET_FICTION(ch), 0, TO_CHAR);
        return;
    }

    if (OBJVAL_FLAGGED(obj, EXIT_CLOSED))
        act("$o уже закрыт$G.", FALSE, ch, obj, 0, TO_CHAR);
    else {
        if (found) {
            if (k->mess_to_char)
                act(k->mess_to_char, "Мп", ch, obj);
            if (k->mess_to_room)
                act(k->mess_to_room, "Кмп", ch, obj);
        } else {
            act("Вы закрыли $o3.", FALSE, ch, obj, 0, TO_CHAR);
            act("$n закрыл$g $o3.", FALSE, ch, obj, 0, TO_ROOM);
        }
        SET_BIT(OBJVAL_FLAGS(obj, EXIT_CLOSED), EXIT_CLOSED);
        GET_FICTION(ch) = NULL;
        obj->page = 0;
    }

}

void go_close_door(struct char_data *ch, byte door)
{
    int back_room = NOWHERE;
    int back_dir = -1;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    //Проверка наличия двери с обратной стороны
    if ((back_room = EXIT(ch, (int) door)->to_room) != NOWHERE) {
        back_dir = rev_dir[(int) door];
        if (!EXITDATA(back_room, back_dir) ||
            (EXITDATA(back_room, back_dir) &&
             DOOR_FLAGGED(EXITDATA(back_room, back_dir), EXIT_CLOSED)))
            back_room = NOWHERE;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CRASHED)) {
        send_to_charf(ch, "%s выбили.\r\n",
                      CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT)));
        return;
    }

    if (!DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CLOSED)) {
        send_to_charf(ch, "Вы закрыли %s.\r\n",
                      get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
        sprintf(buf, "$n закрыл$g %s.",
                get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
        act(buf, FALSE, ch, 0, 0, TO_ROOM);

        SET_BIT(DOOR_FLAGS(EXIT(ch, (int) door), EXIT_CLOSED), EXIT_CLOSED);

        if (back_room != NOWHERE) {
            SET_BIT(DOOR_FLAGS(EXITDATA(back_room, back_dir), EXIT_CLOSED), EXIT_CLOSED);
            if (!EXITDATA(back_room, back_dir)->exit_name) {
                sprintf(buf, "$N закрыл$G %s с другой стороны.",
                        get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
                sprintf(buf2,
                        "Билдер забыл указать название выхода (exit_name) в локации %d направление '%s'",
                        world[back_room].number, DirIs[back_dir]);
                mudlog(buf2, CMP, LVL_IMPL, TRUE);
            } else if (world[back_room].people) {
                sprintf(buf, "$N закрыл$G %s с другой стороны.",
                        get_name_pad(EXITDATA(back_room, back_dir)->exit_name, PAD_VIN,
                                     PAD_OBJECT));
                act(buf, FALSE, world[back_room].people, 0, ch, TO_CHAR);
                act(buf, FALSE, world[back_room].people, 0, ch, TO_ROOM);
            }
        } else
            send_to_charf(ch,
                          "Билдер забыл нарисовать дверь с обратной стороны. Сообщите имморталам.\r\n");

        return;
    } else {
        send_to_charf(ch, "%s уже закрыт%s.\r\n",
                      CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_IMN, PAD_OBJECT)),
                      DSHR(ch, (int) door));
        return;
    }
}

ACMD(do_close)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;

    skip_spaces(&argument);

    if (!*argument) {
        send_to_charf(ch, "Закрыть что?\r\n");
        return;
    }

    two_arguments(argument, type, dir);

    if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
        door = _find_door(ch, type, dir);

    if (door == -2) {
        send_to_charf(ch, "Что Вы хотите закрыть?\r\n");
        return;
    }

    if (!obj && door == -2)
        return;

//обрабатываем двери
    if (!obj && door > -1) {
        go_close_door(ch, door);
        GET_MISSED(ch)++;
    } else if (obj) {
        if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER)
            go_close_container(ch, obj);
        else if (GET_OBJ_TYPE(obj) == ITEM_FICTION)
            go_close_fiction(ch, obj);

        GET_MISSED(ch)++;
    } else
        send_to_charf(ch, "Что Вы хотите закрыть?\r\n");

}

void check_key(struct char_data *ch, obj_vnum key)
{
    struct obj_data *o, *on;

    if (IS_NPC(ch))
        return;

    for (o = ch->carrying; o; o = on) {
        on = o->next_content;

        if (GET_OBJ_VNUM(o) == key && GET_OBJ_TYPE(o) == ITEM_KEY) {
            GET_OBJ_VAL(o, 1)--;
            if (GET_OBJ_VAL(o, 1) <= 0) {
                act("$o рассыпал$U в Ваших руках.", FALSE, ch, o, 0, TO_CHAR);
                extract_obj(o);
            }
            return;
        }
    }

    if (GET_EQ(ch, WEAR_HOLD))
        if (GET_OBJ_VNUM(GET_EQ(ch, WEAR_HOLD)) == key &&
            GET_OBJ_TYPE(GET_EQ(ch, WEAR_HOLD)) == ITEM_KEY) {
            GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 1)--;
            if (GET_OBJ_VAL(GET_EQ(ch, WEAR_HOLD), 1) <= 0) {
                act("$o рассыпал$U в Ваших руках.", FALSE, ch, GET_EQ(ch, WEAR_HOLD), 0, TO_CHAR);
                o = unequip_char(ch, WEAR_HOLD);
                extract_obj(o);
            }
            return;
        }



}


/************************ UNLOCK *************************************/

void go_unlock_container(struct char_data *ch, struct obj_data *obj)
{
    bool found = FALSE;
    int count;
    struct mess_p_data *k;

    /* Проверка на перехват сообщений */
    for (k = obj->mess_data; k; k = k->next, count++)
        if (k->command == CMD_UNLOCK) {
            found = TRUE;
            break;
        }

    if (found && k->stoping) {
        if (k->mess_to_char)
            act(k->mess_to_char, "Мп", ch, obj);
        if (k->mess_to_room && !obj->worn_by && !obj->carried_by)
            act(k->mess_to_room, "Кмп", ch, obj);
        return;
    }

    if (OBJVAL_FLAGGED(obj, EXIT_CRASHED)) {
        act("$o выломан$G.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (GET_OBJ_VAL(obj, 2) && OBJVAL_FLAGGED(obj, EXIT_LOCKED)) {
        if (!has_key(ch, GET_OBJ_VAL(obj, 2))) {
            act("У Вас нет нужного ключа для $o1.", FALSE, ch, obj, 0, TO_CHAR);
            return;
        } else {

            if (found) {
                if (k->mess_to_char)
                    act(k->mess_to_char, "Мп", ch, obj);
                if (k->mess_to_room && !obj->worn_by && !obj->carried_by)
                    act(k->mess_to_room, "Кмп", ch, obj);
            } else {
                act("Вы отперли $o3.", FALSE, ch, obj, 0, TO_CHAR);
                if (!obj->worn_by && !obj->carried_by)
                    act("$n отпер$q $o3.", FALSE, ch, obj, 0, TO_ROOM);
            }
            check_key(ch, GET_OBJ_VAL(obj, 2));
            REMOVE_BIT(OBJVAL_FLAGS(obj, EXIT_LOCKED), EXIT_LOCKED);
        }
    } else
        act("$o уже отперт$G.", FALSE, ch, obj, 0, TO_CHAR);

}


void go_unlock_door(struct char_data *ch, byte door)
{
    int back_room = NOWHERE;
    int back_dir = -1;
    char buf[MAX_STRING_LENGTH];

    //Проверка наличия двери с обратной стороны
    if ((back_room = EXIT(ch, (int) door)->to_room) != NOWHERE) {
        back_dir = rev_dir[(int) door];
        if (!EXITDATA(back_room, back_dir) ||
            (EXITDATA(back_room, back_dir) &&
             !DOOR_FLAGGED(EXITDATA(back_room, back_dir), EXIT_LOCKED)))
            back_room = NOWHERE;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CRASHED)) {
        send_to_charf(ch, "%s выбили.\r\n",
                      CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT)));
        return;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CLOSED)) {
        if (EXIT(ch, (int) door)->key && DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_LOCKED)) {
            if (!has_key(ch, EXIT(ch, (int) door)->key)) {
                send_to_charf(ch, "У Вас нет подходящего ключа для %s.\r\n",
                              get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_ROD, PAD_OBJECT));
            } else {
                send_to_charf(ch, "Вы отперли %s.\r\n",
                              get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
                sprintf(buf, "$n отпер$q %s.",
                        get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
                act(buf, FALSE, ch, 0, 0, TO_ROOM);
                check_key(ch, EXIT(ch, (int) door)->key);
                REMOVE_BIT(DOOR_FLAGS(EXIT(ch, (int) door), EXIT_LOCKED), EXIT_LOCKED);
                if (back_room != NOWHERE) {
                    if (world[back_room].people) {
                        sprintf(buf, "За %s раздался щелчок.",
                                get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_TVR, PAD_OBJECT));
                        act(buf, FALSE, world[back_room].people, 0, 0, TO_CHAR);
                        act(buf, FALSE, world[back_room].people, 0, 0, TO_ROOM);
                    }
                    REMOVE_BIT(DOOR_FLAGS(EXITDATA(back_room, back_dir), EXIT_LOCKED), EXIT_LOCKED);
                }
            }
        } else {
            send_to_charf(ch, "%s не заперт%s.\r\n",
                          CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_IMN, PAD_OBJECT)),
                          DSHR(ch, (int) door));
        }
    } else {
        send_to_charf(ch, "%s не заперт%s и даже открыт%s.\r\n",
                      CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_IMN, PAD_OBJECT)),
                      DSHR(ch, (int) door), DSHR(ch, (int) door));
    }

}

ACMD(do_unlock)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;

    skip_spaces(&argument);

    if (!*argument) {
        send_to_charf(ch, "Отпереть что?\r\n");
        return;
    }

    two_arguments(argument, type, dir);

    if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
        door = _find_door(ch, type, dir);

    if (door == -2) {
        send_to_charf(ch, "Что Вы хотите отпереть?\r\n");
        return;
    }

    if (!obj && door == -2)
        return;

//обрабатываем двери
    if (!obj && door > -1) {
        go_unlock_door(ch, door);
        GET_MISSED(ch)++;
    } else if (obj && (GET_OBJ_TYPE(obj) == ITEM_CONTAINER || GET_OBJ_TYPE(obj) == ITEM_FICTION)) {
        go_unlock_container(ch, obj);
        GET_MISSED(ch)++;
    } else
        send_to_charf(ch, "Что Вы хотите отпереть?\r\n");
}

/************************ LOCK *************************************/

void go_lock_container(struct char_data *ch, struct obj_data *obj)
{
    bool found = FALSE;
    int count;
    struct mess_p_data *k;

    /* Проверка на перехват сообщений */
    for (k = obj->mess_data; k; k = k->next, count++)
        if (k->command == CMD_LOCK) {
            found = TRUE;
            break;
        }

    if (found && k->stoping) {
        if (k->mess_to_char)
            act(k->mess_to_char, "Мп", ch, obj);
        if (k->mess_to_room && !obj->worn_by && !obj->carried_by)
            act(k->mess_to_room, "Кмп", ch, obj);
        return;
    }


    if (OBJVAL_FLAGGED(obj, EXIT_CRASHED)) {
        act("$o выломан$G.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (!OBJVAL_FLAGGED(obj, EXIT_CLOSED)) {
        act("Вы не можете запереть открытый $o.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }
    if (GET_OBJ_VAL(obj, 2) && !OBJVAL_FLAGGED(obj, EXIT_LOCKED)) {
        if (!has_key(ch, GET_OBJ_VAL(obj, 2))) {
            act("У Вас нет нужного ключа для $o1.", FALSE, ch, obj, 0, TO_CHAR);
            return;
        } else {
            if (found) {
                if (k->mess_to_char)
                    act(k->mess_to_char, "Мп", ch, obj);
                if (k->mess_to_room && !obj->worn_by && !obj->carried_by)
                    act(k->mess_to_room, "Кмп", ch, obj);
            } else {
                act("Вы заперли $o3.", FALSE, ch, obj, 0, TO_CHAR);
                if (!obj->worn_by && !obj->carried_by)
                    act("$n запер$q $o3.", FALSE, ch, obj, 0, TO_ROOM);
            }
            SET_BIT(OBJVAL_FLAGS(obj, EXIT_LOCKED), EXIT_LOCKED);
        }
    } else
        act("$o уже заперт$G.", FALSE, ch, obj, 0, TO_CHAR);

}

void go_lock_door(struct char_data *ch, byte door)
{
    int back_room = NOWHERE;
    int back_dir = -1;
    char buf[MAX_STRING_LENGTH];

    //Проверка наличия двери с обратной стороны
    if ((back_room = EXIT(ch, (int) door)->to_room) != NOWHERE) {
        back_dir = rev_dir[(int) door];
        if (!EXITDATA(back_room, back_dir) ||
            (EXITDATA(back_room, back_dir) &&
             (!DOOR_FLAGGED(EXITDATA(back_room, back_dir), EXIT_CLOSED) ||
              DOOR_FLAGGED(EXITDATA(back_room, back_dir), EXIT_LOCKED))))
            back_room = NOWHERE;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CRASHED)) {
        send_to_charf(ch, "%s выбили.\r\n",
                      CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT)));
        return;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CLOSED)) {
        if (EXIT(ch, (int) door)->key && DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_LOCKED)) {
            send_to_charf(ch, "%s уже заперт%s.\r\n",
                          CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_IMN, PAD_OBJECT)),
                          DSHR(ch, (int) door));
        } else {
            if (!has_key(ch, EXIT(ch, (int) door)->key)) {
                send_to_charf(ch, "У Вас нет подходящего ключа для %s.\r\n",
                              get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_ROD, PAD_OBJECT));
            } else {
                send_to_charf(ch, "Вы заперли %s.\r\n",
                              get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
                sprintf(buf, "$n запер$g %s.",
                        get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
                act(buf, FALSE, ch, 0, 0, TO_ROOM);
                SET_BIT(DOOR_FLAGS(EXIT(ch, (int) door), EXIT_LOCKED), EXIT_LOCKED);
                if (back_room != NOWHERE) {
                    if (world[back_room].people) {
                        sprintf(buf, "За %s раздался щелчок.",
                                get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_TVR, PAD_OBJECT));
                        act(buf, FALSE, world[back_room].people, 0, 0, TO_CHAR);
                        act(buf, FALSE, world[back_room].people, 0, 0, TO_ROOM);
                    }
                    SET_BIT(DOOR_FLAGS(EXITDATA(back_room, back_dir), EXIT_LOCKED), EXIT_LOCKED);
                }
            }
        }
    } else {
        send_to_charf(ch, "Запереть открыт%s %s невозможно.\r\n",
                      DSHM(ch, (int) door), get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN,
                                                         PAD_OBJECT));
    }

}

ACMD(do_lock)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;

    if (on_horse(ch)) {
        send_to_char("Для этого нужно спешиться.\r\n", ch);
        return;
    }


    skip_spaces(&argument);

    if (!*argument) {
        send_to_charf(ch, "Запереть что?\r\n");
        return;
    }

    two_arguments(argument, type, dir);

    if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
        door = _find_door(ch, type, dir);

    if (door == -2) {
        send_to_charf(ch, "Что Вы хотите запереть?\r\n");
        return;
    }

    if (!obj && door == -2)
        return;

//обрабатываем двери
    if (!obj && door > -1) {
        go_lock_door(ch, door);
        GET_MISSED(ch)++;
    } else if (obj) {
        go_lock_container(ch, obj);
        GET_MISSED(ch)++;
    } else
        send_to_charf(ch, "Что Вы хотите запереть?\r\n");
}

/************************ PICKOFF *************************************/

void go_pick_container(struct char_data *ch, struct obj_data *obj)
{
    if (OBJVAL_FLAGGED(obj, EXIT_CRASHED)) {
        act("$o выломан$G.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (OBJVAL_FLAGGED(obj, EXIT_CLOSED)) {
        if (OBJVAL_FLAGGED(obj, EXIT_LOCKED)) {
            struct obj_data *lp;

            if (!(lp = have_lockpick(ch))) {
                act("У Вас нет отмычки, чтобы взломать @1в.", "Мп", ch, obj);
                return;
            }
            if (ch->dir_pick != -1 || ch->obj_pick) {
                send_to_charf(ch, "Вы уже что-то взламываете.\r\n");
                return;
            }
            act("Склонившись над замком @2р, Вы приступили к взлому.", "Мпп", ch, lp, obj);
            act("Склонившись над замком @2р, 1+и приступил1(,а,о,и) к взлому.", "Кмпп", ch, lp,
                obj);
            ch->obj_pick = obj;
        } else {
            act("$o не заперт$G.", FALSE, ch, obj, 0, TO_CHAR);
        }
    } else {
        act("$o уже открыт$G.", FALSE, ch, obj, 0, TO_CHAR);
    }
}


void get_pick_door(struct char_data *ch, int door)
{
    int back_room, back_dir = -1, exp;
    char buf[MAX_STRING_LENGTH];

    if ((back_room = EXIT(ch, (int) door)->to_room) != NOWHERE) {
        back_dir = rev_dir[(int) door];
        if (!EXITDATA(back_room, back_dir) ||
            (EXITDATA(back_room, back_dir) &&
             !DOOR_FLAGGED(EXITDATA(back_room, back_dir), EXIT_CLOSED)))
            back_room = NOWHERE;
    }

    if (!DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_NOEXP)) {
        //exp = MIN(2500,(((get_levelexp(ch, GET_LEVEL(ch), 1) / 1600) * EXIT(ch, (int) door)->quality) / 100));
        //gain_exp(ch, exp, TRUE);
        improove_skill(ch, 0, EXIT(ch, door)->quality / 3, SKILL_PICK_LOCK);
    }
    REMOVE_BIT(DOOR_FLAGS(EXIT(ch, (int) door), EXIT_LOCKED), EXIT_LOCKED);
    SET_BIT(DOOR_FLAGS(EXIT(ch, (int) door), EXIT_NOEXP), EXIT_NOEXP);
    if (back_room != NOWHERE && back_dir != -1) {
        if (world[back_room].people) {
            sprintf(buf, "За %s раздался щелчок.",
                    get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_TVR, PAD_OBJECT));
            act(buf, FALSE, world[back_room].people, 0, 0, TO_CHAR);
            act(buf, FALSE, world[back_room].people, 0, 0, TO_ROOM);
        }
        exit_trap_active(ch, door, TRUE);
        REMOVE_BIT(DOOR_FLAGS(EXITDATA(back_room, back_dir), EXIT_LOCKED), EXIT_LOCKED);
        SET_BIT(DOOR_FLAGS(EXITDATA(back_room, back_dir), EXIT_NOEXP), EXIT_NOEXP);
    }

}

void get_pick_cont(struct char_data *ch, struct obj_data *obj)
{
    int exp;

    if (!OBJVAL_FLAGGED(obj, EXIT_NOEXP)) {
        //exp = MIN(2500,(((get_levelexp(ch, GET_LEVEL(ch), 1) / 1600) * GET_OBJ_QUALITY(obj) * 11) / 100));
        //gain_exp(ch, exp, TRUE);
        improove_skill(ch, 0, GET_OBJ_QUALITY(obj), SKILL_PICK_LOCK);
    }

    REMOVE_BIT(OBJVAL_FLAGS(obj, EXIT_LOCKED), EXIT_LOCKED);
    SET_BIT(OBJVAL_FLAGS(obj, EXIT_NOEXP), EXIT_NOEXP);

}

void go_pick_door(struct char_data *ch, byte door)
{
    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CRASHED)) {
        send_to_charf(ch, "%s выбили.\r\n",
                      CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT)));
        return;
    }

    if (!DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_LOCKED)) {
        send_to_charf(ch, "Взломать незаперт%s %s невозможно.\r\n",
                      DSHM(ch, (int) door), get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN,
                                                         PAD_OBJECT));
        return;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CLOSED)) {
        struct obj_data *lp;

        if (!(lp = have_lockpick(ch))) {
            send_to_charf(ch, "У Вас нет отмычки, чтобы взломать %s.\r\n",
                          get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
            return;
        }
        if (ch->dir_pick != -1 || ch->obj_pick) {
            send_to_charf(ch, "Вы уже что-то взламываете.\r\n");
            return;
        }
        act("Склонившись над замком %1, Вы приступили к взлому.", "Мпт", ch, lp,
            get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_ROD, PAD_OBJECT));
        act("Склонившись над замком %1, 1+и приступил1(,а,о,и) к взлому.", "Кмпт", ch, lp,
            get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_ROD, PAD_OBJECT));
        ch->dir_pick = door;
    } else
        send_to_charf(ch, "Взломать уже открыт%s %s невозможно.\r\n",
                      DSHM(ch, (int) door), get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN,
                                                         PAD_OBJECT));

}

ACMD(do_pickoff)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;

    if (!check_fight_command(ch))
        return;

    if (on_horse(ch)) {
        send_to_char("Для этого нужно спешиться.\r\n", ch);
        return;
    }

    skip_spaces(&argument);

    if (!*argument) {
        send_to_charf(ch, "Взломать что?\r\n");
        return;
    }

    two_arguments(argument, type, dir);

    if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
        door = _find_door(ch, type, dir);

    if (door == -2) {
        send_to_charf(ch, "Что Вы хотите взломать?\r\n");
        return;
    }

    if (!obj && door == -2)
        return;

    if (!GET_SKILL(ch, SKILL_PICK_LOCK)) {
        send_to_char("Вы не знаете как.\r\n", ch);
        return;
    }
//обрабатываем двери
    int fnd = FALSE;

    if (!obj && door > -1) {
        go_pick_door(ch, door);
        fnd = TRUE;
    } else if (obj && GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
        go_pick_container(ch, obj);
        fnd = TRUE;
    }
    if (!fnd)
        send_to_charf(ch, "Что Вы хотите взломать?\r\n");

}

/************************ CRASH *************************************/

void go_crash_container(struct char_data *ch, struct obj_data *obj)
{
    int percent = 100, prob = 0;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_CRASHDOOR) : GET_SKILL(ch, SKILL_CRASHDOOR);
    struct timed_type timed;

    if (GET_OBJ_TYPE(obj) != ITEM_CONTAINER) {
        send_to_charf(ch, "Не является контейнером.\r\n");
        return;
    }

    if (timed_by_skill(ch, SKILL_CRASHDOOR)) {
        send_to_char("У Вас не хватит сил.\r\n", ch);
        return;
    }

    if (OBJVAL_FLAGGED(obj, EXIT_CRASHED)) {
        act("$o уже выломан$G.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (OBJVAL_FLAGGED(obj, EXIT_CLOSED)) {
        if (OBJVAL_FLAGGED(obj, EXIT_LOCKED)) {
            if (OBJVAL_FLAGGED(obj, EXIT_NOCRASH))
                percent = -1;
            else
                percent = number(1, 100);

            if (!check_moves(ch, CRASH_MOVES))
                return;

            prob = (GET_REAL_STR(ch) * 3) + GET_REAL_WEIGHT(ch) + skill + RNDSKILL;
            percent = (GET_OBJ_QUALITY(obj) * 30) + RNDSKILL;
            improove_skill(ch, 0, GET_OBJ_QUALITY(obj) * 3, SKILL_CRASHDOOR);

            if (PRF_FLAGGED(ch, PRF_CODERINFO))
                send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);

            timed.skill = SKILL_CRASHDOOR;
            timed.time = 2 * SECS_PER_MUD_TICK;
            timed_to_char(ch, &timed);

            if (prob >= percent && percent != -1) {
                act("Вы выломали замок $o1.", FALSE, ch, obj, 0, TO_CHAR);
                //int exp = MIN(2500,(((get_levelexp(ch, GET_LEVEL(ch), 1) / 1600) * GET_OBJ_QUALITY(obj)) / 10));
                //gain_exp(ch, exp, TRUE);

                if (!obj->worn_by && !obj->carried_by)
                    act("$n выломал$g замок $o1.", FALSE, ch, obj, 0, TO_ROOM);
                REMOVE_BIT(OBJVAL_FLAGS(obj, EXIT_LOCKED), EXIT_LOCKED);
                REMOVE_BIT(OBJVAL_FLAGS(obj, EXIT_CLOSED), EXIT_CLOSED);
                SET_BIT(OBJVAL_FLAGS(obj, EXIT_CRASHED), EXIT_CRASHED);
            } else {
                act("Вам не удалось выломать замок $o1.", FALSE, ch, obj, 0, TO_CHAR);
                if (!obj->worn_by && !obj->carried_by)
                    act("$n не смог$q выломать замок $o1.", FALSE, ch, obj, 0, TO_ROOM);
            }
        } else {
            act("$o не заперт$G.", FALSE, ch, obj, 0, TO_CHAR);
        }
    } else {
        act("$o уже открыт$G.", FALSE, ch, obj, 0, TO_CHAR);
    }
}

void go_crash_door(struct char_data *ch, byte door)
{
    int back_room = NOWHERE;
    int back_dir = -1;
    int percent = 100, prob = 0;
    int skill = IS_MOB(ch) ? GET_SKILL_MOB(ch, SKILL_CRASHDOOR) : GET_SKILL(ch, SKILL_CRASHDOOR);
    char buf[MAX_STRING_LENGTH];
    struct timed_type timed;

    if (timed_by_skill(ch, SKILL_CRASHDOOR)) {
        send_to_char("У Вас не хватит сил.\r\n", ch);
        return;
    }
    //Проверка наличия двери с обратной стороны
    if ((back_room = EXIT(ch, (int) door)->to_room) != NOWHERE) {
        back_dir = rev_dir[(int) door];
        if (!EXITDATA(back_room, back_dir) ||
            (EXITDATA(back_room, back_dir) &&
             !DOOR_FLAGGED(EXITDATA(back_room, back_dir), EXIT_CLOSED)))
            back_room = NOWHERE;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CRASHED)) {
        send_to_charf(ch, "Уже все выбили.\r\n");
        return;
    }

    if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_CLOSED)) {
        if (DOOR_FLAGGED(EXIT(ch, (int) door), EXIT_NOCRASH))
            percent = -1;
        else
            percent = number(1, 100);

        if (!check_moves(ch, CRASH_MOVES))
            return;

        prob = (GET_REAL_STR(ch) * 3) + GET_REAL_WEIGHT(ch) + skill + RNDSKILL;
        percent = (EXIT(ch, (int) door)->quality * 2) + RNDSKILL;

        improove_skill(ch, 0, EXIT(ch, (int) door)->quality / 3, SKILL_CRASHDOOR);

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);

        timed.skill = SKILL_CRASHDOOR;
        timed.time = 2 * SECS_PER_MUD_TICK;
        timed_to_char(ch, &timed);

        if (prob >= percent && percent != -1) {
            send_to_charf(ch, "Вы выбили %s.\r\n",
                          get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
            //int exp = MIN(2500,(((get_levelexp(ch, GET_LEVEL(ch), 1) / 1600) * EXIT(ch,(int) door)->quality) / 100));
            //gain_exp(ch, exp, TRUE);

            sprintf(buf, "$n выбил$g %s.",
                    get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
            act(buf, FALSE, ch, 0, 0, TO_ROOM);
            REMOVE_BIT(DOOR_FLAGS(EXIT(ch, (int) door), EXIT_LOCKED), EXIT_LOCKED);
            REMOVE_BIT(DOOR_FLAGS(EXIT(ch, (int) door), EXIT_CLOSED), EXIT_CLOSED);
            SET_BIT(DOOR_FLAGS(EXIT(ch, (int) door), EXIT_CRASHED), EXIT_CRASHED);
            exit_trap_active(ch, door, TRUE);
            WAIT_STATE(ch, PULSE_VIOLENCE);

            if (back_room != NOWHERE) {
                sprintf(buf, "%s вылетел%s от удара с другой стороны.",
                        CAP(get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_IMN, PAD_OBJECT)),
                        DSHR(ch, (int) door));
                act(buf, FALSE, world[back_room].people, 0, 0, TO_CHAR);
                act(buf, FALSE, world[back_room].people, 0, 0, TO_ROOM);
                REMOVE_BIT(DOOR_FLAGS(EXITDATA(back_room, back_dir), EXIT_LOCKED), EXIT_LOCKED);
                REMOVE_BIT(DOOR_FLAGS(EXITDATA(back_room, back_dir), EXIT_CLOSED), EXIT_CLOSED);
                SET_BIT(DOOR_FLAGS(EXITDATA(back_room, back_dir), EXIT_CRASHED), EXIT_CRASHED);
            }
        } else {
            send_to_charf(ch, "Вы не смогли выбить %s и упали.\r\n",
                          get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
            sprintf(buf, "$n не смог$q выбить %s и упал$g.",
                    get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN, PAD_OBJECT));
            act(buf, FALSE, ch, 0, 0, TO_ROOM);
            GET_POS(ch) = POS_SITTING;
            WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
        }
    } else
        send_to_charf(ch, "Взломать уже открыт%s %s невозможно.\r\n",
                      DSHM(ch, (int) door), get_name_pad(EXIT(ch, (int) door)->exit_name, PAD_VIN,
                                                         PAD_OBJECT));

}

ACMD(do_crashdoor)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;

    if (!check_fight_command(ch))
        return;

    if (on_horse(ch)) {
        send_to_char("Для этого нужно спешиться.\r\n", ch);
        return;
    }

    skip_spaces(&argument);

    if (!*argument) {
        send_to_charf(ch, "Выломать что?\r\n");
        return;
    }

    two_arguments(argument, type, dir);

    if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
        door = _find_door(ch, type, dir);

    if (door == -2) {
        send_to_charf(ch, "Что Вы хотите выломать?\r\n");
        return;
    }

    if (!obj && door == -2)
        return;

    if (!GET_SKILL(ch, SKILL_CRASHDOOR)) {
        send_to_char("Вы не знаете как.\r\n", ch);
        return;
    }

    if (SECT(IN_ROOM(ch)) == SECT_UNDERWATER) {
        send_to_charf(ch, "Под водой ничего выломать не получиться.\r\n");
        return;
    }
//обрабатываем двери
    if (!obj && door > -1)
        go_crash_door(ch, door);
    else if (obj)
        go_crash_container(ch, obj);
    else
        send_to_charf(ch, "Что Вы хотите выломать?\r\n");
}

ACMD(do_page)
{
    struct obj_data *obj;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (!check_fight_command(ch))
        return;

    if (!GET_FICTION(ch)) {
        send_to_charf(ch, "А какую литературу в данный момент Вы читаете?\r\n");
        return;
    }

    obj = GET_FICTION(ch);

    if (OBJVAL_FLAGGED(obj, EXIT_CLOSED)) {
        act("$o закрыт$G.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    one_argument(argument, arg);

    if (!is_positive_number(arg)) {
        send_to_charf(ch, "Странный номер страницы, не правда ли?");
        return;
    }

    if (!find_exdesc(arg, obj->ex_description)) {
        act("Перелистав $o3, Вы не нашли нужной страницы.", FALSE, ch, obj, 0, TO_CHAR);
        act("Перелистав $o3, $n отложил $S в сторону.", FALSE, ch, obj, 0, TO_ROOM);
        go_close_fiction(ch, obj);
        return;
    } else {
        sprintf(buf, "Вы открыли $o3 на странице %s.", arg);
        act(buf, FALSE, ch, obj, 0, TO_CHAR);
        act("$n перелистнул$g страницу $o1.", FALSE, ch, obj, 0, TO_ROOM);

        obj->page = atoi(arg);
    }
}

ACMD(do_overpage)
{
    struct obj_data *obj;
    char buf[MAX_STRING_LENGTH];

    if (!check_fight_command(ch))
        return;

    if (!GET_FICTION(ch)) {
        send_to_charf(ch, "А какую литературу в данный момент Вы читаете?\r\n");
        return;
    }

    obj = GET_FICTION(ch);

    if (OBJVAL_FLAGGED(obj, EXIT_CLOSED)) {
        act("$o закрыт$G.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    obj->page++;
    sprintf(buf, "%d", obj->page);

    if (!find_exdesc(buf, obj->ex_description)) {
        act("Перелистав $o3, Вы не нашли нужной страницы.", FALSE, ch, obj, 0, TO_CHAR);
        act("Перелистав $o3, $n отложил $S в сторону.", FALSE, ch, obj, 0, TO_ROOM);
        go_close_fiction(ch, obj);
    } else {
        sprintf(buf, "Вы перелистнули страницу $o1.");
        act(buf, FALSE, ch, obj, 0, TO_CHAR);
        act("$n перелистнул$g страницу $o1.", FALSE, ch, obj, 0, TO_ROOM);
        read_book(obj, ch);
    }
}

ACMD(do_light_on)
{
    struct obj_data *obj;
    struct char_data *victim;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);

    if (!*argument) {
        send_to_charf(ch, "Зажечь что?\r\n");
        return;
    }

    one_argument(argument, arg);

    if (!generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &victim, &obj)) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if (GET_OBJ_TYPE(obj) != ITEM_LIGHT) {
        act("У Вас не получится зажечь $o3.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (!GET_LIGHT_VAL(obj)) {
        act("$o не может больше давать свет.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (GET_LIGHT_VAL(obj) == -1 || GET_LIGHT_ON(obj)) {
        act("$o уже светится.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    GET_LIGHT_ON(obj) = TRUE;

    act("Вы зажгли $o3.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n зажег $o3.", FALSE, ch, obj, 0, TO_ROOM);
    check_light(ch, LIGHT_NO, LIGHT_NO, LIGHT_NO, LIGHT_NO, 1);
    GET_MISSED(ch)++;
}

ACMD(do_light_off)
{
    struct obj_data *obj;
    struct char_data *victim;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);

    if (!*argument) {
        send_to_charf(ch, "Погасить что?\r\n");
        return;
    }

    one_argument(argument, arg);

    if (!generic_find(arg, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &victim, &obj)) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    if (GET_OBJ_TYPE(obj) != ITEM_LIGHT
        || (GET_OBJ_TYPE(obj) == ITEM_LIGHT && GET_LIGHT_VAL(obj) == -1)) {
        act("У Вас не получится погасить $o3.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    if (!GET_LIGHT_ON(obj)) {
        act("$o не светится.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    GET_LIGHT_ON(obj) = FALSE;

    act("Вы погасили $o3.", FALSE, ch, obj, 0, TO_CHAR);
    act("$n погасил$g $o3.", FALSE, ch, obj, 0, TO_ROOM);
    check_light(ch, LIGHT_YES, LIGHT_NO, LIGHT_NO, LIGHT_NO, 1);
    GET_MISSED(ch)++;
}

void go_saper_door(struct char_data *ch, int door)
{
    struct room_direction_data *exit = EXIT(ch, (int) door);
    int prob, percent;
    struct timed_type timed;

    if (!exit) {
        send_to_charf(ch, "Прежде чем обезвредить ловушку надо ее найти.\r\n");
        return;
    }

    if (check_tfind_char(ch, exit) && !exit->tbreak && exit->shance) {
        percent = number(1, 100);
        std::vector < int >vit;
        std::vector < int >vot;

        vit.push_back(GET_REAL_DEX(ch));
        vit.push_back(GET_REAL_INT(ch));
        vot.push_back(exit->quality / 2);
        prob = calc_like_skill(ch, 0, SKILL_SAPPER, vit, vot);
        improove_skill(ch, 0, exit->quality / 3, SKILL_SAPPER);

        if (percent <= 5)
            prob = 100;
        else if (percent >= 95)
            prob = 0;

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);
        if (prob >= percent) {
            send_to_charf(ch, "Вы обезвредили установленую на %s ловушку.\r\n",
                          get_name_pad(exit->exit_name, PAD_VIN, PAD_OBJECT));
            int exp = (((get_levelexp(ch, GET_LEVEL(ch), 1) / 200) * exit->quality) / 100);

            gain_exp(ch, exp, TRUE);

            exit->tbreak = TRUE;
        } else if (GET_REAL_LCK(ch) >= dice(1, 20)) {
            send_to_charf(ch, "Вам не удалось обезвредить ловушку.\r\n");
            act("$n попытал$u обезвредить ловушку, но не добил$u успеха.", FALSE, ch, 0, 0,
                TO_ROOM);
        } else {
            send_to_charf(ch, "Попытавшись обезвредить ловушку, Вы активировали ее.\r\n");
            act("Попытавшись обезвредить ловушку, $n привел$g ее в действие.", FALSE, ch, 0, 0,
                TO_ROOM);
            exit_trap_active(ch, door, TRUE);
        }

        timed.skill = SKILL_SAPPER;
        timed.time = 5 * SECS_PER_MUD_TICK;
        timed_to_char(ch, &timed);
    } else
        send_to_charf(ch, "Прежде чем обезвредить ловушку надо ее найти.\r\n");
}

void go_saper_container(struct char_data *ch, struct obj_data *obj)
{
    struct obj_trap_data *trap = obj->trap;
    int prob, percent;
    struct timed_type timed;

    if (!trap) {
        send_to_charf(ch, "Прежде чем обезвредить ловушку надо ее найти.\r\n");
        return;
    }

    if (check_tfind_char(ch, obj) && !trap->tbreak && trap->shance) {
        percent = number(1, 100);
        std::vector < int >vit;
        std::vector < int >vot;

        vit.push_back(GET_REAL_DEX(ch));
        vit.push_back(GET_REAL_INT(ch));
        vot.push_back(GET_OBJ_QUALITY(obj) * 6);
        prob = calc_like_skill(ch, 0, SKILL_SAPPER, vit, vot);
        improove_skill(ch, 0, GET_OBJ_QUALITY(obj) * 3, SKILL_SAPPER);

        if (percent <= 5)
            prob = 100;
        else if (percent >= 95)
            prob = 0;

        if (PRF_FLAGGED(ch, PRF_CODERINFO))
            send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);

        if (prob >= percent) {
            act("Вы обезвредили установленую на $o ловушку.", FALSE, ch, obj, 0, TO_CHAR);
            int exp = (((get_levelexp(ch, GET_LEVEL(ch), 1) / 200) * GET_OBJ_QUALITY(obj)) / 10);

            gain_exp(ch, exp, TRUE);

            trap->tbreak = TRUE;
        } else if (GET_REAL_LCK(ch) >= dice(1, 20)) {
            send_to_charf(ch, "Вам не удалось обезвредить ловушку.\r\n");
            act("$n попытал$u обезвредить ловушку, но не добил$u успеха.", FALSE, ch, 0, 0,
                TO_ROOM);
        } else {
            send_to_charf(ch, "Попытавшись обезвредить ловушку, Вы активировали ее.\r\n");
            act("Попытавшись обезвредить ловушку, $n привел$g ее в действие.", FALSE, ch, 0, 0,
                TO_ROOM);
            container_trap_active(ch, obj, TRUE);
        }

        timed.skill = SKILL_SAPPER;
        timed.time = 5 * SECS_PER_MUD_TICK;
        timed_to_char(ch, &timed);
    } else
        send_to_charf(ch, "Прежде чем обезвредить ловушку надо ее найти.\r\n");

}

ACMD(do_saper)
{
    int door = -1;
    char type[MAX_INPUT_LENGTH], dir[MAX_INPUT_LENGTH];
    struct obj_data *obj = NULL;
    struct char_data *victim = NULL;

    skip_spaces(&argument);

    if (!check_fight_command(ch))
        return;

    if (!*argument) {
        send_to_charf(ch, "Обезвредить что?\r\n");
        return;
    }

    two_arguments(argument, type, dir);

    if (!generic_find(type, FIND_OBJ_INV | FIND_OBJ_ROOM, ch, &victim, &obj))
        door = _find_door(ch, type, dir);

    if (door == -2) {
        send_to_charf(ch, "Что Вы хотите обезвредить?\r\n");
        return;
    }

    if (!obj && door == -2)
        return;

    if (!GET_SKILL(ch, SKILL_SAPPER)) {
        send_to_charf(ch, "Вы не умеете обезвреживать ловушки.\r\n");
        return;
    }

    if (!IS_HIGOD(ch) && timed_by_skill(ch, SKILL_SAPPER)) {
        send_to_charf(ch,
                      "Вы не восстановили свои силы для следующей операции обезвреживания.\r\n");
        return;
    }
//обрабатываем двери
    if (!obj && door >= 0)
        go_saper_door(ch, door);
    else if (obj)
        go_saper_container(ch, obj);

}

ACMD(do_show)
{
    struct obj_data *obj = NULL;
    struct char_data *vict = NULL;
    char arg[MAX_STRING_LENGTH];

    argument = one_argument(argument, arg);

    if (!*arg) {
        send_to_char("Показать что?\r\n", ch);
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
        send_to_charf(ch, "У Вас нет '%s'.\r\n", arg);
        return;
    }

    skip_spaces(&argument);

    if (!*argument) {
        act("Вы показали окружающим $o3.", FALSE, ch, obj, 0, TO_CHAR);
        act("$n показал$g откружающим $o3.", FALSE, ch, obj, 0, TO_ROOM);
        return;
    }

    if (!(vict = give_find_vict(ch, argument)))
        return;

    act("Вы показали $o3 $N2.", FALSE, ch, obj, vict, TO_CHAR);
    act("$n показал$g Вам $o3.", FALSE, ch, obj, vict, TO_VICT);
    act("$n показал$g $o3 $N2.", FALSE, ch, obj, vict, TO_NOTVICT);

}

AEVENT(event_break)
{
    int val = 0;
    struct obj_data *object = params->object;
    struct char_data *ch = params->actor;

    if (params == NULL)
        return;

    if (!OBJ_FLAGGED(object, ITEM_NODECAY))
        val = (GET_REAL_CON(ch) + GET_REAL_STR(ch));

    GET_OBJ_CUR(object) -= val;

    if (GET_OBJ_CUR(object) <= 0) {
        act("Вы сломали @1в.", "Мп", ch, object);
        act("1+и сломал1(,а,о,и) @1в.", "Кмп", ch, object);

        create_fragments(object);
        extract_obj(object, FALSE);
        return;
    } else {
        act("Вам не удалось сломать @1в.", "Мп", ch, object);
        act("1+д не удалось сломать @1в.", "Кмп", ch, object);
    }

}

void go_break(struct char_data *ch, struct obj_data *obj)
{
    struct event_param_data params;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    char buf4[MAX_STRING_LENGTH];


    if (GET_OBJ_TYPE(obj) == ITEM_FOOD ||
        GET_OBJ_TYPE(obj) == ITEM_SCROLL ||
        GET_OBJ_TYPE(obj) == ITEM_FRAGMENT ||
        GET_OBJ_TYPE(obj) == ITEM_SPARE || GET_OBJ_TYPE(obj) == ITEM_CORPSE) {
        act("Сломать $o нельзя.", FALSE, ch, obj, 0, TO_CHAR);
        return;
    }

    init_event_param(&params);

    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.object = obj;
    params.actor = ch;

    params.sto_actor = "Крепко обхватив руками @1в, Вы принялись @1ер ломать.";
    params.sto_room = "Крепко обхватив руками @1в, 1+и принял1(ся,ась,ось,ись) @1ер ломать.";

    sprintf(buf1, "Вы ломаете %s.", get_name_pad(obj->names, PAD_VIN, PAD_OBJECT));
    sprintf(buf2, "пытается сломать %s", get_name_pad(obj->names, PAD_VIN, PAD_OBJECT));
    sprintf(buf3, "Вы прекратили ломать %s.", get_name_pad(obj->names, PAD_VIN, PAD_OBJECT));
    sprintf(buf4, "1+и прекратил1(,а,о,и) ломать %s.",
            get_name_pad(obj->names, PAD_VIN, PAD_OBJECT));

    params.status = buf1;
    params.rstatus = buf2;
    params.bto_actor = buf3;
    params.bto_room = buf4;

    add_event(GET_OBJ_QUALITY(obj) * 5, 0, event_break, &params);
}

/****************************************************************************/

ACMD(do_isort)
{
    char arg[MAX_STRING_LENGTH];
    struct obj_data *container = NULL;
    struct char_data *ctmp = NULL;
    int where_bits = FIND_OBJ_EQUIP | FIND_OBJ_INV;

    one_argument(argument, arg);

    if (!*arg) {
        if (ch->carrying) {
            send_to_charf(ch, "Вы раcсортировали содержимое своего инвентаря.\r\n");
            ch->carrying = SortObjects(ch->carrying, SORT_NAME);
        } else
            send_to_charf(ch, "У Вас в инвентаре пусто.\r\n");
    } else {
        generic_find(arg, where_bits, ch, &ctmp, &container);

        if (!container) {
            send_to_charf(ch, "У Вас нет '%s'.\r\n", arg);
            return;
        }

        act("Вы отсортировали содержимое $o1.", FALSE, ch, container, 0, TO_CHAR);
        container->contains = SortObjects(container->contains, SORT_NAME);
    }

}

AEVENT(event_enchant)
{
    int enchnum = params->narg[0];
    struct obj_data *obj = params->object;
    struct char_data *ch = params->actor;
    struct C_obj_affected_type af;
    int skill = GET_SKILL(ch, SKILL_ENCHANT);
    int prob = GET_REAL_WIS(ch) + GET_REAL_WIS(ch) + GET_REAL_WIS(ch) + skill + RNDSKILL;
    int percent = (GET_OBJ_QUALITY(obj) * 3) + 50 + RNDSKILL;
    int i[2], j;

    improove_skill(ch, 0, GET_OBJ_QUALITY(obj) * 3, SKILL_ENCHANT);

    if (prob >= percent) {
        GET_OBJ_EXTRA(obj, ITEM_ENCHANT) |= ITEM_ENCHANT;
        int nums = GET_ENCHANT(enchnum)->GetItem(ECH_APPLY)->GetStrListNumber();

        for (j = 0; j < nums; j++) {
            GET_ENCHANT(enchnum)->GetItem(ECH_APPLY)->GetStrList(j, i[0], i[1]);
            af.type = 0;
            af.duration = 0;
            af.modifier = 0;
            af.location = 0;
            af.bitvector = 0;
            af.extra = 0;
            af.no_flag = 0;
            af.anti_flag = 0;
            af.type = find_spell_num(SPELL_ENCHANT);
            af.location = i[0];
            af.modifier = i[1];
            af.bitvector = 0;
            af.extra = 0;
            af.no_flag = 0;
            af.anti_flag = 0;
            af.duration = -1;
            affect_to_object(obj, &af);
        }

        if (GET_ENCHANT(enchnum)->GetItem(ECH_AFFECT)->GetString()) {
            new_flag test;

            test.flags[0] = 0;
            test.flags[1] = 0;
            test.flags[2] = 0;
            test.flags[3] = 0;

            asciiflag_conv((char *) GET_ENCHANT(enchnum)->GetItem(ECH_AFFECT)->GetString(),
                           &GET_FLAG(test, 0));

            if (IS_SET(GET_FLAG(test, AFF_BLIND), AFF_BLIND))   //СЛЕПОТА
                set_affect_obj(obj, AFF_BLIND);

            if (IS_SET(GET_FLAG(test, AFF_INVISIBLE), AFF_INVISIBLE))   //НЕВИДИМОСТЬ
                set_affect_obj(obj, AFF_INVISIBLE);

            if (IS_SET(GET_FLAG(test, AFF_DETECT_ALIGN), AFF_DETECT_ALIGN))     //ЗНАНИЕ_НАКЛОНОСТЕЙ
                set_affect_obj(obj, AFF_DETECT_ALIGN);

            if (IS_SET(GET_FLAG(test, AFF_DETECT_INVIS), AFF_DETECT_INVIS))     //ВИДЕТЬ_НЕВИДИМОЕ
                set_affect_obj(obj, AFF_DETECT_INVIS);

            if (IS_SET(GET_FLAG(test, AFF_DETECT_MAGIC), AFF_DETECT_MAGIC))     //ВИДЕТЬ_МАГИЮ
                set_affect_obj(obj, AFF_DETECT_MAGIC);

            if (IS_SET(GET_FLAG(test, AFF_SENSE_LIFE), AFF_SENSE_LIFE)) //ЧУВСТВОВАТЬ_ЖИЗНЬ
                set_affect_obj(obj, AFF_SENSE_LIFE);

            if (IS_SET(GET_FLAG(test, AFF_WATERWALK), AFF_WATERWALK))   //ХОЖДЕНИЕ_ПО_ВОДЕ
                set_affect_obj(obj, AFF_WATERWALK);

            //СВЯТОСТЬ
            if (IS_SET(GET_FLAG(test, AFF_INFRAVISION), AFF_INFRAVISION))       //ИНФРАЗРЕНИЕ
                set_affect_obj(obj, AFF_INFRAVISION);

            if (IS_SET(GET_FLAG(test, AFF_PROTECT_EVIL), AFF_PROTECT_EVIL))     //ЗАЩИТА_ОТ_ЗЛА
                set_affect_obj(obj, AFF_PROTECT_EVIL);

            if (IS_SET(GET_FLAG(test, AFF_PROTECT_GOOD), AFF_PROTECT_GOOD))     //ЗАЩИТА_ОТ_ДОБРА
                set_affect_obj(obj, AFF_PROTECT_GOOD);

            if (IS_SET(GET_FLAG(test, AFF_NOTRACK), AFF_NOTRACK))       //НЕ_ВЫСЛЕДИТЬ
                set_affect_obj(obj, AFF_NOTRACK);

            //КРАДЕТЬСЯ
            /* if (IS_SET(GET_FLAG(test,AFF_SNEAK), AFF_SNEAK))
               set_affect_obj(obj,AFF_SNEAK); */

            //ПРЯЧЕТЬСЯ
            /* if (IS_SET(GET_FLAG(test,AFF_HIDE), AFF_HIDE))
               set_affect_obj(obj,AFF_HIDE); */

            //ЯРОСТЬ
            /* if (IS_SET(GET_FLAG(test,AFF_COURAGE), AFF_COURAGE))
               set_affect_obj(obj,AFF_COURAGE); */

            if (IS_SET(GET_FLAG(test, AFF_HOLD), AFF_HOLD))     //ПАРАЛИЧ
                set_affect_obj(obj, AFF_HOLD);

            if (IS_SET(GET_FLAG(test, AFF_FLY), AFF_FLY))       //ПОЛЕТ
                set_affect_obj(obj, AFF_FLY);

            if (IS_SET(GET_FLAG(test, AFF_SIELENCE), AFF_SIELENCE))     //НЕМОЙ
                set_affect_obj(obj, AFF_SIELENCE);

            if (IS_SET(GET_FLAG(test, AFF_AWARNESS), AFF_AWARNESS))     //НАСТОРОЖЕН
                set_affect_obj(obj, AFF_AWARNESS);

            if (IS_SET(GET_FLAG(test, AFF_HOLYLIGHT), AFF_HOLYLIGHT))   //СВЕТ
                set_affect_obj(obj, AFF_HOLYLIGHT);

            if (IS_SET(GET_FLAG(test, AFF_HOLYDARK), AFF_HOLYDARK))     //ТЬМА
                set_affect_obj(obj, AFF_HOLYDARK);

            if (IS_SET(GET_FLAG(test, AFF_DETECT_POISON), AFF_DETECT_POISON))   //ОПРЕДЕЛЕНИЕ_ЯДА
                set_affect_obj(obj, AFF_DETECT_POISON);

            //МАСКИРОВКА
            /* if (IS_SET(GET_FLAG(test,AFF_CAMOUFLAGE), AFF_CAMOUFLAGE))
               set_affect_obj(obj,AFF_CAMOUFLAGE); */

            if (IS_SET(GET_FLAG(test, AFF_WATERBREATH), AFF_WATERBREATH))       //ДЫХАНИЕ_ВОДОЙ
                set_affect_obj(obj, AFF_WATERBREATH);

            //ЧУМА
            if (IS_SET(GET_FLAG(test, AFF_PLAGUE), AFF_PLAGUE))
                set_affect_obj(obj, AFF_PLAGUE);

            if (IS_SET(GET_FLAG(test, AFF_DARKVISION), AFF_DARKVISION)) //НОЧНОЕ_ЗРЕНИЕ
                set_affect_obj(obj, AFF_DARKVISION);

            if (IS_SET(GET_FLAG(test, AFF_DEAFNESS), AFF_DEAFNESS))     //ГЛУХОТА
                set_affect_obj(obj, AFF_DEAFNESS);

            //ЛУЧ_СВЕТА
            /* if (IS_SET(GET_FLAG(test,AFF_SUNBEAM), AFF_SUNBEAM))
               set_affect_obj(obj,AFF_SUNBEAM); */

            if (IS_SET(GET_FLAG(test, AFF_ILLNESS), AFF_ILLNESS))       //БОЛЕЗНЬ
                set_affect_obj(obj, AFF_ILLNESS);

            //SET_BIT(GET_FLAG(test,AFF_LEVIT), AFF_LEVIT);
            if (IS_SET(GET_FLAG(test, AFF_LEVIT), AFF_LEVIT))   //ЛЕВИТАЦИЯ
                set_affect_obj(obj, AFF_LEVIT);
        }
        act("Вы удачно зачаровали @1в.", "Мп", ch, obj);
    } else {
        act("Вы не смогли зачаровать @1в.", "Мп", ch, obj);
    }

    remove_enchant_components(ch, enchnum);
}

ACMD(do_enchant)
{
    int enchnum, ench_wear = 0, spell_enchant = find_spell_num(SPELL_ENCHANT);;
    char *s, *t;
    struct event_param_data params;
    struct obj_data *obj = NULL;
    struct C_obj_affected_type *af;

    init_event_param(&params);

    if (!check_fight_command(ch))
        return;

    if (!GET_SKILL(ch, SKILL_ENCHANT)) {
        send_to_charf(ch, "Вас не научили зачаровывать предметы.\r\n");
        return;
    }

    s = strtok(argument, ch->divd);
    if (s == NULL) {
        send_to_char("Как именно Вы хотите зачаровать?\r\n", ch);
        return;
    }

    s = strtok(NULL, ch->divd);

    if (s == NULL) {
        send_to_charf(ch, "Название рецепта должно быть заключено в символы: &C'%s'&n.\r\n",
                      ch->divd);
        return;
    }

    t = strtok(NULL, "\0");

    enchnum = find_enchant_num(s);

    if (enchnum < 0) {
        send_to_charf(ch, "Вы не знаете такого рецепта зачарования.\r\n");
        return;
    }

    if (!IS_SET(GET_ENCHANT_TYPE(ch, ENCHANT_NO(enchnum)), SPELL_TEMP | SPELL_KNOW) && !IS_GOD(ch)) {
        send_to_charf(ch, "Вы не знаете такого рецепта зачарования.\r\n");
        return;
    }

    if (t == NULL) {
        send_to_charf(ch, "Какой предмет Вы хотите зачаровать?\r\n");
        return;
    }

    if (!(obj = get_obj_in_list_vis(ch, t, ch->carrying))) {
        send_to_charf(ch, "У Вас нет '%s'.\r\n", t);
        return;
    }

    asciiflag_conv((char *) ENCHANT_WEAR(enchnum), &ench_wear);

    if (GET_OBJ_TYPE(obj) != ENCHANT_TYPE(enchnum)) {
        act("Этот рецепт зачарования не подходит к @1д.", "Мп", ch, obj);
        //send_to_charf(ch,"Сравнение %d и %d\r\n",GET_OBJ_TYPE(obj), ENCHANT_TYPE(enchnum));
        return;
    }

    if (ench_wear && !CAN_WEAR(obj, ench_wear)) {
        act("Этот рецепт зачарования не подходит к @1д.", "Мп", ch, obj);
        //send_to_charf(ch,"Сравние %d\r\n",CAN_WEAR(obj, ench_wear));
        return;
    }

    if (!find_enchant_components(ch, enchnum)) {
        send_to_charf(ch, "У Вас нет всех необходимых компонентов.\r\n");
        return;
    }


    for (af = obj->C_affected; af; af = af->next) {
        if (af->type == spell_enchant) {
            act("@1и уже зачарован, эффект зачарования будет заменен.", "Мп", ch, obj);
            affect_obj_removed(obj, af);
            break;
        }
    }

    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    params.stopflag = STOP_HIDDEN;
    params.show_meter = TRUE;
    params.narg[0] = enchnum;
    params.object = obj;
    params.actor = ch;
    params.sto_actor = "Вы начали зачаровывать @1и.";
    params.sto_room = "1+и начал1(,а,о,и) зачаровывать @1и.";
    sprintf(buf1, "Вы занимаетесь зачарованием %s", get_name_pad(obj->names, PAD_ROD, PAD_OBJECT));
    sprintf(buf2, "занимается зачарованием %s", get_name_pad(obj->names, PAD_ROD, PAD_OBJECT));
    params.status = buf1;
    params.rstatus = buf2;

    params.bto_actor = "Вы прервали процесс зачарования.";
    params.bto_room = "1+и прервал1(,а,о,и) процесс зачарования.";

    add_event(60, 0, event_enchant, &params);
}

int focn(struct char_data *ch, int vnum)
{
    int tcount = 0;
    struct obj_data *tobj = NULL;

    for (tobj = ch->carrying; tobj; tobj = tobj->next_content) {
        if (GET_OBJ_VNUM(tobj) != vnum)
            continue;
        tcount++;
    }

    return (tcount);
}


bool foci(struct char_data * ch, int vnum, int count)
{
    bool result = FALSE;
    int tcount = 0;
    struct obj_data *tobj = NULL;

    for (tobj = ch->carrying; tobj; tobj = tobj->next_content) {
        if (GET_OBJ_VNUM(tobj) != vnum)
            continue;
        tcount++;
    }

    if (tcount >= count)
        result = TRUE;

    return (result);
}

void reci(struct char_data *ch, int vnum, int count)
{
    int tcount = 0;
    struct obj_data *tobj = NULL, *onext = NULL;

    for (tobj = ch->carrying; tobj; tobj = onext) {
        onext = tobj->next_content;
        if (GET_OBJ_VNUM(tobj) != vnum)
            continue;
        obj_from_char(tobj);
        extract_obj(tobj);
        tcount++;
        if (tcount >= count)
            return;
    }
}

bool find_enchant_components(struct char_data * ch, int num)
{
    CItem *enchant = xEnchant.GetItem(num);
    int i, numadd, vnum, count;

    numadd = enchant->GetItem(ECH_INCLUDE)->GetNumberItem();
    for (i = 0; i < numadd; i++) {
        CItem *include = enchant->GetItem(ECH_INCLUDE)->GetItem(i);

        vnum = include->GetItem(ECH_INC_TYPE)->GetInt();
        count = include->GetItem(ECH_INC_COUNT)->GetInt();
        //send_to_charf(ch,"Требования компонент %d, количество %d\r\n",vnum,count);
        if (!foci(ch, vnum, count))
            return (FALSE);
    }
    return (TRUE);
}

void remove_enchant_components(struct char_data *ch, int num)
{
    CItem *enchant = xEnchant.GetItem(num);
    int i, numadd, vnum, count;

    numadd = enchant->GetItem(ECH_INCLUDE)->GetNumberItem();
    for (i = 0; i < numadd; i++) {
        CItem *include = enchant->GetItem(ECH_INCLUDE)->GetItem(i);

        vnum = include->GetItem(ECH_INC_TYPE)->GetInt();
        count = include->GetItem(ECH_INC_COUNT)->GetInt();
        //send_to_charf(ch,"Удаляю компонент %d, количество %d\r\n",vnum,count);
        reci(ch, vnum, count);
    }
}

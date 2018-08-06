
#include "sysdep.h"
#include "structs.h"
#include "db.h"
#include "comm.h"
#include "utils.h"
#include "handler.h"
#include "spells.h"
#include "xscripts.h"
#include "xboot.h"
#include "parser_id.h"
#include "pk.h"
#include "events.h"
#include "strlib.h"
#include "expr.h"
#include "interpreter.h"
#include "planescape.h"
#include "mudfilereader.h"

/* local functions */
void scr_damage(struct char_data *ch, CItem * dam, int vnum);

int boot_scripts(void)
{
    if (!Scr.Initialization())
        exit(1);

    MudFileReader reader(mud->scrDir);

    while (reader.hasNext())
        if (!Scr.ReadConfig(reader.next().getCPath()))
            exit(1);

    return Scr.GetNumberItem();
}

void go_script(int vnum, struct char_data *ch)
{
    go_script(vnum, ch, NULL, NULL, NOWHERE, NULL);
}

void go_script(int vnum, struct char_data *ch, struct char_data *victim)
{
    go_script(vnum, ch, victim, NULL, NOWHERE, NULL);
}

void go_script(int vnum, struct char_data *ch, struct char_data *victim, struct obj_data *obj)
{
    go_script(vnum, ch, victim, obj, NOWHERE, NULL);
}

void go_script(int vnum, struct char_data *ch, struct obj_data *obj)
{
    go_script(vnum, ch, NULL, obj, NOWHERE, NULL);
}

void go_script(int vnum, struct char_data *ch, char *arg)
{
    go_script(vnum, ch, NULL, NULL, NOWHERE, arg);
}

void go_script(int vnum, struct char_data *ch, struct char_data *victim, char *arg)
{
    go_script(vnum, ch, victim, NULL, NOWHERE, arg);
}

void go_script(int vnum, struct char_data *ch, struct char_data *victim, struct obj_data *obj,
               char *arg)
{
    go_script(vnum, ch, victim, obj, NOWHERE, arg);
}

void go_script(int vnum, struct char_data *ch, struct obj_data *obj, char *arg)
{
    go_script(vnum, ch, NULL, obj, NOWHERE, arg);
}

void go_script(int vnum, struct char_data *ch, struct char_data *victim, struct obj_data *obj,
               int roomnum, char *arg)
{
    if (vnum == 0)
        return;

    int i, num, in_room = IN_ROOM(ch), n, lcount;
    CItem *Script = Scr.FindItem(vnum);


    if (!Script) {
        log("ОШИБКА: Вызвано неизвестное действие %d", vnum);
        return;
    }
//УСЛОВИЯ
    if (ch && Script->GetItem(SCR_EXPR)->GetExpression()) {
        CExpression *expr;

        expr = (CExpression *) Script->GetItem(SCR_EXPR)->GetExpression();

        if (!expr->Expr(ch, victim, arg)) {
            num = Script->GetItem(SCR_ERROR)->GetNumberItem();
            for (n = 0; n < num; n++) {
                CItem *mess = Script->GetItem(SCR_ERROR)->GetItem(n);

                if (mess->GetItem(SCR_ECHAR)->GetString()) {
                    if (victim)
                        act(mess->GetItem(SCR_ECHAR)->GetString(), "Мм", ch, victim);
                    else
                        act(mess->GetItem(SCR_ECHAR)->GetString(), "М", ch);
                }
                if (mess->GetItem(SCR_EROOM)->GetString()) {
                    if (victim)
                        act(mess->GetItem(SCR_EROOM)->GetString(), "Кмм", ch, victim);
                    else
                        act(mess->GetItem(SCR_EROOM)->GetString(), "Км", ch);
                }
                if (mess->GetItem(SCR_ESCRIPT)->GetInt()) {
                    if (obj)
                        go_script(mess->GetItem(SCR_ESCRIPT)->GetInt(), ch, obj);
                    else
                        go_script(mess->GetItem(SCR_ESCRIPT)->GetInt(), ch);
                }
            }
            //delete expr;
            return;
        }
        //delete expr;
    }
//log("CКРИПТ.НАЧАЛО --- #%d",vnum);
//СООБЩЕНИЯ
    num = Script->GetItem(SCR_MESSAGE)->GetNumberItem();
    for (n = 0; n < num; n++) {
        CItem *mess = Script->GetItem(SCR_MESSAGE)->GetItem(n);

        if (!mess->GetItem(SCR_MVNUM)->GetInt() && !ch)
            continue;
        if (ch && mess->GetItem(SCR_MCHAR)->GetString()) {
            if (victim)
                act(mess->GetItem(SCR_MCHAR)->GetString(), "Мм", ch, victim);
            else
                act(mess->GetItem(SCR_MCHAR)->GetString(), "М", ch);
        }

        if (mess->GetItem(SCR_MVNUM)->GetInt()) {
            int rnum = real_room(mess->GetItem(SCR_MVNUM)->GetInt());

            if (rnum == -1)
                continue;
            if (!world[rnum].people)
                continue;
            if (mess->GetItem(SCR_MROOM)->GetString()) {
                if (victim)
                    act(mess->GetItem(SCR_MROOM)->GetString(), "КМм", world[rnum].people, victim);
                else
                    act(mess->GetItem(SCR_MROOM)->GetString(), "КМ", world[rnum].people);
            }
        } else {
            if (ch && mess->GetItem(SCR_MROOM)->GetString()) {
                if (victim)
                    act(mess->GetItem(SCR_MROOM)->GetString(), "Кмм", ch, victim);
                else
                    act(mess->GetItem(SCR_MROOM)->GetString(), "Км", ch);
            }
        }

    }


//Загрузка
    lcount = Script->GetItem(SCR_LOAD)->GetNumberItem();
    for (n = 0; n < lcount; n++) {
        bool skip = FALSE;
        CItem *load = Script->GetItem(SCR_LOAD)->GetItem(n);

        num = load->GetItem(SCR_LOBJ)->GetStrListNumber();      //предметы
        for (i = 0; i < num; i++) {
            int ovnum = 0, room = 0, rnum = 0;
            struct obj_data *obj = NULL;

            load->GetItem(SCR_LOBJ)->GetStrList(i, ovnum, room);
            if (room == 1)
                rnum = in_room;
            else
                rnum = real_room(room);
            if (!(obj = read_object(ovnum, VIRTUAL, TRUE))) {
                log("ОШИБКА: Не найден ПРЕДМЕТ %d в структуре ДЕЙСТВИЕ.ЗАГРУЗКА %d\r\n", ovnum,
                    vnum);
                continue;
            }
            if (rnum == NOWHERE) {
                log("ОШИБКА: Неизвестная локация %d в ПРЕДМЕТ %d в структуре ДЕЙСТВИЕ.ЗАГРУЗКА %d\r\n", rnum, ovnum, vnum);
                continue;
            }
            obj_to_room(obj, rnum);
        }

        num = load->GetItem(SCR_LMOB)->GetStrListNumber();      //монстры
        for (i = 0; i < num; i++) {
            int mvnum = 0, room = 0, rnum = 0;
            struct char_data *tch = NULL;

            load->GetItem(SCR_LMOB)->GetStrList(i, mvnum, room);
            if (room == 1)
                rnum = in_room;
            else
                rnum = real_room(room);
            if (!(tch = read_mobile(mvnum, VIRTUAL))) {
                log("ОШИБКА: Не найден МОНСТР %d в структуре ДЕЙСТВИЕ.ЗАГРУЗКА %d\r\n", mvnum,
                    vnum);
                continue;
            }
            if (rnum == NOWHERE) {
                log("ОШИБКА: Неизвестная локация %d в МОНСТР %d в структуре ДЕЙСТВИЕ.ЗАГРУЗКА %d\r\n", rnum, mvnum, vnum);
                continue;
            }
            char_to_room(tch, rnum);
        }

        if (ch) {
            const int *equip = load->GetItem(SCR_LEQ)->GetScript(num);

            for (i = 0; i < num; i++) {
                int ovnum = equip[i];
                struct obj_data *obj = NULL;

                if (!(obj = read_object(ovnum, VIRTUAL, TRUE))) {
                    log("ОШИБКА: Не найден ЭКИПИРОВКА %d в структуре ДЕЙСТВИЕ.ЗАГРУЗКА %d", ovnum,
                        vnum);
                    continue;
                }
                log("ПРОВЕРКА: %s %d > %d", GET_OBJ_PNAME(obj, 0), GET_OBJ_LIMIT(obj),
                    count_obj_vnum(ovnum));
                if (GET_OBJ_LIMIT(obj) < count_obj_vnum(ovnum)) {
                    if (load->GetItem(SCR_OBJ_LIMIT)->GetInt()) {       //Если указано действие
                        go_script(load->GetItem(SCR_OBJ_LIMIT)->GetInt(), ch);
                        skip = TRUE;
                    }
                } else
                    obj_to_char(obj, ch);
            }
        }

        if (ch) {
            const int *random = load->GetItem(SCR_LRANDOM)->GetScript(num);

            if (num) {
                int ovnum = random[number(0, num - 1)];
                struct obj_data *obj = NULL;

                if (!(obj = read_object(ovnum, VIRTUAL, TRUE))) {
                    log("ОШИБКА: Не найден СЛУЧАЙНЫЙ %d в структуре ДЕЙСТВИЕ.ЗАГРУЗКА %d", ovnum,
                        vnum);
                    continue;
                }
                log("ПРОВЕРКА: %s %d > %d", GET_OBJ_PNAME(obj, 0), GET_OBJ_LIMIT(obj),
                    count_obj_vnum(ovnum));
                if (GET_OBJ_LIMIT(obj) < count_obj_vnum(ovnum)) {
                    if (load->GetItem(SCR_OBJ_LIMIT)->GetInt()) {       //Если указано действие
                        go_script(load->GetItem(SCR_OBJ_LIMIT)->GetInt(), ch);
                        skip = TRUE;
                    }
                } else
                    obj_to_char(obj, ch);
            }
        }

        if (ch && !skip) {
            if (load->GetItem(SCR_LCHAR)->GetString())
                act(load->GetItem(SCR_LCHAR)->GetString(), "М", ch);
            if (load->GetItem(SCR_LROOM)->GetString())
                act(load->GetItem(SCR_LROOM)->GetString(), "Км", ch);
        }
        //Выход
        num = load->GetItem(SCR_LEXIT)->GetNumberItem();
        for (i = 0; i < num; i++) {
            bool newdoor = FALSE;
            CItem *exit = load->GetItem(SCR_LEXIT)->GetItem(i);
            int room_nr = real_room(exit->GetItem(SCR_LE_ROOM)->GetInt());
            int to_room = real_room(exit->GetItem(SCR_LE_ROOMNUMBER)->GetInt());

            if (room_nr == NOWHERE) {
                log("ОШИБКА: Неизвестная ГДЕ %d в ДЕЙСТВИЕ.ЗАГРУЗКА %d",
                    exit->GetItem(SCR_LE_ROOM)->GetInt(), vnum);
                continue;
            }
            if (to_room == NOWHERE) {
                log("ОШИБКА: Неизвестная КОМНАТА %d в ДЕЙСТВИЕ.ЗАГРУЗКА %d",
                    exit->GetItem(SCR_LE_ROOMNUMBER)->GetInt(), vnum);
                continue;
            }
            int dir = exit->GetItem(SCR_LE_DIRECTION)->GetInt();

            if (!world[room_nr].dir_option[dir]) {
                CREATE(world[room_nr].dir_option[dir], struct room_direction_data, 1);

                newdoor = TRUE;
            }

            world[room_nr].dir_option[dir]->to_room = to_room;
            if (newdoor) {
                if (exit->GetItem(SCR_LE_DESCRIPTION)->GetString())
                    world[room_nr].dir_option[dir]->general_description =
                        str_dup(exit->GetItem(SCR_LE_DESCRIPTION)->GetString());
                if (exit->GetItem(SCR_LE_ALIAS)->GetString())
                    world[room_nr].dir_option[dir]->keyword =
                        str_dup(exit->GetItem(SCR_LE_ALIAS)->GetString());
                if (exit->GetItem(SCR_LE_KEY)->GetInt())
                    world[room_nr].dir_option[dir]->sex = exit->GetItem(SCR_LE_SEX)->GetInt();
                if (exit->GetItem(SCR_LE_NAME)->GetString())
                    world[room_nr].dir_option[dir]->exit_name =
                        str_dup(exit->GetItem(SCR_LE_NAME)->GetString());
            }

            if (exit->GetItem(SCR_LE_KEY)->GetInt())
                world[room_nr].dir_option[dir]->key = exit->GetItem(SCR_LE_KEY)->GetInt();

            if (exit->GetItem(SCR_LE_PROPERTIES)->GetString()) {
                //world[room_nr].dir_option[dir]->exit_data_reset = world[room_nr].dir_option[dir]->exit_data;
                asciiflag_conv((char *) exit->GetItem(SCR_LE_PROPERTIES)->GetString(),
                               &world[room_nr].dir_option[dir]->exit_data);
            } else {
                REMOVE_BIT(GET_FLAG(world[room_nr].dir_option[dir]->exit_data, EXIT_CLOSED),
                           EXIT_CLOSED);
                REMOVE_BIT(GET_FLAG(world[room_nr].dir_option[dir]->exit_data, EXIT_LOCKED),
                           EXIT_LOCKED);
            }

            if (newdoor)
                SET_BIT(GET_FLAG(world[room_nr].dir_option[dir]->exit_data, EXIT_REPOP),
                        EXIT_REPOP);

            if (exit->GetItem(SCR_LE_QUALITY)->GetInt())
                world[room_nr].dir_option[dir]->quality = exit->GetItem(SCR_LE_QUALITY)->GetInt();
            if (exit->GetItem(SCR_LE_TRAP)->GetNumberItem()) {
                CItem *etrap = exit->GetItem(SCR_LE_TRAP)->GetItem(0);  //всегда первая и единственная запись

                world[room_nr].dir_option[dir]->shance =
                    etrap->GetItem(SCR_LE_TRAP_CHANCE)->GetInt();
                if (etrap->GetItem(SCR_LE_TRAP_TYPEDAMAGE)->GetInt())
                    world[room_nr].dir_option[dir]->type_hit =
                        etrap->GetItem(SCR_LE_TRAP_TYPEDAMAGE)->GetInt();
                if (etrap->GetItem(SCR_LE_TRAP_FORCEDAMAGE)->GetString()) {
                    int t[3];

                    if (sscanf
                        (etrap->GetItem(SCR_LE_TRAP_FORCEDAMAGE)->GetString(), "%dd%d+%d", t, t + 1,
                         t + 2) == 3) {
                        world[room_nr].dir_option[dir]->damnodice = t[0];
                        world[room_nr].dir_option[dir]->damsizedice = t[1];
                        world[room_nr].dir_option[dir]->damage = t[2];
                    }
                }
                if (etrap->GetItem(SCR_LE_TRAP_SAVE)->GetInt())
                    world[room_nr].dir_option[dir]->save =
                        etrap->GetItem(SCR_LE_TRAP_SAVE)->GetInt();
                if (etrap->GetItem(SCR_LE_TRAP_MESS_CHAR)->GetString())
                    world[room_nr].dir_option[dir]->trap_damage_char =
                        str_dup(etrap->GetItem(SCR_LE_TRAP_MESS_CHAR)->GetString());
                if (etrap->GetItem(SCR_LE_TRAP_MESS_ROOM)->GetString())
                    world[room_nr].dir_option[dir]->trap_damage_room =
                        str_dup(etrap->GetItem(SCR_LE_TRAP_MESS_ROOM)->GetString());
                if (etrap->GetItem(SCR_LE_TRAP_MESS_SCHAR)->GetString())
                    world[room_nr].dir_option[dir]->trap_nodamage_char =
                        str_dup(etrap->GetItem(SCR_LE_TRAP_MESS_SCHAR)->GetString());
                if (etrap->GetItem(SCR_LE_TRAP_MESS_SROOM)->GetString())
                    world[room_nr].dir_option[dir]->trap_nodamage_room =
                        str_dup(etrap->GetItem(SCR_LE_TRAP_MESS_SROOM)->GetString());
                if (etrap->GetItem(SCR_LE_TRAP_MESS_KCHAR)->GetString())
                    world[room_nr].dir_option[dir]->trap_kill_char =
                        str_dup(etrap->GetItem(SCR_LE_TRAP_MESS_KCHAR)->GetString());
                if (etrap->GetItem(SCR_LE_TRAP_MESS_KROOM)->GetString())
                    world[room_nr].dir_option[dir]->trap_kill_room =
                        str_dup(etrap->GetItem(SCR_LE_TRAP_MESS_KROOM)->GetString());
            }
        }                       //Конец Выход
        //Портал
        num = load->GetItem(SCR_LPORTAL)->GetNumberItem();
        for (i = 0; i < num; i++) {
            CItem *portal = load->GetItem(SCR_LPORTAL)->GetItem(i);
            int dir = portal->GetItem(SCR_LP_DIRECTION)->GetInt();
            int room_nr = real_room(portal->GetItem(SCR_LP_ROOM)->GetInt());
            int to_room = real_room(portal->GetItem(SCR_LP_ROOMNUMBER)->GetInt());

            if (room_nr == NOWHERE) {
                log("ОШИБКА: Неизвестная ГДЕ %d в ДЕЙСТВИЕ.ЗАГРУЗКА %d",
                    portal->GetItem(SCR_LP_ROOM)->GetInt(), vnum);
                continue;
            }
            if (to_room == NOWHERE) {
                log("ОШИБКА: Неизвестная КОМНАТА %d в ДЕЙСТВИЕ.ЗАГРУЗКА %d",
                    portal->GetItem(SCR_LP_ROOMNUMBER)->GetInt(), vnum);
                continue;
            }

            if (!world[room_nr].dir_option[dir]) {
                CREATE(world[room_nr].dir_option[dir], struct room_direction_data, 1);

                world[room_nr].dir_option[dir]->to_room = -1;
            }

            world[room_nr].dir_option[dir]->type_port = 4;      //portal->GetItem(SCR_LP_TYPE)->GetInt();
            world[room_nr].dir_option[dir]->key_port = portal->GetItem(SCR_LP_KEY)->GetInt();
            world[room_nr].dir_option[dir]->room_port = to_room;
            world[room_nr].dir_option[dir]->time = portal->GetItem(SCR_LP_TIME)->GetInt();

            if (portal->GetItem(SCR_LP_DESCRIPTION)->GetString())
                world[room_nr].dir_option[dir]->portal_description =
                    str_dup(portal->GetItem(SCR_LP_DESCRIPTION)->GetString());

            if (portal->GetItem(SCR_LP_ACTIVE)->GetString())
                world[room_nr].dir_option[dir]->mess_to_open =
                    str_dup(portal->GetItem(SCR_LP_ACTIVE)->GetString());

            if (portal->GetItem(SCR_LP_DEACTIVE)->GetString())
                world[room_nr].dir_option[dir]->mess_to_close =
                    str_dup(portal->GetItem(SCR_LP_DEACTIVE)->GetString());

            if (portal->GetItem(SCR_LP_MESS_CHAR)->GetString())
                world[room_nr].dir_option[dir]->mess_char_enter =
                    str_dup(portal->GetItem(SCR_LP_MESS_CHAR)->GetString());

            if (portal->GetItem(SCR_LP_MESS_ROOM)->GetString())
                world[room_nr].dir_option[dir]->mess_room_enter =
                    str_dup(portal->GetItem(SCR_LP_MESS_ROOM)->GetString());
        }
        if (ch && load->GetItem(SCR_LGOLD)->GetInt()) {
            int value = load->GetItem(SCR_LGOLD)->GetInt();

            GET_GOLD(ch) += value;
            send_to_charf(ch, "Вы получили %d %s.\r\n", value, desc_count(value, WHAT_MONEYu));
        }
    }                           //КОНЕЦ ОБРАБОТКИ ЗАГРУЗКА

    lcount = Script->GetItem(SCR_DELETE)->GetNumberItem();
    for (n = 0; n < lcount; n++) {
        CItem *delt = Script->GetItem(SCR_DELETE)->GetItem(n);

        num = delt->GetItem(SCR_DOBJ)->GetStrListNumber();      //предметы
        for (i = 0; i < num; i++) {
            int ovnum = 0, room = 0, rnum = 0;
            struct obj_data *obj = NULL, *next_obj = NULL;

            delt->GetItem(SCR_DOBJ)->GetStrList(i, ovnum, room);
            if (room == 1) {
                for (obj = object_list; obj; obj = next_obj) {
                    next_obj = obj->next;
                    if (obj->in_room == NOWHERE || obj->worn_by || obj->carried_by)
                        continue;
                    if (GET_OBJ_VNUM(obj) == ovnum)
                        extract_obj(obj);
                }
            } else {
                rnum = real_room(room);
                if (rnum == NOWHERE) {
                    log("ОШИБКА: Неизвестная локация %d в ПРЕДМЕТ %d в структуре ДЕЙСТВИЕ.УДАЛЕНИЕ %d", rnum, ovnum, vnum);
                    continue;
                }
                for (obj = world[rnum].contents; obj; obj = next_obj) {
                    next_obj = obj->next_content;
                    if (GET_OBJ_VNUM(obj) == ovnum)
                        extract_obj(obj);
                }
            }
        }

        if (ch) {
            num = delt->GetItem(SCR_DEQ)->GetStrListNumber();   //предметы
            for (i = 0; i < num; i++) {
                struct obj_data *obj = NULL, *next_obj = NULL;
                int ovnum, count = 0, ocount, j;

                delt->GetItem(SCR_DEQ)->GetStrList(i, ovnum, ocount);
                for (j = 0; j < NUM_WEARS; j++) {
                    if (GET_EQ(ch, j) && GET_OBJ_VNUM(GET_EQ(ch, j)) == ovnum)
                        extract_obj(GET_EQ(ch, j));
                }
                for (obj = ch->carrying; obj; obj = next_obj) {
                    next_obj = obj->next_content;
                    if (ocount && count >= ocount)
                        continue;
                    if (GET_OBJ_VNUM(obj) == ovnum) {
                        extract_obj(obj);
                        count++;
                    }
                }
            }
        }

        num = delt->GetItem(SCR_DMOB)->GetStrListNumber();      //монстры
        for (i = 0; i < num; i++) {
            int mvnum = 0, room = 0, rnum = 0;
            struct char_data *tch = NULL, *next_tch = NULL;

            delt->GetItem(SCR_DMOB)->GetStrList(i, mvnum, room);
            if (room == 1) {
                for (tch = character_list; tch; tch = next_tch) {
                    next_tch = tch->next;
                    if (tch->in_room == NOWHERE)
                        continue;
                    if (GET_MOB_VNUM(tch) == mvnum)
                        extract_char(tch, FALSE);
                }
            } else {
                rnum = real_room(room);
                if (rnum == NOWHERE) {
                    log("ОШИБКА: Неизвестная локация %d в МОНСТР %d в структуре ДЕЙСТВИЕ.ЗАГРУЗКА %d", rnum, mvnum, vnum);
                    continue;
                }
                for (tch = world[rnum].people; tch; tch = next_tch) {
                    next_tch = tch->next_in_room;
                    if (GET_MOB_VNUM(tch) == mvnum)
                        extract_char(tch, FALSE);
                }
            }
        }

        if (ch && delt->GetItem(SCR_DGOLD)->GetInt()) {
            int value = delt->GetItem(SCR_DGOLD)->GetInt();
            bool am = FALSE;

            if (value >= GET_GOLD(ch)) {
                value = GET_GOLD(ch);
                am = TRUE;
            }
            GET_GOLD(ch) -= value;
            if (!am)
                send_to_charf(ch, "Вы потеряли %d %s.\r\n", value, desc_count(value, WHAT_MONEYu));
            else
                send_to_charf(ch, "Вы потеряли все наличные деньги.\r\n");
        }
        //УДАЛЕНИЕ.ВЫХОД
        num = delt->GetItem(SCR_DEXIT)->GetNumberItem();
        for (i = 0; i < num; i++) {
            CItem *exit = delt->GetItem(SCR_DEXIT)->GetItem(i);
            int room_nr = real_room(exit->GetItem(SCR_DE_ROOM)->GetInt());

            if (room_nr == NOWHERE) {
                log("ОШИБКА: Неизвестная ГДЕ %d в ДЕЙСТВИЕ.УДАЛЕНИЕ %d",
                    exit->GetItem(SCR_DE_ROOM)->GetInt(), vnum);
                continue;
            }
            int dir = exit->GetItem(SCR_DE_DIRECTION)->GetInt();

            if (world[room_nr].dir_option[dir]) {
                if (world[room_nr].dir_option[dir]->general_description)
                    free(world[room_nr].dir_option[dir]->general_description);
                if (world[room_nr].dir_option[dir]->keyword)
                    free(world[room_nr].dir_option[dir]->keyword);
                if (world[room_nr].dir_option[dir]->exit_name)
                    free(world[room_nr].dir_option[dir]->exit_name);
                free(world[room_nr].dir_option[dir]);
                world[room_nr].dir_option[dir] = NULL;
            }
        }

    }                           //КОНЕЦ ОБРАБОТКИ УДАЛЕНИЕ

//ЗАКЛИНАНИЕ
    if (ch) {
        num = Script->GetItem(SCR_SPELL)->GetNumberItem();
        for (i = 0; i < num; i++) {
            int spellnum = -1, level = 0;
            CItem *spell = Script->GetItem(SCR_SPELL)->GetItem(i);

            spellnum = find_spell_num(spell->GetItem(SCR_SPELL_NO)->GetInt());
            if (spellnum == -1) {
                log("ОШИБКА: Неизвестное заклинание %d в структуре ДЕЙСТВИЕ.ЗАКЛИНАНИЕ %d.",
                    Script->GetItem(SCR_SPELL_NO)->GetInt(), vnum);
                continue;
            }
            level = spell->GetItem(SCR_SPELL_LEVEL)->GetInt();
            if (spell->GetItem(SCR_SPELL_MCHAR)->GetString())
                act(spell->GetItem(SCR_SPELL_MCHAR)->GetString(), "М", ch);
            if (spell->GetItem(SCR_SPELL_MROOM)->GetString())
                act(spell->GetItem(SCR_SPELL_MROOM)->GetString(), "Км", ch);
            call_magic(ch, ch, NULL, spellnum, level, CAST_WAND, FALSE);
        }
    }
//ПЕРЕМЕННАЯ
    if (ch) {
        num = Script->GetItem(SCR_VAR)->GetNumberItem();
        for (i = 0; i < num; i++) {
            CItem *var = Script->GetItem(SCR_VAR)->GetItem(i);

            add_saved_var(ch, var->GetItem(SCR_VAR_NAME)->GetString(),
                          var->GetItem(SCR_VAR_VALUE)->GetString(),
                          var->GetItem(SCR_VAR_TIME)->GetInt());
        }
    }
//ГЛОБАЛЬНАЯ
    num = Script->GetItem(SCR_GLB)->GetNumberItem();
    for (i = 0; i < num; i++) {
        CItem *glb = Script->GetItem(SCR_GLB)->GetItem(i);

        global_vars.add(glb->GetItem(SCR_GLB_NAME)->GetString(),
                        glb->GetItem(SCR_GLB_VALUE)->GetString(),
                        glb->GetItem(SCR_GLB_TIME)->GetInt(), true);
    }

//СБРОС
    if (Script->GetItem(SCR_RESET)->GetInt()) {
        n = Script->GetItem(SCR_RESET)->GetInt();
        for (i = 0; i <= top_of_zone_table; i++)
            if (zone_table[i].number == n) {
                new_reset_zone(i);
                break;
            }
    }
//АГРЕССИЯ
    if (ch) {
        num = Script->GetItem(SCR_AGRO)->GetNumberItem();
        for (i = 0; i < num; i++) {
            CItem *agro = Script->GetItem(SCR_AGRO)->GetItem(i);
            struct char_data *tch = NULL;
            int rmob = real_mobile(agro->GetItem(SCR_AMOB)->GetInt());
            int target = agro->GetItem(SCR_ATARGET)->GetInt();

            if (rmob == -1) {
                log("ОШИБКА: Неизвестный монстр %d в структуре ДЕЙСТВИЕ.АГРЕССИЯ %d.",
                    agro->GetItem(SCR_AMOB)->GetInt(), vnum);
                continue;
            }
            for (tch = character_list; tch; tch = tch->next) {
                if (!IS_NPC(tch))
                    continue;
                if (GET_MOB_RNUM(tch) == rmob) {
                    //log("Добавляю агро %s[%d] к %s",GET_NAME(tch),agro->GetItem(SCR_AMOB)->GetInt(),GET_NAME(ch));
                    switch (target) {   //УСТАНАВЛИВАЕМ ФЛАГ В ЗАВИСИМОСТИ ОТ ЦЕЛИ
                        case 1:        //случайный в группе
                        case 2:        //всей группе
                            inc_pkill_group(ch, tch, 1, 0);
                            break;
                        default:       //только игроку
                            inc_pk_values(ch, tch, 1, 0);
                            break;
                    }
                }
            }
        }
    }
//ВРЕД
    if (ch) {
        num = Script->GetItem(SCR_DAMAGE)->GetNumberItem();
        for (i = 0; i < num; i++) {
            CItem *dam = Script->GetItem(SCR_DAMAGE)->GetItem(i);

            if (dam->GetItem(SCR_DAM_SHANCE)->GetInt() &&
                number(1, 100) > dam->GetItem(SCR_DAM_SHANCE)->GetInt())
                continue;

            switch (dam->GetItem(SCR_DAM_TARGET)->GetInt()) {
                case 1:        //Группа игрока
                    send_to_charf(ch, "Реализации отложена до восстребованности.\r\n");
                    break;
                case 2:        //Случайный в локации
                    send_to_charf(ch, "Реализации отложена до восстребованности.\r\n");
                    break;
                default:
                    scr_damage(ch, dam, vnum);
                    break;
            }
        }
    }
//ПЕРЕМЕЩЕНИЕ
    if (ch) {
        struct follow_type *f;

        num = Script->GetItem(SCR_TELEPORT)->GetNumberItem();
        for (i = 0; i < num; i++) {
            int room, inroom = NOWHERE;
            CItem *telep = Script->GetItem(SCR_TELEPORT)->GetItem(i);

            room = real_room(telep->GetItem(SCR_TROOM)->GetInt());
            if (telep->GetItem(SCR_TWHERE)->GetInt())
                inroom = real_room(telep->GetItem(SCR_TWHERE)->GetInt());
            if (room == NOWHERE)
                continue;
            if (inroom != NOWHERE && inroom != IN_ROOM(ch))
                continue;
            if (telep->GetItem(SCR_TCHAROUT)->GetString())
                act(telep->GetItem(SCR_TCHAROUT)->GetString(), "М", ch);
            if (telep->GetItem(SCR_TROOMOUT)->GetString())
                act(telep->GetItem(SCR_TROOMOUT)->GetString(), "К", ch);
            send_to_charf(ch, "\r\n");
            char_from_room(ch);
            char_to_room(ch, room);
            //перемещаем последователей очарок, ездовых, нежить, призваных
            for (f = ch->followers; f; f = f->next)
                if (f->type != FLW_GROUP && IS_NPC(f->follower)) {
                    char_from_room(f->follower);
                    char_to_room(f->follower, room);
                }
            check_horse(ch);

            look_at_room(ch, TRUE);
            if (telep->GetItem(SCR_TCHARIN)->GetString()) {
                send_to_charf(ch, "\r\n");
                act(telep->GetItem(SCR_TCHARIN)->GetString(), "М", ch);
            }
            if (telep->GetItem(SCR_TROOMIN)->GetString())
                act(telep->GetItem(SCR_TROOMIN)->GetString(), "К", ch);

            event_mob(ch, NULL, EVNT_ENTER, -1);

            break;
        }
    }


    if (ch) {
        //ОПЫТ
        num = Script->GetItem(SCR_EXP)->GetNumberItem();
        for (i = 0; i < num; i++) {
            CItem *exp = Script->GetItem(SCR_EXP)->GetItem(i);
            int ex = 0;

            if (exp->GetItem(SCR_EXP_SGROUP)->GetInt()) {
                send_to_charf(ch, "ГОДНОРОЗОВЫЙ: Нереализовано до востребования.\r\n");
            } else
                if ((ex = exp->GetItem(SCR_EXP_SCHAR)->GetInt())
                    && !get_exp_script_vnum(ch, vnum)) {
                main_gexp(ch, ex, 0, vnum, FALSE);
            }
            if ((ex = exp->GetItem(SCR_EXP_CHAR)->GetInt())) {
                main_gexp(ch, ex, 0, vnum, FALSE);
            }
            if ((ex = exp->GetItem(SCR_EXP_GROUP)->GetInt())) {
                send_to_charf(ch, "ГРУППА: Нереализовано до востребования.\r\n");
            }
        }
    }
//ПРОДОЛЖЕНИЕ
    num = Script->GetItem(SCR_CONTINUE)->GetNumberItem();
    for (i = 0; i < num; i++) {
        CItem *next = Script->GetItem(SCR_CONTINUE)->GetItem(i);
        int scr_vnum = next->GetItem(SCR_CNT_VNUM)->GetInt();
        int timer = next->GetItem(SCR_CNT_TIMER)->GetInt();
        struct event_param_data params;

        init_event_param(&params);
        params.actor = ch;
        params.stopflag = STOP_NONE;

        add_event(timer, scr_vnum, NULL, &params);
    }
//log("CКРИПТ.КОНЕЦ");
}



void scr_damage(struct char_data *ch, CItem * dam, int vnum)
{
    struct P_damage damage;
    struct P_message message;
    int t[2], type = A_POWER;

    damage.valid = true;
    damage.type = dam->GetItem(SCR_DAM_TYPE)->GetInt();
    damage.power = 0;
    if (dam->GetItem(SCR_DAM_DAMAGE)->GetString()) {
        if (sscanf(dam->GetItem(SCR_DAM_DAMAGE)->GetString(), "%d,%d", t, t + 1) == 2)
            damage.dam = number(t[0], t[1]);
        else
            damage.dam = 0;
    }
    damage.check_ac = A_POWER;
    damage.far_min = FALSE;
    damage.magic = TRUE;
    damage.armor = FALSE;       //броня учитывается

    message.valid = true;
    message.kChar = NULL;
    message.hChar = NULL;
    message.mChar = NULL;
    message.aChar = NULL;
    message.bChar = NULL;

    message.kVict = dam->GetItem(SCR_DAM_KVICT)->GetString();
    message.hVict = dam->GetItem(SCR_DAM_HVICT)->GetString();
    message.mVict = dam->GetItem(SCR_DAM_MVICT)->GetString();
    message.aVict = dam->GetItem(SCR_DAM_MVICT)->GetString();
    message.bVict = dam->GetItem(SCR_DAM_MVICT)->GetString();

    message.kRoom = dam->GetItem(SCR_DAM_KROOM)->GetString();
    message.hRoom = dam->GetItem(SCR_DAM_HROOM)->GetString();
    message.mRoom = dam->GetItem(SCR_DAM_MROOM)->GetString();
    message.aRoom = dam->GetItem(SCR_DAM_MROOM)->GetString();
    message.bRoom = dam->GetItem(SCR_DAM_MROOM)->GetString();

    if (dam->GetItem(SCR_DAM_SAVE)->GetInt()) {
        int save = dam->GetItem(SCR_DAM_SAVE)->GetInt(),
            stype = dam->GetItem(SCR_DAM_STYPE)->GetInt();
        if (general_savingthrow_3(ch, stype, save))
            type = N_POWER;
    }

    if ((_damage(ch, ch, 0, 0, type, FALSE, damage, message) == RD_KILL)) {
        char buf[MAX_STRING_LENGTH];

        sprintf(buf, "%s '%s' умер%s от действия #%d.", IS_MOB(ch) ? "Моб" : "Игрок",
                GET_NAME(ch), GET_CH_SUF_4(ch), vnum);
        mudlog(buf, CMP, LVL_GOD, TRUE);
    }


}

const char *check_saved_var(struct char_data *ch, const char *key)
{
    return ch->saved_vars.check(key);
}

void pulse_saved_var(int pulse)
{
    struct descriptor_data *d;

    for (d = descriptor_list; d; d = d->next) {
        if (STATE(d) != CON_PLAYING)
            continue;

        d->character->saved_vars.pulse(pulse);
    }

}

void add_saved_var(struct char_data *ch, char *key, char *value, int time)
{
    ch->saved_vars.add(key, value, time, false);
    set_var_quest(ch, key, value);
}

int del_saved_var(struct char_data *ch, char *key)
{
    return ch->saved_vars.erase(key) >= 1;
}

/*
 * var_storage methods
 */
const char *vars_storage::check(const char *key)
{
    const_iterator v = find(key);

    if (v == end())
        return NULL;

    return v->second.value.c_str();
}

void vars_storage::pulse(int pulse)
{
    vars_storage::iterator v;

    // для невечных переменных: уменьшаем значение таймера и удаляем если таймер 0
    for (v = begin(); v != end();) {
        if (v->second.time == 0) {
            erase(v++);
            continue;
        }

        if (v->second.time >= 1)
            v->second.time--;

        ++v;
    }
}

void vars_storage::add(const char *key, const char *value, int time, bool override)
{
    iterator v = find(key);

    // переменная с таким именем уже была
    if (v != end()) {
        // вне режима override, если и старое и новое значение - числа, запишем их сумму 
        if (!override && is_positive_number(value)
            && is_positive_number(v->second.value.c_str())) {
            int result = atoi(value) + atoi(v->second.value.c_str());

            v->second.value = result;
        }
        // сохраняем новое как строку
        else {
            v->second.value = value;
        }

        v->second.time = (time ? time : -1);
    } else {
        // заполняем новую переменную
        struct vars_data vars;

        vars.value = value;
        vars.time = (time ? time : -1);
        (*this)[key] = vars;
    }
}

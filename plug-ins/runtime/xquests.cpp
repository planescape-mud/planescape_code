#include "sysdep.h"

#include "wrapperbase.h"
#include "register-impl.h"

#include "structs.h"
#include "comm.h"
#include "interpreter.h"
#include "db.h"
#include "constants.h"
#include "utils.h"
#include "handler.h"
#include "xquests.h"
#include "xboot.h"
#include "case.h"
#include "parser_id.h"
#include "strlib.h"
#include "expr.h"


/**
 * Получить виртуальный номер квеста из внума моба и номера квеста.
 * Виртуальный номер используется как уникальный идентификатор 
 * при сохранении инфы о выполненных квестах на персонаже.
 */
static int quest_vnum(int vnum, int number)
{
    char qbuf[16];
    sprintf(qbuf, "%d%d", vnum, number);
    return atoi(qbuf);
}

/**
 * Вывод игроку ch списка квестов и меню действий, доступных рядом с мобов victim.
 * Вызывается из команды listen и из одного из пунктов меню.
 */
int go_show_quests(struct char_data *ch, struct char_data *victim)
{
    struct mob_quest_data *quests = NULL;
    struct mob_qscript_data *qscript = NULL;
    int vnum = 0, acount = 0, ncount = 0, dcount = 0, qcount = 0, mcount = 0, i;
    char aquests[20][MAX_STRING_LENGTH];
    char nquests[20][MAX_STRING_LENGTH];
    char dquests[20][MAX_STRING_LENGTH];
    char mquests[20][MAX_STRING_LENGTH];
    char qscrpt[20][MAX_STRING_LENGTH];
    char buf[MAX_EXTEND_LENGTH];

    // Запомнить с кем разговариваем и сбросить выбранный пункт меню.
    vnum = GET_MOB_VNUM(victim);
    ch->player.current_quest_mob = victim;
    ch->player.select_mode = -1;
    // Составление списка заданий.
    for (quests = victim->quests; quests; quests = quests->next) {
        int number = quests->number;
        int qvnum = quest_vnum(vnum, number);
        char *name = quests->name;

        //Проверка условия, доступен ли персонажу квест в принципе.
        if (!EXPR(quests->expr)->Expr(ch, victim, NULL))
            continue;

        // Проверка, выполнил ли он когда-либо этот квест.
        if (!get_quested(ch, qvnum)) {
            int flcomp = check_quest(ch, vnum, number);

            if (!flcomp) {
                // Запомнить доступные еще не сделанные задания.
                strcpy(aquests[acount], name);
                acount++;
            } else if (flcomp == 2) {
                // Запомнить выполненные задания.
                strcpy(dquests[dcount], name);
                dcount++;
            } else if (flcomp == 1) {
                // Запомнить текущие задания.
                strcpy(nquests[ncount], name);
                ncount++;
            }
        // Все равно показываем многоразовые квесты, даже если они выполнены.
        } else if (quests->multi) {
            // Запомнить многократные задания.
            strcpy(mquests[mcount], name);
            mcount++;
        }
    }

    // Запомнить меню возможных действий рядом с мобом.
    for (qscript = victim->qscripts; qscript; qscript = qscript->next) {
        char *name = qscript->text;

        if (!EXPR(qscript->expr)->Expr(ch, victim, NULL))
            continue;
        strcpy(qscrpt[qcount], name);
        qcount++;
    }

    // Вывод списка заданий и меню на экран.
    *buf = '\0';
    int l = 1;

    if (acount) {
        sprintf(buf + strlen(buf), "Доступные задания:\r\n");
        for (i = 0; i < acount; i++) {
            sprintf(buf + strlen(buf), "%d. %s\r\n", l, aquests[i]);
            l++;
        }
    }
    if (ncount) {
        sprintf(buf + strlen(buf), "Текущие задания:\r\n");
        for (i = 0; i < ncount; i++) {
            sprintf(buf + strlen(buf), "%d. %s\r\n", l, nquests[i]);
            l++;
        }
    }

    if (dcount) {
        if (l > 1)
            sprintf(buf + strlen(buf), "\r\n");
        sprintf(buf + strlen(buf), "Выполненые задания:\r\n");
        for (i = 0; i < dcount; i++) {
            sprintf(buf + strlen(buf), "%d. %s\r\n", l, dquests[i]);
            l++;
        }
    }

    if (mcount) {
        if (l > 1)
            sprintf(buf + strlen(buf), "\r\n");
        sprintf(buf + strlen(buf), "Повторные задания:\r\n");
        for (i = 0; i < mcount; i++) {
            sprintf(buf + strlen(buf), "%d. %s\r\n", l, mquests[i]);
            l++;
        }
    }

    if (qcount) {
        if (l > 1)
            sprintf(buf + strlen(buf), "\r\n");
        sprintf(buf + strlen(buf), "Действия:\r\n");
        for (i = 0; i < qcount; i++) {
            sprintf(buf + strlen(buf), "%d. %s\r\n", l, qscrpt[i]);
            l++;
        }
    }

    // Вывести приветствие или прощание моба.
    if (victim->welcome && l > 1) {
        char buf1[MAX_EXTEND_LENGTH];

        strcpy(buf1, strbraker(victim->welcome, ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)));
        act(buf1, "Мм", ch, victim);
        send_to_char(buf, ch);
        return (TRUE);
    } else if (victim->goodbye) {
        char buf1[MAX_EXTEND_LENGTH];

        strcpy(buf1, strbraker(victim->goodbye, ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)));
        act(buf1, "Мм", ch, victim);
        return (TRUE);
    }

    return (FALSE);
}


void go_show_quest(struct char_data *ch, struct char_data *victim, int pos, int number)
{
    struct mob_quest_data *quests = NULL;
    struct mob_qscript_data *qscript = NULL;
    int vnum = 0, i = 0, j = 0;
    bool found = FALSE;

    vnum = GET_MOB_VNUM(victim);

    if (ch->player.select_mode <= -1) {
        for (quests = victim->quests; quests; quests = quests->next) {
            int qvnum = quest_vnum(vnum, quests->number);

            //Проверка условия
            if (!EXPR(quests->expr)->Expr(ch, victim, NULL)
                || (get_quested(ch, qvnum) && !quests->multi))
                continue;
            i++;
            if ((pos && i == pos) || (number && quests->number == number)) {
                send_to_charf(ch, "&C\"%s\"&n\r\n&K%s&n\r\n\r\n&m", quests->name, quests->pretext);

                for (j = 0; j < (int) quests->objects.size(); j++) {
                    int ocount =
                        get_obj_quest(ch, quests->number, vnum, quests->objects[j].no,
                                      quests->multi);
                    int ncount = quests->objects[j].count;
                    int vobj = real_object(quests->objects[j].no);

                    if (vobj != -1)
                        send_to_charf(ch, "%s (%d/%d)%s\r\n",
                                      CAP(get_name_pad(obj_proto[vobj].names, PAD_IMN, PAD_OBJECT)),
                                      ocount, ncount, (ocount >= ncount) ? " выполнено" : "");
                }

                for (j = 0; j < (int) quests->mobiles.size(); j++) {
                    int ocount = get_mob_quest(ch, quests->number, vnum, quests->mobiles[j].no);
                    int ncount = quests->mobiles[j].count;
                    int vmob = real_mobile(quests->mobiles[j].no);

                    if (vmob != -1)
                        send_to_charf(ch, "%s (%d/%d)%s\r\n",
                                      CAP(get_name_pad(mob_proto[vmob].player.names, PAD_IMN)),
                                      ocount, ncount, (ocount >= ncount) ? " выполнено" : "");
                }

                for (j = 0; j < (int) quests->followers.size(); j++) {
                    int ocount = 0;
                    int vflw = real_mobile(quests->followers[j].no);

                    if (vflw != -1)
                        send_to_charf(ch, "%s (%d/%d)\r\n",
                                      CAP(get_name_pad(mob_proto[vflw].player.names, PAD_IMN)),
                                      ocount, quests->followers[j].count);
                }

                for (j = 0; j < (int) quests->req_var.size(); j++) {
                    const char *ocount =
                        get_var_quest(ch, quests->number, vnum, quests->req_var[j].name.c_str());
                    const char *ncount = quests->req_var[j].value.c_str();

                    send_to_charf(ch, "%s (%s/%s)%s\r\n", quests->req_var[j].title.c_str(),
                                  ocount ? ocount : "0", ncount, (ocount
                                                                  && !strncmp(ocount, ncount,
                                                                              strlen(ocount)) ?
                                                                  " выполнено" : ""));
                }

                send_to_charf(ch, "\r\n&K%s&n\r\n",
                              strbraker(quests->text, ch->sw, PRF_FLAGGED(ch, PRF_AUTOFRM)));
                int flcomp = check_quest(ch, vnum, quests->number);

                if (ch->player.select_mode == -1 && get_quested(ch, qvnum) && quests->multi) {
                    if (check_mutli_quest(ch, victim, vnum, quests->number)) {
                        send_to_charf(ch, "1. Получить награду.\r\n");
                        ch->player.select_mode = quests->number;
                    }
                } else if (ch->player.select_mode == -1 && !flcomp) {
                    send_to_charf(ch, "1. Принять\r\n9. Отказаться\r\n");
                    ch->player.select_mode = quests->number;
                } else if (ch->player.select_mode == -1 && flcomp == 2) {
                    send_to_charf(ch, "1. Получить награду.\r\n");
                    ch->player.select_mode = quests->number;
                }
                found = TRUE;
            }
        }
        for (qscript = victim->qscripts; qscript; qscript = qscript->next) {
            if (!EXPR(qscript->expr)->Expr(ch, victim, NULL))
                continue;
            i++;
            if (i == pos) {
                go_script(qscript->script, ch, victim);
                found = TRUE;
            }
        }
    } else if (ch->player.select_mode > 0) {
        i = 0;
        switch (pos) {
            case 1:
                for (quests = victim->quests; quests; quests = quests->next) {
                    //Проверка условия
                    if (!EXPR(quests->expr)->Expr(ch, victim, NULL))
                        continue;
                    //i++;
                    //if (i == pos)
                    if (quests->number == ch->player.select_mode) {
                        if (check_quest(ch, vnum, quests->number) == 2
                            || check_mutli_quest(ch, victim, vnum, quests->number)) {
                            send_to_charf(ch, "Вы получили награду!!!\r\n");
                            go_complite_quest(ch, vnum, quests->number);
                        } else {
                            send_to_charf(ch, "Вы получили новое задание '%s'.\r\n", quests->name);
                            ch->player.current_quest_mob = NULL;
                            ch->player.select_mode = 0;
                            if (!add_quest(ch, victim, pos))
                                send_to_charf(ch,
                                              "Ошибка получения задания. Сообщите имморталам.\r\n");
                            go_script(quests->accept, ch, victim);
                        }
                        found = TRUE;
                        break;
                    }
                }
                break;
            case 9:
                go_show_quests(ch, victim);
                found = TRUE;
                break;
        }
    }

    if (!found)
        send_to_charf(ch, "Не действительный пункт меню!\r\n");

}


ACMD(do_select)
{

    if (!argument || !*argument) {
        send_to_charf(ch, "Какой пункт выбрать?\r\n");
        return;
    }

    skip_spaces(&argument);

    if (ch->player.current_quest_mob == NULL) {
        send_to_charf(ch, "Вы ни с кем не общаетесь.\r\n");
        return;
    }

    if (!atoi(argument)) {
        send_to_charf(ch, "Введите число!\r\n");
        return;
    }

    go_show_quest(ch, ch->player.current_quest_mob, atoi(argument), 0);
}

static bool mprog_listen(struct char_data *talker, struct char_data *listener)
{
    FENIA_BOOL_CALL(talker, "Listen", "C", listener);
    FENIA_PROTO_BOOL_CALL(talker->npc(), "Listen", "CC", talker, listener);
    return false;
}


ACMD(do_listen)
{
    struct char_data *victim = NULL;
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);
    argument = one_argument(argument, arg);
    skip_spaces(&argument);


    if (!*arg) {
        send_to_charf(ch, "Кого Вы хотите послушать?\r\n");
        return;
    }

    if (!(victim = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        send_to_charf(ch, "Вы не видите здесь '%s'.\r\n", arg);
        return;
    }

    act("$n приготовил$u послушать историю.", TRUE, ch, 0, 0, TO_ROOM);

    bool fr = mprog_listen(victim, ch);
    int r = go_show_quests(ch, victim);

    if (!r && !fr)
        act("$N ничего Вам не рассказал$G.", FALSE, ch, 0, victim, TO_CHAR);
}

/**
 * Добавляет игроку ch задание под номером number, полученное от моба с внумом vnum.
 * Возвращает false при неудаче, true если квест добавлен.
 */
int add_quest(struct char_data *ch, int vnum, int number)
{
    int rnum = real_mobile(vnum);

    if (rnum == -1)
        return 0;

    struct char_data *victim = mob_proto + rnum;
    return add_quest(ch, victim, number);
}

/**
 * Добавляет игроку ch задание под номером number, полученное от моба victim.
 * Возвращает false при неудаче, true если квест добавлен.
 */
int add_quest(struct char_data *ch, struct char_data *victim, int number)
{
    struct mob_quest_data *quests = NULL;
    struct quest_list_data *q;
    int j, i = 0, lastquest = ch->last_quest;

    if (victim == NULL || ch == NULL)
        return (FALSE);

    if (lastquest > MAX_QUESTS)
        return (0);

    for (quests = victim->quests; quests; quests = quests->next) {
        // Проверяем, доступно ли это задание в принципе.
        if (!EXPR(quests->expr)->Expr(ch, victim, NULL))
            continue;
        i++;
        // Задание найдено среди списка квестов этого квестора.
        if (i == number) {
            //send_to_charf(ch,"Добавляю задание с номером %d\r\n",quests->number);
            //CREATE(q,struct quest_list_data,1);
            q = &ch->quest_list[lastquest];
            q->mob_vnum = GET_MOB_VNUM(victim);
            q->number = quests->number;
            for (j = 0; j < (int) quests->mobiles.size(); j++) {
                struct bug_abuse_data done_tmp;

                done_tmp.no = quests->mobiles[j].no;
                done_tmp.owner = quests->mobiles[j].count;
                done_tmp.count = 0;
                q->done_mobiles.push_back(done_tmp);
            }

            for (j = 0; j < (int) quests->objects.size(); j++) {
                struct bug_abuse_data done_tmp;

                done_tmp.no = quests->objects[j].no;
                done_tmp.owner = quests->objects[j].count;
                done_tmp.count = 0;
                q->done_objects.push_back(done_tmp);
            }

            for (j = 0; j < (int) quests->followers.size(); j++) {
                struct bug_abuse_data done_tmp;

                done_tmp.no = quests->followers[j].no;
                done_tmp.owner = quests->followers[j].count;
                done_tmp.count = 0;
                q->done_followers.push_back(done_tmp);
            }

            for (j = 0; j < (int) quests->req_var.size(); j++) {
                struct req_var_data done_tmp;

                done_tmp.title = quests->req_var[j].title;
                done_tmp.name = quests->req_var[j].name;
                done_tmp.value = quests->req_var[j].value;
                done_tmp.current = "";
                q->done_req_var.push_back(done_tmp);
            }
            ch->last_quest++;
        }
    }
    return (TRUE);
}

//Возвращает 0 если нет задания, 1 в процессе выполнения, 2 заверщеное
int get_current_quest(struct char_data *ch, int qvnumber)
{
    struct quest_list_data *q;
    int qvnum = 0, i;

    for (i = 0; i < MAX_QUESTS; i++) {
        q = &ch->quest_list[i];
        if (!q->mob_vnum && !q->number)
            continue;
        qvnum = quest_vnum(q->mob_vnum, q->number);
        if (qvnum == qvnumber) {
            if (q->complite)
                return (2);
            else
                return (1);
        }
    }

    return (0);
}

ACMD(do_quests)
{
    struct quest_list_data *q;
    struct mob_quest_data *quests = NULL;
    int acount = 0, i = 0, j;
    char aquests[20][MAX_STRING_LENGTH];
    char arg[MAX_STRING_LENGTH];

    skip_spaces(&argument);

    if (!argument || !*argument) {
        for (j = 0; j < MAX_QUESTS; j++) {
            q = &ch->quest_list[j];
            if (!q->mob_vnum && !q->number)
                continue;
            int rnum = real_mobile(q->mob_vnum);

            for (quests = mob_proto[rnum].quests; quests; quests = quests->next) {
                if (q->number == quests->number) {
                    sprintf(aquests[acount], "%s%s", quests->name,
                            q->complite ? " (выполнено)" : "");
                    acount++;
                }
            }
        }

        if (acount) {
            send_to_charf(ch, "Текущие задания:\r\n");
            for (i = 0; i < acount; i++)
                send_to_charf(ch, "%d. %s\r\n", i + 1, aquests[i]);
        } else
            send_to_charf(ch, "У Вас нет заданий.\r\n");
        return;
    }

    argument = one_argument(argument, arg);

    skip_spaces(&argument);

    if (!is_positive_number(arg)) {
        send_to_charf(ch, "Укажите номер задания.\r\n");
        return;
    } else {
        int num = atoi(arg);

        for (j = 0; j < MAX_QUESTS; j++) {
            q = &ch->quest_list[j];
            if (!q->mob_vnum && !q->number)
                continue;
            i++;
            if (i == num) {
                int rnum = real_mobile(q->mob_vnum);

                ch->player.select_mode = -2;
                if (!strcmp("удалить", argument)) {
                    send_to_charf(ch, "Вы отказались от задания.\r\n");
                    del_quest(ch, q->mob_vnum, q->number);
                    ch->player.select_mode = 0;
                    ch->player.current_quest_mob = NULL;
                    return;
                } else {
                    go_show_quest(ch, mob_proto + rnum, 0, q->number);
                    ch->player.select_mode = 0;
                }
                return;
            }
        }
    }

}

void set_mob_quest(struct char_data *ch, int vnum)
{
    struct quest_list_data *qlist;
    int i, j;

    for (j = 0; j < MAX_QUESTS; j++) {
        qlist = &ch->quest_list[j];
        if (!qlist->mob_vnum && !qlist->number)
            continue;
        for (i = 0; i < (int) qlist->done_mobiles.size(); i++)
            if (vnum == qlist->done_mobiles[i].no &&
                qlist->done_mobiles[i].count < qlist->done_mobiles[i].owner) {
                qlist->done_mobiles[i].count++;
                if (qlist->done_mobiles[i].count == qlist->done_mobiles[i].owner)
                    check_quest_complite(ch, qlist->mob_vnum, qlist->number);
            }
    }
}

void set_obj_quest(struct char_data *ch, int vnum)
{
    struct quest_list_data *qlist;
    int i, j;

    for (j = 0; j < MAX_QUESTS; j++) {
        qlist = &ch->quest_list[j];
        if (!qlist->mob_vnum && !qlist->number)
            continue;
        for (i = 0; i < (int) qlist->done_objects.size(); i++) {
            if (vnum == qlist->done_objects[i].no &&
                qlist->done_objects[i].count < qlist->done_objects[i].owner) {
                qlist->done_objects[i].count++;
                if (qlist->done_objects[i].count == qlist->done_objects[i].owner)
                    check_quest_complite(ch, qlist->mob_vnum, qlist->number);
                return;         //Добавляем только один предмет к одному квесту
            }
        }
    }
}

void unset_obj_quest(struct char_data *ch, int vnum)
{
    struct quest_list_data *qlist;
    int i, j;

    for (j = 0; j < MAX_QUESTS; j++) {
        qlist = &ch->quest_list[j];
        if (!qlist->mob_vnum && !qlist->number)
            continue;

        for (i = 0; i < (int) qlist->done_objects.size(); i++)
            if (vnum == qlist->done_objects[i].no && qlist->done_objects[i].count) {
                qlist->done_objects[i].count--;
                if (qlist->done_objects[i].count < qlist->done_objects[i].owner)
                    check_quest_complite(ch, qlist->mob_vnum, qlist->number);
            }
    }
}

void set_flw_quest(struct char_data *ch, int vnum)
{
    struct quest_list_data *qlist;
    int i, j;

    for (j = 0; j < MAX_QUESTS; j++) {
        qlist = &ch->quest_list[j];
        if (!qlist->mob_vnum && !qlist->number)
            continue;

        for (i = 0; i < (int) qlist->done_followers.size(); i++) {
            if (vnum == qlist->done_followers[i].no &&
                qlist->done_followers[i].count < qlist->done_followers[i].owner) {
                qlist->done_followers[i].count++;
                if (qlist->done_followers[i].count == qlist->done_followers[i].owner)
                    check_quest_complite(ch, qlist->mob_vnum, qlist->number);
                return;         //Добавляем только один предмет к одному квесту
            }
        }
    }
}

void unset_flw_quest(struct char_data *ch, int vnum)
{
    struct quest_list_data *qlist;
    int i, j;

    for (j = 0; j < MAX_QUESTS; j++) {
        qlist = &ch->quest_list[j];
        if (!qlist->mob_vnum && !qlist->number)
            continue;

        for (i = 0; i < (int) qlist->done_followers.size(); i++)
            if (vnum == qlist->done_followers[i].no && qlist->done_followers[i].count) {
                qlist->done_followers[i].count--;
                if (qlist->done_followers[i].count < qlist->done_followers[i].owner)
                    check_quest_complite(ch, qlist->mob_vnum, qlist->number);
            }
    }
}

void set_var_quest(struct char_data *ch, char *name, char *value)
{
    struct quest_list_data *qlist;
    int i, j;

    for (j = 0; j < MAX_QUESTS; j++) {
        qlist = &ch->quest_list[j];
        if (!qlist->mob_vnum && !qlist->number)
            continue;
        for (i = 0; i < (int) qlist->done_req_var.size(); i++) {
            if (qlist->done_req_var[i].name == name) {
                qlist->done_req_var[i].current = value;
                if (qlist->done_req_var[i].value == value)
                    check_quest_complite(ch, qlist->mob_vnum, qlist->number);
            }
        }
    }
}

int get_mob_quest(struct char_data *ch, int number, int mob_vnum, int vnum)
{
    struct quest_list_data *qlist;
    int i, j;

    for (j = 0; j < MAX_QUESTS; j++) {
        qlist = &ch->quest_list[j];
        if (!qlist->mob_vnum && !qlist->number)
            continue;

        if (qlist->number == number && qlist->mob_vnum == mob_vnum)
            for (i = 0; i < (int) qlist->done_mobiles.size(); i++)
                if (vnum == qlist->done_mobiles[i].no)
                    return (qlist->done_mobiles[i].count);
    }
    return (0);
}

int get_obj_quest(struct char_data *ch, int number, int mob_vnum, int vnum, int multi)
{
    struct quest_list_data *qlist;
    int i, j;

    int qvnum = quest_vnum(mob_vnum, number);

    if (get_quested(ch, qvnum) && multi) {
        int rnum = real_mobile(mob_vnum);

        if (rnum != -1) {
            struct char_data *victim = mob_proto + rnum;
            struct mob_quest_data *quests = NULL;

            for (quests = victim->quests; quests; quests = quests->next) {
                for (j = 0; j < (int) quests->objects.size(); j++)
                    if (vnum == quests->objects[j].no)
                        return focn(ch, vnum);
            }
        }
    } else {
        for (j = 0; j < MAX_QUESTS; j++) {
            qlist = &ch->quest_list[j];
            if (!qlist->mob_vnum && !qlist->number)
                continue;
            if (qlist->number == number && qlist->mob_vnum == mob_vnum)
                for (i = 0; i < (int) qlist->done_objects.size(); i++)
                    if (vnum == qlist->done_objects[i].no)
                        return (qlist->done_objects[i].count);
        }
    }
    return (0);
}

int get_flw_quest(struct char_data *ch, int number, int mob_vnum, int vnum)
{
    struct quest_list_data *qlist;
    int i, j;

    for (j = 0; j < MAX_QUESTS; j++) {
        qlist = &ch->quest_list[j];
        if (!qlist->mob_vnum && !qlist->number)
            continue;
        if (qlist->number == number && qlist->mob_vnum == mob_vnum)
            for (i = 0; i < (int) qlist->done_followers.size(); i++)
                if (vnum == qlist->done_followers[i].no)
                    return (qlist->done_followers[i].count);
    }
    return (0);
}

const char *get_var_quest(struct char_data *ch, int number, int vnum, const char *name)
{
    struct quest_list_data *qlist;
    int i, j;

    for (j = 0; j < MAX_QUESTS; j++) {
        qlist = &ch->quest_list[j];
        if (!qlist->mob_vnum && !qlist->number)
            continue;
        if (qlist->number == number && (int) qlist->done_req_var.size()) {
            for (i = 0; i < (int) qlist->done_req_var.size(); i++) {
                if (qlist->done_req_var[i].name == name)
                    return (qlist->done_req_var[i].current.c_str());
            }
        }
    }
    return (NULL);
}

/**
 * Проверка состояния квеста по номеру number, взятого у моба vnum:
 * 0 - не брали квест
 * 1 - квест взят, но еще не закончен
 * 2 - квест выполнен
 */
int check_quest(struct char_data *ch, int vnum, int number)
{
    struct quest_list_data *qlist;
    int j;

    for (j = 0; j < MAX_QUESTS; j++) {
        qlist = &ch->quest_list[j];
        if (!qlist->mob_vnum && !qlist->number)
            continue;
        if (qlist->number == number && qlist->mob_vnum == vnum) {
            if (qlist->complite)
                return (2);
            else
                return (1);
        }
    }
    return (FALSE);
}

/**
 * Проверка состояния квеста с псевдономером qrnum, полученным конкатенацией
 * vnum-а квестового моба и номера квеста у этого моба.
 * 0 - не брали квест
 * 1 - квест взят, но еще не закончен
 * 2 - квест выполнен
 */
int check_quest(struct char_data *ch, int qrnum)
{
    struct quest_list_data *qlist;
    int j;

    for (j = 0; j < MAX_QUESTS; j++) {
        qlist = &ch->quest_list[j];
        if (!qlist->mob_vnum && !qlist->number)
            continue;

        int qvnum = quest_vnum(qlist->mob_vnum, qlist->number);

        if (qrnum == qvnum) {
            if (qlist->complite)
                return (2);
            else
                return (1);
        }
    }
    return (FALSE);
}

int check_mutli_quest(struct char_data *ch, struct char_data *victim, int vnum, int number)
{
    struct mob_quest_data *quests = NULL;
    bool nm = FALSE, no = FALSE, nf = FALSE, nv = FALSE;
    int j;

    for (quests = victim->quests; quests; quests = quests->next)
        if (quests->number == number) {
            if (!quests->mobiles.size())
                nm = TRUE;
            if (!quests->objects.size())
                no = TRUE;
            if (!quests->followers.size())
                nf = TRUE;
            if (!quests->req_var.size())
                nv = TRUE;
            for (j = 0; j < (int) quests->objects.size(); j++) {
                int ocount =
                    get_obj_quest(ch, quests->number, vnum, quests->objects[j].no, quests->multi);
                int ncount = quests->objects[j].count;

                if (ocount >= ncount)
                    no = TRUE;
                else
                    no = FALSE;
            }

        }

    return (nm && no && nf && nv);
}

void check_quest_complite(struct char_data *ch, int vnum, int number)
{
    struct quest_list_data *qlist;
    struct mob_quest_data *quests = NULL;
    struct char_data *victim = NULL;
    int j = 0, i, rnum;
    bool nm = FALSE, no = FALSE, nf = FALSE, nv = FALSE;

    rnum = real_mobile(vnum);

    if (rnum == -1)
        return;

    victim = mob_proto + rnum;
    for (quests = victim->quests; quests; quests = quests->next)
        if (quests->number == number) {
            if (!quests->mobiles.size())
                nm = TRUE;
            if (!quests->objects.size())
                no = TRUE;
            if (!quests->followers.size())
                nf = TRUE;
            if (!quests->req_var.size())
                nv = TRUE;
            for (j = 0; j < (int) quests->mobiles.size(); j++) {
                int ocount = get_mob_quest(ch, quests->number, vnum, quests->mobiles[j].no);
                int ncount = quests->mobiles[j].count;

                if (ocount == ncount)
                    nm = TRUE;
                else
                    nm = FALSE;
            }
            for (j = 0; j < (int) quests->objects.size(); j++) {
                int ocount =
                    get_obj_quest(ch, quests->number, vnum, quests->objects[j].no, quests->multi);
                int ncount = quests->objects[j].count;

                if (ocount == ncount)
                    no = TRUE;
                else
                    no = FALSE;
            }
            for (j = 0; j < (int) quests->followers.size(); j++) {
                int ocount = get_flw_quest(ch, quests->number, vnum, quests->followers[j].no);
                int ncount = quests->followers[j].count;

                if (ocount == ncount)
                    nf = TRUE;
                else
                    nf = FALSE;
            }
            for (j = 0; j < (int) quests->req_var.size(); j++) {
                const char *ocount =
                    get_var_quest(ch, quests->number, vnum, quests->req_var[j].name.c_str());
                const char *ncount = quests->req_var[j].value.c_str();

                if (ocount && !strncmp(ocount, ncount, strlen(ocount)))
                    nv = TRUE;
                else
                    nv = FALSE;
            }
            if (nm && no && nf && nv) {
                for (i = 0; i < MAX_QUESTS; i++) {
                    qlist = &ch->quest_list[i];
                    if (!qlist->mob_vnum && !qlist->number)
                        continue;
                    if (qlist->number == number && qlist->mob_vnum == vnum)
                        qlist->complite = TRUE;
                }
            } else {
                for (i = 0; i < MAX_QUESTS; i++) {
                    qlist = &ch->quest_list[i];
                    if (!qlist->mob_vnum && !qlist->number)
                        continue;
                    if (qlist->number == number && qlist->mob_vnum == vnum)
                        qlist->complite = FALSE;
                }
            }
        }
}

void del_quest(struct char_data *ch, int mob_vnum, int number)
{
    struct quest_list_data *q = NULL;
    int i;

    if (ch == NULL)
        return;

    for (i = 0; i < MAX_QUESTS; i++) {
        q = &ch->quest_list[i];
        if (!q->mob_vnum && !q->number)
            continue;

        if (q->mob_vnum == mob_vnum && q->number == number) {
            q->done_mobiles.erase(q->done_mobiles.begin(), q->done_mobiles.end());
            q->done_objects.erase(q->done_objects.begin(), q->done_objects.end());
            q->done_followers.erase(q->done_followers.begin(), q->done_followers.end());
            q->done_req_var.erase(q->done_req_var.begin(), q->done_req_var.end());
            q->mob_vnum = 0;
            q->number = 0;
        }
    }

}


///////////////////////////////////////////////////////////////////////////////
void save_quests(struct char_data *ch)
{
    CQstSave quests;
    struct quest_list_data *q = NULL;
    char fname[256];
    int i, j, n;
    bool needsave = FALSE;

    if (ch == NULL)
        return;

    if (IS_NPC(ch))
        return;

    if (!quests.Initialization())
        return;

    get_filename(GET_NAME(ch), fname, QUEST_FILE);

    for (n = 0; n < MAX_QUESTS; n++) {
        q = &ch->quest_list[n];
        if (!q->mob_vnum && !q->number)
            continue;

        i = quests.NewSubItem();
        CItem *xq = quests.GetItem(i);

        xq->GetItem(QUEST_QUEST)->SetParam(i);
        xq->GetItem(QUEST_MOB_VNUM)->SetParam(q->mob_vnum);
        xq->GetItem(QUEST_NUMBER)->SetParam(q->number);
        needsave = TRUE;
        for (j = 0; j < (int) q->done_mobiles.size(); j++)
            xq->GetItem(QUEST_DONE_MOB)->AddParam(q->done_mobiles[j].no, q->done_mobiles[j].count);
        for (j = 0; j < (int) q->done_objects.size(); j++)
            xq->GetItem(QUEST_DONE_OBJ)->AddParam(q->done_objects[j].no, q->done_objects[j].count);
        for (j = 0; j < (int) q->done_followers.size(); j++)
            xq->GetItem(QUEST_DONE_FLW)->AddParam(q->done_followers[j].no,
                                                  q->done_followers[j].count);
        for (j = 0; j < (int) q->done_req_var.size(); j++) {
            if (q->done_req_var[j].current.size() == 0)
                continue;
            int iv = xq->GetItem(QUEST_VAR)->NewSubItem();
            CItem *xvar = xq->GetItem(QUEST_VAR)->GetItem(iv);

            xvar->GetItem(QUEST_VAR_NAME)->SetParam(q->done_req_var[j].name.c_str());
            xvar->GetItem(QUEST_VAR_CURRENT)->SetParam(q->done_req_var[j].current.c_str());
        }
    }

    if (needsave)
        quests.SaveAll(fname, TRUE);
    else
        unlink(fname);

}


void load_quests(struct char_data *ch)
{
    CQstSave quests;
    struct quest_list_data *q = NULL;
    struct mob_quest_data *qt = NULL;
    struct char_data *victim = NULL;
    char fname[256];
    int i, number, j, rnum = -1, numadd, t[2], n;


    if (ch == NULL)
        return;

    if (IS_NPC(ch))
        return;

    if (!quests.Initialization())
        return;

    get_filename(GET_NAME(ch), fname, QUEST_FILE);

    if (!quests.ReadConfig(fname))
        return;

    number = quests.GetNumberItem();
    for (i = 0; i < number; i++) {
        CItem *xqst = quests.GetItem(i);

        rnum = real_mobile(xqst->GetItem(QUEST_MOB_VNUM)->GetInt());
        if (rnum == -1)
            continue;           //Неизвестный монстр

        victim = mob_proto + rnum;
        q = &ch->quest_list[i];
        ch->last_quest++;
        q->mob_vnum = xqst->GetItem(QUEST_MOB_VNUM)->GetInt();
        q->number = xqst->GetItem(QUEST_NUMBER)->GetInt();

        for (qt = victim->quests; qt; qt = qt->next) {
            if (qt->number == q->number) {
                numadd = xqst->GetItem(QUEST_DONE_MOB)->GetStrListNumber();
                for (n = 0; n < (int) qt->mobiles.size(); n++) {
                    struct bug_abuse_data done_tmp;

                    done_tmp.no = qt->mobiles[n].no;
                    done_tmp.owner = qt->mobiles[n].count;
                    for (j = 0; j < numadd; j++) {
                        xqst->GetItem(QUEST_DONE_MOB)->GetStrList(j, t[0], t[1]);
                        if (qt->mobiles[n].no == t[0])
                            done_tmp.count = t[1];
                        else
                            done_tmp.count = 0;
                    }
                    q->done_mobiles.push_back(done_tmp);
                }
                numadd = xqst->GetItem(QUEST_DONE_OBJ)->GetStrListNumber();
                for (n = 0; n < (int) qt->objects.size(); n++) {
                    struct bug_abuse_data done_tmp;

                    done_tmp.no = qt->objects[n].no;
                    done_tmp.owner = qt->objects[n].count;
                    for (j = 0; j < numadd; j++) {
                        xqst->GetItem(QUEST_DONE_OBJ)->GetStrList(j, t[0], t[1]);
                        if (qt->objects[n].no == t[0])
                            done_tmp.count = t[1];
                        else
                            done_tmp.count = 0;
                    }
                    q->done_objects.push_back(done_tmp);
                }
                numadd = xqst->GetItem(QUEST_VAR)->GetNumberItem();
                for (n = 0; n < (int) qt->req_var.size(); n++) {
                    struct req_var_data done_tmp;

                    done_tmp.title = qt->req_var[n].title;
                    done_tmp.name = qt->req_var[n].name;
                    done_tmp.value = qt->req_var[n].value;
                    done_tmp.current = "";

                    for (j = 0; j < numadd; j++) {
                        CItem *var = xqst->GetItem(QUEST_VAR)->GetItem(j);

                        if (done_tmp.name == var->GetItem(QUEST_VAR_NAME)->GetString() &&
                            var->GetItem(QUEST_VAR_CURRENT)->GetString()) {
                            done_tmp.current = var->GetItem(QUEST_VAR_CURRENT)->GetString();
                            //add = TRUE;
                        }
                    }
                    //if (add)
                    {
                        q->done_req_var.push_back(done_tmp);
                        //send_to_charf(ch,"Добавили усл.прем %s",done_tmp.name);
                    }
                }
                check_quest_complite(ch, q->mob_vnum, q->number);
            }
        }
    }
}

void go_complite_quest(struct char_data *ch, int vnum, int number)
{
    struct mob_quest_data *quests = NULL;
    struct char_data *victim = NULL;
    int rnum = -1, j, scrnum = 0;

    WAIT_STATE(ch, PULSE_VIOLENCE);

    rnum = real_mobile(vnum);

    if (rnum == -1)
        return;

    victim = mob_proto + rnum;

    for (quests = victim->quests; quests; quests = quests->next) {
        for (j = 0; j < (int) quests->objects.size(); j++) {
            int ovnum = quests->objects[j].no;
            int count = quests->objects[j].count;

            reci(ch, ovnum, count);
        }

        if (quests->number == number)
            scrnum = quests->done;
    }

    int qvnum = quest_vnum(vnum, number); 

    if (!get_quested(ch, qvnum)) {
        set_quested(ch, qvnum);
        del_quest(ch, vnum, number);
    }

    if (scrnum)
        go_script(scrnum, ch, victim);

}

/**
 * Запомнить, что персонаж выполнил квест number у моба vnum.
 */
void set_quested(struct char_data *ch, int vnum, int number)
{
    int qvnum = quest_vnum(vnum, number); 
    set_quested(ch, qvnum);
}

/**
 * Запомнить, что персонаж выполнил квест с виртуальным номером quest.
 */
void set_quested(struct char_data *ch, int quest)
{
    int i;

    if (IS_NPC(ch))
        return;
    if (ch->Questing.quests) {
        for (i = 0; i < ch->Questing.count; i++)
            if (*(ch->Questing.quests + i) == quest)
                return;
        if (!(ch->Questing.count % 10L))
            RECREATE(ch->Questing.quests, int, (ch->Questing.count / 10L + 1) * 10L);
    } else {
        ch->Questing.count = 0;
        CREATE(ch->Questing.quests, int, 10);
    }
    *(ch->Questing.quests + ch->Questing.count++) = quest;
}

/**
 * Вернуть true, если персонаж выполнил квест number у моба vnum. 
 */
int get_quested(struct char_data *ch, int vnum, int number)
{
    int qvnum = quest_vnum(vnum, number); 
    return get_quested(ch, qvnum);
}

/**
 * Вернуть true, если персонаж выполнил квест с виртуальным номером quest.
 */
int get_quested(struct char_data *ch, int quest)
{
    int i;

    if (IS_NPC(ch))
        return (FALSE);
    if (ch->Questing.quests) {
        for (i = 0; i < ch->Questing.count; i++)
            if (*(ch->Questing.quests + i) == quest)
                return (TRUE);
    }
    return (FALSE);
}

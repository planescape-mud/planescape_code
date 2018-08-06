/*****************************************************************************/
/*                                                                           */
/*****************************************************************************/

#include "sysdep.h"
#include "structs.h"
#include "comm.h"
#include "db.h"
#include "constants.h"
#include "utils.h"
#include "xsave.h"
#include "xspells.h"
#include "spells.h"
#include "xobj.h"
#include "xboot.h"
#include "handler.h"
#include "case.h"
#include "parser_id.h"
#include "feniamanager.h"


///////////////////////////////////////////////////////////////////////////////
void rent_extract_objs(struct obj_data *obj)
{
    int rnum;
    struct obj_data *next;

    for (; obj; obj = next) {
        next = obj->next_content;
        rent_extract_objs(obj->contains);
        if ((rnum = real_object(GET_OBJ_VNUM(obj))) >= 0 && GET_OBJ_TIMER(obj) >= 0) {
            obj_index[rnum].stored++;
        }
        extract_obj(obj);
    }
}

void xsave_rent(struct char_data *ch, int type, int extract)
{
    CRent rent;
    char fname[256], buf[512];
    int i, loadroom = NOWHERE;

    if (ch == NULL)
        return;

    if (IS_NPC(ch))
        return;


    if (!rent.Initialization())
        return;

    get_filename(GET_NAME(ch), fname, TIME_CRASH_FILE);

    log("ЗАПИСЬ: Персонаж [%s]", GET_NAME(ch));

    sprintf(buf, "Запись ренты персонажа %s", GET_NAME(ch));
    mudlog(buf, CMP, LVL_IMPL, TRUE);

    i = rent.NewSubItem();
    CItem *xrnt = rent.GetItem(i);

    xrnt->GetItem(RENT_NUMBER)->SetParam((int) GET_ID(ch));
    xrnt->GetItem(RENT_NAME)->SetParam(GET_NAME(ch));
    xrnt->GetItem(RENT_TYPE)->SetParam(type);
    xrnt->GetItem(RENT_TIME)->SetParam((int) time(0));

    loadroom = real_room(GET_LOADROOM(ch));

    if (loadroom != NOWHERE && world[loadroom].hotel) {
        xrnt->GetItem(RENT_ROOM)->SetParam(world[loadroom].number);
        if (world[loadroom].hotel->type)
            xrnt->GetItem(RENT_QUALITY)->SetParam(world[loadroom].hotel->type);
        else
            xrnt->GetItem(RENT_QUALITY)->SetParam(0);
    } else {
        xrnt->GetItem(RENT_QUALITY)->SetParam(0);
        xrnt->GetItem(RENT_ROOM)->SetParam(-1);
    }

    rent.SaveAll(fname, TRUE);

    /* {
       remove(fname);
       rename(fbackup,fname);
       } */
    save_char_xobj(ch);
//save_pets(ch);
    if (extract) {
        for (i = 0; i < NUM_WEARS; i++) {
            if (GET_EQ(ch, i))
                rent_extract_objs(GET_EQ(ch, i));
            if (GET_TATOO(ch, i))
                rent_extract_objs(GET_TATOO(ch, i));
        }
        rent_extract_objs(ch->carrying);
    }

}

void xload_rent(struct char_data *ch, int copyover)
{
    CRent Rent;
    char fname[256];
    char buf[MAX_STRING_LENGTH];
    int i, type_rent, type_hotel, cost, no_room, freerent = FALSE;
    int d, m, h;
    time_t mytime;
    float num_of_days;

    if (ch == NULL)
        return;

    if (!Rent.Initialization())
        return;

    get_filename(GET_NAME(ch), fname, TIME_CRASH_FILE);

    if (!Rent.ReadConfig(fname)) {
        log("ОШИБКА: Не могут открыть файл %s на чтение", fname);
        return;
    }

    if (copyover) {
        read_char_xobj(ch);
        return;
    }

    CItem *rent = Rent.GetItem(0);

    type_rent = rent->GetItem(RENT_TYPE)->GetInt();
    type_hotel = rent->GetItem(RENT_QUALITY)->GetInt();
    no_room = rent->GetItem(RENT_ROOM)->GetInt();


    num_of_days =
        (float) ((float) (time(0) - rent->GetItem(RENT_TIME)->GetInt()) /
                 (float) SECS_PER_REAL_DAY);
    mytime = time(0) - rent->GetItem(RENT_TIME)->GetInt();
    d = mytime / 86400;
    h = (mytime / 3600) % 24;
    m = (mytime / 60) % 60;

    if (type_rent == RENT_REBOOT &&
        rent->GetItem(RENT_TIME)->GetInt() + free_rebootrent_period * SECS_PER_REAL_HOUR > time(0))
        freerent = TRUE;

    if (type_rent == RENT_CRASH &&
        rent->GetItem(RENT_TIME)->GetInt() + free_crashrent_period * SECS_PER_REAL_HOUR > time(0))
        freerent = TRUE;


    send_to_charf(ch, "\r\nВы были на постое %d %s %d %s %d %s.\r\n", d, desc_count(d, WHAT_DAY), h,
                  desc_count(h, WHAT_HOUR), m, desc_count(m, WHAT_MINu));

    if (mytime / 3600 >= 6) {
        GET_HIT(ch) = GET_MAX_HIT(ch);
        GET_MANA(ch) = GET_MAX_MANA(ch);
    }

    if (mytime / 3600 >= 24) {
        send_to_charf(ch, "Вы как следует выспались и плотно перекусили.\r\n");
        if (GET_COND(ch, FULL) != -1)
            gain_condition(ch, FULL, thirst_race[(int) GET_RACE(ch)].max_cond * 6);
        if (GET_COND(ch, THIRST) != -1)
            gain_condition(ch, THIRST, full_race[(int) GET_RACE(ch)].max_cond * 6);
        if (GET_COND(ch, SLEEP) != -1)
            gain_condition(ch, SLEEP, sleep_race[(int) GET_RACE(ch)].max_cond * 6);
    }

    if (h)
        gain_condition(ch, SLEEP, (mytime / 3600) * SECS_PER_MUD_TICK);

    read_char_xobj(ch);

    if (IS_GOD(ch)) {
        send_to_charf(ch, "Постой бесмертным обходится бесплатно.\r\n");
        return;
    }


    bool confis = FALSE;

    if (num_of_days > 30) {     //Все предметы изымаем
        send_to_charf(ch,
                      "Вы отсутствовали в игре больше 30ти дней и все Ваши предметы были конфискованы.\r\n");

        for (i = 0; i < NUM_WEARS; i++) {
            if (GET_EQ(ch, i))
                rent_extract_objs(GET_EQ(ch, i));
        }
        rent_extract_objs(ch->carrying);

        num_of_days = 30.0;
        confis = TRUE;
    }

    if (freerent) {
        cost = 0;
        send_to_charf(ch, "Постой был бесплатным.\r\n");
    } else {
        cost = (int) ((float) num_of_days * (float) cost_hotel[type_hotel]);
        send_to_charf(ch, "Постой обощелся Вам в %d %s.\r\n", cost, desc_count(cost, WHAT_MONEYu));
    }

    if (GET_BANK_GOLD(ch) + GET_GOLD(ch) < cost) {
        if (!confis)
            send_to_charf(ch,
                          "У Вас не хватило денег на оплату постоя и все Ваши вещи были конфискованы.\r\n");

        for (i = 0; i < NUM_WEARS; i++) {
            if (GET_EQ(ch, i))
                rent_extract_objs(GET_EQ(ch, i));
        }
        rent_extract_objs(ch->carrying);
    } else {
        if (PRF_FLAGGED(ch, PRF_BANK_RENT)) {
            GET_BANK_GOLD(ch) -= MAX(cost - GET_GOLD(ch), 0);
            GET_GOLD(ch) = MAX(GET_GOLD(ch) - cost, 0);
        } else {
            GET_GOLD(ch) -= MAX(cost - GET_BANK_GOLD(ch), 0);
            GET_BANK_GOLD(ch) = MAX(GET_BANK_GOLD(ch) - cost, 0);
        }
        send_to_charf(ch, "После оплаты постоя у Вас осталось %d %s.\r\n",
                      GET_GOLD(ch) + GET_BANK_GOLD(ch), desc_count(GET_GOLD(ch) + GET_BANK_GOLD(ch),
                                                                   WHAT_MONEYa));
    }


    if (copyover)
        sprintf(buf, "%s восстановил%s соединение после copyover.", GET_NAME(ch), GET_CH_SUF_1(ch));
    else {
        sprintf(buf, "%s вош%s в игру с ренты #%d, тип #%d, локация #%d. ",
                GET_NAME(ch), GET_CH_SUF_5(ch), type_rent, type_hotel, no_room);
        sprintf(buf + strlen(buf), "Время на ренте %3.2f суток.", num_of_days);
    }
    mudlog(buf, NRM, MAX(LVL_GOD, GET_INVIS_LEV(ch)), TRUE);

}

void xrent_save_all(int type)
{
    struct descriptor_data *d;

    for (d = descriptor_list; d; d = d->next) {
        if ((STATE(d) == CON_PLAYING) && !IS_NPC(d->character)) {
            {
                save_pets(d->character);
                xsave_rent(d->character, type, FALSE);
                save_char(d->character, NOWHERE);
            }
        }
    }
}

void fenia_mark_guid(long long guid)
{
    if (!guid)
        return;

    if (!FeniaManager::wrapperManager)
        return;

    FeniaManager::wrapperManager->markAlive(guid);
}

#define SHOW_INDEX 0

// Проверка последователей персонажа при старте
void xpets_start_check(int index, bool start)
{
    CPet pet;
    char fname[256];
    int i, number;
    const char *name = player_table[index].name;

    if (!start)
        return;

    log("ПОСЛЕДОВАТЕЛИ: Персонаж [%s]", name);

    if (!pet.Initialization())
        return;

    get_filename(name, fname, PET_FILE);
    if (!pet.ReadConfig(fname))
        return;

    number = pet.GetNumberItem();
    for (i = 0; i < number; i++) {
        CItem *xpet = pet.GetItem(i);
        long long guid = xpet->GetItem(PET_GUID)->GetLongLong();

        fenia_mark_guid(guid);
    }
}

//Проверка и подсчет кол-ва предметов у персонажа
void xrent_start_check(int index, bool start)
{
    struct descriptor_data *d;
    Player *fch;
    char fname[MAX_INPUT_LENGTH];
    char name[MAX_NAME_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int isgod, number = 0, i, vnum, rnum, type_hotel, cost, gold = 0, type_rent, timer;
    bool check_cost = TRUE, needsave = FALSE;
    CRent Rent;
    CObj Wobj;
    float rent_time;

//Проверка, если персонаж в игре, то ничего не делаем
    for (d = descriptor_list; d; d = d->next)
        if ((STATE(d) == CON_PLAYING) && GET_IDNUM(d->character) == player_table[index].id)
            return;

    strcpy(name, player_table[index].name);
    log("ПОСТОЙ: Персонаж [%s]", name);

    fch = new Player();
    if (load_char(name, fch) > -1) {
        gold = GET_GOLD(fch) + GET_BANK_GOLD(fch);
        isgod = IS_GOD(fch);

        if (start)
            fenia_mark_guid(fch->guid);

        xpets_start_check(index, start);
    }
    delete fch;



    /* Загрузка */
    if (!Rent.Initialization()) {
        log("ПОСТОЙ: Ошибка инициализации Rent");
        return;
    }

    if (!Wobj.Initialization()) {
        log("ПОСТОЙ: Ошибка инициализации Wobj");
        return;
    }
    get_filename(name, fname, TIME_CRASH_FILE);
    if (!Rent.ReadConfig(fname))
        return;

    get_filename(name, fname, TEXT_CRASH_FILE);
    if (!Wobj.ReadConfig(fname))
        return;

    log("ПОСТОЙ: Обработка....");
    /* Данные ренты */
    CItem *rent = Rent.GetItem(0);

    rent_time =
        (float) ((float) (time(0) - rent->GetItem(RENT_TIME)->GetInt()) /
                 (float) SECS_PER_REAL_DAY);
    type_hotel = rent->GetItem(RENT_QUALITY)->GetInt();
    type_rent = rent->GetItem(RENT_TYPE)->GetInt();

    number = Wobj.GetNumberItem();

    if (number <= 0) {
        log("ПОСТОЙ: Предметов нет");
        return;
    } else
        log("ПОСТОЙ: Предметов %d", number);

    if (type_rent == RENT_CRASH &&
        rent->GetItem(RENT_TIME)->GetInt() + free_crashrent_period * SECS_PER_REAL_HOUR > time(0)) {
        //log("ПОСТОЙ: ...креш-рента");
        check_cost = FALSE;
    }

    if (type_rent == RENT_REBOOT &&
        rent->GetItem(RENT_TIME)->GetInt() + free_rebootrent_period * SECS_PER_REAL_HOUR >
        time(0)) {
        //log("ПОСТОЙ: ...перезагрузка");
        check_cost = FALSE;
    }

    /* ПРОВЕРКА ПРЕДМЕТОВ */
    for (i = 0; i < number; i++) {
        vnum = Wobj.GetItem(i)->GetItem(OBJ_NUMBER)->GetInt();
        //log("ПОСТОЙ: предмет [%d]",vnum);

        if (start)
            fenia_mark_guid(Wobj.GetItem(i)->GetItem(OBJ_GUID)->GetLongLong());

        struct new_flag obj_prop = { {0, 0, 0, 0} };
        //игнорируем обработку татушек
        if (Wobj.GetItem(i)->GetItem(OBJ_TYPE)->GetInt() == ITEM_TATOO)
            continue;
        if (Wobj.GetItem(i)->GetItem(OBJ_TYPE)->GetInt() == ITEM_MONEY)
            continue;

        cost = Wobj.GetItem(i)->GetItem(OBJ_COST)->GetInt();

        if (!IS_SET(GET_FLAG(obj_prop, ITEM_NODECAY), ITEM_NODECAY)) {
            if (Wobj.GetItem(i)->GetItem(OBJ_TIMELOAD)->GetInt())
                timer =
                    (time(0) - Wobj.GetItem(i)->GetItem(OBJ_TIMELOAD)->GetInt()) >
                    (Wobj.GetItem(i)->GetItem(OBJ_TIMER)->GetInt() * 60);
            else
                timer = -1;
        } else
            timer = 0;

        if ((float) (time(0) - rent->GetItem(RENT_TIME)->GetInt()) / (float) SECS_PER_REAL_DAY >
            30.0)
            timer = 1;

//   log("%s : %ld %3.2f",name,time(0) - rent->GetItem(RENT_TIME)->GetInt(),(float)(time(0) - rent->GetItem(RENT_TIME)->GetInt()) / (float)SECS_PER_REAL_DAY);

        if (Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString())
            asciiflag_conv((char *) Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString(),
                           &obj_prop);


        if (cost > max_obj_cost[type_hotel] && check_cost) {
            if (IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE), ITEM_RENT_DELETE) ||
                IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE2), ITEM_RENT_DELETE2) ||
                IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE3), ITEM_RENT_DELETE3))
                continue;

            if (Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString())
                sprintf(buf, "%sf1", Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString());
            else
                sprintf(buf, "f1");
            Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->SetParam(buf);

            needsave = TRUE;
            if (!start && (rnum = real_object(vnum)) >= 0)
                obj_index[rnum].stored--;

        } else
            if (IS_SET(GET_FLAG(obj_prop, ITEM_NORENT), ITEM_NORENT) ||
                IS_SET(GET_FLAG(obj_prop, ITEM_NORENT), ITEM_ZONEDECAY) ||
                IS_SET(GET_FLAG(obj_prop, ITEM_REPOPDROP), ITEM_REPOPDROP)) {
            if (IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE), ITEM_RENT_DELETE) ||
                IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE2), ITEM_RENT_DELETE2) ||
                IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE3), ITEM_RENT_DELETE3))
                continue;


            if (Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString())
                sprintf(buf, "%sg1", Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString());
            else
                sprintf(buf, "g1");
            Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->SetParam(buf);

            needsave = TRUE;
            if (!start && (rnum = real_object(vnum)) >= 0)
                obj_index[rnum].stored--;
        } else if (timer) {
            if (IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE), ITEM_RENT_DELETE) ||
                IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE2), ITEM_RENT_DELETE2) ||
                IS_SET(GET_FLAG(obj_prop, ITEM_RENT_DELETE3), ITEM_RENT_DELETE3))
                continue;

            if (Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString())
                sprintf(buf, "%sh1", Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->GetString());
            else
                sprintf(buf, "h1");
            Wobj.GetItem(i)->GetItem(OBJ_PROPERTIES)->SetParam(buf);

            needsave = TRUE;
            if (!start && (rnum = real_object(vnum)) >= 0)
                obj_index[rnum].stored--;
        } else if ((rnum = real_object(vnum)) >= 0 && start) {
            obj_index[rnum].stored++;
        }
    }

    log("ПОСТОЙ: ...запись");
    if (needsave)
        Wobj.SaveAll(fname, TRUE);
}



void xrent_check(int act)
{
    int c;

    for (c = 0; c <= top_of_p_table; c++)
        if (player_table[c].unique != -1 && player_table[c].activity == act)
            xrent_start_check(c, FALSE);
}


///////////////////////////////////////////////////////////////////////////////
void save_pets(struct char_data *ch)
{
    CPet pet;
    char fname[256];
    struct follow_type *f, *fn;
    struct char_data *victim;
    int i, needsave = FALSE;

    if (ch == NULL)
        return;

    if (IS_NPC(ch))
        return;

    if (!pet.Initialization())
        return;

    get_filename(GET_NAME(ch), fname, PET_FILE);

    log("ЗАПИСЬ: Последователи [%s]", GET_NAME(ch));

    for (f = ch->followers; f; f = fn) {
        fn = f->next;
        if (IS_NPC(f->follower) && f->type == FLW_CHARM) {
            log("---Последователь [%s]", GET_NAME(f->follower));
            needsave = TRUE;
            victim = f->follower;
            i = pet.NewSubItem();
            CItem *xpet = pet.GetItem(i);

            xpet->GetItem(PET_NUMBER)->SetParam(GET_MOB_VNUM(victim));
            xpet->GetItem(PET_GUID)->SetParam(victim->guid);
            xpet->GetItem(PET_NAME)->SetParam(victim->player.names);
            xpet->GetItem(PET_RACE)->SetParam(GET_RACE(victim));
            xpet->GetItem(PET_TYPE)->SetParam(f->type);
            xpet->GetItem(PET_EXP)->SetParam((int) GET_EXP(victim));
            xpet->GetItem(PET_HIT)->SetParam((int) GET_HIT(victim));
            xpet->GetItem(PET_MOVE)->SetParam((int) GET_MOVE(victim));
            xpet->GetItem(PET_MANA)->SetParam((int) GET_MANA(victim));
            xpet->GetItem(PET_LIKES)->SetParam((int) GET_LIKES(victim));

            for (int icls = 0; icls < NUM_CLASSES; icls++)
                if (victim->classes[icls] > 0) {
                    int classes = xpet->GetItem(PET_CLASS)->NewSubItem();
                    CItem *xcls = xpet->GetItem(PET_CLASS)->GetItem(classes);

                    xcls->GetItem(PET_CLASS_TYPE)->SetParam(icls);
                    xcls->GetItem(PET_CLASS_LEVEL)->SetParam(victim->classes[icls]);
                }

            for (int skl = 0; skl < MAX_SKILLS; skl++)
                if (GET_SKILL_MOB(victim, skl) > 0)
                    xpet->GetItem(PET_SKILLS)->AddParam(skl, 1);

            struct affected_type *af = get_affect_by_vector(victim, AFF_CHARM);

            xpet->GetItem(PET_SPELL)->SetParam(SPELL_NO(af->type));
            xpet->GetItem(PET_TIME)->SetParam(af->duration);
            if (!circle_copyover) {
                victim->extractWrapper(false);
                extract_char(victim, FALSE);
            }
        }
    }

    if (needsave)
        pet.SaveAll(fname, TRUE);
    else
        unlink(fname);
}

void load_pets(struct char_data *ch)
{
    CPet pet;
    char fname[256], name[256];
    struct char_data *victim;
    int i, j, numadd, number, t[2];

    if (ch == NULL)
        return;

    if (IS_NPC(ch))
        return;

    if (IN_ROOM(ch) == NOWHERE)
        return;

    if (!pet.Initialization())
        return;

    get_filename(GET_NAME(ch), fname, PET_FILE);

    if (!pet.ReadConfig(fname)) {
        log("ОШИБКА: Персонаж '%s' не имеет файла последователей", GET_NAME(ch));
        return;
    }

    number = pet.GetNumberItem();
    for (i = 0; i < number; i++) {
        CItem *xpet = pet.GetItem(i);

        victim = read_mobile(xpet->GetItem(PET_NUMBER)->GetInt(), VIRTUAL);
        if (xpet->GetItem(PET_GUID)->GetLongLong())
            victim->guid = xpet->GetItem(PET_GUID)->GetLongLong();
        clear_mob_specials(victim);
        struct affected_type af;

        af.duration = xpet->GetItem(PET_TIME)->GetInt();
        af.type = find_spell_num(xpet->GetItem(PET_SPELL)->GetInt());
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        af.battleflag = 0;
        af.main = TRUE;
        af.owner = GET_ID(ch);
        affect_to_char(victim, &af);
        GET_EXP(victim) = xpet->GetItem(PET_EXP)->GetInt();
        if (GET_EXP(victim) > get_levelexp(victim, GET_LEVEL(victim) + 1, 1) ||
            GET_EXP(victim) < get_levelexp(victim, GET_LEVEL(victim), 1))
            GET_EXP(victim) = level_exp(victim, GET_LEVEL(victim) + 1) - 1;


        if ((numadd = xpet->GetItem(PET_CLASS)->GetNumberItem())) {
            victim->player.level = 0;
            for (j = 0; j < numadd; j++) {
                CItem *mcls = xpet->GetItem(PET_CLASS)->GetItem(j);

                add_class(victim, mcls->GetItem(PET_CLASS_TYPE)->GetInt(),
                          mcls->GetItem(PET_CLASS_LEVEL)->GetInt(), TRUE);
                victim->player.chclass = mcls->GetItem(PET_CLASS_TYPE)->GetInt();
                victim->player.level += mcls->GetItem(PET_CLASS_LEVEL)->GetInt();
            }
        }

        numadd = xpet->GetItem(PET_SKILLS)->GetStrListNumber();
        for (j = 0; j < numadd; j++) {
            xpet->GetItem(PET_SKILLS)->GetStrList(j, t[0], t[1]);
            SET_SKILL(victim, t[0]) = t[1];
        }

        affect_total(victim);

        if (xpet->GetItem(PET_HIT)->GetInt())
            GET_HIT(victim) = xpet->GetItem(PET_HIT)->GetInt();
        if (xpet->GetItem(PET_MOVE)->GetInt())
            GET_MOVE(victim) = xpet->GetItem(PET_MOVE)->GetInt();
        if (xpet->GetItem(PET_MANA)->GetInt())
            GET_MANA(victim) = xpet->GetItem(PET_MANA)->GetInt();
        if (xpet->GetItem(PET_LIKES)->GetInt())
            GET_LIKES(victim) = xpet->GetItem(PET_LIKES)->GetInt();

        if (xpet->GetItem(PET_NAME)->GetString()) {
            sprintf(name, "%s", xpet->GetItem(PET_NAME)->GetString());
            victim->player.names = str_dup(name);
            for (i = 0; i < NUM_PADS; i++)
                GET_PAD(victim, i) = str_dup(get_name_pad(name, i, PAD_MONSTER));

            GET_NAME(victim) = str_dup(get_name_pad(name, PAD_IMN, PAD_MONSTER));
            victim->player.name = str_dup(get_name_pad(name, PAD_IMN, PAD_MONSTER));
        }

        change_pet_name(ch, victim);
        add_follower(victim, ch, xpet->GetItem(PET_TYPE)->GetInt());
        victim->linkWrapper();
        char_to_room(victim, IN_ROOM(ch));
    }

}

///////////////////////////////////////////////////////////////////////////////

void save_vars(struct char_data *ch)
{
    CVarSave vars;
    char fname[256];
    int i;

    vars_storage::iterator v;
    bool needsave = FALSE;

    if (ch == NULL)
        return;

    if (IS_NPC(ch))
        return;

    if (!vars.Initialization())
        return;

    get_filename(GET_NAME(ch), fname, SCRIPT_VARS_FILE);


    for (v = ch->saved_vars.begin(); v != ch->saved_vars.end(); v++) {
        i = vars.NewSubItem();
        CItem *xvar = vars.GetItem(i);

        xvar->GetItem(VARS_NUMBER)->SetParam(i);
        xvar->GetItem(VARS_NAME)->SetParam(v->first.c_str());
        xvar->GetItem(VARS_VALUE)->SetParam(v->second.value.c_str());
        xvar->GetItem(VARS_TIME)->SetParam((int) v->second.time);
        needsave = TRUE;
    }

    if (needsave)
        vars.SaveAll(fname, TRUE);
    else
        unlink(fname);

}

void load_vars(struct char_data *ch)
{

    CVarSave vars;
    char fname[256];
    int i, number;

    if (ch == NULL)
        return;

    if (IS_NPC(ch))
        return;

    if (!vars.Initialization())
        return;

    get_filename(GET_NAME(ch), fname, SCRIPT_VARS_FILE);

    if (!vars.ReadConfig(fname)) {
        return;
    }

    number = vars.GetNumberItem();
    for (i = 0; i < number; i++) {
        CItem *xvar = vars.GetItem(i);

        add_saved_var(ch,
                      xvar->GetItem(VARS_NAME)->GetString(),
                      xvar->GetItem(VARS_VALUE)->GetString(), xvar->GetItem(VARS_TIME)->GetInt());
    }

}

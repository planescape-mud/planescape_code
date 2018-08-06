/**************************************************************************
    МПМ "Грани Мира" (с) 2002-2003 Андрей Ермишин
    Загрузка файлов игрового мира
 **************************************************************************/

#include "sysdep.h"

#include "structs.h"
#include "db.h"
#include "utils.h"
#include "handler.h"
#include "xmob.h"
#include "xbody.h"
#include "spells.h"
#include "xspells.h"
#include "case.h"
#include "parser_id.h"
#include "constants.h"
#include "strlib.h"
#include "xboot.h"
#include "expr.h"
#include "planescape.h"
#include "mudfilereader.h"


void fenia_mark_guid(long long guid);


///////////////////////////////////////////////////////////////////////////////
int get_xrec_mob(void)
{
    Mob = new CMob;

    if (!Mob->Initialization())
        exit(1);

    MudFileReader reader(mud->mobDir);

    while (reader.hasNext())
        if (!Mob->ReadConfig(reader.next().getCPath()))
            exit(1);

    return Mob->GetNumberItem();
}

///////////////////////////////////////////////////////////////////////////////

void boot_mob(void)
{

    struct extra_descr_data *new_descr;
    int i, j, number, numadd, vnum, t[3];
    char buf[MAX_STRING_LENGTH];

    number = Mob->GetNumberItem();
    for (i = 0; i < number; i++) {
        int num_spells = 0;
        CItem *mob = Mob->GetItem(i);

        vnum = mob->GetItem(MOB_NUMBER)->GetInt();
        if (!mud->modQuietLoad)
            log("ЗАГРУЗКА: Монстр [%d]", vnum);

        //Инициализация переменных
        mob_index[imbs].vnum = vnum;
        mob_index[imbs].number = 0;
        mob_index[imbs].func = NULL;
        mob_index[imbs].func_name = NULL;
        new(mob_proto + imbs) Mobile(vnum);

        mob_proto[imbs].npc()->specials.Questor = NULL;
        mob_proto[imbs].player.title = NULL;
        mob_proto[imbs].player.title_r = NULL;
        mob_proto[imbs].points.move = 100;
        mob_proto[imbs].points.max_move = 100;
        mob_proto[imbs].nr = imbs;
        mob_proto[imbs].npc()->specials.move_to = NOWHERE;
        mob_proto[imbs].carrying = NULL;
        mob_proto[imbs].spec_hit = NULL;
        mob_proto[imbs].affected = NULL;
        for (j = 0; j < NUM_WEARS; j++) {
            mob_proto[imbs].equipment[j] = NULL;
            mob_proto[imbs].tatoo[j] = NULL;
        }

        for (j = 0; j < NUM_SAV; j++)
            mob_proto[imbs].npc()->specials.saved[j] = 0;
        mob_proto[imbs].desc = NULL;


        CREATE(mob_proto[imbs].player.name, char, strlen(mob->GetItem(MOB_ALIAS)->GetString()) + 1);

        strcpy(mob_proto[imbs].player.name, mob->GetItem(MOB_ALIAS)->GetString());

        del_spaces(mob->GetItem(MOB_NAME)->GetString());
        CREATE(mob_proto[imbs].player.names, char, strlen(mob->GetItem(MOB_NAME)->GetString()) + 1);

        strcpy(mob_proto[imbs].player.names, mob->GetItem(MOB_NAME)->GetString());

        for (j = 0; j < NUM_PADS; j++) {
            strcpy(buf, get_name_pad((char *) mob->GetItem(MOB_NAME)->GetString(), j, PAD_MONSTER));
            CREATE(GET_PAD(mob_proto + imbs, j), char, strlen(buf) + 1);

            strcpy(GET_PAD(mob_proto + imbs, j), buf);
        }

        strcpy(buf,
               get_name_pad((char *) mob->GetItem(MOB_NAME)->GetString(), PAD_IMN, PAD_MONSTER));
        CREATE(mob_proto[imbs].player.short_descr, char, strlen(buf) + 1);

        strcpy(mob_proto[imbs].player.short_descr, buf);

        CREATE(mob_proto[imbs].player.long_descr, char,
               strlen(mob->GetItem(MOB_LINE)->GetString()) + 1);
        strcpy(mob_proto[imbs].player.long_descr, mob->GetItem(MOB_LINE)->GetString());

        if (mob->GetItem(MOB_DESCRIPTION)->GetString()) {
            CREATE(mob_proto[imbs].player.description, char,
                   strlen(mob->GetItem(MOB_DESCRIPTION)->GetString()) + 1);
            strcpy(mob_proto[imbs].player.description, mob->GetItem(MOB_DESCRIPTION)->GetString());
        }

        numadd = mob->GetItem(MOB_ADDITION)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            CItem *add = mob->GetItem(MOB_ADDITION)->GetItem(j);
            CREATE(new_descr, struct extra_descr_data, 1);
            CREATE(new_descr->keyword, char, strlen(add->GetItem(MOB_ADD_KEY)->GetString()) + 1);

            strcpy(new_descr->keyword, add->GetItem(MOB_ADD_KEY)->GetString());
            CREATE(new_descr->description, char,
                   strlen(add->GetItem(MOB_ADD_TEXT)->GetString()) + 1);
            strcpy(new_descr->description, add->GetItem(MOB_ADD_TEXT)->GetString());

            new_descr->next = mob_proto[imbs].player.ex_description;
            mob_proto[imbs].player.ex_description = new_descr;
        }

        if (mob->GetItem(MOB_PROPERTIES)->GetString())
            asciiflag_conv((char *) mob->GetItem(MOB_PROPERTIES)->GetString(),
                           &MOB_FLAGS(mob_proto + imbs, 0));
        SET_BIT(MOB_FLAGS(mob_proto + imbs, MOB_ISNPC), MOB_ISNPC);

        if (mob->GetItem(MOB_ADDONS)->GetString())
            asciiflag_conv((char *) mob->GetItem(MOB_ADDONS)->GetString(),
                           &NPC_FLAGS(mob_proto + imbs, 0));

        mob_proto[imbs].player.sex = mob->GetItem(MOB_SEX)->GetInt();
        //mob_proto[imbs].player.level = mob->GetItem(MOB_LEVEL)->GetInt();
        //mob_proto[imbs].player.chclass = mob->GetItem(MOB_CLASS)->GetInt();
        if (mob->GetItem(MOB_AGE)->GetInt())
            mob_proto[imbs].npc()->specials.age = mob->GetItem(MOB_AGE)->GetInt();
        else
            mob_proto[imbs].npc()->specials.age = 3;

        //КЛАСС
        if ((numadd = mob->GetItem(MOB_CLASSES)->GetNumberItem()))
            for (j = 0; j < numadd; j++) {
                CItem *mcls = mob->GetItem(MOB_CLASSES)->GetItem(j);

                add_class(mob_proto + imbs, mcls->GetItem(MOB_CLASS_TYPE)->GetInt(),
                          mcls->GetItem(MOB_CLASS_LEVEL)->GetInt(), TRUE);
                mob_proto[imbs].player.chclass = mcls->GetItem(MOB_CLASS_TYPE)->GetInt();
                mob_proto[imbs].player.level += mcls->GetItem(MOB_CLASS_LEVEL)->GetInt();
        } else {
            add_class(mob_proto + imbs, mob->GetItem(MOB_CLASS)->GetInt(),
                      mob->GetItem(MOB_LEVEL)->GetInt(), TRUE);
            mob_proto[imbs].player.chclass = mob->GetItem(MOB_CLASS)->GetInt();
            mob_proto[imbs].player.level = mob->GetItem(MOB_LEVEL)->GetInt();
        }

        mob_proto[imbs].player.race = mob->GetItem(MOB_RACE)->GetInt();
        mob_proto[imbs].player.mob_type = mob->GetItem(MOB_TYPE)->GetInt();
        GET_ALIGNMENT(mob_proto + imbs) = mob->GetItem(MOB_ALIGN)->GetInt();
        GET_DEFAULT_POS(mob_proto + imbs) = mob->GetItem(MOB_POSITION)->GetInt();
        GET_MOVE_TYPE(mob_proto + imbs) = mob->GetItem(MOB_MOVED)->GetInt();

        if (mob->GetItem(MOB_MOVESTR)->GetString()) {
            del_spaces(mob->GetItem(MOB_MOVESTR)->GetString());
            CREATE(mob_proto[imbs].npc()->specials.move_str, char,
                   strlen(mob->GetItem(MOB_MOVESTR)->GetString()) + 1);
            strcpy(mob_proto[imbs].npc()->specials.move_str,
                   mob->GetItem(MOB_MOVESTR)->GetString());
        }

        mob_proto[imbs].real_abils.str = mob->GetItem(MOB_STR)->GetInt();
        mob_proto[imbs].real_abils.con = mob->GetItem(MOB_CON)->GetInt();
        mob_proto[imbs].real_abils.dex = mob->GetItem(MOB_DEX)->GetInt();
        mob_proto[imbs].real_abils.intel = mob->GetItem(MOB_INT)->GetInt();
        mob_proto[imbs].real_abils.wis = mob->GetItem(MOB_WIS)->GetInt();
        mob_proto[imbs].real_abils.cha = mob->GetItem(MOB_CHA)->GetInt();
        mob_proto[imbs].real_abils.size = mob->GetItem(MOB_SIZE)->GetInt();

        mob_proto[imbs].npc()->specials.attack_type = mob->GetItem(MOB_HIT1)->GetInt();
        mob_proto[imbs].npc()->specials.ExtraAttack = mob->GetItem(MOB_COUNT1)->GetInt();

        if (mob->GetItem(MOB_COUNT2)->GetInt() && mob->GetItem(MOB_DAMAGE2)->GetString()) {
            mob_proto[imbs].npc()->specials.attack_type2 = mob->GetItem(MOB_HIT2)->GetInt();
            mob_proto[imbs].npc()->specials.ExtraAttack2 = mob->GetItem(MOB_COUNT2)->GetInt();
        }

        if (mob->GetItem(MOB_HEIGHT)->GetInt())
            mob_proto[imbs].player.height = mob->GetItem(MOB_HEIGHT)->GetInt();
        else
            mob_proto[imbs].player.height = 198;
        if (mob->GetItem(MOB_WEIGHT)->GetInt())
            mob_proto[imbs].player.weight = mob->GetItem(MOB_WEIGHT)->GetInt();
        else
            mob_proto[imbs].player.weight = 200;

        if (mob->GetItem(MOB_GOLD)->GetString() &&
            sscanf(mob->GetItem(MOB_GOLD)->GetString(), "%dd%d+%d", t, t + 1, t + 2) == 3) {
            GET_GOLD(mob_proto + imbs) = t[2];
            GET_GOLD_NoDs(mob_proto + imbs) = t[0];
            GET_GOLD_SiDs(mob_proto + imbs) = t[1];
        }

        if (mob->GetItem(MOB_WIMP)->GetInt())
            mob_proto[imbs].npc()->specials.wimp_level = mob->GetItem(MOB_WIMP)->GetInt();

        //Умения
        SET_SKILL(mob_proto + imbs, SKILL_PUNCH) =
            calc_need_improove(mob_proto + imbs, GET_LEVEL(mob_proto + imbs));
        numadd = mob->GetItem(MOB_SKILL)->GetStrListNumber();
        for (j = 0; j < numadd; j++) {
            mob->GetItem(MOB_SKILL)->GetStrList(j, t[0], t[1]);
            t[1] = calc_need_improove(mob_proto + imbs, GET_LEVEL(mob_proto + imbs));
            t[1] = MIN(150, MAX(0, t[1]));
            SET_SKILL(mob_proto + imbs, t[0]) = t[1];

            //проверяем это скилл магический
            if (t[0] >= SKILL_SP_ENCHANTMENT && t[0] <= SKILL_SP_DESTRUCTION)
                for (int n = 0; n < Spl.GetNumberItem(); n++) {
                    if (SPELL_LSPHERE(n) == t[0] && SPELL_LEVEL(n) <= t[1]) {
                        //if (vnum == 5147)
                        // log("Ставлю %s Заклинание %s",GET_NAME(mob_proto+imbs),SPELL_NAME(n));
                        SET_BIT(GET_SPELL_TYPE(mob_proto + imbs, SPELL_NO(n)), SPELL_KNOW);
                        num_spells++;
                    }
                }
        }                       //КОНЕЦ УМЕНИЯМ

        numadd = mob->GetItem(MOB_EQ)->GetStrListNumber();
        for (j = 0; j < numadd; j++) {
            mob->GetItem(MOB_EQ)->GetStrList(j, t[0], t[1]);
            add_eq_obj(mob_proto + imbs, t[1], t[0]);
        }

        numadd = mob->GetItem(MOB_INV)->GetStrListNumber();
        for (j = 0; j < numadd; j++) {
            mob->GetItem(MOB_INV)->GetStrList(j, t[0], t[1]);
            add_inv_obj(mob_proto + imbs, t[0], t[1]);
        }

        numadd = mob->GetItem(MOB_DEATH)->GetStrListNumber();
        for (j = 0; j < numadd; j++) {
            mob->GetItem(MOB_DEATH)->GetStrList(j, t[0], t[1]);
            add_death_obj(mob_proto + imbs, t[0], t[1]);
        }

        numadd = mob->GetItem(MOB_TATOO)->GetStrListNumber();
        for (j = 0; j < numadd; j++) {
            mob->GetItem(MOB_TATOO)->GetStrList(j, t[0], t[1]);
            add_tatoo_obj(mob_proto + imbs, t[1], t[0]);
        }

        //СЛЕДУЕТ
        if (mob->GetItem(MOB_FOLLOW)->GetInt())
            mob_proto[imbs].follow_vnum = mob->GetItem(MOB_FOLLOW)->GetInt();

        //ГРУППА
        const int *grps = mob->GetItem(MOB_HELPED)->GetScript(numadd);

        for (j = 0; j < numadd; j++) {
            //log("ГРУППА %d",grps[j]);
            (mob_proto + imbs)->helpers.insert(grps[j]);
        }

        //ПУТЬ
        const int *dests = mob->GetItem(MOB_DEST)->GetScript(numadd);

        for (j = 0; j < numadd; j++)
            if (mob_proto[imbs].npc()->specials.dest_count < MAX_DEST) {
                mob_proto[imbs].npc()->specials.dest[mob_proto[imbs].npc()->specials.dest_count] =
                    dests[j];
                mob_proto[imbs].npc()->specials.dest_count++;
            }

        mob_proto[imbs].real_abils.lck = mob->GetItem(MOB_LACKY)->GetInt();
        mob_proto[imbs].npc()->specials.speed = mob->GetItem(MOB_SPEED)->GetInt();
        mob_proto[imbs].npc()->specials.LikeWork = mob->GetItem(MOB_LIKEWORK)->GetInt();


        if (mob->GetItem(MOB_SHOP)->GetNumberItem()) {
            CItem *shop = mob->GetItem(MOB_SHOP)->GetItem(0);
            CREATE(mob_proto[imbs].shop_data, struct mob_shop_data, 1);

            mob_proto[imbs].shop_data->ok = TRUE;

            mob_proto[imbs].shop_data->sell = shop->GetItem(MOB_SHOP_SELL)->GetInt();
            mob_proto[imbs].shop_data->buy = shop->GetItem(MOB_SHOP_BUY)->GetInt();
            mob_proto[imbs].shop_data->repair = shop->GetItem(MOB_SHOP_REPAIR)->GetInt();
            mob_proto[imbs].shop_data->quality = shop->GetItem(MOB_SHOP_QUALITY)->GetInt();

            numadd = shop->GetItem(MOB_SHOP_TYPE)->GetStrListNumber();
            for (j = 0; j < numadd; j++) {
                shop->GetItem(MOB_SHOP_TYPE)->GetStrList(j, t[0], t[1]);
                add_shop_type(mob_proto + imbs, t[0]);
            }

            const int *shop_list = shop->GetItem(MOB_SHOP_LIST)->GetScript(numadd);

            for (j = 0; j < numadd; j++)
                add_shop_obj(mob_proto + imbs, shop_list[j], -1);
        }

        numadd = mob->GetItem(MOB_SPECIAL)->GetNumberItem();
        int v[2];

        for (j = 0; j < numadd; j++) {
            int pos[POS_NUMBER], pcount, pp, p1, p2, saves[NUM_SAV], ii;
            CItem *spec = mob->GetItem(MOB_SPECIAL)->GetItem(j);

            for (ii = 0; ii < NUM_SAV; ii++)
                saves[ii] = 0;

            t[0] = t[1] = t[2] = 0;
            if (spec->GetItem(MOB_SPEC_DAMAGE)->GetString() &&
                sscanf(spec->GetItem(MOB_SPEC_DAMAGE)->GetString(), "%dd%d+%d", t, t + 1,
                       t + 2) != 3) {
                log("ОШИБКА: Неверно указано поле СПЕЦУДАР.ВРЕД, монстр #%d", vnum);
                exit(1);
            }


            v[0] = spec->GetItem(MOB_SPEC_PERC)->GetInt();
            v[1] = spec->GetItem(MOB_SPEC_POWER)->GetInt();
            if (!v[0])
                v[0] = 100;
            if (!v[1])
                v[1] = 100;

            for (pcount = 0; pcount < POS_NUMBER; pcount++)
                pos[pcount] = TRUE;     //по умолчанию все позиции включены

            pp = spec->GetItem(MOB_SPEC_POS)->GetStrListNumber();

            if (pp)
                for (pcount = 0; pcount < POS_NUMBER; pcount++)
                    pos[pcount] = FALSE;        //а теперь выключаем

            for (pcount = 0; pcount < pp; pcount++) {
                spec->GetItem(MOB_SPEC_POS)->GetStrList(pcount, p1, p2);
                if (p1)
                    pos[p1] = TRUE;
                else
                    pos[p1] = FALSE;
            }

            pp = spec->GetItem(MOB_SPEC_SAVE)->GetStrListNumber();
            for (pcount = 0; pcount < pp; pcount++) {
                spec->GetItem(MOB_SPEC_SAVE)->GetStrList(pcount, t[0], t[1]);
                saves[t[0]] = t[1];
            }


            add_spechit(mob_proto + imbs,
                        spec->GetItem(MOB_SPEC_TYPE)->GetInt(),
                        spec->GetItem(MOB_SPEC_HIT)->GetInt(),
                        spec->GetItem(MOB_SPEC_SPELL)->GetInt(),
                        pos,
                        t[0], t[1], t[2], v[0], v[1],
                        spec->GetItem(MOB_SPEC_VICTIM)->GetString(),
                        spec->GetItem(MOB_SPEC_ROOM)->GetString(), saves,
                        spec->GetItem(MOB_SPEC_PROP)->GetString());
        }

        //МЭФФЕКТЫ
        if (mob->GetItem(MOB_AFFECTS)->GetString())
            asciiflag_conv((char *) mob->GetItem(MOB_AFFECTS)->GetString(),
                           &AFF_FLAGS(mob_proto + imbs, 0));

        if (mob->GetItem(MOB_GOD)->GetInt())
            mob_proto[imbs].player.gods = mob->GetItem(MOB_GOD)->GetInt();

        if (mob->GetItem(MOB_FRACTION)->GetInt())
            mob_proto[imbs].player.fraction = mob->GetItem(MOB_FRACTION)->GetInt();

        if (mob->GetItem(MOB_RANG)->GetInt())
            mob_proto[imbs].player.rank = mob->GetItem(MOB_RANK)->GetInt();

        if (mob->GetItem(MOB_HORSE)->GetInt())
            mob_proto[imbs].npc()->specials.vnum_horse = mob->GetItem(MOB_HORSE)->GetInt();

        //СМЕРТЬ
        if (mob->GetItem(MOB_SDEATH)->GetNumberItem()) {
            CItem *sdt = mob->GetItem(MOB_SDEATH)->GetItem(0);

            mob_proto[imbs].npc()->specials.vnum_corpse = sdt->GetItem(MOB_SDT_CORPSE)->GetInt();
            mob_proto[imbs].npc()->specials.death_script = sdt->GetItem(MOB_SDT_SCRIPT)->GetInt();

            if (sdt->GetItem(MOB_SDT_DCHAR)->GetString()) {
                CREATE(mob_proto[imbs].npc()->specials.CMessChar, char,
                       strlen(sdt->GetItem(MOB_SDT_DCHAR)->GetString()) + 1);
                strcpy(mob_proto[imbs].npc()->specials.CMessChar,
                       sdt->GetItem(MOB_SDT_DCHAR)->GetString());
            }
            if (sdt->GetItem(MOB_SDT_DROOM)->GetString()) {
                CREATE(mob_proto[imbs].npc()->specials.CMessRoom, char,
                       strlen(sdt->GetItem(MOB_SDT_DROOM)->GetString()) + 1);
                strcpy(mob_proto[imbs].npc()->specials.CMessRoom,
                       sdt->GetItem(MOB_SDT_DROOM)->GetString());
            }
            if (sdt->GetItem(MOB_SDT_CHAR)->GetString()) {
                CREATE(mob_proto[imbs].npc()->specials.DMessChar, char,
                       strlen(sdt->GetItem(MOB_SDT_CHAR)->GetString()) + 1);
                strcpy(mob_proto[imbs].npc()->specials.DMessChar,
                       sdt->GetItem(MOB_SDT_CHAR)->GetString());
            }
            if (sdt->GetItem(MOB_SDT_ROOM)->GetString()) {
                CREATE(mob_proto[imbs].npc()->specials.DMessRoom, char,
                       strlen(sdt->GetItem(MOB_SDT_ROOM)->GetString()) + 1);
                strcpy(mob_proto[imbs].npc()->specials.DMessRoom,
                       sdt->GetItem(MOB_SDT_ROOM)->GetString());
            }

            if (sdt->GetItem(MOB_SDT_TARGET)->GetInt())
                mob_proto[imbs].npc()->specials.death_flag = sdt->GetItem(MOB_SDT_TARGET)->GetInt();

            if (sdt->GetItem(MOB_SDT_DAMAGE)->GetInt())
                mob_proto[imbs].npc()->specials.d_type_hit = sdt->GetItem(MOB_SDT_DAMAGE)->GetInt();

            if (sdt->GetItem(MOB_SDT_HIT)->GetString()) {
                if (sscanf(sdt->GetItem(MOB_SDT_HIT)->GetString(), "%dd%d+%d", t, t + 1, t + 2) !=
                    3) {
                    log("ОШИБКА: Неверно указано поле СМЕРТЬ.СВРЕД, монстр #%d", vnum);
                    exit(1);
                }
                mob_proto[imbs].npc()->specials.DamageDamDice = t[0];
                mob_proto[imbs].npc()->specials.DamageDamSize = t[1];
                mob_proto[imbs].npc()->specials.DamageDamRoll = t[2];
            }
        }
        //УСИЛЕНИЕ
        if (mob->GetItem(MOB_POWER)->GetInt())
            mob_proto[imbs].npc()->specials.powered = mob->GetItem(MOB_POWER)->GetInt();
        else
            mob_proto[imbs].npc()->specials.powered = 0;

        //ЗАЩИТА
        numadd = mob->GetItem(MOB_SAVE)->GetStrListNumber();
        for (j = 0; j < numadd; j++) {
            mob->GetItem(MOB_SAVE)->GetStrList(j, t[0], t[1]);
            mob_proto[imbs].npc()->specials.saved[t[0]] = t[1];
            //log("******* ЗАГРУЗКА %s t[0] %d t[1] %d",GET_NAME(&mob_proto[imbs]),t[0],t[1]);
        }

        //ОПОВЕЩЕНИЕ
        if (mob->GetItem(MOB_ALARM)->GetNumberItem()) {
            CItem *alarm = mob->GetItem(MOB_ALARM)->GetItem(0);
            const int *grps = alarm->GetItem(MOB_ALARM_HELPER)->GetScript(numadd);

            for (j = 0; j < numadd; j++) {
                mob_proto[imbs].npc()->specials.alr_helper.insert(grps[j]);
            }

            if (alarm->GetItem(MOB_ALARM_LEVEL)->GetInt())
                mob_proto[imbs].npc()->specials.AlrLevel =
                    alarm->GetItem(MOB_ALARM_LEVEL)->GetInt();
            else
                mob_proto[imbs].npc()->specials.AlrLevel = 100;

            if (alarm->GetItem(MOB_ALARM_CHAR)->GetString()) {
                CREATE(mob_proto[imbs].npc()->specials.AlrMessChar, char,
                       strlen(alarm->GetItem(MOB_ALARM_CHAR)->GetString()) + 1);
                strcpy(mob_proto[imbs].npc()->specials.AlrMessChar,
                       alarm->GetItem(MOB_ALARM_CHAR)->GetString());
            }
            if (alarm->GetItem(MOB_ALARM_ROOM)->GetString()) {
                CREATE(mob_proto[imbs].npc()->specials.AlrMessRoom, char,
                       strlen(alarm->GetItem(MOB_ALARM_ROOM)->GetString()) + 1);
                strcpy(mob_proto[imbs].npc()->specials.AlrMessRoom,
                       alarm->GetItem(MOB_ALARM_ROOM)->GetString());
            }
        }
        //ПЕРИОД
        numadd = mob->GetItem(MOB_PERIOD)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            struct mob_period_data *period;
            CREATE(period, struct mob_period_data, 1);
            CItem *prd = mob->GetItem(MOB_PERIOD)->GetItem(j);

            period->start = prd->GetItem(MOB_PRD_START)->GetInt();
            period->stop = prd->GetItem(MOB_PRD_STOP)->GetInt();
            period->period = prd->GetItem(MOB_PRD_TIME)->GetInt();
            period->target = prd->GetItem(MOB_PRD_TARGET)->GetInt();
            period->to_char = str_dup(prd->GetItem(MOB_PRD_CHAR)->GetString());
            period->to_room = str_dup(prd->GetItem(MOB_PRD_ROOM)->GetString());
            period->next = mob_proto[imbs].period;
            mob_proto[imbs].period = period;
        }

        //ПЕРЕХВАТ
        numadd = mob->GetItem(MOB_INTERCEPTION)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            CItem *inter = mob->GetItem(MOB_INTERCEPTION)->GetItem(j);

            add_message(mob_proto + imbs, inter->GetItem(MOB_INT_COMMAND)->GetInt(),
                        inter->GetItem(MOB_INT_STOP)->GetInt(),
                        inter->GetItem(MOB_INT_SCRIPT)->GetInt(),
                        (char *) inter->GetItem(MOB_INT_SARG)->GetString(),
                        (char *) inter->GetItem(MOB_INT_MESSPLAYER)->GetString(),
                        (char *) inter->GetItem(MOB_INT_MESSVICTIM)->GetString(),
                        (char *) inter->GetItem(MOB_INT_MESSOTHER)->GetString(),
                        (char *) inter->GetItem(MOB_INT_MESSROOM)->GetString());
        }

        //ТЕЛО
        if ((numadd = mob->GetItem(MOB_BODY)->GetNumberItem())) {
            (mob_proto + imbs)->ibody = numadd;
            (mob_proto + imbs)->body = new struct body_part[numadd + 1];

            for (j = 0; j < numadd; j++) {
                CItem *body = mob->GetItem(MOB_BODY)->GetItem(j);

                (mob_proto + imbs)->body[j].wear_position =
                    body->GetItem(MOB_BODY_POSITION)->GetInt();
                const char *bname =
                    body->GetItem(MOB_BODY_SNAME)->GetInt()? body_name[body->
                                                                       GetItem(MOB_BODY_SNAME)->
                                                                       GetInt()] : body->
                    GetItem(MOB_BODY_NAME)->GetString();
                if (bname)
                    (mob_proto + imbs)->body[j].name = bname;
                (mob_proto + imbs)->body[j].chance = body->GetItem(MOB_BODY_CHANCE)->GetInt();
                (mob_proto + imbs)->body[j].kdam = body->GetItem(MOB_BODY_KDAM)->GetInt();
            }
        }
        //если тело не задано пытаемся найти в справочнике
        else {
            CItem *body = tBody.FindItem(mob_proto[imbs].player.race);

            if (body) {
                numadd = body->GetItem(TBD_BODY)->GetNumberItem();
                if (numadd) {
                    (mob_proto + imbs)->ibody = numadd;
                    (mob_proto + imbs)->body = new struct body_part[numadd + 1];

                    for (j = 0; j < numadd; j++) {
                        CItem *bdb = body->GetItem(TBD_BODY)->GetItem(j);

                        (mob_proto + imbs)->body[j].wear_position =
                            bdb->GetItem(TBD_BODY_POSITION)->GetInt();
                        const char *bname =
                            bdb->GetItem(TBD_BODY_SNAME)->GetInt()? body_name[bdb->
                                                                              GetItem
                                                                              (TBD_BODY_SNAME)->
                                                                              GetInt()] : bdb->
                            GetItem(TBD_BODY_NAME)->GetString();
                        if (bname)
                            (mob_proto + imbs)->body[j].name = bname;
                        (mob_proto + imbs)->body[j].chance =
                            bdb->GetItem(TBD_BODY_CHANCE)->GetInt();
                        (mob_proto + imbs)->body[j].kdam = bdb->GetItem(TBD_BODY_KDAM)->GetInt();
                    }
                }
            }
        }
        //РЕАКЦИЯ
        if ((numadd = mob->GetItem(MOB_EVENT)->GetNumberItem()))
            for (j = 0; j < numadd; j++) {
                struct event_mob_data *e;
                CREATE(e, struct event_mob_data, 1);
                CItem *event = mob->GetItem(MOB_EVENT)->GetItem(j);

                e->command = event->GetItem(MOB_EVN_TYPE)->GetInt();
                e->argument = event->GetItem(MOB_EVN_ARG)->GetInt();
                e->script = event->GetItem(MOB_EVN_SCRIPT)->GetInt();
                e->next = (mob_proto + imbs)->event_mob;
                (mob_proto + imbs)->event_mob = e;
            }
        //КВЕСЫ
        if (mob->GetItem(MOB_WELCOME)->GetString())
            (mob_proto + imbs)->welcome = str_dup(mob->GetItem(MOB_WELCOME)->GetString());
        if (mob->GetItem(MOB_GOODBYE)->GetString())
            (mob_proto + imbs)->goodbye = str_dup(mob->GetItem(MOB_GOODBYE)->GetString());
        if ((numadd = mob->GetItem(MOB_QSCRIPT)->GetNumberItem())) {
            for (j = 0; j < numadd; j++) {
                struct mob_qscript_data *q;
                CItem *qscript = mob->GetItem(MOB_QSCRIPT)->GetItem(j);
                CREATE(q, struct mob_qscript_data, 1);

                q->number = qscript->GetItem(MOB_QSCR_NUMBER)->GetInt();
                q->text = str_dup(qscript->GetItem(MOB_QSCR_TEXT)->GetString());

                q->expr = new CExpression;
                qscript->GetItem(MOB_QSCR_EXPR)->ReleaseExpression(EXPR(q->expr));

                q->script = qscript->GetItem(MOB_QSCR_SCRIPT)->GetInt();
                q->next = (mob_proto + imbs)->qscripts;
                (mob_proto + imbs)->qscripts = q;
            }
        }

        if ((numadd = mob->GetItem(MOB_QUEST)->GetNumberItem())) {
            int numd, d;

            for (j = 0; j < numadd; j++) {
                struct mob_quest_data *q;
                CItem *quest = mob->GetItem(MOB_QUEST)->GetItem(j);

                q = new mob_quest_data();
                q->number = quest->GetItem(MOB_QNUMBER)->GetInt();
                q->multi = quest->GetItem(MOB_QMULTY)->GetInt();
                //УСЛОВИЯ
                q->expr = new CExpression;
                quest->GetItem(MOB_QEXPR)->ReleaseExpression(EXPR(q->expr));

                q->name = str_dup(quest->GetItem(MOB_QNAME)->GetString());
                q->pretext = str_dup(quest->GetItem(MOB_QPRE)->GetString());
                q->text = str_dup(quest->GetItem(MOB_QTEXT)->GetString());
                q->complite = str_dup(quest->GetItem(MOB_QCOMPLITE)->GetString());
                q->accept = quest->GetItem(MOB_QACCEPT)->GetInt();
                q->done = quest->GetItem(MOB_QDONE)->GetInt();

                if (quest->GetItem(MOB_QREQUEST)->GetNumberItem()) {
                    CItem *request = quest->GetItem(MOB_QREQUEST)->GetItem(0);

                    numd = request->GetItem(MOB_QR_OBJECTS)->GetStrListNumber();
                    for (d = 0; d < numd; d++) {
                        request->GetItem(MOB_QR_OBJECTS)->GetStrList(d, t[0], t[1]);
                        struct bug_abuse_data tmp;

                        tmp.no = t[0];
                        tmp.count = t[1];
                        q->objects.push_back(tmp);
                    }
                    numd = request->GetItem(MOB_QR_MOBILES)->GetStrListNumber();
                    for (d = 0; d < numd; d++) {
                        request->GetItem(MOB_QR_MOBILES)->GetStrList(d, t[0], t[1]);
                        struct bug_abuse_data tmp;

                        tmp.no = t[0];
                        tmp.count = t[1];
                        q->mobiles.push_back(tmp);
                    }
                    numd = request->GetItem(MOB_QR_FOLLOWERS)->GetStrListNumber();
                    for (d = 0; d < numd; d++) {
                        request->GetItem(MOB_QR_FOLLOWERS)->GetStrList(d, t[0], t[1]);
                        struct bug_abuse_data tmp;

                        tmp.no = t[0];
                        tmp.count = t[1];
                        q->followers.push_back(tmp);
                    }
                    numd = request->GetItem(MOB_QR_VAR)->GetNumberItem();
                    for (d = 0; d < numd; d++) {
                        CItem *qvar = request->GetItem(MOB_QR_VAR)->GetItem(d);
                        struct req_var_data tmp;

                        tmp.title = qvar->GetItem(MOB_QRV_TITLE)->GetString();
                        tmp.name = qvar->GetItem(MOB_QRV_NAME)->GetString();
                        tmp.value = qvar->GetItem(MOB_QRV_VALUE)->GetString();
                        q->req_var.push_back(tmp);
                    }

                }


                q->next = (mob_proto + imbs)->quests;
                (mob_proto + imbs)->quests = q;
            }
        }
        //Расчет параметров монстра
        int ia = 0, nn, j, gd = GET_GODS(mob_proto + imbs);

        for (int icls = 0; icls < NUM_CLASSES; icls++) {
            if ((mob_proto + imbs)->init_classes[icls] > 0) {
                for (nn = 0; nn <= (mob_proto + imbs)->init_classes[icls]; nn++)
                    GET_HLEVEL((mob_proto + imbs), nn + ia) = icls;
                ia = (mob_proto + imbs)->init_classes[icls];
            }
        }

        if (!mob->GetItem(MOB_SKILL)->GetStrListNumber())
            for (j = 0; j < MAX_SKILLS; j++)
                for (int icls = 0; icls < NUM_CLASSES; icls++)
                    if (skill_info[j].learn_level[icls][gd]
                        && ((mob_proto + imbs)->init_classes[icls] >=
                            skill_info[j].learn_level[icls][gd])) {
                        t[1] = calc_need_improove(mob_proto + imbs, GET_LEVEL(mob_proto + imbs));
                        t[1] = MIN(150, MAX(0, t[1]));
                        SET_SKILL(mob_proto + imbs, j) = t[1];
                    }

        int skl, sum = 0, koef = 0, level = GET_LEVEL(mob_proto + imbs);
        struct mob_spechit_data *h;

        if (GET_LEVEL(mob_proto + imbs) < 16)
            koef = 50;
        else if (GET_LEVEL(mob_proto + imbs) < 26)
            koef = 65;
        else
            koef = 70;

        GET_EXP(mob_proto + imbs) = (level_exp_mob(level + 1) - level_exp_mob(level)) / koef;
        for (skl = 0; skl <= MAX_SKILLS; skl++)
            if (SET_SKILL((mob_proto + imbs), skl))     //Доп процент за умение
                sum += 5;
        for (h = (mob_proto + imbs)->spec_hit; h; h = h->next)  //Доп процент за спецатаку
            sum += 10;
        if (num_spells)
            sum += num_spells;

        if (sum)
            GET_EXP(mob_proto + imbs) += (GET_EXP(mob_proto + imbs) * sum) / 100;

        /* СпецПроцедуры для моба */
        if (mob_proto[imbs].shop_data)
            spec_func_assign_mob(imbs, "shoper");

        //if (MOB_FLAGGED(mob_proto + imbs, MOB_RENT))
        //   spec_func_assign_mob(imbs, "receptionist");

        if (MOB_FLAGGED(mob_proto + imbs, MOB_POSTMAN))
            spec_func_assign_mob(imbs, "postmaster");

        if (MOB_FLAGGED(mob_proto + imbs, MOB_BANK))
            spec_func_assign_mob(imbs, "bank");

        if (MOB_FLAGGED(mob_proto + imbs, MOB_MASTER))
            spec_func_assign_mob(imbs, "guild");

        if (MOB_FLAGGED(mob_proto + imbs, MOB_TRAINER))
            spec_func_assign_mob(imbs, "trainer");

        /* Феня */
        mob_proto[imbs].linkWrapper();
        fenia_mark_guid(mob_proto[imbs].guid);

        top_of_mobt = imbs++;
    }
    delete Mob;
}

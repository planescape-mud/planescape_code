/**************************************************************************
    МПМ "Грани Мира" (с) 2002-2003 Андрей Ермишин
    Загрузка файлов игрового мира
 **************************************************************************/

#include "sysdep.h"
#include "structs.h"
#include "db.h"
#include "utils.h"
#include "handler.h"
#include "xwld.h"
#include "xboot.h"
#include "parser_id.h"
#include "constants.h"
#include "planescape.h"
#include "mudfilereader.h"
#include "mudfile.h"


void add_force_direction(struct room_data *rmm,
                         int room, int dir, int open, int close, int period, int dx, int dy, int dz,
                         int dtype, int stype, int save, char *mchar, char *mroom, char *exchar,
                         char *exroom, char *enchar, char *enroom, char *kchar, char *kroom);
void fenia_mark_guid(long long guid);


///////////////////////////////////////////////////////////////////////////////


int get_xrec_wld(void)
{
    Wld = new CWld;

    if (!Wld->Initialization())
        exit(1);

    MudFileReader reader(mud->wldDir);

    while (reader.hasNext())
        if (!Wld->ReadConfig(reader.next().getCPath()))
            exit(1);

    return Wld->GetNumberItem();
}

///////////////////////////////////////////////////////////////////////////////

void boot_wld(void)
{
    struct extra_descr_data *new_descr;
    struct db_load_data *new_db_data;
    struct hotel_data *new_hotel;
    int i, j, number, numadd, vnum;
    char buf[MAX_STRING_LENGTH];

    number = Wld->GetNumberItem();
    for (i = 0; i < number; i++) {
        //Загрузка параметров
        CItem *room = Wld->GetItem(i);

        vnum = room->GetItem(WLD_NUMBER)->GetInt();

        //Первичная инициализация
        new(&world[room_nr]) room_data(vnum);

        world[room_nr].number = vnum;
        world[room_nr].zone = real_zone_vnum(vnum / 100);
        //world[room_nr].zone = real_zone_vnum(room->GetItem(WLD_ZONE)->GetInt());
        if (!mud->modQuietLoad)
            log("ЗАГРУЗКА: Локация %d Область %d", vnum, world[room_nr].zone);

        //НАЗВАНИЕ
        CREATE(world[room_nr].name, char, strlen(room->GetItem(WLD_NAME)->GetString()) + 1);

        strcpy(world[room_nr].name, room->GetItem(WLD_NAME)->GetString());

        //ОПИСАНИЕ
        sprintf(buf, "   %s", room->GetItem(WLD_DESCRIPTION)->GetString());
        CREATE(world[room_nr].description, char, strlen(buf) + 1);

        strcpy(world[room_nr].description, buf);

        //ОПИСАНИЕ_НОЧЬ
        if (room->GetItem(WLD_DESCRIPTION_N)->GetString()) {
            sprintf(buf, "   %s", room->GetItem(WLD_DESCRIPTION_N)->GetString());
            CREATE(world[room_nr].description_night, char, strlen(buf) + 1);

            strcpy(world[room_nr].description_night, buf);
        }
        //ДОПОЛНИТЕЛЬНО
        numadd = room->GetItem(WLD_ADDITION)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            CItem *add_desc = room->GetItem(WLD_ADDITION)->GetItem(j);
            CREATE(new_descr, struct extra_descr_data, 1);

            CREATE(new_descr->keyword, char,
                   strlen(add_desc->GetItem(WLD_ADD_KEY)->GetString()) + 1);
            strcpy(new_descr->keyword, add_desc->GetItem(WLD_ADD_KEY)->GetString());
            CREATE(new_descr->description, char,
                   strlen(add_desc->GetItem(WLD_ADD_TEXT)->GetString()) + 1);
            strcpy(new_descr->description, add_desc->GetItem(WLD_ADD_TEXT)->GetString());

            new_descr->next = world[room_nr].ex_description;
            world[room_nr].ex_description = new_descr;
        }

        //КСВОЙСТВА
        if (room->GetItem(WLD_PROPERTIES)->GetString())
            asciiflag_conv((char *) room->GetItem(WLD_PROPERTIES)->GetString(),
                           &world[room_nr].room_flags);

        //МЕСТНОСТЬ
        world[room_nr].sector_type = room->GetItem(WLD_DISTRICT)->GetInt();

        //ПЕРЕХВАТ
        numadd = room->GetItem(WLD_INTERCEPTION)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            CItem *inter = room->GetItem(WLD_INTERCEPTION)->GetItem(j);

            add_message(world + room_nr, inter->GetItem(WLD_INT_COMMAND)->GetInt(),
                        inter->GetItem(WLD_INT_STOP)->GetInt(),
                        inter->GetItem(WLD_INT_SCRIPT)->GetInt(),
                        (char *) inter->GetItem(WLD_INT_MESSPLAYER)->GetString(),
                        (char *) inter->GetItem(WLD_INT_MESSVICTIM)->GetString(),
                        (char *) inter->GetItem(WLD_INT_MESSOTHER)->GetString(),
                        (char *) inter->GetItem(WLD_INT_MESSROOM)->GetString());
        }

        //ТЕЧЕНИЕ
        numadd = room->GetItem(WLD_FORCEDIR)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            int open = -1, close = -1, t[3], dt = 0, st = 0, save = 0;
            CItem *fd = room->GetItem(WLD_FORCEDIR)->GetItem(j);

            if (fd->GetItem(WLD_FD_TIME)->GetString())
                if (sscanf(fd->GetItem(WLD_FD_TIME)->GetString(), "%d-%d", &open, &close) != 2) {
                    log("ОШИБКА ЗАГРУЗКИ: Неправильно указно поле ТЕЧЕНИЕ.ВРЕМЯ");
                    exit(1);
                }
            if (fd->GetItem(WLD_FD_DAMAGE)->GetNumberItem()) {
                if (fd->GetItem(WLD_FD_DAMAGE)->GetItem(0)->GetItem(WLD_FD_DAM_FORCEDAMAGE)->
                    GetString())
                    if (sscanf
                        (fd->GetItem(WLD_FD_DAMAGE)->GetItem(0)->GetItem(WLD_FD_DAM_FORCEDAMAGE)->
                         GetString(), "%dd%d+%d", t, t + 1, t + 2) != 3) {
                        log("ОШИБКА ЗАГРУЗКИ: Неправильно указно поле ТЕЧЕНИЕ.ПОВРЕЖДЕНИЕ.СИЛА");
                        exit(1);
                    }
                dt = fd->GetItem(WLD_FD_DAMAGE)->GetItem(0)->GetItem(WLD_FD_DAM_TYPE)->GetInt();
                st = fd->GetItem(WLD_FD_DAMAGE)->GetItem(0)->GetItem(WLD_FD_DAM_SAVETYPE)->GetInt();
                save = fd->GetItem(WLD_FD_DAMAGE)->GetItem(0)->GetItem(WLD_FD_DAM_SAVE)->GetInt();
            }
            add_force_direction(&world[room_nr],
                                fd->GetItem(WLD_FD_ROOM)->GetInt(),
                                fd->GetItem(WLD_FD_DIR)->GetInt(), open, close,
                                fd->GetItem(WLD_FD_PERIOD)->GetInt(), t[0], t[1], t[2], dt, st,
                                save, fd->GetItem(WLD_FD_MESS_MCHAR)->GetString(),
                                fd->GetItem(WLD_FD_MESS_MROOM)->GetString(),
                                fd->GetItem(WLD_FD_MESS_EXCHAR)->GetString(),
                                fd->GetItem(WLD_FD_MESS_EXROOM)->GetString(),
                                fd->GetItem(WLD_FD_MESS_ENCHAR)->GetString(),
                                fd->GetItem(WLD_FD_MESS_ENROOM)->GetString(),
                                fd->GetItem(WLD_FD_MESS_KCHAR)->GetString(),
                                fd->GetItem(WLD_FD_MESS_KROOM)->GetString());
        }

        //ВЫХОД
        numadd = room->GetItem(WLD_EXIT)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            CItem *exit = room->GetItem(WLD_EXIT)->GetItem(j);
            int dir = exit->GetItem(WLD_EXIT_DIRECTION)->GetInt();

            if (!world[room_nr].dir_option[dir])
                world[room_nr].dir_option[dir] = new room_direction_data();

            world[room_nr].dir_option[dir]->to_room = exit->GetItem(WLD_EXIT_ROOMNUMBER)->GetInt();

            if (exit->GetItem(WLD_EXIT_DESCRIPTION)->GetString()) {
                CREATE(world[room_nr].dir_option[dir]->general_description, char,
                       strlen(exit->GetItem(WLD_EXIT_DESCRIPTION)->GetString()) + 1);
                strcpy(world[room_nr].dir_option[dir]->general_description,
                       exit->GetItem(WLD_EXIT_DESCRIPTION)->GetString());
            }

            if (exit->GetItem(WLD_EXIT_ALIAS)->GetString()) {
                CREATE(world[room_nr].dir_option[dir]->keyword, char,
                       strlen(exit->GetItem(WLD_EXIT_ALIAS)->GetString()) + 1);
                strcpy(world[room_nr].dir_option[dir]->keyword,
                       exit->GetItem(WLD_EXIT_ALIAS)->GetString());
            }

            if (exit->GetItem(WLD_EXIT_KEY)->GetInt())
                world[room_nr].dir_option[dir]->key = exit->GetItem(WLD_EXIT_KEY)->GetInt();

            world[room_nr].dir_option[dir]->sex = exit->GetItem(WLD_EXIT_SEX)->GetInt();

            if (exit->GetItem(WLD_EXIT_NAME)->GetString()) {
                strcpy(buf, exit->GetItem(WLD_EXIT_NAME)->GetString());
                CREATE(world[room_nr].dir_option[dir]->exit_name, char, strlen(buf) + 1);

                strcpy(world[room_nr].dir_option[dir]->exit_name, buf);
            }

            if (exit->GetItem(WLD_EXIT_PROPERTIES)->GetString())
                asciiflag_conv((char *) exit->GetItem(WLD_EXIT_PROPERTIES)->GetString(),
                               &world[room_nr].dir_option[dir]->exit_data);

            if (exit->GetItem(WLD_EXIT_QUALITY)->GetInt())
                world[room_nr].dir_option[dir]->quality = exit->GetItem(WLD_EXIT_QUALITY)->GetInt();

            //ВЫХОД.ЛОВУШКА
            if (exit->GetItem(WLD_EXIT_TRAP)->GetNumberItem()) {
                CItem *etrap = exit->GetItem(WLD_EXIT_TRAP)->GetItem(0);        //всегда первая и единственная запись

                world[room_nr].dir_option[dir]->shance =
                    etrap->GetItem(WLD_EXIT_TRAP_CHANCE)->GetInt();

                if (etrap->GetItem(WLD_EXIT_TRAP_TYPEDAMAGE)->GetInt())
                    world[room_nr].dir_option[dir]->type_hit =
                        etrap->GetItem(WLD_EXIT_TRAP_TYPEDAMAGE)->GetInt();


                if (etrap->GetItem(WLD_EXIT_TRAP_FORCEDAMAGE)->GetString()) {
                    int t[3];

                    if (sscanf
                        (etrap->GetItem(WLD_EXIT_TRAP_FORCEDAMAGE)->GetString(), "%dd%d+%d", t,
                         t + 1, t + 2) == 3) {
                        world[room_nr].dir_option[dir]->damnodice = t[0];
                        world[room_nr].dir_option[dir]->damsizedice = t[1];
                        world[room_nr].dir_option[dir]->damage = t[2];
                    }
                }

                if (etrap->GetItem(WLD_EXIT_TRAP_SAVE)->GetInt())
                    world[room_nr].dir_option[dir]->save =
                        etrap->GetItem(WLD_EXIT_TRAP_SAVE)->GetInt();

                if (etrap->GetItem(WLD_EXIT_TRAP_MESS_CHAR)->GetString()) {
                    CREATE(world[room_nr].dir_option[dir]->trap_damage_char, char,
                           strlen(etrap->GetItem(WLD_EXIT_TRAP_MESS_CHAR)->GetString()) + 1);
                    strcpy(world[room_nr].dir_option[dir]->trap_damage_char,
                           etrap->GetItem(WLD_EXIT_TRAP_MESS_CHAR)->GetString());
                }

                if (etrap->GetItem(WLD_EXIT_TRAP_MESS_ROOM)->GetString()) {
                    CREATE(world[room_nr].dir_option[dir]->trap_damage_room, char,
                           strlen(etrap->GetItem(WLD_EXIT_TRAP_MESS_ROOM)->GetString()) + 1);
                    strcpy(world[room_nr].dir_option[dir]->trap_damage_room,
                           etrap->GetItem(WLD_EXIT_TRAP_MESS_ROOM)->GetString());
                }

                if (etrap->GetItem(WLD_EXIT_TRAP_MESS_SCHAR)->GetString()) {
                    CREATE(world[room_nr].dir_option[dir]->trap_nodamage_char, char,
                           strlen(etrap->GetItem(WLD_EXIT_TRAP_MESS_SCHAR)->GetString()) + 1);
                    strcpy(world[room_nr].dir_option[dir]->trap_nodamage_char,
                           etrap->GetItem(WLD_EXIT_TRAP_MESS_SCHAR)->GetString());
                }

                if (etrap->GetItem(WLD_EXIT_TRAP_MESS_SROOM)->GetString()) {
                    CREATE(world[room_nr].dir_option[dir]->trap_nodamage_room, char,
                           strlen(etrap->GetItem(WLD_EXIT_TRAP_MESS_SROOM)->GetString()) + 1);
                    strcpy(world[room_nr].dir_option[dir]->trap_nodamage_room,
                           etrap->GetItem(WLD_EXIT_TRAP_MESS_SROOM)->GetString());
                }

                if (etrap->GetItem(WLD_EXIT_TRAP_MESS_KCHAR)->GetString()) {
                    CREATE(world[room_nr].dir_option[dir]->trap_kill_char, char,
                           strlen(etrap->GetItem(WLD_EXIT_TRAP_MESS_KCHAR)->GetString()) + 1);
                    strcpy(world[room_nr].dir_option[dir]->trap_kill_char,
                           etrap->GetItem(WLD_EXIT_TRAP_MESS_KCHAR)->GetString());
                }

                if (etrap->GetItem(WLD_EXIT_TRAP_MESS_KROOM)->GetString()) {
                    CREATE(world[room_nr].dir_option[dir]->trap_kill_room, char,
                           strlen(etrap->GetItem(WLD_EXIT_TRAP_MESS_KROOM)->GetString()) + 1);
                    strcpy(world[room_nr].dir_option[dir]->trap_kill_room,
                           etrap->GetItem(WLD_EXIT_TRAP_MESS_KROOM)->GetString());
                }

            }                   // конец ВЫХОД.ЛОВУШКА

            //Бэкап
            world[room_nr].dir_option[dir]->exit_data_reset =
                world[room_nr].dir_option[dir]->exit_data;
        }                       //конец ВЫХОД

        //ПОВРЕЖДЕНИЯ
        if (room->GetItem(WLD_DAMAGE)->GetNumberItem()) {
            CItem *damage = room->GetItem(WLD_DAMAGE)->GetItem(0);
            CREATE(world[room_nr].damage, struct room_damage_data, 1);

            //world[room_nr].damage->chance = damage->GetItem(WLD_DAM_CHANCE)->GetInt();
            world[room_nr].damage->chance = 15;
            world[room_nr].damage->type_hit = damage->GetItem(WLD_DAM_TYPEDAMAGE)->GetInt();
            if (damage->GetItem(WLD_DAM_SAVE)->GetInt())
                world[room_nr].damage->save = damage->GetItem(WLD_DAM_SAVE)->GetInt();

            asciiflag_conv((char *) damage->GetItem(WLD_DAM_TYPE)->GetString(),
                           &world[room_nr].damage->type);

            if (damage->GetItem(WLD_DAM_FORCEDAMAGE)->GetString()) {
                int t[3];

                if (sscanf
                    (damage->GetItem(WLD_DAM_FORCEDAMAGE)->GetString(), "%dd%d+%d", t, t + 1,
                     t + 2) == 3) {
                    world[room_nr].damage->damnodice = t[0];
                    world[room_nr].damage->damsizedice = t[1];
                    world[room_nr].damage->damage = t[2];
                }
            }
            if (damage->GetItem(WLD_DAM_MESS_CHAR)->GetString()) {
                CREATE(world[room_nr].damage->damage_mess_char, char,
                       strlen(damage->GetItem(WLD_DAM_MESS_CHAR)->GetString()) + 1);
                strcpy(world[room_nr].damage->damage_mess_char,
                       damage->GetItem(WLD_DAM_MESS_CHAR)->GetString());
            }

            if (damage->GetItem(WLD_DAM_MESS_ROOM)->GetString()) {
                CREATE(world[room_nr].damage->damage_mess_room, char,
                       strlen(damage->GetItem(WLD_DAM_MESS_ROOM)->GetString()) + 1);
                strcpy(world[room_nr].damage->damage_mess_room,
                       damage->GetItem(WLD_DAM_MESS_ROOM)->GetString());
            }
            if (damage->GetItem(WLD_DAM_MESS_SCHAR)->GetString()) {
                CREATE(world[room_nr].damage->nodamage_mess_char, char,
                       strlen(damage->GetItem(WLD_DAM_MESS_SCHAR)->GetString()) + 1);
                strcpy(world[room_nr].damage->nodamage_mess_char,
                       damage->GetItem(WLD_DAM_MESS_SCHAR)->GetString());
            }
            if (damage->GetItem(WLD_DAM_MESS_SROOM)->GetString()) {
                CREATE(world[room_nr].damage->nodamage_mess_room, char,
                       strlen(damage->GetItem(WLD_DAM_MESS_SROOM)->GetString()) + 1);
                strcpy(world[room_nr].damage->nodamage_mess_room,
                       damage->GetItem(WLD_DAM_MESS_SROOM)->GetString());
            }
        }                       //конец ПОВРЕЖДЕНИЯ

        //ЛОВУШКА
        numadd = room->GetItem(WLD_TRAP)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            CItem *trap = room->GetItem(WLD_TRAP)->GetItem(j);

            //int dir = room->GetItem(WLD_EXIT)->GetItem(j)->GetItem(WLD_EXIT_DIRECTION)->GetInt();
            int dir = trap->GetItem(WLD_TRAP_DIRECTION)->GetInt();

            if (!world[room_nr].trap_option[dir])
                CREATE(world[room_nr].trap_option[dir], struct room_trap_data, 1);

            world[room_nr].trap_option[dir]->direction =
                trap->GetItem(WLD_TRAP_DIRECTION)->GetInt();
            world[room_nr].trap_option[dir]->chance = trap->GetItem(WLD_TRAP_CHANCE)->GetInt();
            world[room_nr].trap_option[dir]->type_hit =
                trap->GetItem(WLD_TRAP_TYPEDAMAGE)->GetInt();
            if (trap->GetItem(WLD_TRAP_SAVE)->GetInt())
                world[room_nr].trap_option[dir]->save = trap->GetItem(WLD_TRAP_SAVE)->GetInt();

            world[room_nr].trap_option[dir]->type = trap->GetItem(WLD_TRAP_TYPE)->GetInt();

            if (trap->GetItem(WLD_TRAP_FORCEDAMAGE)->GetString()) {
                int t[3];

                if (sscanf
                    (trap->GetItem(WLD_TRAP_FORCEDAMAGE)->GetString(), "%dd%d+%d", t, t + 1,
                     t + 2) == 3) {
                    world[room_nr].trap_option[dir]->damnodice = t[0];
                    world[room_nr].trap_option[dir]->damsizedice = t[1];
                    world[room_nr].trap_option[dir]->damage = t[2];
                }
            }
            if (trap->GetItem(WLD_TRAP_MESS_CHAR)->GetString()) {
                CREATE(world[room_nr].trap_option[dir]->damage_mess_char, char,
                       strlen(trap->GetItem(WLD_TRAP_MESS_CHAR)->GetString()) + 1);
                strcpy(world[room_nr].trap_option[dir]->damage_mess_char,
                       trap->GetItem(WLD_TRAP_MESS_CHAR)->GetString());
            }
            if (trap->GetItem(WLD_TRAP_MESS_ROOM)->GetString()) {
                CREATE(world[room_nr].trap_option[dir]->damage_mess_room, char,
                       strlen(trap->GetItem(WLD_TRAP_MESS_ROOM)->GetString()) + 1);
                strcpy(world[room_nr].trap_option[dir]->damage_mess_room,
                       trap->GetItem(WLD_TRAP_MESS_ROOM)->GetString());
            }
            if (trap->GetItem(WLD_TRAP_MESS_SCHAR)->GetString()) {
                CREATE(world[room_nr].trap_option[dir]->nodamage_mess_char, char,
                       strlen(trap->GetItem(WLD_TRAP_MESS_SCHAR)->GetString()) + 1);
                strcpy(world[room_nr].trap_option[dir]->nodamage_mess_char,
                       trap->GetItem(WLD_TRAP_MESS_SCHAR)->GetString());
            }
            if (trap->GetItem(WLD_TRAP_MESS_SROOM)->GetString()) {
                CREATE(world[room_nr].trap_option[dir]->nodamage_mess_room, char,
                       strlen(trap->GetItem(WLD_TRAP_MESS_SROOM)->GetString()) + 1);
                strcpy(world[room_nr].trap_option[dir]->nodamage_mess_room,
                       trap->GetItem(WLD_TRAP_MESS_SROOM)->GetString());
            }

            if (trap->GetItem(WLD_TRAP_MESS_KCHAR)->GetString()) {
                CREATE(world[room_nr].trap_option[dir]->kill_mess_char, char,
                       strlen(trap->GetItem(WLD_TRAP_MESS_KCHAR)->GetString()) + 1);
                strcpy(world[room_nr].trap_option[dir]->kill_mess_char,
                       trap->GetItem(WLD_TRAP_MESS_KCHAR)->GetString());
            }
            if (trap->GetItem(WLD_TRAP_MESS_KROOM)->GetString()) {
                CREATE(world[room_nr].trap_option[dir]->kill_mess_room, char,
                       strlen(trap->GetItem(WLD_TRAP_MESS_KROOM)->GetString()) + 1);
                strcpy(world[room_nr].trap_option[dir]->kill_mess_room,
                       trap->GetItem(WLD_TRAP_MESS_KROOM)->GetString());
            }

            if (trap->GetItem(WLD_TRAP_MESS_ACT_C)->GetString()) {
                CREATE(world[room_nr].trap_option[dir]->trap_active_char, char,
                       strlen(trap->GetItem(WLD_TRAP_MESS_ACT_C)->GetString()) + 1);
                strcpy(world[room_nr].trap_option[dir]->trap_active_char,
                       trap->GetItem(WLD_TRAP_MESS_ACT_C)->GetString());
            }
            if (trap->GetItem(WLD_TRAP_MESS_ACT_R)->GetString()) {
                CREATE(world[room_nr].trap_option[dir]->trap_active_room, char,
                       strlen(trap->GetItem(WLD_TRAP_MESS_ACT_R)->GetString()) + 1);
                strcpy(world[room_nr].trap_option[dir]->trap_active_room,
                       trap->GetItem(WLD_TRAP_MESS_ACT_R)->GetString());
            }


        }                       //конец ЛОВУШКА

        //ПОРТАЛЫ
        numadd = room->GetItem(WLD_PORTAL)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            CItem *portal = room->GetItem(WLD_PORTAL)->GetItem(j);
            int dir = portal->GetItem(WLD_PORTAL_DIRECTION)->GetInt();

            if (!world[room_nr].dir_option[dir]) {
                world[room_nr].dir_option[dir] = new room_direction_data();
                world[room_nr].dir_option[dir]->to_room = -1;
            }

            world[room_nr].dir_option[dir]->type_port = portal->GetItem(WLD_PORTAL_TYPE)->GetInt();
            world[room_nr].dir_option[dir]->key_port = portal->GetItem(WLD_PORTAL_KEY)->GetInt();
            world[room_nr].dir_option[dir]->room_port = portal->GetItem(WLD_PORTAL_ROOM)->GetInt();
            world[room_nr].dir_option[dir]->time = portal->GetItem(WLD_PORTAL_TIME)->GetInt();

            if (portal->GetItem(WLD_PORTAL_DESCRIPTION)->GetString()) {
                CREATE(world[room_nr].dir_option[dir]->portal_description, char,
                       strlen(portal->GetItem(WLD_PORTAL_DESCRIPTION)->GetString()) + 1);
                strcpy(world[room_nr].dir_option[dir]->portal_description,
                       portal->GetItem(WLD_PORTAL_DESCRIPTION)->GetString());
            }
            if (portal->GetItem(WLD_PORTAL_ACTIVE)->GetString()) {
                CREATE(world[room_nr].dir_option[dir]->mess_to_open, char,
                       strlen(portal->GetItem(WLD_PORTAL_ACTIVE)->GetString()) + 1);
                strcpy(world[room_nr].dir_option[dir]->mess_to_open,
                       portal->GetItem(WLD_PORTAL_ACTIVE)->GetString());
            }
            if (portal->GetItem(WLD_PORTAL_DEACTIVE)->GetString()) {
                CREATE(world[room_nr].dir_option[dir]->mess_to_close, char,
                       strlen(portal->GetItem(WLD_PORTAL_DEACTIVE)->GetString()) + 1);
                strcpy(world[room_nr].dir_option[dir]->mess_to_close,
                       portal->GetItem(WLD_PORTAL_DEACTIVE)->GetString());
            }
            if (portal->GetItem(WLD_PORTAL_MESS_CHAR)->GetString()) {
                CREATE(world[room_nr].dir_option[dir]->mess_char_enter, char,
                       strlen(portal->GetItem(WLD_PORTAL_MESS_CHAR)->GetString()) + 1);
                strcpy(world[room_nr].dir_option[dir]->mess_char_enter,
                       portal->GetItem(WLD_PORTAL_MESS_CHAR)->GetString());
            }

            if (portal->GetItem(WLD_PORTAL_MESS_ROOM)->GetString()) {
                CREATE(world[room_nr].dir_option[dir]->mess_room_enter, char,
                       strlen(portal->GetItem(WLD_PORTAL_MESS_ROOM)->GetString()) + 1);
                strcpy(world[room_nr].dir_option[dir]->mess_room_enter,
                       portal->GetItem(WLD_PORTAL_MESS_ROOM)->GetString());
            }

        }

        //МОНСТРЫ
        numadd = room->GetItem(WLD_MOBILE)->GetStrListNumber();
        for (j = 0; j < numadd; j++) {
            int vnum, chance;
            CREATE(new_db_data, struct db_load_data, 1);

            room->GetItem(WLD_MOBILE)->GetStrList(j, vnum, chance);
            new_db_data->vnum = vnum;
            new_db_data->chance = chance;
            new_db_data->next = world[room_nr].mobiles;
            world[room_nr].mobiles = new_db_data;
        }

        //ПРЕДМЕТЫ
        numadd = room->GetItem(WLD_OBJECT)->GetStrListNumber();
        for (j = 0; j < numadd; j++) {
            int vnum, chance;
            CREATE(new_db_data, struct db_load_data, 1);

            room->GetItem(WLD_OBJECT)->GetStrList(j, vnum, chance);
            new_db_data->vnum = vnum;
            new_db_data->chance = chance;
            new_db_data->next = world[room_nr].objects;
            world[room_nr].objects = new_db_data;
        }

        //ПЕРИОД
        numadd = room->GetItem(WLD_PERIOD)->GetNumberItem();
        for (j = 0; j < numadd; j++) {
            CItem *prd = room->GetItem(WLD_PERIOD)->GetItem(j);
            struct room_period_data *p;
            CREATE(p, struct room_period_data, 1);

            p->start = prd->GetItem(WLD_PRD_START)->GetInt();
            p->stop = prd->GetItem(WLD_PRD_STOP)->GetInt();
            p->weather = prd->GetItem(WLD_PRD_WEATHER)->GetInt();
            p->object = prd->GetItem(WLD_PRD_OBJECT)->GetInt();
            p->monster = prd->GetItem(WLD_PRD_MONSTER)->GetInt();
            p->start_room = str_dup(prd->GetItem(WLD_PRD_SRLOCATION)->GetString());
            p->stop_room = str_dup(prd->GetItem(WLD_PRD_SPLOCATION)->GetString());
            p->start_zone = str_dup(prd->GetItem(WLD_PRD_SRZONE)->GetString());
            p->stop_zone = str_dup(prd->GetItem(WLD_PRD_SPZONE)->GetString());
            p->next = world[room_nr].period;
            world[room_nr].period = p;
        }

        //ГОСТИНИЦА
        if (room->GetItem(WLD_HOTEL)->GetNumberItem()) {
            CItem *hotel = room->GetItem(WLD_HOTEL)->GetItem(0);
            CREATE(new_hotel, struct hotel_data, 1);

            new_hotel->type = hotel->GetItem(WLD_HOTEL_TYPE)->GetInt();
            new_hotel->master = hotel->GetItem(WLD_HOTEL_MASTER)->GetInt();

            if (hotel->GetItem(WLD_HOTEL_CHAR)->GetString()) {
                CREATE(new_hotel->MessChar, char,
                       strlen(hotel->GetItem(WLD_HOTEL_CHAR)->GetString()) + 1);
                strcpy(new_hotel->MessChar, hotel->GetItem(WLD_HOTEL_CHAR)->GetString());
            }
            if (hotel->GetItem(WLD_HOTEL_ROOM)->GetString()) {
                CREATE(new_hotel->MessRoom, char,
                       strlen(hotel->GetItem(WLD_HOTEL_ROOM)->GetString()) + 1);
                strcpy(new_hotel->MessRoom, hotel->GetItem(WLD_HOTEL_ROOM)->GetString());
            }
            if (hotel->GetItem(WLD_HOTEL_RETURN)->GetString()) {
                CREATE(new_hotel->MessReturn, char,
                       strlen(hotel->GetItem(WLD_HOTEL_RETURN)->GetString()) + 1);
                strcpy(new_hotel->MessReturn, hotel->GetItem(WLD_HOTEL_RETURN)->GetString());
            }

            world[room_nr].hotel = new_hotel;
        }

        /* СпецПроцедуры для комнат */
        if (ROOM_FLAGGED(room_nr, ROOM_DEATH))
            spec_func_assign_room(room_nr, "dump");

        if (world[room_nr].hotel)
            spec_func_assign_room(room_nr, "hotel");

        /* Феня */
        world[room_nr].linkWrapper();
        fenia_mark_guid(world[room_nr].guid);

        top_of_world = room_nr++;
    }

}


void create_virtual_rooms(void)
{
    int i, last_vnum;

    for (i = 0; i <= top_of_zone_table; i++) {
        last_vnum = zone_table[i].top;

        if (real_room(last_vnum) > 0)
            continue;

        new(&world[room_nr]) room_data(last_vnum);
        world[room_nr].zone = i;
        world[room_nr].number = last_vnum;
        world[room_nr].name = str_dup("Виртуальная комната");
        world[room_nr].description = str_dup("Похоже, здесь Вам делать нечего.");
        world[room_nr].description_night = str_dup("Похоже, здесь Вам делать нечего.");
        world[room_nr].sector_type = SECT_SECRET;

        world[room_nr].linkWrapper();
        fenia_mark_guid(world[room_nr].guid);
        top_of_world = room_nr++;
    }

}

void count_maxfactor_mob(void)
{
    long i, numbr;
    int j, numadd, vnum, chance, rnum;

    numbr = Wld->GetNumberItem();
    for (i = 0; i < numbr; i++) {
        CItem *room = Wld->GetItem(i);

        //МОНСТРЫ
        numadd = room->GetItem(WLD_MOBILE)->GetStrListNumber();
        for (j = 0; j < numadd; j++) {
            room->GetItem(WLD_MOBILE)->GetStrList(j, vnum, chance);
            rnum = real_mobile(vnum);
            if (rnum < 0) {

                log("ОШИБКА: Неопознаный монстр в локации #%d",
                    room->GetItem(WLD_NUMBER)->GetInt());
                continue;
            }
            mob_proto[rnum].npc()->specials.MaxFactor++;
        }
    }

    delete Wld;
}

void count_key_use(void)
{
    long count;
    int door;
    int rnum = -1;

    for (count = 0; count <= top_of_world; count++)
        for (door = 0; door < NUM_OF_DIRS; door++) {
            if (world[count].dir_option[door]) {
                if (world[count].dir_option[door]->key <= 0)
                    continue;
                rnum = real_object(world[count].dir_option[door]->key);
                if (rnum < 0) {
                    log("ОШИБКА: Неизвестный ключ #%d в локации #%d направление '%s'.",
                        world[count].dir_option[door]->key, world[count].number, DirIs[door]);
                    continue;
                }
                if (GET_OBJ_TYPE(&obj_proto[rnum]) == ITEM_KEY)
                    GET_OBJ_VAL(&obj_proto[rnum], 1)++;
            }
        }

    for (count = 0; count <= top_of_objt; count++) {
        if (GET_OBJ_TYPE(&obj_proto[count]) == ITEM_CONTAINER) {
            if (GET_OBJ_VAL(&obj_proto[count], 2) <= 0)
                continue;
            rnum = real_object(GET_OBJ_VAL(&obj_proto[count], 2));
            if (rnum < 0) {
                log("ОШИБКА: Неизвестный ключ #%d в контейнере '%s'",
                    GET_OBJ_VAL(&obj_proto[count], 2), GET_OBJ_PNAME(&obj_proto[count], 0));
                continue;
            }
            if (GET_OBJ_TYPE(&obj_proto[rnum]) == ITEM_KEY)
                GET_OBJ_VAL(&obj_proto[rnum], 1)++;
        }
    }
}


void GetWldMessage(int room, struct P_message &pMess)
{
    pMess.valid = false;

    if (room == NOWHERE)
        return;

    if (!world[room].damage)
        return;

    pMess.valid = true;

    pMess.mChar = NULL;
    pMess.mVict = world[room].damage->nodamage_mess_char;
    pMess.mRoom = world[room].damage->nodamage_mess_room;

    pMess.hChar = NULL;
    pMess.hVict = world[room].damage->damage_mess_char;
    pMess.hRoom = world[room].damage->damage_mess_room;

    pMess.kChar = NULL;
    pMess.kVict = world[room].damage->damage_mess_char;
    pMess.kRoom = world[room].damage->damage_mess_room;

    pMess.pChar = NULL;
    pMess.pVict = world[room].damage->nodamage_mess_char;
    pMess.pRoom = world[room].damage->nodamage_mess_room;

    pMess.aChar = NULL;
    pMess.aVict = world[room].damage->nodamage_mess_char;
    pMess.aRoom = world[room].damage->nodamage_mess_room;

    pMess.bChar = NULL;
    pMess.bVict = world[room].damage->nodamage_mess_char;
    pMess.bRoom = world[room].damage->nodamage_mess_room;
}

/*******************************************************************************/
void add_force_direction(struct room_data *rmm,
                         int room, int dir, int open, int close, int period, int dx, int dy, int dz,
                         int dtype, int stype, int save, char *mchar, char *mroom, char *exchar,
                         char *exroom, char *enchar, char *enroom, char *kchar, char *kroom)
{
    struct room_forcedir_data *fd = NULL;

    CREATE(fd, struct room_forcedir_data, 1);

    fd->vnum = room;
    fd->dir = dir;
    fd->open = open;
    fd->close = close;
    fd->timer = period;
    fd->damage_type = dtype;
    fd->damnodice = dx;
    fd->damsizedice = dy;
    fd->damage = dz;
    fd->save = save;
    fd->save_type = stype;
    fd->mess_fail_char = str_dup(mchar);
    fd->mess_fail_room = str_dup(mroom);
    fd->mess_exit_char = str_dup(exchar);
    fd->mess_exit_room = str_dup(exroom);
    fd->mess_enter_char = str_dup(enchar);
    fd->mess_enter_room = str_dup(enroom);
    fd->mess_kill_char = str_dup(kchar);
    fd->mess_kill_room = str_dup(kroom);
    fd->next = rmm->forcedir;

    rmm->forcedir = fd;

}


///////////////////////////////////////////////////////////////////////////////
//world loot система (/lib/misc/world_loot)


void load_world_loot(void)
{

    if (!Wlt.Initialization())
        exit(1);

    if (!Wlt.ReadConfig(ShareFile(mud->worldLootFile).getCPath())) {
        log("ПРЕДУПРЕЖДЕНИЕ: Отсутствует  файл мирового лута.");
        return;
    }
//Wlt.GetNumberItem();

}



std::vector < int >check_world_loot(int vnum_mob)
{
    int rnum = real_mobile(vnum_mob);
    int nums, i, minl = 0, maxl = 0;
    struct char_data *mob;
    char slevel[255];
    std::vector < int >loot;
    bool race = TRUE, type = TRUE, god = TRUE, clas = TRUE, sex = TRUE, age = TRUE;

    *slevel = '\0';

    if (rnum < 0)
        return (loot);

    mob = mob_proto + rnum;

    nums = Wlt.GetNumberItem();
    for (i = 0; i < nums; i++) {
        CItem *xwlt = Wlt.GetItem(i);

        //Сначала обязательно проверяем уровни
        if (xwlt->GetItem(WRL_LEVEL)->GetString() &&
            sscanf(xwlt->GetItem(WRL_LEVEL)->GetString(), "%d-%d", &minl, &maxl) == 2) {
            if (GET_LEVEL(mob) >= minl && GET_LEVEL(mob) <= maxl) {     //уровень
                if (xwlt->GetItem(WRL_RACE)->GetInt() != RACE_NONE
                    && GET_RACE(mob) != xwlt->GetItem(WRL_RACE)->GetInt())
                    race = FALSE;

                if (xwlt->GetItem(WRL_TYPE)->GetInt() != TMOB_NONE
                    && GET_MOB_TYPE(mob) != xwlt->GetItem(WRL_TYPE)->GetInt())
                    type = FALSE;

                if (xwlt->GetItem(WRL_SEX)->GetInt() != SEX_NONE
                    && GET_SEX(mob) != xwlt->GetItem(WRL_SEX)->GetInt())
                    sex = FALSE;

                /*if (wxlt->GetItem(WRL_AGE)->GetInt() != AGE_NONE && GET_AGE(mob) != wxlt->GetItem(WRL_AGE)->GetInt())
                   age = FALSE; */


                if (race && type && god && clas && sex && age)
                    if (xwlt->GetItem(WRL_SHANCE)->GetInt() >= number(1, 1000))
                        loot.push_back(xwlt->GetItem(WRL_OBJECT)->GetInt());
            }
        }
    }

    return (loot);
}

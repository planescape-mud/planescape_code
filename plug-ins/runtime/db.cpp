/* ************************************************************************
*   File: db.c                                          Part of CircleMUD *
*  Usage: Loading/saving chars, booting/resetting world, internal funcs   *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#include "sysdep.h"
#include "profiler.h"

#include "structs.h"
#include "utils.h"
#include "db.h"
#include "ban.h"
#include "help.h"
#include "comm.h"
#include "iptoaddr.h"
#include "shop.h"
#include "handler.h"
#include "act.social.h"
#include "case.h"
#include "interpreter.h"
#include "spells.h"
#include "mail.h"
#include "interpreter.h"
#include "house.h"
#include "constants.h"
#include "mobmax.h"
#include "pk.h"
#include "diskio.h"
#include "parser.h"
#include "parser_const.h"
#include "xspells.h"
#include "xbody.h"
#include "xboot.h"
#include "events.h"
#include "xenchant.h"
#include "planescape.h"
#include "dlfileop.h"
#include "mudstats.h"
#include "textfileloader.h"
#include "mudfile.h"
#include "mudpluginmanager.h"


/* local functions */
void load_container(struct obj_data *obj);
int check_object_spell_number(struct obj_data *obj, int val);
int check_object_level(struct obj_data *obj, int val);
void index_boot(int mode);
int check_object(struct obj_data *);
void build_player_index(void);
int is_empty(zone_rnum zone_nr);
void boot_world(void);
int count_social_records(FILE * fl, int *messages, int *keywords);
void asciiflag_conv(const char *flag, void *value);
void check_start_rooms(void);
void renum_world(void);
void renum_zone_table(void);
void log_zone_error(zone_rnum zone, int cmd_no, const char *message);
void reset_time(void);
long get_ptable_by_name(char *name);
int mobs_in_room(int m_num, int r_num);
void new_build_player_index(void);
void new_save_char(struct char_data *ch, room_rnum load_room);
void load_skills(void);
void make_maze(int zone);

int hsort(const void *a, const void *b);
int csort(const void *a, const void *b);


#define READ_SIZE 256

/* Separate a 4-character id tag from the data it precedes */
void tag_argument(char *argument, char *tag)
{
    char *tmp = argument, *ttag = tag, *wrt = argument;
    int i;

    for (i = 0; i < 4; i++)
        *(ttag++) = *(tmp++);
    *ttag = '\0';

    while (*tmp == ':' || *tmp == ' ')
        tmp++;

    while (*tmp)
        *(wrt++) = *(tmp++);
    *wrt = '\0';
}


/*************************************************************************
*  routines for booting the system                                       *
*************************************************************************/

/*
 * Too bad it doesn't check the return values to let the user
 * know about -1 values.  This will result in an 'Okay.' to a
 * 'reload' command even when the string was not replaced.
 * To fix later, if desired. -gg 6/24/99
 */
ACMD(do_reboot)
{
    char arg[MAX_STRING_LENGTH];

    one_argument(argument, arg);

    if (!*arg || !str_cmp(arg, "?")) {
        send_to_char("Usage: reload texts | skills | adverbs | socials | helps | plugins\r\n", ch);
        return;
    }

    if (!str_cmp(arg, "all") || *arg == '*' || !str_cmp(arg, "texts")) {
        mud->getTextFileLoader()->loadAll();
    } else if (!str_cmp(arg, "skills") || !str_cmp(arg, "умения")) {
        load_skills();
    } else if (!str_cmp(arg, "helps")) {
        help_destroy();
        help_init();
    } else if (!str_cmp(arg, "наречия") || !str_cmp(arg, "adverbs")) {
        adverb_destroy();
        adverb_init();
    } else if (!str_cmp(arg, "socials")) {
        socials_destroy();
        socials_init();
    } else if (!str_cmp(arg, "плагины") || !str_cmp(arg, "plugins")) {
        send_to_char("Плагины будут перезагружены.\r\n", ch);
        PluginManager::getThis()->setReloadAllRequest();
        return;
    } else {
        send_to_char("Unknown reload option. Use 'reload ?' for help.\r\n", ch);
        return;
    }
    send_to_char(OK, ch);
}

void boot_world(void)
{

    log("Определение размера таблицы зон.");
    index_boot(DB_BOOT_ZON);
    log("Загружаю зоны в xformatе");
    boot_zon();

    log("Определение размера таблицы локаций.");
    index_boot(DB_BOOT_WLD);
    log("Загружаю локации в xformatе");
    boot_wld();
    create_virtual_rooms();


    log("Перенумерация локаций.");
    renum_world();

    log("Проверка стартовых комнат.");
    check_start_rooms();


    log("Загрузка объектов и генерация индекса.");
    index_boot(DB_BOOT_OBJ);
    log("Загружаю предметы в xformatе");
    boot_obj();

    log("Загрузка монстров и генерация индекса");
    index_boot(DB_BOOT_MOB);
    log("Загружаю монстров в xformatе");
    boot_mob();


    log("Установка уровня монстров");
    mob_lev_count();

    log("Установка предела монстров");
    count_maxfactor_mob();

    log("Расчет количества применений ключей.");
    count_key_use();

    log("Ренумерация таблицы зон.");
    renum_zone_table();

    log("Загрузка действий.");
    boot_scripts();
}


/* body of the booting system */
void boot_db(void)
{
    zone_rnum i;

    log("Загружаю базу данных.");

    log("Загружаю справочники полей");
    if (!ParserConst.ReadConst(ShareFile(mud->xformatFile).getCPath()))
        exit(1);

    log("Загружаю список материалов");
    boot_materials();

    log("Загружаю список зачарований");
    boot_enchant();

    log("Загружаю список тел");
    boot_bodys();

    log("Читаю текстовые файлы (news, motd, credits...)");
    mud->getTextFileLoader()->loadAll();

    log("Загружаю определения умений");
    assign_skills();
    load_skills();

    log("Загружаю справочник заклинаний");
    boot_spells();

    log("Загружаю справочник умений");
    boot_skills();

    log("Загружаю определения шаблонов доспехов");
    boot_armtempl();

    log("Загружаю определения шаблонов оружия");
    boot_weaptempl();

    log("Загружаю определения шаблонов снарядов");
    boot_misstempl();

    log("Загружаю определения наборов предметов");
    boot_sets();

    log("Загружаю боевые сообщения");
    boot_hit_messages();

    log("Загружаю мировой лут.");
    load_world_loot();

    log("Загружаю мир");
    boot_world();

    log("Generating player index.");
    build_player_index();

    log("Загрузка почтовых сообщений.");
    if (!scan_file()) {
        log("Файл не найден, почта отключена.");
        no_mail = 1;
    }

    log("Загружаю сохраненые локации.");
    /* TODO */

    /*Timed-out crash and rent files now checking in Crash_timer_obj */

    /* Moved here so the object limit code works. -gg 6/24/98 */
    if (!mud->modMini) {
        log("Загружаем фракции");
        // пока тут пусто
    }
    // Изменено Ладником
    log("Загрузка информации о постоях.");
    for (i = 0; i <= top_of_p_table; i++) {
        (player_table + i)->timer = 0;
        xrent_start_check(i, TRUE);
        //log ("Игрок %s", (player_table + i)->name);
    }

    log("Загружаю сохраненые магазины");
    boot_shops();

    log("Устанавливаю игровое время:");
    reset_time();

    for (i = 0; i <= top_of_zone_table; i++) {
        log("Сброс '%s', локации [%d-%d].", zone_table[i].name,
            (i ? (zone_table[i - 1].top + 1) : 0), zone_table[i].top);
        new_reset_zone(i);
    }

    reset_q.head = reset_q.tail = NULL;

    boot_time = time(0);
    //Конец изменений Ладником

    //log("Создаю лабиринты.");
    //create_mazes();

    log("Конец загрузки мира.");
}


void set_time(struct time_info_data times)
{
    int i, time_hour, time_day, time_month, time_year;

    weather_info.week_day =
        ((time_info.year * MONTHS_PER_YEAR + time_info.month) * DAYS_PER_MONTH +
         time_info.day) % WEEK_CYCLE;

    for (i = 0; i <= top_of_zone_table; ++i) {
        time_hour = time_info.hours + zone_table[i].time_offset;
        time_day = time_info.day;
        time_month = time_info.month;
        time_year = time_info.year;

        /* установка часов */
        if (time_hour < 0) {
            time_hour = HOURS_PER_DAY + time_hour;
            time_day--;
        } else if (time_hour >= HOURS_PER_DAY) {
            time_hour = time_hour - HOURS_PER_DAY;
            time_day++;
        }
        /* установка дней */
        if (time_day < 0) {
            time_day = DAYS_PER_MONTH + time_day;
            time_month--;
        } else if (time_day >= DAYS_PER_MONTH) {
            time_day = time_day - DAYS_PER_MONTH;
            time_month++;
        }
        /* установка лет */
        if (time_month < 0) {
            time_month = MONTHS_PER_YEAR + time_month;
            time_year--;
        } else if (time_month >= MONTHS_PER_YEAR) {
            time_month = time_month - MONTHS_PER_YEAR;
            time_month++;
        }

        zone_table[i].time_info.hours = time_hour;
        zone_table[i].time_info.day = time_day;
        zone_table[i].time_info.month = time_month;
        zone_table[i].time_info.year = time_year;
        zone_table[i].weather_info.week_day =
            ((zone_table[i].time_info.year * MONTHS_PER_YEAR +
              zone_table[i].time_info.month) * DAYS_PER_MONTH +
             zone_table[i].time_info.day) % WEEK_CYCLE;

        /* установка неба */
        if (zone_table[i].time_info.hours < sunrise[zone_table[i].time_info.month][0])
            zone_table[i].weather_info.sunlight = SUN_DARK;
        else if (zone_table[i].time_info.hours == sunrise[zone_table[i].time_info.month][0])
            zone_table[i].weather_info.sunlight = SUN_RISE;
        else if (zone_table[i].time_info.hours < sunrise[zone_table[i].time_info.month][1])
            zone_table[i].weather_info.sunlight = SUN_LIGHT;
        else if (zone_table[i].time_info.hours == sunrise[zone_table[i].time_info.month][1])
            zone_table[i].weather_info.sunlight = SUN_SET;
        else
            zone_table[i].weather_info.sunlight = SUN_DARK;

        // установка времени года
        if (zone_table[i].time_info.month >= MONTH_REGULA
            && zone_table[i].time_info.month <= MONTH_NARCISS)
            zone_table[i].weather_info.season = SEASON_SPRING;
        else if (zone_table[i].time_info.month >= MONTH_TITHING
                 && zone_table[i].time_info.month <= MONTH_PIVOT)
            zone_table[i].weather_info.season = SEASON_SUMMER;
        else if (zone_table[i].time_info.month >= MONTH_CATECHISM
                 && zone_table[i].time_info.month <= MONTH_MORTIS)
            zone_table[i].weather_info.season = SEASON_AUTUMN;
        else
            zone_table[i].weather_info.season = SEASON_WINTER;

        // конец цикла зоны
    }
}

/* reset the time in the game from file */
void reset_time(void)
{
    time_info = *mud_time_passed(time(0), beginning_of_time);

    time_info.hours = 8;

    set_time(time_info);

}



/* generate index table for the player file */
void build_player_index(void)
{
    new_build_player_index();
    return;
}

int count_social_records(FILE * fl, int *messages, int *keywords)
{
    char key[READ_SIZE], next_key[READ_SIZE];
    char line[READ_SIZE], *scan;

    /* get the first keyword line */
    get_one_line(fl, key);

    while (*key != '$') {       /* skip the text */
        do {
            get_one_line(fl, line);
            if (feof(fl))
                goto ackeof;
        } while (*line != '#');

        /* now count keywords */
        scan = key;
        ++(*messages);
        do {
            scan = one_word(scan, next_key);
            if (*next_key)
                ++(*keywords);
        } while (*next_key);

        /* get next keyword line (or $) */
        get_one_line(fl, key);

        if (feof(fl))
            goto ackeof;
    }

    return (TRUE);

    /* No, they are not evil. -gg 6/24/98 */
  ackeof:
    log("SYSERR: Unexpected end of help file.");
    exit(1);                    /* Some day we hope to handle these things better... */
}


void index_boot(int mode)
{
    int rec_count = 0, size[2];

    switch (mode) {
        case DB_BOOT_SOCIAL:
            {
                DLFileRead socFile(ShareFile(mud->socialsFile));

                if (!socFile.open())
                    return;
                rec_count += count_social_records(socFile.getFP(),
                                                  &top_of_socialm, &top_of_socialk);
            }
            break;

            //Добавляем кол-во из XFORMAT
        case DB_BOOT_ZON:
            rec_count += get_xrec_zon();
            break;
        case DB_BOOT_WLD:
            rec_count += get_xrec_wld();
            rec_count += top_of_zone_table;
            break;
        case DB_BOOT_MOB:
            rec_count += get_xrec_mob();
            break;
        case DB_BOOT_OBJ:
            rec_count += get_xrec_obj();
            break;

    }

    if (!rec_count) {
        log("ОШИБКА: загрузка: найдено 0 записей в индексе.");
        exit(1);
    }

    /* Any idea why you put this here Jeremy? */
    rec_count++;

    /*
     * NOTE: "bytes" does _not_ include strings or other later malloc'd things.
     */
    switch (mode) {
        case DB_BOOT_WLD:
            world = new room_data[rec_count];
            size[0] = sizeof(struct room_data) * rec_count;
            log("   %d локаций, %d байт.", rec_count, size[0]);
            break;
        case DB_BOOT_MOB:
            mob_proto = new Mobile[rec_count];
            CREATE(mob_index, struct index_data, rec_count);

            size[0] = sizeof(struct index_data) * rec_count;
            size[1] = sizeof(struct char_data) * rec_count;
            log("   %d монстров, %d байт в индексе, %d байт в прототипах.", rec_count, size[0],
                size[1]);
            break;
        case DB_BOOT_OBJ:
            obj_proto = new obj_data[rec_count];
            CREATE(obj_index, struct index_data, rec_count);

            size[0] = sizeof(struct index_data) * rec_count;
            size[1] = sizeof(struct obj_data) * rec_count;
            log("   %d объектов, %d байт в индексе, %d байт в прототипах.", rec_count, size[0],
                size[1]);
            break;
        case DB_BOOT_ZON:
            CREATE(zone_table, struct zone_data, rec_count);

            size[0] = sizeof(struct zone_data) * rec_count;
            log("   %d зон, %d байт.", rec_count, size[0]);
            break;
        case DB_BOOT_SOCIAL:
            CREATE(soc_mess_list, struct social_messg, top_of_socialm + 1);
            CREATE(soc_keys_list, struct social_keyword, top_of_socialk + 1);

            size[0] = sizeof(struct social_messg) * (top_of_socialm + 1);
            size[1] = sizeof(struct social_keyword) * (top_of_socialk + 1);
            log("   %d социалов (%d слов), %d(%d) байт.", top_of_socialm + 1,
                top_of_socialk + 1, size[0], size[1]);
    }

    //Загрузка в старом формате
    switch (mode) {
        case DB_BOOT_SOCIAL:
            {
                DLFileRead socFile(ShareFile(mud->socialsFile));

                if (socFile.open())
                    load_socials(socFile.getFP());
            }
            break;
    }

    // Сортировки индексов
    switch (mode) {
        case DB_BOOT_SOCIAL:
            log("Сортировка социалов");
            qsort(soc_keys_list, top_of_socialk + 1, sizeof(struct social_keyword), csort);
            break;
    }
}

void asciiflag_conv(const char *flag, void *value)
{
    int *flags = (int *) value;
    int is_number = 1, block = 0, i;
    register const char *p;

    for (p = flag; *p; p += i + 1) {
        i = 0;
        if (islower(*p)) {
            if (*(p + 1) >= '0' && *(p + 1) <= '9') {
                block = (int) *(p + 1) - '0';
                i = 1;
            } else
                block = 0;
            *(flags + block) |= (0x3FFFFFFF & (1 << (*p - 'a')));
        } else if (isupper(*p)) {
            if (*(p + 1) >= '0' && *(p + 1) <= '9') {
                block = (int) *(p + 1) - '0';
                i = 1;
            } else
                block = 0;
            *(flags + block) |= (0x3FFFFFFF & (1 << (26 + (*p - 'A'))));
        }
        if (!IS_DIGIT(*p))
            is_number = 0;
    }

    if (is_number) {
        is_number = atol(flag);
        block = is_number < INT_ONE ? 0 : is_number < INT_TWO ? 1 : is_number < INT_THREE ? 2 : 3;
        *(flags + block) = is_number & 0x3FFFFFFF;
    }
}

char fread_letter(FILE * fp)
{
    char c;

    do {
        c = getc(fp);
    } while (isspace(c));
    return c;
}

/* load the rooms */
/* make sure the start rooms exist & resolve their vnums to rnums */
void check_start_rooms(void)
{
    if ((r_mortal_start_room = real_room(mortal_start_room)) < 0) {
        log("SYSERR:  Mortal start room does not exist.  Change in config.c. %d",
            mortal_start_room);
        exit(1);
    }
    if ((r_immort_start_room = real_room(immort_start_room)) < 0) {
        if (!mud->modMini)
            log("ОШИБКА:  Отсутствует стартовая локация. Проверьте config.c.");
        r_immort_start_room = r_mortal_start_room;
    }
    if ((r_frozen_start_room = real_room(frozen_start_room)) < 0) {
        if (!mud->modMini)
            log("ОШИБКА: Локации для замороженых персонажей не существует.  Проверьте config.c.");
        r_frozen_start_room = r_mortal_start_room;
    }
    if ((r_helled_start_room = real_room(helled_start_room)) < 0) {
        if (!mud->modMini)
            log("ОШИБКА: Локации ад не существует. Проверьте config.c.");
        r_helled_start_room = r_mortal_start_room;
    }
    if ((r_jail_start_room = real_room(jail_start_room)) < 0) {
        if (!mud->modMini)
            log("ОШИБКА: Локации тюрьма не существует. Проверьте config.c.");
        r_jail_start_room = r_mortal_start_room;
    }

    if ((r_named_start_room = real_room(named_start_room)) < 0) {
        if (!mud->modMini)
            log("ОШИБКА: Локации комната имени не существует.  Проверьте config.c.");
        r_named_start_room = r_mortal_start_room;
    }
    if ((r_unreg_start_room = real_room(unreg_start_room)) < 0) {
        if (!mud->modMini)
            log("ОШИБКА:  Локации для регистрации не существует.  Проверьте config.c.");
        r_unreg_start_room = r_mortal_start_room;
    }
}


/* resolve all vnums into rnums in the world */



void renum_world(void)
{
    int room, door;

    for (room = 0; room <= top_of_world; room++)
        for (door = 0; door < NUM_OF_DIRS; door++)
            if (world[room].dir_option[door]) {
                if (world[room].dir_option[door]->to_room != NOWHERE)
                    world[room].dir_option[door]->to_room =
                        real_room(world[room].dir_option[door]->to_room);
                if (world[room].dir_option[door]->room_port != NOWHERE)
                    world[room].dir_option[door]->room_port =
                        real_room(world[room].dir_option[door]->room_port);
            }
}


#define ZCMD zone_table[zone].cmd[cmd_no]
#define ZCMD_CMD(cmd_nom) zone_table[zone].cmd[cmd_nom]

/* resulve vnums into rnums in the zone reset tables */
void renum_zone_table(void)
{
    int cmd_no, a, b, c, olda, oldb, oldc;
    zone_rnum zone;

    for (zone = 0; zone <= top_of_zone_table; zone++) {
        //log("renum zone");
        if (!zone_table[zone].cmd)
            continue;
        //log("zone %d: num %d top %d name %s",zone,zone_table[zone].number,top_of_zone_table,zone_table[zone].name);
        //log("znd %d",zone_table[zone].cmd[0].arg1);

        for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) {
            a = b = c = 0;
            ZCMD.oldarg1 = olda = ZCMD.arg1;
            ZCMD.oldarg2 = oldb = ZCMD.arg2;
            ZCMD.oldarg3 = oldc = ZCMD.arg3;
            ZCMD.oldarg4 = ZCMD.arg3;

            switch (ZCMD.command) {
                case 'M':
                    a = ZCMD.arg1 = real_mobile(ZCMD.arg1);
                    c = ZCMD.arg3 = real_room(ZCMD.arg3);
                    break;
                case 'Q':
                    a = ZCMD.arg1 = real_mobile(ZCMD.arg1);
                    break;
                case 'F':
                    a = ZCMD.arg1 = real_room(ZCMD.arg1);
                    b = ZCMD.arg2 = real_mobile(ZCMD.arg2);
                    c = ZCMD.arg3 = real_mobile(ZCMD.arg3);
                    break;
                case 'O':
                    a = ZCMD.arg1 = real_object(ZCMD.arg1);
                    if (ZCMD.arg3 != NOWHERE)
                        c = ZCMD.arg3 = real_room(ZCMD.arg3);
                    break;
                case 'G':
                    a = ZCMD.arg1 = real_object(ZCMD.arg1);
                    break;
                case 'E':
                    a = ZCMD.arg1 = real_object(ZCMD.arg1);
                    break;
                case 'P':
                    a = ZCMD.arg1 = real_object(ZCMD.arg1);
                    c = ZCMD.arg3 = real_object(ZCMD.arg3);
                    break;
                case 'D':
                    a = ZCMD.arg1 = real_room(ZCMD.arg1);
                    break;
                case 'R':      /* rem obj from room */
                    a = ZCMD.arg1 = real_room(ZCMD.arg1);
                    b = ZCMD.arg2 = real_object(ZCMD.arg2);
                    break;
            }
            if (a < 0 || b < 0 || c < 0) {
                if (!mud->modMini) {    /*sprintf(buf,  "Invalid vnum %d, cmd disabled",
                                           (a < 0) ? olda: ((b < 0) ? oldb: oldc));
                                           log_zone_error(zone, cmd_no, buf); */
                }
                ZCMD.command = '*';
            }
        }
    }
}



/* read all objects from obj file; generate index and prototypes */


void get_one_line(FILE * fl, char *buf)
{
    if (fgets(buf, READ_SIZE, fl) == NULL) {
        log("SYSERR: error reading help file: not terminated with $?");
        exit(1);
    }
    buf[strlen(buf) - 1] = '\0';        /* take off the trailing \n */
}


#if 0
int hsort(const void *a, const void *b)
{
    const struct help_index_element *a1, *b1;

    a1 = (const struct help_index_element *) a;
    b1 = (const struct help_index_element *) b;

    return (str_cmp(a1->keyword, b1->keyword));
}
#endif

int csort(const void *a, const void *b)
{
    const struct social_keyword *a1, *b1;

    a1 = (const struct social_keyword *) a;
    b1 = (const struct social_keyword *) b;

    return (str_cmp(a1->keyword, b1->keyword));
}


/*************************************************************************
*  procedures for resetting, both play-time and boot-time                *
*************************************************************************/


char *alias_vmobile(int vnumber)
{
    int nr, vnumb = 0;

    for (nr = 0; nr <= top_of_mobt; nr++) {
        vnumb = mob_index[mob_proto[nr].nr].vnum;
        if (vnumb == vnumber)
            return mob_proto[nr].player.name;
    }
    return NULL;
}


char *name_vmobile(int vnumber)
{
    int nr, vnumb = 0;

    for (nr = 0; nr <= top_of_mobt; nr++) {
        vnumb = mob_index[mob_proto[nr].nr].vnum;
        if (vnumb == vnumber)
            return mob_proto[nr].player.short_descr;
    }
    return NULL;
}

int vnum_mobile(char *searchname, struct char_data *ch)
{
    int nr, found = 0;
    char buf[MAX_STRING_LENGTH];

    for (nr = 0; nr <= top_of_mobt; nr++) {
        if (isname(searchname, mob_proto[nr].player.name)) {
            sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                    mob_index[nr].vnum, mob_proto[nr].player.short_descr);
            send_to_char(buf, ch);
        }
    }
    return (found);
}



int vnum_object(char *searchname, struct char_data *ch)
{
    int nr, found = 0;
    char buf[MAX_STRING_LENGTH];

    for (nr = 0; nr <= top_of_objt; nr++) {
        if (isname(searchname, obj_proto[nr].name)) {
            sprintf(buf, "%3d. [%5d] %s\r\n", ++found,
                    obj_index[nr].vnum, obj_proto[nr].short_description);
            send_to_char(buf, ch);
        }
    }
    return (found);
}


struct char_data *find_mobile(mob_vnum nr)
{
    struct char_data *i;

    for (i = character_list; i; i = i->next) {
        if (!IS_MOB(i))
            continue;
        {
            if (GET_MOB_VNUM(i) == nr)
                return (i);
        }
    }

    return (NULL);
}

/* create a new mobile from a prototype */
struct char_data *read_mobile(mob_vnum nr, int type)
{                               /* and mob_rnum */
    int is_corpse = 0;
    mob_rnum i;
    struct char_data *mob;

    if (nr < 0) {
        is_corpse = 1;
        nr = -nr;
    }


    if (type == VIRTUAL) {
        if ((i = real_mobile(nr)) < 0) {
            log("WARNING: Mobile vnum %d does not exist in database.", nr);
            return (NULL);
        }
    } else
        i = nr;

    mob = new Mobile(mob_proto[i]);
    mob->next = character_list;
    character_list = mob;

    affect_total(mob);
    GET_HIT(mob) = GET_REAL_MAX_HIT(mob);
    GET_MANA(mob) = GET_REAL_MAX_MANA(mob);

    GET_HORSESTATE(mob) = 200;
    GET_HLT(mob) = 100;
    GET_LASTROOM(mob) = NOWHERE;
    GET_PFILEPOS(mob) = -1;
    if (mob->npc()->specials.speed <= -1)
        GET_ACTIVITY(mob) = number(0, PULSE_MOBILE - 1);
    else
        GET_ACTIVITY(mob) = number(0, mob->npc()->specials.speed);
    EXTRACT_TIMER(mob) = 0;
    mob->points.move = mob->points.max_move;
    GET_GOLD(mob) += dice(GET_GOLD_NoDs(mob), GET_GOLD_SiDs(mob));

    mob->player.time.birth = time(0);
    mob->player.time.played = 0;
    mob->player.time.logon = time(0);
    mob->sw = 78;
    mob->sh = 22;


    mob_index[i].number++;
    GET_ID(mob) = max_id++;


    return (mob);
}


/* create an object, and add it to the object list */
struct obj_data *create_obj(void)
{
    struct obj_data *obj;

    obj = new obj_data();
    obj->next = object_list;
    object_list = obj;
    GET_ID(obj) = max_id++;
    GET_OBJ_ZONE(obj) = NOWHERE;
    OBJ_GET_LASTROOM(obj) = NOWHERE;
    GET_OBJ_TIMELOAD(obj) = time(0);
    return (obj);
}


void load_container(struct obj_data *obj)
{
    struct obj_data *tobj, *cobj, *tmp;
    struct list_obj_data *k, *l;
    int vnum, rnum_obj, chance, cnt_max, cnt_all, cnt_curr, cnt_in;

    tmp = obj;

    for (k = obj->load_obj; k; k = k->next) {
        vnum = k->vnum;
        rnum_obj = real_object(vnum);
        chance = k->percent;
        if (rnum_obj < 0) {
            log("ОШИБКА: Неизвестный предмет #%d в конейтнере #%d", vnum, GET_OBJ_VNUM(obj));
            continue;
        }

        cnt_max = GET_OBJ_LIMIT(obj_proto + rnum_obj);

        //общее кол-во в мире
        cnt_all = count_obj_vnum(vnum);

        cnt_curr = 0;

        //считаем кол-во предметов в контейнере
        for (cobj = obj->contains, cnt_curr = 0; cobj; cobj = cobj->next_content)
            if (rnum_obj == GET_OBJ_RNUM(cobj))
                cnt_curr++;

        cnt_in = 0;

        //считаем сколько должно быть предметов в контейнере
        for (l = tmp->load_obj; l; l = l->next)
            if (l->vnum == vnum)
                cnt_in++;


//    log("КОНТЕЙНЕР %d all %d < max %d",vnum,cnt_all,cnt_max);

        //Проверяем условия загрузки предметов
        if (cnt_all < cnt_max && chance >= number(1, 100) && cnt_curr < cnt_in) {
            tobj = read_object(rnum_obj, REAL, TRUE);
//     log("Загружаю в контейнер [%d]%s zone:%d",
//      GET_OBJ_VNUM(tobj),GET_OBJ_PNAME(tobj,0),GET_OBJ_ZONE(obj));
            if (GET_OBJ_ZONE(obj) != NOWHERE)
                GET_OBJ_ZONE(tobj) = GET_OBJ_ZONE(obj);
            else if (obj->worn_by)
                GET_OBJ_ZONE(tobj) = world[IN_ROOM(obj->worn_by)].zone;
            else if (obj->carried_by)
                GET_OBJ_ZONE(tobj) = world[IN_ROOM(obj->carried_by)].zone;
            obj_to_obj(tobj, obj);
            //IN_ROOM(tobj) = IN_ROOM(obj);
        }
    }
}

/* create a new object from a prototype */

struct obj_data *read_object(obj_vnum nr, int type, int load)
{                               /* and obj_rnum */
    struct obj_data *obj, *tobj;
    obj_rnum i;

    if (nr < 0) {
        log("SYSERR: Trying to create obj with negative (%d) num!", nr);
        return (NULL);
    }
    if (type == VIRTUAL) {
        if ((i = real_object(nr)) < 0) {
            log("Object (V) %d does not exist in database.", nr);
            return (NULL);
        }
    } else
        i = nr;

    obj = new obj_data(obj_proto[i]);

    obj->next = object_list;
    object_list = obj;
    obj_index[i].number++;
    GET_ID(obj) = max_id++;
    GET_OBJ_ZONE(obj) = NOWHERE;
    OBJ_GET_LASTROOM(obj) = NOWHERE;
    GET_OBJ_TIMELOAD(obj) = time(0);

    if (GET_OBJ_TYPE(obj) == ITEM_DRINKCON) {
        if (GET_OBJ_VAL(obj, 1) && GET_OBJ_VAL(obj, 2) > 0)
            name_to_drinkcon(obj, GET_OBJ_VAL(obj, 2));
    }

    if (GET_OBJ_TYPE(obj) == ITEM_LIGHT && GET_LIGHT_VAL(obj) == -1)
        GET_LIGHT_ON(obj) = TRUE;

    if (load) {
        if (obj->load_obj)
            load_container(obj);

        if (obj->bgold) {
            tobj = create_money(obj->bgold);
            if (obj->in_room != NOWHERE)
                GET_OBJ_ZONE(tobj) = world[obj->in_room].zone;
            else if (obj->worn_by)
                GET_OBJ_ZONE(tobj) = world[IN_ROOM(obj->worn_by)].zone;
            else if (obj->carried_by)
                GET_OBJ_ZONE(tobj) = world[IN_ROOM(obj->carried_by)].zone;
            obj_to_obj(tobj, obj);
        }
    }

    if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
        char lock_code[32];

        *lock_code = '\0';

        set_door_code(lock_code, (GET_OBJ_QUALITY(obj) * 15) / 10);
        strcpy(obj->lock_code, lock_code);
        obj->lock_step = 0;
    }

    return (obj);
}



#define ZO_DEAD  9999

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update(void)
{
    int i;
    struct reset_q_element *update_u, *temp;
    char buf[128];

    /* jelson 10/22/92 */
    if (((++zone_update_timer * PULSE_ZONE) / PASSES_PER_SEC) >= 60) {  /* one minute has passed */
        /*
         * NOT accurate unless PULSE_ZONE is a multiple of PASSES_PER_SEC or a
         * factor of 60
         */

        zone_update_timer = 0;

        /* since one minute has passed, increment zone ages */
        for (i = 0; i <= top_of_zone_table; i++) {
            //Сборс зоны по времени
            if (zone_table[i].reset_mode == 3 &&
                zone_table[i].time_info.hours == zone_table[i].lifespan) {
                new_reset_zone(i);
                continue;
            }




            if (zone_table[i].age < zone_table[i].lifespan && zone_table[i].reset_mode)
                (zone_table[i].age)++;

            if (zone_table[i].age >= zone_table[i].lifespan && zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {       /* enqueue zone */
                CREATE(update_u, struct reset_q_element, 1);

                update_u->zone_to_reset = i;
                update_u->next = 0;

                if (!reset_q.head)
                    reset_q.head = reset_q.tail = update_u;
                else {
                    reset_q.tail->next = update_u;
                    reset_q.tail = update_u;
                }

                zone_table[i].age = ZO_DEAD;
            }
        }
    }



    /* end - one minute has passed */
    /* dequeue zones (if possible) and reset                    */
    /* this code is executed every 10 seconds (i.e. PULSE_ZONE) */
    for (update_u = reset_q.head; update_u; update_u = update_u->next)
        if (zone_table[update_u->zone_to_reset].reset_mode == 2 ||
            is_empty(update_u->zone_to_reset)) {
            new_reset_zone(update_u->zone_to_reset);
            sprintf(buf, "Сброс зоны: %s", zone_table[update_u->zone_to_reset].name);
            log(buf);
            //mudlog(buf, CMP, LVL_GOD, FALSE);
            /* dequeue */
            if (update_u == reset_q.head)
                reset_q.head = reset_q.head->next;
            else {
                for (temp = reset_q.head; temp->next != update_u; temp = temp->next);

                if (!update_u->next)
                    reset_q.tail = temp;

                temp->next = update_u->next;
            }

            free(update_u);
            break;
        }
}

void log_zone_error(zone_rnum zone, int cmd_no, const char *message)
{
    char buf[256];

    sprintf(buf, "SYSERR: zone file: %s", message);
    mudlog(buf, NRM, LVL_GOD, TRUE);

    sprintf(buf, "SYSERR: ...offending cmd: '%c' cmd in zone #%d, line %d",
            ZCMD.command, zone_table[zone].number, ZCMD.line);
    mudlog(buf, NRM, LVL_GOD, TRUE);
}

#define ZONE_ERROR(message) \
    { log_zone_error(zone, cmd_no, message); last_cmd = 0; }


void new_reset_zone(zone_rnum zone)
{
    int count, vnum, cnt_curr, cnt_all, cnt_in_room, cnt_max;
    int rnum_obj, rnum_mobile, door;
    int pos, percent, v;
    byte chance;
    struct db_load_data *db_data, *db_data_cnt;
    struct obj_data *obj_room, *obj, *obj_room_next;
    struct list_obj_data *k;
    struct char_data *mob, *ch, *leader, *mob_room, *mob_room_next;


//Обходим все локации мира
//может это и долго зато надежно
    log("Сброс зоны [%d] %s", zone_table[zone].number, zone_table[zone].name);

    if (zone_table[zone].type == 7) {
        log("Генерация лабиринта [%d] %s", zone_table[zone].number, zone_table[zone].name);
        make_maze(zone);
    }
    /* sprintf(buf,"ZONE [%d] %s",zone,zone_table[zone].name);
       mudlog(buf,CMP, LVL_GOD, TRUE); */

    for (count = 0; count <= top_of_world; count++) {
        int back_room, back_dir;

        if (world[count].zone != zone)
            continue;

        //log("КОМНАТА [%d] %s ",world[count].number,world[count].name);
        //Сброс установок дверей
        for (door = 0; door < NUM_OF_DIRS; door++) {
            if (world[count].dir_option[door]
                && IS_SET(GET_FLAG(world[count].dir_option[door]->exit_data, EXIT_REPOP),
                          EXIT_REPOP)) {
                if (world[count].dir_option[door]->general_description)
                    free(world[count].dir_option[door]->general_description);
                if (world[count].dir_option[door]->keyword)
                    free(world[count].dir_option[door]->keyword);
                if (world[count].dir_option[door]->exit_name)
                    free(world[count].dir_option[door]->exit_name);
                world[count].dir_option[door] = NULL;
                continue;
            }
            if (world[count].dir_option[door] && world[count].dir_option[door]->type_port == 4) {
                if (world[count].dir_option[door]->portal_description)
                    free(world[count].dir_option[door]->portal_description);
                if (world[count].dir_option[door]->mess_to_open)
                    free(world[count].dir_option[door]->mess_to_open);
                if (world[count].dir_option[door]->mess_to_close)
                    free(world[count].dir_option[door]->mess_to_close);
                if (world[count].dir_option[door]->mess_char_enter)
                    free(world[count].dir_option[door]->mess_char_enter);
                if (world[count].dir_option[door]->mess_room_enter)
                    free(world[count].dir_option[door]->mess_room_enter);

                world[count].dir_option[door]->type_port = 0;
                world[count].dir_option[door]->key_port = 0;
                world[count].dir_option[door]->room_port = NOWHERE;
                continue;
            }

            if (world[count].dir_option[door] &&
                world[count].dir_option[door]->exit_data_reset.flags[0])
                world[count].dir_option[door]->exit_data =
                    world[count].dir_option[door]->exit_data_reset;

            if (world[count].dir_option[door]) {
                char lock_code[32];

                *lock_code = '\0';
                //установка кода двери
                if (DOOR_FLAGGED(world[count].dir_option[door], EXIT_LOCKED)) {
                    set_door_code(lock_code, (world[count].dir_option[door]->quality * 15) / 100);
                    strcpy(world[count].dir_option[door]->lock_code, lock_code);
                    world[count].dir_option[door]->lock_step = 0;
                }
                world[count].dir_option[door]->tbreak = FALSE;
                world[count].dir_option[door]->tfind.clear();
                //Устанавливаем параметры двери с другой стороны
                if ((back_room = world[count].dir_option[door]->to_room) != NOWHERE) {
                    back_dir = rev_dir[door];
                    if (world[back_room].dir_option[back_dir] &&
                        world[back_room].dir_option[back_dir]->exit_data_reset.flags[0])
                        world[back_room].dir_option[back_dir]->exit_data =
                            world[back_room].dir_option[back_dir]->exit_data_reset;
                    if (world[back_room].dir_option[back_dir]) {
                        //установка кода задней двери
                        if (DOOR_FLAGGED(world[back_room].dir_option[back_dir], EXIT_LOCKED)) {
                            if (!*lock_code)
                                set_door_code(lock_code,
                                              (world[back_room].dir_option[back_dir]->quality *
                                               15) / 100);
                            strcpy(world[back_room].dir_option[back_dir]->lock_code, lock_code);
                            world[back_room].dir_option[back_dir]->lock_step = 0;

                            world[back_room].dir_option[back_dir]->tbreak = FALSE;
                            world[back_room].dir_option[back_dir]->tfind.clear();
                        }
                    }
                }
            }
        }

        /* Сброс предеметов */
        for (obj_room = world[count].contents; obj_room; obj_room = obj_room_next) {
            obj_room_next = obj_room->next_content;

            if (GET_OBJ_TYPE(obj_room) == ITEM_CONTAINER && obj_room->load_obj)
                load_container(obj_room);

            if (GET_OBJ_TYPE(obj_room) == ITEM_CONTAINER) {
                char lock_code[32];

                *lock_code = '\0';

                set_door_code(lock_code, (GET_OBJ_QUALITY(obj_room) * 15) / 10);
                strcpy(obj_room->lock_code, lock_code);
                obj_room->lock_step = 0;
            }

            if (GET_OBJ_TYPE(obj_room) == ITEM_CONTAINER && obj_room->trap) {
                obj_room->trap->tbreak = FALSE;
                obj_room->trap->tfind.clear();
            }

            if (OBJ_FLAGGED(obj_room, ITEM_REPOPDROP)) {
                struct obj_data *tobj = NULL;

                while (obj_room->contains) {
                    tobj = obj_room->contains;
                    obj_from_obj(tobj);
                    extract_obj(tobj);
                }

                extract_obj(obj_room);
            }
        }

        /* Сброс монстров */
        for (mob_room = world[count].people; mob_room; mob_room = mob_room_next) {
            mob_room_next = mob_room->next_in_room;
            if (!IS_MOB(mob_room))
                continue;

            for (obj = mob_room->carrying; obj; obj = obj_room_next) {
                obj_room_next = obj->next_content;
                if (OBJ_FLAGGED(obj, ITEM_REPOPDROP))
                    extract_obj(obj);
            }

            if (NPC_FLAGGED(mob_room, NPC_REPOPKILL))
                extract_char(mob_room, FALSE);
        }

        //log("...расстановка предметов");
        for (db_data = world[count].objects; db_data; db_data = db_data->next) {
            //mudlog("123",CMP,LVL_GOD,TRUE);
            //Инициализация переменных
            cnt_in_room = 0;
            cnt_curr = 0;
            cnt_all = 0;

            vnum = db_data->vnum;
            //log("..Загрузка предмета %d",vnum);
            rnum_obj = real_object(vnum);
            //log("..конец загрузки");
            chance = db_data->chance;

            if (rnum_obj == -1) {
                log("СБРОС: Предмет VNUM: %d осутствует в БД", vnum);
                continue;
            }

            cnt_max = GET_OBJ_LIMIT(obj_proto + rnum_obj);

            //sprintf(buf,"СБРОС: Предмет: %d[%d] шанс: %d",vnum,rnum_obj,chance);
            //mudlog(buf,CMP, LVL_GOD, TRUE);
            //считаем сколько должно быть объектов в текущей локации
            for (db_data_cnt = world[count].objects; db_data_cnt; db_data_cnt = db_data_cnt->next)
                if (db_data_cnt->vnum == vnum)
                    cnt_in_room++;

            //считаем кол-во объектов в текущей локации
            for (obj_room = world[count].contents, cnt_curr = 0; obj_room;
                 obj_room = obj_room->next_content)
                if (rnum_obj == GET_OBJ_RNUM(obj_room))
                    cnt_curr++;

            //общее кол-во в мире
            cnt_all = count_obj_vnum(vnum);

            //log("СБРОС: Предмет: cnt_all %d < cnt_max %d && cnt_in_room %d > cnt_curr %d",
            //            cnt_all,cnt_max,cnt_in_room,cnt_curr);
            //Проверяем условия загрузки предметов
            int achance = 0;

            if (cnt_all < cnt_max)
                achance = chance * 10;  //Если лимит предметов шанс выпасть уменьшаем в 3 раза
            else
                achance = chance * 3;

            if (achance >= number(1, 1000) && cnt_in_room > cnt_curr) {
                obj = read_object(rnum_obj, REAL, FALSE);
                GET_OBJ_ZONE(obj) = world[count].zone;
                //log("..загрузка контейнера");
                //load_container(obj);

                //log("Предмет в локацию");
                obj_to_room(obj, count);
                //IN_ROOM(obj) = count;

                load_container(obj);
            }
        }                       //конец загрузки предметов

        //log("...сброс предметов");
        //Сбрасываем предметы в исходное состояние
        for (obj = world[count].contents; obj; obj = obj->next_content) {
            if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER || GET_OBJ_TYPE(obj) == ITEM_FICTION)
                obj->bflag = obj->bflag_reset;

            if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
                char lock_code[32];

                *lock_code = '\0';

                set_door_code(lock_code, (GET_OBJ_QUALITY(obj) * 15) / 10);
                strcpy(obj->lock_code, lock_code);
                obj->lock_step = 0;
            }
        }
        for (db_data = world[count].mobiles; db_data; db_data = db_data->next) {
            int vnum_room = world[count].number;

            cnt_in_room = 0;
            //log("...растановка монстров %d",vnum_room);
            vnum = db_data->vnum;
            chance = db_data->chance;
            rnum_mobile = real_mobile(vnum);
            if (rnum_mobile == -1) {
                log("СБРОС: Монстр VNUM: %d осутствует в БД", vnum);
                continue;
            }
            //считаем сколько должно быть монстров в текущей локации
            for (db_data_cnt = world[count].mobiles; db_data_cnt; db_data_cnt = db_data_cnt->next)
                if (db_data_cnt->vnum == vnum)
                    cnt_in_room++;

            cnt_all = get_room_mob_number(vnum, vnum_room);
            cnt_all = cnt_in_room - cnt_all;

            //log("room %d cnt_all %d cnt_in_room %d chance %d", world[count].number, cnt_all,cnt_in_room,chance);

            for (cnt_curr = 0; cnt_curr < cnt_all; cnt_curr++)
                if (chance >= number(1, 100)) {
                    //log("СБРОС: УСТАНАВЛИВАЕМ МОНСТРА %d",vnum);
                    mob = read_mobile(rnum_mobile, REAL);
                    //log("УСТАНОВИЛИ МОНСТРА %d",vnum);

                    //log("МОНСТРА В ЛОКАЦИЮ");
                    char_to_room(mob, count);
                    //log("МОНСТРА В ЛОКАЦИЮ - конец");
                    //act("$n появил$u.",FALSE,mob,0,0,TO_ROOM);
                    GET_LOAD_ROOM(mob) = vnum_room;

                    /*Загрузка инвентаря монстра */
                    //log("...загрузка инвентаря");
                    for (k = mob->load_inv; k; k = k->next) {
                        v = k->vnum;
                        pos = k->percent;
                        //log("...загрузка предмета %d",v);
                        obj = read_object(v, VIRTUAL, TRUE);
                        if (!obj)
                            continue;

                        if (GET_OBJ_PERCENT(obj))
                            percent = GET_OBJ_PERCENT(obj);
                        else
                            percent = 100;

                        if (GET_OBJ_LIMIT(obj) < count_obj_vnum(v) || percent < number(1, 100)) {
                            extract_obj(obj);
                            continue;
                        }
                        //log("ПРЕДЕМ ТО МОНСТР");
                        obj_to_char(obj, mob);
                        GET_OBJ_ZONE(obj) = world[count].zone;
                        //IN_ROOM(obj) = count;
                    }

                    //log("...загружаем экипировку монстра");
                    /* Загружаем экипировку моба */
                    for (k = mob->load_eq; k; k = k->next) {
                        v = k->vnum;
                        pos = k->percent;
                        obj = read_object(v, VIRTUAL, TRUE);

                        if (!obj)
                            continue;

                        if (GET_OBJ_PERCENT(obj))
                            percent = GET_OBJ_PERCENT(obj);
                        else
                            percent = 100;

                        if (GET_OBJ_LIMIT(obj) < count_obj_vnum(v) || percent < number(1, 100)) {
                            extract_obj(obj);
                            continue;
                        }

                        GET_OBJ_ZONE(obj) = world[IN_ROOM(mob)].zone;

                        //if (wear_otrigger(obj, mob, pos))
                        {
                            if (!GET_EQ(mob, pos))
                                equip_char(mob, obj, pos);
                            else
                                log("ОШИБКА: Попытка повторно экипировать #%d предметом #%d %s.",
                                    GET_MOB_VNUM(mob), GET_OBJ_VNUM(obj), where[pos]);
                        }
                        //else
                        // obj_to_char(obj, mob);

                        if (!(obj->carried_by == mob) && !(obj->worn_by == mob))
                            extract_obj(obj);

                        // IN_ROOM(obj) = count;
                    }

                    //log("...загружаем тату монстра");
                    /* Загружаем татуировки моба */
                    for (k = mob->load_tatoo; k; k = k->next) {
                        v = k->vnum;
                        pos = k->percent;
                        obj = read_object(v, VIRTUAL, TRUE);
                        if (!obj)
                            continue;

                        GET_OBJ_ZONE(obj) = world[IN_ROOM(mob)].zone;

                        //if (wear_otrigger(obj, mob, pos))
                        equip_tatoo(mob, obj, pos);
                        //else
                        // obj_to_char(obj, mob);

                        //IN_ROOM(obj) = count;
                        if (!(obj->carried_by == mob) && !(obj->worn_by == mob))
                            extract_obj(obj);
                    }

                    //Ездовое животное
                    if (mob->npc()->specials.vnum_horse > 0) {
                        struct char_data *horse;

                        horse = read_mobile(mob->npc()->specials.vnum_horse, VIRTUAL);
                        char_to_room(horse, count);
                        GET_LOAD_ROOM(horse) = vnum_room;
                        if (!IS_MOUNT(horse)) {
                            if (!AFF_FLAGGED(horse, AFF_HORSE_BUY))
                                make_horse(horse, mob);
                            SET_BIT(MOB_FLAGS(horse, MOB_MOUNTING), MOB_MOUNTING);
                            SET_BIT(AFF_FLAGS(horse, AFF_HORSE), AFF_HORSE);
                            SET_BIT(AFF_FLAGS(mob, AFF_HORSE), AFF_HORSE);
                        }
                    }
                }
        }                       //конец загрузки монстров

        //log("...проверка на следование монстра");
        //Проверка на следование монстра
        for (mob = world[count].people; mob; mob = mob->next_in_room) {
            if (!IS_NPC(mob) || !mob->follow_vnum)
                continue;
            leader = NULL;

            for (ch = world[count].people; ch && !leader; ch = ch->next_in_room)
                if (IS_NPC(ch) && GET_MOB_VNUM(ch) == mob->follow_vnum)
                    leader = ch;
            for (ch = world[count].people; ch && leader; ch = ch->next_in_room)
                if (IS_NPC(ch) && ch == mob && leader != ch && ch->master != leader) {
                    if (ch->master)
                        stop_follower(ch, SF_EMPTY);
                    add_follower(ch, leader, FLW_GROUP);
                    join_party(leader, ch);
                }
        }
        //log("...конец проверки на следование монстра");
        //конец проверки
    }

    zone_table[zone].age = 0;

//log("...сборос скриптов");

//log("[Reset] Paste mobiles");
//log("...проверка монстров");
    paste_mobiles(zone);


};


void paste_mobiles(int zone)
{
    struct char_data *ch, *ch_next;
    struct obj_data *obj, *obj_next;
    int time_ok, month_ok, need_move, no_month, no_time, room = -1;

    for (ch = character_list; ch; ch = ch_next) {
        ch_next = ch->next;
        if (!IS_NPC(ch))
            continue;
        if (FIGHTING(ch))
            continue;
        if (GET_POS(ch) < POS_STUNNED)
            continue;
        if (AFF_FLAGGED(ch, AFF_CHARM) || AFF_FLAGGED(ch, AFF_HORSE) || AFF_FLAGGED(ch, AFF_HOLD))
            continue;
        if ((room = IN_ROOM(ch)) == NOWHERE)
            continue;
        if (zone >= 0 && world[room].zone != zone)
            continue;

        time_ok = FALSE;
        month_ok = FALSE;
        need_move = FALSE;
        no_month = TRUE;
        no_time = TRUE;

        if (MOB_FLAGGED(ch, MOB_LIKE_DAY)) {
            if (zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_RISE ||
                zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_LIGHT)
                time_ok = TRUE;
            need_move = TRUE;
            no_time = FALSE;
        }
        if (MOB_FLAGGED(ch, MOB_LIKE_NIGHT)) {
            if (zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_SET ||
                zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_DARK)
                time_ok = TRUE;
            need_move = TRUE;
            no_time = FALSE;
        }
        if (MOB_FLAGGED(ch, MOB_LIKE_FULLMOON)) {
            if ((zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_SET ||
                 zone_table[world[IN_ROOM(ch)].zone].weather_info.sunlight == SUN_DARK) &&
                (zone_table[world[IN_ROOM(ch)].zone].weather_info.moon_day >= 12 &&
                 zone_table[world[IN_ROOM(ch)].zone].weather_info.moon_day <= 15)
                )
                time_ok = TRUE;
            need_move = TRUE;
            no_time = FALSE;
        }
        if (MOB_FLAGGED(ch, MOB_LIKE_WINTER)) {
            if (zone_table[world[IN_ROOM(ch)].zone].weather_info.season == SEASON_WINTER)
                month_ok = TRUE;
            need_move = TRUE;
            no_month = FALSE;
        }
        if (MOB_FLAGGED(ch, MOB_LIKE_SPRING)) {
            if (zone_table[world[IN_ROOM(ch)].zone].weather_info.season == SEASON_SPRING)
                month_ok = TRUE;
            need_move = TRUE;
            no_month = FALSE;
        }
        if (MOB_FLAGGED(ch, MOB_LIKE_SUMMER)) {
            if (zone_table[world[IN_ROOM(ch)].zone].weather_info.season == SEASON_SUMMER)
                month_ok = TRUE;
            need_move = TRUE;
            no_month = FALSE;
        }
        if (MOB_FLAGGED(ch, MOB_LIKE_AUTUMN)) {
            if (zone_table[world[IN_ROOM(ch)].zone].weather_info.season == SEASON_AUTUMN)
                month_ok = TRUE;
            need_move = TRUE;
            no_month = FALSE;
        }
        if (need_move) {
            month_ok |= no_month;
            time_ok |= no_time;
            if (month_ok && time_ok) {
                if (world[room].number != zone_table[world[room].zone].top)
                    continue;
                if (GET_LASTROOM(ch) == NOWHERE) {
                    extract_char(ch, FALSE);
                    continue;
                }
                char_from_room(ch);
                char_to_room(ch, GET_LASTROOM(ch));
                //log("Put %s at room %d",GET_NAME(ch),world[IN_ROOM(ch)].number);
            } else {
                if (world[room].number == zone_table[world[room].zone].top)
                    continue;
                GET_LASTROOM(ch) = room;
                char_from_room(ch);
                room = real_room(zone_table[world[room].zone].top);
                if (room == NOWHERE)
                    room = GET_LASTROOM(ch);
                char_to_room(ch, room);
            }
        }
    }

    for (obj = object_list; obj; obj = obj_next) {
        obj_next = obj->next;
        if (obj->carried_by || obj->worn_by || (room = obj->in_room) == NOWHERE)
            continue;
        if (zone >= 0 && world[room].zone != zone)
            continue;
        time_ok = FALSE;
        month_ok = FALSE;
        need_move = FALSE;
        no_time = TRUE;
        no_month = TRUE;
        //Обнуляем видимость
        if (OBJ_FLAGGED(obj, ITEM_HIDDEN))
            obj->visible_by.clear();

        if (OBJ_FLAGGED(obj, ITEM_DAY)) {
            if (zone_table[world[room].zone].weather_info.sunlight == SUN_RISE ||
                zone_table[world[room].zone].weather_info.sunlight == SUN_LIGHT)
                time_ok = TRUE;
            need_move = TRUE;
            no_time = FALSE;
        }
        if (OBJ_FLAGGED(obj, ITEM_NIGHT)) {
            if (zone_table[world[room].zone].weather_info.sunlight == SUN_SET ||
                zone_table[world[room].zone].weather_info.sunlight == SUN_DARK)
                time_ok = TRUE;
            need_move = TRUE;
            no_time = FALSE;
        }
        if (need_move) {
            month_ok |= no_month;
            time_ok |= no_time;
            if (month_ok && time_ok) {
                if (world[room].number != zone_table[world[room].zone].top)
                    continue;
                if (OBJ_GET_LASTROOM(obj) == NOWHERE) {
                    extract_obj(obj);
                    continue;
                }
                obj_from_room(obj);
                obj_to_room(obj, OBJ_GET_LASTROOM(obj));
                //log("Put %s at room %d",obj->PNames[0],world[OBJ_GET_LASTROOM(obj)].number);
            } else {
                if (world[room].number == zone_table[world[room].zone].top)
                    continue;
                OBJ_GET_LASTROOM(obj) = room;
                obj_from_room(obj);
                room = real_room(zone_table[world[room].zone].top);
                if (room == NOWHERE)
                    room = OBJ_GET_LASTROOM(obj);
                obj_to_room(obj, room);
                //log("Remove %s at room %d",GET_NAME(ch),world[room].number);
            }
        }
    }
}

// Ищет RNUM первой и последней комнаты зоны
// Еси возвращает 0 - комнат в зоне нету
int get_zone_rooms(int zone_nr, int *start, int *stop)
{
    int first_room_vnum, rnum;

    first_room_vnum = zone_table[zone_nr].top;
    rnum = real_room(first_room_vnum);
    if (rnum == -1)
        return 0;
    *stop = rnum;
    rnum = -1;
    while (zone_nr) {
        first_room_vnum = zone_table[--zone_nr].top;
        rnum = real_room(first_room_vnum);
        if (rnum != -1) {
            ++rnum;
            break;
        }
    }
    if (rnum == -1)
        rnum = 0;               // самая первая зона начинается с 0
    *start = rnum;
    return 1;
}

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(zone_rnum zone_nr)
{
    struct descriptor_data *i;
    int rnum_start, rnum_stop;
    struct room_data *start, *stop;
    struct char_data *c;


    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING)
            continue;
        if (IN_ROOM(i->character) == NOWHERE)
            continue;
        if (GET_LEVEL(i->character) >= LVL_IMMORT)
            continue;
        if (world[i->character->in_room].zone != zone_nr)
            continue;

        return (0);
    }

    // Поиск link-dead игроков в зонах комнаты zone_nr
    if (!get_zone_rooms(zone_nr, &rnum_start, &rnum_stop))
        return 1;               // в зоне нет комнат :)

    start = world + rnum_start;
    stop = world + rnum_stop;

    for (; start <= stop; ++start) {
        for (c = start->people; c; c = c->next_in_room)
            if (!IS_NPC(c) && (GET_LEVEL(c) < LVL_GOD))
                return 0;
    }

    //теперь проверю всех товарищей в void комнате STRANGE_ROOM
    for (c = world[STRANGE_ROOM].people; c; c = c->next_in_room) {
        int was = GET_WAS_IN(c);

        if (was == NOWHERE)
            continue;
        if (GET_LEVEL(c) >= LVL_GOD)
            continue;
        if (world[was].zone != zone_nr)
            continue;
        return 0;
    }

    return (1);
}

//считаем кол-во объектов в текущей локации
int obj_in_room(int o_num, int r_num)
{
    struct obj_data *obj_room;
    int count = 0;

    for (obj_room = world[r_num].contents; obj_room; obj_room = obj_room->next_content)
        if (r_num == GET_OBJ_VNUM(obj_room))
            count++;

    return count;
}


int mobs_in_room(int m_num, int r_num)
{
    struct char_data *ch;
    int count = 0;

    for (ch = world[r_num].people; ch; ch = ch->next_in_room)
        if (m_num == GET_MOB_VNUM(ch))
            count++;

    return count;
}


/*************************************************************************
*  stuff related to the save/load player system                          *
*************************************************************************/

long cmp_ptable_by_name(char *name, int len)
{
    int i;
    char arg[MAX_STRING_LENGTH];

    /*len = MIN(len,strlen(name));
       one_argument(name, arg);
       for (i = 0; i <= top_of_p_table; i++)
       if (!strn_cmp(player_table[i].name, arg, MIN(len,strlen(player_table[i].name))))
       return (i);
       return (-1); */

    one_argument(name, arg);
    for (i = 0; i <= top_of_p_table; i++)
        if (!str_cmp(player_table[i].name, arg))
            return (i);

    return (-1);
}

void update_ptable_data(struct char_data *ch)
{
    if (!IS_NPC(ch)) {
        int id;

        if ((id = get_ptable_by_name(GET_NAME(ch))) >= 0) {
            player_table[id].level = GET_LEVEL(ch);
            player_table[id].last_logon = time(0);
            player_table[id].activity = number(0, OBJECT_SAVE_ACTIVITY - 1);
        }
    }
}

long get_ptable_by_name(char *name)
{
    int i;
    char arg[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];

    if (!name)
        return -1;

    one_argument(name, arg);
    for (i = 0; i <= top_of_p_table; i++)
        if (!str_cmp(player_table[i].name, arg))
            return (i);
    sprintf(buf, "Char %s(%s) not found!!!", name, arg);
    mudlog(buf, CMP, LVL_GOD, FALSE);
    return (-1);
}


long get_id_by_name(char *name)
{
    int i;
    char arg[MAX_STRING_LENGTH];

    one_argument(name, arg);
    for (i = 0; i <= top_of_p_table; i++)
        if (!str_cmp(player_table[i].name, arg))
            return (player_table[i].id);

    return (-1);
}

int get_room_mob_number(int vnum_mob, int vnum_room)
{
    int i = 0;
    struct char_data *ch, *ch_next;


    for (ch = character_list; ch; ch = ch_next) {
        ch_next = ch->next;
        if (!IS_NPC(ch))
            continue;
        if (MOB_FLAGGED(ch, MOB_FREE) || MOB_FLAGGED(ch, MOB_DELETE) || IN_ROOM(ch) == NOWHERE)
            continue;
        if (GET_LOAD_ROOM(ch) == vnum_room && GET_MOB_VNUM(ch) == vnum_mob)
            i++;
    }

    return i;

}


char *get_name_by_id(long id)
{
    int i;
    struct char_data *ch, *ch_next;

    for (ch = character_list; ch; ch = ch_next) {
        ch_next = ch->next;

        if (GET_ID(ch) == id)
            return (GET_NAME(ch));
    }

    for (i = 0; i <= top_of_p_table; i++)
        if (player_table[i].id == id)
            return (player_table[i].name);

    return (NULL);
}

char *get_name_by_unique(long unique)
{
    int i;

    for (i = 0; i <= top_of_p_table; i++)
        if (player_table[i].unique == unique)
            return (player_table[i].name);

    return (NULL);
}


void delete_unique(struct char_data *ch)
{
    int i;

    if ((i = get_id_by_name(GET_NAME(ch))) >= 0)
        player_table[i].unique = GET_UNIQUE(ch) = -1;

}

int correct_unique(int unique)
{
    int i;

    for (i = 0; i <= top_of_p_table; i++)
        if (player_table[i].unique == unique)
            return (TRUE);

    return (FALSE);
}

int create_unique(void)
{
    int unique;

    do {
        unique = (number(0, 64) << 24) + (number(0, 255) << 16) +
            (number(0, 255) << 8) + (number(0, 255));
    } while (correct_unique(unique));
    return (unique);
}


#define NUM_OF_SAVE_THROWS 5

/* new load_char reads ascii pfiles */
/* Load a char, returns player ID if loaded, -1 if not */

int load_char_ascii(char *name, Player * ch)
{
    int id, num = 0, num2 = 0, num3 = 0, num4 = 0, num5 = 0, num6 = 0, i;
    long int lnum, lnum1 = 0;
    long long llnum = 0;
    FBFILE *fl;
    char filename[40];
    char buf[128], line[MAX_INPUT_LENGTH + 1], tag[6];
    char line1[MAX_INPUT_LENGTH + 1];
    struct affected_type af;
    struct timed_type timed;
    struct descriptor_data *d;

    *filename = '\0';
    //log("Загрузка пфайла '%s'", name);
    if (now_entrycount) {
        id = 1;
    } else {
        id = find_name(name);
    }
    if (!(id >= 0 && get_filename(name, filename, PLAYERS_FILE)
          && (fl = fbopen(filename, FB_READ)))) {
        log("ОШИБКА: Невозможно загрузить пфайл %d %s", id, filename);
        return (-1);
    }

    /* 
     * placement destructor: release dynamic memory used by
     * fields from previous 'incarnation' 
     */
    d = ch->desc;
    ch->~Player();
    /* 
     * placement constructor: the complete player init 
     */
    new(ch) Player();
    ch->desc = d;


    /*
     * some additional 'magic', probably should go to init_char() for newbies
     * or to character constructor
     */
    ch->player.time.birth = time(0);
    ch->player.time.logon = time(0);
    ch->player.time.played = 0;
    GET_CHA(ch) = 13;
    GET_CON(ch) = 13;
    GET_DEX(ch) = 13;
    GET_INT(ch) = 13;
    GET_STR(ch) = 13;
    GET_WIS(ch) = 13;
    GET_LCK(ch) = 5;
    GET_EXP(ch) = 1;
    GET_HIT(ch) = 20;
    GET_INIT_HIT(ch) = 20;
    GET_MOVE(ch) = 82;
    GET_MAX_MOVE(ch) = 82;
    GET_HEIGHT(ch) = 20;
    GET_WEIGHT(ch) = 20;
    GET_FVCLASS(ch) = 1;
    GET_LEVEL(ch) = 1;
    GET_RELIGION(ch) = 1;
    GET_REMORT(ch)= 0;
    ch->sw = 78;
    ch->sh = 22;
    strcpy(ch->divd, "!");
    strcpy(ch->divr, "#");

    /*
     * read the profile
     */
    while (fbgetline(fl, line)) {
        tag_argument(line, tag);
        for (i = 0; !(line[i] == ' ' || line[i] == '\0'); i++) {
            line1[i] = line[i];
        }
        line1[i] = '\0';
        num = atoi(line1);
        lnum = atol(line1);
        llnum = atoll(line1);


        switch (*tag) {
            case 'A':
                /*if(!strcmp(tag, "Ac  "))
                   GET_AC(ch) = num;
                   else */
                if (!strcmp(tag, "Act "))
                    asciiflag_conv(line, &PLR_FLAGS(ch, 0));
                else if (!strcmp(tag, "Aff "))
                    asciiflag_conv(line, &AFF_FLAGS(ch, 0));
                else if (!strcmp(tag, "Affs")) {
                    i = 0;
                    do {
                        fbgetline(fl, line);
                        sscanf(line, "%d %d %d %d %ld %d %d %ld %s", &num, &num2, &num3, &num4,
                               &lnum, &num5, &num6, &lnum1, line1);
                        if (num > 0) {
                            af.type = num;
                            af.duration = num2;
                            af.modifier = num3;
                            af.location = num4;
                            af.bitvector = lnum;
                            af.main = num5;
                            af.battleflag = num6;
                            af.owner = lnum1;
                            //log("Загружаю эффект %d на %s",num,GET_NAME(ch));
                            affect_to_char(ch, &af);
                            i++;
                        }
                    } while (num != 0);
                } else if (!strcmp(tag, "Alin"))
                    GET_ALIGNMENT(ch) = num;
                break;

            case 'B':
                if (!strcmp(tag, "Badp"))
                    GET_BAD_PWS(ch) = num;
                else if (!strcmp(tag, "Bank"))
                    GET_BANK_GOLD(ch) = lnum;
                else if (!strcmp(tag, "Brth"))
                    ch->player.time.birth = lnum;
                break;

            case 'C':
                if (!strcmp(tag, "Cha "))
                    GET_CHA(ch) = num;
                else if (!strcmp(tag, "Con "))
                    GET_CON(ch) = num;
                else if (!strcmp(tag, "ComS"))
                    SET_COMMSTATE(ch, num);
                break;

            case 'D':
                if (!strcmp(tag, "Desc")) {
                    ch->player.description = fbgetstring(fl);
                } else if (!strcmp(tag, "Dex "))
                    GET_DEX(ch) = num;
                else if (!strcmp(tag, "Divd"))
                    strcpy(ch->divd, line);
                else if (!strcmp(tag, "Divr"))
                    strcpy(ch->divr, line);
                else if (!strcmp(tag, "Drnk"))
                    GET_COND(ch, DRUNK) = num;
                else if (!strcmp(tag, "DrSt"))
                    GET_DRUNK_STATE(ch) = num;
                else if (!strcmp(tag, "DmbD"))
                    DUMB_DURATION(ch) = lnum;
                else if (!strcmp(tag, "Drol"))
                    GET_DR(ch) = num;
                else if (!strcmp(tag, "Dlev"))
                    ch->delete_level = num;
                else if (!strcmp(tag, "Dnam"))
                    strcpy(ch->delete_name, line);
                break;

            case 'E':
                if (!strcmp(tag, "Exp "))
                    GET_EXP(ch) = lnum;
                else if (!strcmp(tag, "EMal"))
                    strcpy(GET_EMAIL(ch), line);
                else if (!strcmp(tag, "Eyes"))
                    GET_EYES(ch) = num;
                else if (!strcmp(tag, "Ench")) {
                    do {
                        fbgetline(fl, line);
                        sscanf(line, "%d %d", &num, &num2);
                        if (num != 0)
                            GET_ENCHANT_TYPE(ch, num) = num2;
                    } while (num != 0);
                }
                break;

            case 'F':
                if (!strcmp(tag, "Frez"))
                    GET_FREEZE_LEV(ch) = num;
                else if (!strcmp(tag, "FrzD"))
                    FREEZE_DURATION(ch) = lnum;
                break;

            case 'G':
                if (!strcmp(tag, "Gold"))
                    GET_GOLD(ch) = num;
                else if (!strcmp(tag, "Gods"))
                    GET_GODS(ch) = num;
                else if (!strcmp(tag, "GodD"))
                    GODS_DURATION(ch) = lnum;
                else if (!strcmp(tag, "Glor"))
                    GET_GLORY(ch) = num;
                else if (!strcmp(tag, "GdFl"))
                    ch->pc()->specials.saved.GodsLike = lnum;
                else if (!strcmp(tag, "GUID"))
                    ch->guid = llnum;
                break;

            case 'H':
                if (!strcmp(tag, "Hit "))
                    GET_HIT(ch) = num;
                else if (!strcmp(tag, "Hite"))
                    GET_HEIGHT(ch) = num;
                else if (!strcmp(tag, "Home"))
                    GET_HOME(ch) = num;
                else if (!strcmp(tag, "Hrol"))
                    GET_HR(ch) = num;
                else if (!strcmp(tag, "Hlth"))
                    GET_HLT(ch) = num;
                else if (!strcmp(tag, "Hung"))
                    GET_COND(ch, FULL) = num;
                else if (!strcmp(tag, "HelD"))
                    HELL_DURATION(ch) = lnum;
                else if (!strcmp(tag, "HsID"))
                    GET_HOUSE_UID(ch) = num;
                else if (!strcmp(tag, "Honr"))
                    GET_HONOR(ch) = num;
                else if (!strcmp(tag, "Host"))
                    strcpy(GET_LASTIP(ch), line);
                else if (!strcmp(tag, "HiSc"))
                    ch->sh = num;
                else if (!strcmp(tag, "Hlvl")) {
                    i = 0;
                    do {
                        fbgetline(fl, line);
                        sscanf(line, "%d %d", &num, &num2);
                        GET_HLEVEL(ch, i) = num2;
                        i++;
                    } while (num2 != 31);
                }
                break;

            case 'I':
                /*if(!strcmp(tag, "IMan"))
                   GET_INIT_MANA(ch) = num;
                   else
                   if(!strcmp(tag, "IHit"))
                   GET_INIT_HIT(ch) = num;
                   else */
                if (!strcmp(tag, "Id  "))
                    GET_IDNUM(ch) = lnum;
                else if (!strcmp(tag, "Int "))
                    GET_INT(ch) = num;
                else if (!strcmp(tag, "Invs"))
                    ch->specials.saved.invis_level = num;
                break;

            case 'L':
                if (!strcmp(tag, "LstL"))
                    LAST_LOGON(ch) = lnum;
                else if (!strcmp(tag, "Levl"))
                    GET_LEVEL(ch) = num;
                else if (!strcmp(tag, "Lck "))  //удача
                    GET_LCK(ch) = num;
                else if (!strcmp(tag, "Last"))
                    ch->player.time.logon = lnum;
                //else if(!strcmp(tag, "LMem"))
                //  GET_LASTMEM(ch) = num;
                break;

            case 'M':
                if (!strcmp(tag, "Mana"))
                    GET_MANA(ch) = num;
                else if (!strcmp(tag, "Move")) {
                    sscanf(line, "%d/%d", &num, &num2);
                    GET_MOVE(ch) = num;
                    GET_MAX_MOVE(ch) = num2;
                } else if (!strcmp(tag, "Mcls")) {
                    do {
                        fbgetline(fl, line);
                        sscanf(line, "%d %d %d", &num, &num2, &num3);
                        if (num2 != 0) {
                            add_class(ch, num, num2, num3);
                        }
                    } while (num2 != 0);
                } else if (!strcmp(tag, "MutD"))
                    MUTE_DURATION(ch) = lnum;
                else if (!strcmp(tag, "Mob ")) {
                    sscanf(line, "%d %d", &num, &num2);
                    inc_kill_vnum(ch, num, num2);
                }

                break;

            case 'N':
                if (!strcmp(tag, "Name"))
                    GET_NAME(ch) = str_dup(line);
                else if (!strcmp(tag, "NmI "))
                    GET_PAD(ch, 0) = str_dup(line);
                else if (!strcmp(tag, "NmR "))
                    GET_PAD(ch, 1) = str_dup(line);
                else if (!strcmp(tag, "NmD "))
                    GET_PAD(ch, 2) = str_dup(line);
                else if (!strcmp(tag, "NmV "))
                    GET_PAD(ch, 3) = str_dup(line);
                else if (!strcmp(tag, "NmT "))
                    GET_PAD(ch, 4) = str_dup(line);
                else if (!strcmp(tag, "NmP "))
                    GET_PAD(ch, 5) = str_dup(line);
                else if (!strcmp(tag, "NamD"))
                    NAME_DURATION(ch) = lnum;
                else if (!strcmp(tag, "NamG"))
                    NAME_GOD(ch) = num;
                else if (!strcmp(tag, "NaID"))
                    NAME_ID_GOD(ch) = lnum;
                break;



            case 'P':
                if (!strcmp(tag, "Pass"))
                    strcpy(GET_PASSWD(ch), line);
                else if (!strcmp(tag, "Plyd"))
                    ch->player.time.played = num;
                else if (!strcmp(tag, "PfIn"))
                    POOFIN(ch) = str_dup(line);
                else if (!strcmp(tag, "PfOt"))
                    POOFOUT(ch) = str_dup(line);
                else if (!strcmp(tag, "Pref"))
                    asciiflag_conv(line, &PRF_FLAGS(ch, 0));
                else if (!strcmp(tag, "PK  "))
                    IS_KILLER(ch) = lnum;
                break;

            case 'Q':
                if (!strcmp(tag, "Qst "))
                    set_quested(ch, num);
                break;


            case 'R':
                if (!strcmp(tag, "Room"))
                    GET_LOADROOM(ch) = num;
                else if (!strcmp(tag, "Race"))
                    GET_RACE(ch) = num;
                else if (!strcmp(tag, "Regs"))
                    ch->registry_code = str_dup(line);
                else if (!strcmp(tag, "Rank"))
                    GET_HOUSE_RANK(ch) = num;
                else if (!strcmp(tag, "RMem")) {
                    do {
                        fbgetline(fl, line);
                        sscanf(line, "%d %ld %s", &num, &lnum, line1);
                        if (num != 0) {
                            log("%s добавляю место %d %ld %s", GET_NAME(ch), num, lnum, line1);
                            add_memory(ch, num, lnum, line1);
                        }
                    } while (num != 0);
                }
                else if (!strcmp(tag, "Rmrt"))
                    GET_REMORT(ch) = num;
                break;

            case 'S':
                if (!strcmp(tag, "Size"))
                    GET_SIZE(ch) = num;
                else if (!strcmp(tag, "Sex "))
                    GET_SEX(ch) = num;
                else if (!strcmp(tag, "Skil")) {
                    do {
                        fbgetline(fl, line);
                        sscanf(line, "%d %d %d %ld", &num, &num2, &num3, &lnum);
                        if (num != 0) {
                            SET_SKILL(ch, num) = num2;
                            GET_SKILL_LEVEL(ch, num) = num3;
                        }
                    } while (num != 0);
                } else if (!strcmp(tag, "SkTm")) {
                    do {
                        fbgetline(fl, line);
                        sscanf(line, "%d %d", &num, &num2);
                        if (num != 0) {
                            timed.skill = num;
                            timed.time = num2;
                            timed_to_char(ch, &timed);
                        }
                    } while (num != 0);
                } else if (!strcmp(tag, "Spel")) {
                    do {
                        fbgetline(fl, line);
                        sscanf(line, "%d %d", &num, &num2);
                        if (num != 0)
                            GET_SPELL_TYPE(ch, num) = num2;
                    } while (num != 0);
                } else if (!strcmp(tag, "SpMe")) {
                    do {
                        fbgetline(fl, line);
                        sscanf(line, "%d %d", &num, &num2);
                        if (num != 0)
                            GET_SPELL_MEM(ch, num) = num2;
                    } while (num != 0);
                } else if (!strcmp(tag, "Str "))
                    GET_STR(ch) = num;
                else if (!strcmp(tag, "Slep"))
                    GET_COND(ch, SLEEP) = num;
                else if (!strcmp(tag, "Spek"))
                    SPEAKING(ch) = num;
                else if (!strcmp(tag, "SExp")) {
                    sscanf(line, "%d %d", &num, &num2);
                    inc_exp_script_num(ch, num, num2);
                }
                break;

            case 'T':
                if (!strcmp(tag, "Thir"))
                    GET_COND(ch, THIRST) = num;
                else if (!strcmp(tag, "Titl"))
                    GET_TITLE(ch) = str_dup(line);
                else if (!strcmp(tag, "Titr"))
                    GET_RTITLE(ch) = str_dup(line);
                else if (!strcmp(tag, "Tick"))
                    GET_TICKS(ch) = lnum;
                break;

            case 'W':
                if (!strcmp(tag, "Wate"))
                    GET_WEIGHT(ch) = num;
                else if (!strcmp(tag, "Wimp"))
                    GET_WIMP_LEV(ch) = num;
                else if (!strcmp(tag, "Wis "))
                    GET_WIS(ch) = num;
                else if (!strcmp(tag, "WiSc"))
                    ch->sw = num;
                break;

            case 'U':
                if (!strcmp(tag, "UIN "))
                    GET_UNIQUE(ch) = num;
                break;
            default:
                sprintf(buf, "SYSERR: Unknown tag %s in pfile %s", tag, name);
        }
    }

    /*affect_total(ch); */
    if (GET_HLT(ch) == 0)
        GET_HLT(ch) = 100;

    /* initialization for imms */
    if (IS_GOD(ch)) {
        GET_SIZE(ch) = 100;
        for (i = 1; i <= TOP_SKILL_DEFINE; i++) {
            SET_SKILL(ch, i) = 100;
            GET_SKILL_LEVEL(ch, i) = 5;
        }
        GET_COND(ch, FULL) = -1;
        GET_COND(ch, THIRST) = -1;
        GET_COND(ch, DRUNK) = -1;
        GET_COND(ch, SLEEP) = -1;
        GET_LOADROOM(ch) = NOWHERE;
    }
    if (IS_GOD(ch)) {
        for (i = 0; i <= TOP_SPELL_DEFINE; i++)
            GET_SPELL_TYPE(ch, i) = GET_SPELL_TYPE(ch, i) |
                SPELL_ITEMS | SPELL_KNOW | SPELL_RUNES | SPELL_SCROLL | SPELL_POTION | SPELL_WAND;
    }

    /*
     * If you're not poisioned and you've been away for more than an hour of
     * real time, we'll set your HMV back to full
     */
    /*  if (!AFF_FLAGGED(ch, AFF_POISON) &&
       (((long) (time(0) - LAST_LOGON(ch))) >= SECS_PER_REAL_HOUR))
       { GET_HIT(ch)  = GET_REAL_MAX_HIT(ch);
       GET_MOVE(ch) = GET_REAL_MAX_MOVE(ch);
       }
       else
       GET_HIT(ch) = MIN(GET_HIT(ch), GET_REAL_MAX_HIT(ch)); */

    if (affected_by_spell(ch, SPELL_HIDE))
        affect_from_char(ch, SPELL_HIDE);
    if (affected_by_spell(ch, SPELL_SNEAK))
        affect_from_char(ch, SPELL_SNEAK);
    if (affected_by_spell(ch, SPELL_CAMOUFLAGE))
        affect_from_char(ch, SPELL_CAMOUFLAGE);
    if (affected_by_spell(ch, SPELL_MEDITATION))
        affect_from_char(ch, SPELL_MEDITATION);
    if (AFF_FLAGGED(ch, AFF_GROUP))
        REMOVE_BIT(AFF_FLAGS(ch, AFF_GROUP), AFF_GROUP);
    if (AFF_FLAGGED(ch, AFF_HORSE))
        REMOVE_BIT(AFF_FLAGS(ch, AFF_HORSE), AFF_HORSE);
    if (AFF_FLAGGED(ch, AFF_MEDITATION))
        REMOVE_BIT(AFF_FLAGS(ch, AFF_MEDITATION), AFF_MEDITATION);
    if (PLR_FLAGGED(ch, PLR_DROPLINK))
        REMOVE_BIT(PLR_FLAGS(ch, PLR_DROPLINK), PLR_DROPLINK);

    if (PLR_FLAGGED(ch, PLR_SOUL) && !EXTRACT_TIMER(ch))
        REMOVE_BIT(PLR_FLAGS(ch, PLR_SOUL), PLR_SOUL);

    if (!GET_EYES(ch))
        GET_EYES(ch) = generate_eyes(ch);

    //Создаем части тел
    CItem *body = tBody.FindItem(GET_RACE(ch));

    if (body) {
        int numadd = body->GetItem(TBD_BODY)->GetNumberItem();

        if (numadd) {
            ch->ibody = numadd;
            ch->body = new struct body_part[numadd + 1];

            for (int j = 0; j < numadd; j++) {
                CItem *bdb = body->GetItem(TBD_BODY)->GetItem(j);

                ch->body[j].wear_position = bdb->GetItem(TBD_BODY_POSITION)->GetInt();
                if (bdb->GetItem(TBD_BODY_SNAME)->GetInt())
                    ch->body[j].name = body_name[bdb->GetItem(TBD_BODY_SNAME)->GetInt()];
                else if (bdb->GetItem(TBD_BODY_NAME)->GetString())
                    ch->body[j].name = bdb->GetItem(TBD_BODY_NAME)->GetString();
                ch->body[j].chance = bdb->GetItem(TBD_BODY_CHANCE)->GetInt();
                ch->body[j].kdam = bdb->GetItem(TBD_BODY_KDAM)->GetInt();
            }
        }
    }

    fbclose(fl);
    return (id);
}

int load_char(char *name, struct char_data *char_element)
{
    int player_i;

    player_i = load_char_ascii(name, char_element->pc());
    if (player_i > -1) {
        GET_PFILEPOS(char_element) = player_i;
    }
    return (player_i);

}

/*
 * write the vital data of a player to the player file
 *
 * NOTE: load_room should be an *RNUM* now.  It is converted to a vnum here.
 */

void save_char(struct char_data *ch, room_rnum load_room)
{
    if (!now_entrycount)
        if (IS_NPC(ch) || /* !ch->desc || */ GET_PFILEPOS(ch) < 0)
            return;

    new_save_char(ch, load_room);
//   write_aliases(ch);
}



/*
 * Create a new entry in the in-memory index table for the player file.
 * If the name already exists, by overwriting a deleted character, then
 * we re-use the old position.
 */
int create_entry(char *name)
{
    int i = 0, pos = 0, found = TRUE;

    if (top_of_p_table == -1) { /* no table */
        CREATE(player_table, struct player_index_element, 1);

        pos = top_of_p_table = 0;
        found = FALSE;
    } else if ((pos = get_ptable_by_name(name)) == -1) {        /* new name */
        i = ++top_of_p_table;
        //RECREATE(player_table, struct player_index_element, i+1);
        pos = top_of_p_table;
        found = FALSE;
    }

    CREATE(player_table[pos].name, char, strlen(name) + 1);

    /* copy lowercase equivalent of name to table field */
    for (i = 0, player_table[pos].name[i] = '\0';
         (player_table[pos].name[i] = LOWER(name[i])); i++);
    /* create new save activity */
    player_table[pos].activity = number(0, OBJECT_SAVE_ACTIVITY - 1);
    player_table[pos].timer = 0;

    return (pos);
}



/************************************************************************
*  funcs of a (more or less) general utility nature                     *
************************************************************************/


/* read and allocate space for a '~'-terminated string from a given file */
char *fread_string(FILE * fl, char *error)
{
    char buf[MAX_EXTEND_LENGTH], tmp[512], *rslt;
    register char *point;
    int done = 0, length = 0, templength;

    *buf = '\0';

    do {
        if (!fgets(tmp, 512, fl)) {
            log("SYSERR: fread_string: format error at or near %s", error);
            exit(1);
        }
        /* If there is a '~', end the string; else put an "\r\n" over the '\n'. */
        if ((point = strchr(tmp, '~')) != NULL) {
            /* Два символа '~' подряд интерпретируются как '~' и
               строка продолжается.
               Можно не использовать.
               Позволяет писать в триггерах что-то типа
               wat 26000 wechoaround %actor% ~%actor.name% появил%actor.u% тут в клубах дыма.
               Чтобы такая строва правильно загрузилась, в trg файле следует указать два символа '~'
               wat 26000 wechoaround %actor% ~~%actor.name% появил%actor.u% тут в клубах дыма.
             */
            if (point[1] != '~') {
                *point = '\0';
                done = 1;
            } else {
                memmove(point, point + 1, strlen(point));
            }
        } else {
            point = tmp + strlen(tmp) - 1;
            *(point++) = '\r';
            *(point++) = '\n';
            *point = '\0';
        }

        templength = strlen(tmp);

        if (length + templength >= MAX_EXTEND_LENGTH) {
            log("SYSERR: fread_string: string too large (db.c)");
            log(error);
            exit(1);
        } else {
            strcat(buf + length, tmp);
            length += templength;
        }
    } while (!done);

    /* allocate space for the new string and copy it */
    if (strlen(buf) > 0) {
        CREATE(rslt, char, length + 1);

        strcpy(rslt, buf);
    } else
        rslt = NULL;

    return (rslt);
}




/* initialize a new character only if class is set */
void init_char(struct char_data *ch)
{
    int i, start_room = NOWHERE;


    /* *** if this is our first player --- he be God *** */

    if (top_of_p_table == 0) {
        GET_EXP(ch) = EXP_IMPL;
        GET_LEVEL(ch) = LVL_IMPL;

        GET_MAX_HIT(ch) = 1000;
        GET_MAX_MOVE(ch) = 82;
        GET_MAX_MANA(ch) = 1000;
        start_room = immort_start_room;

    } else
        start_room = mortal_start_room;

    set_title(ch, NULL);
    ch->player.short_descr = NULL;
    ch->player.long_descr = NULL;
    ch->player.description = NULL;
    ch->player.hometown = 1;
    ch->player.time.birth = time(0);
    ch->player.time.played = 0;
    ch->player.time.logon = time(0);

    for (i = 0; i < MAX_TONGUE; i++)
        GET_TALK(ch, i) = 0;


    ch->points.hit = GET_MAX_HIT(ch);
    ch->points.max_move = 82;
    ch->points.move = GET_MAX_MOVE(ch);

    GET_MANA(ch) = GET_REAL_MAX_MANA(ch);
    GET_AC(ch) = 0;

    if ((i = get_ptable_by_name(GET_NAME(ch))) != -1) {
        player_table[i].id = GET_IDNUM(ch) = ++top_idnum;
        player_table[i].unique = GET_UNIQUE(ch) = create_unique();
        player_table[i].level = 1;
        player_table[i].last_logon = -1;
    } else {
        log("SYSERR: init_char: Character '%s' not found in player table.", GET_NAME(ch));
    }

    for (i = 1; i <= MAX_SKILLS; i++) {
        if (GET_LEVEL(ch) < LVL_GRGOD)
            SET_SKILL(ch, i) = 0;
        else
            SET_SKILL(ch, i) = 100;
    }
    for (i = 1; i <= MAX_SPELLS; i++) {
        if (GET_LEVEL(ch) < LVL_GRGOD)
            GET_SPELL_TYPE(ch, i) = 0;
        else
            GET_SPELL_TYPE(ch, i) = SPELL_KNOW;
    }

    ch->char_specials.saved.affected_by = clear_flags;
    for (i = 0; i < SAV_NUMBER; i++)
        GET_SAVE3(ch, i) = 0;

    for (i = 0; i < 8; i++)
        GET_INC_MAGIC(ch, i) = 0;

    ch->real_abils.intel = 30;
    ch->real_abils.wis = 30;
    ch->real_abils.dex = 30;
    ch->real_abils.str = 30;
    ch->real_abils.con = 30;
    ch->real_abils.cha = 30;
    ch->real_abils.size = 100;


    if (GET_LEVEL(ch) == LVL_IMPL)
        for (i = 0; i < NUM_CLASSES; i++) {
            add_class(ch, i, GET_LEVEL(ch), 1);
        }

    if (GET_LEVEL(ch) != LVL_IMPL)
        init_stats(ch);

    for (i = 0; i < 4; i++) {
        GET_COND(ch, i) = (GET_LEVEL(ch) == LVL_IMPL ? -1 : i == DRUNK ? 0 : 24);
    }
    GET_REMORT(ch) = 0;
    GET_LASTIP(ch)[0] = 0;
    GET_LOADROOM(ch) = start_room;
    SET_BIT(PLR_FLAGS(ch, PLR_LOADROOM), PLR_LOADROOM);
    SET_BIT(PRF_FLAGS(ch, PRF_DISPHP), PRF_DISPHP);

    /*switch (ch->player.chclass)
       {
       case CLASS_MAGIC_USER:
       case CLASS_PRIEST:
       SET_BIT(PRF_FLAGS(ch,PRF_DISPMANA), PRF_DISPMANA);
       break;
       } */
    SET_BIT(PRF_FLAGS(ch, PRF_DISPEXITS), PRF_DISPEXITS);
    SET_BIT(PRF_FLAGS(ch, PRF_DISPMOVE), PRF_DISPMOVE);
    SET_BIT(PRF_FLAGS(ch, PRF_DISPEXP), PRF_DISPMOVE);
    SET_BIT(PRF_FLAGS(ch, PRF_SUMMONABLE), PRF_SUMMONABLE);
    SET_BIT(PRF_FLAGS(ch, PRF_DISPGOLD), PRF_DISPGOLD);
    SET_BIT(PRF_FLAGS(ch, PRF_DISPEXP), PRF_DISPEXP);
    SET_BIT(PRF_FLAGS(ch, PRF_AUTOMEM), PRF_AUTOMEM);
    SET_BIT(PRF_FLAGS(ch, PRF_AUTOFRM), PRF_AUTOFRM);
    SET_BIT(PRF_FLAGS(ch, PRF_COLOR_1), PRF_COLOR_1);
    SET_BIT(PRF_FLAGS(ch, PRF_COLOR_2), PRF_COLOR_2);


    save_char(ch, NOWHERE);
}


/* returns the real number of the room with given virtual number */
/*
room_rnum real_room(room_vnum vnum)
{
  room_rnum bot, top, mid;

  bot = 0;
  top = top_of_world;

  for (;;)
  { mid = (bot + top) / 2;

    log("bot:%d top:%d",bot,top);

    if ((world + mid)->number == vnum)
      return (mid);
    if (bot >= top)
      return (NOWHERE);
    if ((world + mid)->number > vnum)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}
*/

int real_room(room_vnum vnum)
{
    int r = NOWHERE;


    for (r = 0; r <= top_of_world; r++) {
        if (world[r].number == vnum)
            return (r);
    }

    return (-1);
}

mob_rnum real_mobile(mob_vnum vnum)
{
    mob_rnum mob = -1;

    for (mob = 0; mob <= top_of_mobt; mob++)
        if ((mob_index + mob)->vnum == vnum)
            return (mob);

    return (-1);
}

/*
mob_rnum real_mobile(mob_vnum vnum)
{
  mob_rnum bot, top, mid;

  bot = 0;
  top = top_of_mobt;
  log ("top %d",top);

  for (;;)
  { mid = (bot + top) / 2;

    log ("mid %d bot %d top %d vnum %d mob_vnum = %d",mid,bot,top,vnum,
     (mob_index + mid)->vnum);
    if ((mob_index + mid)->vnum == vnum)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((mob_index + mid)->vnum > vnum)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}
*/



/* Возвращает реальный номер зоны по определению через виртуальный */
zone_rnum real_zone_vnum(zone_vnum vnum)
{
    zone_rnum zone = -1;

    for (zone = 0; zone <= top_of_zone_table; zone++)
        if (zone_table[zone].number == vnum)
            return (zone);
    return (-1);
}

/* returns the real number of the object with given virtual number */
obj_rnum real_object(obj_vnum vnum)
{
    obj_rnum obj = -1;

    for (obj = 0; obj <= top_of_objt; obj++) {
        if ((obj_index + obj)->vnum == vnum)
            return (obj);
    }
    return (-1);
}

/*
  bot = 0;
  top = top_of_objt;

  for (;;)
  { mid = (bot + top) / 2;

    if ((obj_index + mid)->vnum == vnum)
      return (mid);
    if (bot >= top)
      return (-1);
    if ((obj_index + mid)->vnum > vnum)
      top = mid - 1;
    else
      bot = mid + 1;
  }
}
*/

/*
 * Extend later to include more checks.
 *
 * TODO: Add checks for unknown bitvectors.
 */
int check_object(struct obj_data *obj)
{
    int error = FALSE;
    char buf[MAX_STRING_LENGTH];

    if (GET_OBJ_WEIGHT(obj) < 0 && (error = TRUE))
        log("SYSERR: Object #%d (%s) has negative weight (%d).",
            GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_WEIGHT(obj));

    sprintbit(GET_OBJ_WEAR(obj), wear_bits, buf, ",");
    if (strstr(buf, "UNDEFINED") && (error = TRUE))
        log("SYSERR: Object #%d (%s) has unknown wear flags.",
            GET_OBJ_VNUM(obj), obj->short_description);

    sprintbits(obj->obj_flags.extra_flags, extra_bits, buf, ",");
    if (strstr(buf, "UNDEFINED") && (error = TRUE))
        log("SYSERR: Object #%d (%s) has unknown extra flags.",
            GET_OBJ_VNUM(obj), obj->short_description);

    sprintbits(obj->obj_flags.affects, affected_bits, buf, ",");

    if (strstr(buf, "UNDEFINED") && (error = TRUE))
        log("SYSERR: Object #%d (%s) has unknown affection flags.",
            GET_OBJ_VNUM(obj), obj->short_description);

    switch (GET_OBJ_TYPE(obj)) {
            /* Что-то тут глючит...
               Теперь мы лишились возможности пить воду... придеться
               набирать  пить желез(ная фляжка)
               вместо пить воду

               case ITEM_DRINKCON:
               { char onealias[MAX_INPUT_LENGTH], *space = strchr(obj->name, ' ');

               int offset = space ? space - obj->name: strlen(obj->name);

               strncpy(onealias, obj->name, offset);
               onealias[offset] = '\0';

               if (search_block(onealias, drinknames, TRUE) < 0 && (error = TRUE))
               log("ОШИБКА: Обьект #%d (%s) не стоит тип напитка на последнем псевдониме. (%s)",
               GET_OBJ_VNUM(obj), obj->short_description, obj->name);
               } */
            /* Fall through. */
        case ITEM_FOUNTAIN:
            if (GET_OBJ_VAL(obj, 1) > GET_OBJ_VAL(obj, 0) && (error = TRUE))
                log("SYSERR: Object #%d (%s) contains (%d) more than maximum (%d).",
                    GET_OBJ_VNUM(obj), obj->short_description,
                    GET_OBJ_VAL(obj, 1), GET_OBJ_VAL(obj, 0));
            break;
        case ITEM_SCROLL:
        case ITEM_POTION:
            error |= check_object_level(obj, 0);
            error |= check_object_spell_number(obj, 1);
            error |= check_object_spell_number(obj, 2);
            error |= check_object_spell_number(obj, 3);
            break;
        case ITEM_BOOK:
            error |= check_object_spell_number(obj, 1);
            break;
        case ITEM_WAND:
        case ITEM_STAFF:
            error |= check_object_level(obj, 0);
            error |= check_object_spell_number(obj, 3);
            if (GET_OBJ_VAL(obj, 2) > GET_OBJ_VAL(obj, 1) && (error = TRUE))
                log("SYSERR: Object #%d (%s) has more charges (%d) than maximum (%d).",
                    GET_OBJ_VNUM(obj), obj->short_description,
                    GET_OBJ_VAL(obj, 2), GET_OBJ_VAL(obj, 1));
            break;
    }

    return (error);
}

int check_object_spell_number(struct obj_data *obj, int val)
{
    int error = FALSE;
    const char *spellname;

    if (GET_OBJ_VAL(obj, val) == -1)    /* i.e.: no spell */
        return (error);

    /*
     * Check for negative spells, spells beyond the top define, and any
     * spell which is actually a skill.
     */
    if (GET_OBJ_VAL(obj, val) < 0)
        error = TRUE;
    if (GET_OBJ_VAL(obj, val) > MAX_SPELLS)
        error = TRUE;
    if (error)
        log("SYSERR: Object #%d (%s) has out of range spell #%d.",
            GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_VAL(obj, val));


    /* Now check for unnamed spells. */
    spellname = spell_name(find_spell_num(GET_OBJ_VAL(obj, val)));
    if ((spellname == unused_spellname || !str_cmp("UNDEFINED", spellname)) && (error = TRUE))
        log("SYSERR: Object #%d (%s) uses '%s' spell #%d.",
            GET_OBJ_VNUM(obj), obj->short_description, spellname, GET_OBJ_VAL(obj, val));

    return (error);
}

int check_object_level(struct obj_data *obj, int val)
{
    int error = FALSE;

    if ((GET_OBJ_VAL(obj, val) < 0 || GET_OBJ_VAL(obj, val) > LVL_IMPL) && (error = TRUE))
        log("SYSERR: Object #%d (%s) has out of range level #%d.",
            GET_OBJ_VNUM(obj), obj->short_description, GET_OBJ_VAL(obj, val));
    return (error);
}

void new_build_player_index(void)
{
    int rec_count = 0, i;
    FBFILE *plr_index;
    char line[256], arg2[80];

    player_table = NULL;
    top_of_p_file = top_of_p_table = -1;

    MudFile file(mud->playerIndexFile);

    if (!(plr_index = fbopen(file.getCPath(), FB_READ))) {
        top_of_p_table = -1;
        log("ПРЕДУПРЕЖДЕНИЕ: Не найден файл идекса игроков. Первый созданый игрок будет имморталом");
        return;
    }

    while (fbgetline(plr_index, line))
        if (*line != '~')
            rec_count++;
    fbrewind(plr_index);

    if (rec_count == 0) {
        player_table = NULL;
        top_of_p_table = -1;
        return;
    }

    CREATE(player_table, struct player_index_element, rec_count + 100);
    for (i = 0; i < rec_count; i++) {
        fbgetline(plr_index, line);
        sscanf(line, "%ld %s %ld %d %d %d", &player_table[i].id, arg2,
               &player_table[i].unique, &player_table[i].level,
               &player_table[i].last_logon, &player_table[i].timer);
        CREATE(player_table[i].name, char, strlen(arg2) + 1);

        strcpy(player_table[i].name, arg2);
        top_idnum = MAX(top_idnum, player_table[i].id);
    }

    fbclose(plr_index);
    top_of_p_file = top_of_p_table = i - 1;
}

void save_player_index(void)
{
    int i;
    FBFILE *index_file;
    MudFile file(mud->playerIndexFile);

    if (!(index_file = fbopen(file.getCPath(), FB_WRITE))) {
        log("ОШИБКА: Не могу записать индексный файл игроков");
        return;
    }

    for (i = 0; i <= top_of_p_table; i++)
        if (*player_table[i].name) {
            fbprintf(index_file, "%ld %s %ld %d %d %d\n", player_table[i].id,
                     player_table[i].name, player_table[i].unique, player_table[i].level,
                     player_table[i].last_logon, player_table[i].timer);
        }
    fbprintf(index_file, "~\n");

    fbclose(index_file);

    log("Сохранено индексов %d (считано при загрузке %d)", i, top_of_p_file + 1);
}


/* Load a char, TRUE if loaded, FALSE if not */

/* remove ^M's from file output */
void kill_ems(char *str)
{
    char *ptr1, *ptr2, *tmp;

    tmp = str;
    ptr1 = str;
    ptr2 = str;

    while (*ptr1) {
        if ((*(ptr2++) = *(ptr1++)) == '\r')
            if (*ptr1 == '\r')
                ptr1++;
    }
    *ptr2 = '\0';
}


void new_save_char(struct char_data *ch, room_rnum load_room)
{
    FILE *saved;
    char filename[MAX_STRING_LENGTH], fbackup[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    room_rnum location;
    int i;
    time_t li;
    struct affected_type *aff;
    struct obj_data *char_eq[NUM_WEARS];
    struct obj_data *char_tatoo[NUM_WEARS];
    struct timed_type *skj;

    if (!now_entrycount)
        if (IS_NPC(ch) || /* !ch->desc || */ GET_PFILEPOS(ch) < 0)
            return;

    if (!PLR_FLAGGED(ch, PLR_LOADROOM)) {
        if (load_room > NOWHERE) {
            GET_LOADROOM(ch) = GET_ROOM_VNUM(load_room);
            log("Player %s save at room %d", GET_NAME(ch), GET_ROOM_VNUM(load_room));
        }
    }
    //log("Save char %s inroom %d", GET_NAME(ch),IN_ROOM(ch));

    //new_save_pkills(ch);


    get_filename(GET_NAME(ch), filename, PLAYERS_FILE);
    sprintf(fbackup, "%s.backup", filename);
    if (!(saved = fopen(filename, "w"))) {
        syserr("ОШИБКА: Не могу открыть для записи %ss", filename);
        return;
    }
    /* подготовка */
    //log("SAVE:Снятие доспехов");
    /* снимаем все возможные аффекты  */
    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_EQ(ch, i)) {
            char_eq[i] = unequip_char(ch, i | 0x80 | 0x40);
        } else
            char_eq[i] = NULL;
    }

    /* снимаем все возможные аффекты  */
    //log("SAVE:Снятие тату");
    for (i = 0; i < NUM_WEARS; i++) {
        if (GET_TATOO(ch, i)) {
            char_tatoo[i] = unequip_tatoo(ch, i | 0x80 | 0x40);
        } else
            char_tatoo[i] = NULL;
    }

    /*
     * remove the affections so that the raw values are stored; otherwise the
     * effects are doubled when the char logs back in.
     */
    /*  
       while (ch->affected)
       affect_remove(ch, ch->affected);

       if ((i >= MAX_AFFECT) && aff && aff->next)
       log("SYSERR: WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!"); */

    /* запись */

    if (GET_NAME(ch))
        fprintf(saved, "Name: %s\n", GET_NAME(ch));
    if (GET_PAD(ch, 0))
        fprintf(saved, "NmI : %s\n", GET_PAD(ch, 0));
    if (GET_PAD(ch, 0))
        fprintf(saved, "NmR : %s\n", GET_PAD(ch, 1));
    if (GET_PAD(ch, 0))
        fprintf(saved, "NmD : %s\n", GET_PAD(ch, 2));
    if (GET_PAD(ch, 0))
        fprintf(saved, "NmV : %s\n", GET_PAD(ch, 3));
    if (GET_PAD(ch, 0))
        fprintf(saved, "NmT : %s\n", GET_PAD(ch, 4));
    if (GET_PAD(ch, 0))
        fprintf(saved, "NmP : %s\n", GET_PAD(ch, 5));
    if (GET_PASSWD(ch))
        fprintf(saved, "Pass: %s\n", GET_PASSWD(ch));
    if (GET_EMAIL(ch))
        fprintf(saved, "EMal: %s\n", GET_EMAIL(ch));
    if (ch->registry_code)
        fprintf(saved, "Regs: %s\n", ch->registry_code);
    if (GET_TITLE(ch))
        fprintf(saved, "Titl: %s\n", GET_TITLE(ch));
    if (GET_RTITLE(ch))
        fprintf(saved, "Titr: %s\n", GET_RTITLE(ch));
    if (ch->player.description && *ch->player.description) {
        strcpy(buf, ch->player.description);
        kill_ems(buf);
        fprintf(saved, "Desc:\n%s~\n", buf);
    }
    if (POOFIN(ch))
        fprintf(saved, "PfIn: %s\n", POOFIN(ch));
    if (POOFOUT(ch))
        fprintf(saved, "PfOt: %s\n", POOFOUT(ch));
    fprintf(saved, "Sex : %d %s\n", GET_SEX(ch), genders[(int) GET_SEX(ch)]);

    fprintf(saved, "Divd: %s\n", ch->divd);
    fprintf(saved, "Divr: %s\n", ch->divr);

    fprintf(saved, "Hlvl:\n");
    for (i = 0; i <= 60; i++)
        fprintf(saved, "%d %d\n", i, GET_HLEVEL(ch, i));
    fprintf(saved, "61 31\n");

    /* Записываем мультиклассы */
    fprintf(saved, "Mcls:\n");
    for (int icls = 0; icls < NUM_CLASSES; icls++)
        if (ch->init_classes[icls])
            fprintf(saved, "%d %d %d %s\n", icls, ch->init_classes[icls], 0, class_name[icls]);
    fprintf(saved, "0 0 0 окончание.классов\n");
    fprintf(saved, "Levl: %d\n", GET_LEVEL(ch));
    fprintf(saved, "Gods: %d\n", GET_GODS(ch));
    if ((location = real_room(GET_HOME(ch))) != NOWHERE)
        fprintf(saved, "Home: %d %s\n", GET_HOME(ch), world[(location)].name);
    li = ch->player.time.birth;
    fprintf(saved, "Brth: %ld %s", li, ctime(&li));
    fprintf(saved, "Plyd: %d\n", ch->player.time.played);
    li = LAST_LOGON(ch);
    li = ch->player.time.logon;
    fprintf(saved, "Last: %ld %s", li, ctime(&li));

    //if (ch->desc)
    //  fprintf(saved,"Host: %s\n", ch->desc->host);

    fprintf(saved, "Hite: %d\n", GET_HEIGHT(ch));
    fprintf(saved, "Wate: %d\n", GET_WEIGHT(ch));
    fprintf(saved, "Size: %d\n", GET_SIZE(ch));
    //fprintf(saved,"IHit: %d\n",GET_INIT_HIT(ch));
    //fprintf(saved,"IMan: %d\n",GET_INIT_MANA(ch));
    /* структуры */
    fprintf(saved, "Alin: %d\n", GET_ALIGNMENT(ch));
    fprintf(saved, "Id  : %ld\n", GET_IDNUM(ch));
    fprintf(saved, "UIN : %d\n", GET_UNIQUE(ch));
    fprintf(saved, "GUID: %lld\n", ch->guid);
    *buf = '\0';
    tascii(&PLR_FLAGS(ch, 0), 4, buf);
    fprintf(saved, "Act : %s \n", buf);
    *buf = '\0';
    tascii(&AFF_FLAGS(ch, 0), 4, buf);
    fprintf(saved, "Aff : %s \n", buf);

    /* статсы */
    fprintf(saved, "Str : %d\n", GET_STR(ch));
    fprintf(saved, "Con : %d\n", GET_CON(ch));
    fprintf(saved, "Dex : %d\n", GET_DEX(ch));
    fprintf(saved, "Int : %d\n", GET_INT(ch));
    fprintf(saved, "Wis : %d\n", GET_WIS(ch));
    fprintf(saved, "Cha : %d\n", GET_CHA(ch));
    fprintf(saved, "Lck : %d\n", GET_LCK(ch));  //удача
    fprintf(saved, "Eyes: %d\n", GET_EYES(ch)); //глаза
    fprintf(saved, "Dcnt: %d\n", GET_COUNT_DEATH(ch));
    fprintf(saved, "Rcnt: %d\n", GET_COUNT_RESSURECT(ch));
    fprintf(saved, "Ccnt: %d\n", GET_COUNT_CAPSOUL(ch));

    /* скилы */
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        fprintf(saved, "Skil:\n");
        for (i = 1; i <= MAX_SKILLS; i++) {
            if (SET_SKILL(ch, i))
                fprintf(saved, "%d %d %d %ld %s\n", i, SET_SKILL(ch, i), GET_SKILL_LEVEL(ch, i),
                        (long int) 0, skill_info[i].name.c_str());
        }
        fprintf(saved, "0 0 0 255\n");
    }

    /* Задержки на скилы */
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        log("SkTM %s", GET_NAME(ch));
        fprintf(saved, "SkTm:\n");
        for (skj = ch->timed; skj; skj = skj->next) {
            if (skj->time > 0)
                fprintf(saved, "%d %d %s\n", skj->skill, skj->time,
                        skill_info[skj->skill].name.c_str());
        }
        fprintf(saved, "0 0\n");
    }

    /* рецепты */
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        int nums = xEnchant.GetNumberItem(), ench_no;

        fprintf(saved, "Ench:\n");
        for (i = 0; i < nums; i++) {
            ench_no = ENCHANT_NO(i);
            if (GET_ENCHANT_TYPE(ch, ench_no))
                fprintf(saved, "%d %d %s\n", ench_no, GET_ENCHANT_TYPE(ch, ench_no),
                        ENCHANT_NAME(i));
        }
        fprintf(saved, "0 0\n");
    }

    /* спелы */
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        int nums = Spl.GetNumberItem(), spell_no;

        fprintf(saved, "Spel:\n");
        for (i = 0; i < nums; i++) {
            spell_no = Spl.GetItem(i)->GetItem(SPL_NUMBER)->GetInt();
            if (GET_SPELL_TYPE(ch, spell_no))
                fprintf(saved, "%d %d %s\n", spell_no, GET_SPELL_TYPE(ch, spell_no), SPELL_NAME(i));
        }
        fprintf(saved, "0 0\n");
    }

    /* Замемленые спелы */
    if (GET_LEVEL(ch) < LVL_IMMORT) {
        fprintf(saved, "SpMe:\n");
        for (i = 1; i <= MAX_SPELLS; i++) {
            if (GET_SPELL_MEM(ch, i))
                fprintf(saved, "%d %ld %s\n", i, GET_SPELL_MEM(ch, i), SPELL_NAME(i));
        }
        fprintf(saved, "0 0\n");
    }
    fprintf(saved, "Hrol: %d\n", GET_HR(ch));
    fprintf(saved, "Drol: %d\n", GET_DR(ch));
    fprintf(saved, "Ac  : %d\n", GET_AC(ch));

    fprintf(saved, "Hlth: %d\n", GET_HLT(ch));
    fprintf(saved, "Hit : %d\n", GET_HIT(ch));
    fprintf(saved, "Mana: %d\n", GET_MANA(ch));
    fprintf(saved, "Move: %d/%d\n", GET_MOVE(ch), GET_MAX_MOVE(ch));
    fprintf(saved, "Gold: %d\n", GET_GOLD(ch));
    fprintf(saved, "Bank: %ld\n", GET_BANK_GOLD(ch));
    fprintf(saved, "Exp : %ld\n", GET_EXP(ch));
    fprintf(saved, "Honr: %ld\n", GET_HONOR(ch));
    fprintf(saved, "PK  : %ld\n", IS_KILLER(ch));
    fprintf(saved, "Wimp: %d\n", GET_WIMP_LEV(ch));
    fprintf(saved, "Frez: %d\n", GET_FREEZE_LEV(ch));
    fprintf(saved, "Invs: %d\n", GET_INVIS_LEV(ch));
    fprintf(saved, "Room: %d\n", GET_LOADROOM(ch));

    fprintf(saved, "Badp: %d\n", GET_BAD_PWS(ch));
    fprintf(saved, "Hung: %d\n", GET_COND(ch, FULL));
    fprintf(saved, "Thir: %d\n", GET_COND(ch, THIRST));
    fprintf(saved, "Drnk: %d\n", GET_COND(ch, DRUNK));
    fprintf(saved, "Slep: %d\n", GET_COND(ch, SLEEP));


    fprintf(saved, "Race: %d %s\n", GET_RACE(ch), race_name[(int) GET_RACE(ch)][(int) GET_SEX(ch)]);
    fprintf(saved, "DrSt: %d\n", GET_DRUNK_STATE(ch));
    fprintf(saved, "ComS: %d\n", GET_COMMSTATE(ch));
    fprintf(saved, "Glor: %d\n", GET_GLORY(ch));
    *buf = '\0';
    tascii(&PRF_FLAGS(ch, 0), 4, buf);
    fprintf(saved, "Pref: %s \n", buf);

    if (GET_HOUSE_UID(ch) != 0)
        fprintf(saved, "HsID: %ld\n", GET_HOUSE_UID(ch));
    if (GET_HOUSE_RANK(ch) != 0)
        fprintf(saved, "Rank: %d\n", GET_HOUSE_RANK(ch));
    if (NAME_DURATION(ch) > 0)
        fprintf(saved, "NamD: %ld\n", NAME_DURATION(ch));
    if (GODS_DURATION(ch) > 0)
        fprintf(saved, "GodD: %ld\n", GODS_DURATION(ch));
    if (MUTE_DURATION(ch) > 0)
        fprintf(saved, "MutD: %ld\n", MUTE_DURATION(ch));
    if (FREEZE_DURATION(ch) > 0)
        fprintf(saved, "FrzD: %ld\n", FREEZE_DURATION(ch));
    if (HELL_DURATION(ch) > 0)
        fprintf(saved, "HelD: %ld\n", HELL_DURATION(ch));
    if (DUMB_DURATION(ch) > 0)
        fprintf(saved, "DmbD: %ld\n", DUMB_DURATION(ch));
    fprintf(saved, "LstL: %ld\n", LAST_LOGON(ch));
    if (GET_TICKS(ch))
        fprintf(saved, "Tick: %ld\n", GET_TICKS(ch));
    fprintf(saved, "GdFl: %ld\n", ch->pc()->specials.saved.GodsLike);
    fprintf(saved, "NamG: %d\n", NAME_GOD(ch));
    fprintf(saved, "NaID: %ld\n", NAME_ID_GOD(ch));
    fprintf(saved, "WiSc: %d\n", ch->sw);
    fprintf(saved, "HiSc: %d\n", ch->sh);
    fprintf(saved, "Spek: %d\n", SPEAKING(ch));

    fprintf(saved, "RMem:\n");
    for (i = 0; i <= GET_LASTMEM(ch); i++) {
        if (GET_MEMORY(ch, i) > 0)
            fprintf(saved, "%d %ld %s\n", GET_MEMORY(ch, i), GET_TIMEMEM(ch, i),
                    GET_DESCMEM(ch, i) ? GET_DESCMEM(ch, i) : "");
        //log("%s записываю место[%d] %d",GET_NAME(ch),i,GET_MEMORY(ch,i));
    }
    fprintf(saved, "0 0\n");
    //fprintf(saved, "LMem: %d\n",GET_LASTMEM(ch));

    /* affected_type */
    fprintf(saved, "Affs:\n");
    for (aff = ch->affected; aff; aff = aff->next)
        if (aff->type) {
            fprintf(saved, "%d %d %d %d %ld %d %d %ld %s\n", aff->type, aff->duration,
                    aff->modifier, aff->location, aff->bitvector, aff->main, aff->battleflag,
                    aff->owner, SPELL_NAME(aff->type));
        }
    fprintf(saved, "0 0 0 0 0 0 0\n");

    /* Квесты */
    if (ch->Questing.quests) {
        for (i = 0; i < ch->Questing.count; i++) {
            fprintf(saved, "Qst : %d\n", *(ch->Questing.quests + i));
        }
    }
    if (ch->ScriptExp.vnum) {
        for (i = 0; i < ch->ScriptExp.count; i++)
            fprintf(saved, "SExp: %d\n", *(ch->ScriptExp.howmany + i));
    }


    if (ch->delete_level)
        fprintf(saved, "Dlev: %d\n", ch->delete_level);

    if (*ch->delete_name)
        fprintf(saved, "Dnam: %s\n", ch->delete_name);

    fprintf(saved, "Rmrt: %d\n", GET_REMORT(ch));

    save_mkill(ch, saved);


    fclose(saved);

    /*   remove(filename);
     *   XXX not really smart to comment this line out
     rename(fbackup,filename);*/

    //save_player_index();

    /* восстанавливаем аффекты */
    /* add spell and eq affections back in now */

    //log("SAVE:Одевание доспехов");
    for (i = 0; i < NUM_WEARS; i++) {
        if (char_eq[i]) {
            //if (wear_otrigger(char_eq[i], ch, i))
            equip_char(ch, char_eq[i], i | 0x80 | 0x40);
            //else
            // obj_to_char(char_eq[i], ch);
        }
    }

    //log("SAVE:Одевание тату");
    for (i = 0; i < NUM_WEARS; i++) {
        if (char_tatoo[i]) {
            //if (wear_otrigger(char_tatoo[i], ch, i))
            equip_tatoo(ch, char_tatoo[i], i | 0x80 | 0x40);
            //else
            //obj_to_char(char_tatoo[i], ch);
        }
    }

    affect_total(ch);

    if ((i = get_ptable_by_name(GET_NAME(ch))) >= 0) {
        //player_table[i].last_logon = time(0);
        player_table[i].level = GET_LEVEL(ch);
    }


}

void rename_char(struct char_data *ch, char *oname)
{
    char filename[MAX_INPUT_LENGTH], ofilename[MAX_INPUT_LENGTH];

// 1) Rename(if need) char and pkill file - directly
    log("Rename char %s->%s", GET_NAME(ch), oname);
    get_filename(oname, ofilename, PLAYERS_FILE);
    get_filename(GET_NAME(ch), filename, PLAYERS_FILE);
    rename(ofilename, filename);

    save_char(ch, GET_LOADROOM(ch));

    get_filename(oname, ofilename, PKILLERS_FILE);
    get_filename(GET_NAME(ch), filename, PKILLERS_FILE);
    rename(ofilename, filename);


// 2) Rename all other files
    get_filename(oname, ofilename, CRASH_FILE);
    get_filename(GET_NAME(ch), filename, CRASH_FILE);
    rename(ofilename, filename);

    get_filename(oname, ofilename, TEXT_CRASH_FILE);
    get_filename(GET_NAME(ch), filename, TEXT_CRASH_FILE);
    rename(ofilename, filename);

    get_filename(oname, ofilename, TIME_CRASH_FILE);
    get_filename(GET_NAME(ch), filename, TIME_CRASH_FILE);
    rename(ofilename, filename);

    get_filename(oname, ofilename, ETEXT_FILE);
    get_filename(GET_NAME(ch), filename, ETEXT_FILE);
    rename(ofilename, filename);

    get_filename(oname, ofilename, ALIAS_FILE);
    get_filename(GET_NAME(ch), filename, ALIAS_FILE);
    rename(ofilename, filename);

    get_filename(oname, ofilename, SCRIPT_VARS_FILE);
    get_filename(GET_NAME(ch), filename, SCRIPT_VARS_FILE);
    rename(ofilename, filename);

    get_filename(oname, ofilename, PMKILL_FILE);
    get_filename(GET_NAME(ch), filename, PMKILL_FILE);
    rename(ofilename, filename);
}


void clear_files(char *name)
{
    char filename[MAX_INPUT_LENGTH];

    get_filename(name, filename, PLAYERS_FILE);
    if (remove(filename) < 0)
        syserr("ОШИБКА: unlink error");

    get_filename(name, filename, ALIAS_FILE);
    if (remove(filename) < 0)
        syserr("ОШИБКА: unlink error");


    get_filename(name, filename, SCRIPT_VARS_FILE);
    if (remove(filename) < 0)
        syserr("ОШИБКА: unlink error");

    get_filename(name, filename, TEXT_CRASH_FILE);
    if (remove(filename) < 0)
        syserr("ОШИБКА: unlink error");

    get_filename(name, filename, TIME_CRASH_FILE);
    if (remove(filename) < 0)
        syserr("ОШИБКА: unlink error");

    get_filename(name, filename, PET_FILE);
    if (remove(filename) < 0)
        syserr("ОШИБКА: unlink error");

    get_filename(name, filename, QUEST_FILE);
    if (remove(filename) < 0)
        syserr("ОШИБКА: unlink error");

}

int delete_char(char *name)
{                               //char   filename[MAX_INPUT_LENGTH];//warning: unused variable 'filename'
    struct char_data *st;
    int id, retval = TRUE;

    st = new Player();
    if ((id = load_char(name, st)) >= 0) {
        // 1) Mark char as deleted
        SET_BIT(PLR_FLAGS(st, PLR_DELETED), PLR_DELETED);

        save_char(st, GET_LOADROOM(st));

        //Crash_clear_objects(id);
        player_table[id].name[0] = '\0';
        player_table[id].unique = -1;
        player_table[id].level = -1;
        player_table[id].last_logon = -1;
        player_table[id].activity = -1;
        save_player_index();

        st->linkWrapper();
        st->extractWrapper(true);

    } else {
        retval = FALSE;
    }

    delete st;

    // 2) Remove all other files
    //Crash_delete_files(id);

    clear_files(name);

    return (retval);
}

void load_skills(void)
{
    char line1[256], line2[256], name[256];
    int i[10], c, sp_num, nclass, templ, koef_i, level, ablt;
    DLFileRead magic(ShareFile(mud->skillsFile));

    if (!magic.open())
        _exit(1);

    while (get_line(magic.getFP(), name)) {
        if (!name[0] || name[0] == ';')
            continue;

        if (!mud->modQuietLoad)
            log("%s", name);

        if (sscanf(name, "%s %s %d %d %d %d %d", line1, line2, i, i + 1, i + 2, i + 3, i + 4) != 7) {
            log("Bad format for skill string!\r\n"
                "Format: <skill name (%%s %%s)>  <class (%%d)> <template (%%d)> <improove (%%d)> <level (%%d)> <abblity (%%d)>! after %s",
                name);
            _exit(1);
        }
        name[0] = '\0';
        strcat(name, line1);
        if (*line2 != '*') {
            *(name + strlen(name) + 1) = '\0';
            *(name + strlen(name) + 0) = ' ';
            strcat(name, line2);
        }
        if ((sp_num = find_skill_num(name)) < 0) {
            log("Skill '%s' not found...", name);
            _exit(1);
        }
        if (i[0] < 0 || i[0] >= NUM_CLASSES) {
            log("Некорректное значение класса для умения \"%s\"...",
                skill_info[sp_num].name.c_str());
            _exit(1);
        }
        if (i[1] < 0 || i[1] > NUM_GODS) {
            log("Некорректное значение религии для умения \"%s\"...",
                skill_info[sp_num].name.c_str());
            _exit(1);
        }
        // i[0] class | i[1] template | i[2] k_improove | i[3] level | i[4] - %

        nclass = i[0];
        templ = i[1];
        koef_i = i[2];
        level = i[3];
        ablt = i[4];

        if (templ == 0)
            for (c = 0; c < NUM_GODS; c++) {
                skill_info[sp_num].learn_level[nclass][c] = level;
                skill_info[sp_num].k_improove[nclass][c] = koef_i;
                skill_info[sp_num].ability[nclass][c] = ablt;
        } else {
            skill_info[sp_num].learn_level[nclass][templ] = level;
            skill_info[sp_num].k_improove[nclass][templ] = koef_i;
            skill_info[sp_num].ability[nclass][templ] = ablt;
        }

    }                           //конец while

    /*  log("ПОСЛЕ ЗАГРУЗКИ");
       for (c=1; c <=TOP_SKILL_DEFINE;c++)
       for(class=0; class < NUM_CLASSES;class++)

       log("%-25s | class %-8s | level %d",
       skill_name(c),class_name[class],
       skill_info[c].learn_level[class][0]); */


}


zone_rnum create_new_zone(zone_vnum vzone_num)
{
    struct zone_data *zone;
    zone_rnum rznum;
    int i;

    for (i = 0; i < top_of_zone_table; i++)
        if (zone_table[i].number == vzone_num) {
            log("That virtual zone already exists.");
            return NOWHERE;
        }

    RECREATE(zone_table, struct zone_data, top_of_zone_table + 2);
    zone_table[top_of_zone_table + 1].number = 32000;

    if (vzone_num > zone_table[top_of_zone_table].number)
        rznum = top_of_zone_table + 1;
    else {
        for (i = top_of_zone_table + 1; i > 0 && vzone_num < zone_table[i - 1].number; i--)
            zone_table[i] = zone_table[i - 1];
        rznum = i;
    }
    zone = &zone_table[rznum];

    /*
     * Ok, insert the new zone here.
     */
    zone->name = strdup("New Zone");
    zone->number = vzone_num;
    zone->top = (vzone_num * 100) + 99;
    zone->lifespan = 30;
    zone->age = 0;
    zone->reset_mode = 2;

    top_of_zone_table++;

    return rznum;
}


void delete_players(void)
{
    struct descriptor_data *d;
    int i, level;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];

    for (d = descriptor_list; d; d = d->next) {
        if ((STATE(d) == CON_PLAYING) && !IS_NPC(d->character)) {       //Crash_crashsave(d->character);
            //xsave_rent(d->character,RENT_NORMAL,FALSE);
            //save_char(d->character, NOWHERE);
            REMOVE_BIT(PLR_FLAGS(d->character, PLR_CRASH), PLR_CRASH);
        }
    }

    for (i = 0; i <= top_of_p_table; i++)
        if (player_table[i].level >= 0 &&
            player_table[i].level <= 20 &&
            player_table[i].last_logon > 0 &&
            player_table[i].last_logon + ((604800 * 2) * MAX(1, player_table[i].level)) < time(0)) {
            level = player_table[i].level;
            strcpy(buf2, player_table[i].name);
            if (delete_char(player_table[i].name)) {
                sprintf(buf, "%s (уровень %d) удален системой", buf2, level);
                mudlog(buf, NRM, LVL_GOD, TRUE);
            }
        }
}


void make_maze(int zone)
{
    int card[400], temp, x, y, dir, room;
    int num, next_room = 0, test, r_back;
    int vnum = zone_table[zone].number;

    for (test = 0; test < 400; test++) {
        card[test] = test;
        temp = test;
        dir = temp / 100;
        temp = temp - (dir * 100);
        x = temp / 10;
        temp = temp - (x * 10);
        y = temp;
        room = (vnum * 100) + (x * 10) + y;
        room = real_room(room);
        if ((x == 0) && (dir == 0))
            continue;
        if ((y == 9) && (dir == 1))
            continue;
        if ((x == 9) && (dir == 2))
            continue;
        if ((y == 0) && (dir == 3))
            continue;
        world[room].dir_option[dir]->to_room = -1;
        REMOVE_BIT(ROOM_FLAGS(room, ROOM_NOTRACK), ROOM_NOTRACK);
    }
    for (x = 0; x < 399; x++) {
        y = number(0, 399);
        temp = card[y];
        card[y] = card[x];
        card[x] = temp;
    }

    for (num = 0; num < 400; num++) {
        temp = card[num];
        dir = temp / 100;
        temp = temp - (dir * 100);
        x = temp / 10;
        temp = temp - (x * 10);
        y = temp;
        room = (vnum * 100) + (x * 10) + y;
        r_back = room;
        room = real_room(room);
        if ((x == 0) && (dir == 0))
            continue;
        if ((y == 9) && (dir == 1))
            continue;
        if ((x == 9) && (dir == 2))
            continue;
        if ((y == 0) && (dir == 3))
            continue;
        if (world[room].dir_option[dir]->to_room != -1)
            continue;
        switch (dir) {
            case 0:
                next_room = r_back - 10;
                break;
            case 1:
                next_room = r_back + 1;
                break;
            case 2:
                next_room = r_back + 10;
                break;
            case 3:
                next_room = r_back - 1;
                break;
        }
        next_room = real_room(next_room);
        test = find_first_step(room, next_room, NULL);
        switch (test) {
            case BFS_ERROR:
                log("Maze making error.");
                break;
            case BFS_ALREADY_THERE:
                log("Maze making error.");
                break;
            case BFS_NO_PATH:

                world[room].dir_option[dir]->to_room = next_room;
                world[next_room].dir_option[(int) rev_dir[dir]]->to_room = room;
                break;
        }
    }
    for (num = 0; num < 100; num++) {
        room = (vnum * 100) + num;
        room = real_room(room);
        /* Remove the next line if you want to be able to track your way through the maze */
        SET_BIT(ROOM_FLAGS(room, ROOM_NOTRACK), ROOM_NOTRACK);
        REMOVE_BIT(ROOM_FLAGS(room, ROOM_BFS_MARK), ROOM_BFS_MARK);
    }
}

void set_door_code(char *pcode, int nums)
{
    int i;

    nums = MAX(1, nums);

    for (i = 0; i < 30; i++)
        pcode[i] = '\0';

    for (i = 0; i < nums; i++)
        pcode[i] = number(0, 1) ? '<' : '>';
    pcode[i] = 0;
}

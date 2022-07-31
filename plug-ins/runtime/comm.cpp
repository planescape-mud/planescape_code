/* ************************************************************************
*   File: comm.c                                        Part of CircleMUD *
*  Usage: Communication, socket handling, main(), central game loop       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
 * Compression support.  Currently could be used with:
 *
 *   MUD Client for Linux, by Erwin S. Andreasen
 *     http://www.andreasen.org/mcl/
 *
 *   mcclient, by Oliver 'Nemon@AR' Jowett
 *     http://homepages.ihug.co.nz/~icecube/compress/
 *
 * Contact them for help with the clients. Contact greerga@circlemud.org
 * for problems with the server end of the connection.  If you think you
 * have found a bug, please test another MUD for the same problem to see
 * if it is a client or server problem.
 */

#define HAVE_ICONV // prool: for UTF-8 codetable

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

#include "sysdep.h"
#include "profiler.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "ban.h"
#include "interpreter.h"
#include "handler.h"
#include "db.h"
#include "xboot.h"
#include "shop.h"
#include "ai.h"
#include "house.h"
#include "events.h"
#include "screen.h"
#include "constants.h"
#include "spells.h"
#include "planescape.h"
#include "mudpluginmanager.h"
#include "mudscheduler.h"
#include "process.h"
#include "register-impl.h"
#include "object.h"
#include "mudstats.h"
#include "textfileloader.h"
#include "mudfile.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

/* local vars */
struct timeval null_time;       /* zero-valued time structure */

/* functions in this file */
void go_copyover(void);

ssize_t perform_socket_read(socket_t desc, char *read_point, size_t space_left);
ssize_t perform_socket_write(socket_t desc, const char *txt, size_t length);
void sanity_check(void);

void flush_queues(struct descriptor_data *d);
void nonblock(socket_t s);
int perform_subst(struct descriptor_data *t, char *orig, char *subst);
void record_usage(void);
void check_idle_passwords(void);
void init_descriptor(struct descriptor_data *newd, int desc, int key);
struct in_addr *get_bind_addr(void);
int parse_ip(const char *addr, struct in_addr *inaddr);
int set_sendbuf(socket_t s);

#if defined(HAVE_ZLIB)
void *zlib_alloc(void *opaque, unsigned int items, unsigned int size);
void zlib_free(void *opaque, void *address);
#endif
int mccp_start(struct descriptor_data *t, int ver);
int mccp_end(struct descriptor_data *t, int ver);

void koi_to_utf8(char *str_i, char *str_o);
void utf8_to_koi(char *str_i, char *str_o);

#ifdef __CXREF__
#undef FD_ZERO
#undef FD_SET
#undef FD_ISSET
#undef FD_CLR
#define FD_ZERO(x)
#define FD_SET(x, y) 0
#define FD_ISSET(x, y) 0
#define FD_CLR(x, y)
#endif


#if defined(HAVE_ZLIB)
/*
 * MUD Client for Linux and mcclient compression support.
 * "The COMPRESS option (unofficial and completely arbitary) is
 * option 85." -- mcclient documentation as of Dec '98.
 *
 * [ Compression protocol documentation below, from Compress.c ]
 *
 * Server sends  IAC WILL COMPRESS
 * We reply with IAC DO COMPRESS
 *
 * Later the server sends IAC SB COMPRESS WILL SE, and immediately following
 * that, begins compressing
 *
 * Compression ends on a Z_STREAM_END, no other marker is used
 *
 * 2001/02/08 - Version 2 support. - mike
 * Version one uses an improper subnegotiation sequence to indicate start of compression
 * This represents the changes in v1 and v2:
 *  It uses the equally arbitrary COMPRESS2 option 86,
 *  It properly terminates the subnegotiation sequence.
 */
#define TELOPT_COMPRESS        85
#define TELOPT_COMPRESS2       86
const char compress_will[] = { IAC, WILL, TELOPT_COMPRESS2,
    IAC, WILL, TELOPT_COMPRESS, '\0'
};
const char compress_start_v1[] = { IAC, SB, TELOPT_COMPRESS, WILL, SE, '\0' };
const char compress_start_v2[] = { IAC, SB, TELOPT_COMPRESS2, IAC, SE, '\0' };
#endif


/***********************************************************************
*  main game loop and related stuff                                    *
***********************************************************************/

#if defined(CIRCLE_WINDOWS) || defined(CIRCLE_MACINTOSH)

/*
 * Windows doesn't have gettimeofday, so we'll simulate it.
 * The Mac doesn't have gettimeofday either.
 * Borland C++ warns: "Undefined structure 'timezone'"
 */


void gettimeofday(struct timeval *t, struct timezone *dummy)
{
#if defined(CIRCLE_WINDOWS)
    DWORD millisec = GetTickCount();
#elif defined(CIRCLE_MACINTOSH)
    unsigned long int millisec;

    millisec = (int) ((float) TickCount() * 1000.0 / 60.0);
#endif

    t->tv_sec = (int) (millisec / 1000);
    t->tv_usec = (millisec % 1000) * 1000;
}

#endif                          /* CIRCLE_WINDOWS || CIRCLE_MACINTOSH */


#define plant_magic(x)  do { (x)[sizeof(x) - 1] = MAGIC_NUMBER; } while (0)
#define test_magic(x)   ((x)[sizeof(x) - 1])

/*
 * Binary option (specific to telnet client)
 */
void set_binary(struct descriptor_data *d)
{
    char binary_string[] = {
        (char) IAC,
        (char) DO,
        (char) TELOPT_BINARY,
        (char) 0,
    };
    write(d->descriptor, binary_string, 3);
}

void end_game()
{
    xrent_save_all(RENT_REBOOT);
    save_player_index();

    mud->getStats()->save();

    if (circle_copyover) {
        save_shops();
        go_copyover();
    }

    log("Closing all sockets.");
    while (descriptor_list)
        close_socket(descriptor_list, TRUE);

    CLOSE_SOCKET(mother_desc);

    mud->getStats()->setRebootLevel(2);
    mud->getStats()->save();

    //Сохраняем магазины
    save_shops();
    if (circle_reboot != 2) {   /* Don't save zones. */
        //Записываем локации на диск
        /* TODO */
    }

    if (circle_reboot) {
        log("Перезагрузка.");
    } else
        log("Игра нормально завершила работу.");
}



/*
 * init_socket sets up the mother descriptor - creates the socket, sets
 * its options up, binds it, and listens.
 */
socket_t init_socket(ush_int port)
{
    socket_t s;
    int opt;
    struct sockaddr_in sa;

    log("Открываем базовое соединение.");

#ifdef CIRCLE_WINDOWS
    {
        WORD wVersionRequested;
        WSADATA wsaData;

        wVersionRequested = MAKEWORD(1, 1);

        if (WSAStartup(wVersionRequested, &wsaData) != 0) {
            log("SYSERR: WinSock not available!");
            exit(1);
        }
        if ((wsaData.iMaxSockets - 4) < max_players) {
            max_players = wsaData.iMaxSockets - 4;
        }
        log("Max players set to %d", max_players);

        if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            log("SYSERR: Error opening network connection: Winsock error #%d", WSAGetLastError());
            exit(1);
        }
    }
#else
    /*
     * Should the first argument to socket() be AF_INET or PF_INET?  I don't
     * know, take your pick.  PF_INET seems to be more widely adopted, and
     * Comer (_Internetworking with TCP/IP_) even makes a point to say that
     * people erroneously use AF_INET with socket() when they should be using
     * PF_INET.  However, the man pages of some systems indicate that AF_INET
     * is correct; some such as ConvexOS even say that you can use either one.
     * All implementations I've seen define AF_INET and PF_INET to be the same
     * number anyway, so the point is (hopefully) moot.
     */

    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        syserr("SYSERR: Error creating socket");
        exit(1);
    }
#endif                          /* CIRCLE_WINDOWS */

#if defined(SO_REUSEADDR) &&!defined(CIRCLE_MACINTOSH)
    opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0) {
        syserr("SYSERR: setsockopt REUSEADDR");
        exit(1);
    }
#endif

    set_sendbuf(s);

    /*
     * The GUSI sockets library is derived from BSD, so it defines
     * SO_LINGER, even though setsockopt() is unimplimented.
     *      (from Dean Takemori <dean@UHHEPH.PHYS.HAWAII.EDU>)
     */
#if defined(SO_LINGER) &&!defined(CIRCLE_MACINTOSH)
    {
        struct linger ld;

        ld.l_onoff = 0;
        ld.l_linger = 0;
        if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0)
            syserr("SYSERR: setsockopt SO_LINGER");     /* Not fatal I suppose. */
    }
#endif

    /* Clear the structure */
    memset((char *) &sa, 0, sizeof(sa));

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr = *(get_bind_addr());

    if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
        syserr("SYSERR: bind");
        CLOSE_SOCKET(s);
        exit(1);
    }
    nonblock(s);
    listen(s, 5);
    return (s);
}


int get_max_players(void)
{
#ifndef CIRCLE_UNIX
    return (max_playing);
#else

    int max_descs = 0;
    const char *method;

    /*
     * First, we'll try using getrlimit/setrlimit.  This will probably work
     * on most systems.  HAS_RLIMIT is defined in sysdep.h.
     */
#ifdef HAS_RLIMIT
    {
        struct rlimit limit;

        /* find the limit of file descs */
        method = "rlimit";
        if (getrlimit(RLIMIT_NOFILE, &limit) < 0) {
            syserr("SYSERR: calling getrlimit");
            exit(1);
        }

        /* set the current to the maximum */
        limit.rlim_cur = limit.rlim_max;
        if (setrlimit(RLIMIT_NOFILE, &limit) < 0) {
            syserr("SYSERR: calling setrlimit");
            exit(1);
        }
#ifdef RLIM_INFINITY
        if (limit.rlim_max == RLIM_INFINITY)
            max_descs = max_playing + NUM_RESERVED_DESCS;
        else
            max_descs =
                MIN((unsigned int) max_playing + (unsigned int) NUM_RESERVED_DESCS,
                    (unsigned int) limit.rlim_max);
#else
        max_descs = MIN(max_playing + NUM_RESERVED_DESCS, limit.rlim_max);
#endif
    }

#elif defined (OPEN_MAX) || defined(FOPEN_MAX)
#if!defined(OPEN_MAX)
#define OPEN_MAX FOPEN_MAX
#endif
    method = "OPEN_MAX";
    max_descs = OPEN_MAX;       /* Uh oh.. rlimit didn't work, but we have
                                 * OPEN_MAX */
#elif defined (_SC_OPEN_MAX)
    /*
     * Okay, you don't have getrlimit() and you don't have OPEN_MAX.  Time to
     * try the POSIX sysconf() function.  (See Stevens' _Advanced Programming
     * in the UNIX Environment_).
     */
    method = "POSIX sysconf";
    errno = 0;
    if ((max_descs = sysconf(_SC_OPEN_MAX)) < 0) {
        if (errno == 0)
            max_descs = max_playing + NUM_RESERVED_DESCS;
        else {
            syserr("SYSERR: Error calling sysconf");
            exit(1);
        }
    }
#else
    /* if everything has failed, we'll just take a guess */
    method = "random guess";
    max_descs = max_playing + NUM_RESERVED_DESCS;
#endif

    /* now calculate max _players_ based on max descs */
    max_descs = MIN(max_playing, max_descs - NUM_RESERVED_DESCS);

    if (max_descs <= 0) {
        log("SYSERR: Non-positive max player limit!  (Set at %d using %s).", max_descs, method);
        exit(1);
    }
    log("Устанавливаю лимит игроков %d используя %s.", max_descs, method);
    return (max_descs);
#endif                          /* CIRCLE_UNIX */
}



void beat_points_update(int pulse);
void pulse_update(int pulse);

#define FRAC_SAVE TRUE

void heartbeat(int pulse)
{
    struct char_data *ch = NULL, *k, *next;
    int check_log = FALSE;

    if (check_log)
        log("---------- Start heartbeat ----------");
    if (check_log)
        log("Process events...");
    pulse_events(pulse);
    if (check_log)
        log("Stop it...");

    if (!(pulse % PASSES_PER_SEC)) {
        pulse_saved_var(pulse);
        global_vars.pulse(pulse);
    }

    if (!(pulse % (30 * PASSES_PER_SEC))) {
        if (check_log)
            log("Sanity check...");
        sanity_check();
        if (check_log)
            log("Stop it...");
    }

    /*** Remove after hour update
    if (!(pulse % PULSE_ZONE))
       {if (check_log) log("Zone update...");
        zone_update();
       }
     ****************************/

    if (!(pulse % (40 * PASSES_PER_SEC))) {     /* 40 seconds */
        if (check_log)
            log("Check idle password...");
        check_idle_passwords();
        if (check_log)
            log("Stop it...");
    }
    /* if (!(pulse % PASSES_PER_SEC))
       {
       if (check_log) log("Mobile check agressive...");
       mobile_agress(1);
       if (check_log) log("Stop it...");
       } */


    if (!(pulse % PULSE_VIOLENCE)) {
        if (check_log)
            log("Perform violence...");
        bt_affect_update();
        perform_violence();
        perform_violence_magic();
        if (check_log)
            log("Stop it...");
        if (check_log)
            log("End battle round...");
        end_battle_round();
        if (check_log)
            log("Stop it...");
    }

    if (!(pulse % PULSE_WEATHER)) {
        if (check_log)
            log("Weather and time...");
        weather_and_time(1);
        if (check_log)
            log("Stop it...");
    }

    if (!(pulse % PULSE_ZONE)) {
        if (check_log)
            log("Zone update...");
        zone_update();
        if (check_log)
            log("Stop it...");
    }

    if (!(pulse % SECS_PER_MUD_HOUR)) {
        if (check_log)
            log("Shop update...");
        regain_shops();
        if (check_log)
            log("Stop it...");
    }


    if (!(pulse % (SECS_PER_MUD_TICK * PASSES_PER_SEC))) {
        if (check_log)
            log("Point update...");
        point_update();
        if (check_log)
            log("Stop it...");
        if (check_log)
            log("Paste mobiles...");
        paste_mobiles(-1);
        if (check_log)
            log("Stop it...");
    }

    if (!(pulse % (SECS_PER_MUD_HOUR * PASSES_PER_SEC))) {
        if (check_log)
            log("Hour msg update...");
        hour_update();
        if (check_log)
            log("Stop it...");
    }


    if (!(pulse % ((SECS_PER_MUD_TICK / 5) * PASSES_PER_SEC))) {
        if (check_log)
            log("Обновление болезней");
        illness_update();
        if (check_log)
            log("Обновление болезней...");
    }

    if (!(pulse % PASSES_PER_SEC)) {
        if (check_log)
            log("Beat points update...");
        beat_points_update(pulse / PASSES_PER_SEC);
        if (check_log)
            log("Stop it...");
        if (check_log)
            log("Character affect update...");
        player_affect_update();
        if (check_log)
            log("Stop it...");
    }

    if (check_log)
        log("Mobile activity...");
    mobile_activity(pulse);
    if (check_log)
        log("Stop it...");

    if (check_log)
        log("Pulse update...");
    pulse_update(pulse);
    if (check_log)
        log("Stop it...");

    if (auto_save && !(pulse % (PASSES_PER_SEC * SECS_PER_MUD_TICK * 60))) {
        if (++mins_since_crashsave >= autosave_time) {
            delete_players();
            save_player_index();
            xrent_check((pulse / PASSES_PER_SEC) % OBJECT_SAVE_ACTIVITY);
            mins_since_crashsave = 0;
        }
    }

    if (!(pulse % (5 * 60 * PASSES_PER_SEC))) { /* 5 minutes */
        if (check_log)
            log("Record usage...");
        record_usage();
        if (check_log)
            log("Stop it...");
    }
    if (check_log)
        log("Real extract char");
    for (k = character_list; k; k = next) {
        next = k->next;
        if (IN_ROOM(k) == NOWHERE || MOB_FLAGGED(k, MOB_DELETE)) {
            if (check_log)
                log("[Heartbeat] Remove from list char %s", GET_NAME(k));
            if (k == character_list)
                character_list = next;
            else
                ch->next = next;
            k->next = NULL;
            if (MOB_FLAGGED(k, MOB_FREE)) {
                if (check_log)
                    log("[Heartbeat] Free char %s", GET_NAME(k));
                delete k;

                if (check_log)
                    log("[Heartbeat] OK free char");
            }
        } else
            ch = k;
    }

    if (check_log)
        log("---------- Stop heartbeat ----------");
}


/* ******************************************************************
*  general utility stuff (for local use)                            *
****************************************************************** */

void record_usage(void)
{
    int sockets_connected = 0, sockets_playing = 0;
    struct descriptor_data *d;

    for (d = descriptor_list; d; d = d->next) {
        sockets_connected++;
        if (STATE(d) == CON_PLAYING)
            sockets_playing++;
    }

    log("nusage: %-3d sockets connected, %-3d sockets playing", sockets_connected, sockets_playing);

#ifdef RUSAGE                   /* Not RUSAGE_SELF because it doesn't guarantee prototype. */
    {
        struct rusage ru;

        getrusage(RUSAGE_SELF, &ru);
        log("rusage: user time: %ld sec, system time: %ld sec, max res size: %ld",
            ru.ru_utime.tv_sec, ru.ru_stime.tv_sec, ru.ru_maxrss);
    }
#endif

}


/*
 * Turn off echoing (specific to telnet client)
 */
void echo_off(struct descriptor_data *d)
{
    const char off_string[] = {
        (char) IAC,
        (char) WILL,
        (char) TELOPT_ECHO,
        (char) 0,
    };

    //SEND_TO_SOCKET(off_string, d->descriptor);
    SEND_TO_Q(off_string, d);
}

/*
 * Turn on echoing (specific to telnet client)
 */
void echo_on(struct descriptor_data *d)
{
    const char on_string[] = {
        (char) IAC,
        (char) WONT,
        (char) TELOPT_ECHO,
        (char) 0,
    };

    //SEND_TO_SOCKET(on_string, d->descriptor);
    SEND_TO_Q(on_string, d);
}

int posi_value(int real, int max)
{
    //if (real < -6)
    if (real < -(max / 9))
        return (-2);
    else if (real <= 0)
        return (-1);
    else if (real >= max)
        return (10);

    return (real * 10 / MAX(max, 1));
}

int posi_value5(int real, int max)
{
    if (real < 0)
        return (0);
    else if (real >= max)
        return (5);

    return (real * 5 / MAX(max, 1));
}

char *color_value(struct char_data *ch, int real, int max)
{
    static char color[256];

    switch (posi_value(real, max)) {
        case -1:
        case -2:
            sprintf(color, "%s", CCIBLK(ch, C_NRM));
            break;
        case 0:
            sprintf(color, "%s", CCRED(ch, C_NRM));
            break;
        case 2:
        case 1:
            sprintf(color, "%s", CCIRED(ch, C_NRM));
            break;
        case 3:
        case 4:
        case 5:
            sprintf(color, "%s", CCIYEL(ch, C_NRM));
            break;
        case 6:
        case 7:
        case 8:
            sprintf(color, "%s", CCIGRN(ch, C_NRM));
            break;
        case 9:
            sprintf(color, "%s", CCGRN(ch, C_NRM));
            break;
        default:
            sprintf(color, "%s", CCWHT(ch, C_NRM));
            break;
    }

    return (color);
}

char *color_value_magic(struct char_data *ch, int real, int max)
{
    static char color[256];

    switch (posi_value(real, max)) {
        case -1:
        case -2:
            sprintf(color, "%s", CCIBLK(ch, C_NRM));
            break;
        case 0:
            sprintf(color, "%s", CCBLU(ch, C_NRM));
            break;
        case 2:
        case 1:
            sprintf(color, "%s", CCCYN(ch, C_NRM));
            break;
        case 3:
        case 4:
        case 5:
            sprintf(color, "%s", CCIBLU(ch, C_NRM));
            break;
        case 6:
        case 7:
        case 8:
            sprintf(color, "%s", CCIBLU(ch, C_NRM));
            break;
        case 9:
            sprintf(color, "%s", CCICYN(ch, C_NRM));
            break;
        default:
            sprintf(color, "%s", CCWHT(ch, C_NRM));
            break;
    }
    return (color);
}

char *color_pos(struct char_data *ch, int pos)
{
    static char color[256];

    /* switch (pos)
       {case 0: case 1:
       sprintf(color,"%s",CCIBLK(ch,C_NRM)); break;
       case 2:
       sprintf(color,"%s",CCRED(ch,C_NRM)); break;
       case 3: case 4:
       sprintf(color,"%s",CCIRED(ch,C_NRM)); break;
       case 5: case 6: case 7:
       sprintf(color,"%s",CCIYEL(ch,C_NRM)); break;
       case 8: case 9: case 10:
       sprintf(color,"%s",CCIGRN(ch,C_NRM)); break;
       case 11:
       sprintf(color,"%s",CCGRN(ch,C_NRM)); break;
       default:
       sprintf(color,"%s",CCWHT(ch,C_NRM)); break;
       } */

    switch (pos) {
        case 0:
        case 1:
            sprintf(color, "%s", CCIBLK(ch, C_NRM));
            break;
        case 2:
            sprintf(color, "%s", CCRED(ch, C_NRM));
            break;
        case 3:
        case 4:
            sprintf(color, "%s", CCRED(ch, C_NRM));
            break;
        case 5:
        case 6:
        case 7:
            sprintf(color, "%s", CCIYEL(ch, C_NRM));
            break;
        case 8:
        case 9:
        case 10:
            sprintf(color, "%s", CCGRN(ch, C_NRM));
            break;
        case 11:
            sprintf(color, "%s", CCGRN(ch, C_NRM));
            break;
        default:
            sprintf(color, "%s", CCWHT(ch, C_NRM));
            break;
    }
    return (color);
}


char *show_state(struct char_data *ch, struct char_data *victim)
{
    int pos;
    char name[MAX_STRING_LENGTH];
    static char buf[MAX_STRING_LENGTH];

    static const char *WORD_STATE[13] = { "при смерти", //0
        "без сознания",         //1
        "ужасное",              //2
        "очень плохое",         //3
        "плохое",               //4
        "плохое",               //5
        "среднее",              //6
        "среднее",              //7
        "хорошее",              //8
        "хорошее",              //9
        "очень хорошее",        //10
        "очень хорошее",        //11
        "великолепное"
    };                          //10

    if (ch == victim)
        sprintf(name, "Вы");
    else
        sprintf(name, "%s", PERS(victim, ch, 0));
    //name[0] = UPPER(name[0]);

    pos = posi_value(GET_HIT(victim), GET_REAL_MAX_HIT(victim)) + 2;

    sprintf(buf, "[%s:%s%s%s] ",
            name,
            color_pos(ch, pos),
            PRF_FLAGGED(ch, PRF_PROMPT) ? WORD_STATE[pos] :
            meter_bar(ch, GET_HIT(victim), GET_REAL_MAX_HIT(victim), pos), CCNRM(ch, C_NRM));

    return buf;
}


char *make_prompt(struct descriptor_data *d)
{
    static char prompt[MAX_PROMPT_LENGTH + 1];
    static const char *dirs[] = { "С", "В", "Ю", "З", "П", "О" };
    struct char_data *vict;

    int next_level;
    int door;
    int perc;

    /* Note, prompt is truncated at MAX_PROMPT_LENGTH chars (structs.h ) */
    if (d->showstr_count)
        sprintf(prompt,
                "\r\n&K[enter] дальше, [к]онец, или номер страницы (%d/%d).&n",
                d->showstr_page, d->showstr_count);
    else if (d->str)
        strcpy(prompt, "] ");
    else if (STATE(d) == CON_PLAYING) {
        int count = 0;

        *prompt = '\0';

        // Invisibitity
        if (IS_NPC(d->character) && !MOB_FLAGGED(d->character, MOB_CLONE))
            count += sprintf(prompt + count, "%s: ", GET_NAME(d->character));
        else if (GET_INVIS_LEV(d->character))
            count += sprintf(prompt + count, "н%d ", GET_INVIS_LEV(d->character));

        // Hits state
        if (PRF_FLAGGED(d->character, PRF_DISPHP)) {
            perc = (100 * GET_HIT(d->character)) / GET_REAL_MAX_HIT(d->character);
            count +=
                sprintf(prompt + count, "%s%dж%s ",
                        color_value(d->character, GET_HIT(d->character),
                                    GET_REAL_MAX_HIT(d->character)), GET_HIT(d->character),
                        CCNRM(d->character, C_NRM));
        }
        // Moves state
        if (PRF_FLAGGED(d->character, PRF_DISPMOVE)) {
            perc = (100 * GET_MOVE(d->character)) / GET_REAL_MAX_MOVE(d->character);
            count +=
                sprintf(prompt + count, "%s%dб%s ",
                        color_value(d->character, GET_MOVE(d->character),
                                    GET_REAL_MAX_MOVE(d->character)), GET_MOVE(d->character),
                        CCNRM(d->character, C_NRM));
        }
        // Mana state
        if (PRF_FLAGGED(d->character, PRF_DISPMANA)) {
            count +=
                sprintf(prompt + count, "%s%dм%s ",
                        color_value_magic(d->character, GET_MANA(d->character),
                                          GET_REAL_MAX_MANA(d->character)), GET_MANA(d->character),
                        CCNRM(d->character, C_NRM));
        }
        // Expirience
        // if (PRF_FLAGGED(d->character, PRF_DISPEXP))
        //    count += sprintf(prompt + count, "%ldx ", GET_EXP(d->character));
        if (PRF_FLAGGED(d->character, PRF_DISPEXP)) {
            if (IS_IMMORTAL(d->character))
                count += sprintf(prompt + count, "??? ");
            else {
                next_level = get_dsu_exp(d->character);

                if (next_level < 0)     //&& !IS_MAX_EXP(d->character))
                    next_level = 0;

                if (PRF_FLAGGED(d->character, PRF_SHOWMEGA))
                    count += sprintf(prompt + count, "%dМо ", next_level / 100000);
                else if (PRF_FLAGGED(d->character, PRF_SHOWKILL))
                    count += sprintf(prompt + count, "%dКо ", next_level / 1000);
                else
                    count += sprintf(prompt + count, "%dо ", next_level);
            }
        }

        /*
           if (PRF_FLAGGED(d->character, PRF_DISPGOLD))
           { if (PRF_FLAGGED(d->character, PRF_SHOWMEGA))
           count += sprintf(prompt + count, "%dМм ", GET_GOLD(d->character)/100000);
           else if (PRF_FLAGGED(d->character, PRF_SHOWKILL))
           count += sprintf(prompt + count, "%dКм ", GET_GOLD(d->character)/1000);
           else
           count += sprintf(prompt + count, "%dм ", GET_GOLD(d->character));
           }
         */

        if (PRF_FLAGGED(d->character, PRF_DISPLEVEL))
            count += sprintf(prompt + count, "%dл ", GET_LEVEL(d->character));

        //count += sprintf(prompt + count, "%s",event_status_bar(d->character));

        //Выводим обычный бой.
        //if (PRF_FLAGGED(d->character, PRF_DISPBOI))
        {
            if (FIGHTING(d->character) && IN_ROOM(d->character) == IN_ROOM(FIGHTING(d->character))) {
                vict = FIGHTING(d->character);
                if (GET_POS(vict) > POS_STUNNED && PRF_FLAGGED(d->character, PRF_DISPBOI))
                    count += sprintf(prompt + count, "%s", show_state(d->character, d->character));

                if ((FIGHTING(FIGHTING(d->character))
                     && FIGHTING(FIGHTING(d->character)) != d->character)
                    && ((posi_value(GET_HIT(vict), GET_REAL_MAX_HIT(vict)) + 1) >= 2))
                    count +=
                        sprintf(prompt + count, "%s",
                                show_state(d->character, FIGHTING(FIGHTING(d->character))));

                if (FIGHTING(d->character))
                    count +=
                        sprintf(prompt + count, "%s",
                                show_state(d->character, FIGHTING(d->character)));
            }
        }
        //Выходы
        if (PRF_FLAGGED(d->character, PRF_DISPEXITS) && IN_ROOM(d->character) != NOWHERE) {
            count += sprintf(prompt + count, "Выходы:");
            if (!AFF_FLAGGED(d->character, AFF_BLIND)
                && (GET_POS(d->character) > POS_SLEEPING))
                for (door = 0; door < NUM_OF_DIRS; door++) {
                    if (EXIT(d->character, door) &&
                        EXIT(d->character, door)->to_room != NOWHERE &&
                        !DOOR_FLAGGED(EXIT(d->character, door), EXIT_HIDDEN)) {
                        if (DOOR_FLAGGED(EXIT(d->character, door), EXIT_CLOSED))
                            count += sprintf(prompt + count, "(%s)", dirs[door]);
                        else
                            count += sprintf(prompt + count, "%s", dirs[door]);
                    } else {
                        if (EXIT(d->character, door) && EXIT(d->character, door)->timer)
                            count += sprintf(prompt + count, "%s", dirs[door]);
                    }
                }
        }

        char *status = event_status_bar(d->character);

        if (*status)
            sprintf(prompt + count, " %s", status);

        if (GET_WAIT(d->character))
            strcat(prompt, "&K>&n ");
        else
            strcat(prompt, "> ");
    } else
        *prompt = '\0';

    /* Выводим промпт снуперу */
    if (d->snoop_by_col) {
        int i_count;

        for (i_count = 0; i_count < d->snoop_by_col; i_count++) {
            SEND_TO_Q("\r\n", d->snoop_by[i_count]);
            SEND_TO_Q(prompt, d->snoop_by[i_count]);
            SEND_TO_Q("%", d->snoop_by[i_count]);
        }
    }

    return (prompt);
}


void write_to_q(const char *txt, struct txt_q *queue, int aliased)
{
    struct txt_block *newt;

    CREATE(newt, struct txt_block, 1);

    newt->text = str_dup(txt);
    newt->aliased = aliased;

    /* queue empty? */
    if (!queue->head) {
        newt->next = NULL;
        queue->head = queue->tail = newt;
    } else {
        queue->tail->next = newt;
        queue->tail = newt;
        newt->next = NULL;
    }
}



int get_from_q(struct txt_q *queue, char *dest, int *aliased)
{
    struct txt_block *tmp;

    /* queue empty? */
    if (!queue->head)
        return (0);

    tmp = queue->head;
    strcpy(dest, queue->head->text);
    *aliased = queue->head->aliased;
    queue->head = queue->head->next;

    free(tmp->text);
    free(tmp);

    return (1);
}



/* Empty the queues before closing connection */
void flush_queues(struct descriptor_data *d)
{
    int dummy;
    char buf2[MAX_STRING_LENGTH];

    if (d->large_outbuf) {
        d->large_outbuf->next = bufpool;
        bufpool = d->large_outbuf;
    }
    while (get_from_q(&d->input, buf2, &dummy));
}

/* Add a new string to a player's output queue */
void write_to_output(const char *txt, struct descriptor_data *t)
{
    int size, i, j;
    char txt1[LARGE_BUFSIZE];

    /* if we're in the overflow state already, ignore this new output */
    if (t->bufptr < 0)
        return;

    if ((ubyte) * txt == 255) {
        return;
    }

    size = strlen(txt);
    for (i = 0, j = 0; i < size; i++, j++)
        if (*(txt + i) == '\r' && *(txt + i + 1) == '\n') {
            txt1[j] = ' ';
            txt1[j + 1] = txt[i];
            j++;
        } else
            txt1[j] = *(txt + i);
    txt1[j] = '\0';

    size = strlen(txt1);
    /* if we have enough space, just write to buffer and that's it! */
    if (t->bufspace >= size) {
        strcpy(t->output + t->bufptr, txt1);
        t->bufspace -= size;
        t->bufptr += size;
        return;
    }
    /*
     * If the text is too big to fit into even a large buffer, chuck the
     * new text and switch to the overflow state.
     */
    if (size + t->bufptr > LARGE_BUFSIZE - 1) {
        t->bufptr = -1;
        buf_overflows++;
        return;
    }
    buf_switches++;

    /* if the pool has a buffer in it, grab it */
    if (bufpool != NULL) {
        t->large_outbuf = bufpool;
        bufpool = bufpool->next;
    } else {                    /* else create a new one */
        CREATE(t->large_outbuf, struct txt_block, 1);
        CREATE(t->large_outbuf->text, char, LARGE_BUFSIZE);

        buf_largecount++;
    }

    strcpy(t->large_outbuf->text, t->output);   /* copy to big buffer */
    t->output = t->large_outbuf->text;  /* make big buffer primary */
    strcat(t->output, txt1);    /* now add new text */

    /* set the pointer for the next write */
    t->bufptr = strlen(t->output);
    /* calculate how much space is left in the buffer */
    t->bufspace = LARGE_BUFSIZE - 1 - t->bufptr;
}



/* ******************************************************************
*  socket handling                                                  *
****************************************************************** */


/*
 * get_bind_addr: Return a struct in_addr that should be used in our
 * call to bind().  If the user has specified a desired binding
 * address, we try to bind to it; otherwise, we bind to INADDR_ANY.
 * Note that inet_aton() is preferred over inet_addr() so we use it if
 * we can.  If neither is available, we always bind to INADDR_ANY.
 */

struct in_addr *get_bind_addr()
{
    static struct in_addr bind_addr;

    /* Clear the structure */
    memset((char *) &bind_addr, 0, sizeof(bind_addr));

    /* If DLFT_IP is unspecified, use INADDR_ANY */
    if (DFLT_IP == NULL) {
        bind_addr.s_addr = htonl(INADDR_ANY);
    } else {
        /* If the parsing fails, use INADDR_ANY */
        if (!parse_ip(DFLT_IP, &bind_addr)) {
            log("SYSERR: DFLT_IP of %s appears to be an invalid IP address", DFLT_IP);
            bind_addr.s_addr = htonl(INADDR_ANY);
        }
    }

    /* Put the address that we've finally decided on into the logs */
    if (bind_addr.s_addr == htonl(INADDR_ANY))
        log("Доступно для всех IP адресов.");
    else
        log("Доступно только для IP адреса: %s", inet_ntoa(bind_addr));

    return (&bind_addr);
}

#ifdef HAVE_INET_ATON

/*
 * inet_aton's interface is the same as parse_ip's: 0 on failure, non-0 if
 * successful
 */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
    return (inet_aton(addr, inaddr));
}

#elif HAVE_INET_ADDR

/* inet_addr has a different interface, so we emulate inet_aton's */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
    long ip;

    if ((ip = inet_addr(addr)) == -1) {
        return (0);
    } else {
        inaddr->s_addr = (unsigned long) ip;
        return (1);
    }
}

#else

/* If you have neither function - sorry, you can't do specific binding. */
int parse_ip(const char *addr, struct in_addr *inaddr)
{
    log("SYSERR: warning: you're trying to set DFLT_IP but your system has no\n"
        "functions to parse IP addresses (how bizarre!)");
    return (0);
}

#endif                          /* INET_ATON and INET_ADDR */

unsigned long get_ip(const char *addr)
{
    static struct in_addr ip;

    parse_ip(addr, &ip);
    return (ip.s_addr);
}


/* Sets the kernel's send buffer size for the descriptor */
int set_sendbuf(socket_t s)
{
#if defined(SO_SNDBUF) &&!defined(CIRCLE_MACINTOSH)
    int opt = MAX_SOCK_BUF;

    if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0) {
        syserr("SYSERR: setsockopt SNDBUF");
        return (-1);
    }
#if 0
    if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *) &opt, sizeof(opt)) < 0) {
        syserr("SYSERR: setsockopt RCVBUF");
        return (-1);
    }
#endif

#endif

    return (0);
}

/* Initialize a descriptor */
void init_descriptor(struct descriptor_data *newd, int desc, int key)
{

    newd->descriptor = desc;
    newd->idle_tics = 0;
    newd->output = newd->small_outbuf;
    newd->bufspace = SMALL_BUFSIZE - 1;
    newd->input_time = time(0);
    newd->login_time = time(0);
    *newd->output = '\0';
    newd->bufptr = 0;
    newd->has_prompt = 1;       /* prompt is part of greetings */
    STATE(newd) = CON_GET_KEYTABLE;
    newd->keytable = key;
    newd->snoop_by_col = 0;     //Обнуление счетчика кол-ва персоонаже которые снупают этот

    CREATE(newd->history, char *, HISTORY_SIZE);

    if (++last_desc == 1000)
        last_desc = 1;
    newd->desc_num = last_desc;

}

int new_descriptor(socket_t s)
{
    socket_t desc;
    int sockets_connected = 0;
    socklen_t i;
    struct descriptor_data *newd;
    struct sockaddr_in peer;
    struct hostent *from;
    char buf[MAX_STRING_LENGTH];

    /* accept the new connection */
    i = sizeof(peer);
    if ((desc = accept(s, (struct sockaddr *) &peer, &i)) == INVALID_SOCKET) {
        syserr("SYSERR: accept");
        return (-1);
    }
    /* keep it from blocking */
    nonblock(desc);

#ifdef IPTOS_LOWDELAY
    {
        // Выставляем приоритет для интерактивных пакетов
        int opt = IPTOS_LOWDELAY;

        if (setsockopt(s, SOL_IP, IP_TOS, &opt, sizeof(opt)) < 0) {
            syserr("Can't set IPTOS_LOWDELAY\n");
        }
    }
#endif
#ifndef _MINGW_
#ifdef TCP_NODELAY
    {
        // не накапливать информацию в буфере перед посылкой абоненту
        // ведет к уменьшению размеров пакетов и их отправлению без накопления информации в буфере
        int opt = 1;

        if (setsockopt(s, SOL_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
            syserr("Can't set TCP_NODELAY\n");
        }
    }
#endif
#endif

    /* set the send buffer size */
    if (set_sendbuf(desc) < 0) {
        CLOSE_SOCKET(desc);
        return (0);
    }

    /* make sure we have room for it */
    for (newd = descriptor_list; newd; newd = newd->next)
        sockets_connected++;

    if (sockets_connected >= max_players) {
        SEND_TO_SOCKET("Sorry, Planescape MUD is full right now... please try again later!\r\n",
                       desc);
        CLOSE_SOCKET(desc);
        return (0);
    }
    /* create a new descriptor */
    CREATE(newd, struct descriptor_data, 1);
    memset((char *) newd, 0, sizeof(struct descriptor_data));

    /* find the sitename */
    if (nameserver_is_slow || !(from = gethostbyaddr((char *) &peer.sin_addr, sizeof(peer.sin_addr), AF_INET))) {       /* resolution failed */
        if (!nameserver_is_slow)
            syserr("SYSERR: gethostbyaddr");

        /* find the numeric site address */
        strncpy(newd->host, (char *) inet_ntoa(peer.sin_addr), HOST_LENGTH);
        *(newd->host + HOST_LENGTH) = '\0';
    } else {
        strncpy(newd->host, from->h_name, HOST_LENGTH);
        newd->ip_addr = ntohl(peer.sin_addr.s_addr);
        *(newd->host + HOST_LENGTH) = '\0';
    }

    /* determine if the site is banned */
    if (isbanned(newd->host) == BAN_ALL) {
        sprintf(buf, "Your host [%s] is banned!\r\n", newd->host);
        perform_socket_write(desc, buf, strlen(buf));
        CLOSE_SOCKET(desc);
        //sprintf(buf2, "Connection attempt denied from [%s]", newd->host);
        //mudlog(buf2, CMP, LVL_HIGOD, TRUE);
        free(newd);
        return (0);
    }
#if 0
    /*
     * Log new connections - probably unnecessary, but you may want it.
     * Note that your immortals may wonder if they see a connection from
     * your site, but you are wizinvis upon login.
     */
    sprintf(buf2, "New connection from [%s]", newd->host);
    mudlog(buf2, CMP, LVL_HIGOD, FALSE);
#endif



    init_descriptor(newd, desc, KT_SELECTMENU);
    //set_binary(newd);
    echo_on(newd);
    SEND_TO_Q(mud->getTextFileLoader()->get("greetings").c_str(), newd);
    /*
     * This isn't exactly optimal but allows us to make a design choice.
     * Do we embed the history in descriptor_data or keep it dynamically
     * allocated and allow a user defined history size?
     */

    /*  CREATE(newd->history, char *, HISTORY_SIZE);

       if (++last_desc == 1000)
       last_desc = 1;
       newd->desc_num = last_desc; */

    /* prepend to list */
    newd->next = descriptor_list;
    descriptor_list = newd;

#ifdef HAVE_ICONV
    SEND_TO_Q("\r\nSelect your charset:"
              " 0.KOI-8" " 1.DOS" " 2.JMC" " 3.ZMUD" " 4.ZMUD6+" " 5.UTF-8" " :", newd);
#else
    SEND_TO_Q("\r\nSelect your charset:"
              " 0.KOI-8" " 1.DOS" " 2.JMC" " 3.ZMUD" " 4.ZMUD6+" " :", newd);
#endif

#if defined(HAVE_ZLIB)
    //write_to_descriptor(newd->descriptor, will_sig, strlen(will_sig));
    write_to_descriptor(newd->descriptor, compress_will, strlen(compress_will));
    newd->OutBytes += strlen(compress_will);
#endif
    return (0);
}

/*
 * Send all of the output that we've accumulated for a player out to
 * the player's descriptor.
 */
int process_output(struct descriptor_data *t)
{
    char i[MAX_SOCK_BUF * 2], o[MAX_SOCK_BUF * 2], *osb = i + 2, *pi, *po;
    int result, written = 0, offset, c, i_count;

    //Снуперы
    if (t->snoop_by_col) {
        for (i_count = 0; i_count < t->snoop_by_col; i_count++) {
            SEND_TO_Q("% ", t->snoop_by[i_count]);
            SEND_TO_Q(t->output, t->snoop_by[i_count]);
            //SEND_TO_Q("", t->snoop_by[i_count]);
        }
    }

    pi = i;
    po = o;

    /* we may need this \r\n for later -- see below */
    strcpy(i, "\r\n");          /* strcpy: OK (for 'MAX_SOCK_BUF >= 3') */

    /* now, append the 'real' output */
    strcpy(osb, t->output);     /* strcpy: OK (t->output:LARGE_BUFSIZE < osb:MAX_SOCK_BUF-2) */

    /* if we're in the overflow state, notify the user */
    if (t->bufspace == 0)
        strcat(osb, "[ ПЕРЕПОЛНЕНИЕ ]\r\n");    /* strcpy: OK (osb:MAX_SOCK_BUF-2 reserves space) */

    /* add the extra CRLF if the person isn't in compact mode */
    if (STATE(t) == CON_PLAYING && t->character && !PRF_FLAGGED(t->character, PRF_COMPACT))
        strcat(i, "\r\n");

    /* add a prompt */
    strncat(i, make_prompt(t), MAX_PROMPT_LENGTH);


    /* easy color */
    if (t->character)
        proc_color(i, (clr(t->character, C_NRM)));

    /*
     * now, send the output.  If this is an 'interruption', use the prepended
     * CRLF, otherwise send the straight output sans CRLF.
     */

    // замена на пробел
    for (c = 0; *(pi + c); c++)
        *(pi + c) = (*(pi + c) == '^') ? ' ' : *(pi + c);

    switch (t->keytable) {
        case KT_ALT:
            for (; *pi; *po = KtoA(*pi), pi++, po++);
            break;
        case KT_WIN:
            for (; *pi; *po = KtoW(*pi), pi++, po++)
                if (*pi == 'я')
                    *(po++) = 255;
            break;
        case KT_WINZ:
            for (; *pi; *po = KtoW2(*pi), pi++, po++);
            break;
        case KT_WINZ6:
            for (; *pi; *po = KtoW2(*pi), pi++, po++);
            break;
#ifdef HAVE_ICONV
	case KT_UTF8:
		koi_to_utf8(pi, po);
		break;
#endif
        default:
            for (; *pi; *po = *pi, pi++, po++);
            break;
    }
    if (t->keytable != KT_UTF8)
	{
		*po = '\0';
	}
    for (c = 0; o[c]; c++) {
        i[c] = o[c];
    }
    i[c] = 0;

    if (t->has_prompt)
        offset = 0;
    else
        offset = 2;

    /*
     * This huge #ifdef could be a function of its own, if desired. -gg 2/27/99
     */

#if defined(HAVE_ZLIB)
    if (t->deflate) {           /* Complex case, compression, write it out. */
        /* Keep compiler happy, and MUD, just in case we don't write anything. */
        result = 1;

        /* First we set up our input data. */
        t->deflate->avail_in = strlen(i + offset);
        t->deflate->next_in = (Bytef *) (i + offset);

        do {
            int df, prevsize = SMALL_BUFSIZE - t->deflate->avail_out;

            /* If there is input or the output has reset from being previously full, run compression again. */
            if (t->deflate->avail_in || t->deflate->avail_out == SMALL_BUFSIZE)
                if ((df = deflate(t->deflate, Z_SYNC_FLUSH)) != 0)
                    log("SYSERR: process_output: deflate() returned %d.", df);

            /* There should always be something new to write out. */
            result =
                write_to_descriptor(t->descriptor, t->small_outbuf + prevsize,
                                    SMALL_BUFSIZE - t->deflate->avail_out - prevsize);
            t->OutBytes += SMALL_BUFSIZE - t->deflate->avail_out - prevsize;
            /* Wrap the buffer when we've run out of buffer space for the output. */
            if (t->deflate->avail_out == 0) {
                t->deflate->avail_out = SMALL_BUFSIZE;
                t->deflate->next_out = (Bytef *) t->small_outbuf;
            }

            /* Oops. This shouldn't happen, I hope. -gg 2/19/99 */
            if (result <= 0)
                return -1;

            /* Need to loop while we still have input or when the output buffer was previously full. */
        } while (t->deflate->avail_out == SMALL_BUFSIZE || t->deflate->avail_in);
    } else
#endif
        do {
            result = write_to_descriptor(t->descriptor, i + offset, strlen(i + offset));
            t->OutBytes += strlen(i + offset);
        } while (0);

    written = result >= 0 ? result : -result;

    if (t->large_outbuf) {
        t->large_outbuf->next = bufpool;
        bufpool = t->large_outbuf;
        t->large_outbuf = NULL;
        t->output = t->small_outbuf;
    }
    /* reset total bufspace back to that of a small buffer */
    t->bufspace = SMALL_BUFSIZE - 1;
    t->bufptr = 0;
    *(t->output) = '\0';

    /* Error, cut off. */
    if (result == 0)
        return (-1);

    /* Normal case, wrote ok. */
    if (result > 0)
        return (1);

    /*
     * We blocked, restore the unwritten output. Known
     * bug in that the snooping immortal will see it twice
     * but details...
     */
    write_to_output(i + written + offset, t);
    return (0);

}


/*
 * perform_socket_write: takes a descriptor, a pointer to text, and a
 * text length, and tries once to send that text to the OS.  This is
 * where we stuff all the platform-dependent stuff that used to be
 * ugly #ifdef's in write_to_descriptor().
 *
 * This function must return:
 *
 * -1  If a fatal error was encountered in writing to the descriptor.
 *  0  If a transient failure was encountered (e.g. socket buffer full).
 * >0  To indicate the number of bytes successfully written, possibly
 *     fewer than the number the caller requested be written.
 *
 * Right now there are two versions of this function: one for Windows,
 * and one for all other platforms.
 */

#if defined(CIRCLE_WINDOWS)

ssize_t perform_socket_write(socket_t desc, const char *txt, size_t length)
{
    ssize_t result;

    result = send(desc, txt, length, 0);

    if (result > 0) {
        /* Write was sucessful */
        return (result);
    }

    if (result == 0) {
        /* This should never happen! */
        log("SYSERR: Huh??  write() returned 0???  Please report this!");
        return (-1);
    }

    /* result < 0: An error was encountered. */

    /* Transient error? */
    if (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAEINTR)
        return (0);

    /* Must be a fatal error. */
    return (-1);
}

#else

#if defined(CIRCLE_ACORN)
#define write   socketwrite
#endif

/* perform_socket_write for all Non-Windows platforms */
ssize_t perform_socket_write(socket_t desc, const char *txt, size_t length)
{
    ssize_t result;

    result = write(desc, txt, length);

    if (result > 0) {
        /* Write was successful. */
        return (result);
    }

    if (result == 0) {
        /* This should never happen! */
        log("SYSERR: Huh??  write() returned 0???  Please report this!");
        return (-1);
    }

    /*
     * result < 0, so an error was encountered - is it transient?
     * Unfortunately, different systems use different constants to
     * indicate this.
     */

#ifdef EAGAIN                   /* POSIX */
    if (errno == EAGAIN)
        return (0);
#endif

#ifdef EWOULDBLOCK              /* BSD */
    if (errno == EWOULDBLOCK)
        return (0);
#endif

#ifdef EDEADLK                  /* Macintosh */
    if (errno == EDEADLK)
        return (0);
#endif

    /* Looks like the error was fatal.  Too bad. */
    return (-1);
}

#endif                          /* CIRCLE_WINDOWS */


/*
 * write_to_descriptor takes a descriptor, and text to write to the
 * descriptor.  It keeps calling the system-level write() until all
 * the text has been delivered to the OS, or until an error is
 * encountered. 'written' is updated to add how many bytes were sent
 * over the socket successfully prior to the return. It is not zero'd.
 *
 * Returns:
 *  +  All is well and good.
 *  0  A fatal or unexpected error was encountered.
 *  -  The socket write would block.
 */

int write_to_descriptor(socket_t desc, const char *txt)
{
    return (write_to_descriptor(desc, txt, strlen(txt) + 1));
}

int write_to_descriptor(socket_t desc, const char *txt, size_t total)
{
    ssize_t bytes_written, total_written = 0;

    if (total == 0) {
        log("write_to_descriptor: write nothing?!");
        return 0;
    }

    while (total > 0) {
        bytes_written = perform_socket_write(desc, txt, total);

        if (bytes_written < 0) {
            /* Fatal error.  Disconnect the player. */
            syserr("SYSERR: write_to_descriptor");
            return (0);
        } else if (bytes_written == 0) {
            /*
             * Temporary failure -- socket buffer full.  For now we'll just
             * cut off the player, but eventually we'll stuff the unsent
             * text into a buffer and retry the write later.  JE 30 June 98.
             * Implemented the anti-cut-off code he wanted. GG 13 Jan 99.
             */
            log("WARNING: write_to_descriptor: socket write would block.");
            return (0);
            //return (total_written);
            //return (-total_written);
        } else {
            txt += bytes_written;
            total -= bytes_written;
            total_written += bytes_written;
            OutBytes += total_written;
        }
    }

    return (total_written);
}


/*
 * Same information about perform_socket_write applies here. I like
 * standards, there are so many of them. -gg 6/30/98
 */
ssize_t perform_socket_read(socket_t desc, char *read_point, size_t space_left)
{
    ssize_t ret;

#ifdef _MINGW_
    ret = recv(desc, read_point, space_left, 0);
#else
    ret = read(desc, read_point, space_left);
#endif

    /* Read was successful. */
    if (ret > 0)
        return (ret);

    /* read() returned 0, meaning we got an EOF. */
    if (ret == 0) {
        log("WARNING: EOF on socket read (connection broken by peer)");
        return (-1);
    }

    /*
     * read returned a value < 0: there was an error
     */

#if defined(CIRCLE_WINDOWS)     /* Windows */
    if (WSAGetLastError() == WSAEWOULDBLOCK || WSAGetLastError() == WSAEINTR)
        return (0);
#else

#ifdef EINTR                    /* Interrupted system call - various platforms */
    if (errno == EINTR)
        return (0);
#endif

#ifdef EAGAIN                   /* POSIX */
    if (errno == EAGAIN)
        return (0);
#endif

#ifdef EWOULDBLOCK              /* BSD */
    if (errno == EWOULDBLOCK)
        return (0);
#endif                          /* EWOULDBLOCK */

#ifdef EDEADLK                  /* Macintosh */
    if (errno == EDEADLK)
        return (0);
#endif

#endif                          /* CIRCLE_WINDOWS */

    /*
     * We don't know what happened, cut them off. This qualifies for
     * a SYSERR because we have no idea what happened at this point.
     */
    syserr("SYSERR: perform_socket_read: about to lose connection");
    return (-1);
}

/*
 * ASSUMPTION: There will be no newlines in the raw input buffer when this
 * function is called.  We must maintain that before returning.
 */
int process_input(struct descriptor_data *t)
{
    int buf_length, failed_subst;
    ssize_t bytes_read;
    size_t space_left;
    char *ptr, *read_point, *write_point, *nl_pos;
    char tmp[MAX_INPUT_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    int i_count = 0;

    /* first, find the point where we left off reading data */
    buf_length = strlen(t->inbuf);
    read_point = t->inbuf + buf_length;
    space_left = MAX_RAW_INPUT_LENGTH - buf_length - 1;

    do {
        if (space_left <= 0) {
            log("WARNING: process_input: about to close connection: input overflow");
            return (-1);
        }

        bytes_read = perform_socket_read(t->descriptor, read_point, space_left);
        t->InBytes += bytes_read;

        if (bytes_read < 0) {   /* Error, disconnect them. */
            return (-1);
        } else if (bytes_read == 0)     /* Just blocking, no problems. */
            return (0);

        /* at this point, we know we got some data from the read */

        read_point[bytes_read] = '\0';  /* terminate the string */

#if defined(HAVE_ZLIB)
        /* Search for an "Interpret As Command" marker. */
        for (ptr = read_point; *ptr; ptr++) {
            if (ptr[0] != (char) IAC)
                continue;
            if (ptr[1] == (char) IAC) {
                // последовательность IAC IAC
                // следует заменить просто на один IAC, но
                // для раскладок KT_WIN/KT_WINZ6 это произойдет ниже.
                // Почему так сделано - не знаю, но заменять не буду.
                ++ptr;
            } else if (ptr[1] == (char) DO) {
                if (ptr[2] == (char) TELOPT_COMPRESS)
                    mccp_start(t, 1);
                else if (ptr[2] == (char) TELOPT_COMPRESS2)
                    mccp_start(t, 2);
                memmove(ptr, ptr + 3, bytes_read - (ptr - read_point) - 3 + 1);
                bytes_read -= 3;
                --ptr;
            } else if (ptr[1] == (char) DONT) {
                if (ptr[2] == (char) TELOPT_COMPRESS)
                    mccp_end(t, 1);
                else if (ptr[2] == (char) TELOPT_COMPRESS2)
                    mccp_end(t, 2);
                memmove(ptr, ptr + 3, bytes_read - (ptr - read_point) - 3 + 1);
                bytes_read -= 3;
                --ptr;
            }
        }


#endif

        /* search for a newline in the data we just read */
        for (ptr = read_point, nl_pos = NULL; *ptr && !nl_pos;) {
            if (ISNEWL(*ptr))
                nl_pos = ptr;
            ptr++;
        }

        read_point += bytes_read;
        space_left -= bytes_read;
        InBytes += bytes_read;


        /*
         * on some systems such as AIX, POSIX-standard nonblocking I/O is broken,
         * causing the MUD to hang when it encounters input not terminated by a
         * newline.  This was causing hangs at the Password: prompt, for example.
         * I attempt to compensate by always returning after the _first_ read, instead
         * of looping forever until a read returns -1.  This simulates non-blocking
         * I/O because the result is we never call read unless we know from select()
         * that data is ready (process_input is only called if select indicates that
         * this descriptor is in the read set).  JE 2/23/95.
         */
#if!defined(POSIX_NONBLOCK_BROKEN)
    } while (nl_pos == NULL);
#else
    }
    while (0);

    if (nl_pos == NULL)
        return (0);
#endif                          /* POSIX_NONBLOCK_BROKEN */

    /*
     * okay, at this point we have at least one newline in the string; now we
     * can copy the formatted data to a new array for further processing.
     */

    read_point = t->inbuf;


    while (nl_pos != NULL) {
        write_point = tmp;
        space_left = MAX_INPUT_LENGTH - 1;

        for (ptr = read_point; (space_left > 1) && (ptr < nl_pos); ptr++) {
            /* Нафиг точку с запятой - задрали уроды с тригерами (Кард) */
            if (*ptr == ';' && (STATE(t) == CON_PLAYING || (STATE(t) == CON_EXDESC))) {
                if (GET_LEVEL(t->character) < LVL_GOD)
                    *ptr = ',';
            }
            if (*ptr == '&' && (STATE(t) == CON_PLAYING || (STATE(t) == CON_EXDESC))) {
                if (GET_LEVEL(t->character) < LVL_GOD)
                    *ptr = '8';
            }
            if (*ptr == '\\' && (STATE(t) == CON_PLAYING || (STATE(t) == CON_EXDESC))) {
                if (GET_LEVEL(t->character) < LVL_GOD)
                    *ptr = '/';
            }
            /*****/
            if (*ptr == '\b' || *ptr == 127) {  /* handle backspacing or delete key */
                if (write_point > tmp) {
                    if (*(--write_point) == '$') {
                        write_point--;
                        space_left += 2;
                    } else
                        space_left++;
                }
            } else if (isascii(*ptr) && isprint(*ptr)) {
                if ((*(write_point++) = *ptr) == '$') { /* copy one character */
                    *(write_point++) = '$';     /* if it's a $, double it */
                    space_left -= 2;
                } else
                    space_left--;
            } else if ((ubyte) * ptr > 127) {
                switch (t->keytable) {
                    default:
                        t->keytable = 0;
                    case 0:
                        *(write_point++) = *ptr;
                        break;
		    case KT_UTF8:
			*(write_point++) = *ptr;
			break;
                    case KT_ALT:
                        *(write_point++) = AtoK(*ptr);
                        break;
                    case KT_WIN:
                    case KT_WINZ6:
                        *(write_point++) = WtoK(*ptr);
                        if (*ptr == (char) 255 && *(ptr + 1) == (char) 255 && ptr + 1 < nl_pos)
                            ptr++;
                        break;
                    case KT_WINZ:
                        *(write_point++) = WtoK(*ptr);
                        break;
                }
                space_left--;
            }
        }

        *write_point = '\0';

#ifdef HAVE_ICONV
		if (t->keytable == KT_UTF8)
		{
			int i;
			char utf8_tmp[MAX_SOCK_BUF * 2 * 3];
			size_t len_i, len_o;

			len_i = strlen(tmp);

			for (i = 0; i < MAX_SOCK_BUF * 2 * 3; i++)
			{
				utf8_tmp[i] = 0;
			}
			utf8_to_koi(tmp, utf8_tmp);
			len_o = strlen(utf8_tmp);
			strncpy(tmp, utf8_tmp, MAX_INPUT_LENGTH - 1);
			space_left = space_left + len_i - len_o;
		}
#endif


        if ((space_left <= 0) && (ptr < nl_pos)) {
            char buffer[MAX_INPUT_LENGTH + 64];

            sprintf(buffer, "Line too long.  Truncated to:\r\n%s\r\n", tmp);
            SEND_TO_Q(buffer, t);
        }
//ADDED BY HMEPAS FOR MULTISNOOPING
        if (t->snoop_by_col)
            for (i_count = 0; i_count < t->snoop_by_col; i_count++) {
                SEND_TO_Q("% ", t->snoop_by[i_count]);
                SEND_TO_Q(tmp, t->snoop_by[i_count]);
                SEND_TO_Q("\r\n", t->snoop_by[i_count]);
            }
//END OF CHAGING
        failed_subst = 0;


        if ((tmp[0] == '~') && (tmp[1] == 0)) {
            // очистка входной очереди
            int dummy;

            while (get_from_q(&t->input, buf2, &dummy));
            // SEND_TO_Q("Входной буфер очищен.\r\n", t);
            tmp[0] = 0;
        } else if (*tmp == '!' && !(*(tmp + 1)))
            /* Redo last command. */
            strcpy(tmp, t->last_input);
        else if (*tmp == '!' && *(tmp + 1)) {
            char *commandln = (tmp + 1);
            int starting_pos = t->history_pos,
                cnt = (t->history_pos == 0 ? HISTORY_SIZE - 1 : t->history_pos - 1);

            skip_spaces(&commandln);
            for (; cnt != starting_pos; cnt--) {
                if (t->history[cnt] && is_abbrev(commandln, t->history[cnt])) {
                    strcpy(tmp, t->history[cnt]);
                    strcpy(t->last_input, tmp);
                    SEND_TO_Q(tmp, t);
                    SEND_TO_Q("\r\n", t);
                    break;
                }
                if (cnt == 0)   /* At top, loop to bottom. */
                    cnt = HISTORY_SIZE;
            }
        } else if (*tmp == '^') {
            if (!(failed_subst = perform_subst(t, t->last_input, tmp)))
                strcpy(t->last_input, tmp);
        } else {
            strcpy(t->last_input, tmp);
            if (t->history[t->history_pos])
                free(t->history[t->history_pos]);       /* Clear the old line. */
            t->history[t->history_pos] = str_dup(tmp);  /* Save the new. */
            if (++t->history_pos >= HISTORY_SIZE)       /* Wrap to top. */
                t->history_pos = 0;
        }

        if (!failed_subst)
            write_to_q(tmp, &t->input, 0);

        /* find the end of this line */
        while (ISNEWL(*nl_pos))
            nl_pos++;

        /* see if there's another newline in the input buffer */
        read_point = ptr = nl_pos;
        for (nl_pos = NULL; *ptr && !nl_pos; ptr++)
            if (ISNEWL(*ptr))
                nl_pos = ptr;
    }

    /* now move the rest of the buffer up to the beginning for the next pass */
    write_point = t->inbuf;
    while (*read_point)
        *(write_point++) = *(read_point++);
    *write_point = '\0';

    return (1);
}



/* perform substitution for the '^..^' csh-esque syntax orig is the
 * orig string, i.e. the one being modified.  subst contains the
 * substition string, i.e. "^telm^tell"
 */
int perform_subst(struct descriptor_data *t, char *orig, char *subst)
{
    char newsub[MAX_INPUT_LENGTH + 5];

    char *first, *second, *strpos;

    /*
     * first is the position of the beginning of the first string (the one
     * to be replaced
     */
    first = subst + 1;

    /* now find the second '^' */
    if (!(second = strchr(first, '^'))) {
        SEND_TO_Q("Invalid substitution.\r\n", t);
        return (1);
    }
    /* terminate "first" at the position of the '^' and make 'second' point
     * to the beginning of the second string */
    *(second++) = '\0';

    /* now, see if the contents of the first string appear in the original */
    if (!(strpos = strstr(orig, first))) {
        SEND_TO_Q("Invalid substitution.\r\n", t);
        return (1);
    }
    /* now, we construct the new string for output. */

    /* first, everything in the original, up to the string to be replaced */
    strncpy(newsub, orig, (strpos - orig));
    newsub[(strpos - orig)] = '\0';

    /* now, the replacement string */
    strncat(newsub, second, (MAX_INPUT_LENGTH - strlen(newsub) - 1));

    /* now, if there's anything left in the original after the string to
     * replaced, copy that too. */
    if (((strpos - orig) + strlen(first)) < strlen(orig))
        strncat(newsub, strpos + strlen(first), (MAX_INPUT_LENGTH - strlen(newsub) - 1));

    /* terminate the string in case of an overflow from strncat */
    newsub[MAX_INPUT_LENGTH - 1] = '\0';
    strcpy(subst, newsub);

    return (0);
}



void close_socket(struct descriptor_data *d, int direct)
{
    char buf[512];
    struct descriptor_data *temp;
    int cyc_count;

    if (!direct && d->character && !IS_NPC(d->character) && RENTABLE(d->character))
        return;

    REMOVE_FROM_LIST(d, descriptor_list, next);
    CLOSE_SOCKET(d->descriptor);
    flush_queues(d);

    /* Forget snooping */
    log("Forget snooping");
    if (d->snooping) {
        del_snooper(d->character);
        d->snooping = NULL;
    }

    if (d->snoop_by_col) {
        for (cyc_count = 0; cyc_count < d->snoop_by_col; cyc_count++) {
            SEND_TO_Q("Ваша жертва больше недоступна.\r\n", d->snoop_by[cyc_count]);
            d->snoop_by[cyc_count]->snooping = NULL;
        }
        d->snoop_by_col = 0;
        d->snoop_by = NULL;
    }


    if (d->character) {         /*
                                 * Plug memory leak, from Eric Green.
                                 */
        if (!IS_NPC(d->character)
            && (PLR_FLAGGED(d->character, PLR_MAILING)
                || PLR_FLAGGED(d->character, PLR_SCRIPTING))
            && d->str) {
            if (*(d->str))
                free(*(d->str));
            free(d->str);
        }
        if (STATE(d) == CON_PLAYING || STATE(d) == CON_DISCONNECT) {
            act("1+и погрузил1(ся,ась,ось,ись) в транс, перестав воспринимать действительность.",
                "Км", d->character);
            if (!IS_NPC(d->character)) {
                save_char(d->character, NOWHERE);
                save_pets(d->character);
                save_vars(d->character);
                save_quests(d->character);
                xsave_rent(d->character, RENT_LD, FALSE);
                check_light(d->character, LIGHT_NO, LIGHT_NO, LIGHT_NO, LIGHT_NO, -1);
                sprintf(buf, "Закрыто соединение с %s.", GET_NAME(d->character));
                mudlog(buf, NRM, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE);
            }
            d->character->desc = NULL;
        } else if (d->character) {
            mudlogf(CMP, LVL_IMMORT, TRUE, "Потеряно соединение с %s.",
                    GET_NAME(d->character) ? GET_NAME(d->character) : "<null>");
            update_ptable_data(d->character);
            delete(d->character);
            d->character = NULL;
        }
    }

    /* JE 2/22/95 -- part of my unending quest to make switch stable */
    if (d->original && d->original->desc)
        d->original->desc = NULL;

    /* Clear the command history. */
    if (d->history) {
        int cnt;

        for (cnt = 0; cnt < HISTORY_SIZE; cnt++)
            if (d->history[cnt])
                free(d->history[cnt]);
        free(d->history);
    }

    if (d->showstr_head)
        free(d->showstr_head);
    if (d->showstr_count)
        free(d->showstr_vector);

#if defined(HAVE_ZLIB)
    if (d->deflate) {
        deflateEnd(d->deflate);
        free(d->deflate);
    }
#endif
    free(d);
}



void check_idle_passwords(void)
{
    struct descriptor_data *d, *next_d;

    for (d = descriptor_list; d; d = next_d) {
        next_d = d->next;
        if (STATE(d) != CON_PASSWORD && STATE(d) != CON_GET_NAME && STATE(d) != CON_GET_KEYTABLE)
            continue;
        if (!d->idle_tics) {
            d->idle_tics++;
            continue;
        } else {
            echo_on(d);
            SEND_TO_Q("\r\nTime out. Socket closed.\r\n", d);
            STATE(d) = CON_CLOSE;
        }
    }
}



/*
 * I tried to universally convert Circle over to POSIX compliance, but
 * alas, some systems are still straggling behind and don't have all the
 * appropriate defines.  In particular, NeXT 2.x defines O_NDELAY but not
 * O_NONBLOCK.  Krusty old NeXT machines!  (Thanks to Michael Jones for
 * this and various other NeXT fixes.)
 */

#if defined(CIRCLE_WINDOWS)

void nonblock(socket_t s)
{
    unsigned long val = 1;

    ioctlsocket(s, FIONBIO, &val);
}

#elif defined(CIRCLE_AMIGA)

void nonblock(socket_t s)
{
    long val = 1;

    IoctlSocket(s, FIONBIO, &val);
}

#elif defined(CIRCLE_ACORN)

void nonblock(socket_t s)
{
    int val = 1;

    socket_ioctl(s, FIONBIO, &val);
}

#elif defined(CIRCLE_VMS)

void nonblock(socket_t s)
{
    int val = 1;

    if (ioctl(s, FIONBIO, &val) < 0) {
        syserr("SYSERR: Fatal error executing nonblock (comm.c)");
        exit(1);
    }
}

#elif defined(CIRCLE_UNIX) || defined(CIRCLE_OS2) || defined(CIRCLE_MACINTOSH)

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

void nonblock(socket_t s)
{
    int flags;

    flags = fcntl(s, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if (fcntl(s, F_SETFL, flags) < 0) {
        syserr("SYSERR: Fatal error executing nonblock (comm.c)");
        exit(1);
    }
}

#endif                          /* CIRCLE_UNIX || CIRCLE_OS2 || CIRCLE_MACINTOSH */


/* ******************************************************************
*  signal-handling functions (formerly signals.c).  UNIX only.      *
****************************************************************** */

#ifdef CIRCLE_UNIX

void Handler(int signalnum)
{
    signal_destroy();

    struct descriptor_data *d;
    char buf[MAX_STRING_LENGTH];

    log("Работа прервана по сигналу %d (%s:%d)", signalnum, __FILE__, __LINE__);
    mud->getStats()->setRebootLevel(1);
    mud->getStats()->save();

    log("Сохраняю персонажей");
    xrent_save_all(RENT_CRASH);
    save_player_index();
    save_shops();

    for (d = descriptor_list; d; d = d->next) {
        toggle_compression(d);

        if ((STATE(d) == CON_PLAYING) &&
            (!PLR_FLAGGED(d->character, PLR_WRITING)) &&
            (!PLR_FLAGGED(d->character, PLR_SCRIPTING)) &&
            (!PLR_FLAGGED(d->character, PLR_MAILING)))

            sprintf(buf, "\r\n"
                    "****************************\r\n"
                    "*    КРИТИЧЕСКАЯ ОШИБКА.   *\r\n"
                    "* ------------------------ *\r\n"
                    "* ИГРА БУДЕТ ПЕРЕЗАГРУЖЕНА *\r\n"
                    "*   Ваш персонаж сохранен  * \r\n" "****************************\r\n");

        switch (d->keytable) {
            default:
            case 0:
                break;
            case KT_ALT:
                koi_to_alt(buf, strlen(buf));
                break;
            case KT_WIN:
            case KT_WINZ6:
                koi_to_win(buf, strlen(buf));
                break;
            case KT_WINZ:
                koi_to_winz(buf, strlen(buf));
                break;
        }

        SEND_TO_SOCKET(buf, d->descriptor);
    }

    signal(SIGSEGV, SIG_DFL);
    char *tmp = NULL;

    *tmp = 1;
    exit(1);
}


void unrestrict_game(int sig)
{
    signal_destroy();
    mudlog("Received SIGUSR2 - completely unrestricting game (emergent)", BRF, LVL_IMMORT, TRUE);
    ban_list = NULL;
    num_invalid = 0;
    mud->modRestrict.override(0);
}


/* clean up our zombie kids to avoid defunct processes */
void reap(int sig)
{
    signal_destroy();
    while (waitpid(-1, NULL, WNOHANG) > 0);

    signal(SIGCHLD, reap);
}


void checkpointing(int sig)
{
    if (!tics) {
        signal_destroy();
        log("SYSERR: CHECKPOINT shutdown: tics not updated. (Infinite loop suspected)");
        mud->getStats()->setRebootLevel(1);
        mud->getStats()->save();
        xrent_save_all(RENT_CRASH);
        save_player_index();
        save_shops();
        abort();
    } else
        tics = 0;
}

void hupsig(int sig)
{
    signal_destroy();
    log("Прерывание работы по системному сбросу. SHUTDOWN...");

    mud->getStats()->setRebootLevel(4);
    mud->getStats()->save();
    xrent_save_all(RENT_NORMAL);
    save_player_index();
    save_shops();
    exit(1);                    /* perhaps something more elegant should
                                 * substituted */
}

void signal_init()
{
    log("Установка обработчиков сигналов");

    struct itimerval itime;
    struct timeval interval;

    interval.tv_sec = 180;
    interval.tv_usec = 0;
    itime.it_interval = interval;
    itime.it_value = interval;
    setitimer(ITIMER_VIRTUAL, &itime, NULL);
    signal(SIGVTALRM, checkpointing);

    signal(SIGHUP, hupsig);
    signal(SIGCHLD, reap);

    signal(SIGINT, hupsig);
    signal(SIGTERM, hupsig);

    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
//    signal(SIGFPE, Handler);
//    signal(SIGSEGV, Handler);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTRAP, SIG_IGN);
//    signal(SIGILL, Handler);
    signal(SIGALRM, SIG_IGN);
//    signal(SIGBUS, Handler);
    signal(SIGSTOP, SIG_IGN);
}

void signal_destroy()
{
    signal(SIGVTALRM, SIG_DFL);

    signal(SIGHUP, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);

    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

    signal(SIGPIPE, SIG_DFL);
    signal(SIGALRM, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTRAP, SIG_DFL);
    signal(SIGILL, SIG_DFL);
    signal(SIGALRM, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGSTOP, SIG_DFL);

    log("Сброшены обработчики сигналов");
}

#endif

/* ****************************************************************
*       Public routines for system-to-player-communication        *
**************************************************************** */
void send_stat_char(struct char_data *ch)
{
    char fline[256];

    sprintf(fline, "%d[%d]HP %d[%d]Mv %dG %dL ",
            GET_HIT(ch), GET_REAL_MAX_HIT(ch),
            GET_MOVE(ch), GET_REAL_MAX_MOVE(ch), GET_GOLD(ch), GET_LEVEL(ch));
    SEND_TO_Q(fline, ch->desc);
}

void send_to_charf(struct char_data *ch, const char *messg, ...)
{
    va_list args;
    char send_buf[MAX_STRING_LENGTH];

    if (!ch->desc || !messg || !*messg)
        return;

    va_start(args, messg);
    vsnprintf(send_buf, MAX_STRING_LENGTH, messg, args);
    va_end(args);

    SEND_TO_Q(send_buf, ch->desc);
}


void send_to_all(const char *messg, ...)
{
    struct descriptor_data *i;
    char send_buf[MAX_STRING_LENGTH];
    va_list args;

    if (!messg || !*messg)
        return;

    va_start(args, messg);
    vsnprintf(send_buf, MAX_STRING_LENGTH, messg, args);
    va_end(args);

    for (i = descriptor_list; i; i = i->next)
        if (STATE(i) == CON_PLAYING)
            SEND_TO_Q(send_buf, i);
}


void send_to_weather(int type_weather, int num_zone)
{
    int type_zone;
    struct descriptor_data *i;
    struct room_data *room;

    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING || i->character == NULL)
            continue;
        if (!AWAKE(i->character) || !OUTSIDE(i->character))
            continue;

        room = &world[IN_ROOM(i->character)];

        type_zone = GET_ZONE(i->character).plane;

        if (world[IN_ROOM(i->character)].zone == num_zone) {
            if (weather_mess[type_zone][type_weather]) {
                SEND_TO_Q(weather_mess[type_zone][type_weather], i);
                SEND_TO_Q("\r\n", i);
            }
        }
    }

}

void send_to_outdoor(const char *messg, int control)
{
    int room;
    struct descriptor_data *i;

    if (!messg || !*messg)
        return;

    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING || i->character == NULL)
            continue;
        if (!AWAKE(i->character) || !OUTSIDE(i->character))
            continue;
        room = IN_ROOM(i->character);
        if (!control ||
            (IS_SET(control, SUN_CONTROL) &&
             room != NOWHERE &&
             SECT(room) != SECT_UNDERWATER && !AFF_FLAGGED(i->character, AFF_BLIND)
            ) ||
            (IS_SET(control, WEATHER_CONTROL) &&
             room != NOWHERE &&
             SECT(room) != SECT_UNDERWATER &&
             !ROOM_FLAGGED(room, ROOM_NOWEATHER) &&
             world[IN_ROOM(i->character)].weather.duration <= 0)
            )
            SEND_TO_Q(messg, i);
    }
}


void send_to_gods(const char *messg)
{
    struct descriptor_data *i;

    if (!messg || !*messg)
        return;

    for (i = descriptor_list; i; i = i->next) {
        if (STATE(i) != CON_PLAYING || i->character == NULL)
            continue;
        if (!IS_GOD(i->character))
            continue;
        SEND_TO_Q(messg, i);
    }
}


void send_to_room(const char *messg, room_rnum room, int to_awake)
{
    struct char_data *i;

    if (messg == NULL)
        return;

    for (i = world[room].people; i; i = i->next_in_room)
        if (i->desc && !IS_NPC(i) && (!to_awake || AWAKE(i)))
            SEND_TO_Q(messg, i->desc);
}



const char *ACTNULL = "<NULL>";

#define CHECK_NULL(pointer, expression) \
    if ((pointer) == NULL) i = ACTNULL; else i = (expression);

/* higher-level communication: the act() function */
void perform_act(const char *orig, struct char_data *ch, struct obj_data *obj,
                 const void *vict_obj, struct char_data *to)
{
    const char *i = NULL;
    char lbuf[MAX_STRING_LENGTH], *buf;
    ubyte padis;
    int stopbyte;

    buf = lbuf;

    for (stopbyte = 0; stopbyte < MAX_STRING_LENGTH; stopbyte++) {
        if (*orig == '$') {
            switch (*(++orig)) {
                case 'n':
                    if (*(orig + 1) < '0' || *(orig + 1) > '5') {
                        if (check_incognito(ch) && CAN_SEE(to, ch))
                            i = hide_race(ch, 0);
                        else
                            i = PERS(ch, to, 0);
                    } else {
                        padis = *(++orig) - '0';
                        if (check_incognito(ch) && CAN_SEE(to, ch))
                            i = hide_race(ch, padis);
                        else
                            i = PERS(ch, to, padis);
                    }
                    break;
                case 'N':
                    if (*(orig + 1) < '0' || *(orig + 1) > '5') {
                        if (vict_obj == NULL)
                            i = ACTNULL;
                        else {
                            if (check_incognito((struct char_data *) vict_obj) &&
                                CAN_SEE(to, (const struct char_data *) vict_obj))
                                i = hide_race((struct char_data *) vict_obj, 0);
                            else
                                i = PERS((struct char_data *) vict_obj, to, 0);
                        }
                    } else {
                        padis = *(++orig) - '0';
                        if (vict_obj == NULL)
                            i = ACTNULL;
                        else {
                            if (check_incognito((struct char_data *) vict_obj) &&
                                CAN_SEE(to, (const struct char_data *) vict_obj))
                                i = hide_race((struct char_data *) vict_obj, padis);
                            else
                                i = PERS((struct char_data *) vict_obj, to, padis);
                        }
                    }
                    break;

                case 'm':
                    i = HMHR(ch, to);
                    break;
                case 'M':
                    if (vict_obj)
                        i = HMHR((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, OMHR(obj));
                    break;

                case 's':
                    i = HSHR(ch, to);
                    break;
                case 'S':
                    if (vict_obj)
                        i = HSHR((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, OSHR(obj));
                    break;

                case 'd':
                    CHECK_NULL(obj, GET_OBJ_VIS_SUF_6(obj, to));
                    break;

                case 'c':
                    CHECK_NULL(obj, GET_OBJ_VIS_SUF_2(obj, to));
                    break;

                case 'l':
                    CHECK_NULL(obj, OSHR(obj));
                    break;

                case 'j':
                    CHECK_NULL(obj, OSSH(obj));
                    break;
                case 't':
                    CHECK_NULL(obj, GET_OBJ_VIS_SUF_3(obj, to));
                    break;
                case 'k':
                    CHECK_NULL(obj, OMHR(obj));
                    break;

                case 'e':
                    i = HSSH(ch, to);
                    break;
                case 'E':
                    if (vict_obj)
                        i = HSSH((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, OSSH(obj));
                    break;

                case 'o':
                    if (*(orig + 1) < '0' || *(orig + 1) > '5') {
                        CHECK_NULL(obj, OBJN(obj, to, 0));
                    } else {
                        padis = *(++orig) - '0';
                        CHECK_NULL(obj, OBJN(obj, to, padis > 5 ? 0 : padis));
                    }
                    break;
                case 'O':
                    if (*(orig + 1) < '0' || *(orig + 1) > '5') {
                        CHECK_NULL(vict_obj, OBJN((const struct obj_data *) vict_obj, to, 0));
                    } else {
                        padis = *(++orig) - '0';
                        CHECK_NULL(vict_obj,
                                   OBJN((const struct obj_data *) vict_obj, to,
                                        padis > 5 ? 0 : padis));
                    }
                    break;

                case 'p':
                    CHECK_NULL(obj, OBJS(obj, to));
                    break;
                case 'P':
                    CHECK_NULL(vict_obj, OBJS((const struct obj_data *) vict_obj, to));
                    break;

                case 'T':
                    CHECK_NULL(vict_obj, (const char *) vict_obj);
                    break;

                case 'F':
                    CHECK_NULL(vict_obj, fname((const char *) vict_obj));
                    break;

                case '$':
                    i = "$";
                    break;

                case 'a':
                    i = GET_CH_VIS_SUF_6(ch, to);
                    break;
                case 'A':
                    if (vict_obj)
                        i = GET_CH_VIS_SUF_6((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, GET_OBJ_VIS_SUF_6(obj, to));
                    break;

                case 'z':
                    i = GET_CH_VIS_SUF_7(ch, to);
                    break;
                case 'Z':
                    if (vict_obj)
                        i = GET_CH_VIS_SUF_7((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, GET_OBJ_VIS_SUF_7(obj, to));
                    break;

                case 'b':
                    i = GET_CH_VIS_SUF_10(ch, to);
                    break;
                case 'B':
                    if (vict_obj)
                        i = GET_CH_VIS_SUF_10((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, GET_OBJ_VIS_SUF_7(obj, to));
                    break;

                case 'x':
                    i = GET_CH_VIS_SUF_8(ch, to);
                    break;
                case 'X':
                    if (vict_obj)
                        i = GET_CH_VIS_SUF_8((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, GET_OBJ_VIS_SUF_8(obj, to));
                    break;

                case 'v':
                    i = GET_CH_VIS_SUF_9(ch, to);
                    break;
                case 'V':
                    if (vict_obj)
                        i = GET_CH_VIS_SUF_9((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, GET_OBJ_VIS_SUF_9(obj, to));
                    break;

                case 'g':
                    i = GET_CH_VIS_SUF_1(ch, to);
                    break;
                case 'f':
                    CHECK_NULL(obj, GET_OBJ_VIS_SUF_1(obj, to));
                    break;
                case 'G':
                    if (vict_obj)
                        i = GET_CH_VIS_SUF_1((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, GET_OBJ_VIS_SUF_1(obj, to));
                    break;

                case 'y':
                    i = GET_CH_VIS_SUF_5(ch, to);
                    break;
                case 'Y':
                    if (vict_obj)
                        i = GET_CH_VIS_SUF_5((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, GET_OBJ_VIS_SUF_5(obj, to));
                    break;

                case 'u':
                    i = GET_CH_VIS_SUF_2(ch, to);
                    break;
                case 'U':
                    if (vict_obj)
                        i = GET_CH_VIS_SUF_2((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, GET_OBJ_VIS_SUF_2(obj, to));
                    break;

                case 'w':
                    i = GET_CH_VIS_SUF_3(ch, to);
                    break;
                case 'W':
                    if (vict_obj)
                        i = GET_CH_VIS_SUF_3((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, GET_OBJ_VIS_SUF_3(obj, to));
                    break;

                case 'q':
                    i = GET_CH_VIS_SUF_4(ch, to);
                    break;
                case 'Q':
                    if (vict_obj)
                        i = GET_CH_VIS_SUF_4((const struct char_data *) vict_obj, to);
                    else
                        CHECK_NULL(obj, GET_OBJ_VIS_SUF_4(obj, to));
                    break;
                default:
                    log("SYSERR: Illegal $-code to act(): %c", *orig);
                    log("SYSERR: %s", orig);
                    i = "";
                    break;
            }
            while ((*buf = *(i++)))
                buf++;
            orig++;
        } else if (*orig == '\\') {
            if (*(orig + 1) == 'r') {
                *(buf++) = '\r';
                orig += 2;
            } else if (*(orig + 1) == 'n') {
                *(buf++) = '\n';
                orig += 2;
            } else
                *(buf++) = *(orig++);
        } else if (!(*(buf++) = *(orig++)))
            break;
    }

    *(--buf) = '\r';
    *(++buf) = '\n';
    *(++buf) = '\0';

    if (to->desc) {
        PHRASE(lbuf);
        SEND_TO_Q(lbuf, to->desc);
    }
}

/* moved this to utils.h --- mah */
#ifndef SENDOK
#define SENDOK(ch)      ((ch)->desc && (to_sleeping || AWAKE(ch)) && \
                         (IS_NPC(ch) ||!PLR_FLAGGED((ch), PLR_WRITING)))
#endif

void act(const char *str, int hide_invisible, struct char_data *ch,
         struct obj_data *obj, const void *vict_obj, int type)
{
    struct char_data *to;
    int to_sleeping, check_deaf, stopcount;

    if (!str || !*str)
        return;

    /*
     * Warning: the following TO_SLEEP code is a hack.
     *
     * I wanted to be able to tell act to deliver a message regardless of sleep
     * without adding an additional argument.  TO_SLEEP is 128 (a single bit
     * high up).  It's ONLY legal to combine TO_SLEEP with one other TO_x
     * command.  It's not legal to combine TO_x's with each other otherwise.
     * TO_SLEEP only works because its value "happens to be" a single bit;
     * do not change it to something else.  In short, it is a hack.
     */

    /* check if TO_SLEEP is there, and remove it if it is. */
    if ((to_sleeping = (type & TO_SLEEP)))
        type &= ~TO_SLEEP;
    if ((check_deaf = (type & CHECK_DEAF)))
        type &= ~CHECK_DEAF;

    if (type == TO_CHAR) {
        if (ch && SENDOK(ch) && IN_ROOM(ch) != NOWHERE)
            perform_act(str, ch, obj, vict_obj, ch);
        return;
    }

    if (type == TO_VICT) {
        if ((to = (struct char_data *) vict_obj) != NULL && SENDOK(to) && IN_ROOM(to) != NOWHERE)
            if ((hide_invisible && ch && CAN_SEE(ch, to)) || !hide_invisible)
                perform_act(str, ch, obj, vict_obj, to);
        return;
    }
    /* ASSUMPTION: at this point we know type must be TO_NOTVICT or TO_ROOM */
    /* or TO_ROOM_HIDE */


    if (ch && IS_SOUL(ch))
        return;

    if (ch && ch->in_room != NOWHERE)
        to = world[ch->in_room].people;
    else if (obj && obj->in_room != NOWHERE)
        to = world[obj->in_room].people;
    else {
        log("SYSERR: no valid target to act()! %s", str);
        return;
    }

    for (stopcount = 0; to && stopcount < 1000; to = to->next_in_room, stopcount++) {
        if (!SENDOK(to) || (to == ch))
            continue;
        if (hide_invisible && ch && !CAN_SEE(to, ch))
            continue;
        if ((type != TO_ROOM && type != TO_ROOM_HIDE) && to == vict_obj)
            continue;
//надо отдельно PRF_DEAF
        /*       if (!IS_NPC(to) && check_deaf && PRF_FLAGGED(to, PRF_NOTELL))
           continue; */
        if (type == TO_ROOM_HIDE && !AFF_FLAGGED(to, AFF_SENSE_LIFE))
            continue;
        perform_act(str, ch, obj, vict_obj, to);
    }
}

/*
 * This function is called every 30 seconds from heartbeat().  It checks
 * the four global buffers in CircleMUD to ensure that no one has written
 * past their bounds.  If our check digit is not there (and the position
 * doesn't have a NUL which may result from snprintf) then we gripe that
 * someone has overwritten our buffer.  This could cause a false positive
 * if someone uses the buffer as a non-terminated character array but that
 * is not likely. -gg
 */
void sanity_check(void)
{
    //int ok = TRUE;

    /*
     * If any line is false, 'ok' will become false also.
     */
    /*
       ok &= (test_magic(buf)  == MAGIC_NUMBER || test_magic(buf)  == '\0');
       ok &= (test_magic(buf1) == MAGIC_NUMBER || test_magic(buf1) == '\0');
       ok &= (test_magic(buf2) == MAGIC_NUMBER || test_magic(buf2) == '\0');
       ok &= (test_magic(buf3) == MAGIC_NUMBER || test_magic(buf3) == '\0');
       ok &= (test_magic(arg)  == MAGIC_NUMBER || test_magic(arg)  == '\0');
     */
    /*
     * This isn't exactly the safest thing to do (referencing known bad memory)
     * but we're doomed to crash eventually, might as well try to get something
     * useful before we go down. -gg
     * However, lets fix the problem so we don't spam the logs. -gg 11/24/98
     */
    /*
       if (!ok) {
       log("SYSERR: *** Buffer overflow! ***\n"
       "buf: %s\nbuf1: %s\nbuf2: %s\narg: %s", buf, buf1, buf2, arg);

       plant_magic(buf);
       plant_magic(buf1);
       plant_magic(buf2);
       plant_magic(buf3);
       plant_magic(arg);
       }

       #if 0
       log("Statistics: buf=%d buf1=%d buf2=%d arg=%d",
       strlen(buf), strlen(buf1), strlen(buf2), strlen(arg));
       #endif
     */
}


#if defined(HAVE_ZLIB)

/* Compression stuff. */

void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
    return calloc(items, size);
}

void zlib_free(void *opaque, void *address)
{
    free(address);
}

#endif
//BY HMEPAS FOR MULTISNOOPING ->>>>>>>>>>>>>>>>>>>>>>>>>>>

void test4snooper(struct descriptor_data *d1)
{
    int i;

    for (i = 0; i < d1->snoop_by_col; i++)
        log("#: %s", d1->snoop_by[i]->character->player.name);

}

void del_snooper(struct char_data *ch)
{
    struct descriptor_data **newd, *vict;
    int i, i1, flag = 0, col;

    vict = ch->desc->snooping;
    col = vict->snoop_by_col;

    if (col * sizeof(struct descriptor_data) <= 0) {
        log("SYSERR: Zero bytes or less requested at %s:%d.", __FILE__, __LINE__);
        return;
    }

    newd = (struct descriptor_data **) calloc(col, sizeof(struct descriptor_data *));
    if (!newd) {
        syserr("SYSERR: malloc failure");
        return;
    }

    for (i = 0, i1 = 0; i < col; i++)
        if (ch->desc == vict->snoop_by[i]) {
            flag = 1;
        } else {
            newd[i1] = vict->snoop_by[i];
            i1++;
        }
    free(vict->snoop_by);
    if (flag) {
        vict->snoop_by_col--;
        vict->snoop_by = (struct descriptor_data **) calloc(col, sizeof(struct descriptor_data *));
        if (!vict->snoop_by) {
            syserr("SYSERR: malloc failure");
            return;
        }
        for (i = 0; i < vict->snoop_by_col; i++)
            vict->snoop_by[i] = newd[i];
    } else
        log("ERROR: snooper was not deleted in del_snooper function");

    free(newd);
}

void add_snooper(struct char_data *ch)
{
    char buf[MAX_STRING_LENGTH];
    int i = 0;
    struct descriptor_data *victim = ch->desc->snooping;
    struct descriptor_data **newd;


    if (victim->snoop_by_col * sizeof(struct descriptor_data) < 0) {
        log("SYSERR: Zero bytes or less requested at %s:%d.", __FILE__, __LINE__);
        return;
    }

    if (!
        (newd =
         (struct descriptor_data **) malloc(sizeof(struct descriptor_data *) *
                                            (victim->snoop_by_col)))) {
        syserr("SYSERR: realloc failure 1");
        return;
    }

    if (victim->snoop_by_col > 0)
        for (i = 0; i < victim->snoop_by_col; i++)
            newd[i] = victim->snoop_by[i];

    free(victim->snoop_by);
    victim->snoop_by_col++;

    if (!
        (victim->snoop_by =
         (struct descriptor_data **) malloc(sizeof(struct descriptor_data *) *
                                            (victim->snoop_by_col)))) {
        syserr("SYSERR: realloc failure 2");
        return;
    }


    if (victim->snoop_by_col > 1)
        for (i = 0; i < victim->snoop_by_col - 1; i++)
            victim->snoop_by[i] = newd[i];

    victim->snoop_by[victim->snoop_by_col - 1] = ch->desc;

    sprintf(buf, "%s начал%s шпионить %s", GET_NAME(ch), GET_CH_SUF_1(ch),
            GET_PAD(victim->character, 3));
    mudlog(buf, CMP, LVL_IMPL, TRUE);

    free(newd);
}

#if defined(HAVE_ZLIB)

int mccp_start(struct descriptor_data *t, int ver)
{
    int derr;

    if (t->deflate)
        return 1;               // компрессия уже включена

    /* Set up zlib structures. */
    CREATE(t->deflate, z_stream, 1);
    t->deflate->zalloc = zlib_alloc;
    t->deflate->zfree = zlib_free;
    t->deflate->opaque = NULL;
    t->deflate->next_in = (Bytef *) t->small_outbuf;
    t->deflate->next_out = (Bytef *) t->small_outbuf;
    t->deflate->avail_out = SMALL_BUFSIZE;
    t->deflate->avail_in = 0;

    /* Initialize. */
    if ((derr = deflateInit(t->deflate, Z_DEFAULT_COMPRESSION)) != 0) {
        log("SYSERR: deflateInit returned %d.", derr);
        free(t->deflate);
        t->deflate = NULL;
        return 0;
    }

    if (ver != 2)
        write_to_descriptor(t->descriptor, compress_start_v1, strlen(compress_start_v1)
            );
    else
        write_to_descriptor(t->descriptor, compress_start_v2, strlen(compress_start_v2)
            );

    t->mccp_version = ver;
    return 1;
}


int mccp_end(struct descriptor_data *t, int ver)
{
    int derr;
    int prevsize, pending;
    unsigned char tmp[1];

    if (t->deflate == NULL)
        return 1;

    if (t->mccp_version != ver)
        return 0;

    t->deflate->avail_in = 0;
    t->deflate->next_in = tmp;
    prevsize = SMALL_BUFSIZE - t->deflate->avail_out;

    log("SYSERR: about to deflate Z_FINISH.");

    if ((derr = deflate(t->deflate, Z_FINISH)) != Z_STREAM_END) {
        log("SYSERR: deflate returned %d upon Z_FINISH. (in: %d, out: %d)", derr,
            t->deflate->avail_in, t->deflate->avail_out);
        return 0;
    }

    pending = SMALL_BUFSIZE - t->deflate->avail_out - prevsize;

    if (!write_to_descriptor(t->descriptor, t->small_outbuf + prevsize, pending)
        )
        return 0;

    if ((derr = deflateEnd(t->deflate)) != Z_OK)
        log("SYSERR: deflateEnd returned %d. (in: %d, out: %d)", derr, t->deflate->avail_in,
            t->deflate->avail_out);

    free(t->deflate);
    t->deflate = NULL;

    return 1;
}
#endif

int toggle_compression(struct descriptor_data *t)
{
#if defined(HAVE_ZLIB)
    if (t->mccp_version == 0)
        return 0;
    if (t->deflate == NULL) {
        return mccp_start(t, t->mccp_version) ? 1 : 0;
    } else {
        return mccp_end(t, t->mccp_version) ? 0 : 1;
    }
#endif
    return 0;
}

/* Reload players after a copyover */
void copyover_recover()
{
    struct descriptor_data *d;
    FILE *fp;
    char host[1024];
    int desc, player_i, key, vnum_room, mccp;
    bool fOld;
    char name[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

    log("Copyover recovery initiated");

    fp = fopen(COPYOVER_FILE, "r");

    if (!fp) {                  /* there are some descriptors open which will hang forever then ? */
        syserr("copyover_recover:fopen");
        log("Файл перезагрузки не найден. Холодная перезагрузка.\n\r");
        exit(1);
    }

    unlink(COPYOVER_FILE);      /* In case something crashes - doesn't prevent reading */

    for (;;) {
        fOld = TRUE;
        fscanf(fp, "%d %s %s %d %d %d\n", &desc, name, host, &key, &vnum_room, &mccp);
        if (desc == -1)
            break;

        /* Write something, and check if it goes error-free */
        if (write_to_descriptor(desc, "\n\r--------------------------------------------------\r\n")
            < 0) {
            log(buf, "%s don't restore connect for copyover. connect lost.", name);
            close(desc);        /* nope */
            continue;
        }

        /* create a new descriptor */
        CREATE(d, struct descriptor_data, 1);

        memset((char *) d, 0, sizeof(struct descriptor_data));
        init_descriptor(d, desc, key);  /* set up various stuff */

        strcpy(d->host, host);
        d->next = descriptor_list;
        descriptor_list = d;

        d->connected = CON_CLOSE;

        /* Now, find the pfile */

        d->character = new Player();
        d->character->desc = d;

        if ((player_i = load_char(name, d->character)) >= 0) {
            GET_PFILEPOS(d->character) = player_i;
            if (!PLR_FLAGGED(d->character, PLR_DELETED)) {
                REMOVE_BIT(PLR_FLAGS(d->character, PLR_WRITING), PLR_WRITING);
                REMOVE_BIT(PLR_FLAGS(d->character, PLR_MAILING), PLR_MAILING);
                REMOVE_BIT(PLR_FLAGS(d->character, PLR_SCRIPTING), PLR_SCRIPTING);
                REMOVE_BIT(PLR_FLAGS(d->character, PLR_CRYO), PLR_CRYO);
            } else
                fOld = FALSE;
        } else
            fOld = FALSE;

        if (!fOld) {            /* Player file not found?! */
            //write_to_descriptor (desc, "\n\rСИСТЕМА: Извините, Вам персонаж потерялся.\n\r");
            close_socket(d, TRUE);
        } else {                /* ok! */
            //write_to_descriptor (desc, "\n\rСИСТЕМА: Инициализация закончена.\n\r");
#if defined(HAVE_ZLIB)
            if (mccp)
                mccp_start(d, mccp);
#endif
            //toggle_compression(d);
            do_entergame_(d, vnum_room);
            d->connected = CON_PLAYING;
            affect_total(d->character);
            check_light(d->character, LIGHT_NO, LIGHT_NO, LIGHT_NO, LIGHT_NO, 1);
            look_at_room(d->character, FALSE);
        }

    }

    mud->getStats()->update();
    fclose(fp);
}

void go_copyover(void)
{
    FILE *fp;
    struct descriptor_data *d, *d_next;
    char arg_C[100], arg_p[100], argv0[MAX_STRING_LENGTH], arg_conf[MAX_STRING_LENGTH];
    char bufz[256];
    int mccp;

    log("COPYOVER");
    mud->getStats()->setRebootLevel(3);
    mud->getStats()->save();

    fp = fopen(COPYOVER_FILE, "w");
    if (!fp) {
        log("Copyover file not writeable, aborted.");
        return;
    }

    sprintf(bufz, "\n\rSYSTEM: Copyover reboot, please stand by few minutes.");

    /* For each playing descriptor, save its state */
    for (d = descriptor_list; d; d = d_next) {
#if defined(HAVE_ZLIB)
        mccp = d->deflate == NULL ? 0 : d->mccp_version;
#else
        mccp = 0;
#endif
        toggle_compression(d);
        struct char_data *och = d->character;

        /* We delete from the list , so need to save this */
        d_next = d->next;

        /* drop those logging on */
        if (!d->character || d->connected > CON_PLAYING) {
            write_to_descriptor(d->descriptor,
                                "\n\rSYSTEM: Copyover reboot, please stand by and reconnect.\n\r");
            close_socket(d, TRUE);      /* throw'em out */
        } else {
            fprintf(fp, "%d %s %s %d %d %d\n", d->descriptor, GET_NAME(och), d->host, d->keytable,
                    world[IN_ROOM(och)].number, mccp);
            /* save och */
            save_char(och, NOWHERE);
            xsave_rent(och, RENT_REBOOT, TRUE);
            save_pets(och);
            save_vars(och);
            save_quests(och);
            SEND_TO_SOCKET(bufz, d->descriptor);
        }
    }

    fprintf(fp, "-1\n");
    fclose(fp);

    /* Close reserve and other always-open files and release other resources */

    /* exec - descriptors are inherited */
    strcpy(argv0, mud->execFileName.c_str());
    sprintf(arg_p, "-p %d", mud->port.getValue());
    sprintf(arg_C, "-C %d", mother_desc);
    strcpy(arg_conf, mud->getConfigFile().getCPath());

    log("copyover cmdline: [%s] %s %s %s %s", argv0, argv0, arg_C, arg_p, arg_conf);
    //execl (EXE_FILE, "psmud.exe", buf2, buf, (char *) NULL);
    execl(argv0, argv0, arg_C, arg_p, arg_conf, (char *) NULL);

    /* Failed - sucessful exec will not return */
    syserr("do_copyover: execl");
    log("Copyover FAILED!");

    exit(1);                    /* too much trouble to try to recover! */
}


void warn_shutdown()
{
    extern int lastmessage;
    int wait;
    char buf[MAX_STRING_LENGTH];

    if (!circle_shutdown)
        return;
    if (!shutdown_time || time(NULL) >= shutdown_time)
        return;
    if (lastmessage == shutdown_time || lastmessage == time(NULL))
        return;

    wait = shutdown_time - time(NULL);
    if ((wait >= 1 && wait <= 5) || wait == 10 || wait == 30 || wait == 60
        || wait == 120 || wait % 300 == 0) {
        if (circle_reboot)
            sprintf(buf, "&WСИСТЕМА: ПЕРЕЗАГРУЗКА через ");
        else
            sprintf(buf, "&WСИСТЕМА: ОСТАНОВКА через ");

        if (wait < 60)
            sprintf(buf + strlen(buf), "%d %s.&n\r\n", wait, desc_count(wait, WHAT_SEC));
        else
            sprintf(buf + strlen(buf), "%d %s.&n\r\n", wait / 60, desc_count(wait / 60, WHAT_MINu));
        send_to_all(buf);
        lastmessage = time(NULL);
    }
}

void warn_copyover()
{
    extern int lastmessage_c;
    int wait;
    char buf[MAX_STRING_LENGTH];

    if (!circle_copyover)
        return;
    if (!copyover_time || time(NULL) >= copyover_time)
        return;
    if (lastmessage_c == copyover_time || lastmessage_c == time(NULL))
        return;
    wait = copyover_time - time(NULL);
    if ((wait >= 1 && wait <= 5) || wait == 10 || wait == 30 || wait == 60
        || wait == 120 || wait % 300 == 0) {
        sprintf(buf, "&WСИСТЕМА: ГОРЯЧАЯ ПЕРЕЗАГРУЗКА через ");

        if (wait < 60)
            sprintf(buf + strlen(buf), "%d %s.&n\r\n", wait, desc_count(wait, WHAT_SEC));
        else
            sprintf(buf + strlen(buf), "%d %s.&n\r\n", wait / 60, desc_count(wait / 60, WHAT_MINu));
        send_to_all(buf);
        lastmessage_c = time(NULL);
    }
}

#ifdef HAVE_ICONV
// UTF-8 coders by prool
void koi_to_utf8(char *str_i, char *str_o)
{
	iconv_t cd;
	size_t len_i, len_o = MAX_SOCK_BUF * 6;
	size_t i;
	char *str_i_orig=str_i;
	char *str_o_orig=str_o;

	if ((cd = iconv_open("UTF-8","KOI8-RU")) == (iconv_t) - 1)
	{
		printf("koi_to_utf8: iconv_open error\n");
		*str_o = 0;
		return;
	}
	len_i = strlen(str_i);
	if ((i = iconv(cd, &str_i, &len_i, &str_o, &len_o)) == (size_t) - 1)
	{
		printf("koi_to_utf8: iconv error ");
		printf("'%s' [", str_i_orig);
		while (*str_i_orig) printf("%02X ", *str_i_orig++);
		printf("] -> '%s' [", str_o_orig);
		while (*str_o_orig) printf("%02X ", *str_o_orig++);
		printf("]\n");

		*str_o = 0;
		return;
	}
	*str_o = 0;
	if (iconv_close(cd) == -1)
	{
		printf("koi_to_utf8: iconv_close error\n");
		return;
	}
}

void utf8_to_koi(char *str_i, char *str_o)
{
	iconv_t cd;
	size_t len_i, len_o = MAX_SOCK_BUF * 6;
	size_t i;
	char *str_i_orig=str_i;
	char *str_o_orig=str_o;

	if ((cd = iconv_open("KOI8-RU", "UTF-8")) == (iconv_t) - 1)
	{
		printf("utf8_to_koi: iconv_open error\n");
		return;
	}
	len_i = strlen(str_i);
	if ((i=iconv(cd, &str_i, &len_i, &str_o, &len_o)) == (size_t) - 1)
	{
		printf("utf8_to_koi: iconv error ");
		printf("'%s' [", str_i_orig);
		while (*str_i_orig) printf("%02X ", *str_i_orig++);
		printf("] -> '%s' [", str_o_orig);
		while (*str_o_orig) printf("%02X ", *str_o_orig++);
		printf("]\n");
		// return;
	}
	*str_o = 0;
	if (iconv_close(cd) == -1)
	{
		printf("utf8_to_koi: iconv_close error\n");
		return;
	}
}
#endif // HAVE_ICONV

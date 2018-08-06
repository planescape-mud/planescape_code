/* ************************************************************************
*   File: graph.c                                       Part of CircleMUD *
*  Usage: various graph algorithms                                        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "sysdep.h"

#include "structs.h"
#include "utils.h"
#include "comm.h"
#include "interpreter.h"
#include "constants.h"
#include "handler.h"
#include "db.h"
#include "spells.h"
#include "ai.h"

/* Externals */
ACMD(do_say);

/* local functions */
void bfs_enqueue(room_rnum room, int dir);
void bfs_dequeue(void);
void bfs_clear_queue(void);

struct bfs_queue_struct {
    room_rnum room;
    char dir;
    struct bfs_queue_struct *next;
};

static struct bfs_queue_struct *queue_head = 0, *queue_tail = 0;

/* Utility macros */
#define MARK(room) (SET_BIT(ROOM_FLAGS(room, ROOM_BFS_MARK), ROOM_BFS_MARK))
#define UNMARK(room) (REMOVE_BIT(ROOM_FLAGS(room, ROOM_BFS_MARK), ROOM_BFS_MARK))
#define IS_MARKED(room) (ROOM_FLAGGED(room, ROOM_BFS_MARK))
#define TOROOM(x, y) (world[(x)].dir_option[(y)]->to_room)
#define IS_CLOSED(x, y) (DOOR_FLAGGED(world[(x)].dir_option[(y)], EXIT_CLOSED))

int VALID_EDGE(room_rnum x, int y)
{
    if (world[x].dir_option[y] == NULL || TOROOM(x, y) == NOWHERE)
        return 0;
    if (track_through_doors == FALSE && IS_CLOSED(x, y))
        return 0;
    if (ROOM_FLAGGED(TOROOM(x, y), ROOM_NOTRACK) || IS_MARKED(TOROOM(x, y)))
        return 0;

    return 1;
}


void bfs_enqueue(room_rnum room, int dir)
{
    struct bfs_queue_struct *curr;

    CREATE(curr, struct bfs_queue_struct, 1);

    curr->room = room;
    curr->dir = dir;
    curr->next = 0;

    if (queue_tail) {
        queue_tail->next = curr;
        queue_tail = curr;
    } else
        queue_head = queue_tail = curr;
}


void bfs_dequeue(void)
{
    struct bfs_queue_struct *curr;

    curr = queue_head;

    if (!(queue_head = queue_head->next))
        queue_tail = 0;

    if (curr)
        free(curr);
}


void bfs_clear_queue(void)
{
    while (queue_head)
        bfs_dequeue();
}


/*
 * find_first_step: given a source room and a target room, find the first
 * step on the shortest path from the source to the target.
 *
 * Intended usage: in mobile_activity, give a mob a dir to go if they're
 * tracking another mob or a PC.  Or, a 'track' skill for PCs.
 */
int find_first_step(room_rnum src, room_rnum target, struct char_data *ch)
{
    int curr_dir;
    room_rnum curr_room;

    if (src < 0 || src > top_of_world || target < 0 || target > top_of_world) {
        log("SYSERR: Illegal value %d or %d passed to find_first_step. (%s)", src, target,
            __FILE__);
        return (BFS_ERROR);
    }
    if (src == target)
        return (BFS_ALREADY_THERE);

    /* MOB track victim in other zone */
    if (ch != NULL && IS_NPC(ch) && MOB_FLAGGED(ch, MOB_STAY_ZONE) &&
        world[src].zone != world[target].zone)
        return (BFS_ERROR);

    /* clear marks first, some OLC systems will save the mark. */
    for (curr_room = 0; curr_room <= top_of_world; curr_room++)
        UNMARK(curr_room);

    MARK(src);

    /* first, enqueue the first steps, saving which direction we're going. */
    for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
        if (VALID_EDGE(src, curr_dir)) {
            MARK(TOROOM(src, curr_dir));
            bfs_enqueue(TOROOM(src, curr_dir), curr_dir);
        }

    /* now, do the classic BFS. */
    while (queue_head) {
        if (queue_head->room == target) {
            curr_dir = queue_head->dir;
            bfs_clear_queue();
            return (curr_dir);
        } else {
            for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
                if (VALID_EDGE(queue_head->room, curr_dir)) {
                    MARK(TOROOM(queue_head->room, curr_dir));
                    bfs_enqueue(TOROOM(queue_head->room, curr_dir), queue_head->dir);
                }
            bfs_dequeue();
        }
    }

    return (BFS_NO_PATH);
}


/********************************************************
* Functions and Commands which use the above functions. *
********************************************************/
int go_track(struct char_data *ch, struct char_data *victim, int skill_no)
{

    if (AFF_FLAGGED(victim, AFF_NOTRACK)) {
        return BFS_ERROR;
    }


    return find_first_step(ch->in_room, victim->in_room, ch);

    /* 101 is a complete failure, no matter what the proficiency.
       percent = number(1,100);


       They passed the skill check. */

}

const char *track_when[] = { "совсем свежие",
    "свежие",
    "менее полудневной давности",
    "примерно полудневной давности",
    "почти дневной давности",
    "примерно дневной давности",
    "совсем старые"
};

int age_track(struct char_data *ch, int time, int calc_track)
{
    int when = 0;

// if (calc_track >= number(1,50))
    {
        if (time & 0x03)        /* 2 */
            when = 0;
        else if (time & 0x1F)   /* 5 */
            when = 1;
        else if (time & 0x3FF)  /* 10 */
            when = 2;
        else if (time & 0x7FFF) /* 15 */
            when = 3;
        else if (time & 0xFFFFF)        /* 20 */
            when = 4;
        else if (time & 0x3FFFFFF)      /* 26 */
            when = 5;
        else
            when = 6;
    }
// else
//    when = number(0,6);
    return (when);
}

int check_trackon(struct char_data *ch, int room)
{
    struct track_data *track;
    long id;

    id = get_id_by_name(GET_NAME(ch));

    /* found victim */
    for (track = world[room].track; track; track = track->next)
        if (track->who == id)
            return (TRUE);

    return (FALSE);
}

void _do_track(struct char_data *ch, char *name)
{
    int c, calc_track = 101;
    struct track_data *track;
    struct char_data *vict = NULL;
    int found = FALSE;
    char buf[MAX_STRING_LENGTH];
    char tname[MAX_STRING_LENGTH];

    if (FIGHTING(ch))
        return;

    *buf = '\0';
    *tname = '\0';

    if (--HUNT_STEP(ch) <= 0) {
        send_to_charf(ch, "Вы потеряли след!\r\n");
        *HUNT_NAME(ch) = '\0';
        HUNT_STEP(ch) = 0;
        return;
    }

    if ((vict = get_char_vis(ch, name, FIND_CHAR_ROOM))) {
        act("Вы нашли $N3!", FALSE, ch, 0, vict, TO_CHAR);
        *HUNT_NAME(ch) = '\0';
        HUNT_STEP(ch) = 0;
        return;
    }

    act("$n нагнул$u и внимательно осмотрел$q землю.", TRUE, ch, 0, 0, TO_ROOM);
    //act("Похоже $n ищет чьи-то следы.",TRUE, ch, 0, 0 , TO_ROOM);

    for (track = world[IN_ROOM(ch)].track; track; track = track->next) {
        *tname = '\0';
        if (IS_SET(track->track_info, TRACK_NPC))
            strcpy(tname, GET_NAME(mob_proto + track->who));
        else
            for (c = 0; c <= top_of_p_table; c++)
                if (player_table[c].id == track->who) {
                    strcpy(tname, player_table[c].name);
                    break;
                }
        if (*tname && isname(name, tname))
            break;
        else
            *tname = '\0';
    }

    if (!*tname || ROOM_FLAGGED(IN_ROOM(ch), ROOM_NOTRACK)) {
        send_to_char("Вы потеряли след!\r\n", ch);
        *HUNT_NAME(ch) = '\0';
        HUNT_STEP(ch) = 0;
        return;
    }

    send_to_charf(ch, "%s:\r\n", CAP(name));
    for (c = 0; c < NUM_OF_DIRS; c++) {
        if ((track && track->time_income[c] && calc_track >= dice(1, 100)) ||
            (!track && calc_track < dice(1, 100))
            ) {
            found = TRUE;
            sprintf(buf + strlen(buf), "- %s следы ведут %s\r\n",
                    track_when[age_track
                               (ch, track ? track->time_income[c] : (1 << number(0, 25)),
                                calc_track)], DirsFrom[Reverse[c]]);
        }
        if ((track && track->time_outgone[c] && calc_track >= number(0, 100)) ||
            (!track && calc_track < dice(1, 100))
            ) {
            found = TRUE;
            SET_BIT(ch->track_dirs, 1 << c);
            sprintf(buf + strlen(buf), "- %s следы ведут %s\r\n",
                    track_when[age_track
                               (ch, track ? track->time_outgone[c] : (1 << number(0, 25)),
                                calc_track)], DirsTo[c]);
        }
    }

    send_to_char(buf, ch);
}

ACMD(do_track)
{
    struct char_data *vict = NULL;
    struct track_data *track;
    int found = FALSE, c, calc_track = 0, track_t, i;
    char name[MAX_INPUT_LENGTH];
    char arg[MAX_STRING_LENGTH];

    if (!GET_SKILL(ch, SKILL_TRACK)) {
        send_to_char("Вы не знаете как.\r\n", ch);
        return;
    }

    *name = '\0';

    one_argument(argument, arg);

    if (!*arg) {
        for (track = world[IN_ROOM(ch)].track; track; track = track->next) {
            *name = '\0';

            if (IS_SET(track->track_info, TRACK_NPC))
                strcpy(name, GET_NAME(mob_proto + track->who));
            else
                strcpy(name, get_name_by_id(track->who));

            if (*name) {
                for (track_t = i = 0; i < NUM_OF_DIRS; i++) {
                    track_t |= track->time_outgone[i];
                    track_t |= track->time_income[i];
                }
                send_to_charf(ch, "%s: следы %s.\r\n", CAP(name),
                              track_when[age_track(ch, track_t, calc_track)]);
                found = TRUE;
            }
        }

        if (!found)
            send_to_char("Вы не нашли ничьих следов.\r\n", ch);
        return;
    }

    if ((vict = get_char_vis(ch, arg, FIND_CHAR_ROOM))) {
        if (vict == ch) {
            act("Вы нашли себя.", FALSE, ch, 0, vict, TO_CHAR);
            return;
        }
        act("Вы в одной комнате с $N4!", FALSE, ch, 0, vict, TO_CHAR);
        return;
    }

    /* found victim */
    for (track = world[IN_ROOM(ch)].track; track; track = track->next) {
        *name = '\0';
        if (IS_SET(track->track_info, TRACK_NPC))
            strcpy(name, GET_NAME(mob_proto + track->who));
        else
            for (c = 0; c <= top_of_p_table; c++)
                if (player_table[c].id == track->who) {
                    strcpy(name, player_table[c].name);
                    break;
                }
        if (*name && isname(arg, name))
            break;
        else
            *name = '\0';
    }

    if (!*name || ROOM_FLAGGED(IN_ROOM(ch), ROOM_NOTRACK)) {
        send_to_char("Вы не видите похожих следов.\r\n", ch);
        return;
    }

    strcpy(HUNT_NAME(ch), name);
    HUNT_STEP(ch) = GET_SKILL(ch, SKILL_TRACK);

    _do_track(ch, HUNT_NAME(ch));

}


void hunt_victim(struct char_data *ch)
{
    int dir;
    byte found;
    struct char_data *tmp;
    char buf[MAX_STRING_LENGTH], sub_buf[MAX_STRING_LENGTH];

    if (!ch || !HUNTING(ch) || FIGHTING(ch))
        return;

    /* make sure the char still exists */
    for (found = FALSE, tmp = character_list; tmp && !found; tmp = tmp->next)
        if (HUNTING(ch) == tmp && IN_ROOM(tmp) != NOWHERE)
            found = TRUE;

    if (!found) {
        //strcat (str,"concatenated.");
        strcpy(sub_buf, "О, Боги!  Моя жертва ускользнула!!");
        do_say(ch, sub_buf, 0, 0, 0);
        HUNTING(ch) = NULL;
        return;
    }

    if ((dir = find_first_step(ch->in_room, HUNTING(ch)->in_room, ch)) < 0) {
        sprintf(buf, "Какая жалость, %s сбежал%s!",
                HMHR(HUNTING(ch), ch), GET_CH_SUF_1(HUNTING(ch)));
        do_say(ch, buf, 0, 0, 0);
        HUNTING(ch) = NULL;
    } else {
        perform_move(ch, dir, 1, FALSE);
        if (ch->in_room == HUNTING(ch)->in_room)
            attack_best(ch, HUNTING(ch));
    }
}

ACMD(do_hidetrack)
{
    struct track_data *track[NUM_OF_DIRS + 1], *temp;
    int percent, prob, i, croom, found = FALSE, dir, rdir;
    struct timed_type timed;

    if (IS_NPC(ch) || !GET_SKILL(ch, SKILL_HIDETRACK)) {
        send_to_char("Но Вы не знаете как.\r\n", ch);
        return;
    }

    croom = IN_ROOM(ch);

    for (dir = 0; dir < NUM_OF_DIRS; dir++) {
        track[dir] = NULL;
        rdir = Reverse[dir];
        if (EXITDATA(croom, dir) &&
            EXITDATA(EXITDATA(croom, dir)->to_room, rdir) &&
            EXITDATA(EXITDATA(croom, dir)->to_room, rdir)->to_room == croom) {
            for (temp = world[EXITDATA(croom, dir)->to_room].track; temp; temp = temp->next)
                if (!IS_SET(temp->track_info, TRACK_NPC) &&
                    GET_ID(ch) == temp->who &&
                    !IS_SET(temp->track_info, TRACK_HIDE) && IS_SET(temp->time_outgone[rdir], 3)
                    ) {
                    found = TRUE;
                    track[dir] = temp;
                    break;
                }
        }
    }

    track[NUM_OF_DIRS] = NULL;
    for (temp = world[IN_ROOM(ch)].track; temp; temp = temp->next)
        if (!IS_SET(temp->track_info, TRACK_NPC) &&
            GET_ID(ch) == temp->who && !IS_SET(temp->track_info, TRACK_HIDE)
            ) {
            found = TRUE;
            track[NUM_OF_DIRS] = temp;
            break;
        }

    if (!found) {
        send_to_char("Вы не видите своих следов.\r\n", ch);
        return;
    }
    if (!check_moves(ch, HIDETRACK_MOVES))
        return;

    if (timed_by_skill(ch, SKILL_HIDETRACK)) {
        send_to_charf(ch, "Так часто нельзя заметать следы.\r\n");
        return;
    }

    percent = number(1, 100);

    std::vector < int >vit;
    std::vector < int >vot;

    vit.push_back(GET_REAL_DEX(ch));
    vit.push_back(GET_REAL_WIS(ch));
    vot.push_back(number(1, 40));
    prob = calc_like_skill(ch, NULL, SKILL_HIDETRACK, vit, vot);
    improove_skill(ch, NULL, 0, SKILL_HIDETRACK);

    timed.skill = SKILL_HIDETRACK;
    timed.time = 1 * SECS_PER_MUD_TICK;
    timed_to_char(ch, &timed);


    act("Вы попытались замести свои следы.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n попытал$u замести свои следы.", TRUE, ch, 0, 0, TO_ROOM);

    if (PRF_FLAGGED(ch, PRF_CODERINFO))
        send_to_charf(ch, "&KПримерение умения %d >= %d\r\n&n", prob, percent);

    if (percent <= prob) {      //send_to_char("Вы успешно замели свои следы.\r\n",ch);
        prob -= percent;
        for (i = 0; i <= NUM_OF_DIRS; i++)
            if (track[i]) {
                if (i < NUM_OF_DIRS)
                    track[i]->time_outgone[Reverse[i]] <<= MIN(31, prob);
                else
                    for (rdir = 0; rdir < NUM_OF_DIRS; rdir++) {
                        track[i]->time_income[rdir] <<= MIN(31, prob);
                        track[i]->time_outgone[rdir] <<= MIN(31, prob);
                    }
            }
    }

    for (i = 0; i <= NUM_OF_DIRS; i++)
        if (track[i])
            SET_BIT(track[i]->track_info, TRACK_HIDE);
}


//************************************************************************//
// Обнуление меток поиска
void ClearPathLabel(void)
{
    for (int i = 0; i <= top_of_world; i++)
        world[i].path_label = 0;
}

// Установить метку поиска
void SetPathLabel(int count)
{
    if (count >= 0 && count <= top_of_world)
        world[count].path_label = 0x01;
}

/*
 * XXX this arrays are not cleared between searches!
 */
int *PathMark = NULL;
int *PathPred = NULL;
int *PathPath = NULL;

int MakePath(int start_world, byte MobSpec, int &find_world, int max)
{
    if (find_world >= 0 && start_world == find_world)
        return 0;               // Пришли
    if (find_world < 0 && world[start_world].path_label & 0x01) {
        // В стартовой локации есть метка
        find_world = start_world;
        return 0;
    }
    // Подготовка памяти
    int r = start_world;
    int n = top_of_world + 1;   // Количество вершин

    if (PathMark == NULL)
        PathMark = new int[n];

    if (PathPred == NULL)
        PathPred = new int[n];

    if (PathPath == NULL)
        PathPath = new int[n];

    // Удаляем метки пути (метки поиска оставляем)
    for (int count = 0; count <= top_of_world; count++)
        world[count].path_label &= 0x01;

    world[r].path_label |= 0x02;        // Помечаем начальную вершину
    int l = 0;                  // Длина маршрута

    PathMark[0] = r;
    int i = 0;                  // Порядковый номер первой вершины в списке
    int k = 1;                  // Количество вершин в списке на расстоянии l
    int k1 = 0;                 // Количество вершин в списке на расстоянии l+1
    int k2 = 0;                 // Порядковый номер последней помеченой вершины в списке
    int i2, m, u, j, v;
    struct room_direction_data *dir;
    int wtype;
    int hour;

    while (1) {
        //log("Main loop. l=%d, i=%d, k=%d, k1=%d, k2=%d", l, i, k, k1, k2);
        if (k == 0)
            return -1;          // Путь не найден
        i2 = i + k - 1;         // Порядковый номер последней вершины в списке
        m = i;                  // Порядковый номер очередной из вершин в списке

        while (1) {
            u = PathMark[m];
            for (j = 0; j < NUM_OF_DIRS; j++) {
                dir = world[u].dir_option[j];
                if (dir == NULL)
                    continue;
                // Проверяем выходы
                while (dir->to_room >= 0) {
                    // Проверить тип выхода
                    if (DOOR_FLAGGED(dir, EXIT_LOCKED) || DOOR_FLAGGED(dir, EXIT_HIDDEN))
                        break;
                    v = dir->to_room;
                    if ((find_world < 0 && world[v].path_label & 0x01) || find_world == v) {
                        // Путь найден
                        find_world = v;
                        PathPred[v] = u;
                        PathPath[l + 1] = v;
                        for (int jj = l; jj >= 0; jj--)
                            PathPath[jj] = PathPred[PathPath[jj + 1]];
                        PathPath[l + 1] = v;
                        return l + 1;
                    }
                    if (world[v].path_label & 0x02)
                        break;  // Вершина помечена
                    wtype = world[v].sector_type;
                    if (wtype == SECT_SECRET ||
                        (wtype == SECT_WATER_SWIM && !(MobSpec & PATH_SWIM)) ||
                        (wtype == SECT_WATER_NOSWIM && !(MobSpec & PATH_WATER)) ||
                        (wtype == SECT_UNDERWATER && !(MobSpec & PATH_UNDERWATER)) ||
                        (wtype == SECT_FLYING && !(MobSpec & PATH_FLY))) {
                        world[v].path_label |= 0x02;
                        break;
                    }
                    // Помечаем вершину
                    world[v].path_label |= 0x02;
                    k1++;
                    PathPred[v] = u;
                    k2++;
                    PathMark[k2] = v;
                    break;
                }
                // Проверяем портал
                while (dir->type_port) {
                    // Есть портал
                    if (dir->open != 0 || dir->close != 0) {
                        // Портал временной
                        hour = zone_table[world[u].zone].time_info.hours;
                        if (dir->open == dir->close)
                            break;
                        if (dir->open < dir->close && (hour < dir->open || hour >= dir->close))
                            break;
                        if (dir->open > dir->close && (hour < dir->open && hour >= dir->close))
                            break;
                    }
                    // Портал открыт
                    v = dir->room_port;
                    if (v < 0)
                        break;
                    if ((find_world < 0 && world[v].path_label & 0x01) || find_world == v) {
                        // Путь найден
                        find_world = v;
                        PathPred[v] = u;
                        PathPath[l + 1] = v;
                        for (int jj = l; jj >= 0; jj--)
                            PathPath[jj] = PathPred[PathPath[jj + 1]];
                        PathPath[l + 1] = v;    // Конечная локация
                        return l + 1;
                    }
                    if (world[v].path_label & 0x02)
                        break;
                    wtype = world[v].sector_type;
                    if (wtype == SECT_SECRET ||
                        (wtype == SECT_WATER_SWIM && !(MobSpec & PATH_SWIM)) ||
                        (wtype == SECT_WATER_NOSWIM && !(MobSpec & PATH_WATER)) ||
                        (wtype == SECT_UNDERWATER && !(MobSpec & PATH_UNDERWATER)) ||
                        (wtype == SECT_FLYING && !(MobSpec & PATH_FLY))) {
                        world[v].path_label |= 0x02;
                        break;
                    }
                    // Помечаем вершину
                    world[v].path_label |= 0x02;
                    k1++;
                    PathPred[v] = u;
                    k2++;
                    PathMark[k2] = v;
                    break;
                }
            }
            m++;
            if (m > i2)
                break;
        }
        // Все вершины на расстоянии l просмотрены
        l++;
        i = i2 + 1;
        k = k1;
        k1 = 0;
        if (max > 0 && l >= max)
            return -1;          // Ограничение длины пути
    }
}

bool GetPathDirection(int num, CPathRetDir * ret)
{
    int src = PathPath[num];
    int dst = PathPath[num + 1];
    struct room_direction_data *dir;
    int hour;

    for (int j = 0; j < NUM_OF_DIRS; j++) {
        dir = world[src].dir_option[j];
        if (dir == NULL)
            continue;
        // Проверяем выходы
        while (dir->to_room >= 0) {
            // Проверить тип выхода
            if (DOOR_FLAGGED(dir, EXIT_LOCKED) || DOOR_FLAGGED(dir, EXIT_HIDDEN))
                //if(dir->type & EXIT_LOCKED || dir->type & EXIT_HIDDEN)
                break;
            if (dir->to_room == dst) {
                // Путь найден
                ret->Portal = false;
                ret->Direction = j;
                return true;
            }
            break;
        }
        // Проверяем портал
        while (dir->type_port) {
            // Есть портал
            if (dir->open != 0 || dir->close != 0) {
                // Портал временной
                hour = zone_table[world[src].zone].time_info.hours;
                if (dir->open == dir->close)
                    break;
                if (dir->open < dir->close && (hour < dir->open || hour >= dir->close))
                    break;
                if (dir->open > dir->close && (hour < dir->open && hour >= dir->close))
                    break;
            }
            // Портал открыт
            if (dst == dir->room_port) {
                // Путь найден
                ret->Portal = true;
                ret->Direction = j;
                return true;
            }
            break;
        }
    }
    return false;
}

int TracePath(int start_world, int &find_world, byte MobSpec, CPathRetDir * ret, int max = 0)
{
    // Строим полный путь
    int len = MakePath(start_world, MobSpec, find_world, max);

    if (len <= 0)
        return len;
    // Есть путь. Расчитываем направление из текущей локации
    if (!GetPathDirection(0, ret))
        return -1;
    return len;
}

void ShowLastPath(int len)
{
    if (len <= 0)
        return;
    CPathRetDir dir;

    for (int i = 0; i < len; i++) {
        if (!GetPathDirection(i, &dir)) {
            log("Ошибка обработки пути");
            return;
        }
        int v = PathPath[i];

        log("[%6d] (%s): %s %s", world[v].number, world[v].name, dir.Portal ? "ПОРТАЛ" : "ИДТИ",
            DirIs[dir.Direction]);
    }
    int v = PathPath[len];

    log("[%6d] (%s)", world[v].number, world[v].name);
}

void FindPath(void)
{
    log("*********************** START MAKE PATH *****************");
    int start = -1, fn = -1;

    //ClearPathLabel();
    for (int i = 0; i <= top_of_world; i++) {
        //if(world[i].number == 5100)
        if (world[i].number == 8900)
            //if(world[i].number == 10601) // Морг
            start = i;
        //if(world[i].number == 10601) // Морг
        //if(world[i].number == 5197) // Рядом с драконом
        //if(world[i].number == 14130) // Крепость (капитан)
        //if(world[i].number == 14147) // Крепость у ворот
        if (world[i].number == 14338)
            //if(world[i].number == 20055)
            //if(world[i].number == 14329)
            fn = i;
    }
    if (start < 0) {
        log("Source not found");
        return;
    }
    struct timeval tv_st, tv_fn;

#ifndef _MINGW_
    struct timezone tmz;

    gettimeofday(&tv_st, &tmz);
#else
    gettimeofday(&tv_st, NULL);
#endif

    CPathRetDir dir;
    int len = TracePath(start, fn, 0, &dir);

#ifndef _MINGW_
    gettimeofday(&tv_fn, &tmz);
#else
    gettimeofday(&tv_fn, NULL);
#endif

    ShowLastPath(len);
    log("len = %d", len);
    if (tv_fn.tv_usec < tv_st.tv_usec) {
        tv_fn.tv_usec += 1000000;
        tv_fn.tv_sec--;
    }
    log("Work time: %lu sec %lu micro sec", tv_fn.tv_sec - tv_st.tv_sec,
        tv_fn.tv_usec - tv_st.tv_usec);
    log("*** DONE ***");
}

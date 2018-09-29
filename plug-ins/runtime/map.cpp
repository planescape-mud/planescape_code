// $RCSfile$     $Date$     $Revision$
// Copyright (c) 2013 Krodo
// Part of Bylins http://www.mud.ru
// adaptation for planescape by bodrich

#include "map.hpp"

#include "sysdep.h"
#include "structs.h"
#include "utils.h"
#include "interpreter.h"
#include "spells.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "screen.h"
#include "pk.h"
#include "xspells.h"
#include "xenchant.h"
#include "xboot.h"
#include "sp_define.h"
#include "ai.h"


#include <map>
#include <sstream>
#include <iomanip>
#include <vector>

int shop_ext(struct char_data *ch, void *me, int cmd, char* argument);
int receptionist(struct char_data *ch, void *me, int cmd, char* argument);
int postmaster(struct char_data *ch, void *me, int cmd, char* argument);
int bank(struct char_data *ch, void *me, int cmd, char* argument);
int exchange(struct char_data *ch, void *me, int cmd, char* argument);
int horse_keeper(struct char_data *ch, void *me, int cmd, char* argument);
int guild_mono(struct char_data *ch, void *me, int cmd, char* argument);
int guild_poly(struct char_data *ch, void *me, int cmd, char* argument);
int torc(struct char_data *ch, void *me, int cmd, char* argument);
namespace Noob
{
int outfit(struct char_data *ch, void *me, int cmd, char* argument);
}
extern int has_boat(struct char_data *ch);

namespace MapSystem
{

// размер поля для отрисовка
const int MAX_LINES = 50;
const  int MAX_LENGHT = 100;
// глубина рекурсии по комнатам
const  int MAX_DEPTH_ROOMS = 10;

// поле для отрисовки
int screen[MAX_LINES][MAX_LENGHT];
int depths[MAX_LINES][MAX_LENGHT];
//screen(boost::extents[MAX_LINES][MAX_LENGHT]);
// копия поля для хранения глубины текущей отрисовки по нужным координатам
// используется для случаев наезжания комнат друг на друга, в этом случае
// ближняя затирает более дальнюю и все остальные после нее

//boost::multi_array<int, 2> depths(boost::extents[MAX_LINES][MAX_LENGHT]);

enum
{
	// свободный проход
	SCREEN_Y_OPEN,
	// закрытая дверь
	SCREEN_Y_DOOR,
	// скрытый проход (иммы)
	SCREEN_Y_HIDE,
	// нет прохода
	SCREEN_Y_WALL,
	SCREEN_X_OPEN,
	SCREEN_X_DOOR,
	SCREEN_X_HIDE,
	SCREEN_X_WALL,
	SCREEN_UP_OPEN,
	SCREEN_UP_DOOR,
	SCREEN_UP_HIDE,
	SCREEN_UP_WALL,
	SCREEN_DOWN_OPEN,
	SCREEN_DOWN_DOOR,
	SCREEN_DOWN_HIDE,
	SCREEN_DOWN_WALL,
	SCREEN_Y_UP_OPEN,
	SCREEN_Y_UP_DOOR,
	SCREEN_Y_UP_HIDE,
	SCREEN_Y_UP_WALL,
	SCREEN_Y_DOWN_OPEN,
	SCREEN_Y_DOWN_DOOR,
	SCREEN_Y_DOWN_HIDE,
	SCREEN_Y_DOWN_WALL,
	// текущая клетка персонажа
	SCREEN_CHAR,
	// переход в другую зону
	SCREEN_NEW_ZONE,
	// мирная комната
	SCREEN_PEACE,
	// дт (иммы + 0 морт)
	SCREEN_DEATH_TRAP,
	// чтобы пробелы не втыкать
	SCREEN_EMPTY,
	// в комнате темно, существ не пишем
	SCREEN_MOB_UNDEF,
	// кол-во существ в комнате
	SCREEN_MOB_1,
	SCREEN_MOB_2,
	SCREEN_MOB_3,
	SCREEN_MOB_4,
	SCREEN_MOB_5,
	SCREEN_MOB_6,
	SCREEN_MOB_7,
	SCREEN_MOB_8,
	SCREEN_MOB_9,
	// в комнате больше 9 существ
	SCREEN_MOB_OVERFLOW,
	// тоже с предметами
	SCREEN_OBJ_UNDEF,
	SCREEN_OBJ_1,
	SCREEN_OBJ_2,
	SCREEN_OBJ_3,
	SCREEN_OBJ_4,
	SCREEN_OBJ_5,
	SCREEN_OBJ_6,
	SCREEN_OBJ_7,
	SCREEN_OBJ_8,
	SCREEN_OBJ_9,
	SCREEN_OBJ_OVERFLOW,
	// мобы со спешиалами
	SCREEN_MOB_SPEC_SHOP,
	SCREEN_MOB_SPEC_RENT,
	SCREEN_MOB_SPEC_MAIL,
	SCREEN_MOB_SPEC_BANK,
	SCREEN_MOB_SPEC_HORSE,
	SCREEN_MOB_SPEC_TEACH,
	SCREEN_MOB_SPEC_EXCH,
	SCREEN_MOB_SPEC_TORC,
	// клетка в которой можно утонуть
	SCREEN_WATER,
	// можно умереть без полета
	SCREEN_FLYING,
	// кладовщики для показа нубам
	SCREEN_MOB_SPEC_OUTFIT,
	// SCREEN_WATER красным, если персонаж без дышки
	SCREEN_WATER_RED,
	// SCREEN_FLYING красным, если персонаж без полета
	SCREEN_FLYING_RED,
	// всегда в конце
	SCREEN_TOTAL
};

 char *signs[] =
{
	// SCREEN_Y
	"&K - &n",
	"&C-=-&n",
	"&R---&n",
	"&G---&n",
	// SCREEN_X
	"&K:&n",
	"&C/&n",
	"&R|&n",
	"&G|&n",
	// SCREEN_UP
	"&K^&n",
	"&C^&n",
	"&R^&n",
	"",
	// SCREEN_DOWN
	"&Kv&n",
	"&Cv&n",
	"&Rv&n",
	"",
	// SCREEN_Y_UP
	"&K -&n",
	"&C-=&n",
	"&R--&n",
	"&G--&n",
	// SCREEN_Y_DOWN
	"&K- &n",
	"&C=-&n",
	"&R--&n",
	"&G--&n",
	// OTHERS
	"&c@&n",
	"&C>&n",
	"&K~&n",
	"&RЖ&n",
	"",
	"&K?&n",
	"&r1&n",
	"&r2&n",
	"&r3&n",
	"&r4&n",
	"&r5&n",
	"&r6&n",
	"&r7&n",
	"&r8&n",
	"&r9&n",
	"&R!&n",
	"&K?&n",
	"&y1&n",
	"&y2&n",
	"&y3&n",
	"&y4&n",
	"&y5&n",
	"&y6&n",
	"&y7&n",
	"&y8&n",
	"&y9&n",
	"&Y!&n",
	"&W$&n",
	"&WR&n",
	"&WM&n",
	"&WB&n",
	"&WH&n",
	"&WT&n",
	"&WE&n",
	"&WG&n",
	"&C,&n",
	"&C`&n",
	"&WO&n",
	"&R,&n",
	"&R`&n"
};

std::map<int /* room vnum */, int /* min depth */> check_dupe;

// отрисовка символа на поле по координатам
void put_on_screen(int y, int x, int num, int depth)
{
	if (y >= MAX_LINES || x >= MAX_LENGHT)
	{
		log("SYSERROR: %d;%d (%s %s %d)", y, x, __FILE__, __func__, __LINE__);
		return;
	}
	if (depths[y][x] == -1)
	{
		// поле было чистое
		screen[y][x] = num;
		depths[y][x] = depth;
	}
	else if (depths[y][x] > depth)
	{
		// уже что-то было отрисовано
		if (screen[y][x] == num)
		{
			// если тот же самый символ,
			// то надо обновить глубину на случай последующих затираний
			depths[y][x] = depth;
		}
		else
		{
			// другой символ и меньшая глубина
			// затираем все символы этой и далее глубины
			// и поверх рисуем текущий символ
			 int hide_num = depths[y][x];
			for (int i = 0; i < MAX_LINES; ++i)
			{
				for (int k = 0; k < MAX_LENGHT; ++k)
				{
					if (depths[i][k] >= hide_num)
					{
						screen[i][k] = -1;
						depths[i][k] = -1;
					}
				}
			}
			screen[y][x] = num;
			depths[y][x] = depth;
		}
	}
	else if ((screen[y][x] >= SCREEN_UP_OPEN && screen[y][x] <= SCREEN_UP_WALL)
			|| (screen[y][x] >= SCREEN_DOWN_OPEN && screen[y][x] <= SCREEN_DOWN_WALL))
	{
		// выходы ^ и v затираются, если есть чем
		screen[y][x] = num;
		depths[y][x] = depth;
	}
}

// важные знаки в центре клеток, которые надо рисовать для выходов вверх/вниз
// затирают символы выходов ^ и v, но не затирают друг друга, т.е. что
// первое отрисовалось, то и остается, поэтому идут по важности
void check_position_and_put_on_screen(int next_y, int next_x, int sign_num, int depth, int exit_num)
{
	if (exit_num == UP)
	{
		switch(sign_num)
		{
		case SCREEN_DEATH_TRAP:
		case SCREEN_WATER:
		case SCREEN_FLYING:
		case SCREEN_WATER_RED:
		case SCREEN_FLYING_RED:
			put_on_screen(next_y - 1, next_x + 1, sign_num, depth);
			return;
		}
	}
	else if (exit_num == DOWN)
	{
		switch(sign_num)
		{
		case SCREEN_DEATH_TRAP:
		case SCREEN_WATER:
		case SCREEN_FLYING:
		case SCREEN_WATER_RED:
		case SCREEN_FLYING_RED:
			put_on_screen(next_y + 1, next_x - 1, sign_num, depth);
			return;
		}
	}
	else
	{
		put_on_screen(next_y, next_x, sign_num, depth);
	}
}

void draw_mobs( struct char_data *ch, int room_rnum, int next_y, int next_x)
{
	/*if (IS_DARK(room_rnum) && !IS_IMMORTAL(ch))
	{
		put_on_screen(next_y, next_x - 1, SCREEN_MOB_UNDEF, 1);
	}
	else
	{
		int cnt = 0;
		for ( struct char_data tch : world[room_rnum]->people)
		{
			if (tch == ch)
			{
				continue;
			}
			if (IS_NPC(tch))
			{
				continue;
			}
			if (!IS_NPC(tch))
			{
				continue;
			}
			if (HERE(tch)
				&& (CAN_SEE(ch, tch)
					|| awaking(tch, AW_HIDE | AW_INVIS | AW_CAMOUFLAGE)))
			{
				++cnt;
			}
		}

		if ((cnt > 0) && (cnt <= 9))
		{
			put_on_screen(next_y, next_x - 1, SCREEN_MOB_UNDEF + cnt, 1);
		}
		else if (cnt > 9)
		{
			put_on_screen(next_y, next_x - 1, SCREEN_MOB_OVERFLOW, 1);
		}
	}*/
}

void draw_objs( struct char_data *ch, int room_rnum, int next_y, int next_x)
{
	/*if (IS_DARK(room_rnum) && !IS_IMMORTAL(ch))
	{
		put_on_screen(next_y, next_x + 1, SCREEN_OBJ_UNDEF, 1);
	}
	else
	{
		int cnt = 0;

		/*for (OBJ_DATA *obj = world[room_rnum]->contents; obj; obj = obj->get_next_content())
		{
			if (CAN_SEE_OBJ(ch, obj))
			{
				++cnt;
			}
		}
		if (cnt > 0 && cnt <= 9)
		{
			put_on_screen(next_y, next_x + 1, SCREEN_OBJ_UNDEF + cnt, 1);
		}
		else if (cnt > 9)
		{
			put_on_screen(next_y, next_x + 1, SCREEN_OBJ_OVERFLOW, 1);
		}
	}*/
}

void drow_spec_mobs( struct char_data *ch, int room_rnum, int next_y, int next_x, int cur_depth)
{
	/*bool all = true;

	for ( struct char_data tch : world[room_rnum]->people)
	{
		auto func = GET_MOB_SPEC(tch);
		if (func)
		{
			if (func == shop_ext
				&& all)
			{
				put_on_screen(next_y, next_x, SCREEN_MOB_SPEC_SHOP, cur_depth);
			}
			else if (func == receptionist
				&& all)
			{
				put_on_screen(next_y, next_x, SCREEN_MOB_SPEC_RENT, cur_depth);
			}
			else if (func == postmaster
				&& (all))
			{
				put_on_screen(next_y, next_x, SCREEN_MOB_SPEC_MAIL, cur_depth);
			}
			else if (func == bank
				&& (all))
			{
				put_on_screen(next_y, next_x, SCREEN_MOB_SPEC_BANK, cur_depth);
			}
			else if (func == exchange
				&& all)
			{
				put_on_screen(next_y, next_x, SCREEN_MOB_SPEC_EXCH, cur_depth);
			}
			else if (func == horse_keeper
				&& all)
			{
				put_on_screen(next_y, next_x, SCREEN_MOB_SPEC_HORSE, cur_depth);
			}
			else if ((func == guild_mono || func == guild_poly)
				&& (all ))
			{
				put_on_screen(next_y, next_x, SCREEN_MOB_SPEC_TEACH, cur_depth);
			}
			else if (func == torc
				&& (all))
			{
				put_on_screen(next_y, next_x, SCREEN_MOB_SPEC_TORC, cur_depth);
			}			
		}
	}*/
}

bool mode_allow( struct char_data *ch, int cur_depth)
{
	return true;
}

void draw_room(struct char_data *ch,  room_data *room, int cur_depth, int y, int x)
{
	// чтобы не ходить по комнатам вторично, но с проверкой на глубину
	std::map<int, int>::iterator i = check_dupe.find(room->number);
	if (i != check_dupe.end())
	{
		if (i->second <= cur_depth)
		{
			return;
		}
		else
		{
			i->second = cur_depth;
		}
	}
	else
	{
		check_dupe.insert(std::make_pair(room->number, cur_depth));
	}

	if (&world[ch->in_room] == room)
	{
		put_on_screen(y, x, SCREEN_CHAR, cur_depth);
		draw_mobs(ch, ch->in_room, y, x);
		draw_objs(ch, ch->in_room, y, x);
	}
	else if (GET_FLAG(room->room_flags, ROOM_PEACEFUL))
	{
		put_on_screen(y, x, SCREEN_PEACE, cur_depth);
	}

	for (int i = 0; i < NUM_OF_DIRS; ++i)
	{
		int cur_y = y, cur_x = x, cur_sign = -1, next_y = y, next_x = x;
		switch(i)
		{
		case NORTH:
			cur_y -= 1;
			next_y -= 2;
			cur_sign = SCREEN_Y_OPEN;
			break;
		case EAST:
			cur_x += 2;
			next_x += 4;
			cur_sign = SCREEN_X_OPEN;
			break;
		case SOUTH:
			cur_y += 1;
			next_y += 2;
			cur_sign = SCREEN_Y_OPEN;
			break;
		case WEST:
			cur_x -= 2;
			next_x -= 4;
			cur_sign = SCREEN_X_OPEN;
			break;
		case UP:
			cur_y -= 1;
			cur_x += 1;
			cur_sign = SCREEN_UP_OPEN;
			break;
		case DOWN:
			cur_y += 1;
			cur_x -= 1;
			cur_sign = SCREEN_DOWN_OPEN;
			break;
		default:
			log("SYSERROR: i=%d (%s %s %d)", i, __FILE__, __func__, __LINE__);
			return;
		}

		if (room->dir_option[i]
			&& room->dir_option[i]->to_room != NOWHERE)
		{
			// отрисовка выхода
			if (DOOR_FLAGGED(room->dir_option[i], EXIT_CLOSED))
			{
				put_on_screen(cur_y, cur_x, cur_sign + 1, cur_depth);
			}
			else
			{
				put_on_screen(cur_y, cur_x, cur_sign, cur_depth);
			}
			// за двери закрытые смотрят только иммы
			if (DOOR_FLAGGED(room->dir_option[i], EXIT_CLOSED) && !IS_IMMORTAL(ch))
			{
				continue;
			}
			// здесь важна очередность, что первое отрисовалось - то и будет
			 room_data *next_room = &world[room->dir_option[i]->to_room];
			bool view_dt = true;

			// дт иммам и нубам с 0 мортов
			/*if (next_room->get_flag(ROOM_DEATH)
				&& (view_dt || IS_IMMORTAL(ch)))
			{
				check_position_and_put_on_screen(next_y, next_x, SCREEN_DEATH_TRAP, cur_depth, i);
			}
			// можно утонуть
			if (next_room->sector_type == SECT_WATER_NOSWIM)
			{
				if (!has_boat(ch))
				{
					check_position_and_put_on_screen(next_y, next_x, SCREEN_WATER_RED, cur_depth, i);
				}
				else
				{
					check_position_and_put_on_screen(next_y, next_x, SCREEN_WATER, cur_depth, i);
				}
			}
			// можно задохнуться
			if (next_room->sector_type == SECT_UNDERWATER)
			{
				if (!AFF_FLAGGED(ch, EAffectFlag::AFF_WATERBREATH))
				{
					check_position_and_put_on_screen(next_y, next_x, SCREEN_WATER_RED, cur_depth, i);
				}
				else
				{
					check_position_and_put_on_screen(next_y, next_x, SCREEN_WATER, cur_depth, i);
				}
			}
			// Флай-дт
			if (next_room->sector_type == SECT_FLYING)
			{
				if (!AFF_FLAGGED(ch, EAffectFlag::AFF_FLY))
				{
					check_position_and_put_on_screen(next_y, next_x, SCREEN_FLYING_RED, cur_depth, i);
				}
				else
				{
					check_position_and_put_on_screen(next_y, next_x, SCREEN_FLYING, cur_depth, i);
				}
			}
			
			// существа
			if (cur_depth == 1
				&& (!DOOR_FLAGGED(room->dir_option[i], EXIT_CLOSED) || IS_IMMORTAL(ch)))
			{
				// в случае вверх/вниз next_y/x = y/x, рисуется относительно
				// координат чара, со смещением, чтобы писать около полей v и ^
				// внутри draw_mobs происходит еще одно смещение на x-1
				if (cur_sign == SCREEN_UP_OPEN)
				{
					draw_mobs(ch, room->dir_option[i]->to_room, next_y - 1, next_x + 3);
				}
				else if (cur_sign == SCREEN_DOWN_OPEN)
				{
					draw_mobs(ch, room->dir_option[i]->to_room, next_y + 1, next_x - 1);
				}
				else
				{
					// остальные выходы вокруг пишутся как обычно, со смещением
					// относительно центра следующей за выходом клетки
					draw_mobs(ch, room->dir_option[i]->to_room, next_y, next_x);
				}
			}
			// предметы
			if (cur_depth == 1
				&& (!DOOR_FLAGGED(room->dir_option[i], EXIT_CLOSED) || IS_IMMORTAL(ch)))
			{
				if (cur_sign == SCREEN_UP_OPEN)
				{
					draw_objs(ch, room->dir_option[i]->to_room, next_y - 1, next_x);
				}
				else if (cur_sign == SCREEN_DOWN_OPEN)
				{
					draw_objs(ch, room->dir_option[i]->to_room, next_y + 1, next_x - 2);
				}
				else
				{
					draw_objs(ch, room->dir_option[i]->to_room, next_y, next_x);
				}
			}*/
            // знаки в центре клетки, не рисующиеся для выходов вверх/вниз
			if (i != UP && i != DOWN)
			{
				// переход в другую зону
				/*if (next_room->zone != world[ch->in_room]->zone)
				{
					put_on_screen(next_y, next_x, SCREEN_NEW_ZONE, cur_depth);
				}*/
				// моб со спешиалом
				drow_spec_mobs(ch, room->dir_option[i]->to_room, next_y, next_x, cur_depth);
			}
			// проход по следующей в глубину комнате
			if (i != UP && i != DOWN
				&& cur_depth < MAX_DEPTH_ROOMS
				&& (!DOOR_FLAGGED(room->dir_option[i], EXIT_CLOSED) || IS_IMMORTAL(ch))
				&& next_room->zone == world[ch->in_room].zone
				&& mode_allow(ch, cur_depth))
			{
				draw_room(ch, next_room, cur_depth + 1, next_y, next_x);
			}
		}
		else
		{
			put_on_screen(cur_y, cur_x, cur_sign + 3, cur_depth);
		}
	}
}

// imm по дефолту = 0, если нет, то распечатанная карта засылается ему
void print_map(struct char_data *ch, struct char_data *imm)
{
	for (int i = 0; i < MAX_LINES; ++i)
	{
		for (int k = 0; k < MAX_LENGHT; ++k)
		{
			screen[i][k] = -1;
			depths[i][k] = -1;
		}
	}
	check_dupe.clear();

	draw_room(ch, &world[ch->in_room], 1, MAX_LINES/2, MAX_LENGHT/2);

	int start_line = -1, end_line = MAX_LINES, char_line = -1;
	// для облегчения кода - делаем проход по экрану
	// для расставления Y символов в зависимости от наличия
	// или отсутствия выходов вверх/вниз
	// заодно убираем пустые строки экрана, чтобы не делать
	// потом аллокации на каждую печатаемую строку
	for (int i = 0; i < MAX_LINES; ++i)
	{
		bool found = false;

		for (int k = 0; k < MAX_LENGHT; ++k)
		{
			if (screen[i][k] > -1 && screen[i][k] < SCREEN_TOTAL)
			{
				found = true;

				if (screen[i][k] == SCREEN_CHAR)
				{
					char_line = i;
				}

				if (screen[i][k] >= SCREEN_Y_OPEN
					&& screen[i][k] <= SCREEN_Y_WALL
					&& k + 1 < MAX_LENGHT && k >= 1)
				{
					if (screen[i][k + 1] > -1
						&& screen[i][k + 1] != SCREEN_UP_WALL)
					{
						screen[i][k - 1] = screen[i][k] + SCREEN_Y_UP_OPEN;
						screen[i][k] = SCREEN_EMPTY;
					}
					else if (screen[i][k - 1] > -1
						&& screen[i][k - 1] != SCREEN_DOWN_WALL)
					{
						screen[i][k] += SCREEN_Y_DOWN_OPEN;
						screen[i][k + 1] = SCREEN_EMPTY;
					}
					else
					{
						screen[i][k - 1] = screen[i][k];
						screen[i][k] = SCREEN_EMPTY;
						screen[i][k + 1] = SCREEN_EMPTY;
					}
				}
				else if (screen[i][k] >= SCREEN_Y_OPEN
					&& screen[i][k] <= SCREEN_Y_WALL)
				{
					screen[i][k] = SCREEN_EMPTY;
					send_to_char("Ошибка при генерации карты (1), сообщите богам!\r\n", ch);
				}
			}
		}

		if (found && start_line < 0)
		{
			start_line = i;
		}
		else if (!found && start_line > 0)
		{
			end_line = i;
			break;
		}
	}

	if (start_line == -1 || char_line == -1)
	{
		log("assert print_map start_line=%d, char_line=%d", start_line, char_line);
		return;
	}

	std::string out;
	out += "\r\n";

	bool fixed_1 = false;
	bool fixed_2 = true;


	if (fixed_1 || fixed_2)
	{
		 int need_top = fixed_2 ? 5 : 3;
		int top_lines = char_line - start_line;
		if (top_lines < need_top)
		{
			for (int i = 1; i <= need_top - top_lines; ++i)
			{
				out += ": \r\n";
			}
		}
	}

	for (int i = start_line; i < end_line; ++i)
	{
		out += ": ";
		for (int k = 0; k < MAX_LENGHT; ++k)
		{
			if (screen[i][k] <= -1)
			{
				out += " ";
			}
			else if (screen[i][k] < SCREEN_TOTAL
				&& screen[i][k] != SCREEN_EMPTY)
			{
				out += signs[screen[i][k]];
			}
		}
		out += "\r\n";
	}

	if (fixed_1 || fixed_2)
	{
		 int need_bot = fixed_2 ? 6 : 4;
		int bot_lines = end_line - char_line;
		if (bot_lines < need_bot)
		{
			for (int i = 1; i <= need_bot - bot_lines; ++i)
			{
				out += ": \r\n";
			}
		}
	}

	out += "\r\n";

	if (imm)
	{
		send_to_char(out.c_str(), imm);
	}
	else
	{
		send_to_char(out.c_str(), ch);
	}
}


void do_map(struct char_data *ch, char *argument, int/* cmd*/, int/* subcmd*/)
{
	if (IS_NPC(ch))
	{
		return;
	}
	MapSystem::print_map(ch);
}
}

// vim: ts=4 sw=4 tw=0 noet syntax=cpp :

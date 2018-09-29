#ifndef INTERPRETER_H
#define INTERPRETER_H
/* ************************************************************************
*   File: interpreter.h                                 Part of CircleMUD *
*  Usage: header file: public procs, macro defs, subcommand defines       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "interp-decl.h"
#include "xformat/list_commands.h"

#define ACMD(name)  \
    void name(struct char_data *ch, char *argument, int cmd, int subcmd, int countcmd)

ACMD(do_move);

#define CMD_NAME (cmd_info[cmd].command)
#define CMD_IS(cmd_name) (!strn_cmp(cmd_name, cmd_info[cmd].command, strlen(cmd_name)))
#define CMD_PTR (cmd_info[cmd].command_pointer)
#define IS_MOVE(cmdnum) (cmd_info[cmdnum].command_pointer == do_move)

struct command_info {
    const char *command;
    const char *command_alias;
    byte minimum_position;
    void (*command_pointer)
     (struct char_data * ch, char *argument, int cmd, int subcmd, int countcmd);
    sbyte minimum_level;
    int subcmd;
    int unhide_percent;
    int hold;
};

extern const struct command_info cmd_info[];

/* interpreter.cpp */
void command_interpreter(struct char_data *ch, char *argument);
int special(struct char_data *ch, int cmd, char *arg);

int search_block(char *arg, const char **list, int exact);
char lower(char c);
char *one_argument(char *argument, char *first_arg);
char *one_word(char *argument, char *first_arg);
char *any_one_arg(char *argument, char *first_arg);
char *two_arguments(char *argument, char *first_arg, char *second_arg);
int fill_word(char *argument);
int reserved_word(char *argument);
void half_chop(char *string, char *arg1, char *arg2);
int is_abbrev(const char *arg1, const char *arg2);
int find_command(const char *command);
void skip_spaces(char **string);
char *delete_doubledollar(char *string);
int find_name(char *name);
int _parse_name(char *arg, char *name);

void nanny(struct descriptor_data *d, char *arg);
void do_entergame(struct descriptor_data *d);
void do_entergame_(struct descriptor_data *d, int load_room);

/* alias.cpp */
void read_aliases(struct char_data *ch);
void write_aliases(struct char_data *ch);
struct alias_data *find_alias(struct alias_data *alias_list, char *str);
void free_alias(struct alias_data *a);
void perform_complex_alias(struct txt_q *input_q, char *orig, struct alias_data *a);
int perform_alias(struct descriptor_data *d, char *orig);

/* modify.cpp */
void show_string(struct descriptor_data *d, char *input);
void page_string(struct descriptor_data *d, const char *str, int keep_internal);
void string_add(struct descriptor_data *d, char *str);
void string_write(struct descriptor_data *d, char **txt, size_t len, long mailto, void *data);

/*
 * SUBCOMMANDS
 *   You can define these however you want to, and the definitions of the
 *   subcommands are independent from function to function.
 */

/* directions */
#define SCMD_NORTH 1
#define SCMD_EAST 2
#define SCMD_SOUTH 3
#define SCMD_WEST 4
#define SCMD_UP  5
#define SCMD_DOWN 6

/* do_gen_ps */
#define SCMD_INFO       0
#define SCMD_HANDBOOK   1
#define SCMD_CREDITS    2
#define SCMD_NEWS       3
#define SCMD_POLICIES   5
#define SCMD_VERSION    6
#define SCMD_IMMLIST    7
#define SCMD_MOTD 8
#define SCMD_IMOTD 9
#define SCMD_CLEAR 10
#define SCMD_WHOAMI 11

/* do_gen_tog */
#define SCMD_NOSUMMON   0
#define SCMD_NOHASSLE   1
#define SCMD_BRIEF      2
#define SCMD_COMPACT    3
#define SCMD_NOTELL 4
#define SCMD_CURSES 5
#define SCMD_NOHOLLER 6
#define SCMD_NOGOSSIP 7
#define SCMD_NOGRATZ 8
#define SCMD_NOWIZ 9
#define SCMD_QUEST 10
#define SCMD_ROOMFLAGS 11
#define SCMD_NOREPEAT 12
#define SCMD_HOLYLIGHT 13
#define SCMD_SLOWNS 14
#define SCMD_AUTOEXIT 15
#define SCMD_TRACK 16
#define SCMD_COLOR      17
#define SCMD_CODERINFO  18
#define SCMD_AUTOMEM    19
#define SCMD_COMPRESS 20
#define SCMD_AUTOZLIB 21
#define SCMD_NOSHOUT 22
#define SCMD_AUTOFRM 23
#define SCMD_WIDTH 24
#define SCMD_TIPS 25
#define SCMD_GRMOB 26
#define SCMD_GROBJ 27
#define SCMD_DISPBOI    28
#define SCMD_SHOWKILL   29
#define SCMD_SHOWMEGA   30
#define SCMD_WIMPY    31
#define SCMD_HP  32
#define SCMD_MOVE 33
#define SCMD_MANA 34
#define SCMD_SCORE 35
#define SCMD_GOLD 36
#define SCMD_EXITS 37
#define SCMD_LEVEL 38
#define SCMD_HEIGHT 39
#define SCMD_NOFOLLOW 40
#define SCMD_NOGIVE 41
#define SCMD_THEME 42
#define SCMD_EXAMIN 43
#define SCMD_EXITRUS 44
#define SCMD_AUTODEVTE 45
#define SCMD_DIVD 46
#define SCMD_DIVR 47
#define SCMD_PROMPT     48
#define SCMD_BANK_RENT 49
#define SCMD_SELFMESS 50
#define SCMD_MAPPER 51
#define SCMD_MINIMAP 52


/* do_wizutil */
#define SCMD_REROLL     0
#define SCMD_PARDON     1
#define SCMD_NOTITLE    2
#define SCMD_SQUELCH    3
#define SCMD_FREEZE     4
#define SCMD_THAW       5
#define SCMD_UNAFFECT   6
#define SCMD_HELL       7
#define SCMD_NAME       8
#define SCMD_REGISTER   9
#define SCMD_MUTE       10
#define SCMD_DUMB       11

/* do_spec_com */
#define SCMD_WHISPER 0
#define SCMD_ASK 1

/* do_gen_com */
#define SCMD_HOLLER 0
#define SCMD_SHOUT 1
#define SCMD_GOSSIP 2
#define SCMD_AUCTION 3
#define SCMD_GRATZ 4

/* do_shutdown */
#define SCMD_SHUTDOW 0
#define SCMD_SHUTDOWN   1

/* do_quit */
#define SCMD_QUI 0
#define SCMD_QUIT 1

/* do_date */
#define SCMD_DATE 0
#define SCMD_UPTIME 1


/* do_commands */
#define SCMD_COMMANDS 0
#define SCMD_ALIAS 1
#define SCMD_SOCIALS 2
#define SCMD_WIZHELP 3

/*  do helpee */
#define SCMD_BUYHELPEE  0
#define SCMD_FREEHELPEE 1

/* do_drop */
#define SCMD_DROP 0
#define SCMD_JUNK 1
#define SCMD_DONATE 2

/* do_gen_write */
#define SCMD_BUG 0
#define SCMD_TYPO 1
#define SCMD_IDEA 2

/* do_look */
#define SCMD_LOOK    0
#define SCMD_READ    1
#define SCMD_LOOK_HIDE 2
#define SCMD_EXAMINE 3          //ADDED BY HMEPAS

/* do_qcomm */
#define SCMD_QSAY 0
#define SCMD_QECHO 1

/* do_pour */
#define SCMD_POUR 0
#define SCMD_FILL 1
#define SCMD_APOUR 2


/* do_poof */
#define SCMD_POOFIN 0
#define SCMD_POOFOUT 1

/* do_hit */
#define SCMD_HIT 0
#define SCMD_MURDER 1

/* do_eat */
#define SCMD_EAT 0
#define SCMD_TASTE 1
#define SCMD_DRINK 2
#define SCMD_SIP 3

/* do_use */
#define SCMD_USE 0
#define SCMD_QUAFF 1
#define SCMD_RECITE 2

/* do_echo */
#define SCMD_ECHO 0
#define SCMD_EMOTE 1

/* do_gen_door */
#define SCMD_OPEN       0
#define SCMD_CLOSE      1
#define SCMD_UNLOCK     2
#define SCMD_LOCK       3
#define SCMD_PICK       4
#define SCMD_CRASH 5

/* do_mixture */
#define SCMD_ITEMS      0
#define SCMD_RUNES      1

#define SCMD_RECIPE        1

/*. do_liblist .*/
#define SCMD_OLIST      0
#define SCMD_MLIST      1
#define SCMD_RLIST      2
#define SCMD_ZLIST      3
/* do_wake*/
#define SCMD_WAKE 0
#define SCMD_WAKEUP 1

/* do_cast */
#define SCMD_CAST 0
#define SCMD_PRAY 1

/* do_turn */
#define SCMD_LPICKR 0
#define SCMD_LPICKL 1

#endif

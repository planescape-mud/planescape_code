#ifndef COMM_DECL_H
#define COMM_DECL_H
/* ************************************************************************
*   File: comm.h                                        Part of CircleMUD *
*  Usage: header file: prototypes of public communication functions       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/* descriptor-related structures ******************************************/

struct txt_block {
    char *text;
    int aliased;
    struct txt_block *next;
};


struct txt_q {
    struct txt_block *head;
    struct txt_block *tail;
};

struct descriptor_data {
    socket_t descriptor; /* file descriptor for socket   */
    char host[HOST_LENGTH+1];  /* hostname       */
    unsigned long ip_addr;
    byte bad_pws;    /* number of bad pw attemps this login  */
    byte idle_tics;    /* tics idle at password prompt   */
    int  connected;    /* mode of 'connectedness'    */
    int  desc_num;   /* unique num assigned to desc    */
    time_t input_time;
    time_t login_time;   /* when the person connected    */
    long InBytes;
    long OutBytes;
    char *showstr_head;    /* for keeping track of an internal str */
    char **showstr_vector; /* for paging through texts   */
    int  showstr_count;    /* number of pages to page through  */
    int  showstr_page;   /* which page are we currently showing? */
    char **str;      /* for the modify-str system    */
    size_t max_str;      /*    -     */
    char *backstr;   /* added for handling abort buffers */
    long mail_to;    /* name for mail system     */
    int  has_prompt;   /* is the user at a prompt?             */
    char inbuf[MAX_RAW_INPUT_LENGTH];  /* buffer for raw input   */
    char last_input[MAX_INPUT_LENGTH]; /* the last input     */
    char small_outbuf[SMALL_BUFSIZE];  /* standard output buffer   */
    char *output;    /* ptr to the current output buffer */
    char **history;    /* History of commands, for ! mostly. */
    int  history_pos;    /* Circular array position.   */
    int  bufptr;     /* ptr to end of current output   */
    int  bufspace;   /* space left in the output buffer  */
    struct txt_block *large_outbuf; /* ptr to large buffer, if we need it */
    struct txt_q input;    /* q of unprocessed input   */
    struct char_data *character; /* linked to char     */
    struct char_data *original;  /* original char if switched    */

//Chaged BY HMEPAS for multisnooping
    struct descriptor_data *snooping; /* Who is this char snooping */
    struct descriptor_data **snoop_by; /* And who is snooping this char  */
    int snoop_by_col;
//END of Hmepas's changing
    struct descriptor_data *next; /* link to next descriptor   */
    ubyte  keytable;
    int  options;     /* descriptor flags      */
#if defined(HAVE_ZLIB)
    z_stream *deflate;    /* compression engine      */
    int mccp_version;
#endif
};

struct ip_table_type {
    char ip[30];
    long timer;
    struct ip_table_type *next;
};

extern struct ip_table_type *ip_table;

extern socket_t mother_desc; /* main server socket */

extern struct descriptor_data *descriptor_list;         /* master desc list */
extern struct txt_block *bufpool;  /* pool of large output buffers */
extern int buf_largecount;         /* # of large buffers which exist */
extern int buf_overflows;          /* # of overflows of output */
extern int buf_switches;           /* # of switches from small to large buf */
extern int circle_shutdown;        /* clean shutdown */
extern int circle_copyover;        /* clean copyover */
extern int circle_reboot;          /* reboot the game after a shutdown */
extern int shutdown_time;          /* reboot at this time */
extern int copyover_time;          /* copyover at this time */
extern int max_players;            /* max descriptors available */
extern int last_desc;              /* last descriptor number */
extern int tics;                   /* for extern checkpointing */
extern long OutBytes;              /* total bytes sent */
extern long InBytes;               /* total bytes received */
extern int mins_since_crashsave;

/* Modes of connectedness: used by descriptor_data.state */
#define CON_PLAYING      0    /* Playing - Nominal state  */
#define CON_CLOSE      1    /* Disconnecting    */
#define CON_GET_NAME   2    /* By what name ..?   */
#define CON_NAME_CNFRM   3    /* Did I get that right, x? */
#define CON_PASSWORD   4    /* Password:      */
#define CON_NEWPASSWD  5    /* Give me a password for x */
#define CON_CNFPASSWD  6    /* Please retype password:  */
#define CON_QSEX       7    /* Sex?       */
#define CON_QCLASS       8    /* Class?     */
#define CON_RMENU      9    /* PRESS RETURN after MOTD  */
#define CON_MENU       10   /* Your choice: (main menu) */
#define CON_EXDESC       11   /* Enter a new description: */
#define CON_CHPWD_GETOLD 12   /* Changing passwd: get old */
#define CON_CHPWD_GETNEW 13   /* Changing passwd: get new */
#define CON_CHPWD_VRFY   14   /* Verify new password    */
#define CON_DELCNF1      15   /* Delete confirmation 1  */
#define CON_DELCNF2      16   /* Delete confirmation 2  */
#define CON_DISCONNECT   17   /* In-game disconnection  */

#define CON_NAME2        24
#define CON_NAME3        25
#define CON_NAME4        26
#define CON_NAME5        27
#define CON_NAME6        28
#define CON_RELIGION     29
#define CON_RACE         30
#define CON_GODS         31
#define CON_GET_KEYTABLE 32
#define CON_GET_EMAIL    33
#define CON_RMOTD        34
#define CON_STATISTICS  35
#define CON_ROLL_STATS   36
#define CON_NAME_NEW 37
#define CON_ALIGN 38
#define CON_TEMP 39
#define CON_CREATENPASS 40
#define CON_REQUST_ANSW 41
#define CON_ENTER_REG 42

#endif

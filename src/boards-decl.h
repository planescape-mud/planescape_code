#ifndef BOARDS_DECL_H
#define BOARDS_DECL_H
/* ************************************************************************
*   File: boards.h                                      Part of CircleMUD *
*  Usage: header file for bulletin boards                                 *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#define NUM_OF_BOARDS  2 /* change if needed! */
#define MAX_BOARD_MESSAGES  256     /* arbitrary -- change if needed */
#define MAX_MESSAGE_LENGTH 4096 /* arbitrary -- change if needed */

#define INDEX_SIZE    ((NUM_OF_BOARDS*MAX_BOARD_MESSAGES) + 5)

#define BOARD_MAGIC 1048575 /* arbitrary number - see modify.c */

struct board_msginfo {
    int slot_num;     /* pos of message in "master index" */
    char *heading;     /* pointer to message's heading */
    int level;        /* level of poster */
    int heading_len;  /* size of header (for file write) */
    int message_len;  /* size of message text (for file write) */
};

struct board_info_type {
    obj_vnum vnum; /* vnum of this board */
    int read_lvl; /* min level to read messages on this board */
    int write_lvl; /* min level to write messages on this board */
    int remove_lvl; /* min level to remove messages from this board */
    char filename[50]; /* file to save this board to */
    obj_rnum rnum; /* rnum of this board */
};

extern struct board_info_type board_info[NUM_OF_BOARDS];
extern char *msg_storage[INDEX_SIZE];
extern int msg_storage_taken[INDEX_SIZE];
extern int num_of_msgs[NUM_OF_BOARDS];
extern struct board_msginfo msg_index[NUM_OF_BOARDS][MAX_BOARD_MESSAGES];
extern bool boards_loaded;

#endif

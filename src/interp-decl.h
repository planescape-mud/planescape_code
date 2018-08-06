#ifndef INTERPRETER_DECL_H
#define INTERPRETER_DECL_H
/* ************************************************************************
*   File: interpreter.h                                 Part of CircleMUD *
*  Usage: header file: public procs, macro defs, subcommand defines       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

/*
 * Alert! Changed from 'struct alias' to 'struct alias_data' in bpl15
 * because a Windows 95 compiler gives a warning about it having similiar
 * named member.
 */
struct alias_data {
    char *alias;
    char *replacement;
    int type;
    struct alias_data *next;
};

#define ALIAS_SIMPLE 0
#define ALIAS_COMPLEX 1

#define ALIAS_SEP_CHAR ';'
#define ALIAS_VAR_CHAR '$'
#define ALIAS_GLOB_CHAR '*'


#endif

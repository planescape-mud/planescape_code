#ifndef MUD_MAIL_H
#define MUD_MAIL_H
/* ************************************************************************
*   File: mail.h                                        Part of CircleMUD *
*  Usage: header file for mail system                                     *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "mail-decl.h"

/******* MUD MAIL SYSTEM HEADER FILE **********************
 ***     written by Jeremy Elson (jelson@circlemud.org) ***
 *********************************************************/

int scan_file(void);
int has_mail(long recipient);
void store_mail(long to, long from, char *message_pointer);
char *read_delete(long recipient);

#endif

/* $Id$
 *
 * ruffina, 2009
 */
#ifndef WRAP_UTILS_H
#define WRAP_UTILS_H

#include "wrapperbase.h"
#include "xmlregister.h"
#include "register-decl.h"

using Scripting::Register;
using Scripting::RegisterList;

struct char_data;
struct room_data;
struct obj_data;

Register wrap( struct char_data * );
Register wrap( struct obj_data * );
Register wrap( struct room_data * );

const Register & get_unique_arg( const RegisterList & );
const Register & arg_one( const RegisterList & );
const Register & arg_two( const RegisterList & );
int args2number( const RegisterList& );
DLString args2string( const RegisterList& );
void args2buf(const RegisterList &args, char *buf, size_t bufsize);

struct char_data * arg2character( const Register & );
struct room_data * arg2room( const Register &reg );
struct obj_data * arg2item( const Register &reg );

#endif


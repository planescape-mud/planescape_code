#ifndef PARSER_TYPES_H
#define PARSER_TYPES_H

#ifndef _MINGW_
typedef unsigned char BYTE;
#endif

#define TYPE_INT  0   // int
#define TYPE_STRING  1   // char*
#define TYPE_STRUCT  2
#define TYPE_VECTOR  3   // char*
#define TYPE_LIST  4   // int
#define TYPE_RANDOM  5   // char*
#define TYPE_SCRIPT  6   // int*
#define TYPE_STRLIST 7   // int, int
#define TYPE_FLOAT  8   // float
#define TYPE_EXPR  9
#define TYPE_LONGLONG 10

#define PARAM_INT  0
#define PARAM_FLOAT  1
#define PARAM_STRING 2
#define PARAM_LONGLONG 3

#endif

#ifndef PARSER_UTILS_H
#define PARSER_UTILS_H

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <strstream>

#include "logstream.h"
#define log notice

#define MEMALARM(size) \
    do { \
        syserr("Memory allocation error, size %d", size); \
        ALARM; \
    } while (0)

#define ALARM \
    do { \
        syserr("Critical error at %s:%d.", __FILE__, __LINE__); \
        abort(); \
    } while(0)

#endif

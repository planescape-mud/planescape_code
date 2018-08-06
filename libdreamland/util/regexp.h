/* $Id: regexp.h,v 1.1.2.7.10.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */

#ifndef REGEXP_H
#define REGEXP_H

#include "../regex/regex.h"
#include <vector>

#include "dlobject.h"
#include "dlstring.h"

#include "exception.h"

class RegExp : public virtual DLObject {
public:
    typedef std::vector<DLString> MatchVector;

    struct Exception : public ::Exception 
    { 
	Exception(const char *e) : ::Exception(e) { }
    };
    
    RegExp( const char * );
    RegExp( const char *, bool );
    ~RegExp( );

    bool match( const char * );
    bool match( const DLString & );
    MatchVector subexpr( const char * );

private:
    void prepare( const char *, bool );

    regex_t preg;
};

#endif


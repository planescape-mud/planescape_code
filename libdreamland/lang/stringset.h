/* $Id: stringset.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#ifndef STRINGSET_H
#define STRINGSET_H

#include <set>
#include "dlstring.h"

class StringSet : public std::set<DLString> {
public:

    DLString toString( ) const;
    void fromString( const DLString & );
};

#endif


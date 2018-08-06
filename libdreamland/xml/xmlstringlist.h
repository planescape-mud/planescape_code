/* $Id: xmlstringlist.h,v 1.1.2.4 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#ifndef XMLSTRINGLIST_H
#define XMLSTRINGLIST_H

#include "stringset.h"
#include "xmlstring.h"
#include "xmllist.h"

class XMLStringList : public XMLListBase<XMLString> {
public:

    void toSet( StringSet & ) const;
};


#endif

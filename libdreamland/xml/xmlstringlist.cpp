/* $Id: xmlstringlist.cpp,v 1.1.2.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
#include "xmlstringlist.h"

void XMLStringList::toSet( StringSet &aset ) const
{
    const_iterator i;

    for (i = begin( ); i != end( ); i++)
	if (*i != "\'" && *i != "\"")
	    aset.insert( *i );
}


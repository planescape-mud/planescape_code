/* $Id: stringset.cpp,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#include "stringset.h"

DLString StringSet::toString( ) const
{
    const_iterator s;
    DLString result;

    for (s = begin( ); s != end( ); s++) 
	result << s->quote( ) << " ";
	
    result.stripWhiteSpace( );
    return result;
}

void StringSet::fromString( const DLString &constStr )
{
    DLString arg;
    DLString str = constStr;
    
    while (!( arg = str.getOneArgument( ) ).empty( ))
	insert( arg );
}



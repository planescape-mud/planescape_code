/* $Id: stringset.cpp,v 1.1.2.3 2011-04-17 19:37:54 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#include "stringset.h"
#include "logstream.h"
#include "integer.h"

DLString StringSet::toString( ) const
{
    const_iterator s;
    DLString result;

    for (s = begin( ); s != end( ); s++) 
	result << s->quote( ) << " ";
	
    result.stripWhiteSpace( );
    return result;
}

StringSet & StringSet::fromString( const DLString &constStr )
{
    DLString arg;
    DLString str = constStr;
    
    while (!( arg = str.getOneArgument( ) ).empty( ))
	insert( arg );

    return *this;
}


void NumberSet::fromStringSet( const StringSet &str )
{
    for (StringSet::const_iterator s = str.begin( ); s != str.end( ); s++)
        try {
            insert( Integer( *s ).getValue( ) );
        } catch (const ExceptionBadType &ex) {
            ex.printStackTrace( LogStream::sendError( ) );
        }
}

StringSet NumberSet::toStringSet( ) const
{
    StringSet str;

    for (const_iterator n = begin( ); n != end( ); n++)
        str.insert( DLString( *n ) );

    return str;
}


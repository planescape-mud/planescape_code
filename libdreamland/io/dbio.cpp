/* $Id: dbio.cpp,v 1.12.2.2.30.4 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
// dbio.cpp: implementation of the DBIO class.
//
//////////////////////////////////////////////////////////////////////

#include <fstream>
#include <sstream>

#include "dlfilestream.h"
#include "dbio.h"

const DLString DBIO::EXT_XML = ".xml";
	
DBIO::DBIO( const DLString & tableName )
	: table( tableName )
{
}

DBIO::DBIO( const DLString & tablePath, const DLString & tableName )
	: table( tablePath, tableName )
{
}

DBIO::DBIO( const DLDirectory & tableDir, const DLString & tableName )
	: table( tableDir, tableName )
{
}

DBIO::~DBIO( )
{
}

void DBIO::open( ) throw( ExceptionDBIO )
{
    table.open( );
}

void DBIO::open( const DLString &tableName ) throw( ExceptionDBIO )
{
    table.open( tableName );
}

DBIO::DBNode DBIO::nextXML( ) throw( ExceptionDBIO, ExceptionDBIOEOF )
{
    DLFile entry = table.nextTypedEntry( EXT_XML );
    DLFileStream stream( table, entry );

    std::basic_ostringstream<char> buf;
    stream.toStream( buf );

    return DBNode( entry.getFileName( ), buf.str( ) );
}

void DBIO::insert( const DBIO::DBNode &dbNode ) throw( ExceptionDBIO )
{
    insert( dbNode.getKey( ), dbNode.getXML( ) );
}

void DBIO::safeInsert( const DBIO::DBNode &dbNode ) throw( ExceptionDBIO )
{
    safeInsert( dbNode.getKey( ), dbNode.getXML( ) );
}

void DBIO::insert( const DLString& key, const DLString& xml ) throw( ExceptionDBIO )
{
    DLFile entry( table, key, EXT_XML );
    DLFileStream stream( entry );
    stream.fromString( xml );
}

void DBIO::safeInsert( const DLString& key, const DLString& xml ) throw( ExceptionDBIO )
{
    DLFile tmpEntry = table.tempEntry( );
    DLFileStream tmpStream( tmpEntry );

    tmpStream.fromString( xml );

    DLFile entry( table, key, EXT_XML );
    tmpEntry.rename( entry );
}
	
DBIO::DBNode DBIO::select( const DLString& key ) throw( ExceptionDBIO )
{
    DLFile entry( table, key, EXT_XML );
    DLFileStream stream( entry );

    std::basic_ostringstream<char> buf;
    stream.toStream( buf );

    return DBNode( key, buf.str( ) );
}

void DBIO::remove( const DLString& key ) throw( ExceptionDBIO )
{
    DLFile entry( table, key, EXT_XML );
    
    if (!entry.remove( ))
	throw ExceptionDBIO( "Unable to delete '" + key + "'" );
}

void DBIO::renameID( const DLString& oldKey, const DLString& newKey ) throw( ExceptionDBIO )
{
    DLFile oldEntry( table, oldKey, EXT_XML );
    DLFile backupTable( table, "backup" );
    DLFile newEntry( backupTable, newKey, EXT_XML );
    
    if (!oldEntry.rename( newEntry ))
	throw ExceptionDBIO( "Unable to rename id '" + oldKey + "' to '" + newKey + "'" );
}



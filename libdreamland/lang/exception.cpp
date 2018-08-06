/* $Id: exception.cpp,v 1.2.4.5 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
/*
 * NoFate, 2001
 */
#ifndef __MINGW32__ 
#include <dlfcn.h>
#endif
#include <stdio.h>
#include "exception.h"
#include "exceptionbadtype.h"
#include "fileformatexception.h"

Exception::Exception( const string &str ) throw( ) : message( str )
{
    fillStackFrames(NULL);
}

const char*
Exception::what( ) const throw( )
{
    return message.c_str( );
}

Exception::~Exception( ) throw( )
{
}

void
Exception::setStr( const string& str )
{
    message = str;
}

void Exception::fillStackFrames( void *a )
{
    void **bp;
    
    for(bp = &a - 3; ; bp = (void **)*bp) {
	if(!*bp)
	    return;
	
	if((char *)*bp <= (char *)bp)
	    return;
	
	if((char *)*bp - (char *)bp > 0x10000)
	    return;

	callstack.push_back(bp[1]);
    }
}

void Exception::printStackTrace( std::ostream &os ) const
{
    std::vector<void *>::const_iterator it;
    
    os << "Exception: " << message << std::endl;

    for(it = callstack.begin( ); it != callstack.end( ); it++) {
	void *ip = *it;

	os << "  at ";
#ifndef __MINGW32__ 
	Dl_info info;
	
	if(dladdr(ip, &info)) 
	    os  << info.dli_fname << '!' 
		<< info.dli_sname << '+'
		<< (char*)ip-(char*)info.dli_saddr 
		<< std::endl;
	else
#endif            
	    os  << ip << std::endl;
    }
}

FileFormatException::FileFormatException( const char * fmt, ... ) throw( )
{
    va_list ap;
    char buf[1024];
    
    va_start( ap, fmt );
    vsnprintf( buf, sizeof( buf ), fmt, ap );
    va_end( ap );

    setStr( string( buf ) );	    
}

FileFormatException::~FileFormatException( ) throw( )
{
}

ExceptionBadType::~ExceptionBadType( ) throw( )
{
}


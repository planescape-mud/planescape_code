/***************************************************************************
                          xmldocument.cpp  -  description
                             -------------------
    begin                : Fri May 4 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <fstream>
#include <iostream>

#undef yyFlexLexer
#define yyFlexLexer xmlFlexLexer
#include <FlexLexer.h>

#include "xmldocument.h"
#include "xmlparser.h"
#include "logstream.h"

XMLDocument::XMLDocument( )
	: encoding( DEFAULT_ENCODING ),
		version( DEFAULT_VERSION )
{
}

void XMLDocument::emit( const XMLNode &node, ostream& ostr, int space, bool& cdataPrev ) const
{
    const NodeList &nlist = node.getNodeList( );
    NodeList::const_iterator inode;
    
    for( inode = nlist.begin( ); inode != nlist.end( ); inode++ )
    {
	XMLNode::Pointer pnode = *inode;
	switch( pnode->getType( ) )
	{
	case XML_CDATA:
	    ostr << "<![CDATA[" << pnode->getCData( ) << "]]>";
	    cdataPrev = true;
	    break;
	case XML_TEXT:
	    for( DLString::const_iterator ipos = pnode->getCData( ).begin( );
		    ipos != pnode->getCData( ).end( );
		    ipos++ )
	    {
		char ch = *ipos;
		switch( ch )
		{
		case '&':   ostr << "&amp;";	break;
		case '\'':  ostr << "&apos;";	break;
		case '>':   ostr << "&gt;";	break;
		case '<':   ostr << "&lt;";	break;
		case '"':   ostr << "&quot;";	break;
		default:    ostr << ch;
		}
	    }
	    cdataPrev = true;
	    break;
	default:
	    cdataPrev = false;
	    ostr << endl;
	    
	    for( int i = 0; i < space; i++ ) 
		ostr << ' ';
	    
	    ostr << '<' << pnode->getName( );
	    
	    const AttributeListType &attrs = pnode->getAttributes( );
	    AttributeListType::const_iterator iattr;
	    
	    for( iattr = attrs.begin( ); iattr != attrs.end( ); iattr++ )
		ostr << ' ' << iattr->first << "=\"" 
		     << iattr->second.substitute('\\', "\\\\").substitute('"', "\\\"")
		     << '"';
	    
	    if( pnode->getType( ) == XML_LEAF )
		ostr << "/>";
	    else {
		ostr << '>';
		
		emit( **pnode, ostr, space + 2, cdataPrev );
		if( !cdataPrev ) {
		    ostr << endl;
		    
		    for( int i = 0; i < space; i++ ) 
			ostr << ' ';
		}

		ostr << "</" << pnode->getName( ) << '>';
		cdataPrev = false;
	    }
	}
    }
}

void XMLDocument::save( ostream& ostr ) const throw( ExceptionXMLError ) 
{
    ostr << "<?xml version=\"" << version << "\" encoding=\"" << encoding << "\"?>";
    bool cdataPrev = false;
    emit( *this, ostr, 0, cdataPrev );
    ostr << endl;
}

void XMLDocument::load( istream& istr ) throw( ExceptionXMLError )
{
    nodes.clear( );
    XMLDocument::Pointer document( this );
    XMLParser lex( document, &istr );

    lex.yylex( );
}
const char* const XMLDocument::DEFAULT_ENCODING = "KOI8-R";
const char* const XMLDocument::DEFAULT_VERSION = "1.0";
	

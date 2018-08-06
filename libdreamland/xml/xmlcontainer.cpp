/* $Id: xmlcontainer.cpp,v 1.1.2.5.10.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */
#include <typeinfo>
#include "xmlcontainer.h"
#include "logstream.h"

void 
XMLContainer::fromXML( const XMLNode::Pointer & parent ) throw( ExceptionBadType )
{
    XMLNode::NodeList::const_iterator ipos;
    const XMLNode::NodeList& children = parent->getNodeList( );

    for (ipos = children.begin( ); ipos != children.end( ); ipos++) 
	if (!nodeFromXML( *ipos )) 
	    LogStream::sendWarning( ) 
		<< "Unparsed node <" << parent->getName( ) 
		<< "> <" << (*ipos)->getName( ) 
		<< "> mytype " << typeid( this ).name( ) << endl;
//	    throw ExceptionBadType( parent->getName( ), (*ipos)->getName( ) );
}

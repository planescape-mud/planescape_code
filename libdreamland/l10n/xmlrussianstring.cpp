/* $Id: xmlrussianstring.cpp,v 1.1.2.3 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2007
 */
#include <string.h>
#include "grammar_entities_impl.h"
#include "xmlrussianstring.h"

using namespace Grammar;

const DLString XMLRussianString::ATTRIBUTE_GRAMMAR = "mg";

void XMLRussianString::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    XMLNode::Pointer node = parent->getFirstNode( );
    
    mg = MultiGender(parent->getAttribute( ATTRIBUTE_GRAMMAR ).c_str());

    if (!node.isEmpty( )) 
	setFullForm(node->getCData( ));
}

bool XMLRussianString::toXML( XMLNode::Pointer& parent ) const
{
    XMLNode::Pointer node( NEW );
    
    node->setType( XMLNode::XML_TEXT );
    node->setCData( getFullForm() );
    
    parent->insertAttribute( ATTRIBUTE_GRAMMAR, mg.toString() );
    parent->appendChild( node );
    return true;
}


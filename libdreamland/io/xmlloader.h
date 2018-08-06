/* $Id: xmlloader.h,v 1.1.2.6.6.4 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2005
 */
#ifndef __XMLLOADER_H__
#define __XMLLOADER_H__

#include "dlstring.h"

class XMLVariable;

class XMLLoader {
public:
    virtual ~XMLLoader( );    
    virtual DLString getTablePath( ) const = 0;
    virtual DLString getTableName( ) const = 0;
    virtual DLString getNodeName( ) const = 0;

    bool loadXML( XMLVariable *, const DLString & ) const;
    bool saveXML( const XMLVariable *, const DLString &, bool fSafe = false ) const;
};

#endif

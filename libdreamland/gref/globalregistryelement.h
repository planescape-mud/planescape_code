/* $Id: globalregistryelement.h,v 1.1.2.3 2009/10/11 18:35:36 rufina Exp $
 *
 * ruffina, Dream Land, 2006
 */
#ifndef __GLOBALREGISTRYELEMENT_H__
#define __GLOBALREGISTRYELEMENT_H__

#include "dlobject.h"
#include "dlstring.h"

class GlobalRegistryElement : public virtual DLObject {
friend class GlobalRegistryBase;
public:
    typedef ::Pointer<GlobalRegistryElement> Pointer;
    
    GlobalRegistryElement( ) : index( -1 )
    {
    }
    
    virtual const DLString &getName( ) const = 0;
    
    virtual bool matchesStrict( const DLString &str ) const 
    {
	return !str.empty( ) && str == getName( );
    }

    virtual bool matchesUnstrict( const DLString &str ) const 
    {
	if (str.empty( ) || getName( ).empty( ))
	    return false;

	if (!str.strPrefix( getName( ) ))
	    return false;

	return true;
    }

    inline int getIndex( ) const
    {
	return index;
    }
    
    virtual bool isValid( ) const 
    {
	return true;
    }

protected:
    inline void setIndex( int i )
    {
	index = i;
    }

private:
    int index;
};

#endif

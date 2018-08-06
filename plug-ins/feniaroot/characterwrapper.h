/* $Id: characterwrapper.h,v 1.1.4.11.6.1 2007/09/11 00:09:28 margo Exp $
 *
 * ruffina, 2004
 */

#ifndef _CHARACTERWRAPPER_H_
#define _CHARACTERWRAPPER_H_

// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "xmlregister.h"
#include "exceptions.h"
// MOC_SKIP_END
#include "pluginnativeimpl.h"

// MOC_SKIP_BEGIN
struct char_data;
// MOC_SKIP_END

using Scripting::Register;
using Scripting::RegisterList;

class CharacterWrapper : public PluginWrapperImpl<CharacterWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<CharacterWrapper> Pointer;

    CharacterWrapper( );

    virtual void setSelf( Scripting::Object * );
    virtual void extract( bool );
    void setTarget( struct char_data * );
    void checkTarget( ) const throw( Scripting::Exception );
    struct char_data *getTarget( ) const;
    
private:
    struct char_data *target;
};

#endif 

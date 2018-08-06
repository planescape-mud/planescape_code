/* $Id: roomwrapper.h,v 1.1.4.8.6.1 2007/09/11 00:09:28 margo Exp $
 *
 * ruffina, 2004
 */

#ifndef _ROOMWRAPPER_H_
#define _ROOMWRAPPER_H_

// MOC_SKIP_BEGIN
#include "lex.h"
#include "scope.h"
#include "xmlregister.h"
#include "exceptions.h"
// MOC_SKIP_END
#include "pluginnativeimpl.h"

struct room_data;

class RoomWrapper : public PluginWrapperImpl<RoomWrapper>
{
XML_OBJECT
NMI_OBJECT
public:
    typedef ::Pointer<RoomWrapper> Pointer;

    RoomWrapper( );

    virtual void setSelf( Scripting::Object * );
    virtual void extract( bool );
    void setTarget( struct room_data *r );
    void checkTarget( ) const throw( Scripting::Exception );
    struct room_data * getTarget( ) const;

private:
    struct room_data *target;
};

#endif 

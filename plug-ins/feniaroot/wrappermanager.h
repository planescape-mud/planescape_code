/* $Id: wrappermanager.h,v 1.1.2.3.18.1 2007/09/21 21:23:53 margo Exp $
 *
 * ruffina, 2004
 */
#ifndef __WRAPPERMANAGER_H__
#define __WRAPPERMANAGER_H__

#include "plugin.h"
#include "register-decl.h"
#include "feniamanager.h"

class WrapperManager: public WrapperManagerBase, public Plugin {
public:
    
    virtual Scripting::Register getWrapper( struct char_data * );
    virtual Scripting::Register getWrapper( struct obj_data * );
    virtual Scripting::Register getWrapper( struct room_data * );
    
    virtual void linkWrapper( struct char_data * );
    virtual void linkWrapper( struct obj_data * );
    virtual void linkWrapper( struct room_data * );

    virtual void getTarget( const Scripting::Register &, struct char_data *& );
    virtual void getTarget( const Scripting::Register &, struct obj_data *& );
    virtual void getTarget( const Scripting::Register &, struct room_data *& );
    
    virtual void initialization( );
    virtual void destruction( );

    static WrapperManager* getThis( );

private:
    template <typename WrapperType, typename TargetType>
    Scripting::Register wrapperAux( long long, TargetType );
    
    template <typename WrapperType, typename TargetType>
    void linkAux( long long, TargetType );
};


#endif


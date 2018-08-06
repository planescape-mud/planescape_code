/* $Id: wrappertarget.h,v 1.1.2.2.18.4 2009/09/17 18:08:56 rufina Exp $
 *
 * ruffina, 2004
 */
#ifndef __WRAPPERTARGET_H__
#define __WRAPPERTARGET_H__

#include "fenia/register-decl.h"

class WrapperBase;

class WrapperTarget {
public:

    WrapperTarget( );
    
    WrapperBase * getWrapper( );
    void extractWrapper(bool forever);

    Scripting::Object *wrapper;
};

#define BASE_VOID_CALL( base, id, fmt... ) \
        if (base) { \
            static Scripting::IdRef onId( "on"id ); \
            base->call( onId, fmt ); \
            \
            static Scripting::IdRef postId( "post"id ); \
            base->postpone( postId, fmt ); \
        } 

#define FENIA_VOID_CALL( var, id, fmt...) \
        if (var) { \
            BASE_VOID_CALL( var->getWrapper(), id, fmt ); \
        }

#define FENIA_PROTO_VOID_CALL( var, id, fmt...) \
        if (var && var->getProto()) { \
            BASE_VOID_CALL( var->getProto()->getWrapper(), id, fmt ); \
        }

#define BASE_BOOL_CALL( base, id, fmt... ) \
        if (base) { \
            bool rc; \
            static Scripting::IdRef onId( "on"id ); \
            rc = base->call( onId, fmt ); \
            \
            static Scripting::IdRef postId( "post"id ); \
            base->postpone( postId, fmt ); \
            \
            if (rc) return true; \
        } 

#define FENIA_BOOL_CALL( var, id, fmt...) \
        if (var) { \
            BASE_BOOL_CALL( var->getWrapper(), id, fmt ); \
        }

#define FENIA_PROTO_BOOL_CALL( var, id, fmt...) \
        if (var && var->getProto()) { \
            BASE_BOOL_CALL( var->getProto()->getWrapper(), id, fmt ); \
        }
        
#define FENIA_STR_CALL( var, id, fmt...) \
        if (var) { \
            WrapperBase *base = var->getWrapper( ); \
            if (base) { \
                static Scripting::IdRef onId( id ); \
                DLString rc = base->stringCall( onId, fmt ); \
                if (!rc.empty( )) \
                    return rc; \
            } \
        }

#endif

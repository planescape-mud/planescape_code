/* $Id$
 *
 * ruffina, 2009
 */

#ifndef __WRAPPERMANAGERBASE_H__
#define __WRAPPERMANAGERBASE_H__

#include <map>

#include "fenia/object.h"
#include "fenia/register-decl.h"

struct char_data;
struct obj_data;
struct room_data;

class WrapperManagerBase : public virtual DLObject {
public:
    typedef ::Pointer<WrapperManagerBase> Pointer;
    typedef std::map<long long, Scripting::Object *> WrapperMap;

    virtual Scripting::Register getWrapper( struct char_data * ) = 0;
    virtual Scripting::Register getWrapper( struct obj_data * ) = 0;
    virtual Scripting::Register getWrapper( struct room_data * ) = 0;

    virtual void linkWrapper( struct char_data * ) = 0;
    virtual void linkWrapper( struct obj_data * ) = 0;
    virtual void linkWrapper( struct room_data * ) = 0;

    virtual void getTarget( const Scripting::Register &, struct char_data *& ) = 0;
    virtual void getTarget( const Scripting::Register &, struct obj_data *& ) = 0;
    virtual void getTarget( const Scripting::Register &, struct room_data *& ) = 0;

    void markAlive(long long id);

    static WrapperMap map;
};

#endif


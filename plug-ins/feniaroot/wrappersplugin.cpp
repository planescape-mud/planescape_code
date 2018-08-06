/* $Id: wrappersplugin.cpp,v 1.1.4.13.6.5 2009/09/05 20:09:58 rufina Exp $
 *
 * ruffina, 2004
 */

#include "feniamanager.h"
#include "planescape.h"
#include "mudscheduler.h"
#include "structs.h"
#include "db.h"
#include "comm.h"

#include "wrappersplugin.h"
#include "root.h"
#include "idcontainer.h"
#include "characterwrapper.h"
#include "objectwrapper.h"
#include "roomwrapper.h"
#include "validatetask.h"
#include "subr.h"

extern bool (*plugin_command_hook)(struct char_data *, char *, char *);
bool cmd_hook(struct char_data *, char *, char *carg);

WrappersPlugin* WrappersPlugin::thisClass = NULL;

WrappersPlugin::WrappersPlugin( ) {
    thisClass = this;
}

WrappersPlugin::~WrappersPlugin( ) {
    thisClass = NULL;
}

void 
WrappersPlugin::linkTargets()
{
    for (struct char_data *ch = character_list; ch; ch = ch->next) 
        if (ch->wrapper) 
            wrapper_cast<CharacterWrapper>(ch->wrapper)->setTarget( ch );

    for (int nr = 0; nr <= top_of_mobt; nr++) {
        Mobile *mob = &mob_proto[nr];
        if (mob->wrapper)
            wrapper_cast<CharacterWrapper>(mob->wrapper)->setTarget( mob );
    }

    for (struct obj_data *obj = object_list; obj; obj = obj->next)
        if (obj->wrapper)
            wrapper_cast<ObjectWrapper>(obj->wrapper)->setTarget( obj );

    for (int nr = 0; nr <= top_of_objt; nr++) {
        struct obj_data *obj = &obj_proto[nr];
        if (obj->wrapper)
            wrapper_cast<ObjectWrapper>(obj->wrapper)->setTarget( obj );
    }

    for (int nr = 0; nr <= top_of_world; nr++) {
        struct room_data *room = &world[nr];
        if (room->wrapper)
            wrapper_cast<RoomWrapper>(room->wrapper)->setTarget( room );
    }
}

void
WrappersPlugin::initialization( ) 
{
    Class::regMoc<Root>( );
    Class::regMoc<CharacterWrapper>( );
    Class::regMoc<ObjectWrapper>( );
    Class::regMoc<RoomWrapper>( );
    
    FeniaManager::getThis( )->recover( );
    
    mud->getScheduler()->putTaskNOW( ValidateTask::Pointer(NEW) );

    linkTargets();

    plugin_command_hook = cmd_hook;
}

void WrappersPlugin::destruction( ) 
{
    plugin_command_hook = 0;

    mud->getScheduler()->slay( ValidateTask::Pointer(NEW) );

    Scripting::gc = false;
    FeniaManager::getThis( )->backup( );

    Class::unregMoc<ObjectWrapper>( );
    Class::unregMoc<RoomWrapper>( );
    Class::unregMoc<CharacterWrapper>( );
    Class::unregMoc<Root>( );
}

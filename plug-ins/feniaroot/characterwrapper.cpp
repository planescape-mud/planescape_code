/* $Id: characterwrapper.cpp,v 1.1.4.50.4.38 2009/09/11 11:24:54 rufina Exp $
 *
 * ruffina, 2004
 */

#include <iostream>

#include "logstream.h"

#include "planescape.h"
#include "sysdep.h"
#include "structs.h"
#include "../runtime/utils.h"
#include "comm.h"
#include "db.h"
#include "spells.h"
#include "constants.h"
#include "interpreter.h"
#include "../runtime/handler.h"

#include "characterwrapper.h"
#include "wrappermanager.h"
#include "reglist.h"
#include "nativeext.h"
#include "wrap_utils.h"
#include "subr.h"

using namespace std;
using Scripting::NativeTraits;

NMI_INIT(CharacterWrapper, "персонаж (моб или игрок)")

CharacterWrapper::CharacterWrapper( ) : target( NULL )
{
}

void CharacterWrapper::setSelf( Scripting::Object *s )
{
    WrapperBase::setSelf( s );
    
    if (!self && target) {
        target->wrapper = 0;
        target = 0;
    }
}

void CharacterWrapper::extract( bool count )
{
    if (target) {
        target->wrapper = 0;
        target = 0;
    } else {
        if (Scripting::gc)
            LogStream::sendError() << "Character wrapper: extract without target" << endl;
    }
    
    GutsContainer::extract( count );
}

void CharacterWrapper::setTarget( struct char_data *target )
{
    this->target = target;
    id = target->guid;
}

void CharacterWrapper::checkTarget( ) const throw( Scripting::Exception )
{
    if (zombie.getValue())
        throw Scripting::Exception( "Character is dead" );

    if (target == NULL) 
        throw Scripting::Exception( "Character is offline" );
}

struct char_data * CharacterWrapper::getTarget( ) const
{
    checkTarget();
    return target;
}

/*
 * utility macros 
 */
#define METHOD_INST_ONLY \
    checkTarget(); \
    if (target->npc() && target->npc()->isProto()) \
        throw Scripting::Exception("Instance method requested on mobile prototype");

#define METHOD_PC_ONLY \
    METHOD_INST_ONLY \
    if (!target->pc()) \
        throw Scripting::Exception("PC method requested on NPC");

#define METHOD_NPC_ONLY \
    METHOD_INST_ONLY \
    if (!target->npc()) \
        throw Scripting::Exception("NPC method requested on PC");

#define METHOD_PROTO_ONLY \
    checkTarget(); \
    if (target->pc()) \
        throw Scripting::Exception("Prototype method requested on player"); \
    if (target->npc()->isProto()) \
        throw Scripting::Exception("Prototype method requested on mobile instance");

#define GETWRAP(x, h) NMI_GET(CharacterWrapper, x, h) { \
    checkTarget(); \
    return wrap(target->x); \
}


/*
 * FIELDS getters
 */
GETWRAP(master, "тот, за кем следуем")


NMI_GET( CharacterWrapper, sex, "номер пола персонажа (1, 2)" )
{
    checkTarget();
    return GET_SEX(target); 
}

NMI_GET( CharacterWrapper, sexname, "название пола персонажа (мужчина, женщина)" )
{
    checkTarget();
    return genders[(int) GET_SEX(target)];
}

NMI_GET( CharacterWrapper, level, "уровень персонажа" )
{
    checkTarget();
    return GET_LEVEL(target); 
}

NMI_GET( CharacterWrapper, race, "номер расы персонажа" )
{
    checkTarget();
    return GET_RACE(target); 
}

NMI_GET( CharacterWrapper, racename, "название расы персонажа в муж.роде (человек, бариаур)" )
{
    checkTarget();
    return race_name[(int) GET_RACE(target)][1]; 
}

NMI_GET( CharacterWrapper, is_pc, "живой (true) игрок или моб(false)" )
{
    checkTarget();
    if (target->pc())
        return true;
    return false;
}

NMI_GET( CharacterWrapper, online, "переменная true если персонаж в мире" )
{
    return Register( target != NULL );
}

NMI_GET( CharacterWrapper, dead, "переменная true если персонаж полностью уничтожен (суицид для pc, extract для npc)" )
{
    return Register( zombie.getValue() );
}

NMI_GET( CharacterWrapper, in_room, "комната в которой сейчас находимся" )
{
    METHOD_INST_ONLY
    return wrap( VALID_RNUM(target->in_room) ? &world[target->in_room] : NULL );
}

NMI_GET( CharacterWrapper, name_i, "имя (именительный падеж)" )
{
    checkTarget();
    return GET_PAD(target, 0); 
}

NMI_GET( CharacterWrapper, name_r, "имя (родительный падеж)" )
{
    checkTarget();
    return GET_PAD(target, 1); 
}

NMI_GET( CharacterWrapper, name_d, "имя (дательный падеж)" )
{
    checkTarget();
    return GET_PAD(target, 2); 
}

NMI_GET( CharacterWrapper, name_v, "имя (винительный падеж)" )
{
    checkTarget();
    return GET_PAD(target, 3); 
}

NMI_GET( CharacterWrapper, name_t, "имя (творительный падеж)" )
{
    checkTarget();
    return GET_PAD(target, 4); 
}

NMI_GET( CharacterWrapper, name_p, "имя (предложный падеж)" )
{
    checkTarget();
    return GET_PAD(target, 5); 
}

NMI_GET( CharacterWrapper, suf_w, "суффикс (ое/ый/ая/ые)" )
{
    checkTarget();
    return GET_CH_SUF_3(target); 
}

NMI_GET( CharacterWrapper, exp, "опыт персонажа" )
{
    checkTarget();
    // can rely on int and long int to be of the same size for ILP32 (4/4/4) model.
    return Register((int)GET_EXP(target)); 
}

/*
 * METHODS common for players, mobile instances and prototypes
 */
NMI_INVOKE( CharacterWrapper, api, "печатает этот API" )
{   
    ostringstream buf;
    Scripting::traitsAPI<CharacterWrapper>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( CharacterWrapper, rtapi, "печатает все поля и методы, установленные в runtime" )
{
    ostringstream buf;
    traitsAPI( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE( CharacterWrapper, clear, "очистка всех runtime полей" )
{
    guts.clear( );
    self->changed();
    return Register( );
}


/*
 * METHODS common for players and mobile instances
 */
NMI_INVOKE( CharacterWrapper, ptc, "print to char, печатает строку" )
{
    METHOD_INST_ONLY
    send_to_char(args2string(args).c_str(), target);
    return Register();
}

static void act_generic(struct char_data *target, const RegisterList &args, int type)
{
    RegisterList::const_iterator a = args.begin();
    DLString fmt;
    struct char_data *ch = NULL;
    struct obj_data *obj = NULL;
    
    if (a->type != Register::STRING)
        throw CustomException("First argument to act() must be a string");

    fmt = a->toString();

    if (++a != args.end()) {
        ch = arg2character(*a);
        if (++a != args.end())
            obj = arg2item(*a); 
    }
    
    act(fmt.c_str(), 0, target, obj, ch, type);
}

NMI_INVOKE( CharacterWrapper, act_char, "печатает этому чару строку в формате act(...,TO_CHAR)" )
{
    METHOD_INST_ONLY
    act_generic(target, args, TO_CHAR);
    return Register();
}

NMI_INVOKE( CharacterWrapper, act_room, "печатает в комнату чара строку в формате act(...,TO_ROOM)" )
{
    METHOD_INST_ONLY
    act_generic(target, args, TO_ROOM);
    return Register();
}

NMI_INVOKE( CharacterWrapper, interpret, "интерпретирует строку, как будто чар ее набрал сам" )
{
    METHOD_INST_ONLY
    char buf[MAX_STRING_LENGTH];
    args2buf(args, buf, sizeof(buf));
    command_interpreter( target, buf ); 
    return Register();
}

NMI_INVOKE( CharacterWrapper, can_see_char, "видит ли чара указанного в параметрах" )
{
    METHOD_INST_ONLY
    return CAN_SEE(target, arg2character(get_unique_arg(args)));
}

NMI_INVOKE( CharacterWrapper, can_see_obj, "видит ли предмет указанный в параметрах" )
{
    METHOD_INST_ONLY
    return CAN_SEE_OBJ(target, arg2item(get_unique_arg(args)));
}

NMI_INVOKE( CharacterWrapper, char_room, "получить видимого нам чара в нашей комнате по имени" )
{
    METHOD_INST_ONLY
    char buf[MAX_STRING_LENGTH];
    args2buf(args, buf, sizeof(buf));
    return wrap( get_char_room_vis( target, buf ) ); 
}

NMI_INVOKE( CharacterWrapper, obj_carry, "получить видимый нам предмет в инвентаре по имени" )
{
    METHOD_INST_ONLY
    char buf[MAX_STRING_LENGTH];
    args2buf(args, buf, sizeof(buf));
    return wrap( get_obj_in_list_vis( target, buf, target->carrying ) ); 
}

NMI_INVOKE( CharacterWrapper, teleport, "поместит чара в указанную комнату" )
{
    METHOD_INST_ONLY
    const Register &reg = get_unique_arg(args);
    int vnum, rnum;

    if (reg.type == Register::NUMBER) 
        vnum = reg.toNumber();
    else 
        vnum = arg2room(reg)->number;
    
    rnum = real_room(vnum);
    if (!VALID_RNUM(rnum))
        throw Scripting::CustomException("Invalid room number");
    
    char_from_room(target);
    char_to_room(target, rnum);
    check_horse(target);

    return Register();
}

/*
 * METHODS for mobile-prototypes only
 */
NMI_INVOKE( CharacterWrapper, create, "создает новый экземпляр моба из этого прототипа и помещает в Гостиную" )
{
    METHOD_PROTO_ONLY
    struct char_data *mob = read_mobile(target->nr, REAL);
    ::char_to_room(mob, 0);
    return wrap( mob );
}


/*
 * METHODS for players only
 */
NMI_INVOKE( CharacterWrapper, gain_exp, "(exp, show_msg) набрать exp очков опыта, выводя сообщения игроку, если show_msg true" )
{
    METHOD_PC_ONLY
    int exp = arg_one(args).toNumber();
    bool show_message = arg_two(args).toBoolean();
    ::gain_exp(target, exp, show_message);
    return Register();
}

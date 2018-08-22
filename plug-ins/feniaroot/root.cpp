/* $Id: root.cpp,v 1.1.4.35.6.23 2009/09/05 20:09:58 rufina Exp $
 *
 * ruffina, 2004
 */

#include <iostream>

#include "logstream.h"
#include "dl_math.h"

#include "root.h"
#include "nativeext.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "reglist.h"
#include "characterwrapper.h"
#include "wrappermanager.h"
#include "codesource.h"
#include "subr.h"
#include "handler.h"
#include "wrap_utils.h"
#include "schedulerwrapper.h"

#include "structs.h"
#include "db.h"
#include "comm.h"

using namespace std;
using namespace Scripting;

NMI_INIT(Root, "корневой объект");

/*
 * METHODS
 */
NMI_INVOKE( Root, Map , "конструктор для структуры") 
{
    return Register::handler<IdContainer>();
}

NMI_INVOKE( Root, Array, "конструктор для массива") 
{
    return Register::handler<RegContainer>();
}

NMI_INVOKE( Root, List , "конструктор для списка") 
{
    return Register::handler<RegList>();
}

NMI_INVOKE( Root, print , "вывести строку в системные логи") 
{
    LogStream::sendNotice() << ">> " << args.front().toString() << endl;
    return Register();
}

NMI_INVOKE( Root, min, "минимальное из двух чисел") 
{
    if (args.size( ) != 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    return Register( ::min(args.front( ).toNumber( ), args.back( ).toNumber( )) );
}

NMI_INVOKE( Root, max, "максимальное из двух чисел") 
{
    if (args.size( ) != 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    return Register( ::max(args.front( ).toNumber( ), args.back( ).toNumber( )) );
}

NMI_INVOKE( Root, abs, "модуль числа") 
{
    int x;

    if (args.empty( ))
	throw Scripting::NotEnoughArgumentsException( );

    x = args.front( ).toNumber( );
    return ::abs( x );
}

NMI_INVOKE( Root, dice, "(x, y) x раз кинуть кубик с y гранями") 
{
    RegisterList::const_iterator i;
    int a, b;

    if (args.size( ) < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    a = i->toNumber( );
    i++;
    b = i->toNumber( );

    return Register( ::dice( a, b ) );
}

NMI_INVOKE( Root, number_range , "(x, y) произвольное число в промежутке от x до y") 
{
    RegisterList::const_iterator i;
    int a, b;

    if (args.size( ) < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    a = i->toNumber( );
    i++;
    b = i->toNumber( );

    return Register( ::number_range( a, b ) );
}

NMI_INVOKE( Root, number_percent , "произвольное число от 1 до 100") 
{
    return Register( ::number_percent( ) );
}

NMI_INVOKE( Root, chance , "(x) true если x < .number_percent()") 
{
    int a;

    if (args.size( ) < 1)
	throw Scripting::NotEnoughArgumentsException( );
    
    a = args.front( ).toNumber( );
    return Register( ::chance( a ) );
}

NMI_INVOKE( Root, chanceOneOf, "(x) true если .number_range(1, x) == 1") 
{
    if (args.size( ) < 1)
	throw Scripting::NotEnoughArgumentsException( );
    
    return Register( ::number_range( 1, args.front( ).toNumber( ) ) == 1);
}

NMI_INVOKE( Root, set_bit, "(mask, b) вернет mask с установленными битом b (логическое 'или')") 
{
    RegisterList::const_iterator i;
    int a, bit;

    if (args.size( ) < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    a = i->toNumber( );
    i++;
    bit = i->toNumber( );

    return a | bit;
}

NMI_INVOKE( Root, unset_bit, "(mask, b) вернет mask со сброшенным битом b") 
{
    RegisterList::const_iterator i;
    int a, bit;

    if (args.size( ) < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    a = i->toNumber( );
    i++;
    bit = i->toNumber( );

    return a & ~bit;
}

NMI_INVOKE( Root, isset_bit, "(mask, b) true если бит b установлен в mask (логическое 'и')") 
{
    RegisterList::const_iterator i;
    int a, bit;

    if (args.size( ) < 2)
	throw Scripting::NotEnoughArgumentsException( );
    
    i = args.begin( );
    a = i->toNumber( );
    i++;
    bit = i->toNumber( );

    return a & bit;
}

NMI_INVOKE( Root, eval , "выполнить выражение указанное в строке") 
{
    if (args.empty())
	throw Scripting::NotEnoughArgumentsException( );
    
    const DLString &src = args.front().toString();
    Scripting::CodeSource &cs = Scripting::CodeSource::manager->allocate();
    
    cs.author = "<unknown>";
    cs.name = "<recursive eval>";

    cs.content = src;

    return cs.eval(Register( ));
}

NMI_INVOKE(Root, api, "печатает этот API" )
{
    ostringstream buf;
    
    Scripting::traitsAPI<Root>( buf );
    return Register( buf.str( ) );
}

NMI_INVOKE(Root, mob_proto_real, "вернет прототип моба по внутреннему номеру")
{
    int rnum = args2number(args);

    if (rnum < 0 || rnum > top_of_mobt)
        throw CustomException("Invalid mob real number");

    return wrap( &mob_proto[rnum] );

}

NMI_INVOKE(Root, mob_proto_virtual, "вернет прототип моба по номеру заданному в зоне")
{
    int rnum = real_mobile(args2number(args));

    if (rnum < 0)
        throw CustomException("Invalid mob virtual number");

    return wrap( &mob_proto[rnum] );
}

NMI_INVOKE(Root, obj_proto_real, "вернет прототип предмета по внутреннему номеру")
{
    int rnum = args2number(args);

    if (rnum < 0 || rnum > top_of_objt)
        throw CustomException("Invalid obj real number");

    return wrap( &obj_proto[rnum] );

}

NMI_INVOKE(Root, obj_proto_virtual, "вернет прототип предмета по номеру заданному в зоне")
{
    int rnum = real_object(args2number(args));

    if (rnum < 0)
        throw CustomException("Invalid obj virtual number");

    return wrap( &obj_proto[rnum] );
}

NMI_INVOKE(Root, room_virtual, "вернет комнату по номеру заданному в зоне")
{
    int rnum = real_room(args2number(args));

    if (rnum < 0)
        throw CustomException("Invalid room virtual number");

    return wrap( &world[rnum] );
}

#if 0
NMI_INVOKE(Root, gecho, "сообщение всем" )
{
    Descriptor *d;

    if (args.empty())
	throw Scripting::NotEnoughArgumentsException( );
    
    DLString txt = args.front().toString() + "\r\n";
    
    for (d = descriptor_list; d != 0; d = d->next)
	if (d->connected == CON_PLAYING && d->character)
	    d->character->send_to( txt.c_str( ) );
    
    return Register( );
}
#endif


NMI_INVOKE(Root, object, "(cистемное) поиск феневого объекта" )
{
    Scripting::Object::id_t id;

    if (args.empty( ))
       throw Scripting::NotEnoughArgumentsException( );

    id = args.front( ).toNumber( );
    Scripting::Object::Map::iterator i = Scripting::Object::manager->find( id );

    if (i == Scripting::Object::manager->end( ))
       return Register( );

    return Register(&*i);
}

/*
 * FIELDS
 */
NMI_GET( Root, character_list , "список всех чаров, поле чара next указывает на следующего") 
{
    return WrapperManager::getThis( )->getWrapper(character_list); 
}


NMI_SET( Root, tmp, "") {
    this->tmp = arg;
    self->changed();
}
NMI_GET( Root, tmp, "") {
    return tmp;
}
    
NMI_GET( Root, scheduler , "объект-планировщик")
{
    if(scheduler.type == Register::NONE) {
	scheduler = Register::handler<SchedulerWrapper>();
	self->changed();
    }

    return scheduler;
}

NMI_GET( Root, current_time, "текущее время в секундах") 
{
    return Register((int)time(NULL));
}


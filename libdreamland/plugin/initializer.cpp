/* $Id: initializer.cpp,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 * 
 * ruffina, Dream Land, 2008
 */
#include <stdlib.h>
#include "initializer.h"
#include "sharedobject.h"

Initializer::Initializer(int prio = INITPRIO_NORMAL)
{
    if(!SharedObject::current)
        abort();

    priority = prio;

    SharedObject::current->addInit(this);
}

Initializer::~Initializer()
{
    if(!SharedObject::current)
        abort();

    SharedObject::current->delInit(this);
}



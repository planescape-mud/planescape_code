/* $Id: feniamanager.cpp,v 1.1.2.9.6.2 2007/09/11 00:01:14 margo Exp $
 *
 * ruffina, 2004
 */
#include <fstream>

#include "class.h"
#include "planescape.h"

#include "fenia/register-impl.h"
#include "fenia/context.h"

#include "feniamanager.h"
#include "schedulerwrapper.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "reglist.h"

using namespace Scripting;

/*----------------------------------------------------------------
 * FeniaManager
 *----------------------------------------------------------------*/
WrapperManagerBase::Pointer FeniaManager::wrapperManager;
FeniaManager * FeniaManager::thisClass = 0;

FeniaManager::FeniaManager() 
{
    checkDuplicate( thisClass );
    thisClass = this;

    current = this;
    scope = NULL;

    Class::regMoc<SchedulerWrapper>( );
    Class::regMoc<FeniaProcess>( );
    Class::regXMLVar<IdContainer>( );
    Class::regXMLVar<RegContainer>( );
    Class::regXMLVar<RegList>( );
    Class::regMoc<RegListCall>( );
}


FeniaManager::~FeniaManager() 
{
    Class::unregMoc<SchedulerWrapper>( );
    Class::unregMoc<FeniaProcess>( );
    Class::unregXMLVar<IdContainer>( );
    Class::unregXMLVar<RegContainer>( );
    Class::unregXMLVar<RegList>( );
    Class::unregMoc<RegListCall>( );

    if(current == this)
	current = NULL;

    thisClass = 0;
}

DbEnvContext *
FeniaManager::getDbEnv( ) const
{
    return mud->getDbEnv( );
}


/* $Id: wrappersplugin.h,v 1.1.4.2 2004/10/29 14:46:17 rufina Exp $
 *
 * ruffina, 2004
 */

#ifndef WRAPPERSPLUGIN_H
#define WRAPPERSPLUGIN_H

#include "plugin.h"
#include "xmlvariablecontainer.h"

class WrappersPlugin : public Plugin, public XMLVariableContainer {
XML_OBJECT
public:        
        typedef ::Pointer<WrappersPlugin> Pointer;
        
public:
        WrappersPlugin( );
        virtual ~WrappersPlugin( );
        
        virtual void initialization( );
        virtual void destruction( );
        
        static inline WrappersPlugin* getThis( ) {
            return thisClass;
        }
        
        static void linkTargets();

private:
        static WrappersPlugin* thisClass;
};

#endif

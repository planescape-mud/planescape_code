/* $Id: classselfregistratorplugin.h,v 1.1.2.2 2009/10/11 18:35:37 rufina Exp $
 * 
 * ruffina, Dream Land, 2008
 */

#ifndef __CLASSSELFREGISTRATORPLUGIN_H__
#define __CLASSSELFREGISTRATORPLUGIN_H__

template <char *&tn>
class ClassSelfRegistratorPlugin :
        public virtual XMLPolymorphVariable, 
        public Class::ClassRegistrator, 
        public virtual Plugin 
{
public:
    typedef ::Pointer<ClassSelfRegistratorPlugin> Pointer;

    ClassSelfRegistratorPlugin() {
        typeName = tn;
    }

    virtual AllocateClass::Pointer clone( ) const {
        return Pointer(this);
    }
    virtual const DLString & getType( ) const {
	return typeName;
    }
protected:
    virtual void initialization( ) 
    {
        Class::regClass(typeName, Pointer(this));
    }
    virtual void destruction( ) 
    {
        Class::unRegClass(typeName);
    }
private:
    DLString typeName;
};

#endif

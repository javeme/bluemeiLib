#include "Object.h"
#include "Class.h"
#include "ClassField.h"
#include "BString.h"

namespace blib{

typedef Object Self;

Object::Object(void)
{
}

Object::~Object(void)
{
}

String Object::toString() const
{
	return String::format("Object:%p",this);
}

void Object::init( Object *pObj )
{
	;
}

void Object::release()
{
	delete this;
}


const Class* Object::thisClass()
{
	//@TODO: does it need to throw exception if exists the class
	return Class::registerClassIfNotExist("Object",Self::createObject,null);
}

Object* Object::createObject()
{
	return new Self();
}

Object* Object::clone() const
{
	return new Self(*this);
}

const Class* Object::getThisClass() const
{
	return thisClass();
}

void Object::setAttribute(cstring name, Object* val)
{
	const FieldInfo* fldInfo = getThisClass()->getField(name);
	return fldInfo->setValue(*this, val);
}

Object* Object::getAttribute(cstring name) const
{
	const FieldInfo* fldInfo = getThisClass()->getField(name);
	return fldInfo->getValue(*this);
}

const void* Object::getAttributeAddr(cstring name) const
{
	const FieldInfo* fldInfo = getThisClass()->getField(name);
	return fldInfo->getAddr(*this);
}

}//end of namespace blib

#include "Converter.h"
#include "RuntimeException.h"

namespace blib{

void throwBadCastException(cstring from, cstring to)
{
	throwpe(BadCastException(from, to));
}

void throwTypeException(const Object* instance, cstring cls)
{
	checkNullPtr(instance);
	throwpe(TypeException(instance->toString(), cls));
}

void checkNullObject(Object* obj)
{
	checkNullPtr(obj);
}

}//end of namespace blib

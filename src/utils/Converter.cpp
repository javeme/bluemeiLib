#pragma once
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

}//end of namespace blib

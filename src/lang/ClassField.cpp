#include "ClassField.h"
#include "BString.h"
#include "RuntimeException.h"

namespace bluemei{

void registerFieldCheck(bool success, cstring name)
{
	if(!success)
	{
		throwpe(RuntimeException(String("failed to register field: ") + name));
	}
}

}
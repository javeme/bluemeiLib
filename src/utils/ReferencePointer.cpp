#include "stdafx.h"
#include "ReferencePointer.h"

namespace bluemei{


ReferencePointer::ReferencePointer(void)
{
	m_nPtrRefCount=1;
}


ReferencePointer::~ReferencePointer(void)
{
}

void ReferencePointer::attach()
{
	m_nPtrRefCount++;
}
ReferencePointer* ReferencePointer::disattach()
{
	m_nPtrRefCount--;
	if(m_nPtrRefCount<=0)
	{
		delete this;
		return NULL;
	}
	return this;
}

}//end of namespace bluemei
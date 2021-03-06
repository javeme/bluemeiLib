#include "Ptr.h"

namespace blib{


PointerReference::PointerReference(void)
{
	m_nPtrRefCount = 0;
}

PointerReference::~PointerReference(void)
{
}

void PointerReference::attach()
{
	m_nPtrRefCount++;
}

PointerReference* PointerReference::disattach()
{
	m_nPtrRefCount--;
	assert(m_nPtrRefCount >= 0);
	if(m_nPtrRefCount == 0)
	{
		delete this;
		return NULL;
	}
	return this;
}

}//end of namespace blib

#ifndef Iterator_H_H
#define Iterator_H_H

#include "bluemeiLib.h"
#include "Ptr.h"

namespace blib{

template <class T>
class Iterator : public PointerReference
{
public:
	virtual ~Iterator(){}
	virtual bool hasNext()=0;
	virtual T next()=0;
	virtual bool remove()=0;
};

}//end of namespace blib

#endif

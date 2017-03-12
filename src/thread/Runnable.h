#ifndef _Runnable_H_H
#define _Runnable_H_H

#include "bluemeiLib.h"
#include "Object.h"

namespace blib{


class BLUEMEILIB_API Runnable : public Object
{
public:
	virtual void run()=0;
};


}//end of namespace blib

#endif

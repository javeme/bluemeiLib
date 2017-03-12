#ifndef _Task_H_H
#define _Task_H_H

#include "Runnable.h"

namespace blib{

class BLUEMEILIB_API Task : public Runnable
{
public:
	virtual void stop()=0;
public:
	virtual bool isRunning()=0;
	virtual void taskStarted()=0;
	virtual void taskFinished()=0;
	virtual int getId()=0;
	virtual void setId(int id)=0;
};

}//end of namespace blib

#endif

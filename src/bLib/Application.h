#pragma once
#include "BeanContainer.h"

namespace blib{

class BLUEMEILIB_API Application : public BeanContainer
{
public:
	Application(void);
	virtual ~Application(void);
protected:
	Application(const Application& other){};
	Application& operator=(const Application& other){return *this;};
};



}//end of namespace blib
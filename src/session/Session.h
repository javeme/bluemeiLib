#ifndef Session_H_H
#define Session_H_H

#include "BeanContainer.h"

namespace blib{

class BLUEMEILIB_API Session : public BeanContainer
{
public:
	Session(void);
	/*Session(Session&& other){
		__super::operator=(other);
	};*/
	virtual ~Session(void);
protected:
	Session(const Session& other){*this=other;};
	Session& operator=(const Session& other){
		BeanContainer::operator=(other);
		return *this;
	};
};

}//end of namespace blib

#endif

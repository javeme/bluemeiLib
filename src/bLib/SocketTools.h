#ifndef SocketTools_H_H
#define SocketTools_H_H

#include "bluemeiLib.h"
#include "CodeUtil.h"

namespace blib{

class BLUEMEILIB_API SocketTools : public CodeUtil
{
public:
	static String getWinsockErrorMsg(int errCode);

	static int initSocketContext();
	static int cleanUpSocketContext();
};

}//end of namespace blib

#endif

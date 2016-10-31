#include "LambdaThread.h"

namespace blib{

LambdaThread::LambdaThread(const ThreadFunction& f,void* pUserParameter)
	:Thread(pUserParameter), m_lambdaFun(f)
{
	//m_pFun=new Function(move(f));
	this->setAutoDestroy(true);
}

LambdaThread::~LambdaThread()
{
	//if(m_pFun!=nullptr)
	//	delete m_pFun;
}

void LambdaThread::run()
{
	//(*m_pFun)(pThreadParameter->pUserParameter);
	m_lambdaFun();
}

}//end of namespace blib
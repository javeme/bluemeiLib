#include "LambdaThread.h"

namespace bluemei{

LambdaThread::LambdaThread(const ThreadFunction& f,void* pUserParameter ):Thread(pUserParameter),m_lambdaFun(f)
{
	//m_pFun=new Function(move(f));
	this->setAutoDestroy(true);
}

LambdaThread::~LambdaThread()
{
	//if(m_pFun!=nullptr)
	//	delete m_pFun;
}

void LambdaThread::run( ThreadParameter *pThreadParameter )
{
	//(*m_pFun)(pThreadParameter->pUserParameter);
	m_lambdaFun(pThreadParameter->pUserParameter);
}

}//end of namespace bluemei
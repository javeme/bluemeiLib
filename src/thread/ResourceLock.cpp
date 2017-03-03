#include "ResourceLock.h"
#include "RuntimeException.h"
#include "AccurateTimeTicker.h"

namespace blib {

#ifdef WIN32

ResourceLock::ResourceLock(unsigned int count,cstring name)
	: m_semaphore( {NULL, 0})
{
	this->init(count,DEFAULT_MAX_COUNT,name);
}

ResourceLock::~ResourceLock(void)
{
	this->destroy();
}

ResourceLock::ResourceLock(ResourceLock&& other)
{
	*this=std::move(other);
}

ResourceLock& ResourceLock::operator=(ResourceLock&& other)
{
	this->destroy();
	this->m_semaphore=std::move(other.m_semaphore);
	other.m_semaphore={NULL, 0};

	return *this;
}

void ResourceLock::notify() throw (Exception)
{
	if (this->getWaitCount() > 0)
		this->signal();
}

// notify all objects (different from java's notifyAll, this will wake up all)
void ResourceLock::notifyAll()
{
	this->signal(this->getWaitCount());
}

void ResourceLock::init(unsigned int count,unsigned int maxCount,cstring name)
{
	m_semaphore.waitcount=0;

	m_semaphore.handle=::CreateSemaphoreA(NULL,count,maxCount,name);
	if(m_semaphore==0)
	{
		int error=::GetLastError();
		String str=String::format("ResourceLock: failed to init semaphore: %d",error);
		throw Exception(str);
	}
}

void ResourceLock::destroy()
{
	if(m_semaphore.handle!=NULL)
	{
		BOOL success=::CloseHandle(m_semaphore.handle);
		assert(success);
		m_semaphore.handle=NULL;
	}
}

bool ResourceLock::wait(unsigned long ms/*=INFINITE*/)
{
	//TODO: 等待的线程数(跨进程时如何保证wait count共享???)
	m_semaphore.waitcount++; //原子操作

	unsigned long result=::WaitForSingleObject(m_semaphore,ms);
	if(result==WAIT_FAILED || result==WAIT_ABANDONED)
	{
		int error=::GetLastError();
		String str=String::format("ResourceLock: failed to wait for object: %d",error);
		throw Exception(str);
	}

	m_semaphore.waitcount--;

	return result!=WAIT_TIMEOUT; //没有超时
}

unsigned int ResourceLock::signal(unsigned int count) throw(Exception)
{
	long previousCount=0;
	//增加一个信号
	BOOL success=::ReleaseSemaphore(m_semaphore,count,&previousCount);
	if(!success)
	{
		int error=::GetLastError();
		String str=String::format("ResourceLock: failed to release semaphore: %d",error);
		throw Exception(str);
	}
	return (unsigned int)previousCount+1;
}

unsigned int ResourceLock::getWaitCount() const
{
	return m_semaphore.waitcount.load();
	//long count=0;
	//bool isSuccess=::ReleaseSemaphore(m_semaphore,0,&count);
	//return count;
}

#else // not WIN32

///////////////////////////////////////////////////////////////////////////////
// semaphore_t implement: http://linux.die.net/man/3/pthread_mutexattr_init
/*
 semaphore_t *semaphore_create(char *semaphore_name);
 semaphore_t *semaphore_open(char *semaphore_name);
 void semaphore_post(semaphore_t *semap);
 void semaphore_wait(semaphore_t *semap);
 void semaphore_close(semaphore_t *semap);
 */

semaphore_t* semaphore_create(const char *semaphore_name,
		unsigned int count=0) {
	int fd = -1;
	int flag = 0;
	int status;
	semaphore_t *semap;
	pthread_mutexattr_t psharedm;
	pthread_condattr_t psharedc;

	if (semaphore_name == null || strlen(semaphore_name) == 0) {
		semap = (semaphore_t *) mmap(NULL, sizeof(semaphore_t),
				PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, fd, 0);
		flag = PTHREAD_PROCESS_PRIVATE;
	}
	else {
		fd = open(semaphore_name, O_RDWR | O_CREAT | O_EXCL, 0666);
		if (fd < 0)
			return (NULL);

		(void) ftruncate(fd, sizeof(semaphore_t));

		semap = (semaphore_t*) mmap(NULL, sizeof(semaphore_t),
				PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		close(fd);
		flag = PTHREAD_PROCESS_SHARED;
	}
	checkNullPtr(semap);

	(void) pthread_mutexattr_init(&psharedm);
	(void) pthread_mutexattr_setpshared(&psharedm, flag);
	(void) pthread_condattr_init(&psharedc);
	(void) pthread_condattr_setpshared(&psharedc, flag);

	status = pthread_mutex_init(&semap->lock, &psharedm);
	assert(status == 0);
	status = pthread_cond_init(&semap->nonzero, &psharedc);
	assert(status == 0);

	semap->count = count;
	semap->waitcount = 0;
	return (semap);
}

semaphore_t* semaphore_open(char *semaphore_name) {
	int fd;
	semaphore_t *semap;

	fd = open(semaphore_name, O_RDWR, 0666);
	if (fd < 0)
		return (NULL);
	semap = (semaphore_t*) mmap(NULL, sizeof(semaphore_t),
	PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	return (semap);
}

int semaphore_post(semaphore_t *semap, unsigned int count,
		unsigned int& previous) {
	int status = 0;
	pthread_mutex_lock(&semap->lock);
	if (semap->count == 0)
		status = pthread_cond_signal(&semap->nonzero);
	previous = semap->count;
	semap->count += count;
	pthread_mutex_unlock(&semap->lock);
	return status;
}

int semaphore_wait(semaphore_t *semap, unsigned long ms) {
	int status = 0;
	semap->waitcount++;

	pthread_mutex_lock(&semap->lock);

	if (ms == INFINITE) {
		while (semap->count == 0 && status == 0)
			status = pthread_cond_wait(&semap->nonzero, &semap->lock);
	} else {
		timeval offset = { ms / 1000, (ms % 1000) * 1000 };
		timeval now; (void) gettimeofday(&now, NULL);
		timeradd(&now, &offset, &offset);
		timespec timeout = { offset.tv_sec, offset.tv_usec * 1000 };
		while (semap->count == 0 && status == 0)
			status = pthread_cond_timedwait(&semap->nonzero, &semap->lock,
					&timeout);
	}
	semap->count--;
	pthread_mutex_unlock(&semap->lock);

	semap->waitcount--;
	return status;
}

void semaphore_close(semaphore_t *semap) {
	munmap((void*) semap, sizeof(semaphore_t));
}

///////////////////////////////////////////////////////////////////////////////
// class ResourceLock
ResourceLock::ResourceLock(unsigned int count, cstring name) :
		m_semaphore(null) {
	this->init(count, DEFAULT_MAX_COUNT, name);
}

ResourceLock::~ResourceLock(void) {
	this->destroy();
}

ResourceLock::ResourceLock(ResourceLock&& other) {
	*this = std::move(other);
}

ResourceLock& ResourceLock::operator=(ResourceLock&& other) {
	this->destroy();
	this->m_semaphore = other.m_semaphore;
	other.m_semaphore = null;

	return *this;
}

void ResourceLock::notify() throw (Exception) {
	if (this->getWaitCount() > 0)
		this->signal();
}

// notify all objects (different from java's notifyAll, this will wake up all)
void ResourceLock::notifyAll() throw (Exception) {
	this->signal(this->getWaitCount());
}

void ResourceLock::init(unsigned int count, unsigned int maxCount,
		cstring name) {
	// TODO: support semaphore open
	m_semaphore = semaphore_create(name, count);
	if (m_semaphore == null) {
		throw Exception(
				String::format("ResourceLock: failed to init semaphore: %s",
						strerror(errno)));
	}
}

void ResourceLock::destroy(void) {
	if (m_semaphore != null) {
		semaphore_close(m_semaphore);
		m_semaphore = null;
	}
}

bool ResourceLock::wait(unsigned long ms/*=INFINITE*/) throw (Exception) {
	checkNullPtr(m_semaphore);
	int result = semaphore_wait(m_semaphore, ms);
	if (result == 0 || result == ETIMEDOUT) {
		return result != ETIMEDOUT; // is it not timeout?
	}
	else if (result == EINVAL || result == EPERM) {
		throw Exception(
				String::format("ResourceLock: failed to wait for semaphore: %s",
						strerror(errno)));
	}
	else {
		assert(result == 0);
		return true;
	}
}

unsigned int ResourceLock::signal(unsigned int count) throw (Exception) {
	checkNullPtr(m_semaphore);
	unsigned int previousCount = 0;
	int result = semaphore_post(m_semaphore, count, previousCount);
	if (result != 0) {
		throw Exception(
				String::format("ResourceLock: failed to release semaphore: %s",
						strerror(errno)));
	}
	return previousCount + 1;
}

unsigned int ResourceLock::getWaitCount() const {
	checkNullPtr(m_semaphore);
	return m_semaphore->waitcount.load();
}

#endif

}//end of namespace blib

#ifndef Atomic_H_H
#define Atomic_H_H

// MSVC < VS2012
#if defined(_MSC_VER) && _MSC_VER < 1900

// implements atomic for VS2010
namespace std
{

template <typename T>
class atomic
{
public:
	atomic(const T& count=0) : m_count(count) {}
	virtual ~atomic() {}

public:
	operator T() const {
		return m_count;
	}

	atomic& operator=(const T& count) {
		m_count = count;
		return *this;
	}

	atomic& operator++() {
		InterlockedIncrement(&m_count);
		return *this;
	}

	atomic& operator--() {
		InterlockedDecrement(&m_count);
		return *this;
	}
	
private:
	volatile T m_count;
};

} // end of namespace std

#else

#include <atomic>

#endif // end of _MSC_VER

#endif

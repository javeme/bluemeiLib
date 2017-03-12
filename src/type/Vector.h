#ifndef Vector_H_H
#define Vector_H_H

#include "bluemeiLib.h"
#include "RuntimeException.h"

namespace blib{

//模板类无法被dll导出 BLUEMEILIB_API
template <class T>
class BLUEMEILIB_TEMPLATE Vector : public Object// : public vector<T>
{
public:
	Vector(void);
	virtual ~Vector(void);
public:
    Vector(const Vector& other);
    Vector& operator=(const Vector& other);
public:
	Vector(Vector&& other);
	Vector& operator=(Vector&& other);
private:
	std::vector<T> m_dataArray;
public:
	unsigned int add(const T& ele);
	bool insert(unsigned int pos, const T& ele);
	bool remove(unsigned int pos,T& value);
	bool pop(T& value);
	bool remove(const T& ele);
	bool get(unsigned int pos,T& value) const;
	T& operator[](unsigned int pos);
	const T& operator[](unsigned int pos) const;
	unsigned int size() const;
	void clear();
};


template <class T>
Vector<T>::Vector(void)
{
	//dataArray.resize(10);
}

template <class T>
Vector<T>::~Vector(void)
{
	;
}

template <class T>
Vector<T>::Vector(const Vector& other)
{
    *this=other;
}

template <class T>
Vector<T>& Vector<T>::operator=(const Vector& other)
{
    this->m_dataArray=other.m_dataArray;
    return *this;
}

template <class T>
Vector<T>::Vector(Vector&& other)
{
	*this=std::move(other);
}

template <class T>
Vector<T>& Vector<T>::operator=(Vector&& other)
{
	this->m_dataArray=std::move(other.m_dataArray);
	return *this;
}

template <class T>
unsigned int Vector<T>::add(const T& ele)
{
	m_dataArray.push_back(ele);
	return this->size();
}

template <class T>
bool Vector<T>::insert(unsigned int pos, const T& ele)
{
	if(pos<0||pos>m_dataArray.size())
		return false;
	m_dataArray.insert(m_dataArray.begin()+pos, ele);
	return true;
}

template <class T>
bool Vector<T>::remove(unsigned int pos,T& value)
{
	if(pos<0||pos>=m_dataArray.size())
		return false;
	typename std::vector<T> ::iterator iter = m_dataArray.begin();
	advance(iter,pos);
	value=*iter;
	m_dataArray.erase(iter);
	return true;
}

template <class T>
bool Vector<T>::pop(T& value)
{
	return remove(size()-1, value);
}

template <class T>
bool Vector<T>::get(unsigned int pos,T& value) const
{
	if(pos<0||pos>=m_dataArray.size())
		return false;
	value=m_dataArray[pos];
	return true;
}

template <class T>
T& Vector<T>::operator[](unsigned int pos)
{
	if(pos<0||pos>=m_dataArray.size())
	{
		char buf[100];
		snprintf(buf,sizeof(buf),"Vector size is %u, can't access %d.",
		        m_dataArray.size(),pos);
		throw OutOfBoundException(buf);
	}
	return m_dataArray[pos];
}

template <class T>
const T& Vector<T>::operator[](unsigned int pos) const
{
	if(pos<0||pos>=m_dataArray.size())
	{
		char buf[100];
		sprintf_s(buf,"Vector size is %d, can't access %d.",m_dataArray.size(),pos);
		throw OutOfBoundException(buf);
	}
	return m_dataArray[pos];
}

template <class T>
bool Vector<T>::remove(const T& ele)
{
	typename std::vector<T>::iterator iter = find(m_dataArray.begin(),
			m_dataArray.end(), ele);
	if (iter != m_dataArray.end())
	{
		m_dataArray.erase(iter);
		return true;
	}
	return false;
}

template <class T>
unsigned int Vector<T>::size() const
{
	return m_dataArray.size();
}

template <class T>
void Vector<T>::clear()
{
	m_dataArray.clear();
}

}//end of namespace blib

#endif

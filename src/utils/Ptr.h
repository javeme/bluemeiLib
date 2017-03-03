#ifndef _Ptr_H_H
#define _Ptr_H_H

#include "bluemeiLib.h"
#include "Object.h"


namespace blib{

class BLUEMEILIB_API PointerReference : virtual public Object
{
public:
	PointerReference(void);
	virtual ~PointerReference(void);

public:
	PointerReference(PointerReference&& other) {
		*this = std::move(other);
	}

	PointerReference& operator=(PointerReference&& other) {
		this->m_nPtrRefCount = other.m_nPtrRefCount;
		other.m_nPtrRefCount = 0;
		return *this;
	}

private:
	explicit PointerReference(const PointerReference&);
	PointerReference&  operator=(const PointerReference&);

public:
	void attach();
	PointerReference* disattach();

protected:
	int m_nPtrRefCount;
};


template<typename T>
class BLUEMEILIB_TEMPLATE Pointer : public Object
{
public:
	Pointer(T* target=null) : m_target(target) {}
	Pointer(const Pointer& other) : m_target(other.m_target) {}

public:
	Pointer(Pointer&& other) {
		*this = std::move(other);
	}

	Pointer& operator=(Pointer&& other) {
		this->m_target = other.m_target;
		other.m_target = null;
		return *this;
	}

protected:
	Pointer& operator=(const Pointer& other) {
		this->m_target = other.m_target;
		return *this;
	}

public:
	virtual bool operator==(T* target) const {
		return this->m_target == target;
	}

	virtual bool operator!=(T* target) const {
		return this->m_target != target;
	}

	virtual bool operator==(const Pointer& other) const {
		return m_target == other.m_target;
	}

	virtual bool operator!=(const Pointer& other) const {
		return m_target != other.m_target;
	}

	operator T* () const {
		return m_target;
	}

	T* operator->() const {
		return m_target;
	}

	T* detach() {
		T* tmp = m_target;
		m_target = null;
		return tmp;
	}

protected:
	T* m_target;
};


//Wrapper for PointerReference
template<typename T>
class BLUEMEILIB_TEMPLATE RefPointer : public Pointer<T>
{
public:
	RefPointer(T* target) : Pointer<T>(target) {
		if(this->m_target != null)
		    this->m_target->attach();
	}

	RefPointer(const RefPointer& other) : Pointer<T>(other) {
		if(this->m_target != null)
		    this->m_target->attach();
	}

	virtual ~RefPointer() {
		if(this->m_target != null)
		    this->m_target->disattach();
	}

	RefPointer& operator=(const RefPointer& other) {
		if(this->m_target != null)
			this->m_target->disattach();
		Pointer<T>::operator=(other);
		if (this->m_target != null)
			this->m_target->attach();
		return *this;
	}

public:
	RefPointer(RefPointer&& other) {
		*this = std::move(other);
	}

	RefPointer& operator=(RefPointer&& other) {
		if(this->m_target != null)
			this->m_target->disattach();
		Pointer<T>::operator=(std::move(other));
		return *this;
	}
};


template<typename T>
class BLUEMEILIB_TEMPLATE ScopePointer : public Pointer<T>
{
public:
	ScopePointer(T* target=null) : Pointer<T>(target) {}

	virtual ~ScopePointer(void) {
		delete this->m_target;
	}

	ScopePointer& operator=(T* target) {
		delete this->m_target;
		this->m_target = target;
		return *this;
	}

public:
	ScopePointer(ScopePointer&& other) {
		*this = std::move(other);
	}

	ScopePointer& operator=(ScopePointer&& other) {
		delete this->m_target;
		Pointer<T>::operator=(std::move(other));
		return *this;
	}

private:
	explicit ScopePointer(const ScopePointer& other);
	ScopePointer& operator=(const ScopePointer& other) const;
};

}//end of namespace blib

#endif

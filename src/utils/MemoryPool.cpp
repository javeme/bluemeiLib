#include "MemoryPool.h"
#include "RuntimeException.h"

namespace blib{

/////////////////////////////////////////////////////////////////////
// class GradeMemoryPool
template <unsigned int N>
struct chars_t
{
	char data[N];
	inline operator char*() { return data; }
};

template <typename T>
T* newn(unsigned int n)
{
	return new T[n];
}

template <typename T>
void deleten(T* array)
{
	delete[] array;
}

#define sizeOfGrade(grade) (1 << (grade))
#define chars(grade) chars_t<sizeOfGrade(grade)>


// class MemoryPool: one Pool per grade
template <unsigned int N>
class GradeMemoryPool : public MemoryPool
{
protected:
	typedef chars(N) Item; // a buffer with N bytes size
	typedef chars(N)* Block;
	typedef std::vector<Block> Blocks;
	typedef std::list<Item*> Items; // list of chars[n]*

	Blocks m_blocks; // block data (one Block == m_size * Item)
	Items m_items; // pointers to Item
	unsigned int m_sizePerBlock; // number of Items per Block
	unsigned int m_capacity; // max number of Items

	bool m_needThrow;
	CriticalLock m_mutexLock;
public:
	GradeMemoryPool(unsigned int size, unsigned int capacity)
		: m_blocks(1, newn<Item>(size)),
		  m_sizePerBlock(size),
		  m_capacity(capacity),
		  m_needThrow(true)
	{
		Block firstBlock = m_blocks[0];
		for(unsigned int i = 0; i < size; i++)
		{
			m_items.push_back(&firstBlock[i]);
		}
		init(size, capacity);
	}

	virtual ~GradeMemoryPool()
	{
		destroy();
	}

public:
	void* get()
	{
		ScopedLock scopedLock(m_mutexLock);

		if(m_items.size() == 0) {
			growBlock();printf("============>blocks: %d\n", m_blocks.size());
		}

		if(m_items.size() > 0) {
			void* item = m_items.front();
			m_items.pop_front();
			return item;
		}
		else {
			if(m_needThrow)
				throw NullPointerException("GradeMemoryPool out of memory.");

			//assert(0);
			return null;
		}
	}

	void release(void* ptr)
	{
		ScopedLock scopedLock(m_mutexLock);

		if(m_items.size() > m_capacity) {
			// TODO: destroy some items (or block)
		}
		m_items.push_back((Item*)ptr);
	}
protected:
	void init(unsigned int initSize, unsigned int maxSize) 
	{
	}

	void destroy()
	{
		ScopedLock scopedLock(m_mutexLock);

		// destroy blocks
		for(unsigned int i = 0; i < m_blocks.size(); i++)
			deleten<Item>(m_blocks[i]);
		m_items.clear();
	}

	void growBlock()
	{
		ScopedLock scopedLock(m_mutexLock);

		Block block = newn<Item>(m_sizePerBlock);
		m_blocks.push_back(block);
		// fill items
		for(unsigned int i = 0; i < m_sizePerBlock; i++)
		{
			m_items.push_back(&block[i]);
		}
	}

	unsigned int available() 
	{
		ScopedLock scopedLock(m_mutexLock);

		return m_items.size();
	}

	virtual bool changeMaxSize(unsigned int maxSize) 
	{
		ScopedLock scopedLock(m_mutexLock);

		this->m_capacity = maxSize;
		return true;
	}

	virtual void enableThrow(bool needThrow) 
	{
		ScopedLock scopedLock(m_mutexLock);

		m_needThrow = needThrow;
	}

};


/////////////////////////////////////////////////////////////////////
// class GradeMemoryPools
GradeMemoryPools::GradeMemoryPools(unsigned int bytesPerPool,
	unsigned int capacityPerPool/*=0*/)
{
	init(bytesPerPool, capacityPerPool);
}

GradeMemoryPools::~GradeMemoryPools()
{
	destroy();
}

void GradeMemoryPools::init(unsigned int sizePerPool,
	unsigned int capacityPerPool)
{
	if(m_pools.size() > 0)
		return;

	if(sizePerPool < sizeOfGrade(GRADE_MAX))
		sizePerPool = sizeOfGrade(GRADE_MAX);

	if(capacityPerPool != 0 && capacityPerPool < sizePerPool)
		capacityPerPool = sizePerPool;

	#define initPool(grade) \
	{ \
		unsigned int size = sizePerPool >> grade; \
		unsigned int capacity = capacityPerPool >> grade; \
		m_pools[grade] = new GradeMemoryPool<grade>(size, capacity); \
	}

	// init GRADE_START ~ GRADE_MAX
	initPool(4); // 16 bytes
	initPool(5);
	initPool(6);
	initPool(7);
	initPool(8);
	initPool(9);
	initPool(10); // 1 kb
}

void GradeMemoryPools::destroy()
{
	// destroy pools
	for(Pools::iterator i = m_pools.begin(); i != m_pools.end(); ++i)
	{
		delete i->second;
	}
	m_pools.clear();
}

MemoryPool* GradeMemoryPools::suitablePool(unsigned int size)
{
	for(unsigned int i = GRADE_START; i <= GRADE_MAX; i++) {
		if(size <= (unsigned int)sizeOfGrade(i)) {
			return m_pools[i];
		}
	}
	return null;
}

void* GradeMemoryPools::alloc(unsigned int size)
{
	MemoryPool* pool = suitablePool(size);
	if(pool) {
		return pool->get();
	}
	else
		return new char[size];
}

void GradeMemoryPools::free(void* ptr, unsigned int size)
{
	MemoryPool* pool = suitablePool(size);
	if(pool)
		pool->release(ptr);
	else
		delete[] (char*)ptr;
}

}//end of namespace blib

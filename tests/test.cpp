
#include "SortArray.h"
#include "HashMap.h"


class MyComper : public Compareable<int>
{
public:
	bool compare(int& ele1,int& ele2)const{ 
		return ele1>ele2; 
	}
};
void testArray()
{
	SortArray<int> a;
	for(int i=0;i<5000;i++)
	{
		a.add(rand()%10000);
	}
	String s1=a.toString();
	MyComper c;
	a.sort(c);
	String s2=a.toString();
}

void testHashMap()
{
	typedef Object ScriptObject;
	ArrayList<SmartPtr<ScriptObject>> m_heapScriptObjectRefrence;
	SmartPtr<ScriptObject> obj=new ScriptObject();
	m_heapScriptObjectRefrence.add(obj);

	/*int maxSize=100;
	typedef Entry<String,SmartPtr<Object>> HashEntry;
	ArrayList<HashEntry*> table(maxSize);
	for(int i=0;i<maxSize;i++)
	{
		String str=String::format("bluemei%d-%0.4f",i,i/0.0674);
		int index=hashCode<cstring>(str)%maxSize;

		while(table[index]!=nullptr)
		{
			index+=1;
			if(index>=table.size())
				index=0;
		}
		table[index]=new HashEntry(str,new String(str));
	}
	String key=table[1]->key;
	int index=hashCode<cstring>(key)%maxSize;
	while(key!=table[index]->key)
	{
		index+=1;
		if(index>=table.size())
			index=0;
	}
	Object* p=table[index]->value;

	String result=table.toString();
	auto list=result.splitWith("<null>");
	//释放table
	for(int i=0;i<table.size();i++)
	{
		delete table[i];
	}*/
	//HashMap<int,double> h;
	int maxSize=100;
	HashMap<String,SmartPtr<Object>> hashMap(maxSize);
	for(int i=0;i<maxSize;i++)
	{
		String str=String::format("%d-what%0.4f",i,i/0.0674);
		hashMap.put(str,new String(str));
	}
	
	auto itor=hashMap.iterator();
	String s;
	while(itor->hasNext())
	{
		s+=itor->next().toString()+",";
	}
	hashMap.releaseIterator(itor);

	s=s.getLeftSub(s.length()-1);
	SmartPtr<Object> v=*hashMap.get("0-what0.0000");
	String result=hashMap.toString();
	int pos=result.find(s);

	for(int i=0;i<maxSize/3;i++)
	{
		String str=String::format("%d-what%0.4f",i,i/0.0674);
		hashMap.remove(str,obj);
	}
	result=hashMap.toString();
	pos=result.find("32-");
}

#include "MultiValueHashMap.h"
template<> Value2String<Entry<String,LinkedList<String>>*>::operator String()const{
	if(value==nullptr)
		return "<null>";
	return value->toString();
}template class Value2String<Entry<String,LinkedList<String>>*>;

void testMVMap()
{
	MultiValueHashMap<String,String> mvMap;
	mvMap.put("lizhmei","18931861012");
	mvMap.put("lizhmei","15810301283");
	mvMap.put("yikong","010-12345678");
	mvMap.put("yikong","010-22345678");
	mvMap.put("yikong","010-22345678");
	mvMap.put("yikong","010-32345678");
	mvMap.put("yikong","010-42345678");
	mvMap.put("yikong","010-52345678");
	mvMap.put("yikong","010-62345678");
	mvMap.put("yikong","010-72345678");
	mvMap.put("yikong","010-82345678");
	mvMap.put("yikong","010-92345678");
	mvMap.put("yikong","010-02345678");
	mvMap.put("hj","13870551634");
	mvMap.put("hj","8420340");
	for(int i=0;i<100;i++)
	{
		String str=String::format("%c%c%c%c",rand()%128,rand()%128,rand()%128,rand()%128);
		mvMap.put(str,String()+i+i+i+i);
	}
	String s=mvMap.toString();
	int pos=sizeof(mvMap)-sizeof(ArrayList<LinkedList<String>>);
	s=((ArrayList<Entry<String,LinkedList<String>>*>*)(((char*)&mvMap)+pos))->toString();
	auto nulllist=s.splitWith("<null>");

	ArrayList<int> l1;
	l1.add(1);
	ArrayList<int> l2=move(l1);
	for(int i=0;i<100;i++)
		l1.add(i);
	s=l1.toString();
}

#include "ByteBuffer.h"

void testByteBuffer()
{
	ByteBuffer bf;
	bool boolVar=false;
	byte byteVar=1;
	char charVar=2;
	short shortVar=3;
	int intVar=4;
	String strVar="test string for ByteBuffer-:)";
	
	bf.writeBoolean(boolVar);
	bf.write(byteVar);
	bf.writeChar(charVar);
	bf.writeShort(shortVar);
	bf.writeInt(intVar);
	bf.writeString(strVar);

	boolVar=bf.readBoolean();
	byteVar=bf.read();
	charVar=bf.readChar();
	shortVar=bf.readShort();
	intVar=bf.readInt();
	strVar=bf.readString();
}


class TestObj : public Object
{
public:
	DECLARE_DCLASS(TestObj)					   														
};

class ChildTestObj : public TestObj
{
public:
	DECLARE_DCLASS(TestObj)					   														
};

void testClass(){
	try
	{
		Object::thisClass()->isMyInstance(new ChildTestObj());
	}
	catch (Exception& e)
	{
		e.printException();
	}	
}



class ObjectTest : public Object
{
public:
	int i;
	virtual void f(){};

	Object()
	{
		i=11;
	}
};
class Object2 : public ObjectTest
{
public:
	Object2(){
		i=12;
	}
	virtual void f2(){};
};
class Object3 : public ObjectTest
{
public:
	Object3(){
		i=13;
	}
	virtual void f3(){};
};

class QQObject : public ObjectTest, public Object2, public Object3
{
public:
	int h;
	virtual void hh(){};
	QQObject(){
		h=22;
	}
};


int testMultObj()
{
	void* p=new QQObject();
	char* s=(char*)p;
	Object *o1=(Object*)static_cast<QQObject*>(p);
	Object *o2=(Object*)(QQObject*)(static_cast<char*>(s));
	Object *o3=static_cast<Object*>(p);

	QQObject* q1=static_cast<QQObject*>(o1);
}

//test
void testLambdaThread(string url,const function<void (int error)>& f)//f为下载完成后的回调函数
{
	auto func=[url,f](void* pUserParameter){
		printf("downloading...\n");
		Sleep(1000*5);
		printf("download finished from %s\n",url.c_str());
		f(0);
	};
	//thread
	Thread *pThread=new LambdaThread(func,nullptr);//下载线程
	pThread->start();
}
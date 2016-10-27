#include "MultiValueHashMap.h"
template<> Value2String<Entry<String,LinkedList<String>>*>::operator String() const{
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
#pragma once
#include "ArrayList.h"

namespace bluemei{

template <class T>
class Compareable
{
public:
	virtual bool compare(T& elem1,T& elem2)const{
		return elem1<elem2;
	}
public:
	static Compareable<T>& default()
	{
		static Compareable<T> defaultComper;
		return defaultComper;
	}
};


template <class T>
class SortArray : public ArrayList<T>
{
public:
	void sort(const Compareable<T>& comper=Compareable<T>::default())
	{
		auto start=m_pData;
		auto end=m_pData+(size()-1);
		quickSort<T>(start,end,comper);
	}

	template <class T>
	static void quickSort(T* start,T* end,const Compareable<T>& comper)
	{
		if(start>=end)
			return;
		T* i=start;
		T* j=end;
		T current=*i;
		while(true)
		{
			//1��β��Ѱ�ұ���С��Ԫ��
			while(j>i && comper.compare(current,*j))//ֱ��current>=*j
				j--;
			if(j>i){
				*i=*j;//ʡ��*j=current;
			}else{
				*i=current;//����2�����һ����
				break;
			}
			//2��ͷ��Ѱ�ұ��Ҵ��Ԫ��
			while(i<j && !comper.compare(current,*i))//ֱ��current<*i
				i++;
			if(j>i){
				*j=*i;
			}else{
				*j=current;//����1�����һ����
				break;
			}
		}
		if(end-start<2)
			return;
		//3���м�����������������
		quickSort(start,i-1,comper);
		quickSort(i+1,end,comper);
	}
};

}//end of namespace bluemei
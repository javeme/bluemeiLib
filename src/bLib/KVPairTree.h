#ifndef KVPairTree_H
#define KVPairTree_H

#include "bluemeiLib.h"
#include "Pair.h"
#include "Exception.h"

namespace bluemei{

template<typename Key,typename Value>
class KVPairTree : public Object
{
public:
	KVPairTree();
	//��ֵ����
	KVPairTree(const Key& k,const Value& v=Value()){		
		clear();
		setSinglePair(k,v);
	}
	//��ֵ����
	KVPairTree(List<KVPairTree>& list){				
		clear();
		bSingleValue=false;
		valueList=list;
	}
	virtual ~KVPairTree();

	//��������
	void setName(const Key& name)	{
		singleValue.key=name;
	}
	//��ȡ����
	Key name()const{
		return singleValue.key;
	}

	//���õ�ֵ�Ե�ֵ
	void setValue(const Value& v){
		bSingleValue=true;
		singleValue.value=v;
	}
	//��ȡ��ֵ�Ե�ֵ
	Value getValue()const{
		return singleValue.value;
	}

	//��ȡ��ֵ��
	Pair<Key,Value>& singlePair(){
		return singleValue;
	}
	void setSinglePair(const Key& k,const Value& v);

	Key getType() const { return type; }
	void setType(const Key& val) { type = val; }

	//������Ŀ
	int itemSize()const{
		return this->valueList.size();
	}
	//�����
	void addItem(const KVPairTree<Key,Value>& v){
		bSingleValue=false;
		this->valueList.push_back(v);
	}

	//�����±��ȡ����
	KVPairTree<Key,Value>& item(unsigned int index) {
		checkBound(index);
		return valueList[index];
	}
	//�������ƻ�ȡ����
	KVPairTree<Key,Value>& item(const Key& k);

	//�Ƴ���
	KVPairTree<Key,Value> removeItem(unsigned int index);
	KVPairTree<Key,Value> removeItem(const Key& k);

	//�������
	void clear() {
		bSingleValue=true;
		valueList.clear();
		singleValue=Pair<Key,Value>();
	}

	//�Ƿ�ΪҶ�ӽڵ�
	bool isSingleValue()const{
		return bSingleValue;
	}

	//�Ƿ����ĳ���ӽڵ�
	bool contain(const Key& k) const;

	//�ж��Ƿ����
	bool operator==(const KVPairTree<Key,Value>& node)const{
		if(bSingleValue==node.bSingleValue && singleValue==node.singleValue && valueList==node.valueList)
			return true;
		else
			return false;
	}
private:
	void checkBound(unsigned int index)const{
		if(index<0 || index>=valueList.size())
			throwpe(OutOfBoundException(index,valueList.size()));
	}
public:
	bool bSingleValue;//�Ƿ�Ϊ��ֵ��
	List<KVPairTree> valueList;//��ֵ���б�
	Pair<Key,Value> singleValue;//��ֵ��
	Key type;//����
};

}//end of namespace bluemei
#endif
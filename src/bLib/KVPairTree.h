#ifndef KVPairTree_H_H
#define KVPairTree_H_H

#include "bluemeiLib.h"
#include "Pair.h"
#include "Exception.h"

namespace blib{

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
		m_bSingleValue=false;
		m_valueList=list;
	}
	virtual ~KVPairTree();

	//��������
	void setName(const Key& name)	{
		m_singleValue.key=name;
	}
	//��ȡ����
	Key name() const{
		return m_singleValue.key;
	}

	//���õ�ֵ�Ե�ֵ
	void setValue(const Value& v){
		m_bSingleValue=true;
		m_singleValue.value=v;
	}
	//��ȡ��ֵ�Ե�ֵ
	Value getValue() const{
		return m_singleValue.value;
	}

	//��ȡ��ֵ��
	Pair<Key,Value>& singlePair(){
		return m_singleValue;
	}
	void setSinglePair(const Key& k,const Value& v);

	Key getType() const { return m_type; }
	void setType(const Key& val) { m_type = val; }

	//������Ŀ
	int itemSize() const{
		return this->m_valueList.size();
	}
	//�����
	void addItem(const KVPairTree<Key,Value>& v){
		m_bSingleValue=false;
		this->m_valueList.push_back(v);
	}

	//�����±��ȡ����
	KVPairTree<Key,Value>& item(unsigned int index) {
		checkBound(index);
		return m_valueList[index];
	}
	//�������ƻ�ȡ����
	KVPairTree<Key,Value>& item(const Key& k);

	//�Ƴ���
	KVPairTree<Key,Value> removeItem(unsigned int index);
	KVPairTree<Key,Value> removeItem(const Key& k);

	//�������
	void clear() {
		m_bSingleValue=true;
		m_valueList.clear();
		m_singleValue=Pair<Key,Value>();
	}

	//�Ƿ�ΪҶ�ӽڵ�
	bool isSingleValue() const{
		return m_bSingleValue;
	}

	//�Ƿ����ĳ���ӽڵ�
	bool contain(const Key& k) const;

	//�ж��Ƿ����
	bool operator==(const KVPairTree<Key,Value>& node) const{
		if(m_bSingleValue==node.m_bSingleValue && m_singleValue==node.singleValue
			&& m_valueList==node.valueList)
			return true;
		else
			return false;
	}
private:
	void checkBound(unsigned int index) const{
		if(index<0 || index>=m_valueList.size())
			throwpe(OutOfBoundException(index,m_valueList.size()));
	}
protected:
	bool m_bSingleValue;//�Ƿ�Ϊ��ֵ��
	List<KVPairTree> m_valueList;//��ֵ���б�
	Pair<Key,Value> m_singleValue;//��ֵ��
	Key m_type;//����
};


template<typename Key,typename Value>
KVPairTree<Key,Value>::KVPairTree()
{
	clear();
}

template<typename Key,typename Value>
KVPairTree<Key,Value>::~KVPairTree()
{

}

template<typename Key,typename Value>
void KVPairTree<Key,Value>::setSinglePair(const Key& k,const Value& v)
{
	m_bSingleValue=true;
	m_singleValue.key=k;
	m_singleValue.value=v;
}

template<typename Key,typename Value>
KVPairTree<Key,Value>& KVPairTree<Key,Value>::item(const Key& k)
{
	for(unsigned int i=0; i<m_valueList.size(); i++)
	{
		if(k==m_valueList[i].name())
			return m_valueList[i];
	}
	String key=Value2String<Key>(k);
	String s=String::format("KVPairTree item(): key '%s' not found",
		key.c_str());
	throwpe(Exception(s));
}

template<typename Key,typename Value>
KVPairTree<Key,Value> KVPairTree<Key, Value>::removeItem(unsigned int index)
{
	checkBound(index);
	KVPairTree<Key,Value> prop=m_valueList[index];
	m_valueList.erase(m_valueList.begin()+index);
	return prop;
}

template<typename Key,typename Value>
KVPairTree<Key,Value> KVPairTree<Key, Value>::removeItem(const Key& k)
{
	for(auto iter = m_valueList.begin(); iter != m_valueList.end();)
	{
		auto& item=*iter;
		if(k==item.name())
		{
			KVPairTree<Key,Value> prop=item;
			iter=m_valueList.erase(iter);
			return prop;
		}
		else
			++iter;
	}
	String key=Value2String<Key>(k);
	String s=String::format("KVPairTree removeItem(): key '%s' not found",
		key.c_str());
	throwpe(Exception(s));
}

template<typename Key,typename Value>
bool KVPairTree<Key, Value>::contain( const Key& k ) const
{
	for(unsigned int i=0; i<m_valueList.size(); i++)
	{
		if(k==m_valueList[i].name())
			return true;
	}
	return false;
}

}//end of namespace blib
#endif


class A : public Object
{
public:
	int a;
	A():a(1){}
};
class B : public Object
{
public:
	int b;
	B():b(2){}
};
class C : public A, public B
{
public:
	int c;
	C():c(3){}
};

void testSmartPtr()
{
	C* c = new C();
	B* b = c;
	A* a = c;
	A* a2 = dynamic_cast<A*>(b);

	int ppp0=(int)c;
	int ppp1=(int)b;
	int ppp2=(int)a;
	int ppp3=(int)&c->a;
	int ppp4=(int)&c->b;
	int ppp5=(int)&c->c;
	/*
	* 1.子类指针赋给SmartPtr
	* 2.动态转换
	* 3.多继承动态转换
	*/
	//SmartPtr<B> bc = c;//Exception, please use: SmartPtr<C>(c)
	SmartPtr<C> cc = c;
	SmartPtr<B> bc = c;//ok, because c has been in cc.
	SmartPtr<A> ac = cc.dynamicCast<A>();
	SmartPtr<B> bc2 = cc.dynamicCast<B>();
	SmartPtr<A> ac2 = bc2.dynamicCast<A>();
	//SmartPtr<A> ac3 = bc2; //can't static_cast
}
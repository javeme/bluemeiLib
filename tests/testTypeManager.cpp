

class A0 : public Object{};
class A1 : public Object{};
class B : public A1,public A0{};
class D{};


void test_objectIsItselfAddr()
{
	B* b = new B();
	A0* a0 = b;
	A1* a1 = b;

	Object* oa = (Object*)(void*)a0;
	void* aaa=dynamic_cast<B*>(oa);
	void* aaa1=dynamic_cast<B*>((Object*)(void*)a1);


	TypeManager::instance()->registerType<A0>();
	TypeManager::instance()->registerType<A1>();
	TypeManager::instance()->registerType<B>();

	auto yyyy1=TypeManager::instance()->objectIsItselfAddr(b);
	auto yyyy2=TypeManager::instance()->objectIsItselfAddr(a0);
	auto yyyy3=TypeManager::instance()->objectIsItselfAddr(a1);
	auto yyyy4=TypeManager::instance()->objectIsItselfAddr(new int());
	auto yyyy5=TypeManager::instance()->objectIsItselfAddr(new D());
}
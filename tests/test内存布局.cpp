
class A
{
public:
	virtual void fa()=0;
	virtual void faa(){};
	int a;
};
class B
{
public:
	virtual void fb()=0;
	virtual void fbb(){};
	int b;
};
class C : virtual public A, public B
{
public:
	virtual void fa(){};
	virtual void fb(){};
	virtual void fc(){};
public:
	virtual void f1(int c){};
	int c;
};
class D : public C
{
public:
	virtual void f2(){};
	int d;
};

typedef void* Fun();
struct MetaClass
{
	Fun** vTable;
	union{
		int (*vbOffsetTable)[10];
		int buf[64];
	};

	Fun** getVbtable(int index=0)const{ 
		int offset=(*vbOffsetTable)[index+1];
		return (Fun**)*(int*)((char*)&vbOffsetTable+offset); 
	}

	Fun** getVbtable(int index,int start)const{ 
		int** pVbt=(int**)((char*)this+start);
		int* vbt=*pVbt;
		int offset=(vbt)[index+1];
		return (Fun**)*(int*)((char*)pVbt+offset); 
	}
};

void printFTable(Fun** vt)
{
	for(int i=0;;i++)
	{
		Fun* fun=vt[i];

		if(fun==NULL)
			break;
		printf("%d: %p\n",i,fun);
		//fun();
	}
}
void testMemMap()
{	
	D* p=new D();
	p->a=11;
	p->b=22;
	p->c=33;
	p->d=44;
	//p->f(3);
	A* p1=p;
	B* p2=p;
	int si=sizeof(*p);

	MetaClass* mc=(MetaClass*)(p);
	//int* vt=(int*)(*(int*)(p));
	auto vt=mc->vTable;
	printFTable(vt);

	/*mc=(MetaClass*)((char*)p+sizeof(A));
	vt=mc->vTable;
	printFTable(vt);*/

	//Ðé»ùÀà
	//vt=mc->getVbtable();
	vt=mc->getVbtable(0,sizeof(B));
	for(int i=0;;i++)
	{
		Fun* fun=vt[i];

		if(fun==NULL)
			break;
		printf("%p",fun);
		fun();
	}

	delete p;
}
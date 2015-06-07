#include "ClassField.h"

class TestField : public Object
{
public:
	DECLARE_DCLASS(TestField);
public:
	int aa;
	float bb;
	double cc;
	Double obj;

	Field<bool> sex;
	Field<int> age;
	Field<float> weight;
	FIELD(int, ival);
	FIELD(float, fval);
	FIELD(Object*, oval);
public:
	TestField(): oval(null) {}
	int ff();
};

REGISTER_CLASS(TestField);
REG_CLS_FIELD(TestField, bb);
REG_CLS_FIELD(TestField, cc);

void testField()
{
	REG_FIELD(TestField::obj);
	TestField test;
	test.oval = new TestField();
	auto aa = regField("aa", &TestField::aa);
	aa.set(test, 123);
	String sssaa=Value2String<const FieldInfo*>(&aa);

	regField("sex", &TestField::sex);
	auto field = regField("age", &TestField::age);
	test.sex = true;
	test.age = 28;
	int age = test.age + 2;
	field.set(test, 80);

	test.setAttributeT("bb", 3.14f);
	test.setAttributeT<Double>("obj", 3.14159260);
	test.setAttributeT<Field<int>>("age", (88));
	age = test.getAttributeT<Field<int>>("age");

	test.setAttribute("cc", &Double(6.28));

	String sss=Value2String<const FieldInfo*>(&field);
	auto cls = TestField::thisClass();
	auto flds = cls->allFields();
	std::cout<<flds.toString().c_str()<<std::endl;

	auto map = objectToMap(&test);
	//auto map = MapObjectHelper<TestField*, true>::object2map(&test);
	std::cout<<map->toString().c_str()<<std::endl;
	delete map;

	ObjectMap* map2 = new ObjectMap();
	map2->putValue("aa", 38);
	map2->put("age", new Field<int>(38));
	auto obj = valueOf<TestField*>(map2);
	delete obj;
	delete map2;

	valueOf<TestField*>(&sss);
}
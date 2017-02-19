
/* test is_convertible */
void testconvertible()
{
	bool convertible;
	convertible = is_convertible<int,int>::value;
	convertible = is_convertible<int,long>::value;
	convertible = is_convertible<long,int>::value;
	convertible = is_convertible<float,int>::value;
	convertible = is_convertible<int,Object*>::value;
	convertible = is_convertible<long,Object*>::value;
	convertible = is_convertible<Object*,void*>::value;
	convertible = is_convertible<void*,Object*>::value;
	convertible = is_convertible<void,void>::value;
	convertible = is_convertible<void,Object*>::value;
	convertible = is_convertible<Object*,void>::value;
}

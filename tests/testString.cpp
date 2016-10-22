void testString()
{
	String str="Hello.";
	str+="test";
	bool b=str.contain("es");
	b=str.contain("esta");
	String sub=str.getLeftSub(2);
	sub=str.getRightSub(11);
	sub=str.getRightSub(4);
	sub=str.substring(1,5);
	sub=str.substring(0,15);
	sub.replace("H","h");
	b=sub.compare(str,false);
	String tmp=String(" ")+str+" "+sub+"   ";
	tmp=tmp.trim();
	TRACE("%s\n",(const char*)tmp);

	String sss = "aaa bbb ccc ddd-eee fff";
	auto list = sss.splitWith(" ");
	String sss2 = String("_").join(list);
	sss2 = String("_").join(list, 1, 4);
	sss2 = String("_").join(list, 2, 3);
	sss2 = String("_").join(list, 1, 20);
	sss2 = String("_").join(list, 3, 2);
}
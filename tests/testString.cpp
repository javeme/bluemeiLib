
void testString()
{
	/******************** constructors ********************/
	// empty
	String str0;
	assert(0 == str0.length());
	assert(true == str0.empty());
	assert(String("") == str0);

	String str01("1", 0);
	assert(String("") == str01);

	// 1 char
	String str1("1");
	assert(String("1") == str1);
	assert("1" != str1);

	String str10("123", 1);
	assert(String("1") == str10);

	// constructor copy
	String str11(str1);
	assert(String("1") == str11);

	// constructor std::string
	String str12(std::string("1"));
	assert(String("1") == str12);

	// constructor move
	String str2("12");
	assert(String("12") == str2);
	String str21(std::move(str2));
	assert(String("12") == str21);
	assert(String("") == str2);

	/******************** operators /********************/
	// operator =
	String str3 = "123";
	String str31 = str3;
	assert(String("123") == str3);
	assert(String("123") == str31);

	// operator = move
	String str32 = std::move(str3);
	assert(String("") == str3);

	// operator == / != / <
	String str41 = "1234";
	String str42 = "1234";
	String str43 = "2234";
	assert(str41 == str42);
	assert(str41 != str43);
	assert(str41 < str43);
	assert(str41 == "1234");
	assert(str41 != "2234");

	// operator +
	String str51 = "12345";
	String str52 = "6";
	String str5 = str51 + str52;
	assert(String("123456") == str5);

	String hi = "hi:";
	assert(String("hi:jack") == hi + "jack");
	assert(String("hi:true") == hi + true);
	assert(String("hi:false") == hi + false);
	assert(String("hi:d") == hi + 'd');
	assert(String("hi:123") == hi + 123);
	assert(String("hi:3.140000") == hi + 3.14f);

	// operator +=
	String str61 = "123456";
	String str62 = "789";
	str61 += str62;
	assert(String("123456789") == str61);

	//******************** conversions /********************/
	// conversion cstring / std::string
	String str7 = "1234567";
	cstring str71 = str7;
	std::string str72 = str7;
	assert(String("1234567") == std::string(str71));
	assert(String("1234567") == str72);

	//******************** member methods ********************/
	// append
	String str8 = "012345678";
	String str81 = str8;
	String str82 = "90";
	str81.append(str82);
	assert(String("01234567890") == str81);

	// charAt
	assert('0' == str8.charAt(0));
	assert('5' == str8.charAt(5));
	assert('8' == str8.charAt(-1));

	// length
	assert(9 == str8.length());

	// empty
	assert(String("").empty());

	// find
	assert(1 == str8.find("12"));
	assert(-1 == str8.find("12", 2));
	assert(5 == str8.find("567"));
	assert(5 == str8.find("567", 5));
	assert(-1 == str8.find("567", 6));
	assert(-1 == str8.find("567", 9));
	assert(-1 == str8.find("5679"));

	assert(0 == String("01234567890").find("0"));
	assert(10 == String("01234567890").find("0", 8));

	assert(0 == String("").find(""));
	assert(0 == String("").find("", 0));
	assert(-1 == String("").find("", 1));
	assert(-1 == String("").find("1"));

	// rfind
	assert(1 == str8.rfind("12"));
	assert(1 == str8.rfind("12", 1));
	assert(-1 == str8.rfind("12", 0));
	assert(5 == str8.rfind("567"));
	assert(5 == str8.rfind("567", 9));
	assert(5 == str8.rfind("567", 5));
	assert(-1 == str8.rfind("567", 4));
	assert(-1 == str8.rfind("567", 0));
	assert(-1 == str8.rfind("5679"));

	assert(10 == String("01234567890").rfind("0")); // from 10
	assert(0 == String("01234567890").rfind("0", 2));

	assert(3 == String("abc").rfind(""));
	assert(0 == String("").rfind(""));
	assert(0 == String("").rfind("", 0));
	assert(0 == String("").rfind("", 1));
	assert(-1 == String("").rfind("1"));

	// contain
	assert(true == str8.contain("12"));
	assert(false == str8.contain("1236"));

	// startWith
	assert(true == str8.startWith("0123"));
	assert(false == str8.startWith("01235"));
	assert(false == str8.startWith("1234"));

	// endWith
	assert(true == str8.endWith("678"));
	assert(false == str8.endWith("578"));
	assert(false == str8.endWith("567"));

	// trim
	String str9 = " 12345 6789 ";
	String str91 = str9.trim();
	String str92 = str9.trim(TRIM_LEFT);
	String str93 = str9.trim(TRIM_RIGHT);
	assert(String(" 12345 6789 ") == str9);
	assert(String("12345 6789") == str91);
	assert(String("12345 6789 ") == str92);
	assert(String(" 12345 6789") == str93);

	// replace
	String stra1 = "123456789abc";
	String stra2 = stra1.replace("abc", "efg");
	assert(String("123456789abc") == stra1);
	assert(String("123456789efg") == stra2);

	// splitWith
	String strb = ",123,456,789,,abc;,,.";
	ArrayList<String> listb = strb.splitWith(",");
	assert(8 == listb.size());
	assert(String("") == listb[0]);
	assert(String("123") == listb[1]);
	assert(String("456") == listb[2]);
	assert(String("789") == listb[3]);
	assert(String("") == listb[4]);
	assert(String("abc;") == listb[5]);
	assert(String("") == listb[6]);
	assert(String(".") == listb[7]);

	assert(1 == (String("").splitWith(",").size()));
	assert(3 == (String(",,").splitWith(",").size()));

	String strb1 = ",123,456,789,,abc;,,.";
	ArrayList<String> listb1 = strb1.splitWith(",", -1, false);
	assert(5 == listb1.size());
	assert(String("123") == listb1[0]);
	assert(String("456") == listb1[1]);
	assert(String("789") == listb1[2]);
	assert(String("abc;") == listb1[3]);
	assert(String(".") == listb1[4]);

	String strb2 = "--123--456----789----";
	ArrayList<String> listb2 = strb2.splitWith("--", -1, false);
	assert(3 == listb2.size());
	assert(String("123") == listb2[0]);
	assert(String("456") == listb2[1]);
	assert(String("789") == listb2[2]);

	ArrayList<String> listb3 = strb2.splitWith("--", 3, false);
	assert(3 == listb3.size());
	assert(String("123") == listb3[0]);
	assert(String("456") == listb3[1]);
	assert(String("--789----") == listb3[2]);

	// join
	listb = listb1;
	String strc = String("-").join(listb);
	assert(String("123-456-789-abc;-.") == strc);

	String strc1 = String("-").join(listb, 1, 5); // [1, 5)
	assert(String("456-789-abc;-.") == strc1);

	String strc2 = String("-").join(listb, 2, 4);
	assert(String("789-abc;") == strc2);

	String strc3 = String("-").join(listb, 1, 20);
	assert(String("456-789-abc;-.") == strc3);

	//String strc4 = String("-").join(listb, 3, 2); // InvalidArgException
	//assert(String("") == strc4);

	// substring
	String strd = "0123456789abcdef";
	String strd1 = strd.substring(0, 10);
	String strd2 = strd.substring(5, 10);
	String strd3 = strd.substring(5, 20);
	String strd4 = strd.substring(10, 3);
	String strd5 = strd.substring(10, 5);
	String strd6 = strd.substring(10);
	assert(String("0123456789abcdef") == strd);
	assert(String("0123456789") == strd1);
	assert(String("56789abcde") == strd2);
	assert(String("56789abcdef") == strd3);
	assert(String("abc") == strd4);
	assert(String("abcde") == strd5);
	assert(String("abcdef") == strd6);

	// getLeftSub / getRightSub
	String stre = "123abc";
	String stre1 = stre.getLeftSub(3);
	String stre2 = stre.getRightSub(3);
	String stre3 = stre.getLeftSub(8);
	String stre4 = stre.getRightSub(8);
	assert(String("123") == stre1);
	assert(String("abc") == stre2);
	assert(String("123abc") == stre3);
	assert(String("123abc") == stre4);

	// compare
	String strf1 = "123abc";
	String strf2 = "123abc";
	String strf3 = "123AbC";
	assert(true == strf1.compare(strf2));
	assert(false == strf1.compare(strf3));
	assert(true == strf1.compare(strf3, false));

	assert(true == strf1.compare("123abc"));
	assert(false == strf1.compare("123d"));
	assert(true == strf1.compare("123Abc", false));

	// toUpper / toLower
	String strg = "123Abc4Def";
	String strg1 = strg.toUpper();
	String strg2 = strg.toLower();
	assert(String("123ABC4DEF") == strg1);
	assert(String("123abc4def") == strg2);

	// toString
	String* strObj = new String("this is a String");
	assert(String("this is a String") == strObj->toString());
	delete strObj;

	// c_str
	String strh = "123abc";
	assert(0 == strcmp("123abc", strh.c_str()));

	// format
	String stri = String::format("str=%s,int=%d,float=%.2f",
		"hi", 1, 3.14);
	assert(String("str=hi,int=1,float=3.14") == stri);


	//******************** test functions********************/
	// others
	String str = "Hello.";
	str += "test";
	assert(String("Hello.test") == str);
	bool b = str.contain("es");
	assert(b);
	b = str.contain("esta");
	assert(!b);

	String sub = str.getLeftSub(2);
	assert(String("He") == sub);
	sub = str.getRightSub(11);
	assert(String("Hello.test") == sub);
	sub = str.getRightSub(4);
	assert(String("test") == sub);

	sub = str.substring(1, 5);
	assert(String("ello.") == sub);
	sub = str.substring(0, 15);
	assert(String("Hello.test") == sub);

	sub = sub.replace("H", "h");
	assert(String("hello.test") == sub);
	b = sub.compare(str, false);
	assert(b);

	String tmp = String(" ") + str + " " + sub + "  ";
	tmp = tmp.trim();
	assert(String("Hello.test hello.test") == tmp);

	printf("string unit test passed successfully!\n");
}

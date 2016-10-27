
void testTemplate()
{
	cstring s = "$hello(\\$hello), "
		"my name is $name, age is $age. "
		"start <% list=[]; list.apppend(10); print list.length; %> end";
	Log::LogCtx ctx;
	ctx.put("name", "blue");
	ctx.put("age", "28");
	ctx.put("hello", "lee-fuck");

	TemplateLoader loader;
	auto tpl = loader.load(s);
	String rs = tpl->render(ctx);
	delete tpl;
	printf("%s\n ====> \n%s", s, rs.c_str());
}

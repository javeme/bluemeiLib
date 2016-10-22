
void testConfigOption()
{
	SimpleCfgFile CONF;

	ConfigGroup server("server");
	server.registerOption(StringOption("host", "127.0.0.1", "server hostname"));
	server.registerOption(IntOption("port", 80, "server port"));
	CONF.registerGroup(server);

	ConfigGroup group("group_test");
	group.registerOption(StringOption("string", "str value", "str description"));
	group.registerOption(IntOption("int", 100, "int description", 1, 120));
	group.registerOption(FloatOption("float", 3.14, "float description", 3, 4));
	group.registerOption(BooleanOption("bool", true, "bool description"));
	group.registerOption(
		ChoiceOption("choices", 20, "choices description")
		.addChoice(10)
		.addChoice(20)
		.addChoice(30)
		.addChoice(40)
		.addChoice(50)
		);
	CONF.registerGroup(group);

	// assert registered options value
	assert(CONF["group_test"].option<String>("string") == "str value");
	assert(CONF["group_test"].option<int>("int") == 100);
	assert(CONF["group_test"].option<double>("float") == 3.14);
	assert(CONF["group_test"].option<bool>("bool") == true);
	assert(CONF["group_test"].option<int>("choices") == 20);

	// test get option value
	String host = CONF.group("server").option<String>("host");
	int port = CONF.group("server").option<int>("port");

	host = CONF["server"].option<String>("host");
	//port = CONF["server"]["port"];

	// test set option value
	CONF["group_test"].update("string", "str2");
	CONF["group_test"].update("int", "101");
	CONF["group_test"].update("float", "3.15");
	CONF["group_test"].update("bool", "0");
	CONF["group_test"].update("choices", "30");

	// assert option values
	String str = CONF["group_test"].option<String>("string");
	assert(str == "str2");

	int intOpt = CONF["group_test"].option<int>("int");
	assert(intOpt == 101);

	double dblOpt = CONF["group_test"].option<double>("float");
	assert(dblOpt == 3.15);

	bool boolOpt = CONF["group_test"].option<bool>("bool");
	assert(boolOpt == false);

	int choice = CONF["group_test"].option<int>("choices");
	assert(choice == 30);

	printf("CONF %s\n", CONF.toString().c_str());
}
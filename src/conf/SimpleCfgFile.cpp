#include "SimpleCfgFile.h"
#include "Util.h"
#include "StringBuilder.h"

namespace blib{

SimpleCfgFile::SimpleCfgFile(const String& path)
{
	if(!path.empty())
		parseFromFile(path);
}

SimpleCfgFile::~SimpleCfgFile(void)
{
}

void SimpleCfgFile::parseFromFile(const String& path)
{
	this->m_filePath = path;
	this->parse();
}

void SimpleCfgFile::parse()
{
	String line, key;

	bool quoting = false;
	StringBuilder value(128);

	String group = "default";
	int pos = -1;
	int lines = 0;

	auto throwError = [&](const String& msg) {
		String pre(String::format("Can't parse config file '%s' at line %d, ",
			m_filePath.c_str(), lines));
		throwpe(ConfigException(pre + msg));
	};

	File file(m_filePath, "r");
	while(file.readLine(line) != -1)
	{
		lines++;
		// exclude comments: "#..."
		pos = line.find("#");
		if(pos >= 0) {
			line = line.substring(0, pos);
		}

		// in quotes?
		if(quoting) {
			// end quote?
			pos = line.find("\"");
			if(pos >= 0) {
				// got a full value
				quoting = false;
				if(!line.endWith("\"")) {
					// warning: ignore contents after a right quote
				}
				value.append(line.substring(0, pos));
				this->setOptionToGroup(group, key, value.toString());
			}
			else {
				value.append(line);
			}
		}
		// is it a section?
		else if(line.startWith("[")) {
			if(!line.endWith("]")) {
				throwError("expect ']'");
			}
			group = line.substring(1, line.length() - 2);
		}
		// is it an option?
		else if((pos = line.find("=")) > 0) {
			// found one option
			key = line.substring(0, pos).trim();
			String val = line.substring(pos + 1).trim(TRIM_LEFT);
			if(val.startWith("\"")) {
				// remove quotes
				if(val.endWith("\"")) {
					val = val.substring(1, val.length() - 2);
					this->setOptionToGroup(group, key, val);
				}
				// waiting for a right quote
				else {
					quoting = true;
					value.clear();
					value.append(val.substring(1));
				}
			}
			else {
				this->setOptionToGroup(group, key, val.trim());
			}
		}
		// unexpected line
		else if(!line.trim().empty()) {
			throwError(String::format("unexpected line: '%s'", line.c_str()));
		}
	}
	file.close();
}

ConfigGroup& SimpleCfgFile::setOptionToGroup(const String& section,
	const String& name, const String& value)
{
	ConfigGroup& grp = this->group(section, true);
	if(grp.name() != section)
		grp.setName(section);

	if(grp.exists(name)) {
		grp.update(name, value);
	}
	else {
		grp.set(name, StringOption(name, value, name));
	}
	return grp;
}

}//end of namespace blib
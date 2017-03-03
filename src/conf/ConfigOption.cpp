#include "ConfigOption.h"
#include "Util.h"
#include "Ptr.h"
#include "StringBuilder.h"
#include <functional>

namespace blib{

/////////////////////////////////////////////////////////////////////
// tools macro
#define map2String(map, ptr, itemSpaces, endSpaces)                   \
[&]() -> String {                                                     \
    StringBuilder sb(map.size() * 16);                                \
    sb.append(name());                                                \
    sb.append(": {\n");                                               \
    for(auto i = map.begin(); i != map.end(); i++)                    \
    {                                                                 \
        String option = i->second ptr toString();                     \
        sb.append(itemSpaces);                                        \
        sb.append(option);                                            \
        sb.append(",\n");                                             \
    }                                                                 \
    if(sb.endWith(','))                                               \
        sb.pop();                                                     \
    sb.append(endSpaces);                                             \
    sb.append("}");                                                   \
    return sb.toString();                                             \
}                                                                     \

/////////////////////////////////////////////////////////////////////
// class ConfigOption
ConfigOption* ConfigOption::clone(const ConfigOption& value)
{
	Object* obj = ((Object*)&value)->clone();
	ConfigOption* option = dynamic_cast<ConfigOption*>(obj);
	if(option == null) {
		delete obj;
		throwpe(ConfigException("ConfigOption.clone() should return "
			"a ConfigOption* object"));
	}
	return option;
}

ConfigOption& ConfigOption::setDeprecated(const String& val)
{
	m_deprecated = val;
	return *this;
}

String ConfigOption::toString() const
{
	return name();
}

/////////////////////////////////////////////////////////////////////
// class StringOption
Object* StringOption::clone() const
{
	return new StringOption(*this);
}

bool StringOption::valueEquals(const ConfigOption& other) const
{
	if(other.type() != this->type())
		return false;
	auto opt = dynamic_cast<const StringOption*>(&other);
	checkNullPtr(opt);
	return opt->value() == value();
}

String StringOption::toString() const
{
	return String::format("%s: %s", name().c_str(), value().c_str());
}

/////////////////////////////////////////////////////////////////////
// class IntOption
void IntOption::setValue(int val)
{
	if(val < m_min || val > m_max) {
		throwpe(ConfigException(String::format("Invalid value %d "
			"for option '%s', should be in range(%d, %d)",
			val, name().c_str(), m_min, m_max)));
	}
	m_value = val;
}

Object* IntOption::clone() const
{
	return new IntOption(*this);
}

bool IntOption::valueEquals(const ConfigOption& other) const
{
	if(other.type() != this->type())
		return false;
	auto opt = dynamic_cast<const IntOption*>(&other);
	checkNullPtr(opt);
	return opt->value() == value();
}

String IntOption::toString() const
{
	return String::format("%s: %d", name().c_str(), value());
}

/////////////////////////////////////////////////////////////////////
// class FloatOption
void FloatOption::setValue(double val)
{
	if(val < m_min || val > m_max) {
		throwpe(ConfigException(String::format("Invalid value %lf "
			"for option '%s', should be in range(%lf, %lf)",
			val, name().c_str(), m_min, m_max)));
	}
	m_value = val;
}

Object* FloatOption::clone() const
{
	return new FloatOption(*this);
}

bool FloatOption::valueEquals(const ConfigOption& other) const
{
	if(other.type() != this->type())
		return false;
	auto opt = dynamic_cast<const FloatOption*>(&other);
	checkNullPtr(opt);
	return opt->value() == value();
}

String FloatOption::toString() const
{
	return String::format("%s: %lf", name().c_str(), value());
}

/////////////////////////////////////////////////////////////////////
// class BooleanOption
Object* BooleanOption::clone() const
{
	return new BooleanOption(*this);
}

bool BooleanOption::valueEquals(const ConfigOption& other) const
{
	if(other.type() != this->type())
		return false;
	auto opt = dynamic_cast<const BooleanOption*>(&other);
	checkNullPtr(opt);
	return opt->value() == value();
}

String BooleanOption::toString() const
{
	return String::format("%s: %s", name().c_str(),
		Util::boolean2Str(value()).c_str());
}

/////////////////////////////////////////////////////////////////////
// class ChoiceOption
ChoiceOption::ChoiceOption(const String& name, const ConfigOption& value,
	const String& description)
	: ConfigOption(OPTION_TYPE, name, description)
{
	m_value = ConfigOption::clone(value);
}

ChoiceOption::ChoiceOption(const String& name, const String& value,
	const String& description)
	: ConfigOption(OPTION_TYPE, name, description)
{
	m_value = new StringOption("", value, "");
}

ChoiceOption::ChoiceOption(const String& name, int value,
	const String& description)
	: ConfigOption(OPTION_TYPE, name, description)
{
	m_value = new IntOption("", value, "");
}

ChoiceOption::ChoiceOption(const String& name, double value,
	const String& description)
	: ConfigOption(OPTION_TYPE, name, description)
{
	m_value = new FloatOption("", value, "");
}

ChoiceOption::ChoiceOption(const String& name, bool value,
	const String& description)
	: ConfigOption(OPTION_TYPE, name, description)
{
	m_value = new BooleanOption("", value, "");
}

ChoiceOption::~ChoiceOption()
{
	delete m_value;

	for(unsigned int i=0; i<m_choices.size(); i++)
	{
		delete m_choices[i];
	}
}

void ChoiceOption::setValue(const ConfigOption& val)
{
	if(!checkChoice(val)) {
		String choice = option2String(val);
		throwpe(ConfigException(String::format("Invalid value '%s' "
			"for choice option '%s', should be in %s",
			choice.c_str(), name().c_str(), choicesString().c_str())));
	}
	delete m_value;
	m_value = ConfigOption::clone(val);
}

ConfigOption& ChoiceOption::value() const
{
	checkNullPtr(m_value);
	return *m_value;
}

bool ChoiceOption::checkChoice(const ConfigOption& val)
{
	for(unsigned int i=0; i<m_choices.size(); i++)
	{
		ConfigOption* option = m_choices[i];
		if(option && option->valueEquals(val))
			return true;
	}
	return false;
}

ChoiceOption& ChoiceOption::addChoice(const ConfigOption& val)
{
	m_choices.push_back(ConfigOption::clone(val));
	return *this;
}

ChoiceOption& ChoiceOption::addChoice(const String& val)
{
	m_choices.push_back(new StringOption("", val, ""));
	return *this;
}

ChoiceOption& ChoiceOption::addChoice(int val)
{
	m_choices.push_back(new IntOption("", val, ""));
	return *this;
}

ChoiceOption& ChoiceOption::addChoice(double val)
{
	m_choices.push_back(new FloatOption("", val, ""));
	return *this;
}

ChoiceOption& ChoiceOption::addChoice(bool val)
{
	m_choices.push_back(new BooleanOption("", val, ""));
	return *this;
}

Object* ChoiceOption::clone() const
{
	ChoiceOption* obj = new ChoiceOption(name(), *m_value, description());

	for(unsigned int i=0; i<m_choices.size(); i++)
	{
		obj->m_choices.push_back(ConfigOption::clone(*m_choices[i]));
	}
	return obj;
}

bool ChoiceOption::valueEquals(const ConfigOption& other) const
{
	return m_value->valueEquals(other);
}

String ChoiceOption::toString() const
{
	String choice = option2String(*m_value);
	return String::format("%s: %s", name().c_str(), choice.c_str());
}

String ChoiceOption::choicesString() const
{
	StringBuilder sb(m_choices.size() * 8);
	sb.append("[");
	for(unsigned int i=0; i<m_choices.size(); i++)
	{
		sb.append(option2String(*m_choices[i]));
		sb.append(",");
	}
	if(sb.endWith(','))
		sb.pop();
	sb.append("]");
	return sb.toString();
}

String ChoiceOption::option2String(const ConfigOption& option)
{
	// expect "name:value"
	String choice = option.toString();
	// if name is empty
	if(choice.startWith(": "))
		choice = choice.substring(2);
	return choice;
}

/////////////////////////////////////////////////////////////////////
// class ConfigGroup
ConfigGroup::ConfigGroup(const String& name/*=""*/) : m_group(name)
{
}

ConfigGroup::ConfigGroup(const ConfigGroup& other)
{
	*this = other;
}

ConfigGroup::~ConfigGroup()
{
	for(auto i = m_propertiesMap.begin(); i != m_propertiesMap.end(); i++)
	{
		delete i->second;
	}
}

ConfigGroup& ConfigGroup::operator=(const ConfigGroup& other)
{
	this->m_group = other.m_group;

	for(auto i = other.m_propertiesMap.begin();
		i != other.m_propertiesMap.end(); i++)
	{
		checkNullPtr(i->second);
		m_propertiesMap.insert(std::make_pair(i->first,
			ConfigOption::clone(*i->second)));
	}

	return *this;
}

bool ConfigGroup::exists(const String& key) const
{
	auto it = m_propertiesMap.find(key);
	if(it != m_propertiesMap.end())
		return true;
	return false;
}

bool ConfigGroup::get(const String& key, ConfigOption*& value) const
{
	auto it = m_propertiesMap.find(key);
	if(it != m_propertiesMap.end())
	{
		value = it->second;
		return true;
	}
	else
		return false;
}

void ConfigGroup::set(const String& key, const ConfigOption& value)
{
	m_propertiesMap.insert(std::make_pair(key, ConfigOption::clone(value)));
}

bool ConfigGroup::update(const String& key, const String& value)
{
	if(!exists(key))
		return false;
	ConfigOption* option = m_propertiesMap[key];
	
	std::function<bool(ConfigOption*, const String&)> setOptionValue;
	setOptionValue = [&](ConfigOption* option, const String& value) -> bool {
		checkNullPtr(option);
		switch(option->type())
		{
		case ConfigOption::OPTION_STRING:
			{
				StringOption* self = dynamic_cast<StringOption*>(option);
				checkNullPtr(self);
				self->setValue(value);
			}
			break;
		case ConfigOption::OPTION_INT:
			{
				IntOption* self = dynamic_cast<IntOption*>(option);
				checkNullPtr(self);
				self->setValue(Util::str2Int(value));
			}
			break;
		case ConfigOption::OPTION_FLOAT:
			{
				FloatOption* self = dynamic_cast<FloatOption*>(option);
				checkNullPtr(self);
				self->setValue(Util::str2Float(value));
			}
			break;
		case ConfigOption::OPTION_BOOLEAN:
			{
				BooleanOption* self = dynamic_cast<BooleanOption*>(option);
				checkNullPtr(self);
				self->setValue(Util::str2Boolean(value));
			}
			break;
		case ConfigOption::OPTION_CHOICE:
			{
				ChoiceOption* self = dynamic_cast<ChoiceOption*>(option);
				checkNullPtr(self);
				// to check the value
				ScopePointer<ConfigOption> tmp = ConfigOption::clone(
					self->value());
				bool success = setOptionValue(tmp, value);
				if(success) {
					self->setValue(*tmp);
				}
				return success;
			}
			break;
		default:
			return false;
		}
		return true;
	};

	return setOptionValue(option, value);
}

void ConfigGroup::registerOption(const ConfigOption& value)
{
	return this->set(value.name(), value);
}

String ConfigGroup::toString() const
{
	auto func = map2String(m_propertiesMap, ->, "    ", "  ");
	return func();
}

template <typename T, typename O>
T getConfigOption(const ConfigGroup* group, const String& key)
{
	ConfigOption* option = null;
	checkNullPtr(group);
	if(group->get(key, option)) {
		// simple option matched
		if(option->type() == O::OPTION_TYPE) {
			//option = option;
		}
		// ChoiceOption
		else if(option->type() == ChoiceOption::OPTION_TYPE) {
			ChoiceOption* choice = dynamic_cast<ChoiceOption*>(option);
			option = choice ? &choice->value() : null;
		}
		// not matched
		else {
			option = null;
		}
	}

	if(option) {
		auto opt = dynamic_cast<O*>(option);
		if(opt) {
			return opt->value();
		}
	}
	throwpe(ConfigException(String::format("Not found option<type %s>: %s.%s",
		typeid(T).name(), group->name().c_str(), key.c_str())));
}

String ConfigGroup::option(const String& key, const String&) const
{
	return getConfigOption<String, StringOption>(this, key);
}

int ConfigGroup::option(const String& key, int) const
{
	return getConfigOption<int, IntOption>(this, key);
}

double ConfigGroup::option(const String& key, double) const
{
	return getConfigOption<double, FloatOption>(this, key);
}

bool ConfigGroup::option(const String& key, bool) const
{
	return getConfigOption<bool, BooleanOption>(this, key);
}


/////////////////////////////////////////////////////////////////////
// class Config
ConfigGroup& Config::registerGroup(const ConfigGroup& value)
{
	String name = value.name();
	m_groups.insert(std::make_pair(name, value));
	return m_groups[name];
}

String Config::toString() const
{
	auto func = map2String(m_groups, ., "  ", "");
	return func() + "\n";
}

}//end of namespace blib

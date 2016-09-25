#include "time.h"
#include "Log.h"
#include "ErrorHandler.h"
#include "Thread.h"
#include "CodeUtil.h"
#include "FilePath.h"
#include "template/TemplateLoader.h"

namespace bluemei{

// name level time message thread process
//#define DFT_LOG_FRMT "%(time)s - %(level)s - %(message)s"
#define DFT_LOG_FRMT "$name - $time - $level - [Thread-$thread] $message"

const Log::LogCtx Log::s_nullCtx;

Log::Log(const String& name, const String& path, LogLevel level/*=LOG_INFO*/)
	: m_name(name), m_path(path), m_level(level), m_formatter(DFT_LOG_FRMT)
{
	m_tmpl = null;

	FilePath filePath(path);
	if(!FileUtil::mkdirs(filePath.dirName()))
	{
		throwpe(IOException(String::format("failed to mkdirs for '%s'",
											filePath.toString().c_str())));
	}
	m_file.openFile(path, "a+");
}

Log::~Log()
{
	delete m_tmpl;
	try{
		m_file.close();
	}catch(Exception& e){
		ErrorHandler::handle(e);
	}
}

void Log::setLevel(const String& val)
{
	setLevel(string2Level(val));
}

String Log::getLevelDscr() const
{
	return level2String(m_level);
}

void Log::printLine(const String& str)
{
	ScopedLock sl(m_lock);
	m_file.writeLine(str);
}

void Log::print(const String& str)
{
	ScopedLock sl(m_lock);
	m_file.writeBytes(str.c_str(), str.length());
}

String Log::level2String(LogLevel val)
{
	static cstring logLevel[LOG_LEVEL_SIZE];
	static bool init = false;
	if(!init)
	{
		logLevel[LOG_UNKNOWN] = "unknown";
		logLevel[LOG_FATAL] = "fatal";
		logLevel[LOG_CRITICAL] = "critical";
		logLevel[LOG_ERROR] = "error";
		logLevel[LOG_WARNING] = "warning";
		logLevel[LOG_NOTICE] = "notice";
		logLevel[LOG_INFO] = "info";
		logLevel[LOG_DEBUG] = "debug";
		logLevel[LOG_TRACE] = "trace";

		init = true;
	}

	if(0 <= val && val < LOG_LEVEL_SIZE)
		return logLevel[val];
	else
		return logLevel[LOG_UNKNOWN];
}

Log::LogLevel Log::string2Level(const String& val)
{
	for(int i=0; i<LOG_LEVEL_SIZE; i++)
	{
		LogLevel level = (LogLevel)i;
		if(level2String(level) == val)
		{
			return level;
		}
	}
	return LOG_UNKNOWN;
}

void Log::updateFormatter(const String& val)
{
	m_formatter = val;
	if(m_tmpl != null)
		delete m_tmpl;
	TemplateLoader loader;
	m_tmpl = loader.load(m_formatter);
}

String Log::format(const LogCtx& ctx)
{
	/*StringBuilder sb(1024);

	sb.append(ctx.getDefault("name", ""));
	sb.append(" - ");
	sb.append(ctx.getDefault("time", ""));
	sb.append(" - ");
	sb.append(ctx.getDefault("level", ""));
	sb.append(" - ");
	sb.append("[Thread-");
	sb.append(ctx.getDefault("thread", ""));
	sb.append("]  ");
	sb.append(ctx.getDefault("message", ""));

	return sb.toString();*/

	if(m_tmpl == null)
	{
		TemplateLoader loader;
		m_tmpl = loader.load(m_formatter);
	}
	String rs = m_tmpl->render(ctx);
	return rs;
}

void Log::log(LogLevel level, const String& msg, const LogCtx& ctx)
{
	if (level > m_level)
		return;

	const size_t BUF_LEN = 128;
	char strTime[BUF_LEN] = {0};
	time_t currentTime = time(NULL);
	tm localTime;
	localtime_s(&localTime, &currentTime);
	strftime(strTime, BUF_LEN,"%H:%M:%S(%Y-%m-%d)", &localTime);

	LogCtx newCtx = ctx;
	//name level time message thread process
	newCtx.put("name", name());
	newCtx.put("level", level2String(level).toUpper());
	newCtx.put("time", strTime);
	newCtx.put("message", msg);
	newCtx.put("thread", CodeUtil::int2Str(Thread::currentThreadId()));
	newCtx.put("process", "0");

	this->printLine(format(newCtx));
}

Log* Log::getLogger(const String& name)
{
	return LogManager::instance().getLogger(name);
}


////////////////////////////////////////////////////////////////////////
//LogManager
LogManager::LogManager()
{
	;
}

LogManager::~LogManager()
{
	this->destroyLoggers();
}

LogManager* LogManager::s_manager = new LogManager();

LogManager& LogManager::instance()
{
	checkNullPtr(s_manager);
	return *s_manager;
}

void LogManager::destroy()
{
	LogManager* manager = LogManager::s_manager;
	LogManager::s_manager = null;
	delete manager;
}

Log* LogManager::getLogger(const String& name)
{
	Log* logger = null;
	if(name == "")
		logger = m_loggers.getDefault(m_default, null);
	else
		logger = m_loggers.getDefault(name, null);

	if(logger == null) {
		cstring logName = "";
		if (name.empty())
			logName = m_default.empty() ? "<default>" : m_default;
		else
			logName = name;
		throw NotFoundException(String("No Logger named: ") + logName);
	}
	return logger;
}

Log* LogManager::initLogger(const String& name, const String& path, Log::LogLevel level)
{
	Log* logger = m_loggers.getDefault(name, null);
	if(!logger)
	{
		logger = new Log(name, path, level);
		m_loggers.put(name, logger);

		if (m_default == "")
			m_default = name;
	}
	else
		logger->setLevel(level);
	return logger;
}

void LogManager::destroyLoggers()
{
	m_default = "";

	auto i = m_loggers.iterator();
	while(i->hasNext()) {
		delete i->next().value;
	}
	m_loggers.clear();
}

}//end of namespace bluemei
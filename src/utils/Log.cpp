#include "time.h"
#include "Log.h"
#include "ErrorHandler.h"
#include "Thread.h"
#include "CodeUtil.h"
#include "FilePath.h"

namespace bluemei{

// name level time message thread process
#define DFT_LOG_FRMT "%(time) - %(level)s - %(message)s"

const Log::LogCtx Log::s_nullCtx;

Log::Log(const String& name, const String& path, LogLevel level/*=LOG_INFO*/)
	: m_name(name), m_path(path), m_level(level), m_formatter(DFT_LOG_FRMT)
{
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
	static String logLevel[LOG_LEVEL_SIZE];
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

String Log::format(const String& formatter, const LogCtx& ctx)
{
	StringBuilder sb(1024);

	//@Todo formatter
	//"%(time)s - %(level)s - %(message)s"
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

	return sb.toString();
}

void Log::log(LogLevel level, const String& msg, const LogCtx& ctx)
{
	if (level > m_level)
		return;

	char strTime[64];
	time_t currentTime=time(NULL);
	strftime(strTime,128,"%H:%M:%S(%Y-%m-%d)",localtime(&currentTime));//localtime_s

	LogCtx newCtx = ctx;
	//name level time message thread process
	newCtx.put("name", name());
	newCtx.put("level", level2String(level).toUpper());
	newCtx.put("time", strTime);
	newCtx.put("message", msg);
	newCtx.put("thread", CodeUtil::int2Str(Thread::currentThreadId()));
	newCtx.put("process", "0");

	this->printLine(format(m_formatter, newCtx));
}

Log* Log::getLogger(const String& name)
{
	return LogManager::instance().getLogger(name);
}

///////////////////////////////////////////////////
//LogManager
LogManager::LogManager()
{
	;
}

LogManager::~LogManager()
{
	auto i = m_loggers.iterator();
	while(i->hasNext()) {
		delete i->next().value;
	}
	m_loggers.releaseIterator(i);
}

LogManager& LogManager::instance()
{
	static LogManager manager;
	return manager;
}

Log* LogManager::getLogger(const String& name)
{
	Log* logger = null;
	if(name == "")
		logger = m_loggers.getDefault(m_default, null);
	else
		logger = m_loggers.getDefault(name, null);

	if(logger == null)
		throw NotFoundException("no Logger named: " + name);
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


}//end of namespace bluemei
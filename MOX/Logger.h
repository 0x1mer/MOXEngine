#pragma once

#include <string>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "MOX_API.h"

// Isn`t thread safe for now

enum class LogLevel
{
	Trace,
	Debug,
	Info,
	Warning,
	Error,
	Critical
};

static std::string GetTimestamp()
{
	using namespace std::chrono;

	auto now = system_clock::now();
	auto time = system_clock::to_time_t(now);

	std::tm tm{};
#ifdef _WIN32
	localtime_s(&tm, &time);
#else
	localtime_r(&time, &tm);
#endif

	std::ostringstream oss;
	oss << std::put_time(&tm, "[%Y.%m.%d %H:%M:%S]");
	return oss.str();
}

static std::string ColorizePrefix(LogLevel level, const std::string& text)
{
	const char* color = "";

	switch (level)
	{
	case LogLevel::Trace:    color = "\033[36m"; break; // Cyan
	case LogLevel::Debug:    color = "\033[34m"; break; // Blue
	case LogLevel::Info:     color = "\033[32m"; break; // Green
	case LogLevel::Warning:  color = "\033[33m"; break; // Yellow
	case LogLevel::Error:    color = "\033[31m"; break; // Red
	case LogLevel::Critical: color = "\033[35m"; break; // Magenta
	default:                 color = "\033[0m";  break;
	}

	return std::string(color) + text + "\033[0m";
}

static std::string GetLogFileName()
{
	using namespace std::chrono;

	auto now = system_clock::now();
	auto time = system_clock::to_time_t(now);

	std::tm tm{};
#ifdef _WIN32
	localtime_s(&tm, &time);
#else
	localtime_r(&time, &tm);
#endif

	std::ostringstream oss;
	oss << "log_"
		<< std::put_time(&tm, "%Y-%m-%d_%H-%M-%S")
		<< ".txt";

	return oss.str();
}

class MOX_API Logger
{
private:
	bool isConsoleWriter = true;
	std::ofstream logFile;
	std::filesystem::path logsFolder = "";

	void InitLogFile()
	{
		if (logsFolder.empty()) [[unlikely]]
		{
			logsFolder = std::filesystem::current_path();
		}

		std::filesystem::create_directories(logsFolder);

		auto filePath = logsFolder / GetLogFileName();
		logFile.open(filePath, std::ios::out | std::ios::app);
	}

	Logger() {
		InitLogFile();
	}

	~Logger()
	{
		if (logFile.is_open())
			logFile.close();
	}

public:
	static Logger& Instance()
	{
		static Logger instance;
		return instance;
	}

	void Log(LogLevel level, const std::string& message)
	{
		const char* prefix;
		switch (level)
		{
		case LogLevel::Trace:    prefix = "[TRACE] "; break;
		case LogLevel::Debug:    prefix = "[DEBUG] "; break;
		case LogLevel::Info:     prefix = "[INFO] "; break;
		case LogLevel::Warning:  prefix = "[WARN] "; break;
		case LogLevel::Error:    prefix = "[ERROR] "; break;
		case LogLevel::Critical: prefix = "[CRIT] "; break;
		default:                 prefix = "[LOG] "; break;
		}

		std::string timestamp = GetTimestamp();

		if (isConsoleWriter)
			std::cout << ColorizePrefix(level, prefix) << timestamp << " " << message << std::endl;

		if (logFile.is_open())
		{
			logFile
				<< timestamp << " "
				<< prefix
				<< message
				<< std::endl;
		}
	}

	void SetLogFolder(const std::filesystem::path& logsFolderPath)
	{
		logsFolder = logsFolderPath;

		if (!std::filesystem::exists(logsFolder))
			std::filesystem::create_directories(logsFolder);

		if (logFile.is_open())
			logFile.close();

		auto filePath = logsFolder / GetLogFileName();
		logFile.open(filePath, std::ios::out | std::ios::app);	
	}

	void SetConsoleOutput(bool enabled)
	{
		isConsoleWriter = enabled;
	}

}; 

#define LOGGER_SET_CONSOLE_OUTPUT(enabled) Logger::Instance().SetConsoleOutput(enabled)
#define LOGGER_SET_FOLDER(path) Logger::Instance().SetLogFolder(path)

#define LOG(msg) Logger::Instance().Log(LogLevel::Info, msg)
#define LOG_ERROR(msg) Logger::Instance().Log(LogLevel::Error, msg)
#define LOG_WARNING(msg) Logger::Instance().Log(LogLevel::Warning, msg)
#define LOG_DEBUG(msg) Logger::Instance().Log(LogLevel::Debug, msg)
#define LOG_TRACE(msg) Logger::Instance().Log(LogLevel::Trace, msg)
#define LOG_CRITICAL(msg) Logger::Instance().Log(LogLevel::Critical, msg)
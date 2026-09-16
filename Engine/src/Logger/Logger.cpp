#include "Logger.h"
#include <chrono>
#include <format>
#include <iostream>

// Declaration of the static value
std::deque<LogEntry> Logger::messages; 

void Logger::Log(const std::string& message)
{
	LogEntry entry; 
	entry.type = LogType::MESSAGE; 
	entry.message = "Log [" + GetTimestamp() + "]: " + message; 
	std::cout << entry.message << std::endl;
	AppendLog(entry);
}

void Logger::Warning(const std::string& message)
{
	LogEntry entry;
	entry.type = LogType::WARNING;
	entry.message = "Warning [" + GetTimestamp() + "]: "
		+ message; 
	std::cout << "\033[33m" << entry.message << "\033[0m" << std::endl;
	AppendLog(entry);
}

void Logger::Error(const std::string& message)
{
	LogEntry entry;
	entry.type = LogType::ERROR;
	entry.message = "Error [" + GetTimestamp() + "]: " + message;
	std::cout << "\033[31m" << entry.message << "\033[0m" << std::endl;
	AppendLog(entry);
}

void Logger::SDLLog(const std::string& message, const char* sdlerror)
{
	LogEntry entry;
	entry.type = LogType::MESSAGE;
	entry.message = "Log [" + GetTimestamp() + "]: " + message + ": " + sdlerror; 
	std::cout << entry.message << std::endl;
	AppendLog(entry);
}

void Logger::SDLError(const std::string& message, const char* sdlerror)
{
	LogEntry entry;
	entry.type = LogType::ERROR;
	entry.message = "Error [" + GetTimestamp() + "]: "
		+ message + ": " + sdlerror;
	std::cout << "\033[31m" << entry.message << "\033[0m" << std::endl;
	AppendLog(entry);
}

std::string Logger::GetTimestamp()
{
	auto now = std::chrono::system_clock::now();
	std::string ts = std::format("{:%Y-%m-%d %H:%M:%S}",
		std::chrono::floor<std::chrono::seconds>(now));
	return ts; 
}

void Logger::AppendLog(LogEntry entry)
{
	if (messages.size() >= 1000)
		messages.pop_front();
	messages.push_back(entry);
}

void Logger::Clear()
{
	messages.clear(); 
}

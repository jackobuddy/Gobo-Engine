#pragma once

#include <string>
#include <deque> 

// TODO: Save the error log to an external file for auditing. 

enum class LogType
{
	MESSAGE, WARNING, ERROR
};

struct LogEntry
{
	LogType type;
	std::string message; 
};

class Logger
{
public:
	// Prints a normal message to log
	static void Log(const std::string& message); 
	// Prints a warning message to log
	static void Warning(const std::string & message);
	// Prints an error message to log
	static void Error(const std::string& message); 
	// Prints a normal SDL message to log
	static void SDLLog(const std::string& message, const char* sdlerror);
	// Prints and SDL error to log
	static void SDLError(const std::string& message, const char* sdlerror); 
	// Clears the log from memory
	static void Clear(); 
	// Gets a copy of the log
	static std::deque<LogEntry>GetLog() { return messages; }

private:
	// A vector that stores all log messages
	static std::deque<LogEntry> messages;
	static std::string GetTimestamp(); 
	static void AppendLog(LogEntry entry);
};
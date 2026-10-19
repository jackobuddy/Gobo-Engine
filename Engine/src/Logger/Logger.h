#pragma once

#include <deque>
#include <string>

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
    // gameRootUtf8 is the directory containing the game executable.
    // Returns false if file logging is unavailable; other logging still works.
    static bool BeginSession(const char* gameRootUtf8) noexcept;
    static void EndSession() noexcept;

    static void Log(const std::string& message);
    static void Warning(const std::string& message);
    static void Error(const std::string& message);
    static void SDLLog(const std::string& message, const char* sdlerror);
    static void SDLError(const std::string& message, const char* sdlerror);

    // Clears only the in-memory log, never files from earlier runs.
    static void Clear();
    static std::deque<LogEntry> GetLog() { return messages; }

private:
    static std::deque<LogEntry> messages;
    static std::string GetTimestamp();
    static void AppendLog(LogEntry entry);
};
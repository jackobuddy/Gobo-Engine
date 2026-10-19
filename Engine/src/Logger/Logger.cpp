#include "Logger.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string_view>
#include <system_error>
#include <vector>

namespace
{
    namespace fs = std::filesystem;

    constexpr std::size_t MaxLogFiles = 5;
    constexpr std::size_t MaxLogBytes = 2 * 1024 * 1024;
    constexpr std::string_view SizeLimitMessage =
        "Logger: file size limit reached; later messages remain in console "
        "and memory only.\n";

    std::ofstream logFile;
    fs::path activeLogPath;
    std::size_t bytesWritten = 0;

    void ReportFileIssue(const char* message) noexcept
    {
        try
        {
            std::cerr << "Logger: " << message << '\n';
        }
        catch (...)
        {
            // A failure while reporting a logging failure must not stop the game.
        }
    }

    void CloseFile() noexcept
    {
        try
        {
            if (logFile.is_open())
                logFile.close();

            logFile.clear();
            activeLogPath.clear();
            bytesWritten = 0;
        }
        catch (...)
        {
            // File logging is optional.
        }
    }

    bool GetLocalTime(std::time_t time, std::tm& local) noexcept
    {
#ifdef _WIN32
        return localtime_s(&local, &time) == 0;
#else
        return localtime_r(&time, &local) != nullptr;
#endif
    }

    void WriteFileLine(const std::string& line) noexcept
    {
        if (!logFile.is_open())
            return;

        try
        {
            const std::size_t remaining =
                bytesWritten < MaxLogBytes ? MaxLogBytes - bytesWritten : 0;

            // Reserve room for a clear truncation notice.
            if (remaining <= SizeLimitMessage.size() ||
                line.size() >= remaining - SizeLimitMessage.size())
            {
                logFile << SizeLimitMessage;
                logFile.flush();
                ReportFileIssue("file size limit reached");
                CloseFile();
                return;
            }

            logFile << line << '\n';
            logFile.flush(); // Preserve recent errors if the process crashes.

            if (!logFile)
            {
                ReportFileIssue(
                    "file write failed; using console and memory only");
                CloseFile();
                return;
            }

            bytesWritten += line.size() + 1;
        }
        catch (...)
        {
            ReportFileIssue(
                "file write failed; using console and memory only");
            CloseFile();
        }
    }

    void PruneOldLogs(const fs::path& directory) noexcept
    {
        try
        {
            std::vector<fs::path> logs;
            std::error_code error;

            fs::directory_iterator it(directory, error);
            const fs::directory_iterator end;

            for (; !error && it != end; it.increment(error))
            {
                const fs::path path = it->path();
                const std::string name = path.filename().string();

                if (name.rfind("GoboLog_", 0) != 0 ||
                    path.extension() != ".log")
                    continue;

                std::error_code entryError;
                if (it->is_regular_file(entryError) && !entryError)
                    logs.push_back(path);
            }

            if (error)
            {
                ReportFileIssue("could not inspect old log files");
                return;
            }

            // The timestamp-first filenames sort newest first.
            std::sort(logs.begin(), logs.end(),
                [](const fs::path& a, const fs::path& b)
                {
                    return a.filename().native() >
                        b.filename().native();
                });

            std::size_t kept = 0;
            for (const fs::path& path : logs)
            {
                // Keep the active file even if the system clock moved backward.
                if (path == activeLogPath || kept < MaxLogFiles)
                {
                    ++kept;
                    continue;
                }

                error.clear();
                fs::remove(path, error);
                if (error)
                    ReportFileIssue("could not remove an old log file");
            }
        }
        catch (...)
        {
            ReportFileIssue("could not clean up old log files");
        }
    }
}

std::deque<LogEntry> Logger::messages;

bool Logger::BeginSession(const char* gameRootUtf8) noexcept
{
    EndSession();

    if (!gameRootUtf8 || !*gameRootUtf8)
    {
        ReportFileIssue(
            "game directory unavailable; using console and memory only");
        return false;
    }

    try
    {
        const fs::path directory = fs::u8path(gameRootUtf8) / "Logs";

        std::error_code error;
        fs::create_directories(directory, error);
        if (error || !fs::is_directory(directory, error) || error)
        {
            ReportFileIssue(
                "Logs folder unavailable; using console and memory only");
            return false;
        }

        const auto now = std::chrono::system_clock::now();
        std::tm local{};
        if (!GetLocalTime(std::chrono::system_clock::to_time_t(now), local))
        {
            ReportFileIssue("could not determine local time");
            return false;
        }

        const auto milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count() % 1000;

        std::ostringstream baseName;
        baseName << "GoboLog_"
            << std::put_time(&local, "%Y-%m-%d_%H-%M-%S")
            << '_' << std::setw(3) << std::setfill('0')
            << milliseconds;

        // The suffix distinguishes launches within the same millisecond.
        for (int attempt = 0; attempt < 10000; ++attempt)
        {
            std::ostringstream name;
            name << baseName.str() << '_'
                << std::setw(4) << std::setfill('0') << attempt
                << ".log";

            const fs::path candidate = directory / name.str();

            error.clear();
            const bool exists = fs::exists(candidate, error);
            if (error)
            {
                ReportFileIssue("could not inspect the Logs folder");
                return false;
            }
            if (exists)
                continue;

            logFile.open(candidate,
                std::ios::out | std::ios::binary | std::ios::trunc);

            if (!logFile)
            {
                CloseFile();
                ReportFileIssue(
                    "could not create log file; using console and memory only");
                return false;
            }

            activeLogPath = candidate;
            bytesWritten = 0;

            WriteFileLine(
                "===== Session started " + GetTimestamp() + " =====");

            if (!logFile.is_open())
                return false;

            PruneOldLogs(directory);
            return true;
        }

        ReportFileIssue("could not choose a unique log filename");
        return false;
    }
    catch (...)
    {
        CloseFile();
        ReportFileIssue(
            "could not start file logging; using console and memory only");
        return false;
    }
}

void Logger::EndSession() noexcept
{
    if (logFile.is_open())
    {
        try
        {
            WriteFileLine(
                "===== Session ended " + GetTimestamp() + " =====");
        }
        catch (...)
        {
            ReportFileIssue("could not write session end");
        }
    }

    CloseFile();
}

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
    entry.message = "Warning [" + GetTimestamp() + "]: " + message;
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
    entry.message = "Log [" + GetTimestamp() + "]: " + message +
        ": " + (sdlerror ? sdlerror : "");
    std::cout << entry.message << std::endl;
    AppendLog(entry);
}

void Logger::SDLError(const std::string& message, const char* sdlerror)
{
    LogEntry entry;
    entry.type = LogType::ERROR;
    entry.message = "Error [" + GetTimestamp() + "]: " + message +
        ": " + (sdlerror ? sdlerror : "");
    std::cout << "\033[31m" << entry.message << "\033[0m" << std::endl;
    AppendLog(entry);
}

std::string Logger::GetTimestamp()
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    if (!GetLocalTime(now, local))
        return "time unavailable";

    std::ostringstream text;
    text << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
    return text.str();
}

void Logger::AppendLog(LogEntry entry)
{
    if (messages.size() >= 1000)
        messages.pop_front();

    messages.push_back(entry);
    WriteFileLine(entry.message);
}

void Logger::Clear()
{
    messages.clear();
}
#include <catch2/catch_test_macros.hpp>
#include <Logger/Logger.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace
{
    namespace fs = std::filesystem;

    constexpr std::uintmax_t MaxLogBytes = 2 * 1024 * 1024;

    struct TestLogRoot
    {
        fs::path path;

        TestLogRoot()
        {
            static unsigned nextID = 0;
            const auto tick =
                std::chrono::steady_clock::now()
                .time_since_epoch().count();

            path = fs::temp_directory_path() /
                ("GoboLoggerTests_" + std::to_string(tick) + "_" +
                    std::to_string(++nextID));

            fs::create_directories(path);
        }

        ~TestLogRoot()
        {
            Logger::EndSession();

            std::error_code error;
            fs::remove_all(path, error);
        }

        std::string Utf8Path() const
        {
            const auto utf8 = path.u8string();
            return std::string(utf8.begin(), utf8.end());
        }
    };

    std::vector<fs::path> LogFiles(const fs::path& gameRoot)
    {
        std::vector<fs::path> files;

        for (const auto& entry :
            fs::directory_iterator(gameRoot / "Logs"))
        {
            if (entry.is_regular_file() &&
                entry.path().extension() == ".log" &&
                entry.path().filename().string().rfind(
                    "GoboLog_", 0) == 0)
            {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end());
        return files;
    }

    std::string ReadFile(const fs::path& path)
    {
        std::ifstream input(path, std::ios::binary);
        return std::string(
            std::istreambuf_iterator<char>(input),
            std::istreambuf_iterator<char>());
    }

    struct CaptureStdout
    {
        std::ostringstream captured;
        std::streambuf* previous = std::cout.rdbuf(captured.rdbuf());

        ~CaptureStdout()
        {
            std::cout.rdbuf(previous);
        }
    };
}

TEST_CASE("Logger writes a readable dated session file", "[logger]")
{
    TestLogRoot root;
    Logger::Clear();

    const std::string rootPath = root.Utf8Path();
    REQUIRE(Logger::BeginSession(rootPath.c_str()));

    Logger::Log("normal marker");
    Logger::Warning("warning marker");
    Logger::Error("error marker");
    Logger::EndSession();

    const auto files = LogFiles(root.path);
    REQUIRE(files.size() == 1);

    const std::regex expectedName(
        R"(GoboLog_\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}_\d{3}_\d{4}\.log)");
    CHECK(std::regex_match(
        files.front().filename().string(), expectedName));

    const std::string contents = ReadFile(files.front());
    CHECK(contents.find("Session started") != std::string::npos);
    CHECK(contents.find("Log [") != std::string::npos);
    CHECK(contents.find("normal marker") != std::string::npos);
    CHECK(contents.find("Warning [") != std::string::npos);
    CHECK(contents.find("warning marker") != std::string::npos);
    CHECK(contents.find("Error [") != std::string::npos);
    CHECK(contents.find("error marker") != std::string::npos);
    CHECK(contents.find("Session ended") != std::string::npos);

    // Clearing memory must not delete the saved log.
    Logger::Clear();
    CHECK(Logger::GetLog().empty());
    CHECK(fs::exists(files.front()));
}

TEST_CASE("Logger retains only the five newest runs", "[logger]")
{
    TestLogRoot root;
    Logger::Clear();

    const std::string rootPath = root.Utf8Path();

    for (int run = 0; run < 7; ++run)
    {
        REQUIRE(Logger::BeginSession(rootPath.c_str()));
        Logger::Log("run-" + std::to_string(run));
        Logger::EndSession();
    }

    const auto files = LogFiles(root.path);
    REQUIRE(files.size() == 5);

    std::string retainedContents;
    for (const fs::path& file : files)
        retainedContents += ReadFile(file);

    CHECK(retainedContents.find("run-0") == std::string::npos);
    CHECK(retainedContents.find("run-1") == std::string::npos);

    for (int run = 2; run < 7; ++run)
    {
        CHECK(retainedContents.find(
            "run-" + std::to_string(run)) != std::string::npos);
    }
}

TEST_CASE("Logger caps a file without stopping memory logging", "[logger]")
{
    TestLogRoot root;
    Logger::Clear();

    const std::string rootPath = root.Utf8Path();
    REQUIRE(Logger::BeginSession(rootPath.c_str()));

    {
        // Keep the deliberately large test message out of test output.
        CaptureStdout capture;
        Logger::Log(std::string(MaxLogBytes, 'x'));
        Logger::Log("after-file-limit");
    }

    Logger::EndSession();

    const auto files = LogFiles(root.path);
    REQUIRE(files.size() == 1);
    CHECK(fs::file_size(files.front()) <= MaxLogBytes);

    const std::string contents = ReadFile(files.front());
    CHECK(contents.find("file size limit reached") !=
        std::string::npos);
    CHECK(contents.find("after-file-limit") ==
        std::string::npos);

    const auto memoryLog = Logger::GetLog();
    REQUIRE_FALSE(memoryLog.empty());
    CHECK(memoryLog.back().message.find("after-file-limit") !=
        std::string::npos);
}

TEST_CASE("Logger falls back when the game root is not writable as a directory",
    "[logger]")
{
    TestLogRoot root;
    Logger::Clear();

    const fs::path blockedRoot = root.path / "regular-file";
    {
        std::ofstream blocker(blockedRoot);
        REQUIRE(blocker.is_open());
    }

    const auto utf8 = blockedRoot.u8string();
    const std::string blockedPath(utf8.begin(), utf8.end());

    CHECK_FALSE(Logger::BeginSession(blockedPath.c_str()));

    Logger::Log("fallback marker");

    const auto memoryLog = Logger::GetLog();
    REQUIRE_FALSE(memoryLog.empty());
    CHECK(memoryLog.back().message.find("fallback marker") !=
        std::string::npos);
}
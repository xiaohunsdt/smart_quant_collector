#include <gtest/gtest.h>
#include "config.h"

class DLoggerTest:public testing::Test
{
    public:
        //Suite
        static void SetUpTestSuite() {
            dolphindb::DLogger::SetMinLevel(dolphindb::DLogger::LevelDebug);
        }
        static void TearDownTestSuite()
        {
            dolphindb::DLogger::SetMinLevel(default_level);
        }

};

TEST_F(DLoggerTest, test_logTextType_with_default_level_debug)
{
    std::tuple<
        const char*,        // 字符串常量
        const void*,        // 指针常量
        std::string,        // std::string
        int,                // int
        char,               // char
        unsigned,           // unsigned int
        long,               // long
        unsigned long,      // unsigned long
        long long,          // long long
        unsigned long long, // unsigned long long
        float,              // float
        double,             // double
        long double         // long double
    > args = std::make_tuple(
        "Hello, DolphinDB!",    // const char*
        nullptr,            // const void*
        std::string("DDB"), // std::string
        42,                 // int
        'A',                // char
        123u,               // unsigned
        -12345L,            // long
        12345UL,            // unsigned long
        -9876543210LL,      // long long
        9876543210ULL,      // unsigned long long
        3.14f,              // float
        2.718,              // double
        1.61803398875L      // long double
    );
    constexpr size_t tuple_size = std::tuple_size<decltype(args)>::value;
    bool res = false;
    for (size_t i = 0; i < tuple_size; ++i) {
        switch (i) {
            case 0:
                res = dolphindb::DLogger::Info("now is :",std::get<0>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<0>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<0>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<0>(args));
                ASSERT_TRUE(res);
                break;
            case 1:
                res = dolphindb::DLogger::Info("now is :",std::get<1>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<1>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<1>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<1>(args));
                ASSERT_TRUE(res);
                break;
            case 2:
                res = dolphindb::DLogger::Info("now is :",std::get<2>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<2>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<2>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<2>(args));
                ASSERT_TRUE(res);
                break;
            case 3:
                res = dolphindb::DLogger::Info("now is :",std::get<3>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<3>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<3>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<3>(args));
                ASSERT_TRUE(res);
                break;
            case 4:
                res = dolphindb::DLogger::Info("now is :",std::get<4>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<4>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<4>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<4>(args));
                ASSERT_TRUE(res);
                break;
            case 5:
                res = dolphindb::DLogger::Info("now is :",std::get<5>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<5>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<5>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<5>(args));
                ASSERT_TRUE(res);
                break;
            case 6:
                res = dolphindb::DLogger::Info("now is :",std::get<6>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<6>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<6>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<6>(args));
                ASSERT_TRUE(res);
                break;
            case 7:
                res = dolphindb::DLogger::Info("now is :",std::get<7>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<7>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<7>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<7>(args));
                ASSERT_TRUE(res);
                break;
            case 8:
                res = dolphindb::DLogger::Info("now is :",std::get<8>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<8>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<8>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<8>(args));
                ASSERT_TRUE(res);
                break;
            case 9:
                res = dolphindb::DLogger::Info("now is :",std::get<9>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<9>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<9>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<9>(args));
                ASSERT_TRUE(res);
                break;
            case 10:
                res = dolphindb::DLogger::Info("now is :",std::get<10>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<10>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<10>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<10>(args));
                ASSERT_TRUE(res);
                break;
            case 11:
                res = dolphindb::DLogger::Info("now is :",std::get<11>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<11>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<11>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<11>(args));
                ASSERT_TRUE(res);
                break;
            case 12:
                res = dolphindb::DLogger::Info("now is :",std::get<12>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Debug("now is :",std::get<12>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Warn("now is :",std::get<12>(args));
                ASSERT_TRUE(res);
                res = dolphindb::DLogger::Error("now is :",std::get<12>(args));
                ASSERT_TRUE(res);
                break;
            default:
                break;
        }
    }
}

TEST_F(DLoggerTest, DLogger_funcs){
    char workDir[256]{};
    getcwd(workDir, sizeof(workDir));
    std::string file = std::string(workDir).append("/tempFile123");
    dolphindb::DLogger::SetLogFilePath(file);
    auto level = dolphindb::DLogger::GetMinLevel();
    dolphindb::DLogger::Info("123", "345");
    dolphindb::DLogger::SetLogFilePath("");
    dolphindb::DLogger::SetMinLevel(dolphindb::DLogger::Level::LevelWarn);
    dolphindb::DLogger::Info("123", "345");
    dolphindb::DLogger::Error("123", "345");
    remove(file.c_str());
}

TEST_F(DLoggerTest, logfile){
    char workDir[256]{};
    const char* path = getcwd(workDir, sizeof(workDir));
    printf("%s\n", path);
    std::string file = std::string(workDir).append("/tempFile123");
    dolphindb::DLogger::SetLogFilePath(file);
    auto level = dolphindb::DLogger::GetMinLevel();
    dolphindb::DLogger::Info("123", "345");
    dolphindb::DLogger::SetLogFilePath("");
    dolphindb::DLogger::SetMinLevel(dolphindb::DLogger::Level::LevelWarn);
    dolphindb::DLogger::Info("123", "345");
    dolphindb::DLogger::Error("123", "345");
    remove(file.c_str());
}

TEST_F(DLoggerTest, test_logLevel)
{
    {
        dolphindb::DLogger::SetMinLevel(dolphindb::DLogger::Level::LevelDebug);
        bool res = dolphindb::DLogger::Debug("debug");
        ASSERT_TRUE(res);
        res = dolphindb::DLogger::Info("info");
        ASSERT_TRUE(res);
        res = dolphindb::DLogger::Warn("warn");
        ASSERT_TRUE(res);
        res = dolphindb::DLogger::Error("error");
        ASSERT_TRUE(res);
    }
    {
        dolphindb::DLogger::SetMinLevel(dolphindb::DLogger::Level::LevelInfo);
        bool res = dolphindb::DLogger::Debug("debug");
        ASSERT_FALSE(res);
        res = dolphindb::DLogger::Info("info");
        ASSERT_TRUE(res);
        res = dolphindb::DLogger::Warn("warn");
        ASSERT_TRUE(res);
        res = dolphindb::DLogger::Error("error");
        ASSERT_TRUE(res);
    }
    {
        dolphindb::DLogger::SetMinLevel(dolphindb::DLogger::Level::LevelWarn);
        bool res = dolphindb::DLogger::Debug("debug");
        ASSERT_FALSE(res);
        res = dolphindb::DLogger::Info("info");
        ASSERT_FALSE(res);
        res = dolphindb::DLogger::Warn("warn");
        ASSERT_TRUE(res);
        res = dolphindb::DLogger::Error("error");
        ASSERT_TRUE(res);
    }
    {
        dolphindb::DLogger::SetMinLevel(dolphindb::DLogger::Level::LevelError);
        bool res = dolphindb::DLogger::Debug("debug");
        ASSERT_FALSE(res);
        res = dolphindb::DLogger::Info("info");
        ASSERT_FALSE(res);
        res = dolphindb::DLogger::Warn("warn");
        ASSERT_FALSE(res);
        res = dolphindb::DLogger::Error("error");
        ASSERT_TRUE(res);
    }
}
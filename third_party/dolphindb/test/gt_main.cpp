#include <gtest/gtest.h>
#include "config.h"

int main(int argc, char *argv[]){
    dolphindb::DLogger::SetMinLevel(default_level);
    dolphindb::DBConnection::initialize();
    testing::InitGoogleTest(&argc, argv);
    // test filter example:
    // ?	单个字符
    // *	任意字符
    // -	排除，如，-a 表示除了a
    // :	取或，如，a:b 表示a或b
    // ::testing::GTEST_FLAG(filter) = "*Counter*:*DolphinDBTest.*huge*:ScalarTest.testGuid";

    // 运行测试
    int res = RUN_ALL_TESTS();
    return res;
}

#include <gtest/gtest.h>
#include "config.h"

class DataformPairTest:public testing::Test
{
    public:
        static dolphindb::DBConnection conn;
        // Suite
        static void SetUpTestSuite()
        {
            bool ret = conn.connect(HOST, PORT, USER, PASSWD);
            if (!ret)
            {
                std::cout << "Failed to connect to the server" << std::endl;
            }
            else
            {
                std::cout << "connect to " + HOST + ":" + std::to_string(PORT) << std::endl;
            }
        }
        static void TearDownTestSuite()
        {
            conn.close();
        }

    protected:
        // Case
        virtual void SetUp()
        {

        }
        virtual void TearDown()
        {

        }
};

dolphindb::DBConnection DataformPairTest::conn(false, false);

TEST_F(DataformPairTest,testAnyPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_ANY);
    ASSERT_EQ(v1.isNull(),true);
}

TEST_F(DataformPairTest,testBlobPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_BLOB);
    v1->set(0, dolphindb::Util::createBlob("123abc"));
    v1->set(1, dolphindb::Util::createBlob("中文*……%#￥#！a"));
    std::string script = "a=pair(blob(`123abc), blob('中文*……%#￥#！a'));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testStringPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_STRING);
    v1->set(0, dolphindb::Util::createString("123abc"));
    v1->set(1, dolphindb::Util::createString("中文*……%#￥#！a"));
    std::string script = "a=pair(`123abc, '中文*……%#￥#！a');a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testStringNullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_STRING);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(string(NULL), string(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testBoolPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_BOOL);
    v1->set(0, dolphindb::Util::createBool(true));
    v1->set(1, dolphindb::Util::createBool(false));
    std::string script = "a=pair(true, false);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testBoolNullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_BOOL);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(bool(NULL), bool(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testCharPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_CHAR);
    v1->set(0, dolphindb::Util::createChar(1));
    v1->set(1, dolphindb::Util::createChar(0));
    std::string script = "a=pair(char(1), char(0));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testCharNullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_CHAR);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(char(NULL), char(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testIntPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_INT);
    v1->set(0, dolphindb::Util::createInt(1));
    v1->set(1, dolphindb::Util::createInt(0));
    std::string script = "a=pair(int(1), int(0));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testIntNullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_INT);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(int(NULL), int(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testLongPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_LONG);
    v1->set(0, dolphindb::Util::createLong(1));
    v1->set(1, dolphindb::Util::createLong(0));
    std::string script = "a=pair(long(1), long(0));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testLongNullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_LONG);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(long(NULL), long(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testShortNullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_SHORT);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(short(NULL), short(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testShortPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_SHORT);
    v1->set(0, dolphindb::Util::createShort(1));
    v1->set(1, dolphindb::Util::createShort(0));
    std::string script = "a=pair(short(1), short(0));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testFloatPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_FLOAT);
    v1->set(0, dolphindb::Util::createFloat(1));
    v1->set(1, dolphindb::Util::createFloat(0));
    std::string script = "a=pair(float(1), float(0));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}


TEST_F(DataformPairTest,testFloatNullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_FLOAT);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(float(NULL), float(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDoublePair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DOUBLE);
    v1->set(0, dolphindb::Util::createDouble(1));
    v1->set(1, dolphindb::Util::createDouble(0));
    std::string script = "a=pair(double(1), double(0));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}


TEST_F(DataformPairTest,testDoubleNullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DOUBLE);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(double(NULL), double(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDatehourPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DATEHOUR);
    v1->set(0, dolphindb::Util::createDateHour(1));
    v1->set(1, dolphindb::Util::createDateHour(100000));
    std::string script = "a=pair(datehour(1), datehour(100000));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDatehourNullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DATEHOUR);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(datehour(NULL), datehour(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDatePair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DATE);
    v1->set(0, dolphindb::Util::createDate(1));
    v1->set(1, dolphindb::Util::createDate(48750));
    std::string script = "a=pair(date(1), date(48750));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDatenullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DATE);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(date(NULL), date(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testMinutePair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_MINUTE);
    v1->set(0, dolphindb::Util::createMinute(1));
    v1->set(1, dolphindb::Util::createMinute(1000));
    std::string script = "a=pair(minute(1), minute(1000));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testMinutenullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_MINUTE);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(minute(NULL), minute(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());

    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDatetimePair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DATETIME);
    v1->set(0, dolphindb::Util::createDateTime(1));
    v1->set(1, dolphindb::Util::createDateTime(48750));
    std::string script = "a=pair(datetime(1), datetime(48750));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDatetimenullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DATETIME);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(datetime(NULL), datetime(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testTimeStampPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_TIMESTAMP);
    v1->set(0, dolphindb::Util::createTimestamp(1));
    v1->set(1, dolphindb::Util::createTimestamp((long long)1000000000000));
    std::string script = "a=pair(timestamp(1), timestamp(1000000000000));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testTimeStampnullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_TIMESTAMP);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(timestamp(NULL), timestamp(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testnanotimePair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_NANOTIME);
    v1->set(0, dolphindb::Util::createNanoTime(1));
    v1->set(1, dolphindb::Util::createNanoTime((long long)1000000000000));
    std::string script = "a=pair(nanotime(1), nanotime(1000000000000));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testnanotimenullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_NANOTIME);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(nanotime(NULL), nanotime(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testnanotimestampPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_NANOTIMESTAMP);
    v1->set(0, dolphindb::Util::createNanoTimestamp(1));
    v1->set(1, dolphindb::Util::createNanoTimestamp((long long)1000000000000));
    std::string script = "a=pair(nanotimestamp(1), nanotimestamp(1000000000000));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testnanotimestampnullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_NANOTIMESTAMP);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(nanotimestamp(NULL), nanotimestamp(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testmonthPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_MONTH);
    v1->set(0, dolphindb::Util::createMonth(1));
    v1->set(1, dolphindb::Util::createMonth(1000));
    std::string script = "a=pair(month(1), month(1000));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testmonthnullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_MONTH);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(month(NULL), month(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testtimePair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_TIME);
    v1->set(0, dolphindb::Util::createTime(1));
    v1->set(1, dolphindb::Util::createTime((long)10000000));
    std::string script = "a=pair(time(1), time(10000000));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testtimenullPair){
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_TIME);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(time(NULL), time(NULL));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDecimal32Pair){
    srand((int)time(NULL));
    int scale = rand()%9;
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DECIMAL32, scale);
    dolphindb::ConstantSP val0 = dolphindb::Util::createConstant(dolphindb::DT_DECIMAL32, scale);
    dolphindb::ConstantSP val1 = dolphindb::Util::createConstant(dolphindb::DT_DECIMAL32, scale);
    val0->setString("1.1111111111111111111111111111");
    val1->setString("-3.1111111111111111111111111111");
    v1->set(0, val0);
    v1->set(1, val1);
    std::string script = "a=pair(decimal32(`1.1111111111111111111111111111,"+std::to_string(scale)+"),decimal32('-3.1111111111111111111111111111',"+std::to_string(scale)+"));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDecimal32NullPair){
    srand((int)time(NULL));
    int scale = rand()%9;
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DECIMAL32, scale);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(decimal32(NULL,"+std::to_string(scale)+"),decimal32(NULL,"+std::to_string(scale)+"));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDecimal64Pair){
    srand((int)time(NULL));
    int scale = rand()%18;
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DECIMAL64, scale);
    dolphindb::ConstantSP val0 = dolphindb::Util::createConstant(dolphindb::DT_DECIMAL64, scale);
    dolphindb::ConstantSP val1 = dolphindb::Util::createConstant(dolphindb::DT_DECIMAL64, scale);
    val0->setString("1.1111111111111111111111111111");
    val1->setString("-3.1111111111111111111111111111");
    v1->set(0, val0);
    v1->set(1, val1);
    std::string script = "a=pair(decimal64(`1.1111111111111111111111111111,"+std::to_string(scale)+"),decimal64('-3.1111111111111111111111111111',"+std::to_string(scale)+"));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDecimal64NullPair){
    srand((int)time(NULL));
    int scale = rand()%9;
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DECIMAL64, scale);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(decimal64(NULL,"+std::to_string(scale)+"),decimal64(NULL,"+std::to_string(scale)+"));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDecimal128Pair){
    srand((int)time(NULL));
    int scale = rand()%38;
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DECIMAL128, scale);
    dolphindb::ConstantSP val0 = dolphindb::Util::createConstant(dolphindb::DT_DECIMAL128, scale);
    dolphindb::ConstantSP val1 = dolphindb::Util::createConstant(dolphindb::DT_DECIMAL128, scale);
    val0->setString("1.1111111111111111111111111111");
    val1->setString("-3.1111111111111111111111111111");
    v1->set(0, val0);
    v1->set(1, val1);
    std::string script = "a=pair(decimal128(`1.1111111111111111111111111111,"+std::to_string(scale)+"),decimal128('-3.1111111111111111111111111111',"+std::to_string(scale)+"));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest,testDecimal128NullPair){
    srand((int)time(NULL));
    int scale = rand()%38;
    dolphindb::VectorSP v1 = dolphindb::Util::createPair(dolphindb::DT_DECIMAL128, scale);
    v1->setNull(0);
    v1->setNull(1);
    std::string script = "a=pair(decimal128(NULL,"+std::to_string(scale)+"),decimal128(NULL,"+std::to_string(scale)+"));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(),true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isPair(),true);
}

TEST_F(DataformPairTest, test_stringPair_exception){
    std::string ss("123\0 123", 8);
    {
        try{
            dolphindb::VectorSP v = dolphindb::Util::createPair(dolphindb::DT_STRING);
            v->appendString(&ss, 1);
        }catch(std::exception& e){
            ASSERT_EQ(std::string(e.what()), "A String cannot contain the character '\\0'");
        }
    }
    {
        dolphindb::VectorSP v = dolphindb::Util::createPair(dolphindb::DT_STRING);
        v->append(dolphindb::Util::createString("aaa"));
        try{
            v->setString(ss);
        }catch(std::exception& e){
            ASSERT_EQ(std::string(e.what()), "A String cannot contain the character '\\0'");
        }
        try{
            v->setString(0, ss);
        }catch(std::exception& e){
            ASSERT_EQ(std::string(e.what()), "A String cannot contain the character '\\0'");
        }
        try{
            v->setString(0, 1, &ss);
        }catch(std::exception& e){
            ASSERT_EQ(std::string(e.what()), "A String cannot contain the character '\\0'");
        }
        try{
            v->appendString(&ss, 1);
        }catch(std::exception& e){
            ASSERT_EQ(std::string(e.what()), "A String cannot contain the character '\\0'");
        }
    }
    {
        dolphindb::VectorSP v = dolphindb::Util::createPair(dolphindb::DT_SYMBOL);
        v->append(dolphindb::Util::createString("aaa"));
        try{
            v->setString(ss);
        }catch(std::exception& e){
            ASSERT_EQ(std::string(e.what()), "A String cannot contain the character '\\0'");
        }
        try{
            v->setString(0, ss);
        }catch(std::exception& e){
            ASSERT_EQ(std::string(e.what()), "A String cannot contain the character '\\0'");
        }
        try{
            v->setString(0, 1, &ss);
        }catch(std::exception& e){
            ASSERT_EQ(std::string(e.what()), "A String cannot contain the character '\\0'");
        }
        try{
            v->appendString(&ss, 1);
        }catch(std::exception& e){
            ASSERT_EQ(std::string(e.what()), "A String cannot contain the character '\\0'");
        }
    }
}
#include <gtest/gtest.h>
#include "config.h"

class AutoFitTableappenderTest:public testing::Test
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

dolphindb::DBConnection AutoFitTableappenderTest::conn(false, false);

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_unconnected){
    dolphindb::DBConnection connNew(false, false);
    ASSERT_ANY_THROW(dolphindb::AutoFitTableAppender appender("", "st1", connNew));

}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_appendErrTableColNums){
    conn.run("temp_tab=table(`a`x`d`s`cs as col1, 2 3 4 5 6 as col2)");
    dolphindb::AutoFitTableAppender appender("", "temp_tab", conn);
    std::vector<std::string> colNames = {"col1", "col2", "col3"};
    std::vector<dolphindb::ConstantSP> cols ={dolphindb::Util::createString("zzz123中文a"), dolphindb::Util::createInt(7), dolphindb::Util::createInt(5)};
    dolphindb::TableSP t = dolphindb::Util::createTable(colNames, cols);
    ASSERT_ANY_THROW(appender.append(t));
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_appendErrTableColType){
    conn.run("temp_tab=table(`a`x`d`s`cs as col1, 2 3 4 5 6 as col2)");
    dolphindb::AutoFitTableAppender appender("", "temp_tab", conn);
    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::ConstantSP> cols ={dolphindb::Util::createString("zzz123中文a"), dolphindb::Util::createDate(1000)};
    dolphindb::TableSP t = dolphindb::Util::createTable(colNames, cols);
    ASSERT_ANY_THROW(appender.append(t));
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_appendAllDataTypesToinMemoryTable){
    srand((int)time(NULL));
    int colNum = 25, rowNum = 1000;
    int scale32 = rand()%9, scale64 = rand()%18, scale128 = rand()%38;
    std::vector<std::string> colNamesVec1;
    for (int i = 0; i < colNum; i++){
        colNamesVec1.emplace_back("col"+std::to_string(i));
    }
    std::vector<dolphindb::DATA_TYPE> colTypesVec1;
    colTypesVec1.emplace_back(dolphindb::DT_CHAR);
    colTypesVec1.emplace_back(dolphindb::DT_BOOL);
    colTypesVec1.emplace_back(dolphindb::DT_SHORT);
    colTypesVec1.emplace_back(dolphindb::DT_INT);
    colTypesVec1.emplace_back(dolphindb::DT_LONG);
    colTypesVec1.emplace_back(dolphindb::DT_DATE);
    colTypesVec1.emplace_back(dolphindb::DT_MONTH);
    colTypesVec1.emplace_back(dolphindb::DT_TIME);
    colTypesVec1.emplace_back(dolphindb::DT_MINUTE);
    colTypesVec1.emplace_back(dolphindb::DT_DATETIME);
    colTypesVec1.emplace_back(dolphindb::DT_SECOND);
    colTypesVec1.emplace_back(dolphindb::DT_TIMESTAMP);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIME);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIMESTAMP);
    colTypesVec1.emplace_back(dolphindb::DT_FLOAT);
    colTypesVec1.emplace_back(dolphindb::DT_DOUBLE);
    colTypesVec1.emplace_back(dolphindb::DT_STRING);
    colTypesVec1.emplace_back(dolphindb::DT_UUID);
    colTypesVec1.emplace_back(dolphindb::DT_IP);
    colTypesVec1.emplace_back(dolphindb::DT_INT128);
    colTypesVec1.emplace_back(dolphindb::DT_BLOB);
    colTypesVec1.emplace_back(dolphindb::DT_DATEHOUR);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL32);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL64);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL128);

    dolphindb::TableSP tab1 = dolphindb::Util::createTable(colNamesVec1, colTypesVec1, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tab1->getColumn(i));

    }
    for (int i = 0; i < rowNum-1; i++){
        columnVecs[0]->set(i, dolphindb::Util::createChar(rand()%CHAR_MAX));
        columnVecs[1]->set(i, dolphindb::Util::createBool((char)(rand()%2)));
        columnVecs[2]->set(i, dolphindb::Util::createShort(rand()%SHRT_MAX));
        columnVecs[3]->set(i, dolphindb::Util::createInt(rand()%INT_MAX));
        columnVecs[4]->set(i, dolphindb::Util::createLong(rand()%LLONG_MAX));
        columnVecs[5]->set(i, dolphindb::Util::createDate(rand()%INT_MAX));
        columnVecs[6]->set(i, dolphindb::Util::createMonth(rand()%INT_MAX));
        columnVecs[7]->set(i, dolphindb::Util::createTime(rand()%INT_MAX));
        columnVecs[8]->set(i, dolphindb::Util::createMinute(rand()%1440));
        columnVecs[9]->set(i, dolphindb::Util::createDateTime(rand()%INT_MAX));
        columnVecs[10]->set(i, dolphindb::Util::createSecond(rand()%86400));
        columnVecs[11]->set(i, dolphindb::Util::createTimestamp(rand()%LLONG_MAX));
        columnVecs[12]->set(i, dolphindb::Util::createNanoTime(rand()%LLONG_MAX));
        columnVecs[13]->set(i, dolphindb::Util::createNanoTimestamp(rand()%LLONG_MAX));
        columnVecs[14]->set(i, dolphindb::Util::createFloat(rand()/float(RAND_MAX)));
        columnVecs[15]->set(i, dolphindb::Util::createDouble(rand()/double(RAND_MAX)));
        columnVecs[16]->set(i, dolphindb::Util::createString("str"+std::to_string(i)));
        columnVecs[17]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_UUID,"5d212a78-cc48-e3b1-4235-b4d91473ee87"));
        columnVecs[18]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_IP,"192.0.0."+std::to_string(rand()%255)));
        columnVecs[19]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec32"));
        columnVecs[20]->set(i, dolphindb::Util::createBlob("blob"+std::to_string(i)));
        columnVecs[21]->set(i, dolphindb::Util::createDateHour(rand()%INT_MAX));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal32(scale32,rand()/float(RAND_MAX)));
        columnVecs[23]->set(i, dolphindb::Util::createDecimal64(scale64,rand()/double(RAND_MAX)));
        columnVecs[24]->set(i, dolphindb::Util::createDecimal128(scale128,rand()/double(RAND_MAX)));
    }
    for (int j = 0; j < colNum; j++)
        columnVecs[j]->setNull(rowNum-1);

    std::string script1;
    script1 += "temp = table(100:0, take(`col,25)+string(0..24), \
    [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, BLOB, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+"), DECIMAL128("+std::to_string(scale128)+")]);";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "temp", conn);
    int res = appender.append(tab1);
    ASSERT_EQ(res, rowNum);

    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "each(eqObj, tab1.values(), temp.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_appendAllDataTypesToindexedTable){
    srand((int)time(NULL));
    int colNum = 25, rowNum = 1000;
    int scale32 = rand()%9, scale64 = rand()%18, scale128 = rand()%38;
    std::vector<std::string> colNamesVec1;
    for (int i = 0; i < colNum; i++){
        colNamesVec1.emplace_back("col"+std::to_string(i));
    }
    std::vector<dolphindb::DATA_TYPE> colTypesVec1;
    colTypesVec1.emplace_back(dolphindb::DT_CHAR);
    colTypesVec1.emplace_back(dolphindb::DT_BOOL);
    colTypesVec1.emplace_back(dolphindb::DT_SHORT);
    colTypesVec1.emplace_back(dolphindb::DT_INT);
    colTypesVec1.emplace_back(dolphindb::DT_LONG);
    colTypesVec1.emplace_back(dolphindb::DT_DATE);
    colTypesVec1.emplace_back(dolphindb::DT_MONTH);
    colTypesVec1.emplace_back(dolphindb::DT_TIME);
    colTypesVec1.emplace_back(dolphindb::DT_MINUTE);
    colTypesVec1.emplace_back(dolphindb::DT_DATETIME);
    colTypesVec1.emplace_back(dolphindb::DT_SECOND);
    colTypesVec1.emplace_back(dolphindb::DT_TIMESTAMP);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIME);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIMESTAMP);
    colTypesVec1.emplace_back(dolphindb::DT_FLOAT);
    colTypesVec1.emplace_back(dolphindb::DT_DOUBLE);
    colTypesVec1.emplace_back(dolphindb::DT_STRING);
    colTypesVec1.emplace_back(dolphindb::DT_UUID);
    colTypesVec1.emplace_back(dolphindb::DT_IP);
    colTypesVec1.emplace_back(dolphindb::DT_INT128);
    colTypesVec1.emplace_back(dolphindb::DT_BLOB);
    colTypesVec1.emplace_back(dolphindb::DT_DATEHOUR);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL32);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL64);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL128);

    dolphindb::TableSP tab1 = dolphindb::Util::createTable(colNamesVec1, colTypesVec1, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tab1->getColumn(i));
    }
    for (int i = 0; i < rowNum-1; i++){
        columnVecs[0]->set(i, dolphindb::Util::createChar(rand()%CHAR_MAX));
        columnVecs[1]->set(i, dolphindb::Util::createBool((char)(rand()%2)));
        columnVecs[2]->set(i, dolphindb::Util::createShort(rand()%SHRT_MAX));
        columnVecs[3]->set(i, dolphindb::Util::createInt(rand()%INT_MAX));
        columnVecs[4]->set(i, dolphindb::Util::createLong(rand()%LLONG_MAX));
        columnVecs[5]->set(i, dolphindb::Util::createDate(rand()%INT_MAX));
        columnVecs[6]->set(i, dolphindb::Util::createMonth(rand()%INT_MAX));
        columnVecs[7]->set(i, dolphindb::Util::createTime(rand()%INT_MAX));
        columnVecs[8]->set(i, dolphindb::Util::createMinute(rand()%1440));
        columnVecs[9]->set(i, dolphindb::Util::createDateTime(rand()%INT_MAX));
        columnVecs[10]->set(i, dolphindb::Util::createSecond(rand()%86400));
        columnVecs[11]->set(i, dolphindb::Util::createTimestamp(rand()%LLONG_MAX));
        columnVecs[12]->set(i, dolphindb::Util::createNanoTime(rand()%LLONG_MAX));
        columnVecs[13]->set(i, dolphindb::Util::createNanoTimestamp(rand()%LLONG_MAX));
        columnVecs[14]->set(i, dolphindb::Util::createFloat(rand()/float(RAND_MAX)));
        columnVecs[15]->set(i, dolphindb::Util::createDouble(rand()/double(RAND_MAX)));
        columnVecs[16]->set(i, dolphindb::Util::createString("str"+std::to_string(i)));
        columnVecs[17]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_UUID,"5d212a78-cc48-e3b1-4235-b4d91473ee87"));
        columnVecs[18]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_IP,"192.0.0."+std::to_string(rand()%255)));
        columnVecs[19]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec32"));
        columnVecs[20]->set(i, dolphindb::Util::createBlob("blob"+std::to_string(i)));
        columnVecs[21]->set(i, dolphindb::Util::createDateHour(rand()%INT_MAX));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal32(scale32,rand()/float(RAND_MAX)));
        columnVecs[23]->set(i, dolphindb::Util::createDecimal64(scale64,rand()/double(RAND_MAX)));
        columnVecs[24]->set(i, dolphindb::Util::createDecimal128(scale128,rand()/double(RAND_MAX)));
    }
    for (int j = 0; j < colNum; j++)
        columnVecs[j]->setNull(rowNum-1);

    std::string script1;
    script1 += "temp = table(100:0, take(`col,25)+string(0..24), \
    [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, BLOB, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+"), DECIMAL128("+std::to_string(scale128)+")]);";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "temp", conn);
    int res = appender.append(tab1);
    ASSERT_EQ(res, rowNum);

    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "each(eqObj, tab1.values(), temp.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
}


TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_appendAllDataTypesTokeyedTable){
    srand((int)time(NULL));
    int colNum = 25, rowNum = 1000;
    int scale32 = rand()%9, scale64 = rand()%18, scale128 = rand()%38;
    std::vector<std::string> colNamesVec1;
    for (int i = 0; i < colNum; i++){
        colNamesVec1.emplace_back("col"+std::to_string(i));
    }
    std::vector<dolphindb::DATA_TYPE> colTypesVec1;
    colTypesVec1.emplace_back(dolphindb::DT_CHAR);
    colTypesVec1.emplace_back(dolphindb::DT_BOOL);
    colTypesVec1.emplace_back(dolphindb::DT_SHORT);
    colTypesVec1.emplace_back(dolphindb::DT_INT);
    colTypesVec1.emplace_back(dolphindb::DT_LONG);
    colTypesVec1.emplace_back(dolphindb::DT_DATE);
    colTypesVec1.emplace_back(dolphindb::DT_MONTH);
    colTypesVec1.emplace_back(dolphindb::DT_TIME);
    colTypesVec1.emplace_back(dolphindb::DT_MINUTE);
    colTypesVec1.emplace_back(dolphindb::DT_DATETIME);
    colTypesVec1.emplace_back(dolphindb::DT_SECOND);
    colTypesVec1.emplace_back(dolphindb::DT_TIMESTAMP);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIME);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIMESTAMP);
    colTypesVec1.emplace_back(dolphindb::DT_FLOAT);
    colTypesVec1.emplace_back(dolphindb::DT_DOUBLE);
    colTypesVec1.emplace_back(dolphindb::DT_STRING);
    colTypesVec1.emplace_back(dolphindb::DT_UUID);
    colTypesVec1.emplace_back(dolphindb::DT_IP);
    colTypesVec1.emplace_back(dolphindb::DT_INT128);
    colTypesVec1.emplace_back(dolphindb::DT_BLOB);
    colTypesVec1.emplace_back(dolphindb::DT_DATEHOUR);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL32);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL64);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL128);

    dolphindb::TableSP tab1 = dolphindb::Util::createTable(colNamesVec1, colTypesVec1, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tab1->getColumn(i));

    }
    for (int i = 0; i < rowNum-1; i++){
        columnVecs[0]->set(i, dolphindb::Util::createChar(rand()%CHAR_MAX));
        columnVecs[1]->set(i, dolphindb::Util::createBool((char)(rand()%2)));
        columnVecs[2]->set(i, dolphindb::Util::createShort(rand()%SHRT_MAX));
        columnVecs[3]->set(i, dolphindb::Util::createInt(rand()%INT_MAX));
        columnVecs[4]->set(i, dolphindb::Util::createLong(rand()%LLONG_MAX));
        columnVecs[5]->set(i, dolphindb::Util::createDate(rand()%INT_MAX));
        columnVecs[6]->set(i, dolphindb::Util::createMonth(rand()%INT_MAX));
        columnVecs[7]->set(i, dolphindb::Util::createTime(rand()%INT_MAX));
        columnVecs[8]->set(i, dolphindb::Util::createMinute(rand()%1440));
        columnVecs[9]->set(i, dolphindb::Util::createDateTime(rand()%INT_MAX));
        columnVecs[10]->set(i, dolphindb::Util::createSecond(rand()%86400));
        columnVecs[11]->set(i, dolphindb::Util::createTimestamp(rand()%LLONG_MAX));
        columnVecs[12]->set(i, dolphindb::Util::createNanoTime(rand()%LLONG_MAX));
        columnVecs[13]->set(i, dolphindb::Util::createNanoTimestamp(rand()%LLONG_MAX));
        columnVecs[14]->set(i, dolphindb::Util::createFloat(rand()/float(RAND_MAX)));
        columnVecs[15]->set(i, dolphindb::Util::createDouble(rand()/double(RAND_MAX)));
        columnVecs[16]->set(i, dolphindb::Util::createString("str"+std::to_string(i)));
        columnVecs[17]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_UUID,"5d212a78-cc48-e3b1-4235-b4d91473ee87"));
        columnVecs[18]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_IP,"192.0.0."+std::to_string(rand()%255)));
        columnVecs[19]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec32"));
        columnVecs[20]->set(i, dolphindb::Util::createBlob("blob"+std::to_string(i)));
        columnVecs[21]->set(i, dolphindb::Util::createDateHour(rand()%INT_MAX));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal32(scale32,rand()/float(RAND_MAX)));
        columnVecs[23]->set(i, dolphindb::Util::createDecimal64(scale64,rand()/double(RAND_MAX)));
        columnVecs[24]->set(i, dolphindb::Util::createDecimal128(scale128,rand()/double(RAND_MAX)));
    }
    for (int j = 0; j < colNum; j++)
        columnVecs[j]->setNull(rowNum-1);

    std::string script1;
    script1 += "temp = table(100:0, take(`col,25)+string(0..24), \
    [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, BLOB, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+"), DECIMAL128("+std::to_string(scale128)+")]);";
    script1 += "st1 = keyedTable(`col16,temp);";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "st1", conn);
    int res = appender.append(tab1);
    ASSERT_EQ(res, rowNum);

    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "each(eqObj, tab1.values(), st1.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_appendAllDataTypesTopartitionedTableOLAP){
    std::string case_=getCaseName();
    srand((int)time(NULL));
    int colNum = 24, rowNum = 1000;
    int scale32 = rand()%9, scale64 = rand()%18, scale128 = rand()%38;
    std::vector<std::string> colNamesVec1;
    for (int i = 0; i < colNum; i++){
        colNamesVec1.emplace_back("col"+std::to_string(i));
    }
    std::vector<dolphindb::DATA_TYPE> colTypesVec1;
    colTypesVec1.emplace_back(dolphindb::DT_CHAR);
    colTypesVec1.emplace_back(dolphindb::DT_BOOL);
    colTypesVec1.emplace_back(dolphindb::DT_SHORT);
    colTypesVec1.emplace_back(dolphindb::DT_INT);
    colTypesVec1.emplace_back(dolphindb::DT_LONG);
    colTypesVec1.emplace_back(dolphindb::DT_DATE);
    colTypesVec1.emplace_back(dolphindb::DT_MONTH);
    colTypesVec1.emplace_back(dolphindb::DT_TIME);
    colTypesVec1.emplace_back(dolphindb::DT_MINUTE);
    colTypesVec1.emplace_back(dolphindb::DT_DATETIME);
    colTypesVec1.emplace_back(dolphindb::DT_SECOND);
    colTypesVec1.emplace_back(dolphindb::DT_TIMESTAMP);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIME);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIMESTAMP);
    colTypesVec1.emplace_back(dolphindb::DT_FLOAT);
    colTypesVec1.emplace_back(dolphindb::DT_DOUBLE);
    colTypesVec1.emplace_back(dolphindb::DT_STRING);
    colTypesVec1.emplace_back(dolphindb::DT_UUID);
    colTypesVec1.emplace_back(dolphindb::DT_IP);
    colTypesVec1.emplace_back(dolphindb::DT_INT128);
    colTypesVec1.emplace_back(dolphindb::DT_DATEHOUR);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL32);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL64);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL128);

    dolphindb::TableSP tab1 = dolphindb::Util::createTable(colNamesVec1, colTypesVec1, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tab1->getColumn(i));

    }
    for (int i = 0; i < rowNum-1; i++){
        columnVecs[0]->set(i, dolphindb::Util::createChar(rand()%CHAR_MAX));
        columnVecs[1]->set(i, dolphindb::Util::createBool((char)(rand()%2)));
        columnVecs[2]->set(i, dolphindb::Util::createShort(rand()%SHRT_MAX));
        columnVecs[3]->set(i, dolphindb::Util::createInt(rand()%INT_MAX));
        columnVecs[4]->set(i, dolphindb::Util::createLong(rand()%LLONG_MAX));
        columnVecs[5]->set(i, dolphindb::Util::createDate(rand()%INT_MAX));
        columnVecs[6]->set(i, dolphindb::Util::createMonth(rand()%INT_MAX));
        columnVecs[7]->set(i, dolphindb::Util::createTime(rand()%INT_MAX));
        columnVecs[8]->set(i, dolphindb::Util::createMinute(rand()%1440));
        columnVecs[9]->set(i, dolphindb::Util::createDateTime(rand()%INT_MAX));
        columnVecs[10]->set(i, dolphindb::Util::createSecond(rand()%86400));
        columnVecs[11]->set(i, dolphindb::Util::createTimestamp(rand()%LLONG_MAX));
        columnVecs[12]->set(i, dolphindb::Util::createNanoTime(rand()%LLONG_MAX));
        columnVecs[13]->set(i, dolphindb::Util::createNanoTimestamp(rand()%LLONG_MAX));
        columnVecs[14]->set(i, dolphindb::Util::createFloat(rand()/float(RAND_MAX)));
        columnVecs[15]->set(i, dolphindb::Util::createDouble(rand()/double(RAND_MAX)));
        columnVecs[16]->set(i, dolphindb::Util::createString("str"+std::to_string(i)));
        columnVecs[17]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_UUID,"5d212a78-cc48-e3b1-4235-b4d91473ee87"));
        columnVecs[18]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_IP,"192.0.0."+std::to_string(rand()%255)));
        columnVecs[19]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec32"));
        columnVecs[20]->set(i, dolphindb::Util::createDateHour(rand()%INT_MAX));
        columnVecs[21]->set(i, dolphindb::Util::createDecimal32(scale32,rand()/float(RAND_MAX)));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal64(scale64,rand()/double(RAND_MAX)));
        columnVecs[23]->set(i, dolphindb::Util::createDecimal128(scale128,rand()/double(RAND_MAX)));
    }
    for (int j = 0; j < colNum; j++){
        if(j == 3)
            columnVecs[3]->set(rowNum-1, dolphindb::Util::createInt(rand()%INT_MAX));  //partition-column's value must be not null
        else
            columnVecs[j]->setNull(rowNum-1);
    }

    std::string dbName ="dfs://"+case_;
    std::string tableName = "pt";
    std::string script = "dbName = \"" + dbName + "\""
            "if(exists(dbName)){dropDatabase(dbName)};"
            "db  = database(dbName, HASH,[INT,1]);"
            "temp = table(1000:0, take(`col,24)+string(0..23), \
            [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+"), DECIMAL128("+std::to_string(scale128)+")]);"
            "pt = createPartitionedTable(db,temp,`pt,`col3);";
    conn.run(script);
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    int res = appender.append(tab1);
    ASSERT_EQ(res, rowNum);

    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "st1 = select * from pt;";
    script3 += "each(eqObj, tab1.values(), st1.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_appendAllDataTypesTopartitionedTableTSDB){
    std::string case_=getCaseName();
    int colNum = 25, rowNum = 1000;
    int scale32 = rand()%9, scale64 = rand()%18, scale128 = rand()%38;
    std::vector<std::string> colNamesVec1;
    for (int i = 0; i < colNum; i++){
        colNamesVec1.emplace_back("col"+std::to_string(i));
    }
    std::vector<dolphindb::DATA_TYPE> colTypesVec1;
    colTypesVec1.emplace_back(dolphindb::DT_CHAR);
    colTypesVec1.emplace_back(dolphindb::DT_BOOL);
    colTypesVec1.emplace_back(dolphindb::DT_SHORT);
    colTypesVec1.emplace_back(dolphindb::DT_INT);
    colTypesVec1.emplace_back(dolphindb::DT_LONG);
    colTypesVec1.emplace_back(dolphindb::DT_DATE);
    colTypesVec1.emplace_back(dolphindb::DT_MONTH);
    colTypesVec1.emplace_back(dolphindb::DT_TIME);
    colTypesVec1.emplace_back(dolphindb::DT_MINUTE);
    colTypesVec1.emplace_back(dolphindb::DT_DATETIME);
    colTypesVec1.emplace_back(dolphindb::DT_SECOND);
    colTypesVec1.emplace_back(dolphindb::DT_TIMESTAMP);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIME);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIMESTAMP);
    colTypesVec1.emplace_back(dolphindb::DT_FLOAT);
    colTypesVec1.emplace_back(dolphindb::DT_DOUBLE);
    colTypesVec1.emplace_back(dolphindb::DT_STRING);
    colTypesVec1.emplace_back(dolphindb::DT_UUID);
    colTypesVec1.emplace_back(dolphindb::DT_IP);
    colTypesVec1.emplace_back(dolphindb::DT_INT128);
    colTypesVec1.emplace_back(dolphindb::DT_BLOB);
    colTypesVec1.emplace_back(dolphindb::DT_DATEHOUR);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL32);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL64);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL128);
    srand((int)time(NULL));
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(colNamesVec1, colTypesVec1, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tab1->getColumn(i));

    }
    for (int i = 0; i < rowNum-1; i++){
        columnVecs[0]->set(i, dolphindb::Util::createChar(rand()%CHAR_MAX));
        columnVecs[1]->set(i, dolphindb::Util::createBool((char)(rand()%2)));
        columnVecs[2]->set(i, dolphindb::Util::createShort(rand()%SHRT_MAX));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
        columnVecs[4]->set(i, dolphindb::Util::createLong(rand()%LLONG_MAX));
        columnVecs[5]->set(i, dolphindb::Util::createDate(rand()%INT_MAX));
        columnVecs[6]->set(i, dolphindb::Util::createMonth(rand()%INT_MAX));
        columnVecs[7]->set(i, dolphindb::Util::createTime(rand()%INT_MAX));
        columnVecs[8]->set(i, dolphindb::Util::createMinute(rand()%1440));
        columnVecs[9]->set(i, dolphindb::Util::createDateTime(rand()%INT_MAX));
        columnVecs[10]->set(i, dolphindb::Util::createSecond(rand()%86400));
        columnVecs[11]->set(i, dolphindb::Util::createTimestamp(rand()%LLONG_MAX));
        columnVecs[12]->set(i, dolphindb::Util::createNanoTime(rand()%LLONG_MAX));
        columnVecs[13]->set(i, dolphindb::Util::createNanoTimestamp(rand()%LLONG_MAX));
        columnVecs[14]->set(i, dolphindb::Util::createFloat(rand()/float(RAND_MAX)));
        columnVecs[15]->set(i, dolphindb::Util::createDouble(rand()/double(RAND_MAX)));
        columnVecs[16]->set(i, dolphindb::Util::createString("str"+std::to_string(i)));
        columnVecs[17]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_UUID,"5d212a78-cc48-e3b1-4235-b4d91473ee87"));
        columnVecs[18]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_IP,"192.0.0."+std::to_string(rand()%255)));
        columnVecs[19]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec32"));
        columnVecs[20]->set(i, dolphindb::Util::createBlob("blob"+std::to_string(i)));
        columnVecs[21]->set(i, dolphindb::Util::createDateHour(rand()%INT_MAX));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal32(scale32,rand()/float(RAND_MAX)));
        columnVecs[23]->set(i, dolphindb::Util::createDecimal64(scale64,rand()/double(RAND_MAX)));
        columnVecs[24]->set(i, dolphindb::Util::createDecimal128(scale128,rand()/double(RAND_MAX)));
    }
    for (int j = 0; j < colNum; j++){
        if(j == 3)
            columnVecs[3]->set(rowNum-1, dolphindb::Util::createInt(rand()%INT_MAX));
        else
            columnVecs[j]->setNull(rowNum-1);
    }
    std::string dbName ="dfs://"+case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName + "\""
            "if(exists(dbName)){dropDatabase(dbName)};"
            "db  = database(dbName, HASH,[STRING,1],,'TSDB');"
            "temp = table(100:0, take(`col,25)+string(0..24), \
            [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, BLOB, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+"), DECIMAL128("+std::to_string(scale128)+")]);"
            "pt = db.createPartitionedTable(temp,`pt,`col16,,`col16);";
    conn.run(script);
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    int res = appender.append(tab1);
    ASSERT_EQ(res, rowNum);

    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "st1 = select * from pt order by col3;res = select * from tab1 order by col3;";
    script3 += "each(eqObj, res.values(), st1.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_convert_to_date){
    std::string script1;
    script1 += "st1=table(100:0, `date1`date2`date3`date4, [DATE, DATE, DATE, DATE])";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "st1", conn);
    std::vector<std::string> colNames = { "date1", "date2", "date3", "date4" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_DATETIME, dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_DATE };
    int colNum = 4, rowNum = 10;
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(tmp1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createDateTime(1969, 1, i + 1, 12, 30, 30));
        columnVecs[1]->set(i, dolphindb::Util::createTimestamp(1969, 1, i + 1, 12, 30, 30, 123));
        columnVecs[2]->set(i, dolphindb::Util::createNanoTimestamp(1969, 1, i + 1, 12, 30, 30, 123456789));
        columnVecs[3]->set(i, dolphindb::Util::createDate(1969, 1, i + 1));
    }
    int res = appender.append(tmp1);
    ASSERT_EQ(res, rowNum);
    std::string script2;
    script2 += "expected = table(1969.01.01+0..9 as date1, 1969.01.01+0..9 as date2, 1969.01.01+0..9 as date3, 1969.01.01+0..9 as date4);";
    script2 += "each(eqObj, st1.values(), expected.values());";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);

    dolphindb::TableSP tmp2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs1;
    columnVecs1.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs1.emplace_back(tmp2->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs1[0]->set(i, dolphindb::Util::createDateTime(2012, 1, i + 1, 12, 30, 30));
        columnVecs1[1]->set(i, dolphindb::Util::createTimestamp(2012, 1, i + 1, 12, 30, 30, 123));
        columnVecs1[2]->set(i, dolphindb::Util::createNanoTimestamp(2012, 1, i + 1, 12, 30, 30, 123456789));
        columnVecs1[3]->set(i, dolphindb::Util::createDate(2012, 1, i + 1));
    }
    int res2 = appender.append(tmp2);
    ASSERT_EQ(res2, rowNum);
    std::string script3;
    script3 += "expected = table(2012.01.01+0..9 as date1, 2012.01.01+0..9 as date2, 2012.01.01+0..9 as date3, 2012.01.01+0..9 as date4);";
    script3 += "each(eqObj, (select * from st1 where date1>1970.01.01).values(), expected.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result2->getInt(i), 1);
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_convert_to_month){
    std::string script1;
    script1 += "st1=table(100:0, `month1`month2`month3`month4`month5, [MONTH, MONTH, MONTH, MONTH, MONTH])";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "st1", conn);
    std::vector<std::string> colNames = { "month1", "month2", "month3", "month4", "month5" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_DATETIME, dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_DATE, dolphindb::DT_MONTH };
    int colNum = 5, rowNum = 10;
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(tmp1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createDateTime(1969, i + 1, 1, 12, 30, 30));
        columnVecs[1]->set(i, dolphindb::Util::createTimestamp(1969, i + 1, 1, 12, 30, 30, 123));
        columnVecs[2]->set(i, dolphindb::Util::createNanoTimestamp(1969, i + 1, 1, 12, 30, 30, 123456789));
        columnVecs[3]->set(i, dolphindb::Util::createDate(1969, i + 1, 1));
        columnVecs[4]->set(i, dolphindb::Util::createMonth(1969, i + 1));
    }
    int res1 = appender.append(tmp1);
    ASSERT_EQ(res1, rowNum);
    std::string script2;
    script2 += "expected = table(1969.01M+0..9 as month1, 1969.01M+0..9 as month2, 1969.01M+0..9 as month3, 1969.01M+0..9 as month4, 1969.01M+0..9 as month5);";
    script2 += "each(eqObj, st1.values(), expected.values());";
    dolphindb::ConstantSP result1 = conn.run(script2);
    for (int i = 0; i<5; i++)
        ASSERT_EQ(result1->getInt(i), 1);

    dolphindb::TableSP tmp2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs2;
    columnVecs2.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs2.emplace_back(tmp2->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs2[0]->set(i, dolphindb::Util::createDateTime(2000, i + 1, 1, 12, 30, 30));
        columnVecs2[1]->set(i, dolphindb::Util::createTimestamp(2000, i + 1, 1, 12, 30, 30, 123));
        columnVecs2[2]->set(i, dolphindb::Util::createNanoTimestamp(2000, i + 1, 1, 12, 30, 30, 123456789));
        columnVecs2[3]->set(i, dolphindb::Util::createDate(2000, i + 1, 1));
        columnVecs2[4]->set(i, dolphindb::Util::createMonth(2000, i + 1));
    }
    int res2 = appender.append(tmp2);
    ASSERT_EQ(res2, rowNum);
    std::string script3;
    script3 += "expected = table(2000.01M+0..9 as month1, 2000.01M+0..9 as month2, 2000.01M+0..9 as month3, 2000.01M+0..9 as month4, 2000.01M+0..9 as month5);";
    script3 += "each(eqObj,(select * from st1 where month1>1970.01M).values(), expected.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<5; i++)
        ASSERT_EQ(result2->getInt(i), 1);
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_convert_to_datetime){
    std::string script1;
    script1 += "st1=table(100:0, `col1`col2`col3`col4, [DATETIME, DATETIME, DATETIME, DATETIME])";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "st1", conn);
    std::vector<std::string> colNames = { "col1", "col2", "col3", "col4" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_DATETIME, dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_DATE };
    int colNum = 4, rowNum = 10;
    dolphindb::TableSP tmp = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(tmp->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createDateTime(1969, 1, 1, 12, 30, i));
        columnVecs[1]->set(i, dolphindb::Util::createTimestamp(1969, 1, 1, 12, 30, i, 123));
        columnVecs[2]->set(i, dolphindb::Util::createNanoTimestamp(1969, 1, 1, 12, 30, i, 123456789));
        columnVecs[3]->set(i, dolphindb::Util::createDate(1969, 1, 1));
    }
    int res1 = appender.append(tmp);
    ASSERT_EQ(res1, rowNum);
    std::string script2;
    script2 += "expected = table(1969.01.01T12:30:00+0..9 as col1, 1969.01.01T12:30:00+0..9 as col2, 1969.01.01T12:30:00+0..9 as col3, take(1969.01.01T00:00:00, 10) as col4);";
    script2 += "each(eqObj, st1.values(), expected.values());";
    dolphindb::ConstantSP result1 = conn.run(script2);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result1->getInt(i), 1);

    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs1;
    columnVecs1.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs1.emplace_back(tmp1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs1[0]->set(i, dolphindb::Util::createDateTime(2001, 1, 1, 12, 30, i));
        columnVecs1[1]->set(i, dolphindb::Util::createTimestamp(2001, 1, 1, 12, 30, i, 123));
        columnVecs1[2]->set(i, dolphindb::Util::createNanoTimestamp(2001, 1, 1, 12, 30, i, 123456789));
        columnVecs1[3]->set(i, dolphindb::Util::createDate(2001, 1, 1));
    }
    int res2 = appender.append(tmp1);
    ASSERT_EQ(res2, rowNum);
    std::string script3;
    script3 += "expected = table(2001.01.01T12:30:00+0..9 as col1, 2001.01.01T12:30:00+0..9 as col2, 2001.01.01T12:30:00+0..9 as col3, take(2001.01.01T00:00:00, 10) as col4);";
    script3 += "each(eqObj, (select * from st1 where col1>1970.01.01T00:00:00).values(), expected.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result2->getInt(i), 1);
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_convert_to_timestamp){
    std::string script1;
    script1 += "st1=table(100:0, `col1`col2`col3`col4`col5, [TIMESTAMP, TIMESTAMP, TIMESTAMP, TIMESTAMP,TIMESTAMP])";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "st1", conn);
    std::vector<std::string> colNames = { "col1", "col2", "col3", "col4", "col5" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_DATETIME, dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_DATE };
    int colNum = 5, rowNum = 10;
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(tmp1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createDateTime(1969, 1, 1, 12, 30, i));
        columnVecs[1]->set(i, dolphindb::Util::createTimestamp(1969, 1, 1, 12, 30, i, 123));
        columnVecs[2]->set(i, dolphindb::Util::createNanoTimestamp(1969, 1, 1, 12, 30, i, 123456789));
        columnVecs[3]->set(i, dolphindb::Util::createNanoTimestamp(1960, 1, 1, 12, 30, 10, 8000000));
        columnVecs[4]->set(i, dolphindb::Util::createDate(1969, 1, 1));
    }
    int res = appender.append(tmp1);
    ASSERT_EQ(res, rowNum);
    std::string script2;
    script2 += "expected = table(temporalAdd(1969.01.01T12:30:00.000, 0..9, \"s\") as col1, temporalAdd(1969.01.01T12:30:00.123, 0..9, \"s\") as col2, temporalAdd(1969.01.01T12:30:00.123, 0..9, \"s\") as col3, take(1960.01.01T12:30:10.008, 10) as col4, take(1969.01.01T00:00:00.000, 10) as col5);";
    script2 += "each(eqObj, st1.values(), expected.values());";
    dolphindb::ConstantSP result1 = conn.run(script2);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result1->getInt(i), 1);

    dolphindb::TableSP tmp2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs1;
    columnVecs1.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs1.emplace_back(tmp2->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs1[0]->set(i, dolphindb::Util::createDateTime(2000, 1, 1, 12, 30, i));
        columnVecs1[1]->set(i, dolphindb::Util::createTimestamp(2000, 1, 1, 12, 30, i, 123));
        columnVecs1[2]->set(i, dolphindb::Util::createNanoTimestamp(2000, 1, 1, 12, 30, i, 123456789));
        columnVecs1[3]->set(i, dolphindb::Util::createNanoTimestamp(2000, 1, 1, 12, 30, 10, 8000000));
        columnVecs1[4]->set(i, dolphindb::Util::createDate(2000, 1, 1));
    }
    int res2 = appender.append(tmp2);
    ASSERT_EQ(res2, rowNum);
    std::string script3;
    script3 += "expected = table(temporalAdd(2000.01.01T12:30:00.000, 0..9, \"s\") as col1, temporalAdd(2000.01.01T12:30:00.123, 0..9, \"s\") as col2, temporalAdd(2000.01.01T12:30:00.123, 0..9, \"s\") as col3, take(2000.01.01T12:30:10.008, 10) as col4, take(2000.01.01T00:00:00.000, 10) as col5);";
    script3 += "each(eqObj, (select * from st1 where date(col1)>1970.01.01).values(), expected.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result2->getInt(i), 1);
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_convert_to_nanotimestamp){
    std::string script1;
    script1 += "st1=table(100:0, `col1`col2`col3`col4, [NANOTIMESTAMP, NANOTIMESTAMP, NANOTIMESTAMP, NANOTIMESTAMP])";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "st1", conn);
    std::vector<std::string> colNames = { "col1", "col2", "col3", "col4" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_DATETIME, dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_DATE };
    int colNum = 4, rowNum = 10;
    dolphindb::TableSP tmp = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(tmp->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createDateTime(1969, 1, 1, 12, 30, i));
        columnVecs[1]->set(i, dolphindb::Util::createTimestamp(1969, 1, 1, 12, 30, i, 123));
        columnVecs[2]->set(i, dolphindb::Util::createNanoTimestamp(1969, 1, 1, 12, 30, i, 123456789));
        columnVecs[3]->set(i, dolphindb::Util::createDate(1969, 1, 1));
    }
    int res1 = appender.append(tmp);
    ASSERT_EQ(res1, rowNum);
    std::string script2;
    script2 += "expected = table(temporalAdd(1969.01.01T12:30:00.000000000, 0..9, \"s\") as col1, temporalAdd(1969.01.01T12:30:00.123000000, 0..9, \"s\") as col2, temporalAdd(1969.01.01T12:30:00.123456789, 0..9, \"s\") as col3, take(1969.01.01T00:00:00.000000000, 10) as col4);";
    script2 += "each(eqObj, st1.values(), expected.values());";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result->getInt(i), 1);

    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs1;
    columnVecs1.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs1.emplace_back(tmp1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs1[0]->set(i, dolphindb::Util::createDateTime(2012, 1, 1, 12, 30, i));
        columnVecs1[1]->set(i, dolphindb::Util::createTimestamp(2012, 1, 1, 12, 30, i, 123));
        columnVecs1[2]->set(i, dolphindb::Util::createNanoTimestamp(2012, 1, 1, 12, 30, i, 123456789));
        columnVecs1[3]->set(i, dolphindb::Util::createDate(2012, 1, 1));
    }
    int res2 = appender.append(tmp1);
    ASSERT_EQ(res2, rowNum);
    std::string script3;
    script3 += "expected = table(temporalAdd(2012.01.01T12:30:00.000000000, 0..9, \"s\") as col1, temporalAdd(2012.01.01T12:30:00.123000000, 0..9, \"s\") as col2, temporalAdd(2012.01.01T12:30:00.123456789, 0..9, \"s\") as col3, take(2012.01.01T00:00:00.000000000, 10) as col4);";
    script3 += "each(eqObj, (select * from st1 where col1>=2012.01.01T12:30:00.000000000).values(), expected.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result2->getInt(i), 1);
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_convert_to_minute){
    std::string script1;
    script1 += "st1=table(100:0, `col1`col2`col3`col4`col5`col6`col7, [MINUTE, MINUTE, MINUTE, MINUTE, MINUTE, MINUTE, MINUTE])";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "st1", conn);
    std::vector<std::string> colNames = { "col1", "col2", "col3", "col4", "col5", "col6", "col7" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_DATETIME, dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_TIME, dolphindb::DT_SECOND, dolphindb::DT_MINUTE, dolphindb::DT_NANOTIME };
    int colNum = 7, rowNum = 10;
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(tmp1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createDateTime(1969, 1, 1, 12, i, 30));
        columnVecs[1]->set(i, dolphindb::Util::createTimestamp(1969, 1, 1, 12, i, 30, 123));
        columnVecs[2]->set(i, dolphindb::Util::createNanoTimestamp(1969, 1, 1, 12, i, 30, 123456789));
        columnVecs[3]->set(i, dolphindb::Util::createTime(12, i, 30, 123));
        columnVecs[4]->set(i, dolphindb::Util::createSecond(12, i, 30));
        columnVecs[5]->set(i, dolphindb::Util::createMinute(12, i));
        columnVecs[6]->set(i, dolphindb::Util::createNanoTime(12, i, 30, 123456789));
    }
    int res1 = appender.append(tmp1);
    ASSERT_EQ(res1, rowNum);
    std::string script2;
    script2 += "expected = table(12:00m+0..9 as col1, 12:00m+0..9 as col2, 12:00m+0..9 as col3, 12:00m+0..9 as col4, 12:00m+0..9 as col5, 12:00m+0..9 as col6, 12:00m+0..9 as col7);";
    script2 += "each(eqObj, st1.values(), expected.values());";
    dolphindb::ConstantSP result1 = conn.run(script2);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result1->getInt(i), 1);

    dolphindb::TableSP tmp2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs1;
    columnVecs1.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs1.emplace_back(tmp2->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs1[0]->set(i, dolphindb::Util::createDateTime(2000, 1, 1, 12, i + 10, 30));
        columnVecs1[1]->set(i, dolphindb::Util::createTimestamp(2000, 1, 1, 12, i + 10, 30, 123));
        columnVecs1[2]->set(i, dolphindb::Util::createNanoTimestamp(2000, 1, 1, 12, i + 10, 30, 123456789));
        columnVecs1[3]->set(i, dolphindb::Util::createTime(12, i + 10, 30, 123));
        columnVecs1[4]->set(i, dolphindb::Util::createSecond(12, i + 10, 30));
        columnVecs1[5]->set(i, dolphindb::Util::createMinute(12, i + 10));
        columnVecs1[6]->set(i, dolphindb::Util::createNanoTime(12, i + 10, 30, 123456789));
    }
    int res2 = appender.append(tmp2);
    ASSERT_EQ(res2, rowNum);
    std::string script3;
    script3 += "expected = table(12:10m+0..9 as col1, 12:10m+0..9 as col2, 12:10m+0..9 as col3, 12:10m+0..9 as col4, 12:10m+0..9 as col5, 12:10m+0..9 as col6, 12:10m+0..9 as col7);";
    script3 += "each(eqObj, (select * from st1 where col1>=12:10m).values(), expected.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result2->getInt(i), 1);
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_convert_to_time){
    std::string script1;
    script1 += "st1=table(100:0, `col1`col2`col3`col4`col5`col6`col7, [TIME, TIME, TIME, TIME, TIME, TIME, TIME])";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "st1", conn);
    std::vector<std::string> colNames = { "col1", "col2", "col3", "col4", "col5", "col6", "col7" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_DATETIME, dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_TIME, dolphindb::DT_SECOND, dolphindb::DT_MINUTE, dolphindb::DT_NANOTIME };
    int colNum = 7, rowNum = 10;
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(tmp1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createDateTime(1969, 1, 1, 12, i, 30));
        columnVecs[1]->set(i, dolphindb::Util::createTimestamp(1969, 1, 1, 12, i, 30, 123));
        columnVecs[2]->set(i, dolphindb::Util::createNanoTimestamp(1969, 1, 1, 12, i, 30, 123456789));
        columnVecs[3]->set(i, dolphindb::Util::createTime(12, i, 30, 123));
        columnVecs[4]->set(i, dolphindb::Util::createSecond(12, i, 30));
        columnVecs[5]->set(i, dolphindb::Util::createMinute(12, i));
        columnVecs[6]->set(i, dolphindb::Util::createNanoTime(12, i, 30, 123456789));
    }
    int res1 = appender.append(tmp1);
    ASSERT_EQ(res1, rowNum);
    std::string script2;
    script2 += "expected = table(temporalAdd(12:00:30.000, 0..9, \"m\") as col1, temporalAdd(12:00:30.123, 0..9, \"m\") as col2, temporalAdd(12:00:30.123, 0..9, \"m\") as col3, temporalAdd(12:00:30.123, 0..9, \"m\") as col4, temporalAdd(12:00:30.000, 0..9, \"m\") as col5, temporalAdd(12:00:00.000, 0..9, \"m\") as col6, temporalAdd(12:00:30.123, 0..9, \"m\") as col7);";
    script2 += "each(eqObj, st1.values(), expected.values());";
    dolphindb::ConstantSP result1 = conn.run(script2);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result1->getInt(i), 1);

    dolphindb::TableSP tmp2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs1;
    columnVecs1.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs1.emplace_back(tmp2->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs1[0]->set(i, dolphindb::Util::createDateTime(2012, 1, 1, 12, i + 10, 30));
        columnVecs1[1]->set(i, dolphindb::Util::createTimestamp(2012, 1, 1, 12, i + 10, 30, 123));
        columnVecs1[2]->set(i, dolphindb::Util::createNanoTimestamp(2012, 1, 1, 12, i + 10, 30, 123456789));
        columnVecs1[3]->set(i, dolphindb::Util::createTime(12, i + 10, 30, 123));
        columnVecs1[4]->set(i, dolphindb::Util::createSecond(12, i + 10, 30));
        columnVecs1[5]->set(i, dolphindb::Util::createMinute(12, i + 10));
        columnVecs1[6]->set(i, dolphindb::Util::createNanoTime(12, i + 10, 30, 123456789));
    }
    int res2 = appender.append(tmp2);
    ASSERT_EQ(res2, rowNum);
    std::string script3;
    script3 += "expected = table(temporalAdd(12:10:30.000, 0..9, \"m\") as col1, temporalAdd(12:10:30.123, 0..9, \"m\") as col2, temporalAdd(12:10:30.123, 0..9, \"m\") as col3, temporalAdd(12:10:30.123, 0..9, \"m\") as col4, temporalAdd(12:10:30.000, 0..9, \"m\") as col5, temporalAdd(12:10:00.000, 0..9, \"m\") as col6, temporalAdd(12:10:30.123, 0..9, \"m\") as col7);";
    script3 += "each(eqObj, (select * from st1 where col1>=12:10:30.000).values(), expected.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result2->getInt(i), 1);
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_convert_to_second){
    std::string script1;
    script1 += "st1=table(100:0, `col1`col2`col3`col4`col5`col6`col7, [SECOND, SECOND, SECOND, SECOND, SECOND, SECOND, SECOND])";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "st1", conn);
    std::vector<std::string> colNames = { "col1", "col2", "col3", "col4", "col5", "col6", "col7" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_DATETIME, dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_TIME, dolphindb::DT_SECOND, dolphindb::DT_MINUTE, dolphindb::DT_NANOTIME };
    int colNum = 7, rowNum = 10;
    dolphindb::TableSP tmp = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(tmp->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createDateTime(1969, 1, 1, 12, i, 30));
        columnVecs[1]->set(i, dolphindb::Util::createTimestamp(1969, 1, 1, 12, i, 30, 123));
        columnVecs[2]->set(i, dolphindb::Util::createNanoTimestamp(1969, 1, 1, 12, i, 30, 123456789));
        columnVecs[3]->set(i, dolphindb::Util::createTime(12, i, 30, 123));
        columnVecs[4]->set(i, dolphindb::Util::createSecond(12, i, 30));
        columnVecs[5]->set(i, dolphindb::Util::createMinute(12, i));
        columnVecs[6]->set(i, dolphindb::Util::createNanoTime(12, i, 30, 123456789));
    }
    int res = appender.append(tmp);
    ASSERT_EQ(res, rowNum);
    std::string script2;
    script2 += "expected = table(temporalAdd(12:00:30, 0..9, \"m\") as col1, temporalAdd(12:00:30, 0..9, \"m\") as col2, temporalAdd(12:00:30, 0..9, \"m\") as col3, temporalAdd(12:00:30, 0..9, \"m\") as col4, temporalAdd(12:00:30, 0..9, \"m\") as col5, temporalAdd(12:00:00, 0..9, \"m\") as col6, temporalAdd(12:00:30, 0..9, \"m\") as col7);";
    script2 += "each(eqObj, st1.values(), expected.values());";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result->getInt(i), 1);

    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs1;
    columnVecs1.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs1.emplace_back(tmp1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs1[0]->set(i, dolphindb::Util::createDateTime(2012, 1, 1, 12, i + 10, 30));
        columnVecs1[1]->set(i, dolphindb::Util::createTimestamp(2012, 1, 1, 12, i + 10, 30, 123));
        columnVecs1[2]->set(i, dolphindb::Util::createNanoTimestamp(2012, 1, 1, 12, i + 10, 30, 123456789));
        columnVecs1[3]->set(i, dolphindb::Util::createTime(12, i + 10, 30, 123));
        columnVecs1[4]->set(i, dolphindb::Util::createSecond(12, i + 10, 30));
        columnVecs1[5]->set(i, dolphindb::Util::createMinute(12, i + 10));
        columnVecs1[6]->set(i, dolphindb::Util::createNanoTime(12, i + 10, 30, 123456789));
    }
    int res2 = appender.append(tmp1);
    ASSERT_EQ(res2, rowNum);
    std::string script3;
    script3 += "expected = table(temporalAdd(12:10:30, 0..9, \"m\") as col1, temporalAdd(12:10:30, 0..9, \"m\") as col2, temporalAdd(12:10:30, 0..9, \"m\") as col3, temporalAdd(12:10:30, 0..9, \"m\") as col4, temporalAdd(12:10:30, 0..9, \"m\") as col5, temporalAdd(12:10:00, 0..9, \"m\") as col6, temporalAdd(12:10:30, 0..9, \"m\") as col7);";
    script3 += "each(eqObj, (select * from st1 where col1>=12:10:30).values(), expected.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result2->getInt(i), 1);
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_convert_to_nanotime){
    std::string script1;
    script1 += "st1=table(100:0, `col1`col2`col3`col4`col5`col6`col7, [NANOTIME, NANOTIME, NANOTIME, NANOTIME, NANOTIME, NANOTIME, NANOTIME])";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "st1", conn);
    std::vector<std::string> colNames = { "col1", "col2", "col3", "col4", "col5", "col6", "col7" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_DATETIME, dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_TIME, dolphindb::DT_SECOND, dolphindb::DT_MINUTE, dolphindb::DT_NANOTIME };
    int colNum = 7, rowNum = 10;
    dolphindb::TableSP tmp = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(tmp->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createDateTime(1969, 1, 1, 12, i, 30));
        columnVecs[1]->set(i, dolphindb::Util::createTimestamp(1969, 1, 1, 12, i, 30, 123));
        columnVecs[2]->set(i, dolphindb::Util::createNanoTimestamp(1969, 1, 1, 12, i, 30, 123456789));
        columnVecs[3]->set(i, dolphindb::Util::createTime(12, i, 30, 123));
        columnVecs[4]->set(i, dolphindb::Util::createSecond(12, i, 30));
        columnVecs[5]->set(i, dolphindb::Util::createMinute(12, i));
        columnVecs[6]->set(i, dolphindb::Util::createNanoTime(12, i, 30, 123456789));
    }
    int res = appender.append(tmp);
    ASSERT_EQ(res, rowNum);
    std::string script2;
    script2 += "expected = table(temporalAdd(12:00:30.000000000, 0..9, \"m\") as col1, temporalAdd(12:00:30.123000000, 0..9, \"m\") as col2, temporalAdd(12:00:30.123456789, 0..9, \"m\") as col3, temporalAdd(12:00:30.123000000, 0..9, \"m\") as col4, temporalAdd(12:00:30.000000000, 0..9, \"m\") as col5, temporalAdd(12:00:00.000000000, 0..9, \"m\") as col6, temporalAdd(12:00:30.123456789, 0..9, \"m\") as col7);";
    script2 += "each(eqObj, st1.values(), expected.values());";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result->getInt(i), 1);

    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs1;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs1.emplace_back(tmp1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs1[0]->set(i, dolphindb::Util::createDateTime(2012, 1, 1, 12, i + 10, 30));
        columnVecs1[1]->set(i, dolphindb::Util::createTimestamp(2012, 1, 1, 12, i + 10, 30, 123));
        columnVecs1[2]->set(i, dolphindb::Util::createNanoTimestamp(2012, 1, 1, 12, i + 10, 30, 123456789));
        columnVecs1[3]->set(i, dolphindb::Util::createTime(12, i + 10, 30, 123));
        columnVecs1[4]->set(i, dolphindb::Util::createSecond(12, i + 10, 30));
        columnVecs1[5]->set(i, dolphindb::Util::createMinute(12, i + 10));
        columnVecs1[6]->set(i, dolphindb::Util::createNanoTime(12, i + 10, 30, 123456789));
    }
    int res2 = appender.append(tmp1);
    ASSERT_EQ(res2, rowNum);
    std::string script3;
    script3 += "expected = table(temporalAdd(12:10:30.000000000, 0..9, \"m\") as col1, temporalAdd(12:10:30.123000000, 0..9, \"m\") as col2, temporalAdd(12:10:30.123456789, 0..9, \"m\") as col3, temporalAdd(12:10:30.123000000, 0..9, \"m\") as col4, temporalAdd(12:10:30.000000000, 0..9, \"m\") as col5, temporalAdd(12:10:00.000000000, 0..9, \"m\") as col6, temporalAdd(12:10:30.123456789, 0..9, \"m\") as col7);";
    script3 += "each(eqObj, (select * from st1 where col1>=12:10:30.000000000).values(), expected.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result2->getInt(i), 1);
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_dfs_table){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script1;
    script1 += "dbPath='"+dbName+"';";
    script1 += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script1 += "db=database(dbPath, VALUE, 2012.01.01..2012.01.10);";
    script1 += "t=table(1:0, [`date], [DATE]);";
    script1 += "if(existsTable(dbPath,`pt)){dropTable(db,`pt)};";
    script1 += "pt=db.createPartitionedTable(t, `pt, `date);";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender(dbName, "pt", conn);
    std::vector<std::string> colNames = { "date" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_DATETIME };
    int colNum = 1, rowNum = 10;
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(tmp1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createDateTime(2012, 1, i + 1, 12, 30, 30));
    }
    int res = appender.append(tmp1);
    ASSERT_EQ(res, rowNum);
    std::string script;
    script += "expected = table(temporalAdd(2012.01.01, 0..9, 'd') as date);";
    script += "each(eqObj, (select * from pt order by date).values(), (select * from expected order by date).values());";
    dolphindb::ConstantSP result = conn.run(script);
    ASSERT_EQ(result->getInt(0), 1);
}

TEST_F(AutoFitTableappenderTest,test_AutoFitTableAppender_convert_to_datehour){
    std::string script1;
    script1 += "st1=table(100:0, `col1`col2`col3`col4`col5, [DATEHOUR, DATEHOUR , DATEHOUR , DATEHOUR, DATEHOUR])";
    conn.run(script1);
    dolphindb::AutoFitTableAppender appender("", "st1", conn);
    std::vector<std::string> colNames = { "col1", "col2", "col3", "col4","col5" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_DATE,dolphindb::DT_DATEHOUR, dolphindb::DT_DATETIME,dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIMESTAMP, };
    int colNum = 5, rowNum = 10;
    dolphindb::TableSP tmp = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(tmp->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createDate(1969, 1, 1));
        columnVecs[1]->set(i, dolphindb::Util::createDateHour(1969, 1, 1, i + 1));
        columnVecs[2]->set(i, dolphindb::Util::createDateTime(1969, 1, 1, i + 1, 30, 30));
        columnVecs[3]->set(i, dolphindb::Util::createTimestamp(1969, 1, 1, i + 1, 30, 30, 123));
        columnVecs[4]->set(i, dolphindb::Util::createNanoTimestamp(1969, 1, 1, i + 1, 30, 30, 123456789));

    }
    int res1 = appender.append(tmp);
    ASSERT_EQ(res1, rowNum);
    std::string script2;
    script2 += "expected =table(take(datehour(1969.01.01T00:00:00),10) as col1, datehour(1969.01.01T01:00:00)+0..9 as col2, datehour(1969.01.01T01:00:00)+0..9 as col3, datehour(1969.01.01T01:00:00)+0..9 as col4, datehour(1969.01.01T01:00:00)+0..9 as col5  );";
    script2 += "each(eqObj, st1.values(), expected.values());";
    dolphindb::ConstantSP result1 = conn.run(script2);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result1->getInt(i), 1);

    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs1;
    columnVecs1.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs1.emplace_back(tmp1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs1[0]->set(i, dolphindb::Util::createDate(2001, 1, 1));
        columnVecs1[1]->set(i, dolphindb::Util::createDateHour(2001, 1, 1, i + 1));
        columnVecs1[2]->set(i, dolphindb::Util::createDateTime(2001, 1, 1, i + 1, 30, 30));
        columnVecs1[3]->set(i, dolphindb::Util::createTimestamp(2001, 1, 1, i + 1, 30, 30, 123));
        columnVecs1[4]->set(i, dolphindb::Util::createNanoTimestamp(2001, 1, 1, i + 1, 30, 30, 123456789));
    }
    int res2 = appender.append(tmp1);
    ASSERT_EQ(res2, rowNum);
    std::string script3;
    script3 += "expected =table(take(datehour(2001.01.01T00:00:00),10) as col1, datehour(2001.01.01T01:00:00)+0..9 as col2, datehour(2001.01.01T01:00:00)+0..9 as col3, datehour(2001.01.01T01:00:00)+0..9 as col4, datehour(2001.01.01T01:00:00)+0..9 as col5  );";
    script3 += "each(eqObj, (select * from st1 where col1>datehour(1970.01.01T00:00:00)).values(), expected.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<colNum; i++)
        ASSERT_EQ(result2->getInt(i), 1);
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withIntArrayVectorToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "dropDatabase(dbName)\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, INT[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_INT_ARRAY, 0, 3);
    av1->append(v1);
    av1->append(v1);
    av1->append(v1);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_INT_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withIntArrayVectorNullToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, INT[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setNull(0);
    v2->setNull(1);
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_INT_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_INT_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withIntArrayVectorToPartitionTableRangeTypeMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,0 70000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, INT[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_INT, 70000, 70000);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    for(unsigned int i=0;i<70000;i++)
        v2->setInt(i, i);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_INT_ARRAY, 0, 70000);
    for(unsigned int i=0;i<3;i++)
        av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_INT_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withCharArrayVectorToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, CHAR[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_CHAR, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setChar(0, 1);
    v2->setChar(1, 0);
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_CHAR_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_CHAR_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withCharArrayVectorNullToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, CHAR[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_CHAR, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setNull(0);
    v2->setNull(1);
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_CHAR_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_CHAR_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withCharArrayVectorToPartitionTableRangeTypeMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,0 70000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, CHAR[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_CHAR, 70000, 70000);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    for(unsigned int i=0;i<70000;i++)
        v2->setChar(i, i);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_CHAR_ARRAY, 0, 70000);
    for(unsigned int i=0;i<3;i++)
        av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_CHAR_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withFloatArrayVectorToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, FLOAT[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_FLOAT, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setFloat(0, 1);
    v2->setFloat(1, 0);
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_FLOAT_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_FLOAT_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withFloatArrayVectorNullToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, FLOAT[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_FLOAT, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setNull(0);
    v2->setNull(1);
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_FLOAT_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_FLOAT_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withFloatArrayVectorToPartitionTableRangeTypeMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,0 70000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, FLOAT[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_FLOAT, 70000, 70000);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    for(unsigned int i=0;i<70000;i++)
        v2->setFloat(i, i);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_FLOAT_ARRAY, 0, 70000);
    for(unsigned int i=0;i<3;i++)
        av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_FLOAT_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withDateArrayVectorToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, DATE[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DATE, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->set(0, dolphindb::Util::createDate(0));
    v2->set(1, dolphindb::Util::createDate(1000));
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_DATE_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withDateArrayVectorNullToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, DATE[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DATE, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setNull(0);
    v2->setNull(1);
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_DATE_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withDateArrayVectorToPartitionTableRangeTypeMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,0 70000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, DATE[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DATE, 70000, 70000);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    for(unsigned int i=0;i<70000;i++)
        v2->set(i, dolphindb::Util::createDate(i));
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 70000);
    for(unsigned int i=0;i<3;i++)
        av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_DATE_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withMonthArrayVectorToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, MONTH[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_MONTH, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->set(0, dolphindb::Util::createMonth(0));
    v2->set(1, dolphindb::Util::createMonth(1000));
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_MONTH_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_MONTH_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withMonthArrayVectorNullToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, MONTH[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_MONTH, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setNull(0);
    v2->setNull(1);
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_MONTH_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_MONTH_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withMonthArrayVectorToPartitionTableRangeTypeMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,0 70000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, MONTH[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_MONTH, 70000, 70000);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    for(unsigned int i=0;i<70000;i++)
        v2->set(i, dolphindb::Util::createMonth(i));
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_MONTH_ARRAY, 0, 70000);
    for(unsigned int i=0;i<3;i++)
        av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_MONTH_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withTimeArrayVectorToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, TIME[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_TIME, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->set(0, dolphindb::Util::createTime(0));
    v2->set(1, dolphindb::Util::createTime(1000));
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_TIME_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_TIME_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withTimeArrayVectorNullToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, TIME[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_TIME, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setNull(0);
    v2->setNull(1);
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_TIME_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_TIME_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withTimeArrayVectorToPartitionTableRangeTypeMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,0 70000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, TIME[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_TIME, 70000, 70000);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    for(unsigned int i=0;i<70000;i++)
        v2->set(i, dolphindb::Util::createTime(i));
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_TIME_ARRAY, 0, 70000);
    for(unsigned int i=0;i<3;i++)
        av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_TIME_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withSecondArrayVectorToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, SECOND[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_SECOND, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->set(0, dolphindb::Util::createSecond(0));
    v2->set(1, dolphindb::Util::createSecond(1000));
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SECOND_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withSecondArrayVectorNullToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, SECOND[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_SECOND, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setNull(0);
    v2->setNull(1);
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SECOND_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withSecondArrayVectorToPartitionTableRangeTypeMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,0 70000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, SECOND[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_SECOND, 70000, 70000);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    for(unsigned int i=0;i<70000;i++)
        v2->set(i, dolphindb::Util::createSecond(i));
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 70000);
    for(unsigned int i=0;i<3;i++)
        av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SECOND_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withDatehourArrayVectorToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, DATEHOUR[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->set(0, dolphindb::Util::createDateHour(0));
    v2->set(1, dolphindb::Util::createDateHour(1000));
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_DATEHOUR_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withDatehourArrayVectorNullToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, DATEHOUR[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setNull(0);
    v2->setNull(1);
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_DATEHOUR_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withDatehourArrayVectorToPartitionTableRangeTypeMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,0 70000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, DATEHOUR[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 70000, 70000);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    for(unsigned int i=0;i<70000;i++)
        v2->set(i, dolphindb::Util::createDateHour(i));

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 70000);
    for(unsigned int i=0;i<3;i++)
        av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_DATEHOUR_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withUuidArrayVectorToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, UUID[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_UUID, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setString(0, "5d212a78-cc48-e3b1-4235-b4d91473ee87");
    v2->setString(1, "5d212a78-cc48-e3b1-4235-b4d91473ee99");
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_UUID_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_UUID_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withUuidArrayVectorNullToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,1 10000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, UUID[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_UUID, 3, 3);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    v2->setNull(0);
    v2->setNull(1);
    v2->setNull(2);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_UUID_ARRAY, 0, 3);
    av1->append(v2);
    av1->append(v2);
    av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_UUID_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_withUuidArrayVectorToPartitionTableRangeTypeMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\"\n"
            "if(exists(dbName)){\n"
            "\tdropDatabase(dbName)\t\n"
            "}\n"
            "db  = database(dbName, RANGE,0 70000,,'TSDB')\n"
            "t = table(1000:0, `id`value,[ INT, UUID[]])\n"
            "pt = db.createPartitionedTable(t,`pt,`id,,`id)";
    conn.run(script);
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_UUID, 70000, 70000);
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    for(unsigned int i=0;i<70000;i++)
        v2->setString(i, "5d212a78-cc48-e3b1-4235-b4d91473ee87");
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_UUID_ARRAY, 0, 70000);
    for(unsigned int i=0;i<3;i++)
        av1->append(v2);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_UUID_ARRAY };
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    dolphindb::AutoFitTableAppender appender(dbName, tableName, conn);
    appender.append(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

class AFTA_append_null : public AutoFitTableappenderTest, public testing::WithParamInterface<std::tuple<std::string, dolphindb::DATA_TYPE>>
{
public:
    static std::vector<std::tuple<std::string, dolphindb::DATA_TYPE>> data_prepare()
    {
        std::vector<std::string> testTypes = {"BOOL", "CHAR", "SHORT", "INT", "LONG", "DATE", "MONTH", "TIME", "MINUTE", "SECOND", "DATETIME", "TIMESTAMP", "NANOTIME", "NANOTIMESTAMP", "DATEHOUR", "FLOAT", "DOUBLE", "STRING", "SYMBOL", "BLOB", "IPADDR", "UUID", "INT128", "DECIMAL32(8)", "DECIMAL64(15)", "DECIMAL128(28)",
                "BOOL[]", "CHAR[]", "SHORT[]", "INT[]", "LONG[]", "DATE[]", "MONTH[]", "TIME[]", "MINUTE[]", "SECOND[]", "DATETIME[]", "TIMESTAMP[]", "NANOTIME[]", "NANOTIMESTAMP[]", "DATEHOUR[]", "FLOAT[]", "DOUBLE[]", "IPADDR[]", "UUID[]", "INT128[]", "DECIMAL32(8)[]", "DECIMAL64(15)[]", "DECIMAL128(25)[]"};
        std::vector<dolphindb::DATA_TYPE> dataTypes = {dolphindb::DT_BOOL, dolphindb::DT_CHAR, dolphindb::DT_SHORT, dolphindb::DT_INT, dolphindb::DT_LONG, dolphindb::DT_DATE, dolphindb::DT_MONTH, dolphindb::DT_TIME, dolphindb::DT_MINUTE, dolphindb::DT_SECOND, dolphindb::DT_DATETIME, dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIME, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_DATEHOUR, dolphindb::DT_FLOAT, dolphindb::DT_DOUBLE, dolphindb::DT_STRING,dolphindb::DT_SYMBOL,dolphindb::DT_BLOB, dolphindb::DT_IP, dolphindb::DT_UUID, dolphindb::DT_INT128, dolphindb::DT_DECIMAL32, dolphindb::DT_DECIMAL64, dolphindb::DT_DECIMAL128,
        dolphindb::DT_BOOL_ARRAY, dolphindb::DT_CHAR_ARRAY, dolphindb::DT_SHORT_ARRAY, dolphindb::DT_INT_ARRAY, dolphindb::DT_LONG_ARRAY, dolphindb::DT_DATE_ARRAY, dolphindb::DT_MONTH_ARRAY, dolphindb::DT_TIME_ARRAY, dolphindb::DT_MINUTE_ARRAY, dolphindb::DT_SECOND_ARRAY, dolphindb::DT_DATETIME_ARRAY, dolphindb::DT_TIMESTAMP_ARRAY, dolphindb::DT_NANOTIME_ARRAY, dolphindb::DT_NANOTIMESTAMP_ARRAY, dolphindb::DT_DATEHOUR_ARRAY, dolphindb::DT_FLOAT_ARRAY, dolphindb::DT_DOUBLE_ARRAY, dolphindb::DT_IP_ARRAY, dolphindb::DT_UUID_ARRAY, dolphindb::DT_INT128_ARRAY, dolphindb::DT_DECIMAL32_ARRAY, dolphindb::DT_DECIMAL64_ARRAY, dolphindb::DT_DECIMAL128_ARRAY};
        std::vector<std::tuple<std::string, dolphindb::DATA_TYPE>> data;
        for    (auto i = 0; i < testTypes.size(); i++)
            data.push_back(make_tuple(testTypes[i], dataTypes[i]));
        return data;
    }

};
INSTANTIATE_TEST_SUITE_P(, AFTA_append_null, testing::ValuesIn(AFTA_append_null::data_prepare()));
TEST_P(AFTA_append_null, test_append_empty_table)
{
    std::string type = std::get<0>(GetParam());
    dolphindb::DATA_TYPE dataType = std::get<1>(GetParam());
    std::cout << "test type: " << type << std::endl;
    std::string colName = "c1";
    std::string script1 =
        "colName = [`" + colName + "];"
        "colType = [" + type + "];"
        "att=table(1:0, colName, colType)";
    conn.run(script1);
    dolphindb::VectorSP col0 = dolphindb::Util::createVector(dataType, 0);
    std::vector<std::string> colNames = { "c1" };
    std::vector<dolphindb::ConstantSP> cols = { col0};
    dolphindb::TableSP empty2 = dolphindb::Util::createTable(colNames, cols);
    dolphindb::AutoFitTableAppender appender("", "att", conn);
    int rows = appender.append(empty2);
    ASSERT_EQ(rows, 0);
    auto res = conn.run("exec * from att");
    ASSERT_EQ(res->rows(), 0);
}

TEST_F(AutoFitTableappenderTest, test_AutoFitTableAppender_with_SCRAM_session)
{
    std::string userName=getRandString(20);
    conn.run(
        "userName='"+userName+"';"
        "try{deleteUser(userName)}catch(ex){};go;createUser(userName, `123456, authMode='scram')"
    );
    dolphindb::DBConnection conn_scram(false, false, 7200, false, false, false, true);
    conn_scram.connect(HOST, PORT, userName, "123456");
    dolphindb::TableSP data = conn_scram.run("t = table(1 2 3 as c1, rand(100.00, 3) as c2);t2 = select top 0* from t; t");
    dolphindb::AutoFitTableAppender appender("", "t2", conn_scram);
    appender.append(data);
    auto res = conn_scram.run("all(each(eqObj, values t2, values t))")->getBool();
    ASSERT_EQ(res, true);
    conn_scram.close();
}

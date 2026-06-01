#include <gtest/gtest.h>
#include "config.h"

class AutoFitTableUpsertTest:public testing::Test
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
        //Case
        virtual void SetUp()
        {

        }
        virtual void TearDown()
        {

        }
};

dolphindb::DBConnection AutoFitTableUpsertTest::conn(false, false);

TEST_F(AutoFitTableUpsertTest,test_AutoFitTableUpsert_unconnected){
    dolphindb::DBConnection connNew(false, false);
    ASSERT_ANY_THROW(dolphindb::AutoFitTableUpsert aftu("", "temp_tab", connNew));
}

TEST_F(AutoFitTableUpsertTest,test_AutoFitTableUpsert_upsertToinMemoryTable){
    conn.run("temp_tab=table(`a`x`d`s`cs as col1, 2 3 4 5 6 as col2)");
    dolphindb::AutoFitTableUpsert aftu("", "temp_tab", conn);
    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::ConstantSP> cols ={dolphindb::Util::createString("zzz123中文a"), dolphindb::Util::createInt(1000)};
    dolphindb::TableSP t = dolphindb::Util::createTable(colNames, cols);
    ASSERT_ANY_THROW(aftu.upsert(t));
}

TEST_F(AutoFitTableUpsertTest,test_AutoFitTableUpsert_upsertErrTableColNums){
    conn.run("temp_tab = keyedTable(`col2, table(`a`x`d`s`cs as col1, 2 3 4 5 6 as col2))");
    std::vector<std::string> colNames = {"col1", "col2", "col3"};
    std::vector<dolphindb::ConstantSP> cols ={dolphindb::Util::createString("zzz123中文a"), dolphindb::Util::createInt(7), dolphindb::Util::createInt(5)};
    dolphindb::TableSP t = dolphindb::Util::createTable(colNames, cols);
    dolphindb::AutoFitTableUpsert aftu("", "temp_tab", conn);
    ASSERT_ANY_THROW(aftu.upsert(t));
}

TEST_F(AutoFitTableUpsertTest,test_AutoFitTableUpsert_upsertErrTableColType){
    conn.run("temp_tab = keyedTable(`col2, table(`a`x`d`s`cs as col1, 2 3 4 5 6 as col2))");
    dolphindb::AutoFitTableUpsert aftu("", "temp_tab", conn);
    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::ConstantSP> cols ={dolphindb::Util::createString("zzz123中文a"), dolphindb::Util::createDate(1000)};
    dolphindb::TableSP t = dolphindb::Util::createTable(colNames, cols);
    ASSERT_ANY_THROW(aftu.upsert(t));
}

TEST_F(AutoFitTableUpsertTest,test_AutoFitTableUpsert_upsertWithNullKeyColNames){
    conn.run("temp_tab = keyedTable(`col2, table(`a`x`d`s`cs as col1, 2 3 4 5 6 as col2))");
    std::vector<std::string> KeyCols = {};
    dolphindb::AutoFitTableUpsert aftu("", "temp_tab", conn, false, &KeyCols);
    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::ConstantSP> cols ={dolphindb::Util::createString("zzz123中文a"), dolphindb::Util::createInt(1000)};
    dolphindb::TableSP t = dolphindb::Util::createTable(colNames, cols);
    aftu.upsert(t);
    ASSERT_TRUE(conn.run("exec count(*) from temp_tab")->getInt() == 6);
}

TEST_F(AutoFitTableUpsertTest,test_AutoFitTableUpsert_upsertAllDataTypesToindexedTable){
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
    script1 += "temp = table(100:0, take(`col,25)+string(take(0..24,25)), \
    [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, BLOB, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+"), DECIMAL128("+std::to_string(scale128)+")]);";
    script1 += "st1 = indexedTable(`col16,temp);";
    conn.run(script1);
    std::vector<std::string> keycolName = {"col16"};
    dolphindb::AutoFitTableUpsert aftu("", "st1", conn, false);
    aftu.upsert(tab1);
    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "each(eqObj, tab1.values(), st1.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
}

TEST_F(AutoFitTableUpsertTest,test_AutoFitTableUpsert_upsertAllDataTypesTokeyedTable){
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
    script1 += "temp = table(100:0, take(`col,25)+string(take(0..24,25)), \
    [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, BLOB, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+"), DECIMAL128("+std::to_string(scale128)+")]);";
    script1 += "st1 = keyedTable(`col16,temp);";
    conn.run(script1);
    std::vector<std::string> keycolName = {"col16"};
    dolphindb::AutoFitTableUpsert aftu("", "st1", conn, false);
    aftu.upsert(tab1);
    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "each(eqObj, tab1.values(), st1.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
}

TEST_F(AutoFitTableUpsertTest,test_AutoFitTableUpsert_upsertAllDataTypesTopartitionedTableOLAP){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
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
            columnVecs[3]->set(rowNum-1, dolphindb::Util::createInt(rand()%INT_MAX));
        else
            columnVecs[j]->setNull(rowNum-1);
    }
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\";"
            "if(exists(dbName)){dropDatabase(dbName)};"
            "db  = database(dbName, HASH,[INT,1]);"
            "temp = table(1000:0, take(`col,24)+string(take(0..23,24)), \
            [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+"), DECIMAL128("+std::to_string(scale128)+")]);"
            "pt = createPartitionedTable(db,temp,`pt,`col3);";
    conn.run(script);
    std::vector<std::string> keycolName = {"col16"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab1);
    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "st1 = select * from pt;";
    script3 += "each(eqObj, tab1.values(), st1.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
}

TEST_F(AutoFitTableUpsertTest,test_AutoFitTableUpsert_upsertAllDataTypesTopartitionedTableTSDB){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
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
            columnVecs[3]->set(rowNum-1, dolphindb::Util::createInt(rand()%INT_MAX));  //partition-column's value must be not null
        else
            columnVecs[j]->setNull(rowNum-1);
    }
    std::string tableName = "pt";
    std::string script = "dbName = \""+dbName+"\";"
            "if(exists(dbName)){dropDatabase(dbName)};"
            "db  = database(dbName, HASH,[STRING,1],,'TSDB');"
            "temp = table(100:0, take(`col,25)+string(take(0..24,25)), \
            [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, BLOB, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+"), DECIMAL128("+std::to_string(scale128)+")]);"
            "pt = db.createPartitionedTable(temp,`pt,`col16,,`col16);";
    conn.run(script);
    std::vector<std::string> keycolName = {"col16"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab1);
    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "st1 = select * from pt order by col3;res = select * from tab1 order by col3;";
    script3 += "each(eqObj, res.values(), st1.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
}

TEST_F(AutoFitTableUpsertTest, test_upsertToPartitionTableRangeType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    std::string script1;
    script1 += "dbName = \""+dbName+"\"\n";
    script1 += "tableName=\""+tableName+"\"\n";
    script1 += "login(\"admin\",\"123456\")\n";
    script1 += "if(existsDatabase(dbName)){\n";
    script1 += " dropDatabase(dbName)\n";
    script1 += "}\n";
    script1 += "db  = database(dbName, RANGE,0 5000 15000);\n";
    script1 += "t = table(1000:0, `symbol`id`value,[SYMBOL, INT, INT]);"\
              "pt = db.createPartitionedTable(t,tableName,`id);";
    conn.run(script1);
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    int colNum = 3, rowNum = 1000;
    std::vector<std::string> colNames = {"symbol", "id", "value"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SYMBOL, dolphindb::DT_INT, dolphindb::DT_INT};
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tmp1->getColumn(i));
    }
    std::string sym[] = {"A", "B", "C", "D"};
    columnVecs[0]->set(0, dolphindb::Util::createString(sym[rand()%4]));
    columnVecs[1]->set(0, dolphindb::Util::createInt(0));
    columnVecs[2]->setNull(0);
    for (int i = 1; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createString(sym[rand()%4]));
        columnVecs[1]->set(i, dolphindb::Util::createInt(i));
        columnVecs[2]->set(i, dolphindb::Util::createInt((int)(rand()%1000)));
    }
    aftu.upsert(tmp1);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ((res->getColumnType(0)==18 || res->getColumnType(0)==17),true);
    ASSERT_EQ(res->getColumnType(1), 4);
    ASSERT_EQ(res->getColumnType(2), 4);
    ASSERT_EQ(tmp1->getString(), res->getString());
}

TEST_F(AutoFitTableUpsertTest, test_upsertToPartitionTableRangeTypeIgnoreNull){
    std::string script1;
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    script1 += "dbName = \""+dbName+"\"\n";
    script1 += "tableName=\""+tableName+"\"\n";
    script1 += "login(\"admin\",\"123456\")\n";
    script1 += "if(existsDatabase(dbName)){\n";
    script1 += " dropDatabase(dbName)\n";
    script1 += "}\n";
    script1 += "db  = database(dbName, RANGE,0 5000 15000);\n";
    script1 += "t = table(10:0, `symbol`id`value,[SYMBOL, INT, INT]);"\
               "tableInsert(t,[`asd,0,10]);"\
               "pt = db.createPartitionedTable(t,tableName,`id);"\
               "tableInsert(pt,t)";
    conn.run(script1);
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, true, &keycolName);
    int colNum = 3, rowNum = 10;
    std::vector<std::string> colNames = {"symbol", "id", "value"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SYMBOL, dolphindb::DT_INT, dolphindb::DT_INT};
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tmp1->getColumn(i));
    }
    std::string sym[] = {"A", "B", "C", "D"};
    columnVecs[0]->set(0, dolphindb::Util::createString("D"));
    columnVecs[1]->set(0, dolphindb::Util::createInt(0));
    columnVecs[2]->setNull(0);
    for (int i = 1; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createString(sym[rand()%4]));
        columnVecs[1]->set(i, dolphindb::Util::createInt(i));
        columnVecs[2]->set(i, dolphindb::Util::createInt((int)(rand()%1000)));
    }
    std::cout<< tmp1->getString()<<std::endl;
    aftu.upsert(tmp1);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ((res->getColumnType(0)==18 || res->getColumnType(0)==17),true);
    ASSERT_EQ(res->getColumnType(1), 4);
    ASSERT_EQ(res->getColumnType(2), 4);
    ASSERT_EQ(res->getRow(0)->getMember(dolphindb::Util::createString("value"))->getInt(), 10);
    ASSERT_EQ(res->getRow(0)->getMember(dolphindb::Util::createString("symbol"))->getString(), "D");
    ASSERT_EQ(res->getRow(0)->getMember(dolphindb::Util::createString("id"))->getInt(), 0);
    for (int i = 1; i < rowNum; i++) {
        ASSERT_EQ(tmp1->getColumn(0)->getRow(i)->getString(), res->getColumn(0)->getRow(i)->getString());
        ASSERT_EQ(tmp1->getColumn(1)->getRow(i)->getString(), res->getColumn(1)->getRow(i)->getString());
        ASSERT_EQ(tmp1->getColumn(2)->getRow(i)->getString(), res->getColumn(2)->getRow(i)->getString());
    }
}

TEST_F(AutoFitTableUpsertTest, test_upsertToKeyedTable){
    std::string script1;
    std::string tableName = "pt";
    script1 += "t = table(1000:0, `symbol`id`value,[SYMBOL, INT, INT]);"\
              "pt = keyedTable(`id, t);";
    conn.run(script1);
    dolphindb::AutoFitTableUpsert aftu("", tableName, conn, false);
    int colNum = 3, rowNum = 1000;
    std::vector<std::string> colNames = {"symbol", "id", "value"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SYMBOL, dolphindb::DT_INT, dolphindb::DT_INT};
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tmp1->getColumn(i));
    }
    std::string sym[] = {"A", "B", "C", "D"};
    columnVecs[0]->set(0, dolphindb::Util::createString(sym[rand()%4]));
    columnVecs[1]->set(0, dolphindb::Util::createInt(0));
    columnVecs[2]->setNull(0);
    for (int i = 1; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createString(sym[rand()%4]));
        columnVecs[1]->set(i, dolphindb::Util::createInt(i));
        columnVecs[2]->set(i, dolphindb::Util::createInt((int)(rand()%1000)));
    }
    aftu.upsert(tmp1);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ((res->getColumnType(0)==18 || res->getColumnType(0)==17),true);
    ASSERT_EQ(res->getColumnType(1), 4);
    ASSERT_EQ(res->getColumnType(2), 4);
    ASSERT_EQ(tmp1->getString(), res->getString());
}

TEST_F(AutoFitTableUpsertTest, test_upsertToKeyedTableIgnoreNull){
    std::string script1;
    std::string tableName = "pt";
    script1 += "t = table(1000:0, `symbol`id`value,[SYMBOL, INT, INT]);"\
              "pt = keyedTable(`id, t);\
              tableInsert(pt,`asd,0,10)";
    conn.run(script1);
    dolphindb::AutoFitTableUpsert aftu("", tableName, conn, true);
    int colNum = 3, rowNum = 1000;
    std::vector<std::string> colNames = {"symbol", "id", "value"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SYMBOL, dolphindb::DT_INT, dolphindb::DT_INT};
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tmp1->getColumn(i));
    }
    std::string sym[] = {"A", "B", "C", "D"};
    columnVecs[0]->set(0, dolphindb::Util::createString("D"));
    columnVecs[1]->set(0, dolphindb::Util::createInt(0));
    columnVecs[2]->setNull(0);
    for (int i = 1; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createString(sym[rand()%4]));
        columnVecs[1]->set(i, dolphindb::Util::createInt(i));
        columnVecs[2]->set(i, dolphindb::Util::createInt((int)(rand()%1000)));
    }
    aftu.upsert(tmp1);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ((res->getColumnType(0)==18 || res->getColumnType(0)==17),true);
    ASSERT_EQ(res->getColumnType(1), 4);
    ASSERT_EQ(res->getColumnType(2), 4);
    ASSERT_EQ(res->getRow(0)->getMember(dolphindb::Util::createString("value"))->getInt(), 10);
    ASSERT_EQ(res->getRow(0)->getMember(dolphindb::Util::createString("symbol"))->getString(), "D");
    ASSERT_EQ(res->getRow(0)->getMember(dolphindb::Util::createString("id"))->getInt(), 0);
    for (int i = 1; i < rowNum; i++) {
        ASSERT_EQ(tmp1->getColumn(0)->getRow(i)->getString(), res->getColumn(0)->getRow(i)->getString());
        ASSERT_EQ(tmp1->getColumn(1)->getRow(i)->getString(), res->getColumn(1)->getRow(i)->getString());
        ASSERT_EQ(tmp1->getColumn(2)->getRow(i)->getString(), res->getColumn(2)->getRow(i)->getString());
    }
}

TEST_F(AutoFitTableUpsertTest, test_upsertToindexedTable){
    std::string script1;
    std::string tableName = "pt";
    script1 += "t = table(1000:0, `symbol`id`value,[SYMBOL, INT, INT]);"\
              "pt = indexedTable(`id, t);";
    conn.run(script1);
    dolphindb::AutoFitTableUpsert aftu("", tableName, conn, false);
    int colNum = 3, rowNum = 1000;
    std::vector<std::string> colNames = {"symbol", "id", "value"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SYMBOL, dolphindb::DT_INT, dolphindb::DT_INT};
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tmp1->getColumn(i));
    }
    std::string sym[] = {"A", "B", "C", "D"};
    columnVecs[0]->set(0, dolphindb::Util::createString(sym[rand()%4]));
    columnVecs[1]->set(0, dolphindb::Util::createInt(0));
    columnVecs[2]->setNull(0);
    for (int i = 1; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createString(sym[rand()%4]));
        columnVecs[1]->set(i, dolphindb::Util::createInt(i));
        columnVecs[2]->set(i, dolphindb::Util::createInt((int)(rand()%1000)));
    }
    aftu.upsert(tmp1);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ((res->getColumnType(0)==18 || res->getColumnType(0)==17),true);
    ASSERT_EQ(res->getColumnType(1), 4);
    ASSERT_EQ(res->getColumnType(2), 4);
    ASSERT_EQ(tmp1->getString(), res->getString());
}

TEST_F(AutoFitTableUpsertTest, test_upsertToindexedTableIgnoreNull){
    std::string script1;
    std::string tableName = "pt";
    script1 += "t = table(1000:0, `symbol`id`value,[SYMBOL, INT, INT]);"\
              "pt = indexedTable(`id, t);\
              tableInsert(pt,`asd,0,10)";
    conn.run(script1);
    dolphindb::AutoFitTableUpsert aftu("", tableName, conn, true);
    int colNum = 3, rowNum = 1000;
    std::vector<std::string> colNames = {"symbol", "id", "value"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SYMBOL, dolphindb::DT_INT, dolphindb::DT_INT};
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tmp1->getColumn(i));
    }
    std::string sym[] = {"A", "B", "C", "D"};
    columnVecs[0]->set(0, dolphindb::Util::createString("D"));
    columnVecs[1]->set(0, dolphindb::Util::createInt(0));
    columnVecs[2]->setNull(0);
    for (int i = 1; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createString(sym[rand()%4]));
        columnVecs[1]->set(i, dolphindb::Util::createInt(i));
        columnVecs[2]->set(i, dolphindb::Util::createInt((int)(rand()%1000)));
    }
    aftu.upsert(tmp1);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ((res->getColumnType(0)==18 || res->getColumnType(0)==17),true);
    ASSERT_EQ(res->getColumnType(1), 4);
    ASSERT_EQ(res->getColumnType(2), 4);
    ASSERT_EQ(res->getRow(0)->getMember(dolphindb::Util::createString("value"))->getInt(), 10);
    ASSERT_EQ(res->getRow(0)->getMember(dolphindb::Util::createString("symbol"))->getString(), "D");
    ASSERT_EQ(res->getRow(0)->getMember(dolphindb::Util::createString("id"))->getInt(), 0);
    for (int i = 1; i < rowNum; i++) {
        ASSERT_EQ(tmp1->getColumn(0)->getRow(i)->getString(), res->getColumn(0)->getRow(i)->getString());
        ASSERT_EQ(tmp1->getColumn(1)->getRow(i)->getString(), res->getColumn(1)->getRow(i)->getString());
        ASSERT_EQ(tmp1->getColumn(2)->getRow(i)->getString(), res->getColumn(2)->getRow(i)->getString());
    }
}

TEST_F(AutoFitTableUpsertTest, test_upsertToPartitionTableRangeTypeWithsortColumns){
    std::string script1;
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string tableName = "pt";
    script1 += "dbName = \""+dbName+"\"\n";
    script1 += "tableName=\""+tableName+"\"\n";
    script1 += "login(\"admin\",\"123456\")\n";
    script1 += "if(existsDatabase(dbName)){\n";
    script1 += " dropDatabase(dbName)\n";
    script1 += "}\n";
    script1 += "db  = database(dbName, RANGE,0 5000 15000);\n";
    script1 += "t = table(1000:0, `symbol`id`value,[SYMBOL, INT, INT]);"\
               "pt = db.createPartitionedTable(t,tableName,`id);";
    conn.run(script1);
    int colNum = 3;
    int rowNum = 1000;
    std::vector<dolphindb::VectorSP> columnVecs;
    std::vector<dolphindb::VectorSP> columnVecs2;
    std::vector<std::string> colNames = {"symbol", "id", "value"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SYMBOL, dolphindb::DT_INT, dolphindb::DT_INT};
    dolphindb::TableSP tmp1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, rowNum);
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(tmp1->getColumn(i));
        }
    std::string sym[] = {"A", "B", "C", "D"};
    columnVecs[0]->set(0, dolphindb::Util::createString("D"));
    columnVecs[1]->set(0, dolphindb::Util::createInt(0));
    columnVecs[2]->setInt(1000);
    for (int i = 1; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createString(sym[rand()%4]));
        columnVecs[1]->set(i, dolphindb::Util::createInt(0));
        columnVecs[2]->set(i, dolphindb::Util::createInt((int)(rowNum-i)));
    }
    conn.upload("tmp1",{tmp1});
    conn.run("tableInsert(pt,tmp1)");
    dolphindb::Util::sleep(2000);
    std::vector<std::string> keycolName = {"id"};
    std::vector<std::string> sortColName = {"value"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName, &sortColName);
    int rowNum2 = 1;
    dolphindb::TableSP tmp2 = dolphindb::Util::createTable(colNames, colTypes, rowNum2, rowNum2);
    columnVecs2.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs2.emplace_back(tmp2->getColumn(i));
    }
    columnVecs2[0]->set(0, dolphindb::Util::createString("D"));
    columnVecs2[1]->set(0, dolphindb::Util::createInt(0));
    columnVecs2[2]->setInt(0);
    aftu.upsert(tmp2);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ((res->getColumnType(0)==18 || res->getColumnType(0)==17),true);
    ASSERT_EQ(res->getColumnType(1), 4);
    ASSERT_EQ(res->getColumnType(2), 4);
    for (int i = 1; i < rowNum; i++){
        ASSERT_EQ((res->getColumn(2)->getRow(i)->getInt() > res->getColumn(2)->getRow(i-1)->getInt()), true);
    }
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithIntArrayVectorToPartitionTableRangeType){
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
    v1->setInt(0, 1);
    v1->setInt(1, 100);
    v1->setInt(2, 9999);
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_INT_ARRAY, 0, 3);
    av1->append(v1);
    av1->append(v1);
    av1->append(v1);
    std::vector<std::string> colNames = { "id", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_INT_ARRAY };
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithIntArrayVectorNullToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithIntArrayVectorToPartitionTableRangeTypeMorethan65535){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithCharArrayVectorToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithCharArrayVectorNullToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithCharArrayVectorToPartitionTableRangeTypeMorethan65535){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithFloatArrayVectorToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithFloatArrayVectorNullToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());

}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithFloatArrayVectorToPartitionTableRangeTypeMorethan65535){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithDateArrayVectorToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithDateArrayVectorNullToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithDateArrayVectorToPartitionTableRangeTypeMorethan65535){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithMonthArrayVectorToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithMonthArrayVectorNullToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithMonthArrayVectorToPartitionTableRangeTypeMorethan65535){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithTimeArrayVectorToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithTimeArrayVectorNullToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithTimeArrayVectorToPartitionTableRangeTypeMorethan65535){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithSecondArrayVectorToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithSecondArrayVectorNullToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithSecondArrayVectorToPartitionTableRangeTypeMorethan65535){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithDatehourArrayVectorToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithDatehourArrayVectorNullToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithDatehourArrayVectorToPartitionTableRangeTypeMorethan65535){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithUuidArrayVectorToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithUuidArrayVectorNullToPartitionTableRangeType){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

TEST_F(AutoFitTableUpsertTest, test_upsertTablewithUuidArrayVectorToPartitionTableRangeTypeMorethan65535){
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
    int colNum = 2, rowNum = 3;
    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }
    std::vector<std::string> keycolName = {"id"};
    dolphindb::AutoFitTableUpsert aftu(dbName, tableName, conn, false, &keycolName);
    aftu.upsert(tab);
    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
}

class AFTU_append_null : public AutoFitTableUpsertTest, public testing::WithParamInterface<std::tuple<std::string, dolphindb::DATA_TYPE>>
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
INSTANTIATE_TEST_SUITE_P(, AFTU_append_null, testing::ValuesIn(AFTU_append_null::data_prepare()));

TEST_P(AFTU_append_null, test_append_empty_table)
{
    std::string case_=getCaseName();
    std::string type = std::get<0>(GetParam());
    std::string dbName="dfs://" + case_ + "_" + type;
    dolphindb::DATA_TYPE dataType = std::get<1>(GetParam());
    std::string colName = "c1";
    std::string script1 =
        "colName = [`ind,`time, `" + colName + "];"
        "colType = [INT, DATETIME, " + type + "];"
        "t=table(1:0, colName, colType);"
        "dbName='"+dbName+"'"
        "if(existsDatabase(dbName)) dropDatabase(dbName);go;"
        "db = database(dbName, VALUE, 1..10,,'TSDB');"
        "db.createPartitionedTable(t, `pt, `ind,,`ind`time)";
    conn.run(script1);
    dolphindb::VectorSP col0 = dolphindb::Util::createVector(dolphindb::DT_INT, 0);
    dolphindb::VectorSP col1 = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 0);
    dolphindb::VectorSP col2 = dolphindb::Util::createVector(dataType, 0);
    std::vector<std::string> colNames = { "ind", "time", "c1"};
    std::vector<dolphindb::ConstantSP> cols = { col0, col1, col2};
    dolphindb::TableSP empty2 = dolphindb::Util::createTable(colNames, cols);
    std::vector<std::string>* kcols = new std::vector<std::string>{"ind", "time"};
    dolphindb::AutoFitTableUpsert upsert(dbName, "pt", conn, true, kcols);
    upsert.upsert(empty2);
    auto res = conn.run("exec * from loadTable(dbName, `pt)");
    ASSERT_EQ(res->rows(), 0);
    conn.run("dropDatabase(dbName);go");
    delete kcols;
}

TEST_F(AutoFitTableUpsertTest, test_AutoFitTableUpsert_with_SCRAM_session)
{
    std::string userName=getRandString(20);
    conn.run(
        "userName='"+userName+"';"
        "try{deleteUser(userName)}catch(ex){};go;createUser(userName, `123456, authMode='scram')"
    );
    dolphindb::DBConnection conn_scram(false, false, 7200, false, false, false, true);
    conn_scram.connect(HOST, PORT, userName, PASSWD);
    dolphindb::TableSP data = conn_scram.run("t = indexedTable(`c1, 1 2 3 as c1, rand(100.00, 3) as c2);t2 = t.copy(); t");
    dolphindb::AutoFitTableUpsert upsert("", "t2", conn_scram);
    upsert.upsert(data);
    auto res = conn_scram.run("all(each(eqObj, values t2, values t))")->getBool();
    ASSERT_EQ(res, true);
    conn_scram.close();
}
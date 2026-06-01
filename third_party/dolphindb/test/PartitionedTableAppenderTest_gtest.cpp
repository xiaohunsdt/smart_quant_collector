#include <gtest/gtest.h>
#include "config.h"

class PartitionedTableAppenderTest:public testing::Test
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

dolphindb::DBConnection PartitionedTableAppenderTest::conn(false, false);

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_int){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,VALUE,0..9);";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`id);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "id", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_date1){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,VALUE,2012.01.01..2012.01.10);";
    script += "dummy = table(1:0, `id`sym`date`value, [INT, SYMBOL, DATE, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`date);";
    conn.run(script);
    std::vector<std::string> colNames = { "id", "sym", "date", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_DATE, dolphindb::DT_INT };
    int colNum = 4, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createDate(2012, 1, i % 10 + 1));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "date", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, take(2012.01.01..2012.01.10, 1000000) as date, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by date, id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by date, id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_date2){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,VALUE,2012.01.01..2012.01.10);";
    script += "dummy = table(1:0, `id`sym`time`value, [INT, SYMBOL, DATETIME, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`time);";
    conn.run(script);
    std::vector<std::string> colNames = { "id", "sym", "time", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_DATETIME, dolphindb::DT_INT };
    int colNum = 4, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createDateTime(2012, 1, i % 10 + 1, 9, 30, 30));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "time", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, concatDateTime(take(2012.01.01..2012.01.10, 1000000), 09:30:30) as time, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by time, id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by time, id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_date3){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,VALUE,2012.01.01..2012.01.10);";
    script += "dummy = table(1:0, `id`sym`time`value, [INT, SYMBOL, TIMESTAMP, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`time);";
    conn.run(script);
    std::vector<std::string> colNames = { "id", "sym", "time", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_TIMESTAMP, dolphindb::DT_INT };
    int colNum = 4, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createTimestamp(2012, 1, i % 10 + 1, 9, 30, 30, 125));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "time", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, concatDateTime(take(2012.01.01..2012.01.10, 1000000), 09:30:30.125) as time, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by time, id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by time, id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_date4){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,VALUE,2012.01.01..2012.01.10);";
    script += "dummy = table(1:0, `id`sym`time`value, [INT, SYMBOL, NANOTIMESTAMP, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`time);";
    conn.run(script);
    std::vector<std::string> colNames = { "id", "sym", "time", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_INT };
    int colNum = 4, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createNanoTimestamp(2012, 1, i % 10 + 1, 9, 30, 30, 00));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "time", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, concatDateTime(take(2012.01.01..2012.01.10, 1000000), 09:30:30.000000000) as time, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by time, id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by time, id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_month1){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,VALUE,2012.01M..2012.10M);";
    script += "dummy = table(1:0, `id`sym`date`value, [INT, SYMBOL, DATE, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`date);";
    conn.run(script);
    std::vector<std::string> colNames = { "id", "sym", "date", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_DATE, dolphindb::DT_INT };
    int colNum = 4, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createDate(2012, i % 10 + 1, 10));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "date", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, take(temporalAdd(2012.01.10, 0..9, \"M\"), 1000000) as date, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by date, id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by date, id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_month2){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,VALUE,2012.01M..2012.10M);";
    script += "dummy = table(1:0, `id`sym`time`value, [INT, SYMBOL, DATETIME, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`time);";
    conn.run(script);
    std::vector<std::string> colNames = { "id", "sym", "time", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_DATETIME, dolphindb::DT_INT };
    int colNum = 4, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createDateTime(2012, i % 10 + 1, 10, 9, 30, 30));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "time", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, concatDateTime(take(temporalAdd(2012.01.10, 0..9, \"M\"), 1000000), 09:30:30) as time, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by time, id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by time, id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_month3){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,VALUE,2012.01M..2012.10M);";
    script += "dummy = table(1:0, `id`sym`time`value, [INT, SYMBOL, TIMESTAMP, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`time);";
    conn.run(script);
    std::vector<std::string> colNames = { "id", "sym", "time", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_TIMESTAMP, dolphindb::DT_INT };
    int colNum = 4, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createTimestamp(2012, i % 10 + 1, 10, 9, 30, 30, 125));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "time", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, concatDateTime(take(temporalAdd(2012.01.10, 0..9, \"M\"), 1000000), 09:30:30.125) as time, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by time, id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by time, id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_symbol){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,VALUE,\"A\"+string(0..9));";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`sym);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 10)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "sym", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..9), 1000000) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_range_int){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "ranges = cutPoints(0..999999, 10);";
    script += "db = database(dbPath,RANGE,ranges);";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`id);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 10)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "id", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(0..999999 as id, take(\"A\"+string(0..9), 1000000) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_range_date){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,RANGE,date(2012.01M..2012.11M));";
    script += "dummy = table(1:0, `id`sym`date`value, [INT, SYMBOL, DATE, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`date);";
    conn.run(script);
    std::vector<std::string> colNames = { "id", "sym", "date", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_DATE, dolphindb::DT_INT };
    int colNum = 4, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createDate(2012, i % 10 + 1, 10));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "date", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, take(temporalAdd(2012.01.10, 0..9, \"M\"), 1000000) as date, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by date, id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by date, id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_range_symbol){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "ranges = cutPoints(\"A\"+string(0..999999), 10);";
    script += "db = database(dbPath,RANGE,ranges);";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`sym);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "sym", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(0..999999 as id, \"A\"+string(0..999999) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_range_string){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "ranges = cutPoints(\"A\"+string(0..999999), 10);";
    script += "db = database(dbPath,RANGE,ranges);";
    script += "dummy = table(1:0, `id`sym`value, [INT, STRING, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`sym);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "sym", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(0..999999 as id, \"A\"+string(0..999999) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_hash_int){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,HASH,[INT, 10]);";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`id);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 10)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "id", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(0..999999 as id, take(\"A\"+string(0..9), 1000000) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_hash_date){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,HASH,[DATE,10]);";
    script += "dummy = table(1:0, `id`sym`date`value, [INT, SYMBOL, DATE, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`date);";
    conn.run(script);
    std::vector<std::string> colNames = { "id", "sym", "date", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_DATE, dolphindb::DT_INT };
    int colNum = 4, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createDate(2012, i % 10 + 1, 10));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "date", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, take(temporalAdd(2012.01.10, 0..9, \"M\"), 1000000) as date, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by date, id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by date, id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_hash_symbol){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,HASH,[SYMBOL,10]);";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`sym);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "sym", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(0..999999 as id, \"A\"+string(0..999999) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_hash_string){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,HASH,[STRING,10]);";
    script += "dummy = table(1:0, `id`sym`value, [INT, STRING, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`sym);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "sym", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(0..999999 as id, \"A\"+string(0..999999) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_list_int){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,LIST,[0..3, 4, 5..7, 8..9]);";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`id);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "id", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_list_date){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,LIST,[2012.01.01..2012.01.03, 2012.01.04..2012.01.07, 2012.01.08..2012.01.10]);";
    script += "dummy = table(1:0, `id`sym`date`value, [INT, SYMBOL, DATE, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`date);";
    conn.run(script);
    std::vector<std::string> colNames = { "id", "sym", "date", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_DATE, dolphindb::DT_INT };
    int colNum = 4, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createDate(2012, 1, i % 10 + 1));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "date", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, take(2012.01.01..2012.01.10, 1000000) as date, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by date, id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by date, id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_list_string){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,LIST,[`A0`A1, `A2, `A3, `A4`A5`A6`A7, `A8`A9]);";
    script += "dummy = table(1:0, `id`sym`value, [INT, STRING, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`sym);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_STRING, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 10)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "sym", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..9), 1000000) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_list_symbol){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,LIST,[`A0`A1, `A2, `A3, `A4`A5`A6`A7, `A8`A9]);";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`sym);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 10)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "sym", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..9), 1000000) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_hash){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db1 = database(\"\",VALUE,`A0`A1`A2`A3`A4`A5`A6`A7`A8`A9);";
    script += "db2 = database(\"\",HASH,[INT, 2]);";
    script += "db = database(dbPath, COMPO, [db1, db2]);";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`sym`id);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 10)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "sym", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..9), 1000000) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_range){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db1 = database(\"\",VALUE,`A0`A1`A2`A3`A4`A5`A6`A7`A8`A9);";
    script += "db2 = database(\"\",RANGE,0 5 10);";
    script += "db = database(dbPath, COMPO, [db1, db2]);";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`sym`id);";
    conn.run(script);
    //create tableSP
    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 10)));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "sym", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from loadTable(dbPath, `pt)";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..9), 1000000) as sym, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by id, sym, value;";
    script2 += "re = select * from loadTable(dbPath, `pt) order by id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<3; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_value_alltypes_OLAP){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    int colNum = 23, rowNum = 1000;
    int scale32 = rand()%9, scale64 = rand()%18;

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
        columnVecs[20]->set(i, dolphindb::Util::createDateHour(rand()%INT_MAX));
        columnVecs[21]->set(i, dolphindb::Util::createDecimal32(rand()%10,rand()/float(RAND_MAX)));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal64(rand()%19,rand()/double(RAND_MAX)));
    }
    for (int j = 0; j < colNum; j++){
        if(j == 3)
            columnVecs[3]->set(rowNum-1, dolphindb::Util::createInt(rowNum-1));  //partition-column's value must be not null
        else
            columnVecs[j]->setNull(rowNum-1);
    }

    std::string tableName = "pt";
    std::string script = "dbName = '"+dbName+"';"
            "if(exists(dbName)){dropDatabase(dbName)};"
            "db  = database(dbName, VALUE,0..1000);"
            "temp = table(1000:0, take(`col,23)+string(take(0..22,23)), \
            [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+")]);"
            "pt = createPartitionedTable(db,temp,`pt,`col3);";
    conn.run(script);
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "col3", pool);
    int res = appender.append(tab1);
    ASSERT_EQ(res, rowNum);

    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "st1 = select * from pt;";
    script3 += "each(eqObj, tab1.values(), st1.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_hash_alltypes_OLAP){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    int colNum = 23, rowNum = 1000;
    int scale32 = rand()%9, scale64 = rand()%18;

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
        columnVecs[21]->set(i, dolphindb::Util::createDecimal32(rand()%10,rand()/float(RAND_MAX)));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal64(rand()%19,rand()/double(RAND_MAX)));
    }
    for (int j = 0; j < colNum; j++){
        if(j == 3)
            columnVecs[3]->set(rowNum-1, dolphindb::Util::createInt(rand()%INT_MAX));  //partition-column's value must be not null
        else
            columnVecs[j]->setNull(rowNum-1);
    }

    std::string tableName = "pt";
    std::string script = "dbName = '"+dbName+"';"
            "if(exists(dbName)){dropDatabase(dbName)};"
            "db  = database(dbName, HASH,[INT,1]);"
            "temp = table(1000:0, take(`col,23)+string(take(0..22,23)), \
            [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+")]);"
            "pt = createPartitionedTable(db,temp,`pt,`col3);";
    conn.run(script);
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "col3", pool);
    int res = appender.append(tab1);
    ASSERT_EQ(res, rowNum);

    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "st1 = select * from pt;";
    script3 += "each(eqObj, tab1.values(), st1.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_range_alltypes_OLAP){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    int colNum = 23, rowNum = 1000;
    int scale32 = rand()%9, scale64 = rand()%18;

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
        columnVecs[20]->set(i, dolphindb::Util::createDateHour(rand()%INT_MAX));
        columnVecs[21]->set(i, dolphindb::Util::createDecimal32(rand()%10,rand()/float(RAND_MAX)));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal64(rand()%19,rand()/double(RAND_MAX)));
    }
    for (int j = 0; j < colNum; j++){
        if(j == 3)
            columnVecs[3]->set(rowNum-1, dolphindb::Util::createInt(rowNum-1));  //partition-column's value must be not null
        else
            columnVecs[j]->setNull(rowNum-1);
    }
    std::string tableName = "pt";
    std::string script = "dbName = '"+dbName+"';"
            "if(exists(dbName)){dropDatabase(dbName)};"
            "db  = database(dbName, RANGE,0 1000);"
            "temp = table(1000:0, take(`col,23)+string(take(0..22,23)), \
            [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+")]);"
            "pt = createPartitionedTable(db,temp,`pt,`col3);";
    conn.run(script);
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "col3", pool);
    int res = appender.append(tab1);
    ASSERT_EQ(res, rowNum);

    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "st1 = select * from pt;";
    script3 += "each(eqObj, tab1.values(), st1.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_list_alltypes_OLAP){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    int colNum = 23, rowNum = 1000;
    int scale32 = rand()%9, scale64 = rand()%18;

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
        columnVecs[20]->set(i, dolphindb::Util::createDateHour(rand()%INT_MAX));
        columnVecs[21]->set(i, dolphindb::Util::createDecimal32(rand()%10,rand()/float(RAND_MAX)));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal64(rand()%19,rand()/double(RAND_MAX)));
    }
    for (int j = 0; j < colNum; j++){
        if(j == 3)
            columnVecs[3]->set(rowNum-1, dolphindb::Util::createInt(rowNum-1));  //partition-column's value must be not null
        else
            columnVecs[j]->setNull(rowNum-1);
    }
    std::string tableName = "pt";
    std::string script = "dbName = '"+dbName+"';"
            "if(exists(dbName)){dropDatabase(dbName)};"
            "db  = database(dbName, LIST,0..1000);"
            "temp = table(1000:0, take(`col,23)+string(take(0..22,23)), \
            [CHAR, BOOL, SHORT, INT, LONG, DATE, MONTH, TIME, MINUTE, DATETIME, SECOND, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, STRING, UUID, IPADDR, INT128, DATEHOUR, DECIMAL32("+std::to_string(scale32)+"), DECIMAL64("+std::to_string(scale64)+")]);"
            "pt = createPartitionedTable(db,temp,`pt,`col3);";
    conn.run(script);
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "col3", pool);
    int res = appender.append(tab1);
    ASSERT_EQ(res, rowNum);

    conn.upload("tab1",tab1);
    std::string script3;
    script3 += "st1 = select * from pt;";
    script3 += "each(eqObj, tab1.values(), st1.values());";
    dolphindb::ConstantSP result2 = conn.run(script3);
    for (int i = 0; i<result2->size(); i++)
        ASSERT_TRUE(result2->get(i)->getBool());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withIntArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::TableSP tab = dolphindb::Util::createTable(colNames, colTypes, 0, 10);
    std::vector<dolphindb::ConstantSP> colVecs{ v1,av1 };
    dolphindb::INDEX insertrows;
    std::string errmsg;
    if (tab->append(colVecs, insertrows, errmsg) == false) {
            std::cerr << errmsg;
    }

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withIntNullArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withIntArrayVectorMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withCharArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withCharNullArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withCharArrayVectorMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withFloatArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;    
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withFloatNullArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withFloatArrayVectorMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withDateArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withDateNullArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withDateArrayVectorMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withMonthArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withMonthNullArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withMonthArrayVectorMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withTimeArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withTimeNullArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withTimeArrayVectorMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withSecondArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withSecondNullArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withSecondArrayVectorMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withDatehourArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withDatehourNullArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withDatehourArrayVectorMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withUuidArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withUuidNullArrayVector){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_withUuidArrayVectorMorethan65535){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
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

    dolphindb::PartitionedTableAppender appender(dbName, tableName, "id", pool);
    appender.append(tab);

    dolphindb::TableSP res=conn.run("select * from pt;");
    ASSERT_EQ(tab->getString(), res->getString());
    ASSERT_EQ(tab->getColumn(0)->getType(), res->getColumn(0)->getType());
    ASSERT_EQ(tab->getColumn(1)->getType(), res->getColumn(1)->getType());
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_partitionColNameNull){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db1 = database(\"\",VALUE,`A0`A1`A2`A3`A4`A5`A6`A7`A8`A9);";
    script += "db2 = database(\"\",RANGE,0 5 10);";
    script += "db = database(dbPath, COMPO, [db1, db2]);";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`sym`id);";
    conn.run(script);
    ASSERT_ANY_THROW(dolphindb::PartitionedTableAppender appender(dbName, "pt", "", pool));
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_partitionColNameValueNull){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    //create dfs db
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db1 = database(\"\",VALUE,`A0`A1`A2`A3`A4`A5`A6`A7`A8`A9);";
    script += "db2 = database(\"\",RANGE,0 5 10);";
    script += "db = database(dbPath, COMPO, [db1, db2]);";
    script += "dummy = table(1:0, `id`sym`value, [INT, SYMBOL, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`sym`id);";
    conn.run(script);

    std::vector<std::string> colNames = { "id", "sym", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_INT };
    int colNum = 3, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString(""));
        columnVecs[2]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "sym", pool);
    ASSERT_ANY_THROW(appender.append(t1));
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_PartitionedTableAppender_compo3){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db1 = database(\"\",VALUE,2012.01.01..2012.01.10);";
    script += "ranges=cutPoints(\"A\"+string(0..999), 5);";
    script += "db2 = database(\"\",RANGE,ranges);";
    script += "db3 = database(\"\",HASH,[INT, 2]);";
    script += "db = database(dbPath,COMPO,[db1, db2, db3]);";
    script += "dummy = table(1:0, `id`sym`date`value, [INT, SYMBOL, DATE, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`date`sym`id);";
    conn.run(script);
    std::vector<std::string> colNames = { "id", "sym", "date", "value" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_DATE, dolphindb::DT_INT };
    int colNum = 4, rowNum = 1000000;
    dolphindb::TableSP t1 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t1->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createInt(i % 10));
        columnVecs[1]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createDate(2012, 1, i % 10 + 1));
        columnVecs[3]->set(i, dolphindb::Util::createInt(i));
    }
    dolphindb::PartitionedTableAppender appender(dbName, "pt", "date", pool);
    appender.append(t1);
    std::string script1;
    script1 += "exec count(*) from pt";
    int total = 0;
    total = conn.run(script1)->getInt();
    ASSERT_EQ(total, 1000000);
    std::string script2;
    script2 += "tmp = table(take(0..9, 1000000) as id, take(\"A\"+string(0..999), 1000000) as sym, take(2012.01.01..2012.01.10, 1000000) as date, 0..999999 as value);";
    script2 += "expected = select *  from tmp order by date, id, sym, value;";
    script2 += "re = select * from pt order by date, id, sym, value;";
    script2 += "each(eqObj, re.values(), expected.values()).int();";
    dolphindb::ConstantSP result = conn.run(script2);
    for (int i = 0; i<4; i++)
        ASSERT_EQ(result->getInt(i), 1);
    pool.shutDown();
}

TEST_F(PartitionedTableAppenderTest,test_mutithread_schema){
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
    int64_t startTime, time;
    startTime = dolphindb::Util::getEpochTime();
    pool.run("sleep(1000)", 0);
    pool.run("sleep(3000)", 1);
    pool.run("sleep(5000)", 2);
    while (true) {
        if (pool.isFinished(0) && pool.isFinished(1) && pool.isFinished(2)) {
            time = (dolphindb::Util::getEpochTime() - startTime) / 1000;
            ASSERT_EQ((int)time, 5);
            break;
        }
    }
    pool.shutDown();
}


class PTA_append_null : public PartitionedTableAppenderTest, public testing::WithParamInterface<std::tuple<std::string, dolphindb::DATA_TYPE>>
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
INSTANTIATE_TEST_SUITE_P(, PTA_append_null, testing::ValuesIn(PTA_append_null::data_prepare()));

TEST_P(PTA_append_null, test_append_empty_table)
{
    dolphindb::DBConnectionPoolSP pool = new dolphindb::DBConnectionPool(HOST, PORT, 10, USER, PASSWD);
    std::string type = std::get<0>(GetParam());
    dolphindb::DATA_TYPE dataType = std::get<1>(GetParam());
    std::string colName = "c1";
    std::string dbName="dfs://"+getRandString(10);
    std::string script1 =
        "colName = [`ind,`time, `" + colName + "];"
        "colType = [INT, DATETIME, " + type + "];"
        "t=table(1:0, colName, colType);"
        "if(existsDatabase('"+dbName+"')) dropDatabase('"+dbName+"');go;"
        "db = database('"+dbName+"', VALUE, 1..10,,'TSDB');"
        "db.createPartitionedTable(t, `pt, `ind,,`ind`time)";

    conn.run(script1);
    dolphindb::VectorSP col0 = dolphindb::Util::createVector(dolphindb::DT_INT, 0);
    dolphindb::VectorSP col1 = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 0);
    dolphindb::VectorSP col2 = dolphindb::Util::createVector(dataType, 0);
    std::vector<std::string> colNames = { "c1", "c2", "c3" };
    std::vector<dolphindb::ConstantSP> cols = { col0, col1, col2};
    dolphindb::TableSP empty2 = dolphindb::Util::createTable(colNames, cols);

    dolphindb::PartitionedTableAppenderSP appender = new dolphindb::PartitionedTableAppender(dbName, "pt", "ind", *pool);
    int rows = appender->append(empty2);
    ASSERT_EQ(rows, 0);
    auto res = conn.run("exec * from loadTable('"+dbName+"', `pt)");
    ASSERT_EQ(res->rows(), 0);
}


TEST_F(PartitionedTableAppenderTest, test_PartitionedTableAppender_SCRAM_user){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string userName=getRandString(20);
    conn.run(
        "userName='"+userName+"';"
        "try{deleteUser(userName)}catch(ex){};go;createUser(userName, `123456, authMode='scram')"
    );
    std::string script;
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db1 = database(\"\",VALUE,2012.01.01..2012.01.10);";
    script += "ranges=cutPoints(\"A\"+string(0..999), 5);";
    script += "db2 = database(\"\",RANGE,ranges);";
    script += "db3 = database(\"\",HASH,[INT, 2]);";
    script += "db = database(dbPath,COMPO,[db1, db2, db3]);";
    script += "share table(1..100 as id, \"A\"+string(0..99) as sym, take(2012.01.01..2013.01.10, 100) as date, 1..100 as value) as "+case_+";";
    script += "pt = db.createPartitionedTable("+case_+",`pt,`date`sym`id);"
                "grant(userName, DB_MANAGE, dbPath);"
                "grant(userName, TABLE_WRITE, dbPath+'/pt');"
                "grant(userName, TABLE_READ, dbPath+'/pt');";
    conn.run(script);
    dolphindb::DBConnectionPool pool(HOST, PORT, 10, userName, "123456");
    pool.run(case_, 1);
    while (!pool.isFinished(1)) {
        dolphindb::Util::sleep(1000);
    }
    dolphindb::TableSP t = pool.getData(1);

    dolphindb::PartitionedTableAppender appender(dbName, "pt", "id", pool);
    int rows = appender.append(t);
    ASSERT_EQ(rows, 100);
    ASSERT_TRUE(conn.run("res = select * from "+case_+" order by id;ex = select * from loadTable('"+dbName+"', `pt) order by id;all(each(eqObj, res.values(), ex.values()))")->getBool());
    pool.shutDown();
}
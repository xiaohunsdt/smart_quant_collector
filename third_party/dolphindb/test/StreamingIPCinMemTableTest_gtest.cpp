//../../lib/gtest-parallel/gtest-parallel -w 4 ./DolphinDBAPI_test
#ifdef __linux__
#include <gtest/gtest.h>
#include "config.h"
#include "Streaming.h"

class StreamingIPCinMemoryTableTest : public testing::Test
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
            std::string case_=getCaseNameHash();
            conn.run("try{dropIPCInMemoryTable(`"+case_+");}catch(ex){};\
                share streamTable(10000:0,`timestamp`temperature`currenttime, [TIMESTAMP,DOUBLE,NANOTIMESTAMP]) as "+case_+";\n\
                shm_test=createIPCInMemoryTable(1000000, \""+case_+"\", `timestamp`temperature`currenttime, [TIMESTAMP, DOUBLE, NANOTIMESTAMP]);\n\
                def shm_append(msg){loadIPCInMemoryTable(`"+case_+").append!(msg)};\
                topic2 = subscribeTable(tableName=\""+case_+"\", actionName=\"act3\", offset=0, handler=shm_append, msgAsTable=true);");
        }
        virtual void TearDown()
        {

        }
};

dolphindb::DBConnection StreamingIPCinMemoryTableTest::conn(false, false);

int insertTask(int insertNum, int sleepMS)
{
    std::string case_=getCaseNameHash();
    StreamingIPCinMemoryTableTest::conn.run("n="+std::to_string(insertNum)+";tableInsert("+case_+",rand(1..100,n),norm(2,0.4,n),take(now(true),n));sleep(" + std::to_string(sleepMS) + ")");
    std::cout << "insert finished!" << std::endl;
    return 0;
}

void print(dolphindb::TableSP table)
{
    std::string case_=getCaseNameHash();
    dolphindb::ConstantSP time_spend = StreamingIPCinMemoryTableTest::conn.run("cur_tm=now(true);insert_total_time = exec max(currenttime) from loadIPCInMemoryTable(`"+case_+");\n\
                                    tm_spend=cur_tm-insert_total_time;\n\
                                    nanotime(tm_spend)");
    std::cout << "total get rows: " << table->rows() << std::endl
         << "total time spend: " << time_spend->getString() << std::endl
         << std::endl;
}

void insertRow(dolphindb::TableSP table)
{
    dolphindb::AutoFitTableAppender appender("", "t", StreamingIPCinMemoryTableTest::conn);
    appender.append(table);
    return;
}

TEST_F(StreamingIPCinMemoryTableTest, test_basic)
{
    std::string case_=getCaseNameHash();
    std::thread th1 = std::thread(insertTask, 100, 0);
    std::string tableName = case_;
    std::cout << "debug::"<<case_ << std::endl;
    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP};
    int rowNum = 0, indexCapacity = 100;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);
    dolphindb::IPCInMemoryStreamClient IPCMclient;
    dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, nullptr, outputTable, false);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    th1.join();

    dolphindb::Util::sleep(2000);
    IPCMclient.unsubscribe(tableName);
    conn.upload("outputTable", {outputTable});
    dolphindb::VectorSP res = conn.run("each(eqObj, outputTable.values(), "+case_+".values())");
    for (auto i = 0; i < res->size(); i++)
    {
        ASSERT_TRUE(res->get(i)->getBool());
    }
}

TEST_F(StreamingIPCinMemoryTableTest, test_subscribe_tableNameNullstr)
{
    std::string case_=getCaseNameHash();
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;

    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP};
    int rowNum = 0, indexCapacity = 10000;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);

    ASSERT_ANY_THROW(dolphindb::ThreadSP thread0 = IPCMclient.subscribe("", nullptr, outputTable, false));
}

TEST_F(StreamingIPCinMemoryTableTest, test_subscribe_tableNameNotExist)
{
    std::string case_=getCaseNameHash();
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;

    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP};
    int rowNum = 0, indexCapacity = 10000;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);

    ASSERT_ANY_THROW(dolphindb::ThreadSP thread0 = IPCMclient.subscribe("errTableName", nullptr, outputTable, false));
}

TEST_F(StreamingIPCinMemoryTableTest, test_subscribe_handlerNull)
{
    std::string case_=getCaseNameHash();
    std::thread th1 = std::thread(insertTask, 10000, 0);
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;

    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP};
    int rowNum = 0, indexCapacity = 10000;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);

    dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, nullptr, outputTable, false);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    th1.join();

    dolphindb::Util::sleep(2000);
    IPCMclient.unsubscribe(tableName);
    conn.upload("outputTable", {outputTable});
    for (int i = 0; i < 100; i++)
    {
        ASSERT_EQ(conn.run(case_)->getColumn(0)->get(i)->getString(), conn.run("outputTable")->getColumn(0)->get(i)->getString());
        ASSERT_EQ(conn.run(case_)->getColumn(1)->get(i)->getString(), conn.run("outputTable")->getColumn(1)->get(i)->getString());
        ASSERT_EQ(conn.run(case_)->getColumn(2)->get(i)->getString(), conn.run("outputTable")->getColumn(2)->get(i)->getString());
    }
}

TEST_F(StreamingIPCinMemoryTableTest, test_subscribe_outputTableNull)
{
    std::string case_=getCaseNameHash();
    std::thread th1 = std::thread(insertTask, 10000, 0);
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;

    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP};
    int rowNum = 0, indexCapacity = 10000;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);
    conn.upload("t", {outputTable});
    dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, insertRow, nullptr, false);
    int rows = conn.run("t.rows()")->getInt();
    while(rows < 10000){
        rows = conn.run("t.rows()")->getInt();
        dolphindb::Util::sleep(500);
    }
    th1.join();

    IPCMclient.unsubscribe(tableName);
    ASSERT_TRUE(conn.run("all(each(eqObj, t.values(), "+case_+".values()))")->getBool());
}

TEST_F(StreamingIPCinMemoryTableTest, test_subscribe_outputTableErrorColType)
{
    std::string case_=getCaseNameHash();
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;
    std::vector<std::string> colNames = {"timestamp", "temperature", "errCol"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_STRING};
    int rowNum = 0, indexCapacity = 10000;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);
    ASSERT_ANY_THROW(dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, nullptr, outputTable, false));
}

TEST_F(StreamingIPCinMemoryTableTest, test_subscribe_outputTableErrorColNum)
{
    std::string case_=getCaseNameHash();
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;
    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime", "errCol"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_STRING};
    int rowNum = 0, indexCapacity = 10000;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);
    ASSERT_ANY_THROW(dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, nullptr, outputTable, false));
}

TEST_F(StreamingIPCinMemoryTableTest, test_subscribe_overwriteTrue)
{
    std::string case_=getCaseNameHash();
    std::thread th1 = std::thread(insertTask, 10000, 0);
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;

    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP};
    int rowNum = 0, indexCapacity = 10000;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);

    dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, nullptr, outputTable, true);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    th1.join();

    dolphindb::Util::sleep(2000);
    IPCMclient.unsubscribe(tableName);
    conn.upload("outputTable", {outputTable});
    ASSERT_EQ(conn.run("outputTable.size()")->getInt(), 0);
}

TEST_F(StreamingIPCinMemoryTableTest, test_unsubscribe_tableNameNullstr)
{
    std::string case_=getCaseNameHash();
    std::thread th1 = std::thread(insertTask, 10000, 0);
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;

    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP};
    int rowNum = 0, indexCapacity = 100;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);

    dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, nullptr, outputTable, false);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    th1.join();

    dolphindb::Util::sleep(2000);
    ASSERT_ANY_THROW(IPCMclient.unsubscribe(""));
    IPCMclient.unsubscribe(tableName);
}

TEST_F(StreamingIPCinMemoryTableTest, test_unsubscribe_tableNameNotExist)
{
    std::string case_=getCaseNameHash();
    std::thread th1 = std::thread(insertTask, 10000, 0);
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;

    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP};
    int rowNum = 0, indexCapacity = 100;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);

    dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, nullptr, outputTable, false);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    th1.join();

    dolphindb::Util::sleep(2000);
    ASSERT_ANY_THROW(IPCMclient.unsubscribe("errname"));
    IPCMclient.unsubscribe(tableName);
}

TEST_F(StreamingIPCinMemoryTableTest, test_subscribe_twice)
{
    std::string case_=getCaseNameHash();
    std::thread th1 = std::thread(insertTask, 10000, 0);
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;

    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP};
    int rowNum = 0, indexCapacity = 100;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);

    dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, nullptr, outputTable, false);
    ASSERT_ANY_THROW(dolphindb::ThreadSP thread1 = IPCMclient.subscribe(tableName, nullptr, outputTable, false));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    th1.join();

    dolphindb::Util::sleep(2000);
    IPCMclient.unsubscribe(tableName);
}

TEST_F(StreamingIPCinMemoryTableTest, test_unsubscribe_twice)
{
    std::string case_=getCaseNameHash();
    std::thread th1 = std::thread(insertTask, 10000, 0);
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;

    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP};
    int rowNum = 0, indexCapacity = 100;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);

    dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, nullptr, outputTable, false);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    th1.join();

    dolphindb::Util::sleep(2000);
    IPCMclient.unsubscribe(tableName);
    IPCMclient.unsubscribe(tableName);
}


TEST_F(StreamingIPCinMemoryTableTest, test_subscribe_empty_table)
{
    std::string case_=getCaseNameHash();
    std::string tableName = case_;
    dolphindb::IPCInMemoryStreamClient IPCMclient;

    std::vector<std::string> colNames = {"timestamp", "temperature", "currenttime"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_DOUBLE, dolphindb::DT_NANOTIMESTAMP};
    int rowNum = 0, indexCapacity = 100;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity);

    dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, [](dolphindb::TableSP t){}, nullptr, false);

    dolphindb::Util::sleep(2000);
    IPCMclient.unsubscribe(tableName);
}

// https://dolphindb1.atlassian.net/browse/AC-216
TEST_F(StreamingIPCinMemoryTableTest, test_subscribeUnsubscribe_with_gt1048576rows)
{
    std::string case_=getCaseNameHash();
    std::string tableName = case_;

    std::thread th_new = std::thread([=]()
                           {
        dolphindb::DBConnection conn_interval(false,false);
        conn_interval.connect(HOST, PORT, USER, PASSWD);
        std::string s = "name=`"+tableName+";dropIPCInMemoryTable(name);go;"
                "shm_SnapshortTable=createIPCInMemoryTable(1000000, name,"
                "`marketType`securityCode`origTime`tradingPhaseCode`preClosePrice`openPrice`highPrice`lowPrice`lastPrice`closePrice`bidPrice1`bidPrice2`bidPrice3`bidPrice4`bidPrice5`bidPrice6`bidPrice7`bidPrice8`bidPrice9`bidPrice10`bidVolume1`bidVolume2`bidVolume3`bidVolume4`bidVolume5`bidVolume6`bidVolume7`bidVolume8`bidVolume9`bidVolume10`offerPrice1`offerPrice2`offerPrice3`offerPrice4`offerPrice5`offerPrice6`offerPrice7`offerPrice8`offerPrice9`offerPrice10`offerVolume1`offerVolume2`offerVolume3`offerVolume4`offerVolume5`offerVolume6`offerVolume7`offerVolume8`offerVolume9`offerVolume10`numTrades`totalVolumeTrade`totalValueTrade`totalBidVolume`totalOfferVolume`weightedAvgBidPrice`weightedAvgOfferPrice`ioPV`yieldToMaturity`highLimited`lowLimited`priceEarningRatio1`priceEarningRatio2`change1`change2`channelNo`mdStreamID`instrumentStatus`preCloseIOPV`altWeightedAvgBidPrice`altWeightedAvgOfferPrice`etfBuyNumber`etfBuyAmount`etfBuyMoney`etfSellNumber`etfSellAmount`etfSellMoney`totalWarrantExecVolume`warLowerPrice`warUpperPrice`withdrawBuyNumber`withdrawBuyAmount`withdrawBuyMoney`withdrawSellNumber`withdrawSellAmount`withdrawSellMoney`totalBidNumber`totalOfferNumber`bidTradeMaxDuration`offerTradeMaxDuration`numBidOrders`bnumOfferOrders`lastTradeTime`varietyCategory`receivedTime`dailyIndex,"
                "[INT, SYMBOL, TIMESTAMP, STRING,"
                "LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG,"
                "LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG,"
                "LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG,"
                "LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG,"
                "INT, STRING, STRING,"
                "LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG,"
                "INT, INT, INT, INT, LONG, CHAR, NANOTIMESTAMP, INT]);go;"
                "for(i in 1:1100000){tableInsert(shm_SnapshortTable, "
                "rand(1 2 3 4, 1), rand(`apl`ffffffffffffffffffffffffffffffffgoog`ms`agggggggggggggma, 1), now(), rand(`s1asdgzxcaw`s2gggggggg`s333333333`s4aaaaasdzx,1), "
                "rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),"
                "rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),"
                "rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),"
                "rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),"
                "rand(100000, 1),rand(`a1zzzzzzz`a2xxxxxxxx`a3qqqqqqqqqqqqqq`a4ghhhhhhhhhhhhhhhhh, 1),rand(`b1nnnnnnnnnnnnnnnn`b2bbbbbbbbbbbbbbbb`b3hhhhhhhhhhhhhhhh`b4uuuuuuuuuuuuuuuuuuuuuuuuuuu, 1),"
                "rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),"
                "rand(100000, 1),rand(100000, 1),rand(100000, 1),rand(100000, 1),rand(10000000l, 1),rand(127c, 1), now(true),rand(100000, 1));if(i % 100000 == 0){print(string(i)+' rows inserted finished!');}}";
        conn_interval.run(s);
        std::cout<< "insert all datas finished!\n";
        conn_interval.close();
        return; });

    std::vector<std::string> colNames = {"marketType", "securityCode", "origTime", "tradingPhaseCode", "preClosePrice", "openPrice", "highPrice", "lowPrice", "lastPrice", "closePrice", "bidPrice1", "bidPrice2", "bidPrice3", "bidPrice4", "bidPrice5", "bidPrice6", "bidPrice7", "bidPrice8", "bidPrice9", "bidPrice10", "bidVolume1", "bidVolume2", "bidVolume3", "bidVolume4", "bidVolume5", "bidVolume6", "bidVolume7", "bidVolume8", "bidVolume9", "bidVolume10", "offerPrice1", "offerPrice2", "offerPrice3", "offerPrice4", "offerPrice5", "offerPrice6", "offerPrice7", "offerPrice8", "offerPrice9", "offerPrice10", "offerVolume1", "offerVolume2", "offerVolume3", "offerVolume4", "offerVolume5", "offerVolume6", "offerVolume7", "offerVolume8", "offerVolume9", "offerVolume10", "numTrades", "totalVolumeTrade", "totalValueTrade", "totalBidVolume", "totalOfferVolume", "weightedAvgBidPrice", "weightedAvgOfferPrice", "ioPV", "yieldToMaturity", "highLimited", "lowLimited", "priceEarningRatio1", "priceEarningRatio2", "change1", "change2", "channelNo", "mdStreamID", "instrumentStatus", "preCloseIOPV", "altWeightedAvgBidPrice", "altWeightedAvgOfferPrice", "etfBuyNumber", "etfBuyAmount", "etfBuyMoney", "etfSellNumber", "etfSellAmount", "etfSellMoney", "totalWarrantExecVolume", "warLowerPrice", "warUpperPrice", "withdrawBuyNumber", "withdrawBuyAmount", "withdrawBuyMoney", "withdrawSellNumber", "withdrawSellAmount", "withdrawSellMoney", "totalBidNumber", "totalOfferNumber", "bidTradeMaxDuration", "offerTradeMaxDuration", "numBidOrders", "bnumOfferOrders", "lastTradeTime", "varietyCategory", "receivedTime", "dailyIndex"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_TIMESTAMP, dolphindb::DT_STRING, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_INT, dolphindb::DT_STRING, dolphindb::DT_STRING, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_INT, dolphindb::DT_INT, dolphindb::DT_INT, dolphindb::DT_INT, dolphindb::DT_LONG, dolphindb::DT_CHAR, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_INT};
    int rowNum = 0, indexCapacity = 10000;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity); // 创建一个和共享内存表结构相同的表
    std::cout << "outputTable created successfully\n";
    auto handler = [=](dolphindb::TableSP tb)
    {
        // auto row = tb->rows();
        // std::cout << "[handler]: now get rows: " << row << '\n';
        // std::cout<<"handler table: \n"<<tb->getString()<<'\n';
    };
    int preRow = 0;

    std::thread th2 = std::thread([&]()
                        {
        dolphindb::Util::sleep(1000);
        for(auto i=0;i<50;i++){
            dolphindb::IPCInMemoryStreamClient IPCMclient;

            dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, handler, outputTable, false);
            dolphindb::Util::sleep(500);

            IPCMclient.unsubscribe(tableName);
            int curRow = outputTable->rows();
            ASSERT_TRUE(curRow >= preRow);
            preRow = curRow;
            dolphindb::Util::sleep(2000);
        } });

    th_new.join();
    th2.join();
}

// https://dolphindb1.atlassian.net/browse/AC-216
TEST_F(StreamingIPCinMemoryTableTest, test_subscribeUnsubscribe_overwriteTrue_with_gt1048576rows)
{
    std::string case_=getCaseNameHash();
    std::string tableName = case_;

    std::thread th_new = std::thread([=]()
                           {
        dolphindb::DBConnection conn_interval(false,false);
        conn_interval.connect(HOST, PORT, USER, PASSWD);
        std::string s = "name=`"+tableName+"\n"
                "dropIPCInMemoryTable(name);go;"
                "shm_SnapshortTable=createIPCInMemoryTable(1000000, name,"
                "`marketType`securityCode`origTime`tradingPhaseCode`preClosePrice`openPrice`highPrice`lowPrice`lastPrice`closePrice`bidPrice1`bidPrice2`bidPrice3`bidPrice4`bidPrice5`bidPrice6`bidPrice7`bidPrice8`bidPrice9`bidPrice10`bidVolume1`bidVolume2`bidVolume3`bidVolume4`bidVolume5`bidVolume6`bidVolume7`bidVolume8`bidVolume9`bidVolume10`offerPrice1`offerPrice2`offerPrice3`offerPrice4`offerPrice5`offerPrice6`offerPrice7`offerPrice8`offerPrice9`offerPrice10`offerVolume1`offerVolume2`offerVolume3`offerVolume4`offerVolume5`offerVolume6`offerVolume7`offerVolume8`offerVolume9`offerVolume10`numTrades`totalVolumeTrade`totalValueTrade`totalBidVolume`totalOfferVolume`weightedAvgBidPrice`weightedAvgOfferPrice`ioPV`yieldToMaturity`highLimited`lowLimited`priceEarningRatio1`priceEarningRatio2`change1`change2`channelNo`mdStreamID`instrumentStatus`preCloseIOPV`altWeightedAvgBidPrice`altWeightedAvgOfferPrice`etfBuyNumber`etfBuyAmount`etfBuyMoney`etfSellNumber`etfSellAmount`etfSellMoney`totalWarrantExecVolume`warLowerPrice`warUpperPrice`withdrawBuyNumber`withdrawBuyAmount`withdrawBuyMoney`withdrawSellNumber`withdrawSellAmount`withdrawSellMoney`totalBidNumber`totalOfferNumber`bidTradeMaxDuration`offerTradeMaxDuration`numBidOrders`bnumOfferOrders`lastTradeTime`varietyCategory`receivedTime`dailyIndex,"
                "[INT, SYMBOL, TIMESTAMP, STRING,"
                "LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG,"
                "LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG,"
                "LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG,"
                "LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG,"
                "INT, STRING, STRING,"
                "LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG, LONG,"
                "INT, INT, INT, INT, LONG, CHAR, NANOTIMESTAMP, INT]);go;"
                "for(i in 1:1100000){tableInsert(shm_SnapshortTable, "
                "rand(1 2 3 4, 1), rand(`apl`ffffffffffffffffffffffffffffffffgoog`ms`agggggggggggggma, 1), now(), rand(`s1asdgzxcaw`s2gggggggg`s333333333`s4aaaaasdzx,1), "
                "rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),"
                "rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),"
                "rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),"
                "rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),"
                "rand(100000, 1),rand(`a1zzzzzzz`a2xxxxxxxx`a3qqqqqqqqqqqqqq`a4ghhhhhhhhhhhhhhhhh, 1),rand(`b1nnnnnnnnnnnnnnnn`b2bbbbbbbbbbbbbbbb`b3hhhhhhhhhhhhhhhh`b4uuuuuuuuuuuuuuuuuuuuuuuuuuu, 1),"
                "rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),rand(1000000l,1),"
                "rand(100000, 1),rand(100000, 1),rand(100000, 1),rand(100000, 1),rand(10000000l, 1),rand(127c, 1), now(true),rand(100000, 1));if(i % 100000 == 0){print(string(i)+' rows inserted finished!');}}";
        conn_interval.run(s);
        conn_interval.close();
        return; });

    std::vector<std::string> colNames = {"marketType", "securityCode", "origTime", "tradingPhaseCode", "preClosePrice", "openPrice", "highPrice", "lowPrice", "lastPrice", "closePrice", "bidPrice1", "bidPrice2", "bidPrice3", "bidPrice4", "bidPrice5", "bidPrice6", "bidPrice7", "bidPrice8", "bidPrice9", "bidPrice10", "bidVolume1", "bidVolume2", "bidVolume3", "bidVolume4", "bidVolume5", "bidVolume6", "bidVolume7", "bidVolume8", "bidVolume9", "bidVolume10", "offerPrice1", "offerPrice2", "offerPrice3", "offerPrice4", "offerPrice5", "offerPrice6", "offerPrice7", "offerPrice8", "offerPrice9", "offerPrice10", "offerVolume1", "offerVolume2", "offerVolume3", "offerVolume4", "offerVolume5", "offerVolume6", "offerVolume7", "offerVolume8", "offerVolume9", "offerVolume10", "numTrades", "totalVolumeTrade", "totalValueTrade", "totalBidVolume", "totalOfferVolume", "weightedAvgBidPrice", "weightedAvgOfferPrice", "ioPV", "yieldToMaturity", "highLimited", "lowLimited", "priceEarningRatio1", "priceEarningRatio2", "change1", "change2", "channelNo", "mdStreamID", "instrumentStatus", "preCloseIOPV", "altWeightedAvgBidPrice", "altWeightedAvgOfferPrice", "etfBuyNumber", "etfBuyAmount", "etfBuyMoney", "etfSellNumber", "etfSellAmount", "etfSellMoney", "totalWarrantExecVolume", "warLowerPrice", "warUpperPrice", "withdrawBuyNumber", "withdrawBuyAmount", "withdrawBuyMoney", "withdrawSellNumber", "withdrawSellAmount", "withdrawSellMoney", "totalBidNumber", "totalOfferNumber", "bidTradeMaxDuration", "offerTradeMaxDuration", "numBidOrders", "bnumOfferOrders", "lastTradeTime", "varietyCategory", "receivedTime", "dailyIndex"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_INT, dolphindb::DT_SYMBOL, dolphindb::DT_TIMESTAMP, dolphindb::DT_STRING, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_INT, dolphindb::DT_STRING, dolphindb::DT_STRING, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_LONG, dolphindb::DT_INT, dolphindb::DT_INT, dolphindb::DT_INT, dolphindb::DT_INT, dolphindb::DT_LONG, dolphindb::DT_CHAR, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_INT};
    int rowNum = 0, indexCapacity = 10000;
    dolphindb::TableSP outputTable = dolphindb::Util::createTable(colNames, colTypes, rowNum, indexCapacity); // 创建一个和共享内存表结构相同的表
    std::cout << "outputTable created successfully\n";
    auto handler = [=](dolphindb::TableSP tb)
    {
        // auto row = tb->rows();
        // std::cout << "[handler]: now get rows: " << row << '\n';
    };

    std::thread th2 = std::thread([&]()
                        {
        dolphindb::Util::sleep(1000);
        for(auto i=0;i<50;i++){
            dolphindb::IPCInMemoryStreamClient IPCMclient;

            dolphindb::ThreadSP thread0 = IPCMclient.subscribe(tableName, handler, outputTable, true);
            dolphindb::Util::sleep(500);

            IPCMclient.unsubscribe(tableName);
            ASSERT_TRUE(outputTable->rows() == 1 | outputTable->rows() == 0 );
            dolphindb::Util::sleep(2000);
        } });

    th_new.join();
    th2.join();
}
#endif // if __linux__
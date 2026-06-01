#ifdef __linux__

#include <gtest/gtest.h>
#include "config.h"
#include "Streaming.h"


namespace UDPCT
{

class StreamingUDPThreadedClientTester : public testing::Test
{
    public:
        static dolphindb::StreamingClientConfig scfg;
        static dolphindb::DBConnectionSP connSp;
        // Suite
        static void SetUpTestSuite()
        {
            bool ret = connSp->connect(HOST, PORT, USER, PASSWD);
            if (!ret)
            {
                std::cout << "Failed to connect to the server" << std::endl;
            }
            else
            {
                std::cout << "connect to " + HOST + ":" + std::to_string(PORT) << std::endl;
            }
            scfg.protocol = dolphindb::TransportationProtocol::UDP;
        }
        static void TearDownTestSuite()
        {
            connSp->close();
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

dolphindb::DBConnectionSP StreamingUDPThreadedClientTester::connSp(new dolphindb::DBConnection);
dolphindb::StreamingClientConfig StreamingUDPThreadedClientTester::scfg;

static void createSharedTableAndReplay(const std::string &st, int rows)
{
    std::string script = "\
            share streamTable(100:0, `datetimev`timestampv`sym`price1`price2,[DATETIME,TIMESTAMP,SYMBOL,DOUBLE,DOUBLE]) as "+st+";\n\
            go;\n\
            setStreamTableFilterColumn(" +
                    st + ", `sym)";
    StreamingUDPThreadedClientTester::connSp->run(script);

    std::string replayScript = "n = " + std::to_string(rows) + ";table1_UDPCT = table(1:0, `datetimev`timestampv`sym`price1`price2, [DATETIME, TIMESTAMP, SYMBOL, DOUBLE, DOUBLE]);\
            share table(1:0, `datetimev`timestampv`sym`price1`price2, [DATETIME, TIMESTAMP, SYMBOL, DOUBLE, DOUBLE]) as "+st+"_res_UDPCT;go;\
            tableInsert(table1_UDPCT, 2012.01.01T01:21:23 + 1..n, 2018.12.01T01:21:23.000 + 1..n, take(`a`b`c,n), rand(100,n)+rand(1.0, n), rand(100,n)+rand(1.0, n));share table1_UDPCT as "+st+"_ex_UDPCT;\
            replay(inputTables=table1_UDPCT, outputTables=`" + st + ", dateColumn=`timestampv, timeColumn=`timestampv)";
    StreamingUDPThreadedClientTester::connSp->run(replayScript);
}

static void createSharedTableAndReplay_withAllDataType(const std::string &st)
{
    int scale32 = rand() % 9;
    int scale64 = rand() % 18;
    int scale128 = rand() % 38;
    printf("#[PARAM] scale32: %d, scale64: %d, scale128: %d\n", scale32, scale64, scale128);
    std::string replayScript =
        "colName =  `ind`cbool`cchar`cshort`cint`clong`cdate`cmonth`ctime`cminute`csecond`cdatetime`ctimestamp`cnanotime`cnanotimestamp`cfloat`cdouble`csymbol`cstring`cblob`cipaddr`cuuid`cint128`cdecimal32`cdecimal64`cdecimal128;"
        "colType = [INT, BOOL, CHAR, SHORT, INT,LONG, DATE, MONTH, TIME, MINUTE, SECOND, DATETIME, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, SYMBOL, STRING, BLOB, IPADDR, UUID, INT128, DECIMAL32(" +
        std::to_string(scale32) + "), DECIMAL64(" + std::to_string(scale64) + "), DECIMAL128(" + std::to_string(scale128) + ")];"
        "share streamTable(100:0,colName, colType) as "+st+";go;"
                "setStreamTableFilterColumn(" +
        st + ", `csymbol);go;"
                "table1_UDPCT = table(1:0, colName, colType);"
                "row_num = 1000;"
                "share table(1:0, colName, colType) as "+st+"_res_UDPCT;go;"
                "col1 = rand(2 ,row_num);col2 = rand(256 ,row_num);col3 = rand(-row_num..row_num ,row_num);col4 = rand(-row_num..row_num ,row_num);col5 = rand(-row_num..row_num ,row_num);col6 = rand(0..row_num ,row_num);col7 = rand(0..row_num ,row_num);"
                "col8 = rand(0..row_num ,row_num);col9 = rand(0..row_num ,row_num);col10 = rand(0..row_num ,row_num);col11 = rand(0..row_num ,row_num);col12 = rand(0..row_num ,row_num);col13 = rand(0..row_num ,row_num);col14= rand(0..row_num ,row_num);"
                "col15 = rand(round(row_num,2) ,row_num);col16 = rand(round(row_num,2) ,row_num);col17 =rand(`a`s`sym`d`zdg`f`y`ddvb,row_num);col18 =rand(`a`s`sym`d`zdg`f`y`ddvb,row_num);col19 =rand(`a`s`sym`d`zdg`f`y`ddvb,row_num);col20 = take(ipaddr(\"192.168.1.13\"),row_num);"
                "col21 = take(uuid(\"5d212a78-cc48-e3b1-4235-b4d91473ee87\"),row_num);col22 = take(int128(\"e1671797c52e15f763380b45e841ec32\"),row_num);col23=rand((-1000..1000)/1000.0000$DECIMAL32(" +
        std::to_string(scale32) + "),row_num);col24=rand((-1000..1000)/1000.0000$DECIMAL64(" + std::to_string(scale64) + "),row_num);col25=rand((-1000..1000)/1000.0000$DECIMAL128(" + std::to_string(scale128) + "),row_num);go;"
                "for (i in 0..(row_num-1)){tableInsert(table1_UDPCT,i,col1[i],col2[i],col3[i],col4[i],col5[i],col6[i],col7[i],col8[i],col9[i],col10[i],col11[i],col12[i],col13[i],col14[i],col15[i],col16[i],col17[i],col18[i],col19[i],col20[i],col21[i],col22[i],col23[i],col24[i],col25[i])};"
                "share table1_UDPCT as "+st+"_ex_UDPCT;go;"
                "replay(inputTables=table1_UDPCT, outputTables=`" +
        st + ", dateColumn=`ctimestamp, timeColumn=`ctimestamp);";
    StreamingUDPThreadedClientTester::connSp->run(replayScript);
}

static void createSharedTableAndReplay_withArrayVector(const std::string &st)
{
    std::string replayScript =
        "colName = `ts`cbool`cchar`cshort`cint`clong`cdate`cmonth`ctime`cminute`csecond`cdatetime`ctimestamp`cnanotime`cnanotimestamp`cdatehour`cfloat`cdouble`cipaddr`cuuid`cint128`cdecimal32`cdecimal64`cdecimal128;"
        "colType = [TIMESTAMP, BOOL[], CHAR[], SHORT[], INT[],LONG[], DATE[], MONTH[], TIME[], MINUTE[], SECOND[], DATETIME[], TIMESTAMP[], NANOTIME[], NANOTIMESTAMP[], DATEHOUR[], FLOAT[], DOUBLE[], IPADDR[], UUID[], INT128[],DECIMAL32(6)[],DECIMAL64(16)[],DECIMAL128(26)[]];"
        "share streamTable(1:0,colName, colType) as "+st+";go;"
        "share table(1:0, colName, colType) as "+st+"_res_UDPCT;go;"
                "row_num = 1000;"
                "ind = [1,2,3,4,row_num];"
                "ts=now()..(now()+row_num-1);"
                "cbool= arrayVector(ind, bool(take(0 1 ,row_num)));cchar = arrayVector(ind, char(take(256 ,row_num)));cshort = arrayVector(ind, short(take(-10000..10000 ,row_num)));cint = arrayVector(ind, int(take(-10000..10000 ,row_num)));"
                "clong = arrayVector(ind, long(take(-10000..10000 ,row_num)));cdate = arrayVector(ind, date(take(10000 ,row_num)));cmonth = arrayVector(ind, month(take(23640..25000 ,row_num)));ctime = arrayVector(ind, time(take(10000 ,row_num)));"
                "cminute = arrayVector(ind, minute(take(100 ,row_num)));csecond = arrayVector(ind, second(take(100 ,row_num)));cdatetime = arrayVector(ind, datetime(take(10000 ,row_num)));ctimestamp = arrayVector(ind, timestamp(take(10000 ,row_num)));"
                "cnanotime = arrayVector(ind, nanotime(take(10000 ,row_num)));cnanotimestamp = arrayVector(ind, nanotimestamp(take(10000 ,row_num)));cdatehour = arrayVector(ind, datehour(take(10000 ,row_num)));cfloat = arrayVector(ind, float(rand(10000.0000,row_num)));cdouble = arrayVector(ind, rand(10000.0000, row_num));"
                "cipaddr = arrayVector(1..row_num, take(ipaddr(['192.168.1.13']),row_num));"
                "cuuid = arrayVector(1..row_num, take(uuid(['5d212a78-cc48-e3b1-4235-b4d91473ee87']),row_num));"
                "cint128 = arrayVector(1..row_num, take(int128(['e1671797c52e15f763380b45e841ec32']),row_num));"
                "cdecimal32 = array(DECIMAL32(6)[], 0, 10).append!(decimal32([1..50, [], rand(100.000000, 10), rand(1..100, 10), take(00i, 3)], 6));"
                "cdecimal64 = array(DECIMAL64(16)[], 0, 10).append!(decimal64([1..50, [], rand(100.000000, 10), rand(1..100, 10), take(00i, 3)], 16));"
                "cdecimal128 = array(DECIMAL128(26)[], 0, 10).append!(decimal128([1..50, [], rand(100.000000, 10), rand(1..100, 10), take(00i, 3)], 26));"
                "for(i in 1..(row_num-5)){"
                "    cbool.append!([bool(take(0 1 ,50))]);"
                "    cchar.append!([char(take(256 ,50))]);cshort.append!([short(take(-10000..10000 ,50))]);cint.append!([int(take(-10000..10000 ,50))]);"
                "    clong.append!([long(take(-10000..10000 ,50))]);cdate.append!([date(take(10000 ,50))]);cmonth.append!([month(take(23640..25000 ,50))]);"
                "    ctime.append!([time(take(10000 ,50))]);cminute.append!([minute(take(100 ,50))]);csecond.append!([second(take(100 ,50))]);"
                "    cdatetime.append!([datetime(take(10000 ,50))]);ctimestamp.append!([timestamp(take(10000 ,50))]);"
                "    cnanotime.append!([nanotime(take(10000 ,50))]);cnanotimestamp.append!([nanotimestamp(take(10000 ,50))]);"
                "    cdatehour.append!([datehour(take(10000 ,50))]);"
                "    cfloat.append!([float(rand(10000.0000,50))]);cdouble.append!([rand(10000.0000, 50)]);"
                "    cdecimal32.append!([decimal32([`1.123123123123123123123123123] ,6)]);"
                "    cdecimal64.append!([decimal64([`1.123123123123123123123123123] ,16)]);"
                "    cdecimal128.append!([decimal128([`1.123123123123123123123123123] ,26)])};go;"
                "table1_UDPCT=table(ts,cbool,cchar,cshort,cint,clong,cdate,cmonth,ctime,cminute,csecond,cdatetime,ctimestamp,cnanotime,cnanotimestamp,cdatehour,cfloat,cdouble,cipaddr,cuuid,cint128,cdecimal32,cdecimal64,cdecimal128);"
                "share table1_UDPCT as "+st+"_ex_UDPCT;go;"
                "replay(inputTables=table1_UDPCT, outputTables=`" +
        st + ", dateColumn=`ts, timeColumn=`ts);";
    StreamingUDPThreadedClientTester::connSp->run(replayScript);
}

dolphindb::TableSP UDPmsgToTable(dolphindb::VectorSP tupleVal)
{
    std::vector<std::string> colNames;
    std::vector<dolphindb::ConstantSP> columnVecs;
    auto col_count = tupleVal->size();
    for(auto i=0;i<col_count;i++){
        colNames.emplace_back("col"+ std::to_string(i));
    }

    columnVecs.reserve(col_count);
    for (auto i=0;i<col_count;i++)
    {
        dolphindb::DATA_FORM form = tupleVal->get(i)->getForm();
        dolphindb::DATA_TYPE _t = tupleVal->get(i)->getType();

        dolphindb::DATA_TYPE type = form == dolphindb::DF_VECTOR && _t < dolphindb::ARRAY_TYPE_BASE ? static_cast<dolphindb::DATA_TYPE>(_t+dolphindb::ARRAY_TYPE_BASE):_t;
        int extraParam = tupleVal->get(i)->getExtraParamForType();

        dolphindb::VectorSP col;
        if (form == dolphindb::DF_VECTOR){
            col = dolphindb::Util::createArrayVector(type, 0, 0, true, extraParam);
        }else{
            col = dolphindb::Util::createVector(type, 0, 0, true, extraParam);
        }
        col->append(tupleVal->get(i));
        columnVecs.emplace_back(col);
    }

    return dolphindb::Util::createTable(colNames, columnVecs);
}

TEST_F(StreamingUDPThreadedClientTester, tableNotExist_should_serial)
{
    dolphindb::ThreadedClient client(scfg);
    ASSERT_ANY_THROW(client.subscribe(HOST, PORT, [](dolphindb::Message msg){}, "ajwdhwjkae", DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD));
}

TEST_F(StreamingUDPThreadedClientTester, subscribe_host_error_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 1000);
    dolphindb::ThreadedClient client(scfg);
    ASSERT_ANY_THROW(client.subscribe("abcd", PORT, [](dolphindb::Message msg){}, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD));
}

TEST_F(StreamingUDPThreadedClientTester, subscribe_port_error_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 1000);
    dolphindb::ThreadedClient client(scfg);

    ASSERT_ANY_THROW(client.subscribe(HOST, -1, [](dolphindb::Message msg){},  st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD));
}

TEST_F(StreamingUDPThreadedClientTester, subscribe_username_error_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 1000);
    dolphindb::ThreadedClient client(scfg);

    ASSERT_ANY_THROW(client.subscribe(HOST, PORT, [](dolphindb::Message msg){}, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, "sjkdwae", "123456"));
}

TEST_F(StreamingUDPThreadedClientTester, subscribe_usernameNull_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 1000);
    dolphindb::ThreadedClient client(scfg);
    bool enableClientAuth = connSp->run("bool(getConfig('enableClientAuth'))")->getBool();
    if (enableClientAuth){
        ASSERT_ANY_THROW(client.subscribe(HOST, PORT, [](dolphindb::Message msg){}, st, DEFAULT_ACTION_NAME, 0, false));
    }else{
        auto thread = client.subscribe(HOST, PORT, [](dolphindb::Message msg){}, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD);
        dolphindb::Util::sleep(1000);
        ASSERT_FALSE(connSp->run("(exec count(*) from getStreamingStat()[`udpPubTables] where tableName =`"+st+") ==0")->getBool());
        client.unsubscribe(HOST, PORT, st, DEFAULT_ACTION_NAME);
        dolphindb::Util::sleep(2000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat()[`udpPubTables] where tableName =`"+st+") ==0")->getBool());
    }
}

TEST_F(StreamingUDPThreadedClientTester, subscribe_password_error_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 1000);
    dolphindb::ThreadedClient client(scfg);

    ASSERT_ANY_THROW(client.subscribe(HOST, PORT, [](dolphindb::Message msg){}, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, "111111"));
}

TEST_F(StreamingUDPThreadedClientTester, subscribe_offset_negative_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 100);
    dolphindb::ConstantSP extra_data = connSp->run("select top 10 * from " + st);
    dolphindb::ThreadedClient client(scfg);
    dolphindb::Signal notify;
    dolphindb::Mutex mtx;
    std::string res_tab = case_+"_res_UDPCT";
    int msg_total = 0;
    dolphindb::AutoFitTableAppender appender("", res_tab, *connSp);

    auto handler = [&](dolphindb::Message msg){
        dolphindb::LockGuard<dolphindb::Mutex> lock(&mtx);
        msg_total += msg->get(0)->size();
        dolphindb::TableSP tmp = UDPmsgToTable(msg);
        int rows = appender.append(tmp);
        // ASSERT_EQ(rows, msg->get(0)->size());
        // ASSERT_EQ(msg.getOffset(), 100);

        if (msg_total == 10)
        {
            notify.set();
        }
    };

    auto thread = client.subscribe(HOST, PORT, handler, st, DEFAULT_ACTION_NAME, -1, true, nullptr, false, false, USER, PASSWD);
    dolphindb::Util::sleep(1000);
    std::vector<dolphindb::ConstantSP> args = {extra_data};
    connSp->run("append!{" + st + "}", args);
    std::cout << "insert extra data finished!\n";

    notify.wait();
    client.unsubscribe(HOST, PORT, st, DEFAULT_ACTION_NAME);
    dolphindb::Util::sleep(2000);
    connSp->upload("extraData", args[0]);
    ASSERT_TRUE(connSp->run("ex = exec * from extraData order by timestampv;"
                        "res = exec * from "+st+"_res_UDPCT order by timestampv;"
                        "all(each(eqObj, ex.values(), res.values()))")->getBool());
    ASSERT_FALSE(connSp->run("`"+st+" in (exec tableName from getStreamingStat().udpPubTables)")->getBool());

}

TEST_F(StreamingUDPThreadedClientTester, subscribe_offset_gt0_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 100);
    dolphindb::ThreadedClient client(scfg);
    dolphindb::Signal notify;
    dolphindb::Mutex mtx;
    std::string res_tab = case_+"_res_UDPCT";
    int msg_total = 0;
    dolphindb::AutoFitTableAppender appender("", res_tab, *connSp);

    auto handler = [&](dolphindb::Message msg){
        dolphindb::LockGuard<dolphindb::Mutex> lock(&mtx);
        // std::cout << msg->getString() << std::endl;
        msg_total += msg->get(0)->size();
        dolphindb::TableSP tmp = UDPmsgToTable(msg);
        int rows = appender.append(tmp);
        ASSERT_EQ(rows, msg->get(0)->size());

        if (msg_total == 100-10)
        {
            notify.set();
        }
    };

    auto thread = client.subscribe(HOST, PORT, handler, st, DEFAULT_ACTION_NAME, 10, true, nullptr, false, false, USER, PASSWD);
    notify.wait();
    client.unsubscribe(HOST, PORT, st, DEFAULT_ACTION_NAME);
    dolphindb::Util::sleep(2000);
    ASSERT_TRUE(connSp->run("ex = (exec * from "+st+"_ex_UDPCT order by datetimev)[10:];"
                        "res = exec * from "+st+"_res_UDPCT order by timestampv;"
                        "all(each(eqObj, ex.values(), res.values()))")->getBool());
    ASSERT_FALSE(connSp->run("`"+st+" in (exec tableName from getStreamingStat().udpPubTables)")->getBool());
}

TEST_F(StreamingUDPThreadedClientTester, subscribe_filter_should_serial)
{
    GTEST_SKIP() << "filter not support yet.";
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 100);
    dolphindb::ThreadedClient client(scfg);
    dolphindb::Signal notify;
    dolphindb::Mutex mtx;
    std::string res_tab = case_+"_res_UDPCT";
    int msg_total = 0;
    dolphindb::AutoFitTableAppender appender("", res_tab, *connSp);

    auto handler = [&](dolphindb::Message msg){
        dolphindb::LockGuard<dolphindb::Mutex> lock(&mtx);
        // std::cout << msg->getString() << std::endl;
        msg_total += msg->get(0)->size();
        dolphindb::TableSP tmp = UDPmsgToTable(msg);
        int rows = appender.append(tmp);
        ASSERT_EQ(rows, msg->get(0)->size());

        if (msg_total == 34)
        {
            notify.set();
        }
    };

    dolphindb::VectorSP filters = dolphindb::Util::createVector(dolphindb::DT_STRING, 1);
    filters->setString(0, "a");
    // std::cout << "filters: " << filters->getString() << std::endl;
    auto thread = client.subscribe(HOST, PORT, handler, st, DEFAULT_ACTION_NAME, 0, true, filters, false, false, USER, PASSWD);
    notify.wait();
    client.unsubscribe(HOST, PORT, st, DEFAULT_ACTION_NAME);
    dolphindb::Util::sleep(2000);
    ASSERT_TRUE(connSp->run("ex = exec * from "+st+"_ex_UDPCT where sym = `a order by timestampv;"
                        "res = exec * from "+st+"_res_UDPCT order by timestampv;"
                        "all(each(eqObj, ex.values(), res.values()))")->getBool());
    ASSERT_FALSE(connSp->run("`"+st+" in (exec tableName from getStreamingStat().udpPubTables)")->getBool());
}


TEST_F(StreamingUDPThreadedClientTester, subscribe_twice_with_same_action_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 2000);
    dolphindb::ThreadedClient client(scfg);
    dolphindb::Signal notify;
    dolphindb::Mutex mtx;
    std::string res_tab = case_+"_res_UDPCT";
    int msg_total = 0;
    dolphindb::AutoFitTableAppender appender("", res_tab, *connSp);

    auto handler = [&](dolphindb::Message msg){
        dolphindb::LockGuard<dolphindb::Mutex> lock(&mtx);
        // std::cout << msg->getString() << std::endl;
        msg_total += msg->get(0)->size();
        dolphindb::TableSP tmp = UDPmsgToTable(msg);
        int rows = appender.append(tmp);
        ASSERT_EQ(rows, msg->get(0)->size());

        if (msg_total == 2000)
        {
            notify.set();
        }
    };

    auto thread = client.subscribe(HOST, PORT, handler, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD);
    ASSERT_ANY_THROW(client.subscribe(HOST, PORT, handler, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD));
    notify.wait();
    client.unsubscribe(HOST, PORT, st, DEFAULT_ACTION_NAME);
    dolphindb::Util::sleep(2000);
    ASSERT_TRUE(connSp->run("ex = exec * from "+st+"_ex_UDPCT order by timestampv;"
                        "res = exec * from "+st+"_res_UDPCT order by timestampv;"
                        "all(each(eqObj, ex.values(), res.values()))")->getBool());
    ASSERT_FALSE(connSp->run("`"+st+" in (exec tableName from getStreamingStat().udpPubTables)")->getBool());
}


TEST_F(StreamingUDPThreadedClientTester, unsubscribe_twice_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 2000);
    dolphindb::ThreadedClient client(scfg);
    dolphindb::Signal notify;
    dolphindb::Mutex mtx;
    std::string res_tab = case_+"_res_UDPCT";
    int msg_total = 0;
    dolphindb::AutoFitTableAppender appender("", res_tab, *connSp);

    auto handler = [&](dolphindb::Message msg){
        dolphindb::LockGuard<dolphindb::Mutex> lock(&mtx);
        // std::cout << msg->getString() << std::endl;
        msg_total += msg->get(0)->size();
        dolphindb::TableSP tmp = UDPmsgToTable(msg);
        int rows = appender.append(tmp);
        ASSERT_EQ(rows, msg->get(0)->size());

        if (msg_total == 2000)
        {
            notify.set();
        }
    };

    auto thread = client.subscribe(HOST, PORT, handler, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD);
    notify.wait();
    client.unsubscribe(HOST, PORT, st, DEFAULT_ACTION_NAME);
    ASSERT_FALSE(client.unsubscribe(HOST, PORT, st, DEFAULT_ACTION_NAME));
    dolphindb::Util::sleep(2000);
    ASSERT_TRUE(connSp->run("ex = exec * from "+st+"_ex_UDPCT order by timestampv;"
                        "res = exec * from "+st+"_res_UDPCT order by timestampv;"
                        "all(each(eqObj, ex.values(), res.values()))")->getBool());
    ASSERT_FALSE(connSp->run("`"+st+" in (exec tableName from getStreamingStat().udpPubTables)")->getBool());
}


TEST_F(StreamingUDPThreadedClientTester, not_unsubscribe_after_subscribe_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 2000);
    dolphindb::ThreadedClient client(scfg);
    dolphindb::Signal notify;
    dolphindb::Mutex mtx;
    std::string res_tab = case_+"_res_UDPCT";
    int msg_total = 0;
    dolphindb::AutoFitTableAppender appender("", res_tab, *connSp);

    auto handler = [&](dolphindb::Message msg){
        dolphindb::LockGuard<dolphindb::Mutex> lock(&mtx);
        // std::cout << msg->getString() << std::endl;
        msg_total += msg->get(0)->size();
        dolphindb::TableSP tmp = UDPmsgToTable(msg);
        int rows = appender.append(tmp);
        ASSERT_EQ(rows, msg->get(0)->size());

        if (msg_total == 2000)
        {
            notify.set();
        }
    };

    auto thread = client.subscribe(HOST, PORT, handler, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD);
    notify.wait();
    ASSERT_TRUE(connSp->run("ex = exec * from "+st+"_ex_UDPCT order by timestampv;"
                        "res = exec * from "+st+"_res_UDPCT order by timestampv;"
                        "all(each(eqObj, ex.values(), res.values()))")->getBool());
    ASSERT_TRUE(connSp->run("`"+st+" in (exec tableName from getStreamingStat().udpPubTables)")->getBool());
    connSp->run("\
        info = exec * from getStreamingStat().udpPubTables where tableName=`"+st+";\
        ip1 = (exec channel.split(':')[0] from info)[0];\
        port1 = (exec channel.split(':')[1] from info)[0];\
        stopPublishTable(ip1, port1, `"+st+",`"+DEFAULT_ACTION_NAME+",,true)");
    dolphindb::Util::sleep(1000);
    ASSERT_FALSE(connSp->run("`"+st+" in (exec tableName from getStreamingStat().udpPubTables)")->getBool());
    ASSERT_ANY_THROW(client.unsubscribe(HOST, PORT, st, DEFAULT_ACTION_NAME));
}

TEST_F(StreamingUDPThreadedClientTester, subscribe_actionName_null_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 2000);
    dolphindb::ThreadedClient client(scfg);
    dolphindb::Signal notify;
    dolphindb::Mutex mtx;
    std::string res_tab = case_+"_res_UDPCT";
    int msg_total = 0;
    dolphindb::AutoFitTableAppender appender("", res_tab, *connSp);

    auto handler = [&](dolphindb::Message msg){
        dolphindb::LockGuard<dolphindb::Mutex> lock(&mtx);
        // std::cout << msg->getString() << std::endl;
        msg_total += msg->get(0)->size();
        dolphindb::TableSP tmp = UDPmsgToTable(msg);
        int rows = appender.append(tmp);
        ASSERT_EQ(rows, msg->get(0)->size());

        if (msg_total == 2000)
        {
            notify.set();
        }
    };

    auto thread = client.subscribe(HOST, PORT, handler, st, "", 0, true, nullptr, false, false, USER, PASSWD);
    notify.wait();
    client.unsubscribe(HOST, PORT, st, "");
    dolphindb::Util::sleep(2000);
    ASSERT_TRUE(connSp->run("ex = exec * from "+st+"_ex_UDPCT order by timestampv;"
                        "res = exec * from "+st+"_res_UDPCT order by timestampv;"
                        "all(each(eqObj, ex.values(), res.values()))")->getBool());
    ASSERT_FALSE(connSp->run("`"+st+" in (exec tableName from getStreamingStat().udpPubTables)")->getBool());
}


TEST_F(StreamingUDPThreadedClientTester, subscribe_multithread_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay(st, 2000);
    int num_clients = 10;

    std::vector<dolphindb::ThreadedClient> clients(num_clients);
    for (int i = 0; i < 10; i++)
    {
        clients[i] = dolphindb::ThreadedClient(scfg);
    }

    std::vector<std::string> res_tabs = {"res_UDPCT0", "res_UDPCT1", "res_UDPCT2", "res_UDPCT3", "res_UDPCT4", "res_UDPCT5", "res_UDPCT6", "res_UDPCT7", "res_UDPCT8", "res_UDPCT9"};
    for (int i = 0; i < num_clients; i++)
    {
        std::string res_tab = st + "_" + res_tabs[i];
        auto handler = [=](dolphindb::Message msg){
            dolphindb::AutoFitTableAppender appender("", res_tab, *connSp);
            dolphindb::TableSP tmp = UDPmsgToTable(msg);
            int rows = appender.append(tmp);
            ASSERT_EQ(rows, msg->get(0)->size());
        };
        std::string action = "action"+std::to_string(i);
        connSp->run("t= select * from "+st+"_res_UDPCT;share t as " + st + "_" + res_tabs[i]);
        clients[i].subscribe(HOST, PORT, handler, st, action, 0, true, nullptr, false, false, USER, PASSWD);
    }

    std::function<void()> wait_all = [&]() {
        while (true)
        {
            connSp->run("st = array(ANY)");
            for (auto i = 0; i < res_tabs.size(); i++){
                connSp->run("st.append!("+st+"_"+res_tabs[i]+")");
            }
            if (connSp->run("all(each(rows, st) == 2000)")->getBool())
            {
                break;
            }
            dolphindb::Util::sleep(1000);
        }
    };
    wait_all();
    // notify.wait();
    for (int i = 0; i < num_clients; i++)
    {
        clients[i].unsubscribe(HOST, PORT, st, "action"+std::to_string(i));
        ASSERT_TRUE(connSp->run("ex = exec * from "+st+"_ex_UDPCT order by timestampv;"
                            "res = exec * from "+st+"_res_UDPCT"+std::to_string(i)+" order by timestampv;"
                            "all(each(eqObj, ex.values(), res.values()))")->getBool());
    }
    ASSERT_FALSE(connSp->run("`"+st+" in (exec tableName from getStreamingStat().udpPubTables)")->getBool());
}

TEST_F(StreamingUDPThreadedClientTester, subscribe_with_alldataTypes_should_serial)
{
    std::string case_=getCaseName();
    std::string st = case_;
    UDPCT::createSharedTableAndReplay_withAllDataType(st);
    dolphindb::ThreadedClient client(scfg);
    dolphindb::Signal notify;
    dolphindb::Mutex mtx;
    std::string res_tab = case_+"_res_UDPCT";
    int msg_total = 0;
    dolphindb::AutoFitTableAppender appender("", res_tab, *connSp);

    auto handler = [&](dolphindb::Message msg){
        dolphindb::LockGuard<dolphindb::Mutex> lock(&mtx);
        // std::cout << msg->getString() << std::endl;
        msg_total += msg->get(0)->size();
        dolphindb::TableSP tmp = UDPmsgToTable(msg);
        int rows = appender.append(tmp);
        ASSERT_EQ(rows, msg->get(0)->size());

        if (msg_total == 1000)
        {
            notify.set();
        }
    };

    auto thread = client.subscribe(HOST, PORT, handler, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD);
    notify.wait();
    client.unsubscribe(HOST, PORT, st, DEFAULT_ACTION_NAME);
    dolphindb::Util::sleep(1000);
    ASSERT_TRUE(connSp->run("ex = exec * from "+st+"_ex_UDPCT order by ind;"
                        "res = exec * from "+st+"_res_UDPCT order by ind;"
                        "all(each(eqObj, ex.values(), res.values()))")->getBool());
    ASSERT_FALSE(connSp->run("`"+st+" in (exec tableName from getStreamingStat().udpPubTables)")->getBool());
}

class StreamingUDPThreadedClientTester_realtime : public testing::Test, public ::testing::WithParamInterface<std::pair<dolphindb::DATA_TYPE, std::string>>
{
    public:
        static dolphindb::StreamingClientConfig scfg_realtime;
        static dolphindb::DBConnectionSP connSp;
        // Suite
        static void SetUpTestSuite()
        {
            bool ret = connSp->connect(HOST, PORT, USER, PASSWD);
            if (!ret)
            {
                std::cout << "Failed to connect to the server" << std::endl;
            }
            else
            {
                std::cout << "connect to " + HOST + ":" + std::to_string(PORT) << std::endl;
            }
            scfg_realtime.protocol = dolphindb::TransportationProtocol::UDP;
        }
        static void TearDownTestSuite()
        {
            connSp->close();
        }

    protected:
        // Case
        virtual void SetUp()
        {

        }
        virtual void TearDown()
        {

        }
    public:
        static std::vector<std::pair<dolphindb::DATA_TYPE, std::string>> getAVData()
        {
            return {
                {dolphindb::DT_BOOL_ARRAY, "take([true false], row_num)"},{dolphindb::DT_BOOL_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_CHAR_ARRAY, "take([10c 23c], row_num)"},{dolphindb::DT_CHAR_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_SHORT_ARRAY, "take([10s 23s], row_num)"},{dolphindb::DT_SHORT_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_INT_ARRAY, "take([10 23], row_num)"},{dolphindb::DT_INT_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_LONG_ARRAY, "take([10l 23l], row_num)"},{dolphindb::DT_LONG_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_DATE_ARRAY, "take([2021.01.01 2021.01.02], row_num)"},{dolphindb::DT_DATE_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_MONTH_ARRAY, "take([2021.01M 2021.02M], row_num)"},{dolphindb::DT_MONTH_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_TIME_ARRAY, "take([10:10:10 11:11:11], row_num)"},{dolphindb::DT_TIME_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_MINUTE_ARRAY, "take([10:10m 11:11m], row_num)"},{dolphindb::DT_MINUTE_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_SECOND_ARRAY, "take([10:10:10 11:11:11], row_num)"},{dolphindb::DT_SECOND_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_DATETIME_ARRAY, "take([2021.01.01T10:10:10 2021.01.02T11:11:11], row_num)"},{dolphindb::DT_DATETIME_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_TIMESTAMP_ARRAY, "take([2021.01.01T10:10:10.000 2021.01.02T11:11:11.000], row_num)"},{dolphindb::DT_TIMESTAMP_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_NANOTIME_ARRAY, "take([10:10:10.000000000 11:11:11.000000000], row_num)"},{dolphindb::DT_NANOTIME_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_NANOTIMESTAMP_ARRAY, "take([2021.01.01T10:10:10.000000000 2021.01.02T11:11:11.000000000], row_num)"},{dolphindb::DT_NANOTIMESTAMP_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_DATEHOUR_ARRAY, "take([datehour(1000 2000)], row_num)"},{dolphindb::DT_DATEHOUR_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_FLOAT_ARRAY, "take([10.00f 23.00f], row_num)"},{dolphindb::DT_FLOAT_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_DOUBLE_ARRAY, "take([10.314 23.445], row_num)"},{dolphindb::DT_DOUBLE_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_IP_ARRAY, "take([rand(ipaddr(), 2)], row_num)"},{dolphindb::DT_IP_ARRAY, "take([[ipaddr()]], row_num)"},
                {dolphindb::DT_UUID_ARRAY, "take([rand(uuid(), 2)], row_num)"},{dolphindb::DT_UUID_ARRAY, "take([[uuid()]], row_num)"},
                {dolphindb::DT_INT128_ARRAY, "take([rand(int128(), 2)], row_num)", }, {dolphindb::DT_INT128_ARRAY, "take([[int128()]], row_num)"},
                {dolphindb::DT_DECIMAL32_ARRAY, "take([decimal32(rand('-1.123''''2.23468965412', 2), 8)], row_num)"}, {dolphindb::DT_DECIMAL32_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_DECIMAL64_ARRAY, "take([decimal64(rand('-1.123''''2.123', 2), 15)], row_num)"}, {dolphindb::DT_DECIMAL64_ARRAY, "take([[00i]], row_num)"},
                {dolphindb::DT_DECIMAL128_ARRAY, "take([decimal128(rand('-1.123''''2.123', 2), 25)], row_num)"}, {dolphindb::DT_DECIMAL128_ARRAY, "take([[00i]], row_num)"},
            };
        };
        static std::vector<std::pair<dolphindb::DATA_TYPE, std::string>> getData()
        {
            return {
                {dolphindb::DT_BOOL, "rand(true false, row_num)"}, {dolphindb::DT_BOOL, "take(bool(), row_num)"},
                {dolphindb::DT_CHAR, "rand(127c, row_num)"}, {dolphindb::DT_CHAR, "take(char(), row_num)"},
                {dolphindb::DT_SHORT, "rand(32767h, row_num)"}, {dolphindb::DT_SHORT, "take(short(), row_num)"},
                {dolphindb::DT_INT, "rand(2147483647, row_num)"}, {dolphindb::DT_INT, "take(int(), row_num)"},
                {dolphindb::DT_LONG, "rand(1000l, row_num)"}, {dolphindb::DT_LONG, "take(long(), row_num)"},
                {dolphindb::DT_DATE, "rand(2019.01.01, row_num)"}, {dolphindb::DT_DATE, "take(date(), row_num)"},
                {dolphindb::DT_MONTH, "rand(2019.01M, row_num)"}, {dolphindb::DT_MONTH, "take(month(), row_num)"},
                {dolphindb::DT_TIME, "rand(12:00:00.123, row_num)"}, {dolphindb::DT_TIME, "take(time(), row_num)"},
                {dolphindb::DT_MINUTE, "rand(12:00m, row_num)"}, {dolphindb::DT_MINUTE, "take(minute(), row_num)"},
                {dolphindb::DT_SECOND, "rand(12:00:00, row_num)"}, {dolphindb::DT_SECOND, "take(second(), row_num)"},
                {dolphindb::DT_DATETIME, "rand(2019.01.01 12:00:00, row_num)"}, {dolphindb::DT_DATETIME, "take(datetime(), row_num)"},
                {dolphindb::DT_DATEHOUR, "rand(datehour(1000), row_num)"}, {dolphindb::DT_DATETIME, "take(datehour(), row_num)"},
                {dolphindb::DT_TIMESTAMP, "rand(2019.01.01 12:00:00.123, row_num)"}, {dolphindb::DT_TIMESTAMP, "take(timestamp(), row_num)"},
                {dolphindb::DT_NANOTIME, "rand(12:00:00.123456789, row_num)"}, {dolphindb::DT_NANOTIME, "take(nanotime(), row_num)"},
                {dolphindb::DT_NANOTIMESTAMP, "rand(2019.01.01 12:00:00.123456789, row_num)"}, {dolphindb::DT_NANOTIMESTAMP, "take(nanotimestamp(), row_num)"},
                {dolphindb::DT_DATEHOUR, "rand(datehour(100), row_num)"}, {dolphindb::DT_DATEHOUR, "take(datehour(), row_num)"},
                {dolphindb::DT_FLOAT, "rand(10.00f, row_num)"}, {dolphindb::DT_FLOAT, "take(float(), row_num)"},
                {dolphindb::DT_DOUBLE, "rand(10.00, row_num)"}, {dolphindb::DT_DOUBLE, "take(double(), row_num)"},
                {dolphindb::DT_IP, "rand(ipaddr(), row_num)"}, {dolphindb::DT_IP, "take(ipaddr(), row_num)"},
                {dolphindb::DT_UUID, "rand(uuid(), row_num)"}, {dolphindb::DT_UUID, "take(uuid(), row_num)"},
                {dolphindb::DT_INT128, "rand(int128(), row_num)"}, {dolphindb::DT_INT128, "take(int128(), row_num)"},
                {dolphindb::DT_DECIMAL32, "decimal32(rand('-1.123''''2.23468965412', row_num), 8)"}, {dolphindb::DT_DECIMAL32, "take(decimal32(NULL, 8), row_num)"},
                {dolphindb::DT_DECIMAL64, "decimal64(rand('-1.123''''2.123123123123123123', row_num), 15)"}, {dolphindb::DT_DECIMAL64, "take(decimal64(NULL, 15), row_num)"},
                {dolphindb::DT_DECIMAL128, "decimal128(rand('-1.123''''2.123', row_num), 25)"}, {dolphindb::DT_DECIMAL128, "take(decimal128(NULL, 25), row_num)"},
                {dolphindb::DT_STRING, "rand(`str1`str2, row_num)"}, {dolphindb::DT_STRING, "take(string(), row_num)"},
                {dolphindb::DT_SYMBOL, "rand(symbol(`sym1`sym2), row_num)"}, {dolphindb::DT_SYMBOL, "symbol(take(string(), row_num))"},
                {dolphindb::DT_BLOB, "rand(blob(`b1`b2`b3), row_num)"}, {dolphindb::DT_BLOB, "take(blob(''), row_num)"},
                };
        };
        void createST(dolphindb::DBConnection& conn, const std::string& name, const std::string& dtStr){
            std::string s =
                "colName = `ts`testCol;"
                "colType = [TIMESTAMP, "+dtStr+"];"
                "share streamTable(1:0, colName, colType) as "+name+";"
                "share table(1:0, colName, colType) as "+name+"_res_UDPCT;go;";
            conn.run(s);
        };
        void insertData(dolphindb::DBConnection& conn, const std::string& name, const std::string& colScript){
            std::string s =
                "row_num = 1000;"
                "col0 = now()..(now()+row_num-1);"
                "col1 = "+colScript+";"
                "for (i in 0..(row_num-1)){insert into "+name+" values([col0[i]], [col1[i]]);};";
            conn.run(s);
        };
};
dolphindb::DBConnectionSP StreamingUDPThreadedClientTester_realtime::connSp(new dolphindb::DBConnection);
dolphindb::StreamingClientConfig StreamingUDPThreadedClientTester_realtime::scfg_realtime;


INSTANTIATE_TEST_SUITE_P(basicTypes, StreamingUDPThreadedClientTester_realtime, testing::ValuesIn(StreamingUDPThreadedClientTester_realtime::getData()));
INSTANTIATE_TEST_SUITE_P(arrayVectorTypes, StreamingUDPThreadedClientTester_realtime, testing::ValuesIn(StreamingUDPThreadedClientTester_realtime::getAVData()));
TEST_P(StreamingUDPThreadedClientTester_realtime, test_realtime_should_serial)
{
    std::string case_=getCaseName();
    dolphindb::DATA_TYPE ttp = GetParam().first;
    std::string typeString = dolphindb::Util::getDataTypeString(ttp);
    if (typeString.compare(0, 9, "DECIMAL32") == 0)
        typeString = typeString.substr(0, 9) + "(8)";
    else if (typeString.compare(0, 9, "DECIMAL64") == 0)
        typeString = typeString.substr(0, 9) + "(15)";
    else if (typeString.compare(0, 10, "DECIMAL128") == 0)
        typeString = typeString.substr(0, 10) + "(25)";

    if (ttp > dolphindb::ARRAY_TYPE_BASE && typeString.compare(0, 7, "DECIMAL") == 0){
        typeString = typeString + "[]";
    }
    std::cout << "test type: " << typeString << std::endl;

    const std::string st = case_;
    createST(*connSp, st, typeString);
    dolphindb::ThreadedClient client(scfg_realtime);
    dolphindb::Signal notify;
    dolphindb::Mutex mtx;
    std::string res_tab = case_+"_res_UDPCT";
    int msg_total = 0;
    dolphindb::AutoFitTableAppender appender("", res_tab, *connSp);

    auto handler = [&](dolphindb::Message msg){
        dolphindb::LockGuard<dolphindb::Mutex> lock(&mtx);
        // std::cout << msg->getString() << std::endl;
        msg_total += msg->get(0)->size();
        // std::cout << "msg_toal: " << msg_total << std::endl;
        dolphindb::TableSP tmp = UDPmsgToTable(msg);
        int rows = appender.append(tmp);
        ASSERT_EQ(rows, msg->get(0)->size());

        if (msg_total == 1000)
        {
            notify.set();
        }
    };

    std::string dataScript = GetParam().second;
    std::thread th = std::thread([&]() {
        insertData(*connSp, st, dataScript);
    });
    auto thread = client.subscribe(HOST, PORT, handler, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD);
    th.join();
    notify.wait();
    dolphindb::Util::sleep(1000);
    std::cout << "msg_toal: " << msg_total << std::endl;
    client.unsubscribe(HOST, PORT, st, DEFAULT_ACTION_NAME);
    ASSERT_TRUE(connSp->run("re = select * from "+st+"_res_UDPCT order by ts;\
                        ex = select * from "+st+" order by ts;\
                        all(each(eqObj, re.values(), ex.values()))")->getBool());
    ASSERT_FALSE(connSp->run("`"+st+" in (exec tableName from getStreamingStat().udpPubTables)")->getBool());
    }


    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_should_serial)
    {
        std::string case_=getCaseName();
    std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);


        auto onehandler = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            bool succeeded = false;
            dolphindb::TableSP tmp = AnyVectorToTable(msg);
            while(!succeeded){
                try
                {
                    appender.append(tmp);
                    succeeded = true;
                }
                catch(const std::exception& e)
                {
                    dolphindb::Util::sleep(100);
                }
            }
            msg_total+=1;
            if (msg.getOffset() == 999)
            {
                notify.set();
            }
        };

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "mutiSchemaOne", 0, true, nullptr, false, false, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaOne");
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("re = select * from "+st+"_res_UDPCT order by datetimev;\
                            ex = select * from "+st+"_ex_UDPCT order by datetimev;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_should_serial)
    {
        GTEST_SKIP() << "not support batch handler yet";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                msg_total += 1;
                bool succeeded = false;
                dolphindb::TableSP tmp = AnyVectorToTable(msg);
                while(!succeeded){
                    try
                    {
                        appender.append(tmp);
                        succeeded = true;
                    }
                    catch(const std::exception& e)
                    {
                        dolphindb::Util::sleep(100);
                    }
                }
                if (msg.getOffset() == 999)
                {
                    notify.set();
                }
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "mutiSchemaBatch", 0, true, nullptr, false, 1, 1.0, false, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaBatch");

        ASSERT_TRUE(connSp->run("re = select * from "+st+"_res_UDPCT order by datetimev;\
                            ex = select * from "+st+"_ex_UDPCT order by datetimev;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);

    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribeUnsubscribe_notExistTable_should_serial)
    {

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        ASSERT_ANY_THROW(threadedClient.subscribe(HOST, PORT, [](dolphindb::Message msg){}, "st_notExist", "actionTest", 0, true, nullptr, false, false, USER, PASSWD));
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_tableNameNull_should_serial)
    {
        auto onehandler = [](dolphindb::Message msg){};


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);

        ASSERT_ANY_THROW(auto thread = threadedClient.subscribe(HOST, PORT, onehandler, "", "cppStreamingAPI", 0, true, nullptr, false, false, USER, PASSWD));
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_tableNameNull_should_serial)
    {
        auto batchhandler = [](std::vector<dolphindb::Message> msgs){};


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        ASSERT_ANY_THROW(auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, "", "cppStreamingAPI", 0, false, nullptr, 1, 1.0, false, "admin", "123456"));
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_offsetNegative_should_serial)
    {
        std::string case_=getCaseName();
    std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;

        auto onehandler = [&](dolphindb::Message msg)
        {
            msg_total += 1;
            // std::cout << msg->getString() << std::endl;
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "actionTest", -1, true, nullptr, false, false, USER, PASSWD);

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 0);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_offsetNegative_should_serial)
    {
        GTEST_SKIP() << "not support batch handler yet";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            for (auto &msg : msgs)
            {
                msg_total += 1;
                // std::cout  << msg->getString() << std::endl;
            }
        };

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "actionTest", -1, true, nullptr, false, 1, 1.0, false, USER, PASSWD);

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 0);
    }

    TEST_F(StreamingUDPThreadedClientTester,test_subscribe_onehandler_offsetInMiddle_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 10);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;

        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto onehandler = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            bool succeeded = false;
            dolphindb::TableSP tmp = AnyVectorToTable(msg);
            while(!succeeded){
                try
                {
                    appender.append(tmp);
                    succeeded = true;
                }
                catch(const std::exception& e)
                {
                    dolphindb::Util::sleep(100);
                }
            }
            msg_total+=1;

            int cur_offset = msg.getOffset();
            ASSERT_EQ(cur_offset, msg_total+4);
            if (cur_offset == 9)
            {
                notify.set();
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "actionTest", 5, true, nullptr, false, false, USER, PASSWD);
        notify.wait();
        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        ASSERT_TRUE(connSp->run(("eqObj("+st+"_res_UDPCT.rows(), 5)"))->getBool());
        ASSERT_TRUE(connSp->run("re = select * from "+st+"_res_UDPCT order by datetimev;\
                            ex = select * from "+st+"_ex_UDPCT order by datetimev limit 5, 5;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 5);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_offsetInMiddle_should_serial)
    {
        GTEST_SKIP() << "not support batch handler yet";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 10);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;

        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                msg_total += 1;
                bool succeeded = false;
                dolphindb::TableSP tmp = AnyVectorToTable(msg);
                while(!succeeded){
                    try
                    {
                        appender.append(tmp);
                        succeeded = true;
                    }
                    catch(const std::exception& e)
                    {
                        dolphindb::Util::sleep(100);
                    }
                }
                int cur_offset = msg.getOffset();
                ASSERT_EQ(cur_offset, msg_total+4);
                if (cur_offset == 9)
                {
                    notify.set();
                }
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "actionTest", 5, true, nullptr, false, 1, 1.0, false, USER, PASSWD);
        notify.wait();
        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        ASSERT_TRUE(connSp->run(("eqObj(res_UDPCT.rows(), 5)"))->getBool());
        ASSERT_TRUE(connSp->run("re = select * from "+st+"_res_UDPCT order by datetimev;\
                            ex = select * from "+st+"_ex_UDPCT order by datetimev limit 5, 5;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 5);
    }


    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_filter_should_serial)
    {
        GTEST_SKIP() << "filter not supported yet";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);
        int target_rows = connSp->run("(exec count(*) from "+st+"_ex_UDPCT where sym='b')[0]")->getInt();

        auto onehandler = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            bool succeeded = false;
            dolphindb::TableSP tmp = AnyVectorToTable(msg);
            while(!succeeded){
                try
                {
                    appender.append(tmp);
                    succeeded = true;
                }
                catch(const std::exception& e)
                {
                    dolphindb::Util::sleep(100);
                }
            }
            msg_total+=1;
            if (msg_total == target_rows)
            {
                notify.set();
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        dolphindb::VectorSP filter = dolphindb::Util::createVector(dolphindb::DT_SYMBOL, 1, 1);
        filter->setString(0, "b");
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "actionTest", 0, true, filter, false, false, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        ASSERT_TRUE(connSp->run("re = exec sym from "+st+"_res_UDPCT; all(re == `b)")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_TRUE(msg_total > 0);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_filter_should_serial)
    {
        GTEST_SKIP() << "filter not supported yet";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                msg_total += 1;
                bool succeeded = false;
                dolphindb::TableSP tmp = AnyVectorToTable(msg);
                while(!succeeded){
                    try
                    {
                        appender.append(tmp);
                        succeeded = true;
                    }
                    catch(const std::exception& e)
                    {
                        dolphindb::Util::sleep(100);
                    }
                }
                if (msg.getOffset() == 999)
                {
                    notify.set();
                }
            }
        };

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        dolphindb::VectorSP filter = dolphindb::Util::createVector(dolphindb::DT_SYMBOL, 1, 1);
        filter->setString(0, "b");

        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "actionTest", 0, true, filter, false, 1, 1.0, false, USER, PASSWD);
        dolphindb::Util::sleep(2000);

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        ASSERT_TRUE(connSp->run("re = exec sym from "+st+"_res_UDPCT; all(re == `b)")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_TRUE(msg_total > 0);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_msgAsTable_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto onehandler = [&](dolphindb::Message msg)
        {
            ASSERT_EQ(msg->getForm(), dolphindb::DF_TABLE);
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            bool succeeded = false;
            while(!succeeded){
                try
                {
                    appender.append(msg);
                    succeeded = true;
                }
                catch(const std::exception& e)
                {
                    dolphindb::Util::sleep(100);
                }
            }
            msg_total += msg->rows();
            int first_offset = msg.getOffset();
            int last_offset = first_offset + msg->rows() -1;
            if (last_offset == 999)
            {
                notify.set();
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "actionTest", 0, false, nullptr, true, false, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        ASSERT_TRUE(connSp->run("re = select * from "+case_+"_res_UDPCT order by datetimev;\
                            ex = select * from "+case_+"_ex_UDPCT order by datetimev;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_msgAsTable_should_serial)
    {
        GTEST_SKIP() << "batchhandler not support yet.";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 10000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                ASSERT_EQ(msg->getForm(), dolphindb::DF_TABLE);
                msg_total += msg->rows();
                std::cout << msg->rows() << std::endl;
                bool succeeded = false;
                while(!succeeded){
                    try
                    {
                        appender.append(msg);
                        succeeded = true;
                    }
                    catch(const std::exception& e)
                    {
                        dolphindb::Util::sleep(100);
                    }
                }
                int first_offset = msg.getOffset();
                int last_offset = first_offset + msg->rows() -1;
                if (last_offset == 9999)
                {
                    notify.set();
                }
            }

        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "actionTest", 0, true, nullptr, false, 2000, 0.1, true, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        ASSERT_TRUE(connSp->run("re = select * from "+case_+"_res_UDPCT order by datetimev;\
                            ex = select * from "+case_+"_ex_UDPCT order by datetimev;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 10000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_allowExists_should_serial)
    {
        GTEST_SKIP() << "server还没有修复allowExists的问题, jira: https://dolphindb1.atlassian.net/browse/D20-14283";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total,msg_total2 = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;

        auto onehandler = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            msg_total += 1;
            if (msg.getOffset() == 999)
            {
                notify.set();
            }
        };
        auto onehandler2 = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            msg_total2 += 1;
            if (msg_total2 == 1000)
            {
                notify.set();
            }
        };

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        dolphindb::ThreadedClient threadedClient2(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "actionTest", 0, true, nullptr, false, true, USER, PASSWD);
        auto thread2 = threadedClient2.subscribe(HOST, PORT, onehandler2, st, "actionTest", 0, true, nullptr, false, true, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient2.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        threadedClient2.exit();
        ASSERT_TRUE(threadedClient2.isExit());

        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(threadedClient2.getQueueDepth(thread2), 0);
        ASSERT_EQ(msg_total, 1000);
        ASSERT_EQ(msg_total2, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_allowExists_should_serial)
    {
        GTEST_SKIP() << "server还没有修复allowExists的问题, jira: https://dolphindb1.atlassian.net/browse/D20-14283";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total,msg_total2 = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                msg_total += 1;
                // std::cout  << msg->getString() << std::endl;
                if (msg.getOffset() == 999)
                    notify.set();
            }
        };
        auto batchhandler2 = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                msg_total2 += 1;
                // std::cout  << msg->getString() << std::endl;
                if (msg.getOffset() == 999)
                    notify.set();
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        dolphindb::ThreadedClient threadedClient2(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "actionTest", 0, true, nullptr, false, true, 1, 1.0, USER, PASSWD);
        auto thread2 = threadedClient2.subscribe(HOST, PORT, batchhandler2, st, "actionTest", 0, true, nullptr, false, true, 1, 1.0, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        threadedClient2.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient2.exit();
        ASSERT_TRUE(threadedClient2.isExit());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);
        ASSERT_EQ(threadedClient2.getQueueDepth(thread2), 0);
        ASSERT_EQ(msg_total2, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_resub_false_should_serial)
    {
        auto onehandler = [&](dolphindb::Message msg){};


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        ASSERT_ANY_THROW(auto thread = threadedClient.subscribe(HOST, PORT, onehandler, "st", "actionTest", 0, false, nullptr, false, false, USER, PASSWD));
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_resub_true_should_serial)
    {
        GTEST_SKIP() << "resub not support yet.";
        std::vector<dolphindb::Message> msgs;
        auto onehandler = [&](dolphindb::Message msg)
        {
            msgs.push_back(msg);
        };

        std::string st = "st_" + getRandString(10);

        connSp->run("share streamTable(1:0, `sym`val, [SYMBOL, INT]) as "+st+";tableInsert("+st+", `sym1, 1)");

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "resubTest", 0, true, nullptr, false, true, USER, PASSWD);
        dolphindb::Util::sleep(2000);
        connSp->run("\
            info = exec * from getStreamingStat().udpPubTables where tableName=`"+st+";\
            ip1 = (exec channel.split(':')[0] from info)[0];\
            port1 = (exec channel.split(':')[1] from info)[0];\
            stopPublishTable(ip1, port1, `"+st+",`"+DEFAULT_ACTION_NAME+",,true)");
        dolphindb::Util::sleep(1000);
        connSp->run("tableInsert("+st+", `sym2, 2)");
        dolphindb::Util::sleep(1000);
        threadedClient.unsubscribe(HOST, PORT, st, "resubTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        ASSERT_EQ(msgs.size(), 2);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_batchSize_should_serial)
    {
        GTEST_SKIP() << "batchhandler not support yet.";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                msg_total += 1;
                bool succeeded = false;
                dolphindb::TableSP tmp = AnyVectorToTable(msg);
                while(!succeeded){
                    try
                    {
                        appender.append(tmp);
                        succeeded = true;
                    }
                    catch(const std::exception& e)
                    {
                        dolphindb::Util::sleep(100);
                    }
                }
                if (msg.getOffset() == 999)
                {
                    notify.set();
                }
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "actionTest", 0, true, nullptr, false, 200, 1.0, false, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        ASSERT_TRUE(connSp->run("re = select * from "+case_+"_res_UDPCT order by datetimev;\
                            ex = select * from "+case_+"_ex_UDPCT order by datetimev;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_throttle_should_serial)
    {
        GTEST_SKIP() << "batchhandler not support yet.";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 10);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);
        long long start, end;

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                msg_total += 1;
                bool succeeded = false;
                dolphindb::TableSP tmp = AnyVectorToTable(msg);
                while(!succeeded){
                    try
                    {
                        appender.append(tmp);
                        succeeded = true;
                    }
                    catch(const std::exception& e)
                    {
                        dolphindb::Util::sleep(100);
                    }
                }
                if (msg.getOffset() == 9)
                {
                    notify.set();
                    end = dolphindb::Util::getEpochTime();
                }
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "actionTest", 0, true, nullptr, false, 5000, 2.5, false, USER, PASSWD);
        start = dolphindb::Util::getEpochTime();
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        ASSERT_TRUE(connSp->run("re = select * from "+case_+"_res_UDPCT order by datetimev;\
                            ex = select * from "+case_+"_ex_UDPCT order by datetimev;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 10);

        auto duration = end - start;
        ASSERT_TRUE(duration >= 2500);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_hostNull_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;

        auto onehandler = [&](dolphindb::Message msg)
        {
            msg_total += 1;
            // std::cout << msg->getString() << std::endl;
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        ASSERT_ANY_THROW(threadedClient.subscribe("", PORT, onehandler, st, "actionTest", 0, true, nullptr, false, true, USER, PASSWD));

    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_hostNull_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            for (auto &msg : msgs)
            {
                msg_total += 1;
                // std::cout  << msg->getString() << std::endl;
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        ASSERT_ANY_THROW(threadedClient.subscribe("", PORT, batchhandler, st, "actionTest", 0, true, nullptr, false, true, 1, 1.0, USER, PASSWD));

    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_portNull_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;

        auto onehandler = [&](dolphindb::Message msg)
        {
            msg_total += 1;
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        ASSERT_ANY_THROW(threadedClient.subscribe(HOST, NULL, onehandler, st, "actionTest", 0, true, nullptr, false, true, USER, PASSWD));

    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_actionNameNull_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;

        auto onehandler = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            msg_total += 1;
            // std::cout << msg->getString() << std::endl;
            if (msg.getOffset() == 999)
            {
                notify.set();
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "", 0, true, nullptr, false, false, USER, PASSWD);
        notify.wait();

        dolphindb::TableSP res = connSp->run("exec msgOffset, actions from getStreamingStat().udpPubTables where tableName=`"+st);
        ASSERT_EQ(res->getColumn(0)->getRow(0)->getInt(), 1000);
        ASSERT_EQ(res->getColumn(1)->getRow(0)->getString(), "");


        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "");
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_actionNameNull_should_serial)
    {
        GTEST_SKIP() << "batchhandler not support yet.";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                msg_total += 1;
                // std::cout  << msg->getString() << std::endl;
                if (msg.getOffset() == 999)
                {
                    notify.set();
                }
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "", 0, true, nullptr, false, true, 1, 1.0, USER, PASSWD);
        notify.wait();

        dolphindb::TableSP stat = connSp->run("select * from getStreamingStat().udpPubTables where tableName=`"+st);

        ASSERT_EQ(stat->getColumn(0)->getRow(0)->getString(), st);
        ASSERT_EQ(stat->getColumn(2)->getRow(0)->getInt(), 1000);
        ASSERT_EQ(stat->getColumn(3)->getRow(0)->getString(), "");

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "");
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_unsubscribe_onehandler_hostNull_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;

        auto onehandler = [&](dolphindb::Message msg)
        {
            msg_total += 1;
            // std::cout << msg->getString() << std::endl;
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "actionTest", 0, true, nullptr, false, false, USER, PASSWD);

        std::cout << "total size: " << msg_total << std::endl;
        ASSERT_FALSE(threadedClient.unsubscribe("", PORT, st, "actionTest"));

        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
    }

    TEST_F(StreamingUDPThreadedClientTester, test_unsubscribe_portNull_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;

        auto onehandler = [&](dolphindb::Message msg)
        {
            msg_total += 1;
            // std::cout << msg->getString() << std::endl;
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "actionTest", 0, true, nullptr, false, false, USER, PASSWD);
        std::cout << "total size: " << msg_total << std::endl;

        ASSERT_FALSE(threadedClient.unsubscribe(HOST, NULL, st, "actionTest"));
        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
    }

    TEST_F(StreamingUDPThreadedClientTester, test_unsubscribe_tableNameNull_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;

        auto onehandler = [&](dolphindb::Message msg)
        {
            msg_total += 1;
            // std::cout << msg->getString() << std::endl;
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "actionTest", 0, true, nullptr, false, false, USER, PASSWD);

        std::cout << "total size: " << msg_total << std::endl;
        ASSERT_FALSE(threadedClient.unsubscribe(HOST, PORT, "", "actionTest"));

        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
    }

    TEST_F(StreamingUDPThreadedClientTester, test_unsubscribe_actionNameNull_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;

        auto onehandler = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            msg_total += 1;
            // handle msg
            if (msg.getOffset() == 999)
            {
                notify.set();
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "actionTest", 0, true, nullptr, false, false, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        ASSERT_FALSE(threadedClient.unsubscribe(HOST, PORT, st, ""));
        dolphindb::TableSP res = connSp->run("exec msgOffset, actions from getStreamingStat().udpPubTables where tableName=`"+st);
        ASSERT_EQ(res->getColumn(0)->getRow(0)->getInt(), 1000);
        ASSERT_EQ(res->getColumn(1)->getRow(0)->getString(), "actionTest");


        threadedClient.unsubscribe(HOST, PORT, st, "actionTest");
        threadedClient.exit();
        ASSERT_TRUE(threadedClient.isExit());

        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());

    }

    TEST_F(StreamingUDPThreadedClientTester, tes_onehandler_subscribe_twice_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mtx;

        auto onehandler = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mtx);
            msg_total += 1;
            // handle msg
            if (msg_total == 1000) notify.set();
        };

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread1 = threadedClient.subscribe(HOST, PORT, onehandler, st, "mutiSchemaOne", 0, true, nullptr, false, false, USER, PASSWD);
        ASSERT_ANY_THROW(auto thread2 = threadedClient.subscribe(HOST, PORT, onehandler, st, "mutiSchemaOne", 0, true, nullptr, false, false, USER, PASSWD));
        notify.wait();
        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaOne");
        dolphindb::Util::sleep(1000);

        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread1), 0);
        ASSERT_EQ(msg_total, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_batchhandler_subscribe_twice_should_serial)
    {
        GTEST_SKIP() << "batchhandler not support yet.";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            for (auto &msg : msgs)
            {
                dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
                msg_total += 1;
                // handle msg
                if (msg.getOffset() == 999)
                {
                    notify.set();
                }
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread1 = threadedClient.subscribe(HOST, PORT, batchhandler, st, "mutiSchemaBatch", 0, true, nullptr, false, true, 1, 1.0, USER, PASSWD);
        ASSERT_ANY_THROW(auto thread2 = threadedClient.subscribe(HOST, PORT, batchhandler, st, "mutiSchemaBatch", 0, true, nullptr, false, true, 1, 1.0, USER, PASSWD));
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaBatch");
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread1), 0);
        ASSERT_EQ(msg_total, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_withAllDataType_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay_withAllDataType(st);
        int msg_total = 0;
        dolphindb::TableSP ex_table = connSp->run(st);
        int index = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto onehandler = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            bool succeeded = false;
            dolphindb::TableSP tmp = AnyVectorToTable(msg);
            while(!succeeded){
                try
                {
                    appender.append(tmp);
                    succeeded = true;
                }
                catch(const std::exception& e)
                {
                    dolphindb::Util::sleep(100);
                }
            }
            msg_total+=1;
            if (msg.getOffset() == 999)
            {
                notify.set();
            }
        };

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "mutiSchemaOne", 0, true, nullptr, false, false, USER, PASSWD);
        notify.wait();
        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaOne");

        ASSERT_TRUE(connSp->run("re = select * from "+case_+"_res_UDPCT order by ind;\
                            ex = select * from "+case_+"_ex_UDPCT order by ind;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_withAllDataType_should_serial)
    {
        GTEST_SKIP() << "batchhandler not support yet.";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay_withAllDataType(st);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                msg_total += 1;
                bool succeeded = false;
                dolphindb::TableSP tmp = AnyVectorToTable(msg);
                while(!succeeded){
                    try
                    {
                        appender.append(tmp);
                        succeeded = true;
                    }
                    catch(const std::exception& e)
                    {
                        dolphindb::Util::sleep(100);
                    }
                }
                if (msg.getOffset() == 999)
                {
                    notify.set();
                }
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "mutiSchemaBatch", 0, true, nullptr, false, true, 1, 1.0, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaBatch");
        ASSERT_TRUE(connSp->run("re = select * from "+case_+"_res_UDPCT order by ind;\
                            ex = select * from "+case_+"_ex_UDPCT order by ind;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_arrayVector_should_serial)
    {
        GTEST_SKIP() << "https://dolphindb1.atlassian.net/browse/D20-20340";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay_withArrayVector(st);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto onehandler = [&](dolphindb::Message msg)
        {
            std::cout << msg->getString() << std::endl;
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            bool succeeded = false;
            dolphindb::TableSP tmp = AnyVectorToTable(msg);
            while(!succeeded){
                try
                {
                    appender.append(tmp);
                    succeeded = true;
                }
                catch(const std::exception& e)
                {
                    dolphindb::Util::sleep(100);
                }
            }
            msg_total+=1;
            if (msg.getOffset() == 999)
            {
                notify.set();
            }
        };

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "arrayVectorTableTest", 0, true, nullptr, false, true, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "arrayVectorTableTest");
        ASSERT_TRUE(connSp->run("re = select * from "+case_+"_res_UDPCT order by ts;\
                            ex = select * from "+case_+"_ex_UDPCT order by ts;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_batchhandler_arrayVector_should_serial)
    {
        GTEST_SKIP() << "batchhandler not support yet.";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay_withArrayVector(st);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                msg_total += 1;
                bool succeeded = false;
                dolphindb::TableSP tmp = AnyVectorToTable(msg);
                while(!succeeded){
                    try
                    {
                        appender.append(tmp);
                        succeeded = true;
                    }
                    catch(const std::exception& e)
                    {
                        dolphindb::Util::sleep(100);
                    }
                }
                if (msg.getOffset() == 999)
                {
                    notify.set();
                }
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "arrayVectorTableTest", 0, true, nullptr, false, true, 1, 1.0, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "arrayVectorTableTest");
        ASSERT_TRUE(connSp->run("re = select * from "+case_+"_res_UDPCT order by ts;\
                            ex = select * from "+case_+"_ex_UDPCT order by ts;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_hugetable_onehandler_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;

        auto onehandler = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            msg_total += 1;
            if (msg_total % 100000 == 0)
                std::cout << "now subscribed rows: " << msg_total << std::endl;
            // handle msg
            if (msg.getOffset() == 999999)
            {
                notify.set();
            }
        };

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "mutiSchemaOne", 0, true, nullptr, false, false, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaOne");
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());

        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_hugetable_batchhandler_should_serial)
    {
        GTEST_SKIP() << "batchhandler not support yet.";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                msg_total += 1;
                if (msg_total % 100000 == 0)
                    std::cout << "now subscribed rows: " << msg_total << std::endl;
                // handle msg
                if (msg.getOffset() == 999999)
                {
                    notify.set();
                }
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "mutiSchemaBatch", 0, true, nullptr, false, true, 1, 1.0, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaBatch");
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());

        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_hugetable_onehandler_msgAsTable_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto onehandler = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            bool succeeded = false;
            while(!succeeded){
                try
                {
                    appender.append(msg);
                    succeeded = true;
                }
                catch(const std::exception& e)
                {
                    dolphindb::Util::sleep(100);
                }
            }
            msg_total += msg->rows();
            int first_offset = msg.getOffset();
            int last_offset = first_offset + msg->rows() - 1;
            if (last_offset == 999999)
            {
                notify.set();
            }
        };

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "mutiSchemaOne", 0, true, nullptr, true, false, USER, PASSWD);
        notify.wait();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaOne");
        ASSERT_TRUE(connSp->run("re = select * from "+case_+"_res_UDPCT order by datetimev;\
                            ex = select * from "+case_+"_ex_UDPCT order by datetimev;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());

        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000000);
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_hugetable_batchhandler_msgAsTable_batchSize_throttle_should_serial)
    {
        GTEST_SKIP() << "batchhandler not support yet.";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000000);
        int msg_total = 0;
        int test_batchSize = rand() % 65535 + 1;
        float test_throttle = (float)rand() / RAND_MAX * 10;

        int actual_batchSize = 1024;
        while (actual_batchSize <= test_batchSize) {
            actual_batchSize += 1024;
        }
        int batch_count = 0;
        int max_batch = 1000000 / actual_batchSize;
        int remain_rows = 1000000 % actual_batchSize;
        long long start_time;
        long long end_time;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *connSp);

        auto batchhandler = [&](std::vector<dolphindb::Message> msgs)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            for (auto &msg : msgs)
            {
                batch_count++;
                int rows = msg->rows();
                ASSERT_TRUE(rows == actual_batchSize || rows == remain_rows);
                msg_total += rows;
                bool succeeded = false;
                while(!succeeded){
                    try
                    {
                        appender.append(msg);
                        succeeded = true;
                    }
                    catch(const std::exception& e)
                    {
                        dolphindb::Util::sleep(100);
                    }
                }
                if (batch_count == max_batch){
                    start_time = dolphindb::Util::getEpochTime();
                }

                int first_offset = msg.getOffset();
                int last_offset = first_offset + rows - 1;
                if (last_offset == 999999)
                {
                    notify.set();
                }
            }
        };


        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, batchhandler, st, "mutiSchemaBatch", 0, true, nullptr, false, test_batchSize, test_throttle, true, USER, PASSWD);
        notify.wait();
        end_time = dolphindb::Util::getEpochTime();

        std::cout << "total size: " << msg_total << std::endl;
        threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaBatch");
        ASSERT_TRUE(connSp->run("re = select * from "+case_+"_res_UDPCT order by datetimev;\
                            ex = select * from "+case_+"_ex_UDPCT order by datetimev;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());

        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000000);
        ASSERT_TRUE(end_time - start_time >= test_throttle * 1000);
    }



    TEST_F(StreamingUDPThreadedClientTester, test_unsubscribe_in_handler_should_serial)
    {
        std::string case_=getCaseName();
        std::string st = case_;
        std::string s = "share streamTable(1:0, `ts`val, [TIMESTAMP, INT]) as "+st+";go;";
        connSp->run(s);
        int msg_total = 0;
        // dolphindb::Signal notify;
        // dolphindb::Mutex mutex;
        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto onehandler = [&](dolphindb::Message msg)
        {
            msg_total+=1;
            std::cout << "msg_total: " << msg_total << std::endl;
            std::cout << "unsubscribe while msg in queue" << std::endl;
            threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaOne");
            // notify.set();
        };
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "mutiSchemaOne", 0, true, nullptr, false, false, USER, PASSWD);
        connSp->run("for (i in 0..9999) {tableInsert(`"+st+", now()+i, rand(1000, 1)[0]);};");
        // notify.wait();
        ASSERT_EQ(msg_total, 1); // only one message is in handler, because unsubscribe is called before the first message received
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
    }

    TEST_F(StreamingUDPThreadedClientTester, test_resub_true_with_resubscribeTimeout_should_serial)
    {
        GTEST_SKIP() << "resub in UDP subscribe not support yet.";
        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);

        dolphindb::ThreadedClient client(scfg);
        unsigned int resubscribeTimeout = 500;
        dolphindb::ThreadedClient client1(scfg);
        client.subscribe(HOST, PORT, [](dolphindb::Message msg){}, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD);
        auto t = client1.subscribe(HOST, PORT, [](dolphindb::Message msg){}, st, DEFAULT_ACTION_NAME, 0, true, nullptr, false, false, USER, PASSWD, nullptr, {}, 100, false, resubscribeTimeout);

        dolphindb::Util::sleep(resubscribeTimeout+1000);
        client.unsubscribe(HOST, PORT, st, DEFAULT_ACTION_NAME);
        client1.exit();
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(t->isComplete());
        ASSERT_TRUE(connSp->run("(exec count(*) from getStreamingStat()[`pubConns] where tables =`"+st+") ==0")->getBool());
    }

    TEST_F(StreamingUDPThreadedClientTester, test_subscribe_onehandler_SCRAM_user_should_serial)
    {
        std::string userName=getRandString(20);
        connSp->run(
            "userName='"+userName+"';"
            "try{deleteUser(userName)}catch(ex){};go;createUser(userName, `123456, authMode='scram')"
        );
        dolphindb::DBConnectionSP conn_scram = new dolphindb::DBConnection(false, false, 7200, false, false, false, true);
        conn_scram->connect(HOST, PORT, userName, "123456");

        std::string case_=getCaseName();
        std::string st = case_;
        UDPCT::createSharedTableAndReplay(st, 1000);
        int msg_total = 0;
        dolphindb::Signal notify;
        dolphindb::Mutex mutex;
        dolphindb::AutoFitTableAppender appender("", case_+"_res_UDPCT", *conn_scram);
        auto onehandler = [&](dolphindb::Message msg)
        {
            dolphindb::LockGuard<dolphindb::Mutex> lock(&mutex);
            bool succeeded = false;
            dolphindb::TableSP tmp = AnyVectorToTable(msg);
            while(!succeeded){
                try
                {
                    appender.append(tmp);
                    succeeded = true;
                }
                catch(const std::exception& e)
                {
                    dolphindb::Util::sleep(100);
                }
            }
            msg_total+=1;
            if (msg.getOffset() == 999)
            {
                notify.set();
            }
        };

        dolphindb::ThreadedClient threadedClient = dolphindb::ThreadedClient(scfg);
        auto thread = threadedClient.subscribe(HOST, PORT, onehandler, st, "mutiSchemaOne", 0, true, nullptr, false, false, userName, "123456");
        notify.wait();
        threadedClient.unsubscribe(HOST, PORT, st, "mutiSchemaOne");
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(conn_scram->run("re = select * from "+case_+"_res_UDPCT order by datetimev;\
                            ex = select * from "+case_+"_ex_UDPCT order by datetimev;\
                            all(each(eqObj, re.values(), ex.values()))")->getBool());
        dolphindb::Util::sleep(1000);
        ASSERT_TRUE(conn_scram->run("(exec count(*) from getStreamingStat().udpPubTables where tableName =`"+st+") ==0")->getBool());
        ASSERT_EQ(threadedClient.getQueueDepth(thread), 0);
        ASSERT_EQ(msg_total, 1000);
        conn_scram->close();
    }

} // UDPCT namespace

#endif // __linux__
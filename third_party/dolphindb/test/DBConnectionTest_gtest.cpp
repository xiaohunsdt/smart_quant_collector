#include <gtest/gtest.h>
#include "config.h"

class DBConnectionTest : public testing::Test
{
    public:
        static dolphindb::DBConnection conn;
        // Suite
        static void SetUpTestSuite()
        {
            bool ret = conn.connect(HOST, PORT, USER, PASSWD, "", false, std::vector<std::string>(), 7200, true);
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

dolphindb::DBConnection DBConnectionTest::conn(false, false);

bool strToBool(std::string val)
{
    std::transform(val.begin(), val.end(), val.begin(), ::tolower);
    std::vector<std::string> falsevec = {"false", "f", "", "0"};
    for (auto &i : falsevec)
    {
        if (val == i)
            return false;
        else
            return true;
    }
    return NULL;
}

void StopCurNode(std::string cur_node)
{
    dolphindb::DBConnection conn1(false, false);
    conn1.connect(HOST_CLUSTER, PORT_CONTROLLER, USER_CLUSTER, PASSWD_CLUSTER);

    conn1.run("try{stopDataNode(\"" + cur_node + "\")}catch(ex){};");
    std::cout << cur_node + " has stopped..." << std::endl;
    dolphindb::Util::sleep(5000);
    std::cout << "restarting " + cur_node + "..." << std::endl;
    conn1.run("startDataNode(exec name from getClusterPerf() where mode !=1 and state != 1);go;"
    "do{sleep(1000);}while((exec distinct state from getClusterPerf()).size() !=1);");
}

bool assertUnConnect()
{
    dolphindb::Util::sleep(2000);
    dolphindb::DBConnection conn2(false, false);
    std::cout << "check if unconnected..." << std::endl;
    return conn2.connect(HOST, PORT_DNODE3, USER_CLUSTER, PASSWD_CLUSTER);
}

TEST_F(DBConnectionTest, test_connect_withErrUserid)
{
    dolphindb::DBConnection conn_demo(false, false);
    ASSERT_FALSE(conn_demo.connect(HOST, PORT, "adminasdvvv", PASSWD));
}

TEST_F(DBConnectionTest, test_connect_withErrPassword)
{
    dolphindb::DBConnection conn_demo(false, false);
    ASSERT_FALSE(conn_demo.connect(HOST, PORT, USER, "123456789"));
}

TEST_F(DBConnectionTest, test_connect_withStartupScript)
{
    std::string script = "startup_tab=table(`1`2`3 as col1,4 5 6 as col2)";
    dolphindb::DBConnection conn_demo(false, false);
    conn_demo.connect(HOST, PORT, USER, PASSWD, script, false, std::vector<std::string>(), 7200, false);

    dolphindb::ConstantSP resVec = conn_demo.run("tab1=table(`1`2`3 as col1,4 5 6 as col2);each(eqObj,tab1.values(),startup_tab.values())");
    for (auto i = 0; i < resVec->size(); i++)
        ASSERT_TRUE(resVec->get(i)->getBool());

    std::string res = conn_demo.getInitScript();
    ASSERT_EQ(script, res);
    conn_demo.close();

    std::string script2 = "startup_tab_2=table(`1`2`3 as col1,4 5 6 as col2)";
    conn_demo.setInitScript(script2);
    conn_demo.connect(HOST, PORT, USER, PASSWD, script2, false, std::vector<std::string>(), 7200, false);
    ASSERT_EQ(conn_demo.getInitScript(), script2);
    conn_demo.close();
}

TEST_F(DBConnectionTest, test_connection_enableSSL)
{
    dolphindb::DBConnection conn_demo(true);
    conn_demo.connect(HOST, PORT);
    conn_demo.login(USER, PASSWD, true);
    conn_demo.run("1+1");
    auto a = conn_demo.run("1 2 3 4");
    auto b = conn_demo.run("`APPL`MSFT`IBM`GOOG");
    auto c = conn_demo.run("2012.06.13 2012.06.14 2012.06.15 2012.06.16");
    std::vector<dolphindb::ConstantSP> cols = {a, b, c};
    std::vector<std::string> colNames = {"a", "b", "c"};
    conn_demo.upload(colNames, cols);
    dolphindb::TableSP t = conn_demo.run("t = table(a as col1, b as col2, c as col3);t");
    ASSERT_EQ(t->getString(), "col1 col2 col3      \n"
                              "---- ---- ----------\n"
                              "1    APPL 2012.06.13\n"
                              "2    MSFT 2012.06.14\n"
                              "3    IBM  2012.06.15\n"
                              "4    GOOG 2012.06.16\n");
    conn_demo.close();
}

TEST_F(DBConnectionTest, test_connection_asyncTask)
{
    std::string case_=getCaseName();
    conn.run("t=table(`1`2`3 as col1, 1 2 3 as col2);"
                   "share t as "+case_+";"
                   "records = [];");
    dolphindb::DBConnection conn_demo(false, true);
    conn_demo.connect(HOST, PORT, USER, PASSWD);
    conn_demo.run("for(i in 1..5){tableInsert("+case_+", string(i), i);sleep(1000)};");
    std::cout << "async job is running";
    do
    {
        conn.run("records.append!(exec count(*) from "+case_+")");
        std::cout << ".";
        dolphindb::Util::sleep(1000);
    } while (conn.run("exec count(*) from "+case_+"")->getInt() != 8);
    conn.run("records.append!(exec count(*) from "+case_+")");
    ASSERT_TRUE(conn.run("a=records.pop!();eqObj(a,8)")->getBool());
    conn_demo.close();
}

TEST_F(DBConnectionTest, test_connection_function_login)
{
    dolphindb::DBConnection conn_demo(false, false);
    conn_demo.connect(HOST, PORT);
    ASSERT_ANY_THROW(conn_demo.login("admin123123", PASSWD, false));
    ASSERT_ANY_THROW(conn_demo.login(USER, "123456789", false));

    conn_demo.login(USER, PASSWD, false);
    std::cout << conn_demo.run("getRecentJobs()")->getString();

    conn_demo.close();
}

TEST_F(DBConnectionTest, test_connection_python_script)
{
    std::string script1 = "import dolphindb as ddb\n"
                        "def list_append(testtype):\n"
                        "\ta= [testtype(1),testtype(2),testtype(3)]\n"
                        "\ta.append(testtype(4))\n"
                        "\ta.append(None)\n"
                        "\tassert a ==[testtype(1),testtype(2),testtype(3),testtype(4),None],'1'\n"
                        "\ndef test_list_append():\n"
                        "\ttypes=[int,long,short,float,double,char,bool,date,minute,month,second,datetime,timestamp,nanotime,nanotimestamp,datehour]\n"
                        "\tfor testtype in types:\n"
                        "\t\tlist_append(testtype)\n"
                        "\t\treturn True;\n"
                        "\n"
                        "def test_list_append_ipaddr_str_uuid():\n"
                        "\ta1= []\n"
                        "\ta2= []\n"
                        "\ta3= ['1','2','3']\n"
                        "\ta1.append(ipaddr(\"192.168.1.13\"))\n"
                        "\ta2.append(uuid(\"9d457e79-1bed-d6c2-3612-b0d31c1881f6\"))\n"
                        "\ta3.append('4')\n"
                        "\tassert a1 == [ipaddr(\"192.168.1.13\")],'2'\n"
                        "\tassert a2==[uuid(\"9d457e79-1bed-d6c2-3612-b0d31c1881f6\")],'3'\n"
                        "\tassert a3==['1','2','3','4'],'4'\n"
                        "\treturn True;\n"
                        "test_list_append()\n"
                        "test_list_append_ipaddr_str_uuid()";
    dolphindb::DBConnectionSP conn300_demo = new dolphindb::DBConnection(false, false, 7200, false, true);
    conn300_demo->connect(HOST, PORT, USER, PASSWD);
    dolphindb::ConstantSP res = conn300_demo->run(script1);
    ASSERT_EQ(res->getBool(), true);
    conn300_demo->close();
}

TEST_F(DBConnectionTest, test_connection_python_dataform)
{
    std::string script1 = "import dolphindb as ddb\n"
                        "a=[1,2,3]\n"
                        "b={1,2,3}\n"
                        "c={1:1,2:2}\n"
                        "d=(12,3,4)";
    dolphindb::DBConnectionSP conn300_demo = new dolphindb::DBConnection(false, false, 7200, false, true);
    conn300_demo->connect(HOST, PORT, USER, PASSWD);
    conn300_demo->run(script1);

    ASSERT_EQ(conn300_demo->run("type(a)")->getString(), "list");
    ASSERT_EQ(conn300_demo->run("type(b)")->getString(), "set");
    ASSERT_EQ(conn300_demo->run("type(c)")->getString(), "dict");
    ASSERT_EQ(conn300_demo->run("type(d)")->getString(), "std::tuple");

    conn300_demo->close();
}

TEST_F(DBConnectionTest, test_connection_python_setInitscriptAndgetInitscript)
{
    std::string script1 = "import dolphindb as ddb";
    dolphindb::DBConnectionSP conn300_demo = new dolphindb::DBConnection(false, false, 7200, false, true);
    conn300_demo->connect(HOST, PORT, USER, PASSWD, script1);
    std::string res = conn300_demo->getInitScript();
    ASSERT_EQ(res, script1);
    conn300_demo->close();
}

TEST_F(DBConnectionTest, test_connection_python_upload)
{
    std::vector<std::string> colName = {"col1", "col2", "col3", "col4", "col5"};
    std::vector<dolphindb::DATA_TYPE> colType = {dolphindb::DT_BOOL, dolphindb::DT_INT, dolphindb::DT_STRING, dolphindb::DT_DATE, dolphindb::DT_FLOAT};
    dolphindb::TableSP t = dolphindb::Util::createTable(colName, colType, 5, 5);

    t->set(0, 0, dolphindb::Util::createBool(true));
    t->set(1, 0, dolphindb::Util::createInt(1));
    t->set(2, 0, dolphindb::Util::createString("abc"));
    t->set(3, 0, dolphindb::Util::createDate(1));
    t->set(4, 0, dolphindb::Util::createFloat(1.123));

    dolphindb::DBConnectionSP conn300_demo = new dolphindb::DBConnection(false, false, 7200, false, true);
    conn300_demo->connect(HOST, PORT, USER, PASSWD);
    conn300_demo->upload("t", {t});
    dolphindb::TableSP t1 = conn300_demo->run("t");
    ASSERT_EQ(t1->getString(), t->getString());
    conn300_demo->close();
}

TEST_F(DBConnectionTest, test_connect_reconnect_should_serial)
{
    bool res;
    dolphindb::DBConnection conn(false, false);
    conn.connect(HOST_CLUSTER, PORT_DNODE3, USER_CLUSTER, PASSWD_CLUSTER, "", false, std::vector<std::string>(), 7200, true);
    std::string cur_node = conn.run("getNodeAlias()")->getString();

    std::thread t1 = std::thread(StopCurNode, cur_node);
    std::thread t2 = std::thread([&]
                                 { res = assertUnConnect(); });

    t1.join();
    t2.join();

    dolphindb::Util::sleep(10000);
    ASSERT_EQ(res, false);
    std::cout << "check passed..." << std::endl;
    std::cout << "check if reconnected..." << std::endl;
    ASSERT_EQ(conn.run("1+1")->getInt(), 2);
    std::cout << "check passed..." << std::endl;
}

TEST_F(DBConnectionTest, test_connectionPool_withErrUserid)
{
    ASSERT_ANY_THROW(dolphindb::DBConnectionPool pool_demo(HOST, PORT, 10, "adminasdccc", PASSWD));
}

TEST_F(DBConnectionTest, test_connectionPool_withErrPassword)
{
    ASSERT_ANY_THROW(dolphindb::DBConnectionPool pool_demo(HOST, PORT, 10, USER, "123456789"));
}


TEST_F(DBConnectionTest, DISABLED_test_connectionPool_loadBalance)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    ASSERT_ANY_THROW(dolphindb::DBConnectionPool pool_demo(HOST, PORT, 10, "adminasdccc", PASSWD, true));
    ASSERT_ANY_THROW(dolphindb::DBConnectionPool pool_demo(HOST, PORT, 10, USER, "123456789", true));
    dolphindb::DBConnectionPool pool_demo(HOST, PORT, 10, USER, PASSWD, true);
    dolphindb::DBConnection controller(false, false);
    conn.run(
        "dbpath=\""+dbName+"\";"
        "if(existsDatabase(dbpath)){dropDatabase(dbpath)};"
        "db=database(dbpath, VALUE, 2000.01.01..2000.12.20);"
        "t=table(100:0, `col1`col2`col3, [SYMBOL,INT,DATE]);"
        "db.createPartitionedTable(t,`dfs_tab,`col3);");

    pool_demo.run("t=table(100:0, `col1`col2`col3, [SYMBOL,INT,DATE]);"
                  "tableInsert(t,rand(`APPLE`YSL`BMW`HW`SAUM`TDB,1000), rand(1000,1000), take(2000.01.01..2000.12.20,1000));"
                  "loadTable('"+dbName+"',`dfs_tab).append!(t);",
                  1000);
    while (!pool_demo.isFinished(1000))
    {
        dolphindb::Util::sleep(100);
    };

    dolphindb::TableSP ex_tab = conn.run("select * from loadTable(dbpath,`dfs_tab)");
    ASSERT_EQ(1000, ex_tab->rows());
    ASSERT_FALSE(pool_demo.isShutDown());

    pool_demo.shutDown();
}

TEST_F(DBConnectionTest, test_connectionPool_compress)
{
    std::string case_=getCaseName();
    dolphindb::DBConnectionPool pool_demo(HOST, PORT, 10, USER, PASSWD, false, false, true);
    const int count = 1000;
    const int scale32 = rand() % 9, scale64 = rand() % 18;

    std::vector<int> time(count);
    std::vector<long long> value(count);
    std::vector<float> cfloat(count);
    std::vector<std::string> name(count);
    std::vector<std::string> blob(count);
    std::vector<std::string> ipaddr(count);
    std::vector<double> decimal32(count);
    std::vector<double> decimal64(count);
    int basetime = dolphindb::Util::countDays(2012, 1, 1);
    for (int i = 0; i < count; i++)
    {
        time[i] = basetime + (i % 15);
        value[i] = (i % 500 == 0) ? i : (long long)nullptr;
        cfloat[i] = float(i) + 0.1;
        name[i] = std::to_string(i);
        blob[i] = "fdsafsdfasd" + std::to_string(i % 32);
        ipaddr[i] = "192.168.100." + std::to_string(i % 255);
        decimal32[i] = 0.13566;
        decimal64[i] = 1.2245667899;
    }

    dolphindb::VectorSP boolVector = dolphindb::Util::createVector(dolphindb::DT_BOOL, count, count);
    dolphindb::VectorSP charVector = dolphindb::Util::createVector(dolphindb::DT_CHAR, count, count);
    dolphindb::VectorSP shortVector = dolphindb::Util::createVector(dolphindb::DT_SHORT, count, count);
    dolphindb::VectorSP intVector = dolphindb::Util::createVector(dolphindb::DT_INT, count, count);
    dolphindb::VectorSP dateVector = dolphindb::Util::createVector(dolphindb::DT_DATE, count, count);
    dolphindb::VectorSP monthVector = dolphindb::Util::createVector(dolphindb::DT_MONTH, count, count);
    dolphindb::VectorSP timeVector = dolphindb::Util::createVector(dolphindb::DT_TIME, count, count);
    dolphindb::VectorSP minuteVector = dolphindb::Util::createVector(dolphindb::DT_MINUTE, count, count);
    dolphindb::VectorSP secondVector = dolphindb::Util::createVector(dolphindb::DT_SECOND, count, count);
    dolphindb::VectorSP datetimeVector = dolphindb::Util::createVector(dolphindb::DT_DATETIME, count, count);
    dolphindb::VectorSP timestampVector = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, count, count);
    dolphindb::VectorSP nanotimeVector = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, count, count);
    dolphindb::VectorSP nanotimestampVector = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, count, count);
    dolphindb::VectorSP floatVector = dolphindb::Util::createVector(dolphindb::DT_FLOAT, count, count);
    dolphindb::VectorSP doubleVector = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, count, count);
    dolphindb::VectorSP symbolVector = dolphindb::Util::createVector(dolphindb::DT_SYMBOL, count, count);
    dolphindb::VectorSP stringVector = dolphindb::Util::createVector(dolphindb::DT_STRING, count, count);
    dolphindb::VectorSP ipaddrVector = dolphindb::Util::createVector(dolphindb::DT_IP, count, count);
    dolphindb::VectorSP blobVector = dolphindb::Util::createVector(dolphindb::DT_BLOB, count, count);
    dolphindb::VectorSP decimal32Vector = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, count, count, true, scale32);
    dolphindb::VectorSP decimal64Vector = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, count, count, true, scale64);

    boolVector->setInt(0, count, time.data());
    charVector->setInt(0, count, time.data());
    shortVector->setInt(0, count, time.data());
    intVector->setInt(0, count, time.data());
    dateVector->setInt(0, count, time.data());
    monthVector->setInt(0, count, time.data());
    timeVector->setInt(0, count, time.data());
    minuteVector->setInt(0, count, time.data());
    secondVector->setInt(0, count, time.data());
    datetimeVector->setLong(0, count, value.data());
    timestampVector->setLong(0, count, value.data());
    nanotimeVector->setLong(0, count, value.data());
    nanotimestampVector->setLong(0, count, value.data());
    floatVector->setFloat(0, count, cfloat.data());
    doubleVector->setFloat(0, count, cfloat.data());
    symbolVector->setString(0, count, name.data());
    stringVector->setString(0, count, name.data());
    ipaddrVector->setString(0, count, ipaddr.data());
    blobVector->setString(0, count, blob.data());
    decimal32Vector->setDouble(0, count, decimal32.data());
    decimal64Vector->setDouble(0, count, decimal64.data());

    std::vector<std::string> colName = {"cbool", "cchar", "cshort", "cint", "cdate", "cmonth", "ctime", "cminute", "csecond", "cdatetime", "ctimestamp", "cnanotime",
                              "cnanotimestamp", "cfloat", "cdouble", "csymbol", "cstring", "cipaddr", "cblob", "cdecimal32", "cdecimal64"};
    std::vector<dolphindb::ConstantSP> colVector{boolVector, charVector, shortVector, intVector, dateVector, monthVector, timeVector, minuteVector, secondVector,
                                 datetimeVector, timestampVector, nanotimeVector, nanotimestampVector, floatVector, doubleVector, symbolVector, stringVector,
                                 ipaddrVector, blobVector, decimal32Vector, decimal64Vector};
    dolphindb::TableSP table = dolphindb::Util::createTable(colName, colVector);
    std::vector<dolphindb::COMPRESS_METHOD> typeVec{dolphindb::COMPRESS_LZ4, dolphindb::COMPRESS_LZ4, dolphindb::COMPRESS_LZ4, dolphindb::COMPRESS_DELTA,
                                    dolphindb::COMPRESS_DELTA, dolphindb::COMPRESS_DELTA, dolphindb::COMPRESS_DELTA, dolphindb::COMPRESS_DELTA,
                                    dolphindb::COMPRESS_DELTA, dolphindb::COMPRESS_DELTA, dolphindb::COMPRESS_DELTA, dolphindb::COMPRESS_DELTA,
                                    dolphindb::COMPRESS_DELTA, dolphindb::COMPRESS_LZ4, dolphindb::COMPRESS_LZ4, dolphindb::COMPRESS_LZ4,
                                    dolphindb::COMPRESS_LZ4, dolphindb::COMPRESS_LZ4, dolphindb::COMPRESS_LZ4, dolphindb::COMPRESS_LZ4, dolphindb::COMPRESS_LZ4};
    table->setColumnCompressMethods(typeVec);
    conn.upload("table", table);

    std::vector<dolphindb::ConstantSP> args{table};
    std::string script = "colName =  `cbool`cchar`cshort`cint`cdate`cmonth`ctime`cminute`csecond`cdatetime`ctimestamp`cnanotime`cnanotimestamp`cfloat`cdouble`csymbol`cstring`cipaddr`cblob`cdecimal32`cdecimal64;\n"
                    "colType = [BOOL, CHAR, SHORT, INT, DATE, MONTH, TIME, MINUTE, SECOND, DATETIME, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, SYMBOL, STRING,IPADDR,BLOB,DECIMAL32(" +
                    std::to_string(scale32) + "),DECIMAL64(" + std::to_string(scale64) + ")];\n"
                    "share streamTable(1:0,colName,colType) as "+case_;
    conn.run(script);

    pool_demo.run("tableInsert{"+case_+"}", args, 1000);
    while (!pool_demo.isFinished(1000))
    {
        dolphindb::Util::sleep(1000);
    };

    ASSERT_EQ(pool_demo.getData(1000)->getInt(), count);
    dolphindb::ConstantSP res = conn.run("each(eqObj,table.values(),"+case_+".values())");
    for (int i = 0; i < res->size(); i++)
        ASSERT_TRUE(res->get(i)->getBool());
    pool_demo.shutDown();
}

TEST_F(DBConnectionTest, test_DBconnectionPool)
{
    std::string case_=getCaseName();
    dolphindb::DBConnectionPool pool_demo(HOST, PORT, 10, USER, PASSWD);
    ASSERT_EQ(pool_demo.getConnectionCount(), 10);
    std::vector<int> ids = {1, 10, 100, 1000};
    std::string srcipt1 = "tb = table(100:0,`col1`col2`col3,[INT,INT,INT]);share tb as "+case_;
    conn.run(srcipt1);
    ASSERT_ANY_THROW(pool_demo.run("for(i in 1..100){tableInsert("+case_+",rand(100,1),rand(100,1),rand(100,1));};select * from "+case_+"", -1));
    pool_demo.run("for(i in 1..100){tableInsert("+case_+",rand(100,1),rand(100,1),rand(100,1));};select * from "+case_+"", ids[0]);

    while (!pool_demo.isFinished(ids[0]))
    {
        dolphindb::Util::sleep(1000);
    };

    dolphindb::TableSP ex_tab = conn.run(case_);
    ASSERT_EQ(pool_demo.getData(ids[0])->getString(), ex_tab->getString());
    ASSERT_FALSE(pool_demo.isShutDown());

    std::vector<dolphindb::ConstantSP> args;
    args.push_back(ex_tab);
    pool_demo.run("tableInsert{"+case_+"}", args, ids[1]);
    while (!pool_demo.isFinished(ids[1]))
    {
        dolphindb::Util::sleep(1000);
    };

    pool_demo.run("exec * from "+case_+"", ids[2]);
    while (!pool_demo.isFinished(ids[2]))
    {
        dolphindb::Util::sleep(1000);
    };
    ex_tab = conn.run(""+case_+"");
    ASSERT_EQ(pool_demo.getData(ids[2])->getString(), ex_tab->getString());
    ASSERT_ANY_THROW(pool_demo.getData(ids[2])); // id can only be used once.
    ASSERT_FALSE(pool_demo.isShutDown());
    pool_demo.shutDown();
}

TEST_F(DBConnectionTest, test_DBconnectionPoolwithFetchSize)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    dolphindb::DBConnectionPool pool_demo(HOST, PORT, 10, USER, PASSWD);
    ASSERT_EQ(pool_demo.getConnectionCount(), 10);
    std::vector<int> ids = {1, 10, 100, 1000};
    std::string tab = "pt";
    conn.run("dbpath = '" + dbName +
                   "';if(existsDatabase(dbpath))"
                   "    {dropDatabase(dbpath)};"
                   "t=table(1:0, `c1`c2`c3`c4`c5, [INT, SYMBOL, TIMESTAMP, DOUBLE, DOUBLE[]]);"
                   "db=database(dbpath, VALUE, 1..10, engine='TSDB');"
                   "pt=db.createPartitionedTable(t,'pt','c1',,`c1);");

    std::string s = "rows = 20000;t=table(rand(1..100,20000) as c1, rand(`a`b`c`d`e`f`g, 20000) as c2, rand(timestamp(10001..50100), 20000) as c3, rand(100.0000, 20000) as c4, arrayVector(1..20000*3, rand(100.0000, 60000)) as c5);"
               "loadTable('" + dbName + "', `" + tab + ").append!(t);"
               "go;select * from loadTable('" + dbName + "', `" + tab + ")";
    pool_demo.run(s, ids[1], 4, 2, 8192);

    dolphindb::AutoFitTableAppender appender("", "t", conn);
    while (!pool_demo.isFinished(ids[1]))
    {
        dolphindb::Util::sleep(1000);
    };

    dolphindb::BlockReaderSP reader = pool_demo.getData(ids[1]);
    int rows{0};
    while (reader->hasNext()) {
        auto res = reader->read();
        int upserts = appender.append(res);
        ASSERT_EQ(upserts, res->rows());
        ASSERT_EQ(res->getForm(), dolphindb::DF_TABLE);
        rows += res->rows();
    }
    ASSERT_EQ(rows, 20000);
    ASSERT_TRUE(conn.run(
        "ex_t=select * from loadTable(dbpath,`"+tab+");"
        "all(each(eqObj,ex_t.values(),t.values()))"
    )->getBool());
    pool_demo.shutDown();
}

TEST_F(DBConnectionTest, test_connection_concurrent_insert_datas)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    srand(time(NULL));
    std::string tab = "pt";

    conn.run("dbpath = '" + dbName +
                   "';if(existsDatabase(dbpath))"
                   "    {dropDatabase(dbpath)};"
                   "t=table(1:0, `c1`c2`c3, [INT, SYMBOL, TIMESTAMP]);"
                   "db=database(dbpath, VALUE, 1..10, chunkGranularity='TABLE');"
                   "pt=db.createPartitionedTable(t,'pt','c1');");

    std::string s = "t=table(rand(1..100,10) as c1, rand(`a`b`c`d`e`f`g, 10) as c2, rand(timestamp(10001..10100), 10) as c3);"
               "loadTable('" +
               dbName + "', `" + tab + ").append!(t);"
                                       "go;"
                                       "exec count(*) from loadTable('" +
               dbName + "', `" + tab + ");";
    auto query = [&s]()
    {
        dolphindb::DBConnection conn0(false, false);
        conn0.connect(HOST, PORT, USER, PASSWD);
        bool success = false;
        while (!success)
        {
            try
            {
                conn0.run(s);
                success = true;
            }
            catch (const std::exception &e)
            {
                std::string err = e.what();
                std::cout << "err is " << err << std::endl;
                ASSERT_TRUE(err.find("RefId:S00002") != std::string::npos || err.find("RefId:S01019") != std::string::npos);
            }
        }
        conn0.close();
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 50; i++)
        threads.emplace_back(query);

    for (auto &thread : threads)
        thread.join();

    ASSERT_EQ(conn.run("exec count(*) from loadTable(dbpath, `pt)")->getInt(), 500);
}

TEST_F(DBConnectionTest, test_connectionPool_concurrent_insert_datas)
{
    srand(time(NULL));
    std::string dbpath = "dfs://test_connectionPool_concurrent_insert_datas";
    std::string tab = "pt";
    int maxConnections = conn.run("int(getConfig('maxConnections'))")->getInt();
    conn.run("dbpath = '" + dbpath +
                   "';if(existsDatabase(dbpath))"
                   "    {dropDatabase(dbpath)};"
                   "t=table(1:0, `c1`c2`c3, [INT, SYMBOL, TIMESTAMP]);"
                   "db=database(dbpath, VALUE, 1..10, chunkGranularity='TABLE');"
                   "pt=db.createPartitionedTable(t,'pt','c1');");

    std::string s = "t=table(rand(1..100,100) as c1, rand(`a`b`c`d`e`f`g, 100) as c2, rand(timestamp(10001..10100), 100) as c3);"
               "loadTable('" +
               dbpath + "', `" + tab + ").append!(t);"
                                       "go;"
                                       "exec count(*) from loadTable('" +
               dbpath + "', `" + tab + ");";
    auto query = [&s]()
    {
        dolphindb::DBConnectionPool pool(HOST, PORT, 10, USER, PASSWD);
        bool success = false;
        int id = rand() % 1000;
        while (!success)
        {
            try
            {
                pool.run(s, id);
                while (!pool.isFinished(id))
                {
                    dolphindb::Util::sleep(500);
                }

                success = true;
            }
            catch (const std::exception &e)
            {
                std::string err = e.what();
                ASSERT_TRUE(err.find("RefId:S00002") != std::string::npos || err.find("RefId:S01019") != std::string::npos);
            }
        }
        pool.shutDown();
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; i++)
        threads.emplace_back(query);

    for (auto &thread : threads)
        thread.join();

    ASSERT_EQ(conn.run("exec count(*) from loadTable('dfs://test_connectionPool_concurrent_insert_datas', `pt)")->getInt(), 500);
}

class connection_insert_null : public DBConnectionTest, public testing::WithParamInterface<std::tuple<std::string, dolphindb::DATA_TYPE>>
{
public:
    static std::vector<std::tuple<std::string, dolphindb::DATA_TYPE>> data_prepare()
    {
        std::vector<std::string> testTypes = {"BOOL", "CHAR", "SHORT", "INT", "LONG", "DATE", "MONTH", "TIME", "MINUTE", "SECOND", "DATETIME", "TIMESTAMP", "NANOTIME", "NANOTIMESTAMP", "DATEHOUR", "FLOAT", "DOUBLE", "STRING", "SYMBOL", "BLOB", "IPADDR", "UUID", "INT128", "DECIMAL32(8)", "DECIMAL64(15)", "DECIMAL128(28)",
                                    "BOOL[]", "CHAR[]", "SHORT[]", "INT[]", "LONG[]", "DATE[]", "MONTH[]", "TIME[]", "MINUTE[]", "SECOND[]", "DATETIME[]", "TIMESTAMP[]", "NANOTIME[]", "NANOTIMESTAMP[]", "DATEHOUR[]", "FLOAT[]", "DOUBLE[]", "IPADDR[]", "UUID[]", "INT128[]", "DECIMAL32(8)[]", "DECIMAL64(15)[]", "DECIMAL128(25)[]"};
        std::vector<dolphindb::DATA_TYPE> dataTypes = {dolphindb::DT_BOOL, dolphindb::DT_CHAR, dolphindb::DT_SHORT, dolphindb::DT_INT, dolphindb::DT_LONG, dolphindb::DT_DATE, dolphindb::DT_MONTH, dolphindb::DT_TIME, dolphindb::DT_MINUTE, dolphindb::DT_SECOND, dolphindb::DT_DATETIME, dolphindb::DT_TIMESTAMP, dolphindb::DT_NANOTIME, dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_DATEHOUR, dolphindb::DT_FLOAT, dolphindb::DT_DOUBLE, dolphindb::DT_STRING, dolphindb::DT_SYMBOL, dolphindb::DT_BLOB, dolphindb::DT_IP, dolphindb::DT_UUID, dolphindb::DT_INT128, dolphindb::DT_DECIMAL32, dolphindb::DT_DECIMAL64, dolphindb::DT_DECIMAL128,
                                       dolphindb::DT_BOOL_ARRAY, dolphindb::DT_CHAR_ARRAY, dolphindb::DT_SHORT_ARRAY, dolphindb::DT_INT_ARRAY, dolphindb::DT_LONG_ARRAY, dolphindb::DT_DATE_ARRAY, dolphindb::DT_MONTH_ARRAY, dolphindb::DT_TIME_ARRAY, dolphindb::DT_MINUTE_ARRAY, dolphindb::DT_SECOND_ARRAY, dolphindb::DT_DATETIME_ARRAY, dolphindb::DT_TIMESTAMP_ARRAY, dolphindb::DT_NANOTIME_ARRAY, dolphindb::DT_NANOTIMESTAMP_ARRAY, dolphindb::DT_DATEHOUR_ARRAY, dolphindb::DT_FLOAT_ARRAY, dolphindb::DT_DOUBLE_ARRAY, dolphindb::DT_IP_ARRAY, dolphindb::DT_UUID_ARRAY, dolphindb::DT_INT128_ARRAY, dolphindb::DT_DECIMAL32_ARRAY, dolphindb::DT_DECIMAL64_ARRAY, dolphindb::DT_DECIMAL128_ARRAY};
        std::vector<std::tuple<std::string, dolphindb::DATA_TYPE>> data;
        for (auto i = 0; i < testTypes.size(); i++)
            data.push_back(make_tuple(testTypes[i], dataTypes[i]));
        return data;
    }
};

INSTANTIATE_TEST_SUITE_P(, connection_insert_null, testing::ValuesIn(connection_insert_null::data_prepare()));
TEST_P(connection_insert_null, test_append_empty_table)
{
    std::string case_=getCaseName();
    std::string type = std::get<0>(GetParam());
    dolphindb::DATA_TYPE dataType = std::get<1>(GetParam());
    std::string colName = "c1";
    std::string table = case_;
    std::string script1 =
        "colName = [`" + colName + "];"
                                   "colType = [" +
        type + "];"
               "share table(1:0, colName, colType) as "+table+";";

    conn.run(script1);
    dolphindb::VectorSP col1 = dolphindb::Util::createVector(dataType, 0);
    std::vector<std::string> colNames = {"c1"};
    std::vector<dolphindb::ConstantSP> cols = {col1};
    dolphindb::TableSP empty2 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("empty", empty2);
    auto res = conn.run("each(eqObj, "+table+".values(), empty.values())");
    ASSERT_EQ(res->getString(), "[1]");

    std::vector<dolphindb::ConstantSP> args = {empty2};
    auto t = conn.run("append!{"+table+"}", args);
    ASSERT_EQ(conn.run("exec count(*) from "+table)->getInt(), 0);
    ASSERT_EQ(t->rows(), 0);
}

TEST_F(DBConnectionTest, test_connection_parallel)
{
    std::string userName=getRandString(20);
    conn.run(
        "userName='"+userName+"';"
        "try{createUser(userName, `123456)}catch(ex){};go;setMaxJobParallelism(userName, 10);"
    );
    { // TestConnectionParallel_lt_MaxJobParallelism
        int priority = 3;
        int parallel = 1;
        dolphindb::DBConnectionSP _tmpC = new dolphindb::DBConnection(false, false);
        _tmpC->connect(HOST, PORT, userName, "123456");
        dolphindb::TableSP res = _tmpC->run("select * from getConsoleJobs() where userID=`"+userName, priority, parallel);
        ASSERT_EQ(res->getColumn(5)->get(0)->getInt(), 3);
        ASSERT_EQ(res->getColumn(6)->get(0)->getInt(), 1);

    }
    { // TestConnectionParallel_gt_MaxJobParallelism
        int priority = 1;
        int parallel = 12;
        dolphindb::DBConnectionSP _tmpC = new dolphindb::DBConnection(false, false);
        _tmpC->connect(HOST, PORT, userName, "123456");
        dolphindb::TableSP res = _tmpC->run("select * from getConsoleJobs() where userID=`"+userName, priority, parallel);
        ASSERT_EQ(res->getColumn(5)->get(0)->getInt(), 1);
        ASSERT_EQ(res->getColumn(6)->get(0)->getInt(), 10);
    }
    { // TestConnectionParallel_default
        dolphindb::DBConnectionSP _tmpC = new dolphindb::DBConnection(false, false);
        _tmpC->connect(HOST, PORT, userName, "123456");
        dolphindb::TableSP res = _tmpC->run("select * from getConsoleJobs() where userID=`"+userName);
        ASSERT_EQ(res->getColumn(5)->get(0)->getInt(), 4);
        ASSERT_EQ(res->getColumn(6)->get(0)->getInt(), 10);
    }
}

TEST_F(DBConnectionTest, test_connectionPool_parallel)
{
    std::string userName=getRandString(20);
    conn.run(
        "userName='"+userName+"';"
        "try{createUser(userName, `123456)}catch(ex){};go;setMaxJobParallelism(userName, 10);"
    );
    { // TestConnectionParallel_lt_MaxJobParallelism
        int priority = 3;
        int parallel = rand() % 10 + 1;
        dolphindb::DBConnectionPoolSP _tmpP = new dolphindb::DBConnectionPool(HOST, PORT, 10, userName, "123456");
        int id = rand() % 1000;
        ASSERT_ANY_THROW(_tmpP->run("select * from getConsoleJobs() where userID=`"+userName+"", id, priority, 0));
        _tmpP->run("select * from getConsoleJobs() where userID=`"+userName+"", id, priority, parallel);
        while (!_tmpP->isFinished(id))
        {
            dolphindb::Util::sleep(500);
        }
        dolphindb::TableSP res = _tmpP->getData(id);
        ASSERT_EQ(res->getColumn(5)->get(0)->getInt(), priority);
        ASSERT_EQ(res->getColumn(6)->get(0)->getInt(), parallel);

    }
    { // TestConnectionParallel_gt_MaxJobParallelism
        int priority = 1;
        int parallel = rand() % 64 + 11;
        dolphindb::DBConnectionPoolSP _tmpP = new dolphindb::DBConnectionPool(HOST, PORT, 10, userName, "123456");
        int id = rand() % 1000;
        _tmpP->run("select * from getConsoleJobs() where userID=`"+userName+"", id, priority, parallel);
        while (!_tmpP->isFinished(id))
        {
            dolphindb::Util::sleep(500);
        }
        dolphindb::TableSP res = _tmpP->getData(id);
        ASSERT_EQ(res->getColumn(5)->get(0)->getInt(), priority);
        ASSERT_EQ(res->getColumn(6)->get(0)->getInt(), 10);
    }
    { // TestConnectionParallel_default
        dolphindb::DBConnectionPoolSP _tmpP = new dolphindb::DBConnectionPool(HOST, PORT, 10, userName, "123456");
        int id = rand() % 1000;
        _tmpP->run("select * from getConsoleJobs() where userID=`"+userName+"", id);
        while (!_tmpP->isFinished(id))
        {
            dolphindb::Util::sleep(500);
        }
        dolphindb::TableSP res = _tmpP->getData(id);
        ASSERT_EQ(res->getColumn(5)->get(0)->getInt(), 4);
        ASSERT_EQ(res->getColumn(6)->get(0)->getInt(), 10);
    }
}

TEST_F(DBConnectionTest, test_connectionPool_run_script)
{
    dolphindb::DBConnectionPool pool(HOST, PORT, 8, USER, PASSWD);
    int id_1 = pool.run("sleep(3000);1");
    while (!pool.isFinished(id_1)){
        dolphindb::Util::sleep(500);
    }
    ASSERT_EQ(pool.getData(id_1)->getString(), "1");
}

TEST_F(DBConnectionTest, test_connectionPool_condition_variable_run_script)
{
    auto cv = std::make_shared<std::condition_variable>();
    std::mutex cv_m;
    std::unique_lock<std::mutex> lk(cv_m);
    dolphindb::DBConnectionPool pool(HOST, PORT, 8, USER, PASSWD);
    int id_1 = pool.run("sleep(3000);1", cv);
    int id_2 = pool.run("sleep(3000);2", cv);
    int id_3 = pool.run("sleep(3000);3", cv);
    int id_4 = pool.run("sleep(3000);4", cv);
    cv->wait(lk, [id_1, &pool]{
        return pool.isFinished(id_1);
    });
    cv->wait(lk, [id_2, &pool]{
        return pool.isFinished(id_2);
    });
    cv->wait(lk, [id_3, &pool]{
        return pool.isFinished(id_3);
    });
    cv->wait(lk, [id_4, &pool]{
        return pool.isFinished(id_4);
    });
    ASSERT_EQ(pool.getData(id_1)->getString(), "1");
    ASSERT_EQ(pool.getData(id_2)->getString(), "2");
    ASSERT_EQ(pool.getData(id_3)->getString(), "3");
    ASSERT_EQ(pool.getData(id_4)->getString(), "4");
}

TEST_F(DBConnectionTest, test_connectionPool_run_function)
{
    dolphindb::DBConnectionPool pool(HOST, PORT, 8, USER, PASSWD);
    std::vector<dolphindb::ConstantSP> args;
    args.emplace_back(dolphindb::Util::createInt(1));
    args.emplace_back(dolphindb::Util::createInt(2));
    int id_1 = pool.run("add", args);
    while (!pool.isFinished(id_1)){
        dolphindb::Util::sleep(500);
    }
    ASSERT_EQ(pool.getData(id_1)->getString(), "3");
}

TEST_F(DBConnectionTest, test_connectionPool_condition_variable_run_function)
{
    auto cv = std::make_shared<std::condition_variable>();
    std::mutex cv_m;
    std::unique_lock<std::mutex> lk(cv_m);
    dolphindb::DBConnectionPool pool(HOST, PORT, 8, USER, PASSWD);
    std::vector<dolphindb::ConstantSP> args;
    args.emplace_back(dolphindb::Util::createInt(1));
    args.emplace_back(dolphindb::Util::createInt(2));
    int id_1 = pool.run("add", args, cv);
    int id_2 = pool.run("add", args, cv);
    int id_3 = pool.run("add", args, cv);
    int id_4 = pool.run("add", args, cv);
    cv->wait(lk, [id_1, &pool]{
        return pool.isFinished(id_1);
    });
    cv->wait(lk, [id_2, &pool]{
        return pool.isFinished(id_2);
    });
    cv->wait(lk, [id_3, &pool]{
        return pool.isFinished(id_3);
    });
    cv->wait(lk, [id_4, &pool]{
        return pool.isFinished(id_4);
    });
    ASSERT_EQ(pool.getData(id_1)->getString(), "3");
    ASSERT_EQ(pool.getData(id_2)->getString(), "3");
    ASSERT_EQ(pool.getData(id_3)->getString(), "3");
    ASSERT_EQ(pool.getData(id_4)->getString(), "3");
}

TEST_F(DBConnectionTest, test_connection_login_encrypt){
    dolphindb::DBConnectionSP _c = new dolphindb::DBConnection(false, false);
    _c->connect(HOST, PORT);
    _c->login(USER, PASSWD, true);
    dolphindb::ConstantSP res = _c->run("1+1");
    ASSERT_EQ(res->getInt(), 2);
    _c->close();
}

TEST_F(DBConnectionTest, test_connection_login_ban_guest){
    {
        dolphindb::DBConnectionSP _c = new dolphindb::DBConnection(false, false);
        _c->connect(HOST, PORT);
        _c->login(USER, PASSWD, false);
        bool res = _c->run("bool(getConfig(`enableClientAuth))")->getBool();
        ASSERT_TRUE(res);
        _c->close();
    }
    {
        dolphindb::DBConnectionSP _c = new dolphindb::DBConnection(false, false);
        _c->connect(HOST, PORT);
        ASSERT_ANY_THROW(_c->run("bool(getConfig(`enableClientAuth))")); // guest user is banned
        _c->close();
    }
    {
        dolphindb::DBConnectionPoolSP _p = new dolphindb::DBConnectionPool(HOST, PORT, 10, USER, PASSWD);
        int id = rand() % 1000;
        _p->run("bool(getConfig(`enableClientAuth))", id);
        while (!_p->isFinished(id))
        {
            dolphindb::Util::sleep(500);
        }
        bool res = _p->getData(id)->getBool();
        ASSERT_TRUE(res);
        _p->shutDown();
    }
    {
        dolphindb::DBConnectionPoolSP _p = new dolphindb::DBConnectionPool(HOST, PORT, 10);
        int id = rand() % 1000;
        try{
            _p->run("bool(getConfig(`enableClientAuth))", id);
            while (!_p->isFinished(id))
            {
                dolphindb::Util::sleep(500);
            }
        }catch(const std::exception& e){
            ASSERT_PRED_FORMAT2(testing::IsSubstring, "RefId: S04009", e.what());
        }
  
        _p->shutDown();
    }

}

TEST_F(DBConnectionTest, test_connection_SCRAM){
    std::string case_=getRandString(20);
    try{
        conn.run(
            "userName='"+case_+"';"
            "try{deleteUser(userName)}catch(ex){};go;createUser(userName, `123456, authMode='scram')"
        );
    }catch(const std::exception& e){
        GTEST_SKIP() << e.what();
    }
    {
        dolphindb::DBConnectionSP _c = new dolphindb::DBConnection(false, false, 7200, false, false, false, true);
        _c->connect(HOST, PORT);
        _c->login(case_, "123456", true);
        dolphindb::ConstantSP res = _c->run("getCurrentSessionAndUser()[1]");
        ASSERT_EQ(res->getString(), case_);
        _c->close();
        std::cout << "test_connection_SCRAM1 passed" << std::endl;
    }
    {
        dolphindb::DBConnection _c = dolphindb::DBConnection(false, false, 7200, false, false, false, true);
        _c.connect(HOST, PORT, case_, "123456");
        dolphindb::ConstantSP res = _c.run("getCurrentSessionAndUser()[1]");
        ASSERT_EQ(res->getString(), case_);
        _c.close();
        std::cout << "test_connection_SCRAM2 passed" << std::endl;
    }
    { // login with no scram auth user admin
        dolphindb::DBConnection _c = dolphindb::DBConnection(false, false, 7200, false, false, false, true);
        _c.connect(HOST, PORT, USER, PASSWD); // stdout: [debug] [140591328847872] : user 'admin' doesn't support scram authMode.
    }
    { // enableEncryption with scram user
        dolphindb::DBConnection _c = dolphindb::DBConnection(HOST, PORT);
        _c.connect();
        _c.login(case_, "123456", true);
        dolphindb::ConstantSP res = _c.run("getCurrentSessionAndUser()[1]");
        ASSERT_EQ(res->getString(), case_);
        _c.close();
    }
    { // async login with scram user
        ASSERT_ANY_THROW(dolphindb::DBConnection(false, true, 7200, false, false, false, true));
    }
}

TEST_F(DBConnectionTest, test_connectionPool_SCRAM){
    std::string userName=getRandString(20);
    conn.run(
        "userName='"+userName+"';"
        "try{deleteUser(userName)}catch(ex){};go;createUser(userName, `123456, authMode='scram')"
    );
    dolphindb::DBConnectionPoolSP _p = new dolphindb::DBConnectionPool(HOST, PORT, 10, userName, "123456");
    _p->run("getCurrentSessionAndUser()[1]", 0);
    while (!_p->isFinished(0)){
        dolphindb::Util::sleep(500);
    }
    ASSERT_EQ(_p->getData(0)->getString(), userName);
    _p->shutDown();
    std::cout << "test_connectionPool_SCRAM passed" << std::endl;
}

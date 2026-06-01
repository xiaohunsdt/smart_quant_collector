#include <gtest/gtest.h>
#include "config.h"
#include "MultithreadedTableWriter.h"

class HighAvailableTest : public testing::Test
{
    public:
        static dolphindb::DBConnection conn;
        //Suite
        static void SetUpTestSuite() {
            bool ret = conn.connect(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER, "", true, sites, 7200, true);
            if (!ret)
            {
                std::cout << "Failed to connect to the server" << std::endl;
            }
            else
            {
                std::cout << "connect to " + HOST_CLUSTER + ":" + std::to_string(PORT_DNODE1) << std::endl;
            }
            conn.run("try{createUser('mtw_test','123456')}catch(ex){};go");
        }
        static void stopNode(const std::string& host, const int& controllerPort, const std::string& nodeName);
        static void restartNode(const std::string& host, const int& controllerPort, const std::string& nodeName);
        static void restartAllNodes(const std::string& host, const int& controllerPort);

    protected:
        // Case
        virtual void SetUp()
        {
            restartAllNodes(HOST_CLUSTER, PORT_CONTROLLER);
        }
        virtual void TearDown()
        {
            restartAllNodes(HOST_CLUSTER, PORT_CONTROLLER);
        }

};
dolphindb::DBConnection HighAvailableTest::conn(false, false);

void HighAvailableTest::stopNode(const std::string& host, const int& controllerPort, const std::string& nodeName)
{
    std::cout << "Try to stop current Datanode: " + nodeName << std::endl;
    dolphindb::DBConnection _conn(false, false);
    _conn.connect(host, controllerPort, USER_CLUSTER, PASSWD_CLUSTER);

    int status = _conn.run("(exec state from getClusterPerf() where name = '" + nodeName + "')[0]")->getInt();
    int try_count = 0;
    while (status == 1 && try_count < 60) {
        _conn.run("try{stopDataNode(\"" + nodeName + "\");}catch(ex){}");
        status = _conn.run("(exec state from getClusterPerf() where name = '" + nodeName + "')[0]")->getInt();
        try_count++;
        dolphindb::Util::sleep(1000);
    }
    if (status == 1) {
        // std::cout << "Stop failed." << std::endl;
        throw std::runtime_error("Stop failed after 60 seconds.");
    }else{
        std::cout << "Stop successfully." << std::endl;
    }
    _conn.close();
}

void HighAvailableTest::restartNode(const std::string& host, const int& controllerPort, const std::string& nodeName)
{
    dolphindb::DBConnection _conn(false, false);
    _conn.connect(host, controllerPort, USER_CLUSTER, PASSWD_CLUSTER);
    std::cout << "Restart the Datanode: " + nodeName << std::endl;

    int state = _conn.run("(exec state from getClusterPerf() where name = '" + nodeName + "')[0]")->getInt();
    int try_count = 0;
    while (state != 1 && try_count < 60) {
        _conn.run("startDataNode(\"" + nodeName + "\")");
        state = _conn.run("(exec state from getClusterPerf() where name = '" + nodeName + "')[0]")->getInt();
        dolphindb::Util::sleep(1000);
        try_count++;
    }
    if (state != 1) {
        // std::cout << "Restart failed." << std::endl;
        throw std::runtime_error("Restart failed after 60 seconds.");
    }else{
        std::cout << "Restart successfully." << std::endl;
    }
    _conn.close();
}

void HighAvailableTest::restartAllNodes(const std::string& host, const int& controllerPort)
{
    dolphindb::DBConnection _conn(false, false);
    _conn.connect(host, controllerPort, USER_CLUSTER, PASSWD_CLUSTER);

    if (_conn.run("all((exec state from getClusterPerf() where mode in 0 4) == 1)")->getBool()){
        return;
    }

    dolphindb::VectorSP nodes = _conn.run("nodes = exec name from getClusterPerf() where mode in 0 4 and state = 0;nodes");
    bool state = _conn.run("all((exec state from getClusterPerf()) == 1)")->getBool();

    int try_count = 0;
    while (try_count < 60 && state != 1) {
        std::cout << "Restart the Datanode: " + nodes->getString() << std::endl;
        _conn.run("startDataNode(nodes)");
        state = _conn.run("all((exec state from getClusterPerf()) == 1)")->getBool();
        if (state){
            break;
        }
        try_count++;
        dolphindb::Util::sleep(1000);
    }
    if (state != 1) {
        // std::cout << "Restart failed." << std::endl;
        throw std::runtime_error("Restart failed after 60 seconds.");
    }else{
        std::cout << "Restart successfully." << std::endl;
    }
    _conn.close();
}

TEST_F(HighAvailableTest, test_connHA_basic_should_serial){
    dolphindb::DBConnectionSP controller = new dolphindb::DBConnection(false, false);
    controller->connect(HOST_CLUSTER, PORT_CONTROLLER, USER_CLUSTER, PASSWD_CLUSTER);
    controller->run("tmp = select name,port from getClusterPerf() where mode in 0 4");
    dolphindb::VectorSP ports = controller->run("tmp.port");
    srand((int)time(NULL));
    int index = rand() % ports->size();
    int nodePort = ports->getInt(index);
    std::string nodeName = controller->run("exec name from getClusterPerf() where mode in 0 4 and port = " + std::to_string(nodePort))->get(0)->getString();
    controller->close();
    std::cout << "selected node to connect: " << nodeName << std::endl;

    std::string init_script = "temp=table(`1`2`3 as col1, 1 2 3 as col2);";
    dolphindb::DBConnection connImp(false, false);
    connImp.connect(HOST_CLUSTER, nodePort, USER_CLUSTER, PASSWD_CLUSTER, init_script, true, sites, 7200, false);

    std::string first_node = connImp.run("getNodeAlias()")->getString();

    std::cout << "Current datanode is " + first_node << std::endl;
    dolphindb::Util::sleep(2000);
    stopNode(HOST_CLUSTER, PORT_CONTROLLER, first_node);

    std::string second_node = connImp.run("getNodeAlias()")->getString();
    ASSERT_TRUE(first_node != second_node);
    ASSERT_TRUE(connImp.run("`temp in objs(true).name")->getBool());
    std::cout << "Now datanode has changed to " + second_node << std::endl;
    dolphindb::Util::sleep(2000);
    stopNode(HOST_CLUSTER, PORT_CONTROLLER, second_node);

    std::string third_node = connImp.run("getNodeAlias()")->getString();
    ASSERT_TRUE(third_node != second_node);
    ASSERT_TRUE(connImp.run("`temp in objs(true).name")->getBool());
    std::cout << "Now datanode has changed to " + third_node << std::endl;
    restartAllNodes(HOST_CLUSTER, PORT_CONTROLLER);
    connImp.close();
}

TEST_F(HighAvailableTest, test_MTW_HA_should_serial)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    const int test_rows = 100;
    dolphindb::DBConnection connImp(false, false);
    connImp.connect(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER, "", false, std::vector<std::string>(), 7200, true);
    std::string node_name = connImp.run("getNodeAlias()")->getString();

    std::string script = "dbName ='"+dbName+"';"
                    "if(exists(dbName)){\n"
                    "\tdropDatabase(dbName)\t\n"
                    "}\n"
                    "db  = database(dbName, HASH,[TIME, 4]);\n";
    script += "t = table(1000:0, `symbol`date`value,[SYMBOL, TIME, INT]);"
              "pt = db.createPartitionedTable(t,`pt,`date);";
    connImp.run(script);
    dolphindb::SmartPointer<dolphindb::MultithreadedTableWriter> mulwrite;
    dolphindb::ErrorCodeInfo pErrorInfo;
    mulwrite = new dolphindb::MultithreadedTableWriter(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER, dbName, "pt", false, true, &sites, 100, 1, 1, "date");
    std::string sym[] = {"A", "B", "C", "D"};
    for (auto i = 0; i < test_rows; i++)
    {   
        ASSERT_TRUE(mulwrite->insert(pErrorInfo, sym[rand() % 4], i + 200, i + 99));
        if (i == test_rows / 2)
        {
            stopNode(HOST_CLUSTER, PORT_CONTROLLER, node_name);
        }
    }
    mulwrite->waitForThreadCompletion();
    restartNode(HOST_CLUSTER, PORT_CONTROLLER, node_name);
    dolphindb::ConstantSP t1 = connImp.run("select * from loadTable('"+dbName+"',`pt) order by date;");
    ASSERT_LE(t1->rows(), test_rows);
    connImp.close();
}

TEST_F(HighAvailableTest, test_MTW_HA_2_should_serial)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    const int test_rows = 100;
    dolphindb::DBConnection connImp(false, false);
    connImp.connect(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER, "", true, sites, 7200, false);
    std::string node_name = connImp.run("getNodeAlias()")->getString();
    std::string script = "dbName = '"+dbName+"';"
                    "if(exists(dbName)){\n"
                    "\tdropDatabase(dbName)\t\n"
                    "}\n"
                    "db  = database(dbName, HASH,[TIME, 4]);\n";
    script += "t = table(1000:0, `symbol`date`value,[SYMBOL, TIME, INT]);"
              "pt = db.createPartitionedTable(t,`pt,`date);";
    connImp.run(script);
    dolphindb::SmartPointer<dolphindb::MultithreadedTableWriter> mulwrite;
    dolphindb::ErrorCodeInfo pErrorInfo;
    mulwrite = new dolphindb::MultithreadedTableWriter(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER, dbName, "pt", false, true, &sites, 1000, 1, 4, "date");
    std::string sym[] = {"A", "B", "C", "D"};
    std::vector<std::vector<dolphindb::ConstantSP> *> datas;
    srand((int)time(NULL));
    for (int i = 0; i < test_rows; i++)
    {
        std::vector<dolphindb::ConstantSP> *prow = new std::vector<dolphindb::ConstantSP>;
        std::vector<dolphindb::ConstantSP> &row = *prow;
        row.push_back(dolphindb::Util::createString(sym[rand() % 4]));
        row.push_back(dolphindb::Util::createTime(i));
        row.push_back(dolphindb::Util::createInt(i + 64));
        datas.push_back(prow);
    }
    dolphindb::MultithreadedTableWriter::Status status;
    std::thread th1 = std::thread([&]
                             { mulwrite->insertUnwrittenData(datas, pErrorInfo); });
    std::thread th2 = std::thread([&]{
        do{
            mulwrite->getStatus(status);
            if(status.sentRows >= test_rows/2){
                stopNode(HOST_CLUSTER, PORT_CONTROLLER, node_name);
                break;
            }
        }while (true); 
    });

    th2.join();
    th1.join();

    mulwrite->waitForThreadCompletion();
    ASSERT_FALSE(pErrorInfo.hasError());

    restartNode(HOST_CLUSTER, PORT_CONTROLLER, node_name);
    ASSERT_EQ(connImp.run("exec count(*) from loadTable(\""+dbName+"\", `pt)")->getInt(),test_rows);
    connImp.close();
}

TEST_F(HighAvailableTest, test_MTW_HA_3_should_serial)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    const int test_rows = 100;
    dolphindb::DBConnection connImp(false, false);
    connImp.connect(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER, "", true, sites, 7200, false);
    std::string node_name = connImp.run("getNodeAlias()")->getString();
    std::string script = "dbName = \""+dbName+"\"\n"
                    "if(exists(dbName)){\n"
                    "\tdropDatabase(dbName)\t\n"
                    "}\n"
                    "db  = database(dbName, HASH,[TIME, 4]);\n";
    script += "t = table(1000:0, `symbol`date`value,[SYMBOL, TIME, INT]);"
              "pt = db.createPartitionedTable(t,`pt,`date);"
              "grant(`mtw_test, TABLE_WRITE, '"+dbName+"/pt');grant(`mtw_test, TABLE_READ, '"+dbName+"/pt');";
    connImp.run(script);
    dolphindb::SmartPointer<dolphindb::MultithreadedTableWriter> mulwrite;
    dolphindb::ErrorCodeInfo pErrorInfo;
    mulwrite = new dolphindb::MultithreadedTableWriter(HOST_CLUSTER, PORT_DNODE1, "mtw_test", PASSWD_CLUSTER, dbName, "pt", false, true, &sites, 100, 1, 1, "date");
    std::string sym[] = {"A", "B", "C", "D"};

    for (auto i = 0; i < test_rows; i++)
    {
        ASSERT_TRUE(mulwrite->insert(pErrorInfo, sym[rand() % 4], i + 200, i + 99));
        if (i % 50 == 0)
        {
            std::cout << "try to close MTW sessions" << std::endl;
            connImp.run("closeSessions(exec sessionId from getSessionMemoryStat() where userId = `mtw_test)");
        }
    }
    mulwrite->waitForThreadCompletion();
    ASSERT_FALSE(pErrorInfo.hasError());

    restartNode(HOST_CLUSTER, PORT_CONTROLLER, node_name);
    dolphindb::ConstantSP t1 = connImp.run("select * from loadTable('"+dbName+"',`pt) order by date;");
    ASSERT_EQ(t1->rows(), test_rows);
    connImp.close();
}

TEST_F(HighAvailableTest, test_DBConnectionPool_HA_should_serial)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    const int test_rows = 100000;
    dolphindb::DBConnectionPool pool(HOST_CLUSTER, PORT_DNODE1, 10, USER_CLUSTER, PASSWD_CLUSTER, false, true);
    dolphindb::DBConnection connImp(false, false);
    connImp.connect(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER, "", true, sites, 7200, false);
    std::string node_name = connImp.run("getNodeAlias()")->getString();
    std::string script;
    script += "dbPath = '"+dbName+"';";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,HASH,[DATE,2]);";
    script += "dummy = table(1:0, `id`sym`date`value, [INT, SYMBOL, DATE, INT]);";
    script += "pt = db.createPartitionedTable(dummy,`pt,`date);";
    connImp.run(script);

    std::vector<std::string> colName = {"id", "sym", "date", "value"};
    std::vector<dolphindb::ConstantSP> cols = {};

    dolphindb::VectorSP col1 = dolphindb::Util::createVector(dolphindb::DT_INT, 0, test_rows);
    dolphindb::VectorSP col2 = dolphindb::Util::createVector(dolphindb::DT_SYMBOL, 0, test_rows);
    dolphindb::VectorSP col3 = dolphindb::Util::createVector(dolphindb::DT_DATE, 0, test_rows);
    dolphindb::VectorSP col4 = dolphindb::Util::createVector(dolphindb::DT_INT, 0, test_rows);

    for (auto i = 0; i < test_rows; i++)
    {
        col1->append(dolphindb::Util::createInt(i));
        col2->append(dolphindb::Util::createString("symbol1"));
        col3->append(dolphindb::Util::createDate(i));
        col4->append(dolphindb::Util::createInt(i + 100));
    }

    cols.push_back(col1);
    cols.push_back(col2);
    cols.push_back(col3);
    cols.push_back(col4);

    dolphindb::TableSP tab = dolphindb::Util::createTable(colName, cols);

    std::vector<dolphindb::ConstantSP> args{tab};
    pool.run("tableInsert{loadTable('"+dbName+"', `pt)}", args, 1);
    int rows = 0;
    while (!pool.isFinished(1))
    {
        rows = connImp.run("exec count(*) from loadTable(\""+dbName+"\", `pt)")->getInt();
        if (rows >= test_rows / 2)
        {
            stopNode(HOST_CLUSTER, PORT_CONTROLLER, node_name);
            break;
        }
    };

    ASSERT_TRUE(pool.getData(1)->getBool());

    restartNode(HOST_CLUSTER, PORT_CONTROLLER, node_name);
    connImp.upload("tab", tab);
    dolphindb::ConstantSP res = connImp.run("res = exec * from loadTable('"+dbName+"', `pt) order by id;each(eqObj,res.values(),tab.values())");
    for (int i = 0; i < res->size(); i++)
    {
        ASSERT_TRUE(res->get(i)->getBool());
    }

    connImp.close();
    pool.shutDown();
}

TEST_F(HighAvailableTest, test_MTW_new_HA_should_serial)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    const int test_rows = 1000000;
    std::shared_ptr<dolphindb::DBConnection> pConn = std::make_shared<dolphindb::DBConnection>(false, false);
    pConn->connect(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER, "", true, sites, 7200, false);
    std::string script = "dbName = '"+dbName+"';"
                    "if(exists(dbName)){\n"
                    "\tdropDatabase(dbName)\t\n"
                    "}\n"
                    "db  = database(dbName, HASH,[TIME, 4]);\n";
    script += "t = table(1000:0, `symbol`date`value,[SYMBOL, TIME, INT]);"
              "pt = db.createPartitionedTable(t,`pt,`date);";
    pConn->run(script);
    dolphindb::MTWConfig config(pConn, "loadTable('"+dbName+"',`pt)");
    int ConnectedOne_called = 0, ConnectedAll_called = 0, ReconnectingOne_called = 0, ReconnectingAll_called = 0;

    auto func = [&](const dolphindb::MTWState state, const std::string &host, const int port) { 
        if (state == dolphindb::MTWState::Initializing) // 内部用来跟其他状态区分的
        {
            std::cout << "MTW initializing" << std::endl;
        }else if (state == dolphindb::MTWState::ConnectedOne){
            std::cout << "MTW connected one at " << host << ":" << port << std::endl;
            ConnectedOne_called++;
        }else if (state == dolphindb::MTWState::ConnectedAll){
            std::cout << "MTW connected all at " << host << ":" << port << std::endl;
            ConnectedAll_called++;
        }else if (state == dolphindb::MTWState::ReconnectingOne){
            std::cout << "MTW reconnecting one at " << host << ":" << port << std::endl;
            ReconnectingOne_called++;
        }else if (state == dolphindb::MTWState::ReconnectingAll){
            std::cout << "MTW reconnecting all at " << host << ":" << port << std::endl;
            ReconnectingAll_called++;
        }else if (state == dolphindb::MTWState::Terminated){ // 内部用来跟其他状态区分的
            std::cout << "MTW terminated" << std::endl;
        }
        return true; 
    };
    config.setThreads(4, "date").setBatching(100, std::chrono::seconds(1)).onConnectionStateChange(func);
    dolphindb::SmartPointer<dolphindb::MultithreadedTableWriter> mulwrite = new dolphindb::MultithreadedTableWriter(config);
    dolphindb::ErrorCodeInfo pErrorInfo;
    std::string sym[] = {"A", "B", "C", "D"};

    for (auto i = 0; i < test_rows; i++)
    {
        ASSERT_TRUE(mulwrite->insert(pErrorInfo, sym[rand() % 4], i + 200, i + 99));
        if (i % 500000 == 0)
        {
            for (auto &node : nodeNames){
                stopNode(HOST_CLUSTER, PORT_CONTROLLER, node);
            }
            dolphindb::Util::sleep(1000);
            restartAllNodes(HOST_CLUSTER, PORT_CONTROLLER);
        }
    }
    mulwrite->waitForThreadCompletion();
    ASSERT_FALSE(pErrorInfo.hasError());
    
    dolphindb::ConstantSP t1 = pConn->run("select * from loadTable('"+dbName+"',`pt) order by date;");
    ASSERT_EQ(t1->rows(), test_rows);
    pConn->close();
    ASSERT_GE(ConnectedOne_called, 1);
    ASSERT_GE(ConnectedAll_called, 1);
    ASSERT_GE(ReconnectingOne_called, 1);
    ASSERT_GE(ReconnectingAll_called, 1);
}

TEST_F(HighAvailableTest, test_MTW_new_HA_2_should_serial)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    const int test_rows = 1000000;
    std::shared_ptr<dolphindb::DBConnection> pConn = std::make_shared<dolphindb::DBConnection>(false, false);
    pConn->connect(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER, "", true, sites, 7200, false);
    std::string script = "dbName = '"+dbName+"';"
                    "if(exists(dbName)){\n"
                    "\tdropDatabase(dbName)\t\n"
                    "}\n"
                    "db  = database(dbName, HASH,[TIME, 4]);\n";
    script += "t = table(1000:0, `symbol`date`value,[SYMBOL, TIME, INT]);"
              "pt = db.createPartitionedTable(t,`pt,`date);";
    pConn->run(script);
    dolphindb::MTWConfig config(pConn, "loadTable('"+dbName+"',`pt)");
    int ConnectedOne_called = 0, ConnectedAll_called = 0, ReconnectingOne_called = 0, ReconnectingAll_called = 0;

    auto func = [&](const dolphindb::MTWState state, const std::string &host, const int port) { 
        if (state == dolphindb::MTWState::Initializing) // 内部用来跟其他状态区分的
        {
            std::cout << "MTW initializing" << std::endl;
        }else if (state == dolphindb::MTWState::ConnectedOne){
            std::cout << "MTW connected one at " << host << ":" << port << std::endl;
            ConnectedOne_called++;
        }else if (state == dolphindb::MTWState::ConnectedAll){
            std::cout << "MTW connected all at " << host << ":" << port << std::endl;
            ConnectedAll_called++;
        }else if (state == dolphindb::MTWState::ReconnectingOne){
            std::cout << "MTW reconnecting one at " << host << ":" << port << std::endl;
            ReconnectingOne_called++;
        }else if (state == dolphindb::MTWState::ReconnectingAll){
            std::cout << "MTW reconnecting all at " << host << ":" << port << std::endl;
            ReconnectingAll_called++;
        }else if (state == dolphindb::MTWState::Terminated){ // 内部用来跟其他状态区分的
            std::cout << "MTW terminated" << std::endl;
        }
        return true; 
    };
    config.setThreads(4, "date").setBatching(1000, std::chrono::seconds(1)).onConnectionStateChange(func);
    dolphindb::SmartPointer<dolphindb::MultithreadedTableWriter> mulwrite = new dolphindb::MultithreadedTableWriter(config);
    dolphindb::ErrorCodeInfo pErrorInfo;
    std::string sym[] = {"A", "B", "C", "D"};
    std::vector<std::vector<dolphindb::ConstantSP> *> datas, datas2;
    srand((int)time(NULL));
    for (int i = 0; i < test_rows / 2; i++)
    {
        std::vector<dolphindb::ConstantSP> *prow = new std::vector<dolphindb::ConstantSP>;
        std::vector<dolphindb::ConstantSP> &row = *prow;
        row.push_back(dolphindb::Util::createString(sym[rand() % 4]));
        row.push_back(dolphindb::Util::createTime(i));
        row.push_back(dolphindb::Util::createInt(i + 64));
        datas.push_back(prow);
    }
    for (int i = test_rows / 2; i < test_rows; i++)
    {
        std::vector<dolphindb::ConstantSP> *prow = new std::vector<dolphindb::ConstantSP>;
        std::vector<dolphindb::ConstantSP> &row = *prow;
        row.push_back(dolphindb::Util::createString(sym[rand() % 4]));
        row.push_back(dolphindb::Util::createTime(i));
        row.push_back(dolphindb::Util::createInt(i + 64));
        datas2.push_back(prow);
    }
    dolphindb::MultithreadedTableWriter::Status status;
    mulwrite->insertUnwrittenData(datas, pErrorInfo);
    for (auto &node : nodeNames){
        stopNode(HOST_CLUSTER, PORT_CONTROLLER, node);
    }
    dolphindb::Util::sleep(1000);
    restartAllNodes(HOST_CLUSTER, PORT_CONTROLLER);
    mulwrite->insertUnwrittenData(datas2, pErrorInfo);

    mulwrite->waitForThreadCompletion();
    ASSERT_FALSE(pErrorInfo.hasError());

    ASSERT_EQ(pConn->run("exec count(*) from loadTable(\""+dbName+"\", `pt)")->getInt(), test_rows);
    pConn->close();
    ASSERT_GE(ConnectedOne_called, 1);
    ASSERT_GE(ConnectedAll_called, 1);
    ASSERT_GE(ReconnectingOne_called, 1);
    ASSERT_GE(ReconnectingAll_called, 1);
}

TEST_F(HighAvailableTest, test_MTW_new_HA_3_should_serial)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    const int test_rows = 1000000;
    std::shared_ptr<dolphindb::DBConnection> pConn = std::make_shared<dolphindb::DBConnection>(false, false);
    pConn->connect(HOST, PORT, "mtw_test", PASSWD, "", true, sites, 7200, false);
    dolphindb::DBConnection connImp(false, false);
    connImp.connect(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER);
    std::string script = "dbName = '"+dbName+"';"
                    "if(exists(dbName)){\n"
                    "\tdropDatabase(dbName)\t\n"
                    "}\n"
                    "db  = database(dbName, HASH,[TIME, 4]);\n";
    script += "t = table(1000:0, `symbol`date`value,[SYMBOL, TIME, INT]);"
              "pt = db.createPartitionedTable(t,`pt,`date);"
              "grant(`mtw_test, TABLE_WRITE, '"+dbName+"/pt');grant(`mtw_test, TABLE_READ, '"+dbName+"/pt');";
    connImp.run(script);
    dolphindb::MTWConfig config(pConn, "loadTable('"+dbName+"',`pt)");

    config.setThreads(4, "date").setBatching(100, std::chrono::seconds(1));
    dolphindb::SmartPointer<dolphindb::MultithreadedTableWriter> mulwrite = new dolphindb::MultithreadedTableWriter(config);
    dolphindb::ErrorCodeInfo pErrorInfo;
    std::string sym[] = {"A", "B", "C", "D"};

    for (auto i = 0; i < test_rows; i++)
    {
        ASSERT_TRUE(mulwrite->insert(pErrorInfo, sym[rand() % 4], i + 200, i + 99));
        if (i % 500000 == 0)
        {
            std::cout << "try to close MTW sessions" << std::endl;
            connImp.run("closeSessions(exec sessionId from getSessionMemoryStat() where userId = `mtw_test)");
        }
    }
    mulwrite->waitForThreadCompletion();
    ASSERT_FALSE(pErrorInfo.hasError());

    dolphindb::ConstantSP t1 = connImp.run("select * from loadTable('"+dbName+"',`pt) order by date;");
    ASSERT_EQ(t1->rows(), test_rows);
    pConn->close();
    connImp.close();
}

struct CallBackStruct{
    int state;
    std::string host;
    int port;

    bool operator==(const CallBackStruct& other) const {
        return state==other.state && host==other.host && port==other.port;
    }
};

std::ostream& operator<<(std::ostream& os, const CallBackStruct& p) {
    os << (int)p.state <<" "<< p.host <<":"<< p.port;
    return os;
}

std::vector<CallBackStruct> callback_vector;

bool callback(const dolphindb::ConnectionState state, const std::string &host, const int port){
    callback_vector.emplace_back(CallBackStruct{(int)state, host, port});
    std::cout << (int)state <<" "<< host <<":"<< port<<std::endl;
    return true;
};

TEST_F(HighAvailableTest, test_connect_callback_should_serial){
    dolphindb::DBConnection conn(false, false);
    conn.onConnectionStateChange(callback);
    conn.connect(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER, "", false, std::vector<std::string>(), 7200, true);
    std::string node = "dnode"+std::to_string(PORT_DNODE1);
    stopNode(HOST_CLUSTER, PORT_CONTROLLER, node);
    restartNode(HOST_CLUSTER, PORT_CONTROLLER, node);
    conn.run("'connect success'");
    conn.close();
    ASSERT_EQ(callback_vector.size(), 3);
    CallBackStruct conn_callback{1, HOST_CLUSTER, PORT_DNODE1};
    CallBackStruct re_conn_callback{3, HOST_CLUSTER, PORT_DNODE1};
    ASSERT_EQ(callback_vector[0], conn_callback);
    ASSERT_EQ(callback_vector[1], re_conn_callback);
    ASSERT_EQ(callback_vector[2], conn_callback);
    callback_vector.clear();
}

TEST_F(HighAvailableTest, test_connect_ha_callback_should_serial){
    GTEST_SKIP() << "cannot auto test";
    dolphindb::DBConnection _conn(false, false);
    _conn.connect(HOST_CLUSTER, PORT_CONTROLLER, USER_CLUSTER, PASSWD_CLUSTER);
    dolphindb::DBConnection conn(false, false);
    conn.onConnectionStateChange(callback);
    conn.connect(HOST_CLUSTER, PORT_DNODE1, USER_CLUSTER, PASSWD_CLUSTER, "", true, sites, 7200, true);
    std::string node = conn.run("getNodeAlias()")->getString();
    int port = conn.run("getNodePort()")->getInt();
    stopNode(HOST_CLUSTER, PORT_CONTROLLER, node);
    std::cout << conn.run("getNodeAlias()")->getString() << std::endl;
    restartNode(HOST_CLUSTER, PORT_CONTROLLER, node);
    conn.close();
    restartNode(HOST_CLUSTER, PORT_CONTROLLER, node);
}
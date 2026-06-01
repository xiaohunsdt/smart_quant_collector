#include <gtest/gtest.h>
#include "config.h"
#include <fstream>

class DolphinDBTest:public testing::Test
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

dolphindb::DBConnection DolphinDBTest::conn(false, false);

int64_t getTimeStampMs() {
    return dolphindb::Util::getEpochTime();
}

static std::string getSymbolVector(const std::string& name, int size)
{
    int kind = 50;
    int count = size / kind;

    std::string result;
    char temp[200];
    result += name;
    sprintf(temp, "=symbol(array(STRING,%d,%d,`0));", count, count);
    result += temp;
    for (int i = 1;i<kind;i++) {
        sprintf(temp, ".append!(symbol(array(STRING,%d,%d,`%d)));", count, count, i);
        result += name;
        result += std::string(temp);
    }

    return result;
}

TEST(DolphinDBDataTypeTest,testDataTypeWithoutConnect){
        dolphindb::VectorSP arrayVector = dolphindb::Util::createArrayVector(dolphindb::DT_DATETIME_ARRAY, 0, 100);
        for (int i = 0; i < 10; i++) {
            dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 5);
            for (int j = 0; j < 5; j++) {
                time->set(j, dolphindb::Util::createDateTime(j * 100000));
            }
            arrayVector->append(time);
        }
        std::cout<< arrayVector->getString()<<std::endl;

        dolphindb::ConstantSP intval= dolphindb::Util::createConstant(dolphindb::DT_INT);
        intval->setInt(1);
        ASSERT_EQ(intval->getInt(),1);

        dolphindb::ConstantSP boolval= dolphindb::Util::createConstant(dolphindb::DT_BOOL);
        boolval->setBool(1);
        ASSERT_EQ(boolval->getBool(),true);

        dolphindb::ConstantSP floatval= dolphindb::Util::createConstant(dolphindb::DT_FLOAT);
        floatval->setFloat(2.33);
        ASSERT_EQ(floatval->getFloat(),(float)2.33);

        dolphindb::ConstantSP longval= dolphindb::Util::createConstant(dolphindb::DT_LONG);
        longval->setLong(10000000);
        ASSERT_EQ(longval->getLong(),(long)10000000);

        dolphindb::ConstantSP stringval= dolphindb::Util::createConstant(dolphindb::DT_STRING);
        stringval->setString("134asd");
        ASSERT_EQ(stringval->getString(),"134asd");

        dolphindb::ConstantSP dateval= dolphindb::Util::createConstant(dolphindb::DT_DATE);
        dateval=dolphindb::Util::createDate(1);
        ASSERT_EQ(dateval->getString(),"1970.01.02");

        dolphindb::ConstantSP minuteval= dolphindb::Util::createMinute(1439);
        ASSERT_EQ(minuteval->getString(),"23:59m");

        dolphindb::ConstantSP nanotimestampval= dolphindb::Util::createNanoTimestamp((long long)100000000000000000);
        ASSERT_EQ(nanotimestampval->getString(),"1973.03.03T09:46:40.000000000");

        dolphindb::ConstantSP uuidval= dolphindb::Util::parseConstant(dolphindb::DT_UUID,"5d212a78-cc48-e3b1-4235-b4d91473ee87");
        ASSERT_EQ(uuidval->getString(),"5d212a78-cc48-e3b1-4235-b4d91473ee87");

        dolphindb::ConstantSP ipaddrval= dolphindb::Util::parseConstant(dolphindb::DT_IP,"192.168.0.16");
        ASSERT_EQ(ipaddrval->getString(),"192.168.0.16");

        std::vector<std::string> colname={"col1","col2","col3"};
        std::vector<dolphindb::DATA_TYPE> coltype={dolphindb::DT_INT,dolphindb::DT_BLOB, dolphindb::DT_SYMBOL};
        dolphindb::TableSP tableval= dolphindb::Util::createTable(colname,coltype,0,3);
        std::cout<< tableval->getString()<<std::endl;    
}

TEST_F(DolphinDBTest,testSymbol){
    std::vector<std::string> expectResults = { "XOM","y" };
    std::string script;
    script += "x=`XOM`y;y=symbol x;y;";
    dolphindb::ConstantSP result = conn.run(script);
    for (unsigned int i = 0;i < expectResults.size(); i++) {
        ASSERT_EQ(result->getString(i), expectResults[i]);
    }
}

TEST_F(DolphinDBTest,testSymbolBase){
    int64_t startTime, time;

    conn.run("v=symbol(string(1..2000000))");
    startTime = getTimeStampMs();
    conn.run("v");
    time = getTimeStampMs() - startTime;
    std::cout << "symbol std::vector: " << time << "ms" << std::endl;

    conn.run("undef(all)");
    conn.run(getSymbolVector("v", 2000000));
    startTime = getTimeStampMs();
    conn.run("v");
    time = getTimeStampMs() - startTime;
    std::cout << "symbol std::vector optimize:" << time << "ms" << std::endl;

    conn.run("undef(all)");
    conn.run("t=table(symbol(string(1..2000000)) as sym)");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "table with one symbol std::vector:" << time << "ms" << std::endl;

    conn.run("undef(all)");
    conn.run(getSymbolVector("v", 2000000));
    conn.run("t=table(v as sym)");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "table with one symbol std::vector optimize:" << time << "ms" << std::endl;

    conn.run("undef(all)");
    conn.run("t=table(symbol(string(1..2000000)) as sym,symbol(string(1..2000000)) as sym1,symbol(string(1..2000000)) as sym2,symbol(string(1..2000000)) as sym3,symbol(string(1..2000000)) as sym4,symbol(string(1..2000000)) as sym5,symbol(string(1..2000000)) as sym6,symbol(string(1..2000000)) as sym7,symbol(string(1..2000000)) as sym8,symbol(string(1..2000000)) as sym9)");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "table with same symbol vectors:" << time << "ms" << std::endl;

    conn.run("undef(all)");
    conn.run("t=table(symbol(take(string(0..20000),2000000)) as sym,symbol(take(string(20000..40000),2000000)) as sym1,symbol(take(string(40000..60000),2000000)) as sym2,symbol(take(string(60000..80000),2000000)) as sym3,symbol(take(string(80000..100000),2000000)) as sym4,symbol(take(string(100000..120000),2000000)) as sym5,symbol(take(string(120000..140000),2000000)) as sym6,symbol(take(string(140000..160000),2000000)) as sym7,symbol(take(string(160000..180000),2000000)) as sym8,symbol(take(string(180000..200000),2000000)) as sym9)");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "table with diff symbol vectors:" << time << "ms" << std::endl;

    conn.run("undef(all)");
    conn.run("d=dict(symbol(string(1..2000000)),symbol(string(1..2000000)))");
    startTime = getTimeStampMs();
    conn.run("d");
    time = getTimeStampMs() - startTime;
    std::cout << "symbol dict:" << time << "ms" << std::endl;

    conn.run("undef(all)");
    conn.run("s=set(symbol(string(1..2000000)))");
    startTime = getTimeStampMs();
    conn.run("s");
    time = getTimeStampMs() - startTime;
    std::cout << "symbol set:" << time << "ms" << std::endl;

    conn.run("undef(all)");
    conn.run("t=(symbol(string(1..2000000)),symbol(string(1..2000000)))");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "std::tuple symbol std::tuple:" << time << "ms" << std::endl;

    conn.run("undef(all)");
}

TEST_F(DolphinDBTest,testSymbolSmall){
    int64_t startTime, time;
    conn.run("v=symbol(string(1..200))");
    startTime = getTimeStampMs();
    conn.run("v");
    time = getTimeStampMs() - startTime;
    std::cout << "symbol std::vector:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run(getSymbolVector("v", 200));
    startTime = getTimeStampMs();
    conn.run("v");
    time = getTimeStampMs() - startTime;
    std::cout << "symbol std::vector optimize:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("t=table(symbol(string(1..200)) as sym)");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "table with one symbol std::vector:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run(getSymbolVector("v", 200));
    conn.run("t=table(v as sym)");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "table with one symbol std::vector optimize:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("t=table(symbol(string(1..200)) as sym,symbol(string(1..200)) as sym1,symbol(string(1..200)) as sym2,symbol(string(1..200)) as sym3,symbol(string(1..200)) as sym4,symbol(string(1..200)) as sym5,symbol(string(1..200)) as sym6,symbol(string(1..200)) as sym7,symbol(string(1..200)) as sym8,symbol(string(1..200)) as sym9)");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "table with same symbol vectors:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("t=table(symbol(take(string(0..20000),200)) as sym,symbol(take(string(20000..40000),200)) as sym1,symbol(take(string(40000..60000),200)) as sym2,symbol(take(string(60000..80000),200)) as sym3,symbol(take(string(80000..100000),200)) as sym4,symbol(take(string(100000..120000),200)) as sym5,symbol(take(string(120000..140000),200)) as sym6,symbol(take(string(140000..160000),200)) as sym7,symbol(take(string(160000..180000),200)) as sym8,symbol(take(string(180000..200000),200)) as sym9)");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "table with diff symbol vectors:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("d=dict(symbol(string(1..200)),symbol(string(1..200)))");
    startTime = getTimeStampMs();
    conn.run("d");
    time = getTimeStampMs() - startTime;
    std::cout << "symbol dict:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("s=set(symbol(string(1..200)))");
    startTime = getTimeStampMs();
    conn.run("s");
    time = getTimeStampMs() - startTime;
    std::cout << "symbol set:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("t=(symbol(string(1..200)),symbol(string(1..200)))");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "std::tuple symbol std::tuple:" << time << "ms" << std::endl;
    conn.run("undef(all)");
}

TEST_F(DolphinDBTest,testSymbolNull){
    int64_t startTime, time;
    conn.run("v=take(symbol(`cy`fty``56e`f65dfyfv),2000000)");
    startTime = getTimeStampMs();
    conn.run("v");
    time = getTimeStampMs() - startTime;
    std::cout << "symbol std::vector:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("t=table(take(symbol(`cy`fty``56e`f65dfyfv),2000000) as sym)");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "table with one symbol std::vector:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("t=table(take(symbol(`cy`fty``56e`f65dfyfv),2000000) as sym,take(symbol(`cy`fty``56e`f65dfyfv),2000000) as sym1,take(symbol(`cy`fty``56e`f65dfyfv),2000000) as sym2,take(symbol(`cy`fty``56e`f65dfyfv),2000000) as sym3,take(symbol(`cy`fty``56e`f65dfyfv),2000000) as sym4,take(symbol(`cy`fty``56e`f65dfyfv),2000000) as sym5,take(symbol(`cy`fty``56e`f65dfyfv),2000000) as sym6,take(symbol(`cy`fty``56e`f65dfyfv),2000000) as sym7,take(symbol(`cy`fty``56e`f65dfyfv),2000000) as sym8,take(symbol(`cy`fty``56e`f65dfyfv),2000000) as sym9)");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "table with same symbol vectors:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("t=table(take(symbol(`cdwy`fty``56e`f652dfyfv),2000000) as sym,take(symbol(`cy`f8ty``56e`f65dfyfv),2000000) as sym1,take(symbol(`c2587y`fty``56e`f65dfyfv),2000000) as sym2,take(symbol(`cy````f65dfy4fv),2000000) as sym3,take(symbol(`cy```56e`f65dfgyfv),2000000) as sym4,take(symbol(`cy`fty``56e`12547),2000000) as sym5,take(symbol(`cy`fty``e`f65d728fyfv),2000000) as sym6,take(symbol(`cy`fty``56e`),2000000) as sym7,take(symbol(`cy`fty``56e`111),2000000) as sym8,take(symbol(`c412y`ft575y```f65dfyfv),2000000) as sym9)");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "table with diff symbol vectors:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("d=dict(take(symbol(`cy`fty``56e`f65dfyfv),2000000),take(symbol(`cy`fty``56e`f65dfyfv),2000000))");
    startTime = getTimeStampMs();
    conn.run("d");
    time = getTimeStampMs() - startTime;
    std::cout << "symbol dict:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("s=set(take(symbol(`cy`fty``56e`f65dfyfv),2000000))");
    startTime = getTimeStampMs();
    conn.run("s");
    time = getTimeStampMs() - startTime;
    std::cout << "symbol set:" << time << "ms" << std::endl;
    conn.run("undef(all)");

    conn.run("t=(take(symbol(`cy`fty``56e`f65dfyfv),2000000),take(symbol(`cy`fty``56e`f65dfyfv),2000000))");
    startTime = getTimeStampMs();
    conn.run("t");
    time = getTimeStampMs() - startTime;
    std::cout << "std::tuple symbol std::tuple:" << time << "ms" << std::endl;
    conn.run("undef(all)");
}

TEST_F(DolphinDBTest,testmixtimevectorUpload){
    dolphindb::VectorSP dates = dolphindb::Util::createVector(dolphindb::DT_ANY, 5, 100);
    dates->set(0, dolphindb::Util::createMonth(2016, 6));
    dates->set(1, dolphindb::Util::createDate(2016, 5, 16));
    dates->set(2, dolphindb::Util::createDateTime(2016, 6, 6, 6, 12, 12));
    dates->set(3, dolphindb::Util::createNanoTime(6, 28, 36, 00));
    dates->set(4, dolphindb::Util::createNanoTimestamp(2020, 8, 20, 2, 20, 20, 00));
    std::vector<dolphindb::ConstantSP> mixtimedata = { dates };
    std::vector<std::string> mixtimename = { "Mixtime" };
    conn.upload(mixtimename, mixtimedata);
}

TEST_F(DolphinDBTest,testFunctionDef){
    std::string script = "def funcAdd(a,b){return a + b};funcAdd(100,200);";
    dolphindb::ConstantSP result = conn.run(script);
    ASSERT_EQ(result->getString(), std::string("300"));
}


TEST_F(DolphinDBTest,testMatrix){
    std::vector<std::string> expectResults = { "{1,2}","{3,4}","{5,6}" };
    std::string script = "1..6$2:3";
    dolphindb::ConstantSP result = conn.run(script);
    for (unsigned int i = 0;i < expectResults.size(); i++) {
        ASSERT_EQ(result->getString(i), expectResults[i]);
    }
}

TEST_F(DolphinDBTest,testTable){
    std::string script;
    script += "n=20000\n";
    script += "syms=`IBM`C`MS`MSFT`JPM`ORCL`BIDU`SOHU`GE`EBAY`GOOG`FORD`GS`PEP`USO`GLD`GDX`EEM`FXI`SLV`SINA`BAC`AAPL`PALL`YHOO`KOH`TSLA`CS`CISO`SUN\n";
    script += "mytrades=table(09:30:00+rand(18000,n) as timestamp,rand(syms,n) as sym, 10*(1+rand(100,n)) as qty,5.0+rand(100.0,n) as price, 1..n as number,rand(syms,n) as sym_2);\n";
    script += "select min(number) as minNum, max(number) as maxNum from mytrades";

    dolphindb::ConstantSP table = conn.run(script);
    ASSERT_EQ(table->getColumn(0)->getString(0), std::string("1"));
    ASSERT_EQ(table->getColumn(1)->getString(0), std::string("20000"));
}

TEST_F(DolphinDBTest,testDictionary){
    std::string script;
    script += "dict(1 2 3,symbol(`IBM`MSFT`GOOG))";
    dolphindb::DictionarySP dict = conn.run(script);

    ASSERT_EQ(dict->get(dolphindb::Util::createInt(1))->getString(), std::string("IBM"));
    ASSERT_EQ(dict->get(dolphindb::Util::createInt(2))->getString(), std::string("MSFT"));
    ASSERT_EQ(dict->get(dolphindb::Util::createInt(3))->getString(), std::string("GOOG"));
}

TEST_F(DolphinDBTest,testSet){
    std::string script;
    script += "x=set(4 5 5 2 3 11 11 11 6 6  6 6  6);x;";
    dolphindb::ConstantSP set = conn.run(script);
    ASSERT_EQ(set->size(), 6);
}

TEST_F(DolphinDBTest,testCharVectorHash){
    std::vector<char> testValues{ 127,-127,12,0,-12,-128 };
    int buckets[5] = { 13,43,71,97,4097 };
    int expected[5][6] = {
        { 10,12,12,0,10,-1 },
    { 41,18,12,0,4,-1 },
    { 56,24,12,0,68,-1 },
    { 30,5,12,0,23,-1 },
    { 127,129,12,0,244,-1 } };
    int hv[6] = { 0 };

    for (unsigned int j = 0;j<5;j++) {
        for (unsigned int i = 0;i < testValues.size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::createChar(testValues[i]);
            hv[i] = val->getHash(buckets[j]);
        }
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
        
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_CHAR, 0);
    v->appendChar(testValues.data(), testValues.size());
    for (unsigned int j = 0;j<5;j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testShortVectorHash){
    std::vector<short> testValues{ 32767,-32767,12,0,-12,-32768 };
    int buckets[5] = { 13,43,71,97,4097 };
    int expected[5][6] = {
        { 7,2,12,0,10,-1 },
    { 1,15,12,0,4,-1 },
    { 36,44,12,0,68,-1 },
    { 78,54,12,0,23,-1 },
    { 4088,265,12,0,244,-1 } };
    int hv[6] = { 0 };

    for (unsigned int j = 0;j<5;j++) {
        for (unsigned int i = 0;i < testValues.size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::createShort(testValues[i]);
            hv[i] = val->getHash(buckets[j]);
        }
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_SHORT, 0);
    v->appendShort(testValues.data(), testValues.size());
    for (unsigned int j = 0;j<5;j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testIntVectorHash){
    std::vector<int> testValues{ INT_MAX,INT_MAX*(-1),12,0,-12,INT_MIN };
    int buckets[5] = { 13,43,71,97,4097 };
    int expected[5][6] = {
        { 10,12,12,0,10,-1 },
    { 7,9,12,0,4,-1 },
    { 39,41,12,0,68,-1 },
    { 65,67,12,0,23,-1 },
    { 127,129,12,0,244,-1 } };
    int hv[6] = { 0 };

    for (unsigned int j = 0;j<5;j++) {
        for (unsigned int i = 0;i < testValues.size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::createInt(testValues[i]);
            hv[i] = val->getHash(buckets[j]);
        }
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_INT, 0);
    v->appendInt(testValues.data(), testValues.size());
    for (unsigned int j = 0;j<5;j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testLongVectorHash){
    std::vector<long long> testValues{ LLONG_MAX,(-1)*LLONG_MAX,12,0,-12,LLONG_MIN };
    int buckets[5] = { 13,43,71,97,4097 };
    int expected[5][6] = {
        { 7,9,12,0,4,-1 },
    { 41,0,12,0,29,-1 },
    { 4,6,12,0,69,-1 },
    { 78,80,12,0,49,-1 },
    { 4088,4090,12,0,4069,-1 } };
    int hv[6] = { 0 };

    for (unsigned int j = 0;j<5;j++) {
        for (unsigned int i = 0;i < testValues.size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::createLong(testValues[i]);
            hv[i] = val->getHash(buckets[j]);
        }
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_LONG, 0);
    v->appendLong(testValues.data(), testValues.size());
    for (unsigned int j = 0;j<5;j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testStringVectorHash){
    std::vector<std::string> testValues{ "9223372036854775807","helloworldabcdefghijklmnopqrstuvwxyz","智臾科技a","hello,智臾科技a","123abc您好！a","" };
    int buckets[5] = { 13,43,71,97,4097 };
    int expected[5][6] = {
        { 8,1,9,3,3,0 },
    { 37,20,25,25,27,0 },
    { 31,0,65,54,15,0 },
    { 24,89,46,52,79,0 },
    { 739,3737,2208,1485,376,0 } };
    int hv[6] = { 0 };

    for (unsigned int j = 0;j<5;j++) {
        for (unsigned int i = 0;i < testValues.size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::createString(testValues[i]);
            hv[i] = val->getHash(buckets[j]);
        }
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_STRING, 0);
    v->appendString(testValues.data(), testValues.size());
    for (unsigned int j = 0;j<5;j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testUUIDvectorHash){
    std::string script;
    script = "a=rand(uuid(),6);table(a as k,hashBucket(a,13) as v1,hashBucket(a,43) as v2,hashBucket(a,71) as v3,hashBucket(a,97) as v4,hashBucket(a,4097) as v5)";
    dolphindb::TableSP t = conn.run(script);

    int buckets[5] = { 13,43,71,97,4097 };
    int hv[6] = { 0 };
    int expected[5][6] = { 0 };

    for (unsigned int j = 0;j<5;j++) {
        for (int i = 0;i < t->size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::parseConstant(dolphindb::DT_UUID, t->getColumn(0)->getString(i));
            hv[i] = val->getHash(buckets[j]);
            expected[j][i] = t->getColumn(j + 1)->getInt(i);
        }
        for (unsigned int k = 0;k < t->size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_UUID, 0);
    for (int i = 0;i < t->size(); i++)
        v->append(dolphindb::Util::parseConstant(dolphindb::DT_UUID, t->getColumn(0)->getString(i)));
    for (unsigned int j = 0;j<5;j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < t->size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testIpAddrvectorHash){
    std::string script;
    script = "a=rand(ipaddr(),6);table(a as k,hashBucket(a,13) as v1,hashBucket(a,43) as v2,hashBucket(a,71) as v3,hashBucket(a,97) as v4,hashBucket(a,4097) as v5)";
    dolphindb::TableSP t = conn.run(script);

    int buckets[5] = { 13,43,71,97,4097 };
    int hv[6] = { 0 };
    int expected[5][6] = { 0 };

    for (unsigned int j = 0;j<5;j++) {
        for (int i = 0;i < t->size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::parseConstant(dolphindb::DT_IP, t->getColumn(0)->getString(i));
            hv[i] = val->getHash(buckets[j]);
            expected[j][i] = t->getColumn(j + 1)->getInt(i);
        }
        for (unsigned int k = 0;k < t->size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_IP, 0);
    for (int i = 0;i < t->size(); i++)
        v->append(dolphindb::Util::parseConstant(dolphindb::DT_IP, t->getColumn(0)->getString(i)));
    for (unsigned int j = 0;j<5;j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < t->size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testInt128vectorHash){
    std::string script;
    script = "a=rand(int128(),6);table(a as k,hashBucket(a,13) as v1,hashBucket(a,43) as v2,hashBucket(a,71) as v3,hashBucket(a,97) as v4,hashBucket(a,4097) as v5)";
    dolphindb::TableSP t = conn.run(script);

    int buckets[5] = { 13,43,71,97,4097 };
    int hv[6] = { 0 };
    int expected[5][6] = { 0 };

    for (unsigned int j = 0;j<5;j++) {
        for (int i = 0;i < t->size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::parseConstant(dolphindb::DT_INT128, t->getColumn(0)->getString(i));
            hv[i] = val->getHash(buckets[j]);
            expected[j][i] = t->getColumn(j + 1)->getInt(i);
        }
        for (unsigned int k = 0;k < t->size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_INT128, 0);
    for (int i = 0;i < t->size(); i++)
        v->append(dolphindb::Util::parseConstant(dolphindb::DT_INT128, t->getColumn(0)->getString(i)));
    for (unsigned int j = 0;j<5;j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < t->size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testCharVectorHash2){
    std::vector<char> testValues { 127, -127, 12, 0, -12, -128 };
    int buckets[5] = { 13, 43, 71, 97, 4097 };
    int expected[5][6] = { { 10, 12, 12, 0, 10, -1 }, { 41, 18, 12, 0, 4, -1 }, { 56, 24, 12, 0, 68, -1 }, { 30, 5, 12, 0, 23, -1 }, { 127, 129, 12, 0, 244, -1 } };
    int hv[6] = { 0 };

    for (unsigned int j = 0; j < 5; j++) {
        for (unsigned int i = 0; i < testValues.size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::createChar(testValues[i]);
            hv[i] = val->getHash(buckets[j]);
            ASSERT_EQ(hv[i], expected[j][i]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_CHAR, 0);
    v->appendChar(testValues.data(), testValues.size());
    for (unsigned int j = 0; j < 5; j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testShortVectorHash2){
    std::vector<short> testValues { 32767, -32767, 12, 0, -12, -32768 };
    int buckets[5] = { 13, 43, 71, 97, 4097 };
    int expected[5][6] = { { 7, 2, 12, 0, 10, -1 }, { 1, 15, 12, 0, 4, -1 }, { 36, 44, 12, 0, 68, -1 }, { 78, 54, 12, 0, 23, -1 }, { 4088, 265, 12, 0, 244, -1 } };
    int hv[6] = { 0 };

    for (unsigned int j = 0; j < 5; j++) {
        for (unsigned int i = 0; i < testValues.size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::createShort(testValues[i]);
            hv[i] = val->getHash(buckets[j]);
            ASSERT_EQ(hv[i], expected[j][i]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_SHORT, 0);
    v->appendShort(testValues.data(), testValues.size());
    for (unsigned int j = 0; j < 5; j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testIntVectorHash2){
    std::vector<int> testValues { INT_MAX, INT_MAX * (-1), 12, 0, -12, INT_MIN };
    int buckets[5] = { 13, 43, 71, 97, 4097 };
    int expected[5][6] = { { 10, 12, 12, 0, 10, -1 }, { 7, 9, 12, 0, 4, -1 }, { 39, 41, 12, 0, 68, -1 }, { 65, 67, 12, 0, 23, -1 }, { 127, 129, 12, 0, 244, -1 } };
    int hv[6] = { 0 };

    for (unsigned int j = 0; j < 5; j++) {
        for (unsigned int i = 0; i < testValues.size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::createInt(testValues[i]);
            hv[i] = val->getHash(buckets[j]);
            ASSERT_EQ(hv[i], expected[j][i]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_INT, 0);
    v->appendInt(testValues.data(), testValues.size());
    for (unsigned int j = 0; j < 5; j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testLongVectorHash2){
    std::vector<long long> testValues { LLONG_MAX, (-1) * LLONG_MAX, 12, 0, -12, LLONG_MIN };
    int buckets[5] = { 13, 43, 71, 97, 4097 };
    int expected[5][6] = { { 7, 9, 12, 0, 4, -1 }, { 41, 0, 12, 0, 29, -1 }, { 4, 6, 12, 0, 69, -1 }, { 78, 80, 12, 0, 49, -1 }, { 4088, 4090, 12, 0, 4069, -1 } };
    int hv[6] = { 0 };

    for (unsigned int j = 0; j < 5; j++) {
        for (unsigned int i = 0; i < testValues.size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::createLong(testValues[i]);
            hv[i] = val->getHash(buckets[j]);
            ASSERT_EQ(hv[i], expected[j][i]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_LONG, 0);
    v->appendLong(testValues.data(), testValues.size());
    for (unsigned int j = 0; j < 5; j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testStringVectorHash2){
    std::vector < std::string > testValues { "9223372036854775807", "helloworldabcdefghijklmnopqrstuvwxyz", "智臾科技a", "hello,智臾科技a", "123abc您好！a", ""};
    int buckets[5] = { 13, 43, 71, 97, 4097 };
    int expected[5][6] = { { 8, 1, 9, 3, 3, 0 }, { 37, 20, 25, 25, 27, 0 }, { 31, 0, 65, 54, 15, 0 }, { 24, 89, 46, 52, 79, 0 }, { 739, 3737, 2208, 1485, 376, 0 } };
    int hv[6] = { 0 };

    for (unsigned int j = 0; j < 5; j++) {
        for (unsigned int i = 0; i < testValues.size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::createString(testValues[i]);
            hv[i] = val->getHash(buckets[j]);
            ASSERT_EQ(hv[i], expected[j][i]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_STRING, 0);
    v->appendString(testValues.data(), testValues.size());
    for (unsigned int j = 0; j < 5; j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < testValues.size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}


TEST_F(DolphinDBTest,testUUIDvectorHash2){
    std::string script;
    script = "a=rand(uuid(),6);table(a as k,hashBucket(a,13) as v1,hashBucket(a,43) as v2,hashBucket(a,71) as v3,hashBucket(a,97) as v4,hashBucket(a,4097) as v5)";
    dolphindb::TableSP t = conn.run(script);

    int buckets[5] = { 13, 43, 71, 97, 4097 };
    int hv[6] = { 0 };
    int expected[5][6] = { 0 };

    for (unsigned int j = 0; j < 5; j++) {
        for (int i = 0; i < t->size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::parseConstant(dolphindb::DT_UUID, t->getColumn(0)->getString(i));
            hv[i] = val->getHash(buckets[j]);
            expected[j][i] = t->getColumn(j + 1)->getInt(i);
            ASSERT_EQ(hv[i], expected[j][i]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_UUID, 0);
    for (int i = 0; i < t->size(); i++)
        v->append(dolphindb::Util::parseConstant(dolphindb::DT_UUID, t->getColumn(0)->getString(i)));
    for (unsigned int j = 0; j < 5; j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < t->size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testIpAddrvectorHash2){
    std::string script;
    script = "a=rand(ipaddr(),6);table(a as k,hashBucket(a,13) as v1,hashBucket(a,43) as v2,hashBucket(a,71) as v3,hashBucket(a,97) as v4,hashBucket(a,4097) as v5)";
    dolphindb::TableSP t = conn.run(script);

    int buckets[5] = { 13, 43, 71, 97, 4097 };
    int hv[6] = { 0 };
    int expected[5][6] = { 0 };

    for (unsigned int j = 0; j < 5; j++) {
        for (int i = 0; i < t->size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::parseConstant(dolphindb::DT_IP, t->getColumn(0)->getString(i));
            hv[i] = val->getHash(buckets[j]);
            expected[j][i] = t->getColumn(j + 1)->getInt(i);
            ASSERT_EQ(hv[i], expected[j][i]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_IP, 0);
    for (int i = 0; i < t->size(); i++)
        v->append(dolphindb::Util::parseConstant(dolphindb::DT_IP, t->getColumn(0)->getString(i)));
    for (unsigned int j = 0; j < 5; j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < t->size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,testInt128vectorHash2){
    std::string script;
    script = "a=rand(int128(),6);table(a as k,hashBucket(a,13) as v1,hashBucket(a,43) as v2,hashBucket(a,71) as v3,hashBucket(a,97) as v4,hashBucket(a,4097) as v5)";
    dolphindb::TableSP t = conn.run(script);

    int buckets[5] = { 13, 43, 71, 97, 4097 };
    int hv[6] = { 0 };
    int expected[5][6] = { 0 };

    for (unsigned int j = 0; j < 5; j++) {
        for (int i = 0; i < t->size(); i++) {
            dolphindb::ConstantSP val = dolphindb::Util::parseConstant(dolphindb::DT_INT128, t->getColumn(0)->getString(i));
            hv[i] = val->getHash(buckets[j]);
            expected[j][i] = t->getColumn(j + 1)->getInt(i);
            ASSERT_EQ(hv[i], expected[j][i]);
        }
    }
    dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_INT128, 0);
    for (int i = 0; i < t->size(); i++)
        v->append(dolphindb::Util::parseConstant(dolphindb::DT_INT128, t->getColumn(0)->getString(i)));
    for (unsigned int j = 0; j < 5; j++) {
        v->getHash(0, 6, buckets[j], hv);
        for (unsigned int k = 0;k < t->size(); k++){
            ASSERT_EQ(hv[k], expected[j][k]);
        }
    }
}

TEST_F(DolphinDBTest,test_symbol_optimization){
    std::string script;
    conn.run(script);
    std::vector<std::string> colNames = { "col1", "col2", "col3", "col4", "col5", "col6", "col7", "col8", "col9", "col10" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_SYMBOL, dolphindb::DT_SYMBOL, dolphindb::DT_SYMBOL, dolphindb::DT_SYMBOL, dolphindb::DT_SYMBOL, dolphindb::DT_SYMBOL, dolphindb::DT_SYMBOL, dolphindb::DT_SYMBOL, dolphindb::DT_SYMBOL, dolphindb::DT_SYMBOL };
    int colNum = 10, rowNum = 2000000;
    dolphindb::TableSP t11 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 2000000);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(t11->getColumn(i));
    for (int i = 0; i < rowNum; i++) {
        columnVecs[0]->set(i, dolphindb::Util::createString("A" + std::to_string(i % 1000)));
        columnVecs[1]->set(i, dolphindb::Util::createString("B" + std::to_string(i % 1000)));
        columnVecs[2]->set(i, dolphindb::Util::createString("C" + std::to_string(i % 1000)));
        columnVecs[3]->set(i, dolphindb::Util::createString("D" + std::to_string(i % 1000)));
        columnVecs[4]->set(i, dolphindb::Util::createString("E" + std::to_string(i % 1000)));
        columnVecs[5]->set(i, dolphindb::Util::createString("F" + std::to_string(i % 1000)));
        columnVecs[6]->set(i, dolphindb::Util::createString("G" + std::to_string(i % 1000)));
        columnVecs[7]->set(i, dolphindb::Util::createString("H" + std::to_string(i % 1000)));
        columnVecs[8]->set(i, dolphindb::Util::createString("I" + std::to_string(i % 1000)));
        columnVecs[9]->set(i, dolphindb::Util::createString("J" + std::to_string(i % 1000)));
    }
    int64_t startTime, time;
    startTime = getTimeStampMs();
    conn.upload("t11", { t11 });
    time = getTimeStampMs() - startTime;
    std::cout << "symbol table:" << time << "ms" << std::endl;
    std::string script1;
    script1 += "n=2000000;";
    script1 += "tmp=table(take(symbol(\"A\"+string(0..999)), n) as col1, take(symbol(\"B\"+string(0..999)), n) as col2,\
     take(symbol(\"C\"+string(0..999)), n) as col3, take(symbol(\"D\"+string(0..999)), n) as col4, \
     take(symbol(\"E\"+string(0..999)), n) as col5, take(symbol(\"F\"+string(0..999)), n) as col6,\
      take(symbol(\"G\"+string(0..999)), n) as col7, take(symbol(\"H\"+string(0..999)), n) as col8, \
      take(symbol(\"I\"+string(0..999)), n) as col9, take(symbol(\"J\"+string(0..999)), n) as col10);";
    script1 += "each(eqObj, t11.values(), tmp.values());";
    dolphindb::ConstantSP result = conn.run(script1);
    for (int i = 0; i<10; i++){
        std::cout<<result->getInt(i);
        ASSERT_EQ(result->getInt(i), 1);}
    dolphindb::ConstantSP res = conn.run("t11");
}

TEST_F(DolphinDBTest,testClearMemory_var){
    std::string script, script1;
    script += "login('admin', '123456');";
    script += "testVar=1000000";
    conn.run(script, 4, 2, 0, true);
    std::string result = conn.run("objs()[`name][0]")->getString();

    ASSERT_EQ(result.length(), size_t(0));    
}

TEST_F(DolphinDBTest,testClearMemory){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script, script1, script2;
    script += "login('admin', '123456');";
    script += "dbPath='"+dbName+"';";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db=database(dbPath, VALUE, 2012.01.01..2012.01.10);";
    script += "t=table(1:0, [`date], [DATE]);";
    script += "pt=db.createPartitionedTable(t, `pt, `date);";
    script += "getSessionMemoryStat()[`memSize][0];";
    script += "select * from pt;";
    conn.run(script, 4, 2, 0, true);
    std::string result = conn.run("objs()[`name][0]")->getString();
    ASSERT_EQ(result.length(), size_t(0));
}

TEST_F(DolphinDBTest,test_symbol_base_exceed_2097152){
    std::vector < std::string > colNames = { "name", "id", "str" };
    std::vector<dolphindb::DATA_TYPE> colTypes = { dolphindb::DT_SYMBOL, dolphindb::DT_INT, dolphindb::DT_STRING };
    int colNum = 3, rowNum = 30000000;
    dolphindb::ConstantSP table = dolphindb::Util::createTable(colNames, colTypes, rowNum, 100);
    std::vector<dolphindb::VectorSP> columnVecs;
    for (int i = 0;i<colNum;i++) {
        columnVecs.push_back(table->getColumn(i));
    }
    try {
        for (int i = 0;i<rowNum;i++) {
            columnVecs[0]->set(i, dolphindb::Util::createString("name_" + std::to_string(i)));
            columnVecs[1]->setInt(i, i);
            columnVecs[2]->setString(i, std::to_string(i));
        }
    }
    catch (std::exception e) {
        std::cout << e.what() << std::endl;
    }
}

TEST_F(DolphinDBTest,test_printMsg){
    std::string script5 = "a=int(1);\
                        b=bool(1);\
                        c=char(1);\
                        d=NULL;\
                        ee=short(1);\
                        f=long(1);\
                        g=date(1);\
                        h=month(1);\
                        i=time(1);\
                        j=minute(1);\
                        k=second(1);\
                        l=datetime(1);\
                        m=timestamp(1);\
                        n=nanotime(1);\
                        o=nanotimestamp(1);\
                        p=float(1);\
                        q=double(1);\
                        r=\"1\";\
                        s=uuid(\"5d212a78-cc48-e3b1-4235-b4d91473ee87\");\
                        t=blob(string[1]);\
                        u=table(1 2 3 as col1, `a`b`c as col2);\
                        v=arrayVector(1 2 3 , 9 9 9);\
                        w=dict(`a`b, '中文123￥……，'`)";
    conn.run(script5);
    std::ofstream file("output.txt");
    std::streambuf* originalBuffer = std::cout.rdbuf();
    std::cout.rdbuf(file.rdbuf());

     conn.run("print(a,b,c,d,ee,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w)");

    std::cout.rdbuf(originalBuffer);
    file.close();
    std::ifstream infile("output.txt");
    std::string content((std::istreambuf_iterator<char>(infile)),
                        std::istreambuf_iterator<char>());
    std::cout << content<<std::endl;
    const std::string expectedContent = "1\n1\n1\n1\n1\n1970.01.02\n0000.02M\n00:00:00.001\n00:01m\n00:00:01\n1970.01.01T00:00:01\n1970.01.01T00:00:00.001\n00:00:00.000000001\n1970.01.01T00:00:00.000000001\n1\n1\n1\n5d212a78-cc48-e3b1-4235-b4d91473ee87\n[\"1\"]\ncol1 col2\n---- ----\n1    a   \n2    b   \n3    c   \n\n[[9],[9],[9]]\nb->\na->中文123￥……，\n\n";
    ASSERT_EQ(content, expectedContent);
    infile.close();
    remove("output.txt");
}
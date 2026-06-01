#include <gtest/gtest.h>
#include "config.h"
#include "ResultSet.h"

class DataformTableTest : public testing::Test
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

dolphindb::DBConnection DataformTableTest::conn(false, false);

TEST_F(DataformTableTest, testStringTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_ANY);
    #ifndef _MSC_VER
        dict1->set("string", dolphindb::Util::createString("*-/%**%#~！#“》（a"));
        dict1->set("sym", dolphindb::Util::createString("zzz123中文a"));
    #else
        dict1->set("sym", dolphindb::Util::createString("zzz123中文a"));
        dict1->set("string", dolphindb::Util::createString("*-/%**%#~！#“》（a"));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createString("zzz123中文a"));
    colVecs[1]->set(0, dolphindb::Util::createString("*-/%**%#~！#“》（a"));

    std::vector<std::string> colNames = {"sym", "string"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_STRING, dolphindb::DT_STRING};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createString("zzz123中文a"));
    columnVecs[1]->set(0, dolphindb::Util::createString("*-/%**%#~！#“》（a"));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createString("zzz123中文a"), dolphindb::Util::createString("*-/%**%#~！#“》（a")};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    std::cout << conn.run("tab1")->getScript() << std::endl;
    std::cout << conn.run("tab2")->getScript() << std::endl;
    std::cout << conn.run("tab3")->getScript() << std::endl;

    std::string script = "a = table(\"zzz123中文a\" as sym,\"*-/%**%#~！#“》（a\" as string);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->get(i)->get(dolphindb::Util::createString("string"))->getBool());
            ASSERT_TRUE(judgeres->get(i)->get(dolphindb::Util::createString("sym"))->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());
    ASSERT_EQ(tab1->getTableType(), dolphindb::BASICTBL);
    ASSERT_EQ(tab2->getTableType(), dolphindb::BASICTBL);
    ASSERT_EQ(tab3->getTableType(), dolphindb::BASICTBL);

    tab2->setName("tab2");
    ASSERT_EQ(tab2->keys()->getString(), "[\"sym\",\"string\"]");
    ASSERT_EQ(tab2->getColumnQualifier(0), tab2->getName());

    ASSERT_TRUE(tab2->isTemporary());
    tab2->setReadOnly(true);
    ASSERT_FALSE(tab2->sizeable());
    tab2->setTemporary(1); // nothing to do

    std::string errMsg;
    int rows = 1;
    std::vector<dolphindb::ConstantSP> vals = {dolphindb::Util::createString("val1"), dolphindb::Util::createString("val2")};
    tab2->append(vals, rows, errMsg);
    ASSERT_EQ(errMsg, "Can't modify read only table.");
    tab1->append(vals, rows, errMsg);

    ASSERT_EQ(tab1->get(0)->get(dolphindb::Util::createString("sym"))->getString(), dict1->get(dolphindb::Util::createString("sym"))->getString());
    ASSERT_EQ(tab1->get(0)->get(dolphindb::Util::createString("string"))->getString(), dict1->get(dolphindb::Util::createString("string"))->getString());
    ASSERT_EQ(tab1->get(dolphindb::Util::createString("sym"))->getString(), tab1->getColumn(0)->getString());
    ASSERT_EQ(tab1->getMember(dolphindb::Util::createString("sym"))->getString(), tab1->getColumn(0)->getString());

    dolphindb::TableSP col0 = tab1->getWindow(0, 1, 0, 2);
    ASSERT_EQ(col0->getColumn(0)->get(0)->getString(), tab1->getColumn(0)->get(0)->getString());
    ASSERT_EQ(col0->getColumn(0)->get(1)->getString(), tab1->getColumn(0)->get(1)->getString());

    dolphindb::TableSP instanceTab1 = tab1->getInstance(0);
    ASSERT_EQ(instanceTab1->keys()->getString(), "[\"sym\",\"string\"]");

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testBlobTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_ANY);
    #ifndef _MSC_VER
        dict1->set("string", dolphindb::Util::createString("*-/%**%#~！#“》（a"));
        dict1->set("sym", dolphindb::Util::createString("zzz123中文a"));
    #else
        dict1->set("sym", dolphindb::Util::createString("zzz123中文a"));
        dict1->set("string", dolphindb::Util::createString("*-/%**%#~！#“》（a"));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createBlob("zzz123中文a"));
    colVecs[1]->set(0, dolphindb::Util::createBlob("*-/%**%#~！#“》（a"));

    std::vector<std::string> colNames = {"sym", "string"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_BLOB, dolphindb::DT_BLOB};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createBlob("zzz123中文a"));
    columnVecs[1]->set(0, dolphindb::Util::createBlob("*-/%**%#~！#“》（a"));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createBlob("zzz123中文a"), dolphindb::Util::createBlob("*-/%**%#~！#“》（a")};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    std::string script = "a = table(\"zzz123中文a\" as sym,\"*-/%**%#~！#“》（a\" as string);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->get(i)->get(dolphindb::Util::createBlob("string"))->getBool());
            ASSERT_TRUE(judgeres->get(i)->get(dolphindb::Util::createBlob("sym"))->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());
    ASSERT_EQ(tab1->getTableType(), dolphindb::BASICTBL);
    ASSERT_EQ(tab2->getTableType(), dolphindb::BASICTBL);
    ASSERT_EQ(tab3->getTableType(), dolphindb::BASICTBL);

    tab2->setName("tab2");
    ASSERT_EQ(tab2->keys()->getString(), "[\"sym\",\"string\"]");
    ASSERT_EQ(tab2->getColumnQualifier(0), tab2->getName());

    ASSERT_TRUE(tab2->isTemporary());
    tab2->setReadOnly(true);
    ASSERT_FALSE(tab2->sizeable());
    tab2->setTemporary(1); // nothing to do

    std::string errMsg;
    int rows = 1;
    std::vector<dolphindb::ConstantSP> vals = {dolphindb::Util::createBlob("val1"), dolphindb::Util::createBlob("val2")};
    tab2->append(vals, rows, errMsg);
    ASSERT_EQ(errMsg, "Can't modify read only table.");
    tab1->append(vals, rows, errMsg);

    ASSERT_EQ(tab1->get(0)->get(dolphindb::Util::createBlob("sym"))->getString(), dict1->get(dolphindb::Util::createBlob("sym"))->getString());
    ASSERT_EQ(tab1->get(0)->get(dolphindb::Util::createBlob("string"))->getString(), dict1->get(dolphindb::Util::createBlob("string"))->getString());
    ASSERT_EQ(tab1->get(dolphindb::Util::createBlob("sym"))->getString(), tab1->getColumn(0)->getString());
    ASSERT_EQ(tab1->getMember(dolphindb::Util::createBlob("sym"))->getString(), tab1->getColumn(0)->getString());

    dolphindb::TableSP col0 = tab1->getWindow(0, 1, 0, 2);
    ASSERT_EQ(col0->getColumn(0)->get(0)->getString(), tab1->getColumn(0)->get(0)->getString());
    ASSERT_EQ(col0->getColumn(0)->get(1)->getString(), tab1->getColumn(0)->get(1)->getString());

    dolphindb::TableSP instanceTab1 = tab1->getInstance(0);
    ASSERT_EQ(instanceTab1->keys()->getString(), "[\"sym\",\"string\"]");

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testBoolTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_BOOL);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createBool(true));
        dict1->set("col1", dolphindb::Util::createBool(false));
    #else
        dict1->set("col1", dolphindb::Util::createBool(false));
        dict1->set("col2", dolphindb::Util::createBool(true));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createBool(true));
    colVecs[1]->set(0, dolphindb::Util::createBool(false));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_BOOL, dolphindb::DT_BOOL};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createBool(true));
    columnVecs[1]->set(0, dolphindb::Util::createBool(false));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createBool(true), dolphindb::Util::createBool(false)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(bool(1) as col1,bool(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testCharTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_CHAR);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createChar(1));
        dict1->set("col1", dolphindb::Util::createChar(0));
    #else
        dict1->set("col1", dolphindb::Util::createChar(0));
        dict1->set("col2", dolphindb::Util::createChar(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createChar(1));
    colVecs[1]->set(0, dolphindb::Util::createChar(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_CHAR, dolphindb::DT_CHAR};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createChar(1));
    columnVecs[1]->set(0, dolphindb::Util::createChar(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createChar(1), dolphindb::Util::createChar(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    std::cout << conn.run("tab1")->getScript() << std::endl;
    std::cout << conn.run("tab2")->getScript() << std::endl;
    std::cout << conn.run("tab3")->getScript() << std::endl;

    std::string script = "a = table(char(1) as col1,char(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    std::vector<dolphindb::ConstantSP> colsNull = {};
    std::vector<std::string> col3Names = {"col1", "col2", "col3"};
    ASSERT_ANY_THROW(dolphindb::TableSP tab4 = dolphindb::Util::createTable(colNames, colsNull));
    ASSERT_ANY_THROW(dolphindb::TableSP tab4 = dolphindb::Util::createTable(col3Names, cols));

    dolphindb::VectorSP col1 = dolphindb::Util::createVector(dolphindb::DT_CHAR, 2, 2);
    col1->set(0, dolphindb::Util::createChar(1));
    col1->set(1, dolphindb::Util::createChar(2));
    dolphindb::VectorSP col2 = dolphindb::Util::createVector(dolphindb::DT_CHAR, 1, 1);
    col2->set(0, dolphindb::Util::createChar(1));
    std::vector<dolphindb::ConstantSP> colsDiffrows = {col1, col2};
    ASSERT_ANY_THROW(dolphindb::TableSP tab4 = dolphindb::Util::createTable(colNames, colsDiffrows));

    dolphindb::TableSP tab4 = tab1->getInstance(1);
    tab4->set(0, dict1);
    ASSERT_EQ(tab4->getString(), "col1 col2\n---- ----\n0    1   \n");
    ASSERT_EQ(tab4->get(dolphindb::Util::createInt(0))->getMember(dolphindb::Util::createString("col1"))->getChar(), (char)0);
    ASSERT_EQ(tab4->get(dolphindb::Util::createInt(0))->getMember(dolphindb::Util::createString("col2"))->getChar(), (char)1);

    std::cout << tab4->getAllocatedMemory() << std::endl;

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testCharNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_CHAR);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createChar(1));
        dict1->set("col1", dolphindb::Util::createChar(0));
    #else
        dict1->set("col1", dolphindb::Util::createChar(0));
        dict1->set("col2", dolphindb::Util::createChar(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_CHAR, dolphindb::DT_CHAR};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_CHAR), dolphindb::Util::createNullConstant(dolphindb::DT_CHAR)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(char(NULL) as col1,char(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testIntTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_INT);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createInt(1));
        dict1->set("col1", dolphindb::Util::createInt(0));
    #else
        dict1->set("col1", dolphindb::Util::createInt(0));
        dict1->set("col2", dolphindb::Util::createInt(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createInt(1));
    colVecs[1]->set(0, dolphindb::Util::createInt(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_INT, dolphindb::DT_INT};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createInt(1));
    columnVecs[1]->set(0, dolphindb::Util::createInt(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createInt(1), dolphindb::Util::createInt(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    std::string script = "a = table(int(1) as col1,int(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    tab1->set(0, dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2));
    std::string errCol = "errCol";
    ASSERT_ANY_THROW(tab1->getColumn(errCol));
    ASSERT_ANY_THROW(tab1->getColumn("sym", errCol));
    ASSERT_ANY_THROW(tab1->getColumn(errCol, errCol));

    ASSERT_FALSE(tab1->set(0, dolphindb::Util::createInt(1)));
    tab1->setName("tab1");
    tab1->setColumnName(1, "modifyedname");
    ASSERT_EQ(tab1->getColumnName(1), "modifyedname");
    ASSERT_EQ(tab1->getColumnIndex("errcol"), -1);
    ASSERT_EQ(tab1->getColumnIndex("modifyedname"), 1);

    ASSERT_TRUE(tab1->contain("modifyedname"));
    ASSERT_TRUE(tab1->contain("tab1", "modifyedname"));

    ASSERT_EQ(tab1->getColumnName(0), "col1");
    ASSERT_EQ(tab1->getColumn("tab1", "col1")->getString(), tab1->getColumn(0)->getString());
    ASSERT_EQ(tab1->getColumn("col1", dolphindb::Util::createInt(0))->getString(), tab1->getColumn(0)->get(0)->getString());
    ASSERT_EQ(tab1->getColumn("col1")->getString(), tab1->getColumn(0)->getString());
    ASSERT_EQ(tab1->getColumn("tab1", "col1", dolphindb::Util::createInt(0))->getString(), tab1->getColumn(0)->get(0)->getString());
    ASSERT_EQ(tab1->getColumn("tab1", "col1")->getString(), tab1->getColumn(0)->getString());
    ASSERT_EQ(tab1->getColumn(0, dolphindb::Util::createInt(0))->getString(), tab1->getColumn(0)->get(0)->getString());
    ASSERT_EQ(tab1->getColumn(0)->getString(), tab1->getColumn(0)->getString());
    // ASSERT_EQ(tab1->get(0,0)->getString(),tab1->getColumn(0)->get(0)->getString());

    ASSERT_EQ(tab1->getColumnLabel()->get(0)->getString(), tab1->getColumnName(0));
    ASSERT_EQ(tab1->getColumnLabel()->get(1)->getString(), tab1->getColumnName(1));

    ASSERT_EQ(tab1->getString(0), " 1 0");
    ASSERT_EQ(tab1->get(dolphindb::Util::createString("col1"))->get(0)->getInt(), 1);
    ASSERT_EQ(tab1->get(dolphindb::Util::createInt(0))->getMember(dolphindb::Util::createString("col1"))->getInt(), 1);
    ASSERT_EQ(tab1->get(dolphindb::Util::createInt(0))->getMember(dolphindb::Util::createString("modifyedname"))->getInt(), 0);

    dolphindb::VectorSP pair1 = dolphindb::Util::createPair(dolphindb::DT_INT);
    pair1->set(0, dolphindb::Util::createInt(0));
    pair1->set(1, dolphindb::Util::createInt(1));
    ASSERT_EQ(tab1->getString(), tab1->get(pair1)->getString());

    dolphindb::VectorSP vec1 = dolphindb::Util::createVector(dolphindb::DT_INT, 1, 1);
    vec1->set(0, dolphindb::Util::createInt(0));
    ASSERT_EQ(tab1->getString(), tab1->get(vec1)->getString());
    ASSERT_EQ(tab1->getString(), tab1->getWindow(0, 2, 0, 1)->getString());

    dolphindb::DictionarySP dict_size121 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_INT);
    for (int i = 0; i < 121; i++)
    {
        dict_size121->set("col" + std::to_string(i), dolphindb::Util::createInt(1));
    }
    dolphindb::TableSP tab_121cols = dolphindb::Util::createTable(dict_size121.get(), 1);
    for (int i = 0; i < 121; i++)
    {
        tab_121cols->getColumn(i)->set(0, dolphindb::Util::createInt(1));
    }
    ASSERT_EQ(tab_121cols->getString(0).substr(dolphindb::Util::DISPLAY_WIDTH), "...");

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testIntNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_INT);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createInt(1));
        dict1->set("col1", dolphindb::Util::createInt(0));
    #else
        dict1->set("col1", dolphindb::Util::createInt(0));
        dict1->set("col2", dolphindb::Util::createInt(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_INT, dolphindb::DT_INT};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_INT), dolphindb::Util::createNullConstant(dolphindb::DT_INT)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(int(NULL) as col1,int(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testLongTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_LONG);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createLong(1));
        dict1->set("col1", dolphindb::Util::createLong(0));
    #else
        dict1->set("col1", dolphindb::Util::createLong(0));
        dict1->set("col2", dolphindb::Util::createLong(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createLong(1));
    colVecs[1]->set(0, dolphindb::Util::createLong(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_LONG, dolphindb::DT_LONG};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createLong(1));
    columnVecs[1]->set(0, dolphindb::Util::createLong(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createLong(1), dolphindb::Util::createLong(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(long(1) as col1,long(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testLongNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_LONG);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createLong(1));
        dict1->set("col1", dolphindb::Util::createLong(0));
    #else
        dict1->set("col1", dolphindb::Util::createLong(0));
        dict1->set("col2", dolphindb::Util::createLong(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_LONG, dolphindb::DT_LONG};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_LONG), dolphindb::Util::createNullConstant(dolphindb::DT_LONG)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(long(NULL) as col1,long(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testShortTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_SHORT);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createShort(1));
        dict1->set("col1", dolphindb::Util::createShort(0));
    #else
        dict1->set("col1", dolphindb::Util::createShort(0));
        dict1->set("col2", dolphindb::Util::createShort(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createShort(1));
    colVecs[1]->set(0, dolphindb::Util::createShort(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SHORT, dolphindb::DT_SHORT};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createShort(1));
    columnVecs[1]->set(0, dolphindb::Util::createShort(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createShort(1), dolphindb::Util::createShort(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(short(1) as col1,short(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testShortNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_SHORT);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createShort(1));
        dict1->set("col1", dolphindb::Util::createShort(0));
    #else
        dict1->set("col1", dolphindb::Util::createShort(0));
        dict1->set("col2", dolphindb::Util::createShort(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SHORT, dolphindb::DT_SHORT};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_SHORT), dolphindb::Util::createNullConstant(dolphindb::DT_SHORT)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(short(NULL) as col1,short(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testFloatTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_FLOAT);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createFloat(1));
        dict1->set("col1", dolphindb::Util::createFloat(0));
    #else
        dict1->set("col1", dolphindb::Util::createFloat(0));
        dict1->set("col2", dolphindb::Util::createFloat(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createFloat(1));
    colVecs[1]->set(0, dolphindb::Util::createFloat(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_FLOAT, dolphindb::DT_FLOAT};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createFloat(1));
    columnVecs[1]->set(0, dolphindb::Util::createFloat(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createFloat(1), dolphindb::Util::createFloat(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(float(1) as col1,float(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testFloatNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_FLOAT);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createFloat(1));
        dict1->set("col1", dolphindb::Util::createFloat(0));
    #else
        dict1->set("col1", dolphindb::Util::createFloat(0));
        dict1->set("col2", dolphindb::Util::createFloat(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_FLOAT, dolphindb::DT_FLOAT};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT), dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(float(NULL) as col1,float(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDoubleTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DOUBLE);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDouble(1));
        dict1->set("col1", dolphindb::Util::createDouble(0));
    #else
        dict1->set("col1", dolphindb::Util::createDouble(0));
        dict1->set("col2", dolphindb::Util::createDouble(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createDouble(1));
    colVecs[1]->set(0, dolphindb::Util::createDouble(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DOUBLE, dolphindb::DT_DOUBLE};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createDouble(1));
    columnVecs[1]->set(0, dolphindb::Util::createDouble(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createDouble(1), dolphindb::Util::createDouble(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(double(1) as col1,double(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDoubleNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DOUBLE);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDouble(1));
        dict1->set("col1", dolphindb::Util::createDouble(0));
    #else
        dict1->set("col1", dolphindb::Util::createDouble(0));
        dict1->set("col2", dolphindb::Util::createDouble(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DOUBLE, dolphindb::DT_DOUBLE};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_DOUBLE), dolphindb::Util::createNullConstant(dolphindb::DT_DOUBLE)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(double(NULL) as col1,double(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDatehourTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DATEHOUR);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDateHour(1));
        dict1->set("col1", dolphindb::Util::createDateHour(0));
    #else
        dict1->set("col1", dolphindb::Util::createDateHour(0));
        dict1->set("col2", dolphindb::Util::createDateHour(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createDateHour(1));
    colVecs[1]->set(0, dolphindb::Util::createDateHour(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DATEHOUR, dolphindb::DT_DATEHOUR};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createDateHour(1));
    columnVecs[1]->set(0, dolphindb::Util::createDateHour(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createDateHour(1), dolphindb::Util::createDateHour(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(datehour(1) as col1,datehour(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDatehourNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DATEHOUR);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDateHour(1));
        dict1->set("col1", dolphindb::Util::createDateHour(0));
    #else
        dict1->set("col1", dolphindb::Util::createDateHour(0));
        dict1->set("col2", dolphindb::Util::createDateHour(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DATEHOUR, dolphindb::DT_DATEHOUR};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_DATEHOUR), dolphindb::Util::createNullConstant(dolphindb::DT_DATEHOUR)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(datehour(NULL) as col1,datehour(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDateTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DATE);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDate(1));
        dict1->set("col1", dolphindb::Util::createDate(0));
    #else
        dict1->set("col1", dolphindb::Util::createDate(0));
        dict1->set("col2", dolphindb::Util::createDate(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createDate(1));
    colVecs[1]->set(0, dolphindb::Util::createDate(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DATE, dolphindb::DT_DATE};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createDate(1));
    columnVecs[1]->set(0, dolphindb::Util::createDate(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createDate(1), dolphindb::Util::createDate(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(date(1) as col1,date(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDateNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DATE);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDate(1));
        dict1->set("col1", dolphindb::Util::createDate(0));
    #else
        dict1->set("col1", dolphindb::Util::createDate(0));
        dict1->set("col2", dolphindb::Util::createDate(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DATE, dolphindb::DT_DATE};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_DATE), dolphindb::Util::createNullConstant(dolphindb::DT_DATE)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(date(NULL) as col1,date(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testMinuteTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_MINUTE);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createMinute(1));
        dict1->set("col1", dolphindb::Util::createMinute(0));
    #else
        dict1->set("col1", dolphindb::Util::createMinute(0));
        dict1->set("col2", dolphindb::Util::createMinute(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createMinute(1));
    colVecs[1]->set(0, dolphindb::Util::createMinute(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_MINUTE, dolphindb::DT_MINUTE};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createMinute(1));
    columnVecs[1]->set(0, dolphindb::Util::createMinute(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createMinute(1), dolphindb::Util::createMinute(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(minute(1) as col1,minute(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testMinuteNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_MINUTE);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createMinute(1));
        dict1->set("col1", dolphindb::Util::createMinute(0));
    #else
        dict1->set("col1", dolphindb::Util::createMinute(0));
        dict1->set("col2", dolphindb::Util::createMinute(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_MINUTE, dolphindb::DT_MINUTE};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_MINUTE), dolphindb::Util::createNullConstant(dolphindb::DT_MINUTE)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(minute(NULL) as col1,minute(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDatetimeTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DATETIME);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDateTime(1));
        dict1->set("col1", dolphindb::Util::createDateTime(0));
    #else
        dict1->set("col1", dolphindb::Util::createDateTime(0));
        dict1->set("col2", dolphindb::Util::createDateTime(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createDateTime(1));
    colVecs[1]->set(0, dolphindb::Util::createDateTime(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DATETIME, dolphindb::DT_DATETIME};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createDateTime(1));
    columnVecs[1]->set(0, dolphindb::Util::createDateTime(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createDateTime(1), dolphindb::Util::createDateTime(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(datetime(1) as col1,datetime(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDatetimeNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DATETIME);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDateTime(1));
        dict1->set("col1", dolphindb::Util::createDateTime(0));
    #else
        dict1->set("col1", dolphindb::Util::createDateTime(0));
        dict1->set("col2", dolphindb::Util::createDateTime(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DATETIME, dolphindb::DT_DATETIME};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_DATETIME), dolphindb::Util::createNullConstant(dolphindb::DT_DATETIME)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(datetime(NULL) as col1,datetime(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testTimestampTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_TIMESTAMP);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createTimestamp(1));
        dict1->set("col1", dolphindb::Util::createTimestamp(0));
    #else
        dict1->set("col1", dolphindb::Util::createTimestamp(0));
        dict1->set("col2", dolphindb::Util::createTimestamp(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createTimestamp(1));
    colVecs[1]->set(0, dolphindb::Util::createTimestamp(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_TIMESTAMP};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createTimestamp(1));
    columnVecs[1]->set(0, dolphindb::Util::createTimestamp(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createTimestamp(1), dolphindb::Util::createTimestamp(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(timestamp(1) as col1,timestamp(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testTimestampNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_TIMESTAMP);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createTimestamp(1));
        dict1->set("col1", dolphindb::Util::createTimestamp(0));
    #else
        dict1->set("col1", dolphindb::Util::createTimestamp(0));
        dict1->set("col2", dolphindb::Util::createTimestamp(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_TIMESTAMP};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_TIMESTAMP), dolphindb::Util::createNullConstant(dolphindb::DT_TIMESTAMP)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(timestamp(NULL) as col1,timestamp(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testNanotimeTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_NANOTIME);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createNanoTime(1));
        dict1->set("col1", dolphindb::Util::createNanoTime(0));
    #else
        dict1->set("col1", dolphindb::Util::createNanoTime(0));
        dict1->set("col2", dolphindb::Util::createNanoTime(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createNanoTime(1));
    colVecs[1]->set(0, dolphindb::Util::createNanoTime(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_NANOTIME, dolphindb::DT_NANOTIME};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createNanoTime(1));
    columnVecs[1]->set(0, dolphindb::Util::createNanoTime(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNanoTime(1), dolphindb::Util::createNanoTime(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(nanotime(1) as col1,nanotime(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testNanotimeNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_NANOTIME);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createNanoTime(1));
        dict1->set("col1", dolphindb::Util::createNanoTime(0));
    #else
        dict1->set("col1", dolphindb::Util::createNanoTime(0));
        dict1->set("col2", dolphindb::Util::createNanoTime(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_NANOTIME, dolphindb::DT_NANOTIME};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIME), dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIME)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(nanotime(NULL) as col1,nanotime(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testNanotimestampTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_NANOTIMESTAMP);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createNanoTimestamp(1));
        dict1->set("col1", dolphindb::Util::createNanoTimestamp(0));
    #else
        dict1->set("col1", dolphindb::Util::createNanoTimestamp(0));
        dict1->set("col2", dolphindb::Util::createNanoTimestamp(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createNanoTimestamp(1));
    colVecs[1]->set(0, dolphindb::Util::createNanoTimestamp(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_NANOTIMESTAMP};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createNanoTimestamp(1));
    columnVecs[1]->set(0, dolphindb::Util::createNanoTimestamp(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNanoTimestamp(1), dolphindb::Util::createNanoTimestamp(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(nanotimestamp(1) as col1,nanotimestamp(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testNanotimestampNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_NANOTIMESTAMP);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createNanoTimestamp(1));
        dict1->set("col1", dolphindb::Util::createNanoTimestamp(0));
    #else
        dict1->set("col1", dolphindb::Util::createNanoTimestamp(0));
        dict1->set("col2", dolphindb::Util::createNanoTimestamp(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_NANOTIMESTAMP};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIMESTAMP), dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIMESTAMP)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(nanotimestamp(NULL) as col1,nanotimestamp(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testMonthTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_MONTH);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createMonth(1));
        dict1->set("col1", dolphindb::Util::createMonth(0));
    #else
        dict1->set("col1", dolphindb::Util::createMonth(0));
        dict1->set("col2", dolphindb::Util::createMonth(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createMonth(1));
    colVecs[1]->set(0, dolphindb::Util::createMonth(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_MONTH, dolphindb::DT_MONTH};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createMonth(1));
    columnVecs[1]->set(0, dolphindb::Util::createMonth(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createMonth(1), dolphindb::Util::createMonth(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(month(1) as col1,month(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testMonthNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_MONTH);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createMonth(1));
        dict1->set("col1", dolphindb::Util::createMonth(0));
    #else
        dict1->set("col1", dolphindb::Util::createMonth(0));
        dict1->set("col2", dolphindb::Util::createMonth(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_MONTH, dolphindb::DT_MONTH};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_MONTH), dolphindb::Util::createNullConstant(dolphindb::DT_MONTH)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(month(NULL) as col1,month(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testTimeTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_TIME);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createTime(1));
        dict1->set("col1", dolphindb::Util::createTime(0));
    #else
        dict1->set("col1", dolphindb::Util::createTime(0));
        dict1->set("col2", dolphindb::Util::createTime(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createTime(1));
    colVecs[1]->set(0, dolphindb::Util::createTime(0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIME, dolphindb::DT_TIME};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createTime(1));
    columnVecs[1]->set(0, dolphindb::Util::createTime(0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createTime(1), dolphindb::Util::createTime(0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(time(1) as col1,time(0) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testTimeNullTable)
{
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_TIME);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createTime(1));
        dict1->set("col1", dolphindb::Util::createTime(0));
    #else
        dict1->set("col1", dolphindb::Util::createTime(0));
        dict1->set("col2", dolphindb::Util::createTime(1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIME, dolphindb::DT_TIME};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_TIME), dolphindb::Util::createNullConstant(dolphindb::DT_TIME)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(time(NULL) as col1,time(NULL) as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testMemoryTable)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script;
    // simulation to generate data to be saved to the memory table
    dolphindb::VectorSP names = dolphindb::Util::createVector(dolphindb::DT_STRING, 5, 100);
    dolphindb::VectorSP dates = dolphindb::Util::createVector(dolphindb::DT_DATE, 5, 100);
    dolphindb::VectorSP prices = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 5, 100);
    for (int i = 0; i < 5; i++)
    {
        names->set(i, dolphindb::Util::createString("name_" + std::to_string(i)));
        dates->set(i, dolphindb::Util::createDate(2010, 1, i + 1));
        prices->set(i, dolphindb::Util::createDouble(i * i));
    }
    std::vector<std::string> allnames = {"names", "dates", "prices"};
    std::vector<dolphindb::ConstantSP> allcols = {names, dates, prices};
    conn.upload(allnames, allcols); // upload data to server
    script += "tglobal=table(names,dates,prices);";
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db=database(dbPath,VALUE,2010.01.01..2010.01.30);";
    script += "pt=db.createPartitionedTable(tglobal,`pt,`dates);";
    script += "pt.append!(tglobal);";
    script += "dropPartition(db,2010.01.01,tableName=`pt);";
    script += "select * from pt;";
    dolphindb::TableSP table = conn.run(script);

    for (unsigned int i = 0; i < 4; i++)
    {
        ASSERT_EQ(table->getColumn(0)->get(i)->getString(), names->get(i + 1)->getString());
        ASSERT_EQ(table->getColumn(1)->get(i)->getString(), dates->get(i + 1)->getString());
        ASSERT_EQ(table->getColumn(2)->get(i)->getString(), prices->get(i + 1)->getString());
    }
}

TEST_F(DataformTableTest, testDecimal32Table)
{
    srand((int)time(NULL));
    int scale1 = rand() % 10;
    int scale2 = rand() % 10;
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DECIMAL32);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDecimal32(scale2, 1));
        dict1->set("col1", dolphindb::Util::createDecimal32(scale1, 0));
    #else
        dict1->set("col1", dolphindb::Util::createDecimal32(scale1, 0));
        dict1->set("col2", dolphindb::Util::createDecimal32(scale2, 1));
    #endif // _MSC_VER
    dict1->set("col2", dolphindb::Util::createDecimal32(scale2, 1));
    dict1->set("col1", dolphindb::Util::createDecimal32(scale1, 0));
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createDecimal32(scale1, 1));
    colVecs[1]->set(0, dolphindb::Util::createDecimal32(scale2, 0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DECIMAL32, dolphindb::DT_DECIMAL32};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1, {scale1, scale2});
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createDecimal32(scale1, 1));
    columnVecs[1]->set(0, dolphindb::Util::createDecimal32(scale2, 0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createDecimal32(scale1, 1), dolphindb::Util::createDecimal32(scale2, 0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(decimal32(1," + std::to_string(scale1) + ") as col1,decimal32(0," + std::to_string(scale2) + ") as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab_res->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab_res->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab2->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab3->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab1->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab2->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab3->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDecimal32NullTable)
{
    srand((int)time(NULL));
    int scale1 = rand() % 10;
    int scale2 = rand() % 10;
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DECIMAL32);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDecimal32(scale2, 1));
        dict1->set("col1", dolphindb::Util::createDecimal32(scale1, 0));
    #else
        dict1->set("col1", dolphindb::Util::createDecimal32(scale1, 0));
        dict1->set("col2", dolphindb::Util::createDecimal32(scale2, 1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DECIMAL32, dolphindb::DT_DECIMAL32};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1, {scale1, scale2});
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL32, scale1), dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL32, scale2)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(decimal32(NULL," + std::to_string(scale1) + ") as col1,decimal32(NULL," + std::to_string(scale2) + ") as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab_res->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab_res->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab2->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab3->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab1->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab2->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab3->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDecimal64Table)
{
    srand((int)time(NULL));
    int scale1 = rand() % 19;
    int scale2 = rand() % 19;
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DECIMAL64);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDecimal64(scale2, 1));
        dict1->set("col1", dolphindb::Util::createDecimal64(scale1, 0));
    #else
        dict1->set("col1", dolphindb::Util::createDecimal64(scale1, 0));
        dict1->set("col2", dolphindb::Util::createDecimal64(scale2, 1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createDecimal64(scale1, 1));
    colVecs[1]->set(0, dolphindb::Util::createDecimal64(scale2, 0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DECIMAL64, dolphindb::DT_DECIMAL64};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1, {scale1, scale2});
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createDecimal64(scale1, 1));
    columnVecs[1]->set(0, dolphindb::Util::createDecimal64(scale2, 0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createDecimal64(scale1, 1), dolphindb::Util::createDecimal64(scale2, 0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(decimal64(1," + std::to_string(scale1) + ") as col1,decimal64(0," + std::to_string(scale2) + ") as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab_res->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab_res->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab2->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab3->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab1->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab2->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab3->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDecimal64NullTable)
{
    srand((int)time(NULL));
    int scale1 = rand() % 19;
    int scale2 = rand() % 19;
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DECIMAL64);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDecimal64(scale2, 1));
        dict1->set("col1", dolphindb::Util::createDecimal64(scale1, 0));
    #else
        dict1->set("col1", dolphindb::Util::createDecimal64(scale1, 0));
        dict1->set("col2", dolphindb::Util::createDecimal64(scale2, 1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DECIMAL64, dolphindb::DT_DECIMAL64};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1, {scale1, scale2});
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL64, scale1), dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL64, scale2)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(decimal64(NULL," + std::to_string(scale1) + ") as col1,decimal64(NULL," + std::to_string(scale2) + ") as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab_res->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab_res->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab2->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab3->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab1->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab2->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab3->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDecimal128Table)
{
    srand((int)time(NULL));
    int scale1 = rand() % 38;
    int scale2 = rand() % 38;
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DECIMAL128);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDecimal128(scale2, 1));
        dict1->set("col1", dolphindb::Util::createDecimal128(scale1, 0));
    #else
        dict1->set("col1", dolphindb::Util::createDecimal128(scale1, 0));
        dict1->set("col2", dolphindb::Util::createDecimal128(scale2, 1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->set(0, dolphindb::Util::createDecimal128(scale1, 1));
    colVecs[1]->set(0, dolphindb::Util::createDecimal128(scale2, 0));

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DECIMAL128, dolphindb::DT_DECIMAL128};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1, {scale1, scale2});
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->set(0, dolphindb::Util::createDecimal128(scale1, 1));
    columnVecs[1]->set(0, dolphindb::Util::createDecimal128(scale2, 0));

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createDecimal128(scale1, 1), dolphindb::Util::createDecimal128(scale2, 0)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(decimal128(1," + std::to_string(scale1) + ") as col1,decimal128(0," + std::to_string(scale2) + ") as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab_res->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab_res->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab2->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab3->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab1->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab2->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab3->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

TEST_F(DataformTableTest, testDecimal128NullTable)
{
    srand((int)time(NULL));
    int scale1 = rand() % 38;
    int scale2 = rand() % 38;
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_DECIMAL128);
    #ifndef _MSC_VER
        dict1->set("col2", dolphindb::Util::createDecimal128(scale2, 1));
        dict1->set("col1", dolphindb::Util::createDecimal128(scale1, 0));
    #else
        dict1->set("col1", dolphindb::Util::createDecimal128(scale1, 0));
        dict1->set("col2", dolphindb::Util::createDecimal128(scale2, 1));
    #endif // _MSC_VER
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(dict1.get(), 1);
    std::vector<dolphindb::VectorSP> colVecs;
    colVecs.reserve(2);
    for (int i = 0; i < 2; i++)
    {
        colVecs.emplace_back(tab1->getColumn(i));
    }
    colVecs[0]->setNull(0);
    colVecs[1]->setNull(0);

    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DECIMAL128, dolphindb::DT_DECIMAL128};
    int colNum = 2, rowNum = 1;
    dolphindb::TableSP tab2 = dolphindb::Util::createTable(colNames, colTypes, rowNum, 1, {scale1, scale2});
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab2->getColumn(i));
    }
    columnVecs[0]->setNull(0);
    columnVecs[1]->setNull(0);

    std::vector<dolphindb::ConstantSP> cols = {dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL128, scale1), dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL128, scale2)};
    dolphindb::TableSP tab3 = dolphindb::Util::createTable(colNames, cols);

    conn.upload("tab1", {tab1});
    conn.upload("tab2", {tab2});
    conn.upload("tab3", {tab3});

    // std::cout<<conn.run("tab1")->getString()<<std::endl;
    // std::cout<<conn.run("tab2")->getString()<<std::endl;
    // std::cout<<conn.run("tab3")->getString()<<std::endl;

    std::string script = "a = table(decimal128(NULL," + std::to_string(scale1) + ") as col1,decimal128(NULL," + std::to_string(scale2) + ") as col2);a";
    dolphindb::TableSP tab_res = conn.run(script);

    std::string judgestr = "res1=(a==tab1);res2=(a==tab2);res3=(a==tab3);\n\
                    res1.append!(res2)\n\
                    res1.append!(res3)\n\
                    res1";

    dolphindb::TableSP judgeres = conn.run(judgestr);
    for (unsigned int i = 0; i < judgeres->columns(); i++)
    {
        for (unsigned int j = 0; j < judgeres->rows(); j++)
        {
            ASSERT_TRUE(judgeres->getColumn(i)->get(j)->getBool());
        }
    }

    ASSERT_EQ(tab_res->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab_res->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab2->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab3->getColumn(0)->getExtraParamForType(), scale1);
    ASSERT_EQ(tab1->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab2->getColumn(1)->getExtraParamForType(), scale2);
    ASSERT_EQ(tab3->getColumn(1)->getExtraParamForType(), scale2);

    ASSERT_EQ(tab1->getString(), tab_res->getString());
    ASSERT_EQ(tab2->getString(), tab_res->getString());
    ASSERT_EQ(tab3->getString(), tab_res->getString());
    ASSERT_EQ(tab1->getType(), tab_res->getType());
    ASSERT_EQ(tab2->getType(), tab_res->getType());
    ASSERT_EQ(tab3->getType(), tab_res->getType());

    // ASSERT_TRUE(tab1->rows() > 0);
    ASSERT_TRUE(tab1->clear());
    ASSERT_EQ(tab1->rows(), 0);
}

dolphindb::TableSP createDemoTable()
{
    std::vector<std::string> colNames = {"name", "date", "price"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_STRING, dolphindb::DT_DATE, dolphindb::DT_DOUBLE};
    int colNum = 3, rowNum = 3;
    dolphindb::ConstantSP table = dolphindb::Util::createTable(colNames, colTypes, rowNum, 100);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(table->getColumn(i));

    for (int i = 0; i < rowNum; i++)
    {
        columnVecs[0]->set(i, dolphindb::Util::createString("name_" + std::to_string(i)));
        columnVecs[1]->set(i, dolphindb::Util::createDate(2010, 1, i + 1));
        columnVecs[2]->set(i, dolphindb::Util::createDouble(i * i));
    }
    return table;
}

dolphindb::TableSP createDemoTableSetStringWrong()
{
    std::vector<std::string> colNames = {"name", "date", "price"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_STRING, dolphindb::DT_DATE, dolphindb::DT_DOUBLE};
    int colNum = 3, rowNum = 3;
    dolphindb::ConstantSP table = dolphindb::Util::createTable(colNames, colTypes, rowNum, 100);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(table->getColumn(i));

    for (int i = 0; i < rowNum; i++)
    {
        // wrong usage
        columnVecs[0]->setString("name_" + std::to_string(i));
        columnVecs[1]->set(i, dolphindb::Util::createDate(2010, 1, i + 1));
        columnVecs[2]->set(i, dolphindb::Util::createDouble(i * i));
    }
    return table;
}

dolphindb::TableSP createDemoTableSetString()
{
    std::vector<std::string> colNames = {"name", "date", "price"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_STRING, dolphindb::DT_DATE, dolphindb::DT_DOUBLE};
    int colNum = 3, rowNum = 3;
    dolphindb::ConstantSP table = dolphindb::Util::createTable(colNames, colTypes, rowNum, 100);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
        columnVecs.emplace_back(table->getColumn(i));

    for (int i = 0; i < rowNum; i++)
    {
        // wrong usage
        columnVecs[0]->setString(i, "name_" + std::to_string(i));
        columnVecs[1]->set(i, dolphindb::Util::createDate(2010, 1, i + 1));
        columnVecs[2]->set(i, dolphindb::Util::createDouble(i * i));
    }
    return table;
}

TEST_F(DataformTableTest, testDiskTable)
{
    std::string case_=getCaseName();
    dolphindb::TableSP table = createDemoTable();
    conn.upload("mt", table);
    std::string dbPath = conn.run("getHomeDir()")->getString() + "/cpp_test/"+case_;
    replace(dbPath.begin(), dbPath.end(), '\\', '/');
    std::string script;
    script += "dbPath=\"" + dbPath + "\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db=database(dbPath,VALUE,2010.01.01..2010.01.30);";
    script += "tDiskGlobal=db.createPartitionedTable(mt,`tDiskGlobal,`date);";
    script += "tDiskGlobal.append!(mt);";
    script += "res = select * from tDiskGlobal;";
    conn.run(script);
    std::cout << conn.run("res")->getString();
}

TEST_F(DataformTableTest, testDFSTable)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script;
    dolphindb::TableSP table = createDemoTable();
    conn.upload("mt", table);
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "tableName = `demoTable;";
    script += "db = database(dbPath,VALUE,2010.01.01..2010.01.30);";
    script += "date = db.createPartitionedTable(mt,tableName,`date);";
    script += "tradTable=database(dbPath).loadTable(tableName).append!(mt);";
    script += "res = select * from date;";
    conn.run(script);
    for (int i = 0; i < 3; i++)
        ASSERT_TRUE(conn.run("each(eqObj, res.values(), mt.values());")->get(i)->getBool());
}

TEST_F(DataformTableTest, testDimensionTable)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script;
    dolphindb::TableSP table = createDemoTable();
    conn.upload("mt", table);
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db = database(dbPath,VALUE,2010.01.01..2010.01.30);";
    script += "dt = db.createTable(mt,`dt).append!(mt);";
    script += "res = select * from dt;";
    conn.run(script);
    for (int i = 0; i < 3; i++)
        ASSERT_TRUE(conn.run("each(eqObj, res.values(), mt.values());")->get(i)->getBool());
}

TEST_F(DataformTableTest, testDFSTableSetStringWrong)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script;
    dolphindb::TableSP table = createDemoTableSetStringWrong();
    conn.upload("mt", table);
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "tableName = `demoTable;";
    script += "db = database(dbPath,VALUE,2010.01.01..2010.01.30);";
    script += "db.createPartitionedTable(mt,tableName,`date);";
    script += "tradTable=database(dbPath).loadTable(tableName).append!(mt);";
    script += "select * from tradTable;";
    dolphindb::TableSP result = conn.run(script);
    std::cout << result->getString() << std::endl;
}

TEST_F(DataformTableTest, testDFSTableSetString)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script;
    dolphindb::TableSP table = createDemoTableSetString();
    conn.upload("mt", table);
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "tableName = `demoTable;";
    script += "db = database(dbPath,VALUE,2010.01.01..2010.01.30);";
    script += "db.createPartitionedTable(mt,tableName,`date);";
    script += "tradTable=database(dbPath).loadTable(tableName).append!(mt);";
    script += "res = select * from tradTable;";
    conn.run(script);
    for (int i = 0; i < 3; i++)
        ASSERT_TRUE(conn.run("each(eqObj, res.values(), mt.values());")->get(i)->getBool());
}

TEST_F(DataformTableTest, testinMemoryTableMoreThan65535)
{
    int colNum = 21, rowNum = 3000;
    std::vector<std::string> colNamesVec1;
    for (int i = 0; i < colNum; i++)
    {
        colNamesVec1.emplace_back("col" + std::to_string(i));
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

    srand((int)time(NULL));
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(colNamesVec1, colTypesVec1, rowNum, colNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab1->getColumn(i));
    }
    for (int i = 0; i < rowNum; i++)
    {
        columnVecs[0]->set(i, dolphindb::Util::createChar(rand() % CHAR_MAX));
        columnVecs[1]->set(i, dolphindb::Util::createBool((char)(rand() % 2)));
        columnVecs[2]->set(i, dolphindb::Util::createShort(rand() % SHRT_MAX));
        columnVecs[3]->set(i, dolphindb::Util::createInt(rand() % INT_MAX));
        columnVecs[4]->set(i, dolphindb::Util::createLong(rand() % LONG_MAX));
        columnVecs[5]->set(i, dolphindb::Util::createDate(rand() % LONG_MAX));
        columnVecs[6]->set(i, dolphindb::Util::createMonth(rand() % LONG_MAX));
        columnVecs[7]->set(i, dolphindb::Util::createTime(rand() % LONG_MAX));
        columnVecs[8]->set(i, dolphindb::Util::createMinute(rand() % 1439));
        columnVecs[9]->set(i, dolphindb::Util::createDateTime(rand() % LLONG_MAX));
        columnVecs[10]->set(i, dolphindb::Util::createSecond(rand() % 86400));
        columnVecs[11]->set(i, dolphindb::Util::createTimestamp(rand() % LLONG_MAX));
        columnVecs[12]->set(i, dolphindb::Util::createNanoTime(rand() % LLONG_MAX));
        columnVecs[13]->set(i, dolphindb::Util::createNanoTimestamp(rand() % LLONG_MAX));
        columnVecs[14]->set(i, dolphindb::Util::createFloat(rand() / float(RAND_MAX)));
        columnVecs[15]->set(i, dolphindb::Util::createDouble(rand() / double(RAND_MAX)));
        columnVecs[16]->set(i, dolphindb::Util::createString("str" + std::to_string(i)));
        columnVecs[17]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_UUID, "5d212a78-cc48-e3b1-4235-b4d91473ee87"));
        columnVecs[18]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_IP, "192.0.0." + std::to_string(rand() % 255)));
        columnVecs[19]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_INT128, "e1671797c52e15f763380b45e841ec32"));
        columnVecs[20]->set(i, dolphindb::Util::createBlob("blob" + std::to_string(i)));
    }

    conn.upload("tab1", {tab1});
    dolphindb::TableSP res_tab = conn.run("tab1");

    ASSERT_EQ(tab1->getString(), res_tab->getString());
    for (int i = 0; i < colNum; i++)
        ASSERT_EQ(tab1->getColumn(i)->getType(), res_tab->getColumn(i)->getType());
}

TEST_F(DataformTableTest, testRun)
{
    // 所有参数都在服务器端
    /*conn.run("x = [1, 3, 5]; y = [2, 4, 6]");
    dolphindb::ConstantSP result = conn.run("add(x,y)");
    std::cout<<result->getString()<<std::endl;*/
    // 仅有一个参数在服务器端
    /*conn.run("x = [1, 3, 5]");
    std::vector<dolphindb::ConstantSP> args;
    dolphindb::ConstantSP y = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 3);
    double array_y[] = {1.5, 2.5, 7};
    y->setDouble(0, 3, array_y);
    args.push_back(y);
    dolphindb::ConstantSP result = conn.run("add{x,}", args);
    std::cout<<result->getString()<<std::endl;*/
    // 两个参数都在客户端
    std::vector<dolphindb::ConstantSP> args;
    dolphindb::ConstantSP x = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 3);
    double array_x[] = {1.5, 2.5, 7};
    x->setDouble(0, 3, array_x);
    dolphindb::ConstantSP y = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 3);
    double array_y[] = {8.5, 7.5, 3};
    y->setDouble(0, 3, array_y);
    args.push_back(x);
    args.push_back(y);
    dolphindb::ConstantSP result = conn.run("add", args);
    std::cout << result->getString() << std::endl;
}

TEST_F(DataformTableTest, test_BlockReader_getInmemoryTable)
{
    int64_t tm1, tm2, tm3 = 0;
    long total_size = 10000000;
    std::string script;
    script.append("n=" + std::to_string(total_size) + "\n");
    script.append("syms= `IBM`C`MS`MSFT`JPM`ORCL`BIDU`SOHU`GE`EBAY`GOOG`FORD`GS`PEP`USO`GLD`GDX`EEM`FXI`SLV`SINA`BAC`AAPL`PALL`YHOO`KOH`TSLA`CS`CISO`SUN\n");
    script.append("mytrades=table(09:30:00+rand(18000, n) as timestamp, rand(syms, n) as sym, 10*(1+rand(100, n)) as qty, 5.0+rand(100.0, n) as price); \n");
    conn.run(script);

    std::string sb = "select * from mytrades";
    int fetchSize = 100000;

    tm1 = dolphindb::Util::getEpochTime();
    dolphindb::TableSP mytrades = conn.run(sb);
    tm2 = dolphindb::Util::getEpochTime();
    dolphindb::BlockReaderSP reader = conn.run(sb, 4, 2, fetchSize);
    tm3 = dolphindb::Util::getEpochTime();
    std::vector<dolphindb::TableSP> fetchTableVec;
    long long total = 0;
    while (reader->hasNext())
    {
        for (int i = 0; i < total_size / fetchSize; i++)
        {
            dolphindb::TableSP fetchTable = reader->read();
            fetchTableVec.emplace_back(fetchTable);
            total += fetchTable->size();
            std::cout << fetchTable->size() << ".";
        }
    }
    ASSERT_EQ(total, mytrades->rows());
    std::cout << "total get: " + std::to_string(total) << std::endl;
    std::cout << "without fetchSize,time spend: " << (double)(tm2 - tm1) / (double)1000 << std::endl;
    std::cout << "with fetchSize,time spend: " << (double)(tm3 - tm2) / (double)1000 << std::endl;
}

TEST_F(DataformTableTest, test_BlockReader_getPartitionedTable)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    int64_t tm1, tm2, tm3 = 0;
    long total_size = 10000000;
    std::string script;
    script = 
            "dbName=\""+dbName+"\"\n"
            "n=" + std::to_string(total_size) + "\n\
            syms=`IBM`C`MS`MSFT`JPM`ORCL`BIDU`SOHU`GE`EBAY`GOOG`FORD`GS`PEP`USO`GLD`GDX`EEM`FXI`SLV`SINA`BAC`AAPL`PALL`YHOO`KOH`TSLA`CS`CISO`SUN\n\
            t=table(09:30:00+rand(18000, n) as timestamp, rand(syms, n) as sym, 10*(1+rand(100, n)) as qty, 5.0+rand(100.0, n) as price);\n\
            if(existsDatabase(dbName)){dropDatabase(dbName)};\n\
            db = database(dbName, VALUE, syms)\n\
            test_run_tab = db.createPartitionedTable(table=t, tableName=\"test_BlockReader_getPartitionedTable\", partitionColumns=\"sym\");\n\
            test_run_tab.append!(t)";
    conn.run(script);

    std::string sb = "select * from test_run_tab";
    int fetchSize = 100000;

    tm1 = dolphindb::Util::getEpochTime();
    dolphindb::TableSP mytrades = conn.run(sb);
    tm2 = dolphindb::Util::getEpochTime();
    dolphindb::BlockReaderSP reader = conn.run(sb, 4, 2, fetchSize);
    tm3 = dolphindb::Util::getEpochTime();
    std::vector<dolphindb::TableSP> fetchTableVec;
    long long total = 0;
    while (reader->hasNext())
    {
        for (int i = 0; i < total_size / fetchSize; i++)
        {
            dolphindb::TableSP fetchTable = reader->read();
            fetchTableVec.emplace_back(fetchTable);
            total += fetchTable->size();
            std::cout << fetchTable->size() << ".";
        }
    }
    ASSERT_EQ(total, mytrades->rows());
    std::cout << "total get: " + std::to_string(total) << std::endl;
    std::cout << "without fetchSize,time spend: " << (double)(tm2 - tm1) / (double)1000 << std::endl;
    std::cout << "with fetchSize,time spend: " << (double)(tm3 - tm2) / (double)1000 << std::endl;
}

TEST_F(DataformTableTest, test_BlockReader_skipALL)
{
    std::string script;
    script += R"(login("admin","123456");)";
    script += "pt = table(1..100000 as col1);";
    script += "select * from pt;";
    dolphindb::BlockReaderSP reader = conn.run(script, 4, 2, 8200);
    dolphindb::ConstantSP t = reader->read();
    std::cout << reader->read()->rows() << std::endl;
    ASSERT_TRUE(reader->hasNext());
    reader->skipAll();
    ASSERT_FALSE(reader->hasNext());
}

static void Block_Reader_DFStable()
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script;
    std::string script1;
    dolphindb::TableSP table = createDemoTable();
    DataformTableTest::conn.upload("mt", table);
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "tableName = `pt;";
    script += "db = database(dbPath,VALUE,2010.01.01..2010.01.30);";
    script += "pt = db.createPartitionedTable(mt,tableName,`date);";
    script += "pt.append!(mt);";
    script += "n=12450;";
    script += "t_Block_Reader_DFStable=table(take(`name_3,n) as name,take(2010.01.01,n) as date,rand(30,n) as price);";
    script += "pt.append!(t_Block_Reader_DFStable);";
    DataformTableTest::conn.run(script);
    script1 += "select * from pt;";
    int fetchsize1 = 12453;
    dolphindb::BlockReaderSP reader = DataformTableTest::conn.run(script1, 4, 2, fetchsize1);
    dolphindb::ConstantSP t;
    int total = 0;
    while (reader->hasNext())
    {
        t = reader->read();
        total += t->size();
        ASSERT_EQ(t->size(), fetchsize1);
    }
    ASSERT_EQ(total, 12453);

    int fetchsize2 = 8200;
    dolphindb::BlockReaderSP reader2 = DataformTableTest::conn.run(script1, 4, 2, fetchsize2);
    dolphindb::ConstantSP t2;
    int total2 = 0;
    int tmp = fetchsize2;
    while (reader2->hasNext())
    {
        t2 = reader2->read();
        total2 += t2->size();
        ASSERT_EQ(t2->size(), tmp);
        tmp = 12453 - tmp;
    }
    ASSERT_EQ(total2, 12453);

    int fetchsize3 = 15000;
    dolphindb::BlockReaderSP reader3 = DataformTableTest::conn.run(script1, 4, 2, fetchsize3);
    dolphindb::ConstantSP t3;
    int total3 = 0;
    while (reader3->hasNext())
    {
        t3 = reader3->read();
        total3 += t3->size();
        ASSERT_EQ(t3->size(), 12453);
    }
    ASSERT_EQ(total3, 12453);
}

static void Block_Table()
{
    std::string script;
    std::string script1;
    script += "rows=12453;";
    script += "testblock=table(take(1,rows) as id,take(`A,rows) as symbol,take(2020.08.01..2020.10.01,rows) as date, rand(50,rows) as size,rand(50.5,rows) as price);";
    DataformTableTest::conn.run(script);
    script1 += "select * from testblock ";
    int fetchsize1 = 12453;
    dolphindb::BlockReaderSP reader = DataformTableTest::conn.run(script1, 4, 2, fetchsize1);
    dolphindb::ConstantSP t;
    int total = 0;
    while (reader->hasNext())
    {
        t = reader->read();
        total += t->size();
        ASSERT_EQ(t->size(), fetchsize1);
    }
    ASSERT_EQ(total, 12453);

    int fetchsize2 = 8200;
    dolphindb::BlockReaderSP reader2 = DataformTableTest::conn.run(script1, 4, 2, fetchsize2);
    dolphindb::ConstantSP t2;
    int total2 = 0;
    int tmp = fetchsize2;
    while (reader2->hasNext())
    {
        t2 = reader2->read();
        total2 += t2->size();
        ASSERT_EQ(t2->size(), tmp);
        tmp = 12453 - tmp;
    }
    ASSERT_EQ(total2, 12453);

    int fetchsize3 = 15000;
    dolphindb::BlockReaderSP reader3 = DataformTableTest::conn.run(script1, 4, 2, fetchsize3);
    dolphindb::ConstantSP t3;
    int total3 = 0;
    while (reader3->hasNext())
    {
        t3 = reader3->read();
        total3 += t3->size();
        ASSERT_EQ(t3->size(), 12453);
    }
    ASSERT_EQ(total3, 12453);
}

TEST_F(DataformTableTest, test_huge_table)
{
    std::string script;
    std::string script1;
    script += "rows=20000000;";
    script += "testblock=table(take(1,rows) as id,take(`A,rows) as symbol,take(2020.08.01..2020.10.01,rows) as date, rand(50,rows) as size,rand(50.5,rows) as price);";
    conn.run(script);
    script1 += "select * from testblock;";
    dolphindb::BlockReaderSP reader = conn.run(script1, 4, 2, 8200);
    dolphindb::ConstantSP t;
    int total = 0;
    int i = 1;
    int fetchsize = 8200;
    while (reader->hasNext())
    {
        t = reader->read();
        total += t->size();
        // std::cout<< "read" <<t->size()<<std::endl;

        if (t->size() == 8200)
        {
            ASSERT_EQ(t->size(), 8200);
        }
        else
        {
            ASSERT_EQ(t->size(), 200);
        }
    }
    // std::cout<<"total is"<<total<<std::endl;
    ASSERT_EQ(total, 20000000);
}

TEST_F(DataformTableTest, test_huge_DFS)
{
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script;
    std::string script1;
    dolphindb::TableSP table = createDemoTable();
    conn.upload("mt", table);
    script += "dbPath = \""+dbName+"\";";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "tableName = `pt;";
    script += "db = database(dbPath,VALUE,2010.01.01..2010.01.30);";
    script += "pt = db.createPartitionedTable(mt,tableName,`date);";
    script += "pt.append!(mt);";
    script += "n=2000000;";
    script += "t_test_huge_DFS=table(take(`name_3,n) as name,take(2010.01.01,n) as date,rand(30,n) as price);";
    script += "pt.append!(t_test_huge_DFS);";
    conn.run(script);
    script1 += "select * from pt;";
    int fetchsize1 = 8200;
    dolphindb::BlockReaderSP reader = conn.run(script1, 4, 2, fetchsize1);
    dolphindb::ConstantSP t;
    int total = 0;
    while (reader->hasNext())
    {
        t = reader->read();
        total += t->size();
        if (t->size() == 8200)
        {
            ASSERT_EQ(t->size(), 8200);
        }
        else
        {
            ASSERT_EQ(t->size(), 7403);
        }
    }
    ASSERT_EQ(total, 2000003);

    int fetchsize2 = 200000;
    dolphindb::BlockReaderSP reader2 = conn.run(script1, 4, 2, fetchsize2);
    dolphindb::ConstantSP t2;
    int total2 = 0;
    while (reader2->hasNext())
    {
        t2 = reader2->read();
        total2 += t2->size();

        if (t2->size() == 200000)
        {
            ASSERT_EQ(t2->size(), 200000);
        }
        else
        {
            ASSERT_EQ(t2->size(), 3);
        }

    }
    ASSERT_EQ(total2, 2000003);
}

TEST_F(DataformTableTest, test_Block_Reader_DFStable_While_Block_Table)
{
    std::thread t1(Block_Table);
    t1.join();
    std::thread t2(Block_Reader_DFStable);
    t2.join();
}

TEST_F(DataformTableTest, testinMemoryTableMoreThan1048576withAlldataTypes)
{
    int colNum = 26, rowNum = 70000; // create a table with 70000 rows.
    std::vector<std::string> colNamesVec1;
    for (int i = 0; i < colNum; i++)
    {
        colNamesVec1.emplace_back("col" + std::to_string(i));
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
    colTypesVec1.emplace_back(dolphindb::DT_SYMBOL);

    srand((int)time(NULL));
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(colNamesVec1, colTypesVec1, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab1->getColumn(i));
    }
    for (int i = 0; i < rowNum - 1; i++)
    {
        columnVecs[0]->set(i, dolphindb::Util::createChar(rand() % CHAR_MAX));
        columnVecs[1]->set(i, dolphindb::Util::createBool((char)(rand() % 2)));
        columnVecs[2]->set(i, dolphindb::Util::createShort(rand() % SHRT_MAX));
        columnVecs[3]->set(i, dolphindb::Util::createInt(rand() % INT_MAX));
        columnVecs[4]->set(i, dolphindb::Util::createLong(rand() % LLONG_MAX));
        columnVecs[5]->set(i, dolphindb::Util::createDate(rand() % INT_MAX));
        columnVecs[6]->set(i, dolphindb::Util::createMonth(rand() % INT_MAX));
        columnVecs[7]->set(i, dolphindb::Util::createTime(rand() % INT_MAX));
        columnVecs[8]->set(i, dolphindb::Util::createMinute(rand() % 1440));
        columnVecs[9]->set(i, dolphindb::Util::createDateTime(rand() % INT_MAX));
        columnVecs[10]->set(i, dolphindb::Util::createSecond(rand() % 86400));
        columnVecs[11]->set(i, dolphindb::Util::createTimestamp(rand() % LLONG_MAX));
        columnVecs[12]->set(i, dolphindb::Util::createNanoTime(rand() % LLONG_MAX));
        columnVecs[13]->set(i, dolphindb::Util::createNanoTimestamp(rand() % LLONG_MAX));
        columnVecs[14]->set(i, dolphindb::Util::createFloat(rand() / float(RAND_MAX)));
        columnVecs[15]->set(i, dolphindb::Util::createDouble(rand() / double(RAND_MAX)));
        columnVecs[16]->set(i, dolphindb::Util::createString("str" + std::to_string(i)));
        columnVecs[17]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_UUID, "5d212a78-cc48-e3b1-4235-b4d91473ee87"));
        columnVecs[18]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_IP, "192.0.0." + std::to_string(rand() % 255)));
        columnVecs[19]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_INT128, "e1671797c52e15f763380b45e841ec32"));
        columnVecs[20]->set(i, dolphindb::Util::createBlob("blob" + std::to_string(i)));
        columnVecs[21]->set(i, dolphindb::Util::createDateHour(rand() % INT_MAX));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal32(rand() % 10, rand() / float(RAND_MAX)));
        columnVecs[23]->set(i, dolphindb::Util::createDecimal64(rand() % 19, rand() / double(RAND_MAX)));
        columnVecs[24]->set(i, dolphindb::Util::createDecimal128(rand() % 38, rand() / double(RAND_MAX)));
        columnVecs[25]->set(i, dolphindb::Util::createString("sym" + std::to_string(i)));
    }
    for (int j = 0; j < colNum; j++)
        columnVecs[j]->setNull(rowNum - 1);

    conn.upload("tab1", {tab1});
    dolphindb::TableSP res_tab = conn.run("tab1");
    std::cout << res_tab->getString();
    ASSERT_EQ(tab1->getString(), res_tab->getString());
    for (int i = 0; i < colNum; i++)
    {
        if (i == 25)
            ASSERT_EQ(tab1->getColumn(i)->getType(), dolphindb::DT_SYMBOL);
        else
            ASSERT_EQ(tab1->getColumn(i)->getType(), res_tab->getColumn(i)->getType());
    }

    { // test class dolphindb::ResultSet(dolphindb::TableSP)
        dolphindb::ResultSet resultSet(tab1);
        ASSERT_FALSE(resultSet.isBeforeFirst());
        ASSERT_EQ(resultSet.position(), 0);
        ASSERT_ANY_THROW(resultSet.position(-1));
        ASSERT_ANY_THROW(resultSet.position(tab1->rows() + 1));
        resultSet.position(5);
        ASSERT_EQ(resultSet.position(), 5);
        resultSet.first();
        while (resultSet.isAfterLast() == false)
        {
            int rowIndex = resultSet.position();
            int colIndex = 0;
            if (resultSet.isFirst())
                std::cout << "resultset assert begins" << std::endl;

            for (auto i = 0; i < colNum; i++)
            {
                resultSet.getBinary(i);
                // ASSERT_EQ(*resultSet.getBinary(i), *(res_tab->getColumn(i)->get(rowIndex)->getBinary())); // https://dolphindb1.atlassian.net/browse/AC-408
                if (i == 25)
                    ASSERT_EQ(resultSet.getDataType(i), dolphindb::DT_SYMBOL);
                else
                    ASSERT_EQ(resultSet.getDataType(i), res_tab->getColumnType(i));
            }
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createChar(resultSet.getChar(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createBool(resultSet.getBool(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createShort(resultSet.getShort(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createInt(resultSet.getInt(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createLong(resultSet.getLong(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createDate(resultSet.getInt(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createMonth(resultSet.getInt(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createTime(resultSet.getInt(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createMinute(resultSet.getInt(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createDateTime(resultSet.getInt(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createSecond(resultSet.getInt(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createTimestamp(resultSet.getLong(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createNanoTime(resultSet.getLong(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createNanoTimestamp(resultSet.getLong(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createFloat(resultSet.getFloat(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createDouble(resultSet.getDouble(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createString(resultSet.getString(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getString(colIndex++), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createDateHour(resultSet.getInt(colIndex++)))->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), res_tab->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getString(colIndex++), res_tab->getColumn(colIndex)->get(rowIndex)->getString());

            if (resultSet.isLast())
                std::cout << "resultset assert finished" << std::endl;
            resultSet.next();
        }
        resultSet.last();
        resultSet.next();
        ASSERT_TRUE(resultSet.isAfterLast());
        ASSERT_EQ(resultSet.position(), tab1->rows());
    }
}

TEST_F(DataformTableTest, testTablewithArrayVector)
{
    int colNum = 23, rowNum = 100, size = 10; // create a table with 100 rows.
    std::vector<std::string> colNamesVec1;
    for (int i = 0; i < colNum; i++)
    {
        colNamesVec1.emplace_back("col" + std::to_string(i));
    }
    std::vector<dolphindb::DATA_TYPE> colTypesVec1;
    colTypesVec1.emplace_back(dolphindb::DT_CHAR_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_BOOL_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_SHORT_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_INT_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_LONG_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_DATE_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_MONTH_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_TIME_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_MINUTE_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_DATETIME_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_SECOND_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_TIMESTAMP_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIME_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_NANOTIMESTAMP_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_DATEHOUR_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_FLOAT_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_DOUBLE_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_UUID_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_IP_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_INT128_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL32_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL64_ARRAY);
    colTypesVec1.emplace_back(dolphindb::DT_DECIMAL128_ARRAY);

    srand(time(NULL));
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(colNamesVec1, colTypesVec1, rowNum - 1, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab1->getColumn(i));
    }

    dolphindb::DdbVector<char> charV(0, size);
    dolphindb::DdbVector<char> boolV(0, size);
    dolphindb::DdbVector<short> shortV(0, size);
    dolphindb::DdbVector<int> intV(0, size);
    dolphindb::DdbVector<long long> longV(0, size);
    dolphindb::DdbVector<int> dateV(0, size);
    dolphindb::DdbVector<int> monthV(0, size);
    dolphindb::DdbVector<int> timeV(0, size);
    dolphindb::DdbVector<int> minuteV(0, size);
    dolphindb::DdbVector<int> datetimeV(0, size);
    dolphindb::DdbVector<int> secondV(0, size);
    dolphindb::DdbVector<long long> timestampV(0, size);
    dolphindb::DdbVector<long long> nanotimeV(0, size);
    dolphindb::DdbVector<long long> nanotimestampV(0, size);
    dolphindb::DdbVector<int> datehourV(0, size);
    dolphindb::DdbVector<float> floatV(0, size);
    dolphindb::DdbVector<double> doubleV(0, size);
    dolphindb::DdbVector<dolphindb::Guid> uuidV(0, size);
    dolphindb::DdbVector<dolphindb::Guid> ipV(0, size);
    dolphindb::DdbVector<dolphindb::Guid> int128V(0, size);
    dolphindb::VectorSP ddbdecimal32V = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, 0, size, true, 6);
    dolphindb::VectorSP ddbdecimal64V = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, 0, size, true, 16);
    dolphindb::VectorSP ddbdecimal128V = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, 0, size, true, 26);
    for (auto i = 0; i < size - 1; i++)
    {
        doubleV.add(i);
        floatV.add(i);
        intV.add(i);
        shortV.add(i);
        longV.add(i);
        boolV.add(i == 0 ? 0 : 1);
        charV.add(i);
        uuidV.add(dolphindb::Guid(true));
        int128V.add(dolphindb::Guid(true));
        ipV.add(dolphindb::Guid(true));
        dateV.add(i);
        minuteV.add(i);
        datetimeV.add(i);
        nanotimeV.add((long long)i);
        datehourV.add(i);
        monthV.add(i);
        timeV.add(i);
        secondV.add(i);
        timestampV.add(i);
        nanotimestampV.add(i);
        ddbdecimal32V->append(dolphindb::Util::createInt(i));
        ddbdecimal64V->append(dolphindb::Util::createInt(i));
        ddbdecimal128V->append(dolphindb::Util::createInt(i));
    }
    doubleV.addNull();
    floatV.addNull();
    intV.addNull();
    shortV.addNull();
    longV.addNull();
    boolV.addNull();
    charV.addNull();
    int128V.addNull();
    uuidV.addNull();
    ipV.addNull();
    dateV.addNull();
    minuteV.addNull();
    datetimeV.addNull();
    nanotimeV.addNull();
    datehourV.addNull();
    monthV.addNull();
    timeV.addNull();
    secondV.addNull();
    timestampV.addNull();
    nanotimestampV.addNull();
    ddbdecimal32V->append(dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL32, 6));
    ddbdecimal64V->append(dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL64, 16));
    ddbdecimal128V->append(dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL128, 26));

    dolphindb::VectorSP ddbcharV = charV.createVector(dolphindb::DT_CHAR);
    dolphindb::VectorSP ddbboolV = boolV.createVector(dolphindb::DT_BOOL);
    dolphindb::VectorSP ddbshortV = shortV.createVector(dolphindb::DT_SHORT);
    dolphindb::VectorSP ddbintV = intV.createVector(dolphindb::DT_INT);
    dolphindb::VectorSP ddblongV = longV.createVector(dolphindb::DT_LONG);
    dolphindb::VectorSP ddbdateV = dateV.createVector(dolphindb::DT_DATE);
    dolphindb::VectorSP ddbmonthV = monthV.createVector(dolphindb::DT_MONTH);
    dolphindb::VectorSP ddbtimeV = timeV.createVector(dolphindb::DT_TIME);
    dolphindb::VectorSP ddbminuteV = minuteV.createVector(dolphindb::DT_MINUTE);
    dolphindb::VectorSP ddbdatetimeV = datetimeV.createVector(dolphindb::DT_DATETIME);
    dolphindb::VectorSP ddbsecondV = secondV.createVector(dolphindb::DT_SECOND);
    dolphindb::VectorSP ddbtimestampV = timestampV.createVector(dolphindb::DT_TIMESTAMP);
    dolphindb::VectorSP ddbnanotimeV = nanotimeV.createVector(dolphindb::DT_NANOTIME);
    dolphindb::VectorSP ddbnanotimestampV = nanotimestampV.createVector(dolphindb::DT_NANOTIMESTAMP);
    dolphindb::VectorSP ddbdatehourV = datehourV.createVector(dolphindb::DT_DATEHOUR);
    dolphindb::VectorSP ddbfloatV = floatV.createVector(dolphindb::DT_FLOAT);
    dolphindb::VectorSP ddbdoubleV = doubleV.createVector(dolphindb::DT_DOUBLE);
    dolphindb::VectorSP ddbuuidV = uuidV.createVector(dolphindb::DT_UUID);
    dolphindb::VectorSP ddbipV = ipV.createVector(dolphindb::DT_IP);
    dolphindb::VectorSP ddbint128V = int128V.createVector(dolphindb::DT_INT128);

    // 上传int128, uuid, ip的value vector到server作为数据构造arrayVector
    std::vector<std::string> binaryVname = {"uuidv", "ipv", "int128v"};
    std::vector<dolphindb::ConstantSP> binaryV = {ddbuuidV, ddbipV, ddbint128V};
    conn.upload(binaryVname, binaryV);

    for (int i = 0; i < rowNum - 1; i++)
    {
        columnVecs[0]->set(i, ddbcharV);
        columnVecs[1]->set(i, ddbboolV);
        columnVecs[2]->set(i, ddbshortV);
        columnVecs[3]->set(i, ddbintV);
        columnVecs[4]->set(i, ddblongV);
        columnVecs[5]->set(i, ddbdateV);
        columnVecs[6]->set(i, ddbmonthV);
        columnVecs[7]->set(i, ddbtimeV);
        columnVecs[8]->set(i, ddbminuteV);
        columnVecs[9]->set(i, ddbdatetimeV);
        columnVecs[10]->set(i, ddbsecondV);
        columnVecs[11]->set(i, ddbtimestampV);
        columnVecs[12]->set(i, ddbnanotimeV);
        columnVecs[13]->set(i, ddbnanotimestampV);
        columnVecs[14]->set(i, ddbdatehourV);
        columnVecs[15]->set(i, ddbfloatV);
        columnVecs[16]->set(i, ddbdoubleV);
        columnVecs[17]->set(i, ddbuuidV);
        columnVecs[18]->set(i, ddbipV);
        columnVecs[19]->set(i, ddbint128V);
        columnVecs[20]->set(i, ddbdecimal32V);
        columnVecs[21]->set(i, ddbdecimal64V);
        columnVecs[22]->set(i, ddbdecimal128V);
    }
    for (int j = 0; j < colNum; j++){
        columnVecs[j]->append(dolphindb::Util::createNullConstant(dolphindb::DT_VOID));
    }
    tab1->updateSize();

    conn.upload("tab1", {tab1});
    dolphindb::TableSP res_tab = conn.run("tab1");
    ASSERT_EQ(tab1->getString(), res_tab->getString());
    for (int i = 0; i < colNum; i++)
    {
        ASSERT_EQ(tab1->getColumn(i)->getType(), res_tab->getColumn(i)->getType());
        ASSERT_EQ(tab1->getColumn(i)->getExtraParamForType(), res_tab->getColumn(i)->getExtraParamForType());
    }

    conn.run(
        "cbool= array(BOOL[]).append!([(0..8).append!(NULL)]);"
        "cchar = array(CHAR[]).append!([(0..8).append!(NULL)]);"
        "cshort = array(SHORT[]).append!([(0..8).append!(NULL)]);"
        "cint = array(INT[]).append!([(0..8).append!(NULL)]);"
        "clong = array(LONG[]).append!([(0..8).append!(NULL)]);"
        "cdate = array(DATE[]).append!([(0..8).append!(NULL)]);"
        "cmonth = array(MONTH[]).append!([(0..8).append!(NULL)]);"
        "ctime = array(TIME[]).append!([(0..8).append!(NULL)]);"
        "cminute = array(MINUTE[]).append!([(0..8).append!(NULL)]);"
        "csecond = array(SECOND[]).append!([(0..8).append!(NULL)]);"
        "cdatetime = array(DATETIME[]).append!([(0..8).append!(NULL)]);"
        "ctimestamp = array(TIMESTAMP[]).append!([(0..8).append!(NULL)]);"
        "cnanotime = array(NANOTIME[]).append!([(0..8).append!(NULL)]);"
        "cnanotimestamp = array(NANOTIMESTAMP[]).append!([(0..8).append!(NULL)]);"
        "cdatehour = array(DATEHOUR[]).append!([(0..8).append!(NULL)]);"
        "cfloat = array(FLOAT[]).append!([(0..8).append!(NULL)]);"
        "cdouble = array(DOUBLE[]).append!([(0..8).append!(NULL)]);"
        "cipaddr = array(IPADDR[]).append!([ipv]);"
        "cuuid = array(UUID[]).append!([uuidv]);"
        "cint128 = array(INT128[]).append!([int128v]);"
        "cdecimal32 = array(DECIMAL32(6)[], 0, 10).append!(decimal32([(0..8).append!(NULL)], 6));"
        "cdecimal64 = array(DECIMAL64(16)[], 0, 10).append!(decimal64([(0..8).append!(NULL)], 16));"
        "cdecimal128 = array(DECIMAL128(26)[], 0, 10).append!(decimal128([(0..8).append!(NULL)], 26));"
        "table1=table(cchar,cbool,cshort,cint,clong,cdate,cmonth,ctime,cminute,cdatetime,csecond,ctimestamp,cnanotime,cnanotimestamp,cdatehour,cfloat,cdouble,cuuid,cipaddr,cint128,cdecimal32,cdecimal64,cdecimal128);"
        "for (i in 1:"+std::to_string(rowNum-1)+"){"
        "    tableInsert(table1, cchar,cbool,cshort,cint,clong,cdate,cmonth,ctime,cminute,cdatetime,csecond,ctimestamp,cnanotime,cnanotimestamp,cdatehour,cfloat,cdouble,cuuid,cipaddr,cint128,cdecimal32,cdecimal64,cdecimal128)}"
        "tableInsert(table1,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL)"
    );
    conn.run("for (i in 0:"+std::to_string(colNum-1)+"){assert i, eqObj(table1.column(i), tab1.column(i))}");
}

TEST_F(DataformTableTest, test_MvccTableMoreThan1048576_withAlldataTypes)
{
    int colNum = 26, rowNum = 70000; // create a table with 70000 rows.
    std::vector<std::string> colNamesVec1;
    for (int i = 0; i < colNum; i++)
    {
        colNamesVec1.emplace_back("col" + std::to_string(i));
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
    colTypesVec1.emplace_back(dolphindb::DT_SYMBOL);

    srand((int)time(NULL));
    dolphindb::TableSP tab1 = dolphindb::Util::createTable(colNamesVec1, colTypesVec1, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++)
    {
        columnVecs.emplace_back(tab1->getColumn(i));
    }
    for (int i = 0; i < rowNum - 1; i++)
    {
        columnVecs[0]->set(i, dolphindb::Util::createChar(rand() % CHAR_MAX));
        columnVecs[1]->set(i, dolphindb::Util::createBool((char)(rand() % 2)));
        columnVecs[2]->set(i, dolphindb::Util::createShort(rand() % SHRT_MAX));
        columnVecs[3]->set(i, dolphindb::Util::createInt(rand() % INT_MAX));
        columnVecs[4]->set(i, dolphindb::Util::createLong(rand() % LLONG_MAX));
        columnVecs[5]->set(i, dolphindb::Util::createDate(rand() % INT_MAX));
        columnVecs[6]->set(i, dolphindb::Util::createMonth(rand() % INT_MAX));
        columnVecs[7]->set(i, dolphindb::Util::createTime(rand() % INT_MAX));
        columnVecs[8]->set(i, dolphindb::Util::createMinute(rand() % 1440));
        columnVecs[9]->set(i, dolphindb::Util::createDateTime(rand() % INT_MAX));
        columnVecs[10]->set(i, dolphindb::Util::createSecond(rand() % 86400));
        columnVecs[11]->set(i, dolphindb::Util::createTimestamp(rand() % LLONG_MAX));
        columnVecs[12]->set(i, dolphindb::Util::createNanoTime(rand() % LLONG_MAX));
        columnVecs[13]->set(i, dolphindb::Util::createNanoTimestamp(rand() % LLONG_MAX));
        columnVecs[14]->set(i, dolphindb::Util::createFloat(rand() / float(RAND_MAX)));
        columnVecs[15]->set(i, dolphindb::Util::createDouble(rand() / double(RAND_MAX)));
        columnVecs[16]->set(i, dolphindb::Util::createString("str" + std::to_string(i)));
        columnVecs[17]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_UUID, "5d212a78-cc48-e3b1-4235-b4d91473ee87"));
        columnVecs[18]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_IP, "192.0.0." + std::to_string(rand() % 255)));
        columnVecs[19]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_INT128, "e1671797c52e15f763380b45e841ec32"));
        columnVecs[20]->set(i, dolphindb::Util::createBlob("blob" + std::to_string(i)));
        columnVecs[21]->set(i, dolphindb::Util::createDateHour(rand() % INT_MAX));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal32(rand() % 10, rand() / float(RAND_MAX)));
        columnVecs[23]->set(i, dolphindb::Util::createDecimal64(rand() % 19, rand() / double(RAND_MAX)));
        columnVecs[24]->set(i, dolphindb::Util::createDecimal128(rand() % 39, rand() / double(RAND_MAX)));
        columnVecs[25]->set(i, dolphindb::Util::createString("sym" + std::to_string(i)));
    }
    for (int j = 0; j < colNum; j++)
        columnVecs[j]->setNull(rowNum - 1);

    conn.upload("tab1", {tab1});
    dolphindb::TableSP mvcc_tab2 = conn.run("colNames = schema(tab1).colDefs[`name];colTypes = schema(tab1).colDefs[`TypeInt];tab2 = mvccTable(1:0, colNames,colTypes).append!(tab1);tab2");
    // std::cout << mvcc_tab2->getString();
    ASSERT_EQ(tab1->getString(), mvcc_tab2->getString());
    for (int i = 0; i < colNum; i++)
    {
        if (i == colNum-1)
            ASSERT_EQ(tab1->getColumn(i)->getType(), dolphindb::DT_SYMBOL);
        else
            ASSERT_EQ(tab1->getColumn(i)->getType(), mvcc_tab2->getColumn(i)->getType());
        ASSERT_EQ(tab1->getColumn(i)->getExtraParamForType(), mvcc_tab2->getColumn(i)->getExtraParamForType());
    }

    { // test class dolphindb::ResultSet(dolphindb::TableSP)
        dolphindb::ResultSet resultSet(tab1);
        ASSERT_FALSE(resultSet.isBeforeFirst());
        ASSERT_EQ(resultSet.position(), 0);
        resultSet.first();
        while (resultSet.isAfterLast() == false)
        {
            int rowIndex = resultSet.position();
            int colIndex = 0;
            if (resultSet.isFirst())
                std::cout << "resultset assert begins" << std::endl;

            for (auto i = 0; i < colNum; i++)
            {
                if (i == 22 || i == 23 || i == 24)
                    ASSERT_EQ(*resultSet.getBinary(i), *(mvcc_tab2->getColumn(i)->get(rowIndex)->getBinary()));
                else if (i == colNum-1)
                    ASSERT_EQ(resultSet.getDataType(i), dolphindb::DT_SYMBOL);
                else
                    ASSERT_EQ(resultSet.getDataType(i), mvcc_tab2->getColumnType(i));
            }
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createChar(resultSet.getChar(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createBool(resultSet.getBool(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createShort(resultSet.getShort(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createInt(resultSet.getInt(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createLong(resultSet.getLong(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createDate(resultSet.getInt(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createMonth(resultSet.getInt(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createTime(resultSet.getInt(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createMinute(resultSet.getInt(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createDateTime(resultSet.getInt(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createSecond(resultSet.getInt(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createTimestamp(resultSet.getLong(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createNanoTime(resultSet.getLong(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createNanoTimestamp(resultSet.getLong(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createFloat(resultSet.getFloat(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createDouble(resultSet.getDouble(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createString(resultSet.getString(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getString(colIndex++), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(((dolphindb::ConstantSP)dolphindb::Util::createDateHour(resultSet.getInt(colIndex++)))->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getObject(colIndex++)->getString(), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());
            ASSERT_EQ(resultSet.getString(colIndex++), mvcc_tab2->getColumn(colIndex)->get(rowIndex)->getString());

            if (resultSet.isLast())
                std::cout << "resultset assert finished" << std::endl;
            resultSet.next();
        }
        resultSet.last();
        resultSet.next();
        ASSERT_TRUE(resultSet.isAfterLast());
        ASSERT_EQ(resultSet.position(), tab1->rows());
    }
}

TEST_F(DataformTableTest, test_append_vector_to_scalar_column)
{
    std::vector<std::string> colNames = {"col1", "col2"};
    std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SYMBOL, dolphindb::DT_STRING};
    dolphindb::TableSP table = dolphindb::Util::createTable(colNames, colTypes, 0, 1);

    std::vector<dolphindb::ConstantSP> bufferedOutput_;
    for (dolphindb::INDEX i = 0; i < table->columns(); ++i)
    {
        bufferedOutput_.push_back(dolphindb::Util::createVector(table->getColumnType(i), 1));
    }

    bufferedOutput_[0]->setString("code1");
    bufferedOutput_[1]->setString("code1");
    dolphindb::INDEX insertedRows = -1;
    std::string lastMessage;
    if (!table->append(bufferedOutput_, insertedRows, lastMessage))
    {
        throw dolphindb::RuntimeException("append matchResult Table Fail for " + lastMessage);
    }

    ASSERT_EQ(table->getString(), "col1  col2 \n"
                                  "----- -----\n"
                                  "code1 code1\n");
}

TEST_F(DataformTableTest, test_table_getRowJson)
{
    std::string script=R"(
        table(
            take([true,false,00b],100) as `bool_type,
            take([1c,0c,00c],100) as `char_type,
            take([1h,0h,00h],100) as `short_type,
            take([1i,0i,00i],100) as `int_type,
            take([1l,0l,00l],100) as `long_type,
            take([2013.06.13d,1970.01.01d,00d],100) as `date_type,
            take([2013.06M,1970.01M,00M],100) as `month_type,
            take([13:30:10.008t,00:00:00.000t,00t],100) as `time_type,
            take([13:30m,00:00m,00m],100) as `minute_type,
            take([13:30:00s,00:00:00s,00s],100) as `second_type,
            take([2012.06.13T13:30:10D,1970.01.01T00:00:00D,00D],100) as `datetime_type,
            take([2012.06.13T13:30:10.008T,1970.01.01T00:00:00.000T,00T],100) as `timestamp_type,
            take([13:30:10.008007006n,00:00:00.000000000n,00n],100) as `nanotime_type,
            take([2012.06.13T13:30:10.008007006N,1970.01.01T00:00:00.000000000N,00N],100) as `nanotimestamp_type,
            take([float('nan'),float('inf'),3.14f,00f],100) as `float_type,
            take([double('nan'),double('inf'),3.14F,00F],100) as `double_type,
            take(["123,456","123",""],100) as `string_type,
            take([uuid("5d212a78-cc48-e3b1-4235-b4d91473ee87"),uuid("00000000-0000-0000-0000-000000000000")],100) as `uuid_type,
            take([datehour("2012.06.13T13"),datehour("1970.01.01T00"),datehour(null)],100) as `datehour_type,
            take([ipaddr("127.0.0.1"),ipaddr("0.0.0.0")],100) as `ipaddr_type,
            take([int128("e1671797c52e15f763380b45e841ec32"),int128("00000000000000000000000000000000")],100) as `int128_type,
            take([blob("123"),blob("")],100) as `blob_type,
            take([decimal32("3.14",2),decimal32("0",2),decimal32("nan",2)],100) as `decimal32_type,
            take([decimal64("3.14",2),decimal64("0",2),decimal64("nan",2)],100) as `decimal64_type,
            take([decimal128("3.14",2),decimal128("0",2),decimal128("nan",2)],100) as `decimal128_type,
            take(array(BOOL[],0,3).append!([true false 00b,true true,false false]),100) as `bool_array_type,
            take(array(CHAR[],0,3).append!([1c 0c 00c,1c 1c,0c 0c]),100) as `char_array_type,
            take(array(SHORT[],0,3).append!([1h 0h 00h,1h 1h,0h 0h]),100) as `short_array_type,
            take(array(INT[],0,3).append!([1i 0i 00i,1i 1i,0i 0i]),100) as `int_array_type,
            take(array(LONG[],0,3).append!([1l 0l 00l,1l 1l,0l 0l]),100) as `long_array_type,
            take(array(DATE[],0,3).append!([2013.06.13d 1970.01.01d 00d]),100) as `date_array_type,
            take(array(MONTH[],0,3).append!([2013.06M 1970.01M 00M]),100) as `month_array_type,
            take(array(TIME[],0,3).append!([13:30:10.008t 00:00:00.000t 00t]),100) as `time_array_type,
            take(array(MINUTE[],0,3).append!([13:30m 00:00m 00m]),100) as `minute_array_type,
            take(array(SECOND[],0,3).append!([13:30:00s 00:00:00s 00s]),100) as `second_array_type,
            take(array(DATETIME[],0,3).append!([2012.06.13T13:30:10D 1970.01.01T00:00:00D 00D]),100) as `datetime_array_type,
            take(array(TIMESTAMP[],0,3).append!([2012.06.13T13:30:10.008T 1970.01.01T00:00:00.000T 00T]),100) as `timestamp_array_type,
            take(array(NANOTIME[],0,3).append!([13:30:10.008007006n 00:00:00.000000000n 00n]),100) as `nanotime_array_type,
            take(array(NANOTIMESTAMP[],0,3).append!([2012.06.13T13:30:10.008007006N 1970.01.01T00:00:00.000000000N 00N]),100) as `nanotimestamp_array_type,
            take(array(FLOAT[],0,3).append!([[float('nan'),float('inf'),3.14f,00f]]),100) as `float_array_type,
            take(array(DOUBLE[],0,3).append!([[double('nan'),double('inf'),3.14F,00F]]),100) as `double_array_type,
            take(array(UUID[],0,3).append!([[uuid("5d212a78-cc48-e3b1-4235-b4d91473ee87"),uuid("00000000-0000-0000-0000-000000000000")]]),100) as `uuid_array_type,
            take(array(DATEHOUR[],0,3).append!([[datehour("2012.06.13T13"),datehour("1970.01.01T00"),datehour(null)]]),100) as `datehour_array_type,
            take(array(IPADDR[],0,3).append!([[ipaddr("127.0.0.1"),ipaddr("0.0.0.0")]]),100) as `ipaddr_array_type,
            take(array(INT128[],0,3).append!([[int128("e1671797c52e15f763380b45e841ec32"),int128("00000000000000000000000000000000")]]),100) as `int128_array_type,
            take(array(DECIMAL32(2)[],0,3).append!([[decimal32("3.14",2),decimal32("0",2),decimal32("nan",2)]]),100) as `decimal32_array_type,
            take(array(DECIMAL64(2)[],0,3).append!([[decimal64("3.14",2),decimal64("0",2),decimal64("nan",2)]]),100) as `decimal64_array_type,
            take(array(DECIMAL128(2)[],0,3).append!([[decimal128("3.14",2),decimal128("0",2),decimal128("nan",2)]]),100) as `decimal128_array_type
        )
    )";
    dolphindb::TableSP res = conn.run(script);
    std::string json_0 = R"({"blob_type":"123","bool_array_type":"[1,0,]","bool_type":"1","char_array_type":"[1,0,]","char_type":"1","date_array_type":"[2013.06.13,1970.01.01,]","date_type":"2013.06.13","datehour_array_type":"[2012.06.13T13,1970.01.01T00,]","datehour_type":"2012.06.13T13","datetime_array_type":"[2012.06.13T13:30:10,1970.01.01T00:00:00,]","datetime_type":"2012.06.13T13:30:10","decimal128_array_type":"[3.14,0.00,]","decimal128_type":"3.14","decimal32_array_type":"[3.14,0.00,]","decimal32_type":"3.14","decimal64_array_type":"[3.14,0.00,]","decimal64_type":"3.14","double_array_type":"[,,3.14,]","double_type":"","float_array_type":"[NaN,inf,3.14,]","float_type":"NaN","int128_array_type":"[e1671797c52e15f763380b45e841ec32,]","int128_type":"e1671797c52e15f763380b45e841ec32","int_array_type":"[1,0,]","int_type":"1","ipaddr_array_type":"[127.0.0.1,]","ipaddr_type":"127.0.0.1","long_array_type":"[1,0,]","long_type":"1","minute_array_type":"[13:30m,00:00m,]","minute_type":"13:30m","month_array_type":"[2013.06M,1970.01M,]","month_type":"2013.06M","nanotime_array_type":"[13:30:10.008007006,00:00:00.000000000,]","nanotime_type":"13:30:10.008007006","nanotimestamp_array_type":"[2012.06.13T13:30:10.008007006,1970.01.01T00:00:00.000000000,]","nanotimestamp_type":"2012.06.13T13:30:10.008007006","second_array_type":"[13:30:00,00:00:00,]","second_type":"13:30:00","short_array_type":"[1,0,]","short_type":"1","string_type":"123,456","time_array_type":"[13:30:10.008,00:00:00.000,]","time_type":"13:30:10.008","timestamp_array_type":"[2012.06.13T13:30:10.008,1970.01.01T00:00:00.000,]","timestamp_type":"2012.06.13T13:30:10.008","uuid_array_type":"[5d212a78-cc48-e3b1-4235-b4d91473ee87,]","uuid_type":"5d212a78-cc48-e3b1-4235-b4d91473ee87"})";
    ASSERT_EQ(res->getRowJson(0).dump(),json_0);
    std::string json_1 = R"({"blob_type":"","bool_array_type":"[1,1]","bool_type":"0","char_array_type":"[1,1]","char_type":"0","date_array_type":"[2013.06.13,1970.01.01,]","date_type":"1970.01.01","datehour_array_type":"[2012.06.13T13,1970.01.01T00,]","datehour_type":"1970.01.01T00","datetime_array_type":"[2012.06.13T13:30:10,1970.01.01T00:00:00,]","datetime_type":"1970.01.01T00:00:00","decimal128_array_type":"[3.14,0.00,]","decimal128_type":"0.00","decimal32_array_type":"[3.14,0.00,]","decimal32_type":"0.00","decimal64_array_type":"[3.14,0.00,]","decimal64_type":"0.00","double_array_type":"[,,3.14,]","double_type":"","float_array_type":"[NaN,inf,3.14,]","float_type":"inf","int128_array_type":"[e1671797c52e15f763380b45e841ec32,]","int128_type":"00000000000000000000000000000000","int_array_type":"[1,1]","int_type":"0","ipaddr_array_type":"[127.0.0.1,]","ipaddr_type":"0.0.0.0","long_array_type":"[1,1]","long_type":"0","minute_array_type":"[13:30m,00:00m,]","minute_type":"00:00m","month_array_type":"[2013.06M,1970.01M,]","month_type":"1970.01M","nanotime_array_type":"[13:30:10.008007006,00:00:00.000000000,]","nanotime_type":"00:00:00.000000000","nanotimestamp_array_type":"[2012.06.13T13:30:10.008007006,1970.01.01T00:00:00.000000000,]","nanotimestamp_type":"1970.01.01T00:00:00.000000000","second_array_type":"[13:30:00,00:00:00,]","second_type":"00:00:00","short_array_type":"[1,1]","short_type":"0","string_type":"123","time_array_type":"[13:30:10.008,00:00:00.000,]","time_type":"00:00:00.000","timestamp_array_type":"[2012.06.13T13:30:10.008,1970.01.01T00:00:00.000,]","timestamp_type":"1970.01.01T00:00:00.000","uuid_array_type":"[5d212a78-cc48-e3b1-4235-b4d91473ee87,]","uuid_type":"00000000-0000-0000-0000-000000000000"})";
    ASSERT_EQ(res->getRowJson(1).dump(),json_1);
}
#include <gtest/gtest.h>
#include "config.h"
#include "Set.h"

class DataformSetTest:public testing::Test
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

dolphindb::DBConnection DataformSetTest::conn(false, false);

TEST_F(DataformSetTest,testAnySet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_ANY,5);
    ASSERT_EQ(v1.isNull(),true);
}

TEST_F(DataformSetTest,testDecimal32Set){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DECIMAL32,5);
    ASSERT_EQ(v1.isNull(),true);
}

TEST_F(DataformSetTest,testDecimal64Set){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DECIMAL64,5);
    ASSERT_EQ(v1.isNull(),true);
}

TEST_F(DataformSetTest,testDecimal128Set){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DECIMAL128,5);
    ASSERT_EQ(v1.isNull(),true);
}

TEST_F(DataformSetTest,testBlobSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_BLOB,5);
    v1->append(dolphindb::Util::createBlob("123abc"));
    v1->append(dolphindb::Util::createBlob("中文*……%#￥#！a"));

    std::string script = "a=set(blob([`123abc, '中文*……%#￥#！a']));a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(\"中文*……%#￥#！a\",\"123abc\")");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(\"123abc\",\"中文*……%#￥#！a\")");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createString("中文*……%#￥#！a"));
    ASSERT_EQ(v1->getValue()->getString(),"set(\"123abc\")");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createString("123abc"),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createString("abc")));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_BLOB,1);
    temp1->append(dolphindb::Util::createString("dolphindb"));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_BLOB,5);
    v2->append(dolphindb::Util::createString("123abc"));
    v2->append(dolphindb::Util::createString("中文*……%#￥#！a"));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));
    ASSERT_TRUE(v2->isSuperset(v2->keys()->get(0)));
    ASSERT_FALSE(v1->isSuperset(v2->keys()->get(0)));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createString("123abc"))->getString(),"set(\"123abc\")");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createString("中文*……%#￥#！a"))->getString(),"set(\"中文*……%#￥#！a\")");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createString("444"))->getString(),"set()");
    dolphindb::VectorSP judge_res = dolphindb::Util::createVector(dolphindb::DT_BOOL, v2->size());
    for(auto i=0;i<v2->size();i++){
        v2->contain(v2->interaction(v2), judge_res);
    }
    ASSERT_EQ(judge_res->getString(),"[1,1]");

    std::cout<<v2->getString()<<std::endl;
    v1->append(v2);

    dolphindb::ConstantSP result = dolphindb::Util::createNullConstant(dolphindb::DT_BOOL);
    for(auto i=0;i<v2->size();i++){
        v1->contain(v2->keys()->get(i), result);
        ASSERT_EQ(result->getBool(), true);
    }

    v1->remove(v2);
    ASSERT_EQ(v1->size(), 0);
    ASSERT_EQ(v2->getSubVector(1,1)->get(0)->getString(),v2->keys()->get(1)->getString());

    v2->clear();
    ASSERT_EQ(v2->size(), 0);
}

TEST_F(DataformSetTest,testStringSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_STRING,5);
    v1->append(dolphindb::Util::createString("123abc"));
    v1->append(dolphindb::Util::createString("中文*……%#￥#！a"));

    std::string script = "a=set([`123abc, '中文*……%#￥#！a']);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(\"中文*……%#￥#！a\",\"123abc\")");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(\"123abc\",\"中文*……%#￥#！a\")");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createString("中文*……%#￥#！a"));
    ASSERT_EQ(v1->getValue()->getString(),"set(\"123abc\")");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createString("123abc"),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createString("abc")));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_STRING,1);
    temp1->append(dolphindb::Util::createString("dolphindb"));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_STRING,5);
    v2->append(dolphindb::Util::createString("123abc"));
    v2->append(dolphindb::Util::createString("中文*……%#￥#！a"));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));
    ASSERT_TRUE(v2->isSuperset(v2->keys()->get(0)));
    ASSERT_FALSE(v1->isSuperset(v2->keys()->get(0)));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createString("123abc"))->getString(),"set(\"123abc\")");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createString("中文*……%#￥#！a"))->getString(),"set(\"中文*……%#￥#！a\")");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createString("444"))->getString(),"set()");
    dolphindb::VectorSP judge_res = dolphindb::Util::createVector(dolphindb::DT_BOOL, v2->size());
    for(auto i=0;i<v2->size();i++){
        v2->contain(v2->interaction(v2), judge_res);
    }
    ASSERT_EQ(judge_res->getString(),"[1,1]");

    std::cout<<v2->getString()<<std::endl;
    v1->append(v2);

    dolphindb::ConstantSP result = dolphindb::Util::createNullConstant(dolphindb::DT_BOOL);
    for(auto i=0;i<v2->size();i++){
        v1->contain(v2->keys()->get(i), result);
        ASSERT_EQ(result->getBool(), true);
    }

    v1->remove(v2);
    ASSERT_EQ(v1->size(), 0);
    ASSERT_EQ(v2->getSubVector(1,1)->get(0)->getString(),v2->keys()->get(1)->getString());

    v2->clear();
    ASSERT_EQ(v2->size(), 0);
}

TEST_F(DataformSetTest,testStringNullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_STRING,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_STRING));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_STRING));
    std::string script = "a=set([string(NULL), string(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}


TEST_F(DataformSetTest,testSymbolSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_SYMBOL,5);
    v1->append(dolphindb::Util::createString("123abc"));
    v1->append(dolphindb::Util::createString("中文*……%#￥#！a"));

    std::string script = "a=set(symbol([`123abc, '中文*……%#￥#！a']));a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(\"中文*……%#￥#！a\",\"123abc\")");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(\"123abc\",\"中文*……%#￥#！a\")");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(dolphindb::DT_STRING, res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createString("中文*……%#￥#！a"));
    ASSERT_EQ(v1->getValue()->getString(),"set(\"123abc\")");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createString("123abc"),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createString("abc")));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_SYMBOL,1);
    temp1->append(dolphindb::Util::createString("dolphindb"));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_SYMBOL,5);
    v2->append(dolphindb::Util::createString("123abc"));
    v2->append(dolphindb::Util::createString("中文*……%#￥#！a"));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));
    ASSERT_TRUE(v2->isSuperset(v2->keys()->get(0)));
    ASSERT_FALSE(v1->isSuperset(v2->keys()->get(0)));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createString("123abc"))->getString(),"set(\"123abc\")");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createString("中文*……%#￥#！a"))->getString(),"set(\"中文*……%#￥#！a\")");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createString("444"))->getString(),"set()");
    dolphindb::VectorSP judge_res = dolphindb::Util::createVector(dolphindb::DT_BOOL, v2->size());
    for(auto i=0;i<v2->size();i++){
        v2->contain(v2->interaction(v2), judge_res);
    }
    ASSERT_EQ(judge_res->getString(),"[1,1]");

    std::cout<<v2->getString()<<std::endl;
    v1->append(v2);

    dolphindb::ConstantSP result = dolphindb::Util::createNullConstant(dolphindb::DT_BOOL);
    for(auto i=0;i<v2->size();i++){
        v1->contain(v2->keys()->get(i), result);
        ASSERT_EQ(result->getBool(), true);
    }

    v1->remove(v2);
    ASSERT_EQ(v1->size(), 0);
    ASSERT_EQ(v2->getSubVector(1,1)->get(0)->getString(),v2->keys()->get(1)->getString());

    v2->clear();
    ASSERT_EQ(v2->size(), 0);
}

TEST_F(DataformSetTest,testBoolSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_BOOL,5);
    ASSERT_EQ(v1.isNull(),true);
}

TEST_F(DataformSetTest,testCharSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_CHAR,5);
    v1->append(dolphindb::Util::createChar(1));
    v1->append(dolphindb::Util::createChar(0));
    std::string script = "a=set([char(1), char(0)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(0,1)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(1,0)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createChar(0));
    ASSERT_EQ(v1->getValue()->getString(),"set(1)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createChar(1),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createChar(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_CHAR,1);
    temp1->append(dolphindb::Util::createChar(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_CHAR,5);
    v2->append(dolphindb::Util::createChar(1));
    v2->append(dolphindb::Util::createChar(0));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));
    ASSERT_TRUE(v2->isSuperset(v2->keys()->get(0)));
    ASSERT_FALSE(v1->isSuperset(v2->keys()->get(0)));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createChar(1))->getString(),"set(1)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createChar(0))->getString(),"set(0)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createChar(2))->getString(),"set()");
    dolphindb::VectorSP judge_res = dolphindb::Util::createVector(dolphindb::DT_BOOL, v2->size());
    for(auto i=0;i<v2->size();i++){
        v2->contain(v2->interaction(v2), judge_res);
    }
    ASSERT_EQ(judge_res->getString(),"[1,1]");

    std::cout<<v2->getString()<<std::endl;
    v1->append(v2);

    dolphindb::ConstantSP result = dolphindb::Util::createNullConstant(dolphindb::DT_BOOL);
    for(auto i=0;i<v2->size();i++){
        v1->contain(v2->keys()->get(i), result);
        ASSERT_EQ(result->getBool(), true);
    }

    v1->remove(v2);
    ASSERT_EQ(v1->size(), 0);
    ASSERT_EQ(v2->getSubVector(1,1)->get(0)->getString(),v2->keys()->get(1)->getString());

    for(auto i=0; i<200; i++)
        v2->append(dolphindb::Util::createChar(i));
    std::cout<<v2->getString()<<std::endl;

}

TEST_F(DataformSetTest,testCharNullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_CHAR,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_CHAR));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_CHAR));
    std::string script = "a=set([char(NULL), char(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testIntSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_INT,5);
    v1->append(dolphindb::Util::createInt(1));
    v1->append(dolphindb::Util::createInt(0));
    std::string script = "a=set([int(1), int(0)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(0,1)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(1,0)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createInt(0));
    ASSERT_EQ(v1->getValue()->getString(),"set(1)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createInt(1),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createInt(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_INT,1);
    temp1->append(dolphindb::Util::createInt(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_INT,5);
    v2->append(dolphindb::Util::createInt(1));
    v2->append(dolphindb::Util::createInt(0));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));
    ASSERT_TRUE(v2->isSuperset(v2->keys()->get(0)));
    ASSERT_FALSE(v1->isSuperset(v2->keys()->get(0)));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createInt(1))->getString(),"set(1)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createInt(0))->getString(),"set(0)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createInt(2))->getString(),"set()");
    dolphindb::VectorSP judge_res = dolphindb::Util::createVector(dolphindb::DT_BOOL, v2->size());
    for(auto i=0;i<v2->size();i++){
        v2->contain(v2->interaction(v2), judge_res);
    }
    ASSERT_EQ(judge_res->getString(),"[1,1]");

    std::cout<<v2->getString()<<std::endl;
    v1->append(v2);

    dolphindb::ConstantSP result = dolphindb::Util::createNullConstant(dolphindb::DT_BOOL);
    for(auto i=0;i<v2->size();i++){
        v1->contain(v2->keys()->get(i), result);
        ASSERT_EQ(result->getBool(), true);
    }

    v1->remove(v2);
    ASSERT_EQ(v1->size(), 0);
    ASSERT_EQ(v2->getSubVector(1,1)->get(0)->getString(),v2->keys()->get(1)->getString());
}

TEST_F(DataformSetTest,testIntNullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_INT,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_INT));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_INT));
    std::string script = "a=set([int(NULL), int(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testLongSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_LONG,5);
    v1->append(dolphindb::Util::createLong(1));
    v1->append(dolphindb::Util::createLong(0));
    std::string script = "a=set([long(1), long(0)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(0,1)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(1,0)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createLong(0));
    ASSERT_EQ(v1->getValue()->getString(),"set(1)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createLong(1),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createLong(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_LONG,1);
    temp1->append(dolphindb::Util::createLong(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_LONG,5);
    v2->append(dolphindb::Util::createLong(1));
    v2->append(dolphindb::Util::createLong(0));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));
    ASSERT_TRUE(v2->isSuperset(v2->keys()->get(0)));
    ASSERT_FALSE(v1->isSuperset(v2->keys()->get(0)));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createLong(1))->getString(),"set(1)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createLong(0))->getString(),"set(0)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createLong(2))->getString(),"set()");
    dolphindb::VectorSP judge_res = dolphindb::Util::createVector(dolphindb::DT_BOOL, v2->size());
    for(auto i=0;i<v2->size();i++){
        v2->contain(v2->interaction(v2), judge_res);
    }
    ASSERT_EQ(judge_res->getString(),"[1,1]");

    std::cout<<v2->getString()<<std::endl;
    v1->append(v2);

    dolphindb::ConstantSP result = dolphindb::Util::createNullConstant(dolphindb::DT_BOOL);
    for(auto i=0;i<v2->size();i++){
        v1->contain(v2->keys()->get(i), result);
        ASSERT_EQ(result->getBool(), true);
    }

    v1->remove(v2);
    ASSERT_EQ(v1->size(), 0);
    ASSERT_EQ(v2->getSubVector(1,1)->get(0)->getString(),v2->keys()->get(1)->getString());
}

TEST_F(DataformSetTest,testLongNullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_LONG,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_LONG));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_LONG));
    std::string script = "a=set([long(NULL), long(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testShortNullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_SHORT,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_SHORT));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_SHORT));
    std::string script = "a=set([short(NULL), short(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testShortSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_SHORT,5);
    v1->append(dolphindb::Util::createShort(1));
    v1->append(dolphindb::Util::createShort(0));
    std::string script = "a=set([short(1), short(0)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(0,1)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(1,0)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createShort(0));
    ASSERT_EQ(v1->getValue()->getString(),"set(1)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createShort(1),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createShort(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_SHORT,1);
    temp1->append(dolphindb::Util::createShort(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_SHORT,5);
    v2->append(dolphindb::Util::createShort(1));
    v2->append(dolphindb::Util::createShort(0));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));
    ASSERT_TRUE(v2->isSuperset(v2->keys()->get(0)));
    ASSERT_FALSE(v1->isSuperset(v2->keys()->get(0)));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createShort(1))->getString(),"set(1)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createShort(0))->getString(),"set(0)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createShort(2))->getString(),"set()");
    dolphindb::VectorSP judge_res = dolphindb::Util::createVector(dolphindb::DT_BOOL, v2->size());
    for(auto i=0;i<v2->size();i++){
        v2->contain(v2->interaction(v2), judge_res);
    }
    ASSERT_EQ(judge_res->getString(),"[1,1]");

    std::cout<<v2->getString()<<std::endl;
    v1->append(v2);

    dolphindb::ConstantSP result = dolphindb::Util::createNullConstant(dolphindb::DT_BOOL);
    for(auto i=0;i<v2->size();i++){
        v1->contain(v2->keys()->get(i), result);
        ASSERT_EQ(result->getBool(), true);
    }

    v1->remove(v2);
    ASSERT_EQ(v1->size(), 0);
    ASSERT_EQ(v2->getSubVector(1,1)->get(0)->getString(),v2->keys()->get(1)->getString());
}


TEST_F(DataformSetTest,testFloatSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_FLOAT,5);
    v1->append(dolphindb::Util::createFloat(1));
    v1->append(dolphindb::Util::createFloat(0));
    std::string script = "a=set([float(1), float(0)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(0,1)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(1,0)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createFloat(0));
    ASSERT_EQ(v1->getValue()->getString(),"set(1)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createFloat(1),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createFloat(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_FLOAT,1);
    temp1->append(dolphindb::Util::createFloat(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_FLOAT,5);
    v2->append(dolphindb::Util::createFloat(1));
    v2->append(dolphindb::Util::createFloat(0));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));
    ASSERT_TRUE(v2->isSuperset(v2->keys()->get(0)));
    ASSERT_FALSE(v1->isSuperset(v2->keys()->get(0)));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createFloat(1))->getString(),"set(1)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createFloat(0))->getString(),"set(0)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createFloat(2))->getString(),"set()");
    dolphindb::VectorSP judge_res = dolphindb::Util::createVector(dolphindb::DT_BOOL, v2->size());
    for(auto i=0;i<v2->size();i++){
        v2->contain(v2->interaction(v2), judge_res);
    }
    ASSERT_EQ(judge_res->getString(),"[1,1]");

    std::cout<<v2->getString()<<std::endl;
    v1->append(v2);

    dolphindb::ConstantSP result = dolphindb::Util::createNullConstant(dolphindb::DT_BOOL);
    for(auto i=0;i<v2->size();i++){
        v1->contain(v2->keys()->get(i), result);
        ASSERT_EQ(result->getBool(), true);
    }

    v1->remove(v2);
    ASSERT_EQ(v1->size(), 0);
    ASSERT_EQ(v2->getSubVector(1,1)->get(0)->getString(),v2->keys()->get(1)->getString());
}


TEST_F(DataformSetTest,testFloatNullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_FLOAT,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT));
    std::string script = "a=set([float(NULL), float(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testDoubleSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DOUBLE,5);
    v1->append(dolphindb::Util::createDouble(1));
    v1->append(dolphindb::Util::createDouble(0));
    std::string script = "a=set([double(1), double(0)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(0,1)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(1,0)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createDouble(0));
    ASSERT_EQ(v1->getValue()->getString(),"set(1)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createDouble(1),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createDouble(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_DOUBLE,1);
    temp1->append(dolphindb::Util::createDouble(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);

    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_DOUBLE,5);
    v2->append(dolphindb::Util::createDouble(1));
    v2->append(dolphindb::Util::createDouble(0));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));
    ASSERT_TRUE(v2->isSuperset(v2->keys()->get(0)));
    ASSERT_FALSE(v1->isSuperset(v2->keys()->get(0)));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createDouble(1))->getString(),"set(1)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createDouble(0))->getString(),"set(0)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createDouble(2))->getString(),"set()");
    dolphindb::VectorSP judge_res = dolphindb::Util::createVector(dolphindb::DT_BOOL, v2->size());
    for(auto i=0;i<v2->size();i++){
        v2->contain(v2->interaction(v2), judge_res);
    }
    ASSERT_EQ(judge_res->getString(),"[1,1]");

    std::cout<<v2->getString()<<std::endl;
    v1->append(v2);

    dolphindb::ConstantSP result = dolphindb::Util::createNullConstant(dolphindb::DT_BOOL);
    for(auto i=0;i<v2->size();i++){
        v1->contain(v2->keys()->get(i), result);
        ASSERT_EQ(result->getBool(), true);
    }

    v1->remove(v2);
    ASSERT_EQ(v1->size(), 0);
    ASSERT_EQ(v2->getSubVector(1,1)->get(0)->getString(),v2->keys()->get(1)->getString());
}


TEST_F(DataformSetTest,testDoubleNullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DOUBLE,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_DOUBLE));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_DOUBLE));
    std::string script = "a=set([double(NULL), double(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testDatehourSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATEHOUR,5);
    v1->append(dolphindb::Util::createDateHour(1));
    v1->append(dolphindb::Util::createDateHour(100000));
    std::string script = "a=set([datehour(1), datehour(100000)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(1981.05.29T16,1970.01.01T01)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(1970.01.01T01,1981.05.29T16)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createDateHour(1));
    ASSERT_EQ(v1->getValue()->getString(),"set(1981.05.29T16)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createDateHour(100000),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createDateHour(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_STRING,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_DATEHOUR,1);
    temp1->append(dolphindb::Util::createDateHour(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_DATEHOUR,5);
    v2->append(dolphindb::Util::createDateHour(1));
    v2->append(dolphindb::Util::createDateHour(100000));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createDateHour(100000))->getString(),"set(1981.05.29T16)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createDateHour(1))->getString(),"set(1970.01.01T01)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createDateHour(2))->getString(),"set()");
}

TEST_F(DataformSetTest,testDatehourNullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATEHOUR,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_DATEHOUR));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_DATEHOUR));
    std::string script = "a=set([datehour(NULL), datehour(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testDateSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATE,5);
    v1->append(dolphindb::Util::createDate(1));
    v1->append(dolphindb::Util::createDate(48750));
    std::string script = "a=set([date(1), date(48750)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(2103.06.23,1970.01.02)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(1970.01.02,2103.06.23)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createDate(1));
    ASSERT_EQ(v1->getValue()->getString(),"set(2103.06.23)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createDate(48750),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createDate(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_STRING,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_DATE,1);
    temp1->append(dolphindb::Util::createDate(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_DATE,5);
    v2->append(dolphindb::Util::createDate(1));
    v2->append(dolphindb::Util::createDate(48750));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createDate(48750))->getString(),"set(2103.06.23)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createDate(1))->getString(),"set(1970.01.02)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createDate(2))->getString(),"set()");
}

TEST_F(DataformSetTest,testDatenullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATE,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_DATE));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_DATE));
    std::string script = "a=set([date(NULL), date(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testMinuteSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_MINUTE,5);
    v1->append(dolphindb::Util::createMinute(1));
    v1->append(dolphindb::Util::createMinute(1000));
    std::string script = "a=set([minute(1), minute(1000)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(16:40m,00:01m)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(00:01m,16:40m)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createMinute(1));
    ASSERT_EQ(v1->getValue()->getString(),"set(16:40m)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createMinute(1000),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createMinute(2)));

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createString("2")));
     v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_MINUTE,5);
    v2->append(dolphindb::Util::createMinute(1));
    v2->append(dolphindb::Util::createMinute(1000));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createMinute(1000))->getString(),"set(16:40m)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createMinute(1))->getString(),"set(00:01m)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createMinute(2))->getString(),"set()");
}

TEST_F(DataformSetTest,testMinutenullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_MINUTE,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_MINUTE));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_MINUTE));
    std::string script = "a=set([minute(NULL), minute(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());

    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testDatetimeSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATETIME,5);
    v1->append(dolphindb::Util::createDateTime(1));
    v1->append(dolphindb::Util::createDateTime(48750));
    std::string script = "a=set([datetime(1), datetime(48750)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(1970.01.01T13:32:30,1970.01.01T00:00:01)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(1970.01.01T00:00:01,1970.01.01T13:32:30)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createDateTime(1));
    ASSERT_EQ(v1->getValue()->getString(),"set(1970.01.01T13:32:30)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createDateTime(48750),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createDateTime(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_STRING,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_DATETIME,1);
    temp1->append(dolphindb::Util::createDateTime(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_DATETIME,5);
    v2->append(dolphindb::Util::createDateTime(1));
    v2->append(dolphindb::Util::createDateTime(48750));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createDateTime(48750))->getString(),"set(1970.01.01T13:32:30)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createDateTime(1))->getString(),"set(1970.01.01T00:00:01)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createDateTime(2))->getString(),"set()");
}

TEST_F(DataformSetTest,testDatetimenullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATETIME,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_DATETIME));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_DATETIME));
    std::string script = "a=set([datetime(NULL), datetime(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testTimeStampSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_TIMESTAMP,5);
    v1->append(dolphindb::Util::createTimestamp(1));
    v1->append(dolphindb::Util::createTimestamp(1000000000000));
    std::string script = "a=set([timestamp(1), timestamp(1000000000000)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(2001.09.09T01:46:40.000,1970.01.01T00:00:00.001)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(1970.01.01T00:00:00.001,2001.09.09T01:46:40.000)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createTimestamp(1));
    ASSERT_EQ(v1->getValue()->getString(),"set(2001.09.09T01:46:40.000)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createTimestamp(1000000000000),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createTimestamp(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_STRING,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_TIMESTAMP,1);
    temp1->append(dolphindb::Util::createTimestamp(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_TIMESTAMP,5);
    v2->append(dolphindb::Util::createTimestamp(1));
    v2->append(dolphindb::Util::createTimestamp(1000000000000));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createTimestamp(1))->getString(),"set(1970.01.01T00:00:00.001)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createTimestamp(1000000000000))->getString(),"set(2001.09.09T01:46:40.000)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createTimestamp(2))->getString(),"set()");
}

TEST_F(DataformSetTest,testTimeStampnullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_TIMESTAMP,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_TIMESTAMP));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_TIMESTAMP));
    std::string script = "a=set([timestamp(NULL), timestamp(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testNanotimeSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,5);
    v1->append(dolphindb::Util::createNanoTime(1));
    v1->append(dolphindb::Util::createNanoTime(1000000000000));
    std::string script = "a=set([nanotime(1), nanotime(1000000000000)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(00:16:40.000000000,00:00:00.000000001)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(00:00:00.000000001,00:16:40.000000000)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createNanoTime(1));
    ASSERT_EQ(v1->getValue()->getString(),"set(00:16:40.000000000)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createNanoTime(1000000000000),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createNanoTime(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_STRING,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    temp1->append(dolphindb::Util::createNanoTime(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,5);
    v2->append(dolphindb::Util::createNanoTime(1));
    v2->append(dolphindb::Util::createNanoTime(1000000000000));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createNanoTime(1000000000000))->getString(),"set(00:16:40.000000000)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createNanoTime(1))->getString(),"set(00:00:00.000000001)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createNanoTime(2))->getString(),"set()");
}

TEST_F(DataformSetTest,testNanotimenullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIME));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIME));
    std::string script = "a=set([nanotime(NULL), nanotime(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testNanotimestampSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_NANOTIMESTAMP,5);
    v1->append(dolphindb::Util::createNanoTimestamp(1));
    v1->append(dolphindb::Util::createNanoTimestamp(1000000000000));
    std::string script = "a=set([nanotimestamp(1), nanotimestamp(1000000000000)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(1970.01.01T00:16:40.000000000,1970.01.01T00:00:00.000000001)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(1970.01.01T00:00:00.000000001,1970.01.01T00:16:40.000000000)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createNanoTimestamp(1));
    ASSERT_EQ(v1->getValue()->getString(),"set(1970.01.01T00:16:40.000000000)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createNanoTimestamp(1000000000000),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createNanoTimestamp(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_STRING,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_NANOTIMESTAMP,1);
    temp1->append(dolphindb::Util::createNanoTimestamp(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_NANOTIMESTAMP,5);
    v2->append(dolphindb::Util::createNanoTimestamp(1));
    v2->append(dolphindb::Util::createNanoTimestamp(1000000000000));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createNanoTimestamp(1000000000000))->getString(),"set(1970.01.01T00:16:40.000000000)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createNanoTimestamp(1))->getString(),"set(1970.01.01T00:00:00.000000001)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createNanoTimestamp(2))->getString(),"set()");
}

TEST_F(DataformSetTest,testNanotimestampnullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_NANOTIMESTAMP,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIMESTAMP));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIMESTAMP));
    std::string script = "a=set([nanotimestamp(NULL), nanotimestamp(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testMonthSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_MONTH,5);
    v1->append(dolphindb::Util::createMonth(1));
    v1->append(dolphindb::Util::createMonth(1000));
    std::string script = "a=set([month(1), month(1000)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(0083.05M,0000.02M)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(0000.02M,0083.05M)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createMonth(1));
    ASSERT_EQ(v1->getValue()->getString(),"set(0083.05M)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createMonth(1000),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createMonth(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_STRING,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_MONTH,1);
    temp1->append(dolphindb::Util::createMonth(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_MONTH,5);
    v2->append(dolphindb::Util::createMonth(1));
    v2->append(dolphindb::Util::createMonth(1000));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createMonth(1))->getString(),"set(0000.02M)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createMonth(1000))->getString(),"set(0083.05M)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createMonth(2))->getString(),"set()");
}

TEST_F(DataformSetTest,testMonthnullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_MONTH,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_MONTH));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_MONTH));
    std::string script = "a=set([month(NULL), month(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testTimeSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_TIME,5);
    v1->append(dolphindb::Util::createTime(1));
    v1->append(dolphindb::Util::createTime((long)10000000));
    std::string script = "a=set([time(1), time(10000000)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(02:46:40.000,00:00:00.001)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(00:00:00.001,02:46:40.000)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createTime(1));
    ASSERT_EQ(v1->getValue()->getString(),"set(02:46:40.000)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(dolphindb::Util::createTime(10000000),res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(dolphindb::Util::createTime(2)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_STRING,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_TIME,1);
    temp1->append(dolphindb::Util::createTime(9));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_TIME,5);
    v2->append(dolphindb::Util::createTime(1));
    v2->append(dolphindb::Util::createTime(10000000));
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));

    ASSERT_EQ(v2->interaction(dolphindb::Util::createTime(10000000))->getString(),"set(02:46:40.000)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createTime(1))->getString(),"set(00:00:00.001)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createTime(2))->getString(),"set()");
}

TEST_F(DataformSetTest,testTimenullSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_TIME,5);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_TIME));
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_TIME));
    std::string script = "a=set([time(NULL), time(NULL)]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->getString(),res_v->keys()->getString());
}

TEST_F(DataformSetTest,testInt128Set){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_INT128,5);
    dolphindb::ConstantSP int128val=dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec32");
    v1->append(int128val);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_INT128));
    std::string script = "a=set([int128(`e1671797c52e15f763380b45e841ec32), NULL]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(00000000000000000000000000000000,e1671797c52e15f763380b45e841ec32)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(e1671797c52e15f763380b45e841ec32,00000000000000000000000000000000)");
    #endif // _MSC_VER

    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createNullConstant(dolphindb::DT_INT128));
    ASSERT_EQ(v1->getValue()->getString(),"set(e1671797c52e15f763380b45e841ec32)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(int128val,res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(int128val));
    ASSERT_FALSE(v1->inverse(dolphindb::Util::createNullConstant(dolphindb::DT_INT128)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_INT128,1);
    temp1->append(dolphindb::Util::createNullConstant(dolphindb::DT_INT128));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_INT128,5);
    v2->append(dolphindb::Util::createNullConstant(dolphindb::DT_INT128));
    v2->append(int128val);
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));
    ASSERT_TRUE(v2->isSuperset(v2->keys()->get(0)));
    ASSERT_FALSE(v1->isSuperset(v2->keys()->get(0)));

    ASSERT_EQ(v2->interaction(int128val)->getString(),"set(e1671797c52e15f763380b45e841ec32)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createNullConstant(dolphindb::DT_INT128))->getString(),"set(00000000000000000000000000000000)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec33"))->getString(),"set()");
    dolphindb::VectorSP judge_res = dolphindb::Util::createVector(dolphindb::DT_BOOL, v2->size());
    for(auto i=0;i<v2->size();i++){
        v2->contain(v2->interaction(v2), judge_res);
    }
    ASSERT_EQ(judge_res->getString(),"[1,1]");

    std::cout<<v2->getString()<<std::endl;
    v1->append(v2);

    dolphindb::ConstantSP result = dolphindb::Util::createNullConstant(dolphindb::DT_BOOL);
    for(auto i=0;i<v2->size();i++){
        v1->contain(v2->keys()->get(i), result);
        ASSERT_EQ(result->getBool(), true);
    }

    v1->remove(v2);
    ASSERT_EQ(v1->size(), 0);
    ASSERT_EQ(v2->getSubVector(1,1)->get(0)->getString(),v2->keys()->get(1)->getString());
}

TEST_F(DataformSetTest,testIpaddrSet){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_IP,5);
    dolphindb::ConstantSP ipaddrval=dolphindb::Util::parseConstant(dolphindb::DT_IP,"192.168.1.13");
    v1->append(ipaddrval);
    v1->append(dolphindb::Util::createNullConstant(dolphindb::DT_IP));
    std::string script = "a=set([ipaddr(`192.168.1.13), NULL]);a";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});

    ASSERT_EQ(v1->getInstance()->getString(),"set()");
    #ifndef _MSC_VER
        ASSERT_EQ(v1->getValue()->getString(),"set(0.0.0.0,192.168.1.13)");
    #else
        ASSERT_EQ(v1->getValue()->getString(),"set(192.168.1.13,0.0.0.0)");
    #endif // _MSC_VER
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isSet(),true);
    ASSERT_EQ(v1->sizeable(),true);
    ASSERT_EQ(v1->keys()->get(0)->getString(),res_v->keys()->get(1)->getString());
    ASSERT_EQ(v1->keys()->get(1)->getString(),res_v->keys()->get(0)->getString());

    v1->remove(dolphindb::Util::createNullConstant(dolphindb::DT_IP));
    ASSERT_EQ(v1->getValue()->getString(),"set(192.168.1.13)");
    dolphindb::ConstantSP res=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    res->setNull();
    v1->contain(ipaddrval,res);
    ASSERT_TRUE(res->getBool());
    res1->setNull();
    v1->contain(v1,res1);
    ASSERT_TRUE(res1->getBool());
    res1->setNull();
    v1->contain(v1->keys(),res1);
    ASSERT_TRUE(res1->getBool());

    ASSERT_FALSE(v1->inverse(ipaddrval));
    ASSERT_FALSE(v1->inverse(dolphindb::Util::createNullConstant(dolphindb::DT_IP)));
    dolphindb::SetSP temp = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1);
    dolphindb::SetSP temp1 = dolphindb::Util::createSet(dolphindb::DT_IP,1);
    temp1->append(dolphindb::Util::createNullConstant(dolphindb::DT_IP));
    ASSERT_FALSE(v1->inverse(temp));
    ASSERT_TRUE(v1->inverse(temp1));
    v1->inverse(v1);
    ASSERT_EQ(v1->getString(),"set()");

    dolphindb::SetSP v2 = dolphindb::Util::createSet(dolphindb::DT_IP,5);
    v2->append(dolphindb::Util::createNullConstant(dolphindb::DT_IP));
    v2->append(ipaddrval);
    ASSERT_TRUE(v2->isSuperset(v1));
    ASSERT_FALSE(v1->isSuperset(v2));

    ASSERT_EQ(v2->interaction(ipaddrval)->getString(),"set(192.168.1.13)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::createNullConstant(dolphindb::DT_IP))->getString(),"set(0.0.0.0)");
    ASSERT_EQ(v2->interaction(dolphindb::Util::parseConstant(dolphindb::DT_IP,"192.168.1.14"))->getString(),"set()");
}

TEST_F(DataformSetTest,testCharSetEqual128){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_CHAR,128);
    for(int i=0;i<128;i++)
        v1->append(dolphindb::Util::createChar(i));
    std::string script = "z=set(char(0..127));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(v1->getScript(),res_v->getScript());
}

TEST_F(DataformSetTest,testShortSetEqual256){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_SHORT,256);
    for(int i=0;i<256;i++)
        v1->append(dolphindb::Util::createShort(i));
    std::string script = "z=set(short(0..255));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testIntSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_INT,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createInt(i));
    std::string script = "z=set(int(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testLongSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_LONG,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createLong(i));
    std::string script = "z=set(long(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testDateSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATE,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createDate(i));
    std::string script = "z=set(date(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testMonthSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_MONTH,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createMonth(i));
    std::string script = "z=set(month(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}


TEST_F(DataformSetTest,testTimeSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_TIME,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createTime(i));
    std::string script = "z=set(time(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testMinuteSetEqual1440){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_MINUTE,1440);
    for(int i=0;i<1440;i++)
        v1->append(dolphindb::Util::createMinute(i));
    std::string script = "z=set(minute(0..1439));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testSecondSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_SECOND,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createSecond(i));
    std::string script = "z=set(second(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testDatetimeSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATETIME,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createDateTime(i));
    std::string script = "z=set(datetime(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testTimestampSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_TIMESTAMP,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createTimestamp(i));
    std::string script = "z=set(timestamp(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testNanotimeSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createNanoTime(i));
    std::string script = "z=set(nanotime(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testNanotimestampSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_NANOTIMESTAMP,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createNanoTimestamp(i));
    std::string script = "z=set(nanotimestamp(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testFloatSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_FLOAT,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createFloat(i));
    std::string script = "z=set(float(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testDoubleSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DOUBLE,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createDouble(i));
    std::string script = "z=set(double(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testStringSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_STRING,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createString(std::to_string(i)));
    std::string script = "z=set(string(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testDatehourSetMoreThan65535){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATEHOUR,70000);
    for(int i=0;i<70000;i++)
        v1->append(dolphindb::Util::createDateHour(i));
    std::string script = "z=set(datehour(0..69999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testIntSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_INT,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createInt(i));
    std::string script = "z=set(int(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testLongSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_LONG,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createLong(i));
    std::string script = "z=set(long(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testDateSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATE,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createDate(i));
    std::string script = "z=set(date(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testMonthSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_MONTH,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createMonth(i));
    std::string script = "z=set(month(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}


TEST_F(DataformSetTest,testTimeSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_TIME,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createTime(i));
    std::string script = "z=set(time(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testSecondSetEqual86400){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_SECOND,86400);
    for(int i=0;i<86400;i++)
        v1->append(dolphindb::Util::createSecond(i));
    std::string script = "z=set(second(0..86399));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testDatetimeSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATETIME,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createDateTime(i));
    std::string script = "z=set(datetime(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testTimestampSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_TIMESTAMP,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createTimestamp(i));
    std::string script = "z=set(timestamp(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testNanotimeSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_NANOTIME,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createNanoTime(i));
    std::string script = "z=set(nanotime(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testNanotimestampSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_NANOTIMESTAMP,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createNanoTimestamp(i));
    std::string script = "z=set(nanotimestamp(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testFloatSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_FLOAT,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createFloat(i));
    std::string script = "z=set(float(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testDoubleSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DOUBLE,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createDouble(i));
    std::string script = "z=set(double(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testStringSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_STRING,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createString(std::to_string(i)));
    std::string script = "z=set(string(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest,testDatehourSetMoreThan1048576){
    dolphindb::SetSP v1 = dolphindb::Util::createSet(dolphindb::DT_DATEHOUR,1100000);
    for(int i=0;i<1100000;i++)
        v1->append(dolphindb::Util::createDateHour(i));
    std::string script = "z=set(datehour(0..1099999));z";
    dolphindb::SetSP res_v = conn.run(script);
    conn.upload("v1",{v1});
    ASSERT_EQ(conn.run("v1.size()")->getInt(),res_v->size());
    ASSERT_EQ(conn.run("v1")->getScript(),conn.run("z")->getScript());
}

TEST_F(DataformSetTest, test_stringSet_exception){
    std::string ss("123\0 123", 8);
    {
        try{
            dolphindb::SetSP v = dolphindb::Util::createSet(dolphindb::DT_STRING, 0);
            v->append(dolphindb::Util::createString(ss));
        }catch(std::exception& e){
            ASSERT_EQ(std::string(e.what()), "A String cannot contain the character '\\0'");
        }
    }
    {
        dolphindb::SetSP v = dolphindb::Util::createSet(dolphindb::DT_STRING, 0);
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
    }
    {
        dolphindb::SetSP v = dolphindb::Util::createSet(dolphindb::DT_SYMBOL, 0);
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
    }
}
#include <gtest/gtest.h>
#include "config.h"

class DataformDictionaryTest:public testing::Test
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

dolphindb::DBConnection DataformDictionaryTest::conn(false, false);

TEST_F(DataformDictionaryTest,testBlobDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_BLOB, dolphindb::DT_BLOB);
    dict1->set(dolphindb::Util::createBlob("zzz123中文a"), dolphindb::Util::createBlob("*-/%**%#~！#“》（a"));
    std::string script = "a=blob(\"zzz123中文a\");b=blob(\"*-/%**%#~！#“》（a\");c=dict(BLOB,BLOB);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getScript(), res_d->getScript());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"zzz123中文a");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"*-/%**%#~！#“》（a");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_BLOB);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_BLOB);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_BLOB,0,2);
    valVec->append(dolphindb::Util::createBlob("*-/%**%#~！#“》（a"));
    dict1->contain(dolphindb::Util::createBlob("zzz123中文a"),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createBlob("demo1"), dolphindb::Util::createBlob("value1"));
    dict1->set(dolphindb::Util::createBlob("demo2"), dolphindb::Util::createBlob("value2"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_STRING,2,2);
    valVec2->set(0,dolphindb::Util::createBlob("demo2"));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createBlob("zzz123中文a"))->getString(),"*-/%**%#~！#“》（a");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"value2");
    dict1->remove(dolphindb::Util::createBlob("demo1"));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"zzz123中文a->*-/%**%#~！#“》（a\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testStringDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createString("zzz123中文a"), dolphindb::Util::createString("*-/%**%#~！#“》（a"));
    std::string script = "a=\"zzz123中文a\";b=\"*-/%**%#~！#“》（a\";c=dict(STRING,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getScript(), res_d->getScript());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"zzz123中文a");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"*-/%**%#~！#“》（a");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_STRING);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_STRING,0,2);
    valVec->append(dolphindb::Util::createString("*-/%**%#~！#“》（a"));
    dict1->contain(dolphindb::Util::createString("zzz123中文a"),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createString("demo1"), dolphindb::Util::createString("value1"));
    dict1->set(dolphindb::Util::createString("demo2"), dolphindb::Util::createString("value2"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_STRING,2,2);
    valVec2->set(0,dolphindb::Util::createString("demo2"));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createString("zzz123中文a"))->getString(),"*-/%**%#~！#“》（a");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"value2");
    dict1->remove(dolphindb::Util::createString("demo1"));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"zzz123中文a->*-/%**%#~！#“》（a\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
    ASSERT_ANY_THROW(dict1->set(dolphindb::Util::createInt(1), dolphindb::Util::createString("value1")));
    dolphindb::VectorSP k1 = conn.run("blob(`b1`b2)");
    dolphindb::VectorSP v1 = conn.run("blob(`v1`v2)");
    dolphindb::VectorSP v2 = conn.run("blob(`v1`v2`v3)");
    ASSERT_FALSE(dict1->set(k1, v2));
    ASSERT_TRUE(dict1->set(k1, v1));
    ASSERT_TRUE(dict1->set(dolphindb::Util::createBlob("b3"), dolphindb::Util::createString("aaaa")));
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createString("b3"))->getString(),"aaaa");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createBlob("b1"))->getString(),"v1");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createBlob("b2"))->getString(),"v2");
    ASSERT_ANY_THROW(dict1->getMember(dolphindb::Util::createInt(1)));
    ASSERT_ANY_THROW(dict1->remove(dolphindb::Util::createInt(1)));
    ASSERT_TRUE(dict1->remove(dolphindb::Util::createBlob("b1")));
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createString("b1"))->getString(), "");
}


TEST_F(DataformDictionaryTest,testStringAnyDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_ANY);
    ASSERT_ANY_THROW(dict1->set(dolphindb::Util::createInt(1), dolphindb::Util::createDateTime(10000)));
    dict1->set(dolphindb::Util::createString("key_str"), dolphindb::Util::createDateTime(10000));
    std::string script = "a=\"key_str\";b = datetime(10000);c=dict(STRING,ANY);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("typestr(dict1)")->getString(),"STRING->ANY DICTIONARY");
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"key_str");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"1970.01.01T02:46:40");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_STRING);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DATETIME);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_DATETIME);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_STRING,0,2);
    valVec->append(dolphindb::Util::createString("key_str"));

    ASSERT_ANY_THROW(dict1->contain(dolphindb::Util::createInt(1), res1));
    dict1->contain(dolphindb::Util::createString("key_str"), res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_DATE);
    dict1->contain(valVec, res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createString("key2"), dolphindb::Util::createInt(1));
    dict1->set(dolphindb::Util::createString("key3"), dolphindb::Util::createFloat(3.2155));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_STRING,2,2);
    valVec2->set(0,dolphindb::Util::createString("key3"));
    valVec2->setNull(1);

    ASSERT_ANY_THROW(dict1->getMember(dolphindb::Util::createInt(1)));
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createString("key2"))->getInt(), 1);
    ASSERT_NEAR(dict1->getMember(valVec2)->get(0)->getFloat(), 3.2155, 4);
    ASSERT_ANY_THROW(dict1->remove(dolphindb::Util::createInt(1)));
    dict1->remove(dolphindb::Util::createString("key2"));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"key_str->1970.01.01T02:46:40\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());

    dolphindb::VectorSP matrix_val = dolphindb::Util::createMatrix(dolphindb::DT_INT, 2, 1, 2);
    matrix_val->set(0, 0, dolphindb::Util::createInt(999));
    matrix_val->set(0, 1, dolphindb::Util::createInt(888));
    dolphindb::DictionarySP dict2 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_INT);
    dict2->set(dolphindb::Util::createString("sym"), dolphindb::Util::createInt(23456));
    dict1->set(dolphindb::Util::createString("matrix"), matrix_val);
    dict1->set(dolphindb::Util::createString("dict"), dict2);

    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
    dolphindb::VectorSP k1 = conn.run("`a`b`c");
    dolphindb::VectorSP k2 = conn.run("`a`b");
    dolphindb::VectorSP v1 = conn.run("`10`20`30");
    dolphindb::VectorSP v2 = conn.run("[table(take(1, 1000) as c1), 2]");
    ASSERT_ANY_THROW(dict1->set(k1, v2));
    ASSERT_TRUE(dict1->set(k1, v1));
    ASSERT_TRUE(dict1->set(k2, v2));
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createString("a"))->getString(),"c1\n--\n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n1 \n...\n");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createString("b"))->getString(),"2");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createString("c"))->getString(),"30");
}


TEST_F(DataformDictionaryTest,testIntAnyDictionary){
    ASSERT_ANY_THROW(dolphindb::Util::createDictionary(dolphindb::DT_ANY, dolphindb::DT_INT));
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_ANY);
    dict1->set(dolphindb::Util::createInt(123), dolphindb::Util::createBool(true));
    std::string script = "a=int(123);b = bool(1);c=dict(INT,ANY);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"123");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"1");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_INT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_BOOL);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_BOOL,0,2);
    valVec->append(dolphindb::Util::createBool(true));
    dict1->contain(dolphindb::Util::createInt(123),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createInt(4), dolphindb::Util::createString("1"));
    dict1->set(dolphindb::Util::createInt(7), dolphindb::Util::createBlob("2"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_INT,2,2);
    valVec2->set(0,dolphindb::Util::createInt(7));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(4))->getString(),"1");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"2");
    dict1->remove(dolphindb::Util::createInt(4));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"123->1\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}


TEST_F(DataformDictionaryTest,testLongAnyDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_LONG, dolphindb::DT_ANY);
    ASSERT_ANY_THROW(dict1->set(dolphindb::Util::createString("1"), dolphindb::Util::createDateTime(10000)));
    dict1->set(dolphindb::Util::createLong(1300000), dolphindb::Util::createDateTime(10000));
    std::string script = "a=long(1300000);b = datetime(10000);c=dict(LONG,ANY);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("typestr(dict1)")->getString(),"LONG->ANY DICTIONARY");
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getLong(), (long long)1300000);
    ASSERT_EQ(dict1->values()->get(0)->getString(),"1970.01.01T02:46:40");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_LONG);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DATETIME);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_DATETIME);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_LONG,0,2);
    valVec->append(dolphindb::Util::createLong(1300000));

    ASSERT_ANY_THROW(dict1->contain(dolphindb::Util::createInt(1), res1));
    dict1->contain(dolphindb::Util::createLong(1300000), res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_DATE);
    dict1->contain(valVec, res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createLong(1000000000), dolphindb::Util::createInt(1));
    dict1->set(dolphindb::Util::createLong(10000000000000), dolphindb::Util::createFloat(3.2155));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_LONG,2,2);
    valVec2->set(0,dolphindb::Util::createLong(10000000000000));
    valVec2->setNull(1);

    ASSERT_ANY_THROW(dict1->getMember(dolphindb::Util::createInt(1)));
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createLong(1000000000))->getInt(), 1);
    ASSERT_NEAR(dict1->getMember(valVec2)->get(0)->getFloat(), 3.2155, 4);
    ASSERT_ANY_THROW(dict1->remove(dolphindb::Util::createString("1")));
    dict1->remove(dolphindb::Util::createLong(1000000000));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"1300000->1970.01.01T02:46:40\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
    ASSERT_FALSE(dict1->containNotMarshallableObject());

    dolphindb::VectorSP matrix_val = dolphindb::Util::createMatrix(dolphindb::DT_INT, 2, 1, 2);
    matrix_val->set(0, 0, dolphindb::Util::createInt(999));
    matrix_val->set(0, 1, dolphindb::Util::createInt(888));
    dolphindb::DictionarySP dict2 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_INT);
    dict2->set(dolphindb::Util::createString("sym"), dolphindb::Util::createInt(23456));
    dict1->set(dolphindb::Util::createLong(1), matrix_val);
    dict1->set(dolphindb::Util::createLong(2), dict2);
    std::cout<<dict1->getString()<<std::endl;
}


TEST_F(DataformDictionaryTest,testSymbolDictionary){
    ASSERT_ANY_THROW(dolphindb::Util::createDictionary(dolphindb::DT_SYMBOL, dolphindb::DT_STRING));
}

TEST_F(DataformDictionaryTest,testBoolDictionary){
    ASSERT_ANY_THROW(dolphindb::Util::createDictionary(dolphindb::DT_BOOL, dolphindb::DT_STRING));
}

TEST_F(DataformDictionaryTest,testCharDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_CHAR, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createChar(1), dolphindb::Util::createString("0"));
    std::string script = "a=char(1);b = string(0);c=dict(CHAR,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"1");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_CHAR);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_CHAR,0,2);
    valVec->append(dolphindb::Util::createChar(0));
    dict1->contain(dolphindb::Util::createChar(1),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createChar(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createChar(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_CHAR,2,2);
    valVec2->set(0,dolphindb::Util::createChar(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createChar(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createChar(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"1->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());

    dolphindb::VectorSP k1 = conn.run("1c 2c 3c");
    dolphindb::VectorSP k2 = conn.run("1c 2c");
    dolphindb::VectorSP v1 = conn.run("`10`20`30");
    dolphindb::VectorSP v2 = conn.run("`40`50");
    ASSERT_FALSE(dict1->set(k1, v2));
    ASSERT_TRUE(dict1->set(k1, v1));
    ASSERT_TRUE(dict1->set(k2, v2));
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(1))->getString(),"40");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(2))->getString(),"50");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(3))->getString(),"30");
}

TEST_F(DataformDictionaryTest,testCharNullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_CHAR, dolphindb::DT_CHAR);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_CHAR), dolphindb::Util::createNullConstant(dolphindb::DT_CHAR));
    std::string script = "a=char(NULL);b = char(NULL);c=dict(CHAR,CHAR);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_CHAR);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_CHAR);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_CHAR,0,2);
    valVec->append(dolphindb::Util::createChar(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_CHAR),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testIntDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createInt(1), dolphindb::Util::createString("0"));
    std::string script = "a=int(1);b = string(0);c=dict(INT,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"1");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_INT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_INT,0,2);
    valVec->append(dolphindb::Util::createInt(0));
    dict1->contain(dolphindb::Util::createInt(1),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createInt(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createInt(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_CHAR,2,2);
    valVec2->set(0,dolphindb::Util::createInt(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createInt(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"1->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
    dolphindb::VectorSP k1 = conn.run("1 2 3");
    dolphindb::VectorSP k2 = conn.run("1 2");
    dolphindb::VectorSP v1 = conn.run("`10`20`30");
    dolphindb::VectorSP v2 = conn.run("`40`50");
    ASSERT_FALSE(dict1->set(k1, v2));
    ASSERT_TRUE(dict1->set(k1, v1));
    ASSERT_TRUE(dict1->set(k2, v2));
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(1))->getString(),"40");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(2))->getString(),"50");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(3))->getString(),"30");

}

TEST_F(DataformDictionaryTest,testIntNullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_INT);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_INT), dolphindb::Util::createNullConstant(dolphindb::DT_INT));
    std::string script = "a=int(NULL);b = int(NULL);c=dict(INT,INT);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_INT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_INT);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_INT,0,2);
    valVec->append(dolphindb::Util::createInt(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_INT),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}


TEST_F(DataformDictionaryTest,testLongDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_LONG, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createLong(100000000), dolphindb::Util::createString("0"));
    std::string script = "a=long(100000000);b = string(0);c=dict(LONG,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"100000000");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_LONG);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_LONG,0,2);
    valVec->append(dolphindb::Util::createLong(0));
    dict1->contain(dolphindb::Util::createLong(100000000),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createLong(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createLong(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_LONG,2,2);
    valVec2->set(0,dolphindb::Util::createLong(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createLong(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createLong(2));
    dict1->remove(valVec2);
    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"100000000->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
    dolphindb::VectorSP k1 = conn.run("1l 2l 3l");
    dolphindb::VectorSP k2 = conn.run("1l 2l");
    dolphindb::VectorSP v1 = conn.run("`10`20`30");
    dolphindb::VectorSP v2 = conn.run("`40`50");
    ASSERT_FALSE(dict1->set(k1, v2));
    ASSERT_TRUE(dict1->set(k1, v1));
    ASSERT_TRUE(dict1->set(k2, v2));
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(1))->getString(),"40");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(2))->getString(),"50");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(3))->getString(),"30");
}

TEST_F(DataformDictionaryTest,testLongNullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_LONG, dolphindb::DT_LONG);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_LONG), dolphindb::Util::createNullConstant(dolphindb::DT_LONG));
    std::string script = "a=long(NULL);b = long(NULL);c=dict(LONG,LONG);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_LONG);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_LONG);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_LONG,0,2);
    valVec->append(dolphindb::Util::createLong(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_LONG),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testShortDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_SHORT, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createShort(100), dolphindb::Util::createString("0"));
    std::string script = "a=short(100);b = string(0);c=dict(SHORT,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"100");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_SHORT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_SHORT,0,2);
    valVec->append(dolphindb::Util::createShort(0));
    dict1->contain(dolphindb::Util::createShort(100),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createShort(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createShort(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_SHORT,2,2);
    valVec2->set(0,dolphindb::Util::createShort(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createShort(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createShort(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"100->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());

    dolphindb::VectorSP k1 = conn.run("1h 2h 3h");
    dolphindb::VectorSP k2 = conn.run("1h 2h");
    dolphindb::VectorSP v1 = conn.run("`10`20`30");
    dolphindb::VectorSP v2 = conn.run("`40`50");
    ASSERT_FALSE(dict1->set(k1, v2));
    ASSERT_TRUE(dict1->set(k1, v1));
    ASSERT_TRUE(dict1->set(k2, v2));
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(1))->getString(),"40");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(2))->getString(),"50");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(3))->getString(),"30");
}

TEST_F(DataformDictionaryTest,testShortNullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_SHORT, dolphindb::DT_SHORT);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_SHORT), dolphindb::Util::createNullConstant(dolphindb::DT_SHORT));
    std::string script = "a=short(NULL);b = short(NULL);c=dict(SHORT,SHORT);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_SHORT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_SHORT);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_SHORT,0,2);
    valVec->append(dolphindb::Util::createShort(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_SHORT),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testFloatDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_FLOAT, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createFloat(100.2333), dolphindb::Util::createString("0"));

    std::string script = "a=float(100.2333);b = string(0);c=dict(FLOAT,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"100.233299");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_FLOAT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_FLOAT,0,2);
    valVec->append(dolphindb::Util::createFloat(0));
    dict1->contain(dolphindb::Util::createFloat(100.2333),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createFloat(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createFloat(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_FLOAT,2,2);
    valVec2->set(0,dolphindb::Util::createFloat(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createFloat(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createFloat(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"100.233299->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
    dolphindb::VectorSP k1 = conn.run("1f 2f 3f");
    dolphindb::VectorSP k2 = conn.run("1f 2f");
    dolphindb::VectorSP v1 = conn.run("`10`20`30");
    dolphindb::VectorSP v2 = conn.run("`40`50");
    ASSERT_FALSE(dict1->set(k1, v2));
    ASSERT_TRUE(dict1->set(k1, v1));
    ASSERT_TRUE(dict1->set(k2, v2));
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(1))->getString(),"40");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(2))->getString(),"50");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(3))->getString(),"30");
}


TEST_F(DataformDictionaryTest,testFloatNullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_FLOAT, dolphindb::DT_FLOAT);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT), dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT));
    std::string script = "a=float(NULL);b = float(NULL);c=dict(FLOAT,FLOAT);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_FLOAT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_FLOAT);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_FLOAT,0,2);
    valVec->append(dolphindb::Util::createFloat(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testDoubleDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DOUBLE, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createDouble(100.2333), dolphindb::Util::createString("0"));
    std::string script = "a=double(100.2333);b = string(0);c=dict(DOUBLE,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"100.2333");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_DOUBLE);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_DOUBLE,0,2);
    valVec->append(dolphindb::Util::createDouble(0));
    dict1->contain(dolphindb::Util::createDouble(100.2333),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createDouble(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createDouble(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_DOUBLE,2,2);
    valVec2->set(0,dolphindb::Util::createDouble(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createDouble(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createDouble(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"100.2333->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
    dolphindb::VectorSP k1 = conn.run("1.0 2.0 3.0");
    dolphindb::VectorSP k2 = conn.run("1.0 2.0");
    dolphindb::VectorSP v1 = conn.run("`10`20`30");
    dolphindb::VectorSP v2 = conn.run("`40`50");
    ASSERT_FALSE(dict1->set(k1, v2));
    ASSERT_TRUE(dict1->set(k1, v1));
    ASSERT_TRUE(dict1->set(k2, v2));
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(1))->getString(),"40");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(2))->getString(),"50");
    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(3))->getString(),"30");

}


TEST_F(DataformDictionaryTest,testDoubleNullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DOUBLE, dolphindb::DT_DOUBLE);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_DOUBLE), dolphindb::Util::createNullConstant(dolphindb::DT_DOUBLE));
    std::string script = "a=double(NULL);b = double(NULL);c=dict(DOUBLE,DOUBLE);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_DOUBLE);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DOUBLE);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_DOUBLE,0,2);
    valVec->append(dolphindb::Util::createDouble(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_DOUBLE),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testDatehourDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATEHOUR, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createDateHour(100), dolphindb::Util::createString("0"));
    std::string script = "a=datehour(100);b = string(0);c=dict(DATEHOUR,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"1970.01.05T04");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_DATEHOUR);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_DATEHOUR,0,2);
    valVec->append(dolphindb::Util::createDateHour(0));
    dict1->contain(dolphindb::Util::createDateHour(100),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createDateHour(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createDateHour(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_DATEHOUR,2,2);
    valVec2->set(0,dolphindb::Util::createDateHour(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createDateHour(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createDateHour(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"1970.01.05T04->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testDatehourNullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATEHOUR, dolphindb::DT_DATEHOUR);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_DATEHOUR), dolphindb::Util::createNullConstant(dolphindb::DT_DATEHOUR));
    std::string script = "a=datehour(NULL);b = datehour(NULL);c=dict(DATEHOUR,DATEHOUR);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_DATEHOUR);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DATEHOUR);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_DATEHOUR,0,2);
    valVec->append(dolphindb::Util::createDateHour(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_DATEHOUR),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testDateDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATE, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createDate(100), dolphindb::Util::createString("0"));
    std::string script = "a=date(100);b = string(0);c=dict(DATE,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"1970.04.11");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_DATE);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_DATE,0,2);
    valVec->append(dolphindb::Util::createDate(0));
    dict1->contain(dolphindb::Util::createDate(100),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createDate(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createDate(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_DATE,2,2);
    valVec2->set(0,dolphindb::Util::createDate(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createDate(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createDate(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"1970.04.11->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
    ASSERT_FALSE(dict1->set(dolphindb::Util::createString("a"), dolphindb::Util::createString("100")));
    ASSERT_ANY_THROW(dict1->getMember(dolphindb::Util::createInt(100)));
}

TEST_F(DataformDictionaryTest,testDatenullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATE, dolphindb::DT_DATE);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_DATE), dolphindb::Util::createNullConstant(dolphindb::DT_DATE));
    std::string script = "a=date(NULL);b = date(NULL);c=dict(DATE,DATE);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_DATE);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DATE);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_DATE,0,2);
    valVec->append(dolphindb::Util::createDate(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_DATE),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testMinuteDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_MINUTE, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createMinute(1000), dolphindb::Util::createString("0"));
    std::string script = "a=minute(1000);b = string(0);c=dict(MINUTE,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"16:40m");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_MINUTE);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_MINUTE,0,2);
    valVec->append(dolphindb::Util::createMinute(0));
    dict1->contain(dolphindb::Util::createMinute(1000),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createMinute(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createMinute(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_MINUTE,2,2);
    valVec2->set(0,dolphindb::Util::createMinute(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createMinute(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createMinute(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"16:40m->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testMinutenullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_MINUTE, dolphindb::DT_MINUTE);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_MINUTE), dolphindb::Util::createNullConstant(dolphindb::DT_MINUTE));
    std::string script = "a=minute(NULL);b = minute(NULL);c=dict(MINUTE,MINUTE);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_MINUTE);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_MINUTE);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_MINUTE,0,2);
    valVec->append(dolphindb::Util::createMinute(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_MINUTE),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testDatetimeDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATETIME, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createDateTime(1000000000), dolphindb::Util::createString("0"));
    std::string script = "a=datetime(1000000000);b = string(0);c=dict(DATETIME,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"2001.09.09T01:46:40");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_DATETIME);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_DATETIME,0,2);
    valVec->append(dolphindb::Util::createDateTime(0));
    dict1->contain(dolphindb::Util::createDateTime(1000000000),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createDateTime(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createDateTime(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_DATETIME,2,2);
    valVec2->set(0,dolphindb::Util::createDateTime(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createDateTime(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createDateTime(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"2001.09.09T01:46:40->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testDatetimenullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATETIME, dolphindb::DT_DATETIME);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_DATETIME), dolphindb::Util::createNullConstant(dolphindb::DT_DATETIME));
    std::string script = "a=datetime(NULL);b = datetime(NULL);c=dict(DATETIME,DATETIME);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_DATETIME);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DATETIME);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_DATETIME,0,2);
    valVec->append(dolphindb::Util::createDateTime(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_DATETIME),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testTimeStampDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_TIMESTAMP, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createTimestamp((long long)10000000000000), dolphindb::Util::createString("0"));
    std::string script = "a=timestamp(10000000000000);b = string(0);c=dict(TIMESTAMP,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"2286.11.20T17:46:40.000");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_TIMESTAMP);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP,0,2);
    valVec->append(dolphindb::Util::createTimestamp(0));
    dict1->contain(dolphindb::Util::createTimestamp((long long)10000000000000),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createTimestamp(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createTimestamp(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP,2,2);
    valVec2->set(0,dolphindb::Util::createTimestamp(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createTimestamp(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createTimestamp(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"2286.11.20T17:46:40.000->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testTimeStampnullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_TIMESTAMP, dolphindb::DT_TIMESTAMP);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_TIMESTAMP), dolphindb::Util::createNullConstant(dolphindb::DT_TIMESTAMP));
    std::string script = "a=timestamp(NULL);b = timestamp(NULL);c=dict(TIMESTAMP,TIMESTAMP);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_TIMESTAMP);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_TIMESTAMP);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP,0,2);
    valVec->append(dolphindb::Util::createTimestamp(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_TIMESTAMP),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testnanotimeDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_NANOTIME, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createNanoTime((long long)10000000000000), dolphindb::Util::createString("0"));
    std::string script = "a=nanotime(10000000000000);b = string(0);c=dict(NANOTIME,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"02:46:40.000000000");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_NANOTIME);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_NANOTIME,0,2);
    valVec->append(dolphindb::Util::createNanoTime(0));
    dict1->contain(dolphindb::Util::createNanoTime((long long)10000000000000),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createNanoTime(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createNanoTime(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_NANOTIME,2,2);
    valVec2->set(0,dolphindb::Util::createNanoTime(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createNanoTime(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createNanoTime(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"02:46:40.000000000->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
    ASSERT_FALSE(dict1->set(dolphindb::Util::createString("a"), dolphindb::Util::createString("10000000000")));
}

TEST_F(DataformDictionaryTest,testnanotimenullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_NANOTIME, dolphindb::DT_NANOTIME);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIME), dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIME));
    std::string script = "a=nanotime(NULL);b = nanotime(NULL);c=dict(NANOTIME,NANOTIME);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_NANOTIME);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_NANOTIME);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_NANOTIME,0,2);
    valVec->append(dolphindb::Util::createNanoTime(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIME),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testnanotimestampDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createNanoTimestamp((long long)10000000000000), dolphindb::Util::createString("0"));
    std::string script = "a=nanotimestamp(10000000000000);b = string(0);c=dict(NANOTIMESTAMP,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"1970.01.01T02:46:40.000000000");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_NANOTIMESTAMP);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP,0,2);
    valVec->append(dolphindb::Util::createNanoTimestamp(0));
    dict1->contain(dolphindb::Util::createNanoTimestamp((long long)10000000000000),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createNanoTimestamp(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createNanoTimestamp(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP,2,2);
    valVec2->set(0,dolphindb::Util::createNanoTimestamp(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createNanoTimestamp(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createNanoTimestamp(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"1970.01.01T02:46:40.000000000->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
    ASSERT_ANY_THROW(dict1->getMember(dolphindb::Util::createLong(10)));
}

TEST_F(DataformDictionaryTest,testnanotimestampnullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_NANOTIMESTAMP);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIMESTAMP), dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIMESTAMP));
    std::string script = "a=nanotimestamp(NULL);b = nanotimestamp(NULL);c=dict(NANOTIMESTAMP,NANOTIMESTAMP);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_NANOTIMESTAMP);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_NANOTIMESTAMP);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP,0,2);
    valVec->append(dolphindb::Util::createNanoTimestamp(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIMESTAMP),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testmonthDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_MONTH, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createMonth(10000), dolphindb::Util::createString("0"));
    std::string script = "a=month(10000);b = string(0);c=dict(MONTH,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"0833.05M");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_MONTH);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_MONTH,0,2);
    valVec->append(dolphindb::Util::createMonth(0));
    dict1->contain(dolphindb::Util::createMonth(10000),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createMonth(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createMonth(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_MONTH,2,2);
    valVec2->set(0,dolphindb::Util::createMonth(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createMonth(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createMonth(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"0833.05M->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testmonthnullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_MONTH, dolphindb::DT_MONTH);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_MONTH), dolphindb::Util::createNullConstant(dolphindb::DT_MONTH));
    std::string script = "a=month(NULL);b = month(NULL);c=dict(MONTH,MONTH);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_MONTH);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_MONTH);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_MONTH,0,2);
    valVec->append(dolphindb::Util::createMonth(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_MONTH),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testtimeDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_TIME, dolphindb::DT_STRING);
    dict1->set(dolphindb::Util::createTime(909000), dolphindb::Util::createString("0"));
    std::string script = "a=time(909000);b = string(0);c=dict(TIME,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"00:15:09.000");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_TIME);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_TIME,0,2);
    valVec->append(dolphindb::Util::createTime(0));
    dict1->contain(dolphindb::Util::createTime(909000),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dict1->set(dolphindb::Util::createTime(2), dolphindb::Util::createString("2"));
    dict1->set(dolphindb::Util::createTime(3), dolphindb::Util::createString("3"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_TIME,2,2);
    valVec2->set(0,dolphindb::Util::createTime(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createTime(2))->getString(),"2");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"3");
    dict1->remove(dolphindb::Util::createTime(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"00:15:09.000->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testtimenullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_TIME, dolphindb::DT_TIME);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_TIME), dolphindb::Util::createNullConstant(dolphindb::DT_TIME));
    std::string script = "a=time(NULL);b = time(NULL);c=dict(TIME,TIME);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_TIME);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_TIME);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_TIME,0,2);
    valVec->append(dolphindb::Util::createTime(0));
    dict1->contain(dolphindb::Util::createNullConstant(dolphindb::DT_TIME),res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testInt128Dictionary){
    dolphindb::DictionarySP dict0 = dolphindb::Util::createDictionary(dolphindb::DT_INT128, dolphindb::DT_INT128);
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT128, dolphindb::DT_STRING);
    dolphindb::ConstantSP int128val=dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec32");
    dict0->set(int128val, int128val);
    dict1->set(int128val, dolphindb::Util::createString("0"));

    std::string script = "a=int128(\"e1671797c52e15f763380b45e841ec32\");b = string(0);c=dict(INT128,STRING);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.run("c0 = dict(INT128,INT128);c0[a]=a");
    conn.upload("dict1",{dict1});
    conn.upload("dict0",{dict0});

    std::string judgestr= "res=array(BOOL)\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    res.append!(eqObj(c0.keys()[0],dict0.keys()[0]))\n\
                    res.append!(eqObj(c0.values()[0],dict0.values()[0]))\n\
                    eqObj(res, [true,true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"e1671797c52e15f763380b45e841ec32");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"0");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_INT128);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_STRING);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_INT128,0,2);
    valVec->append(dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec34"));
    dict1->contain(int128val, res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dolphindb::ConstantSP int128val_2=dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec33");
    dolphindb::ConstantSP int128val_3=dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec34");
    dict1->set(int128val_2, dolphindb::Util::createString("value1"));
    dict1->set(int128val_3, dolphindb::Util::createString("value2"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_INT128,2,2);
    valVec2->set(0,int128val_3);
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(int128val_2)->getString(),"value1");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"value2");
    dict1->remove(int128val_2);
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"e1671797-c52e-15f7-6338-0b45e841ec32->0\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
    dolphindb::VectorSP k1 = conn.run("x=rand(int128(),3);x");
    dolphindb::VectorSP k2 = conn.run("x[:2]");
    dolphindb::VectorSP v1 = conn.run("`10`20`30");
    dolphindb::VectorSP v2 = conn.run("`40`50");
    ASSERT_FALSE(dict1->set(k1, v2));
    ASSERT_TRUE(dict1->set(k1, v1));
    ASSERT_TRUE(dict1->set(k2, v2));
    ASSERT_EQ(dict1->getMember(k1->get(0))->getString(),"40");
    ASSERT_EQ(dict1->getMember(k1->get(1))->getString(),"50");
    ASSERT_EQ(dict1->getMember(k1->get(2))->getString(),"30");
    ASSERT_ANY_THROW(dict1->getMember(dolphindb::Util::createInt(4)));
}


TEST_F(DataformDictionaryTest,testInt128AnyDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT128, dolphindb::DT_ANY);
    dolphindb::ConstantSP int128val=dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec32");
    dict1->set(int128val, dolphindb::Util::createInt(1));
    std::string script = "a=int128(\"e1671797c52e15f763380b45e841ec32\");b = int(1);c=dict(INT128,ANY);c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    // std::cout<<dict1->getString();
    // std::cout<<conn.run("dict1")->getString();
    // std::cout<<conn.run("c")->getString();
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"e1671797c52e15f763380b45e841ec32");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"1");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_INT128);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_INT);

    dolphindb::ConstantSP res1=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dolphindb::VectorSP valVec=dolphindb::Util::createVector(dolphindb::DT_INT128,0,2);
    valVec->append(dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec34"));
    dict1->contain(int128val, res1);
    ASSERT_TRUE(res1->getBool());

    dolphindb::ConstantSP res2=dolphindb::Util::createConstant(dolphindb::DT_BOOL);
    dict1->contain(valVec,res2);
    ASSERT_FALSE(res2->getBool());

    dolphindb::ConstantSP int128val_2=dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec33");
    dolphindb::ConstantSP int128val_3=dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec34");
    dict1->set(int128val_2, dolphindb::Util::createString("value1"));
    dict1->set(int128val_3, dolphindb::Util::createString("value2"));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_INT128,2,2);
    valVec2->set(0,int128val_3);
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(int128val_2)->getString(),"value1");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"value2");
    dict1->remove(int128val_2);
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"e1671797c52e15f763380b45e841ec32->1\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}


TEST_F(DataformDictionaryTest,testDecimal32Dictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_DECIMAL32);
    dict1->set(dolphindb::Util::createInt(1), dolphindb::Util::createDecimal32(6, 1.3));
    std::string script = "a=int(1);b = decimal32(1.3, 6);c=dict(INT,DECIMAL32(6));c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=array(BOOL)\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getInt(), 1);
    ASSERT_EQ(dict1->values()->get(0)->getString(),"1.300000");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_INT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DECIMAL32);

    dict1->set(dolphindb::Util::createInt(2), dolphindb::Util::createDecimal32(1, 1.34));
    dict1->set(dolphindb::Util::createInt(3), dolphindb::Util::createDecimal32(0, -2));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_INT,2,2);
    valVec2->set(0,dolphindb::Util::createInt(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(2))->getString(),"1.3");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"-2");
    dict1->remove(dolphindb::Util::createInt(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"1->1.300000\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testDecimal32NullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_DECIMAL32);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_INT), dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL32, 6));
    std::string script = "a=int(NULL);b = decimal32(NULL, 6);c=dict(INT,DECIMAL32(6));c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_INT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DECIMAL32);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}


TEST_F(DataformDictionaryTest,testDecimal64Dictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_DECIMAL64);
    dict1->set(dolphindb::Util::createInt(1), dolphindb::Util::createDecimal64(16, 1.3456));
    std::string script = "a=int(1);b = decimal64(1.3456, 16);c=dict(INT,DECIMAL64(16));c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=array(BOOL)\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getInt(), 1);
    ASSERT_EQ(dict1->values()->get(0)->getString(),"1.3456000000000000");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_INT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DECIMAL64);

    dict1->set(dolphindb::Util::createInt(2), dolphindb::Util::createDecimal64(1, 1.3456));
    dict1->set(dolphindb::Util::createInt(3), dolphindb::Util::createDecimal64(0, -2));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_INT,2,2);
    valVec2->set(0,dolphindb::Util::createInt(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(2))->getString(),"1.3");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"-2");
    dict1->remove(dolphindb::Util::createInt(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"1->1.3456000000000000\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testDecimal64NullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_DECIMAL64);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_INT), dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL64, 16));
    std::string script = "a=int(NULL);b = decimal64(NULL, 16);c=dict(INT,DECIMAL64(16));c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_INT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DECIMAL64);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}


TEST_F(DataformDictionaryTest,testDecimal128Dictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_DECIMAL128);
    dolphindb::ConstantSP dsm128val = dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL128, 26);
    dsm128val->setString("1.12345678901");
    dict1->set(dolphindb::Util::createInt(1), dsm128val);
    std::string script = "a=int(1);b = decimal128('1.12345678901', 26);c=dict(INT,DECIMAL128(26));c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=array(BOOL)\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getInt(), 1);
    ASSERT_EQ(dict1->values()->get(0)->getString(),"1.12345678901000000000000000");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_INT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DECIMAL128);

    dict1->set(dolphindb::Util::createInt(2), dolphindb::Util::createDecimal128(1, 1.3456));
    dict1->set(dolphindb::Util::createInt(3), dolphindb::Util::createDecimal128(0, -2));
    std::cout<<dict1->getAllocatedMemory()<<std::endl;
    dolphindb::VectorSP valVec2=dolphindb::Util::createVector(dolphindb::DT_INT,2,2);
    valVec2->set(0,dolphindb::Util::createInt(3));
    valVec2->setNull(1);

    ASSERT_EQ(dict1->getMember(dolphindb::Util::createInt(2))->getString(),"1.3");
    ASSERT_EQ(dict1->getMember(valVec2)->get(0)->getString(),"-2");
    dict1->remove(dolphindb::Util::createInt(2));
    dict1->remove(valVec2);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"1->1.12345678901000000000000000\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}

TEST_F(DataformDictionaryTest,testDecimal128NullDictionary){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_DECIMAL128);
    dict1->set(dolphindb::Util::createNullConstant(dolphindb::DT_INT), dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL128, 26));
    std::string script = "a=int(NULL);b = decimal128(NULL, 26);c=dict(INT,DECIMAL128(26));c[a]=b;c";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::string judgestr= "res=[true]\n\
                    res.append!(eqObj(c.keys()[0],dict1.keys()[0]))\n\
                    res.append!(eqObj(c.values()[0],dict1.values()[0]))\n\
                    eqObj(res, [true,true,true])";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
    ASSERT_EQ(dict1->getString(), res_d->getString());
    ASSERT_EQ(dict1->getType(), res_d->getType());
    ASSERT_EQ(conn.run("dict1")->isDictionary(),true);

    ASSERT_EQ(dict1->keys()->get(0)->getString(),"");
    ASSERT_EQ(dict1->values()->get(0)->getString(),"");
    ASSERT_EQ(dict1->keys()->get(0)->getType(),dolphindb::DT_INT);
    ASSERT_EQ(dict1->values()->get(0)->getType(),dolphindb::DT_DECIMAL128);

    ASSERT_EQ(dict1->count(),1);
    ASSERT_EQ(dict1->getValue()->getString(),"->\n");
    dict1->clear();
    ASSERT_EQ(dict1->getString(),dict1->getInstance()->getString());
}


TEST_F(DataformDictionaryTest,testDictionarywithValueAllTypes){
    std::vector<dolphindb::ConstantSP> items = {
        dolphindb::Util::createChar('\1'),dolphindb::Util::createBool(true),
        dolphindb::Util::createShort(1),dolphindb::Util::createInt(1),
        dolphindb::Util::createLong(1),dolphindb::Util::createDate(1),dolphindb::Util::createMonth(1),dolphindb::Util::createTime(1),
        dolphindb::Util::createMinute(1),dolphindb::Util::createDateTime(1),dolphindb::Util::createSecond(1),dolphindb::Util::createTimestamp(1),
        dolphindb::Util::createNanoTime(1),dolphindb::Util::createNanoTimestamp(1),dolphindb::Util::createFloat(1),
        dolphindb::Util::createDouble(1),dolphindb::Util::createString("1"),dolphindb::Util::parseConstant(dolphindb::DT_UUID,"5d212a78-cc48-e3b1-4235-b4d91473ee87"),
        dolphindb::Util::parseConstant(dolphindb::DT_IP,"1:1:1:1:1:1:1:1"),dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec32"),dolphindb::Util::createBlob("1"),
        dolphindb::Util::createDateHour(1),dolphindb::Util::createDecimal32(6,1.0),dolphindb::Util::createDecimal64(16,1.0),
        dolphindb::Util::createDecimal128(26,1.0)};

    dolphindb::VectorSP values = dolphindb::Util::createVector(dolphindb::DT_ANY,0,items.size());
    for(int i=0; i<items.size(); i++) values->append(items[i]);

    conn.run("items = [1c,true,1h,1,1l,date(1),month(1),time(1),minute(1),datetime(1),second(1),timestamp(1),nanotime(1),nanotimestamp(1),1f,1.0,`1,\
                uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'),ipaddr('1:1:1:1:1:1:1:1'),int128('e1671797c52e15f763380b45e841ec32'),\
                blob(`1),datehour(1),decimal32(1,6),decimal64(1,16),decimal128(1,26)]");

    for (auto i=0; i< items.size();i++){
        dolphindb::ConstantSP key = items[i];
        if (key->getType() == dolphindb::DT_BOOL || key->getType() == dolphindb::DT_DECIMAL32 || key->getType() == dolphindb::DT_DECIMAL64 || key->getType() == dolphindb::DT_DECIMAL128) continue;

        for(auto j=0; j< items.size();j++){
            dolphindb::ConstantSP val = items[j];
            // std::cout << "keyType:" << dolphindb::Util::getDataTypeString(key->getType()) << ", valType:" << dolphindb::Util::getDataTypeString(val->getType()) << std::endl;
            dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(key->getType(), val->getType());
            dict1->set(key, val);
            conn.upload("dict1",dict1);
            conn.upload("key", key);

            conn.run(
                "ex = dict([key], [items["+std::to_string(j)+"]]);"
                "assert eqObj(ex[key], dict1[key])");
            dolphindb::ConstantSP ex = conn.run("ex");
            ASSERT_EQ(dict1->get(key)->getType(), ex->get(key)->getType());
            ASSERT_EQ(dict1->get(key)->getString(), ex->get(key)->getString());
        }
    }

}

TEST_F(DataformDictionaryTest,testStringDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++){
        dict1->set(dolphindb::Util::createString(std::to_string(i)), dolphindb::Util::createString("val_"+std::to_string(i)));
    }
    std::string script = "a=array(STRING,0);b=array(STRING,0);for (i in 0..69999){a.append!(string(i));b.append!(\"val_\"+string(i));};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testAnyDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_ANY);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createInt(i), dolphindb::Util::createBool(true));
    std::string script = "a=array(INT,0);b=array(BOOL,0);for (i in 0..69999){a.append!(int(i));b.append!(bool(1))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testCharDictionaryEqule128){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_CHAR, dolphindb::DT_STRING);
    for(int i=0;i<128;i++)
        dict1->set(dolphindb::Util::createChar(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(CHAR,0);b=array(STRING,0);for (i in 0..127){a.append!(char(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::cout<<conn.run("dict1.keys()")->getString()<<std::endl;
    std::cout<<conn.run("z.keys()")->getString()<<std::endl;
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,129))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testIntDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createInt(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(INT,0);b=array(STRING,0);for (i in 0..69999){a.append!(int(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testLongDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_LONG, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createLong(i*1000), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(LONG,0);b=array(STRING,0);for (i in 0..69999){a.append!(long(i*1000));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testShortDictionaryEqual256){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_SHORT, dolphindb::DT_STRING);
    for(int i=0;i<256;i++)
        dict1->set(dolphindb::Util::createShort(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(SHORT,0);b=array(STRING,0);for (i in 0..255){a.append!(short(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::cout<<conn.run("dict1.keys()")->getString()<<std::endl;
    std::cout<<conn.run("z.keys()")->getString()<<std::endl;
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,257))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testFloatDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_FLOAT, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createFloat(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(FLOAT,0);b=array(STRING,0);for (i in 0..69999){a.append!(float(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testDoubleDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DOUBLE, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createDouble(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(DOUBLE,0);b=array(STRING,0);for (i in 0..69999){a.append!(double(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}


TEST_F(DataformDictionaryTest,testDatehourDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATEHOUR, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createDateHour(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(DATEHOUR,0);b=array(STRING,0);for (i in 0..69999){a.append!(datehour(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testDateDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATE, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createDate(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(DATE,0);b=array(STRING,0);for (i in 0..69999){a.append!(date(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testMinuteDictionaryMoreThan1024){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_MINUTE, dolphindb::DT_STRING);
    for(int i=0;i<1440;i++)
        dict1->set(dolphindb::Util::createMinute(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(MINUTE,0);b=array(STRING,0);for (i in 0..1439){a.append!(minute(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    std::cout<<conn.run("dict1.keys()")->getString()<<std::endl;
    std::cout<<conn.run("z.keys()")->getString()<<std::endl;
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1441))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testDatetimeDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATETIME, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createDateTime(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(DATETIME,0);b=array(STRING,0);for (i in 0..69999){a.append!(datetime(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testTimeStampDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_TIMESTAMP, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createTimestamp(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(TIMESTAMP,0);b=array(STRING,0);for (i in 0..69999){a.append!(timestamp(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testnanotimeDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_NANOTIME, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createNanoTime(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(NANOTIME,0);b=array(STRING,0);for (i in 0..69999){a.append!(nanotime(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testnanotimestampDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createNanoTimestamp(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(NANOTIMESTAMP,0);b=array(STRING,0);for (i in 0..69999){a.append!(nanotimestamp(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testmonthDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_MONTH, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createMonth(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(MONTH,0);b=array(STRING,0);for (i in 0..69999){a.append!(month(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testtimeDictionaryMoreThan65535){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_TIME, dolphindb::DT_STRING);
    for(int i=0;i<70000;i++)
        dict1->set(dolphindb::Util::createTime(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(TIME,0);b=array(STRING,0);for (i in 0..69999){a.append!(time(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,70001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testStringDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_STRING, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++){
        dict1->set(dolphindb::Util::createString(std::to_string(i)), dolphindb::Util::createString("val_"+std::to_string(i)));
    }
    std::string script = "a=array(STRING,0);b=array(STRING,0);for (i in 0..1099999){a.append!(string(i));b.append!(\"val_\"+string(i));};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testAnyDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_ANY);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createInt(i), dolphindb::Util::createBool(true));
    std::string script = "a=array(INT,0);b=array(BOOL,0);for (i in 0..1099999){a.append!(int(i));b.append!(bool(1))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testIntDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createInt(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(INT,0);b=array(STRING,0);for (i in 0..1099999){a.append!(int(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testLongDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_LONG, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createLong(i*1000), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(LONG,0);b=array(STRING,0);for (i in 0..1099999){a.append!(long(i*1000));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testFloatDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_FLOAT, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createFloat(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(FLOAT,0);b=array(STRING,0);for (i in 0..1099999){a.append!(float(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testDoubleDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DOUBLE, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createDouble(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(DOUBLE,0);b=array(STRING,0);for (i in 0..1099999){a.append!(double(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testDatehourDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATEHOUR, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createDateHour(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(DATEHOUR,0);b=array(STRING,0);for (i in 0..1099999){a.append!(datehour(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testDateDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATE, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createDate(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(DATE,0);b=array(STRING,0);for (i in 0..1099999){a.append!(date(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testDatetimeDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_DATETIME, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createDateTime(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(DATETIME,0);b=array(STRING,0);for (i in 0..1099999){a.append!(datetime(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testTimeStampDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_TIMESTAMP, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createTimestamp(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(TIMESTAMP,0);b=array(STRING,0);for (i in 0..1099999){a.append!(timestamp(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testnanotimeDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_NANOTIME, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createNanoTime(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(NANOTIME,0);b=array(STRING,0);for (i in 0..1099999){a.append!(nanotime(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testnanotimestampDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createNanoTimestamp(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(NANOTIMESTAMP,0);b=array(STRING,0);for (i in 0..1099999){a.append!(nanotimestamp(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testmonthDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_MONTH, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createMonth(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(MONTH,0);b=array(STRING,0);for (i in 0..1099999){a.append!(month(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

TEST_F(DataformDictionaryTest,testtimeDictionaryMoreThan1048576){
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(dolphindb::DT_TIME, dolphindb::DT_STRING);
    for(int i=0;i<1100000;i++)
        dict1->set(dolphindb::Util::createTime(i), dolphindb::Util::createString("val_"+std::to_string(i)));
    std::string script = "a=array(TIME,0);b=array(STRING,0);for (i in 0..1099999){a.append!(time(i));b.append!(\"val_\"+string(i))};z=dict(a,b);z";
    dolphindb::DictionarySP res_d = conn.run(script);
    conn.upload("dict1",{dict1});
    ASSERT_EQ(conn.run("dict1.size()")->getInt(),res_d->size());
    std::string judgestr= "res=[false];for(key in z.keys()){res.append!(dict1[key].isNull())};eqObj(res,take(false,1100001))";
    ASSERT_EQ(conn.run(judgestr)->getBool(),true);
}

class DataformDictionaryTest_AnyDictionary : public DataformDictionaryTest, public testing::WithParamInterface<std::tuple<dolphindb::DATA_TYPE,std::string,std::string>> 
{
public:
    static std::vector<std::tuple<dolphindb::DATA_TYPE,std::string,std::string>> getFloatingData(){
        return {
            std::make_tuple(dolphindb::DT_FLOAT, "float(1..5)", "200f 300f"),
            std::make_tuple(dolphindb::DT_DOUBLE, "double(1..5)", "200.0 300.0"),
            std::make_tuple(dolphindb::DT_CHAR, "char(1..5)", "100c 101c"),
            std::make_tuple(dolphindb::DT_SHORT, "short(1..5)", "200h 300h"),
            std::make_tuple(dolphindb::DT_INT, "int(1..5)", "200 300"),
            std::make_tuple(dolphindb::DT_LONG, "long(1..5)", "200l 300l"),
            std::make_tuple(dolphindb::DT_DATE, "2021.01.01..2021.01.05", "2022.01.01 2022.01.02"),
            std::make_tuple(dolphindb::DT_MONTH, "2021.01M..2021.05M", "2022.01M 2022.02M"),
            std::make_tuple(dolphindb::DT_TIME, "10:10:10.101..10:10:10.105", "10:10:11.101 10:10:11.102"),
            std::make_tuple(dolphindb::DT_MINUTE, "10:11m..10:15m", "11:11m 11:12m"),
            std::make_tuple(dolphindb::DT_SECOND, "10:10:11..10:10:15", "10:11:12 10:11:13"),
            std::make_tuple(dolphindb::DT_DATETIME, "2021.01.01T10:10:10.101..2021.01.01T10:10:10.105", "2022.01.01T10:10:11.101 2022.01.01T10:10:11.102"),
            std::make_tuple(dolphindb::DT_TIMESTAMP, "2021.01.01T10:10:10.100000001..2021.01.01T10:10:10.100000005", "2022.01.01T10:10:11.101000000 2022.01.01T10:10:11.102000000"),
            std::make_tuple(dolphindb::DT_NANOTIME, "10:10:10.100000001..10:10:10.100000005", "10:10:11.101000000 10:10:11.102000000"),
            std::make_tuple(dolphindb::DT_NANOTIMESTAMP, "2021.01.01T10:10:10.100000001..2021.01.01T10:10:10.100000005", "2022.01.01T10:10:11.101000000 2022.01.01T10:10:11.102000000"),
            std::make_tuple(dolphindb::DT_INT128, "rand(int128(), 5)", "rand(int128(), 2)"),
        };
    };
};

INSTANTIATE_TEST_SUITE_P(, DataformDictionaryTest_AnyDictionary, testing::ValuesIn(DataformDictionaryTest_AnyDictionary::getFloatingData()));
TEST_P(DataformDictionaryTest_AnyDictionary, test_anyDict)
{
    dolphindb::DATA_TYPE keyType = std::get<0>(GetParam());
    dolphindb::DictionarySP dict1 = dolphindb::Util::createDictionary(keyType, dolphindb::DT_ANY);
    ASSERT_EQ(dict1->size(), 0);
    ASSERT_EQ(dict1->getKeyType(), keyType);

    dolphindb::VectorSP Keys1 = conn.run("keys1 = "+std::get<1>(GetParam())+";keys1");
    dolphindb::VectorSP Keys2 = conn.run("keys2 = "+std::get<2>(GetParam())+";keys2");
    dolphindb::VectorSP Values1 = conn.run("vals1 = [decimal64(`1.1333'-0.1236657', 3), int128(`e1671797c52e15f763380b45e841ec32), matrix(1 2, 3 4), table(take(`a`b`c, 100) as c1, take(100.0000, 100) as c2), dict(timestamp(1 2), symbol(`sym1`sym2))];vals1");
    dolphindb::VectorSP Values2 = conn.run("vals2=[1,2];vals2");
    // if (keyType != dolphindb::DT_FLOAT && keyType != dolphindb::DT_DOUBLE)
    //     conn.run("ex_dict1= dict(keys1.append!(keys2), vals1.append!(vals2[0]).append!(vals2[1]))");

    /* set */
    ASSERT_ANY_THROW(dict1->set(dolphindb::Util::createString("a"), dolphindb::Util::createInt(1)));
    ASSERT_FALSE(dict1->set(Keys1, Values2));
    ASSERT_TRUE(dict1->set(Keys1, Values1));
    ASSERT_TRUE(dict1->set(Keys2, Values2));

    /* getString */
    std::cout << dict1->getString() << std::endl;
    conn.upload("dict1",{dict1});
    // if (keyType != dolphindb::DT_FLOAT && keyType != dolphindb::DT_DOUBLE)
    //     ASSERT_TRUE(conn.run("all((dict1 == ex_dict1).values())")->getBool());

    std::cout<< dict1->getAllocatedMemory() << std::endl;

    /* contain */
    dolphindb::ConstantSP res = dolphindb::Util::createBool(false);
    dolphindb::VectorSP resV = dolphindb::Util::createVector(dolphindb::DT_BOOL, Keys1->size());;
    ASSERT_ANY_THROW(dict1->contain(dolphindb::Util::createString("a"), res));
    dolphindb::ConstantSP test_key = Keys1->get(0);
    dict1->contain(test_key, res);
    ASSERT_TRUE(res->getBool());

    dict1->contain(Keys1, resV);
    for (int i = 0; i < resV->size(); i++)
        ASSERT_TRUE(resV->getBool(i));

    /* getMember */
    ASSERT_ANY_THROW(dict1->getMember(dolphindb::Util::createString("a")));
    ASSERT_EQ(dict1->getMember(test_key)->getString(), "[1.133,-0.124]");
    dolphindb::ConstantSP errKey = dolphindb::Util::createNullConstant(keyType);
    ASSERT_TRUE(dict1->getMember(errKey)->isNull());
    dolphindb::VectorSP res1 = dict1->getMember(Keys1);
    conn.upload("res1",{res1});
    // if (keyType != dolphindb::DT_FLOAT && keyType != dolphindb::DT_DOUBLE)
    //     ASSERT_TRUE(conn.run("all((res1 == ex_dict1[keys1]).values())")->getBool());

    /* remove */
    ASSERT_ANY_THROW(dict1->remove(dolphindb::Util::createString("a")));
    dolphindb::ConstantSP test_key2 = Keys2->get(0);
    ASSERT_TRUE(dict1->remove(test_key2));
    conn.upload("dict1",{dict1});
    // if (keyType != dolphindb::DT_FLOAT && keyType != dolphindb::DT_DOUBLE)
    //     ASSERT_TRUE(conn.run("all((dict1 == ex_dict1[keys1]).values())")->getBool());

    ASSERT_TRUE(dict1->remove(Keys1));
    ASSERT_TRUE(dict1->remove(Keys2));
    ASSERT_EQ(dict1->size(), 0);
    ASSERT_EQ(dict1->getString(), "");

}
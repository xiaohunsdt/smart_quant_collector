#include <gtest/gtest.h>
#include "config.h"
#include "ConstantImp.h"

class ArrayVectorTest : public testing::Test
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

dolphindb::DBConnection ArrayVectorTest::conn(false, false);

#ifndef _MSC_VER
TEST_F(ArrayVectorTest, testArrayVector_append)
{
    dolphindb::VectorSP vec = dolphindb::Util::createArrayVector(dolphindb::DT_BOOL_ARRAY, 0, 1);
    dolphindb::SmartPointer<dolphindb::FastArrayVector> av1 = vec;
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 0, 6);
    ASSERT_TRUE(av1->append(v1));
    ASSERT_EQ(av1->size(), 1);

    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 6, 6);
    v2->setNull(0);
    ASSERT_TRUE(av1->append(v2));
    ASSERT_EQ(av1->size(), 2);

    dolphindb::VectorSP indexArray = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2);
    indexArray->set(0, dolphindb::Util::createInt(0));
    indexArray->set(1, dolphindb::Util::createInt(1));

    ASSERT_FALSE(av1->append(dolphindb::Util::createInt(1), indexArray));
    ASSERT_FALSE(av1->append(v2, indexArray));

    dolphindb::VectorSP anyVector = dolphindb::Util::createVector(dolphindb::DT_ANY, 2, 2);
    ASSERT_TRUE(av1->append(anyVector, indexArray));
    ASSERT_EQ(av1->size(), 4);

    ASSERT_TRUE(av1->append(av1, indexArray));
    ASSERT_EQ(av1->size(), 6);
}

TEST_F(ArrayVectorTest, testArrayVector_checkVectorSize)
{
    dolphindb::VectorSP vec = dolphindb::Util::createArrayVector(dolphindb::DT_BOOL_ARRAY, 0, 2);
    dolphindb::SmartPointer<dolphindb::FastArrayVector> av = vec;
    ASSERT_EQ(av->checkVectorSize(), 0);

    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 6, 6);
    ASSERT_TRUE(av->append(v1));
    ASSERT_EQ(av->checkVectorSize(), 6);

    v1 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 5, 6);
    ASSERT_TRUE(av->append(v1));
    ASSERT_EQ(av->checkVectorSize(), -1);
}

TEST_F(ArrayVectorTest, testArrayVector_count)
{
    dolphindb::VectorSP vec = dolphindb::Util::createArrayVector(dolphindb::DT_BOOL_ARRAY, 0, 1);
    dolphindb::SmartPointer<dolphindb::FastArrayVector> av1 = vec;
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 0, 6);
    ASSERT_TRUE(av1->append(v1));
    ASSERT_EQ(av1->count(0, 1), 0);

    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 6, 6);
    v2->setNull(0);
    ASSERT_TRUE(av1->append(v2));
    ASSERT_EQ(av1->count(0, 2), 1);

    dolphindb::VectorSP indexArray = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2);
    indexArray->set(0, dolphindb::Util::createInt(0));
    indexArray->set(1, dolphindb::Util::createInt(1));

    ASSERT_FALSE(av1->append(dolphindb::Util::createInt(1), indexArray));
    ASSERT_ANY_THROW(av1->count(0, 3));
    ASSERT_FALSE(av1->append(v2, indexArray));
    ASSERT_ANY_THROW(av1->count(0, 4));

    dolphindb::VectorSP anyVector = dolphindb::Util::createVector(dolphindb::DT_ANY, 2, 2);
    ASSERT_TRUE(av1->append(anyVector, indexArray));
    ASSERT_EQ(av1->size(), 4);
    ASSERT_ANY_THROW(av1->count(0, 5));

    ASSERT_TRUE(av1->append(av1, indexArray));
    ASSERT_EQ(av1->size(), 6);
    ASSERT_EQ(av1->count(0, 6), 2);
}

TEST_F(ArrayVectorTest, testArrayVector_fill)
{
    dolphindb::VectorSP vec = dolphindb::Util::createArrayVector(dolphindb::DT_BOOL_ARRAY, 0, 1);
    dolphindb::SmartPointer<dolphindb::FastArrayVector> av1 = vec;

    av1->fill(0, 0, dolphindb::Util::createBool(true));
    ASSERT_EQ(av1->size(), 0);

    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 0, 6);
    ASSERT_TRUE(av1->append(v1));
    ASSERT_EQ(av1->size(), 1);

    av1->fill(0, 1, dolphindb::Util::createBool(true));
    ASSERT_EQ(av1->size(), 1);

    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 6, 6);
    v2->setNull(0);
    ASSERT_TRUE(av1->append(v2));
    ASSERT_EQ(av1->size(), 2);

    dolphindb::VectorSP indexArray = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2);
    indexArray->set(0, dolphindb::Util::createInt(0));
    indexArray->set(1, dolphindb::Util::createInt(1));

    av1->fill(0, 1, dolphindb::Util::createVector(dolphindb::DT_BOOL, 2, 2));

    ASSERT_FALSE(av1->append(dolphindb::Util::createInt(1), indexArray));
    ASSERT_FALSE(av1->append(v2, indexArray));

    dolphindb::VectorSP anyVector = dolphindb::Util::createVector(dolphindb::DT_ANY, 2, 2);
    ASSERT_TRUE(av1->append(anyVector, indexArray));
    ASSERT_EQ(av1->size(), 4);

    ASSERT_TRUE(av1->append(av1, indexArray));
    ASSERT_EQ(av1->size(), 6);
}

TEST_F(ArrayVectorTest, testArrayVector_get)
{
    dolphindb::VectorSP vec = dolphindb::Util::createArrayVector(dolphindb::DT_BOOL_ARRAY, 0, 1);
    dolphindb::SmartPointer<dolphindb::FastArrayVector> av1 = vec;

    ASSERT_ANY_THROW(av1->get(dolphindb::Util::createInt(-1)));

    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 2, 2);
    v1->set(0, dolphindb::Util::createBool(true));
    v1->set(1, dolphindb::Util::createBool(false));
    ASSERT_TRUE(av1->append(v1));
    ASSERT_EQ(av1->size(), 1);
    dolphindb::ConstantSP res = av1->get(dolphindb::Util::createInt(0));
    ASSERT_EQ(res->getString(), "[1]");

    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 6, 6);
    v2->setNull(0);
    ASSERT_TRUE(av1->append(v2));
    ASSERT_EQ(av1->size(), 2);

    dolphindb::ConstantSP pair = dolphindb::Util::createPair(dolphindb::DT_INT);
    pair->set(0, dolphindb::Util::createInt(-1));
    pair->set(1, dolphindb::Util::createInt(1));
    ASSERT_ANY_THROW(res = av1->get(pair));

    pair->set(0, dolphindb::Util::createInt(1));
    pair->set(1, dolphindb::Util::createInt(1));
    ASSERT_ANY_THROW(res = av1->get(pair));

    pair->set(0, dolphindb::Util::createInt(0));
    pair->set(1, dolphindb::Util::createInt(1));
    res = av1->get(pair);
    ASSERT_EQ(res->getString(), "[[1],[]]");

    dolphindb::VectorSP indexArray = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2);
    indexArray->set(0, dolphindb::Util::createInt(0));
    indexArray->set(1, dolphindb::Util::createInt(1));
    res = av1->get(indexArray);

    ASSERT_FALSE(av1->append(dolphindb::Util::createInt(1), indexArray));
    ASSERT_FALSE(av1->append(v2, indexArray));

    dolphindb::VectorSP anyVector = dolphindb::Util::createVector(dolphindb::DT_ANY, 2, 2);
    ASSERT_TRUE(av1->append(anyVector, indexArray));
    ASSERT_EQ(av1->size(), 4);
    ASSERT_ANY_THROW(res = av1->get(anyVector));
}

TEST_F(ArrayVectorTest, testArrayVector_getSubVector)
{
    dolphindb::VectorSP vec = dolphindb::Util::createArrayVector(dolphindb::DT_BOOL_ARRAY, 0, 1);
    dolphindb::SmartPointer<dolphindb::FastArrayVector> av1 = vec;

    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 2, 2);
    ASSERT_TRUE(av1->append(v1));
    dolphindb::ConstantSP av2 = av1->getSubVector(0, -1, 1);
    ASSERT_EQ(av2->getString(), "[]");

    av2 = av1->getSubVector(1, -1, 2);
    ASSERT_EQ(av2->getString(), "[]");
}

TEST_F(ArrayVectorTest, testArrayVector_set)
{
    dolphindb::VectorSP vec = dolphindb::Util::createArrayVector(dolphindb::DT_INT_ARRAY, 0, 2);
    dolphindb::SmartPointer<dolphindb::FastArrayVector> av1 = vec;

    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2);
    v1->set(0, dolphindb::Util::createInt(0));
    v1->set(1, dolphindb::Util::createInt(1));
    ASSERT_TRUE(av1->append(v1));
    ASSERT_FALSE(av1->set(v1, dolphindb::Util::createInt(0)));

    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3);
    v2->set(0, dolphindb::Util::createInt(0));
    v2->set(1, dolphindb::Util::createInt(1));
    v2->set(2, dolphindb::Util::createInt(2));
    ASSERT_FALSE(av1->set(v1, v2));

    dolphindb::VectorSP v3 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1, 1);
    v3->set(0, dolphindb::Util::createInt(0));
    ASSERT_FALSE(av1->set(v1, v3));

    dolphindb::VectorSP v4 = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2);
    v4->set(0, dolphindb::Util::createInt(0));
    v4->set(1, dolphindb::Util::createInt(1));
    ASSERT_FALSE(av1->set(v1, v4));

    dolphindb::VectorSP v5 = dolphindb::Util::createVector(dolphindb::DT_INT_ARRAY, 0, 2);
    v5->append(v2);
    v5->append(v2);
    ASSERT_FALSE(av1->set(v1, v5));
}
#endif // _MSC_VER

TEST_F(ArrayVectorTest, test_BoolArrayVector)
{
    std::vector<char> testValues{1, -1, 12, 0, -12};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_BOOL, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setBool(i, testValues[i]);
    }
    v1->setNull(5);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_BOOL_ARRAY, 0, 1);
    dolphindb::VectorSP av2 = dolphindb::Util::createArrayVector(dolphindb::DT_BOOL_ARRAY, 1, 1);
    dolphindb::VectorSP av3 = dolphindb::Util::createArrayVector(dolphindb::DT_BOOL_ARRAY, 1, 1);
    dolphindb::VectorSP av4 = dolphindb::Util::createArrayVector(dolphindb::DT_BOOL_ARRAY, 1, 1);
    av1->append(v1);
    av2->fill(0, 1, v1);
    av3->set(0, v1);
    av4->append(dolphindb::Util::createVector(dolphindb::DT_BOOL, 6, 6));
    av4->set(dolphindb::Util::createInt(1), v1);

    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "BOOL[]");
    std::vector<std::string> names = {"av1", "av2", "av3", "av4"};
    std::vector<dolphindb::ConstantSP> datas = {av1, av2, av3, av4};
    conn.upload(names, datas);
    std::string script = "value = bool[1,-1,12,0,-12,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    auto ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());

    ASSERT_EQ(av1->getRawType(), dolphindb::DT_BOOL);
    ASSERT_EQ(av1->getForm(), dolphindb::DF_VECTOR);
    ASSERT_FALSE(av1->validIndex(2));
    ASSERT_FALSE(av1->validIndex(0, 1, 2));
    ASSERT_EQ(av1->getUnitLength(), 1);
    ASSERT_TRUE(av1->sizeable());
    ASSERT_TRUE(av1->isIndexArray());

    ASSERT_ANY_THROW(av1->compare(0, dolphindb::Util::createInt(1)));
    ASSERT_ANY_THROW(av1->neg());
    ASSERT_ANY_THROW(av1->prev(0));
    ASSERT_ANY_THROW(av1->next(0));
    int *buf = new int[1];
    ASSERT_ANY_THROW(av1->getHash(0, 1, 1, buf));
    delete[] buf;
}

TEST_F(ArrayVectorTest, test_CharArrayVector)
{
    std::vector<char> testValues{1, -1, 12, 0, -12};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_CHAR, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setChar(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_CHAR_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "CHAR[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = char[1,-1,12,0,-12,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_ShortArrayVector)
{
    std::vector<short> testValues{1, -1, 12, 0, -12};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_SHORT, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setShort(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_SHORT_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "SHORT[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = short[1,-1,12,0,-12,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_IntArrayVector)
{
    std::vector<int> testValues{1, -1, 12, 0, -12};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setInt(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_INT_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "INT[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = int[1,-1,12,0,-12,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());

    dolphindb::VectorSP inst_av1 = av1->getInstance(0);
    ASSERT_EQ(inst_av1->getString(), "[]");
    dolphindb::VectorSP inst_av2 = av1->getInstance(1);
    ASSERT_EQ(inst_av2->size(), 1);

    dolphindb::VectorSP indexVec = dolphindb::Util::createIndexVector(0, 1);
    dolphindb::ConstantSP index = dolphindb::Util::createInt(0);
    dolphindb::INDEX ind = 0;
    dolphindb::ConstantSP val_1 = dolphindb::Util::createInt(1000);
    dolphindb::VectorSP val_v1 = dolphindb::Util::createVector(dolphindb::DT_INT, 1, 1);
    val_v1->set(0, dolphindb::Util::createInt(2000));
    dolphindb::VectorSP val_v2 = dolphindb::Util::createVector(dolphindb::DT_INT, 1, 1);
    val_v2->set(0, dolphindb::Util::createInt(3000));
    dolphindb::VectorSP val_t3 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1, 1);
    val_t3->set(0, dolphindb::Util::createInt(4000));
    dolphindb::VectorSP val_av1 = dolphindb::Util::createArrayVector(dolphindb::DT_INT_ARRAY, 0, 1);
    dolphindb::VectorSP val_av2 = dolphindb::Util::createArrayVector(dolphindb::DT_INT_ARRAY, 0, 2);
    val_av1->append(val_v2);

    ASSERT_FALSE(av1->set(ind, val_av2));
    av1->set(ind, val_1);
    ASSERT_EQ(av1->getString(0), "[1000]");
    av1->set(ind, val_v1);
    ASSERT_EQ(av1->getString(0), "[2000]");
    av1->set(ind, val_av1);
    ASSERT_EQ(av1->getString(0), "[3000]");
    av1->set(ind, val_t3);
    ASSERT_EQ(av1->getString(0), "[4000]");
    val_t3->append(dolphindb::Util::createInt(4000));
    ASSERT_FALSE(av1->set(ind, val_t3));

    av1->set(0, val_v2);
    val_t3->remove(1);
    av1->set(indexVec, val_1);
    ASSERT_EQ(av1->getString(0), "[1000]");
    av1->set(indexVec, val_v1);
    ASSERT_EQ(av1->getString(0), "[2000]");
    av1->set(indexVec, val_av1);
    ASSERT_EQ(av1->getString(0), "[3000]");
    av1->set(indexVec, val_t3);
    ASSERT_EQ(av1->getString(0), "[4000]");
    val_t3->append(dolphindb::Util::createInt(4000));
    ASSERT_FALSE(av1->set(indexVec, val_t3));

    val_t3->remove(1);
    dolphindb::TableSP tab1 = conn.run("table(1 2 3 as col1)");
    ASSERT_ANY_THROW(av1->fill(0, 10, val_1));
    ASSERT_ANY_THROW(av1->fill(0, 1, tab1));
    ASSERT_ANY_THROW(av1->fill(1, 1, val_1));

    av1->fill(0, 1, val_1);
    ASSERT_EQ(av1->getString(), "[[1000]]");
    av1->append(val_1);
    val_av1->append(val_v2);

    av1->fill(0, 2, val_av1);
    ASSERT_EQ(av1->getString(), "[[3000],[3000]]");
    val_t3->append(dolphindb::Util::createInt(4000));
    av1->fill(0, 2, val_t3);
    ASSERT_EQ(av1->getString(), "[[4000],[4000]]");

    av1->set(0, val_v1);

    av1->reverse(0, 0);
    av1->reverse();
    ASSERT_EQ(av1->getString(), "[[4000],[2000]]");
    av1->reverse(0, 2);
    ASSERT_EQ(av1->getString(), "[[2000],[4000]]");

    av1->append(val_1, 1);
    ASSERT_EQ(av1->getString(), "[[2000],[4000],[1000]]");
    av1->append(val_av1, 1);
    ASSERT_EQ(av1->getString(), "[[2000],[4000],[1000],[3000]]");
    av1->append(val_t3, 1);
    ASSERT_EQ(av1->getString(), "[[2000],[4000],[1000],[3000],[4000]]");

    ASSERT_FALSE(av1->remove(10));
    av1->remove(2);
    ASSERT_EQ(av1->getString(), "[[2000],[4000],[1000]]");
    av1->remove(-1);
    ASSERT_EQ(av1->getString(), "[[4000],[1000]]");
    av1->remove(2);
    ASSERT_EQ(av1->getString(), "[]");
    ASSERT_TRUE(av1->append(val_v1));
    av1->remove(-1);
    ASSERT_EQ(av1->getString(), "[]");

    av1->append(val_v1);
    av1->append(val_v2);
    av1->append(val_t3);
    dolphindb::VectorSP nulIndexVec = dolphindb::Util::createIndexVector(0, 0);
    dolphindb::VectorSP IndexVec = dolphindb::Util::createIndexVector(0, 1);
    dolphindb::VectorSP IndexVec1 = dolphindb::Util::createIndexVector(0, 3);
    ASSERT_TRUE(av1->remove(nulIndexVec));
    ASSERT_FALSE(av1->remove(dolphindb::Util::createInt(1)));
    av1->remove(IndexVec);
    ASSERT_EQ(av1->getString(), "[[3000],[4000],[4000]]");
    av1->remove(IndexVec1);
    ASSERT_EQ(av1->getString(), "[]");

    av1->append(val_v1);
    av1->append(val_v2);
    av1->append(val_t3);
    ASSERT_EQ(av1->get(0, 0, 1)->getString(), av1->get(0)->getString());
    ASSERT_EQ(av1->get(0, 1, 2)->getString(), av1->get(1)->getString());
    ASSERT_EQ(av1->get(0, 2, 3)->getString(), av1->get(2)->getString());

    ASSERT_FALSE(av1->isNull());
    av1->append(dolphindb::Util::createNullConstant(dolphindb::DT_INT));
    ASSERT_FALSE(av1->isNull(3));
    ASSERT_TRUE(av1->isNull(4));

    char *buf = new char[5];
    char *buf1 = new char[5];
    av1->isNull(0, 5, buf);
    av1->isValid(0, 5, buf1);
    for (int i = 0; i < 4; i++)
    {
        ASSERT_TRUE((int)buf1[i]);
        ASSERT_FALSE((int)buf[i]);
    }
    ASSERT_TRUE((int)buf[4]);
    ASSERT_FALSE((int)buf1[4]);

    delete[] buf;
    delete[] buf1;

    for (unsigned int i = 0; i < 121; i++)
        val_v1->append(dolphindb::Util::createInt(1));
    av1->set(0, val_v1);
    ASSERT_EQ(av1->getString(0), "[2000,1,1...]");
    ASSERT_EQ(av1->getString(), "[[2000,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1...],[3000],[4000],[4000],[]]");
}

TEST_F(ArrayVectorTest, test_LongArrayVector)
{
    std::vector<long> testValues{1, -1, 12, 0, -12};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_LONG, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setLong(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_LONG_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "LONG[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = long[1,-1,12,0,-12,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_DateArrayVector)
{
    std::vector<int> testValues{1, -1, 12, 0, -12};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_DATE, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setInt(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "DATE[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = date[1,-1,12,0,-12,NULL];index = [6];b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_MonthArrayVector)
{
    std::vector<int> testValues{1, -1, 12, 0, -12};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_MONTH, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setInt(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_MONTH_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "MONTH[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = month[1,-1,12,0,-12,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_TimeArrayVector)
{
    std::vector<int> testValues{1, 123123, 12, 0, 111};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_TIME, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setInt(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_TIME_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "TIME[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = time[1,123123,12,0,111,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_MinuteArrayVector)
{
    std::vector<int> testValues{1, 120, 12, 0, 111};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_MINUTE, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setInt(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_MINUTE_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "MINUTE[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = minute[1,120,12,0,111,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_SecondArrayVector)
{
    std::vector<int> testValues{1, 123, 12, 0, 86399};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_SECOND, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setInt(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "SECOND[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = second[1,123,12,0,86399,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_DatetimeArrayVector)
{
    std::vector<int> testValues{1, -1, 12, 0, -12};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setInt(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATETIME_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "DATETIME[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = datetime[1,-1,12,0,-12,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_TimestampArrayVector)
{
    std::vector<int> testValues{1, -1, 12, 0, -12};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setInt(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_TIMESTAMP_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "TIMESTAMP[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = timestamp[1,-1,12,0,-12,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_NanotimeArrayVector)
{
    std::vector<long long> testValues{1, 123, 12, 0, 10000000000000};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setLong(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIME_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "NANOTIME[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = nanotime[1,123,12,0,10000000000000,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_NanotimestampArrayVector)
{
    std::vector<long long> testValues{1, -1, -12, 0, 100000000000000000};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setLong(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIMESTAMP_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "NANOTIMESTAMP[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = nanotimestamp[1,-1,-12,0,100000000000000000,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_FloatArrayVector)
{
    std::vector<float> testValues{1.522222f, -1.5f, -12.0f, 0, 100000000000000000.1f};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_FLOAT, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setFloat(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_FLOAT_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "FLOAT[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = float[1.522222,-1.5,-12.0,0,100000000000000000.1,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_DoubleArrayVector)
{
    std::vector<double> testValues{1.533333333333, -1.5, -12.0, 0, 100000000000000000.1};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setDouble(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DOUBLE_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "DOUBLE[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = double[1.533333333333,-1.5,-12.0,0,100000000000000000.1,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_SymbolArrayVector)
{
    std::vector<std::string> testValues{"a123", "智臾科技a", "你好！a", "~`!@#$%^&*()/*-a", "~·！@#￥%……&*（）+——a"};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_SYMBOL, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setString(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    ASSERT_ANY_THROW(dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_SYMBOL_ARRAY, 0, 1));
}

TEST_F(ArrayVectorTest, test_StringArrayVector)
{
    std::vector<std::string> testValues{"a123", "智臾科技a", "你好！a", "~`!@#$%^&*()/*-a", "~·！@#￥%……&*（）+——a"};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_STRING, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setString(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    ASSERT_ANY_THROW(dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_STRING_ARRAY, 0, 1));
}

TEST_F(ArrayVectorTest, test_UuidArrayVector)
{
    std::vector<std::string> testValues{"5d212a78-cc48-e3b1-4235-b4d91473ee87", "5d212a78-cc48-e3b1-4235-b4d91473ee88",
                              "5d212a78-cc48-e3b1-4235-b4d91473ee89", "5d212a78-cc48-e3b1-4235-b4d91473ee90", "5d212a78-cc48-e3b1-4235-b4d91473ee91"};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_UUID, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setString(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_UUID_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "UUID[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = uuid[\"5d212a78-cc48-e3b1-4235-b4d91473ee87\",\"5d212a78-cc48-e3b1-4235-b4d91473ee88\",\
                                \"5d212a78-cc48-e3b1-4235-b4d91473ee89\",\"5d212a78-cc48-e3b1-4235-b4d91473ee90\",\"5d212a78-cc48-e3b1-4235-b4d91473ee91\",NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_AnyArrayVector)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    v1->setInt(0, 5);
    v1->setDouble(1, 1.326586);
    v1->setString(2, "abc");
    v1->setBool(3, 1);
    v1->setShort(4, 5);
    v1->setNull(5);
    anyv1->set(0, v1);

    ASSERT_ANY_THROW(dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_ANY_ARRAY, 0, 1));
}

TEST_F(ArrayVectorTest, test_BlobArrayVector)
{
    std::vector<std::string> testValues{"a123", "智臾科技a", "你好！a", "~`!@#$%^&*()/*-a", "~·！@#￥%……&*（）+——a"};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_BLOB, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setString(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    ASSERT_ANY_THROW(dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_BLOB_ARRAY, 0, 1));
}

TEST_F(ArrayVectorTest, test_CompressArrayVector)
{
    std::vector<int> testValues{1, -1, 12, 0, -12};
    ASSERT_ANY_THROW(dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_COMPRESS, 6, 6));
}

TEST_F(ArrayVectorTest, test_DatehourArrayVector)
{
    std::vector<int> testValues{1, -1, 12, 0, -12};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setInt(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "DATEHOUR[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = datehour[1,-1,12,0,-12,NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_IpaddrArrayVector)
{
    std::vector<std::string> testValues{"192.168.1.13", "192.168.1.14",
                              "192.168.1.15", "192.168.1.16", "192.168.1.17"};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_IP, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setString(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_IP_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "IPADDR[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = ipaddr[\"192.168.1.13\",\"192.168.1.14\",\
                                \"192.168.1.15\",\"192.168.1.16\",\"192.168.1.17\",NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, test_Int128ArrayVector)
{
    std::vector<std::string> testValues{"e1671797c52e15f763380b45e841ec32", "e1671797c52e15f763380b45e841ec33",
                              "e1671797c52e15f763380b45e841ec34", "e1671797c52e15f763380b45e841ec35", "e1671797c52e15f763380b45e841ec36"};
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_INT128, 6, 6);
    dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);

    for (unsigned i = 0; i < testValues.size(); i++)
    {
        v1->setString(i, testValues[i]);
    }
    v1->setNull(5);
    anyv1->set(0, v1);

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_INT128_ARRAY, 0, 1);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "INT128[]");
    av1->append(anyv1);
    conn.upload("av1", {av1});
    std::string script = "value = int128[\"e1671797c52e15f763380b45e841ec32\",\"e1671797c52e15f763380b45e841ec33\",\
                                \"e1671797c52e15f763380b45e841ec34\",\"e1671797c52e15f763380b45e841ec35\",\"e1671797c52e15f763380b45e841ec36\",NULL]\n\
                    index = [6]\n\
                    b=arrayVector(index, value);b";
    dolphindb::TableSP ex_av1 = conn.run(script);
    dolphindb::ConstantSP res = conn.run("eqObj(av1,b)");

    ASSERT_TRUE(res->getBool());
    ASSERT_EQ(av1->getString(), ex_av1->getString());
    ASSERT_EQ(av1->getType(), ex_av1->getType());
}

TEST_F(ArrayVectorTest, testUploadDatetimeArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createDateTime(i + 1));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas1"};
    conn.upload(dataname, datas);
    res = conn.run("datas1+1");
    dolphindb::VectorSP expection = conn.run("a = take(datetime(1..10),10);res = arrayVector([2,5,8,10],a);res+1");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadMonthArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_MONTH, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createMonth(i + 1));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas"};
    conn.upload(dataname, datas);
    res = conn.run("re1=datas+1;re1;");
    dolphindb::VectorSP expection = conn.run("a = take(month(1..10),10);res = arrayVector([2,5,8,10],a);re2=res+1;re2");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadDoubleArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createDouble(i + 1.1));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas3"};
    conn.upload(dataname, datas);
    res = conn.run("datas3+1");
    dolphindb::VectorSP expection = conn.run("a = take(1..10,10)+0.1;res = arrayVector([2,5,8,10],a);res+1");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadDatehourArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createDateHour(i + 1));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas"};
    conn.upload(dataname, datas);
    res = conn.run("datas+1");
    dolphindb::VectorSP expection = conn.run("a = take(datehour(1..10),10);res = arrayVector([2,5,8,10],a);res+1");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadTimeArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_TIME, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createTime(i + 1000));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas4"};
    conn.upload(dataname, datas);
    dolphindb::VectorSP expection = conn.run("a = take(time(0..9+1000),10);res = arrayVector([2,5,8,10],a);re2=res+1;re2;");
    dolphindb::VectorSP res = conn.run("datas4+1;");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadNanotimeArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createNanoTime(i + 1));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas"};
    conn.upload(dataname, datas);
    res = conn.run("datas+1");
    dolphindb::VectorSP expection = conn.run("a = take(nanotime(1..10),10);res = arrayVector([2,5,8,10],a);res+1");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadTimeStampArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createTimestamp(i + 1));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas"};
    conn.upload(dataname, datas);
    res = conn.run("datas+1");
    dolphindb::VectorSP expection = conn.run("a = take(timestamp(1..10),10);res = arrayVector([2,5,8,10],a);res+1");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadIntArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    dolphindb::VectorSP nonFastindex = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10, false);
    dolphindb::VectorSP nullValue = dolphindb::Util::createVector(dolphindb::DT_INT, 10, 20);
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_INT, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createInt(i + 1));
        nullValue->setNull(i);
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));

    dolphindb::VectorSP nullData = dolphindb::Util::createArrayVector(index, nullValue);
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas"};
    conn.upload(dataname, datas);
    dolphindb::VectorSP expection = conn.run("a = take(1..10,10);res = arrayVector([2,5,8,10],a);re2=res+1;re2;");
    res = conn.run("re1=datas+1;re1;");
    ASSERT_ANY_THROW(data = dolphindb::Util::createArrayVector(nonFastindex, value));
    ASSERT_EQ(nullData->getString(), "[[,],[,,],[,,],[,]]");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadDateArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_DATE, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createDate(2012, 1, 2));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas"};
    conn.upload(dataname, datas);
    res = conn.run("re1=datas+1;re1");
    dolphindb::VectorSP expection = conn.run("a = take(2012.01.02,10);res = arrayVector([2,5,8,10],a);re2=res+1;re2;");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadCharArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_CHAR, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createChar(i + 1));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas1"};
    conn.upload(dataname, datas);
    res = conn.run("datas1+1");
    dolphindb::VectorSP expection = conn.run("a = take(char(1..10),10);res = arrayVector([2,5,8,10],a);res+1");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadBoolArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_BOOL, 10, 20);
    for (char i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createBool((char)((i + 1) % 2)));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas1"};
    conn.upload(dataname, datas);
    dolphindb::VectorSP expection = conn.run("a = take(bool((1..10)%2),10);res = arrayVector([2,5,8,10],a);res+1;");
    res = conn.run("datas1+1");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadShortArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_SHORT, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createShort((i + 1)));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas1"};
    conn.upload(dataname, datas);
    dolphindb::VectorSP expection = conn.run("a = take(short(1..10),10);res = arrayVector([2,5,8,10],a);res+1;");
    res = conn.run("datas1+1");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadLongArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_LONG, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createLong(i + 1));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas1"};
    conn.upload(dataname, datas);
    dolphindb::VectorSP expection = conn.run("a = take(long(1..10),10);res = arrayVector([2,5,8,10],a);res+1;");
    res = conn.run("re2=datas1+1;re2");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadMinuteArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_MINUTE, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createMinute(i + 1));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas2"};
    conn.upload(dataname, datas);
    res = conn.run("re2=datas2+1;re2");
    dolphindb::VectorSP expection = conn.run("a=take(minute(1..10),10);res = arrayVector([2,5,8,10],a);res+1;");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testUploadFloatArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_FLOAT, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createFloat(i + 1.1));
    }
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(10));
    data = dolphindb::Util::createArrayVector(index, value);
    std::vector<dolphindb::ConstantSP> datas = {data};
    std::vector<std::string> dataname = {"datas1"};
    conn.upload(dataname, datas);
    dolphindb::VectorSP expection = conn.run("a=take(float(1..10)+0.1,10);res = arrayVector([2,5,8,10],a);re1=res+1;re1;");
    res = conn.run("re2=datas1+1;re2;");
    ASSERT_EQ(res->getString(), expection->getString());
}

TEST_F(ArrayVectorTest, testIntArrayVector_1)
{
    // NULL
    dolphindb::VectorSP v1 = conn.run("a=[int()];"
                           "b = arrayVector([1],a);b");
    dolphindb::ConstantSP v2 = v1->get(0);
    dolphindb::ConstantSP re1 = conn.run("a = [int()];a");
    ASSERT_EQ(v2->getString(), re1->getString());

    // not NULL
    v1 = conn.run("a=[1];"
                  "b = arrayVector([1],a);b");
    v2 = v1->get(0);
    ASSERT_EQ(v2->getInt(0), 1);
}

TEST_F(ArrayVectorTest, testIntArrayVectorSmaller256)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=take(1..128,128);"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=take(1..128,128);\n"
                           "index = rand(1..127,10);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(int(),128);\n"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(5 3 2 6, 32).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 32);
    ASSERT_EQ(v1->getValueSize(), 128);
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getInt(j), data1->getInt(idx + j));
            ASSERT_EQ(temp2->getInt(j), data2->getInt(idx + j));
            ASSERT_EQ(temp3->getInt(j), data3->getInt(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testIntArrayVectorSmaller65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=take(1..32768,32768);"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=take(1..32768,32768);\n"
                           "index = rand(1..32767,100);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(int(),32768);\n"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(256 200 312 256, 128).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 128);
    ASSERT_EQ(v1->getValueSize(), 32768);
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getInt(j), data1->getInt(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testIntArrayVectorBigger65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=take(1..100,524288);"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=take(1..100,524288);\n"
                           "index = rand(1..524287,10000);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(int(),524288);\n"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(65536 66072 60000 70536, 8).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 8);
    ASSERT_EQ(v1->getValueSize(), 524288);
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getInt(j), data1->getInt(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDoubleArrayVector_1)
{
    // NULL
    dolphindb::VectorSP v1 = conn.run("a=[double()];"
                           "b = arrayVector([1],a);b");
    dolphindb::ConstantSP v2 = v1->get(0);
    dolphindb::ConstantSP re1 = conn.run("[double()]");
    ASSERT_EQ(v2->getString(), re1->getString());

    // not NULL
    v1 = conn.run("a=[1.5];"
                  "b = arrayVector([1],a);b");
    v2 = v1->get(0);
    ASSERT_EQ(v2->getDouble(0), 1.5);
}

TEST_F(ArrayVectorTest, testDoubleArrayVectorSmaller256)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=rand(128.0,128);"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=rand(128.0,128);\n"
                           "index = rand(1..127,10);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(double(),128);\n"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(5 3 2 6, 32).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 32);
    ASSERT_EQ(v1->getValueSize(), 128);
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_DOUBLE_EQ(temp1->getDouble(j), data1->getDouble(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDoubleArrayVectorSmaller65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=rand(32768.0,32768);"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=rand(32768.0,32768);\n"
                           "index = rand(1..32767,100);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(int(),32768);\n"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(256 200 312 256, 128).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 128);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_DOUBLE_EQ(temp1->getDouble(j), data1->getDouble(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDoubleArrayVectorBigger65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=rand(524288.0,524288);"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=rand(524288.0,524288);\n"
                           "index = rand(1..524287,10000);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(double(),524288);\n"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(65536 66072 60000 70536, 8).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 8);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_DOUBLE_EQ(temp1->getDouble(j), data1->getDouble(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDatehourArrayVector_1)
{
    // NULL
    dolphindb::VectorSP v1 = conn.run("a=[datehour()];"
                           "b = arrayVector([1],a);b");
    dolphindb::ConstantSP v2 = v1->get(0);
    dolphindb::ConstantSP re1 = conn.run("[datehour()]");
    ASSERT_EQ(v2->getString(), re1->getString());

    // not NULL
    v1 = conn.run("a=[datehour(1)];"
                  "b = arrayVector([1],a);b");
    v2 = v1->get(0);
    ASSERT_EQ(v2->getString(0), "1970.01.01T01");
}

TEST_F(ArrayVectorTest, testDatehourArrayVectorSmaller256)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=datehour(1..128);"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=datehour(1..128);\n"
                           "index = rand(1..127,10);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(datehour(),128);\n"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(5 3 2 6, 32).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 32);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDatehourArrayVectorSmaller65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=datehour(1..32768);"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=datehour(1..32768);\n"
                           "index = rand(1..32767,100);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(datehour(),32768);\n"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(256 200 312 256, 128).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 128);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDatehourArrayVectorBigger65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=datehour(1..524288);"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=datehour(1..524288);\n"
                           "index = rand(1..524287,10000);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(datehour(),524288);\n"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(65536 66072 60000 70536, 8).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 8);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_DOUBLE_EQ(temp1->getDouble(j), data1->getDouble(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDateArrayVector_1)
{
    // NULL
    dolphindb::VectorSP v1 = conn.run("a=[date()];"
                           "b = arrayVector([1],a);b");
    dolphindb::ConstantSP v2 = v1->get(0);
    dolphindb::ConstantSP re1 = conn.run("[date()]");
    ASSERT_EQ(v2->getString(), re1->getString());

    // not NULL
    v1 = conn.run("a=[date(1)];"
                  "b = arrayVector([1],a);b");
    v2 = v1->get(0);
    ASSERT_EQ(v2->getString(0), "1970.01.02");
}

TEST_F(ArrayVectorTest, testDateArrayVectorSmaller256)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=date(1..128);"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=date(1..128);\n"
                           "index = rand(1..127,10);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(date(),128);\n"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(5 3 2 6, 32).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 32);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDateArrayVectorSmaller65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=date(1..32768);"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=date(1..32768);\n"
                           "index = rand(1..32767,100);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(date(),32768);\n"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(256 200 312 256, 128).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 128);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDateArrayVectorBigger65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=date(1..524288);"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=date(1..524288);\n"
                           "index = rand(1..524287,10000);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(date(),524288);\n"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(65536 66072 60000 70536, 8).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 8);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_DOUBLE_EQ(temp1->getDouble(j), data1->getDouble(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDatetimeArrayVector_1)
{
    // NULL
    dolphindb::VectorSP v1 = conn.run("a=[datetime()];"
                           "b = arrayVector([1],a);b");
    dolphindb::ConstantSP v2 = v1->get(0);
    dolphindb::ConstantSP re1 = conn.run("[datetime()]");
    ASSERT_EQ(v2->getString(), re1->getString());

    // not NULL
    v1 = conn.run("a=[datetime(1)];"
                  "b = arrayVector([1],a);b");
    v2 = v1->get(0);
    ASSERT_EQ(v2->getString(0), "1970.01.01T00:00:01");
}

TEST_F(ArrayVectorTest, testDatetimeArrayVectorSmaller256)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=datetime(1..128);"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=datetime(1..128);\n"
                           "index = rand(1..127,10);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(datetime(),128);\n"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(5 3 2 6, 32).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 32);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDatetimeArrayVectorSmaller65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=datetime(1..32768);"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=datetime(1..32768);\n"
                           "index = rand(1..32767,100);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(datetime(),32768);\n"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(256 200 312 256, 128).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 128);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDatetimeArrayVectorBigger65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=datetime(1..524288);"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=datetime(1..524288);\n"
                           "index = rand(1..524287,10000);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(datetime(),524288);\n"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a;");
    dolphindb::ConstantSP index = conn.run("index=0 join take(65536 66072 60000 70536, 8).cumsum().int();index");
    int size1 = v1->size();
    ASSERT_EQ(size1, 8);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testTimeStampArrayVector_1)
{
    // NULL
    dolphindb::VectorSP v1 = conn.run("a=[timestamp()];"
                           "b = arrayVector([1],a);b");
    dolphindb::ConstantSP v2 = v1->get(0);
    dolphindb::ConstantSP re1 = conn.run("[timestamp()]");
    ASSERT_EQ(v2->getString(), re1->getString());

    // not NULL
    v1 = conn.run("a=[timestamp(1)];"
                  "b = arrayVector([1],a);b");
    v2 = v1->get(0);
    ASSERT_EQ(v2->getString(0), "1970.01.01T00:00:00.001");
}

TEST_F(ArrayVectorTest, testTimeStampArrayVectorSmaller256)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=timestamp(1..128);"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=timestamp(1..128);\n"
                           "index = rand(1..127,10);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(timestamp(),128);\n"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(5 3 2 6, 32).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 32);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testTimeStampArrayVectorSmaller65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=timestamp(1..32768);"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=timestamp(1..32768);\n"
                           "index = rand(1..32767,100);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(timestamp(),32768);\n"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(256 200 312 256, 128).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 128);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testTimeStampArrayVectorBigger65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=timestamp(1..524288);"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=timestamp(1..524288);\n"
                           "index = rand(1..524287,10000);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(timestamp(),524288);\n"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("index=0 join take(65536 66072 60000 70536, 8).cumsum().int();index");
    int size1 = v1->size();
    ASSERT_EQ(size1, 8);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_DOUBLE_EQ(temp1->getDouble(j), data1->getDouble(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testNanotimestampArrayVector_1)
{
    // NULL
    dolphindb::VectorSP v1 = conn.run("a=[nanotimestamp()];"
                           "b = arrayVector([1],a);b");
    dolphindb::ConstantSP v2 = v1->get(0);
    dolphindb::ConstantSP re1 = conn.run("[nanotimestamp()]");
    ASSERT_EQ(v2->getString(), re1->getString());

    // not NULL
    v1 = conn.run("a=[nanotimestamp(1)];"
                  "b = arrayVector([1],a);b");
    v2 = v1->get(0);
    ASSERT_EQ(v2->getString(0), "1970.01.01T00:00:00.000000001");
}

TEST_F(ArrayVectorTest, testNanotimestampArrayVectorSmaller256)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=nanotimestamp(1..128);"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=nanotimestamp(1..128);\n"
                           "index = rand(1..127,10);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(nanotimestamp(),128);\n"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(5 3 2 6, 32).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 32);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testNanotimestampArrayVectorSmaller65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=nanotimestamp(1..32768);"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=nanotimestamp(1..32768);\n"
                           "index = rand(1..16384,100);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(nanotimestamp(),32768);\n"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(256 200 312 256, 128).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 128);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testNanotimestampArrayVectorBigger65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=nanotimestamp(1..524288);"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=nanotimestamp(1..524288);\n"
                           "index = rand(1..524287,10000);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(nanotimestamp(),524288);\n"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("index=0 join take(65536 66072 60000 70536, 8).cumsum().int();index");
    int size1 = v1->size();
    ASSERT_EQ(size1, 8);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_DOUBLE_EQ(temp1->getDouble(j), data1->getDouble(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testMonthArrayVector_1)
{
    // NULL
    dolphindb::VectorSP v1 = conn.run("a=[month()];"
                           "b = arrayVector([1],a);b");
    dolphindb::ConstantSP v2 = v1->get(0);
    dolphindb::ConstantSP re1 = conn.run("[month()]");
    ASSERT_EQ(v2->getString(), re1->getString());

    // not NULL
    v1 = conn.run("a=[month(1970.01.11)];"
                  "b = arrayVector([1],a);b");
    v2 = v1->get(0);
    ASSERT_EQ(v2->getString(0), "1970.01M");
}

TEST_F(ArrayVectorTest, testMonthArrayVectorSmaller256)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=month(1970.01.12+1..128);"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=month(1970.01.12+1..128);\n"
                           "index = rand(1..127,10);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(month(),128);\n"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(5 3 2 6, 32).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 32);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testMonthArrayVectorSmaller65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=month(1970.01.12+1..32768);"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=month(1970.01.12+1..32768);\n"
                           "index = rand(1..16384,100);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(month(),32768);\n"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(256 200 312 256, 128).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 128);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testMonthArrayVectorBigger65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=month(1921.01.01+1..524288);"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=month(1921.01.01+1..524288);\n"
                           "index = rand(1..524287,10000);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(month(),524288);\n"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("index=0 join take(65536 66072 60000 70536, 8).cumsum().int();index");
    int size1 = v1->size();
    ASSERT_EQ(size1, 8);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testTimeArrayVector_1)
{
    // NULL
    dolphindb::VectorSP v1 = conn.run("a=[time()];"
                           "b = arrayVector([1],a);b");
    dolphindb::ConstantSP v2 = v1->get(0);
    dolphindb::ConstantSP re1 = conn.run("[time()]");
    ASSERT_EQ(v2->getString(), re1->getString());

    // not NULL
    v1 = conn.run("a=[time(13:30:10.008)];"
                  "b = arrayVector([1],a);b");
    v2 = v1->get(0);
    ASSERT_EQ(v2->getString(0), "13:30:10.008");
}

TEST_F(ArrayVectorTest, testTimeArrayVectorSmaller256)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=time(13:30:10.008+1..128);"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=time(13:30:10.008+1..128);\n"
                           "index = rand(1..127,10);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(time(),128);\n"
                           "b = arrayVector(take(5 3 2 6, 32).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(5 3 2 6, 32).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 32);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testTimeArrayVectorSmaller65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=time(13:30:10.008+1..32768);"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=time(13:30:10.008+1..32768);\n"
                           "index = rand(1..16383,100);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(time(),32768);\n"
                           "b = arrayVector(take(256 200 312 256, 128).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("0 join take(256 200 312 256, 128).cumsum().int()");
    int size1 = v1->size();
    ASSERT_EQ(size1, 128);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testTimeArrayVectorBigger65535)
{
    // not NULL
    dolphindb::VectorSP v1 = conn.run("a=time(13:30:10.008+1..524288);"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data1 = conn.run("a");

    dolphindb::VectorSP v2 = conn.run("a=time(13:30:10.008+1..524288);\n"
                           "index = rand(1..262143,10000);\n"
                           "for(idx in index){\n"
                           "\ta[idx] = NULL\t\n"
                           "};"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data2 = conn.run("a");
    dolphindb::VectorSP v3 = conn.run("a=take(time(),524288);\n"
                           "b = arrayVector(take(65536 66072 60000 70536, 8).cumsum().int(),a);b");
    dolphindb::ConstantSP data3 = conn.run("a");
    dolphindb::ConstantSP index = conn.run("index=0 join take(65536 66072 60000 70536, 8).cumsum().int();index");
    int size1 = v1->size();
    ASSERT_EQ(size1, 8);
    ASSERT_EQ(v1->getValueSize(), index->getInt(index->size() - 1));
    int idx = 0;
    for (int i = 0; i < size1; ++i)
    {
        dolphindb::ConstantSP temp1 = v1->get(i);
        dolphindb::ConstantSP temp2 = v2->get(i);
        dolphindb::ConstantSP temp3 = v3->get(i);
        int size2 = temp1->size();
        for (int j = 0; j < size2; ++j)
        {
            idx = index->getInt(i);
            ASSERT_EQ(temp1->getString(j), data1->getString(idx + j));
            ASSERT_EQ(temp2->getString(j), data2->getString(idx + j));
            ASSERT_EQ(temp3->getString(j), data3->getString(idx + j));
        }
    }
}

TEST_F(ArrayVectorTest, testDiffTypeArrayVector)
{
    dolphindb::VectorSP v1;
    dolphindb::ConstantSP re;
    v1 = conn.run("a=[int()];"
                  "b = arrayVector([1],a);b");

    re = conn.run("[1,2,3]");
    ASSERT_EQ(v1->get(0)->getType(), re->getType());
    ASSERT_EQ(v1->get(0)->getRawType(), re->getRawType());

    v1 = conn.run("a=[double(1.3)];"
                  "b = arrayVector([1],a);b");

    re = conn.run("[1.3,3.2]");
    ASSERT_EQ(v1->get(0)->getType(), re->getType());
    ASSERT_EQ(v1->get(0)->getRawType(), re->getRawType());

    v1 = conn.run("a=[datehour(1)];"
                  "b = arrayVector([1],a);b");

    re = conn.run("[datehour(1)]");
    ASSERT_EQ(v1->get(0)->getType(), re->getType());
    ASSERT_EQ(v1->get(0)->getRawType(), re->getRawType());

    v1 = conn.run("a=[date(1)];"
                  "b = arrayVector([1],a);b");

    re = conn.run("[date(1)]");
    ASSERT_EQ(v1->get(0)->getType(), re->getType());
    ASSERT_EQ(v1->get(0)->getRawType(), re->getRawType());

    v1 = conn.run("a=[datetime(1)];"
                  "b = arrayVector([1],a);b");

    re = conn.run("[datetime(1)]");
    ASSERT_EQ(v1->get(0)->getType(), re->getType());
    ASSERT_EQ(v1->get(0)->getRawType(), re->getRawType());

    v1 = conn.run("a=[timestamp(1)];"
                  "b = arrayVector([1],a);b");

    re = conn.run("[timestamp(1)]");
    ASSERT_EQ(v1->get(0)->getType(), re->getType());
    ASSERT_EQ(v1->get(0)->getRawType(), re->getRawType());

    v1 = conn.run("a=[nanotime(1)];"
                  "b = arrayVector([1],a);b");

    re = conn.run("[nanotime(1)]");
    ASSERT_EQ(v1->get(0)->getType(), re->getType());
    ASSERT_EQ(v1->get(0)->getRawType(), re->getRawType());

    v1 = conn.run("a=[month(1)];"
                  "b = arrayVector([1],a);b");

    re = conn.run("[month(1)]");
    ASSERT_EQ(v1->get(0)->getType(), re->getType());
    ASSERT_EQ(v1->get(0)->getRawType(), re->getRawType());

    v1 = conn.run("a=[time(1)];"
                  "b = arrayVector([1],a);b");

    re = conn.run("[time(1)]");
    ASSERT_EQ(v1->get(0)->getType(), re->getType());
    ASSERT_EQ(v1->get(0)->getRawType(), re->getRawType());
}

TEST_F(ArrayVectorTest, testGetIndexAndDataArrayVector)
{
    dolphindb::VectorSP v1;
    dolphindb::ConstantSP data_re;
    dolphindb::ConstantSP index_re;
    dolphindb::INDEX *index_array;
    // int
    v1 = conn.run("a=int(1..262144);"
                  "b = arrayVector(take(5 3 2 6, 65536).cumsum().int(),a);b");

    data_re = conn.run("a");
    index_re = conn.run("take(5 3 2 6, 65536).cumsum().int()");
    int *data_int_array = (int *)v1->getDataArray();
    index_array = v1->getIndexArray();
    for (int i = 0; i < data_re->size(); ++i)
    {
        ASSERT_EQ(*(data_int_array + i), data_re->getInt(i));
    }
    for (int i = 0; i < index_re->size(); ++i)
    {
        ASSERT_EQ(*(index_array + i), index_re->getInt(i));
    }

    // double
    v1 = conn.run("a=double(1..262144);"
                  "b = arrayVector(take(5 3 2 6, 65536).cumsum().int(),a);b");

    data_re = conn.run("a");
    index_re = conn.run("take(5 3 2 6, 65536).cumsum().int()");
    double *data_double_array = (double *)v1->getDataArray();
    index_array = v1->getIndexArray();
    for (int i = 0; i < data_re->size(); ++i)
    {
        ASSERT_EQ(*(data_double_array + i), data_re->getDouble(i));
    }
    for (int i = 0; i < index_re->size(); ++i)
    {
        ASSERT_EQ(*(index_array + i), index_re->getInt(i));
    }

    // datehour
    v1 = conn.run("a=datehour(1..262144);"
                  "b = arrayVector(take(5 3 2 6, 65536).cumsum().int(),a);b");

    data_re = conn.run("a");
    index_re = conn.run("take(5 3 2 6, 65536).cumsum().int()");
    int *data_datehour_array = (int *)v1->getDataArray();
    index_array = v1->getIndexArray();
    for (int i = 0; i < data_re->size(); ++i)
    {
        ASSERT_EQ(*(data_datehour_array + i), data_re->getInt(i));
    }
    for (int i = 0; i < index_re->size(); ++i)
    {
        ASSERT_EQ(*(index_array + i), index_re->getInt(i));
    }

    // data
    v1 = conn.run("a=date(1..262144);"
                  "b = arrayVector(take(5 3 2 6, 65536).cumsum().int(),a);b");

    data_re = conn.run("a");
    index_re = conn.run("take(5 3 2 6, 65536).cumsum().int()");
    int *data_date_array = (int *)v1->getDataArray();
    index_array = v1->getIndexArray();
    for (int i = 0; i < data_re->size(); ++i)
    {
        ASSERT_EQ(*(data_date_array + i), data_re->getInt(i));
    }
    for (int i = 0; i < index_re->size(); ++i)
    {
        ASSERT_EQ(*(index_array + i), index_re->getInt(i));
    }

    // datetime
    v1 = conn.run("a=datetime(1..262144);"
                  "b = arrayVector(take(5 3 2 6, 65536).cumsum().int(),a);b");

    data_re = conn.run("a");
    index_re = conn.run("take(5 3 2 6, 65536).cumsum().int()");
    int *data_datetime_array = (int *)v1->getDataArray();
    index_array = v1->getIndexArray();
    for (int i = 0; i < data_re->size(); ++i)
    {
        ASSERT_EQ(*(data_datetime_array + i), data_re->getInt(i));
    }
    for (int i = 0; i < index_re->size(); ++i)
    {
        ASSERT_EQ(*(index_array + i), index_re->getInt(i));
    }

    // TimeStamp
    v1 = conn.run("a=timestamp(2009.10.12T00:00:00.000+1..262144);"
                  "b = arrayVector(take(5 3 2 6, 65536).cumsum().int(),a);b");

    data_re = conn.run("a");
    index_re = conn.run("take(5 3 2 6, 65536).cumsum().int()");
    long long *data_timestamp_array = (long long *)v1->getDataArray();
    index_array = v1->getIndexArray();
    for (int i = 0; i < data_re->size(); ++i)
    {
        ASSERT_EQ(*(data_timestamp_array + i), data_re->getLong(i));
    }
    for (int i = 0; i < index_re->size(); ++i)
    {
        ASSERT_EQ(*(index_array + i), index_re->getInt(i));
    }

    // nanotime
    v1 = conn.run("a=nanotime(13:30:10.008007007+1..262144);"
                  "b = arrayVector(take(5 3 2 6, 65536).cumsum().int(),a);b");

    data_re = conn.run("a");
    index_re = conn.run("take(5 3 2 6, 65536).cumsum().int()");
    long long *data_nanotime_array = (long long *)v1->getDataArray();
    index_array = v1->getIndexArray();
    for (int i = 0; i < data_re->size(); ++i)
    {
        ASSERT_EQ(*(data_nanotime_array + i), data_re->getLong(i));
    }
    for (int i = 0; i < index_re->size(); ++i)
    {
        ASSERT_EQ(*(index_array + i), index_re->getInt(i));
    }

    // nanotimestamp
    v1 = conn.run("a=nanotimestamp(2012.06.13T13:30:10.008007006+1..262144);"
                  "b = arrayVector(take(5 3 2 6, 65536).cumsum().int(),a);b");

    data_re = conn.run("a");
    index_re = conn.run("take(5 3 2 6, 65536).cumsum().int()");
    long long *data_nanotimestamp_array = (long long *)v1->getDataArray();
    index_array = v1->getIndexArray();
    for (int i = 0; i < data_re->size(); ++i)
    {
        ASSERT_EQ(*(data_nanotimestamp_array + i), data_re->getLong(i));
    }
    for (int i = 0; i < index_re->size(); ++i)
    {
        ASSERT_EQ(*(index_array + i), index_re->getInt(i));
    }

    // month
    v1 = conn.run("a=month(2012.06.13T13:30:10.008007006+1..262144);"
                  "b = arrayVector(take(5 3 2 6, 65536).cumsum().int(),a);b");

    data_re = conn.run("a");
    index_re = conn.run("take(5 3 2 6, 65536).cumsum().int()");
    int *data_month_array = (int *)v1->getDataArray();
    index_array = v1->getIndexArray();
    for (int i = 0; i < data_re->size(); ++i)
    {
        ASSERT_EQ(*(data_month_array + i), data_re->getInt(i));
    }
    for (int i = 0; i < index_re->size(); ++i)
    {
        ASSERT_EQ(*(index_array + i), index_re->getInt(i));
    }

    // time
    v1 = conn.run("a=time(13:30:10.008007006+1..262144);"
                  "b = arrayVector(take(5 3 2 6, 65536).cumsum().int(),a);b");

    data_re = conn.run("a");
    index_re = conn.run("take(5 3 2 6, 65536).cumsum().int()");
    int *data_time_array = (int *)v1->getDataArray();
    index_array = v1->getIndexArray();
    for (int i = 0; i < data_re->size(); ++i)
    {
        ASSERT_EQ(*(data_time_array + i), data_re->getInt(i));
    }
    for (int i = 0; i < index_re->size(); ++i)
    {
        ASSERT_EQ(*(index_array + i), index_re->getInt(i));
    }
}

TEST_F(ArrayVectorTest, testAppendIntArrayVector)
{
    dolphindb::VectorSP v1 = conn.run("a = array(INT[], 0, 1000).append!(arrayVector((1..1000)*50, rand(-100..100 join NULL, 1000*50)));a");
    // constant
    dolphindb::ConstantSP data1 = dolphindb::Util::createInt(100);
    v1->append(data1);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[100]");

    // std::tuple
    dolphindb::ConstantSP data2 = conn.run("t=(1,2,3,4,5);t");
    v1->append(data2);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[5]");

    // array std::vector
    dolphindb::VectorSP data3 = conn.run("arrayVector(take(5 3,2).cumsum().int(), take(1..8,8))");
    v1->append(data3);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[6,7,8]");
}

TEST_F(ArrayVectorTest, testAppendDoubleArrayVector)
{
    dolphindb::VectorSP v1 = conn.run("a = array(DOUBLE[], 0, 1000).append!(arrayVector((1..1000)*50, rand(double(-100..100 join NULL), 1000*50)));a");
    // constant
    dolphindb::ConstantSP data1 = dolphindb::Util::createDouble(100.0);
    v1->append(data1);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[100]");

    // std::tuple
    dolphindb::ConstantSP data2 = conn.run("(34.6 32.4 53.3 12.2 5 43 1.0,)");
    v1->append(data2);
    ASSERT_EQ(v1->get(v1->size() - 2)->getString(), "[34.6,32.4,53.3,12.2,5,43,1]");

    // array std::vector
    dolphindb::VectorSP data3 = conn.run("arrayVector(take(5 3,2).cumsum().int(), double(1..8))");
    v1->append(data3);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[6,7,8]");
}

TEST_F(ArrayVectorTest, testAppendDatehourArrayVector)
{
    dolphindb::VectorSP v1 = conn.run("a = array(DATEHOUR[], 0, 1000).append!(arrayVector((1..1000)*50, rand(datehour(-100..100 join NULL), 1000*50)));a");
    // constant
    dolphindb::ConstantSP data1 = dolphindb::Util::createDateHour(100.4);
    v1->append(data1);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[1970.01.05T04]");

    // std::tuple
    dolphindb::ConstantSP data2 = conn.run("(datehour(34 32 53 12 5 43 11),)");
    v1->append(data2);
    ASSERT_EQ(v1->get(v1->size() - 2)->getString(), "[1970.01.02T10,1970.01.02T08,1970.01.03T05,1970.01.01T12,1970.01.01T05,1970.01.02T19,1970.01.01T11]");

    // array std::vector
    dolphindb::VectorSP data3 = conn.run("arrayVector(take(5 3,2).cumsum().int(), datehour(1..8))");
    v1->append(data3);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[1970.01.01T06,1970.01.01T07,1970.01.01T08]");
}

TEST_F(ArrayVectorTest, testAppendDateArrayVector)
{
    dolphindb::VectorSP v1 = conn.run("a = array(DATE[], 0, 1000).append!(arrayVector((1..1000)*50, rand(date(-100..100 join NULL), 1000*50)));a");
    // constant
    dolphindb::ConstantSP data1 = dolphindb::Util::createDate(100);
    v1->append(data1);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[1970.04.11]");

    // std::tuple
    dolphindb::ConstantSP data2 = conn.run("(date(34 32 53 12 5 43 11),)");
    v1->append(data2);
    ASSERT_EQ(v1->get(v1->size() - 2)->getString(), "[1970.02.04,1970.02.02,1970.02.23,1970.01.13,1970.01.06,1970.02.13,1970.01.12]");

    // array std::vector
    dolphindb::VectorSP data3 = conn.run("arrayVector(take(5 3,2).cumsum().int(), double(1..8))");
    v1->append(data3);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[1970.01.07,1970.01.08,1970.01.09]");
}

TEST_F(ArrayVectorTest, testAppendDatetimeArrayVector)
{
    dolphindb::VectorSP v1 = conn.run("a = array(DATETIME[], 0, 1000).append!(arrayVector((1..1000)*50, rand(datetime(-100..100 join NULL), 1000*50)));a");
    // constant
    dolphindb::ConstantSP data1 = dolphindb::Util::createDateTime(100);
    v1->append(data1);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[" + data1->getString() + "]");

    // std::tuple
    dolphindb::ConstantSP data2 = conn.run("(datetime(34 32 53 12 5 43 11),)");
    v1->append(data2);
    ASSERT_EQ(v1->get(v1->size() - 2)->getString(), data2->getString(0));

    // array std::vector
    dolphindb::VectorSP data3 = conn.run("arrayVector(take(5 3,2).cumsum().int(), datetime(1..8))");
    v1->append(data3);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), data3->get(1)->getString());
}

TEST_F(ArrayVectorTest, testAppendTimestampArrayVector)
{
    dolphindb::VectorSP v1 = conn.run("a = array(TIMESTAMP[], 0, 1000).append!(arrayVector((1..1000)*50, rand(timestamp(-100..100 join NULL), 1000*50)));a");
    // constant
    dolphindb::ConstantSP data1 = dolphindb::Util::createTimestamp(10000000);
    v1->append(data1);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[" + data1->getString() + "]");

    // std::tuple
    dolphindb::ConstantSP data2 = conn.run("(timestamp(34 32 53 12 5 43 11),)");
    v1->append(data2);
    ASSERT_EQ(v1->get(v1->size() - 2)->getString(), data2->getString(0));

    // array std::vector
    dolphindb::VectorSP data3 = conn.run("arrayVector(take(5 3,2).cumsum().int(), timestamp(1..8))");
    v1->append(data3);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), data3->get(1)->getString());
}

TEST_F(ArrayVectorTest, testAppendNanotimeArrayVector)
{
    dolphindb::VectorSP v1 = conn.run("a = array(NANOTIME[], 0, 1000).append!(arrayVector((1..1000)*50, rand(nanotime(-100..100 join NULL), 1000*50)));a");
    // constant
    dolphindb::ConstantSP data1 = dolphindb::Util::createNanoTime(10000000);
    v1->append(data1);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[" + data1->getString() + "]");

    // std::tuple
    dolphindb::ConstantSP data2 = conn.run("(nanotime(34 32 53 12 5 43 11),)");
    v1->append(data2);
    ASSERT_EQ(v1->get(v1->size() - 2)->getString(), data2->getString(0));

    // array std::vector
    dolphindb::VectorSP data3 = conn.run("arrayVector(take(5 3,2).cumsum().int(), nanotime(1..8))");
    v1->append(data3);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), data3->get(1)->getString());
}

TEST_F(ArrayVectorTest, testAppendMonthArrayVector)
{
    dolphindb::VectorSP v1 = conn.run("a = array(MONTH[], 0, 1000).append!(arrayVector((1..1000)*50, rand(month(-100..100 join NULL), 1000*50)));a");
    // constant
    dolphindb::ConstantSP data1 = dolphindb::Util::createMonth(10000);
    v1->append(data1);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[" + data1->getString() + "]");

    // std::tuple
    dolphindb::ConstantSP data2 = conn.run("(month(34 32 53 12 5 43 11),)");
    v1->append(data2);
    ASSERT_EQ(v1->get(v1->size() - 2)->getString(), data2->getString(0));

    // array std::vector
    dolphindb::VectorSP data3 = conn.run("arrayVector(take(5 3,2).cumsum().int(), month(1..8))");
    v1->append(data3);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), data3->get(1)->getString());
}

TEST_F(ArrayVectorTest, testAppendTimeArrayVector)
{
    dolphindb::VectorSP v1 = conn.run("a = array(TIME[], 0, 1000).append!(arrayVector((1..1000)*50, rand(time(-100..100 join NULL), 1000*50)));a");
    // constant
    dolphindb::ConstantSP data1 = dolphindb::Util::createTime(10000);
    v1->append(data1);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), "[" + data1->getString() + "]");

    // std::tuple
    dolphindb::ConstantSP data2 = conn.run("(time(34 32 53 12 5 43 11),)");
    v1->append(data2);
    ASSERT_EQ(v1->get(v1->size() - 2)->getString(), data2->getString(0));

    // array std::vector
    dolphindb::VectorSP data3 = conn.run("arrayVector(take(5 3,2).cumsum().int(), time(1..8))");
    v1->append(data3);
    ASSERT_EQ(v1->get(v1->size() - 1)->getString(), data3->get(1)->getString());
}

TEST_F(ArrayVectorTest, testgetSubVectorArrayVector)
{
    dolphindb::VectorSP v1;
    dolphindb::ConstantSP data_re;
    // int
    v1 = conn.run("a=int(1..262144);"
                  "b = arrayVector(take(5 3 2 6, 65536).cumsum().int(),a);b;");

    data_re = v1->getSubVector(35, 102);
    for (int i = 0; i < 67; ++i)
    {
        ASSERT_EQ(data_re->get(i)->getString(), v1->get(i + 35)->getString());
    }
}

TEST_F(ArrayVectorTest, testErrorIndexArrayVector)
{
    dolphindb::VectorSP value;
    dolphindb::VectorSP index;
    dolphindb::VectorSP data;
    dolphindb::VectorSP res;
    index = dolphindb::Util::createVector(dolphindb::DT_INT, 4, 10);
    value = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 10, 20);
    for (int i = 0; i < 10; ++i)
    {
        value->set(i, dolphindb::Util::createDateTime(i + 1));
    }
    // bigger
    index->set(0, dolphindb::Util::createInt(2));
    index->set(1, dolphindb::Util::createInt(5));
    index->set(2, dolphindb::Util::createInt(8));
    index->set(3, dolphindb::Util::createInt(11));
    ASSERT_ANY_THROW(dolphindb::Util::createArrayVector(index, value));

    // small
    index->set(3, dolphindb::Util::createInt(8));
    ASSERT_ANY_THROW(dolphindb::Util::createArrayVector(index, value));
}

TEST_F(ArrayVectorTest, testCastTemporalDatetimeToDatetime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATETIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_INT));
    av1 = av0->castTemporal(dolphindb::DT_DATETIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[datetime(0), datetime(1),datetime(0), datetime(1),datetime(0), datetime(1),datetime(0), datetime(1),datetime(0), datetime(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATETIME);
}

TEST_F(ArrayVectorTest, testCastTemporalDatetimeToDatehour)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATETIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateTime(j * 3600));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_DATEHOUR_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATEHOUR);
}

TEST_F(ArrayVectorTest, testCastTemporalDatetimeToNanotime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATETIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotime(0), nanotime(1000000000),nanotime(0), nanotime(1000000000),nanotime(0), nanotime(1000000000),nanotime(0), nanotime(1000000000),nanotime(0), nanotime(1000000000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIME);
}

TEST_F(ArrayVectorTest, testCastTemporalDatetimeToNanotimestamp)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATETIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIMESTAMP_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotimestamp(0), nanotimestamp(1000000000),nanotimestamp(0), nanotimestamp(1000000000),nanotimestamp(0), nanotimestamp(1000000000),nanotimestamp(0), nanotimestamp(1000000000),nanotimestamp(0), nanotimestamp(1000000000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIMESTAMP);
}

TEST_F(ArrayVectorTest, testCastTemporalDatetimeToDate)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATETIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateTime(j * 100000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_DATE_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[date(0), date(1),date(0), date(1),date(0), date(1),date(0), date(1),date(0), date(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATE);
}

TEST_F(ArrayVectorTest, testCastTemporalDatetimeToMonth)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATETIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateTime(j * 3000000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_MONTH_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[month(23640), month(23641),month(23640), month(23641),month(23640), month(23641),month(23640), month(23641),month(23640), month(23641)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_MONTH);
}

TEST_F(ArrayVectorTest, testCastTemporalDatetimeToSecond)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATETIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_SECOND_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_SECOND);
}

TEST_F(ArrayVectorTest, testCastTemporalDatetimeToInt)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATETIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateTime(j * 100000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_INT_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalDatetimeToString)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATETIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateTime(j * 100000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_STRING_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalDatehourToDatehour)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP datehour_av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateHour(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        datehour_av1->append(anyv1);
    }
    av1 = datehour_av1->castTemporal(dolphindb::DT_DATEHOUR_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATEHOUR);
}

TEST_F(ArrayVectorTest, testCastTemporalDatehourToDate)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP datehour_av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateHour(j * 24));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        datehour_av1->append(anyv1);
    }
    av1 = datehour_av1->castTemporal(dolphindb::DT_DATE_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[date(0), date(1),date(0), date(1),date(0), date(1),date(0), date(1),date(0), date(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATE);
}

TEST_F(ArrayVectorTest, testCastTemporalDatehourToDatetime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP datehour_av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateHour(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        datehour_av1->append(anyv1);
    }
    av1 = datehour_av1->castTemporal(dolphindb::DT_DATETIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[datetime(0), datetime(3600),datetime(0), datetime(3600),datetime(0), datetime(3600),datetime(0), datetime(3600),datetime(0), datetime(3600)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATETIME);
}

TEST_F(ArrayVectorTest, testCastTemporalDatehourToNanotime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateHour(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotime(0), nanotime(3600000000000),nanotime(0), nanotime(3600000000000),nanotime(0), \
                    nanotime(3600000000000),nanotime(0), nanotime(3600000000000),nanotime(0), nanotime(3600000000000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIME);
}

TEST_F(ArrayVectorTest, testCastTemporalDatehourToNanotimestamp)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateHour(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIMESTAMP_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotimestamp(0), nanotimestamp(3600000000000),nanotimestamp(0), nanotimestamp(3600000000000),\
                    nanotimestamp(0), nanotimestamp(3600000000000),nanotimestamp(0), nanotimestamp(3600000000000),nanotimestamp(0), nanotimestamp(3600000000000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIMESTAMP);
}

TEST_F(ArrayVectorTest, testCastTemporalDatehourToMonth)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateHour(j * 800));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_MONTH_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[month(23640), month(23641),month(23640), month(23641),month(23640), \
                    month(23641),month(23640), month(23641),month(23640), month(23641)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_MONTH);
}

TEST_F(ArrayVectorTest, testCastTemporalDatehourToSecond)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateHour(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_SECOND_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[second(0), second(3600),second(0), second(3600),second(0), second(3600),second(0), second(3600),second(0), second(3600)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_SECOND);
}

TEST_F(ArrayVectorTest, testCastTemporalDatehourToInt)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateHour(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_INT_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalDatehourToString)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATEHOUR_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDateHour(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_STRING_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalDateToDate)
{
    dolphindb::VectorSP av0;
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDate(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av1->append(anyv1);
    }
    av0 = av1->castTemporal(dolphindb::DT_DATE_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[date(0), date(1),date(0), date(1),date(0), date(1),date(0), date(1),date(0), date(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av0->getString(), av1_ex->getString());
    ASSERT_EQ(av0->getType(), av1_ex->getType());
    ASSERT_EQ(av0->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av0->get(0)->get(0)->getType(), dolphindb::DT_DATE);
}

TEST_F(ArrayVectorTest, testCastTemporalDateToDatehour)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP datehour_av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDate(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        datehour_av1->append(anyv1);
    }
    av1 = datehour_av1->castTemporal(dolphindb::DT_DATEHOUR_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[datehour(0), datehour(24),datehour(0), datehour(24),datehour(0), datehour(24),datehour(0), datehour(24),datehour(0), datehour(24)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATEHOUR);
}

TEST_F(ArrayVectorTest, testCastTemporalDateToDatetime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP datehour_av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDate(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        datehour_av1->append(anyv1);
    }
    av1 = datehour_av1->castTemporal(dolphindb::DT_DATETIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[datetime(0), datetime(86400),datetime(0), datetime(86400),datetime(0), datetime(86400),datetime(0), datetime(86400),datetime(0), datetime(86400)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATETIME);
}

TEST_F(ArrayVectorTest, testCastTemporalDateToNanotime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDate(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_NANOTIME_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalDateToTimestamp)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDate(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_TIMESTAMP_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[timestamp(0), timestamp(86400000),timestamp(0), timestamp(86400000),\
                    timestamp(0), timestamp(86400000),timestamp(0), timestamp(86400000),timestamp(0), timestamp(86400000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_TIMESTAMP);
}

TEST_F(ArrayVectorTest, testCastTemporalDateToNanotimestamp)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDate(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIMESTAMP_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotimestamp(0), nanotimestamp(86400000000000),nanotimestamp(0), nanotimestamp(86400000000000),\
                    nanotimestamp(0), nanotimestamp(86400000000000),nanotimestamp(0), nanotimestamp(86400000000000),nanotimestamp(0), nanotimestamp(86400000000000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIMESTAMP);
}

TEST_F(ArrayVectorTest, testCastTemporalDateToSecond)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDate(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_SECOND_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalDateToInt)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDate(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_INT_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalDateToString)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_DATE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_DATE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createDate(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_STRING_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalTimeToTime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_TIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[time(0), time(1),time(0), time(1),time(0), time(1),time(0), time(1),time(0), time(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_TIME);
}

TEST_F(ArrayVectorTest, testCastTemporalTimeToDatetime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_DATETIME_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalTimeToNanotime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotime(0), nanotime(1000000),nanotime(0), nanotime(1000000),nanotime(0), nanotime(1000000),nanotime(0), nanotime(1000000),nanotime(0), nanotime(1000000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIME);
}

TEST_F(ArrayVectorTest, testCastTemporalTimeToSecond)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTime(j * 1000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_SECOND_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_SECOND);
}

TEST_F(ArrayVectorTest, testCastTemporalTimeToMinute)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTime(j * 60000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_MINUTE_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[minute(0), minute(1),minute(0), minute(1),minute(0), minute(1),minute(0), minute(1),minute(0), minute(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_MINUTE);
}

TEST_F(ArrayVectorTest, testCastTemporalTimeToInt)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_INT_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalTimeToString)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_STRING_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalMinuteToTime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_MINUTE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_MINUTE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createMinute(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_TIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[time(0), time(60000),time(0), time(60000),time(0), time(60000),time(0), time(60000),time(0), time(60000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_TIME);
}

TEST_F(ArrayVectorTest, testCastTemporalMinuteToDatetime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_MINUTE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_MINUTE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createMinute(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_DATETIME_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalMinuteToNanotime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_MINUTE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_MINUTE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createMinute(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotime(0), nanotime(60000000000),nanotime(0), nanotime(60000000000),\
                    nanotime(0), nanotime(60000000000),nanotime(0), nanotime(60000000000),nanotime(0), nanotime(60000000000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIME);
}

TEST_F(ArrayVectorTest, testCastTemporalMinuteToSecond)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_MINUTE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_MINUTE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createMinute(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_SECOND_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[second(0), second(60),second(0), second(60),second(0), second(60),second(0), second(60),second(0), second(60)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_SECOND);
}

TEST_F(ArrayVectorTest, testCastTemporalMinuteToMinute)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_MINUTE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_MINUTE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createMinute(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_MINUTE_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[minute(0), minute(1),minute(0), minute(1),minute(0), minute(1),minute(0), minute(1),minute(0), minute(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_MINUTE);
}

TEST_F(ArrayVectorTest, testCastTemporalMinuteToInt)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_MINUTE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_MINUTE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createMinute(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_INT_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalMinuteToString)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_MINUTE_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_MINUTE, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createMinute(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_STRING_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalSecondToTime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_SECOND, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createSecond(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_TIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[time(0), time(1000),time(0), time(1000),time(0), time(1000),time(0), time(1000),time(0), time(1000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_TIME);
}

TEST_F(ArrayVectorTest, testCastTemporalSecondToDatetime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_SECOND, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createSecond(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_DATETIME_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalSecondToNanotime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_SECOND, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createSecond(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotime(0), nanotime(1000000000),nanotime(0), nanotime(1000000000),\
                    nanotime(0), nanotime(1000000000),nanotime(0), nanotime(1000000000),nanotime(0), nanotime(1000000000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIME);
}

TEST_F(ArrayVectorTest, testCastTemporalSecondToSecond)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_SECOND, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createSecond(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_SECOND_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_SECOND);
}

TEST_F(ArrayVectorTest, testCastTemporalSecondToMinute)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_SECOND, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createSecond(j * 60));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_MINUTE_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[minute(0), minute(1),minute(0), minute(1),minute(0), minute(1),minute(0), minute(1),minute(0), minute(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_MINUTE);
}

TEST_F(ArrayVectorTest, testCastTemporalSecondToInt)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_SECOND, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createSecond(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_INT_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalSecondToString)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_SECOND, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createSecond(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_STRING_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimeToNanotime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createSecond(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotime(0), nanotime(1),nanotime(0), nanotime(1),nanotime(0), nanotime(1),nanotime(0), nanotime(1),nanotime(0), nanotime(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIME);
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimeToTime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTime(j * 1000000000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_TIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[time(0), time(1000),time(0), time(1000),time(0), time(1000),time(0), time(1000),time(0), time(1000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_TIME);
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimeToDatetime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_DATETIME_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimeToSecond)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTime(j * 1000000000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_SECOND_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_SECOND);
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimeToMinute)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTime(j * (long long)60000000000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_MINUTE_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[minute(0), minute(1),minute(0), minute(1),minute(0), minute(1),minute(0), minute(1),minute(0), minute(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_MINUTE);
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimeToInt)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_INT_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimeToString)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIME_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTime(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_STRING_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalTimestampToTime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTimestamp(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_TIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[time(0), time(1),time(0), time(1),time(0), time(1),time(0), time(1),time(0), time(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_TIME);
}

TEST_F(ArrayVectorTest, testCastTemporalTimestampToDatetime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTimestamp(j * 1000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_DATETIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[datetime(0), datetime(1),datetime(0), datetime(1),datetime(0), datetime(1),datetime(0), datetime(1),datetime(0), datetime(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATETIME);
}

TEST_F(ArrayVectorTest, testCastTemporalTimestampToDatehour)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTimestamp(j * 3600000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_DATEHOUR_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATEHOUR);
}

TEST_F(ArrayVectorTest, testCastTemporalTimestampToNanotime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTimestamp(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotime(0), nanotime(1000000),nanotime(0), nanotime(1000000),nanotime(0), nanotime(1000000),nanotime(0), nanotime(1000000),nanotime(0), nanotime(1000000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIME);
}

TEST_F(ArrayVectorTest, testCastTemporalTimestampToNanotimestamp)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTimestamp(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIMESTAMP_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotimestamp(0), nanotimestamp(1000000),nanotimestamp(0), nanotimestamp(1000000),\
                    nanotimestamp(0), nanotimestamp(1000000),nanotimestamp(0), nanotimestamp(1000000),nanotimestamp(0), nanotimestamp(1000000)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIMESTAMP);
}

TEST_F(ArrayVectorTest, testCastTemporalTimestampToDate)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTimestamp(j * 86400000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_DATE_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[date(0), date(1),date(0), date(1),date(0), date(1),date(0), date(1),date(0), date(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATE);
}

TEST_F(ArrayVectorTest, testCastTemporalTimestampToMonth)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTimestamp(j * 3000000000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_MONTH_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[month(23640), month(23641),month(23640), month(23641),month(23640), month(23641),month(23640), month(23641),month(23640), month(23641)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_MONTH);
}

TEST_F(ArrayVectorTest, testCastTemporalTimestampToSecond)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTimestamp(j * 1000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_SECOND_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_SECOND);
}

TEST_F(ArrayVectorTest, testCastTemporalTimestampToInt)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTimestamp(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_INT_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalTimestampToString)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_TIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createTimestamp(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_STRING_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimestampToDatetime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTimestamp(j * 1000000000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_DATETIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[datetime(0), datetime(1),datetime(0), datetime(1),datetime(0), datetime(1),datetime(0), datetime(1),datetime(0), datetime(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATETIME);
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimestampToDatehour)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTimestamp(j * 3600000000000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_DATEHOUR_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1),datehour(0), datehour(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATEHOUR);
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimestampToNanotime)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTimestamp(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIME_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotime(0), nanotime(1),nanotime(0), nanotime(1),nanotime(0), nanotime(1),nanotime(0), nanotime(1),nanotime(0), nanotime(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIME);
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimestampToNanotimestamp)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTimestamp(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_NANOTIMESTAMP_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[nanotimestamp(0), nanotimestamp(1),nanotimestamp(0), nanotimestamp(1),\
                    nanotimestamp(0), nanotimestamp(1),nanotimestamp(0), nanotimestamp(1),nanotimestamp(0), nanotimestamp(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_NANOTIMESTAMP);
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimestampToDate)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTimestamp(j * (long long)86400000000000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_DATE_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[date(0), date(1),date(0), date(1),date(0), date(1),date(0), date(1),date(0), date(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_DATE);
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimestampToMonth)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTimestamp(j * (long long)3000000000000000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_MONTH_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[month(23640), month(23641),month(23640), month(23641),month(23640), month(23641),month(23640), month(23641),month(23640), month(23641)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_MONTH);
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimestampToSecond)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTimestamp(j * 1000000000));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    av1 = av0->castTemporal(dolphindb::DT_SECOND_ARRAY);

    std::string script1 = "index=2 4 6 8 10;\
                    v=[second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1),second(0), second(1)];\
                    arrayVector(index,v)";
    dolphindb::VectorSP av1_ex = conn.run(script1);
    ASSERT_EQ(av1->getString(), av1_ex->getString());
    ASSERT_EQ(av1->getType(), av1_ex->getType());
    ASSERT_EQ(av1->get(0)->getForm(), dolphindb::DF_VECTOR);
    ASSERT_EQ(av1->get(0)->get(0)->getType(), dolphindb::DT_SECOND);
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimestampToInt)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTimestamp(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_INT_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalNanotimestampToString)
{
    dolphindb::VectorSP av1;
    dolphindb::VectorSP av0 = dolphindb::Util::createArrayVector(dolphindb::DT_NANOTIMESTAMP_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createNanoTimestamp(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av0->append(anyv1);
    }
    ASSERT_ANY_THROW(av1 = av0->castTemporal(dolphindb::DT_STRING_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalMonthToDate)
{
    dolphindb::VectorSP av0;
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_MONTH_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_MONTH, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createMonth(j / 30));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av1->append(anyv1);
    }
    ASSERT_ANY_THROW(av0 = av1->castTemporal(dolphindb::DT_DATE_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalIntToDate)
{
    dolphindb::VectorSP av0;
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_INT_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_INT, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createInt(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av1->append(anyv1);
    }
    ASSERT_ANY_THROW(av0 = av1->castTemporal(dolphindb::DT_DATE_ARRAY));
}

TEST_F(ArrayVectorTest, testCastTemporalSecondToDate)
{
    dolphindb::VectorSP av0;
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_SECOND_ARRAY, 0, 10);
    for (int i = 0; i < 5; i++)
    {
        dolphindb::VectorSP time = dolphindb::Util::createVector(dolphindb::DT_SECOND, 2);
        for (int j = 0; j < 2; j++)
        {
            time->set(j, dolphindb::Util::createSecond(j * 1));
        }
        dolphindb::VectorSP anyv1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 1);
        anyv1->set(0, time);
        av1->append(anyv1);
    }
    ASSERT_ANY_THROW(av0 = av1->castTemporal(dolphindb::DT_DATE_ARRAY));
}

TEST_F(ArrayVectorTest, testDecimal32ArrayVector)
{
    int scale = 2;
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DECIMAL32_ARRAY, 0, 4, true, scale);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "DECIMAL32[]");
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, 0, 8, true, scale);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, 0, 1, true, scale);
    dolphindb::VectorSP v3 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, 0, 1, true, scale);
    dolphindb::VectorSP v4 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, 0, 4, true, scale);
    dolphindb::ConstantSP val1 = dolphindb::Util::createDecimal32(scale, 4);
    dolphindb::ConstantSP val2 = dolphindb::Util::createDecimal32(scale, 1336432.032);
    dolphindb::ConstantSP val3 = dolphindb::Util::createDecimal32(scale, 0.3546299566546123);
    dolphindb::ConstantSP val4 = dolphindb::Util::createDecimal32(scale, -1336432.032);
    dolphindb::ConstantSP val5 = dolphindb::Util::createDecimal32(scale, -0.3546299566546123);
    dolphindb::ConstantSP val6 = dolphindb::Util::createDecimal32(scale, 3);
    dolphindb::ConstantSP val_null = dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL32, scale);
    dolphindb::VectorSP vec_null1 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, 0, 1, true, scale);
    vec_null1->append(val_null);
    dolphindb::VectorSP vec_null2 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, 1, 1, true, scale);
    vec_null2->setNull(0);
    // v1 -> [4, 1336432.032, NULL, 0.3546299566546123, NULL, -1336432.032, -0.3546299566546123, 3]
    v1->append(val1);
    v1->append(val2);
    v1->append(val_null);
    v1->append(val3);
    v1->append(val_null);
    v1->append(val4);
    v1->append(val5);
    v1->append(val6);
    // v2 -> []
    v2 = vec_null1;
    // v3 -> []
    v3 = vec_null2;
    // v4 -> [1336432.032, 2, 0.3546299566546123, -1336432.032]
    v4->append(val2);
    v4->append(val1);
    v4->append(val3);
    v4->append(val4);

    av1->append(v1);
    av1->append(v2);
    av1->append(v3);
    av1->append(v4);
    conn.upload("av1", av1);
    dolphindb::ConstantSP res = conn.run("sca = " + std::to_string(scale) + ";res=array(BOOL);go;\
                                ex = array(DECIMAL32(sca)[]).append!([[4, 1336432.032, NULL, 0.3546299566546123, NULL, -1336432.032, -0.3546299566546123, 3], [], [00i], [1336432.032, 4, 0.3546299566546123, -1336432.032]]);\
                                for(i in 0:(ex.size())){res.append!(each(eqFloat, ex[i],av1[i]))};res");
    ASSERT_EQ(res->getString(), "[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]");
}

TEST_F(ArrayVectorTest, testDecimal32ArrayVector_null)
{
    srand(time(NULL));
    for (auto i = 0; i < 1000; i++)
    {
        int scale = rand() % 10;
        dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DECIMAL32_ARRAY, 0, 10, true, scale);
        dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, 0, 2, true, scale);
        dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, 0, 2, true, scale);
        dolphindb::VectorSP v3 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, 0, 2, true, scale);
        dolphindb::ConstantSP null_val = dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL32, scale);
        v1->append(null_val);
        v2->append(null_val);
        v2->append(null_val);
        v3->append(null_val);
        av1->append(v1);
        av1->append(v2);
        av1->append(v3);
        conn.upload("av1", av1);
        dolphindb::ConstantSP res = conn.run("sca = " + std::to_string(scale) + ";go;\
                                    m = array(DECIMAL32(sca)[], 0, 10).append!(decimal32([NULL, take(decimal32(NULL, sca),2), []],sca));\
                                    eqObj(m,av1)");
        ASSERT_TRUE(res->getBool());
        dolphindb::ConstantSP ex = conn.run("m");
        ASSERT_EQ(ex->getString(), av1->getString());
    }
}

TEST_F(ArrayVectorTest, testDecimal64ArrayVector)
{
    int scale = 16;
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DECIMAL64_ARRAY, 0, 4, true, scale);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "DECIMAL64[]");
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, 0, 8, true, scale);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, 0, 1, true, scale);
    dolphindb::VectorSP v3 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, 0, 1, true, scale);
    dolphindb::VectorSP v4 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, 0, 4, true, scale);
    dolphindb::ConstantSP val1 = dolphindb::Util::createDecimal64(scale, 4.0);
    dolphindb::ConstantSP val2 = dolphindb::Util::createDecimal64(scale, 1.3);
    dolphindb::ConstantSP val3 = dolphindb::Util::createDecimal64(scale, 0.3546299566546123);
    dolphindb::ConstantSP val4 = dolphindb::Util::createDecimal64(scale, -1.3);
    dolphindb::ConstantSP val5 = dolphindb::Util::createDecimal64(scale, -0.3546299566546123);
    dolphindb::ConstantSP val6 = dolphindb::Util::createDecimal64(scale, 3.0);
    dolphindb::ConstantSP val_null = dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL64, scale);
    dolphindb::VectorSP vec_null1 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, 0, 1, true, scale);
    vec_null1->append(val_null);
    dolphindb::VectorSP vec_null2 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, 1, 1, true, scale);
    vec_null2->setNull(0);
    // v1 -> [4, 1.3, NULL, 0.3546299566546123, NULL, -1.3, -0.3546299566546123, 3]
    v1->append(val1);
    v1->append(val2);
    v1->append(val_null);
    v1->append(val3);
    v1->append(val_null);
    v1->append(val4);
    v1->append(val5);
    v1->append(val6);
    // v2 -> []
    v2 = vec_null1;
    // v3 -> []
    v3 = vec_null2;
    // v4 -> [1.3, 2, 0.3546299566546123, -1.3]
    v4->append(val2);
    v4->append(val1);
    v4->append(val3);
    v4->append(val4);

    av1->append(v1);
    av1->append(v2);
    av1->append(v3);
    av1->append(v4);
    conn.upload("av1", av1);
    dolphindb::ConstantSP res = conn.run("sca = " + std::to_string(scale) + ";res=array(BOOL);go;\
                                ex = array(DECIMAL64(sca)[], 0, 4).append!([[4, 1.3, NULL, 0.3546299566546123, NULL, -1.3, -0.3546299566546123, 3], [], [00i], [1.3, 4, 0.3546299566546123, -1.3]]);\
                                for(i in 0:(ex.size())){res.append!(each(eqFloat, ex[i],av1[i]))};res");
    ASSERT_EQ(res->getString(), "[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]");
}

TEST_F(ArrayVectorTest, testDecimal64ArrayVector_null)
{
    srand(time(NULL));
    for (auto i = 0; i < 1000; i++)
    {
        int scale = rand() % 10 + 6;
        dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DECIMAL64_ARRAY, 0, 10, true, scale);
        dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, 0, 2, true, scale);
        dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, 0, 2, true, scale);
        dolphindb::VectorSP v3 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, 0, 2, true, scale);
        dolphindb::ConstantSP null_val = dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL64, scale);
        v1->append(null_val);
        v2->append(null_val);
        v2->append(null_val);
        v3->append(null_val);
        av1->append(v1);
        av1->append(v2);
        av1->append(v3);
        conn.upload("av1", av1);
        dolphindb::ConstantSP res = conn.run("sca = " + std::to_string(scale) + ";go;\
                                    m = array(DECIMAL64(sca)[], 0, 10).append!(decimal64([NULL, take(decimal64(NULL, sca),2), []],sca));\
                                    eqObj(m,av1)");
        ASSERT_TRUE(res->getBool());
        dolphindb::ConstantSP ex = conn.run("m");
        ASSERT_EQ(ex->getString(), av1->getString());
    }
}

TEST_F(ArrayVectorTest, testDecimal128ArrayVector)
{
    int scale = 26;
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DECIMAL128_ARRAY, 0, 4, true, scale);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(av1->getType()), "DECIMAL128[]");
    dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, 0, 8, true, scale);
    dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, 0, 1, true, scale);
    dolphindb::VectorSP v3 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, 0, 1, true, scale);
    dolphindb::VectorSP v4 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, 0, 4, true, scale);
    dolphindb::ConstantSP val1 = dolphindb::Util::createDecimal128(scale, 4);
    dolphindb::ConstantSP val2 = dolphindb::Util::createDecimal128(scale, 456789.2695);
    dolphindb::ConstantSP val3 = dolphindb::Util::createDecimal128(scale, 0);
    val3->setString("1.666666666666666666666666666666");
    dolphindb::ConstantSP val4 = dolphindb::Util::createDecimal128(scale, -456789.2695);
    dolphindb::ConstantSP val5 = dolphindb::Util::createDecimal128(scale, 0);
    val5->setString("-1.666666666666666666666666666666");
    dolphindb::ConstantSP val6 = dolphindb::Util::createDecimal128(scale, 3);
    dolphindb::ConstantSP val_null = dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL128, scale);
    dolphindb::VectorSP vec_null1 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, 0, 1, true, scale);
    vec_null1->append(val_null);
    dolphindb::VectorSP vec_null2 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, 1, 1, true, scale);
    vec_null2->setNull(0);
    // v1 -> [4, 456789.2695, NULL, 1.666666666666666666666666666666, NULL, -456789.2695, -1.666666666666666666666666666666, 3]
    v1->append(val1);
    v1->append(val2);
    v1->append(val_null);
    v1->append(val3);
    v1->append(val_null);
    v1->append(val4);
    v1->append(val5);
    v1->append(val6);
    // v2 -> []
    v2 = vec_null1;
    // v3 -> []
    v3 = vec_null2;
    // v4 -> [456789.2695, 2, 1.666666666666666666666666666666, -456789.2695]
    v4->append(val2);
    v4->append(val1);
    v4->append(dolphindb::Util::createDouble(1.666666666666666666666666666666));
    v4->append(val4);

    av1->append(v1);
    av1->append(v2);
    av1->append(v3);
    av1->append(v4);
    conn.upload("av1", av1);
    dolphindb::ConstantSP res = conn.run("sca = " + std::to_string(scale) + ";res=array(BOOL);go;\
                                ex = array(DECIMAL128(sca)[], 0, 4).append!([[4, 456789.2695, NULL, decimal128(`1.666666666666666666666666666666,sca), NULL, -456789.2695, decimal128('-1.666666666666666666666666666666',sca), 3], [], [00i], [456789.2695, 4, 1.666666666666666666666666666666, -456789.2695]]);\
                                for(i in 0:(ex.size())){res.append!(each(eqFloat, ex[i],av1[i]))};res");
    ASSERT_EQ(res->getString(), "[1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]");
}

TEST_F(ArrayVectorTest, testDecimal128ArrayVector_null)
{
    srand(time(NULL));
    for (auto i = 0; i < 1000; i++)
    {
        int scale = rand() % 20 + 16;
        dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(dolphindb::DT_DECIMAL128_ARRAY, 0, 10, true, scale);
        dolphindb::VectorSP v1 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, 0, 2, true, scale);
        dolphindb::VectorSP v2 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, 0, 2, true, scale);
        dolphindb::VectorSP v3 = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, 0, 2, true, scale);
        dolphindb::ConstantSP null_val = dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL128, scale);
        v1->append(null_val);
        v2->append(null_val);
        v2->append(null_val);
        v3->append(null_val);
        av1->append(v1);
        av1->append(v2);
        av1->append(v3);
        conn.upload("av1", av1);
        dolphindb::ConstantSP res = conn.run("sca = " + std::to_string(scale) + ";go;\
                                    m = array(DECIMAL128(sca)[], 0, 10).append!(decimal128([NULL, take(decimal128(NULL, sca),2), []],sca));\
                                    eqObj(m,av1)");
        ASSERT_TRUE(res->getBool());
        dolphindb::ConstantSP ex = conn.run("m");
        ASSERT_EQ(ex->getString(), av1->getString());
    }
}

TEST_F(ArrayVectorTest, testDecimal32ArrayVector_gt65535)
{
    dolphindb::VectorSP indV = conn.run("1..35000*2");
    dolphindb::VectorSP valV = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, 70000, 70000, true, 2);
    dolphindb::ConstantSP val1 = dolphindb::Util::createDecimal32(2, 2.35123);
    dolphindb::ConstantSP val2 = dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL32, 2);
    for (auto i = 0; i < valV->size() / 2; i++)
    {
        valV->set(2 * i, val1);
        valV->set(2 * i + 1, val2);
    }

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(indV, valV);
    conn.upload("av1", av1);

    dolphindb::ConstantSP res = conn.run("index = 1..35000*2;val = take([decimal32(2.35123,2),decimal32(NULL,2),decimal32(2.35123,2),decimal32(NULL,2)],70000);\
                                ex=arrayVector(index,val);eqObj(ex,av1)");
    ASSERT_TRUE(res->getBool());

    dolphindb::ConstantSP ex = conn.run("ex");
    ASSERT_EQ(ex->getString(), av1->getString());
}

TEST_F(ArrayVectorTest, testDecimal64ArrayVector_gt65535)
{
    dolphindb::VectorSP indV = conn.run("1..35000*2");
    dolphindb::VectorSP valV = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, 70000, 70000, true, 11);
    dolphindb::ConstantSP val1 = dolphindb::Util::createDecimal64(11, 2.35123);
    dolphindb::ConstantSP val2 = dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL64, 11);
    for (auto i = 0; i < valV->size() / 2; i++)
    {
        valV->set(2 * i, val1);
        valV->set(2 * i + 1, val2);
    }

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(indV, valV);
    conn.upload("av1", av1);

    dolphindb::ConstantSP res = conn.run("index = 1..35000*2;val = take([decimal64(2.35123,11),decimal64(NULL,11),decimal64(2.35123,11),decimal64(NULL,11)],70000);\
                                ex=arrayVector(index,val);eqObj(ex,av1)");
    ASSERT_TRUE(res->getBool());

    dolphindb::ConstantSP ex = conn.run("ex");
    ASSERT_EQ(ex->getString(), av1->getString());
}

TEST_F(ArrayVectorTest, testDecimal128ArrayVector_gt65535)
{
    dolphindb::VectorSP indV = conn.run("1..35000*2");
    dolphindb::VectorSP valV = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, 70000, 70000, true, 26);
    dolphindb::ConstantSP val1 = dolphindb::Util::createConstant(dolphindb::DT_DECIMAL128, 26);
    val1->setString("2.1111111111111111111111111111111111111111111");
    dolphindb::ConstantSP val2 = dolphindb::Util::createNullConstant(dolphindb::DT_DECIMAL128, 26);
    for (auto i = 0; i < valV->size() / 2; i++)
    {
        valV->set(2 * i, val1);
        valV->set(2 * i + 1, val2);
    }

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(indV, valV);
    conn.upload("av1", av1);

    dolphindb::ConstantSP res = conn.run("index = 1..35000*2;val = take([decimal128('2.1111111111111111111111111111111111111111111',26),decimal128(NULL,26),decimal128('2.1111111111111111111111111111111111111111111',26),decimal128(NULL,26)],70000);\
                                ex=arrayVector(index,val);eqObj(ex,av1)");
    ASSERT_TRUE(res->getBool());

    dolphindb::ConstantSP ex = conn.run("ex");
    ASSERT_EQ(ex->getString(), av1->getString());
}

TEST_F(ArrayVectorTest, test_set_null)
{
    dolphindb::VectorSP vec1 = conn.run("x=array(INT[]).append!([1 2 3]).append!([4 5 6]);x");
    dolphindb::VectorSP vec2 = conn.run("x=array(INT[]).append!([7 8 9]).append!([10 11 12]);x");
    ASSERT_FALSE(vec1->hasNull());
    ASSERT_FALSE(vec2->hasNull());

    dolphindb::VectorSP indV = dolphindb::Util::createIndexVector(0, 2);
    dolphindb::VectorSP nullAV = conn.run("u=array(INT[]).append!([take(00i, 3)]).append!([take(00i, 3)]);u");
    dolphindb::VectorSP nullV = conn.run("u=array(ANY).append!(take(00i, 3)).append!(take(00i, 3));u");;

    ASSERT_TRUE(vec1->set(indV, nullAV));
    ASSERT_FALSE(vec1->isNull());
    ASSERT_TRUE(vec1->get(0)->hasNull());
    ASSERT_TRUE(vec1->get(1)->hasNull());
    ASSERT_EQ(vec1->getString(), "[[,,],[,,]]");

    ASSERT_TRUE(vec2->set(indV, nullV));
    ASSERT_FALSE(vec2->isNull());
    ASSERT_TRUE(vec2->get(0)->hasNull());
    ASSERT_TRUE(vec2->get(1)->hasNull());
    ASSERT_EQ(vec2->getString(), "[[,,],[,,]]");
}

TEST_F(ArrayVectorTest, test_get_subArrayVector)
{
    dolphindb::ConstantSP ind = dolphindb::Util::createInt(0);
    dolphindb::VectorSP indv = dolphindb::Util::createIndexVector(0, 2);
    dolphindb::VectorSP indav = conn.run("x=array(INT[]).append!([0 1]);x");
    dolphindb::VectorSP av = conn.run("x=array(INT[]).append!([[1]]).append!([[2]]).append!([[3,4]]).append!([[]]);x");
    ASSERT_ANY_THROW(dynamic_cast<dolphindb::FastArrayVector*>(av.get())->get(0, ind));
    ASSERT_ANY_THROW(dynamic_cast<dolphindb::FastArrayVector*>(av.get())->get(0, indav));
    dolphindb::VectorSP subAV =  dynamic_cast<dolphindb::FastArrayVector*>(av.get())->get(0, indv);
    ASSERT_EQ(subAV->getString(), "[[1],[2]]");
    subAV =  dynamic_cast<dolphindb::FastArrayVector*>(av.get())->get(2, indv);
    ASSERT_EQ(subAV->getString(), "[[3,4],[]]");
}

class ArrayVectorTest_set : public ArrayVectorTest, public ::testing::WithParamInterface<std::tuple<dolphindb::DATA_TYPE, dolphindb::DATA_TYPE, std::string, std::string>>
{
public:
    static const std::vector<std::tuple<dolphindb::DATA_TYPE, dolphindb::DATA_TYPE, std::string, std::string>> get_testData()
    {
        return {
            std::make_tuple(dolphindb::DT_BOOL_ARRAY, dolphindb::DT_BOOL, "x = array(ANY);for(i in 0:100){x.append!(true false)};x", "ex=array(BOOL[]);for(i in 0:50){ex.append!([true false])};for(i in 0:50){ex.append!([[bool(NULL)]])};"),
            std::make_tuple(dolphindb::DT_CHAR_ARRAY, dolphindb::DT_CHAR, "x = array(ANY);for(i in 0:100){x.append!(1c NULL)};x", "ex=array(CHAR[]);for(i in 0:50){ex.append!([1c NULL])};for(i in 0:50){ex.append!([[char(NULL)]])};"),
            std::make_tuple(dolphindb::DT_SHORT_ARRAY, dolphindb::DT_SHORT, "x = array(ANY);for(i in 0:100){x.append!(1h NULL)};x", "ex=array(SHORT[]);for(i in 0:50){ex.append!([1h NULL])};for(i in 0:50){ex.append!([[short(NULL)]])};"),
            std::make_tuple(dolphindb::DT_INT_ARRAY, dolphindb::DT_INT, "x = array(ANY);for(i in 0:100){x.append!(1 NULL)};x", "ex=array(INT[]);for(i in 0:50){ex.append!([1 NULL])};for(i in 0:50){ex.append!([[int(NULL)]])};"),
            std::make_tuple(dolphindb::DT_LONG_ARRAY, dolphindb::DT_LONG, "x = array(ANY);for(i in 0:100){x.append!(1l NULL)};x", "ex=array(LONG[]);for(i in 0:50){ex.append!([1l NULL])};for(i in 0:50){ex.append!([[long(NULL)]])};"),
            std::make_tuple(dolphindb::DT_FLOAT_ARRAY, dolphindb::DT_FLOAT, "x = array(ANY);for(i in 0:100){x.append!(1f NULL)};x", "ex=array(FLOAT[]);for(i in 0:50){ex.append!([1f NULL])};for(i in 0:50){ex.append!([[float(NULL)]])};"),
            std::make_tuple(dolphindb::DT_DOUBLE_ARRAY, dolphindb::DT_DOUBLE, "x = array(ANY);for(i in 0:100){x.append!(1.0 NULL)};x", "ex=array(DOUBLE[]);for(i in 0:50){ex.append!([1.0 NULL])};for(i in 0:50){ex.append!([[double(NULL)]])};"),
            std::make_tuple(dolphindb::DT_DATE_ARRAY, dolphindb::DT_DATE, "x = array(ANY);for(i in 0:100){x.append!(2023.01.01 NULL)};x", "ex=array(DATE[]);for(i in 0:50){ex.append!([2023.01.01 NULL])};for(i in 0:50){ex.append!([[date(NULL)]])};"),
            std::make_tuple(dolphindb::DT_MONTH_ARRAY, dolphindb::DT_MONTH, "x = array(ANY);for(i in 0:100){x.append!(2013.11M NULL)};x", "ex=array(MONTH[]);for(i in 0:50){ex.append!([2013.11M NULL])};for(i in 0:50){ex.append!([[month(NULL)]])};"),
            std::make_tuple(dolphindb::DT_TIME_ARRAY, dolphindb::DT_TIME, "x = array(ANY);for(i in 0:100){x.append!(12:00:00.123 NULL)};x", "ex=array(TIME[]);for(i in 0:50){ex.append!([12:00:00.123 NULL])};for(i in 0:50){ex.append!([[time(NULL)]])};"),
            std::make_tuple(dolphindb::DT_MINUTE_ARRAY, dolphindb::DT_MINUTE, "x = array(ANY);for(i in 0:100){x.append!(13:30m NULL)};x", "ex=array(MINUTE[]);for(i in 0:50){ex.append!([13:30m NULL])};for(i in 0:50){ex.append!([[minute(NULL)]])};"),
            std::make_tuple(dolphindb::DT_SECOND_ARRAY, dolphindb::DT_SECOND, "x = array(ANY);for(i in 0:100){x.append!(13:30:30 NULL)};x", "ex=array(SECOND[]);for(i in 0:50){ex.append!([13:30:30 NULL])};for(i in 0:50){ex.append!([[second(NULL)]])};"),
            std::make_tuple(dolphindb::DT_DATETIME_ARRAY, dolphindb::DT_DATETIME, "x = array(ANY);for(i in 0:100){x.append!(2012.06.13T13:30:10 NULL)};x", "ex=array(DATETIME[]);for(i in 0:50){ex.append!([2012.06.13T13:30:10 NULL])};for(i in 0:50){ex.append!([[datetime(NULL)]])};"),
            std::make_tuple(dolphindb::DT_TIMESTAMP_ARRAY, dolphindb::DT_TIMESTAMP, "x = array(ANY);for(i in 0:100){x.append!(2023.01.01T12:00:00.123 NULL)};x", "ex=array(TIMESTAMP[]);for(i in 0:50){ex.append!([2023.01.01T12:00:00.123 NULL])};for(i in 0:50){ex.append!([[timestamp(NULL)]])};"),
            std::make_tuple(dolphindb::DT_NANOTIME_ARRAY, dolphindb::DT_NANOTIME, "x = array(ANY);for(i in 0:100){x.append!(13:30:10.008007006 NULL)};x", "ex=array(NANOTIME[]);for(i in 0:50){ex.append!([13:30:10.008007006 NULL])};for(i in 0:50){ex.append!([[nanotime(NULL)]])};"),
            std::make_tuple(dolphindb::DT_NANOTIMESTAMP_ARRAY, dolphindb::DT_NANOTIMESTAMP, "x = array(ANY);for(i in 0:100){x.append!(2012.06.13T13:30:10.008007006 NULL)};x", "ex=array(NANOTIMESTAMP[]);for(i in 0:50){ex.append!([2012.06.13T13:30:10.008007006 NULL])};for(i in 0:50){ex.append!([[nanotimestamp(NULL)]])};"),
            std::make_tuple(dolphindb::DT_DATEHOUR_ARRAY, dolphindb::DT_DATEHOUR, "x = array(ANY);for(i in 0:100){x.append!(datehour(10000 NULL))};x", "ex=array(DATEHOUR[]);for(i in 0:50){ex.append!([datehour(10000 NULL)])};for(i in 0:50){ex.append!([[datehour(NULL)]])};"),
            std::make_tuple(dolphindb::DT_INT128_ARRAY, dolphindb::DT_INT128, "x = array(ANY);for(i in 0:100){x.append!(int128(`e1671797c52e15f763380b45e841ec32`))};x", "ex=array(INT128[]);for(i in 0:50){ex.append!([int128(`e1671797c52e15f763380b45e841ec32`)])};for(i in 0:50){ex.append!([[int128()]])};"),
            std::make_tuple(dolphindb::DT_UUID_ARRAY, dolphindb::DT_UUID, "x = array(ANY);for(i in 0:100){x.append!(uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'`))};x", "ex=array(UUID[]);for(i in 0:50){ex.append!([uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'`)])};for(i in 0:50){ex.append!([[uuid()]])};"),
            std::make_tuple(dolphindb::DT_IP_ARRAY, dolphindb::DT_IP, "x = array(ANY);for(i in 0:100){x.append!(ipaddr('1.1.1.1'`))};x", "ex=array(IPADDR[]);for(i in 0:50){ex.append!([ipaddr('1.1.1.1'`)])};for(i in 0:50){ex.append!([[ipaddr()]])};"),
            std::make_tuple(dolphindb::DT_DECIMAL32_ARRAY, dolphindb::DT_DECIMAL32, "x = array(ANY);for(i in 0:100){x.append!(decimal32('-1.453254'`,5))};x", "ex=array(DECIMAL32(5)[]);for(i in 0:50){ex.append!([decimal32('-1.453254'`,5)])};for(i in 0:50){ex.append!([[decimal32(,5)]])};"),
            std::make_tuple(dolphindb::DT_DECIMAL64_ARRAY, dolphindb::DT_DECIMAL64, "x = array(ANY);for(i in 0:100){x.append!(decimal64('-1.453254'`,5))};x", "ex=array(DECIMAL64(5)[]);for(i in 0:50){ex.append!([decimal64('-1.453254'`,5)])};for(i in 0:50){ex.append!([[decimal64(, 5)]])};"),
            std::make_tuple(dolphindb::DT_DECIMAL128_ARRAY, dolphindb::DT_DECIMAL128, "x = array(ANY);for(i in 0:100){x.append!(decimal128('-1.453254'`,5))};x", "ex=array(DECIMAL128(5)[]);for(i in 0:50){ex.append!([decimal128('-1.453254'`,5)])};for(i in 0:50){ex.append!([[decimal128(, 5)]])};"),

        };
    }
};

INSTANTIATE_TEST_SUITE_P(, ArrayVectorTest_set, ::testing::ValuesIn(ArrayVectorTest_set::get_testData()));
TEST_P(ArrayVectorTest_set, testArrayVector_set)
{
    int size = 100;
    dolphindb::DATA_TYPE av_type = std::get<0>(GetParam());
    dolphindb::DATA_TYPE v_type = std::get<1>(GetParam());
    std::string v0_script = std::get<2>(GetParam());
    std::string ex_av1_script = std::get<3>(GetParam());
    int extra = 0;
    if (v_type == dolphindb::DT_DECIMAL32 || v_type == dolphindb::DT_DECIMAL64 || v_type == dolphindb::DT_DECIMAL128)
    {
        extra = 5;
    }
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(av_type, size, size, true, extra);
    dolphindb::VectorSP indexV = dolphindb::Util::createIndexVector(0, size);
    dolphindb::VectorSP v0 = conn.run(v0_script);

    ASSERT_FALSE(av1->set(indexV, v0));
    for (auto i = 0; i < 50; i++)
    {
        ASSERT_TRUE(av1->set(indexV->get(i), v0->get(i)));
    }
    for (auto i = 50; i < 100; i++)
    {
        ASSERT_TRUE(av1->set(indexV->get(i), dolphindb::Util::createNullConstant(v_type, 5)));
    }

    conn.upload("av1", av1);
    conn.run(ex_av1_script);
    dolphindb::ConstantSP res = conn.run("eqObj(ex, av1)");
    ASSERT_TRUE(res->getBool());
}

class ArrayVectorTest_set_av : public ArrayVectorTest, public ::testing::WithParamInterface<std::vector<std::string>>
{
public:
    static const std::vector<std::vector<std::string>> get_testData()
    {
        return {
            {"bool_arrayVector", "x = array(BOOL[]).append!([take(false, 2)]);vals = take(x, 5);vals", "x = array(BOOL[]).append!([take(true, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(true true);data_anyv = take(x, 5);data_anyv"},
            {"char_arrayVector", "x = array(CHAR[]).append!([rand(127c, 2)]);vals = take(x, 5);vals", "x = array(CHAR[]).append!([rand(127c, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(127c, 2));data_anyv = take(x, 5);data_anyv"},
            {"short_arrayVector", "x = array(SHORT[]).append!([rand(32767h, 2)]);vals = take(x, 5);vals", "x = array(SHORT[]).append!([rand(32767h, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(32767h, 2));data_anyv = take(x, 5);data_anyv"},
            {"int_arrayVector", "x = array(INT[]).append!([1..5]);vals = take(x, 5);vals", "x = array(INT[]).append!([6..10]);data_av = take(x, 5);data_av", "x = array(ANY).append!(6..10);data_anyv = take(x, 5);data_anyv"},
            {"long_arrayVector", "x = array(LONG[]).append!([rand(2147483647l, 2)]);vals = take(x, 5);vals", "x = array(LONG[]).append!([rand(2147483647l, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2147483647l, 2));data_anyv = take(x, 5);data_anyv"},
            {"float_arrayVector", "x = array(FLOAT[]).append!([rand(100.00f, 2)]);vals = take(x, 5);vals", "x = array(FLOAT[]).append!([rand(100.00f, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(100.00f, 2));data_anyv = take(x, 5);data_anyv"},
            {"double_arrayVector", "x = array(DOUBLE[]).append!([rand(100.00, 2)]);vals = take(x, 5);vals", "x = array(DOUBLE[]).append!([rand(100.00, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(100.00, 2));data_anyv = take(x, 5);data_anyv"},
            {"date_arrayVector", "x = array(DATE[]).append!([rand(2023.01.01,2)]);vals = take(x, 5);vals", "x = array(DATE[]).append!([rand(2023.01.01,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2023.01.01,2));data_anyv = take(x, 5);data_anyv"},
            {"month_arrayVector", "x = array(MONTH[]).append!([rand(2023.01M,2)]);vals = take(x, 5);vals", "x = array(MONTH[]).append!([rand(2023.01M,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2023.01M,2));data_anyv = take(x, 5);data_anyv"},
            {"time_arrayVector", "x = array(TIME[]).append!([rand(23:00:00.000,2)]);vals = take(x, 5);vals", "x = array(TIME[]).append!([rand(23:00:00.000,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(23:00:00.000,2));data_anyv = take(x, 5);data_anyv"},
            {"minute_arrayVector", "x = array(MINUTE[]).append!([rand(23:00m,2)]);vals = take(x, 5);vals", "x = array(MINUTE[]).append!([rand(23:00m,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(23:00m,2));data_anyv = take(x, 5);data_anyv"},
            {"second_arrayVector", "x = array(SECOND[]).append!([rand(23:00:00,2)]);vals = take(x, 5);vals", "x = array(SECOND[]).append!([rand(23:00:00,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(23:00:00,2));data_anyv = take(x, 5);data_anyv"},
            {"datetime_arrayVector", "x = array(DATETIME[]).append!([rand(2023.01.01T00:00:00,2)]);vals = take(x, 5);vals", "x = array(DATETIME[]).append!([rand(2023.01.01T00:00:00,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2023.01.01T00:00:00,2));data_anyv = take(x, 5);data_anyv"},
            {"timestamp_arrayVector", "x = array(TIMESTAMP[]).append!([rand(2023.01.01T00:00:00.000,2)]);vals = take(x, 5);vals", "x = array(TIMESTAMP[]).append!([rand(2023.01.01T00:00:00.000,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2023.01.01T00:00:00.000,2));data_anyv = take(x, 5);data_anyv"},
            {"nanotime_arrayVector", "x = array(NANOTIME[]).append!([rand(23:00:00.000000000,2)]);vals = take(x, 5);vals", "x = array(NANOTIME[]).append!([rand(23:00:00.000000000,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(23:00:00.000000000,2));data_anyv = take(x, 5);data_anyv"},
            {"nanotimestamp_arrayVector", "x = array(NANOTIMESTAMP[]).append!([rand(2023.01.01T00:00:00.000000000,2)]);vals = take(x, 5);vals", "x = array(NANOTIMESTAMP[]).append!([rand(2023.01.01T00:00:00.000000000,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2023.01.01T00:00:00.000000000,2));data_anyv = take(x, 5);data_anyv"},
            {"int128_arrayVector", "x = array(INT128[]).append!([rand(int128('e1671797c52e15f763380b45e841ec32'),2)]);vals = take(x, 5);vals", "x = array(INT128[]).append!([rand(int128('e1671797c52e15f763380b45e841ec32'),2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(int128('e1671797c52e15f763380b45e841ec32'),2));data_anyv = take(x, 5);data_anyv"},
            {"uuid_arrayVector", "x = array(UUID[]).append!([rand(uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'),2)]);vals = take(x, 5);vals", "x = array(UUID[]).append!([rand(uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'),2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'),2));data_anyv = take(x, 5);data_anyv"},
            {"ipaddr_arrayVector", "x = array(IPADDR[]).append!([rand(ipaddr('255.255.255.0'),2)]);vals = take(x, 5);vals", "x = array(IPADDR[]).append!([rand(ipaddr('255.255.255.0'),2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(ipaddr('255.255.255.0'),2));data_anyv = take(x, 5);data_anyv"},
            {"decimal32_arrayVector", "x = array(DECIMAL32(5)[]).append!([decimal32(rand(100.00,2), 5)]);vals = take(x, 5);vals", "x = array(DECIMAL32(5)[]).append!([decimal32(rand(100.00,2), 5)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(decimal32(rand(100.00,2), 5));data_anyv = take(x, 5);data_anyv"},
            {"decimal64_arrayVector", "x = array(DECIMAL64(10)[]).append!([decimal64(rand(100.00,2), 10)]);vals = take(x, 5);vals", "x = array(DECIMAL64(10)[]).append!([decimal64(rand(100.00,2), 10)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(decimal64(rand(100.00,2), 10));data_anyv = take(x, 5);data_anyv"},
            {"decimal128_arrayVector", "x = array(DECIMAL128(20)[]).append!([decimal128(rand(100.00,2), 20)]);vals = take(x, 5);vals", "x = array(DECIMAL128(20)[]).append!([decimal128(rand(100.00,2), 20)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(decimal128(rand(100.00,2), 20));data_anyv = take(x, 5);data_anyv"},
        };
    }
};

INSTANTIATE_TEST_SUITE_P(, ArrayVectorTest_set_av, testing::ValuesIn(ArrayVectorTest_set_av::get_testData()),
    [](const testing::TestParamInfo<ArrayVectorTest_set_av::ParamType>& info){ 
        return info.param[0]; 
    });
TEST_P(ArrayVectorTest_set_av, set_valueIsArrayVector){
    std::string s1 = GetParam()[1];
    std::string s2 = GetParam()[2];
    std::string s3 = GetParam()[3];
    dolphindb::VectorSP av1 = conn.run(s1);
    dolphindb::VectorSP av2 = av1->getInstance();

    dolphindb::VectorSP indexV = dolphindb::Util::createIndexVector(0, 5);
    dolphindb::VectorSP data_av = conn.run(s2);
    dolphindb::VectorSP data_anyv = conn.run(s3);

    ASSERT_TRUE(av1->set(indexV, data_av));
    ASSERT_TRUE(av2->set(indexV, data_anyv));

    std::vector<std::string> names = {"av1", "av2"};
    std::vector<dolphindb::ConstantSP> vectors = {av1, av2};
    conn.upload(names, vectors);

    ASSERT_TRUE(conn.run("eqObj(av1, data_av);")->getBool());
    ASSERT_TRUE(conn.run("res = array(BOOL);\
        for (i in 0:data_anyv.size()){\
            res.append!(eqObj(av2.row(i).sort(), data_anyv[i].sort()));\
        };all(res)")->getBool());

}

TEST_F(ArrayVectorTest, test_fill_scalar){
    dolphindb::VectorSP av0 = conn.run("x=array(INT[]).append!([1 2]).append!([1..100]).append!([[1]]).append!([[int(NULL)]]);x");
    std::cout << av0->getString() << std::endl;
    ASSERT_EQ(av0->getString(), "[[1,2],[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30...],[1],[]]");

    // set scalar
    av0->fill(0, 1, dolphindb::Util::createInt(888));
    std::cout << av0->getString() << std::endl;
    ASSERT_EQ(av0->getString(), "[[888],[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30...],[1],[]]");
    av0->fill(1, 2, dolphindb::Util::createNullConstant(dolphindb::DT_INT));
    std::cout << av0->getString() << std::endl;
    ASSERT_EQ(av0->getString(), "[[888],[],[],[]]");

    av0->fill(2, 1, dolphindb::Util::createInt(999));
    std::cout << av0->getString() << std::endl;
    ASSERT_EQ(av0->getString(), "[[888],[],[999],[]]");

    av0->fill(3, 1, dolphindb::Util::createInt(1));
    std::cout << av0->getString() << std::endl;
    ASSERT_EQ(av0->getString(), "[[888],[],[999],[1]]");

}


TEST_F(ArrayVectorTest, test_fill_tuple){
    dolphindb::VectorSP av0 = conn.run("x=array(INT[]).append!([1 2]).append!([1..100]).append!([[1]]).append!([[int(NULL)]]);x");
    std::cout << av0->getString() << std::endl;
    ASSERT_EQ(av0->getString(), "[[1,2],[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30...],[1],[]]");

    dolphindb::VectorSP tupleVal_1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 2);
    tupleVal_1->setNull(0);
    tupleVal_1->setNull(1);

    dolphindb::VectorSP tupleVal_2 = conn.run("x=array(ANY).append!(0);x");
    dolphindb::VectorSP tupleVal_3 = conn.run("x=array(ANY).append!(3 4).append!(5 6);x");
    dolphindb::VectorSP tupleVal_4 = conn.run("x=array(ANY).append!(NULL).append!(take(int(NULL),2));x");
    av0->fill(1, 2, tupleVal_1);
    std::cout << av0->getString() << std::endl;
    ASSERT_EQ(av0->getString(), "[[1,2],[],[],[]]");

    av0->fill(1, 0, tupleVal_1);
    std::cout << av0->getString() << std::endl;
    ASSERT_EQ(av0->getString(), "[[1,2],[],[],[]]");

    av0->fill(1, 1, tupleVal_2);
    std::cout << av0->getString() << std::endl;
    ASSERT_EQ(av0->getString(), "[[1,2],[0],[],[]]");
    ASSERT_ANY_THROW(av0->fill(1, 2, tupleVal_2));
    av0->fill(2, 2, tupleVal_3);
    std::cout << av0->getString() << std::endl;
    ASSERT_EQ(av0->getString(), "[[1,2],[0],[3,4],[5,6]]");
    av0->fill(2, 2, tupleVal_4);
    std::cout << av0->getString() << std::endl;
    ASSERT_EQ(av0->getString(), "[[1,2],[0],[],[,]]");

    dolphindb::VectorSP av1 = conn.run("x=array(INT[]).append!([[1]]).append!([[2]]);x");
    av1->fill(0, 2, tupleVal_4);
    std::cout << av1->getString() <<std::endl;
    ASSERT_EQ(av1->getString(), "[[],[,]]");

}


TEST_F(ArrayVectorTest, test_fill_array){
    dolphindb::VectorSP av2 = conn.run("x=array(INT[]).append!([1 2]).append!([1..100]).append!([[1]]).append!([[int(NULL)]]);x");
    ASSERT_EQ(av2->getString(), "[[1,2],[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30...],[1],[]]");
    dolphindb::VectorSP vecVal_1 = dolphindb::Util::createVector(dolphindb::DT_INT_ARRAY, 2);
    vecVal_1->setNull(0);
    vecVal_1->setNull(1);

    dolphindb::VectorSP vecVal_2 = conn.run("x=array(INT).append!(999);x");
    dolphindb::VectorSP vecVal_3 = conn.run("x=array(INT[]).append!([3 4]).append!([5 6]);x");
    dolphindb::VectorSP vecVal_4 = conn.run("x=array(INT[]).append!([[int(NULL)]]).append!([take(int(NULL),2)]);x");
    dolphindb::VectorSP vecVal_5 = conn.run("x=array(INT).append!(NULL).append!(NULL);x");
    av2->fill(1, 2, vecVal_1);
    std::cout << av2->getString() << std::endl;
    ASSERT_EQ(av2->getString(), "[[1,2],[],[],[]]");

    av2->fill(1, 0, vecVal_1);
    std::cout << av2->getString() << std::endl;
    ASSERT_EQ(av2->getString(), "[[1,2],[],[],[]]");

    av2->fill(1, 1, vecVal_2);
    std::cout << av2->getString() << std::endl;
    ASSERT_EQ(av2->getString(), "[[1,2],[999],[],[]]");
    ASSERT_ANY_THROW(av2->fill(1, 2, vecVal_2));
    av2->fill(2, 2, vecVal_3);
    std::cout << av2->getString() << std::endl;
    ASSERT_EQ(av2->getString(), "[[1,2],[999],[3,4],[5,6]]");
    av2->fill(2, 2, vecVal_4);
    std::cout << av2->getString() << std::endl;
    ASSERT_EQ(av2->getString(), "[[1,2],[999],[],[,]]");

    dolphindb::VectorSP av3 = conn.run("x=array(INT[]).append!([[1]]).append!([[2]]);x");
    av3->fill(0, 2, vecVal_4);
    std::cout << av3->getString() <<std::endl;
    ASSERT_EQ(av3->getString(), "[[],[,]]");

    ASSERT_ANY_THROW(av3->fill(0, 2, vecVal_5));
}

class ArrayVectorTest_create : public ArrayVectorTest, public ::testing::WithParamInterface<dolphindb::DATA_TYPE>{
};

INSTANTIATE_TEST_SUITE_P(, ArrayVectorTest_create, testing::Values(dolphindb::DT_BOOL_ARRAY,dolphindb::DT_CHAR_ARRAY,dolphindb::DT_SHORT_ARRAY,dolphindb::DT_INT_ARRAY,dolphindb::DT_LONG_ARRAY,dolphindb::DT_FLOAT_ARRAY,dolphindb::DT_DOUBLE_ARRAY,dolphindb::DT_DATE_ARRAY,dolphindb::DT_MONTH_ARRAY,dolphindb::DT_TIME_ARRAY,dolphindb::DT_MINUTE_ARRAY,dolphindb::DT_SECOND_ARRAY,dolphindb::DT_DATETIME_ARRAY,dolphindb::DT_TIMESTAMP_ARRAY,dolphindb::DT_NANOTIME_ARRAY,dolphindb::DT_NANOTIMESTAMP_ARRAY,dolphindb::DT_INT128_ARRAY,dolphindb::DT_UUID_ARRAY,dolphindb::DT_IP_ARRAY,dolphindb::DT_DECIMAL32_ARRAY,dolphindb::DT_DECIMAL64_ARRAY,dolphindb::DT_DECIMAL128_ARRAY,dolphindb::DT_DATEHOUR_ARRAY));

TEST_P(ArrayVectorTest_create, test_create_ArraryVector){
    dolphindb::DATA_TYPE type = GetParam();
    dolphindb::VectorSP av = dolphindb::Util::createArrayVector(type, 70000);

    ASSERT_EQ(av->size(), 70000);
    if (type == dolphindb::DT_INT128_ARRAY)
        ASSERT_EQ(av->getString(), "[[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000],[00000000000000000000000000000000]...]");
    else if (type == dolphindb::DT_UUID_ARRAY)
        ASSERT_EQ(av->getString(), "[[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000],[00000000-0000-0000-0000-000000000000]...]");
    else if (type == dolphindb::DT_IP_ARRAY)
        ASSERT_EQ(av->getString(), "[[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0],[0.0.0.0]...]");
    else
        ASSERT_EQ(av->getString(), "[[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[],[]...]");
    for (auto i=0; i< av->size(); i++){
        ASSERT_EQ(av->get(i)->size(), 1);
        ASSERT_TRUE(av->get(i)->isNull(0));
    }
}


class ArrayVectorTest_append_anyv : public ArrayVectorTest, public ::testing::WithParamInterface<std::tuple<dolphindb::DATA_TYPE, dolphindb::DATA_TYPE, std::string, std::string>>
{
public:
    static const std::vector<std::tuple<dolphindb::DATA_TYPE, dolphindb::DATA_TYPE, std::string, std::string>> get_testData()
    {
        return {
            std::make_tuple(dolphindb::DT_BOOL_ARRAY, dolphindb::DT_BOOL, "x = array(ANY);for(i in 0:100){x.append!(true false)};x", "ex=array(BOOL[]);for(i in 0:50){ex.append!([true false])};for(i in 0:50){ex.append!([[bool(NULL)]])};"),
            std::make_tuple(dolphindb::DT_CHAR_ARRAY, dolphindb::DT_CHAR, "x = array(ANY);for(i in 0:100){x.append!(1c NULL)};x", "ex=array(CHAR[]);for(i in 0:50){ex.append!([1c NULL])};for(i in 0:50){ex.append!([[char(NULL)]])};"),
            std::make_tuple(dolphindb::DT_SHORT_ARRAY, dolphindb::DT_SHORT, "x = array(ANY);for(i in 0:100){x.append!(1h NULL)};x", "ex=array(SHORT[]);for(i in 0:50){ex.append!([1h NULL])};for(i in 0:50){ex.append!([[short(NULL)]])};"),
            std::make_tuple(dolphindb::DT_INT_ARRAY, dolphindb::DT_INT, "x = array(ANY);for(i in 0:100){x.append!(1 NULL)};x", "ex=array(INT[]);for(i in 0:50){ex.append!([1 NULL])};for(i in 0:50){ex.append!([[int(NULL)]])};"),
            std::make_tuple(dolphindb::DT_LONG_ARRAY, dolphindb::DT_LONG, "x = array(ANY);for(i in 0:100){x.append!(1l NULL)};x", "ex=array(LONG[]);for(i in 0:50){ex.append!([1l NULL])};for(i in 0:50){ex.append!([[long(NULL)]])};"),
            std::make_tuple(dolphindb::DT_FLOAT_ARRAY, dolphindb::DT_FLOAT, "x = array(ANY);for(i in 0:100){x.append!(1f NULL)};x", "ex=array(FLOAT[]);for(i in 0:50){ex.append!([1f NULL])};for(i in 0:50){ex.append!([[float(NULL)]])};"),
            std::make_tuple(dolphindb::DT_DOUBLE_ARRAY, dolphindb::DT_DOUBLE, "x = array(ANY);for(i in 0:100){x.append!(1.0 NULL)};x", "ex=array(DOUBLE[]);for(i in 0:50){ex.append!([1.0 NULL])};for(i in 0:50){ex.append!([[double(NULL)]])};"),
            std::make_tuple(dolphindb::DT_DATE_ARRAY, dolphindb::DT_DATE, "x = array(ANY);for(i in 0:100){x.append!(2023.01.01 NULL)};x", "ex=array(DATE[]);for(i in 0:50){ex.append!([2023.01.01 NULL])};for(i in 0:50){ex.append!([[date(NULL)]])};"),
            std::make_tuple(dolphindb::DT_MONTH_ARRAY, dolphindb::DT_MONTH, "x = array(ANY);for(i in 0:100){x.append!(2013.11M NULL)};x", "ex=array(MONTH[]);for(i in 0:50){ex.append!([2013.11M NULL])};for(i in 0:50){ex.append!([[month(NULL)]])};"),
            std::make_tuple(dolphindb::DT_TIME_ARRAY, dolphindb::DT_TIME, "x = array(ANY);for(i in 0:100){x.append!(12:00:00.123 NULL)};x", "ex=array(TIME[]);for(i in 0:50){ex.append!([12:00:00.123 NULL])};for(i in 0:50){ex.append!([[time(NULL)]])};"),
            std::make_tuple(dolphindb::DT_MINUTE_ARRAY, dolphindb::DT_MINUTE, "x = array(ANY);for(i in 0:100){x.append!(13:30m NULL)};x", "ex=array(MINUTE[]);for(i in 0:50){ex.append!([13:30m NULL])};for(i in 0:50){ex.append!([[minute(NULL)]])};"),
            std::make_tuple(dolphindb::DT_SECOND_ARRAY, dolphindb::DT_SECOND, "x = array(ANY);for(i in 0:100){x.append!(13:30:30 NULL)};x", "ex=array(SECOND[]);for(i in 0:50){ex.append!([13:30:30 NULL])};for(i in 0:50){ex.append!([[second(NULL)]])};"),
            std::make_tuple(dolphindb::DT_DATETIME_ARRAY, dolphindb::DT_DATETIME, "x = array(ANY);for(i in 0:100){x.append!(2012.06.13T13:30:10 NULL)};x", "ex=array(DATETIME[]);for(i in 0:50){ex.append!([2012.06.13T13:30:10 NULL])};for(i in 0:50){ex.append!([[datetime(NULL)]])};"),
            std::make_tuple(dolphindb::DT_TIMESTAMP_ARRAY, dolphindb::DT_TIMESTAMP, "x = array(ANY);for(i in 0:100){x.append!(2023.01.01T12:00:00.123 NULL)};x", "ex=array(TIMESTAMP[]);for(i in 0:50){ex.append!([2023.01.01T12:00:00.123 NULL])};for(i in 0:50){ex.append!([[timestamp(NULL)]])};"),
            std::make_tuple(dolphindb::DT_NANOTIME_ARRAY, dolphindb::DT_NANOTIME, "x = array(ANY);for(i in 0:100){x.append!(13:30:10.008007006 NULL)};x", "ex=array(NANOTIME[]);for(i in 0:50){ex.append!([13:30:10.008007006 NULL])};for(i in 0:50){ex.append!([[nanotime(NULL)]])};"),
            std::make_tuple(dolphindb::DT_NANOTIMESTAMP_ARRAY, dolphindb::DT_NANOTIMESTAMP, "x = array(ANY);for(i in 0:100){x.append!(2012.06.13T13:30:10.008007006 NULL)};x", "ex=array(NANOTIMESTAMP[]);for(i in 0:50){ex.append!([2012.06.13T13:30:10.008007006 NULL])};for(i in 0:50){ex.append!([[nanotimestamp(NULL)]])};"),
            std::make_tuple(dolphindb::DT_DATEHOUR_ARRAY, dolphindb::DT_DATEHOUR, "x = array(ANY);for(i in 0:100){x.append!(datehour(10000 NULL))};x", "ex=array(DATEHOUR[]);for(i in 0:50){ex.append!([datehour(10000 NULL)])};for(i in 0:50){ex.append!([[datehour(NULL)]])};"),
            std::make_tuple(dolphindb::DT_INT128_ARRAY, dolphindb::DT_INT128, "x = array(ANY);for(i in 0:100){x.append!(int128(`e1671797c52e15f763380b45e841ec32`))};x", "ex=array(INT128[]);for(i in 0:50){ex.append!([int128(`e1671797c52e15f763380b45e841ec32`)])};for(i in 0:50){ex.append!([[int128()]])};"),
            std::make_tuple(dolphindb::DT_UUID_ARRAY, dolphindb::DT_UUID, "x = array(ANY);for(i in 0:100){x.append!(uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'`))};x", "ex=array(UUID[]);for(i in 0:50){ex.append!([uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'`)])};for(i in 0:50){ex.append!([[uuid()]])};"),
            std::make_tuple(dolphindb::DT_IP_ARRAY, dolphindb::DT_IP, "x = array(ANY);for(i in 0:100){x.append!(ipaddr('1.1.1.1'`))};x", "ex=array(IPADDR[]);for(i in 0:50){ex.append!([ipaddr('1.1.1.1'`)])};for(i in 0:50){ex.append!([[ipaddr()]])};"),
            std::make_tuple(dolphindb::DT_DECIMAL32_ARRAY, dolphindb::DT_DECIMAL32, "x = array(ANY);for(i in 0:100){x.append!(decimal32('-1.453254'`,5))};x", "ex=array(DECIMAL32(5)[]);for(i in 0:50){ex.append!([decimal32('-1.453254'`,5)])};for(i in 0:50){ex.append!([[decimal32(,5)]])};"),
            std::make_tuple(dolphindb::DT_DECIMAL64_ARRAY, dolphindb::DT_DECIMAL64, "x = array(ANY);for(i in 0:100){x.append!(decimal64('-1.453254'`,5))};x", "ex=array(DECIMAL64(5)[]);for(i in 0:50){ex.append!([decimal64('-1.453254'`,5)])};for(i in 0:50){ex.append!([[decimal64(, 5)]])};"),
            std::make_tuple(dolphindb::DT_DECIMAL128_ARRAY, dolphindb::DT_DECIMAL128, "x = array(ANY);for(i in 0:100){x.append!(decimal128('-1.453254'`,5))};x", "ex=array(DECIMAL128(5)[]);for(i in 0:50){ex.append!([decimal128('-1.453254'`,5)])};for(i in 0:50){ex.append!([[decimal128(, 5)]])};"),
        };
    }
};

INSTANTIATE_TEST_SUITE_P(, ArrayVectorTest_append_anyv, ::testing::ValuesIn(ArrayVectorTest_append_anyv::get_testData()));
TEST_P(ArrayVectorTest_append_anyv, testArrayVector_append_anyv_and_scalar)
{
    int size = 100;
    dolphindb::DATA_TYPE av_type = std::get<0>(GetParam());
    dolphindb::DATA_TYPE v_type = std::get<1>(GetParam());
    std::string v0_script = std::get<2>(GetParam());
    std::string ex_av1_script = std::get<3>(GetParam());
    int extra = 0;
    if (v_type == dolphindb::DT_DECIMAL32 || v_type == dolphindb::DT_DECIMAL64 || v_type == dolphindb::DT_DECIMAL128)
    {
        extra = 5;
    }
    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(av_type, 0, 0, true, extra);
    dolphindb::VectorSP v0 = conn.run(v0_script);

    ASSERT_FALSE(av1->append(v0->get(0), 0, 1));
    ASSERT_TRUE(av1->append(v0, 0, 50));

    for (int i = 50; i < 100; i++) {
        ASSERT_TRUE(av1->append(dolphindb::Util::createNullConstant(v_type, extra), i, 1));
    }

    conn.upload("av1", av1);
    conn.run(ex_av1_script);
    dolphindb::ConstantSP res = conn.run("eqObj(ex, av1)");
    ASSERT_TRUE(res->getBool());
}

class ArrayVectorTest_append_av : public ArrayVectorTest, public ::testing::WithParamInterface<std::vector<std::string>>
{
public:
    static const std::vector<std::vector<std::string>> get_testData()
    {
        return {
            {"bool_arrayVector", "x = array(BOOL[]).append!([take(false, 2)]);vals = take(x, 5);vals", "x = array(BOOL[]).append!([take(true, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(true true);data_anyv = take(x, 5);data_anyv"},
            {"char_arrayVector", "x = array(CHAR[]).append!([rand(127c, 2)]);vals = take(x, 5);vals", "x = array(CHAR[]).append!([rand(127c, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(127c, 2));data_anyv = take(x, 5);data_anyv"},
            {"short_arrayVector", "x = array(SHORT[]).append!([rand(32767h, 2)]);vals = take(x, 5);vals", "x = array(SHORT[]).append!([rand(32767h, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(32767h, 2));data_anyv = take(x, 5);data_anyv"},
            {"int_arrayVector", "x = array(INT[]).append!([1..5]);vals = take(x, 5);vals", "x = array(INT[]).append!([6..10]);data_av = take(x, 5);data_av", "x = array(ANY).append!(6..10);data_anyv = take(x, 5);data_anyv"},
            {"long_arrayVector", "x = array(LONG[]).append!([rand(2147483647l, 2)]);vals = take(x, 5);vals", "x = array(LONG[]).append!([rand(2147483647l, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2147483647l, 2));data_anyv = take(x, 5);data_anyv"},
            {"float_arrayVector", "x = array(FLOAT[]).append!([rand(100.00f, 2)]);vals = take(x, 5);vals", "x = array(FLOAT[]).append!([rand(100.00f, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(100.00f, 2));data_anyv = take(x, 5);data_anyv"},
            {"double_arrayVector", "x = array(DOUBLE[]).append!([rand(100.00, 2)]);vals = take(x, 5);vals", "x = array(DOUBLE[]).append!([rand(100.00, 2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(100.00, 2));data_anyv = take(x, 5);data_anyv"},
            {"date_arrayVector", "x = array(DATE[]).append!([rand(2023.01.01,2)]);vals = take(x, 5);vals", "x = array(DATE[]).append!([rand(2023.01.01,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2023.01.01,2));data_anyv = take(x, 5);data_anyv"},
            {"month_arrayVector", "x = array(MONTH[]).append!([rand(2023.01M,2)]);vals = take(x, 5);vals", "x = array(MONTH[]).append!([rand(2023.01M,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2023.01M,2));data_anyv = take(x, 5);data_anyv"},
            {"time_arrayVector", "x = array(TIME[]).append!([rand(23:00:00.000,2)]);vals = take(x, 5);vals", "x = array(TIME[]).append!([rand(23:00:00.000,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(23:00:00.000,2));data_anyv = take(x, 5);data_anyv"},
            {"minute_arrayVector", "x = array(MINUTE[]).append!([rand(23:00m,2)]);vals = take(x, 5);vals", "x = array(MINUTE[]).append!([rand(23:00m,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(23:00m,2));data_anyv = take(x, 5);data_anyv"},
            {"second_arrayVector", "x = array(SECOND[]).append!([rand(23:00:00,2)]);vals = take(x, 5);vals", "x = array(SECOND[]).append!([rand(23:00:00,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(23:00:00,2));data_anyv = take(x, 5);data_anyv"},
            {"datetime_arrayVector", "x = array(DATETIME[]).append!([rand(2023.01.01T00:00:00,2)]);vals = take(x, 5);vals", "x = array(DATETIME[]).append!([rand(2023.01.01T00:00:00,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2023.01.01T00:00:00,2));data_anyv = take(x, 5);data_anyv"},
            {"timestamp_arrayVector", "x = array(TIMESTAMP[]).append!([rand(2023.01.01T00:00:00.000,2)]);vals = take(x, 5);vals", "x = array(TIMESTAMP[]).append!([rand(2023.01.01T00:00:00.000,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2023.01.01T00:00:00.000,2));data_anyv = take(x, 5);data_anyv"},
            {"nanotime_arrayVector", "x = array(NANOTIME[]).append!([rand(23:00:00.000000000,2)]);vals = take(x, 5);vals", "x = array(NANOTIME[]).append!([rand(23:00:00.000000000,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(23:00:00.000000000,2));data_anyv = take(x, 5);data_anyv"},
            {"nanotimestamp_arrayVector", "x = array(NANOTIMESTAMP[]).append!([rand(2023.01.01T00:00:00.000000000,2)]);vals = take(x, 5);vals", "x = array(NANOTIMESTAMP[]).append!([rand(2023.01.01T00:00:00.000000000,2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(2023.01.01T00:00:00.000000000,2));data_anyv = take(x, 5);data_anyv"},
            {"datehour_arrayVector", "x = array(DATEHOUR[]).append!([rand(datehour(10000),2)]);vals = take(x, 5);vals", "x = array(DATEHOUR[]).append!([rand(datehour(10000),2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(datehour(10000),2));data_anyv = take(x, 5);data_anyv"},
            {"int128_arrayVector", "x = array(INT128[]).append!([rand(int128('e1671797c52e15f763380b45e841ec32'),2)]);vals = take(x, 5);vals", "x = array(INT128[]).append!([rand(int128('e1671797c52e15f763380b45e841ec32'),2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(int128('e1671797c52e15f763380b45e841ec32'),2));data_anyv = take(x, 5);data_anyv"},
            {"uuid_arrayVector", "x = array(UUID[]).append!([rand(uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'),2)]);vals = take(x, 5);vals", "x = array(UUID[]).append!([rand(uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'),2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'),2));data_anyv = take(x, 5);data_anyv"},
            {"ipaddr_arrayVector", "x = array(IPADDR[]).append!([rand(ipaddr('255.255.255.0'),2)]);vals = take(x, 5);vals", "x = array(IPADDR[]).append!([rand(ipaddr('255.255.255.0'),2)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(rand(ipaddr('255.255.255.0'),2));data_anyv = take(x, 5);data_anyv"},
            {"decimal32_arrayVector", "x = array(DECIMAL32(5)[]).append!([decimal32(rand(100.00,2), 5)]);vals = take(x, 5);vals", "x = array(DECIMAL32(5)[]).append!([decimal32(rand(100.00,2), 5)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(decimal32(rand(100.00,2), 5));data_anyv = take(x, 5);data_anyv"},
            {"decimal64_arrayVector", "x = array(DECIMAL64(10)[]).append!([decimal64(rand(100.00,2), 10)]);vals = take(x, 5);vals", "x = array(DECIMAL64(10)[]).append!([decimal64(rand(100.00,2), 10)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(decimal64(rand(100.00,2), 10));data_anyv = take(x, 5);data_anyv"},
            {"decimal128_arrayVector", "x = array(DECIMAL128(20)[]).append!([decimal128(rand(100.00,2), 20)]);vals = take(x, 5);vals", "x = array(DECIMAL128(20)[]).append!([decimal128(rand(100.00,2), 20)]);data_av = take(x, 5);data_av", "x = array(ANY).append!(decimal128(rand(100.00,2), 20));data_anyv = take(x, 5);data_anyv"},
        };
    }
};

INSTANTIATE_TEST_SUITE_P(, ArrayVectorTest_append_av, testing::ValuesIn(ArrayVectorTest_append_av::get_testData()),
    [](const testing::TestParamInfo<ArrayVectorTest_append_av::ParamType>& info){ 
        return info.param[0]; 
    });
TEST_P(ArrayVectorTest_append_av, testArrayVector_append_av){
    std::string s1 = GetParam()[1];
    std::string s2 = GetParam()[2];
    std::string s3 = GetParam()[3];
    dolphindb::VectorSP av1 = conn.run(s1);
    dolphindb::VectorSP av2 = av1->getInstance();

    dolphindb::VectorSP indexV = dolphindb::Util::createIndexVector(0, 5);
    dolphindb::VectorSP data_av = conn.run(s2);
    dolphindb::VectorSP data_anyv = conn.run(s3);

    ASSERT_TRUE(av1->set(indexV, data_av));
    ASSERT_TRUE(av2->set(indexV, data_anyv));

    std::vector<std::string> names = {"av1", "av2"};
    std::vector<dolphindb::ConstantSP> vectors = {av1, av2};
    conn.upload(names, vectors);

    ASSERT_TRUE(conn.run("eqObj(av1, data_av);")->getBool());
    ASSERT_TRUE(conn.run("res = array(BOOL);\
        for (i in 0:data_anyv.size()){\
            res.append!(eqObj(av2.row(i).sort(), data_anyv[i].sort()));\
        };all(res)")->getBool());
}

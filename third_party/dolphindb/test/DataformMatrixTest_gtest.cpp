#include <gtest/gtest.h>
#include "config.h"
#include "ConstantImp.h"

class DataformMatrixTest : public testing::Test
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

dolphindb::DBConnection DataformMatrixTest::conn(false, false);

TEST_F(DataformMatrixTest, testStringMatrix)
{
    ASSERT_ANY_THROW(dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_STRING, 2, 2, 4));
}

TEST_F(DataformMatrixTest, testBlobMatrix)
{
    ASSERT_ANY_THROW(dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_BLOB, 2, 2, 4));
}

TEST_F(DataformMatrixTest, testAnyMatrix)
{
    ASSERT_ANY_THROW(dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_ANY, 2, 2, 4));
}

TEST_F(DataformMatrixTest, testDecimal32Matrix)
{
    ASSERT_ANY_THROW(dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DECIMAL32, 2, 2, 4));
}

TEST_F(DataformMatrixTest, testDecimal64Matrix)
{
    ASSERT_ANY_THROW(dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DECIMAL64, 2, 2, 4));
}

TEST_F(DataformMatrixTest, testDecimal128Matrix)
{
    ASSERT_ANY_THROW(dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DECIMAL128, 2, 2, 4));
}

#ifndef _MSC_VER
TEST_F(DataformMatrixTest, testBoolMatrix_getValue)
{
    char val[4] = {1, 1, 0, 0};
    dolphindb::FastBoolMatrix *matrix1 = new dolphindb::FastBoolMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testBoolMatrix_set)
{
    char val1[4] = {1, 1, 0, 0};
    dolphindb::FastBoolMatrix *matrix1 = new dolphindb::FastBoolMatrix(2, 2, 4, val1, false);
    char val2[4] = {1, 1, 1, 1};
    dolphindb::FastBoolMatrix *matrix2 = new dolphindb::FastBoolMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_BOOL, 2, 2);
    insertVec->set(0, dolphindb::Util::createBool(true));
    insertVec->set(1, dolphindb::Util::createBool(true));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testCharMatrix_getValue)
{
    char val[4] = {1, 1, 0, 0};
    dolphindb::FastCharMatrix *matrix1 = new dolphindb::FastCharMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testCharMatrix_set)
{
    char val1[4] = {1, 1, 0, 0};
    dolphindb::FastCharMatrix *matrix1 = new dolphindb::FastCharMatrix(2, 2, 4, val1, false);
    char val2[4] = {1, 1, 1, 1};
    dolphindb::FastCharMatrix *matrix2 = new dolphindb::FastCharMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_CHAR, 2, 2);
    insertVec->set(0, dolphindb::Util::createChar(1));
    insertVec->set(1, dolphindb::Util::createChar(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testShortMatrix_getValue)
{
    short val[4] = {1, 1, 0, 0};
    dolphindb::FastShortMatrix *matrix1 = new dolphindb::FastShortMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testShortMatrix_set)
{
    short val1[4] = {1, 1, 0, 0};
    dolphindb::FastShortMatrix *matrix1 = new dolphindb::FastShortMatrix(2, 2, 4, val1, false);
    short val2[4] = {1, 1, 1, 1};
    dolphindb::FastShortMatrix *matrix2 = new dolphindb::FastShortMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_SHORT, 2, 2);
    insertVec->set(0, dolphindb::Util::createShort(1));
    insertVec->set(1, dolphindb::Util::createShort(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testIntMatrix_getValue)
{
    int val[4] = {1, 1, 0, 0};
    dolphindb::FastIntMatrix *matrix1 = new dolphindb::FastIntMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testIntMatrix_set)
{
    int val1[4] = {1, 1, 0, 0};
    dolphindb::FastIntMatrix *matrix1 = new dolphindb::FastIntMatrix(2, 2, 4, val1, false);
    int val2[4] = {1, 1, 1, 1};
    dolphindb::FastIntMatrix *matrix2 = new dolphindb::FastIntMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2);
    insertVec->set(0, dolphindb::Util::createInt(1));
    insertVec->set(1, dolphindb::Util::createInt(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testLongMatrix_getValue)
{
    long long val[4] = {1, 1, 0, 0};
    dolphindb::FastLongMatrix *matrix1 = new dolphindb::FastLongMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testLongMatrix_set)
{
    long long val1[4] = {1, 1, 0, 0};
    dolphindb::FastLongMatrix *matrix1 = new dolphindb::FastLongMatrix(2, 2, 4, val1, false);
    long long val2[4] = {1, 1, 1, 1};
    dolphindb::FastLongMatrix *matrix2 = new dolphindb::FastLongMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_LONG, 2, 2);
    insertVec->set(0, dolphindb::Util::createLong(1));
    insertVec->set(1, dolphindb::Util::createLong(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testFloatMatrix_getValue)
{
    float val[4] = {1, 1, 0, 0};
    dolphindb::FastFloatMatrix *matrix1 = new dolphindb::FastFloatMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testFloatMatrix_set)
{
    float val1[4] = {1, 1, 0, 0};
    dolphindb::FastFloatMatrix *matrix1 = new dolphindb::FastFloatMatrix(2, 2, 4, val1, false);
    float val2[4] = {1, 1, 1, 1};
    dolphindb::FastFloatMatrix *matrix2 = new dolphindb::FastFloatMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_FLOAT, 2, 2);
    insertVec->set(0, dolphindb::Util::createFloat(1));
    insertVec->set(1, dolphindb::Util::createFloat(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testDoubleMatrix_getValue)
{
    double val[4] = {1, 1, 0, 0};
    dolphindb::FastDoubleMatrix *matrix1 = new dolphindb::FastDoubleMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testDoubleMatrix_set)
{
    double val1[4] = {1, 1, 0, 0};
    dolphindb::FastDoubleMatrix *matrix1 = new dolphindb::FastDoubleMatrix(2, 2, 4, val1, false);
    double val2[4] = {1, 1, 1, 1};
    dolphindb::FastDoubleMatrix *matrix2 = new dolphindb::FastDoubleMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 2, 2);
    insertVec->set(0, dolphindb::Util::createDouble(1));
    insertVec->set(1, dolphindb::Util::createDouble(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testDateMatrix_getValue)
{
    int val[4] = {1, 1, 0, 0};
    dolphindb::FastDateMatrix *matrix1 = new dolphindb::FastDateMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testDateMatrix_set)
{
    int val1[4] = {1, 1, 0, 0};
    dolphindb::FastDateMatrix *matrix1 = new dolphindb::FastDateMatrix(2, 2, 4, val1, false);
    int val2[4] = {1, 1, 1, 1};
    dolphindb::FastDateMatrix *matrix2 = new dolphindb::FastDateMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_DATE, 2, 2);
    insertVec->set(0, dolphindb::Util::createDate(1));
    insertVec->set(1, dolphindb::Util::createDate(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testDateTimeMatrix_getValue)
{
    int val[4] = {1, 1, 0, 0};
    dolphindb::FastDateTimeMatrix *matrix1 = new dolphindb::FastDateTimeMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testDateTimeMatrix_set)
{
    int val1[4] = {1, 1, 0, 0};
    dolphindb::FastDateTimeMatrix *matrix1 = new dolphindb::FastDateTimeMatrix(2, 2, 4, val1, false);
    int val2[4] = {1, 1, 1, 1};
    dolphindb::FastDateTimeMatrix *matrix2 = new dolphindb::FastDateTimeMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 2, 2);
    insertVec->set(0, dolphindb::Util::createDateTime(1));
    insertVec->set(1, dolphindb::Util::createDateTime(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testDateHourMatrix_set)
{
    int val1[4] = {1, 1, 0, 0};
    dolphindb::FastDateHourMatrix *matrix1 = new dolphindb::FastDateHourMatrix(2, 2, 4, val1, false);
    int val2[4] = {1, 1, 1, 1};
    dolphindb::FastDateHourMatrix *matrix2 = new dolphindb::FastDateHourMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 2, 2);
    insertVec->set(0, dolphindb::Util::createDateHour(1));
    insertVec->set(1, dolphindb::Util::createDateHour(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testMonthMatrix_getValue)
{
    int val[4] = {1, 1, 0, 0};
    dolphindb::FastMonthMatrix *matrix1 = new dolphindb::FastMonthMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testMonthMatrix_set)
{
    int val1[4] = {1, 1, 0, 0};
    dolphindb::FastMonthMatrix *matrix1 = new dolphindb::FastMonthMatrix(2, 2, 4, val1, false);
    int val2[4] = {1, 1, 1, 1};
    dolphindb::FastMonthMatrix *matrix2 = new dolphindb::FastMonthMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_MONTH, 2, 2);
    insertVec->set(0, dolphindb::Util::createMonth(1));
    insertVec->set(1, dolphindb::Util::createMonth(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testTimeMatrix_getValue)
{
    int val[4] = {1, 1, 0, 0};
    dolphindb::FastTimeMatrix *matrix1 = new dolphindb::FastTimeMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testTimeMatrix_set)
{
    int val1[4] = {1, 1, 0, 0};
    dolphindb::FastTimeMatrix *matrix1 = new dolphindb::FastTimeMatrix(2, 2, 4, val1, false);
    int val2[4] = {1, 1, 1, 1};
    dolphindb::FastTimeMatrix *matrix2 = new dolphindb::FastTimeMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_TIME, 2, 2);
    insertVec->set(0, dolphindb::Util::createTime(1));
    insertVec->set(1, dolphindb::Util::createTime(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testSecondMatrix_getValue)
{
    int val[4] = {1, 1, 0, 0};
    dolphindb::FastSecondMatrix *matrix1 = new dolphindb::FastSecondMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testSecondMatrix_set)
{
    int val1[4] = {1, 1, 0, 0};
    dolphindb::FastSecondMatrix *matrix1 = new dolphindb::FastSecondMatrix(2, 2, 4, val1, false);
    int val2[4] = {1, 1, 1, 1};
    dolphindb::FastSecondMatrix *matrix2 = new dolphindb::FastSecondMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_SECOND, 2, 2);
    insertVec->set(0, dolphindb::Util::createSecond(1));
    insertVec->set(1, dolphindb::Util::createSecond(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testMinuteMatrix_getValue)
{
    int val[4] = {1, 1, 0, 0};
    dolphindb::FastMinuteMatrix *matrix1 = new dolphindb::FastMinuteMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testMinuteMatrix_set)
{
    int val1[4] = {1, 1, 0, 0};
    dolphindb::FastMinuteMatrix *matrix1 = new dolphindb::FastMinuteMatrix(2, 2, 4, val1, false);
    int val2[4] = {1, 1, 1, 1};
    dolphindb::FastMinuteMatrix *matrix2 = new dolphindb::FastMinuteMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_MINUTE, 2, 2);
    insertVec->set(0, dolphindb::Util::createMinute(1));
    insertVec->set(1, dolphindb::Util::createMinute(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testNanoTimeMatrix_getValue)
{
    long long val[4] = {1, 1, 0, 0};
    dolphindb::FastNanoTimeMatrix *matrix1 = new dolphindb::FastNanoTimeMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testNanoTimeMatrix_set)
{
    long long val1[4] = {1, 1, 0, 0};
    dolphindb::FastNanoTimeMatrix *matrix1 = new dolphindb::FastNanoTimeMatrix(2, 2, 4, val1, false);
    long long val2[4] = {1, 1, 1, 1};
    dolphindb::FastNanoTimeMatrix *matrix2 = new dolphindb::FastNanoTimeMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, 2, 2);
    insertVec->set(0, dolphindb::Util::createNanoTime(1));
    insertVec->set(1, dolphindb::Util::createNanoTime(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testTimestampMatrix_getValue)
{
    long long val[4] = {1, 1, 0, 0};
    dolphindb::FastTimestampMatrix *matrix1 = new dolphindb::FastTimestampMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testTimestampMatrix_set)
{
    long long val1[4] = {1, 1, 0, 0};
    dolphindb::FastTimestampMatrix *matrix1 = new dolphindb::FastTimestampMatrix(2, 2, 4, val1, false);
    long long val2[4] = {1, 1, 1, 1};
    dolphindb::FastTimestampMatrix *matrix2 = new dolphindb::FastTimestampMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2, 2);
    insertVec->set(0, dolphindb::Util::createTimestamp(1));
    insertVec->set(1, dolphindb::Util::createTimestamp(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testNanoTimestampMatrix_getValue)
{
    long long val[4] = {1, 1, 0, 0};
    dolphindb::FastNanoTimestampMatrix *matrix1 = new dolphindb::FastNanoTimestampMatrix(2, 2, 4, val, false);
    dolphindb::ConstantSP matrix2 = matrix1->getValue();
    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}

TEST_F(DataformMatrixTest, testNanoTimestampMatrix_set)
{
    long long val1[4] = {1, 1, 0, 0};
    dolphindb::FastNanoTimestampMatrix *matrix1 = new dolphindb::FastNanoTimestampMatrix(2, 2, 4, val1, false);
    long long val2[4] = {1, 1, 1, 1};
    dolphindb::FastNanoTimestampMatrix *matrix2 = new dolphindb::FastNanoTimestampMatrix(2, 2, 4, val2, false);

    dolphindb::ConstantSP insertVec = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 2, 2);
    insertVec->set(0, dolphindb::Util::createNanoTimestamp(1));
    insertVec->set(1, dolphindb::Util::createNanoTimestamp(1));
    matrix1->set(1, 0, insertVec);

    ASSERT_EQ(matrix1->getString(), matrix2->getString());
}
#endif // _MSC_VER

TEST_F(DataformMatrixTest, testBoolMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_BOOL, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createBool(true));
    v1->set(1, 0, dolphindb::Util::createBool(true));
    v1->set(0, 1, dolphindb::Util::createBool(false));
    v1->set(1, 1, dolphindb::Util::createBool(false));
    std::string script = "a=matrix([[bool(1),bool(0)],[bool(1),bool(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});

    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createBool(true)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createBool(true)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createBool(true));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_BOOL, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createBool(false));
    vals->append(dolphindb::Util::createBool(false));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createBool(false));
    vals->append(dolphindb::Util::createBool(false));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0 #1\n-- --\n0  0 \n0  0 \n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createBool(true));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createBool(false));
    winMatrix1->set(1, 0, dolphindb::Util::createBool(true));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testBoolNullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_BOOL, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_BOOL));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_BOOL));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_BOOL));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_BOOL));
    std::string script = "a=matrix([[bool(NULL),bool(NULL)],[bool(NULL),bool(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testCharMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_CHAR, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createChar(1));
    v1->set(1, 0, dolphindb::Util::createChar(1));
    v1->set(0, 1, dolphindb::Util::createChar(0));
    v1->set(1, 1, dolphindb::Util::createChar(0));
    std::string script = "a=matrix([[char(1),char(0)],[char(1),char(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createChar(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createChar(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createChar(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_CHAR, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createChar(0));
    vals->append(dolphindb::Util::createChar(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createChar(0));
    vals->append(dolphindb::Util::createChar(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0 #1\n-- --\n0  0 \n0  0 \n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createChar(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createChar(0));
    winMatrix1->set(1, 0, dolphindb::Util::createChar(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testCharNullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_CHAR, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_CHAR));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_CHAR));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_CHAR));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_CHAR));
    std::string script = "a=matrix([[char(NULL),char(NULL)],[char(NULL),char(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testIntMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_INT, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createInt(1));
    v1->set(1, 0, dolphindb::Util::createInt(1));
    v1->set(0, 1, dolphindb::Util::createInt(0));
    v1->set(1, 1, dolphindb::Util::createInt(0));
    std::string script = "a=matrix([[int(1),int(0)],[int(1),int(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createInt(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createInt(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createInt(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createInt(0));
    vals->append(dolphindb::Util::createInt(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createInt(0));
    vals->append(dolphindb::Util::createInt(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0 #1\n-- --\n0  0 \n0  0 \n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createBool(true));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createInt(0));
    winMatrix1->set(1, 0, dolphindb::Util::createInt(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testIntNullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_INT, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_INT));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_INT));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_INT));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_INT));
    std::string script = "a=matrix([[int(NULL),int(NULL)],[int(NULL),int(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testLongMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_LONG, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createLong(1));
    v1->set(1, 0, dolphindb::Util::createLong(1));
    v1->set(0, 1, dolphindb::Util::createLong(0));
    v1->set(1, 1, dolphindb::Util::createLong(0));
    std::string script = "a=matrix([[long(1),long(0)],[long(1),long(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createLong(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createLong(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createLong(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_LONG, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createLong(0));
    vals->append(dolphindb::Util::createLong(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createLong(0));
    vals->append(dolphindb::Util::createLong(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0 #1\n-- --\n0  0 \n0  0 \n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createLong(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createLong(0));
    winMatrix1->set(1, 0, dolphindb::Util::createLong(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testLongNullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_LONG, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_LONG));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_LONG));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_LONG));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_LONG));
    std::string script = "a=matrix([[long(NULL),long(NULL)],[long(NULL),long(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testShortNullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_SHORT, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_SHORT));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_SHORT));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_SHORT));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_SHORT));
    std::string script = "a=matrix([[short(NULL),short(NULL)],[short(NULL),short(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testShortMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_SHORT, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createShort(1));
    v1->set(1, 0, dolphindb::Util::createShort(1));
    v1->set(0, 1, dolphindb::Util::createShort(0));
    v1->set(1, 1, dolphindb::Util::createShort(0));
    std::string script = "a=matrix([[short(1),short(0)],[short(1),short(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createShort(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createShort(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createShort(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_SHORT, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createShort(0));
    vals->append(dolphindb::Util::createShort(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createShort(0));
    vals->append(dolphindb::Util::createShort(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0 #1\n-- --\n0  0 \n0  0 \n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createShort(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createShort(0));
    winMatrix1->set(1, 0, dolphindb::Util::createShort(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testFloatMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_FLOAT, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createFloat(1));
    v1->set(1, 0, dolphindb::Util::createFloat(1));
    v1->set(0, 1, dolphindb::Util::createFloat(0));
    v1->set(1, 1, dolphindb::Util::createFloat(0));
    std::string script = "a=matrix([[float(1),float(0)],[float(1),float(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createFloat(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createFloat(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createFloat(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_FLOAT, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createFloat(0));
    vals->append(dolphindb::Util::createFloat(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createFloat(0));
    vals->append(dolphindb::Util::createFloat(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0 #1\n-- --\n0  0 \n0  0 \n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createFloat(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);
    std::cout << v1->getString() << std::endl;

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createFloat(0));
    winMatrix1->set(1, 0, dolphindb::Util::createFloat(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testFloatNullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_FLOAT, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT));
    std::string script = "a=matrix([[float(NULL),float(NULL)],[float(NULL),float(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDoubleMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DOUBLE, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createDouble(1));
    v1->set(1, 0, dolphindb::Util::createDouble(1));
    v1->set(0, 1, dolphindb::Util::createDouble(0));
    v1->set(1, 1, dolphindb::Util::createDouble(0));
    std::string script = "a=matrix([[double(1),double(0)],[double(1),double(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createDouble(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createDouble(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createDouble(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createDouble(0));
    vals->append(dolphindb::Util::createDouble(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createDouble(0));
    vals->append(dolphindb::Util::createDouble(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0 #1\n-- --\n0  0 \n0  0 \n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createDouble(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createDouble(0));
    winMatrix1->set(1, 0, dolphindb::Util::createDouble(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testDoubleNullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DOUBLE, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_DOUBLE));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_DOUBLE));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_DOUBLE));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_DOUBLE));
    std::string script = "a=matrix([[double(NULL),double(NULL)],[double(NULL),double(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDatehourMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATEHOUR, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createDateHour(1));
    v1->set(1, 0, dolphindb::Util::createDateHour(1));
    v1->set(0, 1, dolphindb::Util::createDateHour(0));
    v1->set(1, 1, dolphindb::Util::createDateHour(0));
    std::string script = "a=matrix([[datehour(1),datehour(0)],[datehour(1),datehour(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createDateHour(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createDateHour(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createDateHour(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createDateHour(0));
    vals->append(dolphindb::Util::createDateHour(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createDateHour(0));
    vals->append(dolphindb::Util::createDateHour(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0            #1           \n------------- -------------\n" + v1->getColumn(0)->get(0)->getString() + " " + v1->getColumn(1)->get(0)->getString() + "\n" + v1->getColumn(0)->get(1)->getString() + " " + v1->getColumn(1)->get(1)->getString() + "\n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createDateHour(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createDateHour(0));
    winMatrix1->set(1, 0, dolphindb::Util::createDateHour(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testDatehourNullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATEHOUR, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_DATEHOUR));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_DATEHOUR));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_DATEHOUR));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_DATEHOUR));
    std::string script = "a=matrix([[datehour(NULL),datehour(NULL)],[datehour(NULL),datehour(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDateMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATE, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createDate(1));
    v1->set(1, 0, dolphindb::Util::createDate(1));
    v1->set(0, 1, dolphindb::Util::createDate(0));
    v1->set(1, 1, dolphindb::Util::createDate(0));
    std::string script = "a=matrix([[date(1),date(0)],[date(1),date(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createDate(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createDate(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createDate(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_DATE, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createDate(0));
    vals->append(dolphindb::Util::createDate(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createDate(0));
    vals->append(dolphindb::Util::createDate(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0         #1        \n---------- ----------\n1970.01.01 1970.01.01\n1970.01.01 1970.01.01\n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createDate(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createDate(0));
    winMatrix1->set(1, 0, dolphindb::Util::createDate(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testDatenullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATE, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_DATE));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_DATE));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_DATE));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_DATE));
    std::string script = "a=matrix([[date(NULL),date(NULL)],[date(NULL),date(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testMinuteMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_MINUTE, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createMinute(1));
    v1->set(1, 0, dolphindb::Util::createMinute(1));
    v1->set(0, 1, dolphindb::Util::createMinute(0));
    v1->set(1, 1, dolphindb::Util::createMinute(0));
    std::string script = "a=matrix([[minute(1),minute(0)],[minute(1),minute(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createMinute(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createMinute(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createMinute(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_MINUTE, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createMinute(0));
    vals->append(dolphindb::Util::createMinute(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createMinute(0));
    vals->append(dolphindb::Util::createMinute(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0     #1    \n------ ------\n00:00m 00:00m\n00:00m 00:00m\n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createMinute(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createMinute(0));
    winMatrix1->set(1, 0, dolphindb::Util::createMinute(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testMinutenullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_MINUTE, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_MINUTE));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_MINUTE));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_MINUTE));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_MINUTE));
    std::string script = "a=matrix([[minute(NULL),minute(NULL)],[minute(NULL),minute(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testSecondMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_SECOND, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createSecond(1));
    v1->set(1, 0, dolphindb::Util::createSecond(1));
    v1->set(0, 1, dolphindb::Util::createSecond(0));
    v1->set(1, 1, dolphindb::Util::createSecond(0));
    std::string script = "a=matrix([[second(1),second(0)],[second(1),second(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createSecond(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createSecond(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createSecond(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_SECOND, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createSecond(0));
    vals->append(dolphindb::Util::createSecond(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createSecond(0));
    vals->append(dolphindb::Util::createSecond(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0       #1      \n-------- --------\n00:00:00 00:00:00\n00:00:00 00:00:00\n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createSecond(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createSecond(0));
    winMatrix1->set(1, 0, dolphindb::Util::createSecond(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testSecondnullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_SECOND, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_SECOND));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_SECOND));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_SECOND));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_SECOND));
    std::string script = "a=matrix([[second(NULL),second(NULL)],[second(NULL),second(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDatetimeMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATETIME, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createDateTime(1));
    v1->set(1, 0, dolphindb::Util::createDateTime(1));
    v1->set(0, 1, dolphindb::Util::createDateTime(0));
    v1->set(1, 1, dolphindb::Util::createDateTime(0));
    std::string script = "a=matrix([[datetime(1),datetime(0)],[datetime(1),datetime(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createDateTime(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createDateTime(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createDateTime(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_DATETIME, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createDateTime(0));
    vals->append(dolphindb::Util::createDateTime(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createDateTime(0));
    vals->append(dolphindb::Util::createDateTime(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0                  #1                 \n------------------- -------------------\n1970.01.01T00:00:00 1970.01.01T00:00:00\n1970.01.01T00:00:00 1970.01.01T00:00:00\n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createDateTime(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createDateTime(0));
    winMatrix1->set(1, 0, dolphindb::Util::createDateTime(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testDatetimenullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATETIME, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_DATETIME));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_DATETIME));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_DATETIME));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_DATETIME));
    std::string script = "a=matrix([[datetime(NULL),datetime(NULL)],[datetime(NULL),datetime(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testTimeStampMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_TIMESTAMP, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createTimestamp(1));
    v1->set(1, 0, dolphindb::Util::createTimestamp(1));
    v1->set(0, 1, dolphindb::Util::createTimestamp(0));
    v1->set(1, 1, dolphindb::Util::createTimestamp(0));
    std::string script = "a=matrix([[timestamp(1),timestamp(0)],[timestamp(1),timestamp(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createTimestamp(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createTimestamp(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createTimestamp(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createTimestamp(0));
    vals->append(dolphindb::Util::createTimestamp(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createTimestamp(0));
    vals->append(dolphindb::Util::createTimestamp(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0                      #1                     \n----------------------- -----------------------\n1970.01.01T00:00:00.000 1970.01.01T00:00:00.000\n1970.01.01T00:00:00.000 1970.01.01T00:00:00.000\n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createTimestamp(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createTimestamp(0));
    winMatrix1->set(1, 0, dolphindb::Util::createTimestamp(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testTimeStampnullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_TIMESTAMP, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_TIMESTAMP));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_TIMESTAMP));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_TIMESTAMP));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_TIMESTAMP));
    std::string script = "a=matrix([[timestamp(NULL),timestamp(NULL)],[timestamp(NULL),timestamp(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testnanotimeMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_NANOTIME, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNanoTime(1));
    v1->set(1, 0, dolphindb::Util::createNanoTime(1));
    v1->set(0, 1, dolphindb::Util::createNanoTime(0));
    v1->set(1, 1, dolphindb::Util::createNanoTime(0));
    std::string script = "a=matrix([[nanotime(1),nanotime(0)],[nanotime(1),nanotime(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createNanoTime(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createNanoTime(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createNanoTime(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createNanoTime(0));
    vals->append(dolphindb::Util::createNanoTime(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createNanoTime(0));
    vals->append(dolphindb::Util::createNanoTime(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0                 #1                \n------------------ ------------------\n00:00:00.000000000 00:00:00.000000000\n00:00:00.000000000 00:00:00.000000000\n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createNanoTime(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createNanoTime(0));
    winMatrix1->set(1, 0, dolphindb::Util::createNanoTime(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testnanotimenullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_NANOTIME, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIME));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIME));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIME));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIME));
    std::string script = "a=matrix([[nanotime(NULL),nanotime(NULL)],[nanotime(NULL),nanotime(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testnanotimestampMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_NANOTIMESTAMP, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNanoTimestamp(1));
    v1->set(1, 0, dolphindb::Util::createNanoTimestamp(1));
    v1->set(0, 1, dolphindb::Util::createNanoTimestamp(0));
    v1->set(1, 1, dolphindb::Util::createNanoTimestamp(0));
    std::string script = "a=matrix([[nanotimestamp(1),nanotimestamp(0)],[nanotimestamp(1),nanotimestamp(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createNanoTimestamp(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createNanoTimestamp(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createNanoTimestamp(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createNanoTimestamp(0));
    vals->append(dolphindb::Util::createNanoTimestamp(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createNanoTimestamp(0));
    vals->append(dolphindb::Util::createNanoTimestamp(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0                        #1                       \n------------------------- -------------------------\n1970.01.01T00:00:00.000...1970.01.01T00:00:00.00...\n1970.01.01T00:00:00.000...1970.01.01T00:00:00.00...\n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createNanoTimestamp(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createNanoTimestamp(0));
    winMatrix1->set(1, 0, dolphindb::Util::createNanoTimestamp(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testnanotimestampnullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_NANOTIMESTAMP, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIMESTAMP));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIMESTAMP));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIMESTAMP));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_NANOTIMESTAMP));
    std::string script = "a=matrix([[nanotimestamp(NULL),nanotimestamp(NULL)],[nanotimestamp(NULL),nanotimestamp(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testmonthMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_MONTH, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createMonth(1));
    v1->set(1, 0, dolphindb::Util::createMonth(1));
    v1->set(0, 1, dolphindb::Util::createMonth(0));
    v1->set(1, 1, dolphindb::Util::createMonth(0));
    std::string script = "a=matrix([[month(1),month(0)],[month(1),month(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createMonth(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createMonth(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createMonth(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_MONTH, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createMonth(0));
    vals->append(dolphindb::Util::createMonth(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createMonth(0));
    vals->append(dolphindb::Util::createMonth(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0       #1      \n-------- --------\n0000.01M 0000.01M\n0000.01M 0000.01M\n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createMonth(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createMonth(0));
    winMatrix1->set(1, 0, dolphindb::Util::createMonth(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testmonthnullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_MONTH, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_MONTH));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_MONTH));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_MONTH));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_MONTH));
    std::string script = "a=matrix([[month(NULL),month(NULL)],[month(NULL),month(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testtimeMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_TIME, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createTime(1));
    v1->set(1, 0, dolphindb::Util::createTime(1));
    v1->set(0, 1, dolphindb::Util::createTime(0));
    v1->set(1, 1, dolphindb::Util::createTime(0));
    std::string script = "a=matrix([[time(1),time(0)],[time(1),time(0)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);

    ASSERT_FALSE(v1->reshape(2, 1));

    v1->reshape(1, 4);
    ASSERT_EQ(v1->rows(), 4);
    ASSERT_EQ(v1->columns(), 1);
    v1->reshape(4, 1);
    ASSERT_EQ(v1->rows(), 1);
    ASSERT_EQ(v1->columns(), 4);
    v1->reshape(2, 2);
    ASSERT_EQ(v1->rows(), 2);
    ASSERT_EQ(v1->columns(), 2);
    ASSERT_EQ(v1->getRowLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());
    ASSERT_EQ(v1->getColumnLabel()->getString(), dolphindb::Util::createNullConstant(dolphindb::DT_STRING)->getString());

    ASSERT_EQ(v1->getString(1), "{" + v1->getColumn(1)->get(0)->getString() + "," + v1->getColumn(1)->get(1)->getString() + "}");
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(3), dolphindb::Util::createTime(1)));
    ASSERT_ANY_THROW(v1->set(dolphindb::Util::createInt(-1), dolphindb::Util::createTime(1)));
    v1->set(dolphindb::Util::createInt(1), dolphindb::Util::createTime(1));
    ASSERT_EQ(v1->getColumn(1)->get(1)->getBool(), 1);
    dolphindb::VectorSP vals = dolphindb::Util::createVector(dolphindb::DT_TIME, 0, 2);
    dolphindb::VectorSP cols = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 2);
    cols->append(dolphindb::Util::createInt(0));
    cols->append(dolphindb::Util::createInt(1));
    vals->append(dolphindb::Util::createTime(0));
    vals->append(dolphindb::Util::createTime(0));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    vals->append(dolphindb::Util::createTime(0));
    vals->append(dolphindb::Util::createTime(0));
    v1->set(cols, vals);
    ASSERT_EQ(v1->getString(), "#0           #1          \n------------ ------------\n00:00:00.000 00:00:00.000\n00:00:00.000 00:00:00.000\n");
    cols->append(dolphindb::Util::createInt(-1));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    cols->set(2, dolphindb::Util::createInt(3));
    ASSERT_ANY_THROW(v1->set(cols, vals));
    // dolphindb::ConstantSP constCol = dolphindb::Util::createInt(0);
    // ASSERT_EQ(v1->get(constCol)->getString(),"#0\n--\n0 \n0 \n");

    v1->setItem(0, dolphindb::Util::createTime(1));
    ASSERT_EQ(v1->getItem(0)->getString(), "[" + v1->getColumn(0)->get(0)->getString() + "," + v1->getColumn(0)->get(1)->getString() + "]");

    dolphindb::VectorSP v2 = v1->getInstance(4);
    ASSERT_EQ(v2->rows(), v1->rows());
    ASSERT_EQ(v2->columns(), 4);
    dolphindb::VectorSP v3 = v1->getInstance();
    ASSERT_EQ(v3->rows(), v1->rows());
    ASSERT_EQ(v3->columns(), v1->columns());

    std::cout << v1->getString() << std::endl;
    dolphindb::VectorSP rowlabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowlabelvec->append(dolphindb::Util::createString("lab0"));
    rowlabelvec->append(dolphindb::Util::createString("lab1"));
    ASSERT_ANY_THROW(v1->setRowLabel(dolphindb::Util::createString("lab")));
    v1->setRowLabel(rowlabelvec);
    dolphindb::VectorSP collabelvec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    collabelvec->append(dolphindb::Util::createString("collab0"));
    collabelvec->append(dolphindb::Util::createString("collab1"));
    ASSERT_ANY_THROW(v1->setColumnLabel(dolphindb::Util::createString("lab")));
    v1->setColumnLabel(collabelvec);

    ASSERT_EQ(v1->getRow(0)->getString(), "[" + v1->getRow(0)->get(0)->getString() + "," + v1->getRow(0)->get(1)->getString() + "]");
    dolphindb::VectorSP winMatrix1 = v1->getWindow(0, 2, 0, 1);
    winMatrix1->set(0, 0, dolphindb::Util::createTime(0));
    winMatrix1->set(1, 0, dolphindb::Util::createTime(1));
    ASSERT_EQ(winMatrix1->getValue(2)->getString(), "[" + winMatrix1->getRow(0)->get(0)->getString() + "," + winMatrix1->getRow(0)->get(1)->getString() + "]");
}

TEST_F(DataformMatrixTest, testtimenullMatrix)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_TIME, 2, 2, 4);
    v1->set(0, 0, dolphindb::Util::createNullConstant(dolphindb::DT_TIME));
    v1->set(1, 0, dolphindb::Util::createNullConstant(dolphindb::DT_TIME));
    v1->set(0, 1, dolphindb::Util::createNullConstant(dolphindb::DT_TIME));
    v1->set(1, 1, dolphindb::Util::createNullConstant(dolphindb::DT_TIME));
    std::string script = "a=matrix([[time(NULL),time(NULL)],[time(NULL),time(NULL)]]);a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getString(), res_v->getString());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testBoolMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_BOOL, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createBool(true));
        }
    }
    std::string script = "a=matrix(BOOL,300,300,90000,bool(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testCharMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_CHAR, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createChar(1));
        }
    }
    std::string script = "a=matrix(CHAR,300,300,90000,char(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testIntMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_INT, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createInt(1));
        }
    }
    std::string script = "a=matrix(INT,300,300,90000,int(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testLongMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_LONG, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createLong(1));
        }
    }
    std::string script = "a=matrix(LONG,300,300,90000,long(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testShortMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_SHORT, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createShort(1));
        }
    }
    std::string script = "a=matrix(SHORT,300,300,90000,short(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testFloatMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_FLOAT, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createFloat(1));
        }
    }
    std::string script = "a=matrix(FLOAT,300,300,90000,float(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDoubleMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DOUBLE, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createDouble(1));
        }
    }
    std::string script = "a=matrix(DOUBLE,300,300,90000,double(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDatehourMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATEHOUR, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createDateHour(1));
        }
    }
    std::string script = "a=matrix(DATEHOUR,300,300,90000,datehour(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDateMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATE, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createDate(1));
        }
    }
    std::string script = "a=matrix(DATE,300,300,90000,date(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testMinuteMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_MINUTE, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createMinute(1));
        }
    }
    std::string script = "a=matrix(MINUTE,300,300,90000,minute(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDatetimeMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATETIME, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createDateTime(1));
        }
    }
    std::string script = "a=matrix(DATETIME,300,300,90000,datetime(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testTimeStampMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_TIMESTAMP, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createTimestamp(1));
        }
    }
    std::string script = "a=matrix(TIMESTAMP,300,300,90000,timestamp(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testnanotimeMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_NANOTIME, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createNanoTime(1));
        }
    }
    std::string script = "a=matrix(NANOTIME,300,300,90000,nanotime(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testnanotimestampMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_NANOTIMESTAMP, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createNanoTimestamp(1));
        }
    }
    std::string script = "a=matrix(NANOTIMESTAMP,300,300,90000,nanotimestamp(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testmonthMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_MONTH, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createMonth(1));
        }
    }
    std::string script = "a=matrix(MONTH,300,300,90000,month(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testtimeMatrixMoreThan65535)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_TIME, 300, 300, 90000);
    for (int i = 0; i < 300; i++)
    {
        for (int j = 0; j < 300; j++)
        {
            v1->set(i, j, dolphindb::Util::createTime(1));
        }
    }
    std::string script = "a=matrix(TIME,300,300,90000,time(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testBoolMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_BOOL, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createBool(true));
        }
    }
    std::string script = "a=matrix(BOOL,1100,1100,1210000,bool(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testCharMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_CHAR, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createChar(1));
        }
    }
    std::string script = "a=matrix(CHAR,1100,1100,1210000,char(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testIntMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_INT, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createInt(1));
        }
    }
    std::string script = "a=matrix(INT,1100,1100,1210000,int(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testLongMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_LONG, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createLong(1));
        }
    }
    std::string script = "a=matrix(LONG,1100,1100,1210000,long(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testShortMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_SHORT, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createShort(1));
        }
    }
    std::string script = "a=matrix(SHORT,1100,1100,1210000,short(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testFloatMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_FLOAT, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createFloat(1));
        }
    }
    std::string script = "a=matrix(FLOAT,1100,1100,1210000,float(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDoubleMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DOUBLE, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createDouble(1));
        }
    }
    std::string script = "a=matrix(DOUBLE,1100,1100,1210000,double(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDatehourMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATEHOUR, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createDateHour(1));
        }
    }
    std::string script = "a=matrix(DATEHOUR,1100,1100,1210000,datehour(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDateMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATE, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createDate(1));
        }
    }
    std::string script = "a=matrix(DATE,1100,1100,1210000,date(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testMinuteMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_MINUTE, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createMinute(1));
        }
    }
    std::string script = "a=matrix(MINUTE,1100,1100,1210000,minute(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testDatetimeMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_DATETIME, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createDateTime(1));
        }
    }
    std::string script = "a=matrix(DATETIME,1100,1100,1210000,datetime(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testTimeStampMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_TIMESTAMP, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createTimestamp(1));
        }
    }
    std::string script = "a=matrix(TIMESTAMP,1100,1100,1210000,timestamp(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testnanotimeMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_NANOTIME, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createNanoTime(1));
        }
    }
    std::string script = "a=matrix(NANOTIME,1100,1100,1210000,nanotime(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testnanotimestampMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_NANOTIMESTAMP, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createNanoTimestamp(1));
        }
    }
    std::string script = "a=matrix(NANOTIMESTAMP,1100,1100,1210000,nanotimestamp(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testmonthMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_MONTH, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createMonth(1));
        }
    }
    std::string script = "a=matrix(MONTH,1100,1100,1210000,month(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, testtimeMatrixMoreThan1048576)
{
    dolphindb::VectorSP v1 = dolphindb::Util::createMatrix(dolphindb::DT_TIME, 1100, 1100, 1210000);
    for (int i = 0; i < 1100; i++)
    {
        for (int j = 0; j < 1100; j++)
        {
            v1->set(i, j, dolphindb::Util::createTime(1));
        }
    }
    std::string script = "a=matrix(TIME,1100,1100,1210000,time(1));a";
    dolphindb::VectorSP res_v = conn.run(script);
    conn.upload("v1", {v1});
    ASSERT_EQ(conn.run("eqObj(v1,a)")->getBool(), true);
    ASSERT_EQ(v1->getScript(), res_v->getScript());
    ASSERT_EQ(v1->getType(), res_v->getType());
    ASSERT_EQ(conn.run("v1")->isMatrix(), true);
}

TEST_F(DataformMatrixTest, test_Indexedmatrix)
{
    dolphindb::VectorSP mat = conn.run("m=matrix(1..5, 11..15);\
                m.rename!(2020.01.01..2020.01.05, `A`B);\
                m.setIndexedMatrix!();m");
    ASSERT_EQ(dolphindb::Util::getDataFormString(mat->getForm()), "MATRIX");
    ASSERT_EQ(mat->getColumnLabel()->getString(), "[\"A\",\"B\"]");
    ASSERT_EQ(mat->getRowLabel()->getString(), "[2020.01.01,2020.01.02,2020.01.03,2020.01.04,2020.01.05]");
    ASSERT_EQ(mat->getString(), "           A B \n"
                                "           - --\n"
                                "2020.01.01|1 11\n"
                                "2020.01.02|2 12\n"
                                "2020.01.03|3 13\n"
                                "2020.01.04|4 14\n"
                                "2020.01.05|5 15\n");
}

TEST_F(DataformMatrixTest, test_IndexedSeries)
{
    dolphindb::VectorSP series = conn.run("s1 = indexedSeries(2012.01.01..2012.01.04, [10, 20, 30, 40]);\
                            s2 = indexedSeries(2011.12.30..2012.01.01, [50, 60, 70]);\
                            s1 + s2");
    ASSERT_EQ(dolphindb::Util::getDataFormString(series->getForm()), "MATRIX");
    ASSERT_EQ(series->getColumnLabel()->getString(), "");
    ASSERT_EQ(series->getRowLabel()->getString(), "[2011.12.30,2011.12.31,2012.01.01,2012.01.02,2012.01.03,2012.01.04]");
    ASSERT_EQ(series->getString(), "           #0\n"
                                   "           --\n"
                                   "2011.12.30|  \n"
                                   "2011.12.31|  \n"
                                   "2012.01.01|80\n"
                                   "2012.01.02|  \n"
                                   "2012.01.03|  \n"
                                   "2012.01.04|  \n");
}

TEST_F(DataformMatrixTest, test_Indexedmatrix_gt1048576)
{
    dolphindb::VectorSP mat = conn.run("m=matrix(1..1100000, -1..-1100000);\
                m.rename!(datetime(1..1100000), `A`B);\
                m.setIndexedMatrix!();m");
    ASSERT_EQ(dolphindb::Util::getDataFormString(mat->getForm()), "MATRIX");
    ASSERT_EQ(mat->getColumnLabel()->getString(), "[\"A\",\"B\"]");
    dolphindb::VectorSP row_labelv = mat->getRowLabel();
    for (auto i = 0; i < row_labelv->size(); i++)
    {
        ASSERT_EQ(row_labelv->get(i)->getString(), dolphindb::Util::createDateTime(i + 1)->getString());
    }
    ASSERT_EQ(mat->getString(), "                    A  B  \n                    -- ---\n1970.01.01T00:00:01|1  -1 \n1970.01.01T00:00:02|2  -2 \n1970.01.01T00:00:03|3  -3 \n1970.01.01T00:00:04|4  -4 \n1970.01.01T00:00:05|5  -5 \n1970.01.01T00:00:06|6  -6 \n1970.01.01T00:00:07|7  -7 \n1970.01.01T00:00:08|8  -8 \n1970.01.01T00:00:09|9  -9 \n1970.01.01T00:00:10|10 -10\n1970.01.01T00:00:11|11 -11\n1970.01.01T00:00:12|12 -12\n1970.01.01T00:00:13|13 -13\n1970.01.01T00:00:14|14 -14\n1970.01.01T00:00:15|15 -15\n1970.01.01T00:00:16|16 -16\n1970.01.01T00:00:17|17 -17\n1970.01.01T00:00:18|18 -18\n1970.01.01T00:00:19|19 -19\n1970.01.01T00:00:20|20 -20\n1970.01.01T00:00:21|21 -21\n1970.01.01T00:00:22|22 -22\n1970.01.01T00:00:23|23 -23\n1970.01.01T00:00:24|24 -24\n1970.01.01T00:00:25|25 -25\n1970.01.01T00:00:26|26 -26\n1970.01.01T00:00:27|27 -27\n1970.01.01T00:00:28|28 -28\n1970.01.01T00:00:29|29 -29\n1970.01.01T00:00:30|30 -30\n...\n");
}

TEST_F(DataformMatrixTest, test_IndexedSeries_gt1048576)
{
    dolphindb::VectorSP series = conn.run("s1 = indexedSeries(date(1..1100000), 1..1100000);\
                            s2 = indexedSeries(date(1100001..2200000), 1100001..2200000);\
                            s1 + s2");
    ASSERT_EQ(dolphindb::Util::getDataFormString(series->getForm()), "MATRIX");
    ASSERT_EQ(series->getColumnLabel()->getString(), "");
    dolphindb::VectorSP row_labelv = series->getRowLabel();
    for (auto i = 0; i < row_labelv->size(); i++)
    {
        ASSERT_EQ(row_labelv->get(i)->getString(), dolphindb::Util::createDate(i + 1)->getString());
    }
    ASSERT_EQ(series->getString(), "           #0\n           --\n1970.01.02|  \n1970.01.03|  \n1970.01.04|  \n1970.01.05|  \n1970.01.06|  \n1970.01.07|  \n1970.01.08|  \n1970.01.09|  \n1970.01.10|  \n1970.01.11|  \n1970.01.12|  \n1970.01.13|  \n1970.01.14|  \n1970.01.15|  \n1970.01.16|  \n1970.01.17|  \n1970.01.18|  \n1970.01.19|  \n1970.01.20|  \n1970.01.21|  \n1970.01.22|  \n1970.01.23|  \n1970.01.24|  \n1970.01.25|  \n1970.01.26|  \n1970.01.27|  \n1970.01.28|  \n1970.01.29|  \n1970.01.30|  \n1970.01.31|  \n...\n");
}

class DataformMatrixTest_funcs : public DataformMatrixTest, public testing::WithParamInterface<std::string>
{
public:
    static const std::vector<std::string> getData(){
        return {
            "matrix(true false, true false)",
            "matrix(1c 2c, 3c 4c)",
            "matrix(1h 2h, 3h 4h)",
            "matrix(1 2, 3 4)",
            "matrix(1l 2l, 4l 5l)",
            "matrix(1.0f 2.2f, 3.3f 4.4f)",
            "matrix(1.0 2.2, 3.3 4.4)",
            "matrix(2020.01.01 2020.01.02, 2020.01.03 2020.01.04)",
            "matrix(2020.01.01T00:00:01 2020.01.01T00:00:02, 2020.01.01T00:00:03 2020.01.01T00:00:04)",
            "matrix(datehour(1234 5555), datehour(7894 1346))",
            "matrix(2020.01M 2020.02M, 2020.03M 2020.04M)",
            "matrix(00:00:01.000 00:00:02.000, 00:00:03.000 00:00:04.000)",
            "matrix(00:00:01 00:00:02, 00:00:03 00:00:04)",
            "matrix(00:01m 00:02m, 00:03m 00:04m)",
            "matrix(00:00:01.000000000 00:00:02.000000000, 00:00:03.000000000 00:00:04.000000000)",
            "matrix(2020.01.01T00:00:01.000 2020.01.01T00:00:02.000, 2020.01.01T00:00:03.000 2020.01.01T00:00:04.000)",
            "matrix(2020.01.01T00:00:01.000000000 2020.01.01T00:00:02.000000000, 2020.01.01T00:00:03.000000000 2020.01.01T00:00:04.000000000)",
        };
    }
};

INSTANTIATE_TEST_SUITE_P(, DataformMatrixTest_funcs, testing::ValuesIn(DataformMatrixTest_funcs::getData()));
TEST_P(DataformMatrixTest_funcs, test_getWindow_getRow_getColumn){
    dolphindb::VectorSP m = conn.run("x="+GetParam()+";x");
    std::cout << m->getString() << std::endl;
    std::string *res = new std::string();
    std::string *ex = new std::string();

    /* getWindow */
    *res = m->getWindow(0, -1, 1, 0)->getString();
    // std::cout << *res << std::endl;
    *ex = conn.run("x[0:0,-1:0]")->getString();
    ASSERT_EQ(*res, *ex);

    *res = m->getWindow(0, -1, 1, 1)->getString();
    // std::cout << *res << std::endl;
    *ex = conn.run("x[1:2, 0:1]")->getString();
    ASSERT_EQ(*res, *ex);

    *res = m->getWindow(0, -1, 1, 2)->getString();
    // std::cout << *res << std::endl;
    *ex = conn.run("x[1:3, 0:1]")->getString();
    ASSERT_EQ(*res, *ex);

    *res = m->getWindow(0, -1, 0, -1)->getString();
    // std::cout << *res << std::endl;
    *ex = conn.run("x[[0],[0]]")->getString();
    ASSERT_EQ(*res, *ex);

    /* getRow */
    *res = m->getRow(1)->getString();
    // std::cout << *res << std::endl;
    *ex = conn.run("flatten(x[[1],])")->getString();
    ASSERT_EQ(*res, *ex);

    *res = m->getRow(2)->getString();
    // std::cout << *res << std::endl;
    *ex = conn.run("flatten(x[[2],])")->getString();
    ASSERT_EQ(*res, *ex);

    /* getColumn */
    *res = m->getColumn(1)->getString();
    // std::cout << *res << std::endl;
    *ex = conn.run("flatten(x[,[1]])")->getString();
    ASSERT_EQ(*res, *ex);

    ASSERT_ANY_THROW(*res = m->getColumn(2)->getString());
    // std::cout << *res << std::endl;
    // *ex = conn.run("flatten(x[,[2]])")->getString();

    // setLabel
    dolphindb::VectorSP rowLabel = dolphindb::Util::createVector(dolphindb::DT_STRING, 2);
    dolphindb::VectorSP colLabel = dolphindb::Util::createVector(dolphindb::DT_STRING, 2);
    colLabel->setString(0, "c1");
    colLabel->setString(1, "c2");
    m->setColumnLabel(colLabel);
    rowLabel->setString(0, "r1");
    rowLabel->setString(1, "r2");
    m->setRowLabel(rowLabel);
    conn.run("x.rename!(`r1`r2, `c1`c2)");

    *res = m->getColumn(1)->getString();
    *ex = conn.run("flatten(x[,[1]])")->getString();
    ASSERT_EQ(*res, *ex);

    *res = m->getRow(1)->getString();
    *ex = conn.run("flatten(x[[1],])")->getString();
    ASSERT_EQ(*res, *ex);
    delete res, ex;
}

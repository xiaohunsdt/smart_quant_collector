#include <gtest/gtest.h>
#include "config.h"

class CompressTest : public testing::Test
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

dolphindb::DBConnection CompressTest::conn(false, false, 7200, true);

TEST_F(CompressTest,CompressLong){
    const int count = 60000;
    std::vector<std::string> colName={"time","value"};
    std::vector<int> time(count);
    std::vector<long long>value(count);

    int basetime = dolphindb::Util::countDays(2012,1,1);
    for(int i=0;i<count;i++){
        time[i] = basetime + (i%15);
        value[i] = i;
    }

    dolphindb::VectorSP timeVector = dolphindb::Util::createVector(dolphindb::DT_DATE,count, count);
    dolphindb::VectorSP valueVector = dolphindb::Util::createVector(dolphindb::DT_LONG, count,count);
    timeVector->setInt(0,count,time.data());
    valueVector->setLong(0,count,value.data());
    std::vector<dolphindb::ConstantSP> colVector {timeVector,valueVector};
    dolphindb::TableSP table  = dolphindb::Util::createTable(colName,colVector);
    std::vector<dolphindb::COMPRESS_METHOD> typeVec{dolphindb::COMPRESS_DELTA, dolphindb::COMPRESS_DELTA};
    table->setColumnCompressMethods(typeVec);

    std::vector<dolphindb::ConstantSP> args{table};
    conn.run("table1=streamTable(1:0, `time`value,[DATE,LONG])");
    int success = conn.run("tableInsert{table1}",args)->getInt();
    ASSERT_EQ(success, count);

    dolphindb::TableSP t1 = conn.run("select * from table1");
    ASSERT_EQ(t1->getString(),table->getString());

}

TEST_F(CompressTest,CompressVectorLongerThanTable){
    const int count = 60000;
    std::vector<std::string> colName={"time","value"};
    std::vector<int> time(count);
    std::vector<long long>value(count);

    int basetime = dolphindb::Util::countDays(2012,1,1);
    for(int i=0;i<count;i++){
        time[i] = basetime + (i%15);
        value[i] = i;
    }

    dolphindb::VectorSP timeVector = dolphindb::Util::createVector(dolphindb::DT_DATE,count, count);
    dolphindb::VectorSP valueVector = dolphindb::Util::createVector(dolphindb::DT_LONG, count,count);
    timeVector->setInt(0,0,time.data());
    valueVector->setLong(0,0,value.data());
    std::vector<dolphindb::ConstantSP> colVector {timeVector,valueVector};
    dolphindb::TableSP table  = dolphindb::Util::createTable(colName,colVector);
    std::vector<dolphindb::COMPRESS_METHOD> typeVec{dolphindb::COMPRESS_DELTA, dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_LZ4};
    ASSERT_ANY_THROW(table->setColumnCompressMethods(typeVec));
}

TEST_F(CompressTest,CompressVectorLessThanTable){
    const int count = 600000;
    std::vector<std::string> colName={"time","value"};
    std::vector<int> time(count);
    std::vector<long long>value(count);

    int basetime = dolphindb::Util::countDays(2012,1,1);
    for(int i=0;i<count;i++){
        time[i] = basetime + (i%15);
        value[i] = i;
    }

    dolphindb::VectorSP timeVector = dolphindb::Util::createVector(dolphindb::DT_DATE,count, count);
    dolphindb::VectorSP valueVector = dolphindb::Util::createVector(dolphindb::DT_LONG, count,count);
    timeVector->setInt(0,count,time.data());
    valueVector->setLong(0,count,value.data());
    std::vector<dolphindb::ConstantSP> colVector {timeVector,valueVector};
    dolphindb::TableSP table  = dolphindb::Util::createTable(colName,colVector);
    std::vector<dolphindb::COMPRESS_METHOD> typeVec{dolphindb::COMPRESS_METHOD::COMPRESS_DELTA};
    ASSERT_ANY_THROW(table->setColumnCompressMethods(typeVec));
}

TEST_F(CompressTest,CompressWithErrorDataType){
    const int count = 600000;
    std::vector<std::string> colName={"time","value","name"};
    std::vector<int> time(count);
    std::vector<long long>value(count);
    std::vector<std::string> name(count);

    int basetime = dolphindb::Util::countDays(2012,1,1);
    for(int i=0;i<count;i++){
        time[i] = basetime + (i%15);
        value[i] = i;
        name[i] = std::to_string(i);
    }

    dolphindb::VectorSP timeVector = dolphindb::Util::createVector(dolphindb::DT_DATE,count, count);
    dolphindb::VectorSP valueVector = dolphindb::Util::createVector(dolphindb::DT_LONG, count,count);
    dolphindb::VectorSP nameVector = dolphindb::Util::createVector(dolphindb::DT_STRING,count,count,count);
    timeVector->setInt(0,count,time.data());
    valueVector->setLong(0,count,value.data());
    nameVector->setString(0,count,name.data());
    std::vector<dolphindb::ConstantSP> colVector {timeVector,valueVector,nameVector};
    dolphindb::TableSP table  = dolphindb::Util::createTable(colName,colVector);
    std::vector<dolphindb::COMPRESS_METHOD> typeVec{dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_LZ4,dolphindb::COMPRESS_DELTA};
    ASSERT_ANY_THROW(table->setColumnCompressMethods(typeVec));
}

TEST_F(CompressTest,CompressIncludeNull){
    const int count = 600000;
    std::vector<std::string> colName={"time","value","name"};
    std::vector<int> time(count);
    std::vector<long long>value(count);
    std::vector<std::string> name(count);

    int basetime = dolphindb::Util::countDays(2012,1,1);
    for(int i=0;i<count;i++){
        time[i] = basetime + (i%15);
        value[i] = (i % 500 == 0) ? i : (long long) nullptr ;
        name[i] = std::to_string(i);
    }

    dolphindb::VectorSP timeVector = dolphindb::Util::createVector(dolphindb::DT_DATE,count, count);
    dolphindb::VectorSP valueVector = dolphindb::Util::createVector(dolphindb::DT_LONG, count,count);
    dolphindb::VectorSP nameVector = dolphindb::Util::createVector(dolphindb::DT_STRING,count,count,count);
    timeVector->setInt(0,count,time.data());
    valueVector->setLong(0,count,value.data());
    nameVector->setString(0,count,name.data());
    std::vector<dolphindb::ConstantSP> colVector {timeVector,valueVector,nameVector};
    dolphindb::TableSP table  = dolphindb::Util::createTable(colName,colVector);
    std::vector<dolphindb::COMPRESS_METHOD> typeVec{dolphindb::COMPRESS_METHOD::COMPRESS_DELTA,dolphindb::COMPRESS_METHOD::COMPRESS_LZ4,dolphindb::COMPRESS_METHOD::COMPRESS_LZ4};
    table->setColumnCompressMethods(typeVec);

    std::vector<dolphindb::ConstantSP> args{table};
    conn.run("table1=streamTable(1:0, `time`value`name,[DATE,LONG,STRING])");
    int success = conn.run("tableInsert{table1}",args)->getInt();
    ASSERT_EQ(success, count);

    dolphindb::TableSP t1 = conn.run("select * from table1");
    ASSERT_EQ(t1->getString(),table->getString());
}

TEST_F(CompressTest,insertTableCompressWithAllType){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    const int count = 70000;
    const int scale32=rand()%9, scale64=rand()%18, scale128=rand()%38;

    std::vector<char> boolval(count);
    std::vector<char> chval(count);
    std::vector<short> shtval(count);
    std::vector<int> time(count);
    std::vector<long long>value(count);
    std::vector<float> cfloat(count);
    std::vector<double> cdouble(count);
    std::vector<std::string> name(count);
    std::vector<std::string> blob(count);
    std::vector<std::string> ipaddr(count);
    std::vector<double> decimal32(count);
    std::vector<double> decimal64(count);
    std::vector<double> decimal128(count);
    int basetime = dolphindb::Util::countDays(2012,1,1);
    for(int i=0;i<count-1;i++){
        boolval[i] = i%2==0;
        chval[i] = char(i);
        shtval[i] = i;
        time[i] = basetime + (i%15);
        value[i] = (i % 500 == 0) ? i : (long long) nullptr;
        cfloat[i] = float(i)+0.1;
        cdouble[i] = double(i)+0.1;
        name[i] = std::to_string(i);
        blob[i] = "fdsafsdfasd"+ std::to_string(i%32);
        ipaddr[i] = "192.168.100." + std::to_string(i%255);
        decimal32[i] = 0.1231555;
        decimal64[i] = 2.454223387226;
        decimal128[i] = (long double)2.454223387226111111111111111111;
    }
    // add null value to the last row
    boolval[count-1] = CHAR_MIN;
    chval[count-1] = CHAR_MIN;
    shtval[count-1] = SHRT_MIN;
    time[count-1] = INT_MIN;
    value[count-1] = LLONG_MIN;
    cfloat[count-1] = FLT_MIN;
    cdouble[count-1] = DBL_MIN;
    name[count-1] = "";
    blob[count-1] = "";
    ipaddr[count-1] = "";
    decimal32[count-1] = DBL_MIN;
    decimal64[count-1] = 0.0;
    decimal128[count-1] = 0.0;

    dolphindb::VectorSP boolVector = dolphindb::Util::createVector(dolphindb::DT_BOOL,count, count);
    dolphindb::VectorSP charVector = dolphindb::Util::createVector(dolphindb::DT_CHAR, count,count);
    dolphindb::VectorSP shortVector = dolphindb::Util::createVector(dolphindb::DT_SHORT,count,count);
    dolphindb::VectorSP intVector = dolphindb::Util::createVector(dolphindb::DT_INT,count,count);
    dolphindb::VectorSP dateVector = dolphindb::Util::createVector(dolphindb::DT_DATE,count,count);
    dolphindb::VectorSP monthVector = dolphindb::Util::createVector(dolphindb::DT_MONTH,count,count);
    dolphindb::VectorSP timeVector = dolphindb::Util::createVector(dolphindb::DT_TIME,count,count);
    dolphindb::VectorSP minuteVector = dolphindb::Util::createVector(dolphindb::DT_MINUTE,count,count);
    dolphindb::VectorSP secondVector = dolphindb::Util::createVector(dolphindb::DT_SECOND,count,count);
    dolphindb::VectorSP datetimeVector = dolphindb::Util::createVector(dolphindb::DT_DATETIME,count,count);
    dolphindb::VectorSP timestampVector = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP,count,count);
    dolphindb::VectorSP nanotimeVector = dolphindb::Util::createVector(dolphindb::DT_NANOTIME,count,count);
    dolphindb::VectorSP nanotimestampVector = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP,count,count);
    dolphindb::VectorSP floatVector = dolphindb::Util::createVector(dolphindb::DT_FLOAT,count,count);
    dolphindb::VectorSP doubleVector = dolphindb::Util::createVector(dolphindb::DT_DOUBLE,count,count);
    dolphindb::VectorSP symbolVector = dolphindb::Util::createVector(dolphindb::DT_SYMBOL,count,count);
    dolphindb::VectorSP stringVector = dolphindb::Util::createVector(dolphindb::DT_STRING,count,count);
    dolphindb::VectorSP ipaddrVector = dolphindb::Util::createVector(dolphindb::DT_IP, count,count);
    dolphindb::VectorSP blobVector = dolphindb::Util::createVector(dolphindb::DT_BLOB, count,count);
    dolphindb::VectorSP decimal32Vector = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, count,count, true, scale32);
    dolphindb::VectorSP decimal64Vector = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, count,count, true, scale64);
    dolphindb::VectorSP decimal128Vector = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, count,count, true, scale128);

    boolVector->setBool(0,count, boolval.data());
    charVector->setChar(0,count,chval.data());
    shortVector->setShort(0,count,shtval.data());
    intVector->setInt(0,count,time.data());
    dateVector->setInt(0,count,time.data());
    monthVector->setInt(0,count,time.data());
    timeVector->setInt(0,count,time.data());
    minuteVector->setInt(0,count,time.data());
    secondVector->setInt(0,count,time.data());
    datetimeVector->setInt(0,count,time.data());
    timestampVector->setLong(0,count,value.data());
    nanotimeVector->setLong(0,count,value.data());
    nanotimestampVector->setLong(0,count,value.data());
    floatVector->setFloat(0,count,cfloat.data());
    doubleVector->setDouble(0,count,cdouble.data());
    symbolVector->setString(0,count,name.data());
    stringVector->setString(0,count,name.data());
    ipaddrVector->setString(0,count,ipaddr.data());
    blobVector->setString(0,count,blob.data());
    decimal32Vector->setDouble(0,count,decimal32.data());
    decimal64Vector->setDouble(0,count,decimal64.data());
    decimal128Vector->setDouble(0,count,decimal128.data());

    dolphindb::VectorSP indV = dolphindb::Util::createIndexVector(0,count);
    std::vector<std::string> colName={"ind", "cbool","cchar","cshort","cint","cdate","cmonth","ctime","cminute", "csecond","cdatetime","ctimestamp","cnanotime",
                            "cnanotimestamp","cfloat","cdouble","csymbol","cstring","cipaddr","cblob","cdecimal32","cdecimal64", "cdecimal128"};
    std::vector<dolphindb::ConstantSP> colVector {indV, boolVector,charVector,shortVector,intVector,dateVector,monthVector,timeVector,minuteVector,secondVector,
                                  datetimeVector,timestampVector,nanotimeVector,nanotimestampVector,floatVector,doubleVector,symbolVector,stringVector,
                                  ipaddrVector,blobVector,decimal32Vector,decimal64Vector,decimal128Vector};
    dolphindb::TableSP table  = dolphindb::Util::createTable(colName,colVector);
    std::vector<dolphindb::COMPRESS_METHOD> typeVec{dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_LZ4,dolphindb::COMPRESS_LZ4,dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_DELTA,
                                    dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_DELTA,
                                    dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_DELTA,
                                    dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_LZ4,dolphindb::COMPRESS_LZ4,dolphindb::COMPRESS_LZ4,
                                    dolphindb::COMPRESS_LZ4,dolphindb::COMPRESS_LZ4,dolphindb::COMPRESS_LZ4,dolphindb::COMPRESS_LZ4,dolphindb::COMPRESS_LZ4, dolphindb::COMPRESS_LZ4};
    table->setColumnCompressMethods(typeVec);

    std::vector<dolphindb::ConstantSP> args{table};
    std::string script = "colName =  `ind`cbool`cchar`cshort`cint`cdate`cmonth`ctime`cminute`csecond`cdatetime`ctimestamp`cnanotime`cnanotimestamp`cfloat`cdouble`csymbol`cstring`cipaddr`cblob`cdecimal32`cdecimal64`cdecimal128;\n"
                    "colType = [INT, BOOL, CHAR, SHORT, INT, DATE, MONTH, TIME, MINUTE, SECOND, DATETIME, TIMESTAMP, NANOTIME, NANOTIMESTAMP, FLOAT, DOUBLE, SYMBOL, STRING,IPADDR,BLOB,DECIMAL32("+std::to_string(scale32)+"),DECIMAL64("+std::to_string(scale64)+"),DECIMAL128("+std::to_string(scale128)+")];\n"
                    "t=table(1:0, colName, colType);"
                    "dbname = '"+dbName+"';tableName = 'insertTableCompressWithAllType';"
                    "if(existsDatabase(dbname)){dropDatabase(dbname)};go;"
                    "db = database(dbname,HASH,[INT, 2],,'TSDB');"
                    "compress_methods={cbool:`lz4, cchar:`lz4, cshort:`delta, cint:`delta, cdate:`delta, cmonth:`delta, ctime:`delta, cminute:`delta, csecond:`delta, cdatetime:`delta, ctimestamp:`lz4, cnanotime:`lz4, cnanotimestamp:`lz4, cfloat:`lz4, cdouble:`lz4, csymbol:`lz4, cstring:`lz4, cipaddr:`lz4, cblob:`lz4, cdecimal32:`lz4, cdecimal64:`lz4, cdecimal128:`lz4};"
                    "db.createPartitionedTable(t,tableName,`ind, compress_methods,'ind').append!(t);";
    conn.run(script);
    int success = conn.run("tableInsert{loadTable(dbname,tableName)}",args)->getInt();
    ASSERT_EQ(success, count);

    conn.upload("tmp",table);

    dolphindb::ConstantSP ans = conn.run("ex= select * from loadTable(dbname,tableName) order by ind;"
                                       "res = select * from tmp order by ind;"
                                       "all(each(eqObj,res.values(),ex.values()))");
    ASSERT_TRUE(ans->getBool());
    dolphindb::TableSP res = conn.run("select * from loadTable(dbname,tableName) order by ind");
    for (int i = 0; i < res->columns(); i++) {
        ASSERT_EQ(res->getColumn(i)->getString(), table->getColumn(i)->getString());
    }
}

TEST_F(CompressTest,CompressVectorWithDiffType){
    const int count = 600000;
    std::vector<std::string> colName={"time","value"};
    std::vector<int> time(count);
    std::vector<long long>value(count);

    int basetime = dolphindb::Util::countDays(2012,1,1);
    for(int i=0;i<count;i++){
        time[i] = basetime + (i%15);
        value[i] = i;
    }

    dolphindb::VectorSP timeVector = dolphindb::Util::createVector(dolphindb::DT_DATE,count, count);
    dolphindb::VectorSP valueVector = dolphindb::Util::createVector(dolphindb::DT_LONG, count,count);
    timeVector->setInt(0,count,time.data());
    valueVector->setLong(0,count,value.data());
    std::vector<dolphindb::ConstantSP> colVector {timeVector,valueVector};
    dolphindb::TableSP table  = dolphindb::Util::createTable(colName,colVector);
    std::vector<dolphindb::COMPRESS_METHOD> typeVec{dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_LZ4};
    table->setColumnCompressMethods(typeVec);
    std::vector<dolphindb::ConstantSP> args{table};
    conn.run("table1=streamTable(1:0, `time`value,[DATE,LONG])");
    int success = conn.run("tableInsert{table1}",args)->getInt();
    ASSERT_EQ(success, count);
    dolphindb::TableSP t1 = conn.run("select * from table1");
    ASSERT_EQ(t1->getString(),table->getString());
}

TEST_F(CompressTest,CompressWithDiffIndfsTable){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string stricpt = "dbName = \""+dbName+"\"\n"
                     "if(existsDatabase(dbName)){\n"
                     "\tdropDatabase(dbName)\t\n"
                     "}\n"
                     "db = database(dbName,VALUE,`A`B`C)\n"
                     "t_CWDT = table(1:0,`id`name`value,[INT,SYMBOL,LONG])\n"
                     "pt = db.createPartitionedTable(t_CWDT,`pt,`name,{\"id\":\"lz4\",\"value\":\"delta\"});";
    conn.run(stricpt);
    const int count = 600000;
    std::vector<std::string> colName={"id","name","value"};
    std::vector<int> id(count);
    std::vector<std::string> name(count);
    std::vector<long long>value(count);

    std::string names[] = {"A","B", "C"};
    for(int i=0;i<count;i++){
        id[i] = i%15;
        name[i] = names[i%3];
        value[i] = i+1000;
    }
    dolphindb::VectorSP idVector = dolphindb::Util::createVector(dolphindb::DT_INT,count,count);
    dolphindb::VectorSP nameVector = dolphindb::Util::createVector(dolphindb::DT_SYMBOL,count,count);
    dolphindb::VectorSP valueVector = dolphindb::Util::createVector(dolphindb::DT_LONG,count,count);
    idVector->setInt(0,count,id.data());
    nameVector->setString(0,count,name.data());
    valueVector->setLong(0,count,value.data());
    std::vector<dolphindb::ConstantSP> colVector{idVector,nameVector,valueVector};
    dolphindb::TableSP table = dolphindb::Util::createTable(colName,colVector);
    std::vector<dolphindb::ConstantSP> args{table};
    std::vector<dolphindb::COMPRESS_METHOD> compress = {dolphindb::COMPRESS_DELTA,dolphindb::COMPRESS_LZ4,dolphindb::COMPRESS_LZ4};
    table->setColumnCompressMethods(compress);
    int success = conn.run("tableInsert{loadTable(dbName,`pt)}",args)->getInt();
    ASSERT_EQ(success,count);
    dolphindb::TableSP t1 = conn.run("select * from loadTable(dbName,`pt) order by value");
    ASSERT_EQ(t1->getString(),table->getString());
    conn.run("dropDatabase(dbName)");
}

TEST_F(CompressTest, uploadArrayVectorwithCompress)
{
    std::vector<std::string> colName(20);
    for (int i = 0; i < 20; i++)
    {
        colName[i] = "factor" + std::to_string(i);
    }

    dolphindb::VectorSP Index = conn.run("take(5 3 7 3,60000).cumsum().int()");
    int count = 270000;

    dolphindb::VectorSP boolvector = dolphindb::Util::createVector(dolphindb::DT_BOOL, count, count);
    dolphindb::VectorSP charvector = dolphindb::Util::createVector(dolphindb::DT_CHAR, count, count);
    dolphindb::VectorSP shortvector = dolphindb::Util::createVector(dolphindb::DT_SHORT, count, count);
    dolphindb::VectorSP intvector = dolphindb::Util::createVector(dolphindb::DT_INT, count, count);
    dolphindb::VectorSP longvector = dolphindb::Util::createVector(dolphindb::DT_LONG, count, count);
    dolphindb::VectorSP floatvector = dolphindb::Util::createVector(dolphindb::DT_FLOAT, count, count);
    dolphindb::VectorSP doublevector = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, count, count);
    dolphindb::VectorSP datevector = dolphindb::Util::createVector(dolphindb::DT_DATE, count, count);
    dolphindb::VectorSP timestampvector = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP, count, count);
    dolphindb::VectorSP datehourvector = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR, count, count);
    dolphindb::VectorSP datetimevector = dolphindb::Util::createVector(dolphindb::DT_DATETIME, count, count);
    dolphindb::VectorSP timevector = dolphindb::Util::createVector(dolphindb::DT_TIME, count, count);
    dolphindb::VectorSP minutevector = dolphindb::Util::createVector(dolphindb::DT_MINUTE, count, count);
    dolphindb::VectorSP monthvector = dolphindb::Util::createVector(dolphindb::DT_MONTH, count, count);
    dolphindb::VectorSP secondvector = dolphindb::Util::createVector(dolphindb::DT_SECOND, count, count);
    dolphindb::VectorSP nanotimevector = dolphindb::Util::createVector(dolphindb::DT_NANOTIME, count, count);
    dolphindb::VectorSP nanotimestampVector = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP, count, count);
    dolphindb::VectorSP int128Vector = dolphindb::Util::createVector(dolphindb::DT_INT128, count, count);
    dolphindb::VectorSP uuidVector = dolphindb::Util::createVector(dolphindb::DT_UUID, count, count);
    dolphindb::VectorSP ipaddrVector = dolphindb::Util::createVector(dolphindb::DT_IP, count, count);

    std::vector<int> arrayValues(count);
    for (int i = 0; i < count; i++)
    {
        arrayValues[i] = i;
    }
    boolvector->setInt(0, count, arrayValues.data());
    charvector->setInt(0, count, arrayValues.data());
    shortvector->setInt(0, count, arrayValues.data());
    intvector->setInt(0, count, arrayValues.data());
    longvector->setInt(0, count, arrayValues.data());
    floatvector->setInt(0, count, arrayValues.data());
    doublevector->setInt(0, count, arrayValues.data());
    datevector->setInt(0, count, arrayValues.data());
    timestampvector->setInt(0, count, arrayValues.data());
    datehourvector->setInt(0, count, arrayValues.data());
    datetimevector->setInt(0, count, arrayValues.data());
    timevector->setInt(0, count, arrayValues.data());
    minutevector->setInt(0, count, arrayValues.data());
    monthvector->setInt(0, count, arrayValues.data());
    secondvector->setInt(0, count, arrayValues.data());
    nanotimevector->setInt(0, count, arrayValues.data());
    nanotimestampVector->setInt(0, count, arrayValues.data());
    int128Vector->setString(0, count, (char **)"e1671797c52e15f763380b45e841ec32");
    uuidVector->setString(0, count, (char **)"5d212a78-cc48-e3b1-4235-b4d91473ee87");
    ipaddrVector->setString(0, count, (char **)"192.168.1.13");

    dolphindb::VectorSP boolArray = dolphindb::Util::createArrayVector(Index, boolvector);
    dolphindb::VectorSP charArray = dolphindb::Util::createArrayVector(Index, charvector);
    dolphindb::VectorSP shortArray = dolphindb::Util::createArrayVector(Index, shortvector);
    dolphindb::VectorSP longArray = dolphindb::Util::createArrayVector(Index, longvector);
    dolphindb::VectorSP intArray = dolphindb::Util::createArrayVector(Index, intvector);
    dolphindb::VectorSP floatArray = dolphindb::Util::createArrayVector(Index, floatvector);
    dolphindb::VectorSP doubleArray = dolphindb::Util::createArrayVector(Index, doublevector);
    dolphindb::VectorSP dateArray = dolphindb::Util::createArrayVector(Index, datevector);
    dolphindb::VectorSP timestampArray = dolphindb::Util::createArrayVector(Index, timestampvector);
    dolphindb::VectorSP datetimeArray = dolphindb::Util::createArrayVector(Index, datetimevector);
    dolphindb::VectorSP datehourArray = dolphindb::Util::createArrayVector(Index, datehourvector);
    dolphindb::VectorSP timeArray = dolphindb::Util::createArrayVector(Index, timevector);
    dolphindb::VectorSP minuteArray = dolphindb::Util::createArrayVector(Index, minutevector);
    dolphindb::VectorSP monthArray = dolphindb::Util::createArrayVector(Index, monthvector);
    dolphindb::VectorSP secondeArray = dolphindb::Util::createArrayVector(Index, secondvector);
    dolphindb::VectorSP nanotimeArray = dolphindb::Util::createArrayVector(Index, nanotimevector);
    dolphindb::VectorSP nanotimestampArray = dolphindb::Util::createArrayVector(Index, nanotimestampVector);
    dolphindb::VectorSP int128Array = dolphindb::Util::createArrayVector(Index, int128Vector);
    dolphindb::VectorSP uuidArray = dolphindb::Util::createArrayVector(Index, uuidVector);
    dolphindb::VectorSP ipaddrArray = dolphindb::Util::createArrayVector(Index, ipaddrVector);

    std::vector<dolphindb::ConstantSP> colVector{boolArray, charArray, shortArray, longArray, intArray, floatArray, doubleArray,
                                 dateArray, timestampArray, datetimeArray, datehourArray, timeArray, minuteArray, monthArray,
                                 secondeArray, nanotimeArray, nanotimestampArray, int128Array, uuidArray, ipaddrArray};
    conn.upload(colName, colVector);
    dolphindb::ConstantSP objs = conn.run("exec * from objs() where name like 'factor%'");
    ASSERT_EQ(objs->getColumn(1)->getString(), "[\"BOOL[]\",\"CHAR[]\",\"SHORT[]\",\"LONG[]\",\"INT[]\",\"FLOAT[]\",\"DOUBLE[]\",\"DATE[]\",\"TIMESTAMP[]\",\"DATETIME[]\",\"DATEHOUR[]\",\"TIME[]\",\"MINUTE[]\",\"MONTH[]\",\"SECOND[]\",\"NANOTIME[]\",\"NANOTIMESTAMP[]\",\"INT128[]\",\"UUID[]\",\"IPADDR[]\"]");

}

TEST_F(CompressTest,uploadArrayVectorTablewithCompress){
    int count = 60000;
    std::vector<std::string> colName(22);
    colName[0] = "time";
    colName[1] = "values";
    for(int i=2;i<22;i++){
        colName[i] = "factor"+ std::to_string(i);
    }
    std::vector<int> time(count);
    std::vector<long long>value(count);

    int basetime = dolphindb::Util::countDays(2012,1,1);
    for(int i=0;i<count;i++){
        time[i] = basetime + (i%15);
        value[i] = i;
    }
    dolphindb::VectorSP timeVector = dolphindb::Util::createVector(dolphindb::DT_DATE,count, count);
    dolphindb::VectorSP valueVector = dolphindb::Util::createVector(dolphindb::DT_LONG, count,count);
    timeVector->setInt(0,count,time.data());
    valueVector->setLong(0,count,value.data());

    //array std::vector
    dolphindb::VectorSP Index = conn.run("take(5 3 7 3,60000).cumsum().int()");
    count = 270000;

    dolphindb::VectorSP boolvector = dolphindb::Util::createVector(dolphindb::DT_BOOL,count,count);
    dolphindb::VectorSP charvector = dolphindb::Util::createVector(dolphindb::DT_CHAR,count,count);
    dolphindb::VectorSP shortvector = dolphindb::Util::createVector(dolphindb::DT_SHORT,count,count);
    dolphindb::VectorSP intvector = dolphindb::Util::createVector(dolphindb::DT_INT,count,count);
    dolphindb::VectorSP longvector = dolphindb::Util::createVector(dolphindb::DT_LONG,count,count);
    dolphindb::VectorSP floatvector = dolphindb::Util::createVector(dolphindb::DT_FLOAT,count,count);
    dolphindb::VectorSP doublevector = dolphindb::Util::createVector(dolphindb::DT_DOUBLE, count,count);
    dolphindb::VectorSP datevector = dolphindb::Util::createVector(dolphindb::DT_DATE,count,count);
    dolphindb::VectorSP timestampvector = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP,count,count);
    dolphindb::VectorSP datehourvector = dolphindb::Util::createVector(dolphindb::DT_DATEHOUR,count,count);
    dolphindb::VectorSP datetimevector = dolphindb::Util::createVector(dolphindb::DT_DATETIME,count,count);
    dolphindb::VectorSP timevector = dolphindb::Util::createVector(dolphindb::DT_TIME,count,count);
    dolphindb::VectorSP minutevector = dolphindb::Util::createVector(dolphindb::DT_MINUTE, count,count);
    dolphindb::VectorSP monthvector = dolphindb::Util::createVector(dolphindb::DT_MONTH,count,count);
    dolphindb::VectorSP secondvector = dolphindb::Util::createVector(dolphindb::DT_SECOND,count,count);
    dolphindb::VectorSP nanotimevector =  dolphindb::Util::createVector(dolphindb::DT_NANOTIME,count,count);
    dolphindb::VectorSP nanotimestampVector  =dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP,count,count);
    dolphindb::VectorSP int128Vector = dolphindb::Util::createVector(dolphindb::DT_INT128, count, count);
    dolphindb::VectorSP uuidVector = dolphindb::Util::createVector(dolphindb::DT_UUID, count, count);
    dolphindb::VectorSP ipaddrVector = dolphindb::Util::createVector(dolphindb::DT_IP, count, count);

    std::vector<int> arrayValues(count);
    for(int i=0;i<count;i++){
        arrayValues[i] = i;
    }
    boolvector->setInt(0,count, arrayValues.data());
    charvector->setInt(0,count,arrayValues.data());
    shortvector->setInt(0,count,arrayValues.data());
    intvector->setInt(0,count,arrayValues.data());
    longvector->setInt(0,count,arrayValues.data());
    floatvector->setInt(0,count,arrayValues.data());
    doublevector->setInt(0,count,arrayValues.data());
    datevector->setInt(0,count,arrayValues.data());
    timestampvector->setInt(0,count,arrayValues.data());
    datehourvector->setInt(0,count,arrayValues.data());
    datetimevector->setInt(0,count,arrayValues.data());
    timevector->setInt(0,count,arrayValues.data());
    minutevector->setInt(0,count,arrayValues.data());
    monthvector->setInt(0,count,arrayValues.data());
    secondvector->setInt(0,count,arrayValues.data());
    nanotimevector->setInt(0,count,arrayValues.data());
    nanotimestampVector->setInt(0,count,arrayValues.data());
    int128Vector->setString(0, count, (char **)"e1671797c52e15f763380b45e841ec32");
    uuidVector->setString(0, count, (char **)"5d212a78-cc48-e3b1-4235-b4d91473ee87");
    ipaddrVector->setString(0, count, (char **)"192.168.1.13");

    dolphindb::VectorSP boolArray = dolphindb::Util::createArrayVector(Index, boolvector);
    dolphindb::VectorSP charArray = dolphindb::Util::createArrayVector(Index, charvector);
    dolphindb::VectorSP shortArray  = dolphindb::Util::createArrayVector(Index, shortvector);
    dolphindb::VectorSP longArray  = dolphindb::Util::createArrayVector(Index, longvector);
    dolphindb::VectorSP intArray  = dolphindb::Util::createArrayVector(Index, intvector);
    dolphindb::VectorSP floatArray = dolphindb::Util::createArrayVector(Index, floatvector);
    dolphindb::VectorSP doubleArray = dolphindb::Util::createArrayVector(Index, doublevector);
    dolphindb::VectorSP dateArray = dolphindb::Util::createArrayVector(Index, datevector);
    dolphindb::VectorSP timestampArray = dolphindb::Util::createArrayVector(Index, timestampvector);
    dolphindb::VectorSP datetimeArray = dolphindb::Util::createArrayVector(Index, datetimevector);
    dolphindb::VectorSP datehourArray = dolphindb::Util::createArrayVector(Index, datehourvector);
    dolphindb::VectorSP timeArray = dolphindb::Util::createArrayVector(Index, timevector);
    dolphindb::VectorSP minuteArray = dolphindb::Util::createArrayVector(Index, minutevector);
    dolphindb::VectorSP monthArray = dolphindb::Util::createArrayVector(Index, monthvector);
    dolphindb::VectorSP secondeArray = dolphindb::Util::createArrayVector(Index, secondvector);
    dolphindb::VectorSP nanotimeArray = dolphindb::Util::createArrayVector(Index, nanotimevector);
    dolphindb::VectorSP nanotimestampArray = dolphindb::Util::createArrayVector(Index, nanotimestampVector);
    dolphindb::VectorSP int128Array = dolphindb::Util::createArrayVector(Index, int128Vector);
    dolphindb::VectorSP uuidArray = dolphindb::Util::createArrayVector(Index, uuidVector);
    dolphindb::VectorSP ipaddrArray = dolphindb::Util::createArrayVector(Index, ipaddrVector);

    std::vector<dolphindb::ConstantSP> colVector{timeVector, valueVector, boolArray, charArray, shortArray, longArray, intArray, floatArray, doubleArray,
                                 dateArray, timestampArray, datetimeArray, datehourArray, timeArray, minuteArray, monthArray,
                                 secondeArray, nanotimeArray, nanotimestampArray, int128Array, uuidArray, ipaddrArray};
    dolphindb::TableSP table  = dolphindb::Util::createTable(colName,colVector);

    std::vector<dolphindb::COMPRESS_METHOD> typeVec(22);
    typeVec[0] = dolphindb::COMPRESS_DELTA;
    typeVec[1] = dolphindb::COMPRESS_DELTA;
    for(int i=0;i<20;i++){
        typeVec[i+2] = dolphindb::COMPRESS_LZ4;
    }
    table->setColumnCompressMethods(typeVec);

    std::vector<dolphindb::ConstantSP> args{table};
    conn.run("colName = `time`values\n"
             "for(i in 2..21){\n"
             "\tcolName.append!(\"factor\"+string(i))\n"
             "}\n"
             "colType = [DATE,LONG,BOOL[],CHAR[],SHORT[],INT[],LONG[],FLOAT[],DOUBLE[],DATE[],TIMESTAMP[],DATEHOUR[],DATETIME[],\n"
             "              TIME[],MINUTE[],MONTH[],SECOND[],NANOTIME[],NANOTIMESTAMP[], INT128[], UUID[], IPADDR[]]\n"
             "table1=streamTable(100:0,colName,colType)");
    int success = conn.run("tableInsert{table1}",args)->getInt();
    ASSERT_EQ(success, 60000);

    dolphindb::TableSP t1 = conn.run("select * from table1");
    ASSERT_EQ(t1->getString(),table->getString());
}

TEST_F(CompressTest,insertTableCompressWithDecimal32ArrayVector){
    int count = 600000;

    dolphindb::VectorSP indV = conn.run("1..300000*2");
    dolphindb::VectorSP valV = dolphindb::Util::createVector(dolphindb::DT_DECIMAL32, count, count, true, 2);
    dolphindb::ConstantSP val1 = dolphindb::Util::createDecimal32(2, 2.35123);
    for(auto i =0;i<valV->size();i++){
        valV->set(i, val1);
    }

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(indV, valV);
    std::vector<std::string> colName={"decimal32av"};
    std::vector<dolphindb::ConstantSP> colVector {av1};
    dolphindb::TableSP table  = dolphindb::Util::createTable(colName,colVector);
    std::vector<dolphindb::COMPRESS_METHOD> typeVec{dolphindb::COMPRESS_LZ4};
    table->setColumnCompressMethods(typeVec);

    std::vector<dolphindb::ConstantSP> args{table};
    std::string script = "colName =  [`decimal32av];"
                    "colType = [DECIMAL32(2)[]];"
                    "table1=streamTable(1:0,colName,colType)";
    conn.run(script);
    int success = conn.run("tableInsert{table1}",args)->getInt();
    ASSERT_EQ(success, count/2);

    conn.upload("table",table);

    dolphindb::ConstantSP res = conn.run("each(eqObj,table.values(),table1.values())");
    for (int i = 0; i < res->size(); i++)
        ASSERT_TRUE(res->get(i)->getBool());

    dolphindb::TableSP ex = conn.run("table1");
    for(auto i=0;i<ex->columns();i++){
        for(auto j=0;j<ex->rows();j++){
            ASSERT_EQ(ex->getColumn(i)->get(j)->getString(), table->getColumn(i)->get(j)->getString());
        }
    }
}

TEST_F(CompressTest,insertTableCompressWithDecimal64ArrayVector){
    int count = 600000;
    int scale64=rand()%18;

    dolphindb::VectorSP indV = conn.run("1..300000*2");
    dolphindb::VectorSP valV = dolphindb::Util::createVector(dolphindb::DT_DECIMAL64, count, count, true, scale64);
    dolphindb::ConstantSP val1 = dolphindb::Util::createDecimal64(scale64, 1.1054876666452);
    for(auto i =0;i<valV->size();i++){
        valV->set(i, val1);
    }

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(indV, valV);
    std::vector<std::string> colName={"decimal64av"};
    std::vector<dolphindb::ConstantSP> colVector {av1};
    dolphindb::TableSP table  = dolphindb::Util::createTable(colName,colVector);
    std::vector<dolphindb::COMPRESS_METHOD> typeVec{dolphindb::COMPRESS_LZ4};
    table->setColumnCompressMethods(typeVec);

    std::vector<dolphindb::ConstantSP> args{table};
    std::string script = "colName =  [`decimal64av];"
                    "colType = [DECIMAL64("+std::to_string(scale64)+")[]];"
                    "table1=streamTable(1:0,colName,colType)";
    conn.run(script);
    int success = conn.run("tableInsert{table1}",args)->getInt();
    ASSERT_EQ(success, count/2);

    conn.upload("table",table);

    dolphindb::ConstantSP res = conn.run("each(eqObj,table.values(),table1.values())");
    for (int i = 0; i < res->size(); i++)
        ASSERT_TRUE(res->get(i)->getBool());

    dolphindb::TableSP ex = conn.run("table1");
    for(auto i=0;i<ex->columns();i++){
        for(auto j=0;j<ex->rows();j++){
            ASSERT_EQ(ex->getColumn(i)->get(j)->getString(), table->getColumn(i)->get(j)->getString());
        }
    }
}


TEST_F(CompressTest,insertTableCompressWithDecimal128ArrayVector){
    int count = 600000;
    int scale128=rand()%38;

    dolphindb::VectorSP indV = conn.run("1..300000*2");
    dolphindb::VectorSP valV = dolphindb::Util::createVector(dolphindb::DT_DECIMAL128, count, count, true, scale128);
    dolphindb::ConstantSP val1 = dolphindb::Util::createDecimal128(scale128, 1.1054876666452);
    std::cout << val1->getString() << std::endl;
    for(auto i =0;i<valV->size();i++){
        valV->set(i, val1);
    }

    dolphindb::VectorSP av1 = dolphindb::Util::createArrayVector(indV, valV);
    std::vector<std::string> colName={"decimal128av"};
    std::vector<dolphindb::ConstantSP> colVector {av1};
    dolphindb::TableSP table  = dolphindb::Util::createTable(colName,colVector);
    std::vector<dolphindb::COMPRESS_METHOD> typeVec{dolphindb::COMPRESS_LZ4};
    table->setColumnCompressMethods(typeVec);

    std::vector<dolphindb::ConstantSP> args{table};
    std::string script = "colName =  [`decimal128av];"
                    "colType = [DECIMAL128("+std::to_string(scale128)+")[]];"
                    "table1=streamTable(1:0,colName,colType)";
    conn.run(script);
    int success = conn.run("tableInsert{table1}",args)->getInt();
    ASSERT_EQ(success, count/2);

    conn.upload("table",table);

    dolphindb::ConstantSP res = conn.run("each(eqObj,table.values(),table1.values())");
    for (int i = 0; i < res->size(); i++)
        ASSERT_TRUE(res->get(i)->getBool());

    dolphindb::TableSP ex = conn.run("table1");
    for(auto i=0;i<ex->columns();i++){
        for(auto j=0;j<ex->rows();j++){
            ASSERT_EQ(ex->getColumn(i)->get(j)->getString(), table->getColumn(i)->get(j)->getString());
        }
    }
}

TEST_F(CompressTest, upload_other_dataforms_with_compress)
{
    auto matrx = conn.run("a = matrix(1 2, 3 4);a");
    auto vec = conn.run("b = [`abc, 1h, 3.14, ipaddr('1.1.1.1')];b");
    auto par = conn.run("c = 1:2;c");
    auto st = conn.run("d = set(1 1 2 3);d");
    auto dt = conn.run("ee = dict(int(1 2),int(3 4));ee");
    std::vector<dolphindb::ConstantSP> vals{matrx, vec, par, st, dt};
    for (auto i = 0; i < vals.size(); i++)
    {
        conn.upload("val" + std::to_string(i), vals[i]);
    }
    ASSERT_TRUE(conn.run("eqObj(val0, a)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(val1, b)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(val2, c)")->getBool());
    conn.run("assert 1, d.size() == val3.size();"
                      "assert 2, 1 in d;"
                      "assert 3, 2 in d;"
                      "assert 4, 3 in d");
    conn.run("assert 5, each(eqObj, sort(val4.keys()), sort(ee.keys()));"
                      "assert 6, each(eqObj, sort(val4.values()), sort(ee.values()));");
}

class compressObjectTest : public CompressTest, public testing::WithParamInterface<std::vector<std::string>> 
{
public:
    static std::vector<std::vector<std::string>> get_data(){
        return {
            {"compress(1 2 3)", "[1,2,3]"},
            {"compress([2024.04.16T17:18:07.155,2024.04.16T17:18:07.155])", "[2024.04.16T17:18:07.155,2024.04.16T17:18:07.155]"},
            {"compress(`str1`str2)", "[\"str1\",\"str2\"]"},
            {"compress([44.277738,34.558771])", "[44.277738,34.558771]"},
            {"compress(int128(`e1671797c52e15f763380b45e841ec32`e1671797c52e15f763380b45e841ec33))", "[e1671797c52e15f763380b45e841ec32,e1671797c52e15f763380b45e841ec33]"},
            {"compress(uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87''5d212a78-cc48-e3b1-4235-b4d91473ee88'))", "[5d212a78-cc48-e3b1-4235-b4d91473ee87,5d212a78-cc48-e3b1-4235-b4d91473ee88]"},
            {"compress(decimal64('1.1054876666452''1.1054876666453', 15))", "[1.105487666645200,1.105487666645300]"},
            {"compress(array(INT[]).append!([1 2 3]))", "[[1,2,3]]"},
            {"compress(array(DATEHOUR[]).append!([datehour('2020.01.01T01''2020.02.01T01')]))", "[[2020.01.01T01,2020.02.01T01]]"},
            {"compress(table(1:0, `c1`c2`c3`c4, [SYMBOL,INT,DOUBLE,DOUBLE[]]))", "c1 c2 c3 c4\n-- -- -- --\n"},
            {"compress(table(rand(`APPL`GOOL`MSFT`TSLA, 70000) as sym, rand(100.0000, 70000) as price, rand(100, 70000) as qty, arrayVector(1..70000*5, rand(100.0000,     350000)) as value))", ""},
            // delta compression
            {"compress(1 2 3, 'delta')", "[1,2,3]"},
            {"compress([2024.04.16T17:18:07.155,2024.04.16T17:18:07.155], 'delta')", "[2024.04.16T17:18:07.155,2024.04.16T17:18:07.155]"},
            {"compress(decimal64('1.1054876666452''1.1054876666453', 15), 'delta')", "[1.105487666645200,1.105487666645300]"},
            {"compress(table(1:0, `c1`c2`c3`c4, [SYMBOL,INT,DOUBLE,DOUBLE[]]), 'delta')", "c1 c2 c3 c4\n-- -- -- --\n"},
            {"compress(table(rand(`APPL`GOOL`MSFT`TSLA, 70000) as sym, rand(100.0000, 70000) as price, rand(100, 70000) as qty, arrayVector(1..70000*5, rand(100.0000,     350000)) as value), 'delta')", ""},
        };
    }
};

INSTANTIATE_TEST_SUITE_P(, compressObjectTest, testing::ValuesIn(compressObjectTest::get_data()));
TEST_P(compressObjectTest, test_download_and_upload_compress_object){
    std::string s1 = GetParam()[0];
    std::string s2 = GetParam()[1];
    dolphindb::ConstantSP obj = conn.run("a = "+s1+";a");

    conn.upload("obj", obj);
    if (obj->getForm() != dolphindb::DF_TABLE){
        ASSERT_EQ(obj->getString(), s2);
        ASSERT_TRUE(conn.run("eqObj(obj, a.decompress())")->getBool());
    }
    else
    {
        ASSERT_TRUE(conn.run("all(each(eqObj, obj.values(), a.decompress().values()))")->getBool());
    }
}
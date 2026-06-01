#include <gtest/gtest.h>
#include "config.h"

class ExceptionTest:public testing::Test
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

dolphindb::DBConnection ExceptionTest::conn(false, false);

static std::string eMsg="test";

TEST_F(ExceptionTest,testIncompatibleTypeException){
    dolphindb::DATA_TYPE dt1=dolphindb::DT_INT;
    dolphindb::DATA_TYPE dt2=dolphindb::DT_STRING;
    try
    {
        dolphindb::IncompatibleTypeException e(dt1,dt2);
        throw e;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        std::string ex_str="Incompatible type. Expected: " + dolphindb::Util::getDataTypeString(dt1) + ", Actual: " + dolphindb::Util::getDataTypeString(dt2);
        ASSERT_STREQ(e.what(),ex_str.c_str());
    }
    
}

TEST_F(ExceptionTest,testIllegalArgumentException){
    try
    {
        std::string eFuncName="func";
        dolphindb::IllegalArgumentException e(eFuncName,eMsg);
        throw e;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        ASSERT_STREQ(e.what(),eMsg.c_str());
    }
    
}

TEST_F(ExceptionTest,testRuntimeException){
    try
    {
        dolphindb::RuntimeException e(eMsg);
        throw e;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        ASSERT_STREQ(e.what(),eMsg.c_str());
    }
    
}

TEST_F(ExceptionTest,testOperatorRuntimeException){
    try
    {
        std::string eOptr="func";
        dolphindb::OperatorRuntimeException e(eOptr,eMsg);
        throw e;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        ASSERT_STREQ(e.what(),eMsg.c_str());
    }
    
}

TEST_F(ExceptionTest,testTableRuntimeException){
    try
    {
        dolphindb::TableRuntimeException e(eMsg);
        throw e;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        ASSERT_STREQ(e.what(),eMsg.c_str());
    }
    
}

TEST_F(ExceptionTest,testMemoryException){
    try
    {
        dolphindb::MemoryException e;
        throw e;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        std::string ex_str= "Out of memory";
        ASSERT_STREQ(e.what(),ex_str.c_str());
    }
    
}

TEST_F(ExceptionTest,testIOException){
    std::vector<dolphindb::IO_ERR> ioErrVec={dolphindb::OK,dolphindb::DISCONNECTED,dolphindb::NODATA,dolphindb::NOSPACE,dolphindb::TOO_LARGE_DATA,dolphindb::INPROGRESS,dolphindb::INVALIDDATA,dolphindb::END_OF_STREAM,dolphindb::READONLY,dolphindb::WRITEONLY,dolphindb::NOTEXIST,dolphindb::CORRUPT,dolphindb::NOT_LEADER,dolphindb::OTHERERR};
    std::string getMsg;
    for(int i=0;i<ioErrVec.size();i++){
        try
        {
            dolphindb::IOException e(getMsg,ioErrVec[i]);
            throw e;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    for(int i=0;i<ioErrVec.size();i++){
        try
        {
            dolphindb::IOException e(ioErrVec[i]);
            throw e;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }
    try
    {
        dolphindb::IOException e(getMsg);
        throw e;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}

TEST_F(ExceptionTest,testDataCorruptionException){
    try
    {
        dolphindb::DataCorruptionException e(eMsg);
        throw e;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        ASSERT_STREQ(e.what(),("<DataCorruption>"+eMsg).c_str());
    }
    
}

TEST_F(ExceptionTest,testNotLeaderException){
    std::string newLeader="datanode3";
    try
    {
        dolphindb::NotLeaderException e(eMsg);
        throw e;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        ASSERT_STREQ(e.what(),("<NotLeader>"+eMsg).c_str());
    }
    try
    {
        dolphindb::NotLeaderException e(newLeader);
        throw e;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        ASSERT_STREQ(e.what(),("<NotLeader>"+newLeader).c_str());
    }
    
}

TEST_F(ExceptionTest,testMathException){
    try
    {
        dolphindb::MathException e(eMsg);
        throw e;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        ASSERT_STREQ(e.what(),eMsg.c_str());
    }
    
}

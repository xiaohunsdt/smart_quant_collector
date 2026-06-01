#include <gtest/gtest.h>
#include "config.h"
#include "SysIO.h"
#include "ConstantMarshall.h"
#include "fstream"

class SysIOTest:public testing::Test
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
dolphindb::DBConnection SysIOTest::conn(false, false);

class SysIOTest_Parameterized: public::testing::TestWithParam<dolphindb::DATA_TYPE>{

};

TEST_F(SysIOTest, test_Socket_No_Parameter){
    dolphindb::Socket s1 = dolphindb::Socket();
    ASSERT_TRUE(s1.isValid());
    s1.connect(HOST, PORT, false, 7200);
    ASSERT_EQ(s1.getHost(),HOST);
    ASSERT_EQ(s1.getPort(),PORT);
    dolphindb::Socket s2 = dolphindb::Socket(s1.getHandle(), true, 7200);

    char *val = (char *)"aksdjopqweknmg812378195;'']=-_1!";
    size_t actualLength = 1;

    for(auto i=0;i<10;i++){
        dolphindb::IO_ERR res = s1.write(val, 6, actualLength);
        ASSERT_EQ(res, dolphindb::OK);
    }
    ASSERT_EQ(s1.read(val,6,actualLength), dolphindb::NODATA); // when send tcp-data to connected socket(host:port), it can read datas;
    ASSERT_EQ(s2.read(val,6,actualLength), dolphindb::NODATA);

    s1.enableTcpNoDelay(true);
    ASSERT_TRUE(s1.ENABLE_TCP_NODELAY);

    ASSERT_TRUE(s1.skipAll());
    ASSERT_TRUE(s2.skipAll());

    s1.close();
    s2.close();
    ASSERT_EQ(s1.write(val,7, actualLength),dolphindb::DISCONNECTED);
    ASSERT_FALSE(s1.isValid());
}

TEST_F(SysIOTest, test_Socket_with_Parameter){
    dolphindb::Socket s1 = dolphindb::Socket(HOST, PORT, true, 7200);
    ASSERT_FALSE(s1.isValid());
    ASSERT_EQ(s1.getHost(), HOST);
    ASSERT_EQ(s1.getPort(), PORT);
    std::cout<<"now get socket handle: "<<s1.getHandle()<<std::endl;

    char *val = (char *)"ccc579";
    size_t actualLength = 1;
    for(auto i=0;i<10;i++){
        dolphindb::IO_ERR res = s1.write(val,10, actualLength);
        std::cout<<res<<std::endl;
        // ASSERT_EQ(res, OK);
        dolphindb::Util::sleep(100);
    }
    ASSERT_EQ(s1.read(val,10,actualLength), dolphindb::OTHERERR);

    s1.enableTcpNoDelay(true);
    ASSERT_TRUE(s1.ENABLE_TCP_NODELAY);

    ASSERT_TRUE(s1.skipAll());

    s1.close();
    ASSERT_EQ(s1.write(val,7, actualLength),dolphindb::DISCONNECTED);
    ASSERT_FALSE(s1.isValid());
}

TEST_F(SysIOTest, test_DataStream_scalar_int)
{
    srand(time(NULL));
    int test_val = (int)rand()%INT_MAX;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createInt(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getInt(), test_val);

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();

        outStream->write(test_val);
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        int readValue;
        inputStream->readInt(readValue);
        ASSERT_EQ(readValue, test_val);
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_scalar_intNull)
{
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createNullConstant(dolphindb::DT_INT);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getInt(), object->getInt());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();

        outStream->write(int(NULL));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        int readValue;
        inputStream->readInt(readValue);
        ASSERT_EQ(readValue, int(NULL));
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_scalar_bool)
{
    srand(time(NULL));
    char test_val = rand() % 2;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createBool(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getInt(), test_val);

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();

        outStream->write(test_val);
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        bool readValue;
        inputStream->readBool(readValue);
        ASSERT_EQ(readValue, test_val);
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_scalar_boolNull)
{
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createNullConstant(dolphindb::DT_BOOL);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getBool(), object->getBool());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();

        outStream->write(bool(NULL));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        bool readValue;
        inputStream->readBool(readValue);
        ASSERT_EQ(readValue, bool(NULL));
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_scalar_INDEX)
{
    srand(time(NULL));
    dolphindb::INDEX test_val = (dolphindb::INDEX)rand() % dolphindb::INDEX_MAX;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createInt(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getIndex(), test_val);

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();

        outStream->write(test_val);
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        dolphindb::INDEX readValue;
        inputStream->readIndex(readValue);
        ASSERT_EQ(readValue, test_val);
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can snot be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_scalar_float)
{
    srand(time(NULL));
    float test_val = rand()/float(RAND_MAX);
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createFloat(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getFloat(), test_val);

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();

        outStream->write(test_val);
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        float readValue;
        inputStream->readFloat(readValue);
        ASSERT_EQ(readValue, test_val);

        dolphindb::DataStreamSP inputStream1 = new dolphindb::DataStream(pOutbuf, outSize);
        inputStream1->enableReverseIntegerByteOrder(); // reverseOrder == true
        float readValue1;
        inputStream1->readFloat(readValue1);
        std::cout<<"result: "<<readValue1<<std::endl;

        inputStream->close();
        inputStream1->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_scalar_floatNull)
{
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createNullConstant(dolphindb::DT_FLOAT);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getFloat(), object->getFloat());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();

        outStream->write(float(NULL));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        float readValue;
        inputStream->readFloat(readValue);
        ASSERT_EQ(readValue, float(NULL));

        dolphindb::DataStreamSP inputStream1 = new dolphindb::DataStream(pOutbuf, outSize);
        inputStream1->enableReverseIntegerByteOrder(); // reverseOrder == true
        float readValue1;
        inputStream1->readFloat(readValue1);
        std::cout<<"result: "<<readValue1<<std::endl;

        inputStream->close();
        inputStream1->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_scalar_string)
{
    srand(time(NULL));
    std::vector<std::string> rand_str = {"sd","dag","xxx","智臾科技a","23!@#$%","^&#%……@","/,m[[`"};
    std::string test_val = rand_str[rand() % rand_str.size()];
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createString(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), test_val);

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        outStream->write(test_val);
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        std::string readValue;
        inputStream->readString(readValue);
        ASSERT_EQ(inputStream->readString(readValue, 1000), dolphindb::END_OF_STREAM);
        ASSERT_EQ(readValue, test_val);
        ASSERT_TRUE(inputStream->isReadable());
        inputStream->isReadable(0);
        ASSERT_FALSE(inputStream->isReadable());
        ASSERT_TRUE(inputStream->isArrayStream());
        ASSERT_FALSE(inputStream->isWritable());
        inputStream->isWritable(1);
        ASSERT_TRUE(inputStream->isWritable());
        ASSERT_FALSE(inputStream->isFileStream());
        ASSERT_FALSE(inputStream->isSocketStream());

        inputStream->clearReadBuffer();
        ASSERT_EQ(inputStream->getDataSizeInArray(),0);

        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        char buf[] = "aadddccc";
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        outStream->write(buf, 9);
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        std::string readValue;
        inputStream->readString(readValue);
        ASSERT_EQ(inputStream->readString(readValue, 1000), dolphindb::END_OF_STREAM);
        ASSERT_EQ(readValue, buf);
        ASSERT_TRUE(inputStream->isReadable());
        inputStream->isReadable(0);
        ASSERT_FALSE(inputStream->isReadable());
        ASSERT_TRUE(inputStream->isArrayStream());
        ASSERT_FALSE(inputStream->isWritable());
        inputStream->isWritable(1);
        ASSERT_TRUE(inputStream->isWritable());
        ASSERT_FALSE(inputStream->isFileStream());
        ASSERT_FALSE(inputStream->isSocketStream());

        inputStream->clearReadBuffer();
        ASSERT_EQ(inputStream->getDataSizeInArray(),0);

        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        char buf[] = "aadddccc";
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        size_t actwrite;
        outStream->write(buf, 9, actwrite);
        ASSERT_EQ(actwrite, 9);
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        std::string readValue;
        inputStream->readString(readValue);
        ASSERT_EQ(inputStream->readString(readValue, 1000), dolphindb::END_OF_STREAM);
        ASSERT_EQ(readValue, buf);
        ASSERT_TRUE(inputStream->isReadable());
        inputStream->isReadable(0);
        ASSERT_FALSE(inputStream->isReadable());
        ASSERT_TRUE(inputStream->isArrayStream());
        ASSERT_FALSE(inputStream->isWritable());
        inputStream->isWritable(1);
        ASSERT_TRUE(inputStream->isWritable());
        ASSERT_FALSE(inputStream->isFileStream());
        ASSERT_FALSE(inputStream->isSocketStream());

        inputStream->clearReadBuffer();
        ASSERT_EQ(inputStream->getDataSizeInArray(),0);

        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }
}


TEST_F(SysIOTest, test_DataStream_scalar_stringNull)
{
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createNullConstant(dolphindb::DT_STRING);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        outStream->write(std::string(""));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        std::string readValue;
        inputStream->readString(readValue);
        ASSERT_EQ(inputStream->readString(readValue, 1000), dolphindb::END_OF_STREAM);
        ASSERT_EQ(readValue, std::string(""));
        ASSERT_TRUE(inputStream->isReadable());
        inputStream->isReadable(0);
        ASSERT_FALSE(inputStream->isReadable());
        ASSERT_TRUE(inputStream->isArrayStream());
        ASSERT_FALSE(inputStream->isWritable());
        inputStream->isWritable(1);
        ASSERT_TRUE(inputStream->isWritable());
        ASSERT_FALSE(inputStream->isFileStream());
        ASSERT_FALSE(inputStream->isSocketStream());

        inputStream->clearReadBuffer();
        ASSERT_EQ(inputStream->getDataSizeInArray(),0);

        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        char buf[] = "";
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        outStream->write(buf, 1);
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        std::string readValue;
        inputStream->readString(readValue);
        ASSERT_EQ(inputStream->readString(readValue, 1), dolphindb::END_OF_STREAM);
        ASSERT_EQ(readValue, buf);

        std::cout<<inputStream->clearReadBuffer()<<std::endl;
        ASSERT_EQ(inputStream->getDataSizeInArray(), 0);

        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

    { // c++ scalar
        char *pOutbuf;
        int outSize;
        char buf[] = "";
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        size_t actwrite;
        outStream->write(buf, 1, actwrite);
        ASSERT_EQ(actwrite, 1);
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        std::string readValue;
        inputStream->readString(readValue);
        ASSERT_EQ(inputStream->readString(readValue, 1), dolphindb::END_OF_STREAM);
        ASSERT_EQ(readValue, buf);
        ASSERT_TRUE(inputStream->isReadable());
        inputStream->isReadable(0);
        ASSERT_FALSE(inputStream->isReadable());
        ASSERT_TRUE(inputStream->isArrayStream());
        ASSERT_FALSE(inputStream->isWritable());
        inputStream->isWritable(1);
        ASSERT_TRUE(inputStream->isWritable());
        ASSERT_FALSE(inputStream->isFileStream());
        ASSERT_FALSE(inputStream->isSocketStream());

        inputStream->clearReadBuffer();
        ASSERT_EQ(inputStream->getDataSizeInArray(), 0);

        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }
}

TEST_F(SysIOTest, test_DataStream_scalar_date)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createDate(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_scalar_month)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createMonth(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_scalar_time)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createTime(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_scalar_minute)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createMinute(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_scalar_second)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createSecond(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_scalar_datetime)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createDateTime(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_scalar_timestamp)
{
    srand(time(NULL));
    long long test_val = rand()%LLONG_MAX;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createTimestamp(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_scalar_nanotime)
{
    srand(time(NULL));
    long long test_val = rand()%LLONG_MAX;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createNanoTime(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_scalar_nanotimestamp)
{
    srand(time(NULL));
    long long test_val = rand()%LLONG_MAX;
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createNanoTimestamp(test_val);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_scalar_int128)
{
    srand(time(NULL));
    unsigned char int128[16];
    for (auto i = 0; i < 16; i++)
        int128[i] = rand() % CHAR_MAX;

    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createConstant(dolphindb::DT_INT128);
        object->setBinary(int128, sizeof(dolphindb::Guid));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_scalar_ip)
{
    srand(time(NULL));
    unsigned char ip[16];
    for (auto i = 0; i < 16; i++)
        ip[i] = rand() % CHAR_MAX;
    std::string test_val = dolphindb::Guid(ip).getString();

    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createConstant(dolphindb::DT_IP);
        object->setBinary(ip, sizeof(dolphindb::Guid));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_scalar_uuid)
{
    srand(time(NULL));
    unsigned char uuid[16];
    for (auto i = 0; i < 16; i++)
        uuid[i] = rand() % CHAR_MAX;

    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createConstant(dolphindb::DT_UUID);
        object->setBinary(uuid, sizeof(dolphindb::Guid));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

INSTANTIATE_TEST_SUITE_P(test_DataStream_scalar_Temporal_Null, SysIOTest_Parameterized, testing::Values(dolphindb::DT_DATE,dolphindb::DT_MONTH,dolphindb::DT_TIME,dolphindb::DT_MINUTE,dolphindb::DT_SECOND,dolphindb::DT_DATETIME,dolphindb::DT_TIMESTAMP,dolphindb::DT_NANOTIME,dolphindb::DT_NANOTIMESTAMP,dolphindb::DT_DATEHOUR));
TEST_P(SysIOTest_Parameterized, test_DataStream_scalar_Temporal_Null){
    dolphindb::DATA_TYPE test_type =  GetParam();
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createNullConstant(test_type);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }
}


INSTANTIATE_TEST_SUITE_P(test_DataStream_scalar_Integral_Null, SysIOTest_Parameterized, testing::Values(dolphindb::DT_UUID, dolphindb::DT_IP, dolphindb::DT_INT128));
TEST_P(SysIOTest_Parameterized, test_DataStream_scalar_Integral_Null){
    dolphindb::DATA_TYPE test_type =  GetParam();
    { // DDB scalar
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createNullConstant(test_type);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_SCALAR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }
}


TEST_F(SysIOTest, test_DataStream_vector_int)
{
    srand(time(NULL));
    int test_val = (int)rand()%INT_MAX;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_INT,1,1);
        object->set(0,dolphindb::Util::createInt(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_vector_bool)
{
    srand(time(NULL));
    char test_val = rand() % 2;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_BOOL,1,1);
        object->set(0,dolphindb::Util::createBool(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_vector_INDEX)
{
    srand(time(NULL));
    dolphindb::INDEX test_val = (dolphindb::INDEX)rand() % dolphindb::INDEX_MAX;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_INDEX,1,1);
        object->set(0,dolphindb::Util::createInt(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_vector_float)
{
    srand(time(NULL));
    float test_val = rand()/float(RAND_MAX);
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_FLOAT,1,1);
        object->set(0,dolphindb::Util::createFloat(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_vector_string)
{
    std::vector<std::string> rand_str = {"sd","dag","xxx","智臾科技a","23!@#$%","^&#%……@","/,m[[`"};
    std::string test_val = rand_str[rand() % rand_str.size()];
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_STRING,1,1);
        object->set(0,dolphindb::Util::createString(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_vector_date)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_DATE,1,1);
        object->set(0,dolphindb::Util::createDate(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_vector_month)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_MONTH,1,1);
        object->set(0,dolphindb::Util::createMonth(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_vector_time)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_TIME,1,1);
        object->set(0,dolphindb::Util::createTime(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_vector_minute)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_MINUTE,1,1);
        object->set(0,dolphindb::Util::createMinute(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_vector_second)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_SECOND,1,1);
        object->set(0,dolphindb::Util::createSecond(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_vector_datetime)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_DATETIME,1,1);
        object->set(0,dolphindb::Util::createDateTime(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_vector_timestamp)
{
    srand(time(NULL));
    long long test_val = rand()%LLONG_MAX;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_TIMESTAMP,1,1);
        object->set(0,dolphindb::Util::createTimestamp(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_vector_nanotime)
{
    srand(time(NULL));
    long long test_val = rand()%LLONG_MAX;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_NANOTIME,1,1);
        object->set(0,dolphindb::Util::createNanoTime(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_vector_nanotimestamp)
{
    srand(time(NULL));
    long long test_val = rand()%LLONG_MAX;
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_NANOTIMESTAMP,1,1);
        object->set(0,dolphindb::Util::createNanoTimestamp(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_vector_uuid)
{
    srand(time(NULL));
    dolphindb::Guid uuid=dolphindb::Guid(true);
    dolphindb::DdbVector<dolphindb::Guid> test_val(0, 1);
    test_val.add(uuid);

    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = test_val.createVector(dolphindb::DT_UUID);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_vector_symbol)
{
    std::vector<std::string> rand_str = {"sd","dag","xxx","智臾科技a","23!@#$%","^&#%……@","/,m[[`"};
    std::string test_val = rand_str[rand() % rand_str.size()];
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_SYMBOL,1,1);
        object->set(0,dolphindb::Util::createString(test_val));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_vector_ip)
{
    srand(time(NULL));
    dolphindb::Guid ip = dolphindb::Guid(true);
    dolphindb::DdbVector<dolphindb::Guid> test_val(0, 1);
    test_val.add(ip);

    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = test_val.createVector(dolphindb::DT_IP);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_vector_int128)
{
    srand(time(NULL));
    dolphindb::Guid int128 = dolphindb::Guid(true);
    dolphindb::DdbVector<dolphindb::Guid> test_val(0, 1);
    test_val.add(int128);

    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = test_val.createVector(dolphindb::DT_INT128);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


INSTANTIATE_TEST_SUITE_P(test_DataStream_vector_Null, SysIOTest_Parameterized, testing::Values(dolphindb::DT_BOOL,dolphindb::DT_CHAR,dolphindb::DT_SHORT,dolphindb::DT_INT,dolphindb::DT_LONG,dolphindb::DT_DATE,dolphindb::DT_MONTH,dolphindb::DT_TIME,dolphindb::DT_MINUTE,dolphindb::DT_SECOND,dolphindb::DT_DATETIME,dolphindb::DT_TIMESTAMP,dolphindb::DT_NANOTIME,dolphindb::DT_NANOTIMESTAMP,
    dolphindb::DT_FLOAT,dolphindb::DT_DOUBLE,dolphindb::DT_SYMBOL,dolphindb::DT_STRING,dolphindb::DT_UUID,dolphindb::DT_DATEHOUR,
    dolphindb::DT_IP,dolphindb::DT_INT128,dolphindb::DT_BLOB, dolphindb::DT_DECIMAL32, dolphindb::DT_DECIMAL64));
TEST_P(SysIOTest_Parameterized, test_DataStream_vector_Null){
    dolphindb::DATA_TYPE test_type =  GetParam();
    { // DDB std::vector
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP object = dolphindb::Util::createVector(test_type, 1);
        object->setNull(0);
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }
}

TEST_F(SysIOTest, test_DataStream_Vector_withAllType)
{
    srand(time(NULL));
    unsigned char uuid[16];
    for (auto i = 0; i < 16; i++)
        uuid[i] = rand() % CHAR_MAX;
    std::string uuidval = dolphindb::Guid(uuid).getString();
    char boolval = rand() % 2;
    char charval = rand() % CHAR_MAX;
    short shortval = rand() % SHRT_MAX;
    int intval = rand() % INT_MAX;
    long long longval = rand() % LLONG_MAX;
    float floatval = rand()/float(RAND_MAX);
    double doubleval = rand()/double(RAND_MAX);
    std::string strval = "dolphindb";

    dolphindb::ConstantSP object = dolphindb::Util::createVector(dolphindb::DT_ANY,19,19);
    object->set(0,dolphindb::Util::createBool(boolval));
    object->set(1,dolphindb::Util::createChar(charval));
    object->set(2,dolphindb::Util::createShort(shortval));
    object->set(3,dolphindb::Util::createInt(intval));
    object->set(4,dolphindb::Util::createLong(longval));
    object->set(5,dolphindb::Util::createFloat(floatval));
    object->set(6,dolphindb::Util::createDouble(doubleval));
    object->set(7,dolphindb::Util::createDate(intval));
    object->set(8,dolphindb::Util::createMonth(intval));
    object->set(9,dolphindb::Util::createTime(intval));
    object->set(10,dolphindb::Util::createMinute(intval));
    object->set(11,dolphindb::Util::createSecond(intval));
    object->set(12,dolphindb::Util::createDateTime(intval));
    object->set(13,dolphindb::Util::createTimestamp(longval));
    object->set(14,dolphindb::Util::createNanoTime(longval));
    object->set(15,dolphindb::Util::createNanoTimestamp(longval));
    object->set(16,dolphindb::Util::createString(strval));
    object->set(17,dolphindb::Util::createBlob(strval));
    object->set(18,dolphindb::Util::parseConstant(dolphindb::DT_UUID, uuidval));

    { // DDB std::vector
        char *pOutbuf;
        int outSize;

        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_VECTOR);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_table_int)
{
    srand(time(NULL));
    int test_val = (int)rand()%INT_MAX;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_INT, dolphindb::DT_INT};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createInt(test_val));
        object->set(1, 0, dolphindb::Util::createInt(test_val+1));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_table_bool)
{
    srand(time(NULL));
    char test_val = rand() % 2;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_BOOL, dolphindb::DT_BOOL};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createBool(test_val));
        object->set(1, 0, dolphindb::Util::createBool((char)(test_val+1)));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_table_INDEX)
{
    srand(time(NULL));
    dolphindb::INDEX test_val = (dolphindb::INDEX)rand() % dolphindb::INDEX_MAX;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_INDEX, dolphindb::DT_INDEX};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createInt(test_val));
        object->set(1, 0, dolphindb::Util::createInt(test_val+1));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_table_float)
{
    srand(time(NULL));
    float test_val = rand()/float(RAND_MAX);
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_FLOAT, dolphindb::DT_FLOAT};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createFloat(test_val+2.333));
        object->set(1, 0, dolphindb::Util::createFloat(test_val+1.231));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_table_string)
{
    std::vector<std::string> rand_str = {"sd","dag","xxx","智臾科技a","23!@#$%","^&#%……@","/,m[[`"};
    std::string test_val = rand_str[rand() % rand_str.size()];
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_STRING, dolphindb::DT_STRING};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createString(test_val));
        object->set(1, 0, dolphindb::Util::createString(""));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_table_date)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DATE, dolphindb::DT_DATE};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createDate(test_val));
        object->set(1, 0, dolphindb::Util::createDate(0));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_table_month)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_MONTH, dolphindb::DT_MONTH};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createMonth(test_val));
        object->set(1, 0, dolphindb::Util::createMonth(0));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_table_time)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIME, dolphindb::DT_TIME};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createTime(test_val));
        object->set(1, 0, dolphindb::Util::createTime(0));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_table_minute)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_MINUTE, dolphindb::DT_MINUTE};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createMinute(test_val));
        object->set(1, 0, dolphindb::Util::createMinute(0));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_table_second)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SECOND, dolphindb::DT_SECOND};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createSecond(test_val));
        object->set(1, 0, dolphindb::Util::createSecond(0));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_table_datetime)
{
    srand(time(NULL));
    int test_val = rand()%INT_MAX;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_DATETIME, dolphindb::DT_DATETIME};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createDateTime(test_val));
        object->set(1, 0, dolphindb::Util::createDateTime(0));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_table_timestamp)
{
    srand(time(NULL));
    long long test_val = rand()%LLONG_MAX;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_TIMESTAMP, dolphindb::DT_TIMESTAMP};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createTimestamp(test_val));
        object->set(1, 0, dolphindb::Util::createTimestamp(0));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_table_nanotime)
{
    srand(time(NULL));
    long long test_val = rand()%LLONG_MAX;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_NANOTIME, dolphindb::DT_NANOTIME};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createNanoTime(test_val));
        object->set(1, 0, dolphindb::Util::createNanoTime(0));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_table_nanotimestamp)
{
    srand(time(NULL));
    long long test_val = rand()%LLONG_MAX;
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_NANOTIMESTAMP, dolphindb::DT_NANOTIMESTAMP};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createNanoTimestamp(test_val));
        object->set(1, 0, dolphindb::Util::createNanoTimestamp(0));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_table_uuid)
{
    srand(time(NULL));
    dolphindb::Guid uuid=dolphindb::Guid(true);
    dolphindb::DdbVector<dolphindb::Guid> test_val(2, 2);
    test_val.set(0, uuid);
    test_val.setNull(1);

    { // DDB table
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP colVals = test_val.createVector(dolphindb::DT_UUID);
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_UUID, dolphindb::DT_UUID};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, colVals->get(0));
        object->set(1, 0, colVals->get(1));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_table_symbol)
{
    std::vector<std::string> rand_str = {"sd","dag","xxx","智臾科技a","23!@#$%","^&#%……@","/,m[[`"};
    std::string test_val = rand_str[rand() % rand_str.size()];
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_SYMBOL, dolphindb::DT_SYMBOL};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createString(test_val));
        object->set(1, 0, dolphindb::Util::createString(""));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}

TEST_F(SysIOTest, test_DataStream_table_ip)
{
    srand(time(NULL));
    dolphindb::Guid ip=dolphindb::Guid(true);
    dolphindb::DdbVector<dolphindb::Guid> test_val(2, 2);
    test_val.set(0,ip);
    test_val.setNull(1);

    { // DDB table
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP colVals = test_val.createVector(dolphindb::DT_IP);
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_IP, dolphindb::DT_IP};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, colVals->get(0));
        object->set(1, 0, colVals->get(1));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


TEST_F(SysIOTest, test_DataStream_table_int128)
{
    srand(time(NULL));
    dolphindb::Guid int128=dolphindb::Guid(true);
    dolphindb::DdbVector<dolphindb::Guid> test_val(2, 2);
    test_val.set(0,int128);
    test_val.setNull(1);

    { // DDB table
        char *pOutbuf;
        int outSize;
        dolphindb::ConstantSP colVals = test_val.createVector(dolphindb::DT_INT128);
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {dolphindb::DT_INT128, dolphindb::DT_INT128};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, colVals->get(0));
        object->set(1, 0, colVals->get(1));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}


INSTANTIATE_TEST_SUITE_P(test_DataStream_table_Null, SysIOTest_Parameterized, testing::Values(dolphindb::DT_BOOL,dolphindb::DT_CHAR,dolphindb::DT_SHORT,dolphindb::DT_INT,dolphindb::DT_LONG,dolphindb::DT_DATE,dolphindb::DT_MONTH,dolphindb::DT_TIME,dolphindb::DT_MINUTE,dolphindb::DT_SECOND,dolphindb::DT_DATETIME,dolphindb::DT_TIMESTAMP,dolphindb::DT_NANOTIME,dolphindb::DT_NANOTIMESTAMP,
    dolphindb::DT_FLOAT,dolphindb::DT_DOUBLE,dolphindb::DT_SYMBOL,dolphindb::DT_STRING,dolphindb::DT_UUID,dolphindb::DT_DATEHOUR,
    dolphindb::DT_IP,dolphindb::DT_INT128,dolphindb::DT_BLOB, dolphindb::DT_DECIMAL32, dolphindb::DT_DECIMAL64));
TEST_P(SysIOTest_Parameterized, test_DataStream_table_Null){
    dolphindb::DATA_TYPE test_type =  GetParam();
    { // DDB table
        char *pOutbuf;
        int outSize;
        std::vector<std::string> cols = {"col1", "col2"};
        std::vector<dolphindb::DATA_TYPE> colTypes = {test_type, test_type};
        dolphindb::ConstantSP object = dolphindb::Util::createTable(cols, colTypes, 1, 1);
        object->set(0, 0, dolphindb::Util::createNullConstant(test_type));
        object->set(1, 0, dolphindb::Util::createNullConstant(test_type));
        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }
}

TEST_F(SysIOTest, test_DataStream_table_withAllType)
{
    int colNum = 25, rowNum = 1;
    std::vector<std::string> colNamesVec1;
    for (int i = 0; i < colNum; i++){
        colNamesVec1.emplace_back("col"+std::to_string(i));
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
    colTypesVec1.emplace_back(dolphindb::DT_SYMBOL);

    srand((int)time(NULL));
    dolphindb::TableSP object = dolphindb::Util::createTable(colNamesVec1, colTypesVec1, rowNum, rowNum);
    std::vector<dolphindb::VectorSP> columnVecs;
    columnVecs.reserve(colNum);
    for (int i = 0; i < colNum; i++){
        columnVecs.emplace_back(object->getColumn(i));

    }
    for (int i = 0; i < rowNum-1; i++){
        columnVecs[0]->set(i, dolphindb::Util::createChar(rand()%CHAR_MAX));
        columnVecs[1]->set(i, dolphindb::Util::createBool((char)(rand()%2)));
        columnVecs[2]->set(i, dolphindb::Util::createShort(rand()%SHRT_MAX));
        columnVecs[3]->set(i, dolphindb::Util::createInt(rand()%INT_MAX));
        columnVecs[4]->set(i, dolphindb::Util::createLong(rand()%LLONG_MAX));
        columnVecs[5]->set(i, dolphindb::Util::createDate(rand()%INT_MAX));
        columnVecs[6]->set(i, dolphindb::Util::createMonth(rand()%INT_MAX));
        columnVecs[7]->set(i, dolphindb::Util::createTime(rand()%INT_MAX));
        columnVecs[8]->set(i, dolphindb::Util::createMinute(rand()%1440));
        columnVecs[9]->set(i, dolphindb::Util::createDateTime(rand()%INT_MAX));
        columnVecs[10]->set(i, dolphindb::Util::createSecond(rand()%86400));
        columnVecs[11]->set(i, dolphindb::Util::createTimestamp(rand()%LLONG_MAX));
        columnVecs[12]->set(i, dolphindb::Util::createNanoTime(rand()%LLONG_MAX));
        columnVecs[13]->set(i, dolphindb::Util::createNanoTimestamp(rand()%LLONG_MAX));
        columnVecs[14]->set(i, dolphindb::Util::createFloat(rand()/float(RAND_MAX)));
        columnVecs[15]->set(i, dolphindb::Util::createDouble(rand()/double(RAND_MAX)));
        columnVecs[16]->set(i, dolphindb::Util::createString("str"+std::to_string(i)));
        columnVecs[17]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_UUID,"5d212a78-cc48-e3b1-4235-b4d91473ee87"));
        columnVecs[18]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_IP,"192.0.0."+std::to_string(rand()%255)));
        columnVecs[19]->set(i, dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec32"));
        columnVecs[20]->set(i, dolphindb::Util::createBlob("blob"+std::to_string(i)));
        columnVecs[21]->set(i, dolphindb::Util::createDateHour(rand()%INT_MAX));
        columnVecs[22]->set(i, dolphindb::Util::createDecimal32(rand()%10,rand()/float(RAND_MAX)));
        columnVecs[23]->set(i, dolphindb::Util::createDecimal64(rand()%19,rand()/double(RAND_MAX)));
        columnVecs[24]->set(i, dolphindb::Util::createString("sym"+std::to_string(i)));
    }
    for (int j = 0; j < colNum; j++)
        columnVecs[j]->setNull(rowNum-1);

    { // DDB table
        char *pOutbuf;
        int outSize;

        dolphindb::DataOutputStreamSP outStream = new dolphindb::DataOutputStream();
        dolphindb::ConstantMarshallFactory marshallFactory(outStream);
        dolphindb::ConstantMarshall* marshall = marshallFactory.getConstantMarshall(object->getForm());
        dolphindb::IO_ERR ret;
        ASSERT_TRUE(marshall->start(object, true, false, ret));
        outSize = outStream->size();
        pOutbuf = new char[outSize];
        memcpy(pOutbuf, outStream->getBuffer(), outSize);
        marshall->reset();
        outStream->close();

        dolphindb::DataStreamSP inputStream = new dolphindb::DataStream(pOutbuf, outSize);
        short flag;
        inputStream->readShort(flag);
        dolphindb::ConstantUnmarshallFactory unmarshallFactory(inputStream);
        dolphindb::ConstantUnmarshall* unmarshall = unmarshallFactory.getConstantUnmarshall(dolphindb::DF_TABLE);
        dolphindb::IO_ERR ret2;
        ASSERT_TRUE(unmarshall->start(flag, true, ret2)) ;
        dolphindb::ConstantSP unmarsh_object=unmarshall->getConstant();
        ASSERT_EQ(unmarsh_object->getString(), object->getString());

        unmarshall->reset();
        inputStream->close();
        // delete[] pOutbuf; // when copy == false, pOutbuf can not be delete[].
    }

}



TEST_F(SysIOTest, test_class_Buffer)
{
    char buf1[]="aaaccc";
    char buf2[]="bbbddd";

    dolphindb::Buffer* bf1 = new dolphindb::Buffer();
    dolphindb::Buffer* bf2 = new dolphindb::Buffer(256);
    dolphindb::Buffer* bf3 = new dolphindb::Buffer(buf1, 256);
    dolphindb::Buffer* bf4 = new dolphindb::Buffer(buf2, -1, 256);
    std::size_t actlen;
    bf1->write(buf1,10,actlen);
    ASSERT_EQ((std::string)bf1->getBuffer(), buf1);

    bf2->write(buf2,20,actlen);
    ASSERT_EQ((std::string)bf2->getBuffer(), buf2);

    ASSERT_EQ((std::string)bf3->getBuffer(), buf1);
    ASSERT_EQ((std::string)bf4->getBuffer(), buf2);

    ASSERT_EQ(bf1->capacity(),256);
    ASSERT_EQ(bf2->capacity(),256);
    ASSERT_EQ(bf1->size(),10);
    ASSERT_EQ(bf2->size(),20);

    bf1->clear();
    ASSERT_EQ(bf1->size(), 0);
    std::string str = "lll1235678";
    bf2->clear();
    bf2->writeData(str);
    std::string bf2str(bf2->getBuffer(), str.length());
    ASSERT_EQ(bf2str, str);

    bf3->write(buf2, 7);
    ASSERT_EQ((std::string)bf3->getBuffer(), buf2);
}

#ifndef _WIN32
TEST_F(SysIOTest, test_DataStream_File){
    char workDir[256]{};
    getcwd(workDir, sizeof(workDir));
    std::string file = std::string(workDir).append(1, '/').append("tempFile123");
    {
        FILE* f = fopen(file.c_str(), "w+");
        ASSERT_NE(f, nullptr);
        dolphindb::DataStreamSP stream = new dolphindb::DataStream(f, true, true);
        char buf1[] = "Hello!\r\n";
        std::size_t sent = 0;
        stream->write(buf1, strlen(buf1), sent);
        char buf2[] = "My Name is";
        stream->writeLine(buf2, "\r\n");
        char buf3[] = "010!";
        stream->write(buf3, strlen(buf3), sent);
        std::cout << stream->getDescription() << std::endl;
        long long position;
        stream->seek(0, 0, position);
        ASSERT_EQ(position, 0);
    }
    remove(file.c_str());
}

TEST_F(SysIOTest, test_DataOutputStream_write){
    char workDir[256]{};
    getcwd(workDir, sizeof(workDir));
    std::string file = std::string(workDir).append(1, '/').append("tempFile123");
    { //FILE_STREAM
        FILE* f = fopen(file.c_str(), "w+");
        ASSERT_NE(f, nullptr);
        dolphindb::DataOutputStreamSP output = new dolphindb::DataOutputStream(f, true);
        char buf1[] = "Hello!\r\n";
        std::size_t sent = 0;
        ASSERT_EQ(output->write(buf1, strlen(buf1), sent), dolphindb::OK);
        std::string str = "My Name is\r\n";
        ASSERT_EQ(output->writeData(str), dolphindb::OK);
        char buf3[] = "汤姆克兰西！";
        ASSERT_EQ(output->write(buf3, strlen(buf3), sent), dolphindb::OK);
        ASSERT_EQ(output->close(), dolphindb::OK);

        std::ifstream ifs(file);
        std::string data = "";
        std::string line;
        while (std::getline(ifs, line)) {
            data.append(line);
            data.append("\n");
        }
        ifs.close();
        ASSERT_EQ(data, "Hello!\r\nMy Name is\r\n汤姆克兰西！\n");
    }
    { // FILE_STREAM
        FILE* f = fopen(file.c_str(), "r"); // readonly
        ASSERT_NE(f, nullptr);
        dolphindb::DataOutputStreamSP output = new dolphindb::DataOutputStream(f, true);
        size_t sent = 0;
        ASSERT_EQ(output->write("123456", 6, sent), dolphindb::OTHERERR);
    }
    remove(file.c_str());
}

#endif

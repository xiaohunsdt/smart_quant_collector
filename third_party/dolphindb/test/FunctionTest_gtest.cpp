#include <gtest/gtest.h>
#include "config.h"
#include "Set.h"
#include "ConstantImp.h"
#include "DFSChunkMeta.h"
#include "ConstantMarshall.h"
#include "TableImp.h"

class FunctionTest:public testing::Test
{
    public:
        static dolphindb::DBConnection conn;
        // Suite
        static void SetUpTestSuite() {
            bool ret = conn.connect(HOST, PORT, USER, PASSWD);
            if (!ret) {
                std::cout << "Failed to connect to the server" << std::endl;
            }
            else {
                std::string script = "a=int(1);\
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
                        w=set(1 2 3);\
                        x=matrix([0],[0]);\
                        y={\"sym\":123};\
                        z=int128(\"e1671797c52e15f763380b45e841ec32\");\
                        vec=1 2 3;\
                        sym=symbol(`a`b`c`d);";
                conn.run(script);
                std::cout << "connect to " + HOST + ":" + std::to_string(PORT) << std::endl;
            }
        }
        static void TearDownTestSuite(){
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

dolphindb::DBConnection FunctionTest::conn(false, false);

TEST_F(FunctionTest,test_function_get){
    dolphindb::TableSP tab1=conn.run("u");
    dolphindb::ConstantSP intval=conn.run("a");
    dolphindb::VectorSP vec1= conn.run("vec"); // 1 2 3
    dolphindb::VectorSP av1=conn.run("v"); // 1 2 3 9 9 9
    dolphindb::SetSP set1=conn.run("w"); // set(1 2 3)
    dolphindb::DictionarySP dict1=conn.run("y"); // {"sym":123}
    dolphindb::ConstantSP voidval=conn.run("d"); // NULL
    dolphindb::ConstantSP uuidval=conn.run("s");
    dolphindb::ConstantSP symval=conn.run("sym"); // symbol(`a`b`c`d)

    // const std::vector<std::string> ex_sym = {"a", "b", "c", "d"};
    // for(auto i=0;i<symval->getdolphindb::SymbolBase()->size();i++)
    //     ASSERT_EQ(symval->getdolphindb::SymbolBase()->getSymbol(i), ex_sym[i]);
    ASSERT_EQ(tab1->getObjectType(), dolphindb::CONSTOBJ);
    ASSERT_EQ(dolphindb::Util::getTableTypeString(tab1->getTableType()), "BASIC");
    ASSERT_EQ(vec1->get(0,0,1)->getString(), "[1]");
    ASSERT_EQ(vec1->getWindow(0,1,0,1)->getString(), "[1]");
    ASSERT_EQ(vec1->getSubVector(0,2,3)->getString(), "[1,2]");
    ASSERT_EQ(vec1->getSubVector(0,3)->getString(), "[1,2,3]");

    std::cout<<vec1->getAllocatedMemory(3)<<std::endl;
    ASSERT_EQ(dolphindb::Util::getDataTypeString(dict1->getRawType()), "ANY");
    ASSERT_EQ(dolphindb::Util::getCategoryString(dict1->getCategory()), "MIXED");
    ASSERT_EQ(dolphindb::Util::getDataTypeString(dict1->getKeyType()), "STRING");
    std::cout<<"----------------------------------------------"<<std::endl;

    ASSERT_THROW(dict1->getString(0),dolphindb::RuntimeException);

    ASSERT_ANY_THROW(tab1->getBool());
    ASSERT_ANY_THROW(tab1->getChar());
    ASSERT_ANY_THROW(tab1->getShort());
    ASSERT_ANY_THROW(tab1->getInt());
    ASSERT_ANY_THROW(tab1->getLong());
    ASSERT_ANY_THROW(voidval->getIndex());
    ASSERT_ANY_THROW(tab1->getFloat());
    ASSERT_ANY_THROW(tab1->getDouble());
    ASSERT_ANY_THROW(tab1->getBinary());
    ASSERT_ANY_THROW(vec1->getValueSize());
    ASSERT_ANY_THROW(dict1->getStringRef());
    ASSERT_ANY_THROW(dict1->get(1,1));
    ASSERT_ANY_THROW(dict1->getColumn(1));
    ASSERT_ANY_THROW(dict1->getRow(1));
    ASSERT_ANY_THROW(dict1->getItem(1));

    int buckets=2;
    ASSERT_EQ(voidval->getHash(buckets),-1);
    ASSERT_EQ(voidval->getString(),"");
    ASSERT_EQ(voidval->getStringRef(),dolphindb::Constant::EMPTY);
    ASSERT_EQ(voidval->getStringRef(0),dolphindb::Constant::EMPTY);
    
    ASSERT_EQ(intval->getIndex(0),1);
    ASSERT_EQ(intval->get(0)->getInt(),1);
    ASSERT_EQ(intval->get(0,0)->getInt(),1);
    ASSERT_EQ(intval->get(dolphindb::Util::createInt(0))->getInt(),1);
    ASSERT_EQ(intval->getColumn(0)->getInt(),1);
    ASSERT_EQ(intval->getWindow(0,1,0,1)->getInt(),1);
    vec1->setName("vec");
    ASSERT_EQ(vec1->getName(),"vec");
    ASSERT_EQ(set1->getRawType(),dolphindb::DT_INT);
    ASSERT_EQ(tab1->getRawType(),dolphindb::DT_DICTIONARY);
    ASSERT_EQ(dolphindb::Util::getCategoryString(intval->getCategory()),"INTEGRAL");
    ASSERT_EQ(dolphindb::Util::getCategoryString(set1->getCategory()),"INTEGRAL");
    ASSERT_EQ(dolphindb::Util::getCategoryString(tab1->getCategory()),"MIXED");

    char *buf = new char[2]; 
    short *buf1 = new short[2];
    int *buf2 = new int[2];
    dolphindb::INDEX *buf4 = new dolphindb::INDEX[2];
    long long *buf3 = new long long[2];
    float *buf5 = new float[2];
    double *buf6 = new double[2];
    std::string **buf8 = new std::string*[2];
    char **buf9 = new char*[2];
    unsigned char *buf10 = new unsigned char[2];
    dolphindb::SymbolBase *symbase= new dolphindb::SymbolBase(1);
    void *buf11=NULL;

    ASSERT_FALSE(uuidval->getBool(0,1,buf));
    ASSERT_FALSE(uuidval->getChar(0,1,buf));
    ASSERT_FALSE(uuidval->getShort(0,1,buf1)); 
    ASSERT_FALSE(uuidval->getInt(0,1,buf2));
    ASSERT_FALSE(uuidval->getLong(0,1,buf3));
    ASSERT_FALSE(uuidval->getIndex(0,1,buf4));
    ASSERT_FALSE(uuidval->getFloat(0,1,buf5));
    ASSERT_FALSE(uuidval->getDouble(0,1,buf6));   
    ASSERT_FALSE(uuidval->getSymbol(0,1,buf2,symbase,false)); 
    ASSERT_FALSE(uuidval->getString(0,1,buf8));
    ASSERT_FALSE(uuidval->getString(0,1,buf9));
    ASSERT_TRUE(voidval->getBinary(0,1,1,buf10));
    ASSERT_FALSE(voidval->getHash(0,1,buckets,buf2));

    ASSERT_ANY_THROW(uuidval->getBoolConst(0,1,buf));
    ASSERT_ANY_THROW(uuidval->getCharConst(0,1,buf));
    ASSERT_ANY_THROW(uuidval->getShortConst(0,1,buf1)); 
    ASSERT_ANY_THROW(uuidval->getIntConst(0,1,buf2));
    ASSERT_ANY_THROW(uuidval->getLongConst(0,1,buf3));
    ASSERT_ANY_THROW(uuidval->getIndexConst(0,1,buf4));
    ASSERT_ANY_THROW(uuidval->getFloatConst(0,1,buf5));
    ASSERT_ANY_THROW(uuidval->getDoubleConst(0,1,buf6));   
    ASSERT_ANY_THROW(uuidval->getSymbolConst(0,1,buf2,symbase,false)); 
    ASSERT_ANY_THROW(uuidval->getStringConst(0,1,buf8));
    ASSERT_ANY_THROW(uuidval->getStringConst(0,1,buf9));
    ASSERT_ANY_THROW(voidval->getBinaryConst(0,1,1,buf10));
    ASSERT_ANY_THROW(set1->get(dolphindb::Util::createInt(0)));
    ASSERT_ANY_THROW(set1->getStringRef());
    ASSERT_ANY_THROW(set1->get(0));
    ASSERT_ANY_THROW(set1->get(0,1));
    ASSERT_ANY_THROW(set1->getColumn(0));
    ASSERT_ANY_THROW(set1->getRow(0));
    ASSERT_ANY_THROW(set1->getItem(0));

    std::cout<<uuidval->getBoolBuffer(0,1,buf)<<std::endl;
    std::cout<<uuidval->getCharBuffer(0,1,buf)<<std::endl;
    std::cout<<uuidval->getShortBuffer(0,1,buf1)<<std::endl; 
    std::cout<<uuidval->getIntBuffer(0,1,buf2)<<std::endl;
    std::cout<<uuidval->getLongBuffer(0,1,buf3)<<std::endl;
    std::cout<<uuidval->getIndexBuffer(0,1,buf4)<<std::endl;
    std::cout<<uuidval->getFloatBuffer(0,1,buf5)<<std::endl;
    std::cout<<uuidval->getDoubleBuffer(0,1,buf6)<<std::endl;   
    std::cout<<voidval->getBinaryBuffer(0,1,1,buf10)<<std::endl;
    std::cout<<voidval->getDataBuffer(0,1,buf11)<<std::endl;
    std::cout<<uuidval->getAllocatedMemory()<<std::endl;
    std::cout<<set1->getAllocatedMemory()<<std::endl;

    ASSERT_ANY_THROW(uuidval->getMember(dolphindb::Util::createConstant(dolphindb::DT_INT)));
    ASSERT_ANY_THROW(uuidval->keys());
    ASSERT_ANY_THROW(uuidval->values());

    ASSERT_EQ(uuidval->getDataArray(),(void* )0);
    ASSERT_EQ(uuidval->getDataSegment(),(void** )0);
    ASSERT_EQ(uuidval->getIndexArray(),(dolphindb::INDEX* )NULL);
    ASSERT_EQ(uuidval->getHugeIndexArray(),(dolphindb::INDEX** )NULL);
    ASSERT_EQ(uuidval->getSegmentSize(),1);
    ASSERT_EQ(uuidval->getSegmentSizeInBit(),0);
    ASSERT_ANY_THROW(uuidval->castTemporal(dolphindb::DT_INT128));

    ASSERT_ANY_THROW(tab1->getColumn("abcd"));
    tab1->setName("table1");
    #ifndef _MSC_VER
        ASSERT_EQ(tab1->get(0)->getString(),"col2->a\ncol1->1\n");
    #else
        ASSERT_EQ(tab1->get(0)->getString(),"col1->1\ncol2->a\n");
    #endif
    ASSERT_EQ(tab1->getColumn("table1","col1")->getString(),"[1,2,3]");
    ASSERT_EQ(tab1->getColumn("table1","col1",0)->getString(),"[1,2,3]");
    ASSERT_EQ(tab1->getColumn(1,0)->getString(),"[\"a\",\"b\",\"c\"]");
    ASSERT_ANY_THROW(tab1->getColumn("table2","col2"));

    tab1->setColumnName(0, "col111");
    ASSERT_EQ(tab1->getColumnName(0),"col111");
    ASSERT_EQ(tab1->getColumnIndex("col111"),0);
    ASSERT_EQ(tab1->getColumnIndex("col1111"),-1);
    ASSERT_TRUE(tab1->contain("col111"));
    
    ASSERT_EQ(tab1->getColumnLabel()->getString(),"[\"col111\",\"col2\"]");
    ASSERT_EQ(tab1->values()->getString(),"([1,2,3],[\"a\",\"b\",\"c\"])");

    std::cout<<"-------------------------------------"<<std::endl;
    ASSERT_EQ(tab1->getWindow(0,2,0,2)->getColumn(0)->getValue()->getString(),"[1,2]");
    ASSERT_EQ(tab1->getWindow(0,2,0,2)->getColumn(1)->getValue()->getString(),"[\"a\",\"b\"]");

    ASSERT_EQ(tab1->getWindow(0,-1,0,2)->getColumn(0)->getValue()->getString(),"[1,2]");
    ASSERT_EQ(tab1->getWindow(0,1,0,1)->getColumn(0)->getValue()->getString(),"[1]");
    ASSERT_EQ(tab1->getMember(dolphindb::Util::createString("col111"))->getString(),"[1,2,3]");
    dolphindb::VectorSP memvec=dolphindb::Util::createVector(dolphindb::DT_STRING,2,2);
    memvec->set(0,dolphindb::Util::createString("col111"));
    memvec->set(1,dolphindb::Util::createString("col2"));
    ASSERT_EQ(tab1->getMember(memvec)->get(0)->getString(),"[1,2,3]");
    ASSERT_EQ(tab1->getMember(memvec)->get(1)->getString(),"[\"a\",\"b\",\"c\"]");
    ASSERT_EQ(tab1->getValue()->getColumn(0)->getString(),"[1,2,3]");
    ASSERT_EQ(tab1->getValue()->getColumn(1)->getString(),"[\"a\",\"b\",\"c\"]");
    ASSERT_EQ(tab1->getValue(0)->getColumn(0)->getString(),"[1,2,3]");
    ASSERT_EQ(tab1->getValue(0)->getColumn(1)->getString(),"[\"a\",\"b\",\"c\"]");
    ASSERT_EQ(tab1->getValue(2)->getColumn(0)->getString(),"[1,2,3]");
    ASSERT_EQ(tab1->getValue(2)->getColumn(1)->getString(),"[\"a\",\"b\",\"c\"]");
    ASSERT_EQ(tab1->getSubTable({0})->getForm(),dolphindb::DF_TABLE);
    ASSERT_EQ(tab1->getSubTable({0})->getRow(0)->getString(),tab1->getRow(0)->getString());

    std::cout<<tab1->getInstance(tab1->size())->getColumn(0)->getString()<<std::endl;
    std::cout<<tab1->getInstance(tab1->size())->getColumn(1)->getString()<<std::endl;

}

TEST_F(FunctionTest,test_function_is){
    dolphindb::TableSP tab1=conn.run("u");
    dolphindb::ConstantSP intval=conn.run("a");
    dolphindb::VectorSP vec1= conn.run("vec");
    dolphindb::VectorSP av1=conn.run("v");
    dolphindb::SetSP set1=conn.run("w");
    dolphindb::DictionarySP dict1=conn.run("y");
    dolphindb::ConstantSP voidval=conn.run("d");
    dolphindb::ConstantSP uuidval=conn.run("s");
    dolphindb::VectorSP matrixval=conn.run("x");
    ASSERT_FALSE(tab1->isDatabase());
    ASSERT_FALSE(intval->isDatabase());
    ASSERT_TRUE(intval->isNumber());
    char *buf = new char[2];
    ASSERT_FALSE(dict1->isValid(0,1,buf));
    ASSERT_FALSE(dict1->isNull(0,1,buf));
    ASSERT_FALSE(intval->isLargeConstant());
    ASSERT_FALSE(intval->isIndexArray());
    ASSERT_FALSE(vec1->isHugeIndexArray());
    ASSERT_ANY_THROW(av1->isSorted(true));
    ASSERT_TRUE(set1->isLargeConstant());
    ASSERT_TRUE(dict1->isLargeConstant());
    ASSERT_TRUE(tab1->isLargeConstant());

    ASSERT_FALSE(uuidval->isIndexArray());
    ASSERT_FALSE(uuidval->isHugeIndexArray());
    ASSERT_TRUE(matrixval->isLargeConstant());
    ASSERT_FALSE(vec1->isLargeConstant());
}

TEST_F(FunctionTest,test_function_set){
    dolphindb::VectorSP vec1= conn.run("vec");
    dolphindb::ConstantSP uuidval=conn.run("s");
    dolphindb::DictionarySP dict1=conn.run("y");
    dolphindb::ConstantSP voidval=conn.run("d");
    dolphindb::ConstantSP intval=conn.run("a");
    dolphindb::TableSP tab1=conn.run("u");

    intval->setChar((char)5);
    ASSERT_EQ(intval->getChar(),(char)5);
    intval->setLong((long long)5);
    ASSERT_EQ(intval->getLong(),(long long)5);
    intval->setIndex((dolphindb::INDEX)5);
    ASSERT_EQ(intval->getIndex(),(dolphindb::INDEX)5);
    intval->setFloat((float)5);
    ASSERT_EQ(intval->getFloat(),(float)5);
    unsigned char* val=new unsigned char[1];
    intval->setBinary(val,1);

    vec1->setChar(0,1);
    ASSERT_EQ(vec1->get(0)->getChar(),(char)1);
    vec1->setLong(0,1);
    ASSERT_EQ(vec1->get(0)->getLong(),(long long)1);
    vec1->setIndex(0,1);
    ASSERT_EQ(vec1->get(0)->getIndex(),(dolphindb::INDEX)1);
    vec1->setFloat(0,1);
    ASSERT_EQ(vec1->get(0)->getFloat(),(float)1);
    vec1->setNull(0);
    ASSERT_TRUE(vec1->get(0)->isNull());
    vec1->setItem(0,dolphindb::Util::createInt(6));
    ASSERT_EQ(vec1->get(0)->getInt(),6); 

    ASSERT_FALSE(uuidval->set(0,dolphindb::Util::createInt(1)));
    ASSERT_FALSE(uuidval->set(0,0,dolphindb::Util::createInt(1)));
    ASSERT_FALSE(uuidval->set(dolphindb::Util::createInt(0),dolphindb::Util::createInt(1)));
    ASSERT_FALSE(uuidval->setColumn(0,dolphindb::Util::createInt(1)));
    uuidval->setRowLabel(dolphindb::Util::createInt(1));
    uuidval->setColumnLabel(dolphindb::Util::createInt(1));
    uuidval->setNullFlag(false);

    char *buf = new char[2]; 
    short *buf1 = new short[2];
    int *buf2 = new int[2];
    dolphindb::INDEX *buf4 = new dolphindb::INDEX[2];
    long long *buf3 = new long long[2];
    float *buf5 = new float[2];
    double *buf6 = new double[2];
    char **buf9 = new char*[2];
    unsigned char *buf10 = new unsigned char[2];
    dolphindb::SymbolBase *symbase= new dolphindb::SymbolBase(1);
    void *buf11=NULL;

    ASSERT_FALSE(uuidval->setBool(0,1,buf));
    ASSERT_FALSE(uuidval->setChar(0,1,buf));
    ASSERT_FALSE(uuidval->setShort(0,1,buf1)); 
    ASSERT_FALSE(uuidval->setInt(0,1,buf2));
    ASSERT_FALSE(uuidval->setLong(0,1,buf3));
    ASSERT_FALSE(uuidval->setIndex(0,1,buf4));
    ASSERT_FALSE(uuidval->setFloat(0,1,buf5));
    ASSERT_FALSE(uuidval->setDouble(0,1,buf6));   
    ASSERT_FALSE(uuidval->setString(0,1,buf9));
    ASSERT_FALSE(voidval->setBinary(0,1,1,buf10));
    ASSERT_FALSE(voidval->setData(0,1,buf2));

    dict1->set("123",dolphindb::Util::createInt(1));
    ASSERT_FALSE(tab1->set(0,dolphindb::Util::createInt(5)));
    std::cout<<tab1->getString();

}

TEST_F(FunctionTest,test_function_has){}

TEST_F(FunctionTest,test_function_append){
    dolphindb::TableSP tab1=conn.run("u");
    std::string errMsg1;
    dolphindb::INDEX insertedRows=1;
    std::vector<dolphindb::ConstantSP> cols4={dolphindb::Util::createInt(4),dolphindb::Util::createString("d")};

    tab1->setReadOnly(true);
    ASSERT_FALSE(tab1->append(cols4,insertedRows,errMsg1));
    ASSERT_EQ(errMsg1,"Can't modify read only table.");
    tab1->setReadOnly(false);

    //append constant
    std::vector<std::string> colNames = { "col1", "col2" };
    std::vector<dolphindb::ConstantSP> err_colnum_values={dolphindb::Util::createInt(4)};
    ASSERT_FALSE(tab1->append(err_colnum_values,insertedRows,errMsg1));
    ASSERT_EQ(errMsg1, "The number of table columns doesn't match the number of columns to append.");

    errMsg1.clear();
    dolphindb::VectorSP tempVec=dolphindb::Util::createVector(dolphindb::DT_STRING,2,2);
    std::vector<dolphindb::ConstantSP> err_colsize_values={dolphindb::Util::createInt(4),tempVec};
    ASSERT_FALSE(tab1->append(err_colsize_values,insertedRows,errMsg1));
    ASSERT_EQ(errMsg1, "Inconsistent length of values to insert.");

    errMsg1.clear();
    std::vector<dolphindb::ConstantSP> err_coltype_values={dolphindb::Util::createString("2"),dolphindb::Util::createString("d")};
    ASSERT_FALSE(tab1->append(err_coltype_values,insertedRows,errMsg1));
    ASSERT_EQ(errMsg1, "Failed to append data to column 'col1' with error: Incompatible type. Expected: INT, Actual: STRING");


    //append std::tuple
    errMsg1.clear();
    dolphindb::VectorSP tuple_value=dolphindb::Util::createVector(dolphindb::DT_ANY,2,2);
    tuple_value->set(0,dolphindb::Util::createInt(4));
    tuple_value->set(1,dolphindb::Util::createString("d"));
    dolphindb::VectorSP err_colnum_tuple=dolphindb::Util::createVector(dolphindb::DT_ANY,1,2);
    err_colnum_tuple->set(0,dolphindb::Util::createInt(4));
    dolphindb::VectorSP err_coltype_tuple=dolphindb::Util::createVector(dolphindb::DT_ANY,2,2);
    err_coltype_tuple->set(0,dolphindb::Util::createString("4"));
    err_coltype_tuple->set(1,dolphindb::Util::createInt(5));

    std::vector<dolphindb::ConstantSP> tuple_values={tuple_value};
    ASSERT_TRUE(tab1->append(tuple_values,insertedRows,errMsg1));
    std::vector<dolphindb::ConstantSP> tuple_values1={err_colnum_tuple};
    ASSERT_FALSE(tab1->append(tuple_values1,insertedRows,errMsg1));
    std::vector<dolphindb::ConstantSP> tuple_values2={err_coltype_tuple};
    ASSERT_FALSE(tab1->append(tuple_values2,insertedRows,errMsg1));

    //append table
    std::cout<<tab1->getString();
    errMsg1.clear();
    std::vector<dolphindb::ConstantSP> tabVec={};
  
    std::vector<dolphindb::ConstantSP> errSizeCols={dolphindb::Util::createInt(4),dolphindb::Util::createString("d"),dolphindb::Util::createInt(0)};
    std::vector<std::string> errSizecolNames = { "col1", "col2", "col3"};
    dolphindb::TableSP tab_3_cols=dolphindb::Util::createTable(errSizecolNames, errSizeCols);
    tabVec.emplace_back(tab_3_cols);
    ASSERT_FALSE(tab1->append(tabVec,insertedRows,errMsg1));
    ASSERT_EQ(errMsg1,"Number of columns for the original table and the table to insert are different.");

    errMsg1.clear();
    tabVec.clear();
    std::vector<dolphindb::ConstantSP> errTypeCols={dolphindb::Util::createString("2"),dolphindb::Util::createString("d")};
    dolphindb::TableSP tab_errType_cols=dolphindb::Util::createTable(colNames, errTypeCols);
    tabVec.emplace_back(tab_errType_cols);
    ASSERT_FALSE(tab1->append(tabVec,insertedRows,errMsg1));
    ASSERT_EQ(errMsg1,"Failed to append data to column 'col1' with error: Incompatible type. Expected: INT, Actual: STRING");

}

TEST_F(FunctionTest,test_function_update){
    dolphindb::TableSP tab1=conn.run("u");
    std::vector<std::string> colNames = { "col1", "col2" };
    dolphindb::VectorSP indexs = dolphindb::Util::createIndexVector(0,1);

    dolphindb::VectorSP val1 = dolphindb::Util::createVector(dolphindb::DT_INT,1,1);
    val1->set(0,dolphindb::Util::createInt(5));
    dolphindb::VectorSP val2 = dolphindb::Util::createVector(dolphindb::DT_STRING,1,1);
    val2->set(0,dolphindb::Util::createString("f"));
    std::vector<dolphindb::ConstantSP> vector_cols={val1,val2};
    std::vector<dolphindb::ConstantSP> row={dolphindb::Util::createInt(4),dolphindb::Util::createString("d")};
    std::vector<dolphindb::ConstantSP> errtype_cols={dolphindb::Util::createString("4"),dolphindb::Util::createString("d")};
    std::vector<dolphindb::ConstantSP> err_colnum_values={dolphindb::Util::createVector(dolphindb::DT_INT,2,2)};
    std::string errMsg1;
    dolphindb::INDEX insertedRows=1;

    tab1->setReadOnly(true);
    ASSERT_FALSE(tab1->update(row,dolphindb::Util::createInt(0),colNames,errMsg1));
    ASSERT_EQ(errMsg1,"Can't modify read only table.");
    errMsg1.clear();
    tab1->setReadOnly(false); 

    ASSERT_TRUE(tab1->update(row,dolphindb::Util::createInt(2),colNames,errMsg1));
    // ASSERT_ANY_THROW(tab1->update(row,dolphindb::Util::createInt(3),colNames,errMsg1));
    tab1->update(errtype_cols,dolphindb::Util::createInt(2),colNames,errMsg1);
    ASSERT_EQ(errMsg1,"The category of the value to update does not match the column col1");

    errMsg1.clear();
    std::vector<std::string> err_colname={"col3","col2"};
    tab1->update(row,dolphindb::Util::createInt(2),err_colname,errMsg1);
    ASSERT_EQ(errMsg1,"The column col3 does not exist. To add a new column, the table shouldn't be shared and the value size must match the table.");

    errMsg1.clear();
    tab1->update(err_colnum_values,dolphindb::Util::createInt(2),colNames,errMsg1);
    ASSERT_EQ(errMsg1,"Inconsistent length of values to update.");

    errMsg1.clear();
    std::cout<<tab1->getString()<<std::endl;
    ASSERT_TRUE(tab1->update(row,indexs,colNames,errMsg1));
    std::cout<<tab1->getString()<<std::endl;
    ASSERT_EQ(tab1->getRow(0)->getMember(dolphindb::Util::createString("col1"))->getInt(),4);
    ASSERT_EQ(tab1->getRow(0)->getMember(dolphindb::Util::createString("col2"))->getString(),"d");

    errMsg1.clear();
    tab1->update(vector_cols,indexs,colNames,errMsg1);
    ASSERT_EQ(tab1->getRow(0)->getMember(dolphindb::Util::createString("col1"))->getInt(),5);
    ASSERT_EQ(tab1->getRow(0)->getMember(dolphindb::Util::createString("col2"))->getString(),"f");

}

TEST_F(FunctionTest,test_function_remove){
    dolphindb::TableSP tab1=conn.run("u");
    
    std::string errMsg1;
    dolphindb::VectorSP vec1=dolphindb::Util::createVector(dolphindb::DT_INT,0,1);
    vec1->append(dolphindb::Util::createInt(1));

    tab1->setReadOnly(true);
    ASSERT_FALSE(tab1->remove(vec1,errMsg1));
    ASSERT_EQ(errMsg1,"Can't remove rows from a read only in-memory table.");
    errMsg1.clear();
    tab1->setReadOnly(false);

    ASSERT_ANY_THROW(tab1->remove(dolphindb::Util::createInt(1),errMsg1)); //not surport remove with scalar parameter.
    ASSERT_TRUE(tab1->remove(vec1,errMsg1));

    errMsg1.clear();
    ASSERT_EQ(tab1->rows(),2);
    ASSERT_TRUE(tab1->remove(NULL,errMsg1));

    errMsg1.clear();
    ASSERT_EQ(tab1->rows(),0);
}

TEST_F(FunctionTest,test_function_drop){
    dolphindb::TableSP tab1=conn.run("u");

    tab1->setReadOnly(true);
    std::vector<int> *dropColsIndex=new std::vector<int>;
    dropColsIndex->push_back(1);
    ASSERT_ANY_THROW(tab1->drop(*dropColsIndex));
    tab1->setReadOnly(false);

    dropColsIndex->clear();
    dropColsIndex->push_back(1);
    tab1->drop(*dropColsIndex);
    ASSERT_EQ(tab1->columns(),1);

    dolphindb::TableSP tab2=conn.run("u");
    dropColsIndex->clear();
    dropColsIndex->push_back(2);
    tab2->drop(*dropColsIndex);
    ASSERT_EQ(tab2->columns(),2);

    dolphindb::TableSP tab3=conn.run("u");
    dropColsIndex->clear();
    dropColsIndex->push_back(0);
    dropColsIndex->push_back(1);
    tab3->drop(*dropColsIndex);
    ASSERT_EQ(tab3->columns(),0);

}

TEST_F(FunctionTest,test_Util_functions){
    std::cout << dolphindb::Util::escape((char)14)<<std::endl;

    ASSERT_EQ(dolphindb::Util::getMonthEnd(1),30);
    ASSERT_EQ(dolphindb::Util::getMonthEnd(31),58);
    ASSERT_EQ(dolphindb::Util::getMonthStart(1),0);
    ASSERT_EQ(dolphindb::Util::getMonthStart(60),59);

    const char *buf = dolphindb::Util::allocateMemory(10);
    buf="0123456789";
    ASSERT_TRUE(dolphindb::Util::allocateMemory(-1,false)==NULL);
    ASSERT_ANY_THROW(dolphindb::Util::allocateMemory(-1));

    dolphindb::DictionarySP dict1=dolphindb::Util::createDictionary(dolphindb::DT_TIME,dolphindb::DT_ANY);
    dolphindb::DictionarySP dict2=dolphindb::Util::createDictionary(dolphindb::DT_STRING,dolphindb::DT_ANY);
    dolphindb::DictionarySP dict3=dolphindb::Util::createDictionary(dolphindb::DT_STRING,dolphindb::DT_DATEHOUR);
    ASSERT_FALSE(dolphindb::Util::isFlatDictionary(dict1.get()));
    ASSERT_TRUE(dolphindb::Util::isFlatDictionary(dict2.get()));
    ASSERT_TRUE(dolphindb::Util::isFlatDictionary(dict3.get()));
    dict2->set(dolphindb::Util::createString("str1"),dolphindb::Util::createNullConstant(dolphindb::DT_INT));
    ASSERT_FALSE(dolphindb::Util::isFlatDictionary(dict2.get()));
    dict3->set(dolphindb::Util::createString("str1"),dolphindb::Util::createDateHour(1000));
    ASSERT_FALSE(dolphindb::Util::isFlatDictionary(dict3.get()));

    ASSERT_EQ(dolphindb::Util::getDataType("int"),dolphindb::DT_INT);
    ASSERT_EQ(dolphindb::Util::getDataForm("vector"),dolphindb::DF_VECTOR);
    ASSERT_EQ(dolphindb::Util::getDataTypeString(dolphindb::DT_INT),"INT");
    ASSERT_EQ(dolphindb::Util::getDataFormString(dolphindb::DF_VECTOR),"VECTOR");

    ASSERT_EQ(dolphindb::Util::getDataType('v'),dolphindb::DT_VOID);
    ASSERT_EQ(dolphindb::Util::getDataType('b'),dolphindb::DT_BOOL);
    ASSERT_EQ(dolphindb::Util::getDataType('c'),dolphindb::DT_CHAR);
    ASSERT_EQ(dolphindb::Util::getDataType('h'),dolphindb::DT_SHORT);
    ASSERT_EQ(dolphindb::Util::getDataType('i'),dolphindb::DT_INT);
    ASSERT_EQ(dolphindb::Util::getDataType('f'),dolphindb::DT_FLOAT);
    ASSERT_EQ(dolphindb::Util::getDataType('F'),dolphindb::DT_DOUBLE);
    ASSERT_EQ(dolphindb::Util::getDataType('d'),dolphindb::DT_DATE);
    ASSERT_EQ(dolphindb::Util::getDataType('M'),dolphindb::DT_MONTH);
    ASSERT_EQ(dolphindb::Util::getDataType('m'),dolphindb::DT_MINUTE);
    ASSERT_EQ(dolphindb::Util::getDataType('s'),dolphindb::DT_SECOND);
    ASSERT_EQ(dolphindb::Util::getDataType('t'),dolphindb::DT_TIME);
    ASSERT_EQ(dolphindb::Util::getDataType('D'),dolphindb::DT_DATETIME);
    ASSERT_EQ(dolphindb::Util::getDataType('T'),dolphindb::DT_TIMESTAMP);
    ASSERT_EQ(dolphindb::Util::getDataType('n'),dolphindb::DT_NANOTIME);
    ASSERT_EQ(dolphindb::Util::getDataType('N'),dolphindb::DT_NANOTIMESTAMP);
    ASSERT_EQ(dolphindb::Util::getDataType('S'),dolphindb::DT_SYMBOL);
    ASSERT_EQ(dolphindb::Util::getDataType('W'),dolphindb::DT_STRING);
    ASSERT_EQ(dolphindb::Util::getDataType('l'),dolphindb::DT_LONG);

    dolphindb::VectorSP matrixval = dolphindb::Util::createDoubleMatrix(1,1);
    std::string ex_martval=conn.run("matrix(DOUBLE,1,1)")->getString();
    ASSERT_EQ(ex_martval,matrixval->getString());
    ASSERT_TRUE(matrixval->isMatrix());
    ASSERT_EQ(matrixval->getForm(),dolphindb::DF_MATRIX);
    ASSERT_EQ(matrixval->getType(),dolphindb::DT_DOUBLE);

    dolphindb::VectorSP indexvec=dolphindb::Util::createIndexVector(-1,-1);
    dolphindb::VectorSP indexvec1=dolphindb::Util::createIndexVector(0,1);
    std::cout<<indexvec->getString()<<std::endl;
    std::string ex_indexvec=conn.run("array(INDEX,1,1)")->getString();
    ASSERT_EQ(indexvec1->getString(),ex_indexvec);

    ASSERT_EQ(dolphindb::Util::trim(" 1 2 3      "),"1 2 3");
    ASSERT_EQ(dolphindb::Util::ltrim("   1 2 3      "), "1 2 3      ");

    ASSERT_EQ(dolphindb::Util::strip(" \t\r\n 1 2 3 \t\n\r"),"1 2 3");
    ASSERT_EQ(dolphindb::Util::wc("1 23 4 abc A *&^%$#!\t\n\r"),5);

    ASSERT_EQ(dolphindb::Util::replace("abc","d","e"),"abc");
    ASSERT_EQ(dolphindb::Util::replace("abc","c","e"),"abe");
    ASSERT_EQ(dolphindb::Util::replace("abc","a","cba"),"cbabc");

    ASSERT_EQ(dolphindb::Util::replace("abc",'d','e'),"abc");
    ASSERT_EQ(dolphindb::Util::replace("abc",'c','e'),"abe");
    // ASSERT_EQ(dolphindb::Util::replace("abc",'a','cba'),"cbabc");

    ASSERT_EQ(dolphindb::Util::upper("abc"),"ABC");
    ASSERT_EQ(dolphindb::Util::toUpper('a'),'A');

    ASSERT_EQ(dolphindb::Util::longToString((long long)999999999999999),"999999999999999");
    ASSERT_EQ(dolphindb::Util::doubleToString((double)2.321597810),"2.321598");

    ASSERT_FALSE(dolphindb::Util::endWith("dolphindb", ""));
    ASSERT_FALSE(dolphindb::Util::endWith("dolphindb", "nihao"));
    ASSERT_TRUE(dolphindb::Util::endWith("dolphindb", "db"));

    ASSERT_FALSE(dolphindb::Util::startWith("dolphindb", ""));
    ASSERT_FALSE(dolphindb::Util::startWith("dolphindb", "nihao"));
    ASSERT_TRUE(dolphindb::Util::startWith("dolphindb", "dolphin"));

    std::string teststrval="abc\"123\" dolphindb";
    ASSERT_EQ(dolphindb::Util::literalConstant(teststrval),"\"abc\\\"123\\\" dolphindb\"");

    std::cout<<dolphindb::Util::getNanoBenchmark()<<std::endl;
    std::cout<<dolphindb::Util::getNanoEpochTime()<<std::endl;
    tm local_time;
    dolphindb::Util::getLocalTime(local_time);
    std::cout<<std::to_string(1900+local_time.tm_year)+"."+std::to_string(1+local_time.tm_mon)+"."+std::to_string(local_time.tm_mday)+" "+std::to_string(local_time.tm_hour)+":"+std::to_string(local_time.tm_min)+":"+std::to_string(local_time.tm_sec)<<std::endl;
    
    int* timeval_int=new int[1];
    timeval_int[0]=60;
    dolphindb::Util::toLocalDateTime(timeval_int,1);
    ASSERT_EQ(timeval_int[0],dolphindb::Util::toLocalDateTime(60));

    // long long* timeval_long=new long long[1];
    // timeval_long[0]=(long long)10000000000;
    // dolphindb::Util::toLocalNanoTimestamp(timeval_long,1);
    // ASSERT_EQ(timeval_long[0],dolphindb::Util::toLocalNanoTimestamp((long long)10000000000));

    long long* timeval_long2=new long long[1];
    timeval_long2[0]=(long long)10000000000;
    dolphindb::Util::toLocalTimestamp(timeval_long2,1);
    ASSERT_EQ(timeval_long2[0],dolphindb::Util::toLocalTimestamp((long long)10000000000));

    ASSERT_FALSE(dolphindb::Util::strWildCmp("dolphindb","DolphinDB"));
    ASSERT_TRUE(dolphindb::Util::strWildCmp("DolphinDB","DolphinDB"));

    std::string dest = "dolphindb";
    std::string source = "1";
    dolphindb::Util::writeDoubleQuotedString(dest,source);
    ASSERT_EQ(dest,"dolphindb\"1\"");

    std::cout<<dolphindb::Util::getLastErrorCode()<<std::endl;
    std::cout<<dolphindb::Util::getLastErrorMessage()<<std::endl;
    std::cout<<dolphindb::Util::getErrorMessage(dolphindb::Util::getLastErrorCode())<<std::endl;

    ASSERT_EQ(dolphindb::Util::getPartitionTypeString(dolphindb::VALUE),"VALUE");

    std::cout<<"-----------test dolphindb::Util::createObject()--------------"<<std::endl;
    std::nullptr_t voidconst = nullptr;
    bool boolconst = true;
    char charconst = 1;
    short shortconst = 1;
    const char* pcharconst = "1";
    std::string strconst = "dolphindb";
    // unsigned char charconst2 = 1;
    // const unsigned char* pval = "1";
    unsigned char charconstvec[] = {1};
    long long longconst = 1;
    long int longintconst = 1;
    int intconst = 1;
    float floatconst = 1;
    double doubleconst = 1;
    std::vector<dolphindb::DATA_TYPE> testTypes = {dolphindb::DT_BOOL,dolphindb::DT_CHAR,dolphindb::DT_SHORT,dolphindb::DT_INT,dolphindb::DT_LONG,dolphindb::DT_DATE,dolphindb::DT_MONTH,dolphindb::DT_TIME,
                                    dolphindb::DT_MINUTE,dolphindb::DT_SECOND,dolphindb::DT_DATETIME,dolphindb::DT_TIMESTAMP,dolphindb::DT_NANOTIME,dolphindb::DT_NANOTIMESTAMP,
                                    dolphindb::DT_FLOAT,dolphindb::DT_DOUBLE,dolphindb::DT_SYMBOL,dolphindb::DT_STRING,dolphindb::DT_UUID,dolphindb::DT_DATEHOUR,dolphindb::DT_IP,dolphindb::DT_INT128,dolphindb::DT_BLOB };

    for(int i =0;i<testTypes.size();i++){
        dolphindb::ConstantSP ddbval=dolphindb::Util::createObject((dolphindb::DATA_TYPE)testTypes[i],voidconst);
        ASSERT_TRUE(ddbval->getType()==(dolphindb::DATA_TYPE)testTypes[i] || ddbval->getType()==dolphindb::DT_STRING);
        ASSERT_TRUE(ddbval->isNull());
    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        switch (dataType) {
        case dolphindb::DATA_TYPE::DT_BOOL:
        {
            dolphindb::ConstantSP ddbval=dolphindb::Util::createObject(dataType,boolconst);
            ASSERT_EQ(ddbval->getBool(),boolconst);
            ASSERT_TRUE(ddbval->getType()==dataType);
            break;
        }
        default:
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,boolconst));
            break;
        }
        }
    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if(i<5){
            dolphindb::ConstantSP ddbval=dolphindb::Util::createObject(dataType,charconst);
            ASSERT_EQ(ddbval->getBool(),charconst);
            ASSERT_TRUE(ddbval->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,charconst));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if(i<5 && i!=0){
            dolphindb::ConstantSP ddbval=dolphindb::Util::createObject(dataType,shortconst);
            ASSERT_EQ(ddbval->getShort(),shortconst);
            ASSERT_TRUE(ddbval->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,shortconst));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if((i<14 && i!=0) || i==19){
            dolphindb::ConstantSP ddbval=dolphindb::Util::createObject(dataType,longconst);
            ASSERT_EQ(ddbval->getLong(),longconst);
            ASSERT_TRUE(ddbval->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,longconst));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if((i<14 && i!=0) || i==19){
            dolphindb::ConstantSP ddbval=dolphindb::Util::createObject(dataType,longintconst);
            ASSERT_EQ(ddbval->getInt(),longintconst);
            ASSERT_TRUE(ddbval->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,longintconst));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if((i<11 && i!=0) || i==19){
            dolphindb::ConstantSP ddbval=dolphindb::Util::createObject(dataType,intconst);
            ASSERT_EQ(ddbval->getInt(),intconst);
            ASSERT_TRUE(ddbval->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,intconst));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if(i==14||i==15) {
            dolphindb::ConstantSP ddbval=dolphindb::Util::createObject(dataType,doubleconst);
            ASSERT_EQ(ddbval->getDouble(),doubleconst);
            ASSERT_TRUE(ddbval->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,doubleconst));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if(i==14||i==15) {
            dolphindb::ConstantSP ddbval=dolphindb::Util::createObject(dataType,floatconst);
            ASSERT_EQ(ddbval->getFloat(),floatconst);
            ASSERT_TRUE(ddbval->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,floatconst));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if( i==16 || i==17 || i==22){
            dolphindb::ConstantSP ddbval=dolphindb::Util::createObject(dataType,pcharconst);
            ASSERT_EQ(ddbval->getString(),"1");
            ASSERT_TRUE(ddbval->getType()==dataType || ddbval->getType()==dolphindb::DT_STRING);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,pcharconst));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if(i==16 || i==17 || i==22){
            dolphindb::ConstantSP ddbval=dolphindb::Util::createObject(dataType,strconst);
            ASSERT_EQ(ddbval->getString(),strconst);
            ASSERT_TRUE(ddbval->getType()==dataType || ddbval->getType()==dolphindb::DT_STRING);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,strconst));
        }

    }
    std::vector<std::nullptr_t> voidconstVec = {nullptr,nullptr};
    std::vector<bool> boolconstVec = {true};
    std::vector<char> charconstVec = {1};
    std::vector<short> shortconstVec = {1};
    std::vector<const char*> pcharconstVec = {"1"};
    std::vector<std::string> strconstVec = {"dolphindb"};
    std::vector<unsigned char> charconst2Vec = {1};
    std::vector<const unsigned char*> pvalVec = {&charconst2Vec[0]};
    std::vector<long long> longconstVec = {1};
    std::vector<long int> longintconstVec = {1};
    std::vector<int> intconstVec = {1};
    std::vector<float> floatconstVec = {1};
    std::vector<double> doubleconstVec = {1};

    for(int i =0;i<testTypes.size();i++){
        dolphindb::VectorSP ddbval=dolphindb::Util::createObject(testTypes[i],voidconstVec);
        ASSERT_EQ(ddbval->getForm(), dolphindb::DF_VECTOR);
        ASSERT_TRUE(ddbval->get(0)->getType()==(dolphindb::DATA_TYPE)testTypes[i] || ddbval->getType()==dolphindb::DT_STRING);
        ASSERT_TRUE(ddbval->get(0)->get(0)->isNull());
        ASSERT_TRUE(ddbval->get(0)->get(1)->isNull());
        // ASSERT_EQ(ddbval->get(0)->isNull());
    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if(i<5){
            dolphindb::VectorSP ddbval=dolphindb::Util::createObject(dataType,charconstVec);
            ASSERT_EQ(ddbval->getForm(), dolphindb::DF_VECTOR);
            ASSERT_EQ(ddbval->get(0)->getChar(),charconstVec[0]);
            ASSERT_TRUE(ddbval->get(0)->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,charconstVec));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,charconst2Vec));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if(i<5 && i!=0){
            dolphindb::VectorSP ddbval=dolphindb::Util::createObject(dataType,shortconstVec);
            ASSERT_EQ(ddbval->getForm(), dolphindb::DF_VECTOR);
            ASSERT_EQ(ddbval->get(0)->getShort(),shortconstVec[0]);
            ASSERT_TRUE(ddbval->get(0)->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,shortconstVec));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if((i<14 && i!=0) || i==19){
            dolphindb::VectorSP ddbval=dolphindb::Util::createObject(dataType,longconstVec);
            ASSERT_EQ(ddbval->getForm(), dolphindb::DF_VECTOR);
            ASSERT_EQ(ddbval->get(0)->getLong(),longconstVec[0]);
            ASSERT_TRUE(ddbval->get(0)->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,longconstVec));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if((i<14 && i!=0) || i==19){
            dolphindb::VectorSP ddbval=dolphindb::Util::createObject(dataType,longintconstVec);
            ASSERT_EQ(ddbval->getForm(), dolphindb::DF_VECTOR);
            ASSERT_EQ(ddbval->get(0)->getInt(),longintconstVec[0]);
            ASSERT_TRUE(ddbval->get(0)->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,longintconstVec));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if((i<11 && i!=0) || i==19){
            dolphindb::VectorSP ddbval=dolphindb::Util::createObject(dataType,intconstVec);
            ASSERT_EQ(ddbval->getForm(), dolphindb::DF_VECTOR);
            ASSERT_EQ(ddbval->get(0)->getInt(),intconstVec[0]);
            ASSERT_TRUE(ddbval->get(0)->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,intconstVec));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if(i==14||i==15) {
            dolphindb::VectorSP ddbval=dolphindb::Util::createObject(dataType,doubleconstVec);
            ASSERT_EQ(ddbval->getForm(), dolphindb::DF_VECTOR);
            ASSERT_EQ(ddbval->get(0)->getDouble(),doubleconstVec[0]);
            ASSERT_TRUE(ddbval->get(0)->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,doubleconstVec));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if(i==14||i==15) {
            dolphindb::VectorSP ddbval=dolphindb::Util::createObject(dataType,floatconstVec);
            ASSERT_EQ(ddbval->getForm(), dolphindb::DF_VECTOR);
            ASSERT_EQ(ddbval->get(0)->getFloat(),floatconstVec[0]);
            ASSERT_TRUE(ddbval->get(0)->getType()==dataType);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,floatconstVec));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if( i==16 || i==17 || i==22){
            dolphindb::VectorSP ddbval=dolphindb::Util::createObject(dataType,pcharconstVec);
            ASSERT_EQ(ddbval->get(0)->getString(),"[\"1\"]");
            ASSERT_TRUE(ddbval->get(0)->getType()==dataType || ddbval->get(0)->getType()==dolphindb::DT_STRING);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,pcharconstVec));
        }

    }

    for(int i =0;i<testTypes.size();i++){
        dolphindb::DATA_TYPE dataType = (dolphindb::DATA_TYPE)testTypes[i];
        if(i==16 || i==17 || i==22){
            dolphindb::VectorSP ddbval=dolphindb::Util::createObject(dataType,strconstVec);
            ASSERT_EQ(ddbval->get(0)->getString(),"[\""+strconstVec[0]+"\"]");
            ASSERT_TRUE(ddbval->get(0)->getType()==dataType || ddbval->get(0)->getType()==dolphindb::DT_STRING);
        }
        else
        {
            ASSERT_ANY_THROW(dolphindb::Util::createObject(dataType,strconstVec));
        }

    }


    std::cout<<"-----------test dolphindb::Util::parseConstant()--------------"<<std::endl;
    ASSERT_EQ(dolphindb::Util::parseYear(365),1971);
    ASSERT_EQ(dolphindb::Util::parseYear(0),1970);
    int year,month,day;
    dolphindb::Util::parseDate(365,year,month,day);
    std::cout<<year<<month<<day<<std::endl;
    ASSERT_EQ(year,1971);
    ASSERT_EQ(month,1);
    ASSERT_EQ(day,1);

    dolphindb::ConstantSP voidval=dolphindb::Util::parseConstant(dolphindb::DT_VOID,"");
    dolphindb::ConstantSP boolval=dolphindb::Util::parseConstant(dolphindb::DT_BOOL,"1");
    dolphindb::ConstantSP charval=dolphindb::Util::parseConstant(dolphindb::DT_CHAR,"1");
    dolphindb::ConstantSP shortval=dolphindb::Util::parseConstant(dolphindb::DT_SHORT,"1");
    dolphindb::ConstantSP intval=dolphindb::Util::parseConstant(dolphindb::DT_INT,"1");
    dolphindb::ConstantSP longval=dolphindb::Util::parseConstant(dolphindb::DT_LONG,"1");
    dolphindb::ConstantSP dateval=dolphindb::Util::parseConstant(dolphindb::DT_DATE,"2013.06.13");
    dolphindb::ConstantSP monthval=dolphindb::Util::parseConstant(dolphindb::DT_MONTH,"2012.06");
    dolphindb::ConstantSP timeval=dolphindb::Util::parseConstant(dolphindb::DT_TIME,"13:30:10.008");
    dolphindb::ConstantSP minuteval=dolphindb::Util::parseConstant(dolphindb::DT_MINUTE,"13:30");
    dolphindb::ConstantSP secondval=dolphindb::Util::parseConstant(dolphindb::DT_SECOND,"13:30:10");
    dolphindb::ConstantSP datetimeval=dolphindb::Util::parseConstant(dolphindb::DT_DATETIME,"2012.06.13T13:30:10");
    dolphindb::ConstantSP timestampval=dolphindb::Util::parseConstant(dolphindb::DT_TIMESTAMP,"2012.06.13T13:30:10.008");
    dolphindb::ConstantSP nanotimeval=dolphindb::Util::parseConstant(dolphindb::DT_NANOTIME,"13:30:10.008007006");
    dolphindb::ConstantSP nanotimestampval=dolphindb::Util::parseConstant(dolphindb::DT_NANOTIMESTAMP,"2012.06.13T13:30:10.008007006");
    dolphindb::ConstantSP floatval=dolphindb::Util::parseConstant(dolphindb::DT_FLOAT,"2.1");
    dolphindb::ConstantSP doubleval=dolphindb::Util::parseConstant(dolphindb::DT_DOUBLE,"2.1");
    // dolphindb::ConstantSP symbolval=dolphindb::Util::parseConstant(dolphindb::DT_SYMBOL,"sym"); //not support
    dolphindb::ConstantSP stringval=dolphindb::Util::parseConstant(dolphindb::DT_STRING,"str");
    dolphindb::ConstantSP uuidval=dolphindb::Util::parseConstant(dolphindb::DT_UUID,"5d212a78-cc48-e3b1-4235-b4d91473ee87");
    // dolphindb::ConstantSP functiondefval=dolphindb::Util::parseConstant(dolphindb::DT_FUNCTIONDEF,"def f1(a,b) {return a+b;}"); //not support
    dolphindb::ConstantSP datehourval=dolphindb::Util::parseConstant(dolphindb::DT_DATEHOUR,"2012.06.13T13");
    dolphindb::ConstantSP ipaddrval=dolphindb::Util::parseConstant(dolphindb::DT_IP,"192.168.1.13");
    dolphindb::ConstantSP int128val=dolphindb::Util::parseConstant(dolphindb::DT_INT128,"e1671797c52e15f763380b45e841ec32");
    // dolphindb::ConstantSP blobval=dolphindb::Util::parseConstant(dolphindb::DT_BLOB,"blob1"); //not support
    std::vector<std::string> nameVec = {"voidval","boolval","charval","shortval","intval","longval","dateval","monthval","timeval","minuteval","secondval",\
                                "datetimeval","timestampval","nanotimeval","nanotimestampval","floatval","doubleval","stringval","uuidval",\
                                "datehourval","ipaddrval","int128val"};
    std::vector<dolphindb::ConstantSP> valVec = {voidval,boolval,charval,shortval,intval,longval,dateval,monthval,timeval,minuteval,secondval,datetimeval,timestampval,\
                                    nanotimeval,nanotimestampval,floatval,doubleval,stringval,uuidval,datehourval,ipaddrval,int128val};
    conn.upload(nameVec,valVec);

    ASSERT_TRUE(conn.run("eqObj(voidval,NULL)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(boolval,true)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(charval,char(1))")->getBool());
    ASSERT_TRUE(conn.run("eqObj(shortval,short(1))")->getBool());
    ASSERT_TRUE(conn.run("eqObj(intval,int(1))")->getBool());
    ASSERT_TRUE(conn.run("eqObj(longval,long(1))")->getBool());
    ASSERT_TRUE(conn.run("eqObj(dateval,2013.06.13)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(monthval,2012.06M)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(timeval,13:30:10.008)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(minuteval,13:30m)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(secondval,13:30:10)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(datetimeval,2012.06.13T13:30:10)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(timestampval,2012.06.13T13:30:10.008)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(nanotimeval,13:30:10.008007006)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(nanotimestampval,2012.06.13T13:30:10.008007006)")->getBool());
    ASSERT_TRUE(conn.run("eqObj(floatval,float(2.1))")->getBool());
    ASSERT_TRUE(conn.run("eqObj(doubleval,double(2.1))")->getBool());
    ASSERT_TRUE(conn.run("eqObj(stringval,\"str\")")->getBool());
    ASSERT_TRUE(conn.run("eqObj(uuidval,uuid('5d212a78-cc48-e3b1-4235-b4d91473ee87'))")->getBool());
    ASSERT_TRUE(conn.run("eqObj(datehourval,datehour('2012.06.13T13'))")->getBool());
    ASSERT_TRUE(conn.run("eqObj(ipaddrval,ipaddr('192.168.1.13'))")->getBool());
    ASSERT_TRUE(conn.run("eqObj(int128val,int128('e1671797c52e15f763380b45e841ec32'))")->getBool());
    std::cout<<"--------------All cases passed----------------"<<std::endl;
}
#ifndef _WIN32
TEST_F(FunctionTest,test_FastVector_get){
    // todo: when will the variable `index` not IndexArray
    auto base = new dolphindb::SymbolBase(0);
    base->findAndInsert("test1");
    base->findAndInsert("test2");
    int *test1 = new int[2];
    test1[0] = 1;
    test1[1] = 2;
    double *test2 = new double[2];
    test2[0] = 1.1;
    test2[1] = 1.2;
    auto fastSymbolVec = new dolphindb::FastSymbolVector(base, 2, 6, test1, true);
    auto indexVec1 = dolphindb::Util::createIndexVector(0, 2);
    auto indexVec2 = new dolphindb::FastDoubleVector(2, 2, test2, true);
    ASSERT_EQ(indexVec1->isIndexArray(), true);
    ASSERT_EQ(indexVec2->isIndexArray(), false);
    ASSERT_EQ(indexVec1->size(), 2);
    ASSERT_EQ(indexVec2->size(), 2);

    std::cout << fastSymbolVec->get(indexVec1)->getString() << std::endl;
    std::cout << fastSymbolVec->get(indexVec2)->getString() << std::endl;
}

TEST_F(FunctionTest,test_FastVector_fill){
    auto base = new dolphindb::SymbolBase(0);
    int *test1 = new int[2];
    test1[0] = 1;
    test1[1] = 2;
    auto fastSymbolVec = new dolphindb::FastSymbolVector(base, 2, 6, test1, true);
    auto val1 = dolphindb::ConstantSP(new dolphindb::String("test"));
    fastSymbolVec->fill(0, 1, val1);

    std::vector<std::string> test2 = {"test1", "test2"};
    auto val2 = dolphindb::ConstantSP(new dolphindb::StringVector(test2, test2.size(), true));
    fastSymbolVec->fill(0, 3, val2);
    fastSymbolVec->fill(0, 2, val2);

    // todo: getCategory() != LITERAL
    double *test3 = new double[2];
    test3[0] = 1.1;
    test3[1] = 1.2;
    auto val3 = dolphindb::ConstantSP(new dolphindb::FastDoubleVector(2, 2, test3, true));
    ASSERT_ANY_THROW(fastSymbolVec->fill(0, 2, val3));
    // ASSERT_NE(val3->getCategory(), LITERAL);
    // ASSERT_EQ(val3->getCategory(), dolphindb::DT_DOUBLE);

}


TEST_F(FunctionTest,Guid){
    dolphindb::Guid uuid(1, 0);
    ASSERT_FALSE(uuid.isZero());
    dolphindb::GuidHash hash;
    std::cout << hash(uuid) << std::endl;
    dolphindb::VectorSP con = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 1);
    int num = 0, partial = 0;
    ASSERT_ANY_THROW(con->Constant::serialize(nullptr, 0, 0, 0, num, partial));
    ASSERT_ANY_THROW(con->Constant::serialize(nullptr, 0, 0, 0, 0, num, partial));
    ASSERT_ANY_THROW(con->Constant::deserialize(nullptr, 0, 0, num));
    ASSERT_EQ("", con->Constant::getRowLabel()->getString());
    ASSERT_EQ("", con->Constant::getColumnLabel()->getString());
    ASSERT_EQ("", con->Vector::getColumnLabel()->getString());

    dolphindb::VectorSP matrix = dolphindb::Util::createMatrix(dolphindb::DT_INT, 3, 3, 9);
    matrix->setName("123");
    ASSERT_EQ("123", matrix->getScript());
    int* data = new int[3]{1, 2, 3};
    dolphindb::VectorSP vec = dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3, true, 0, data);
    ASSERT_EQ("[1,2,3]", vec->getScript());
    dolphindb::VectorSP vec2 = dolphindb::Util::createIndexVector(0,200);
    ASSERT_EQ("array()", vec2->getScript());
    vec2->setName("234");
    ASSERT_EQ("234", vec2->getScript());
    dolphindb::VectorSP vec3 = dolphindb::Util::createVector(dolphindb::DT_INT, 0, 3);
    ASSERT_EQ("[]", vec3->getScript());
}

TEST_F(FunctionTest,Matrix_reshape){
    int* data = new int[16]{1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
    dolphindb::VectorSP matrix = dolphindb::Util::createMatrix(dolphindb::DT_INT, 4, 4, 16, 0, data);
    std::vector<std::string> rowLabels{"row1", "row2", "row3", "row4"};
    std::vector<std::string> columnLabels{"col1", "col2", "col3", "col4"};
    dolphindb::VectorSP rowVec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 4);
    rowVec->appendString(rowLabels.data(), rowLabels.size());
    rowVec->setTemporary(false);
    dolphindb::VectorSP colVec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 4);
    colVec->appendString(columnLabels.data(), columnLabels.size());
    colVec->setTemporary(false);
    matrix->setRowLabel(rowVec);
    matrix->setColumnLabel(colVec);
    matrix->getString();
    ASSERT_TRUE(matrix->reshape(4, 4));
    ASSERT_TRUE(matrix->reshape(3, 4));
    ASSERT_TRUE(matrix->reshape(2, 6));
    ASSERT_FALSE(matrix->reshape(2, 5));
}

TEST_F(FunctionTest,Matrix_getString){
    int* data = new int[48]{1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
    dolphindb::VectorSP matrix = dolphindb::Util::createMatrix(dolphindb::DT_INT, 1, 48, 48, 0, data);
    matrix->getString();
    matrix->getString(0);
    ASSERT_TRUE(matrix->reshape(48, 1));
    matrix->getString();
}


TEST_F(FunctionTest,Matrix_get){
    int* data = new int[16]{1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
    dolphindb::VectorSP matrix = dolphindb::Util::createMatrix(dolphindb::DT_INT, 4, 4, 16, 0, data);
    dolphindb::ConstantSP index = dolphindb::Util::createInt(1);
    ASSERT_EQ("[5,6,7,8]", dynamic_cast<dolphindb::FastIntMatrix*>(matrix.get())->get(index)->getString());
    dolphindb::ConstantSP index1 = dolphindb::Util::createInt(-1);
    dolphindb::ConstantSP index2 = dolphindb::Util::createInt(10);
    ASSERT_ANY_THROW(dynamic_cast<dolphindb::FastIntMatrix*>(matrix.get())->get(index1));
    ASSERT_ANY_THROW(dynamic_cast<dolphindb::FastIntMatrix*>(matrix.get())->get(index2));

    dolphindb::ConstantSP index3 = dolphindb::Util::createPair(dolphindb::DT_INT);
    index3->setInt(0, 0);
    index3->setInt(1, 1);
    dynamic_cast<dolphindb::FastIntMatrix*>(matrix.get())->get(index3)->getString();
    index3->setInt(0, INT_MIN);
    index3->setInt(1, INT_MIN);
    dynamic_cast<dolphindb::FastIntMatrix*>(matrix.get())->get(index3)->getString();
    index3->setInt(0, 1);
    index3->setInt(1, 0);
    dynamic_cast<dolphindb::FastIntMatrix*>(matrix.get())->get(index3)->getString();

    dolphindb::ConstantSP index4 = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2);
    index4->setInt(0, -1);
    index4->setInt(1, 0);
    ASSERT_ANY_THROW(dynamic_cast<dolphindb::FastIntMatrix*>(matrix.get())->get(index4));
    index4->setInt(0, 6);
    index4->setInt(1, 0);
    ASSERT_ANY_THROW(dynamic_cast<dolphindb::FastIntMatrix*>(matrix.get())->get(index4));
}


TEST_F(FunctionTest,Matrix_set){
    int* data = new int[16]{1, 2, 3, 4, 5, 6, 7, 8, 1, 2, 3, 4, 5, 6, 7, 8};
    dolphindb::VectorSP matrix = dolphindb::Util::createMatrix(dolphindb::DT_INT, 4, 4, 16, 0, data);
    
    dolphindb::ConstantSP index = dolphindb::Util::createIndexVector(0, 3);
    dolphindb::ConstantSP value = dolphindb::Util::createInt(9);
    ASSERT_TRUE(matrix->set(index, value));
    dolphindb::ConstantSP index1 = dolphindb::Util::createIndexVector(0, 6);
    ASSERT_ANY_THROW(matrix->set(index1, value));
    dolphindb::ConstantSP index2 = dolphindb::Util::createIndexVector(-1, 6);
    ASSERT_ANY_THROW(matrix->set(index2, value));
}

TEST_F(FunctionTest, DFSChunkMeta_Constructor){
    dolphindb::IO_ERR ret;
    short flag = 0, size = 0;
    std::vector<std::string> sites1 = {"192.168.0.16:9002:datanode1", "192.168.0.16:9003:datanode2", "192.168.0.16:9004:datanode3", "192.168.0.16:9005:datanode4"};
    std::vector<std::string> sites2;
    dolphindb::Guid id("314");
    dolphindb::DFSChunkMetaSP chunk = new dolphindb::DFSChunkMeta("/home/appadmin/data", id, 3, 1, dolphindb::SPLIT_TABLET_CHUNK, sites2, 315);
    chunk->getString();
    chunk = new dolphindb::DFSChunkMeta("/home/appadmin/data", id, 3, 1, dolphindb::TABLET_CHUNK, sites1, 315);
    chunk->getString();
    std::cout << chunk->getAllocatedMemory() << std::endl;
    chunk = new dolphindb::DFSChunkMeta("/home/appadmin/data", id, 3, 1, dolphindb::FILE_CHUNK, sites1.data(), 0, 315);

    dolphindb::DataOutputStreamSP outStream1 = new dolphindb::DataOutputStream(1024);
    dolphindb::ConstantMarshallSP marshall1 = dolphindb::ConstantMarshallFactory::getInstance(dolphindb::DF_CHUNK, outStream1);
    ASSERT_TRUE(marshall1->start(chunk, false, false, ret));
    dolphindb::DataInputStreamSP inStream1 = new dolphindb::DataInputStream(outStream1->getBuffer(), outStream1->size());
    inStream1->readShort(flag);
    inStream1->readShort(size);
    dolphindb::DFSChunkMetaSP chunk1 = new dolphindb::DFSChunkMeta(inStream1);
    ASSERT_ANY_THROW(new dolphindb::DFSChunkMeta(inStream1));

    chunk = new dolphindb::DFSChunkMeta("/home/appadmin/data", id, 3, 1, dolphindb::FILE_CHUNK, sites1.data(), 4, 315);
    dolphindb::DataOutputStreamSP outStream2 = new dolphindb::DataOutputStream(1024);
    dolphindb::ConstantMarshallSP marshall2 = dolphindb::ConstantMarshallFactory::getInstance(dolphindb::DF_CHUNK, outStream2);
    ASSERT_TRUE(marshall2->start(chunk, false, false, ret));
    dolphindb::DataInputStreamSP inStream2 = new dolphindb::DataInputStream(outStream2->getBuffer(), outStream2->size());
    inStream2->readShort(flag);
    inStream2->readShort(size);
    dolphindb::DFSChunkMetaSP chunk2 = new dolphindb::DFSChunkMeta(inStream2);
}

TEST_F(FunctionTest, DFSChunkMeta_getMember){
    std::vector<std::string> sites1 = {"192.168.0.16:9002:datanode1", "192.168.0.16:9003:datanode2", "192.168.0.16:9004:datanode3", "192.168.0.16:9005:datanode4"};
    dolphindb::Guid id("314");
    dolphindb::DFSChunkMetaSP chunk = new dolphindb::DFSChunkMeta("/home/appadmin/data", id, 3, 1, dolphindb::FILE_CHUNK, sites1, 315);
    
    dolphindb::ConstantSP key1 = dolphindb::Util::createInt(1);
    ASSERT_ANY_THROW(chunk->getMember(key1));
    dolphindb::ConstantSP key2 = dolphindb::Util::createString("path");
    ASSERT_EQ("/home/appadmin/data", chunk->getMember(key2)->getString());
    dolphindb::ConstantSP key3 = dolphindb::Util::createPair(dolphindb::DT_STRING);
    ASSERT_ANY_THROW(chunk->getMember(key3));
    dolphindb::VectorSP key4 = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 8);
    std::vector<std::string> vec{"id", "cid", "version", "sites", "size", "isTablet", "splittable", "none"};
    key4->appendString(vec.data(), vec.size());
    chunk->getMember(key4);
    chunk->keys();
    chunk->values();
}

TEST_F(FunctionTest, DBConnection_upload){
    dolphindb::ConstantSP value = dolphindb::Util::createInt(1);
    ASSERT_ANY_THROW(conn.upload("0123", value));
    std::vector<dolphindb::ConstantSP> obj{value};
    std::vector<std::string> names{"123"};
    ASSERT_ANY_THROW(conn.upload(names, obj));
    names.push_back("234");
    ASSERT_ANY_THROW(conn.upload(names, obj));
    obj.clear();
    names.clear();
    conn.upload(names, obj);
    ASSERT_ANY_THROW(conn.run("1+1", 4, 2, 100));
    dolphindb::DBConnection dc;
    dolphindb::DBConnection dd;
    ASSERT_ANY_THROW(dc.run("1+1"));
    dc = std::move(dc);
    dd = std::move(dc);
}


dolphindb::BasicTableSP createTable(){
    int* data1 = new int[2]{1, 2};
    int* data2 = new int[2]{3, 4};
    std::vector<dolphindb::ConstantSP> cols ={dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2, true, 0, data1), dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2, true, 0, data2)};
    std::vector<std::string> colNames = { "col1", "col2"};
    return new dolphindb::BasicTable(cols, colNames);
}

TEST_F(FunctionTest, Table_Constuctor){
    dolphindb::BasicTableSP t = createTable();
    dolphindb::ConstantSP value = new dolphindb::Int(1);
    ASSERT_ANY_THROW(t->AbstractTable::set(1, value));
    ASSERT_ANY_THROW(t->AbstractTable::setColumnName(1, "123"));
    dolphindb::ConstantSP emptyFilter;
    ASSERT_EQ("[3,4]", t->AbstractTable::getColumn(std::string("col2"), emptyFilter)->getString());
    ASSERT_EQ("4", t->AbstractTable::get(1, 1)->getString());
    t->setName("Harry");
    ASSERT_FALSE(t->contain("Tom", "col1"));
    ASSERT_FALSE(t->contain("Tom", "col3"));

    std::vector<std::string> colNames2 = { "col1", "col2", "col3", "col4", "col5", "col6", "col7", "col8", "col9", "col0", "col1", "col2", "col3"};
    std::vector<dolphindb::ConstantSP> cols2;
    for(int i = 0; i < 13; ++i){
        dolphindb::VectorSP v = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 1);
        std::string content = "12345678901234567890";
        v->appendString(&content, 1);
        cols2.push_back(v);
    }
    dolphindb::BasicTableSP t1 = new dolphindb::BasicTable(cols2, colNames2);
    t1->getString(0);
    t1->getString();
    ASSERT_EQ(dolphindb::COMPRESS_NONE, t1->getColumnCompressMethod(2));
}

TEST_F(FunctionTest, Table_setColumnCompressMethods){
    dolphindb::BasicTableSP t = createTable();
    std::vector<dolphindb::COMPRESS_METHOD> methods{dolphindb::COMPRESS_DELTA, dolphindb::COMPRESS_NONE};
    ASSERT_ANY_THROW(t->setColumnCompressMethods(methods));
}

TEST_F(FunctionTest, Table_getInternal){
    int* data1 = new int[2]{1, 2};
    int* data2 = new int[2]{3, 4};
    std::vector<dolphindb::ConstantSP> cols ={dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2, true, 0, data1), dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2, true, 0, data2)};
    std::vector<std::string> colNames = { "col1", "col2"};
    dolphindb::BasicTableSP t = new dolphindb::BasicTable(cols, colNames);
    dolphindb::ConstantSP index = dolphindb::Util::createPair(dolphindb::DT_INT);
    index->setInt(0, INT_MIN);
    index->setInt(1, INT_MIN);
    t->get(index);
    index->setInt(0, 0);
    index->setInt(1, 1);
    t->get(index);
    t->getWindow(1, 1, 1, 1);
    ASSERT_ANY_THROW(t->AbstractTable::getInstance(1));
    ASSERT_ANY_THROW(t->AbstractTable::getValue());
    ASSERT_ANY_THROW(t->AbstractTable::getValue(1));
    std::string errMsg;
    dolphindb::INDEX rows = 0;
    std::vector<dolphindb::ConstantSP> values;
    ASSERT_FALSE(t->AbstractTable::append(values, rows, errMsg));
    ASSERT_FALSE(t->AbstractTable::update(values, index, colNames, errMsg));
    ASSERT_FALSE(t->AbstractTable::remove(index, errMsg));
}

TEST_F(FunctionTest, BasicTable_init){
    int* data1 = new int[2]{1, 2};
    int* data2 = new int[2]{3, 4};
    std::vector<dolphindb::ConstantSP> cols(2);
    std::vector<std::string> colNames = { "col1", "col2"};
    ASSERT_ANY_THROW(new dolphindb::BasicTable(cols, colNames));
    cols[0] = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2, true, 0, data1);
    cols[1] = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2, true, 0, data2);
    dolphindb::BasicTableSP t = new dolphindb::BasicTable(cols, colNames);
    dolphindb::ConstantSP value = dolphindb::Util::createDictionary(dolphindb::DT_INT, dolphindb::DT_INT);
    ASSERT_FALSE(t->set(0, value));
}

TEST_F(FunctionTest, BasicTable_appendTable){
    int* data1 = new int[2]{1, 2};
    int* data2 = new int[2]{3, 4};
    std::vector<dolphindb::ConstantSP> cols ={dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2, true, 0, data1), dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2, true, 0, data2)};
    std::vector<std::string> colNames = { "col1", "col2"};
    dolphindb::BasicTableSP t = new dolphindb::BasicTable(cols, colNames);

    std::vector<std::string> data3{"123", "456"};
    int* data4 = new int[2]{3, 4};
    std::vector<dolphindb::ConstantSP> cols2(2);
    dolphindb::VectorSP vec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    vec->appendString(data3.data(), data3.size());
    cols2[1] = vec;
    cols2[0] = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2, true, 0, data4);
    std::vector<std::string> colNames2 = { "col1", "col2"};
    dolphindb::BasicTableSP t2 = new dolphindb::BasicTable(cols2, colNames2);

    std::vector<dolphindb::ConstantSP> values{t2};
    std::string errMsg;
    dolphindb::INDEX rows = 0;
    ASSERT_FALSE(t->append(values, rows, errMsg));
    
    t2 = t->getValue();
    values[0] = t2;
    ASSERT_TRUE(t->append(values, rows, errMsg));
}

TEST_F(FunctionTest, BasicTable_appendTuple){
    std::string errMsg;
    dolphindb::INDEX rows = 0;
    dolphindb::BasicTableSP t = createTable();
    
    dolphindb::VectorSP vec = dolphindb::Util::createVector(dolphindb::DT_ANY, 0, 2);
    vec->append(dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2));
    vec->append(dolphindb::Util::createString("123"));
    std::vector<dolphindb::ConstantSP> values{vec};
    ASSERT_FALSE(t->append(values, rows, errMsg));
    
    dolphindb::VectorSP vec1 = dolphindb::Util::createVector(dolphindb::DT_ANY, 0, 2);
    vec1->append(dolphindb::Util::createInt(2));
    vec1->append(dolphindb::Util::createString("123"));
    values[0] = vec1;
    ASSERT_FALSE(t->append(values, rows, errMsg));

    dolphindb::VectorSP vec2 = dolphindb::Util::createVector(dolphindb::DT_ANY, 0, 2);
    vec2->append(dolphindb::Util::createInt(2));
    vec2->append(dolphindb::Util::createInt(2));
    values[0] = vec2;
    ASSERT_TRUE(t->append(values, rows, errMsg));

    dolphindb::VectorSP vec3 = dolphindb::Util::createVector(dolphindb::DT_ANY, 0, 2);
    vec3->append(dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2));
    vec3->append(dolphindb::Util::createVector(dolphindb::DT_STRING, 2, 2));
    values[0] = vec3;
    ASSERT_FALSE(t->append(values, rows, errMsg));
}

TEST_F(FunctionTest, BasicTable_appendNormal){
    std::string errMsg;
    dolphindb::INDEX rows = 0;
    dolphindb::BasicTableSP t = createTable();
    
    std::vector<dolphindb::ConstantSP> values;
    values.push_back(dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2));
    values.push_back(dolphindb::Util::createString("123"));
    ASSERT_FALSE(t->append(values, rows, errMsg));
    
    values[0] = dolphindb::Util::createInt(2);
    values[1] = dolphindb::Util::createString("123");
    ASSERT_FALSE(t->append(values, rows, errMsg));

    values[0] = dolphindb::Util::createInt(2);
    values[1] = dolphindb::Util::createInt(2);
    ASSERT_TRUE(t->append(values, rows, errMsg));

    values[0] = dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2);
    std::vector<std::string> columnLabels{"col1", "col2"};
    dolphindb::VectorSP rowVec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 4);
    rowVec->appendString(columnLabels.data(), columnLabels.size());
    values[1] = rowVec;
    ASSERT_FALSE(t->append(values, rows, errMsg));
}

TEST_F(FunctionTest, BasicTable_update){
    dolphindb::BasicTableSP t = createTable();

    dolphindb::ConstantSP index = dolphindb::Util::createIndexVector(0, 1);
    index->setNothing(true);
    std::vector<dolphindb::ConstantSP> values{dolphindb::Util::createVector(dolphindb::DT_INT, 3, 3)};
    std::vector<std::string> colName{"col3"};
    std::string errMsg;
    ASSERT_FALSE(t->update(values, index, colName, errMsg));
    values[0] = dolphindb::Util::createInt(1);
    ASSERT_TRUE(t->update(values, index, colName, errMsg));
    colName[0] = "col4";
    values[0] = dolphindb::Util::createIndexVector(0, 2);
    ASSERT_TRUE(t->update(values, index, colName, errMsg));
    colName[0] = "col5";
    values[0] = dolphindb::Util::createIndexVector(0, 2);
    values[0]->setTemporary(false);
    ASSERT_TRUE(t->update(values, index, colName, errMsg));

    colName[0] = "col1";
    values[0] = dolphindb::Util::createIndexVector(0, 2);
    ASSERT_TRUE(t->update(values, index, colName, errMsg));

    values[0] = dolphindb::Util::createPair(dolphindb::DT_STRING);
    ASSERT_FALSE(t->update(values, index, colName, errMsg));
}

TEST_F(FunctionTest, BasicTable_updateVector){
    int* data1 = new int[2]{1, 2};
    std::vector<std::string> columnLabels{"col1", "col2"};
    dolphindb::VectorSP rowVec = dolphindb::Util::createVector(dolphindb::DT_STRING, 0, 2);
    rowVec->appendString(columnLabels.data(), columnLabels.size());
    std::vector<dolphindb::ConstantSP> cols ={dolphindb::Util::createVector(dolphindb::DT_INT, 2, 2, true, 0, data1), rowVec};
    std::vector<std::string> colNames = { "col1", "col2"};
    dolphindb::BasicTableSP t = new dolphindb::BasicTable(cols, colNames);
    
    std::vector<dolphindb::ConstantSP> values;
    values.push_back(dolphindb::Util::createIndexVector(0, 2));
    dolphindb::ConstantSP index = dolphindb::Util::createIndexVector(0, 1);
    std::string errMsg;
    std::vector<std::string> colName{"col2"};
    ASSERT_FALSE(t->update(values, index, colName, errMsg));

    colName[0] = "col1";
    values[0] = new dolphindb::Void();
    ASSERT_TRUE(t->update(values, index, colName, errMsg));
 
    values[0] = dolphindb::Util::createVector(dolphindb::DT_FLOAT, 2, 2);
    ASSERT_FALSE(t->update(values, index, colName, errMsg));
    values[0] = dolphindb::Util::createInt(1);
    index->setNothing(true);
    ASSERT_TRUE(t->update(values, index, colName, errMsg));
}

TEST_F(FunctionTest, BasicTable_remove){
    dolphindb::BasicTableSP t = createTable();

    std::string errMsg;
    dolphindb::ConstantSP index = dolphindb::Util::createIndexVector(0, 1);
    index->setNothing(true);
    ASSERT_TRUE(t->remove(index, errMsg));
}

TEST_F(FunctionTest, BasicTable_join){
    dolphindb::BasicTableSP t = createTable();
    t->setReadOnly(true);

    std::vector<dolphindb::ConstantSP> columns;
    ASSERT_FALSE(t->join(columns));
    t->updateSize();
    ASSERT_FALSE(t->clear());
    columns.push_back(dolphindb::Util::createPair(dolphindb::DT_INT));
    t->setReadOnly(false);
    ASSERT_FALSE(t->join(columns));
    columns[0] = dolphindb::Util::createIndexVector(0, 3);
    ASSERT_FALSE(t->join(columns));
    columns[0] = dolphindb::Util::createIndexVector(0, 2);
    ASSERT_FALSE(t->join(columns));
    dynamic_cast<dolphindb::Vector*>(columns[0].get())->setName("col1");
    ASSERT_FALSE(t->join(columns));
    dynamic_cast<dolphindb::Vector*>(columns[0].get())->setName("col3");
    ASSERT_TRUE(t->join(columns));
    ((dolphindb::Vector*)t->getColumn(0).get())->append(dolphindb::Util::createIndexVector(0, 2));
    ASSERT_ANY_THROW(t->updateSize());
}

TEST_F(FunctionTest,test_FastFixedLengthVector_getDataArray){
    unsigned char data[5] = "abcd";
    auto fastIntVec = new dolphindb::FastInt128Vector(dolphindb::DT_INT, 2, 4, data, false);
    auto fastIntVec1 = new dolphindb::FastInt128Vector(dolphindb::DT_INT, 2, 4, data, true);
    bool hasNull = false;
    auto idx1 = dolphindb::Util::createIndexVector(1, 2);
    fastIntVec->get(idx1);
    double *vec1 = new double[2];
    vec1[0] = 0.1;
    vec1[1] = 1.2;
    double *vec2 = new double[2];
    vec2[0] = 3.1;
    vec2[1] = 3.2;
    auto idx2 = new dolphindb::FastDoubleVector(2, 2, vec1, true);
    auto idx3 = new dolphindb::FastDoubleVector(2, 2, vec2, true);
    fastIntVec1->get(idx2);
    fastIntVec1->get(idx3);
}

#endif


TEST_F(FunctionTest,test_upload_not_initialized_constant){
    dolphindb::ConstantSP data;
    ASSERT_ANY_THROW(conn.upload({"data"}, {data}));
}


TEST_F(FunctionTest, test_createDate){
    auto date_min = dolphindb::Util::createDate(INT_MIN);
    auto date_max = dolphindb::Util::createDate(INT_MAX);
    auto date0 = dolphindb::Util::createDate(0);
    auto date1 = dolphindb::Util::createDate(9999, 12, 31);
    ASSERT_EQ(date_min->getString(), "");
    ASSERT_EQ(date_max->getString(), "5881580.07.11");
    ASSERT_EQ(date0->getString(), "1970.01.01");
    ASSERT_EQ(date1->getString(), "9999.12.31");
}
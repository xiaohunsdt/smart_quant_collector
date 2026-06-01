#include <gtest/gtest.h>
#include <thread>
#include "config.h"
#include "BatchTableWriter.h"

class BatchTableWriterTest:public testing::Test
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

dolphindb::DBConnection BatchTableWriterTest::conn(false, false);

static std::vector<std::string> type = { "BOOL", "CHAR","SHORT", "STRING", "LONG", "NANOTIME","NANOTIMESTAMP", "TIMESTAMP","FLOAT", "DOUBLE","INT", "DATE","MONTH", "TIME","SECOND", "MINUTE","DATETIME", "DATEHOUR", "IPADDR", "INT128", "SYMBOL" };

TEST_F(BatchTableWriterTest,test_batchTableWriter_insert){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script;
    script += "dbPath = '"+dbName+"';";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db=database(dbPath,VALUE,2010.01.01..2010.01.10);";
    script += "t=table(100:0,`dbbool `dbchar `dbshort `dbstring_char `dbstring `dblong `dbnanotime `dbnanotimestamp `dbtimestamp `dbfloat `dbdouble `dbint `dbdate `dbmonth `dbtime `dbsecond `dbminute `dbdatetime `dbdatehour , [BOOL, CHAR, SHORT, STRING, STRING, LONG, NANOTIME, NANOTIMESTAMP, TIMESTAMP, FLOAT, DOUBLE, INT, DATE, MONTH, TIME, SECOND, MINUTE, DATETIME, DATEHOUR]);";
    script += "share table(100:0,`dbbool `dbchar `dbshort `dbstring_char `dbstring `dblong `dbnanotime `dbnanotimestamp `dbtimestamp `dbfloat `dbdouble `dbint `dbdate `dbmonth `dbtime `dbsecond `dbminute `dbdatetime `dbdatehour , [BOOL, CHAR, SHORT, STRING, STRING, LONG, NANOTIME, NANOTIMESTAMP, TIMESTAMP, FLOAT, DOUBLE, INT, DATE, MONTH, TIME, SECOND, MINUTE, DATETIME, DATEHOUR]) as "+case_+";";
    script += "db.createTable(t,`dtable).append!(t);";
    script += "db.createPartitionedTable(t,`ptable,`dbdate).append!(t);";
    conn.run(script);
    dolphindb::BatchTableWriter btw(HOST, PORT, USER, PASSWD, true);
    btw.addTable(case_, "");
    const char* strtest = "test";
    long long testlong = 100;
    std::string expected = "expected = table( bool(1) as dbbool, char('t') as dbchar, short(100) as dbshort, string('test') as dbstring_char, string('test') as dbstring, long(100) as dblong, nanotime(100) as dbnanotime, nanotimestamp(100) as dbnanotimestamp, timestamp(100) as dbtimestamp,float(100) as dbfloat, double(100) as dbdouble, int(100) as dbint, date(100) as dbdate, month(100) as dbmonth, time(100) as dbtime, second(100) as dbsecond, minute(100) as dbminute, datetime(100) as dbdatetime, datehour(100) as dbdatehour);";
    conn.run(expected);
    std::string expected1 = "expected1 = table( take(bool(1),3003000) as dbbool, take(char('t'),3003000) as dbchar, take(short(100),3003000) as dbshort, take(string('test'),3003000) as dbstring_char, take(string('test'),3003000) as dbstring, take(long(100),3003000) as dblong, take(nanotime(100),3003000) as dbnanotime, take(nanotimestamp(100),3003000) as dbnanotimestamp, take(timestamp(100),3003000) as dbtimestamp, take(float(100),3003000) as dbfloat, take(double(100),3003000) as dbdouble, take(int(100),3003000) as dbint, take(date(100),3003000) as dbdate, take(month(100),3003000) as dbmonth, take(time(100),3003000) as dbtime, take(second(100),3003000) as dbsecond, take(minute(100),3003000) as dbminute, take(datetime(100),3003000) as dbdatetime, take(datehour(100),3003000) as dbdatehour);";
    conn.run(expected1);

    ASSERT_ANY_THROW(btw.insert(case_, "", int(0), int(0), short(100), strtest, std::string("test"), testlong, testlong, testlong, testlong, float(100), double(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100)));
    ASSERT_ANY_THROW(btw.insert(case_, "", char('0'), char('t'), short(100), strtest, std::string("test"), testlong, testlong, testlong, testlong, float(100), double(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100)));

    //size1000
    for (int i = 0;i<1000;i++) {
        btw.insert(case_, "", char('0'), char('t'), short(100), strtest, std::string("test"), testlong, testlong, testlong, testlong, float(100), double(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100));
    }
    {
        while(btw.getAllStatus()->getColumn(3)->getString(0).c_str() != std::to_string(1000)) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout<<"now rows:"<<btw.getAllStatus()->getColumn(3)->getString(0).c_str()<<std::endl;
        }
        ASSERT_EQ(btw.getAllStatus()->getColumn(3)->getInt(0),1000);
    }
    int result1 = 0;
    for (int i = 0;i<1000;i++) {
        std::string script = "eqObj("+case_+"[" + std::to_string(i) + "].values(),expected[0].values());";
        result1 += conn.run(script)->getInt();
    }
    ASSERT_EQ(result1, 1000);
    //size2000
    for (int i = 0;i<2000;i++) {
        btw.insert(case_, "", char('0'), char('t'), short(100), strtest, std::string("test"), testlong, testlong, testlong, testlong, float(100), double(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100));
    }
    {
            while(btw.getAllStatus()->getColumn(3)->getString(0).c_str() != std::to_string(3000)) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout<<"now rows:"<<btw.getAllStatus()->getColumn(3)->getString(0).c_str()<<std::endl;
        }
        ASSERT_EQ(btw.getAllStatus()->getColumn(3)->getInt(0),3000);
    }
    result1 = 0;
    for (int i = 0;i<3000;i++) {
        std::string script = "eqObj("+case_+"[" + std::to_string(i) + "].values(),expected[0].values());";
        result1 += conn.run(script)->getInt();
    }
    ASSERT_EQ(result1, 3000);
    btw.removeTable(case_, "");
    //size1000
    btw.addTable(dbName, "dtable");
    for (int i = 0;i<1000;i++) {
        btw.insert(dbName, "dtable", char('0'), char('t'), short(100), strtest, std::string("test"), testlong, testlong, testlong, testlong, float(100), double(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100));
    }
    {
        
        while(btw.getAllStatus()->getColumn(3)->getString(0).c_str() != std::to_string(1000)) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout<<"now rows:"<<btw.getAllStatus()->getColumn(3)->getString(0).c_str()<<std::endl;
        }
        ASSERT_EQ(btw.getAllStatus()->getColumn(3)->getInt(0),1000);
    }
    int result2 = 0;
    for (int i = 0;i<1000;i++) {
        std::string script = "dt=loadTable(dbPath, 'dtable'); eqObj(( select * from dt)[" + std::to_string(i) + "].values(),expected[0].values());";
        result2 += conn.run(script)->getInt();
    }
    ASSERT_EQ(result2, 1000);
    //size2000
    for (int i = 0;i<2000;i++) {
        btw.insert(dbName, "dtable", char('0'), char('t'), short(100), strtest, std::string("test"), testlong, testlong, testlong, testlong, float(100), double(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100));
    }
    {
        
        while(btw.getAllStatus()->getColumn(3)->getString(0).c_str() != std::to_string(3000)) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout<<"now rows:"<<btw.getAllStatus()->getColumn(3)->getString(0).c_str()<<std::endl;
        }
        ASSERT_EQ(btw.getAllStatus()->getColumn(3)->getInt(0),3000);
    }
    result2 = 0;
    for (int i = 0;i<3000;i++) {
        std::string script = "dt=loadTable(dbPath, 'dtable'); eqObj(( select * from dt)[" + std::to_string(i) + "].values(),expected[0].values());";
        result2 += conn.run(script)->getInt();
    }
    ASSERT_EQ(result2, 3000);
    btw.removeTable(dbName, "dtable");
    //size1000
    btw.addTable(dbName, "ptable");
    for (int i = 0;i<1000;i++) {
        btw.insert(dbName, "ptable", char('0'), char('t'), short(100), strtest, std::string("test"), testlong, testlong, testlong, testlong, float(100), double(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100));
    }
    {
        while(btw.getAllStatus()->getColumn(3)->getString(0).c_str() != std::to_string(1000)) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout<<"now rows:"<<btw.getAllStatus()->getColumn(3)->getString(0).c_str()<<std::endl;
        }
        ASSERT_EQ(btw.getAllStatus()->getColumn(3)->getInt(0),1000);
    }
    int result3 = 0;
    for (int i = 0;i<1000;i++) {
        std::string script = "pt=loadTable(dbPath, 'ptable'); eqObj(( select * from pt)[" + std::to_string(i) + "].values(),expected[0].values());";
        result3 += conn.run(script)->getInt();
    }
    ASSERT_EQ(result3, 1000);
    //size2000
    for (int i = 0;i<2000;i++) {
        btw.insert(dbName, "ptable", char('0'), char('t'), short(100), strtest, std::string("test"), testlong, testlong, testlong, testlong, float(100), double(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100), int(100));
    }
    {
            while(btw.getAllStatus()->getColumn(3)->getString(0).c_str() != std::to_string(3000)) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::cout<<"now rows:"<<btw.getAllStatus()->getColumn(3)->getString(0).c_str()<<std::endl;
        }
        ASSERT_EQ(btw.getAllStatus()->getColumn(3)->getInt(0),3000);
    }
    result3 = 0;
    for (int i = 0;i<3000;i++) {
        std::string script = "pt=loadTable(dbPath, 'ptable'); eqObj(( select * from pt)[" + std::to_string(i) + "].values(),expected[0].values());";
        result3 += conn.run(script)->getInt();
    }
    ASSERT_EQ(result3, 3000);
    btw.removeTable(dbName, "ptable");
}

TEST_F(BatchTableWriterTest,test_batchTableWriter_insert_symbol_in_memory){
    std::string case_=getCaseName();
    std::string script;
    script += "share table(3000000:0, `c1`c2`c3, [SYMBOL, SYMBOL, SYMBOL]) as "+case_;
    conn.run(script);
    dolphindb::BatchTableWriter btw(HOST, PORT, USER, PASSWD, true);
    btw.addTable(case_);
    for (int i = 0; i < 3000000; i++) {
        btw.insert(case_, "", "A" + std::to_string(i%10), "B" + std::to_string(i%10), "C" + std::to_string(i%10));
    }
    
    while(btw.getAllStatus()->getColumn(3)->getString(0).c_str() != std::to_string(3000000)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<"now rows:"<<btw.getAllStatus()->getColumn(3)->getString(0).c_str()<<std::endl;
    }
    ASSERT_EQ(btw.getAllStatus()->getColumn(3)->getInt(0),3000000);
    std::string script2;
    script2 += "c1v=`A + string(0..9);";
    script2 += "c2v=`B + string(0..9);";
    script2 += "c3v=`C + string(0..9);";
    script2 += "expected=table(symbol(loop(take{, 300000}, c1v).flatten()) as c1, symbol(loop(take{, 300000}, c2v).flatten()) as c2, symbol(loop(take{, 300000}, c3v).flatten()) as c3);";
    script2 += "each(eqObj, (select * from expected order by c1, c2, c3).values(), (select * from "+case_+" order by c1, c2, c3).values()).all()";
    int result1;
    result1 = conn.run(script2)->getInt();
    ASSERT_EQ(result1, 1);
    btw.removeTable(case_);
}

TEST_F(BatchTableWriterTest,test_batchTableWriter_insert_symbol_dfs){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script;
    script += "dbName='"+dbName+"';";
    script += "if(existsDatabase(dbName)){dropDatabase(dbName)};";
    script += "db=database(dbName, HASH, [SYMBOL, 10]);";
    script += "t = table(1:0, `c1`c2`c3, [SYMBOL, SYMBOL, SYMBOL]);";
    script += "pt=db.createPartitionedTable(t, `pt, `c1);";
    conn.run(script);
    dolphindb::BatchTableWriter btw(HOST, PORT, USER, PASSWD, true);
    btw.addTable(dbName, "pt");
    for (int i = 0; i < 3000000; i++) {
        btw.insert(dbName, "pt", "A" + std::to_string(i % 10), "B" + std::to_string(i % 10), "C" + std::to_string(i % 10));
    }
    
    while(btw.getAllStatus()->getColumn(3)->getString(0).c_str() != std::to_string(3000000)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<"now rows:"<<btw.getAllStatus()->getColumn(3)->getString(0).c_str()<<std::endl;
    }
    ASSERT_EQ(btw.getAllStatus()->getColumn(3)->getInt(0),3000000);
    std::string script2;
    script2 += "c1v=`A + string(0..9);";
    script2 += "c2v=`B + string(0..9);";
    script2 += "c3v=`C + string(0..9);";
    script2 += "expected=table(symbol(loop(take{, 300000}, c1v).flatten()) as c1, symbol(loop(take{, 300000}, c2v).flatten()) as c2, symbol(loop(take{, 300000}, c3v).flatten()) as c3);";
    script2 += "each(eqObj, (select * from expected order by c1, c2, c3).values(), (select * from pt order by c1, c2, c3).values()).all()";
    int result1;
    result1 = conn.run(script2)->getInt();
    ASSERT_EQ(result1, 1);
    btw.removeTable(dbName, "pt");
}

TEST_F(BatchTableWriterTest,test_batchTableWriter_insert_16_bytes){
    std::string case_=getCaseName();
    std::string script;
    script += "share table(3000000:0, `c1`c2`c3, [UUID, INT128, IPADDR]) as "+ case_;
    conn.run(script);
    dolphindb::BatchTableWriter btw(HOST, PORT, USER, PASSWD, true);
    btw.addTable(case_);
    unsigned char data[16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    for (int i = 0; i < 10; i++) {
        btw.insert(case_, "", data, data, data);
    }
    
    while(btw.getAllStatus()->getColumn(3)->getString(0).c_str() != std::to_string(10)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<"now rows:"<<btw.getAllStatus()->getColumn(3)->getString(0).c_str()<<std::endl;
    }
    ASSERT_EQ(btw.getAllStatus()->getColumn(3)->getInt(0),10);
    btw.removeTable(case_);
}

// TEST_F(BatchTableWriterTest,test_batchTableWriter_insert_char_len_not_16){
//     std::string case_=getCaseName();
//     std::string script;
//     script += "share table(3000000:0, `c1`c2`c3, [UUID, INT128, IPADDR]) as "+ case_;
//     conn.run(script);
//     dolphindb::BatchTableWriter btw(HOST, PORT, USER, PASSWD, true);
//     btw.addTable(case_);
//     unsigned char data[15] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
//     for (int i = 0; i < 10; i++) {
//         btw.insert(case_, "", data, data, data);
//     }
//     while(btw.getAllStatus()->getColumn(3)->getString(0).c_str() != std::to_string(10)) {    
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//         std::cout<<"now rows:"<<btw.getAllStatus()->getColumn(3)->getString(0).c_str()<<std::endl;
//     }
//     ASSERT_EQ(btw.getAllStatus()->getColumn(3)->getInt(0),10);
//     dolphindb::TableSP result;
//     result = conn.run(case_);
// }

TEST_F(BatchTableWriterTest,test_batchTableWriter_getUnwrittenData){
    std::string case_=getCaseName();
    std::string script;
    script += "share table(3000000:0, `c1`c2`c3, [SYMBOL, SYMBOL, SYMBOL]) as "+ case_;
    conn.run(script);
    dolphindb::BatchTableWriter btw(HOST, PORT, USER, PASSWD, true);
    btw.addTable(case_);
    for (int i = 0; i < 65536; i++) {
        btw.insert(case_, "", "A" + std::to_string(i % 10), "B" + std::to_string(i % 10), "C" + std::to_string(i % 10));
    }
    std::string cur_rows = btw.getAllStatus()->getColumn(3)->getString(0).c_str();
    while(cur_rows != std::to_string(65536)) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout<<"now rows:"<<btw.getAllStatus()->getColumn(3)->getString(0).c_str()<<std::endl;
        cur_rows = btw.getAllStatus()->getColumn(3)->getString(0).c_str();
    }
    ASSERT_EQ(btw.getAllStatus()->getColumn(3)->getInt(0),65536);
    dolphindb::TableSP tableUnwritten2;
    tableUnwritten2 = btw.getUnwrittenData(case_);
    int rowNum2;
    rowNum2 = tableUnwritten2->getColumn(0)->size();
    ASSERT_EQ(rowNum2, 0);
    btw.removeTable(case_);
}

TEST_F(BatchTableWriterTest,test_batchTableWriter_addTable){
    std::string case_=getCaseName();
    std::string script;
    script += "share table(100:0,`test1 `test2, [INT,INT]) as "+case_;
    conn.run(script);
    dolphindb::BatchTableWriter btw(HOST, PORT, USER, PASSWD, true);
    btw.addTable(case_, "");
    ASSERT_ANY_THROW(btw.addTable(case_, ""));
    std::this_thread::sleep_for(std::chrono::seconds(1));
    btw.removeTable(case_, "");
}

TEST_F(BatchTableWriterTest,test_batchTableWriter_removeTable){
    std::string case_=getCaseName();
    std::string script;
    script += "share table(100:0,`test1 `test2, [INT,INT]) as "+case_;
    conn.run(script);
    dolphindb::BatchTableWriter btw(HOST, PORT, USER, PASSWD, true);
    btw.addTable(case_, "");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    btw.removeTable(case_, "");
    ASSERT_ANY_THROW(std::get<0>(btw.getStatus(case_, "")));
}

static void test_batchTableWriter_insert_thread_fuction(int id, dolphindb::BatchTableWriter &btw, std::string dbName, std::string tableName,
    dolphindb::TableSP data) {
    dolphindb::DBConnection connNew;
    connNew.connect(HOST, PORT, USER, PASSWD);
    size_t dataRow = data->rows();
    size_t dataColumn = data->columns();
    std::vector<dolphindb::VectorSP> dataVec;
    for (int i = 0; i < dataColumn; ++i) {
        dataVec.push_back(data->getColumn(i));
    }

    std::vector<char> boolBuffer(10);
    std::vector<char> charBuffer(10);
    std::vector<short> shortBuffer(10);
    std::vector<char*> stringBuffer(10);
    std::vector<long long> longBuffer(10);
    std::vector<long long> nanotimeBuffer(10);
    std::vector<long long> nanotimestampBuffer(10);
    std::vector<long long> timestampBuffer(10);
    std::vector<float> floatBuffer(10);
    std::vector<double> doubleBuffer(10);
    std::vector<int> intBuffer(10);
    std::vector<int> dateBuffer(10);
    std::vector<int> monthBuffer(10);
    std::vector<int> timeBuffer(10);
    std::vector<int> secondBuffer(10);
    std::vector<int> minuteBuffer(10);
    std::vector<int> datetimeBuffer(10);
    std::vector<int> datehourBuffer(10);
    std::vector<int> iidBuffer(10);

    const int maxIndex = 10;
    const char *boolPtr = dataVec[0]->getBoolConst(0, maxIndex, boolBuffer.data());
    const char *charPtr = dataVec[1]->getCharConst(0, maxIndex, charBuffer.data());
    const short *shortPtr = dataVec[2]->getShortConst(0, maxIndex, shortBuffer.data());
    char **stringPtr = dataVec[3]->getStringConst(0, maxIndex, (char **)stringBuffer.data());
    const long long *longPtr = dataVec[4]->getLongConst(0, maxIndex, longBuffer.data());
    const long long *nanotimePtr = dataVec[5]->getLongConst(0, maxIndex, nanotimeBuffer.data());
    const long long *nanotimestampPtr = dataVec[6]->getLongConst(0, maxIndex, nanotimestampBuffer.data());
    const long long *timestampPtr = dataVec[7]->getLongConst(0, maxIndex, timestampBuffer.data());
    const float *floatPtr = dataVec[8]->getFloatConst(0, maxIndex, floatBuffer.data());
    const double *doublePtr = dataVec[9]->getDoubleConst(0, maxIndex, doubleBuffer.data());
    const int *intPtr = dataVec[10]->getIntConst(0, maxIndex, intBuffer.data());
    const int *datePtr = dataVec[11]->getIntConst(0, maxIndex, dateBuffer.data());
    const int *monthPtr = dataVec[12]->getIntConst(0, maxIndex, monthBuffer.data());
    const int *timePtr = dataVec[13]->getIntConst(0, maxIndex, timeBuffer.data());
    const int *secondPtr = dataVec[14]->getIntConst(0, maxIndex, secondBuffer.data());
    const int *minutePtr = dataVec[15]->getIntConst(0, maxIndex, minuteBuffer.data());
    const int *dateTimePtr = dataVec[16]->getIntConst(0, maxIndex, dateBuffer.data());
    const int *datehourPtr = dataVec[17]->getIntConst(0, maxIndex, datehourBuffer.data());
    const int *iidPtr = dataVec[18]->getIntConst(0, maxIndex, datehourBuffer.data());
    for (int i = 0; i < dataRow; ++i) {
        if (i % maxIndex == 0) {
            int getSize = std::min(i - i / maxIndex * maxIndex, maxIndex);
            boolPtr = dataVec[0]->getBoolConst(i, getSize, boolBuffer.data());
            charPtr = dataVec[1]->getCharConst(i, getSize, charBuffer.data());
            shortPtr = dataVec[2]->getShortConst(i, getSize, shortBuffer.data());
            stringPtr = dataVec[3]->getStringConst(i, getSize, (char **)stringBuffer.data());
            longPtr = dataVec[4]->getLongConst(i, getSize, longBuffer.data());
            nanotimePtr = dataVec[5]->getLongConst(i, getSize, nanotimeBuffer.data());
            nanotimestampPtr = dataVec[6]->getLongConst(i, getSize, nanotimestampBuffer.data());
            timestampPtr = dataVec[7]->getLongConst(i, getSize, timestampBuffer.data());
            floatPtr = dataVec[8]->getFloatConst(i, getSize, floatBuffer.data());
            doublePtr = dataVec[9]->getDoubleConst(i, getSize, doubleBuffer.data());
            intPtr = dataVec[10]->getIntConst(i, getSize, intBuffer.data());
            datePtr = dataVec[11]->getIntConst(i, getSize, dateBuffer.data());
            monthPtr = dataVec[12]->getIntConst(i, getSize, monthBuffer.data());
            timePtr = dataVec[13]->getIntConst(i, getSize, timeBuffer.data());
            secondPtr = dataVec[14]->getIntConst(i, getSize, secondBuffer.data());
            minutePtr = dataVec[15]->getIntConst(i, getSize, minuteBuffer.data());
            dateTimePtr = dataVec[16]->getIntConst(i, getSize, dateBuffer.data());
            datehourPtr = dataVec[17]->getIntConst(i, getSize, datehourBuffer.data());
            iidPtr = dataVec[18]->getIntConst(i, getSize, datehourBuffer.data());
        }
        btw.insert(dbName, tableName, dolphindb::Util::createInt(id),
            dolphindb::Util::createBool(boolPtr[i % maxIndex]),
            dolphindb::Util::createChar(charPtr[i % maxIndex]),
            dolphindb::Util::createShort(shortPtr[i%maxIndex]),
            dolphindb::Util::createString(std::string((char *)(stringPtr[i % maxIndex]))),
            dolphindb::Util::createLong(longPtr[i % maxIndex]),
            dolphindb::Util::createNanoTime(nanotimePtr[i % maxIndex]),
            dolphindb::Util::createNanoTimestamp(nanotimestampPtr[i % maxIndex]),
            dolphindb::Util::createTimestamp(timestampPtr[i % maxIndex]),
            dolphindb::Util::createFloat(floatPtr[i % maxIndex]),
            dolphindb::Util::createDouble(doublePtr[i % maxIndex]),
            dolphindb::Util::createInt(intPtr[i % maxIndex]),
            dolphindb::Util::createDate(datePtr[i % maxIndex]),
            dolphindb::Util::createMonth(monthPtr[i % maxIndex]),
            dolphindb::Util::createTime(timePtr[i % maxIndex]),
            dolphindb::Util::createSecond(secondPtr[i % maxIndex]),
            dolphindb::Util::createMinute(minutePtr[i % maxIndex]),
            dolphindb::Util::createDateTime(dateTimePtr[i % maxIndex]),
            dolphindb::Util::createDateHour(datehourPtr[i % maxIndex]),
            dolphindb::Util::createInt(iidPtr[i % maxIndex])
        );
    }
    connNew.close();
}

static void test_batchTableWriter_insert_thread_fuction_using_cpp_type(int id, dolphindb::BatchTableWriter &btw, std::string dbName, std::string tableName,
    dolphindb::TableSP data) {
    dolphindb::DBConnection connNew;
    connNew.connect(HOST, PORT, USER, PASSWD);
    size_t dataRow = data->rows();
    size_t dataColumn = data->columns();
    std::vector<dolphindb::VectorSP> dataVec;
    for (int i = 0; i < dataColumn; ++i) {
        dataVec.push_back(data->getColumn(i));
    }
    std::vector<char> boolBuffer(10);
    std::vector<char> charBuffer(10);
    std::vector<short> shortBuffer(10);
    std::vector<char*> stringBuffer(10);
    std::vector<long long> longBuffer(10);
    std::vector<long long> nanotimeBuffer(10);
    std::vector<long long> nanotimestampBuffer(10);
    std::vector<long long> timestampBuffer(10);
    std::vector<float> floatBuffer(10);
    std::vector<double> doubleBuffer(10);
    std::vector<int> intBuffer(10);
    std::vector<int> dateBuffer(10);
    std::vector<int> monthBuffer(10);
    std::vector<int> timeBuffer(10);
    std::vector<int> secondBuffer(10);
    std::vector<int> minuteBuffer(10);
    std::vector<int> datetimeBuffer(10);
    std::vector<int> datehourBuffer(10);
    std::vector<int> iidBuffer(10);
    std::vector<char*>stringCPPBuffer(10);

    const int maxIndex = 10;

    const char *boolPtr = dataVec[0]->getBoolConst(0, maxIndex, boolBuffer.data());
    const char *charPtr = dataVec[1]->getCharConst(0, maxIndex, charBuffer.data());
    const short *shortPtr = dataVec[2]->getShortConst(0, maxIndex, shortBuffer.data());
    char **stringPtr = dataVec[3]->getStringConst(0, maxIndex, (char **)stringBuffer.data());
    const long long *longPtr = dataVec[4]->getLongConst(0, maxIndex, longBuffer.data());
    const long long *nanotimePtr = dataVec[5]->getLongConst(0, maxIndex, nanotimeBuffer.data());
    const long long *nanotimestampPtr = dataVec[6]->getLongConst(0, maxIndex, nanotimestampBuffer.data());
    const long long *timestampPtr = dataVec[7]->getLongConst(0, maxIndex, timestampBuffer.data());
    const float *floatPtr = dataVec[8]->getFloatConst(0, maxIndex, floatBuffer.data());
    const double *doublePtr = dataVec[9]->getDoubleConst(0, maxIndex, doubleBuffer.data());
    const int *intPtr = dataVec[10]->getIntConst(0, maxIndex, intBuffer.data());
    const int *datePtr = dataVec[11]->getIntConst(0, maxIndex, dateBuffer.data());
    const int *monthPtr = dataVec[12]->getIntConst(0, maxIndex, monthBuffer.data());
    const int *timePtr = dataVec[13]->getIntConst(0, maxIndex, timeBuffer.data());
    const int *secondPtr = dataVec[14]->getIntConst(0, maxIndex, secondBuffer.data());
    const int *minutePtr = dataVec[15]->getIntConst(0, maxIndex, minuteBuffer.data());
    const int *dateTimePtr = dataVec[16]->getIntConst(0, maxIndex, dateBuffer.data());
    const int *datehourPtr = dataVec[17]->getIntConst(0, maxIndex, datehourBuffer.data());
    const int *iidPtr = dataVec[18]->getIntConst(0, maxIndex, datehourBuffer.data());
    char **stringCPPPtr = dataVec[3]->getStringConst(0, maxIndex, (char **)stringCPPBuffer.data());
    for (int i = 0; i < dataRow; ++i) {
        if (i % maxIndex == 0) {
            int getSize = std::min(i - i / maxIndex * maxIndex, maxIndex);
            boolPtr = dataVec[0]->getBoolConst(i, getSize, boolBuffer.data());
            charPtr = dataVec[1]->getCharConst(i, getSize, charBuffer.data());
            shortPtr = dataVec[2]->getShortConst(i, getSize, shortBuffer.data());
            stringPtr = dataVec[3]->getStringConst(i, getSize, (char **)stringBuffer.data());
            longPtr = dataVec[4]->getLongConst(i, getSize, longBuffer.data());
            nanotimePtr = dataVec[5]->getLongConst(i, getSize, nanotimeBuffer.data());
            nanotimestampPtr = dataVec[6]->getLongConst(i, getSize, nanotimestampBuffer.data());
            timestampPtr = dataVec[7]->getLongConst(i, getSize, timestampBuffer.data());
            floatPtr = dataVec[8]->getFloatConst(i, getSize, floatBuffer.data());
            doublePtr = dataVec[9]->getDoubleConst(i, getSize, doubleBuffer.data());
            intPtr = dataVec[10]->getIntConst(i, getSize, intBuffer.data());
            datePtr = dataVec[11]->getIntConst(i, getSize, dateBuffer.data());
            monthPtr = dataVec[12]->getIntConst(i, getSize, monthBuffer.data());
            timePtr = dataVec[13]->getIntConst(i, getSize, timeBuffer.data());
            secondPtr = dataVec[14]->getIntConst(i, getSize, secondBuffer.data());
            minutePtr = dataVec[15]->getIntConst(i, getSize, minuteBuffer.data());
            dateTimePtr = dataVec[16]->getIntConst(i, getSize, dateBuffer.data());
            datehourPtr = dataVec[17]->getIntConst(i, getSize, datehourBuffer.data());
            iidPtr = dataVec[18]->getIntConst(i, getSize, datehourBuffer.data());
            stringCPPPtr = dataVec[3]->getStringConst(i, getSize, (char **)stringCPPBuffer.data());
        }
        btw.insert(dbName, tableName, id,
            boolPtr[i%maxIndex],
            charPtr[i%maxIndex],
            shortPtr[i%maxIndex],
            (const char*)(stringPtr[i%maxIndex]),
            longPtr[i%maxIndex],
            nanotimePtr[i%maxIndex],
            nanotimestampPtr[i%maxIndex],
            timestampPtr[i%maxIndex],
            floatPtr[i%maxIndex],
            doublePtr[i%maxIndex],
            intPtr[i%maxIndex],
            datePtr[i%maxIndex],
            monthPtr[i%maxIndex],
            timePtr[i%maxIndex],
            secondPtr[i%maxIndex],
            minutePtr[i%maxIndex],
            dateTimePtr[i%maxIndex],
            datehourPtr[i%maxIndex],
            iidPtr[i%maxIndex],
            std::string(stringCPPPtr[i%maxIndex])
        );
    }
    connNew.close();
}

static bool stopFlag = false;

static void batchTableWriter_thread_getAllStatus(dolphindb::BatchTableWriter &bts) {
    while (!stopFlag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        dolphindb::TableSP t = bts.getAllStatus();
    }
}

static void batchTableWriter_thread_getStatus(dolphindb::BatchTableWriter &bts, std::string dbName, std::string tableName) {
    while (!stopFlag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        std::tuple<int, bool, bool> t = bts.getStatus(dbName, tableName);
    }
}

TEST_F(BatchTableWriterTest,test_batchTableWriter_getStatus_exception){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script;
    script += "dbPath = '"+dbName+"';";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    conn.run(script);
    dolphindb::BatchTableWriter btw(HOST, PORT, USER, PASSWD, true);
    try {
        std::tuple<int, bool, bool> status = btw.getStatus(dbName, "");
    }
    catch (std::exception& e) {
        ASSERT_STREQ(e.what(),"Failed to get queue depth. Please use addTable to add infomation of database and table first.");
    }
}

static void test_fuc_addTable(dolphindb::BatchTableWriter &btw, std::string dbName, std::string tableName, int cycles) {
    try {
        for (int i = 0; i < cycles; ++i) {
            btw.addTable(dbName, tableName);
        }
    }
    catch (std::exception &e) {
        std::cout<<e.what()<<std::endl;
        std::cout<<"FAILED--test_fuc_addTable"<<std::endl;
        ASSERT_EQ(1,2);
    }
}

static void test_fuc_removeTable(dolphindb::BatchTableWriter &btw, std::string dbName, std::string tableName, int cycles) {
    try {
        for (int i = 0; i < cycles; ++i) {
            btw.removeTable(dbName, tableName);
        }
    }
    catch (std::exception &e) {
        std::cout<<e.what()<<std::endl;
        std::cout<<"FAILED--test_fuc_removeTable"<<std::endl;
        ASSERT_EQ(1,2);
    }
}

static void test_fuc_getStatus(dolphindb::BatchTableWriter &btw, std::string dbName, std::string tableName, int cycles) {
    try {
        for (int i = 0; i < cycles; ++i) {
            std::tuple<int, bool, bool> status =btw.getStatus(dbName, tableName);
            ASSERT_EQ(std::get<0>(status),0);
        }
    }
    catch (std::exception &e) {
        std::cout<<e.what()<<std::endl;
        std::cout<<"FAILED--test_fuc_getStatus"<<std::endl;
        ASSERT_EQ(1,2);
    }
}

static void test_fuc_getAllStatus(dolphindb::BatchTableWriter &btw, int cycles) {    
    try {
        for (int i = 0; i < cycles; ++i) {
            dolphindb::TableSP allStatus = btw.getAllStatus();
            for(int j=0;j<allStatus->rows();j++)
            ASSERT_EQ(allStatus->getColumn(2)->get(j)->getInt(),0);
        }
    }
    catch (std::exception &e) {
        std::cout<<e.what()<<std::endl;
        std::cout<<"FAILED--test_fuc_getAllStatus"<<std::endl;
        ASSERT_EQ(1,2);            
    }
}

TEST_F(BatchTableWriterTest,test_multithread){
    std::string case_=getCaseName();
    std::string dbName="dfs://" + case_;
    std::string script;
    script += "dbPath = '"+dbName+"';";
    script += "if(existsDatabase(dbPath)){dropDatabase(dbPath)};";
    script += "db=database(dbPath,VALUE,1..100);";
    script += "t=table(100:0,`id`dbbool `dbchar `dbshort  `dbstring `dblong `dbnanotime `dbnanotimestamp `dbtimestamp `dbfloat `dbdouble `dbint `dbdate `dbmonth `dbtime `dbsecond `dbminute `dbdatetime `dbdatehour `iid, [INT, BOOL, CHAR, SHORT, STRING, LONG, NANOTIME, NANOTIMESTAMP, TIMESTAMP, FLOAT, DOUBLE, INT, DATE, MONTH, TIME, SECOND, MINUTE, DATETIME, DATEHOUR, INT]);";
    script += "db.createPartitionedTable(t,`test_multithread,`id);";
    conn.run(script);
    dolphindb::BatchTableWriter btw(HOST, PORT, USER, PASSWD, true);
    std::string tableName = "test_multithread";
    int cycles = 10000;
    btw.addTable(dbName, tableName);
    std::thread t2(test_fuc_getStatus, std::ref(btw), dbName, tableName, cycles);
    std::thread t3(test_fuc_getAllStatus, std::ref(btw), cycles);
    t2.join();
    t3.join();
    std::thread t4(test_fuc_removeTable, std::ref(btw), dbName, tableName, cycles);
    t4.join();
}

static void BatchTableWriter_insert_error_type(std::string destType, dolphindb::BatchTableWriter& btw) {
    if(destType == "BOOL" || destType== "CHAR"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", short(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", int(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createNanoTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createNanoTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "SHORT"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", int(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createNanoTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createNanoTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "STRING"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", int(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createNanoTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createNanoTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "LONG"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", int(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createNanoTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createNanoTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "NANOTIME"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", int(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createNanoTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "NANOTIMESTAMP"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", int(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "TIMESTAMP"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", int(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "FLOAT"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", int(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "DOUBLE"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", int(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "INT"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "DATE"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "MONTH"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "TIME"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "SECOND"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "MINUTE"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "DATETIME"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateHour(1)));
    }
    else if(destType== "DATEHOUR"){
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", bool(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", char(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", long(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", (long long)(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", double(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", float(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", std::to_string(1).c_str()));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createShort(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createLong(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createInt(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDouble(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createFloat(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDate(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMonth(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createMinute(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createSecond(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createDateTime(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createTimestamp(1)));
        ASSERT_ANY_THROW(btw.insert("batchTableWriter", "", dolphindb::Util::createString("1")));
    }
}

TEST_F(BatchTableWriterTest,test_BatchTableWriter_insert_error_type){
    std::string case_=getCaseName();
    for (int i = 0; i < type.size(); i++) {
        std::string script;
        script += "share table(100:0,[`test], [" + type[i] + "]) as "+case_;
        conn.run(script);
        dolphindb::BatchTableWriter btw(HOST, PORT, USER, PASSWD, true);
        btw.addTable(case_, "");
        BatchTableWriter_insert_error_type(type[i], std::ref(btw));
    }
    std::string script;
    conn.run(script);
}
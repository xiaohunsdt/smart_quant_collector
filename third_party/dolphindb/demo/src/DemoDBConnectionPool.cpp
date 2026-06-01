#include "DolphinDB.h"
#include <iostream>
using namespace dolphindb;
using namespace std;

int main()
{
    auto cv = std::make_shared<std::condition_variable>();
    DBConnectionPool p("127.0.0.1", 8848, 1, "admin", "123456");
    int id = p.run("version()", cv);
    std::mutex cv_m;
    std::unique_lock<std::mutex> lk(cv_m);
    cv->wait(lk, [id, &p] {
        return p.isFinished(id);
    });
    std::cout << p.getData(id)->getString() << std::endl;
}

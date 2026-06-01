#include "TaskStatusMgmt.h"
#include "Constant.h"
#include "Exceptions.h"

#include <cassert>
#include <mutex>
#include <string>
#include <utility>

namespace dolphindb {

bool TaskStatusMgmt::isFinished(int identity){
    std::unique_lock<std::mutex> guard(mutex_);
    if(results.count(identity) == 0)
        throw RuntimeException("Task [" + std::to_string(identity) + "] does not exist.");
    if(results[identity].stage == TaskStatus::ERRORED)
        throw RuntimeException("Task [" + std::to_string(identity) + "] come across exception : " + results[identity].errMsg);
    return results[identity].stage == TaskStatus::FINISHED;
}

void TaskStatusMgmt::setResult(int identity, TaskResult r){
    std::unique_lock<std::mutex> guard(mutex_);
    results[identity] = std::move(r);
}

ConstantSP TaskStatusMgmt::getData(int identity){
    std::unique_lock<std::mutex> guard(mutex_);
    if(results.count(identity) == 0)
        throw RuntimeException("Task [" + std::to_string(identity) + "] does not exist, the result may be fetched yet.");
    assert(results[identity].stage == TaskStatus::FINISHED);
    ConstantSP re = results[identity].result;
    results.erase(identity);
    return re;
}

} // namespace dolphindb

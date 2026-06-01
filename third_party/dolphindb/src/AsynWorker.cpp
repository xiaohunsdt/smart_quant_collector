#include "AsynWorker.h"
#include "Constant.h"
#include "DolphinDB.h"
#include "Exceptions.h"
#include "ScalarImp.h"
#include "TaskStatusMgmt.h"

#include <iostream>

namespace dolphindb {
    void AsynWorker::run() {
    while(true) {
        if(pool_.isShutDown()){
            conn_->close();
            latch_->countDown();
            std::cout<<"Asyn worker closed peacefully."<<std::endl;
            break;
        }

        Task task;
        ConstantSP result;
        bool errorFlag = false;
        if (!queue_->blockingPop(task, 1000))
            continue;
        if (task.script.empty())
            continue;
        while(true) {
            try {
                if(task.isFunc){
                    result = conn_->run(task.script, task.arguments, task.param.priority, task.param.parallelism, task.param.fetchSize, task.param.clearMemory);
                }
                else{
                    result = conn_->run(task.script, task.param.priority, task.param.parallelism, task.param.fetchSize, task.param.clearMemory);
                }
                break;
            }
            catch(IOException & ex){
                errorFlag = true;
                std::cerr<<"Async task worker come across exception : "<<ex.what()<<std::endl;
                taskStatus_.setResult(task.identity, TaskResult(TaskStatus::ERRORED, new Void(), ex.what()));
                if (task.finished != nullptr) {
                    task.finished->notify_all();
                }
                break;
            }
        }
        if (errorFlag) {
            continue;
        }
        taskStatus_.setResult(task.identity, TaskResult(TaskStatus::FINISHED,  result));
        if (task.finished != nullptr) {
            task.finished->notify_all();
        }
    }
}

} // namespace dolphindb

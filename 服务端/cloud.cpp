#include "manager.hpp"
#include "config.hpp"
#include "util.hpp"
#include "service.hpp"
#include <thread>

using namespace cloud_util;
using namespace cloud;

void HotRun(){
    HotManager hm;
    hm.RunModule();
}

void ServiceRun(){
    Service server;
    server.RunModule();
}

int main(int argc, const char* argv[]){
    
    std::thread thread_hot_manager(HotRun);
    std::thread thread_service(ServiceRun);

    thread_hot_manager.join();
    thread_service.join();

    return 0;
}

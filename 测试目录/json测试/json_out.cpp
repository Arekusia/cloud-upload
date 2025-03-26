#include<iostream>
#include<sstream>
#include<string>
#include<memory>
#include<jsoncpp/json/json.h>

int main(){
    const char* name = "mike";
    int age = 12;
    float score[] = {12.3, 45, 67};
    Json::Value root;
    root["name"] = name;
    root["age"] = age;
    root["score"].append(score[0]);
    root["score"].append(score[2]);
    root["score"].append(score[2]);
    
    Json::StreamWriterBuilder swb;
    std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
    std::stringstream ss;
    sw->write(root, &ss);
    std::cout << ss.str() << std::endl;
    return 0;
}

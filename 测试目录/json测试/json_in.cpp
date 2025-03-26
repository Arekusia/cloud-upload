#include<iostream>
#include<string>
#include<memory>
#include<jsoncpp/json/json.h>

int main(){
    std::string str = R"({"age": 12, "name": "mike","score": [12.300000190734863,67,67]})";
    Json::Value root;
    Json::CharReaderBuilder crb;
    std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
    std::string errmsg;
    bool res = cr->parse(str.c_str(), str.c_str() + str.size(), &root, &errmsg);
    if(!res){
        std::cout << errmsg << std::endl;
        return -1;
    }
    std::cout << root["name"].asString() << std::endl;
    std::cout << root["age"].asInt() << std::endl;
    for(unsigned int i = 0; i < root["score"].size(); ++i){
        std::cout << root["score"][i] << " ";
    }std::cout << std::endl;
    return 0;
}

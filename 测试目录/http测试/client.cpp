#include"httplib.h"

#define SERVER_IP "8.130.20.13"
#define SERVER_PORT 8080

int main(){
    using namespace httplib;
    Client client(SERVER_IP, SERVER_PORT);
    MultipartFormData item;
    item.name = "file";
    item.filename = "hello.txt";
    item.content = "Hello world";
    item.content_type = "text/plain";

    MultipartFormDataItems items;
    items.push_back(item);
    auto res = client.Post("/multipart", items);
    std::cout << res->status << std::endl;
    std::cout << res->body << std::endl;
    
    return 0;
}

#include"httplib.h"

int main(){
    using namespace httplib;

    Server server;
    server.Get("/hi", [](const Request& req, Response& rep){
        rep.set_content("hello niumo", "text/plain");
        rep.status = 200;
    });
    server.Get(R"(/numbers/(\d+))", [](const Request& req, Response& rep){
        auto num = req.matches[1];
        rep.set_content(num, "text/plain");
        rep.status = 200;
    });
    server.Post("/multipart", [](const Request& req, Response& rep){
        auto res = req.has_file("file");
        if(res == false){
            std::cout << "wrong file upload" << std::endl;
            rep.status = 400;
            return;
        }
        const auto& file = req.get_file_value("file");
        rep.body.clear();
        rep.body = file.filename;
        rep.body += "\n";
        rep.body += file.content;
        rep.set_header("Content-Type", "text/plain");
        rep.status = 200;
        return;
    });
    int a;
    std::cin >> a;
    if(a == 0){
        server.listen("0.0.0.0", 8080);
    }else if(a == 1){
        server.listen("8.130.20.13", 8080);
    }else{ 
        server.listen("172.16.0.0", 8080);
    }
    return 0;
}

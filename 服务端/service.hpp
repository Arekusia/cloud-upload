#pragma once 
#include "manager.hpp"
#include "httplib.h"

using namespace cloud_util;
using namespace httplib;

namespace cloud{
    class Service{
        public:
            bool RunModule(){
                _server.Post("/upload", Upload);
                _server.Get("/listshow", ListShow);
                _server.Get("/", ListShow);
                _server.Get(cfg->GetDownloadPrefix() + "(.*)", Download);
                std::cout << "server begin listen" << std::endl;
                _server.listen(cfg->GetServerIp(), cfg->GetServerPort());
                return true;
            }
        private:
            static Config* cfg;
            Server _server;

            static std::string TimeToString(time_t t){
                return std::ctime(&t);
            }
            static std::string GetEtag(const BackupInfo& info){
                std::string etag = info.file_name;
                etag += "-";
                etag += std::to_string(info.fsize);
                etag += "-";
                etag += std::to_string(info.mtime);
                return etag;
            }

            static void Upload(const Request& req, Response& rep){
                auto res = req.has_file("file");
                if(!res){
                    rep.status = 400;
                    std::cerr << "Upload file failed" << std::endl;
                    return;
                }

                const auto& file = req.get_file_value("file");

                std::string filename = cfg->GetBackDir() + file.filename;
                FileUtil fu(filename);
                try{
                    fu.SetContent(file.content);
                }catch(std::exception& e){
                    std::cerr << "Upload file SetContent failed" << std::endl;
                    std::cerr << e.what() << std::endl;
                    rep.status = 400;
                    return;
                }

                BackupInfo info(filename);
                _data->Insert(info);
                std::cout << "Upload file " << filename <<" success" << std::endl;
            }
            static void ListShow(const Request& req, Response& rep){
                std::vector<BackupInfo> arry = _data->GetAll();
                std::stringstream ss;

                ss << R"(<!DOCTYPE html><html lang="en"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1.0"><title>download</title></head><body><h1>Download</h1><table>)";
                for(auto& val : arry){
                    ss << R"(<tr>)";
                    ss << R"(<td><a href = ")" << val.url_path << R"(">)" << val.file_name << R"(</a></td>)";
                    ss << R"(<td align = "right">)" << TimeToString(val.mtime) << R"(</td>)";
                    ss << R"(<td align = "right">)" << val.fsize / 1024 << R"(k</td>)";
                    ss << R"(</tr>)";
                }
                ss << R"(</table></body></html>)";

                rep.body = ss.str();
                rep.set_header("Content-Type", "text/html");
                rep.status = 200;
                return;
            }
            static void Download(const Request& req, Response& rep){
                BackupInfo info = _data->GetOneByUrl(req.path);
                if(info.pack_flag){// 解压文件
                    FileUtil fu(info.pack_path);
                    fu.UnCompress(info.real_path);
                    fu.Remove();
                    info.pack_flag = false;
                    _data->Update(info);
                }

                bool resend = false;// 断点续传判断
                if(req.has_header("If-Range")){// 检测下载是否中止
                    std::string old_etag = req.get_header_value("If-Range");
                    if(old_etag == GetEtag(info)){// 判断中断的文件是否为同一个文件
                        resend = true;
                    }
                }
                FileUtil fu(info.real_path);
                if(!resend){// 正常传输
                    std::string body = fu.GetContent();
                    rep.set_header("Accept-Ranges", "bytes");
                    rep.set_header("ETag", GetEtag(info));
                    rep.set_header("Content-Type", "application/octet-stream");
                    rep.status = 200;
                }else{// 断点续传
                    std::string body = fu.GetContent();
                    rep.set_header("Accept-Ranges", "bytes");
                    rep.set_header("ETag", GetEtag(info));
                    rep.status = 206;
                }
                std::cout << "Download " << info.real_path << " success" << std::endl;
            }
    };
    Config* Service::cfg = Config::GetInstance();
}

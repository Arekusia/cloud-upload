#pragma once

#include <mutex>
#include "util.hpp"

namespace cloud{
#define CONFIG_FILE "./cloud.conf"

    using namespace cloud_util;
    class Config{
        public:
            static Config* GetInstance(){
                std::lock_guard<std::mutex> lock(_mutex);
                if(_instance == nullptr){
                    _instance = new Config();
                }
                return _instance;
            }

            int GetHotTime(){
                return _hot_time;
            }
            int GetServerPort(){
                return _server_port;
            }
            std::string GetServerIp(){
                return _server_ip;
            }
            std::string GetDownloadPrefix(){
                return _download_prefix;
            }
            std::string GetPackfileSuffix(){
                return _packfile_suffix;
            }
            std::string GetPackDir(){
                return _pack_dir;
            }
            std::string GetBackDir(){
                return _back_dir;
            }
            std::string GetBackupFile(){
                return _backup_file;
            }

        private:
            Config(){
                ReadConfigFile();
                FileUtil backdir(_back_dir);
                FileUtil packdir(_pack_dir);

                if(!backdir.Exists()){// 检查备份文件夹是否存在
                    backdir.CreateDirectory();
                }
                if(!packdir.Exists()){// 检查压缩文件夹是否存在
                    packdir.CreateDirectory();
                }
            }
            Config(const Config&) = delete;
            Config& operator=(const Config&) = delete;
            ~Config(){
                if(_instance){
                    delete _instance;
                }
            }

            bool ReadConfigFile(){// 读取配置文件
                try{
                    FileUtil fu(CONFIG_FILE);
                    std::string body = fu.GetContent();

                    Json::Value root;
                    if(!JsonUtil::UnSerialize(body, root)){
                        std::cerr << "ReadConfigFile set conf failed" << std::endl;
                        return false;
                    }

                    _hot_time = root["hot_time"].asInt();
                    _server_port = root["server_port"].asInt();
                    _server_ip = root["server_ip"].asString();
                    _download_prefix = root["download_prefix"].asString();
                    _packfile_suffix = root["packfile_suffix"].asString();
                    _pack_dir = root["pack_dir"].asString();
                    _back_dir = root["back_dir"].asString();
                    _backup_file = root["backup_file"].asString();

                    return true;
                }catch(std::exception& e){
                    std::cerr << "ReadConfigFile get conf failed" << std::endl;
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            static Config* _instance;
            static std::mutex _mutex;

            int _hot_time;// 热点时间
            int _server_port;// 服务器端口
            std::string _server_ip;// 服务器ip
            std::string _download_prefix;// 下载请求前缀
            std::string _packfile_suffix;// 压缩后缀
            std::string _pack_dir;// 压缩路径
            std::string _back_dir;// 备份路径
            std::string _backup_file;// 持久化存储文件
    };
    Config* Config::_instance = nullptr;
    std::mutex Config::_mutex;
}

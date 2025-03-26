#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <experimental/filesystem>
#include <jsoncpp/json/json.h>
#include <unistd.h>
#include "bundle.h"

namespace cloud_util{
    namespace fs = std::experimental::filesystem;
    class FileUtil{
        public:
            FileUtil(const std::string& filename):
                _filename(filename){}//传入文件路径或文件夹路径，不可使用文件不支持的方法

            bool Exists(){// 判断文件是否存在
                return fs::exists(_filename);
            }

            bool IsFile() {// 判断格式是否为文件
                return !(_filename.find(".") == std::string::npos || _filename[_filename.size() - 1] == '/');
            }

            int64_t FileSize(){// 获取文件(文件夹)大小
                struct stat st;
                if(stat(_filename.c_str(), &st) < 0){
                    std::cerr << "get file FileSize failed" << std::endl;
                    return -1;
                }
                return st.st_size;
            }
            
            time_t LastMtime(){// 获取文件(文件夹)最后修改时间
                struct stat st;
                if(stat(_filename.c_str(), &st) < 0){
                    std::cerr << "get file LastMtime failed" << std::endl;
                    return -1;
                }
                return st.st_mtime;
            }
            
            time_t LastAtime(){// 获取文件(文件夹)最后访问时间
                struct stat st;
                if(stat(_filename.c_str(), &st) < 0){
                    std::cerr << "get file LastAtime failed" << std::endl;
                    return -1;
                }
                return st.st_atime;
            }
            
            std::string FileName(){// 从文件路径中截取文件名
                size_t pos = _filename.rfind("/");
                if(pos == std::string::npos){
                    return _filename;
                }
                return _filename.substr(pos + 1);
            }
            
            std::string GetContent(size_t pos, size_t len){// 从pos开始长度为len的文件内容以string格式写入到返回值

                if(!IsFile()){
                    throw std::runtime_error("GetContent don't support directory");// 传入的路径不是文件
                }

                size_t fsize = this->FileSize();
                if(pos + len > fsize){
                    throw std::out_of_range("GetContent file size overflow");
                }

                std::ifstream ifs;
                ifs.open(_filename, std::ios::binary);
                if(!ifs.is_open()){
                    throw std::runtime_error("GetContent open file failed");
                }
                ifs.seekg(pos, std::ios::beg);

                std::string body;
                body.resize(len);
                ifs.read(&body[0], len);
                
                if(!ifs.good()){
                    ifs.close();
                    throw std::runtime_error("GetContent file read falied");
                }
                ifs.close();
                return body;
            }
            
            std::string GetContent(){// 将文件所有内容写入到body
                return GetContent(0, this->FileSize());
            }
            
            bool SetContent(const std::string& body){// 将body中的内容写入到文件中
                if(!IsFile()){
                    throw std::runtime_error("SetContent don't support directory");// 不可将内容写入文件夹
                }

                std::ofstream ofs;
                ofs.open(_filename, std::ios::binary);
                if(!ofs.is_open()){
                    std::cerr << "SetContent file open failed" << std::endl;
                    return false;
                }
                ofs.write(body.c_str(),body.size());
                if(!ofs.good()){
                    std::cerr << "SetContent file write failed" << std::endl;
                    ofs.close();
                    return false; 
                }
                ofs.close();
                return true;
            }

            bool Remove(){// 删除文件(文件夹)
                if(!Exists()){
                    std::cerr << "Remove file not exists" << std::endl;
                    return false;
                }
                try {
                    if (IsFile()) {
                        if (std::remove(_filename.c_str()) != 0) {
                            std::cerr << "Remove file failed" << std::endl;
                            return false;
                        }
                    } else {
                        fs::remove_all(_filename);
                    }
                } catch (const fs::filesystem_error& e) {
                    std::cerr << "Remove failed: " << e.what() << std::endl;
                    return false;
                }
                return true;
            }
            
            bool Compress(const std::string& packpath){// 将文件压缩后存放到packpath(含压缩后的文件名)路径下
                try{
                    if(!IsFile()){
                        throw std::runtime_error("Compress don't support directory");// 不可压缩文件夹
                    }

                    std::string body = GetContent();
                    std::string packed = bundle::pack(bundle::LZIP, body);
                    FileUtil fu(packpath);
                    if(!fu.SetContent(packed)){
                        std::cerr << "Compress file set failed" << std::endl;
                        return false;
                    }
                    return true;

                }catch(std::exception& e){
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            bool UnCompress(const std::string& filepath){// 将文件解压后存放到filepath(含解压后的文件名)路径下
                try{
                    if(!IsFile()){
                        throw std::runtime_error("UnCompress don't support directory");// 不可解压文件夹
                    }

                    std::string body = GetContent();
                    std::string unpacked = bundle::unpack(body);
                    FileUtil fu(filepath);
                    if(!fu.SetContent(unpacked)){
                        std::cerr << "UnCompress file set failed" << std::endl;
                        return false;
                    }
                    return true;

                }catch(std::exception& e){
                    std::cerr << e.what() << std::endl;
                    return false;
                }
            }

            bool CreateDirectory(){// 根据路径创建文件夹
                if(IsFile()){
                    return false;
                }
                if(Exists()){
                    return true;
                }
                return fs::create_directories(_filename);
            }

            std::vector<std::string> ScanDirectory(){// 遍历文件夹内的非文件夹内容
                if(IsFile()){
                    throw std::runtime_error("ScanDirectory don't support file");// 不可遍历文件
                }
                std::vector<std::string> arry;
                for(auto& it : fs::directory_iterator(_filename)){
                    if(fs::is_directory(it.path())){
                        continue;
                    }
                    arry.push_back(fs::path(it).relative_path().string());
                }
                return arry;
            }
            
        private:
            std::string _filename;// 文件路径或文件夹路径
    };

    class JsonUtil{
        public:
            static bool Serialize(const Json::Value& root, std::string& str){// 序列化(json->string)
                Json::StreamWriterBuilder swb;
                std::unique_ptr<Json::StreamWriter> sw(swb.newStreamWriter());
                std::stringstream ss;
                if(sw->write(root, &ss) < 0){
                    std::cerr << "write json to string failed" << std::endl;
                    return false;
                }
                str = ss.str();
                return true;
            }
            static bool UnSerialize(const std::string& str, Json::Value& root){// 反序列化(string->json)
                Json::CharReaderBuilder crb;
                std::unique_ptr<Json::CharReader> cr(crb.newCharReader());
                std::string errmsg;
                if(!cr->parse(str.c_str(), str.c_str() + str.size(), &root, &errmsg)){
                    std::cerr << errmsg << std::endl;
                    return false;
                }
                return true;
            }
    };
}

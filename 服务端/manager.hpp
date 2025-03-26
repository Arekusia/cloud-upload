#pragma once 
#include <unordered_map>
#include <mutex>
#include "config.hpp"
#include "util.hpp"

using namespace cloud_util;
namespace cloud{
    struct BackupInfo{// 用于持久化存储
        bool pack_flag;// 是否压缩
        std::string file_name;// 文件名
        size_t fsize;// 文件大小
        time_t atime;// 最后访问时间
        time_t mtime;// 最后修改时间
        std::string real_path;// 相对路径
        std::string pack_path;// 压缩路径
        std::string url_path;// 下载请求资源路径
        BackupInfo():
            pack_flag(false),
            file_name("error"),
            fsize(0),
            atime(0),
            mtime(0),
            real_path("error"),
            pack_path("error"),
            url_path("error")
        {}
        BackupInfo(const std::string& realpath):
            pack_flag(false),
            file_name("error"),
            fsize(0),
            atime(0),
            mtime(0),
            real_path("error"),
            pack_path("error"),
            url_path("error")
        {
            FileUtil fu(realpath);
            if(!fu.Exists()){
                std::cerr << "BackupInfo_t file not exists" << std::endl;
                return;
            }
            Config* cfg = Config::GetInstance();

            pack_flag = false;
            file_name = fu.FileName();
            fsize = fu.FileSize();
            mtime = fu.LastMtime();
            atime = fu.LastAtime();
            real_path = realpath;
            pack_path = cfg->GetPackDir() + fu.FileName() + cfg->GetPackfileSuffix();
            // ./packdir/ + filename.txt + .lz 
            url_path = cfg->GetDownloadPrefix() + fu.FileName();
            // /download/ + filename.txt
            // 存在风险,不同路径的同名文件会生成相同的url_path,上同
        }
    };
    
    class DataManager{//持久化存储管理
        public:
            DataManager():
                _backup_file(Config::GetInstance()->GetBackupFile())
            {
                InitLoad();
            }
            ~DataManager(){
                Storage();
            }
            bool Storage(){// 存在风险，如果类实例化了多个对象，后析构的会覆盖掉先析构所前一个对象保存的的数据，导致数据残缺
                std::lock_guard<std::mutex> lock(_rwlock);
                std::vector<BackupInfo> arry = GetAll();
                Json::Value root;
                for(auto& val : arry){
                    Json::Value tmp;
                    tmp["pack_flag"] = val.pack_flag;
                    tmp["file_name"] = val.file_name;
                    tmp["fsize"] = static_cast<Json::Value::UInt64>(val.fsize);
                    tmp["atime"] = static_cast<Json::Value::UInt64>(val.atime);
                    tmp["mtime"] = static_cast<Json::Value::UInt64>(val.mtime);
                    tmp["real_path"] = val.real_path;
                    tmp["pack_path"] = val.pack_path;
                    tmp["url_path"] = val.url_path;
                    root.append(tmp);
                }
                std::string body;
                JsonUtil::Serialize(root, body);
                std::ofstream ofs;
                FileUtil fu(_backup_file);
                try{
                    fu.SetContent(body);
                }catch(std::exception& e){
                    std::cerr << "Storage SetContent failed" << std::endl;
                    std::cerr << e.what() << std::endl;
                    return false;
                }
                return true;
            }
            bool InitLoad(){
                std::lock_guard<std::mutex> lock(_rwlock);
                FileUtil fu(_backup_file);
                if(!fu.Exists()){
                    return true;
                }
                std::string body =fu.GetContent(); 
                Json::Value root;
                JsonUtil::UnSerialize(body, root);

                for(int i = 0; i < root.size(); ++i){
                    BackupInfo info;
                    info.pack_flag = root[i]["pack_flag"].asBool();
                    info.file_name = root[i]["file_name"].asString();
                    info.fsize = root[i]["fsize"].asUInt64();
                    info.atime = root[i]["atime"].asUInt64();
                    info.mtime = root[i]["mtime"].asUInt64();
                    info.real_path = root[i]["real_path"].asString();
                    info.pack_path = root[i]["pack_path"].asString();
                    info.url_path = root[i]["url_path"].asString();
                    InitInsert(info);
                }
                return true;
            }
            
            bool Insert(const BackupInfo& info){// 仅会传入文件名
                _rwlock.lock();
                _table[info.url_path] = info;// 同名文件直接覆盖
                _rwlock.unlock();
                Storage();// 不安全
                return true;
            }
            bool Update(const BackupInfo& info){
                _rwlock.lock();
                _table[info.url_path] = info;// 覆盖原文件
                _rwlock.unlock();
                Storage();//不安全
                return true;
            }
            
            BackupInfo GetOneByUrl(const std::string& url){
                auto it = _table.find(url);
                if(it == _table.end()){
                    return BackupInfo();
                }
                return it->second;
            }
            BackupInfo GetOneByRealpath(const std::string & realpath){
                for(auto& val : _table){
                    if(val.second.real_path == realpath){
                        return val.second;
                    }
                }
                return BackupInfo();
            }
            
            std::vector<BackupInfo> GetAll(){
                std::vector<BackupInfo> arry;
                for(auto& val : _table){
                    arry.push_back(val.second);
                }
                return std::forward<std::vector<BackupInfo>>(arry);
            }
    
        private:
            std::string _backup_file;// 持久化存储文件
            std::unordered_map<std::string, BackupInfo> _table;
            std::mutex _rwlock;

            bool InitInsert(const BackupInfo& info){//适用于初始化的无存储插入
                _table[info.url_path] = info;
                return true;
            }
    };
    
    
        std::unique_ptr<DataManager> _data(new DataManager());//用于其他文件的调用，需确保头文件包含顺序正确

    class HotManager{
        public:
            HotManager():
                cfg(Config::GetInstance())        
            {}
            bool RunModule(){
                while(1){
                    FileUtil fu(cfg->GetBackDir());
                    std::vector<std::string> arry = fu.ScanDirectory();

                    for(auto& val : arry){
                        if(!HotJudge(val)){// 非热点文件
                            BackupInfo info = _data->GetOneByRealpath(val);
                            if(info.pack_path == "error"){// 文件存在但未备份
                                info = BackupInfo(val);
                            }
                            FileUtil tmp(val);
                            tmp.Compress(info.pack_path);
                            tmp.Remove();
                            info.pack_flag = true;
                            _data->Update(info);
                        }
                    }
                    usleep(100000);
                }
                return true;
            }
        private:
            Config* cfg;

            bool HotJudge(const std::string& filename){// 热点文件返回真
                FileUtil fu(filename);
                time_t ltime = fu.LastAtime();
                time_t ctime = time(NULL);
                if(ctime - ltime < cfg->GetHotTime()){
                    return true;
                }
                return false;
            }
    };
}

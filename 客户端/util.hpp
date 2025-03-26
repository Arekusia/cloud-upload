#pragma once
 
namespace cloud {
    namespace fs = std::experimental::filesystem;
    class FileUtil {
    public:
        FileUtil(const std::string& filename) :
            _filename(filename) {}//传入包含文件名的文件路径或者文件夹路径

        int64_t FileSize() {// 获取文件大小
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0) {
                std::cerr << "get file FileSize failed" << std::endl;
                return -1;
            }
            return st.st_size;
        }

        time_t LastMtime() {// 获取文件最后修改时间
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0) {
                std::cerr << "get file LastMtime failed" << std::endl;
                return -1;
            }
            return st.st_mtime;
        }

        time_t LastAtime() {// 获取文件最后访问时间
            struct stat st;
            if (stat(_filename.c_str(), &st) < 0) {
                std::cerr << "get file LastAtime failed" << std::endl;
                return -1;
            }
            return st.st_atime;
        }

        std::string FileName() {// 从文件路径中截取文件名
            size_t pos = _filename.rfind("\\");
            if (pos == std::string::npos) {
                return _filename;
            }
            return _filename.substr(pos + 1);
            // return fs::path(_filename).filename().string();
        }

        bool GetContent(std::string& body, size_t pos, size_t len) {// 从pos开始长度为len的文件内容写入到body
            size_t fsize = this->FileSize();
            if (pos + len > fsize) {
                std::cerr << "GetContent file size overflow" << std::endl;
                return false;
            }
            std::ifstream ifs;
            ifs.open(_filename, std::ios::binary);
            if (!ifs.is_open()) {
                std::cerr << "GetContent open file failed" << std::endl;
                return false;
            }
            ifs.seekg(pos, std::ios::beg);
            body.resize(len);
            ifs.read(&body[0], len);

            if (!ifs.good()) {
                std::cerr << "GetContent file read falied" << std::endl;
                ifs.close();
                return false;
            }
            ifs.close();
            return true;
        }

        bool GetContent(std::string& body) {// 将文件所有内容写入到body
            return GetContent(body, 0, this->FileSize());
        }

        bool SetContent(const std::string& body) {// 将body中的内容写入到文件中
            std::ofstream ofs;
            ofs.open(_filename, std::ios::binary);
            if (!ofs.is_open()) {
                std::cerr << "SetContent file open failed" << std::endl;
                return false;
            }
            ofs.write(body.c_str(), body.size());
            if (!ofs.good()) {
                std::cerr << "SetContent file write failed" << std::endl;
                ofs.close();
                return false;
            }
            ofs.close();
            return true;
        }

        bool Remove() {// 删除文件
            if (!Exists()) {
                std::cerr << "Remove file not exists" << std::endl;
                return false;
            }
            remove(_filename.c_str());
            return true;
        }

        bool Exists() {// 判断文件是否存在
            return fs::exists(_filename);
        }

        bool CreateDirectory() {// 根据路径创建文件夹
            if (Exists()) {
                return true;
            }
            return fs::create_directories(_filename);
        }

        bool ScanDirectory(std::vector<std::string>& arry) {// 遍历文件夹内的非文件夹内容
            CreateDirectory();
            for (auto& it : fs::directory_iterator(_filename)) {
                if (fs::is_directory(it.path())) {
                    continue;
                }
                arry.push_back(fs::path(it).relative_path().string());
            }
            return true;
        }

    private:
        std::string _filename;// 文件路径
    };
}

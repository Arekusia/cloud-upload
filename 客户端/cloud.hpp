#pragma once

namespace cloud {
#define SERVER_IP "8.130.20.13"
#define SERVER_PORT 8080

	class Backup {
	public:
		Backup(const string& back_dir, const string& back_file) :
			_back_dir(back_dir),
			_data(DataManager(back_file))
		{}
		string GetFileIdentifier(const string& filename) {
			FileUtil fu(filename);
			stringstream ss;
			ss <<  fu.FileName() << "-" << fu.FileSize() << "-" << fu.LastMtime();
			return ss.str();
		}

		bool UpLoad(const std::string& filename) {
			FileUtil fu(filename);
			std::string body;
			if (!fu.GetContent(body)) {
				std::cerr << "Failed to read file: " << filename << std::endl;
				return false;
			}

			httplib::Client client(SERVER_IP, SERVER_PORT);
			httplib::MultipartFormData item;
			item.content = body;
			item.filename = fu.FileName();
			item.name = "file";
			item.content_type = "application/octet-stream";
			httplib::MultipartFormDataItems items = { item };

			auto res = client.Post("/upload", items);

			if (!res || res->status != 200) {
				std::cout << res->status << endl;
				std::cerr << "Upload failed: " << (res ? res->body : "Unknown error") << std::endl;
				return false;
			}
			std::cout << "Upload success: " << filename << std::endl;
			return true;
		}

		bool NeedUpload(const string& filename) {
			string id;
			if (_data.GetOneByKey(filename, id)) {
				string newid = GetFileIdentifier(filename);
				if (newid == id) {
					return false;
				}
			}
			FileUtil fu(filename);
			if (time(NULL) - fu.LastMtime() < 5) {
				return false;
			}
			return true;
		}

		bool RunModule() {
			while(1){
				FileUtil fu(_back_dir);
				vector<string> arry;
				fu.ScanDirectory(arry);
				for (auto& val : arry) {
					if (!NeedUpload(val)) {
						continue;
					}
					if (UpLoad(val)) {
						_data.Insert(val, GetFileIdentifier(val));
					}
				}
				Sleep(1000);
			}
		}

	private:
		string  _back_dir;
		DataManager _data;
	};

}
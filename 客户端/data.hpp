#pragma once

namespace cloud {

	vector<string> split(const string& str, const string& sep) {
		int rpos = str.size();
		vector<string> res;
		regex filter("[a-zA-Z0-9]");

		while (rpos > 0) {

			while (rpos > 0 && !regex_match(string(1, str[rpos - 1]), filter)) {
				--rpos;
			}
			if (rpos == 0) {
				reverse(res.begin(), res.end());
				return res;
			}
			int lpos = str.rfind(sep, rpos - 1);
			if (lpos == string::npos) {
				res.push_back(str.substr(0, rpos));
				reverse(res.begin(), res.end());
				return res;
			}
			res.push_back(str.substr(lpos + sep.size(), rpos - lpos - sep.size()));
			rpos = lpos;

		}
		reverse(res.begin(), res.end());
	}
	 
	class DataManager {

	public:
		DataManager(const string& backup_file):
			_backup_file(backup_file)
		{
			InitLoad();
		}
		~DataManager() {
			Storage();
		}
		bool Storage() {
			stringstream ss;
			for (auto& val : _table) {
				ss << val.first << " " << val.second << endl;
			}
			cloud::FileUtil fu(_backup_file);
			fu.SetContent(ss.str());
			return true; 
		}
		bool InitLoad() {
			cloud::FileUtil fu(_backup_file);
			string body;
			fu.GetContent(body);
			vector<string> arry = split(body, "\n");
			for (auto& val : arry) {
				vector<string> tmp = split(val, " ");
				if (tmp.size() < 2) {
					continue;
				}
				_table[tmp[0]] = tmp[1];
			}
			return true;
		} 
		bool Insert(const string&  key, const string& val) {
			_table[key] = val;
			Storage();
			return true;
		}
		bool Update(const string& key, const string& val) {
			_table[key] = val;
			Storage();
			return true;
		}
		bool GetOneByKey(const string& key, string& res) {
			auto it = _table.find(key);
			if (it == _table.end()) {
				return false;
			}
			res = it->second;
			return true;
		}
	private:
		string _backup_file;
		unordered_map<string, string>_table;
	};
}
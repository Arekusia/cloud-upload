#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <experimental/filesystem>
#include<regex>
#include<sstream>
#include<unordered_map>


#define BACKUP_FILE "./backup.dat"
#define BACKUP_DIR "./backup/"


#include "httplib.h"
using namespace httplib;
#include<windows.h>
using namespace std;
#include "util.hpp"
#include "data.hpp"
#include "cloud.hpp"
using namespace cloud;

int main(int argc, const char* argv[]){

    Backup bk(BACKUP_DIR, BACKUP_FILE);
    bk.RunModule();

    int a = 10;
    cout << typeid(a).name() << endl;

    return 0;
}

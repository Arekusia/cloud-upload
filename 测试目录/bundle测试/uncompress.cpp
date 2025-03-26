#include<iostream>
#include<fstream>
#include<string>
#include"bundle.h"

int main(int argc,const char* argv[]){
    if(argc < 3){
        std::cout << argv[1] << "is zipped file" << std::endl;
        std::cout << argv[2] << "is unzipped file" << std::endl;
        return -1;
    }
    std::string ifilename = argv[1];
    std::string ofilename = argv[2];

    std::ifstream ifs;
    ifs.open(ifilename, std::ios::binary);
    ifs.seekg(0, std::ios::end);
    size_t fsize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::string body;
    body.resize(fsize);
    ifs.read(&body[0], fsize);
    ifs.close();

    std::string unpacked = bundle::unpack(body);
    std::ofstream ofs;
    ofs.open(ofilename, std::ios::binary);
    ofs.write(&unpacked[0], unpacked.size());
    ofs.close();

    return 0;
}

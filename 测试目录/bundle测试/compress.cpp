#include<iostream>
#include<string>
#include<fstream>
#include"bundle.h"

int main(int argc, char* argv[]){
    std::cout << "argv[1] is origin file path name" << std::endl;
    std::cout << "argv[2] is zipped name" << std::endl;
    
    std::string ifilename = argv[1];
    std::string ofilename = argv[2];

    std::ifstream ifs;
    ifs.open(ifilename, std::ios::binary);
    ifs.seekg(0, std::ios::end);// 跳转文件末尾
    size_t fsize = ifs.tellg();// 获取原始文件大小
    
    ifs.seekg(0, std::ios::beg);//跳转文件开头
    std::string body;
    body.resize(fsize);
    ifs.read(&body[0], fsize);// 将原始文件的数据写入body
    std::string packed = bundle::pack(bundle::LZIP, body);// 将压缩之后的数据写入packed
    
    std::ofstream ofs;
    ofs.open(ofilename, std::ios::binary);
    ofs.write(&packed[0], packed.size());// 将压缩后的数据写入到目标文件
    
    ifs.close();
    ofs.close();

    return 0;
}

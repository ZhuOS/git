// ResourceHost：资源池，用来将服务器上的文件加载到Resource对象
#ifndef RESOURCEHOST_H
#define RESOURCEHOST_H



#include <iostream>
#include <string>
#include <unordered_map>
//#include <sstream>
//#include <cstring>
//#include <dirent.h>
//function stat()
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
//ifstream
#include <fstream>

#include "Resource.h"

#define  BASE_PATH  "./htdoc"

class ResourceHost {
private:
    // 基础目录，其他的访问路径都是该目录下的相对路径
    std::string baseDiskPath;

    // 构建扩展名与MimeType对应的字典，从文件MimeTypes.inc中获取        //???
    std::unordered_map<std::string, std::string> mimeMap = {
        #define STR_PAIR(K,V) std::pair<std::string, std::string>(K,V)
        #include "MimeTypes.inc"
    };
	//std::unordered_map<std::string, std::string> mimeMap;
	string getMimeType(const string &ext);
	
public:
	
	ResourceHost(const string &basePath = BASE_PATH);
	
    // 根据URI获取Resource资源对象
    Resource* getResource(std::string uri);
	//读文件资源
	Resource* readFile(const string &path);
};


#endif
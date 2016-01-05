//Resource.cpp

#include "Resource.h"

void Resource::init()
{
	msg = NULL;
	size = 0;
	mimeType = "";
    // 资源在服务器上存储的路径
    location = "";
    // 是否是目录类型
    directory = false;
}
Resource::Resource()
{
	init();
}
Resource::Resource( const string &dir, bool isDir)
{
	msg = NULL;
	size = 0;
	mimeType = "";                   
    // 资源在服务器上存储的路径
    location = dir;
    // 是否是目录类型
    directory = isDir;
}

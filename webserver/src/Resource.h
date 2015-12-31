// Resource：HTTP响应消息中的文件资源数据
#ifndef RESOURCE_H
#define RESOURCE_H

#include<iostream>
#include<string>

using namespace std;

typedef unsigned char byte;

class Resource {
private:
    // 文件资源
    byte* data;
    // 资源大小
    unsigned int size;
    // 资源类型
    std::string mimeType;                   //??????
    // 资源在服务器上存储的路径
    std::string location;
    // 是否是目录类型
    bool directory;
	void init();
public:
	Resource();
	Resource( const string &dir, bool isDir);
	//seting
	void setData(byte* dat, unsigned int len)
	{
		data = dat;
		size = len; 
	}	
	void setMimeType(const string &mity)
	{
		mimeType = mity; 
	}
	void setLocation(const string &dir, bool isDir)
	{
		location = dir; directory = isDir; 
	}
	//fetch
	string getMimeType(){ return mimeType; }
	byte*  getData(){ return data; }
	unsigned int	   getSize(){ return size; }
	
	
	
};

#endif
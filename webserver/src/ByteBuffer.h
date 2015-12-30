//ByteBuffer.h
#ifndef BYTEBUFFER_H
#define BYTEBUFFER_H

#include<iostream>
#include<vector>
#include<string.h>

using namespace std;

typedef unsigned char byte;

class ByteBuffer{
private:
    // 读写位置索引
    vector<byte>::size_type rpos, wpos;
    // 缓存内容使用容器存储
    vector<byte> buff;
    // 清空队列并重置读写索引
public:
	ByteBuffer();
	void clear();
    // 返回存储空间大小
    unsigned int size(); 
    // 读取队列头部的数据同时移动rpos
    byte get();
	byte getBytes(unsigned int index);
	//
	unsigned int getrpos(){return rpos;}
	void	setrpos(unsigned int rp){ rpos = rp; }
	unsigned int getwpos(){return wpos;}
	void	setwpos(unsigned int wp){ wpos = wp; }
    // 读取队列头部指定长度的数据到buf中
    void getBytes(byte* buf, unsigned int len);
	void getBytes(byte* buf, unsigned int start_pos, unsigned int len);
	//读取buff中所有数据
	void getBytes(byte* buf);
	//末尾加入\r\n
	void putcrlf();
    // 将b写入队列同时移动wpos
    void put(byte b);
	void putBytes(byte* b, unsigned int len);
    // 将长度为len的缓存b写入队列指定位置
    void putBytes(byte* b, unsigned int len, unsigned int index); 
	
	//获取第一个plit分隔前的Bytes
	string getFirstPart(const string &plit,  int len,  int bpos );
	//获取第一个plit分隔的下标
	int my_find_first_of(const string &strBytes,  int len,  int bpos);
	
	
};
    
#endif

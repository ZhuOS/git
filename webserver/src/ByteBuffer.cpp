//ByteBuffer.cpp

#include "ByteBuffer.h"
/*
class ByteBuffer{
    // 读写位置索引
    unsigned int rpos, wpos;
    // 缓存内容使用容器存储
    std::vector<byte> buf;
    // 清空队列并重置读写索引
    void clear();
    // 返回存储空间大小
    unsigned int size(); 
   // 读取队列头部的数据同时移动rpos
    byte get();
    // 读取队列头部指定长度的数据到buf中
    void getBytes(byte* buf, unsigned int len); 
    
    // 将b写入队列同时移动wpos
    void put(byte b);
    // 将长度为len的缓存b写入队列指定位置
    void putBytes(byte* b, unsigned int len, unsigned int index); 
};
*/
ByteBuffer::ByteBuffer()
{
	rpos=wpos=0;
}
void ByteBuffer::clear()
{
    rpos = 0;
    wpos = 0;
    buff.clear();
}
unsigned int ByteBuffer::size()
{
    return buff.size();
}

byte ByteBuffer::get()
{
    return buff[rpos];
}

byte ByteBuffer::getBytes(unsigned int index)
{
	if(index>=size())
		return ' ';
	return buff[index];
	
}
void ByteBuffer::getBytes(byte* buf)
{
	for(unsigned int i = 0; i<size(); i++){
		buf[i] = buff[i];
	}
}

void ByteBuffer::getBytes(byte* buf, unsigned int len)
{
    for(unsigned int i = 0; i<len; i++)
        buf[i] = buff[i];
}
void ByteBuffer::getBytes(byte* buf, unsigned int start_pos, unsigned int len)
{
	for(unsigned int i = start_pos; i<start_pos+len; i++)
        buf[i] = buff[i];
}
void ByteBuffer::put(byte b)
{
    buff[wpos++] = b;
}
	//末尾加入\r\n
void ByteBuffer::putcrlf()
{ 
	string str = "\r\n"; 
	putBytes( (byte*)&str,2);
}

void ByteBuffer::putBytes(byte* b, unsigned int len)
{
	cout<<"1 write position: "<<wpos<<endl;
	buff.resize(len);
	memcpy(&buff[wpos],b,len);
	wpos+=len;
	cout<<"2 write position: "<<wpos<<endl;
}
void ByteBuffer::putBytes(byte* b, unsigned int len, unsigned int index)
{
    for(unsigned int i = 0; i<len; i++)
        buff[index+i] = *b++;
	wpos+=len;
}

string ByteBuffer::getFirstPart(const string &plit,  int len,  int bpos )
{

    unsigned int epos = my_find_first_of(plit, plit.size(), bpos);
	//cout<<"getFirstPart "<<"find rpos: "<<bpos<<endl;
	cout<<"string size: "<<bpos<<"->"<<epos<<"="<<endl;
	unsigned int size = epos - bpos;
	byte *strBytes = new byte[size];

	getBytes(strBytes, bpos, size);		//#######出错,

	//strBytes[size-1] = '\0';
	//cout<<"strBytes:"<<strBytes<<endl;

	setrpos(epos+len);					//移动rpos	

	//byte* to string
	char *chr = new char[size];
	sprintf(chr,"%s",strBytes);
	string ss(chr);
	ss.resize(size);
	cout<<"ss:"<<ss<<endl;
	return ss;					//#######出错,解决
}


//查找vecByte中第一次出现pByte序列的下标
int ByteBuffer::my_find_first_of(const string &strBytes,  int len,  int bpos)
{
	if(len == 1)
		for(unsigned int i = bpos; i<size(); i++){
			if(getBytes(i)==strBytes[0])
				return i;
		}
	if(len == 2)
		for( unsigned int i = bpos; i<size(); i++){
			if(getBytes(i)==strBytes[0]&&getBytes(i+1)==strBytes[1])
				return i;
		}
	if(len == 4)
		for(unsigned int i = bpos; i<size(); i++){
			if(	getBytes(i)==strBytes[0]
				&&getBytes(i+1)==strBytes[1]
				&&getBytes(i+2)==strBytes[2]
				&&getBytes(i+3)==strBytes[3])
				return i;
		}
	return -1;
}







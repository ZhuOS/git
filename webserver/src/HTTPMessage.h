//HTTPMessage.h
#ifndef HTTPMESSAGE_H
#define HTTPMESSAGE_H


#include<map>
#include<string>
#include "ByteBuffer.h"

#define		HTTP_VERSION	"HTTP/1.1"
#define	    METHOD_NUM      9
enum Method{
    GET = 0,
    HEAD,
    OPTIONS,
    POST,
	PUT,
	DEL,
	TRACE,
	CONNECT,
	PATCH
};

const static char* const MethodString[METHOD_NUM]={
					"GET",
					"HEAD",
					"OPTIONS",
					"POST",
					"PUT",
					"DEL",
					"TRACE",
					"CONNECT",
					"PATCH"  };
// HTTP回应状态码
enum Status {
    // 1xx 消息类型
    CONTINUE = 100,
    
    // 2xx 成功
    OK = 200,
    
    // 3xx 重定向
    
    // 4xx 客户端请求错误
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    
    // 5xx 服务器错误
    SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501
};

class HTTPMessage : public ByteBuffer {
private:
	int datalen;
protected:
    // HTTP版本号
    std::string version;
    
    // 消息报头
    std::map<std::string, std::string> *headers;            
    // 消息数据
    // 响应消息中表示资源，请求消息中表示额外的参数
    byte* data;

public:
	HTTPMessage();
	HTTPMessage(const string &ver, map<std::string, std::string> *phed, const string &dat);
	//设置
	void setVersion(const string &ver){ version = ver; }
	void setHeaders( map<string,string> *phed){ headers = phed; }
	void setData( const string &dat ){ data = (byte*)dat.c_str(); }
	void setData( byte* dat ){ data = dat; }
	
	//获取信息
	string getVersion(){ return version; }
	map<string, string>* getHeaders(){ return headers; }
	byte* getData(){ return data; }
	
	//buff加入请求行信息
	virtual void putLine()=0;
	//buff加入消息报头信息
	void putHeaders();
	//buff加入请求正文信息
	void putData();
	
    // 创建请求数据包消息
	virtual byte* create()=0;
    // 解析消息
    virtual int parse()=0;
	int parseHeaders();
	
	void disHeadData();
};

#endif

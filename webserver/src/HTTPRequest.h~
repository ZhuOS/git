// HTTPRequest.h：HTTP请求消息，从客户端发给服务器的消息
#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include "HTTPMessage.h"

// HTTP请求方法




class HTTPRequest : public HTTPMessage {
private:

    // 请求的方法类型
    int method;

    // 请求的URL
    std::string requestUri;

protected:

    // 初始化消息
    void init();

public:
	HTTPRequest();
	HTTPRequest(byte *buf, unsigned int len);
	HTTPRequest(const string &ver, map<string,string> *phed, const string &dat, int mid, const string &rq_uri);
	
	//设置
	void setMethod(int mid){ method = mid; }
	void setReqUri(const string &rpuri){ requestUri = rpuri; }
	
	//获取信息
	int  getMethod() { return method; }
	string getReqUri(){ return requestUri; }
	
	//buff加入请求行信息
	void putLine();
	//创建数据包
	byte* create();
	//解析函数
	int parse();
    // 方法字符串与数字相互转换
	string methodInt2Str(unsigned int mid);
	int methodStr2Int( const string &mstr );
	// 显示请求信息
	void displayMessage();
};
#endif

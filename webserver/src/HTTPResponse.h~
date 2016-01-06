// HTTPResponse：HTTP响应消息，从服务器发给客户端的消息
#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include "HTTPMessage.h"



class HTTPResponse : public HTTPMessage {
private:    
    // 响应状态码和信息
    int statusCode;
    std::string reason;
protected:
    void init();
	
public:
	//Constructor
	HTTPResponse();
	HTTPResponse(byte* buf, unsigned int len);
	HTTPResponse(const string &ver, int sc, const string &rea, map<string, string>*phed, const string &dat);
	
	//setting
	void setStatusCode(int stcode){ statusCode = stcode; }
	void setReason(const string &rea){ reason = rea; }
	void insertHeaders( const string &key, const string &mapped );
	
	
	//fetch
	int getStatusCode(){ return statusCode; }
	string getReason(){ return reason; }
	
	//buff加入请求行信息
	void putLine();
	//创建数据包
	byte* create();
	//解析函数
	int parse();
	
	//statusCode 整型/字符串 转换
	string statusCodeInt2Str(int scInt);
	int	statusCodeStr2Int(const string &strSC);
	//display message
	void displayMessage();
	
};


#endif

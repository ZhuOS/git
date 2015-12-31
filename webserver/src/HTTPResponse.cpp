//HTTPResponse.cpp
#include "HTTPResponse.h"

void HTTPResponse::init()
{
	statusCode = 100;
	reason = "CONTINUE";
}
	
//Constructor
HTTPResponse::HTTPResponse():HTTPMessage()
{
	init();
}
HTTPResponse::HTTPResponse(byte* buf, unsigned int len):HTTPMessage()
{
	init();
	putBytes(buf,len);
}
HTTPResponse::HTTPResponse(	const string &ver, int sc, const string &rea, 
							map<string, string>*phed, const string &dat):HTTPMessage(ver,phed,dat){
	statusCode = sc;
	reason = rea;
}

string HTTPResponse::statusCodeInt2Str(int scInt)
{
	switch(scInt)
	{
		case 100:
			return "100";
			break;
		case 200:
			return "200";
			break;
		case 400:
			return "400";
			break;
		case 404:
			return "404";
			break;
		case 500:
			return "500";
			break;
		case 501:
			return "501";
			break;
		default:
			perror("Status-Code Int Not Found");
			return "000";
			break;
	}
}	
int HTTPResponse::statusCodeStr2Int(const string &strSC)
{
		if(strSC == "CONTINUE" )
			return 100;
		else if( strSC == "OK" )
			return 200;
		else if( strSC == "BAD_REQUEST" )
			return 400;
		else if( strSC == "NOT_FOUND" )
			return 404;
		else if( strSC == "SERVER_ERROR" )
			return 500;
		else if( strSC == "NOT_IMPLEMENTED" )
			return 501;
		else{
			perror("Status-Code String Not Found");
			return -1;
		}
}
//buff加入请求行信息
void HTTPResponse::putLine()
{
	string strSC = statusCodeInt2Str(statusCode);
	string strLine = version+" "+strSC+" "+reason+"\r\n";
	putBytes((byte*)strLine.c_str(),strLine.size());
	
}
//创建数据包
byte* HTTPResponse::create()
{	
	
	cout<<"putline"<<endl;
	putLine();
	
	cout<<"putheaders"<<endl;
	putHeaders();		//####################出错了
	cout<<"putdata"<<endl;
	putData();
	cout<<"get msg"<<endl;
	byte* ReqMsg = new byte[size()];
	getBytes(ReqMsg);
	return ReqMsg;
}
//解析函数
int HTTPResponse::parse()
{
	string strPlitSpace(" ");
	//解析HTTP-version
	version = getFirstPart(strPlitSpace,strPlitSpace.size(),getrpos());
	//解析status-Code
	string strSC = getFirstPart(strPlitSpace,strPlitSpace.size(),getrpos());
	if( (statusCode=statusCodeStr2Int(strSC))==-1 ){
		perror("Status-Code String Not Found");
		return 0;
	}
	//解析reason-Phrase
	string strPlitCRLF("\r\n");
	reason = getFirstPart(strPlitCRLF,strPlitCRLF.size(),getrpos());
	//解析Headers
	if( !parseHeaders() )
		return 0;
	//解析正文消息 data
	if(getrpos()<size())
		getBytes(data,size()-1-getrpos());
	return 1;
}	
void HTTPResponse::insertHeaders( const string &key, const string &mapped )
{
	headers->insert(make_pair(key, mapped));
}
	
void HTTPResponse::displayMessage()
{
	
	cout<<"dispaly response"<<endl<<getVersion()<<" "<<statusCodeInt2Str(getStatusCode())<<" "<<getReason()<<endl;
	disHeadData();
	
}
	
	
	
	
	
	
	
	
	
	
	
	
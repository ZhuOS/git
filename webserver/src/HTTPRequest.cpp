//HTTPRequest.cpp

#include<iostream>
#include<string>
#include "HTTPRequest.h"
using namespace std;


HTTPRequest::HTTPRequest():HTTPMessage()
{
	init();
}

//根据接收到的数据创建请求消息
HTTPRequest::HTTPRequest(byte *buf,unsigned int len):HTTPMessage()
{
	init();
	
	putBytes(buf, len);//########出错了,解决
	//cout<<"HTTPRequest(byte*,int)"<<endl<<buf<<endl; //test
}

HTTPRequest::HTTPRequest(const string &ver, map<string,string> *phed, const string &dat, 
						 int mid, const string &rq_uri):HTTPMessage(ver, phed, dat){
	method = mid;
	requestUri = rq_uri;
}
	
void HTTPRequest::init()
{
    //HTTPRequest
	requestUri = "";
	method = OPTIONS;
}
//method 整型转字符串


string HTTPRequest::methodInt2Str( int mid )
{
	if( mid<0 || METHOD_NUM<mid )
		return "";
	
	return MethodString[mid];								//感觉，有问题
}
int HTTPRequest::methodStr2Int( const string &mstr )
{	
//	cout<<"	methodStr2Int: "<<mstr<<endl;
	for(int i = 0; i<METHOD_NUM; i++){
		if( 0 ==strcmp( MethodString[i],mstr.c_str() ) ){
//			cout<<"	methodStr2Int: "<<i<<endl;				//test		
			return i;
		}
	}
	perror("method error");
	return -1;
}

//请求行加入buff
void HTTPRequest::putLine()
{
	string methStr = methodInt2Str(method);
	string strLine = methStr+" "+requestUri+" "+version+"\r\n";
	putBytes((byte*)strLine.c_str(), strLine.size());
}
//创建请求消息
byte* HTTPRequest::create()
{
	putLine();
	putHeaders();
	putData();
	byte* ReqMsg = new byte[size()];
	getBytes(ReqMsg);
	return ReqMsg;
}

//请求数据包 解析函数
int HTTPRequest::parse()
{
	//cout<<"Request parse"<<endl;
	
	string strPlitSpace(" ");
	string strMeth = getFirstPart(strPlitSpace, strPlitSpace.size(), getrpos());//########出错了
	
	//解析 method
	if( (method = methodStr2Int(strMeth))== -1 ){
		perror("	get method error");
		return 0;
	}
	//cout<<"method: "<<method<<endl;					//test
	//解析request-URI
	requestUri = getFirstPart(strPlitSpace, strPlitSpace.size(), getrpos());
	//cout<<"Request-Uri: "<<requestUri<<endl;			//test
	//解析HTTP-version
	string strPlitCRLF("\r\n");
	version = getFirstPart(strPlitCRLF,strPlitCRLF.size(),getrpos());
	//cout<<"Version: "<<version<<endl;					//test
	//解析Headers
	if( !parseHeaders() ){
		perror("	parseHeaders error");
		return 0;
	}
	//解析正文消息 data
	//cout<<"##parse data:"<<getrpos()<<"="<<size()<<endl;//test
	if(getrpos()<size()){
		//cout<<"	have data\n";							//test
		getBytes(data,size()-getrpos());
		setData(data,size()-getrpos());
	}else{
		;//cout<<"	no data\n";				//test
	}	
	//cout<<"Request parse success"<<endl;					//test
	return 1;
}
// 显示请求信息
void HTTPRequest::displayMessage()
{
	cout<<methodInt2Str(method)<<" "<<requestUri<<" "<<version<<endl;
	disHeadData();							//########出错了,solved
	//cout<<"request display success"<<endl;//test
}




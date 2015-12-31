//HTTPMessage.cpp
#include "HTTPMessage.h"

HTTPMessage::HTTPMessage()
{
	version = HTTP_VERSION;
	headers = new map<string,string>();
	data = NULL;
	datalen = 0;
}
HTTPMessage::HTTPMessage(const string &ver, map<std::string, std::string> *phed, const 				string &dat)
{
	version = ver;
	headers = phed;
	data = (byte*)dat.c_str();
	datalen = dat.size();
}
// 创建消息
void HTTPMessage::putHeaders()//#########出错了
{	
	cout<<"putHeaders: size "<<headers->size()<<endl;			//test
	map<string,string>::const_iterator it;
	for(it = headers->begin(); it != headers->end(); it++){
		string temp = it->first+":"+it->second+"\r\n";
		cout<<"	temp "<<temp<<endl;					//test		
		putBytes((byte*)temp.c_str(),temp.size());
	}
	cout<<"putcrlf"<<endl;
	putcrlf();
	cout<<"putHeaders: exit"<<endl;//test
}

void HTTPMessage::putData()
{
	putBytes(data, datalen);
}

//解析Headers函数
int HTTPMessage::parseHeaders()
{
	unsigned int epos = my_find_first_of("\r\n\r\n", 4, getrpos()); 
	if(epos == (unsigned int)-1){				
		perror("headers error");
		return 0;
	}

	while(getrpos()<epos){											//构造headers
		string strFirst = getFirstPart(":",1,getrpos());
		string strSecond = getFirstPart("\r\n",2,getrpos());
		headers->insert( make_pair(strFirst,strSecond) );
	}
	cout<<"	parseHeaders test: epos="<<epos<<"getrpos()="<<getrpos()<<endl;///test
	setrpos(epos+4);
	cout<<"	getpos()="<<getrpos()<<endl;//test
	return 1;
}
void HTTPMessage::disHeadData()
{
	//显示报头
	//cout<<"<<disHeadData: "<<endl;//test
	for(map<string,string>::iterator it = headers->begin(); it != headers->end(); it++)
		cout<<it->first<<":"<<it->second<<endl;
	cout<<endl;
	//显示正文
	//cout<<"data:"<<datalen<<endl;				//########data显示出错
	if( datalen>0 ){
		char* cc = new char[datalen];
		string str;
		sprintf(cc, "%s", data);
		str = cc;
		str.resize(datalen);
		cout<<str<<endl;
	}
	cout<<"disHeadData success>>"<<endl;//test
}






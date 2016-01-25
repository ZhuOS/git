//HTTPServer.cpp

#include"HTTPServer.h"
//readFile error 有错误
HTTPServer::HTTPServer()
{
	En = false;
	listenSocket = INVALID_SOCKET;

	// Create a resource host serving the base path ./htdocs on disk
    ResourceHost* resHost = new ResourceHost("./htdocs");
	hostList.push_back(resHost);

	// Setup the resource host serving htdocs to provide for the following vhosts:
	// <HostName:ResourceHost*>
	vhosts.insert(std::pair<std::string, ResourceHost*>("localhost:8080", resHost));
	vhosts.insert(std::pair<std::string, ResourceHost*>("127.0.0.1:8080", resHost));
	vhosts.insert(std::pair<std::string, ResourceHost*>("218.192.168.85:8080", resHost));
}

bool HTTPServer::start(int port)
{
	En = false;
	//step 1 初始化
	//server 地址信息
	listenPort = port;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(listenPort);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
	//server 创建描述符
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
    if( serverSocket < 0 ){
        perror("#socket error");
        return false;
    }

	// 设置socket为非阻塞
	fcntl(serverSocket, F_SETFL, O_NONBLOCK);

	//step 2：绑定
	//解决方法是使用setsockopt,使得socket可以被重用，是最常用的服务器编程要点。
	int opt = 1;
   	setsockopt(serverSocket,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	if(bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
        perror("#bind error");
        return false;
    }

	// Step 3：监听
    int ret = listen(serverSocket,5);
    if(ret < 0){
        perror("#listen error");
        return false;
    }
    // Step 4：创建kqueue
	kqfd = kqueue();
    // Step 5：添加serverSocket到kqfd中
	Register(kqfd,serverSocket);
	En = true;
	return true;
}

/*******************kqueue***********************/
//注册事件函数
bool HTTPServer::Register(int kq, int fd)
{
    struct kevent changes[1];
    EV_SET(&changes[0], fd, EVFILT_READ, EV_ADD, 0, 0, NULL); //初始化changes
    kevent(kq, changes, 1, NULL, 0, NULL);
	/*if( !ret ){
		perror("Register error");
		return false;
	}*/
	return true;
}



//
void HTTPServer::stop()
{
	En = false;
	if( serverSocket != INVALID_SOCKET ){
		//disconnect all clients
		for(unordered_map<int,Client*>::const_iterator it = clientMap.begin();
			it != clientMap.end(); it++ ){
			disconnet(it->second);
		}
		//remove serverSocket from kqueue
		updateKqueue(serverSocket,EVFILT_READ, EV_DELETE,0,0,NULL);
		//close kqueue
		if(kqfd!=-1){
			close(kqfd);
			kqfd = -1;
		}
		//close serverSocket
		close(serverSocket);
	}
	cout<<":Server Closed!"<<endl;
}
//更新kqueue
bool HTTPServer::updateKqueue(	int ident, short filter, u_short flags,
					u_int fflags, int data, void *udata)
{
	struct kevent kev;
	EV_SET(&kev, ident, filter, flags, fflags, data, udata);
	kevent(kqfd, &kev, 1, NULL, 0, NULL);
	//if( ret ){
	//	cout<<"#Update Kqueue error,ret="<<ret<<endl;
	//	return false;
	//}
	return true;
}
void HTTPServer::process()
{
	// Step 6：kqueue等待就绪事件
	//cout<<"start process"<<endl;
	int ret = 0;
	while(En){
		cout<<":waiting..."<<endl;
		ret = kevent(kqfd, NULL, 0, evList, QUEUE_SIZE, NULL); //等待就绪事件
		cout<<":"<<ret<<" events occur."<<endl;
		if(ret <= 0){
			perror("#kqueue wait error");
			continue;
		}
		HandleEvent(kqfd, ret); 		//事件处理
	}
}
//事件处理函数
void HTTPServer::HandleEvent(int kq, int nevents)
{
    for (int i = 0; i < nevents; i++)
    {
		int sockfd = evList[i].ident;
        int data = evList[i].data;
        if(sockfd == serverSocket)	//新连接
            Accept(kq, data);
        else 						//新消息到来
			Receive(sockfd, i);
    }
}

/***********************socket handle************************************/
//新连接函数
void HTTPServer::Accept(int kq, int connSize)
{
    for (int i = 0; i < connSize; i++)
    {
		struct sockaddr_in clientAddr;
		socklen_t clientAddrLength = sizeof(struct sockaddr_in);
		int client_fd = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLength);

    	if(client_fd == INVALID_SOCKET)
        	return;

		fcntl(client_fd, F_SETFL, O_NONBLOCK);

    	// 创建客户端对象
		Client *clt = new Client(client_fd,clientAddr);
    
		// 添加客户端socket到kqueue中，读取和写入，注意写入初始状态是关闭
		updateKqueue(client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		updateKqueue(client_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL); 
	    
		// 添加客户端对象到字典	
		clientMap.insert( make_pair(client_fd,clt) );

		cout<<"["<<inet_ntoa(clientAddr.sin_addr)<<"/"
			<<ntohs(clientAddr.sin_port)<<"/"<<client_fd
			<<"] connect("<<clientMap.size()<<" connected)"<<endl;

		//cout<<"add client to clientMap"<<endl;
    }
}
//接收数据
void HTTPServer::Receive(int sockfd, int index)
{
	//cout<<"clientfd:"<<sockfd<<" Receive data:"<<availBytes<<endl;//for test
	Client *clt = getClient(sockfd);
	if(clt == NULL) {
		std::cout << "Could not find client" << std::endl;
		return;
	}
	// 客户端要断开连接
	if(evList[index].flags & EV_EOF) {
		cout<<"["<<clt->getIP()<<"/"<<clt->getPort()<<"/"<<sockfd<<"] exit"
			<<"("<<clientMap.size()-1<<" connected)"<<endl;
		disconnet(clt);
		return;
	}
	if(evList[index].filter == EVFILT_READ){
		cout<<": read  client "<<endl;	
		//接收数据				
		readClient( clt, evList[index].data );
		// 设置读取事件处理完成，触发写入事件
		updateKqueue(evList[index].ident, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
		updateKqueue(evList[index].ident, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
	}else if(evList[index].filter == EVFILT_WRITE){
		cout<<": write client "<<endl;	
		if(!writeClient(clt, evList[index].data)){
			// 设置读取事件处理完成，触发写入事件
			cout<<": write client end"<<endl;
			updateKqueue(evList[index].ident, EVFILT_READ, EV_ENABLE, 0, 0, NULL);
			updateKqueue(evList[index].ident, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
		}
	}
}
//根据描述符号，获取Client类
Client* HTTPServer::getClient(int clfd) 
{
	std::unordered_map<int, Client*>::const_iterator it;
    it = clientMap.find(clfd);
	
	if(it == clientMap.end()){
		cout<<"#Client not exist"<<endl;
		return NULL;
	}
	//cout<<"getClient"<<endl;
    return it->second;
}
//读取消息到Client中
void HTTPServer::readClient(Client* pclt, int msgLen)
{    
	// data_len为0时有可能是客户端要断开连接，先设置 data_len 为以太网默认最大的MTU值
    if(msgLen <= 0)
    	msgLen = 1400;
    
	char *buffRecv = new char[msgLen];
	int n = recv(pclt->getsocketDesc(), buffRecv, msgLen, 0);
	if( n<0 ){			//出错
		perror("#receive message error");
		disconnet(pclt);
		return;
	}else if(n == 0){	//连接已经断开
		cout<<"["<<pclt->getIP()<<"/"<<pclt->getPort()<<"/"<<pclt->getsocketDesc()<<"] exit"
			<<"("<<clientMap.size()-1<<" connected)"<<endl;
		disconnet(pclt);
	}else{				//处理接收的数据包
		//cout<<"readClient"<<endl;
		HandleMsg(pclt,buffRecv, n);
		//###############################################到这里了

	}
	delete [] buffRecv;
}
bool HTTPServer::writeClient(Client* pclt, int msgLen) {
	//cout<<"****************"<<msgLen<<endl;
	int actual_sent = 0; // 实际发送给客户端的数据
	int attempt_sent = 0; // 尝试发送的数据
	int remaining = 0; // 等待发送的数据
	bool disconnect = false;
	byte* pData = NULL;

	// 调整要发送的数据大小
	if(msgLen > 1400) {
		// 设置为最大的MTU值
		msgLen = 1400;
	} else if(msgLen == 0) {
		// 有些操作系统显示为0但仍然会有数据发出
		msgLen = 128;
	} else if(msgLen < 0) {
		cout<<"# Write Client Error"<<endl;
	}

    // 获取发送队列里的元素
	SendQueueItem* item = pclt->frontOfSendQueue();

	if(item == NULL){
		cout<<": item == NULL"<<endl;
		return false;
	}

	//cout<<"1Size:"<<item->getSize()<<endl;
	//cout<<"1Offset:"<<item->getOffset()<<endl;

	
	pData = item->getData();
	disconnect = item->getDisConnect();
	remaining = item->getSize() - item->getOffset();
	
	if(msgLen >= remaining) {
		// 如果发送缓存大于需要发送的数据量，则将剩余数据都发出
		attempt_sent = remaining;
	} else {
		// 如果发送缓存小于需要发送的数据量，则发出发送缓存大小的数据
		attempt_sent = msgLen;
	}

	// 发送数据并根据实际发送的数据调整偏移
	actual_sent = send(pclt->getsocketDesc(), pData+(item->getOffset()), attempt_sent, 0);
	//cout<<"actual_sent:"<<actual_sent<<endl;
	if(actual_sent >= 0)
		item->setOffset(item->getOffset() + actual_sent);
	else{
		perror("# Write Client send message error!");
		return false;
	}
	//cout<<"2Size:"<<item->getSize()<<endl;
	//cout<<"2Offset:"<<item->getOffset()<<endl;
	// 如果偏移量大于了实际的数据大小，
    // 表示已经发送完毕不再需要这个SendQueueItem了则出队操作
	if(item->getOffset() >= item->getSize())
		pclt->popOfSendQueue();
	
	if(disconnect) {
		disconnet(pclt);
		cout<<" disconnet client"<<endl;
		return false;
	}
	//cout<<"======================="<<endl;
	return true;
}
//Client断开连接
bool HTTPServer::disconnet(Client* clt)
{	
	//cout<<":disconnet"<<endl;
	if(clt == NULL){
		return false;
	}
	//kqueue中清除
	if(!updateKqueue(clt->getsocketDesc(),EVFILT_READ, EV_DELETE, 0, 0, NULL)){
		perror("#disconnect error");
		return false;
	}
	//clientMap中清除
	clientMap.erase(clt->getsocketDesc());
	//关闭socket
	close(clt->getsocketDesc());
	//释放内存
	delete clt;
	return true;
}
/***********************Receive Message Handle********************/
//message handling function
bool HTTPServer::HandleMsg(Client *clt, char* message, int len)
{
	//cout<<"HandleMsg:"<<endl;
	//message[len]='\0';
	//cout<<message<<":"<<len<<endl;
	
	HTTPRequest* req = new HTTPRequest( (byte*)message, len );//########出错了，解决
	//cout<<"mark"<<endl;
	if( !req->parse() ){				//########出错了,解决
		perror("#request parse error");
		return false;
	}
	else{
		//显示请求消息
		//cout<<"##receive message: \n";
		//cout<<req->getMethod()<<req->getReqUri()<<endl;//test
		//cout<<"Request message:"<<endl;
		
		//req->displayMessage();			//########出错了,solved
		
		int method = req->getMethod();
		//cout<<"Method method= "<<method<<endl;

		cout<<"["<<clt->getIP()<<"/"<<clt->getPort()<<"/"
		<<clt->getsocketDesc()<<"] ";
		switch(method){
			case HEAD:					//方法HEAD
				cout<<"HEAD Request"<<endl;
			case GET:
				cout<<"GET Request"<<endl; 
				handleGet(clt,req);			//方法GET
				break;
			case OPTIONS:					//方法OPTIONS
				cout<<"OPTIONS Request"<<endl; 
				handleOptions(clt,req);
				break;
			case POST:
				cout<<"POST Request"<<endl;
				break;
			case PUT:
				cout<<"PUT Request"<<endl;
				break;
			case DEL:
				cout<<"DEL Request"<<endl;
				break;
			case TRACE:
				cout<<"TRACE Request"<<endl;
				break;
			case CONNECT:
				cout<<"CONNECT Request"<<endl;
				break;
			case PATCH:
				cout<<"PATCH Request"<<endl;
				break;
			default:
				cout<<"\n#error Request!"<<endl;
				break;
		}
		//delete req;
		return true;
	}
}

//GET 请求获取Request-URI所标识的资源
//HEAD    请求获取由Request-URI所标识的资源的响应消息报头
void HTTPServer::handleGet(Client* clt, HTTPRequest* req)
{
	//cout<<"handleGet()"<<endl;//test
	// 查找报头中Host对应的HostName
	string hostName = "";
    std::map<std::string, std::string> *heds = req->getHeaders();
    string hostStr("Host");
	map<string,string>::const_iterator itHost = heds->find(hostStr);
	hostName = itHost->second;
	// 查找HostName对应的虚拟主机vhost
	ResourceHost *resHost;
	unordered_map<string,ResourceHost*>::const_iterator itVhost = vhosts.find(hostName);
	if( itVhost == vhosts.end() ){ 	//not find virtual host
		if(hostList.size()>0)
			resHost = hostList[0];	//use the default virtual host
	}
	else
		resHost = itVhost->second;

	// ResourceHost 无法找到时返回BAD_REQUEST
	if(resHost == NULL) {
		cout<<"#Invalid/No Host specified: "<<hostName<<endl;
		return;
	}
	// get the Request-Uri
	string uri = req->getReqUri();

	// conduct HTTPResponse
	HTTPResponse* resp=new HTTPResponse();//#####################################
	resp->setVersion(HTTP_VERSION);
	resp->setStatusCode(Status(OK));
	resp->setReason("OK");
//	resp->insertHeaders("Server","ZhuOS/1.1");

	// get resource
	Resource *res = resHost->getResource(uri);


	std::stringstream sz;
	sz << res->getSize();					//##出错了,solved
	
	cout<<"#resource size: "<<sz.str()<<endl;		//test

	resp->insertHeaders("Content-Length",sz.str());
	resp->insertHeaders("Content-Type",res->getMimeType());	//####出错了,solved
	
	// 只为GET方法返回的数据
	if(req->getMethod() == Method(GET))
		resp->setData(res->getData(),res->getSize());			//有错,solved

	//cout<<"create"<<endl;						//test	
	//resp->displayMessage();					//test
	//resp->create();							//##出错了,检查
	
	// 判断通信完后，客户端是否断开
	bool dc = false;
	map<string,string>::const_iterator it = heds->find("Connection");
	if(it != heds->end()){	
		if(it->second.compare("close") == 0)
			dc = true;
	}
	
	sendResponse(clt, resp, dc);
	//delete resp;
	//delete res;
	//delete resp;
	//cout<<"handleGet() success"<<endl;//test
}
//OPTIONS 请求查询服务器的性能，或者查询与资源相关的选项和需求
void HTTPServer::handleOptions(Client* clt, HTTPRequest* req)
{
	// conduct HTTPResponse
	string ServerInfo = "CPU-kurui999";
	HTTPResponse* resp = new HTTPResponse();
	resp->insertHeaders("Content-Length","0");

	resp->setStatusCode( Status(OK) );
	resp->setReason( "OK" );
	resp->insertHeaders( "Allow",ServerInfo );
	resp->insertHeaders( "Server","ZhuOS/1.1" );	
	resp->create();
	// display Response-Message 
	//cout<<"Response OPTIONS message"<<endl; 
	//resp->displayMessage();
	// send Response-Message 
	// 判断通信完后，客户端是否断开
    std::map<std::string, std::string> *heds = req->getHeaders();
	bool dc = false;
	map<string,string>::const_iterator it = heds->find("Connection");
	if(it != heds->end()){	
		if(it->second.compare("close") == 0)
			dc = true;
	}
	sendResponse(clt, resp, dc);
	//delete resp;
}

/*********************Send Message Handle****************************/
void HTTPServer::sendResponse(Client* clt, HTTPResponse* resp, bool dc)
{

	// 服务器头部
	resp->insertHeaders("Server","shiyanlouserver/1.0");

	// 响应的时间信息
	std::string tstr;
	char tbuf[36];
	time_t rawtime;
	struct tm* ptm;
	time(&rawtime);
	ptm = gmtime(&rawtime);
	// Ex: Fri, 31 Dec 1999 23:59:59 GMT
	strftime(tbuf, 36, "%a, %d %b %Y %H:%M:%S GMT", ptm);
	tstr = tbuf;
	resp->insertHeaders("Date", tstr);

	// 是否需要在头部中添加断开连接的信息
	if(dc)
		resp->insertHeaders("Connection", "close");
	
	// 创建响应数据
	byte* pData = resp->create();

	cout<<"#resp.size() "<<resp->size()<<endl;			//for test
	pData[resp->size()] = '\0';
	cout<<"#data: "<<pData<<endl; 

	//byte* buffSend = new byte[resp->size()];
	//resp->getBytes( buffSend );

	// 将数据添加到客户端的发送队列
	clt->addToSendQueue(new SendQueueItem(pData, resp->size(), dc)); 




	/*	
	// get the sending data
	//cout<<"sendResponse message"<<endl;
	char* buffSend = new char[resp->size()];
	resp->getBytes( (byte*)buffSend );
	
	//send message
	if( send(clt->getsocketDesc(), buffSend, resp->size(), 0)<0 ){
		perror("#send Responsing-Message error!");
		return;
	}
	//cout<<"sendResponse success"<<endl;
	//delete buffSend;
	*/
}


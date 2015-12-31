//HTTPServer.cpp

#include"HTTPServer.h"

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
        perror("listen error");
        return false;
    }
    // Step 4：创建kqueue
	kqfd = kqueue();
    // Step 5：添加serverSocket到kqfd中
	Register(kqfd,serverSocket);
	En = true;
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
	cout<<"Server closed!"<<endl;
}

void HTTPServer::process()
{
	// Step 6：kqueue等待就绪事件
	cout<<"start process"<<endl;
	int ret = 0;
	while(En){
		ret = kevent(kqfd, NULL, 0, evList, QUEUE_SIZE, NULL); //等待就绪事件
		if(ret < 0){
			perror("kqueue wait error");
			continue;
		}
		HandleEvent(kqfd, evList, ret); 		//事件处理
	}
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
//更新kqueue
bool HTTPServer::updateKqueue(	int ident, short filter, u_short flags,
					u_int fflags, int data, void *udata)
{
	struct kevent kev;
	EV_SET(&kev, ident, filter, flags, fflags, data, udata);
	int ret = kevent(kqfd, &kev, 1, NULL, 0, NULL);
	if( !ret ){
		perror("Updata Kqueue error");
		return false;
	}
	return true;
}

//事件处理函数
void HTTPServer::HandleEvent(int kq, struct kevent *events, int nevents)
{
    for (int i = 0; i < nevents; i++)
    {
		int sockfd = events[i].ident;
        int data = events[i].data;
        if(sockfd == serverSocket)	//新连接
            Accept(kq, data);
        else 						//新消息到来
			Receive(sockfd, data);
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

		Register(kq, client_fd);//add to kqueue
		cout<<"\nConnected from IP: "
			<<inet_ntoa(clientAddr.sin_addr)<<" Port: "
			<<ntohs(clientAddr.sin_port)<<" clientfd: "
			<<client_fd<<endl;

		Client *clt = new Client(client_fd,clientAddr);
		clientMap.insert( make_pair(client_fd,clt) );
		cout<<"add client to clientMap"<<endl;
    }
}
//接收数据
void HTTPServer::Receive(int sockfd, int availBytes)
{
	cout<<"fd:"<<sockfd<<" Receive data:"<<availBytes<<endl;//for test
    if(availBytes == 0){		//连接中断
		close(sockfd);
		clientMap.erase(sockfd);
		cout<<"\nclientID "<<sockfd
            <<" exit, remain "<<clientMap.size()
			<<" clients connected"<<endl;
	}else if(availBytes<0){		//出错
		perror("receive error");
		return;
	}else{						//接收数据
		Client* clt = getClient( sockfd );
		readClient( clt, availBytes );
	}
}
//根据描述符号，获取Client类
Client* HTTPServer::getClient(int clfd) 
{
	std::unordered_map<int, Client*>::const_iterator it;
    it = clientMap.find(clfd);
	
	if(it == clientMap.end()){
		cout<<"getClient no"<<endl;
		return NULL;
	}
		cout<<"getClient"<<endl;
    return it->second;
}
//读取消息到Client中
void HTTPServer::readClient(Client* pclt, int msgLen)
{
	char *buffRecv = new char[msgLen];
	int n = recv(pclt->getsocketDesc(), buffRecv, msgLen, 0);
	if( n<0 ){			//出错
		perror("receive message error");
		disconnet(pclt);
		return;
	}else if(n == 0){	//连接已经断开
		cout<<"client:"<<pclt->getIP()<<" exit."<<endl;
		disconnet(pclt);
	}else{				//处理接收的数据包
		cout<<"readClient"<<endl;
		HandleMsg(pclt,buffRecv, n);
		//###############################################到这里了

	}
}
//Client断开连接
bool HTTPServer::disconnet(Client* clt)
{	
	cout<<"disconnet"<<endl;
	if(clt == NULL){
		return false;
	}
	//kqueue中清除
	if(!updateKqueue(clt->getsocketDesc(),EVFILT_READ, EV_DELETE, 0, 0, NULL)){
		perror("disconnect error");
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
	cout<<"HandleMsg:"<<endl;
	//message[len]='\0';
	//cout<<message<<":"<<len<<endl;
	
	HTTPRequest* req = new HTTPRequest( (byte*)message, len );//########出错了，解决
	cout<<"mark"<<endl;
	if( !req->parse() ){				//########出错了,解决
		perror("request parse error");
		return false;
	}
	else{
		//显示请求消息
		cout<<"##receive message: \n";
		//cout<<req->getMethod()<<req->getReqUri()<<endl;//test
		
		req->displayMessage();			//########出错了,solved

		int method = req->getMethod();
		cout<<"Method method= "<<method<<endl;
		switch(method){
			case HEAD:					//方法HEAD
				cout<<"###get HEAD message"<<endl;//test
			case GET:
				cout<<"###GET message"<<endl; //test
				handleGet(clt,req);		//方法GET
				break;
			case OPTIONS:				//方法OPTIONS
				handleOptions(clt,req);
				break;
			case POST:
			case PUT:
			case DEL:
			case TRACE:
			case CONNECT:
			case PATCH:
			default:
				break;
		}
		return true;
	}
}

//GET 请求获取Request-URI所标识的资源
//HEAD    请求获取由Request-URI所标识的资源的响应消息报头
void HTTPServer::handleGet(Client* clt, HTTPRequest* req)
{
	cout<<"handleGet()"<<endl;//test
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
	// get the Request-Uri
	string uri = req->getReqUri();
	// get resource
	Resource *res = resHost->getResource(uri);
	// conduct HTTPResponse
	HTTPResponse* resp = new HTTPResponse();
	resp->setVersion(HTTP_VERSION);
	resp->setStatusCode(Status(OK));
	resp->setReason("OK");
	cout<<">>getMimeType:"<<res->getMimeType()<<endl;		//test
	resp->insertHeaders("Content-Type",res->getMimeType());	//####出错了,solved
	cout<<"<<getMimeType"<<endl;							//test
	
	std::stringstream sz;
	sz << res->getSize();									//##出错了
	cout<<"resource size"<<sz.str()<<endl<<"content-length"<<endl;//test
	resp->insertHeaders("Content-Length",sz.str());
	cout<<"server"<<endl;//test
	resp->insertHeaders("Server","ZhuOS/1.1");	
	cout<<"data"<<endl;//test
	resp->setData(res->getData(),res->getSize());
	cout<<"create"<<endl;//test	
	resp->create();
	
	// display Response-Message 
	resp->displayMessage();
	// send Response-Message 
	sendResponse(clt, resp);
	cout<<"handleGet() success"<<endl;//test
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
	resp->displayMessage();
	// send Response-Message 
	sendResponse(clt,resp);
}

/*********************Send Message Handle****************************/
void HTTPServer::sendResponse(Client* clt, HTTPResponse* resp)
{
	// get the sending data
	char* buffSend = new char[resp->size()];
	resp->getBytes( (byte*)buffSend );
	//send message
	if( send(clt->getsocketDesc(), buffSend, resp->size(), 0)<0 ){
		perror("send Responsing-Message error!");
		return;
	}
}


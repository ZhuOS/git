// HTTPServer：Web服务器类，提供服务器的创建，启动，停止等管理操作
#ifndef	HTTPSERVER_H
#define HTTPSERVER_H

#include <unordered_map>
#include <map>
#include <vector>
//#include <string>
#include <sstream>
#include <time.h>
#include <sys/types.h>
//#include <sys/socket.h>
#include <sys/time.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <netdb.h>
//#include <fcntl.h>
#include <kqueue/sys/event.h> // libkqueue Linux

#include"HTTPRequest.h"
#include"HTTPResponse.h"
#include"ResourceHost.h"
#include"Client.h"

#define QUEUE_SIZE  20
#define MAXBUFF		4096
// 非法的socket
#define INVALID_SOCKET -1

class HTTPServer {    
private:
	int serverSocket;
    // 监听的端口号
    int listenPort;

    // 监听的socket
    int listenSocket;

    // 服务器地址信息
    struct sockaddr_in serverAddr; 

    // kqueue 描述符
    int kqfd; 

    // kevent队列
    struct kevent evList[QUEUE_SIZE]; 

    // 客户端字典，映射客户端的socket和客户端对象
    std::unordered_map<int, Client*> clientMap;

    // 资源主机及文件系统列表
    std::vector<ResourceHost*> hostList;

    // 虚拟主机，映射请求的地址到不同的ResourceHost
    std::unordered_map<std::string, ResourceHost*> vhosts;
	
private://私有成员方法
	//注册事件函数
	bool Register(int kq, int fd); 
	//更新kqueue
	bool updateKqueue(	int ident, short filter, u_short flags,
					u_int fflags, int data, void *udata);
	//事件处理函数
	void HandleEvent(int kq, int nevents); 
	//新连接函数
	void Accept(int kq, int connSize); 
	//接收数据
	void Receive(int sockfd, int availBytes); 
	//根据描述符号，获取Client类
	Client* getClient(int clfd); 
	//读取消息到Client中
	void readClient( Client* pclt, int msgLen );
    bool writeClient(Client* pclt, int msgLen);
	//Client断开连接
	bool disconnet( Client* clt );
	//message handling function
	bool HandleMsg( Client *clt, char* message, int len );
	void handleGet( Client* cl, HTTPRequest* req);
	void handleOptions( Client* clt, HTTPRequest* req );
	//void handlePost(HTTPRequest* resq);
	// 发送响应消息给客户端
	void sendResponse(Client* clt, HTTPResponse* resp, bool dc);
public:
	bool En;
public:
	HTTPServer();
    // 处理客户端连接
    //void acceptConnection();
    //void disconnectClient(Client* cl, bool mapErase = true);
    //void readClient(Client* cl, int data_len); 
    //bool writeClient(Client* cl, int avail_bytes); 

    // 处理客户端请求
    //void handleHead(Client* cl, HTTPRequest* req);
    //void handleGet(Client* cl, HTTPRequest* req, ResourceHost* resHost);
    //void handleOptions(Client* cl, HTTPRequest* req);
	
    // 发送响应消息给客户端
    //void sendStatusResponse(Client* cl, int status, std::string msg = "");
    //void sendResponse(Client* clt, HTTPResponse* resp);

    // 启动及停止Web服务器
    bool start(int port);
    void stop();

    // Web服务器主循环
	void process();
};

#endif

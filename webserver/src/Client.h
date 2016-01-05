// HTTP客户端
// 
#ifndef CLIENT_H
#define CLIENT_H

#include<stdio.h>
#include<stdlib.h>
#include<iostream>
#include<string>
#include<string.h>
//#include<sys/epoll.h>
#include<sys/resource.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<netdb.h>
#include<unistd.h>
#include<fcntl.h>
#include <arpa/inet.h>
//#include <queue>

/*
class SendQueueItem{
    
    byte* data;
public:
    void create(byte* dat){ data = dat; }
    byte* get(){ return data; }
};
*/
class Client{
private:
    // 连接的socket
    int socketDesc; 

    // 地址信息
    struct sockaddr_in clientAddr;

    // 数据发送队列
    //std::queue<SendQueueItem*> sendQueue;

public:
	Client(int clientfd, sockaddr_in address)
	{
		socketDesc = clientfd;
		clientAddr = address;
	}

	int getsocketDesc(){return socketDesc; }
	
	std::string getIP(){ return inet_ntoa(clientAddr.sin_addr); }
	

    // 发送队列操作
    // 添加新的数据到发送队列
    //void addToSendQueue(SendQueueItem* item);

    // 发送队列长度
    //unsigned int sendQueueSize();

    // 获取发送队列中第一个元素
    //SendQueueItem* nextInSendQueue();

    // 出队操作，删除第一个元素
    //void dequeueFromSendQueue();

    // 清空发送队列
    //void clearSendQueue();
};

#endif

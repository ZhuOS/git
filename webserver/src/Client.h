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
#include <queue>
#include "SendQueueItem.h"


class Client{
private:
    // 连接的socket
    int socketDesc; 

    // 地址信息
    struct sockaddr_in clientAddr;

    // 数据发送队列
    std::queue<SendQueueItem*> sendQueue;

public:
	Client(int clientfd, sockaddr_in address)
	{
		socketDesc = clientfd;
		clientAddr = address;
	}

	int getsocketDesc(){return socketDesc; }
	
	std::string getIP(){ return inet_ntoa(clientAddr.sin_addr); }
	
	int getPort(){ 
		//cout<<"getPort:"<<ntohs(clientAddr.sin_port)<<endl;	
		return ntohs(clientAddr.sin_port); }
    // 发送队列操作
    // 添加新的数据到发送队列
    void addToSendQueue(SendQueueItem* item){
		sendQueue.push(item);
	}

    // 发送队列长度
    unsigned int sendQueueSize(){
		return sendQueue.size();
	}

    // 获取发送队列中第一个元素
    SendQueueItem* frontOfSendQueue(){
		if(sendQueue.empty())
			return NULL;

		return sendQueue.front();
	}

    // 出队操作，删除第一个元素
    void popOfSendQueue(){
		SendQueueItem* item = frontOfSendQueue();
		if(item != NULL){
			sendQueue.pop();
			delete item;
		}
	}
    // 清空发送队列
    void clearSendQueue(){
		while(!sendQueue.empty()) {
			delete sendQueue.front();
			sendQueue.pop();
		}
	}
};

#endif

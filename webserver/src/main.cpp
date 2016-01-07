//main.cpp

#include <iostream>
#include <string>
#include <signal.h>
#include "HTTPServer.h"


static HTTPServer* svr;

// 忽视 SIGPIPE "Broken pipe" 信号
void handleSigPipe(int snum) {
    return;
}

// 中断处理函数
void handleTermSig(int snum) {
    // 设置标志位表示停止服务器
	svr->En = false;
}

int main (int argc, const char * argv[])
{
    // 忽视 SIGPIPE "Broken pipe" 信号
    //std::cout << "main begin" << std::endl;
    signal(SIGPIPE, handleSigPipe);

	// 注册中断处理
	signal(SIGABRT, &handleTermSig);
	signal(SIGINT, &handleTermSig);
	signal(SIGTERM, &handleTermSig);

    // 创建并启动HTTPServer实例
	svr = new HTTPServer();
	svr->start(8080);

	// 进入主循环
	svr->process();
	//cout<<"to stop"<<endl;
	// 停止服务器
	svr->stop();
	delete svr;
	
    return 0;
}

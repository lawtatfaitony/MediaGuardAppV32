#pragma once
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdint> // 用於 uint16_t, uint32_t 等類型
#include <io.h>
#ifdef _WIN32
#include <winsock2.h>  // Windows socket
#include <ws2tcpip.h>  // Windows TCP/IP
#else
#include <arpa/inet.h> // POSIX socket
#include <unistd.h>    // POSIX standard
#endif

#include "../Common.h" 
#include "Config/DeviceConfig.h"

//https://baike.baidu.com/item/stun/3131387?fr=ge_ala 原理
//https://www.bilibili.com/opus/727141412236165127
 

class NatHeartBean {
 
public:
	explicit NatHeartBean();
	~NatHeartBean();
private:
	const char* STUN_SERVER_IP = "stun.l.google.com"; // STUN 伺服器 IP
	const int STUN_SERVER_PORT = 19302; // STUN 伺服器端口 
	// STUN 請求的大小
	const int STUN_REQUEST_SIZE = 20; // 根據需要調整大小 
public:
	 
	//獲取 IP and PORT
	int get_stun_ip(); 
	//設置 STUN 請求的內容
	void create_stun_request();
	char stun_request[20];
	int request_length;
};
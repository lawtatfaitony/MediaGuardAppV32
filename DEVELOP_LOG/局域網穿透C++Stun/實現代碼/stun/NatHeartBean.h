#pragma once
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdint> // �Ω� uint16_t, uint32_t ������
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

//https://baike.baidu.com/item/stun/3131387?fr=ge_ala ��z
//https://www.bilibili.com/opus/727141412236165127
 

class NatHeartBean {
 
public:
	explicit NatHeartBean();
	~NatHeartBean();
private:
	const char* STUN_SERVER_IP = "stun.l.google.com"; // STUN ���A�� IP
	const int STUN_SERVER_PORT = 19302; // STUN ���A���ݤf 
	// STUN �ШD���j�p
	const int STUN_REQUEST_SIZE = 20; // �ھڻݭn�վ�j�p 
public:
	 
	//��� IP and PORT
	int get_stun_ip(); 
	//�]�m STUN �ШD�����e
	void create_stun_request();
	char stun_request[20];
	int request_length;
};
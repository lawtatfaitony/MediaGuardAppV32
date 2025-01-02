#pragma once
#include <sstream> 
#include <chrono>
#include <cstdio>
#include "httplib/httplib.h"
#include "../File.h"

#include "../stun/NatHeartBean.h"

#include <iostream>
#include <stdlib.h>

#include "comm.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#endif // _WIN32

#define SERVER_CERT_FILE "./cert.pem"；
#define SERVER_PRIVATE_KEY_FILE "./key.pem"；
#define CPPHTTPLIB_OPENSSL_SUPPORT

using namespace std;
 
class httpserver
{

public:
	httpserver();
	~httpserver();

	
	static int http_server_run(void); 
	static void get_http_local_device_url(std::string& http_local_device_url); 
	static void get_http_local_dev_internet_url(std::string& http_local_dev_internet_url);
	 
private:
	
};
 

bool UserPwdValidByBearAuthorization(const httplib::Request& req, httplib::Response& res);

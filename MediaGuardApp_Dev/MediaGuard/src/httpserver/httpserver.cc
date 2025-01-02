//  cpp-httplib 組件來源與學習
//  HttpServer.cc  from https://gitee.com/AIQICcorg/cpp-httplib support
//  Copyright (c) 2019 Yuji Hirose. All rights reserved.
//  MIT License
#pragma once
 
#include <chrono>  //from httpcgi example 2024-3-10
#include <cstdio>  //from httpcgi example 2024-3-10

#include "httplib/httplib.h"
#include "httpserver.h"
#include "../httpserver/comm.h"
#include "../Common.h"
#include "../hmac/hmac_sha1.h"
#include "Common/Cmd5.h"
#include "Config/DeviceConfig.h"
 

 
httpserver::~httpserver() 
{
	// 析構函數  
	 
}
httpserver::httpserver() 
{ 
}
 
void httpserver::get_http_local_device_url(std::string& http_local_device_url)
{
	const std::string http_scheme_local = "http";

	std::stringstream http_local_device_url_str_stream;
	{ http_local_device_url_str_stream << http_scheme_local << "://" << DEVICE_CONFIG.cfgDevice.device_ip << ":" << DEVICE_CONFIG.cfgDevice.device_port << "/"; }
	 
	http_local_device_url = http_local_device_url_str_stream.str();
}

/// <summary>
/// 獲取設備公網IP URL
/// </summary>
/// <param name="http_local_device_url"></param>
void httpserver::get_http_local_dev_internet_url(std::string& http_local_dev_internet_url)
{
	const std::string http_scheme_local = "http";

	//這裡還是沿用設備配置的本地端口,所以必須先在路由器設置 NAT 地址轉換 對應 local ip 和 local port 
	NatHeartBean natHeartBean;
	std::string internetIp = natHeartBean.get_public_ip_by_curl_memory();
	std::stringstream http_local_dev_internet_url_stream;
	{ http_local_dev_internet_url_stream << http_scheme_local << "://" << internetIp << ":" << DEVICE_CONFIG.cfgDevice.device_port << "/"; }

	http_local_dev_internet_url = http_local_dev_internet_url_stream.str();
}


void allowCorsAccess(httplib::Response& res) {

	// 定義允許的來源  
	/*std::set<std::string> allowed_origins = {
		DEVICE_CONFIG.cfgHttpServerCloud.url,
		DEVICE_CONFIG.cfgDevice.local_device_url
	};*/

	// 允许跨域请求
	res.set_header("Access-Control-Allow-Origin", "*");
	// 是否允許夾帶COOKIE 等等
	res.set_header("Access-Control-Allow-Credentials", "true");
	// 是否允許私域請求Internet (目前Chrome要求帶證書才允許)
	res.set_header("Access-Control-Allow-Private-Network", "true");
	// 所有http動作都允許
	res.set_header("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE, PATCH");
	// 貌似使用通配符(*)是無效的
	//res.set_header("Access-Control-Allow-Headers", "*"); //添加這個特定的試試，之前是注釋掉的 2023-8-6
	 
	res.set_header(
		"Access-Control-Allow-Headers",
		"X-Custom-Header,DNT,User-Agent,X-Requested-With,If-Modified-Since,Cache-"
		"Control,Content-Type,Range,Accept,Accept-Encoding,Accept-Language,"
		"Access-Control-Request-Headers,Access-Control-Request-Method,Connection,"
		"Host,Origin,Sec-Fetch-Mode,Referer,Authorization");
}

std::string dump_headers(const httplib::Headers& headers) {
	std::string s;
	char buf[BUFSIZ];

	for (auto it = headers.begin(); it != headers.end(); ++it) {
		const auto& x = *it;
		snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
		s += buf;
	}

	return s;
}

//request http log
std::string log(const httplib::Request& req, const httplib::Response& res) {
	std::string s;
	char buf[BUFSIZ];

	s += "\n-------------------------------------------------\n";

	snprintf(buf, sizeof(buf), "%s %s %s", req.method.c_str(),
		req.version.c_str(), req.path.c_str());
	s += buf;

	std::string query;
	for (auto it = req.params.begin(); it != req.params.end(); ++it) {
		const auto& x = *it;
		snprintf(buf, sizeof(buf), "%c%s=%s",
			(it == req.params.begin()) ? '?' : '&', x.first.c_str(),
			x.second.c_str());
		query += buf;
	}
	snprintf(buf, sizeof(buf), "%s\n", query.c_str());
	s += buf;

	s += dump_headers(req.headers);

	s += "\n-------------------------------------------------\n";

	snprintf(buf, sizeof(buf), "%d %s\n", res.status, res.version.c_str());
	s += buf;
	s += dump_headers(res.headers);
	s += "\n";

	if (!res.body.empty()) { s += res.body; }

	s += "\n";

	return s;
}

// 校验token參數
bool UserPasswordValidByToken(const httplib::Request& req, httplib::Response& res) {

	std::string local_login = DEVICE_CONFIG.cfgDevice.local_login;
	std::string local_password = DEVICE_CONFIG.cfgDevice.local_password;

	// 获取请求路径
	std::string path = req.path;
#ifdef DEBUG
	printf("\n[funs::UserPasswordValidByToken]path = %s\n\n", path.c_str());
#endif // DEBUG
	 
	// 认证用户秘钥
	std::string token = req.get_param_value("token");

	std::stringstream md5_stringstream;
	{ md5_stringstream << local_login << ":" << local_password; }
	 
	std::string local_authorization = Cmd5::get_md5(md5_stringstream.str());
#ifdef DEBUG
	printf("\nUserPasswordValidByToken: \ntoken =%s\nlocal_authorization=%s \t ...lower md5(local_login:local_password) \n\n", token.c_str(), local_authorization.c_str());
#endif // DEBUG
	 
	if (token.empty()) {
		res.status = 401;
		res.set_content("No Authentic 401 (token empty)", "text/plain");
		return false;
	}
	if (token != local_authorization) {
		res.status = 401;
		return false;
	}
	else {
		return true;
	} 
}

/// <summary>
/// 校验用户名密码以Bearer Token 形式 (沒其他函數引用此功能)
/// 用户名密碼保存在 ./conf/device.json下的節點device_config.local_login:device_config.local_password
/// 用於本地MediaGuard請求,注意:這裡不是雲端賬號
bool UserPwdValidByBearAuthorization(const httplib::Request& req, httplib::Response& res)
{
	// 认证用户秘钥
	std::string authentication = req.get_header_value("Authorization");
	if (authentication.empty())
	{
		res.status = 401;
		res.set_content("No Header Authorization To Access! (token empty :401)", "text/plain"); 
		return false;
	}
	
	// 认证用户秘钥
	std::string local_login = DEVICE_CONFIG.cfgDevice.local_login;
	std::string local_password = DEVICE_CONFIG.cfgDevice.local_password;

	std::stringstream md5_stringstream;
	{ md5_stringstream << local_login << ":" << local_password; }
  
	//device.json -> user:password MD5 (device.json-> user:password )
	std::string md5_lower_string = Cmd5::get_md5(md5_stringstream.str());
	//========================================================================================================================================
	//std::string local_authorization = DEVICE_CONFIG.cfgDevice.local_authorization;
	//避免設置device.json出錯,改為:
	std::string local_authorization = md5_lower_string;
	if (authentication != local_authorization)
	{
		res.status = 401;
		res.set_content(transcoding::T2UTF8("not authorized to access this interface(authentication no equal)!").c_str(), "text/plain");
		return false;
	}
	return true;
}

//初始化和定義
int httpserver::http_server_run(void) {
	 
//http server
httplib::Server svr;
   
	std::string local_device_url;
	httpserver::get_http_local_device_url(local_device_url);

	std::string http_local_dev_internet_url;
	httpserver::get_http_local_dev_internet_url(http_local_dev_internet_url);

	std::string local_login = DEVICE_CONFIG.cfgDevice.local_login;
	std::string local_password = DEVICE_CONFIG.cfgDevice.local_password;

	if (!svr.is_valid()) {
		printf("server has an error...\n");
		return -1;
	}

	printf("\n[HTTP SERVER STARTING......] \n\n[TEST HTML URL : %stest] redirect to /web/playtest.html \n\n", local_device_url.c_str());
	
	// User defined file extension and MIME type mappings
	svr.set_file_extension_and_mimetype_mapping("txt", "text/plain");
	svr.set_file_extension_and_mimetype_mapping("md", "text/plain");
	svr.set_file_extension_and_mimetype_mapping("html", "text/html");
	svr.set_file_extension_and_mimetype_mapping("htm", "text/html");
	svr.set_file_extension_and_mimetype_mapping("bmp", "image/bmp");
	svr.set_file_extension_and_mimetype_mapping("jpg", "image/jpeg");
	svr.set_file_extension_and_mimetype_mapping("jpeg", "image/jpeg");

	svr.set_file_extension_and_mimetype_mapping("mp3", "video/mp3");
	svr.set_file_extension_and_mimetype_mapping("mp4", "video/mp4");
	svr.set_file_extension_and_mimetype_mapping("flv", "video/x-flv");
	svr.set_file_extension_and_mimetype_mapping("mpeg", "video/mpeg");
	svr.set_file_extension_and_mimetype_mapping("mp3", "audio/mp3");
	svr.set_file_extension_and_mimetype_mapping("avi", "video/x-msvideo");
	svr.set_file_extension_and_mimetype_mapping("m3u8", "application/vnd.apple.mpegurl");
	svr.set_file_extension_and_mimetype_mapping("ts", "video/mp2t");

	svr.Get("/", [=](const httplib::Request& /*req*/, httplib::Response& res) {
		res.set_redirect("/hi");
		printf("\n%s/ redirect to /hi \n\n", local_device_url.c_str());
	});

	//*測試頁面 from: index_template.html */
	svr.Get("/test", [=](const httplib::Request& /*req*/, httplib::Response& res) {

		int nCode = 0;
		std::string file_content;
		if (nCode == File::readFileContent("./web/index_template.html", file_content))
		{ 
			//device.json -> user:password MD5 (device.json-> user:password )
			std::stringstream md5_stringstream;
			{ md5_stringstream << local_login << ":" << local_password; }
			 
			std::string local_authorization = Cmd5::get_md5(md5_stringstream.str());

			std::string replace_admin_phrase = "$admin$";
			std::string replace_password_phrase = "$password$";
			std::string replace_local_authorization_phrase = "$local_authorization$"; 
			std::string replace_camera_hls_phrase = "$camera_hls$";
			//格式實例 : http://192.168.0.128:180/hls/8/index.m3u8?token=7ad166bdb8395514bb54cc0ac21db289 
			std::stringstream camera_hls_stringstream;
			{ camera_hls_stringstream << local_device_url << "hls/8/index.m3u8?token=" << local_authorization; }

			std::string replace_camera_hls_by_stun_phrase = "$camera_hls_by_stun$";
			//格式實例 : http://42.3.90.118:180/hls/8/index.m3u8?token=7ad166bdb8395514bb54cc0ac21db289 
			std::stringstream camera_hls_bu_stun_stringstream;
			{ camera_hls_bu_stun_stringstream << http_local_dev_internet_url << "hls/8/index.m3u8?token=" << local_authorization; }
			
			// 替換所有出現的標記
			File::replaceAll(file_content, replace_admin_phrase, local_login);
			File::replaceAll(file_content, replace_password_phrase, local_password);
			File::replaceAll(file_content, replace_local_authorization_phrase, local_authorization);
			File::replaceAll(file_content, replace_camera_hls_phrase, camera_hls_stringstream.str());
			File::replaceAll(file_content, replace_camera_hls_by_stun_phrase, camera_hls_bu_stun_stringstream.str());

			res.set_content(file_content, "text/html");
		}
		else
		{
			file_content = "read file fail (utf8 format required";
			res.set_content(file_content, "text/plain");
		}
		
		printf("\n[■TEST HTML PAGE■] %stest redirect to //web/index_template.html \n\n", local_device_url.c_str());
	});

	// web/index.html from: ./web/index.html 
	svr.Get("/index", [=](const httplib::Request& /*req*/, httplib::Response& res) {

		int nCode = 0;
		std::string file_content;
		if (nCode == File::readFileContent("./web/index.html", file_content))
		{ 
			res.set_content(file_content, "text/html");
		}
		else
		{
			file_content = "read file:./web/index.html fail (utf8 format required";
			res.set_content(file_content, "text/plain");
		}

		printf("\n[TEST HTML PAGE] %sweb/playtest.html redirect to //web/playtest.html \n\n", local_device_url.c_str());
		});
	svr.set_pre_routing_handler(
		[](const auto& req, auto& res) -> httplib::Server::HandlerResponse {
			if (req.path == "/hello") { //全局處理路由

				//<meta charset="UTF-8">
				std::string templateHtml = R"(
                        <!DOCTYPE html>
                        <html lang="en">
                        <head> 
							<title>Hello the world</title>
							<meta http-equiv="Content-Type" content="text/html;charset=UTF-8"/>
                        </head>
                        <body>
                        <h1> $hello$ </h1>
                        <script>
                        const ev1 = new EventSource("event1");
                        ev1.onmessage = function(e) {
                          console.log('ev1', e.data);
                        }
                        const ev2 = new EventSource("event2");
                        ev2.onmessage = function(e) {
                          console.log('ev2', e.data);
                        }
                        </script>
                        </body>
                        </html>
                        )";
				cout << "the current page is basic on utf8" << endl;
				const char* src_char = "Hello the world!";
				cout << "origin string: " << src_char << endl;

				std::string stringToken = "$hello$";

				std::string newstr = templateHtml.replace(
					templateHtml.find(stringToken),
					stringToken.length(),
					src_char); //str_utf8

				cout << "templateHtml: " << templateHtml.c_str() << endl;

				res.set_content(templateHtml.c_str(), "text/html");

				return httplib::Server::HandlerResponse::Handled;
			}
			return httplib::Server::HandlerResponse::Unhandled;
		});

	svr.Get("/hi", [](const httplib::Request& /*req*/, httplib::Response& res) {
		res.set_content("Hello World!\n", "text/plain");
		return httplib::Server::HandlerResponse::Handled; //1.1
	});

	svr.Get("/slow", [](const httplib::Request& req, httplib::Response& res) {
		std::this_thread::sleep_for(std::chrono::seconds(2));

		if (req.has_param("key")) { auto val = req.get_param_value("key"); }
		res.set_content("Slow...\n", "text/plain");
		});

	// 探测请求
	svr.Options(R"(/video/(\d+)-(\d+)-(\d+))",
		[](const httplib::Request& req, httplib::Response& res) {
			// 允许跨域请求
			allowCorsAccess(res);
			printf("Video Options");
		});

	svr.Get(R"(/video/(\d+)-(\d+)-(\d+))", [&](const httplib::Request& req, httplib::Response& res) {
		if (req.has_param("token")) {
			auto token = req.get_param_value("token");
			std::stringstream authentic_content;
			{ authentic_content << "Athentic token = " << token; }
			printf(authentic_content.str().c_str());

			if (UserPasswordValidByToken(req, res) == false) {
				printf("video req.path = %s\n\n", req.path.c_str());
				return;
			}
			 
		}
		else {

			std::string msg = "<p>User Authentic Status (?token=...):<span"
				"style='color:red;'><br>No Atuthentic : User Authentic Status:401</span> </p> <p>Unsupported Authentication!</p>";
			const char* fmt = "No Atuthentic : User Authentic Status:401";
			char buf[BUFSIZ];
			res.status = 401;
			snprintf(buf, sizeof(buf), fmt, res.status);
			res.set_content(msg, "text/html");
		}
		});

	// 探测请求
	svr.Options("/video",
		[](const httplib::Request& req, httplib::Response& res) {
			// 允许跨域请求
			allowCorsAccess(res);
			printf("Video Options");
		});
	
	// authentic with token
	svr.Get("/video", [](const httplib::Request& req, httplib::Response& res) {
		if (req.has_param("token")) {
			auto token = req.get_param_value("token");
			std::stringstream authentic_content;
			{ authentic_content << "Athentic token = " << token; }
			printf(authentic_content.str().c_str());

			if (UserPasswordValidByToken(req, res) == false) {
				printf("video req.path = %s\n\n", req.path.c_str());
				return;
			}
		}
		else {

			std::string msg = "<p>User Authentic Status: <span "
				"style='color:red;'>%d</span> </p> <p>Unsupported Authentication!</p>";
			const char* fmt = msg.c_str();
			char buf[BUFSIZ];
			res.status = 401;
			snprintf(buf, sizeof(buf), fmt, res.status);
			res.set_content("No Atuthentic : User Authentic Status:401", "text/html");
		}
		});
	
	// 探测请求
	svr.Options("/video", [](const httplib::Request& req, httplib::Response& res) {
		// 允许跨域请求
		allowCorsAccess(res);
	});

	// 探测请求
	svr.Get("/play", [](const httplib::Request& req, httplib::Response& res) {
		allowCorsAccess(res);
		res.status = 200;
		res.set_content("{'success':true,errorCode:-1}", "text/html");
		});
	
	svr.Options("/play", [](const httplib::Request& req, httplib::Response& res) {
		// 允许跨域请求
		allowCorsAccess(res);
	});
	 
	// 播放视频
	//svr.Get("/play", [](const httplib::Request& req, httplib::Response& res) {
	//	// 允许跨域请求
	//	allowCorsAccess(res);
	//	// 用户名密码校验
	//	if (!UserPasswordValidByHeader(req, res))
	//	{
	//		return;
	//	}

	//	//DOC: http://192.168.8.19:180/play?deviceId=11&channel=1&stream=1
	//	// 获取播放参数
	//	std::string deviceId = req.get_param_value("deviceId");
	//	std::string channel = req.get_param_value("channel");
	//	std::string stream = req.get_param_value("stream");

	//	std::stringstream mpeg_path;
	//	{mpeg_path <<  File::GetWorkPath() << "\\hls\\" << deviceId << "\\index.m3u8"; } 
	//	});

	svr.Get("/dump", [](const httplib::Request& req, httplib::Response& res) {
		res.set_content(dump_headers(req.headers), "text/plain");
	});

	svr.Get("/stop",
		[&](const httplib::Request& /*req*/, httplib::Response& /*res*/) { svr.stop(); });

	// 更新录像策略
	httplib::Server::Handler updateStrategy = [&](const httplib::Request& req, httplib::Response& res) {
		/*
		{
			"id": "13",
			"saveMpeg4": false,
			"sundayStart": "00:00:00",
			"sundayEnd": "23:59:59",
			"mondayStart": "00:00:00",
			"mondayEnd": "23:59:59",
			"tuesdayStart": "00:00:00",
			"tuesdayEnd": "23:59:59",
			"wednesdayStart": "00:00:00",
			"wednesdayEnd": "23:59:59",
			"thursdayStart": "00:00:00",
			"thursdayEnd": "23:59:59",
			"fridayStart": "00:00:00",
			"fridayEnd": "23:59:59",
			"saturdayStart": "00:00:00",
			"saturdayEnd": "23:59:59"
		}
		*/
		// 允许跨域请求
		allowCorsAccess(res);
		res.set_content("add strategy success!", "text/plain");
	};

	svr.Post("/updateStrategy", updateStrategy);

	svr.set_logger([](const httplib::Request& req, const httplib::Response& res) {
		if (res.status == 404 || res.status == 401 || res.status == 302 || res.status == 206 || res.status == 500)
		{
			printf("\n[http_server_logger] [log] [StatusCode=404||401||302||206||500] res.status = %d \n\n%s", res.status, log(req, res).c_str());
		}
		else {
#ifdef DEBUG
			printf("\n[http_server_logger] request_status_code = %d\n", res.status);
#endif // DEBUG 
		}
	});
	 
	//mount begin ---------------------------------------------
	auto ret1 = svr.set_mount_point("/video", "./video");

	if (!ret1) {
		printf("http_server [%svideo] folder does not exist! %s\n\n", local_device_url.c_str(),"./video");
	}
	else {
		printf("http_server mount [%svideo] success! %s\n\n", local_device_url.c_str(), "./video");
	}

	 
	auto ret2 = svr.set_mount_point("/hls", "./hls");
	if (!ret2) {
		printf("[%shls] folder does not exist! %s\n\n", local_device_url.c_str(), "./hls");
	}
	else {
		printf("http_server mount [%shls] success! %s\n\n", local_device_url.c_str(), "./hls");
	}

	// 探测请求 格式 http://192.168.0.128:180/hls/12/index.m3u8?token=7ad166bdb8395514bb54cc0ac21db289
	svr.Options("/hls", [](const httplib::Request& req, httplib::Response& res) {
		// 允许跨域请求
		allowCorsAccess(res); 
	});
	 
	auto ret3 = svr.set_mount_point("/picture", "./picture");
	if (!ret3) {
		printf("http_server [%spicture] folder does not exist! %s\n\n", local_device_url.c_str(), "./picture");
	}
	else {
#ifdef DEBUG
		printf("http_server mount [%spicture] success! %s\n\n", local_device_url.c_str(), "./picture");
#endif
	}
	 
	auto ret4 = svr.set_mount_point("/web", "./web");
	if (!ret4) {
		printf("[%sweb] folder does not exist! %s\n\n", local_device_url.c_str(), "./web");
	}
	else {
		printf("http_server mount [%sweb] success!%s\n\n", local_device_url.c_str(), "./web");
	}
	/// mount end ---------------------------------------------

	// 文件请求响应处理器(请求成功响应之前)
	svr.set_file_request_handler(
		[](const httplib::Request& req, httplib::Response& res) {
			// 允许跨域请求
			allowCorsAccess(res);
			// 获取请求路径：/hls/1/index.m3u8
			std::string path = req.path;
#ifdef DEBUG
			printf("\n[funs::set_file_request_handler] file_request path = %s\n\n", path.c_str());
#endif 
			// html/htm文件是否免用户校验
			if (path.find("index.html") != -1 || path.find("index.htm") != -1)  //html或htm : 需要用戶驗證 token的情況  
			{
				//開關token驗證
				if (!UserPasswordValidByToken(req, res)) {
					printf("request *.html or *.htm reuired token ,html unauthorized!!!(set_file_request_handler %s)\n\n", path.c_str());
					return;
				}
			}

			// m3u8索引文件請求
			if (path.find("index.m3u8") != -1) {

				res.set_header("Content-Type", "application/vnd.apple.mpegurl"); 
				if (!UserPasswordValidByToken(req, res)) {
					printf(".ts unauthorized!!! fun::set_file_request_handler %s\n\n", path.c_str()); 
					return;
				}
			}

			// ts文件免用户校验
			// request status code = 206 部分請求成功 可能index0.ts包含的分片文件受限制
			if (path.find(".ts") != -1)
			{
#ifdef DEBUG

				printf("[ts file request] Content-Type video/mp2t %s\n\n", path.c_str());
				 
				//這段導致.ts文件不能放開訪問
				/*if (!UserPasswordValidByToken(req, res)) {
					printf(".ts unauthorized!!! fun::set_file_request_handler %s\n\n", path.c_str());
					printf("*.ts unauthorized!!! Content-Type video/mp2t %s\n\n", path.c_str());
					return;
				}*/
#endif
			}

			// flv/mp4索引文件下载
			if (path.find(".mp4") != -1) {

				res.set_header("Content-Type", "video/mp4");

				if (!UserPasswordValidByToken(req, res)) {
					printf("media mp4 unauthorized!!! set_file_request_handler %s\n\n", path.c_str());
					return;
				}
			}

			if (path.find(".flv") != -1) {

				res.set_header("Content-Type", "video/x-flv");

				if (!UserPasswordValidByToken(req, res)) {
					printf("media flv unauthorized!!! set_file_request_handler %s\n\n", path.c_str());
					return;
				}
			}
		});

	// 错误处理
	httplib::Server::Handler errorHandle = [](const auto& req, auto& res) {

		auto err_string = "\n[Media Http Server] http media server error status: %d %s ----------------------------------------\n";
		char buf_screen[BUFSIZ];
		snprintf(buf_screen, sizeof(buf_screen), err_string, res.status, Time::GetCurrentSystemTime());

		auto fmt = "<p>Error Status: <span style='color:red;'>media server error: %d</span></p>"; 
		char buf[BUFSIZ]; 
		res.set_content(buf, "text/html");
	};
	svr.set_error_handler(errorHandle);

	//---------------------------------------------------------------------------------------------
	std::string local_ip = DEVICE_CONFIG.cfgDevice.device_ip;
	const int local_port = DEVICE_CONFIG.cfgDevice.device_port;;
	svr.listen(local_ip.c_str(), local_port);

	return 0;
}

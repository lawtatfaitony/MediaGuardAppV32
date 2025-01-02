#pragma once  
#include <sstream>
#include <fstream> 
#include "../Common/JsonHelper.h" 
#include "DeviceConfig.h"
#include "../../Common.h"

#include "../Basic/CrossPlat.h"   
#include "../Common/JsonHelper.h"
#include "../Common/Macro.h" 
#include "../ErrorInfo/ErrorCode.h"
#include "../Http/LibcurlHelper.h"

//修改為使用 std 17  --2024-3-13 
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <filesystem>  // C++17 標準庫

using namespace rapidjson;
namespace fs = std::filesystem;

bool DeviceConfig::Init()
{
	std::string file_path = Config::GetDeviceConfigFile();
	const std::string config_content = File::readJsonFile(file_path);
	bool res = parse_config(config_content);
	if (res == false)
		//LOG(INFO) << "device.json file format required only utf8(no bom):\n" << file_path;
		std::cout <<  "device.json file format required only utf8(no bom):\n"   << std::endl;
	return res;
}
 
bool DeviceConfig::parse_config(const std::string& datrData)
{
	/*TEST 带签名的UTF BOM前面几个特殊符号导致无法解析为rapidjson::Document doc*/
	//const std::string datrData1 = "{ \"device_config\": { \"device_serial_no\": \"BFEBFBFF000906EA\" } }"; // "{\"device_config\":\"rapidjson\",\"stars\":10}";
	rapidjson::Document doc;
	JS_PARSE_OBJECT_RETURN(doc, datrData, false);

	/*rapidjson::Document doc;*/

	if (doc.HasMember("device_config") && doc["device_config"].IsObject())
	{
		config.cfgDevice.LoadDevFromJson(doc["device_config"]);
	}

	if (doc.HasMember("http_server_cloud") && doc["http_server_cloud"].IsObject())
	{
		config.cfgHttpServerCloud.LoadFromJson(doc["http_server_cloud"]);
	}

	if (doc.HasMember("http_server") && doc["http_server"].IsObject())
	{
		config.cfgHttpServer.LoadFromJson(doc["http_server"]);
	}

	if (doc.HasMember("storage_server") && doc["storage_server"].IsObject())
	{
		config.cfgStorageServer.LoadFromJson(doc["storage_server"]);
	}

	if (doc.HasMember("global_setting") && doc["global_setting"].IsObject())
	{
		config.cfgGlobalSetting.LoadFromJson(doc["global_setting"]);
	}

	if (doc.HasMember("database") && doc["database"].IsObject())
	{
		config.cfgDatabase.LoadFromJson(doc["database"]);
	}

	if (doc.HasMember("pool") && doc["pool"].IsObject())
	{
		config.cfgPool.LoadFromJson(doc["pool"]);
	}

	if (doc.HasMember("carPlateRecogBusiness") && doc["carPlateRecogBusiness"].IsObject())
	{
		config.cfgCarPlateRecogBusiness.LoadFromJson(doc["carPlateRecogBusiness"]);

		if (config.cfgCarPlateRecogBusiness.enable == true)
		{
			config.cfgCarPlateRecogBusiness.LoadCamListFromJson(doc["carPlateRecogBusiness"], config.cfgCarPlateRecogBusiness.PermitedCamList);
		}
	}

	return true;
}

/*设备基本配置以及登录云端api账户密码*/
void DeviceConfig::Device::LoadDevFromJson(rapidjson::Value& value)
{

	JS_PARSE_OPTION(maincom_id, value, String, maincom_id);
	JS_PARSE_OPTION(device_ip, value, String, device_ip);
	JS_PARSE_OPTION(device_port, value, Int, device_port);
	JS_PARSE_OPTION(device_serial_no, value, String, device_serial_no);
	JS_PARSE_OPTION(device_serial_no_hmac, value, String, device_serial_no_hmac);
	JS_PARSE_OPTION(device_is_online_always, value, Bool, device_is_online_always);

	JS_PARSE_OPTION(user, value, String, user);
	JS_PARSE_OPTION(password, value, String, password);
	JS_PARSE_OPTION(password_format, value, String, password_format);
	JS_PARSE_OPTION(local_login, value, String, local_login);			//訪問本地設備API賬戶
	JS_PARSE_OPTION(local_password, value, String, local_password); //訪問本地設備API密碼
	JS_PARSE_OPTION(local_authorization, value, String, local_authorization); //訪問本地local_authorization token
	JS_PARSE_OPTION(reamrks, value, String, reamrks);
	JS_PARSE_OPTION(language_code, value, String, language_code);

	{
		std::stringstream local_device_url_stream;
		local_device_url_stream << "http" << "://" << device_ip << ":" << device_port << "/";
		local_device_url = local_device_url_stream.str();
	}
}

/*云端配置Url*/
void DeviceConfig::HttpServerCloud::LoadFromJson(rapidjson::Value& value)
{
	JS_PARSE_OPTION(scheme, value, String, scheme);
	JS_PARSE_OPTION(host, value, String, host);
	JS_PARSE_OPTION(port, value, Int, port);
	JS_PARSE_OPTION(remarks, value, String, remarks);

	{
		std::stringstream ss;
		ss << scheme << "://" << host << ":" << port << "/";
		url = ss.str();
	}
}

/*配置Web Url*/
void DeviceConfig::HttpServer::LoadFromJson(rapidjson::Value& value)
{
	JS_PARSE_OPTION(scheme, value, String, scheme);
	JS_PARSE_OPTION(host, value, String, host);
	JS_PARSE_OPTION(port, value, Int, port);
	JS_PARSE_OPTION(remarks, value, String, remarks);
	{
		std::stringstream ss;
		ss << scheme << "://" << host << ":" << port << "/";
		url = ss.str();
	}
}

/*配置存储SEVER Url*/
void DeviceConfig::StorageServer::LoadFromJson(rapidjson::Value& value)
{
	JS_PARSE_OPTION(scheme, value, String, scheme);
	JS_PARSE_OPTION(host, value, String, host);
	JS_PARSE_OPTION(port, value, Int, port);
	JS_PARSE_OPTION(remarks, value, String, remarks);
	{
		std::stringstream ss;
		ss << scheme << "://" << host << ":" << port << "/";
		url = ss.str();
	}
}

/*配置存储SEVER Url*/
void DeviceConfig::GlobalSetting::LoadFromJson(rapidjson::Value& value)
{
	JS_PARSE_OPTION(recordTimeMinutes, value, Int, recordTimeMinutes);
	JS_PARSE_OPTION(picRemainMinutes, value, Int, picRemainMinutes);
	JS_PARSE_OPTION(videoRemainMinutes, value, Int, picRemainMinutes);
	JS_PARSE_OPTION(storageLimitedbytes, value, Int64, storageLimitedbytes);  //rapidjson::Value::GetInt64()
	JS_PARSE_OPTION(ffmpegOpenInfo, value, Bool, ffmpegOpenInfo);
	JS_PARSE_OPTION(nHDType, value, String, nHDType);
	JS_PARSE_OPTION(remarks, value, String, remarks);
}

/*配置存储SEVER DATABASE*/
void DeviceConfig::Database::LoadFromJson(rapidjson::Value& value)
{
	JS_PARSE_OPTION(strIp, value, String, ip);
	JS_PARSE_OPTION(strFrontBase, value, String, base_frontend);
	JS_PARSE_OPTION(strFrontBase, value, String, base_frontend);
	JS_PARSE_OPTION(strHistoryBase, value, String, base_history);
	JS_PARSE_OPTION(stUser, value, String, user);
	JS_PARSE_OPTION(strPassword, value, String, password);

	auto func_generate_conn_str = [](std::string& strConn, const std::string& strHost,
		const std::string& strBaseName, const std::string& strUser, const std::string& strPassword) {
			std::stringstream ss;
			ss << "mysql://"
				<< "host=" << strHost
				<< " port=" << 3306
				<< " db=" << strBaseName
				<< " user=" << strUser
				<< " password=" << strPassword
				<< " charset = utf8";
			strConn = ss.str();
	};
	/*前端数据库的连接串*/
	func_generate_conn_str(strConnFrontendBase, strIp, strFrontBase, stUser, strPassword);
	/*后端数据库的连接串*/
	func_generate_conn_str(strConnHistoryBase, strIp, strHistoryBase, stUser, strPassword);
}

/*配置存储SEVER DATABASE 连接池数量*/
void DeviceConfig::PoolConfig::LoadFromJson(rapidjson::Value& value)
{
	JS_PARSE_OPTION(database_pool, value, Int, database_pool);
}

/*配置 Car Plate Recog Business*/
void DeviceConfig::CarPlateRecogBusiness::LoadFromJson(rapidjson::Value& value)
{
	JS_PARSE_OPTION(enable, value, Bool, enable);
	JS_PARSE_OPTION(http_server_api, value, String, http_server_api);
	JS_PARSE_OPTION(http_detect_server_api, value, String, http_detect_server_api);
	JS_PARSE_OPTION(threshold, value, Double, threshold);
	JS_PARSE_OPTION(user, value, String, user);
	JS_PARSE_OPTION(password, value, String, password);
	JS_PARSE_OPTION(remarks, value, String, remarks);

	/*if (enable)
	{
		LoadCamListFromJson(value["permited_cameraId"], DeviceConfig::CarPlateRecogBusiness::PermitedCamList);
	}*/
}
 
/*如果啟動第三方車牌鏡頭識別服務則返回鏡頭列表*/
void DeviceConfig::CarPlateRecogBusiness::LoadCamListFromJson(rapidjson::Value& value, PermitedCameraList& permitedCameraList)
{
	const rapidjson::Value& a = value["permited_cameraId"];
	if (a.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < a.Size(); i++)
		{
			PermitedCameras permitedCameras;
			JS_PARSE_OPTION(permitedCameras.CameraId, a[i], Int, cameraId);
			JS_PARSE_OPTION(permitedCameras.Name, a[i], String, name);
			JS_PARSE_OPTION(permitedCameras.RtspIp, a[i], String, rtspIp);

			permitedCameraList.push_back(permitedCameras);
		}
	}
}

/*获取本地CUP SERIAL NO*/
void DeviceConfig::getcpuid(unsigned int* CPUInfo, unsigned int InfoType)
{
#if defined(__GNUC__)// GCC  
	 
	__asm__ __volatile__(
		"movl %1, %%eax;"
		"cpuid;"
		"movl %%eax, (%0);"
		"movl %%ebx, 4(%0);"
		"movl %%edx, 8(%0);"
		"movl %%ecx, 12(%0);"
		: "=r" (CPUInfo)
		: "r" (InfoType)
		: "%eax", "%ebx", "%ecx", "%edx"
	);



#elif defined(_MSC_VER)// MSVC  
#if _MSC_VER >= 1400 //VC2005才支持__cpuid  
	__cpuid((int*)(void*)CPUInfo, (int)(InfoType));
#else //其他使用getcpuidex  
	get_cpuidex(CPUInfo, InfoType, 0);
#endif  
#endif
}

void DeviceConfig::get_cpuidex(unsigned int* CPUInfo, unsigned int InfoType, unsigned int ECXValue)
{
#if defined(_MSC_VER) // MSVC
	
#if defined(_WIN64) // 64位下不支持内联汇编. 1600: VS2010, 据说VC2008 SP1之后才支持__cpuidex.  
	__cpuidex((int*)(void*)CPUInfo, (int)InfoType, (int)ECXValue);
#else  
	if (NULL == CPUInfo)
		return;
	_asm {
		// load. 读取参数到寄存器.  
		mov edi, CPUInfo;
		mov eax, InfoType;
		mov ecx, ECXValue;
		// CPUID  
		cpuid;
		// save. 将寄存器保存到CPUInfo  
		mov[edi], eax;
		mov[edi + 4], ebx;
		mov[edi + 8], ecx;
		mov[edi + 12], edx;
	}
#endif  
#endif  
}



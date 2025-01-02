#pragma once
#include "../Common.h" 
  
#include <map>
#include <mutex>
#include <fstream>
#include <vector>
#include <list>

using namespace rapidjson;

#define DEVICE_CONFIG DeviceConfig::Instance().GetConfig()

struct DeviceConfigReturn {
	DeviceConfigReturn() {

		this->DeviceConfig = "";
		this->ExpiredTimeOut = 0;
	};
	std::string DeviceConfig;
	int64_t ExpiredTimeOut;
};

static struct DeviceConfigReturn DeviceConfigRet;

struct PermitedCameras
{
	PermitedCameras() {
		this->CameraId = 0;
		this->Name = "";
		this->RtspIp = "";
	};
	int CameraId;
	std::string Name;
	std::string RtspIp;
};

class DeviceConfig
{
	typedef std::list<PermitedCameras>  PermitedCameraList;
public:
	struct Device
	{
		std::string maincom_id;
		std::string device_ip;
		int device_port = 8080;
		std::string device_serial_no;
		std::string device_serial_no_hmac;
		bool device_is_online_always = false;
		std::string user = "admin";
		std::string password = "0192023A7BBD73250516F069DF18B500";
		std::string password_format = "md5_32_lower (default)";
		/// <summary>
		/// //訪問本地MdiaGuard(設備)API賬戶
		/// </summary>
		std::string local_login = "mediaGuardAdmin";
		/// <summary>
		/// 訪問本地MediaGuard(設備)API密碼
		/// </summary>
		std::string local_password = "admin888";
		/// <summary>
		/// Authorization 
		/// Basic加密方式如下所示：
		/// Bearer认证方式更安全，不易被破解攻击，加密方式如下
		/// Authorization: Bearer md5(username:password)
		/// 默認： admin:admin123 的 Authorization token = 4a6a6c7f2cdd12a826e2f15675a6c6ac
		/// </summary>
		std::string local_authorization = "4a6a6c7f2cdd12a826e2f15675a6c6ac";
		std::string local_device_url = "http://192.168.0.128:180/";
		std::string reamrks = "serial no is hamc by toll from STAR-LangMXcore password format is md5_32_upcase (default)";
		std::string language_code = "hk";
		void LoadDevFromJson(rapidjson::Value& value);
	};

	struct HttpServerCloud
	{
		std::string scheme = "http";
		std::string host = "127.0.0.1";
		int port = 5000;
		std::string url = "";
		std::string remarks = "cloud server *****";
		void LoadFromJson(rapidjson::Value& value);
	};

	struct HttpServer
	{
		std::string scheme = "http";
		std::string host = "127.0.0.1";
		int port = 10009;
		std::string url = "(default)";
		std::string remarks = "web server ****";
		void LoadFromJson(rapidjson::Value& value);
	};

	/* 存储服务配置 */
	struct StorageServer
	{
		std::string scheme = "http";
		std::string host = "127.0.0.1";
		int port = 10009;
		std::string url = "(default)";
		std::string remarks = "  for upload mp4/flv/jpg of mediaGuard";
		void LoadFromJson(rapidjson::Value& value);
	};

	/* 全局设置 */
	struct GlobalSetting
	{
		int recordTimeMinutes = 360000;  //每文件的錄像時長 300000 = 60 * 1000 * 6 = 6分鐘
		int picRemainMinutes = 15;
		int videoRemainMinutes = 15; //无论超出容量限制都要最少保留15分钟的video，预设必须考虑最少可以存储15分钟的影片
		int64_t storageLimitedbytes = 3 * 102400000;  //102400 kb //100m
		bool ffmpegOpenInfo = false;
		// hard device accelate
		std::string nHDType = "kHWDeviceTypeNone"; //硬件类型 默认是没有
		std::string remarks = "The node is used to configure the recording system and hardware environment";
		void LoadFromJson(rapidjson::Value& value);
	};

	struct Database
	{
		std::string strIp = "127.0.0.1";
		int port = 3306;
		std::string strFrontBase = "camera_guard_business";
		std::string strHistoryBase = "camera_guard_history";
		std::string stUser = "root(default)";
		std::string strPassword = "root(default)";
		std::string strConnFrontendBase;
		std::string strConnHistoryBase;
		void LoadFromJson(rapidjson::Value& value);
	};

	struct PoolConfig
	{
		int database_pool = 50;
		void LoadFromJson(rapidjson::Value& value);
	};

	struct CarPlateRecogBusiness
	{
		bool enable = false;
		std::string http_server_api; // = "http://localhost:5000/";
		std::string http_detect_server_api; // = "http://localhost:5600/detect";
		double threshold = 0.7;
		std::string user = "admin";
		std::string password = "123456";
		std::string remarks = "for card plate recognition,post to detct server and return the result and then post back to http server";
		PermitedCameraList PermitedCamList;
		void LoadFromJson(rapidjson::Value& value);
		void LoadCamListFromJson(rapidjson::Value& value, PermitedCameraList& permitedCameraList);
	};


	struct ConfigInfo
	{
		Device cfgDevice;
		HttpServerCloud cfgHttpServerCloud;
		HttpServer cfgHttpServer;
		StorageServer cfgStorageServer;
		GlobalSetting cfgGlobalSetting;
		Database cfgDatabase;
		PoolConfig cfgPool;
		CarPlateRecogBusiness cfgCarPlateRecogBusiness;
	};

private:
	DeviceConfig() { Init(); };
	std::string read_file(const std::string& strFile);
	bool parse_config(const std::string& datrData);


public:
	static DeviceConfig& Instance()
	{
		static DeviceConfig g_Instance;
		return g_Instance;
	}
	bool Init();
	ConfigInfo GetConfig() { return config; }
	void getcpuid(unsigned int* CPUInfo, unsigned int InfoType);
	void get_cpuidex(unsigned int* CPUInfo, unsigned int InfoType, unsigned int ECXValue);
private:
	ConfigInfo config;
};


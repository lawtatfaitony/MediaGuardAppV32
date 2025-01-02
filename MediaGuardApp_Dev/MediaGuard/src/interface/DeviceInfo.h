#pragma once
#include <string>
#ifdef _WIN32
#include <Time.h> 
#elif __linux__
// 请自己补linux内容
#endif
#include <sstream> 
namespace Service
{
	// api DeviceInfo For Api
	struct DeviceInfoStruct
	{
		std::string deviceId = "";
		std::string sysModuleId = 0;
		std::string deviceName;
		int deviceEntryMode = -1;
		std::string deviceSerialNo = "";
		std::string config = "";
		std::string mainComId = "";
		std::string operatedUser = "";
		time_t  updateDateTime = time(0);
		int status = 0;
		std::string siteId = "";
		bool isReverseHex = false;
	};
}

//#define CFG_HTTP_SERVER_CLOUD CAMERA_GUARD_CONFIG.cfgHttpServerCloud

class DeviceInfo
{
public:
	DeviceInfo();
	virtual ~DeviceInfo();
	std::string device_info_api_url;
private:
};
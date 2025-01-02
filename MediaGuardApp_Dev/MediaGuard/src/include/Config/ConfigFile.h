/**************************************************************
* @brief:       配置文件变量定义 
* @copyright:
*
***************************************************************/
#pragma once
#include <string>
 
namespace Config
{
 
	static std::string kConfigDir = "conf";
	static std::string kDeviceName = "device.json";
	static std::string kConfigName = "config.json"; //作廢, 這個配置文件改為 device.json
	static std::string kCameraList = "camera_list.json";
	 

	/************************************************************************
	*                                   config                              *
	*************************************************************************/
	static std::string GetConfigDir()
	{
		return kConfigDir;
	}

	static std::string GetConfigFile()
	{ 
		fs::path app_root = File::GetWorkPath();
		std::string conf_folder = GetConfigDir();
		fs::path confConfigFilePath = app_root / conf_folder / kConfigName;
		std::string config_path = confConfigFilePath.string();
		return  config_path;
	}
	static std::string GetDeviceConfigFile()
	{
		fs::path app_root = File::GetWorkPath();
		std::string conf_path = GetConfigDir(); 
		fs::path deviceConfigFile = app_root / conf_path / kDeviceName;
		std::string device_config_path = deviceConfigFile.string();
		return  device_config_path;
	}
	/* 獲取 conf/camera_list.json 詳細路徑 */
	static std::string GeCamerListConfigFile()
	{
		fs::path app_root = File::GetWorkPath();
		std::string conf_path = GetConfigDir();
		fs::path camera_list_json_path = app_root / conf_path/ kCameraList; 
		return  camera_list_json_path.string();
	} 

}

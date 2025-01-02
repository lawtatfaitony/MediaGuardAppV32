#pragma once 
#include "CameraMpeg.h"

CameraMpeg::CameraMpeg()
{
	//api url---------------------------------------------------------------------------------------------------------------
	api_add_camera_mpeg;
	{
		std::stringstream ss1;
		ss1 << DEVICE_CONFIG.cfgHttpServerCloud.url << "CamMpeg/AddCameraMpeg";
		api_add_camera_mpeg = ss1.str();
	}

	api_camera_stream_info_list;
	{
		std::stringstream ss2;
		ss2 << DEVICE_CONFIG.cfgHttpServerCloud.url << "CamMpeg/CameraStreamInfoList";
		api_camera_stream_info_list = ss2.str();
	}
	//通过DeviceId获得镜头列表
	api_camera_list;
	{
		std::stringstream ss3;
		ss3 << DEVICE_CONFIG.cfgHttpServerCloud.url << DEVICE_CONFIG.cfgDevice.language_code << "/Cam/CameraList";
		api_camera_list = ss3.str();
	}
}
CameraMpeg::~CameraMpeg()
{
}
//http://rapidjson.org/zh-cn/

namespace JsonHelper
{
	static void to_json(const Service::CameraMpegInfo& cameraMpegInfo, rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator)
	{
		value.AddMember("recordId", cameraMpegInfo.RecordId, allocator);
		value.AddMember("cameraId", cameraMpegInfo.CameraId, allocator);
		value.AddMember("isGroup", cameraMpegInfo.IsGroup, allocator);

		value.AddMember("deviceSerialNo", rapidjson::Value(cameraMpegInfo.DeviceSerialNo.c_str(), allocator), allocator);
		value.AddMember("mpegFileName", rapidjson::Value(cameraMpegInfo.MpegFileName.c_str(), allocator), allocator);
		value.AddMember("fileFormat", rapidjson::Value(cameraMpegInfo.FileFormat.c_str(), allocator), allocator);

		value.AddMember("fileSize", cameraMpegInfo.FileSize, allocator);
		value.AddMember("startTimestamp", cameraMpegInfo.StartTimestamp, allocator);
		value.AddMember("endTimestamp", cameraMpegInfo.EndTimestamp, allocator);
	}
}

/*
  http://81.71.xx.yy:5002/Authentication/requestToken
  获取cloud Token
*/
bool CameraMpeg::request_token(std::string& token)
{
	if (Service::TokenRet.TokenExpiredTimeOut != 0)
	{
		int64_t now = Time::GetMilliTimestamp();
		if (Service::TokenRet.TokenExpiredTimeOut > now && Service::TokenRet.AccessToken != "")
		{ 
			token = Service::TokenRet.AccessToken;
			return true;
		}
	}

	bool ret = false;
	std::string strRst;
	LibcurlHelper clientCurl;
	HttpPara para;
	para.nTransTimeout = 5000;
	para.nConnectTimeout = 3000;
	//http://81.71.xx.yy:5002/Authentication/requestToken
	para.strUrl;
	{
		std::stringstream s1;
		s1 << DEVICE_CONFIG.cfgHttpServerCloud.url << "Authentication/requestToken";
		para.strUrl = s1.str();
	}

	std::string strBodyJson = "{\"userName\":\"admin\",\"password\":\"0192023A7BBD73250516F069DF18B500\"}";
	{
		std::stringstream sAuth;
		sAuth << "{\"userName\":\"" << DEVICE_CONFIG.cfgDevice.user << "\",\"password\":\"" << DEVICE_CONFIG.cfgDevice.password << "\"}";
		strBodyJson = sAuth.str();
	}
	int nCode = clientCurl.Post(para, strBodyJson, strRst);

	rapidjson::Document doc;
	JS_PARSE_OBJECT_RETURN(doc, strRst, false);
 

	if (doc.HasMember("meta") && doc["meta"].IsObject())
	{
		Service::Meta meta;
		JS_PARSE_OPTION(meta.success, doc["meta"], Bool, success);
		JS_PARSE_OPTION(meta.message, doc["meta"], String, message);
		JS_PARSE_OPTION(meta.errorCode, doc["meta"], Int, errorCode);

		if (meta.success == true)
		{
			ret = meta.success;
			if (doc.HasMember("data") && doc["data"].IsObject())
			{
				//token
				JS_PARSE_OPTION(token, doc["data"], String, accessToken);
				token = "Bearer " + token;
				Service::TokenRet.TokenReturn::AccessToken = token;

				if (doc.HasMember("data") && doc["data"].IsObject())
				{
					rapidjson::Value& data_doc = doc["data"];

					if (data_doc.HasMember("authProfile") && data_doc["authProfile"].IsObject())
					{
						int64_t auth_time = 0;
						std::string user_name = "";
						JS_PARSE_OPTION(auth_time, data_doc["authProfile"], Int64, accessExpiration);
						JS_PARSE_OPTION(user_name, data_doc["authProfile"], String, userName);

						Service::TokenRet.TokenExpiredTimeOut = auth_time;
						Service::TokenRet.UserName = user_name;
					}
				}
				return ret;
			}
		}
		else
		{
			//TGET TOKEN FAIL
			//LOG(INFO) << "[META::SUCCESS=FALSE] func::request_token get token result \n" << strRst;
			std::cout <<   "[META::SUCCESS=FALSE] func::request_token get token result \n"  << std::endl;
		}
	}
	else
	{
		//TGET TOKEN FAIL
		//LOG(INFO) << "[FAIL] func::request_token get token result \n" << strRst;
		std::cout << "[FAIL] func::request_token get token result \n" << std::endl;
	}
	return ret;
}

void CameraMpeg::camera_mpeg_add(Service::CameraMpegInfo& cameraMpegInfo)
{
	std::string strMsg;
	LibcurlHelper clientCurl;
	HttpPara para;
	para.strUrl = api_add_camera_mpeg;// "http://127.0.0.1:5000/CamMpeg/AddCameraMpeg";

	/*std::string andStr = "&";
	std::string equalStr = "=";*/

	bool parseJsonResult = camera_mpeg_to_json(cameraMpegInfo, strMsg);
	if (!parseJsonResult)
	{
		//XXXXLOG(INFO) << "camera_mpeg_to_json" << std::endl;
	}
	std::string strResponse;
	int nCode = clientCurl.Post(para, strMsg, strResponse);

	if (nCode != CP_OK)
	{
		//LOG(INFO) << "camera_mpeg_add post resturn error code " << strResponse;
		std::cout << "camera_mpeg_add post resturn error code " << strResponse << std::endl;
	}
}

/* 添加錄像文件信息到雲端 RtspStreamHandle.cpp 每x分鐘解碼保存的mp4/flv文件信息 */
bool CameraMpeg::camera_mpeg_add2(Service::CameraMpegInfo& cameraMpegInfo, std::string& strResponse)
{ 
	std::stringstream ss;
	{
		ss << DEVICE_CONFIG.cfgHttpServer.url << "CamMpeg/AddCameraMpeg";
	}
	std::string strMsg;
	LibcurlHelper clientCurl;
	HttpPara para;
	para.strUrl = ss.str();
	para.nConnectTimeout = 1000;
	para.nTransTimeout = 3000;

	std::string userName = DEVICE_CONFIG.cfgDevice.user;
	std::string password = DEVICE_CONFIG.cfgDevice.password;
	std::stringstream userJson;
	{
		userJson << "{\"userName\": \"" << userName << "\", \"password\" : \"" << password << "\"}";
	}
	std::string bear_token;
	bool succ = request_token(bear_token);
	if (!succ)
	{
		//LOG(TRACE) << "camera_mpeg_add2::get_api_token " << succ << " FAIL";
		std::cout << "camera_mpeg_add2::get_api_token " << succ << " FAIL" << std::endl;
		return false;
	}
	para.Authorization = bear_token;

	bool mpeg_to_json_succ = camera_mpeg_to_json(cameraMpegInfo, strMsg);
	if (!mpeg_to_json_succ)
	{
		//LOG(TRACE) << "camera_mpeg_add2::camera_mpeg_to_json " << mpeg_to_json_succ << " FAIL";
		std::cout << "camera_mpeg_add2::camera_mpeg_to_json " << mpeg_to_json_succ << " FAIL" << std::endl;
		return false;
	}
	int nCode = clientCurl.Post(para, strMsg, strResponse);

	if (nCode != CP_OK)
	{
		//LOG(ERROR) << "camera_mpeg_add post resturn error code: " << nCode << "\n" << para.strUrl << "\n POST JSON \n" << strMsg << "\n" << strResponse;
		std::cout << "camera_mpeg_add post resturn error code: " << nCode << "\n" << para.strUrl << "\n POST JSON RESULT :\n" << strMsg << "\n" << strResponse << std::endl;
	}

	return true;
}

/* 更新設備公網IP到雲端 Request 
{
    "deviceId":"3012",
    "internetIp":"219.77.12.21",
    "internetPort":"8080",
    "localIp":"192.168.0.188",
    "localPort":"8080"
} 
*/
bool CameraMpeg::update_divice_internet_ip(Service::DeviceInterNetIpInfo& deviceInterNetIpInfo, std::string& strResponse)
{
	std::stringstream ss;
	{
		ss << DEVICE_CONFIG.cfgHttpServer.url << DEVICE_CONFIG.cfgDevice.language_code << "/Device/UpdateDiviceInternetIp";
	}
	std::string strMsg;
	LibcurlHelper clientCurl;
	HttpPara para;
	para.strUrl = ss.str();
	para.nConnectTimeout = 1000;
	para.nTransTimeout = 3000;
	  
	std::string bear_token;  
	bool succ = request_token(bear_token);
	if (!succ)
	{
		//LOG(TRACE) << "UpdateDiviceInternetIp::get_api_token " << succ << " FAIL";
		std::cout << "UpdateDiviceInternetIp::get_api_token " << succ << " FAIL" << std::endl;
		return false;
	}
	para.Authorization = bear_token;

	bool mpeg_to_json_succ = device_internet_ip_info_to_json(deviceInterNetIpInfo, strMsg);
	if (!mpeg_to_json_succ)
	{
		//LOG(TRACE) << "UpdateDiviceInternetIp::camera_mpeg_to_json " << mpeg_to_json_succ << " FAIL";
		std::cout << "UpdateDiviceInternetIp::camera_mpeg_to_json " << mpeg_to_json_succ << " FAIL" << std::endl;
		return false;
	}
	int nCode = clientCurl.Post(para, strMsg, strResponse);

	if (nCode != CP_OK)
	{
		//LOG(ERROR) << "UpdateDiviceInternetIp post resturn error code: " << nCode << "\n" << para.strUrl << "\n POST JSON \n" << strMsg << "\n" << strResponse;
		std::cout << "UpdateDiviceInternetIp post resturn error code: " << nCode << "\n" << para.strUrl << "\n POST JSON  RESULT :\n" << strMsg << "\n" << strResponse << std::endl;
	}

	return true;
}

/*
{
  "recordId": 0,
  "cameraId": 0,
  "deviceSerialNo": "string",
  "isGroup": false,
  "mpegFileName": "string",
  "fileSize": 0,
  "fileFormat": "string",
  "startTimestamp": 0,
  "endTimestamp": 0
}
*/
bool CameraMpeg::camera_mpeg_to_json(Service::CameraMpegInfo& cameraMpegInfo, std::string& strResult)
{
	try
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();

		//1 recordId
		rapidjson::Value recordIdVal;
		recordIdVal.SetInt64(0);
		doc.AddMember("recordId", cameraMpegInfo.RecordId, typeAllocate);

		//2 cameraId
		rapidjson::Value cameraIdVal;
		cameraIdVal.SetInt(cameraMpegInfo.CameraId);
		doc.AddMember("cameraId", cameraIdVal, typeAllocate);

		//3 deviceSerialNo
		doc.AddMember("deviceSerialNo", rapidjson::Value(cameraMpegInfo.DeviceSerialNo.c_str(), typeAllocate), typeAllocate);

		//4 isGroup
		rapidjson::Value IsGroupVal;
		IsGroupVal.SetBool(true);
		doc.AddMember("isGroup", IsGroupVal, typeAllocate);

		//5 mpegFileName
		doc.AddMember("mpegFileName", rapidjson::Value(cameraMpegInfo.MpegFileName.c_str(), typeAllocate), typeAllocate);

		//6 fileSize
		rapidjson::Value fileSizeVal;
		fileSizeVal.SetInt64(cameraMpegInfo.FileSize);
		doc.AddMember("fileSize", fileSizeVal, typeAllocate);

		//7 fileFormat
		doc.AddMember("fileFormat", rapidjson::Value(cameraMpegInfo.FileFormat.c_str(), typeAllocate), typeAllocate);

		//8 startTimestamp
		rapidjson::Value startTimestampVal;
		startTimestampVal.SetInt64(cameraMpegInfo.StartTimestamp);
		doc.AddMember("startTimestamp", startTimestampVal, typeAllocate);

		//9 endTimestamp
		rapidjson::Value endTimestampVal;
		endTimestampVal.SetInt64(cameraMpegInfo.EndTimestamp);
		doc.AddMember("endTimestamp", endTimestampVal, typeAllocate);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
		doc.Accept(writer);
		strResult = buffer.GetString();
		return true;
	}
	catch (std::exception& e)
	{
		//LOG(ERROR) << "[CameraMpeg::camera_mpeg_to_json] " << e.what() << std::endl;
		std::cout << "[CameraMpeg::camera_mpeg_to_json] " << e.what() << std::endl;
		return false;
	}
}

bool CameraMpeg::device_internet_ip_info_to_json(Service::DeviceInterNetIpInfo& deviceInterNetIpInfo, std::string& strResult)
{
	try
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();

		//1 deviceId
		rapidjson::Value deviceIdVal;  
		doc.AddMember("deviceId", rapidjson::Value(deviceInterNetIpInfo.deviceId.c_str(), typeAllocate), typeAllocate);

		//2 internetIp
		rapidjson::Value cameraIdVal; 
		doc.AddMember("internetIp", rapidjson::Value(deviceInterNetIpInfo.internetIp.c_str(), typeAllocate), typeAllocate);

		//3 internetPort
		doc.AddMember("internetPort", rapidjson::Value(deviceInterNetIpInfo.internetPort.c_str(), typeAllocate), typeAllocate);

		//4 localIp
		rapidjson::Value IsLocalIpVal;
		doc.AddMember("localIp", rapidjson::Value(deviceInterNetIpInfo.localIp.c_str(), typeAllocate), typeAllocate);

		//5 localPort
		doc.AddMember("localPort", rapidjson::Value(deviceInterNetIpInfo.localPort.c_str(), typeAllocate), typeAllocate);
		  
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
		doc.Accept(writer);
		strResult = buffer.GetString();
		return true;
	}
	catch (std::exception& e)
	{
		//LOG(ERROR) << "[CameraMpeg::camera_mpeg_to_json] " << e.what() << std::endl;
		std::cout << "[CameraMpeg::camera_mpeg_to_json] " << e.what() << std::endl;
		return false;
	}
}

bool CameraMpeg::device_serial_no_input_to_json(Service::DeviceSerialNoInput& deviceSerialNoInput, std::string& strResult)
{
	try
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();

		doc.AddMember("deviceSerialNo", rapidjson::Value(deviceSerialNoInput.DeviceSerialNo.c_str(), typeAllocate), typeAllocate);
		doc.AddMember("deviceToken", rapidjson::Value(deviceSerialNoInput.DeviceToken.c_str(), typeAllocate), typeAllocate);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
		doc.Accept(writer);
		strResult = buffer.GetString();
		return true;
	}
	catch (...)
	{
		strResult = "";
		return false;
	}
}

/*
通过设备序列号和Hamc By year的TOKEN组成的授权查询镜头列表
是早前Ver0版本 现在不用，但保留，目标是和Media Dvr 一致的API
*/
bool CameraMpeg::get_camera_list(std::string& strRst)
{
	std::string  device_serial_no;
	std::string strMsg;
	LibcurlHelper clientCurl;
	HttpPara para;
	para.strUrl = api_camera_stream_info_list; // "https://127.0.0.1:5001/CamMpeg/CameraStreamInfoList";

	if (device_serial_no.empty())
	{
		device_serial_no = DEVICE_CONFIG.cfgDevice.device_serial_no;
	}
	CHMAC_SHA1 chs;
	std::string key = chs.GetCurrentYear();//Exact match to year  // 1: chs.GetCurrentYear();  2 :chs.GetCurrentSystemTimeLong();
	std::string device_token = chs.hmac_sha1(device_serial_no, key); //device_token = DEVICE_CONFIG.cfgDevice.device_serial_no_hmac;

	Service::DeviceSerialNoInput deviceSerialNoInput;
	deviceSerialNoInput.DeviceSerialNo = device_serial_no;
	deviceSerialNoInput.DeviceToken = device_token;

	bool parseJsonResult = device_serial_no_input_to_json(deviceSerialNoInput, strMsg);
	std::cout << "\n\n Input Paras =\n " << strMsg << std::endl;

	if (!parseJsonResult)
	{
		//LOG(INFO) << "get_camera_list parseJsonResult = " << parseJsonResult << std::endl;
		std::cout << "get_camera_list parseJsonResult = " << parseJsonResult << std::endl;
	}
	int nCode = clientCurl.Post(para, strMsg, strRst);
	//TEST
	//nCode = 1;
	//strRst = "{\"meta\":{\"success\":true,\"message\":\"OK\",\"errorCode\":-1},\"data\":[{\"cameraId\": 17,\"refCount\": 0,\"input\": \"rtsp://admin:hik12345@192.168.0.92:554/h264/ch1/main/av_stream\",\"output\": \"rtmp://127.0.0.1:1935/live/1\",\"savePic\": true,\"saveVideo\": false,\"rtmp\": false,\"width\": 1280,\"height\": 720,\"PixFmt\": -1,\"HDType\": 25,\"frameRate\": 25,\"videoIndex\": -1,\"audioIndex\": -1,\"videoTime\": 900}]}";
	//std::cout << "\n\n clientCurl.Post return strRst =\n " << strRst << std::endl;

	return CP_OK;
}

/*
★★★★★通过设备序列号获取镜头列表，和Media的标准接口一致，参考doc :DVR开发 2023-2-28
return CURLE_OK=0  fail=CURLE_FAILED_INIT=2
*/
bool CameraMpeg::camera_list(Service::StreamInfoApiList& streamInfoApiList)
{
	std::string strRst;  //from api or json file 的整體內容 雲端或本地(/conf/camera_list.json) 獲取
	// 獲取雲端 API Token
	std::string token1;

	std::string web_server_url = DEVICE_CONFIG.cfgHttpServer.url;
	Service::CameraListInput cameraListInput;
	std::string strMsg;
	LibcurlHelper clientCurl;
	HttpPara para;
	para.strUrl = api_camera_list; // "/{Language}/Cam/CameraList";

	cameraListInput.MaincomId = DEVICE_CONFIG.cfgDevice.maincom_id;

	//向雲端獲取Camerlist之前判斷設備是否設置離線,如果是則指向 if業務邏輯(獲取本地設備保存的camera_list)
	if (DEVICE_CONFIG.cfgDevice.device_is_online_always == false)
	{
		std::string camera_list_path_file = Config::GeCamerListConfigFile();
		strRst = File::readJsonFile(camera_list_path_file);
	}
	else
	{
		Service::DeviceDetails deviceDetails;
		bool ret = device_by_serial_no(deviceDetails);
		if (ret == true)
		{
			cameraListInput.DeviceId = deviceDetails.deviceId;
		}
		else {
			return false;
		}

		bool parseRes = camera_list_input_to_json(cameraListInput, strMsg);

		if (!parseRes)
		{
			//LOG(INFO) << "camera_list_input_to_json parseRes = " << parseRes << std::endl;
			std::cout << "camera_list_input_to_json parseRes = " << parseRes << std::endl;
			return false;
		}

		
		bool res = request_token(token1);
		if (res) { 
			para.Authorization = token1;
		}
		else {
			//LOG(INFO) << "func::request_token(token1) get token fail! ";
			std::cout <<  "func::request_token(token1) get token fail! " << std::endl;
			return false;
		}

		int ncode_camera_list = clientCurl.Post(para, strMsg, strRst);

		//CURLE_OK = ncode_camera_list = 0 则获取成功 strRst ::camera list
		if (ncode_camera_list != CURLE_OK)
		{
			std::cout << " get camera list by post  fail: " << para.strUrl;
			return CURLE_FAILED_INIT;
		}
		else {
			//成功返回json camera list 
			std::stringstream saveJPathFile;
			{  saveJPathFile << File::GetWorkPath() << kSeprator << "conf" << kSeprator  << "camera_list.json"; }

			bool saveJsonResult = File::saveJsonFile(strRst, saveJPathFile.str());
			if (!saveJsonResult)
			{
				//LOG(INFO) << "File::saveJsonFile : Save ncode_camera_list.json fail: " << saveJPathFile.str();
				std::cout << "File::saveJsonFile : Save ncode_camera_list.json fail: " << saveJPathFile.str() << std::endl;
				return CURLE_FAILED_INIT;
			}
		}
	}
	  
	rapidjson::Document doc;
	JS_PARSE_OBJECT_RETURN(doc, strRst, false);
	if (doc.HasMember("meta") && doc["meta"].IsObject())
	{
		Service::Meta meta;
		JS_PARSE_OPTION(meta.success, doc["meta"], Bool, success);
		JS_PARSE_OPTION(meta.message, doc["meta"], String, message);
		JS_PARSE_OPTION(meta.errorCode, doc["meta"], Int, errorCode);

		if (meta.success == true)
		{
			 bool ret = meta.success;

			if (doc.HasMember("data"))
			{
				const rapidjson::Value& a = doc["data"];

				if (a.IsArray())
				{
					for (rapidjson::SizeType i = 0; i < a.Size(); i++)
					{
						Service::StreamInfoForApi  streamInfo;

						JS_PARSE_OPTION(streamInfo.nCameraId, a[i], Int, cameraId);
						JS_PARSE_OPTION(streamInfo.strInput, a[i], String, rtsp);

						//鏡頭狀態 0:停用;  1:啟用
						int camera_state = 1;
						JS_PARSE_OPTION(camera_state, a[i], Int, state);
						streamInfo.cameraStatus = camera_state == 1;
						 
						//获取镜头设置 雲端或者本地
						Service::CamSettingNSchedule cam_setting;
						auto stream_camera_id = std::to_string(streamInfo.nCameraId);
						bool get_cam_setting_succ = setting_n_schedule_by_camera_id(stream_camera_id, token1, cam_setting);
						if (get_cam_setting_succ == false) {
							try {
								//LOG(ERROR) << "GET CAMER CAMERA SCHEDULE CONFIG JSON TO OBJ FAIL (func::camera_list<setting_n_schedule_by_camera_id).camera id = " << std::to_string(streamInfo.nCameraId);
								std::cout << "GET CAMER CAMERA SCHEDULE CONFIG JSON TO OBJ FAIL (func::camera_list<setting_n_schedule_by_camera_id).camera id = " << std::to_string(streamInfo.nCameraId) << std::endl;
							}
							catch (...)
							{
								printf("func::setting_n_schedule_by_camera_id(..) fail...");
							}
							continue;
						}

						//No.1 ★★★★★ 使用什麼硬件類型解碼 從本地設備配置獲得(device.json->nHDType) 
						std::string strHdType = DEVICE_CONFIG.cfgGlobalSetting.nHDType; 
						/*device.config 全局设置是字符串形式，下列一一对应到 ENUM : HWDeviceType*/
						if (strHdType == "kHWDeviceTypeCUDA")  //Navida显卡解码 ★★★★★
							streamInfo.nHDType = 2;
						else if (strHdType == "kHWDeviceTypeNone") //没有硬件解码 ★
							streamInfo.nHDType = 0;
						else if (strHdType == "kHWDeviceTypeVDPAU")
							streamInfo.nHDType = 1;
						else if (strHdType == "kHWDeviceTypeVAAPI")
							streamInfo.nHDType = 3;
						else if (strHdType == "kHWDeviceTypeDXVA2") //INTEL 集成顯卡 ★
							streamInfo.nHDType = 4;
						else if (strHdType == "kHWDeviceTypeQSV")
							streamInfo.nHDType = 5;
						else if (strHdType == "kHWDeviceTypeVIDEOTOOLBOX")
							streamInfo.nHDType = 6;
						else if (strHdType == "kHWDeviceTypeD3D11VA")
							streamInfo.nHDType = 7;
						else if (strHdType == "kHWDeviceTypeDRM")
							streamInfo.nHDType = 8;
						else if (strHdType == "kHWDeviceTypeOPENCL") //OPENCL 集成顯卡 ★
							streamInfo.nHDType = 9;
						else if (strHdType == "kHWDeviceTypeMEDIACODEC")
							streamInfo.nHDType = 10;

						streamInfo.rtspIp = cam_setting.CameraIp;
						streamInfo.bSavePic = cam_setting.SavePic;						//No.4     // 保存圖片到本地硬盤
						streamInfo.mediaFormate = cam_setting.SaveMpeg4 == true ? (int)MediaFormate::MPEG : (int)MediaFormate::FLV;							
																						//No.5  //存储为MPEG=0 
						streamInfo.savePictRate = cam_setting.SavePictRate;				//No.7 //帧率/savePictRate = 保存的频率 每25帧保存5帧
						streamInfo.bSaveVideo = cam_setting.SaveVideo;					//No.8   // save video  
						streamInfo.nVideoTime = DEVICE_CONFIG.cfgGlobalSetting.recordTimeMinutes;	
																						//No.9     //录像时间 time for each video 
						streamInfo.bRtmp = cam_setting.RtmpOutput;						//No.X	開啟Rtmp

						if (cam_setting.HlsOutput)
							streamInfo.nStreamDecodeType = StreamDecodeType::HLS;	//No.10 bRtmp 改为解码流类型
						else if (cam_setting.RtmpOutput == false)					//目前不启动Rtmp,如果不输出HLS则不输出流
							streamInfo.nStreamDecodeType = StreamDecodeType::NOSTREAM;

						std::stringstream output_s;	//No.11 HLS 输出流
						output_s << web_server_url << "hls/" << std::to_string(streamInfo.nCameraId) << "/index.m3u8";
						streamInfo.strOutput = output_s.str();

						streamInfo.nWidth = 0;								//No.12 //system default value  
						streamInfo.nHeight = 0;								//No.13 //system default value
						streamInfo.nPixFmt = AV_PIX_FMT_NONE;				//No.14 //system default value // AV_PIX_FMT_NONE = -1,

						streamInfo.nFrameRate = 25;							//No.15 //帧率 只用于整除计算多少帧保存一次图片 system default value
						streamInfo.nVideoIndex = -1;						//No.16 //system default value
						streamInfo.nAudioIndex = -1;						//No.17 //system default value
						streamInfo.nRefCount = 0;

						streamInfoApiList.push_back(streamInfo);
					}
				}
			}
		}
	}
	return CP_OK;
}

/// <summary>
/// 把API接口的參數對象轉換為本地的參數對象 
/// </summary>
/// <param name="streamInfoApiList"></param>
/// <param name="streamInList"></param>
/// <returns></returns>
bool CameraMpeg::camera_list_trans_to_strean_info(Service::StreamInfoApiList& sList, Service::StreamInList& streamInList)
{
	//false = camera_list無記錄
	if (sList.size() == 0)
		return false;

	std::list<Service::StreamInfoForApi>::iterator itr;
	for (itr = sList.begin(); itr != sList.end(); ++itr)
	{
		StreamInfo streamInfo;
		streamInfo.nCameraId = itr->nCameraId;
		streamInfo.rtspIp = itr->rtspIp;
		streamInfo.nHDType = itr->nHDType;  
		streamInfo.strInput = itr->strInput;
		streamInfo.bSavePic = itr->bSavePic;
		streamInfo.mediaFormate = itr->mediaFormate;
		streamInfo.savePictRate = itr->savePictRate;
		streamInfo.bSaveVideo = itr->bSaveVideo;
		streamInfo.nVideoTime = itr->nVideoTime;
		streamInfo.nStreamDecodeType = itr->nStreamDecodeType;
		streamInfo.strOutput = itr->strOutput;
		streamInfo.nWidth = itr->nWidth;
		streamInfo.nHeight = itr->nHeight;
		streamInfo.nPixFmt = itr->nPixFmt;
		streamInfo.nFrameRate = itr->nFrameRate;
		streamInfo.nVideoIndex = itr->nVideoIndex;
		streamInfo.nAudioIndex = itr->nAudioIndex;
		streamInfo.nRefCount = itr->nRefCount;
		streamInfo.bRtmp = itr->bRtmp;  //不開啟rtmp,
		streamInList.push_back(streamInfo);
	}
	return true;
}

bool CameraMpeg::camera_list_input_to_json(Service::CameraListInput& input, std::string& strResult)
{
	try
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();

		doc.AddMember("deviceId", rapidjson::Value(input.DeviceId.c_str(), typeAllocate), typeAllocate);
		doc.AddMember("maincomId", rapidjson::Value(input.MaincomId.c_str(), typeAllocate), typeAllocate);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
		doc.Accept(writer);
		strResult = buffer.GetString();
		return true;
	}
	catch (...)
	{
		strResult = "";
		return false;
	}
}

/*
  /{Language}/Device/GetMainComBySerialNo/{SerialNo}
  获取设备详情 并返回 DeviceDetails 结构体
  這裡是獲取系統載體的設備明細 不是掛載的設備
  設備序列號來自 DEVICE_CONFIG.cfgDevice.device_serial_no
  注意 這裡是應用程序運行的平台設備(如MediaGuard + Liux 合成成為設備) 不是APP 下掛的設備
*/
bool CameraMpeg::device_by_serial_no(Service::DeviceDetails& deviceDetails)
{
	bool ret = false;

	Service::CameraListInput cameraListInput;

	LibcurlHelper clientCurl;
	HttpPara para;
	// "/{Language}/Device/GetMainComBySerialNo/{SerialNo}";
	para.strUrl;
	{
		std::stringstream s1;
		s1 << DEVICE_CONFIG.cfgHttpServerCloud.url << DEVICE_CONFIG.cfgDevice.language_code << "/Device/GetMainComBySerialNo/" << DEVICE_CONFIG.cfgDevice.device_serial_no;
		para.strUrl = s1.str();
	}

	std::string strRst; 
	std::stringstream ss_path_file;
	{  ss_path_file << File::GetWorkPath() << kSeprator << "conf" << kSeprator << "main_company_details_by_serialno.json"; }
	std::string mainCompanyDetailsJsonFile = ss_path_file.str();

	if (DEVICE_CONFIG.cfgDevice.device_is_online_always == false)
	{
		// 通過設備序列號 成功返回json GetMainComBySerialNo 
		strRst = File::readJsonFile(mainCompanyDetailsJsonFile);
		if (strRst=="")
		{
			//LOG(INFO) << "CameraMpeg::device_by_serial_no File::readJsonFile device_by_serial_no : [READ] main_company_details_by_serialno.json fail: " << mainCompanyDetailsJsonFile;
			std::cout << "CameraMpeg::device_by_serial_no File::readJsonFile device_by_serial_no : [READ] main_company_details_by_serialno.json fail: " << mainCompanyDetailsJsonFile << std::endl;
		}
	}
	else {
		
		int ncode = clientCurl.Get(para, strRst);
		//TEST 
		//std::cout << "\n\n clientCurl.Post return strRst =\n " << strRst << std::endl;

		if (ncode != CURLE_OK)
		{
			//LOG(INFO) << "File::device_by_serial_no : Save main_company_details_by_serialno.json fail: " << mainCompanyDetailsJsonFile;
			std::cout << "File::device_by_serial_no : Save main_company_details_by_serialno.json fail: " << mainCompanyDetailsJsonFile << std::endl;
			return CURLE_FAILED_INIT;
		}
		else {
			//成功返回json camera list
			std::string saveJPathFile = mainCompanyDetailsJsonFile;
			bool saveJsonResult = File::saveJsonFile(strRst, saveJPathFile);
			if (!saveJsonResult)
			{
				//LOG(INFO) << "CameraMpeg::device_by_serial_no File::readJsonFile device_by_serial_no : [SAVE] main_company_details_by_serialno.json fail: " << saveJPathFile;
				std::cout << "CameraMpeg::device_by_serial_no File::readJsonFile device_by_serial_no : [SAVE] main_company_details_by_serialno.json fail: " << saveJPathFile << std::endl;
				return CURLE_FAILED_INIT;
			}
		}
	}
	 
	rapidjson::Document doc;
	JS_PARSE_OBJECT_RETURN(doc, strRst, false);

	if (doc.HasMember("meta") && doc["meta"].IsObject())
	{
		Service::Meta meta;
		JS_PARSE_OPTION(meta.success, doc["meta"], Bool, success);
		JS_PARSE_OPTION(meta.message, doc["meta"], String, message);
		JS_PARSE_OPTION(meta.errorCode, doc["meta"], Int, errorCode);

		if (meta.success == true)
		{
			ret = meta.success;

			if (doc.HasMember("data") && doc["data"].IsObject())
			{
				//deviceDetails
				JS_PARSE_OPTION(deviceDetails.deviceId, doc["data"], String, deviceId);
				JS_PARSE_OPTION(deviceDetails.sysModuleId, doc["data"], String, sysModuleId);
				JS_PARSE_OPTION(deviceDetails.deviceName, doc["data"], String, deviceName);
				JS_PARSE_OPTION(deviceDetails.deviceEntryMode, doc["data"], Int, deviceEntryMode);
				JS_PARSE_OPTION(deviceDetails.deviceSerialNo, doc["data"], String, deviceSerialNo);
				JS_PARSE_OPTION(deviceDetails.config, doc["data"], String, config);
				JS_PARSE_OPTION(deviceDetails.mainComId, doc["data"], String, mainComId);
				JS_PARSE_OPTION(deviceDetails.operatedUser, doc["data"], String, operatedUser);
				JS_PARSE_OPTION(deviceDetails.updateDateTime, doc["data"], String, updateDateTime);
				JS_PARSE_OPTION(deviceDetails.status, doc["data"], Int, status);
				JS_PARSE_OPTION(deviceDetails.siteId, doc["data"], Int, siteId);
				JS_PARSE_OPTION(deviceDetails.isReverseHex, doc["data"], Bool, isReverseHex);

				return ret;
			}
		}
	}
	return ret;
}

/*
  http://81.71.xx.yy:5002/Cam/ReturnCamerScheduleConfigJson/12
  获取镜头配置策略
*/
bool CameraMpeg::setting_n_schedule_by_camera_id(std::string& camera_id, std::string token, Service::CamSettingNSchedule& cam_set)
{
	bool ret = false;
	Service::CameraListInput cameraListInput;
	LibcurlHelper clientCurl;
	HttpPara para;
	para.Authorization = token;
	para.strUrl;
	{
		std::stringstream s1;
		s1 << DEVICE_CONFIG.cfgHttpServerCloud.url << "Cam/ReturnCamerScheduleConfigJson/" << camera_id;
		para.strUrl = s1.str();
	}
	//API 或 本地 schedule 結果
	std::string strRst;

	std::stringstream ss_camera_schedule_json_file_name;
	ss_camera_schedule_json_file_name << "camera_schedule_" << camera_id << ".json";
	std::stringstream ss_camera_schedule_pathfile;
	ss_camera_schedule_pathfile << "./conf/" << ss_camera_schedule_json_file_name.str();

	//如果聯機
	if (DEVICE_CONFIG.cfgDevice.device_is_online_always == false)
	{ 
		strRst = File::readJsonFile(ss_camera_schedule_pathfile.str());
		if (strRst == "")
		{
			//LOG(INFO) << "CameraMpeg::setting_n_schedule_by_camera_id  [READ JSON]  fail: " << ss_camera_schedule_pathfile.str();
			std::cout << "CameraMpeg::setting_n_schedule_by_camera_id  [READ JSON]  fail: " << ss_camera_schedule_pathfile.str() << std::endl;
		}
	}
	else {
		//聯機 則通過 API 獲得 鏡頭的 Schedule
		int ncode = clientCurl.Get(para, strRst); 
		if (ncode != 0)
		{
			//LOG(TRACE) << "func::cameraMpeg::setting_n_schedule_by_camera_id() fail!!! ncode =" << ncode;
			std::cout << "func::cameraMpeg::setting_n_schedule_by_camera_id() fail!!! ncode =" << ncode << std::endl;
			return false;
		}
		else {
			//成功返回 則保存json  
			bool saveJsonResult = File::saveJsonFile(strRst, ss_camera_schedule_pathfile.str());
			if (!saveJsonResult)
			{
				//(INFO) << "CameraMpeg::setting_n_schedule_by_camera_id fail: [SAVE JSON]" << ss_camera_schedule_pathfile.str();
				std::cout << "CameraMpeg::setting_n_schedule_by_camera_id fail: [SAVE JSON]" << ss_camera_schedule_pathfile.str() << std::endl;
				return CURLE_FAILED_INIT;
			}
		}
	}

	rapidjson::Document doc;
	JS_PARSE_OBJECT_RETURN(doc, strRst, false);

	if (doc.HasMember("meta") && doc["meta"].IsObject())
	{
		Service::Meta meta;
		JS_PARSE_OPTION(meta.success, doc["meta"], Bool, success);
		JS_PARSE_OPTION(meta.message, doc["meta"], String, message);
		JS_PARSE_OPTION(meta.errorCode, doc["meta"], Int, errorCode);

		if (meta.success == true)
		{
			ret = meta.success;

			if (doc.HasMember("data") && doc["data"].IsObject())
			{
				//CamSettingNSchedule
				JS_PARSE_OPTION(cam_set.CameraId, doc["data"], Int, cameraId);
				JS_PARSE_OPTION(cam_set.CameraName, doc["data"], String, cameraName);
				JS_PARSE_OPTION(cam_set.CameraIp, doc["data"], String, cameraIp);
				JS_PARSE_OPTION(cam_set.DeviceId, doc["data"], Int, deviceId);
				JS_PARSE_OPTION(cam_set.DeviceName, doc["data"], String, deviceName);
				JS_PARSE_OPTION(cam_set.SiteId, doc["data"], String, siteId);
				JS_PARSE_OPTION(cam_set.MaincomId, doc["data"], String, maincomId);
				JS_PARSE_OPTION(cam_set.PolicyIsStart, doc["data"], Bool, policyIsStart);
				JS_PARSE_OPTION(cam_set.SavePic, doc["data"], Bool, savePic);
				JS_PARSE_OPTION(cam_set.SaveVideo, doc["data"], Bool, saveVideo);
				JS_PARSE_OPTION(cam_set.SaveMpeg4, doc["data"], Bool, saveMpeg4);
				JS_PARSE_OPTION(cam_set.HlsOutput, doc["data"], Bool, hlsOutput);
				JS_PARSE_OPTION(cam_set.RtmpOutput, doc["data"], Bool, rtmpOutput);
				JS_PARSE_OPTION(cam_set.SundayStart, doc["data"], String, sundayStart);
				JS_PARSE_OPTION(cam_set.SundayEnd, doc["data"], String, sundayEnd);
				JS_PARSE_OPTION(cam_set.MondayStart, doc["data"], String, mondayStart);
				JS_PARSE_OPTION(cam_set.MondayEnd, doc["data"], String, MondayEnd);
				JS_PARSE_OPTION(cam_set.TuesdayStart, doc["data"], String, tuesdayStart);
				JS_PARSE_OPTION(cam_set.TuesdayEnd, doc["data"], String, tuesdayEnd);
				JS_PARSE_OPTION(cam_set.WednesdayStart, doc["data"], String, wednesdayStart);
				JS_PARSE_OPTION(cam_set.WednesdayEnd, doc["data"], String, wednesdayEnd);
				JS_PARSE_OPTION(cam_set.ThursdayStart, doc["data"], String, thursdayStart);
				JS_PARSE_OPTION(cam_set.ThursdayEnd, doc["data"], String, thursdayEnd);
				JS_PARSE_OPTION(cam_set.FridayStart, doc["data"], String, fridayStart);
				JS_PARSE_OPTION(cam_set.FridayEnd, doc["data"], String, fridayEnd);
				JS_PARSE_OPTION(cam_set.SaturdayStart, doc["data"], String, saturdayStart);
				JS_PARSE_OPTION(cam_set.SaturdayEnd, doc["data"], String, saturdayEnd);
				JS_PARSE_OPTION(cam_set.RtmpOutput, doc["data"], String, rtmpOutput);
				return ret;
			}
		}
	}
	return ret;
}

int CameraMpeg::query_stream_info_record(Service::StreamInfoApiList& streamInfoApiList, const std::string& strMsg, std::string& strRst)
{
	rapidjson::Document document;
	char* json = (char*)strMsg.c_str();
	document.Parse(json);

	bool succ = document["meta"]["success"].GetBool();
	if (succ)
	{
		const rapidjson::Value& value = document["data"];

		if (value.IsArray()) {
			for (unsigned int i = 0; i < value.Size(); i++)
			{
				Service::StreamInfoForApi streamInfoForApi;
				const rapidjson::Value& object = value[i];
				int cameraId = object["cameraId"].GetInt();
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.nCameraId, object, Int, cameraId, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.rtspIp, object, String, rtspIp, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.nRefCount, object, Int, refCount, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.strInput, object, String, input, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.strOutput, object, String, strOutput, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.bSavePic, object, Bool, bSavePic, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.bSaveVideo, object, Bool, bSaveVideo, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.bRtmp, object, Bool, bRtmp, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.nWidth, object, Int, nWidth, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.nHeight, object, Int, nHeight, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.nPixFmt, object, Int, nPixFmt, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.nHDType, object, Int, nHDType, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.nFrameRate, object, Int, nFrameRate, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.nVideoIndex, object, Int, nVideoIndex, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.nAudioIndex, object, Int, nAudioIndex, strRst, CP_INVALID_PARA);
				JS_PARSE_REQUIRED_RETURN(streamInfoForApi.nVideoTime, object, Int, nVideoTime, strRst, CP_INVALID_PARA);
				std::cout << "\n cameraId --- " << cameraId << std::endl;
				streamInfoApiList.push_back(streamInfoForApi);
			}
			std::cout << "\nResule strRst = " << strRst << "\n streamInfoApiList.size = " << streamInfoApiList.size() << std::endl;
		}
		return CP_OK;
	}
	else
	{
		return CP_INVALID_JSON;
	}
}

/// <summary>
/// 其實是沒被調用的 應該不是用於啟動流的 2024-12-22
/// 嘗試注釋掉 看看運行結果是否受到影響
/// </summary>
/// <param name="streamInfoApiList"></param>
//void CameraMpeg::stream_info_start_list(Service::StreamInfoApiList& streamInfoApiList)
//{
//	std::list< Service::StreamInfoForApi>::iterator list_iter;
//	for (list_iter = streamInfoApiList.begin(); list_iter != streamInfoApiList.end(); list_iter++)
//	{
//		Service::StreamInfoForApi streamInfoForApi = (Service::StreamInfoForApi)*list_iter;
//		StreamInfo  infoStream;
//		get_stream_info(streamInfoForApi, infoStream);
//#ifdef _WIN32
//		std::thread* thStreamHandle = new std::thread(&CameraMpeg::stream_handle_start, this, infoStream);
//#elif __linux__
//		// linux编译器传引用需要使用std::ref包装才能编译
//		std::thread* thStreamHandle = new std::thread(&CameraMpeg::stream_handle_start, this, std::ref(infoStream));
//#endif
//	}
//	return;
//}

/// <summary>
/// 其實是沒被調用的 應該不是用於啟動流的 2024-12-22
/// 嘗試注釋掉 看看運行結果是否受到影響
/// </summary>
//void CameraMpeg::stream_handle_start(StreamInfo& infoStream)
//{
//	StreamHandle stream;
//
//	stream.ListSupportedHD();
//	stream.StartDecode(infoStream);
//	std::this_thread::sleep_for(std::chrono::milliseconds(260000));
//	//stream.StopDecode(); 
//	int i = getchar();
//	return;
//}

void CameraMpeg::get_stream_info(Service::StreamInfoForApi& streamInfoForApi, StreamInfo& infoStream)
{
	infoStream.nCameraId = streamInfoForApi.nCameraId;							//No.1
	infoStream.rtspIp = streamInfoForApi.rtspIp;								//No.1b
	infoStream.nHDType = streamInfoForApi.nHDType;								//No.2  
	infoStream.strInput = streamInfoForApi.strInput;  							//No.3       
	infoStream.bSavePic = streamInfoForApi.bSavePic; 							//No.4  
	infoStream.mediaFormate = streamInfoForApi.mediaFormate; 					//No.5  
	infoStream.savePictRate = streamInfoForApi.savePictRate; 					//No.6  
	infoStream.bSaveVideo = streamInfoForApi.bSaveVideo; 						//No.7  
	infoStream.nVideoTime = streamInfoForApi.nVideoTime; ;   					//No.8     
	infoStream.bRtmp = streamInfoForApi.bRtmp;    							//No.9
	infoStream.strOutput = "rtmp://127.0.0.1:1935/live/"
		+ std::to_string(streamInfoForApi.nCameraId);							//No.10

	infoStream.nWidth = streamInfoForApi.nWidth;								//No.11  
	infoStream.nHeight = streamInfoForApi.nHeight;								//No.12 
	infoStream.nPixFmt = streamInfoForApi.nPixFmt;								//No.13

	infoStream.nFrameRate = streamInfoForApi.nFrameRate;						//No.14 
	infoStream.nVideoIndex = streamInfoForApi.nVideoIndex;						//No.15  
	infoStream.nAudioIndex = streamInfoForApi.nAudioIndex;						//No.16  
	infoStream.nRefCount = streamInfoForApi.nRefCount;							//No.17
}

/// <summary>
/// 车牌识别业务 提交需要校测的图片到AI 车牌识别并返回结果，判断精确率高于预设的则POST到CarParking_CMS Web 服务器平台
/// </summary>
/// <param name="pict_info"></param>
/// <returns></returns>
bool CameraMpeg::detect_and_handle(const StreamInfo& infoStream, const PictInfo& pict_info)
{
	//TEST
	//printf("\nfunc::detect_and_handle path_filename = %s\n", pict_info.path_filename.c_str());

	/*检测服务器*/
	std::string http_detect_server_api = DEVICE_CONFIG.cfgCarPlateRecogBusiness.http_detect_server_api;
	/*回传Web 服务器*/
	std::string http_server_api_url = DEVICE_CONFIG.cfgCarPlateRecogBusiness.http_server_api;

	double cfgThreshold = DEVICE_CONFIG.cfgCarPlateRecogBusiness.threshold;

	bool ret = false;
	//LOG(INFO) << "func::CameraMpeg::detect_and_handle begin....... path_filename=" << pict_info.path_filename << "\n";
	std::cout << "func::CameraMpeg::detect_and_handle begin....... path_filename=" << pict_info.path_filename << "\n" << std::endl;

	std::string strRst;
	LibcurlHelper clientCurl;
	HttpPara para;
	para.strUrl = DEVICE_CONFIG.cfgCarPlateRecogBusiness.http_detect_server_api;
	para.nConnectTimeout = 300; //毫秒
	para.nTransTimeout = 3000; //毫秒
	 
	const std::string pathFileame = pict_info.path_filename;

	int nCode = clientCurl.UploadFile(para, pathFileame, strRst);

	//成功上存文件到AI SERVER后回传结果到WEB SERVER
	if (nCode == 0)
	{
		if (strRst.length() > 0)
		{
			//TEST
			//LOG(INFO) << strRst;
			rapidjson::Document doc;
			JS_PARSE_OBJECT_RETURN(doc, strRst, false);

			int error = 0;
			JS_PARSE_OPTION(error, doc, Int, error);
			if (error != 0)
			{
				std::cout << "\ndescription':'nothing to recognize any text!!!\n" << std::endl;
				return  false; //返回的识别结果提示有错误，返回。
			}

			rapidjson::Value& valueDoc = doc["value"];  //value数组类型

			if (valueDoc.IsArray())
			{
				Service::PlateReturnList plateReturnList;

				for (unsigned int i = 0; i < valueDoc.Size(); i++)
				{
					Service::PlateReturn plateReturn;
					const rapidjson::Value& object = valueDoc[i];

					JS_PARSE_REQUIRED_RETURN(plateReturn.plate, object, String, text, strRst, CP_INVALID_PARA);
					JS_PARSE_REQUIRED_RETURN(plateReturn.color, object, String, color, strRst, CP_INVALID_PARA);
					JS_PARSE_REQUIRED_RETURN(plateReturn.confidence, object, Double, precise, strRst, CP_INVALID_PARA);

					plateReturn.ip = infoStream.rtspIp;

					time_t timet = Time::GetTimestamp();

					auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
					std::stringstream ss;
					ss << std::put_time(std::localtime(&time), "%Y-%m-%dT%H:%M:%S");
					plateReturn.date = ss.str();

					plateReturn.image64 = pict_info.frameBase64;
					plateReturn.ParkingNoCalcType = 0;

					plateReturnList.push_back(plateReturn);
				}

				//循环post多个车牌
				std::list<Service::PlateReturn>::iterator itr;
				for (itr = plateReturnList.begin(); itr != plateReturnList.end(); ++itr)
				{
					if (itr->confidence > cfgThreshold) //如果精确度符合预设的，则post回Web Server
					{
						LibcurlHelper clientCurl2;
						HttpPara para2;
						para2.strUrl = http_server_api_url;
						para2.nTransTimeout = 3000;
						para2.nConnectTimeout = 300;

						std::string strJson;
						Service::PlateReturn item;
						item.plate = itr->plate;
						item.date = itr->date;
						item.country = itr->country;
						item.confidence = itr->confidence;
						item.color = itr->color;
						item.ip = itr->ip;
						item.image64 = itr->image64; //另外以jpg file 形式上存 经查是API问题，所以采用原有方式 直接上存。不需要另独立上存图片
						item.ParkingNoCalcType = itr->ParkingNoCalcType; //计时类型的log=0

						/*此處負責過濾掉不符合規則的字符*/
						std::string currDate = Time::GetCurrentDate();
						std::string::size_type idx = item.plate.find(currDate);
						if (idx != std::string::npos)
						{
							std::cout << "INVALID CAR PLATE STRING:" << item.plate << std::endl;
							continue;  //next item
						}

						bool parseRes = parse_plate_return_json(item, strJson);
						if (parseRes == false)
							return false;
						std::string resPlateReturn;
						int plateReturn_ret = clientCurl2.PostBase64Json(para2, strJson, resPlateReturn);

						//CURLE_OK = 0 则获取成功 strRst  
						if (plateReturn_ret != CURLE_OK)
						{
							//LOG(INFO) << "post  fail: " << http_server_api_url << " pathFileame:" << pathFileame << "API::PlateReturn::strResponse:\n" << resPlateReturn;
							std::cout << "post  fail: " << http_server_api_url << " pathFileame:" << pathFileame << "API::PlateReturn::strResponse:\n" << resPlateReturn << std::endl;
							return CURLE_FAILED_INIT;
						}

						//最后上存图片 
						rapidjson::Document doc_plate_return;
						JS_PARSE_OBJECT_RETURN(doc_plate_return, resPlateReturn, false);
						bool plate_return_succ = false;
						JS_PARSE_OPTION(plate_return_succ, doc_plate_return, Bool, SUCCESS);

						int64_t logId = 0;
						JS_PARSE_OPTION(logId, doc_plate_return, Int, id);
						std::cout << "POST API [PlateReturn] SUCCESS: RETURN PlateLotId = " << std::to_string(logId) << " RETURN JSON:" << resPlateReturn << std::endl;

						//关闭独立上存图片功能，应该是之前的PlateReturn API数据库问题。BASE64 JSON格式上存已解決，無需要獨立上存圖片
						if (plate_return_succ && 2 == 3)  //关闭
						{
							LibcurlHelper clientCurl3;
							HttpPara para3;
							std::stringstream ss3;
							{
								ss3 << http_server_api_url << "UpFile/" << std::to_string(logId);
							}
							para3.strUrl = ss3.str();
							para3.nTransTimeout = 3000;
							para3.nConnectTimeout = 300;
							std::string upfileRet;
							int upFile_ret = clientCurl3.UploadFile(para3, pict_info.path_filename, upfileRet);

							//LOG(INFO) << "func::detect_and_handle::PlateReturnUpFile up file result: " << upfileRet;
							std::cout << "func::detect_and_handle::PlateReturnUpFile up file result: " << upfileRet << std::endl;
						}
					}
				}
				//strRst 打印 json （回传结果）
				//LOG(INFO) << "func::detect_and_handle completed file: " << pathFileame;
				std::cout <<  "func::detect_and_handle completed file: "  << std::endl;

				//最后清理list
				if (plateReturnList.size() > 0)
				{
					plateReturnList.clear();
				}

				return true;
			}
			else {
				//LOG(INFO) << "func::detect_and_handle return the json not contain the reognition result array!!! ";
				std::cout << "func::detect_and_handle return the json not contain the reognition result array!!! " << std::endl;
				return false;
			}
		}
	}
	else {
		return  false;  //未上存到检测AI SERVER 
	}
}

/*车牌识别回传到Web Server 的对象参数*/
bool CameraMpeg::parse_plate_return_json(Service::PlateReturn& plateReturn, std::string& jsonResult)
{
	try
	{
		//"{"plate":null,"date":null,"country":null,"confidence":null,"ip":null,"image64":null,"parkingNoCalcType":0}"

		rapidjson::Document doc(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();

		//plate 
		doc.AddMember("plate", rapidjson::Value(plateReturn.plate.c_str(), typeAllocate), typeAllocate);
		//dateVal
		doc.AddMember("date", rapidjson::Value(plateReturn.date.c_str(), typeAllocate), typeAllocate);

		//country
		doc.AddMember("country", rapidjson::Value(plateReturn.country.c_str(), typeAllocate), typeAllocate);

		//confidence
		std::string confidence = std::to_string(plateReturn.confidence);
		doc.AddMember("confidence", rapidjson::Value(confidence.c_str(), typeAllocate), typeAllocate);
		//ip
		doc.AddMember("ip", rapidjson::Value(plateReturn.ip.c_str(), typeAllocate), typeAllocate);

		//image64
		doc.AddMember("image64", rapidjson::Value(plateReturn.image64.c_str(), typeAllocate), typeAllocate);

		//ParkingNoCalcType
		rapidjson::Value ParkingNoCalcTypeVal;
		ParkingNoCalcTypeVal.SetInt(0);
		doc.AddMember("parkingNoCalcType", ParkingNoCalcTypeVal, typeAllocate);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
		doc.Accept(writer);
		jsonResult = buffer.GetString();
		return true;
	}
	catch (std::exception& e)
	{
		//LOG(ERROR) << "[CameraMpeg::parse_plate_return_json] " << e.what() << std::endl;
		std::cout << "[CameraMpeg::parse_plate_return_json] " << e.what() << std::endl;
		return false;
	}
}

/*
獲取鏡頭詳細資料
*/
bool CameraMpeg::get_camera_details(int& cameraId, Service::CameraDetails& cameraDetails)
{ 
	std::string strRst;
	LibcurlHelper clientCurl;
	HttpPara para;
	// http://81.71.74.135:5002/zh-CN/Cam/CameraDetails?CameraId=12
	std::stringstream api_camera_details;
	{
		api_camera_details << DEVICE_CONFIG.cfgHttpServer.url << DEVICE_CONFIG.cfgDevice.language_code << "/Cam/CameraDetails?CameraId=" << cameraId;
	}
	para.strUrl = api_camera_details.str();
	para.nConnectTimeout = 1000;
	para.nTransTimeout = 3000;

	//脫機或離線獲取JSON
	std::stringstream ss_camera_details_json_file_name;
	ss_camera_details_json_file_name << "camera_details_" << cameraId << ".json";
	
	std::stringstream ss_camera_details_pathfile;
	{  ss_camera_details_pathfile << File::GetWorkPath() << kSeprator << "conf" << kSeprator << ss_camera_details_json_file_name.str(); }
	 

	if (DEVICE_CONFIG.cfgDevice.device_is_online_always == false)
	{
		// 通過設備序列號 成功返回json GetMainComBySerialNo 
		strRst = File::readJsonFile(ss_camera_details_pathfile.str());
		if (strRst == "")
		{
			//LOG(INFO) << "CameraMpeg::get_camera_details File::readJsonFile cameraId : [READ] camera_details_{cameraId}.json fail: " << ss_camera_details_pathfile.str();
			std::cout << "CameraMpeg::get_camera_details File::readJsonFile cameraId : [READ] camera_details_{cameraId}.json fail: " << ss_camera_details_pathfile.str() << std::endl;
		}
	}
	else {
		std::string bear_token;
		bool succ = request_token(bear_token);
		if (!succ)
		{
			//LOG(TRACE) << "camera_mpeg_add2::get_api_token " << succ << " FAIL";
			std::cout << "camera_mpeg_add2::get_api_token " << succ << " FAIL" << std::endl;
			return false;
		}
		para.Authorization = bear_token;
		int nCode = clientCurl.Get(para, strRst);
		if (nCode != 0)
		{
			//LOG(TRACE) << "func::cameraMpeg::get_camera_details() fail!!! nCode =" << nCode;
			std::cout << "func::cameraMpeg::get_camera_details() fail!!! nCode =" << nCode << std::endl;
			return false;
		}
		else {
			//成功返回 則保存json  
			bool saveJsonResult = File::saveJsonFile(strRst, ss_camera_details_pathfile.str());
			if (!saveJsonResult)
			{
				//LOG(INFO) << "CameraMpeg::get_camera_details fail: " << ss_camera_details_pathfile.str();
				std::cout << "CameraMpeg::get_camera_details fail: " << ss_camera_details_pathfile.str() << std::endl;
				return CURLE_FAILED_INIT;
			}
		}
	}
	 
	rapidjson::Document doc;
	JS_PARSE_OBJECT_RETURN(doc, strRst, false);
	if (doc.HasMember("meta") && doc["meta"].IsObject())
	{
		Service::Meta meta;
		JS_PARSE_OPTION(meta.success, doc["meta"], Bool, success);
		JS_PARSE_OPTION(meta.message, doc["meta"], String, message);
		JS_PARSE_OPTION(meta.errorCode, doc["meta"], Int, errorCode);

		if (meta.success == true)
		{
			if (doc.HasMember("data"))
			{
				const rapidjson::Value& doc_data = doc["data"];

				JS_PARSE_OPTION(cameraDetails.CameraId, doc_data, Int, cameraId);
				JS_PARSE_OPTION(cameraDetails.MaincomId, doc_data, String, maincomId);
				JS_PARSE_OPTION(cameraDetails.Name, doc_data, String, name);
				JS_PARSE_OPTION(cameraDetails.CameraIp, doc_data, String, cameraIp);
				JS_PARSE_OPTION(cameraDetails.SiteId, doc_data, Int, siteId);
				JS_PARSE_OPTION(cameraDetails.SiteName, doc_data, String, siteName);
				JS_PARSE_OPTION(cameraDetails.Rtsp, doc_data, String, rtsp);
				JS_PARSE_OPTION(cameraDetails.Type, doc_data, Int, type);
				JS_PARSE_OPTION(cameraDetails.Online, doc_data, String, online);
				JS_PARSE_OPTION(cameraDetails.Remark, doc_data, String, remark);
				JS_PARSE_OPTION(cameraDetails.CreateTime, doc_data, String, createTime);
				JS_PARSE_OPTION(cameraDetails.DeviceId, doc_data, Int, deviceId);
				JS_PARSE_OPTION(cameraDetails.DeviceName, doc_data, String, deviceName);
				JS_PARSE_OPTION(cameraDetails.RecordStatus, doc_data, Int, recordStatus);
				JS_PARSE_OPTION(cameraDetails.DeviceType, doc_data, Int, deviceType);
				JS_PARSE_OPTION(cameraDetails.Onlive, doc_data, Bool, onlive);
			}
		}
	}
	return true;
}

/*
* 通過鏡頭ID獲取關聯的任務信息列表
*/
bool CameraMpeg::get_task_list_by_camera_id(int& cameraId, Service::TaskInfoList& taskInfoList)
{
	//{Language}/Task/GetTaskListByCameraId/{cameraId}
	std::string strRst;
	LibcurlHelper clientCurl;
	HttpPara para;
	// http://81.71.74.135:5002/zh-CN/Cam/CameraDetails?CameraId=12
	std::stringstream api_camera_task_lists;
	{
		api_camera_task_lists << DEVICE_CONFIG.cfgHttpServer.url << DEVICE_CONFIG.cfgDevice.language_code << "/Task/GetTaskListByCameraId/" << cameraId;
	}
	para.strUrl = api_camera_task_lists.str();
	para.nConnectTimeout = 1000;
	para.nTransTimeout = 3000;

	// 通過脫機或者聯機獲取鏡頭的任務列表 

	std::stringstream ss_tasks_list_json_file_name;
	ss_tasks_list_json_file_name << "camera_tasks_list_" << cameraId <<".json";

	std::stringstream ss_path_file_name;
	{  ss_path_file_name << File::GetWorkPath() << kSeprator << "conf" << kSeprator << ss_tasks_list_json_file_name.str(); }
	 

	if (DEVICE_CONFIG.cfgDevice.device_is_online_always == false)
	{
		// 通過設備序列號 成功返回json GetMainComBySerialNo 
		strRst = File::readJsonFile(ss_path_file_name.str());
		if (strRst == "")
		{
			//LOG(INFO) << "CameraMpeg::get_task_list_by_camera_id File::readJsonFile cameraId : [READ] main_company_details_by_serialno.json fail: " << ss_path_file_name.str();
			std::cout << "CameraMpeg::get_task_list_by_camera_id File::readJsonFile cameraId : [READ] main_company_details_by_serialno.json fail: " << ss_path_file_name.str()  << std::endl;
		}
	}
	else {
		std::string bear_token;
		bool succ = request_token(bear_token);
		if (!succ)
		{
			//LOG(TRACE) << "get_task_list_by_camera_id-> camera_mpeg_add2::get_api_token " << succ << " FAIL";
			std::cout << "get_task_list_by_camera_id-> camera_mpeg_add2::get_api_token " << succ << " FAIL" << std::endl;
			return false;
		}
		para.Authorization = bear_token;
		int nCode = clientCurl.Get(para, strRst);
		if (nCode != 0)
		{
			//LOG(TRACE) << "func::cameraMpeg::get_task_list_by_camera_id() fail!!! nCode =" << nCode;
			std::cout << "func::cameraMpeg::get_task_list_by_camera_id() fail!!! nCode =" << std::endl;
			return false;
		}
		else {
			//成功返回 則保存json  
			bool saveJsonResult = File::saveJsonFile(strRst, ss_path_file_name.str());
			if (!saveJsonResult)
			{
				//LOG(INFO) << "CameraMpeg::get_task_list_by_camera_id fail: " << ss_path_file_name.str();
				std::cout << "CameraMpeg::get_task_list_by_camera_id fail: " << ss_path_file_name.str() << std::endl;
				return false; //return CURLE_FAILED_INIT;
			}
		}
	}
	 
	rapidjson::Document doc;
	JS_PARSE_OBJECT_RETURN(doc, strRst, false);
	if (doc.HasMember("meta") && doc["meta"].IsObject())
	{
		Service::Meta meta;
		JS_PARSE_OPTION(meta.success, doc["meta"], Bool, success);
		JS_PARSE_OPTION(meta.message, doc["meta"], String, message);
		JS_PARSE_OPTION(meta.errorCode, doc["meta"], Int, errorCode);

		if (meta.success == true)
		{
			if (doc.HasMember("data"))
			{
				const rapidjson::Value& a = doc["data"];

				if (a.IsArray())
				{
					for (rapidjson::SizeType i = 0; i < a.Size(); i++)
					{
						Service::TaskInfo taskInfo;
						JS_PARSE_OPTION(taskInfo.TaskId, a[i], Int, taskId);
						JS_PARSE_OPTION(taskInfo.Name, a[i], String, name);
						JS_PARSE_OPTION(taskInfo.Type, a[i], Int, type);
						JS_PARSE_OPTION(taskInfo.LibList, a[i], String, libList);
						JS_PARSE_OPTION(taskInfo.Interval, a[i], Int, interval);
						JS_PARSE_OPTION(taskInfo.Threshold, a[i], Double, threshold);
						JS_PARSE_OPTION(taskInfo.State, a[i], Int, state);

						taskInfoList.push_back(taskInfo);
					}

					if (taskInfoList.size() > 0)
						return true;
				}
				return false;
			}
		}
	}
	else {
		return false;
	}
}
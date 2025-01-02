#pragma once
#include "Common.h" 
#include "Config/DeviceConfig.h"

namespace Stream {

	////去掉 前面的  CarPlate::
	//bool detect_and_handle(PictInfo& pict_info)
	//{
	//	bool ret = false;
	//	LOG(INFO) << "func::CarPlate::detect_and_handle path_filename=" << pict_info.path_filename << "\n";

	//	std::string strRst;
	//	LibcurlHelper clientCurl;
	//	HttpPara para;
	//	para.strUrl = DEVICE_CONFIG.cfgCarPlateRecogBusiness.http_detect_server_api;
	//	para.nConnectTimeout = 100; //毫秒
	//	para.nTransTimeout = 3000; //毫秒

	//	LOG(INFO) << pict_info.path_filename;

	//	const std::string pathFileame = pict_info.path_filename;

	//	int nCode = clientCurl.UploadFile(para, pathFileame, strRst);
	//	LOG(INFO) << strRst;
	//	rapidjson::Document doc;
	//	JS_PARSE_OBJECT_RETURN(doc, strRst, false);

	//	ret = true;
	//	return ret;
	//};

	class CarPlate {
	public:
		explicit CarPlate();
		~CarPlate();
	public:
		std::string token;
		/// <summary>
		/// 提交检测 返回结果判断ThreshHold精度，是否提交到结果处理
		/// </summary>
		/// <param name="streamInfo">传入镜头的相关信息</param>
		/// <param name="pictInfo"></param>
		/// <returns></returns>
		bool detect_and_handle1(std::string& path_filename);  //ref https://learn.microsoft.com/zh-cn/cpp/error-messages/compiler-errors-1/compiler-error-c2143?view=msvc-170

		//bool detect_and_handle2(PictInfo& pict_info);

		/// <summary>
		/// 把符合的检测结果POST到WEB SERVER 提供停车计时。
		/// </summary>
		/// <param name="streamInfo"></param>
		/// <param name="pictInfo"></param>
		/// <returns></returns>
		//bool post_detect_result(StreamInfo& streamInfo, PictInfo& pictInfo);

		//bool pict_info_to_json(PictInfo& pictInfo, std::string& toJsonString);

	};



}



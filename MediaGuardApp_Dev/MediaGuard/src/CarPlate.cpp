#pragma once
#include "CarPlate.h" 


/*
CarPlate 类 主要作用是 把 镜头解码的图片 post到 车牌识别服务器进行识别是否包含车牌
如果识别成功则把图片post回Web server
*/

namespace Stream {


	CarPlate::CarPlate()
	{
	}
	CarPlate::~CarPlate()
	{
	}

	bool CarPlate::detect_and_handle1(std::string& path_filename)
	{
		//LOG(INFO) << "func::CarPlate::detect_and_handle path_filename=" << path_filename << "\n";
		std::cout << "Func::CarPlate::detect_and_handle path_filename=" << path_filename << "\n" << std::endl;
		bool ret = false;
		std::string strRst;
		LibcurlHelper clientCurl;
		HttpPara para;
		para.strUrl = DEVICE_CONFIG.cfgCarPlateRecogBusiness.http_detect_server_api;
		para.nConnectTimeout = 100; //毫秒
		para.nTransTimeout = 3000; //毫秒
		 
		const std::string pathFileame = path_filename;

		//如果沒有本地保存圖片則導致出錯 (FATAL :camera_schedule_12.json->savePic =false)
		//後續這裡需要修改
		int nCode = clientCurl.UploadFile(para, pathFileame, strRst);
	 
		rapidjson::Document doc;
		JS_PARSE_OBJECT_RETURN(doc, strRst, false);

		ret = true;
		return ret;
	}


	//bool CarPlate::post_detect_result(Stream::StreamInfo& streamInfo, Stream::PictInfo& pictInfo)
	//{
	//	bool ret = false;
	//
	//	return ret;
	//}
	//
	//bool CarPlate::pict_info_to_json(Stream::PictInfo& pictInfo, std::string& toJsonString)
	//{
	//	try
	//	{
	//		rapidjson::Document doc(rapidjson::kObjectType);
	//		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();
	//
	//		//recordId
	//		rapidjson::Value cameraId;
	//		cameraId.SetInt(pictInfo.camera_id);
	//		doc.AddMember("cameraId", cameraId, typeAllocate);
	//
	//		rapidjson::Value createTime;
	//		createTime.SetInt(pictInfo.create_time);
	//		doc.AddMember("createTime", createTime, typeAllocate);
	//
	//		rapidjson::Value frameBase64;
	//		frameBase64.SetString(pictInfo.frameBase64.c_str(), typeAllocate);
	//		doc.AddMember("frameBase64", frameBase64, typeAllocate);
	//
	//		rapidjson::Value pathFilename;
	//		pathFilename.SetString(pictInfo.path_filename.c_str(), typeAllocate);
	//		doc.AddMember("pathFilename", pathFilename, typeAllocate);
	//
	//		rapidjson::Value width;
	//		width.SetInt(pictInfo.width);
	//		doc.AddMember("width", width, typeAllocate);
	//
	//		rapidjson::Value height;
	//		height.SetInt(pictInfo.height);
	//		doc.AddMember("height", height, typeAllocate);
	//
	//
	//		rapidjson::StringBuffer buffer;
	//		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
	//		doc.Accept(writer);
	//		toJsonString = buffer.GetString();
	//		return true;
	//	}
	//	catch (std::exception& e)
	//	{
	//		LOG(ERROR) << "[CarPlate::pict_info_to_json] " << e.what() << std::endl;
	//		return false;
	//	}
	//}
}



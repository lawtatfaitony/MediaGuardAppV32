#pragma once 
#include <string>
#include <sstream> 
#include <thread>
#include <iostream>
#include <exception> 
#include <iosfwd>
#include <stdbool.h> 
#include <openssl/crypto.h>
#include "Basic/ThreadPool.h"
#include "Basic/ThreadObject.h"

#include "Common/JsonHelper.h"
#include "Common/Macro.h" 
#include "ErrorInfo/ErrorCode.h" 
#include "ErrorInfo/ErrorMessage.h"
#include "Http/LibcurlHelper.h" 
#include "Config/DeviceConfig.h" 

#include "../hmac/hmac_sha1.h"

#include "../StreamHandle.h" 
#include "../StreamDefine.h"
#include "../interface/CameraMpeg.h"
#include "../interface/ResponseDefine.h"

namespace Service
{
	// 全局設備ID 指的是 LINUX 盒子/ WIN 盒子等 操作系統 載體平台的設備ID
	// 使用設備序列號獲得
	// 注意不要混淆是 鏡頭或者HIK 的 掛載設備的ID
	// 是APPLICATION 應用程序運行下的 虛擬的DEVICE ID 
	// static std::string DeviceId = "-";
	/*
	API JSON FORMAT
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
	struct CameraMpegInfo
	{
		int64_t RecordId = 0;
		int CameraId = 0;
		std::string DeviceSerialNo = "0";
		bool IsGroup = false;
		std::string MpegFileName = "";
		int64_t FileSize = 0;
		std::string FileFormat = "video/mpeg";
		int64_t StartTimestamp = 0;
		int64_t EndTimestamp = 0;
	};
	/* 更新設備公網IP到雲端 Request
{
    "deviceId":"3012",
    "internetIp":"219.77.12.21",
    "internetPort":"8080",
    "localIp":"192.168.0.188",
    "localPort":"8080"
}
*/
	struct DeviceInterNetIpInfo
	{ 
		std::string deviceId;
		std::string internetIp;
		std::string internetPort;
		std::string localIp;
		std::string localPort;
	};

	/*
	任務信息詳情 對應的表Ft_Task  對應接口：​/{Language}​/Task​/GetTaskListByCameraId​/{cameraId}
	部分沒必要的屬性，在這裡被省略
	enum class TaskType 在StreamDefine.h中定義了。
	*/
	struct TaskInfo
	{
		TaskInfo() {
			this->TaskId = 0;
			this->Name = "";
			this->LibList = "";
			this->Type = 0;
			this->Interval = 0;
			this->Threshold = 0;
			this->State = 0;
		};
		int TaskId;
		std::string Name;
		int Type;
		std::string LibList;
		int Interval;
		double Threshold;
		int State;
	};
	typedef std::list<TaskInfo> TaskInfoList;
	/*
	鏡頭詳細資料 包括鏡頭狀態 For API:"http://81.71.74.135:5002/zh-CN/Cam/CameraDetails?CameraId=12"
	*/
	struct CameraDetails {
		std::string MaincomId;
		int CameraId;
		std::string Name;
		std::string CameraIp;
		int	SiteId;
		std::string SiteName;
		std::string Rtsp;
		int Type;
		std::string Online;
		std::string Remark;
		std::string CreateTime;
		std::string DeviceId;
		std::string DeviceName;
		int RecordStatus;  //CameraRecordStatus {IN_STOP = 0,IN_RECORD = 1,SUSPEND_RECORD = 2
		int DeviceType;
		bool Onlive; //狀態
	};
	// api StreamInfoForApi
	struct StreamInfoForApi
	{
		int nCameraId = 0;										//No.1
		std::string rtspIp = "192.168.0.111";				    //No.1b  补充Camera IP
		int nHDType = kHWDeviceTypeNone;						//No.2 // hard device accelate   kHWDeviceTypeNone = 0
		std::string strInput;									//No.3      // input rtsp://root2:123456@192.168.10.90:554/axis-media/media.amp?videocodec=h264&resolution=640x480
		bool bSavePic = false;									//No.4     // save frame
		int mediaFormate = 0;									//No.5  //存储为MPEG=0 
		int savePictRate = 10;									//No.7 //帧率/savePictRate = 保存的频率 每25帧保存5帧
		bool bSaveVideo = false;								//No.8   // save video 
		int64_t nVideoTime = 60 * 1000 * 5; 					//No.9     //录像时间 time for each video
		int nStreamDecodeType = StreamDecodeType::NOSTREAM;     //No.10 bRtmp 改为解码流类型
		std::string strOutput = "rtmp://127.0.0.1:1935/live/" + std::to_string(nCameraId); //No.11

		int nWidth = 0;											//No.12 //system default value  
		int nHeight = 0;										//No.13 //system default value
		int nPixFmt = AV_PIX_FMT_NONE;							//No.14 //system default value // AV_PIX_FMT_NONE = -1,

		int nFrameRate = 25;									//No.15 //帧率 只用于整除计算多少帧保存一次图片 system default value
		int nVideoIndex = -1;									//No.16 //system default value
		int nAudioIndex = -1;									//No.17 //system default value
		int nRefCount = 0;										//No.18 //av_dict_set(&pOptions, "refcounted_frames", m_infoStream.nRefCount ? "1" : "0", 0);
		bool bRtmp = false;										//No.19  保留用于兼容AudioStreamHandle和UsbStreamHandle 后续开发这两个需要去掉，统一使用RtspStreamHandle的逻辑
		bool cameraStatus = true;								//No.20  鏡頭狀態 0:停用;  1:啟用
	};
	// api para
	struct DeviceSerialNoInput
	{
		std::string DeviceToken = "";
		std::string DeviceSerialNo = "0";
	};

	/*
	 * 通过设备ID返回镜头列表以及状态
	 * /{Language}/Cam/CameraList
	*/
	struct CameraListInput
	{
		std::string MaincomId = "";
		std::string DeviceId = "0";
	};

	/*DVR设备详情*/
	struct DeviceDetails
	{
		std::string deviceId = "";
		std::string sysModuleId = "";  //目前只有两种：
		std::string  deviceName = "";
		int deviceEntryMode = 0;  //常量值 在EnumCode可以找到（search）: DeviceEntryMode :UNDEFINED IN OUT INOUT 设备出入口属性，用于门禁
		std::string  deviceSerialNo = "";  //电脑：CPU ID 作为设备序列号 手机：IMEI作为设备序列号
		std::string  config = ""; //设备配置 空白。需要另外的api path 获取。
		std::string mainComId = "";  //公司ID  云计算模式 必须的。 本地部储 则默认为6000014
		std::string operatedUser = "SYSTEM";
		std::string updateDateTime = "2022-06-25T23:58:53.4557019+08:00";  //ref https://www.shuzhiduo.com/A/D854Qw73dE/
		int	status = 1; //通用状态 1=ACTIVE  ; 0=DEACTIVE
		std::string siteId = ""; //位置编号 是指设备的位置
		bool isReverseHex = true; //是否反向交叉解析16进制。用于NFC拍卡获取的16进制
	};

	/// <summary>
	/// 镜头录像设置与录像策略配置 源名 ：CameraDVRSettingNSchedule
	/// </summary>
	struct CamSettingNSchedule
	{
		int CameraId = 0;
		std::string CameraName;

		/// <summary>
		/// 从设备配置中获取 Camera Ip
		/// </summary>
		std::string CameraIp;

		/// <summary>
		/// 从设备配置中获取 没有则提示先定义设备
		/// </summary>
		int DeviceId;

		/// <summary>
		/// 从设备配置中获取
		/// </summary>
		std::string DeviceName;

		/// <summary>
		/// 从设备配置中获取
		/// 没有则 等于 string.empty
		/// </summary>
		std::string SiteId;

		/// <summary>
		/// 从设备配置中获取.必须的。
		/// </summary>
		std::string MaincomId;

		/// <summary>
		/// 是否啟動策略
		/// </summary>
		bool PolicyIsStart = false;

		/// <summary>
		/// 保存錄像
		/// </summary>
		bool SaveVideo = true;

		/// <summary>
		/// 保存MPEG 4 格式 否則FLV
		/// </summary>
		bool SaveMpeg4 = false;

		/// <summary>
		/// 是否输出HLS
		/// </summary>
		bool HlsOutput = false;

		/// <summary>
		/// 是否输出RTMP
		/// </summary>
		bool RtmpOutput = false;

		/// <summary>
		/// 是否保存图片流 保存照片流才可以启动个种识别任务
		/// </summary>
		bool SavePic = false;

		/// <summary>
		/// 保存图片的间隔
		/// </summary>
		int SavePictRate = 25;

		//------------------------------------------------------------------
		/// <summary>
		/// 星期日 开始时间的录像调度安排
		/// </summary> 
		std::string SundayStart = "00:00:00";
		/// <summary>
		/// 星期日 结束时间录像调度安排
		/// </summary>
		std::string SundayEnd = "23:59:00";

		/// <summary>
		/// 星期一 开始时间的录像调度安排
		/// </summary>
		std::string  MondayStart = "00:00:00";
		/// <summary>
		/// 星期一结束时间录像调度安排
		/// </summary>
		std::string MondayEnd = "23:59:00";
		/// <summary>
		/// 星期二 开始时间的录像调度安排
		/// </summary>
		std::string TuesdayStart = "00:00:00";
		/// <summary>
		/// 星期二结束时间录像调度安排
		/// </summary>
		std::string  TuesdayEnd = "23:59:00";

		/// <summary>
		/// 星期三 开始时间的录像调度安排
		/// </summary>
		std::string  WednesdayStart = "00:00:00";
		/// <summary>
		/// 星期三结束时间录像调度安排
		/// </summary> 
		std::string WednesdayEnd = "23:59:00";

		/// <summary>
		/// 星期四 开始时间的录像调度安排
		/// </summary>
		std::string ThursdayStart = "00:00:00";
		/// <summary>
		/// 星期四结束时间录像调度安排
		/// </summary>
		std::string ThursdayEnd = "23:59:00";

		/// <summary>
		/// 星期五 开始时间的录像调度安排
		/// </summary>
		std::string FridayStart = "00:00:00";
		/// <summary>
		/// 星期五 结束时间录像调度安排
		/// </summary>
		std::string FridayEnd = "23:59:00";

		/// <summary>
		/// 星期六 开始时间的录像调度安排
		/// </summary>
		std::string SaturdayStart = "00:00:00";
		/// <summary>
		/// 星期六 结束时间录像调度安排
		/// </summary>
		std::string SaturdayEnd = "23:59:00";
	};

	typedef std::list<StreamInfoForApi> StreamInfoApiList;

	typedef std::list<StreamInfo> StreamInList;

	struct PlateReturn
	{
		std::string plate = "";
		std::string date = "2023-01-03T11:10:39";  //目前只有两种：
		std::string  country = "HongKong";
		std::string  color = "";
		double confidence = 0;
		std::string  ip = "";
		std::string  image64 = "";
		int ParkingNoCalcType = 0;
	};

	typedef std::list<PlateReturn> PlateReturnList;

	struct TokenReturn {
		TokenReturn() {
			this->AccessToken = "";
			this->UserName = "";
			this->TokenExpiredTimeOut = 0;
		};
		std::string AccessToken;
		std::string UserName;
		int64_t TokenExpiredTimeOut;
	};

	static struct Service::TokenReturn TokenRet;

	/// <summary>
	/// 定義的任務列表
	/// </summary>
	struct TaskInfoListReturn {

		Service::TaskInfoList TaskInfoListTemp;
		int64_t TaskInfoListExpiredTimeOut = 0;
	};

	struct CameraTaskInfo {

		Service::TaskInfo TaskInfo;
		int CameraId = 0;
	};
}

class CameraMpeg
{

public:
	CameraMpeg();
	virtual ~CameraMpeg();

	bool request_token(std::string& token);

	/*int get_api_token(const std::string& strMsg, std::string& strRst);*/

	void camera_mpeg_add(Service::CameraMpegInfo& cameraMpegInfo);

	bool camera_mpeg_add2(Service::CameraMpegInfo& cameraMpegInfo, std::string& strResponse);

	bool update_divice_internet_ip(Service::DeviceInterNetIpInfo& deviceInterNetIpInfo, std::string& strResponse);

	bool camera_mpeg_to_json(Service::CameraMpegInfo& cameraMpegInfo, std::string& strRst);

	bool device_internet_ip_info_to_json(Service::DeviceInterNetIpInfo& deviceInterNetIpInfo, std::string& strResult);

	bool device_serial_no_input_to_json(Service::DeviceSerialNoInput& deviceSerialNoInput, std::string& strResult);

	bool get_camera_list(std::string& device_serial_no);

	bool camera_list(Service::StreamInfoApiList& streamInfoApiList);

	bool camera_list_trans_to_strean_info(Service::StreamInfoApiList& sList, Service::StreamInList& streamInList);

	bool camera_list_input_to_json(Service::CameraListInput& input, std::string& strResult);

	bool device_by_serial_no(Service::DeviceDetails& deviceDetails);

	bool setting_n_schedule_by_camera_id(std::string& camera_id, std::string token, Service::CamSettingNSchedule& cam_set);

	int query_stream_info_record(Service::StreamInfoApiList& streamInfoApiList, const std::string& strMsg, std::string& strRst);

	//應該是沒用到的
	//void stream_info_start_list(Service::StreamInfoApiList& streamInfoApiList);

	void get_stream_info(Service::StreamInfoForApi& streamInfoForApi, StreamInfo& infoStream);

	//應該是沒用到的
	//void stream_handle_start(StreamInfo& infoStream);

	bool detect_and_handle(const StreamInfo& infoStream, const PictInfo& pict_info);

	bool parse_plate_return_json(Service::PlateReturn& plateReturn, std::string& jsonResult);

	bool get_camera_details(int& cameraId, Service::CameraDetails& cameraDetails);

	bool get_task_list_by_camera_id(int& cameraId, Service::TaskInfoList& taskInfoList);


	//--------------------
	std::thread m_thRun;
	std::string api_add_camera_mpeg;
	std::string api_camera_stream_info_list; //CameraStreamInfoList
	std::string api_camera_list;  ///{Language}/Cam/CameraList

private:

};
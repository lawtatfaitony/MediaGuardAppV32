#pragma once
#include "Common.h"
#include <string>
#include <opencv2/core/hal/interface.h> 

namespace Stream
{
	//获取的目录图片信息保存这里后 以供队列模式删除
	struct PictInfo
	{
		int camera_id;
		std::string frameBase64;
		std::string path_filename;
		int64_t create_time;
		int width;
		int height;
	};

	enum class CameraConnectingStatus
	{
		InStopped = 0,
		InPlaying = 1,
		InRequestStopped = 2,
		InDisConnencted = 3,
	};
	enum HWDeviceType
	{
		kHWDeviceTypeNone,
		kHWDeviceTypeVDPAU,
		kHWDeviceTypeCUDA,
		kHWDeviceTypeVAAPI,
		kHWDeviceTypeDXVA2,
		kHWDeviceTypeQSV,
		kHWDeviceTypeVIDEOTOOLBOX,
		kHWDeviceTypeD3D11VA,
		kHWDeviceTypeDRM,
		kHWDeviceTypeOPENCL,
		kHWDeviceTypeMEDIACODEC,
	}; 

	// device.json->global_setting  全局设置 
	struct GlobalSetting
	{
		int picRemainMinutes = 15;
		int videoRemainMinutes = 15; //无论超出容量限制都要最少保留15分钟的video，预设必须考虑最少可以存储15分钟的影片
		int64_t storageLimitedbytes = 3 * 102400000;  //102400 kb //100m
		// hard device accelate
		HWDeviceType nHDType = HWDeviceType::kHWDeviceTypeNone; //硬件类型  
	};

	enum StreamType
	{
		kStreamTypeRtsp = 0,    // rtsp
		kStreamTypeUsb = 1,     // usb
		kStreamTypeAudio = 2,   // audio device
	};

	/*
	扩展 StreamInfo.bRtmp 的功能
	暂时先保留StreamInfo.bRtmp 后续全部使用StreamDecodeType替代
	*/
	enum StreamDecodeType
	{
		NOSTREAM = 0,		//无解码流
		RTMP = 1,			//RTMP 解码流 （需要Nigix支持）      
		HLS = 2,			//HLS 解码流 内置HttpServer   
	};
	enum MediaFormate
	{
		MPEG = 0,
		FLV = 1
	};

	/*
	是否保存 如果保存，则保存什么类型 用于扩展 StreamInfo 的 bSaveVideo
	*/
	enum SaveVideoEnum
	{
		NotSave = 0,
		SaveMp4 = 1,
		SaveFlv = 2,
	};
	/// <summary>
	/// hard code 實例要修改對應參數
	/// 如果要從雲端獲取則要從幾個API綜合獲取對應參數。
	/// API:device_by_serial_no;setting_n_schedule_by_camera_id
	/// 還有一個是邏輯轉換的api camera_list_trans_to_strean_info， 主要是把雲端邏輯轉換為具體情況的：
	/// 例如：bRtmp = ttrue是開啟RTMP的，但新規則改為 StreamDecodeType (StreamInfo.nStreamDecodeType)的類型，HLS/RTMP/進行切換。
	/// 目前網上雲端沒有bRtmp對應的參數，對應的 CamSettingNSchedule。RtmpOutput和雲端對應的沒有UI改動的，只有默認FALSE.所有對RTMP比較混亂的，所以默認就是FALSE
	/// </summary>
	struct StreamInfo
	{
		int nCameraId = 0;										//No.1	 CameraId
		std::string rtspIp = "192.168.0.111";					//No.1b  补充Camera IP
		int nHDType = kHWDeviceTypeNone ;					    //No.2   hard device accelate   kHWDeviceTypeNone = 0 kHWDeviceTypeCUDA kHWDeviceTypeNone;
		std::string strInput;									//No.3   input rtsp://root2:123456@192.168.10.90:554/axis-media/media.amp?videocodec=h264&resolution=640x480
		bool bSavePic = false;									//No.4   save frame
		int mediaFormate = (int)MediaFormate::MPEG;				//No.5   存储为MPEG 
		int savePictRate = 40;									//No.7   帧率/savePictRate = 保存的频率 每25帧保存5帧 
		bool bSaveVideo = false;								//No.8   save video 
		int64_t nVideoTime = 60 * 1000 * 5; ;					//No.9   录像时间 time for each video
		int  nStreamDecodeType = StreamDecodeType::NOSTREAM;	//No.10  bRtmp 改为解码流类型
		std::string strOutput = "rtmp://127.0.0.1:1935/live/"
			+ std::to_string(nCameraId);						//No.11

		int nWidth = 0;											//No.12  system default value  
		int nHeight = 0;										//No.13  system default value
		int nPixFmt = -1;										//No.14  system default value // AVPixelFormat::AV_PIX_FMT_NONE = -1,

		int nFrameRate = 25;									//No.15  帧率 只用于整除计算多少帧保存一次图片 system default value
		int nVideoIndex = -1;									//No.16  system default value
		int nAudioIndex = -1;									//No.17  system default value
		int nRefCount = 0;										//No.18  av_dict_set(&pOptions, "refcounted_frames", m_infoStream.nRefCount ? "1" : "0", 0);
		bool bRtmp = false;										//No.19  保留用于兼容AudioStreamHandle和UsbStreamHandle 后续开发这两个需要去掉，统一使用RtspStreamHandle的逻辑
		bool cameraStatus = true;								//No.20  鏡頭狀態 0:停用;  1:啟用
	};



	enum FileNameType
	{
		kFileTypePicture = 0,       // picture
		kFileTypeVideo = 1,         // video/MP4
		kFileTypeRtmp = 2,          // rtmp
		kFileTypeAudio = 3,         // audio
		kFileTypeHls = 4,         // HLS
	};

	enum class FType
	{
		kFileTypePicture = 0,       // picture
		kFileTypeVideo = 1,         // video(MP4/FLV)
		kFileTypeRtmp = 2,          // rtmp
		kFileTypeAudio = 3,         // audio
		kFileTypeHls = 4,         // hls
	};

	enum CameraRecordStatus
	{
		IN_STOP = 0,//錄像停止状态
		IN_RECORD = 1, //錄像進行中
		SUSPEND_RECORD = 2 //錄像暫停中
	};

	/// <summary>
	/// 和雲端系統一一對應的任務類型 
	/// 部分不啟動是因為待開發中，完成一個功能就啟動一個功能
	/// </summary>
	enum class TaskType
	{
		/// <summary>
		/// 任務類型: UNDEFINED 未定義的任務
		/// </summary> 
		UNDEFINED = 0,

		///// <summary>
		///// 任務類型:CAMERA_GUARD 桌面人臉識別系統
		///// </summary> 
		//CAMERA_GUARD = 1,

		///// <summary>
		///// 任務類型: CAMERA_DVR 桌面錄像系統
		///// </summary> 
		//CAMERA_DVR = 2,

		///// <summary>
		///// 任務類型: HIK_DATA_RETRIVE 桌面海康設備數據獲取系統
		///// </summary> 
		//DESKTOP_HIK_DATA_RETRIVE = 3,

		///// <summary>
		///// 任務類型: HIK_DATA_ANDROID_RETRIVE  手機版的海康設備數據獲取系統
		///// </summary> 
		//ANDROID_HIK_DATA_RETRIVE = 4,

		///// <summary>
		///// 任務類型: ANDROID_CIC_DATA_RETRIVE  手機版的 CIC的NFC拍卡數據獲取系統
		///// </summary> 
		//ANDROID_CIC_DATA_RETRIVE = 5,

		/// <summary>
		/// 任務類型: 车牌识别 POST到Python处理后返回
		/// </summary> 
		CAR_PLATE_RECOGNITION = 6,

		/// <summary>
		/// 任務類型: 人脸识别
		/// </summary>
		FACE_RECOGNITION = 7,

		/// <summary>
		/// 任務類型: 工程着装识别
		/// </summary> 
		WORK_CLOTHES_RECOGNITION = 8,

		/// <summary>
		/// 任務類型: 佩戴头盔识别
		/// </summary> 
		WEARING_HELMET_RECOGNITION = 9,

		/// <summary>
		/// 任務類型: 有人闖入
		/// </summary> 
		SOMEONE_BROKE_IN_ENABLE = 10
	};
};

#pragma once
 
#ifdef _WIN32
#include <io.h>
#endif
#include "Common.h"
#include "StreamDefine.h"
#include "ManagerController.h" 
#include "StreamHandle.h"
#include "StreamManager.h"
#include "./interface/CameraMpeg.h" 
#include "httpserver/comm.h"
#include "httpserver/httpserver.h"

#define MEDIA_GUARD_VERSION 3.2

using namespace Stream; 

GlobalSetting g_GlobalSetting; //全局配置
// std::atomic需使用默认构造初始化, 不然有的编译器会因不支持拷贝构造导致编译不通过
std::atomic<bool> g_bInited(false);
std::atomic<bool> g_bExit(false);
std::atomic<bool> main_exit(false); //通过信号判断是否退出
std::mutex g_mtLock;
std::condition_variable g_cvCond;
std::thread g_thRun;
std::thread g_thPict;
std::thread g_thInterneIp;
std::thread g_thVideoStore;  

typedef std::list<StreamMangement> StreamMgtList;

typedef std::shared_ptr<StreamMangement> StreamHandlePtr;  //共享指針

ManagerController::ManagerController()
{ 
	// 启动线程池
	m_ThreadPollForCtrl.Start();

	g_bExit = false;

	// 获取device.json 節點 global_setting 的設置
	GlobalSetting globalSetting;  
	globalSetting.picRemainMinutes = DEVICE_CONFIG.cfgGlobalSetting.picRemainMinutes;
	globalSetting.storageLimitedbytes = DEVICE_CONFIG.cfgGlobalSetting.storageLimitedbytes;
	globalSetting.videoRemainMinutes = DEVICE_CONFIG.cfgGlobalSetting.videoRemainMinutes;
	// 硬件解碼 device.json->global_setting->nHDType
	globalSetting.nHDType = ManagerController::StrToHWDeviceType(DEVICE_CONFIG.cfgGlobalSetting.nHDType);

	// 规则保留最少15分钟的video文件
	if (globalSetting.videoRemainMinutes < 15) 
		globalSetting.videoRemainMinutes = 15;

	g_GlobalSetting = globalSetting;

	Init();
}

ManagerController::~ManagerController()
{
}

/* 每秒運行邏輯 是用於判斷 對時間比較敏感的目錄進行判斷*/
void run()
{   
	main_exit = false;
	bool bFirstRun = true;
	 
	while (!g_bExit.load())
	{ 
		if (bFirstRun)  
		{ 
			fs::path video_path = fs::current_path() / kVideoDir / Time::GetCurrentDate();
			//fs::path audio_path = fs::current_path() / kAudioDir / Time::GetCurrentDate();
			 
			if (!File::isDirectoryExists(video_path.string()))
				File::CreateSingleDirectory(video_path.string());

			//audio文件夹暂时没用上，不启动创建
			/*if (!File::isDirectoryExists(audio_path.string()))
				File::CreateSingleDirectory(audio_path.string());*/
			  
			bFirstRun = false;
		}
		   
		SHARED_LOCK(g_mtLock);
		g_cvCond.wait_for(locker, std::chrono::milliseconds(1000), []() {
			auto isExit = g_bExit.load();
			//TEST
			//std::cout << Time::GetCurrentSystemTime() << "  g_bExit.load():" << isExit << std::endl;
			return isExit;
			});
	}
}

/* 每3分鐘運行一次 */
void clean_picture_run()
{
	int clean_picture_circle_mins = 3; //每3分鐘運行一次
#ifdef  DEBUG
		clean_picture_circle_mins = 1; //測試使用 1分鐘循環清理圖片
#endif //  DEBUG

	while (!g_bExit.load())
	{
		ManagerController::clean_picture(g_GlobalSetting.picRemainMinutes);
		SHARED_LOCK(g_mtLock);
		g_cvCond.wait_for(locker, std::chrono::minutes(clean_picture_circle_mins), []() {  
			auto isExit = g_bExit.load(); 
			return isExit;
		});
	}
}

/* 每16分鐘運行一次公網同步 */
void heartbean_nat_internet_ip_run()
{
	int heartbean_circle_mins = 16; //每16分鐘運行一次公網同步
#ifdef  DEBUG
	heartbean_circle_mins = 1; //測試使用 1分鐘循環清理圖片
#endif //  DEBUG

	while (!g_bExit.load())
	{
		ManagerController::update_device_nat_internet_ip();
		SHARED_LOCK(g_mtLock);
		g_cvCond.wait_for(locker, std::chrono::minutes(heartbean_circle_mins), []() {
			auto isExit = g_bExit.load();
			return isExit;
			});
	}
}

/*
* 定時清理錄像
*/
void clean_video_store_run()
{
	while (!g_bExit.load())
	{
		ManagerController::clean_video_store();
		SHARED_LOCK(g_mtLock);
		g_cvCond.wait_for(locker, std::chrono::milliseconds(60000), []() {  //test 60's 实际设置长点，过于频繁不好，消耗资源
			auto isExit = g_bExit.load();
			//TEST
			//LOG(INFO) << "clean_video_store_run : 10's isExit=" << isExit << std::endl;
			return isExit;
			});
	}
}


void ManagerController::Init()
{
	if (!g_bInited.load()) {
		  
		g_thRun = std::thread(run);
		g_thPict = std::thread(clean_picture_run);
		g_thInterneIp = std::thread(heartbean_nat_internet_ip_run);
		g_thVideoStore = std::thread(clean_video_store_run);

		bool bInited = false;
		if (g_bInited.compare_exchange_strong(bInited, true, std::memory_order_acq_rel)) {
			// it needs do this while open dshow
			avformat_network_init();  //增加网络初始化
			avdevice_register_all();
			//显示 ffmpeg log记录等级
			if (DEVICE_CONFIG.cfgGlobalSetting.ffmpegOpenInfo)
				av_log_set_level(AV_LOG_INFO);
			else
				av_log_set_level(AV_LOG_ERROR);

		}

		// for clean hls index.m3u8 cache ts files 作废 不需要使用这个函数 已经在传入字典中定义清理碎片
		//m_thread_hls_clear = std::thread(std::bind(&ManagerController::clean_hls_ts_run, this));
	}
}

/*
* A  撤換測試模式
1、單個實例的方式，硬數據傳入  批處理啟動 測試用
* 對應修改 infoStream.strInput = "rtsp://admin:Admin123@192.168.2.64:554";
*/
void ManagerController::run_media_list()
{
	//这里是把一个带有infoStream信息配置的rtsp列表 批处理启动StreamMangement对象
	//任意键停止也是通过列表批处理停止
	//现在是测试环境下 独立一个运行

	Stream::StreamInfo infoStream;
	infoStream.nCameraId = 12;
	infoStream.bSavePic = true;
	infoStream.mediaFormate = MediaFormate::FLV;
	infoStream.bSaveVideo = true;
	infoStream.savePictRate = 25;  //25%12 = 保存n张图片
	infoStream.nVideoTime = 60 * 1000 * 1; //60seconds*5  (record duration :5minutes)
	infoStream.nStreamDecodeType = (int)StreamDecodeType::HLS;
	// input
	//infoStream.strInput = "rtsp://admin:Admin123@192.168.2.64:554";
	infoStream.strInput = "video.mp4";

	std::cout << "Input Rtsp Url if need:" << "\n";
	std::string rtsp_url;
	getline(std::cin, rtsp_url);
	//std::cout << rtsp_url << std::endl;
	if (rtsp_url != "")
		infoStream.strInput = rtsp_url;

	// the camera in notebook
   // infoStream.strInput = "video=USB2.0 HD UVC WebCam";
	//infoStream.strInput = "video=USB2.0 PC CAMERA";
   // infoStream.strInput = "rtsp://root2:123456@192.168.10.90:554/axis-media/media.amp?videocodec=h264&resolution=640x480";

	// output
	//infoStream.strOutput = "test.mp4";
	infoStream.strOutput = "http://127.0.0.1:180/hls/0/index.m3u8";

	// hard device accelate
	//是否使用硬件加速，需要已知機器配置
	//infoStream.nHDType = Stream::kHWDeviceTypeNone; 此傳入參數作廢 改用程式判斷是否有顯卡
	infoStream.nHDType = Stream::kHWDeviceTypeCUDA;
	//infoStream.nHDType = Stream::kHWDeviceTypeDXVA2;

	//RTSP方式 RTSP模式录像:   
	int golPicRemainMinutes = 2;
	StreamMangement hStream(Stream::kStreamTypeRtsp);
	  
	infoStream.nStreamDecodeType = (int)StreamDecodeType::HLS; //输出hls
	hStream.StartDecode(infoStream);

	ManagerController::signal_check_main(); //判断ctrl+c ctrl+b 提示exit 退出

	//LOG(INFO) << "It'll exit in few seconds normally!"; 
	std::cout << "It'll exit in few seconds normally!" << std::endl;

	//任意键退出 前需要关闭其他线程  多个rtsp handle单元则for list退出
	hStream.StopDecode();

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

	std::cout << "It'll exit in few seconds." << "\n";

	system("pause");
}

/*
* B 正式運行
* 2、多個鏡頭 共享指針隊列任務 批處理啟動
* typedef std::list<std::shared_ptr<StreamMangement>>方式獲取實例指針  批處理啟動
*/
void ManagerController::run_media_batch_list()
{
	std::cout << "\nDEVICE_CONFIG::DEVICE SERIAL NUMBER : " << DEVICE_CONFIG.cfgDevice.device_serial_no << " | " << " Storage Limited bytes : " << to_string(DEVICE_CONFIG.cfgGlobalSetting.storageLimitedbytes) << "\n" << std::endl;
	if (DEVICE_CONFIG.cfgDevice.device_serial_no.length() == 0)
	{
		//LOG(INFO) << "device.json uft8 unformat!!!(uft8 without bom) [System Pause]" << std::endl;;
		std::cout << "device.json uft8 unformat!!!(uft8 without bom) [System Pause]" << std::endl;;
		system("pause");
		return;
	}
	//獲取鏡頭列表
	CameraMpeg cameraMpeg;
	Service::StreamInfoApiList sList;
	bool get_list_succ = true;
	int i = 0;
	while (get_list_succ) {
		int nCode = cameraMpeg.camera_list(sList);
		if (nCode != CP_OK)
		{
			//LOG(INFO) << "func::cameraMpeg.camera_list -> GET CAMERA LIST FROM CLOUD FAIL!!!";
			std::cout << "func::cameraMpeg.camera_list -> GET CAMERA LIST FROM CLOUD FAIL!!!" << std::endl;
		}
		else
			get_list_succ = false;

		if (i > 10)  //最多10次嘗試
		{
			//LOG(INFO) << "func::cameraMpeg.camera_list -> TRY TO GET MORE THAN 10 Times, FAIL!!!";
			std::cout <<  "func::cameraMpeg.camera_list -> TRY TO GET MORE THAN 10 Times, FAIL!!!" << std::endl;
			break;
		}
	}

	Service::StreamInList listx;
	//獲取鏡頭列表對應的 StreamInfo(鏡頭解碼傳入參數結構體) 
	//由於這個 StreamInfo結構體 的參數來自介個方面: 來自本地device.json, 雲端API->鏡頭列表\Camera list Schedule 等等綜合起來的結構體對象
	//所以需要一個特定的函數處理綜合性業務
	bool ret_trans = cameraMpeg.camera_list_trans_to_strean_info(sList, listx);
	if (!ret_trans)
	{
		//LOG(INFO) << "func::cameraMpeg.camera_list_trans_to_strean_info -> TRANS TO STREAM INFO FAIL!!!";
		std::cout << "func::cameraMpeg.camera_list_trans_to_strean_info -> TRANS TO STREAM INFO FAIL!!!" << std::endl;
	}
	
	//鏡頭的傳入參數列表 
	Service::StreamInList::iterator itr;
	for (itr = listx.begin(); itr != listx.end(); ++itr)
	{
		std::cout << "\nCamera Rtsp = " << itr->strInput << " ---------------------- \n" << std::endl;

		//用途 : 例如, RtspStreamHandle::StartDecode(const StreamInfo& infoStream)
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
		streamInfo.nStreamDecodeType = itr->nStreamDecodeType; //解碼類型直接確定撤換到那個類型: RtspStreamHandle.cpp/AudioStreamHandle.cpp/UsbStreamHandle.cpp
		streamInfo.strOutput = itr->strOutput;
		streamInfo.nWidth = itr->nWidth;
		streamInfo.nHeight = itr->nHeight;
		streamInfo.nPixFmt = itr->nPixFmt;
		streamInfo.nFrameRate = itr->nFrameRate;
		streamInfo.nVideoIndex = itr->nVideoIndex;
		streamInfo.nAudioIndex = itr->nAudioIndex;
		streamInfo.nRefCount = itr->nRefCount;
		streamInfo.bRtmp = itr->bRtmp;   //false;  //不開啟rtmp, 雲端默認就是 false 所以這裡無需要Hard Code
		streamInfo.cameraStatus = itr->cameraStatus;

		if (streamInfo.cameraStatus)
		{
			std::shared_ptr<StreamMangement>  m_StreamPtrHandle = std::make_shared<StreamMangement>(Stream::kStreamTypeRtsp);

			bool isStart = m_StreamPtrHandle->StartDecode(streamInfo);

			if (isStart)
			{
				m_StreamPtrList.push_back(m_StreamPtrHandle);
			}
		}
		else {
			std::cout << "\n[WARNING] Camera Id = " << streamInfo.nCameraId << "\t Current Camera Status : 0 (Disable)" << " need to update status from the cloud or local camera_list.json -> state ---------------------- \n" << std::endl;
		}
	}

	//輸入exit則退出
	ManagerController::signal_check_main(); //判断ctrl+c ctrl+b 提示exit 退出

	//LOG(INFO) << "It'll exit in few seconds normally!";
	std::cout << "It'll exit in few seconds normally!" << std::endl;

	//任意键退出 前需要关闭其他线程  多个rtsp handle单元则for list退出

	for (auto itr_mgt = m_StreamPtrList.begin(); itr_mgt != m_StreamPtrList.end(); ++itr_mgt)
	{
		(*itr_mgt)->StopDecode();  //auto x = (std::shared_ptr<StreamMangement>)(*itr_mgt);

		//LOG(INFO) << "CameraId = " << (*itr_mgt)->m_infoStream.nCameraId << " PLEASE WAIT TO STOP IN TEN SECONDS, " << (*itr_mgt)->m_infoStream.rtspIp << std::endl;
		std::cout << "CameraId = " << (*itr_mgt)->m_infoStream.nCameraId << " PLEASE WAIT TO STOP IN TEN SECONDS, " << (*itr_mgt)->m_infoStream.rtspIp << std::endl;
		//例如 全局圖片List需要時間去處理。 關閉時間不足，導致圖片識別業務線程池未處理完畢導致變量關閉無法讀取
		std::this_thread::sleep_for(std::chrono::milliseconds(9000)); 

		//LOG(INFO) << "CameraId = " << (*itr_mgt)->m_infoStream.nCameraId << " COMPLETED TO STOP CAMERA ID = " << (*itr_mgt)->m_infoStream.nCameraId << " CAMERA IP " << (*itr_mgt)->m_infoStream.rtspIp << std::endl;
		std::cout << "CameraId = " << (*itr_mgt)->m_infoStream.nCameraId << " COMPLETED TO STOP CAMERA ID = " << (*itr_mgt)->m_infoStream.nCameraId << " CAMERA IP " << (*itr_mgt)->m_infoStream.rtspIp << std::endl;

	}
	
	m_StreamPtrList.clear();

	std::this_thread::sleep_for(std::chrono::milliseconds(3000));

	std::cout << "It'll exit in few seconds." << "\n";

	system("pause");
}
 
void ManagerController::Uninit()
{
	g_bExit.store(true);
	g_cvCond.notify_all();
	if (g_thRun.joinable())
		g_thRun.join();

	if (g_thPict.joinable())
		g_thPict.join();
	 
	if (g_thInterneIp.joinable())
		g_thInterneIp.join();

	if (g_thVideoStore.joinable())
		g_thVideoStore.join();
}

/*
	获取piture 目录下的每个子目录 并判断目录下的图片是否已经过期
	picRemainMinutes 单位是分钟
*/
void ManagerController::clean_picture(int64_t picRemainMinutes)
{
	//限制最大值6分鐘 兩個鏡頭 15分鐘 1500張圖片,數量太大,影響性能 
	if (picRemainMinutes > 6)
		picRemainMinutes = 6;

	//如果0分鐘 則 使用內存保存,這裡不執行
	if (picRemainMinutes == 0)
		return;

	const fs::path picture_path = fs::current_path().append("picture"); //ManagerController::current_working_directory();
  
	//tu-tu said:
	//c++17 跨平台 std::fs::current_path() 
	//如果编译器支持C++17，则建议使用std::filesystem::current_path
	//如果只在windows平台使用，可使用_getcwd
	//如果只在linux平台使用，可使用getcwd
	//如果代码要跨平台使用，可以使用通过编译统一的版本 
	//ref：https ://www.jianshu.com/p/2d9a5e0c7c48
	   
	std::vector<std::string> vecFile;
	File::GetFilesOfDir(picture_path.string(), vecFile);

#ifdef DEBUG
	//定時清理圖片
	std::cout << "\n[Folder Picture :" << picture_path.string() << "] [Total Files :" << vecFile.size() << "}\n" << std::endl;
#endif 
	

	//文件夾存在文件的情況下判斷是否大於設置的15分鐘
	if (vecFile.size() > 0)
	{
		for (size_t i = 0; i < vecFile.size(); i++) {

			const std::string picutre_path_filename = vecFile[i].c_str();

			if (fs::exists(picutre_path_filename))
			{
				// 獲取文件的最後寫入時間 std::filesystem::file_time_type
				auto ftime = fs::last_write_time(picutre_path_filename); // ftime 是 file_time_type 的一個實例
				// 將 file_time_type 轉換為 system_clock::time_point
				auto sctp = ManagerController::to_time_t(ftime); // 使用 clock 來獲取 time_t

				//當前的时间
				auto curr = std::chrono::system_clock::now();

				//device.json -> picRemainMinutes (15分鐘)前的時間
				auto before_minutes = curr - std::chrono::minutes(picRemainMinutes); //

				auto before_minutes_t = std::chrono::system_clock::to_time_t(before_minutes);

				if (sctp < before_minutes_t)
				{
					try {

						fs::path picture_path_file(picutre_path_filename);
						File::deleteFile(picutre_path_filename);
					}
					catch (...) {
						std::cout << Time::GetCurrentSystemTime() << " " << picutre_path_filename << " [EXCEPTION] DELETED FAIL" << std::endl;
					}
				}
				//测试完毕注释掉2023-3-16
				else {
#ifdef DEBUG
					std::cout << "[Delete Picture] " << picutre_path_filename << "/t"
						<< Time::GetCurrentSystemTime() << "[file not in "
						<< picRemainMinutes << " minutes ago] file last modified (ftime):"
						<< std::ctime(&sctp);
#endif 
				}
			}
		}
	} 
}

template <typename TP>
std::time_t ManagerController::to_time_t(TP tp)
{
	using namespace std::chrono;
	auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
		+ system_clock::now());
	return system_clock::to_time_t(sctp);
}

// 將字符串轉換為 HWDeviceType 的函數
HWDeviceType ManagerController::StrToHWDeviceType(const std::string& str)
{
	if (str == "kHWDeviceTypeNone") return kHWDeviceTypeNone;
	if (str == "kHWDeviceTypeVDPAU") return kHWDeviceTypeVDPAU;
	if (str == "kHWDeviceTypeCUDA") return kHWDeviceTypeCUDA;
	if (str == "kHWDeviceTypeVAAPI") return kHWDeviceTypeVAAPI;
	if (str == "kHWDeviceTypeDXVA2") return kHWDeviceTypeDXVA2;
	if (str == "kHWDeviceTypeQSV") return kHWDeviceTypeQSV;
	if (str == "kHWDeviceTypeVIDEOTOOLBOX") return kHWDeviceTypeVIDEOTOOLBOX;
	if (str == "kHWDeviceTypeD3D11VA") return kHWDeviceTypeD3D11VA;
	if (str == "kHWDeviceTypeDRM") return kHWDeviceTypeDRM;
	if (str == "kHWDeviceTypeOPENCL") return kHWDeviceTypeOPENCL;
	if (str == "kHWDeviceTypeMEDIACODEC") return kHWDeviceTypeMEDIACODEC;

	// 如果沒有匹配，返回一個預設值
	return kHWDeviceTypeNone; // 或者根據需要處理錯誤情況
}

/*
	如果指定限定存储容量，则执行清理存储的媒体文件，不包括图片，图片属于上面clean_picture函数的逻辑
	如果设置容量太小不适合此规则，例如100m，导致一个文件都容不下的情况下，录像中的文件可能会被卡住。或删除失败的情况
*/
void ManagerController::clean_video_store()
{
	if (g_GlobalSetting.storageLimitedbytes == 0) //不限制容量
		return;

	const fs::path video_path = fs::current_path().append("video");
	 
	if (!fs::is_directory(video_path))
	{
		//LOG(INFO) << "NO SUCH DIRECTORY [VIDEO] " << video_path << "\n" << std::endl;
		std::cout << "NO SUCH DIRECTORY [VIDEO] " << video_path << "\n" << std::endl;
		return;
	}

	std::vector<std::string> vecDires;
 
	File::GetDirsOfDir(video_path.string(), vecDires);

	//TEST
	//LOG(INFO) << "video_folder =" << video_folder << std::endl;

	int64_t storage_capacity = get_filesize(video_path.string());

	for (size_t i = 0; i < vecDires.size(); i++) {

		fs::path date_video_path = vecDires[i];
		  
		if (File::isDirectory(date_video_path.string()))
		{
			std::vector<std::string> vector_date_file;
			File::GetFilesOfDir(date_video_path.string(), vector_date_file);

			for (size_t j = 0; j < vector_date_file.size(); j++)
			{
				if (storage_capacity > g_GlobalSetting.storageLimitedbytes)
				{ 
					int iCreateTime, iFileLen;
					if (true ==File:: get_file_info(vector_date_file[j].c_str(), iCreateTime, iFileLen))
					{
						//规则保留最少15分钟的文件
						//获取多少分钟前的时间
						std::chrono::system_clock::time_point curr = std::chrono::system_clock::now();
						std::chrono::system_clock::time_point before_minutes_time = curr - std::chrono::minutes(g_GlobalSetting.videoRemainMinutes);
						auto longremaintime = std::chrono::duration_cast<std::chrono::seconds>(before_minutes_time.time_since_epoch());
						int64_t longCreateTime = static_cast<int64_t>(iCreateTime);
						if (longCreateTime < longremaintime.count())
						{
							fs::path video_path_file(vector_date_file[j]);
							bool del_res = File::deleteFile(video_path_file.string());
						}

						//删除后再次检查是否满足限制容量的要求，如果满足则退出删除检查循环
						storage_capacity = get_filesize(video_path.string());
						if (storage_capacity < g_GlobalSetting.storageLimitedbytes)
						{ 
							break;
						}
					}
					else {
						//无法获取文件信息 File::get_file_info(...)
					}
				}
				storage_capacity = get_filesize(video_path.string());
				 
			}
		}
  
		//最后判断非当天的文件夹是否为NULL 规则：没有文件的文件夹删除
		std::vector<std::string> vec_check_blank_date_file;
		File::GetFilesOfDir(vecDires[i].c_str(), vec_check_blank_date_file);
		int64_t date_folder_files_count = vec_check_blank_date_file.size();
		const std::string todate = Time::GetCurrentDate();
		if (date_folder_files_count == 0)
		{
			if (vecDires[i].find(todate) == vecDires[i].npos)
			{
				fs::remove_all(vecDires[i]); 
				//TEST
				//LOG(INFO) << "Delete Date Folder Result: " << date_folder << "del_date_folder_res =" << del_date_folder_res << std::endl;
			}
		}

		storage_capacity = get_filesize(video_path.string()); //已经在子循环内进行检查
		if (storage_capacity < g_GlobalSetting.storageLimitedbytes)
		{
			break;
		}
	}
}

std::string ManagerController::current_working_directory()
{ 
	return fs::current_path().string();
//#ifdef _WIN32
//	char buff[MAX_PATH];
//	_getcwd(buff, MAX_PATH);
//	std::string current_working_directory(buff);
//	return current_working_directory;
//#elif __linux__
//	
//#include <unistd.h>
//#include <limits.h> 
//
//	char buff[PATH_MAX];
//	if (getcwd(buff, PATH_MAX) != NULL) {
//		std::string current_working_directory(buff);
//		return current_working_directory;
//	}
//	else {
//		return "";  // 或者返回其他錯誤處理
//	}
//	 
//#endif
}
 
/*
https ://blog.csdn.net/weixin_47826078/article/details/117751305
cout<<"test文件夹的大小："<< get_filesize(path)<<"bytes"<<endl;
获取文件大小的函数，返回值以KB为单位
*/
double ManagerController::get_length(string filePath) {
	double size_byte = 0;

	ifstream fin(filePath);
	if (fin.is_open())
	{
		fin.seekg(0, ios::end);
		double size = fin.tellg();
		if (size == 0) {
			size_byte = 0;
		}
		else {
			size_byte = size;
		}
		fin.close();
	}
	return size_byte;
}
/*
* 用法
* string path("C:\\Users\\SeanWayen\\Desktop\\test");
* cout<<"test文件夹的大小："<< get_filesize(path)<<"bytes"<<endl;
*/
int64_t ManagerController::get_filesize(string path)//递归核心代码
{
#ifdef _WIN32
	//文件句柄
	intptr_t hFile = 0;
	vector<string> files;
	//文件信息
	struct _finddata_t fileinfo;
	string p;
	double filesize = 0;
	int i = 0;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))//判断是否是文件夹 
			{
				//如果是目录,递归查找并累积size
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0) {
					filesize += ManagerController::get_filesize(p.assign(path).append("\\").append(fileinfo.name));
				}
			}
			else
			{
				//如果不是，直接累加size
				filesize += ManagerController::get_length(path + "\\" + fileinfo.name);
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
	int64_t size_byte_int64 = static_cast<int64_t>(round(filesize)); // round(filesize);

	return size_byte_int64;
#elif __linux__
	struct stat fileStat;

	if (stat(path.c_str(), &fileStat) == 0) {
		return fileStat.st_size;
	}
	else {
		return -1; // 獲取文件大小失敗
	}
	return 0;
#endif
}

/* 
* 防止誤操作導致程序中斷 ctrl+c 無效
*/
void ManagerController::valid_op(int n)
{
	if (n == SIGINT)
	{
		std::cout << "\n按下 ctrl+c 無效！請輸入 exit\nPressing ctrl+c has no effect! Please enter exit\n" << std::endl;
		signal(SIGINT, valid_op);
		main_exit = false;
	}
	if (n == SIGABRT)
	{
		std::cout << "\nSIGABRT：因中止呼叫而觸發的異常終止\nSIGABRT: Abnormal termination triggered by abort call\n";
		main_exit = false;
	}
}
 
/*
* 判斷 ctrl+c ctrl+break 等等終斷操作的 拒絕操作提示返回.
*/
void ManagerController::signal_check_main()
{
	std::string input;

	while (!main_exit.load())
	{
		if (input != "")
		{
			transform(input.begin(), input.end(), input.begin(), ::tolower);
		}
		else {
			std::cout << "\nInput exit to treminate!\n\n" << std::endl;
		}
 
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		if (std::cin.fail())
		{
			std::cout << "\nInput exit to treminate!\n" << std::endl;
			std::cin.clear();
			std::cin.sync();
		}
		if (input == "exit")
		{
			//LOG(INFO) << "\nnow will be exit as you input = " << input << "\n";
			std::cout << "\nnow will be exit as you input = " << input << "\n";
			main_exit = true;
			break;
		}
		else if (input != "") {

			std::cout << "\nyou input :(" << input << ") invalid,input exit to end.\n" << std::endl;
		}

		getline(std::cin, input);

		signal(SIGINT, valid_op); // /* 處理 SIGINT 信號 注册ctrl+c信号捕获函数*/
		signal(SIGABRT, valid_op); // 處理 SIGABRT 信號 | SIGABRT : abnormal termination triggered by abort call 
		  
	};
}

/*主控台初始化顯示的系統環境信息與程序配置信息 例如 Device.json的設置*/
void ManagerController::main_initialize()
{

// 設置終端標題
#ifdef _WIN32
	SetConsoleTitle("\nMedia Guard Ver3.2");
#elif __linux__ 
	std::cout << "\033]0;Media Guard Ver3.2\007";
#endif

	//系統信息參考 SYSTEM INFORMATION REFERENCE 提供部署安裝的系統與設備信息
#pragma region HeaderRegion 系統信息參考 SYSTEM INFORMATION REFERENCE 提供部署安裝的系統與設備信息
	
	//首次運行1次 更新主體設備公網IP到雲端
	ManagerController::update_device_nat_internet_ip();

	//公共IP查詢 Public internet IP Reference
	std::cout << "\n===================== PUBLIC INTERNET IP REFERENCE =====================\n" << std::endl;
	NatHeartBean natHeartBean;

	std::string public_ip = natHeartBean.get_public_ip_by_curl();
	std::string public_ip2 = natHeartBean.get_public_ip_by_curl_memory();
	
	if (!public_ip.empty()) {
		std::cout << "Public IP Method Curl 1: " << public_ip << "------------------------------save to: ./public_ip_by_curl.txt " << std::endl;
		std::cout << "Public IP Method Curl 2: " << public_ip2 << std::endl; 
	}
	else {
		std::cout << "Failed to retrieve public IP." << std::endl;
	}
	 
	//系統信息參考 System Information Reference
	std::cout << "\n===================== SYSTEM INFORMATION REFERENCE =====================\n" << std::endl;

	printf("OpenSSL version: %s\n", OpenSSL_version(OPENSSL_VERSION));

	// 初始化 FFmpeg 
	std::cout << "\n===================== FFMPEG AVFORMAT NETWORK INIT =====================\n" << std::endl;
	avformat_network_init();

	// 獲取 FFmpeg 的版本號
	const char* ffmpeg_version = av_version_info();
	std::cout << "\nFFmpeg Version: " << ffmpeg_version << std::endl;
	unsigned codec_version = avcodec_version();
	std::cout << "\nFFMPEG AVCODEC VERSION: "
		<< (codec_version >> 16) << "."        // 主版本號
		<< ((codec_version >> 8) & 0xFF) << "." // 次版本號
		<< (codec_version & 0xFF) << "\n" << std::endl; // 修訂版本號

#ifdef DEBUG
	printf("avcodec_configuration details: \n%s\n\n", avcodec_configuration()); //解碼配置
#endif // DEBUG

	std::cout << "\n\n\nPRESS ENTER TO CONTINUE .......\n\n\n" << std::endl; 
	char c1;
	std::cin.get(c1);

	// 應用程序配置信息
	std::cout << "\n===================== APP DEVICE CONFIGURATION INFOMATION =====================\n" << std::endl;
	 
	std::cout << "\nThis Device SerialNo:" << DEVICE_CONFIG.cfgDevice.device_serial_no << "\n" << std::endl;

	std::string main_root = current_working_directory();
	std::cout << "\n\nCurrent Working Directory:" << main_root << "\n" << std::endl;

	std::cout << "\nHttp Server Cloud:" << DEVICE_CONFIG.cfgHttpServerCloud.url << "\n" << std::endl;

	//列出支持的硬件
	std::cout << "\n===================== LIST SUPPORTED HARDWARES =====================\n" << std::endl;
	StreamMangement::list_supported_hardware();

	//列出電腦USB CAMERA
	std::cout << "\n===================== LIST DSSHOW DEVICES(USB CAMERAS) =====================\n" << std::endl;
	StreamMangement::list_dshow_device();


	std::cout << "\n\nIf it is running, you can terminate it by input exit\n" << std::endl;
	std::cout << "\n--------------------------------------------------------------------------------------\n" << std::endl;

	//--------------------------------------------------------------------------------------

	std::string test_url;
	httpserver::get_http_local_device_url(test_url);
	test_url = test_url.append("test");
	std::cout << "\nOpen test url to verlify [" << test_url << "] when started...................\n" << std::endl;

	std::cout << "\n\n\nPRESS ENTER TO START .......\n\n\n" << std::endl;
	char c2;
	std::cin.get(c2);
	  
#pragma endregion

	//clear screen
	ManagerController::clearScreen();

	std::cout << "NOW TO START......\n" << c2 << std::endl;
	std::cout << "\nHello The World...................\n" << std::endl;
	 
	//創建應有的程序文件夾 例如 Video , Picture , Hls 
	ManagerController::create_main_media_folder();
}

/// <summary>
/// 啟動 HttpServer
/// </summary>
void ManagerController::http_server_start()
{
	httpserver::http_server_run();
	return;
}

/* 
* 創建主要的文件夾 video ; hls ; picture ; 
* 必須放在 ManagerController::http_server_start http服務啟動之前
*/
void ManagerController::create_main_media_folder()
{
	fs::path video_path = fs::current_path() / kVideoDir;
	fs::path picture_path = fs::current_path() / kPictureDir;
	fs::path hls_path = fs::current_path() / kHlsDir;

	if (!File::isDirectoryExists(video_path.string()))
		File::CreateSingleDirectory(video_path.string());

	if (!File::isDirectoryExists(picture_path.string()))
		File::CreateSingleDirectory(picture_path.string());

	if (!File::isDirectoryExists(hls_path.string()))
		File::CreateSingleDirectory(hls_path.string());
}

/* 
* 清理主控屏幕
*/
void ManagerController::clearScreen() {

#ifdef _WIN32
	// Windows 平台
	system("cls");
#elif __linux__
	// Linux 平台
	system("clear");
#else
	std::cout << "\n\n清屏功能不支援此平台。\nThe clear screen function is not supported on this platform\n\n" << std::endl;
#endif
}
 
/* 獲取 設備對應的公網IP和端口 並且每15分鐘 同步心跳一次
*  注意 這裡是應用程序運行的平台設備(如MediaGuard + Liux 合成成為設備) 不是APP 下掛的設備
*/
void ManagerController::update_device_nat_internet_ip()
{ 
	if (DEVICE_CONFIG.cfgDevice.device_is_online_always == false)
	{
		return;
	}
	 
	CameraMpeg cameraMpeg;
	NatHeartBean natHeartBean;
	Service::DeviceInterNetIpInfo deviceInterNetIpInfo;

	std::cout << "\n===================== CLOUD REGISTRATION MAIN DEVICE INFOMATION =====================\n" << std::endl;
	
	Service::DeviceDetails deviceDetails;
	cameraMpeg.device_by_serial_no(deviceDetails);

	if (deviceDetails.deviceId.length() > 0)
	{
		std::cout << "\nMAIN DEVICE INFO OF CLOUD REGISTRATION:\n" << std::endl;
		std::cout << "MAIN DEVICE_SERIAL_NO REGISTED : " << deviceDetails.deviceSerialNo << "\n" << std::endl;
		std::cout << "MAIN DEVICE_ID REGISTED: " << deviceDetails.deviceId << "\n" << std::endl;
		std::cout << "MAIN DEVICE_NAME REGISTED : " << deviceDetails.deviceName << "\n" << std::endl;
		 
		deviceInterNetIpInfo.deviceId = deviceDetails.deviceId; 
		deviceInterNetIpInfo.internetIp = natHeartBean.get_public_ip_by_curl_memory();
		
		deviceInterNetIpInfo.localIp = DEVICE_CONFIG.cfgDevice.device_ip;
		deviceInterNetIpInfo.localPort = to_string(DEVICE_CONFIG.cfgDevice.device_port);
		
		std::cout << "MAIN DEVICE LOCAL SETTING IP AND PORT: " << deviceInterNetIpInfo.localIp << ":" << deviceInterNetIpInfo.localPort << "\n" << std::endl;
		std::cout << "MAIN DEVICE PUBLIC INTERNET IP AND PORT: " << deviceInterNetIpInfo.internetIp << ":" << deviceInterNetIpInfo.localPort << "------Cycle every 15 minutes:" << Time::GetCurrentSystemTime() << "\n" << std::endl;

		// 不使用STUN協議的路由端口,
		// 而是通過路由器設置的 NAT 轉發配置設備的端口要與路由器轉發的端口一致
		// 例如: 默認配置的端口是 180 則 路由器配置的轉發端口要和這個本地端口一致:180
		// 如果使用STUN協議 則需要 整體轉移到使用 STUN 技術規則
		deviceInterNetIpInfo.internetPort = to_string(DEVICE_CONFIG.cfgDevice.device_port);

		std::string strResponse;
		cameraMpeg.update_divice_internet_ip(deviceInterNetIpInfo, strResponse);
	}
	else {
		std::cout << "\nMAIN DEVICE IS REQUIRED TO REGIST ON THE CLOUD FIRST!!!!!!!!! " <<  "\n" << std::endl;
	}
}
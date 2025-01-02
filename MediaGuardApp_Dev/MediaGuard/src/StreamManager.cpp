#pragma once
#include "Common.h"
#include <vector>
#include <errno.h>
#include <thread>
#include "StreamManager.h"
#include "UsbStreamHandle.h"
#include "RtspStreamHandle.h"
#include "AudioStreamHandle.h" 
#include "StreamDefine.h"
#include "interface/CameraMpeg.h"

using namespace Stream;

StreamMangement::StreamMangement(Stream::StreamType nStreamType) : m_bExit(false)
{
	/*
	   1.为什么使用make_shared？ make_shared函数的主要功能是在动态内存中分配一个对象并初始化它，返回指向此对象的shared_ptr;由于是通过shared_ptr管理内存，因此一种安全分配和使用动态内存的方法
	*/
	m_pHandle = nullptr;
	switch (nStreamType)
	{
	case kStreamTypeUsb:
		m_pHandle = std::make_shared<UsbStreamHandle>();
		break;
	case kStreamTypeAudio:
		m_pHandle = std::make_shared<AudioStreamHandle>();
		break;
	case kStreamTypeRtsp:
		m_pHandle = std::make_shared<RtspStreamHandle>();
		break;
	default:
		m_pHandle = std::make_shared<RtspStreamHandle>();
		break;
	}
	m_bExit = false;

	//启动线程池
	m_thPollReconn.Start();
}

StreamMangement::~StreamMangement()
{
}

void StreamMangement::list_supported_hardware()
{
	std::stringstream ss;
	enum AVHWDeviceType nType = AV_HWDEVICE_TYPE_NONE;
	while ((nType = av_hwdevice_iterate_types(nType)) != AV_HWDEVICE_TYPE_NONE)
		ss << av_hwdevice_get_type_name(nType) << ",";
	printf("list_supported_hardware - Supported hard device:%s\n ", ss.str().c_str());
}

void StreamMangement::list_dshow_device()
{
	AVFormatContext* pFormatCtx = avformat_alloc_context();
	AVDictionary* options = NULL;
	av_dict_set(&options, "list_devices", "true", 0);
	AVInputFormat* iformat = (AVInputFormat*)av_find_input_format("dshow");
	 
	avformat_open_input(&pFormatCtx, "video=dummy", iformat, &options);
	printf("\n");
	avformat_free_context(pFormatCtx);
}

bool StreamMangement::StartDecode(const StreamInfo& infoStream)
{
	VERIFY_RETURN(check_handle());
	VERIFY_RETURN(check_para(infoStream));

	m_infoStream = infoStream;

	/*啟動之前獲取圖片AI業務功能開關狀態
	  例如：處理車牌識別（系統內部的），處理人臉識別的，處理頭盔，工作服識別
		bool CAR_PLATE_RECOGNITION_ENABLE = false;  //系統車牌識別
		bool FACE_RECOGNITION_ENABLE = false;		//人臉識別
		bool WORK_CLOTHES_RECOGNITION_ENABLE = false;  //工衣穿戴識別
		bool WEARING_HELMET_RECOGNITION_ENABLE = false; //頭盔識別
		bool SOMEONE_BROKE_IN_ENABLE = false; //有人闖入
	*/
	//獲取識別任務列表 
	CAR_PLATE_RECOGNITION_ENABLE = GetTaskInfoIfHave(m_infoStream.nCameraId, TaskType::CAR_PLATE_RECOGNITION, cameraTaskInfo_for_CAR_PLATE_RECOGNITION);

#ifdef DEBUG
	std::cout << "FUNC::CAR_PLATE_RECOGNITION_ENABLE=" << CAR_PLATE_RECOGNITION_ENABLE << std::endl;
#endif

	//-------------------------------------------------------------------------------------------------------------------------------------------------
	bool bIsStart = m_pHandle->StartDecode(infoStream);

	if (bIsStart) {

		m_bExit = false;

		  
		//把rtsp视频单元[RtspStreamHandle.cpp]转移到全局队列以作识别业务处理
		if (infoStream.bSavePic) {

			//TEST
			//LOG(INFO) << "m_thPictListToGlobal(std::thread) Begin to Run : func::PopFrameToList" << std::endl;
			m_thPictListToGlobal = std::thread(std::bind(&StreamMangement::PopFrameToList, this));

			//处理全局图片列表的队列
			m_thHandleFrameBusiness = std::thread(std::bind(&StreamMangement::HandleFrameRecogBusiness, this));
		}
	}
	return bIsStart;
}

void StreamMangement::StopDecode()
{
	VERIFY_RETURN_VOID(check_handle());

	m_pHandle->StopDecode();
	m_bExit = true;

	m_thPollReconn.Stop();  //關閉圖片識別業務線程池

	if (m_thHandleFrameBusiness.joinable())
		m_thHandleFrameBusiness.join();

	if (m_thPictListToGlobal.joinable())
		m_thPictListToGlobal.join();

	if (m_thChkConnect.joinable())
		m_thChkConnect.join();

	if (m_thPictListToGlobal.joinable())
		m_thPictListToGlobal.join();


	if (m_thHandleFrameBusiness.joinable())
		m_thHandleFrameBusiness.join();


	try {
		m_thPollReconn.Stop(); //停止线程
	}
	catch (std::exception ex)
	{
		//LOG(ERROR) << "func::StreamMangement::StopDecode()->m_thPollReconn.Stop() Exception what:" << ex.what();
		std::cout << "func::StreamMangement::StopDecode()->m_thPollReconn.Stop() Exception what:" << ex.what() << std::endl;
	}
}

bool StreamMangement::GetRtmpUrl(std::string& strRtmp)
{
	VERIFY_RETURN(check_handle());
	m_pHandle->GetRtmpUrl(strRtmp);

	return true;
}

bool StreamMangement::check_handle()
{
	if (nullptr == m_pHandle)
	{
		printf("Invalid stream handle\n");
		return false;
	}
	return true;
}

bool StreamMangement::check_para(const StreamInfo& infoStream)
{
	if (infoStream.strInput.empty())
	{
		printf("Invalid stream input\n");
		return false;
	}
	if (!(infoStream.bRtmp || infoStream.bSavePic || infoStream.bSaveVideo))
	{
		printf("[check_para]Nothing to start decode, all false : save picture, save video, save video of push rtmp\n");
		return false;
	}

	return true;
}

/// <summary>
/// 在這裡進行重連會出現邏輯問題
/// 或者這裡改為 心跳返回播放狀態
/// </summary>
void StreamMangement::m_check_connect() {

	if (m_pHandle != nullptr)
	{
		m_pHandle->get_stream_info(m_infoStream);

		while (!m_bExit.load())
		{
			CameraConnectingStatus cameraConnectingStatus = m_pHandle->get_camera_connecting_status();

			switch (cameraConnectingStatus)
			{
			case CameraConnectingStatus::InDisConnencted:  //意外断开 请求重连
			{
				StopDecode(); 
				//LOG(INFO) << "RTSP OF CAMERA RECONNECTING....(case=InDisConnencted) Status=" << std::to_string((int)cameraConnectingStatus);
				std::cout << "RTSP OF CAMERA RECONNECTING....(case=InDisConnencted) Status=" << std::to_string((int)cameraConnectingStatus) << std::endl;
				StartDecode(m_infoStream);
				break;
			}

			case CameraConnectingStatus::InRequestStopped:  //请求停止
			{
				StopDecode();
				//LOG(INFO) << "IN STOP REQUEST (CameraId=" << m_infoStream.nCameraId << ") STOPPING.... (case=InRequestStopped)RTSP URL" << m_infoStream.strInput << "Status=" << std::to_string((int)cameraConnectingStatus);
				std::cout << "IN STOP REQUEST (CameraId=" << m_infoStream.nCameraId << ") STOPPING.... (case=InRequestStopped)RTSP URL" << m_infoStream.strInput << "Status=" << std::to_string((int)cameraConnectingStatus) << std::endl;
				break;
			}
			default:
			{
				//FOR TEST
				//LOG(INFO) << "func::m_check_connect default switch case: (CameraId=" << std::to_string(m_infoStream.nCameraId) << ")  Status=" << std::to_string((int)cameraConnectingStatus) << " InPlaying = 1, RTSP URL:" << m_infoStream.strInput;
				//其他case还没有写
				break;
			}
			}

			//默认60秒运行一次重连 过于频密影响效能
			SHARED_LOCK(m_mtCheck);
			m_cvCheck.wait_for(locker, std::chrono::milliseconds(60000*15), [this]() {
				auto exit = m_bExit.load();
				//TEST 检查是否处于退出状态
				//std::cout << "\nfunc::m_check_connect exit =" << exit << "\n" << std::endl;
				return m_bExit.load();
				});
		}
	}
}

/// <summary>
/// 把镜头RTSP处理单元（RtspStreamHandle）产生的图片队列转移到全局的图片（Frame）队列
/// </summary>
void StreamMangement::PopFrameToList() {

	if (m_pHandle != nullptr)
	{
		while (!m_bExit.load())
		{
			//一次性把队列转移到全局队列以作后续处理
			while (true)
			{
				SHARED_LOCK(g_mtPopFrame);
				PictInfo pictInfo;
				bool bGetTopFrame = m_pHandle->PopFrame(pictInfo);
				if (bGetTopFrame)
				{
					g_list_pict.push_back(pictInfo);
					/*size_t st = g_list_pict.size();
					std:cout << "func::PopFrameToList to move pict to g_list_pict size = " << to_string(st) << std::endl;*/
				}
				else {
					break;
				}
				/*
				默认最大10毫秒(半秒)搬运一次 间隔太长影响图片识别处理
				*/
				m_PopFrameQueue.wait_for(locker, std::chrono::milliseconds(10), [this]() {
					auto exit = m_bExit.load();
					return m_bExit.load();
					});
			}
		}
	}
}

/*
处理图片识别业务 相关逻辑都在这里
*/
void StreamMangement::HandleFrameRecogBusiness() {

	if (m_pHandle != nullptr)
	{
		m_pHandle->get_stream_info(m_infoStream);
	}
	//獲取是否識別車牌的配置（第三方）
	bool permited_car_plate_enable = DEVICE_CONFIG.cfgCarPlateRecogBusiness.enable;

	/*並且在鏡頭列表中有存在定義 否則列表沒有，取消車牌第三方識別任務*/
	bool cameraIdChk = CarPlateExistCameraId(m_infoStream.nCameraId);
	if (cameraIdChk == false)
		permited_car_plate_enable = false;


	//一次性把队列转移到全局队列以作后续处理
	while (!m_bExit.load())
	{
		if (g_list_pict.size() == 0)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			//TEST
			//LOG(INFO) << "func::HandleFrameRecogBusiness() : no Frame to handle, sleep one seconds."; //如果退出状态下，则没有帧队列。
			continue;
		};

		/*
		*其实不用转移，直接 m_pHandle-》pop_frame()
		*直接从RtspStreamhandle.ccp单元获取图也可以
		*/
		const PictInfo pictInfo = g_list_pict.front();
		g_list_pict.pop_front(); //成功获取后删除第一条

		Stream::StreamInfo infoStream = m_infoStream;
		/*size_t st = g_list_pict.size();
		std:cout << "func::HandleFrameRecogBusiness to after pop_front size = " << to_string(st) << "\n" << pictInfo.path_filename;
		获取图片过来后处理图片的具体业务
		Business Handle

		TEST*/
		if (permited_car_plate_enable)
		{
			m_thPollReconn.Commit([infoStream, pictInfo]()
				{
					CameraMpeg cameraMpeg;
					bool ret = cameraMpeg.detect_and_handle(infoStream, pictInfo);
				});
		}

		//RECOGNITION TASK START------------------------------------------------------------------------------------------ 
		if (CAR_PLATE_RECOGNITION_ENABLE) //CAR_PLATE_RECOGNITION_ENABLE
		{
			/*處理車牌識別警報，此業務是來源雲端的任務配置 非本地device.json的配置*/
			//std::cout << "\nCameraId = " << m_infoStream.nCameraId << " 處理車牌識別警報，此業務是來源雲端的任務配置 非本地device.json的第三方配置\n";


		}

		//RECOGNITION TASK END--------------------------------------------------------------------------------------------


		SHARED_LOCK(g_mtPopFrame);
		m_PopFrameQueue.wait_for(locker, std::chrono::milliseconds(100), [this]() {
			auto exit = m_bExit.load();
			//TEST
			//std::cout << "\nfunc::HandleFrameRecogBusiness exit =" << exit << "\n" << std::endl;
			return m_bExit.load();
			});
	}
}

/// <summary>
/// 從緩存中獲取任務列表
/// </summary>
/// <param name="cameraId"></param>
/// <param name="taskInfoList"></param>
/// <returns></returns>
bool StreamMangement::GetTaskInfoList(int& cameraId, Service::TaskInfoList& taskInfoList) {

	if (m_TaskInfoListRet.TaskInfoListExpiredTimeOut != 0) //緩存1小時,否則通過API/本地設備獲取
	{
		int64_t now = Time::GetMilliTimestamp();
		if (m_TaskInfoListRet.TaskInfoListExpiredTimeOut > now && m_TaskInfoListRet.TaskInfoListTemp.size() > 0)
		{
			//TEST
			//LOG(INFO) << "func::StreamMangement.GetTaskInfoList " << m_TaskInfoListRet.TaskInfoListExpiredTimeOut << std::endl;
			taskInfoList = m_TaskInfoListRet.TaskInfoListTemp;
			return true;
		}
	}
	else {
		CameraMpeg cameraMpeg;

		bool succ = cameraMpeg.get_task_list_by_camera_id(cameraId, taskInfoList);
		if (succ)
		{
			m_TaskInfoListRet.TaskInfoListTemp = taskInfoList;
			m_TaskInfoListRet.TaskInfoListExpiredTimeOut = Time::GetMilliTimestamp() + static_cast<int64_t>(60 * 60 * 1000);
			return true;
		}
		else {
			//LOG(INFO) << "func::StreamMangement.GetTaskInfoList FAIL!!!! TaskInfoListExpiredTimeOut = " << m_TaskInfoListRet.TaskInfoListExpiredTimeOut << std::endl;
			std::cout << "func::StreamMangement.GetTaskInfoList FAIL!!!! TaskInfoListExpiredTimeOut = " << m_TaskInfoListRet.TaskInfoListExpiredTimeOut << std::endl;
			return false;
		}
	}
}

/*通過類型獲得識別任務詳情*/
bool StreamMangement::GetTaskInfoIfHave(int& cameraId, TaskType taskType, Service::CameraTaskInfo& cameraTaskInfo)
{
	Service::TaskInfoList taskInfoList;
	bool succ = GetTaskInfoList(cameraId, taskInfoList);
	bool ret = false;
	if (succ)
	{
		for (auto item = taskInfoList.begin(); item != taskInfoList.end(); ++item)
		{
			if (item->Type == (int)taskType)
			{
				Service::TaskInfo taskInfo;
				taskInfo.Interval = item->Interval;
				taskInfo.Threshold = item->Threshold;
				taskInfo.Type = item->Type;
				taskInfo.Name = item->Name;
				taskInfo.LibList = item->LibList;
				taskInfo.TaskId = item->TaskId;
				taskInfo.State = item->State;
				ret = true;
				break;
			}
		}
		return ret;
	}
	else {
		return ret;
	}
}

/*查詢當前鏡頭是否在Device.json的第三方車牌配置節點中的鏡頭Id列表*/
bool StreamMangement::CarPlateExistCameraId(int& cameraId)
{
	bool ret = false;

	auto permitedCamList = DEVICE_CONFIG.cfgCarPlateRecogBusiness.PermitedCamList;

	for (auto item = permitedCamList.begin(); item != permitedCamList.end(); ++item)
	{
		if (item->CameraId == cameraId)
		{
			ret = true;
			return ret;
		}
	}
	return ret;
}
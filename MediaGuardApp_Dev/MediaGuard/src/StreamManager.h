#pragma once
#include <list>
#include <memory>
#include "StreamDefine.h"
#ifdef _WIN32
#include <xkeycheck.h>
#endif
#include "./interface/CameraMpeg.h"
using namespace Stream;

class StreamHandle;
class StreamMangement
{
	typedef std::shared_ptr<StreamHandle> StreamHandlePtr;

public:
	StreamInfo m_infoStream;
	std::list<PictInfo> g_list_pict;   //记录所有camera获取图片帧以作批处理

	/*
	g_mtPopFrame m_PopFrameQueue 用于并发处理，从单元转移图片到 其他识别业务获取全局队列图片都在这个互斥量和并条件变量处理
	g_list_pict g_mtPopFrame m_PopFrameQueue  这三个是逻辑上一起使用
	*/
	std::mutex g_mtPopFrame; // 互斥量 For 转移到全局图片（Frame）队列
	std::condition_variable m_PopFrameQueue; //提交到线程池后，私有并发变量不知能不能被访问

	std::thread m_thHandleFrameBusiness; //负责函数 HandleFrameRecogBusiness

public:
	explicit StreamMangement(Stream::StreamType nStreamType);
	~StreamMangement();
	static void list_supported_hardware();
	static void list_dshow_device();
	bool StartDecode(const Stream::StreamInfo& infoStream);
	void StopDecode();
	bool GetRtmpUrl(std::string& strRtmp);
	void PopFrameToList();

private:
	bool check_handle();
	bool check_para(const Stream::StreamInfo& infoStream);
	void m_check_connect();
	bool GetTaskInfoList(int& cameraId, Service::TaskInfoList& taskInfoList);
	bool GetTaskInfoIfHave(int& cameraId, TaskType taskType, Service::CameraTaskInfo& cameraTaskInfo);
	bool CarPlateExistCameraId(int& cameraId);
	void HandleFrameRecogBusiness();
	ThreadPool m_thPollReconn;
private:
	StreamHandlePtr m_pHandle;

	std::thread m_thCheckConnect;
	// std::atomic<bool> m_bExit = false;
	std::atomic<bool> m_bExit;
	std::mutex m_mtCheck;
	std::condition_variable m_cvCheck;

	std::thread m_thPictListToGlobal; //转移到全局的队列线程

	std::thread m_thChkConnect;

	struct Service::TaskInfoListReturn m_TaskInfoListRet;

	/*識別業務開關功能*/
	bool CAR_PLATE_RECOGNITION_ENABLE = false;  //系統車牌識別
	bool FACE_RECOGNITION_ENABLE = false;		//人臉識別
	bool WORK_CLOTHES_RECOGNITION_ENABLE = false;  //工衣穿戴識別
	bool WEARING_HELMET_RECOGNITION_ENABLE = false; //頭盔識別 
	bool SOMEONE_BROKE_IN_ENABLE = false; //有人闖入

	Service::CameraTaskInfo  cameraTaskInfo_for_CAR_PLATE_RECOGNITION;
};


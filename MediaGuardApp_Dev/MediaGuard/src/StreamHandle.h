#pragma once
#include "StreamDefine.h"

using namespace Stream;

static const std::string kVideoDir = "video";
static const std::string kPictureDir = "picture";
static const std::string kAudioDir = "audio";
static const std::string kHlsDir = "hls";

class StreamHandle
{
public:
	static std::list<PictInfo> g_listFrame; //全局图片信息队列
	const std::string kVidoeFormat = ".mp4"; //test mp4 flv
	const std::string kVidoeFormatFlv = ".flv"; //test mp4 flv
	const std::string kPictureFormat = ".jpg";
	const std::string kAudioFormat = ".aac";
	int kInvalidStreamIndex = -1;
public:
	StreamHandle() { };
	~StreamHandle() {};
	virtual bool StartDecode(const Stream::StreamInfo& infoStream) = 0;
	virtual void StopDecode() = 0;
	virtual bool PopFrame(PictInfo& pictInfo) = 0; //获取图片
	virtual void GetRtmpUrl(std::string& strRtmp) = 0;
	virtual CameraConnectingStatus get_camera_connecting_status() = 0;
	virtual void get_stream_info(StreamInfo& streamInfo) = 0; //返回 StreamInfo 
};

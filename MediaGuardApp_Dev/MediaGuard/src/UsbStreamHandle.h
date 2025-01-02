/**************************************************************
* @brief:       USB stream handle
*
***************************************************************/
#pragma once
#include "Common.h"
#include "StreamHandle.h"
#include "StreamDefine.h"



class UsbStreamHandle : public StreamHandle
{
private:
	static int read_interrupt_cb(void* pContext);

public:
	UsbStreamHandle();
	~UsbStreamHandle();
	virtual bool StartDecode(const Stream::StreamInfo& infoStream);
	virtual void StopDecode();
	virtual void GetRtmpUrl(std::string& strRtmp);
	virtual CameraConnectingStatus get_camera_connecting_status(); //获取镜头的链接状态
	virtual bool PopFrame(PictInfo& pictInfo);
	virtual void get_stream_info(StreamInfo& streamInfo);
private:
	bool start_decode(AVInputFormat* pInputFormat = nullptr);
	// input
	bool open_input_stream(AVInputFormat* pInputFormat = nullptr);
	bool open_codec_context(int& nStreamIndex,
		AVCodecContext** pCodecCtx,
		AVFormatContext* pFmtCtx,
		enum AVMediaType nMediaType);
	void close_input_stream();
	// output
	bool open_output_stream(AVFormatContext*& pFormatCtx, bool bRtmp = false);
	void init_output_packet();
	void close_output_stream();
	void release_output_format_context(AVFormatContext*& pFmtContext);
	// decode
	void do_decode();
	bool decode_video_packet(AVPacket* packet, int64_t& nLoop);
	void save_stream(AVFormatContext* pFormatCtx, const AVPacket& packet);
	std::string get_filename(int nType);
	std::string get_error_msg(int nErrorCode);

private:
	bool m_bExit;
	bool m_bFirstRun;
	std::string m_strToday;
	StreamInfo m_infoStream;
	// input
	AVFormatContext* m_pInputAVFormatCtx;
	AVCodecContext* m_pVideoDecoderCtx;
	// output
	AVFormatContext* m_pOutputFileAVFormatCtx;
	AVFormatContext* m_pOutputStreamAVFormatCtx;
	SwsContext* m_pSwsCtx;
	AVFrame* m_pOutFrame;
	AVPacket m_pktOut;
	unsigned char* m_pSrcData[AV_NUM_DATA_POINTERS];
	unsigned char* m_pDstData[AV_NUM_DATA_POINTERS];
	int m_arrSrcLinesize[AV_NUM_DATA_POINTERS];
	int m_arrDstLinesize[AV_NUM_DATA_POINTERS];
	bool m_bInputInited;
	bool m_bOutputInited;

	// decode
	std::thread m_thDecode;
	std::mutex m_mtPacket;
	std::condition_variable m_cvFrame;
	std::list<AVPacket> m_listPacket;

	// cache the frame
	int m_nFrame;
	std::mutex m_mtFrame;
	std::list<cv::Mat> m_listFrame;
	ThreadPool m_poolSavePic;
	CameraConnectingStatus cameraConnectingStatus = CameraConnectingStatus::InStopped; //Camera链接状态
};


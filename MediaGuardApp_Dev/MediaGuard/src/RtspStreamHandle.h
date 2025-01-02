#pragma once
#include "./Common.h"
#include "Common/CvMatToBase64.h"


//#pragma warning(push, 0)  //本來為了解決Enum Class的警告提示的，但不起作用
//#include <../ffmpeg/include/libavutil/avutil.h>
//#pragma warning(pop)

using namespace Stream;

class RtspStreamHandle : public StreamHandle, public std::enable_shared_from_this<RtspStreamHandle>
{
private:
	static int read_interrupt_cb(void* pContext);
	static enum AVPixelFormat get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts);
	int hw_decoder_init(AVCodecContext* ctx, const enum AVHWDeviceType type);

public:
	RtspStreamHandle();
	~RtspStreamHandle();
	virtual bool StartDecode(const StreamInfo& infoStream);
	virtual void StopDecode();
	virtual void GetRtmpUrl(std::string& strRtmp);
	virtual CameraConnectingStatus get_camera_connecting_status(); //获取镜头的链接状态 
	virtual bool PopFrame(PictInfo& pictInfo);
	virtual void get_stream_info(StreamInfo& streamInfo); //在运行中返回当前的 StreamInfo (部分与传入的参数已经赋值 如width height) 

	void addnew_record_file_info(StreamInfo& streamInfo, int64_t& startTimestamp, int64_t& endTimestamp);

	void GetVideoSize(int& width, int& height);
	void PushFrame(const cv::Mat& frame);


private:
	bool start_decode();

	/*
		首先做的：打开Stream以获取上下文法
	*/
	bool open_input_stream();

	/*
	* 打開解碼器
	 */
	bool open_codec_context(int& nStreamIndex, AVCodecContext** pDecoderCtx, AVFormatContext* pFmtCtx, enum AVMediaType avMediaType);

	/*关闭STREAM*/
	void close_input_stream();

	/*
	打开输出STREAM，如果rtmp输出则同时打开
	*/
	bool open_output_stream(AVFormatContext*& pFormatCtx, bool bRtmp = false);

	/*从打开的输出STREAM复制新的STREAM 并设定新的输出是HLS协议STREAM输出*/
	bool open_output_hls_stream(AVFormatContext*& pFormatCtx, int nStreamDecodeType);
	/*
		写Packet到hls的上下文法
	*/
	void write_output_hls_stream(AVFormatContext* pFormatCtx, const AVPacket& packet);
	/*
	用于hls解码调整时基
	*/
	void av_packet_rescale_hls_ts(AVPacket* pkt, AVRational src_tb, AVRational dst_tb);
 
	/*
	关闭输出STREAM
	*/
	void close_output_stream();

	/*解码*/
	void do_decode();
	/*
	重新调整帧的播放时间基/PTS，解码时间基/DTS,帧的时长DURATION
	*/
	void av_packet_rescale_ts(AVPacket* pkt, AVRational src_tb, AVRational dst_tb, AVRational r_frame_rate);

	/*
	把图片Push到到队列列表以作后面的队列处理
	*/
	void push_packet(const AVPacket& packet);
	bool decode_video_packet(AVPacket* packet);
	bool decode_audio_packet(const AVPacket& packet);
	void save_stream(AVFormatContext* pFormatCtx, const AVPacket& packet);
	void release_output_format_context(AVFormatContext*& pFmtContext);
	std::string get_filename(Stream::FType ftype);
	std::string get_error_msg(int nErrorCode);
	cv::Mat avframe_to_mat(const AVFrame* frame);

private:
	int64_t start_time = 0; //记录录像开始时间
	int64_t recfirstPTS = 0;// 0 表示第一幀，1表示已經過了第一幀  

	std::atomic<bool> m_bExit/* = false*/;//bool m_bExit;
	std::mutex m_mtLock;
	std::condition_variable m_cvCond;
	std::thread m_thHlsClear;
	std::thread m_thAddNewRecordInfo;
	bool m_bFirstRun;
	std::string m_strToday;
	StreamInfo m_infoStream;
	AVFormatContext* m_pInputAVFormatCtx;
	AVCodecContext* m_pVideoDecoderCtx;
	AVCodecContext* m_pAudioDecoderCtx;
	AVFormatContext* m_pOutputFileAVFormatCtx;
	AVFormatContext* m_pOutputStreamAVFormatCtx;
	AVFormatContext* m_pHlsOutStreamAVFormatCtx;
	const AVPixelFormat* m_AVPixelFormat; //新增解码像素格式，行336引用，解决FFmpeg接口警告提示：[swscaler @ ...] deprecated pixel format used, make sure you did set range correctly
	AVBufferRef* m_pHDCtx;
	SwsContext* m_pSwsCtx;
	bool m_bInputInited;
	bool m_bOutputInited;
	bool m_bHlsOutInited;

	/*
	处理解码的线程互斥锁
	*/
	std::thread m_thDecode;

	/*
	处理Packet的互斥锁
	*/
	std::mutex m_mtPacket;
	/*
	用于处理Frame的并发线程
	*/
	std::condition_variable m_cvFrame;

	std::list<AVPacket> m_listPacket;

	// cache the frame 
	int64_t m_nFrame;
	std::mutex m_mtFrame;
	std::list<PictInfo> m_listFrame; //rtsp stream unit 里面的图片信息队列

	/*
	图片写入硬盘的任务线程池 主要是写入操作，
	并把cv::mat格式的图片转为Base64保存到当前单元（RtspStreamRtsp）
	的单元队列List<PictInfo>以作转移到全局的单元队列做各种与图片相关的处理业务
	*/
	ThreadPool m_poolSavePic;

	/*
	当前镜头的解码单元的链接状态，提供外部获取状态
	*/
	CameraConnectingStatus cameraConnectingStatus = CameraConnectingStatus::InStopped;

	/*
	当前保存的文件名
	*/
	std::string m_path_filename;
};
typedef std::shared_ptr<RtspStreamHandle> RtspStreamHandlePtr;

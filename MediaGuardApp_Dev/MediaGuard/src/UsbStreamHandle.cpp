#pragma once
#include "UsbStreamHandle.h"
#include "StreamHandle.h"  
#include <iostream>
#include "Time.h"
#include <memory>

/*
Command:
 ffmpeg -f dshow -i video="USB 2861 Device" -f dshow -i audio="线路 (3- USB Audio Device)" -vcodec libx264 -acodec aac -strict -2 mycamera.mkv
 上面的命令行用video=指定视频设备，用audio=指定音频设备，后面的参数是定义编码器的格式和属性，输出为一个名为mycamera.mkv的文件。

  ffmpeg -list_options true -f dshow -i video="USB 2861 Device"
  这个命令行的作用是获取指定视频采集设备支持的分辨率、帧率和像素格式等属性，返回的是一个列表，结果如下：

 ref:https://blog.csdn.net/fuhanghang/article/details/123553898
 输入下面的语句即可列出电脑的设备

ffmpeg -list_devices true -f dshow -i dummy

infoStreamII.strInput = "Integrated Camera";
命令 ffplay -f dshow -i video="Integrated Camera"  经测试是OK的
ffmpeg -f dshow -i video="Integrated Camera" -s 1280x720 -t 30 -b:v 1000k -b:a 128k output.mp4 经测试OK
-t 30 表示录像30秒

*/

using namespace Stream;

// avformat_open_input/av_read_frame timeout callback
// return: 0(continue original call), other(interrupt original call)
int UsbStreamHandle::read_interrupt_cb(void* pContext)
{
	return 0;
}

UsbStreamHandle::UsbStreamHandle()
	: m_bExit(false)
	, m_pInputAVFormatCtx(nullptr)
	, m_pOutputFileAVFormatCtx(nullptr)
	, m_pOutputStreamAVFormatCtx(nullptr)
	, m_bInputInited(false)
	, m_bOutputInited(false)
	, m_bFirstRun(true)
	, m_pVideoDecoderCtx(nullptr)
	, m_nFrame(0)
	, m_pSwsCtx(nullptr)
	, cameraConnectingStatus(CameraConnectingStatus::InPlaying)
{
}


UsbStreamHandle::~UsbStreamHandle()
{
	StopDecode();
}

bool UsbStreamHandle::StartDecode(const Stream::StreamInfo& infoStream)
{
	m_infoStream = infoStream;
	AVInputFormat* pInputFormat = nullptr;
	pInputFormat = (AVInputFormat*)av_find_input_format("dshow");
	return start_decode(pInputFormat);
}

void UsbStreamHandle::StopDecode()
{
	m_bExit = true;
	m_cvFrame.notify_one();
	if (m_thDecode.joinable())
		m_thDecode.join();
	close_input_stream();
	close_output_stream();
	m_poolSavePic.Stop();
	sws_freeContext(m_pSwsCtx);
}

void UsbStreamHandle::GetRtmpUrl(std::string& strRtmp)
{

}
bool UsbStreamHandle::PopFrame(PictInfo& pictInfo)
{
	//std::lock_guard<std::mutex> lock(m_mtFrame);
	//if (m_listFrame.empty()) return false;
	//pictInfo = m_listFrame.front();
	//m_listFrame.pop_front();
	return false;
}
void UsbStreamHandle::get_stream_info(StreamInfo& streamInfo)
{
	//代写
}
bool UsbStreamHandle::start_decode(AVInputFormat* pInputFormat)
{
	if (!open_input_stream(pInputFormat))
	{
		printf("Can't open input:%s\n", m_infoStream.strInput.c_str());
		return false;
	}
	if (m_infoStream.bRtmp)
		open_output_stream(m_pOutputStreamAVFormatCtx, m_infoStream.bRtmp);
	if (m_infoStream.bSaveVideo)
		open_output_stream(m_pOutputFileAVFormatCtx);
	m_thDecode = std::thread(std::bind(&UsbStreamHandle::do_decode, this));
	return true;
}

bool UsbStreamHandle::open_input_stream(AVInputFormat* pInputFormat)
{
	char szVedioSize[20] = { '\0' };
	snprintf(szVedioSize, 20, "%dx%d", 320, 240);

	AVDictionary* pOptions = nullptr;
	av_dict_set(&pOptions, "video_size", szVedioSize, 0);
	av_dict_set(&pOptions, "pixel_format", av_get_pix_fmt_name(AV_PIX_FMT_YUYV422), 0);
	av_dict_set(&pOptions, "framerate", "25", 0);  //預設幀率25

	m_pInputAVFormatCtx = avformat_alloc_context();
	m_pInputAVFormatCtx->interrupt_callback = { read_interrupt_cb, this };
	int nCode = avformat_open_input(&m_pInputAVFormatCtx,
		m_infoStream.strInput.c_str(),
		pInputFormat,
		&pOptions);
	if (0 != nCode)
	{
		std::string strError = "Can't open input:" + m_infoStream.strInput
			+ get_error_msg(nCode);
		return false;
	}

	av_dict_free(&pOptions);
	if (open_codec_context(m_infoStream.nVideoIndex, &m_pVideoDecoderCtx, m_pInputAVFormatCtx, AVMEDIA_TYPE_VIDEO))
	{
		m_infoStream.nWidth = m_pVideoDecoderCtx->width;
		m_infoStream.nHeight = m_pVideoDecoderCtx->height;
		m_infoStream.nPixFmt = m_pVideoDecoderCtx->pix_fmt;
	}
	return true;
}

bool UsbStreamHandle::open_codec_context(int& nStreamIndex,
	AVCodecContext** pCodecCtx,
	AVFormatContext* pFmtCtx,
	enum AVMediaType nMediaType)
{
	int nCode = avformat_find_stream_info(pFmtCtx, nullptr);
	if (0 != nCode)
	{
		printf("Can't find stream info\n");
		return false;
	}

	av_dump_format(pFmtCtx, 0, m_infoStream.strInput.c_str(), 0);
	nStreamIndex = av_find_best_stream(pFmtCtx, nMediaType, -1, -1, NULL, 0);
	if (-1 == nStreamIndex)
	{
		fprintf(stderr, "Couldn't find %s stream in input\n",
			av_get_media_type_string(nMediaType));
		return false;
	}
	// encoder
	AVCodec* pEncoder = nullptr;
	pEncoder = (AVCodec*)avcodec_find_encoder(AV_CODEC_ID_H264);
	if (nullptr == pEncoder)
	{
		fprintf(stderr, "Can't find encoder, media type: %s\n", av_get_media_type_string(nMediaType));
		return false;
	}
	/* Allocate a codec context for the decoder */
	(*pCodecCtx) = avcodec_alloc_context3(pEncoder);
	if (nullptr == pCodecCtx)
	{
		fprintf(stderr, "Failed to allocate the %s codec context\n",
			av_get_media_type_string(nMediaType));
		return false;
	}
	auto pInstream = pFmtCtx->streams[nStreamIndex];
	(*pCodecCtx)->codec_id = pEncoder->id;
	(*pCodecCtx)->bit_rate = 400000;
	(*pCodecCtx)->width = pInstream->codecpar->width;
	(*pCodecCtx)->height = pInstream->codecpar->height;
	(*pCodecCtx)->time_base.num = 1;
	(*pCodecCtx)->time_base.den = 25;
	(*pCodecCtx)->framerate.num = 25;
	(*pCodecCtx)->framerate.den = 1;
	(*pCodecCtx)->gop_size = 10;
	(*pCodecCtx)->max_b_frames = 0;
	(*pCodecCtx)->pix_fmt = AV_PIX_FMT_YUV420P;
	(*pCodecCtx)->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	/* Some formats want stream headers to be separate. */
	//if (m_pOutputAVFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
	//	encodec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	AVDictionary* pOptions = NULL;
	av_dict_set(&pOptions, "preset", "superfast", 0);
	av_dict_set(&pOptions, "tune", "zerolatency", 0);
	av_dict_set(&pOptions, "profile", "main", 0);

	nCode = avcodec_open2((*pCodecCtx), pEncoder, &pOptions);
	if (nCode < 0)
	{
		fprintf(stderr, "Failed to open %s codec, error: %s\n",
			av_get_media_type_string(nMediaType),
			get_error_msg(nCode).c_str());
		return false;
	}

	av_dict_free(&pOptions);
	return true;
}

void UsbStreamHandle::close_input_stream()
{
	if (m_pInputAVFormatCtx)
		avformat_close_input(&m_pInputAVFormatCtx);
	if (m_pVideoDecoderCtx)
		avcodec_free_context(&m_pVideoDecoderCtx);
}

bool UsbStreamHandle::open_output_stream(AVFormatContext*& pFormatCtx, bool bRtmp)
{
	std::string strFormatName = bRtmp ? "flv" : "mp4";
	std::string strOutputPath = bRtmp ? m_infoStream.strOutput : get_filename(kFileTypeVideo);
	int nCode = avformat_alloc_output_context2(&pFormatCtx, NULL, strFormatName.c_str(), strOutputPath.c_str());
	if (nullptr == pFormatCtx)
	{
		printf("Can't alloc output context \n");
		return false;
	}
	AVStream* pOutputStream = avformat_new_stream(pFormatCtx, m_pVideoDecoderCtx->codec);
	if (nullptr == pOutputStream)
	{
		printf("Can't allocate out stream\n");
		return false;
	}
	nCode = avcodec_parameters_from_context(pOutputStream->codecpar, m_pVideoDecoderCtx);
	if (0 != nCode)
	{
		printf("Can't copy parameters for context, code: %d, msg: %s\n",
			nCode, get_error_msg(nCode).c_str());
		return false;
	}

	//av_stream_set_r_frame_rate(output_stream, av_make_q(1, fps));
	nCode = avio_open(&pFormatCtx->pb, strOutputPath.c_str(), AVIO_FLAG_WRITE);
	if (nCode != 0)
	{
		printf("Can't open output io, file: %s, code: %d, msg: %s\n",
			strOutputPath.c_str(), nCode, get_error_msg(nCode).c_str());
		return false;
	}

	av_dump_format(pFormatCtx, 0, strOutputPath.c_str(), 1);

	nCode = avformat_write_header(pFormatCtx, NULL);
	if (nCode < 0)
	{
		printf("Can't write outputstream header, URL:%s, code: %d, msg: %s\n",
			strOutputPath.c_str(), nCode, get_error_msg(nCode).c_str());
		return false;
	}
	init_output_packet();
	return true;
}

void UsbStreamHandle::init_output_packet()
{
	if (m_pSwsCtx != nullptr) return;
	int nWidth = 320;
	int nHeight = 240;
	auto pInputStream = m_pInputAVFormatCtx->streams[m_infoStream.nVideoIndex];
	m_pSwsCtx = sws_getContext(pInputStream->codecpar->width,
		pInputStream->codecpar->height,
		(AVPixelFormat)pInputStream->codecpar->format,
		320,
		240,
		AV_PIX_FMT_YUV420P,
		SWS_BILINEAR, NULL, NULL, NULL);
	int src_bufsize = av_image_alloc(m_pSrcData,
		m_arrSrcLinesize,
		pInputStream->codecpar->width,
		pInputStream->codecpar->height,
		(AVPixelFormat)pInputStream->codecpar->format,
		16);
	int nDstSize = av_image_alloc(m_pDstData, m_arrDstLinesize, nWidth, nHeight, AV_PIX_FMT_YUV420P, 1);

	m_pOutFrame = av_frame_alloc();
	unsigned char* picture_buf = (uint8_t*)av_malloc(nDstSize);
	av_image_fill_arrays(m_pOutFrame->data,
		m_pOutFrame->linesize,
		picture_buf,
		m_pVideoDecoderCtx->pix_fmt,
		m_pVideoDecoderCtx->width,
		m_pVideoDecoderCtx->height,
		1);
	m_pOutFrame->format = m_pVideoDecoderCtx->pix_fmt;
	m_pOutFrame->width = m_pVideoDecoderCtx->width;
	m_pOutFrame->height = m_pVideoDecoderCtx->height;

	av_new_packet(&m_pktOut, nDstSize);
}

void UsbStreamHandle::close_output_stream()
{
	release_output_format_context(m_pOutputFileAVFormatCtx);
	release_output_format_context(m_pOutputStreamAVFormatCtx);
	m_bOutputInited = false;
}

void UsbStreamHandle::release_output_format_context(AVFormatContext*& pFmtContext)
{
	if (pFmtContext)
	{
		av_write_trailer(pFmtContext);
		if (!(pFmtContext->oformat->flags & AVFMT_NOFILE))
		{
			if (pFmtContext->pb)
			{
				avio_close(pFmtContext->pb);
			}
		}
		avformat_free_context(pFmtContext);
		pFmtContext = nullptr;
	}
}

void UsbStreamHandle::do_decode()
{
	int64_t nLoop = 0;
	int got_picture = -1;
	int delayedFrame = 0;
	AVPacket packet;
	av_init_packet(&packet);
	int64_t nLastSaveVideo = Time::GetMilliTimestamp();
	int64_t nVideoTime = m_infoStream.nVideoTime * 1000;
	while (!m_bExit)
	{
		int nCode = av_read_frame(m_pInputAVFormatCtx, &packet);
		if (nCode < 0)
		{
			cameraConnectingStatus = CameraConnectingStatus::InRequestStopped;
			break;
		}
		if (packet.stream_index == m_infoStream.nVideoIndex)
		{
			if (decode_video_packet(&packet, nLoop))
			{
				if (m_infoStream.bSaveVideo)
				{
					save_stream(m_pOutputFileAVFormatCtx, packet);
					auto nMillSecond = Time::GetMilliTimestamp();
					if (nMillSecond - nLastSaveVideo >= nVideoTime)
					{
						nLastSaveVideo = nMillSecond;
						release_output_format_context(m_pOutputFileAVFormatCtx);
						open_output_stream(m_pOutputFileAVFormatCtx);
					}
				}
				if (m_infoStream.bRtmp)
					save_stream(m_pOutputStreamAVFormatCtx, packet);
			}
		}
		av_packet_unref(&packet);
	}
}

bool UsbStreamHandle::decode_video_packet(AVPacket* packet, int64_t& nLoop)
{
	bool bSucc = true;
	auto nYSize = m_pVideoDecoderCtx->width * m_pVideoDecoderCtx->height;
	memcpy(m_pSrcData[0], packet->data, packet->size);
	sws_scale(m_pSwsCtx,
		m_pSrcData,
		m_arrSrcLinesize,
		0,
		m_pInputAVFormatCtx->streams[m_infoStream.nVideoIndex]->codecpar->height,
		m_pDstData,
		m_arrDstLinesize);
	m_pOutFrame->data[0] = m_pDstData[0];
	m_pOutFrame->data[1] = m_pDstData[0] + nYSize;
	m_pOutFrame->data[2] = m_pDstData[0] + nYSize * 5 / 4;
	m_pOutFrame->pts = nLoop++;
	int nCode = avcodec_send_frame(m_pVideoDecoderCtx, m_pOutFrame);
	if (nCode < 0)
		bSucc = false;
	nCode = avcodec_receive_packet(m_pVideoDecoderCtx, &m_pktOut);
	return bSucc;
}

void  UsbStreamHandle::save_stream(AVFormatContext* pFormatCtx, const AVPacket& packet)
{
	AVPacket pktFrame /*= packet*/;
	av_init_packet(&pktFrame);
	av_packet_ref(&pktFrame, &m_pktOut);

	AVStream* pOutStream = pFormatCtx->streams[packet.stream_index];
	pktFrame.stream_index = packet.stream_index;
	AVRational itime = m_pInputAVFormatCtx->streams[packet.stream_index]->time_base;
	AVRational otime = pFormatCtx->streams[packet.stream_index]->time_base;

	pktFrame.pts = av_rescale_q_rnd(packet.pts, itime, otime, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pktFrame.dts = av_rescale_q_rnd(packet.dts, itime, otime, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pktFrame.duration = av_rescale_q_rnd(packet.duration, itime, otime, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
	pktFrame.pos = -1;

	int nCode = av_interleaved_write_frame(pFormatCtx, &pktFrame);
	if (nCode < 0)
		printf("Error: %d while writing frame, %s\n", nCode, get_error_msg(nCode).c_str());
}

std::string UsbStreamHandle::get_filename(int nType)
{
	auto generate_filename = [=](int nType)
	{
		std::string strToday = Time::GetCurrentDate();
		if (strToday != m_strToday)
			m_strToday = strToday;
		std::string strTimestamp = std::to_string(Time::GetTimestamp());
		std::string strFilename;
		switch (nType)
		{
		case kFileTypeVideo:
			strFilename = kVideoDir + "/" + m_strToday + "/" + strTimestamp + kVidoeFormat;
			break;
		case kFileTypeRtmp:
			strFilename = "";
			break;
		default:
			strFilename = kPictureDir + "/" + m_strToday + "/" + strTimestamp + kVidoeFormat;
			break;
		}
		return strFilename;
	};
	return generate_filename(nType);
}


std::string UsbStreamHandle::get_error_msg(int nErrorCode)
{
	char szMsg[AV_ERROR_MAX_STRING_SIZE] = { 0 };
	av_make_error_string(szMsg, AV_ERROR_MAX_STRING_SIZE, nErrorCode);
	std::stringstream ss;
	{
		ss << "Code[" << nErrorCode << "]:" << szMsg;
	}
	return ss.str();
}

CameraConnectingStatus UsbStreamHandle::get_camera_connecting_status()
{
	return cameraConnectingStatus;
}
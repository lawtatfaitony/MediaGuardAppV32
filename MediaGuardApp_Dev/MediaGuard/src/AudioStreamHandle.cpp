#include "AudioStreamHandle.h"
#include "StreamHandle.h"  
#include <memory>

using namespace Stream;

/*
   参考： 获取音视频设备
   https://blog.csdn.net/qq_24671901/article/details/127140449
*/
// avformat_open_input/av_read_frame timeout callback
// return: 0(continue original call), other(interrupt original call)
int AudioStreamHandle::read_interrupt_cb(void* pContext)
{
	return 0;
}

AudioStreamHandle::AudioStreamHandle()
	: m_bExit(false)
	, m_pInputAVFormatCtx(nullptr)
	, m_pOutputFileAVFormatCtx(nullptr)
	, m_pOutputStreamAVFormatCtx(nullptr)
	, m_bInputInited(false)
	, m_bOutputInited(false)
	, m_bFirstRun(true)
	, m_pEncoderCtx(nullptr)
	, m_pAudioDecoderCtx(nullptr)
	, m_nFrame(0)
	, m_pSwsCtx(nullptr)
	, m_pFilterGraph(nullptr)
	, m_pBufferSinkCtx(nullptr)
	, m_pBufferSrcCtx(nullptr)
	, cameraConnectingStatus(CameraConnectingStatus::InPlaying)
{
}

AudioStreamHandle::~AudioStreamHandle()
{
	StopDecode();
}

bool AudioStreamHandle::StartDecode(const Stream::StreamInfo& infoStream)
{
	m_infoStream = infoStream;
	AVInputFormat* pInputFormat = nullptr;
	pInputFormat = (AVInputFormat*)av_find_input_format("dshow");
	return start_decode(pInputFormat);
}

void AudioStreamHandle::StopDecode()
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

void AudioStreamHandle::GetRtmpUrl(std::string& strRtmp)
{

}

bool AudioStreamHandle::PopFrame(PictInfo& pictInfo)
{
	//std::lock_guard<std::mutex> lock(m_mtFrame);
	//if (m_listFrame.empty()) return false;
	//pictInfo = m_listFrame.front();
	//m_listFrame.pop_front();
	return false;
}

void AudioStreamHandle::get_stream_info(StreamInfo& streamInfo)
{
	//待写。。。
}

bool AudioStreamHandle::start_decode(AVInputFormat* pInputFormat)
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
	m_thDecode = std::thread(std::bind(&AudioStreamHandle::do_decode, this));
	return true;
}

bool AudioStreamHandle::open_input_stream(AVInputFormat* pInputFormat)
{

	AVDictionary* pOptions = nullptr;
	av_dict_set_int(&pOptions, "audio_buffer_size", 20, 0);
	m_pInputAVFormatCtx = avformat_alloc_context();
	m_pInputAVFormatCtx->interrupt_callback = { read_interrupt_cb, this };
	int nCode = avformat_open_input(&m_pInputAVFormatCtx,
		m_infoStream.strInput.c_str(),
		pInputFormat,
		nullptr);
	if (0 != nCode)
	{
		std::string strError = "Can't open input:" + m_infoStream.strInput
			+ get_error_msg(nCode);
		return false;
	}
	av_dict_free(&pOptions);
	if (open_codec_context(m_infoStream.nVideoIndex, &m_pAudioDecoderCtx, m_pInputAVFormatCtx, AVMEDIA_TYPE_VIDEO))
	{
		m_infoStream.nWidth = m_pAudioDecoderCtx->width;
		m_infoStream.nHeight = m_pAudioDecoderCtx->height;
		m_infoStream.nPixFmt = m_pAudioDecoderCtx->pix_fmt;
	}
	return true;
}

bool AudioStreamHandle::open_codec_context(int& nStreamIndex,
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
	//打印关于输入或输出格式的详细信息，例如持续时间，比特率，流，容器，程序，元数据，边数据，编解码器和时基。
	av_dump_format(pFmtCtx, 0, m_infoStream.strInput.c_str(), 0);
	nStreamIndex = av_find_best_stream(pFmtCtx, nMediaType, -1, -1, NULL, 0);
	if (-1 == nStreamIndex)
	{
		fprintf(stderr, "Couldn't find %s stream in input\n",
			av_get_media_type_string(nMediaType));
		return false;
	}
	// decoder
	AVCodec* pDecoder = nullptr;
	//FFmpeg提供两种方式查找解码器，通过codecId查找avcodec_find_decoder()与通过名字查找avcodec_find_decoder_by_name()。
	//同样地，也提供两种方式查找编码器，通过codecId查找avcodec_find_encoder()与通过名字查找avcodec_find_encoder_by_name()。源码位于libavcodec/allcodecs.c中。
	pDecoder = (AVCodec*)avcodec_find_decoder(pFmtCtx->streams[nStreamIndex]->codecpar->codec_id);
	if (nullptr == pDecoder)
	{
		fprintf(stderr, "Can't find decoder, media type: %s\n", av_get_media_type_string(nMediaType));
		return false;
	}
	/* Allocate a codec context for the decoder */
	//申请AVCodecContext空间。需要传递一个编码器，也可以不传，但不会包含编码器。
	(*pCodecCtx) = avcodec_alloc_context3(pDecoder);  //**AVCodecContext
	if (nullptr == pCodecCtx)
	{
		fprintf(stderr, "Failed to allocate the %s codec context\n",
			av_get_media_type_string(nMediaType));
		return false;
	}
	//FFmpeg提供了函数avcodec_parameters_to_context将音频流信息拷贝到新的AVCodecContext结构体中
	avcodec_parameters_to_context(*pCodecCtx, pFmtCtx->streams[nStreamIndex]->codecpar);

	//该函数用于 初始化 一个视音频编解码器的AVCodecContext。
	//參數說明 avctx：需要初始化的AVCodecContext。  codec：输入的AVCodec    options：一些选项。例如使用libx264编码的时候，“preset”，“tune”等都可以通过该参数设置。
	nCode = avcodec_open2(*pCodecCtx, pDecoder, NULL);
	if (nCode < 0)
	{
		printf("Failed to open decoder, error[%d]:%s", nCode, get_error_msg(nCode).c_str());
		return false;
	}
	//打印关于输入或输出格式的详细信息，例如持续时间，比特率，流，容器，程序，元数据，边数据，编解码器和时基。
	av_dump_format(pFmtCtx, 0, m_infoStream.strInput.c_str(), 0); //？？？到底這句還要不要，上文已經存在依據

	auto pInputStream = pFmtCtx->streams[nStreamIndex];
	printf("fmt=%s, bit_rate=%lld, sample_rate=%d\n", av_get_sample_fmt_name((AVSampleFormat)pInputStream->codecpar->format)
		, pInputStream->codecpar->bit_rate
		, pInputStream->codecpar->sample_rate);

	return true;
}

bool AudioStreamHandle::init_encoder(int nStreamIndex,
	AVCodecContext** pCodecCtx,
	AVFormatContext* pFmtCtx,
	enum AVMediaType nMediaType)
{
	int nCode = 0;
	// encoder
	AVCodec* pEncoder = (AVCodec*)avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (nullptr == pEncoder)
	{
		printf("Failed to open encoder, error[%d]:%s", nCode, get_error_msg(nCode).c_str());
		return false;
	}
	(*pCodecCtx) = avcodec_alloc_context3(pEncoder);
	if ((*pCodecCtx) == nullptr)
	{
		fprintf(stderr, "Failed to allocate the %s codec context, error[%d]: %s\n",
			av_get_media_type_string(nMediaType), nCode, get_error_msg(nCode).c_str());
		return false;
	}

	(*pCodecCtx)->codec = pEncoder;
	(*pCodecCtx)->sample_rate = 48000;
	(*pCodecCtx)->channel_layout = 3;
	(*pCodecCtx)->channels = 2;
	(*pCodecCtx)->sample_fmt = AVSampleFormat::AV_SAMPLE_FMT_FLTP;
	(*pCodecCtx)->codec_tag = 0;
	(*pCodecCtx)->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

	nCode = avcodec_open2(*pCodecCtx, pEncoder, nullptr);
	if (nCode < 0)
	{
		printf("Failed to open encoder context, error[%d]:%s", nCode, get_error_msg(nCode).c_str());
		return false;
	}
	return true;
}

void AudioStreamHandle::close_input_stream()
{
	if (m_pInputAVFormatCtx)
		avformat_close_input(&m_pInputAVFormatCtx);
	if (m_pAudioDecoderCtx)
		avcodec_free_context(&m_pAudioDecoderCtx);
	if (m_pEncoderCtx)
		avcodec_free_context(&m_pEncoderCtx);
}

bool AudioStreamHandle::open_output_stream(AVFormatContext*& pOutputCtx, bool bRtmp)
{
	std::string strFormatName = bRtmp ? "flv" : "aac";
	std::string strOutputPath = bRtmp ? m_infoStream.strOutput : get_filename(kFileTypeAudio);
	int nCode = avformat_alloc_output_context2(&pOutputCtx, 0, strFormatName.c_str(), strOutputPath.c_str());
	if (nullptr == pOutputCtx)
	{
		printf("Can't alloc output context \n");
		return false;
	}

	AVStream* pOutputStream = avformat_new_stream(pOutputCtx, m_pEncoderCtx->codec);
	if (nullptr == pOutputStream)
	{
		return false;
	}

	nCode = avcodec_parameters_from_context(pOutputStream->codecpar, m_pEncoderCtx);
	if (0 != nCode)
	{
		return false;
	}

	nCode = avio_open2(&pOutputCtx->pb, strOutputPath.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr);
	if (nCode < 0)
	{

		return false;
	}

	av_dump_format(pOutputCtx, 0, strOutputPath.c_str(), 1);
	// Resampling initialization
	init_audio_sample(m_pInputAVFormatCtx, m_pFilterGraph, m_pBufferSinkCtx, m_pBufferSrcCtx);

	nCode = avformat_write_header(pOutputCtx, NULL);
	if (nCode < 0)
	{
		return false;
	}
	return true;
}

int AudioStreamHandle::init_audio_sample(AVFormatContext* pFormatCtx,
	AVFilterGraph* pFilterGraph,
	AVFilterContext*& pBufferSinkCtx,
	AVFilterContext*& pBufferSrcCtx)
{
	char args[512] = { '\0' };
	int ret;
	const AVFilter* abuffersrc = avfilter_get_by_name("abuffer");
	const AVFilter* abuffersink = avfilter_get_by_name("abuffersink");
	AVFilterInOut* outputs = avfilter_inout_alloc();
	AVFilterInOut* inputs = avfilter_inout_alloc();

	auto audioDecoderContext = pFormatCtx->streams[0]->codecpar;
	if (!audioDecoderContext->channel_layout)
		audioDecoderContext->channel_layout = av_get_default_channel_layout(audioDecoderContext->channels);

	static const enum AVSampleFormat out_sample_fmts[] = { AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_NONE };
	static const int64_t out_channel_layouts[] = { audioDecoderContext->channel_layout, -1 };
	static const int out_sample_rates[] = { audioDecoderContext->sample_rate , -1 };

	AVRational time_base = pFormatCtx->streams[0]->time_base;
	pFilterGraph = avfilter_graph_alloc();
	pFilterGraph->nb_threads = 1;

#ifdef _WIN32
	sprintf_s(args, sizeof(args),
		"time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%I64x",
		time_base.num, time_base.den, audioDecoderContext->sample_rate,
		av_get_sample_fmt_name((AVSampleFormat)audioDecoderContext->format), audioDecoderContext->channel_layout);
#elif __linux__
	// 请自行补充
#endif

	printf("[init_audio_sample]args=%s\n", args);

	ret = avfilter_graph_create_filter(&pBufferSrcCtx, abuffersrc, "in",
		args, NULL, pFilterGraph);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer source\n");
		return ret;
	}

	/* buffer audio sink: to terminate the filter chain. */
	ret = avfilter_graph_create_filter(&pBufferSinkCtx, abuffersink, "out",
		NULL, NULL, pFilterGraph);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot create audio buffer sink\n");
		return ret;
	}

	ret = av_opt_set_int_list(pBufferSinkCtx, "sample_fmts", out_sample_fmts, -1,
		AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot set output sample format\n");
		return ret;
	}

	ret = av_opt_set_int_list(pBufferSinkCtx, "channel_layouts", out_channel_layouts, -1,
		AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot set output channel layout\n");
		return ret;
	}

	ret = av_opt_set_int_list(pBufferSinkCtx, "sample_rates", out_sample_rates, -1,
		AV_OPT_SEARCH_CHILDREN);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot set output sample rate\n");
		return ret;
	}

	/* Endpoints for the filter graph. */
	outputs->name = av_strdup("in");
	outputs->filter_ctx = pBufferSrcCtx;;
	outputs->pad_idx = 0;
	outputs->next = NULL;

	inputs->name = av_strdup("out");
	inputs->filter_ctx = pBufferSinkCtx;
	inputs->pad_idx = 0;
	inputs->next = NULL;

	if ((ret = avfilter_graph_parse_ptr(pFilterGraph, "anull",
		&inputs, &outputs, nullptr)) < 0)
		return ret;

	if ((ret = avfilter_graph_config(pFilterGraph, NULL)) < 0)
		return ret;

	av_buffersink_set_frame_size(pBufferSinkCtx, 1024);
	return 0;
}

void AudioStreamHandle::close_output_stream()
{
	release_output_format_context(m_pOutputFileAVFormatCtx);
	release_output_format_context(m_pOutputStreamAVFormatCtx);
	m_bOutputInited = false;
}

void AudioStreamHandle::release_output_format_context(AVFormatContext*& pFmtContext)
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
	if (m_pFilterGraph)
		avfilter_graph_free(&m_pFilterGraph);

}

void AudioStreamHandle::do_decode()
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
		if (nCode < 0) {
			cameraConnectingStatus = CameraConnectingStatus::InRequestStopped;
			break;
		}

		if (packet.stream_index == m_infoStream.nAudioIndex)
		{
			if (decode_audio_packet(packet, nLoop))
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

bool AudioStreamHandle::decode_audio_packet(const AVPacket& packet, int64_t& nLoop)
{
	int nCode = avcodec_send_packet(m_pAudioDecoderCtx, &packet);
	if (nCode != 0) return false;
	bool bSucc = true;
	AVFrame* pFrame = av_frame_alloc();
	while (nCode >= 0)
	{
		nCode = avcodec_receive_frame(m_pAudioDecoderCtx, pFrame);
		if (nCode < 0) break;

		if (av_buffersrc_add_frame_flags(m_pBufferSrcCtx, pFrame, AV_BUFFERSRC_FLAG_PUSH) < 0) {
			av_log(NULL, AV_LOG_ERROR, "buffer src add frame error!\n");
			bSucc = false;
		}
		nCode = av_buffersink_get_frame_flags(m_pBufferSinkCtx, m_pOutFrame, AV_BUFFERSINK_FLAG_NO_REQUEST);
		if (nCode < 0)
			bSucc = false;
		break;
	}
	nCode = avcodec_send_frame(m_pEncoderCtx, pFrame);
	if (nCode < 0)
	{
		bSucc = false;
		av_log(NULL, AV_LOG_ERROR, "avcodec_send_frame error.\n");
	}

	nCode = avcodec_receive_packet(m_pEncoderCtx, &m_pktOut);
	av_frame_free(&pFrame);

	return 0 == nCode ? true : false;
}

void AudioStreamHandle::save_stream(AVFormatContext* pFormatCtx, const AVPacket& packet)
{
	// check inited
	if (!m_bOutputInited || nullptr == pFormatCtx) {
		return;
	}
	// check the packet
	if (nullptr == packet.buf || 0 == packet.buf->size) {
		return;
	}
	AVPacket pktFrame /*= packet*/;
	av_init_packet(&pktFrame);
	av_packet_ref(&pktFrame, &packet);
	AVStream* pInStream = m_pInputAVFormatCtx->streams[pktFrame.stream_index];
	AVStream* pOutStream = pFormatCtx->streams[pktFrame.stream_index];
	// convert the pts and dts
	/*
	1. I 帧/P 帧/B 帧
		I 帧：I 帧(Intra-coded picture, 帧内编码帧，常称为关键帧)包含一幅完整的图像信息，属于帧内编码图像，不含运动矢量，在解码时不需要参考其他帧图像。因此在 I 帧图像处可以切换频道，而不会导致图像丢失或无法解码。I 帧图像用于阻止误差的累积和扩散。在闭合式 GOP 中，每个 GOP 的第一个帧一定是 I 帧，且当前 GOP 的数据不会参考前后 GOP 的数据。
		P 帧：P 帧(Predictive-coded picture, 预测编码图像帧)是帧间编码帧，利用之前的 I 帧或 P 帧进行预测编码。
		B 帧：B 帧(Bi-directionally predicted picture, 双向预测编码图像帧)是帧间编码帧，利用之前和(或)之后的 I 帧或 P 帧进行双向预测编码。B 帧不可以作为参考帧。
		B 帧具有更高的压缩率，但需要更多的缓冲时间以及更高的 CPU 占用率，因此 B 帧适合本地存储以及视频点播，而不适用对实时性要求较高的直播系统。
		DTS(Decoding Time Stamp, 解码时间戳)，表示压缩帧的解码时间。
		PTS(Presentation Time Stamp, 显示时间戳)，表示将压缩帧解码后得到的原始帧的显示时间。
		音频中 DTS 和 PTS 是相同的。视频中由于 B 帧需要双向预测，B 帧依赖于其前和其后的帧，因此含 B 帧的视频解码顺序与显示顺序不同，即 DTS 与 PTS 不同。当然，不含 B 帧的视频，其 DTS 和 PTS 是相同的。下图以一个开放式 GOP 示意图为例，说明视频流的解码顺序和显示顺序
	*/
	try {
		pktFrame.pts = av_rescale_q_rnd(pktFrame.pts, pInStream->time_base, pOutStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pktFrame.dts = av_rescale_q_rnd(pktFrame.dts, pInStream->time_base, pOutStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
		pktFrame.duration = av_rescale_q(pktFrame.duration, pInStream->time_base, pOutStream->time_base);
		pktFrame.pos = -1;
	}
	catch (const std::exception& e)
	{
		printf("Convert dts/pts exception:%s\n", e.what());
		return;
	}
	catch (...)
	{
		printf("Unkonw error\n");
		return;
	}
	switch (pInStream->codecpar->codec_type)
	{
	case AVMEDIA_TYPE_AUDIO:
	case AVMEDIA_TYPE_VIDEO:
	{
		//場景 ： 音频和视频来自外部来源(麦克风和摄像头)，并且无需任何压缩即可捕获为原始数据(即使是视频)。我使用h264编码视频，而没有压缩音频(PCM)。捕获的音频为:16位，44100khz，立体声。捕获的视频为25FPS。
		/*
		解决办法
		最好的办法是使用此类应用程序将音频/视频捕获为pts和dts时给出的时间戳。因此，这些并非完全实时的时间戳（来自时钟），而是媒体捕获的时间戳。

		av_interleaved_write_frame以这种方式写入otput数据包，以使它们正确交织(也许在内部对其进行排队)。 “正确插入”取决于容器格式，但是通常这意味着输出文件中数据包的DTS标记单调增加。

		像大多数FFmpeg API一样，不应由具有相同AVFormatContext的两个线程同时调用av_interleaved_write_frame。我假设您使用互斥锁来确保这一点。如果您这样做，则无论是多线程应用程序还是现在都没有关系。

		与在单线程应用程序中的方式相同。 Dts通常是由pts的编解码器生成的。 Pts通常来自捕获设备/解码器以及相应的音频/视频数据。

		可以使用实时时间戳，但这实际上取决于您获取它们的方式和时间。
		*/
		int nError = av_interleaved_write_frame(pFormatCtx, &pktFrame);
		if (nError != 0)
		{
			printf("Error: %d while writing frame, %s\n", nError, get_error_msg(nError).c_str());
		}
		break;
	}
	default:
		break;
	}
}

std::string AudioStreamHandle::get_filename(int nType)
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
		case kFileTypeAudio:
			strFilename = kVideoDir + "/" + m_strToday + "/" + strTimestamp + kAudioFormat;
			break;
		default:
			strFilename = kPictureDir + "/" + m_strToday + "/" + strTimestamp + kVidoeFormat;
			break;
		}
		return strFilename;
	};
	return generate_filename(nType);
}

std::string AudioStreamHandle::get_error_msg(int nErrorCode)
{
	char szMsg[AV_ERROR_MAX_STRING_SIZE] = { 0 };
	av_make_error_string(szMsg, AV_ERROR_MAX_STRING_SIZE, nErrorCode);
	std::stringstream ss;
	{
		ss << "Code[" << nErrorCode << "]:" << szMsg;
	}
	return ss.str();
}

CameraConnectingStatus AudioStreamHandle::get_camera_connecting_status()
{
	return cameraConnectingStatus;
}
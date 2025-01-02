 

```
//原始
bool RtspStreamHandle::open_codec_context(int& nStreamIndex, AVCodecContext** pDecoderCtx, AVFormatContext* pFmtCtx, enum AVMediaType nMediaType)
{
	AVStream* pStream = nullptr;
	AVCodec* pDecoder = nullptr;
	AVDictionary* pOptions = nullptr;
 
#ifdef _WIN32
	int stream_index = av_find_best_stream(pFmtCtx, nMediaType, -1, -1, (AVCodec**)&pDecoder, 0);  //據說可以 把參數 AVCodec **decoder_ret 傳入 null
#endif 

#ifdef __linux__
	int stream_index = av_find_best_stream(pFmtCtx, nMediaType, -1, -1, (const AVCodec**)&pDecoder, 0);
#endif 
	

	if (stream_index < 0)
	{
		fprintf(stderr, "Couldn't find %s stream in input\n", av_get_media_type_string(nMediaType));
		return false;
	}
	nStreamIndex = stream_index;
	AVPixelFormat nPixeFmt = AV_PIX_FMT_NONE;

	if (AVMEDIA_TYPE_VIDEO == nMediaType)  //if (AVMEDIA_TYPE_VIDEO == nMediaType && m_infoStream.nHDType > AV_HWDEVICE_TYPE_NONE)
	{
		// get hard device
		for (int i = 0;; i++)
		{
			try {
				const AVCodecHWConfig* pConfig = avcodec_get_hw_config(pDecoder, i);
				if (nullptr == pConfig) continue;
				if (!pConfig)
				{ 
				    //打印解码器类型
					av_log(NULL, AV_LOG_INFO, "Decoder %s does support device type %s.\n", pDecoder->name, av_hwdevice_get_type_name((AVHWDeviceType)m_infoStream.nHDType));
				    //LOG(INFO) << "\nDecoder " << pDecoder->name << " does support device type" << "av_hwdevice_get_type_name = "<< av_hwdevice_get_type_name((AVHWDeviceType)m_infoStream.nHDType)<<"\n";
					std::cout << "\nDecoder " << pDecoder->name << " does support device type" << "av_hwdevice_get_type_name = " << av_hwdevice_get_type_name((AVHWDeviceType)m_infoStream.nHDType) << "\n" << std::endl;
					
					// 檢查是否支持 CUDA
					if (pConfig->device_type == AV_HWDEVICE_TYPE_CUDA) {
						printf("解碼器支持 CUDA 硬件加速\n");
						return true;
					}

					// 無效CUDA則返回
					return false; 
				}
				if (pConfig->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
					pConfig->device_type == m_infoStream.nHDType) {
					nPixeFmt = pConfig->pix_fmt;

					fprintf(stderr, "Decoder %s does support device type %s.\n", pDecoder->name, av_hwdevice_get_type_name((AVHWDeviceType)m_infoStream.nHDType));  //打印解码器类型
					av_log(NULL, AV_LOG_INFO, "Decoder %s does support device type %s.\n", pDecoder->name, av_hwdevice_get_type_name((AVHWDeviceType)m_infoStream.nHDType));
					break;
				}
			}
			catch (...)
			{
				fprintf(stderr, "hw_decoder_init fail %s \n", av_get_media_type_string(nMediaType));
				av_log(NULL, AV_LOG_INFO, "hw_decoder_init fail %s \n", av_get_media_type_string(nMediaType));
			}
		}
	}
	
	/* Allocate a codec context for the decoder */
	*pDecoderCtx = avcodec_alloc_context3(pDecoder);
	if (!*pDecoderCtx)
	{
		fprintf(stderr, "Failed to allocate the %s codec context\n",
			av_get_media_type_string(nMediaType));
		return false;
	}

	/* Copy codec parameters from input stream to output codec context */
	pStream = m_pInputAVFormatCtx->streams[nStreamIndex];
	if ((stream_index = avcodec_parameters_to_context(*pDecoderCtx, pStream->codecpar)) < 0)
	{
		fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n", av_get_media_type_string(nMediaType));
		return false;
	}
	// init the hard device decoder
	if (AVMEDIA_TYPE_VIDEO == nMediaType && nPixeFmt != AV_PIX_FMT_NONE)
	{
		//沒有音頻的情況出現嚴重錯誤
		try {
			(*pDecoderCtx)->get_format = get_hw_format;
			(*pDecoderCtx)->pix_fmt = nPixeFmt;
			hw_decoder_init(*pDecoderCtx, (AVHWDeviceType)m_infoStream.nHDType);
			fprintf(stderr, "hw_decoder_init success %s \n", av_get_media_type_string(nMediaType));
			av_log(NULL, AV_LOG_INFO, "hw_decoder_init success %s \n", av_get_media_type_string(nMediaType));
		}
		catch (...)
		{
			fprintf(stderr, "hw_decoder_init fail!!! %s \n", av_get_media_type_string(nMediaType));
			av_log(NULL, AV_LOG_INFO, "hw_decoder_init success %s \n", av_get_media_type_string(nMediaType));
		} 
	}
	else {
		fprintf(stderr, "init the hard device decoder fail!! : %s \n", av_get_media_type_string(nMediaType));
	}

	//初始化音频 的硬件解码 以下的没法测试 ,补充于2023年1月28日
	if (AVMEDIA_TYPE_AUDIO == nMediaType && nPixeFmt != AV_PIX_FMT_NONE && m_infoStream.nAudioIndex != -1)
	{
		try {
			(*pDecoderCtx)->get_format = get_hw_format;
			(*pDecoderCtx)->pix_fmt = nPixeFmt;
			hw_decoder_init(*pDecoderCtx, (AVHWDeviceType)m_infoStream.nHDType); //HWDeviceType
		}
		catch (...)
		{
			fprintf(stderr, "audio decode hw_decoder_init fail!!! %s \n", av_get_media_type_string(nMediaType));
			av_log(NULL, AV_LOG_INFO, "audio decode hw_decoder_init fail %s \n", av_get_media_type_string(nMediaType));
		}
	}
	else {
		fprintf(stderr, "No AVMEDIA_TYPE_AUDIO (line 351): %s \n", av_get_media_type_string(nMediaType));
	}

	/* Init the decoders, with or without reference counting */
	av_dict_set(&pOptions, "refcounted_frames", m_infoStream.nRefCount ? "1" : "0", 0);
	if ((stream_index = avcodec_open2(*pDecoderCtx, pDecoder, &pOptions)) < 0)
	{
		fprintf(stderr, "Failed to open %s codec\n",
			av_get_media_type_string(nMediaType));
		return false;
	}
	return true;
}
```


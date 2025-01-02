# 自动适配CUDA硬件编解码

原理：



    //来自代码片段
    X:\MediaGuardAppV31\MediaGuardApp_Dev\MediaGuard\src\RtspStreamHandle.cpp
    原代码：
    
    //获取硬件格式
    enum AVPixelFormat RtspStreamHandle::get_hw_format(AVCodecContext* ctx, const enum AVPixelFormat* pix_fmts)
    {
    	const enum AVPixelFormat* pPixeFmt;
    	for (pPixeFmt = pix_fmts; *pPixeFmt != -1; pPixeFmt++)
    	{
    		if (*pPixeFmt == ctx->pix_fmt)
    			return *pPixeFmt;
    	}
    	fprintf(stderr, "Failed to get HW surface format.\n");
    	return AV_PIX_FMT_NONE;
    }
    
    //硬件解码初始化
    int RtspStreamHandle::hw_decoder_init(AVCodecContext* ctx, const enum AVHWDeviceType type)
    {
    	int nCode = 0;
    	if ((nCode = av_hwdevice_ctx_create(&m_pHDCtx, type, NULL, NULL, 0)) < 0) {
    		fprintf(stderr, "Failed to create specified HW device.maybe no hardware available \n");
    
    		//LOG(INFO) << "Failed to create specified hardware device for ffmpeg decode/encode.maybe no hardware available \n" << std::endl;
    		std::cout << "Failed to create specified hardware device for ffmpeg decode/encode.maybe no hardware available \n" << std::endl;
    
    		return nCode;
    	}
    	ctx->hw_device_ctx = av_buffer_ref(m_pHDCtx);
    
    	return nCode;
    }

 
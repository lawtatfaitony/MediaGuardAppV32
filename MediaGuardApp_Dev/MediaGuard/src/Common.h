#pragma once 
#include <iostream>
#include <cstdio>
#include <string>
#include <fstream>
#include <list>

#ifdef _WIN32
#include <io.h>
#include <direct.h>
#endif 

#include <chrono>
#include <thread>

//opencv 4.8.1 組件
#include "opencv2/core/hal/interface.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
 
//跨平台的時間業務文件 位於src下
#include "File.h"
#include "Time.h"

//業務功能
#include "CarPlate.h" 
#include "StreamDefine.h"
#include "StreamHandle.h"

#include "Basic/ThreadPool.h"
#include "ErrorInfo/ErrorCode.h" 

//curl 組件
#include "curl/curl.h"

//配置相关
#include "Config/DeviceConfig.h" //device.json配置文件相关
#include "Config/ConfigFile.h"

#include "Common/CrossPlat.h" 
#include "Common/Macro.h" 
#include "Common/JsonHelper.h"
#include "Common/TransCoding.h" 

#include "Http/LibcurlHelper.h"
 
//FFmpeg 4.2.2
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/avio.h>
#include <libavfilter/avfilter.h>
#include "libavutil/audio_fifo.h"
#include "libavfilter/buffersink.h"
#include <libavutil/imgutils.h>
#include "libavutil/samplefmt.h"
#include "libswresample/swresample.h"
#include "libavfilter/buffersrc.h"
#include "libavutil/time.h"
#include "libavutil/pixfmt.h"
}

#define VERIFY_RETURN(expr) \
    do {    \
        auto ret = expr;    \
        if(!ret) {  \
            return false;   \
        }   \
    } while (false);

#define VERIFY_RETURN_VOID(expr) \
    do {    \
        auto ret = expr;    \
        if(!ret) {  \
            return;   \
        }   \
    } while (false);


#define ENUM_INITIALIZE() \
   {    \
        CameraConnectingStatus CameraConnectingStatus[0];  \
        AVCodecID AVCodecID[AV_CODEC_ID_AAC];   \
   }    \

#define GUARD_LOCK(lock)    \
    std::lock_guard<std::mutex> locker(lock);

#define SHARED_LOCK(lock)    \
    std::unique_lock<std::mutex> locker(lock);

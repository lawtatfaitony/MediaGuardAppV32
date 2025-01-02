

# FFmpeg 硬解码和软解码

FFmpeg是一个开源的多媒体处理工具，可以用来进行视频/音频的编码和解码等多种任务。在C++中使用FFmpeg进行硬件加速解码通常需要使用FFmpeg的libavcodec库，并且配合系统的视频加速API，如VA-API（对于Linux）或者DXVA2（对于Windows）。

以下是一个简单的例子，展示如何使用FFmpeg在C++中初始化并使用硬件解码器。

`extern "C" {`
`#include <libavcodec/avcodec.h>`
`#include <libavformat/avformat.h>`
`#include <libswscale/swscale.h>`
`}`

`int main() {`
    `// 注册库，这应该在程序初始化时调用一次`
    `av_register_all();`

    // 打开视频文件
    AVFormatContext* pFormatCtx = avformat_alloc_context();
    if (avformat_open_input(&pFormatCtx, "input.mp4", NULL, NULL) != 0) {
        // 打开文件失败
        return -1;
    }
     
    // 查找流信息
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        // 获取流信息失败
        return -1;
    }
     
    // 找到视频流的索引
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
     
    if (videoStreamIndex == -1) {
        // 没有找到视频流
        return -1;
    }
     
    // 获取解码器并打开
    AVCodecContext* pCodecCtx = pFormatCtx->streams[videoStreamIndex]->codec;
    AVCodec* pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL) {
        // 没有找到解码器
        return -1;
    }
     
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        // 打开解码器失败
        return -1;
    }
     
    // 硬件解码设置
    AVDictionary* opts = NULL;
    av_dict_set(&opts, "hwdev", "quartz_gl", 0); // 例子中使用Mac的OpenGL
    av_dict_set(&opts, "sw_pix_fmt", "yuv420p", 0); // 指定软件解码格式
    if (avcodec_open2(pCodecCtx, pCodec, &opts) < 0) {
        // 硬件解码失败，尝试软件解码
        avcodec_open2(pCodecCtx, pCodec, NULL);
    }
     
    // 解码循环
    AVPacket packet;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {
        if (packet.stream_index == videoStreamIndex) {
            // 解码视频帧
            AVFrame* pFrame = av_frame_alloc();
            int gotFrame = 0;
            if (avcodec_decode_video2(pCodecCtx, pFrame, &gotFrame, &packet) >= 0) {
                if (gotFrame) {
                    // 处理解码后的帧
                }
            }
            av_frame_free(&pFrame);
        }
        av_packet_unref(&packet);
    }
     
    // 释放资源
    avformat_close_input(&pFormatCtx);
    avcodec_free_context
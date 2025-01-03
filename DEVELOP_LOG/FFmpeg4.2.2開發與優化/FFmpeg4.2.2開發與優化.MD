# FFmpeg4.2.2開發與優化

##  



AVPacket* packet = av_packet_alloc(); 

之前是 AVPacket* packet ; 沒有進行分配 2025-1-3

```
while (!m_bExit) {
    AVPacket* packet = av_packet_alloc(); // 在每次迭代中分配新的 AVPacket
    if (!packet) {
        // 處理錯誤，例如內存分配失敗
        fprintf(stderr, "Could not allocate AVPacket\n");
        break; // 退出循環
    }

    int nCode = av_read_frame(m_pInputAVFormatCtx, packet);
    if (nCode < 0) {
        cameraConnectingStatus = CameraConnectingStatus::InRequestStopped;
        av_packet_free(&packet); // 釋放 AVPacket
        break; // 退出循環
    }

    if (packet->stream_index == m_infoStream.nAudioIndex) {
        if (decode_audio_packet(packet, nLoop)) {
            if (m_infoStream.bSaveVideo) {
                save_stream(m_pOutputFileAVFormatCtx, packet);
                auto nMillSecond = Time::GetMilliTimestamp();
                if (nMillSecond - nLastSaveVideo >= nVideoTime) {
                    nLastSaveVideo = nMillSecond;
                    release_output_format_context(m_pOutputFileAVFormatCtx);
                    open_output_stream(m_pOutputFileAVFormatCtx);
                }
            }
            if (m_infoStream.bRtmp)
                save_stream(m_pOutputStreamAVFormatCtx, packet);
        }
    }

    av_packet_free(&packet); // 釋放 AVPacket
}

```

修改 File.h 函數 static std::string readJsonFile(const std::string& strFile) 

跳轉到文件 : C:\Users\xxx\AppData\Local\Microsoft\Linux\HeaderCache\1.0\-418563307\usr\include\stdio.h

```
出現的問題:
#if defined __USE_MISC || defined __USE_XOPEN
/* Generate a unique temporary filename using up to five characters of PFX
   if it is not NULL.  The directory to put this file in is searched for
   as follows: First the environment variable "TMPDIR" is checked.
   If it contains the name of a writable directory, that directory is used.
   If not and if DIR is not NULL, that value is checked.  If that fails,
   P_tmpdir is tried and finally "/tmp".  The storage for the filename
   is allocated by `malloc'.  */
extern char *tempnam (const char *__dir, const char *__pfx)
   __THROW __attribute_malloc__ __wur __attr_dealloc_free;
#endif
```


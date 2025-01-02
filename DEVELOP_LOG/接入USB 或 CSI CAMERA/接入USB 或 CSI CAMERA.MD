# 接入USB 或 CSI CAMERA



## FFmpeg 列出可用的鏡頭設備



要在 Windows 上使用 `ffmpeg` 列出可用的設備，您可以使用 `cmd` 命令行並輸入如下命令：

1. **打開 cmd**：按 `Win + R` 打開運行窗口，輸入 `cmd`，然後按 `Enter`。

2. **運行以下命令**，列出 DirectShow 設備選項：

   bash

   ```
   ffmpeg -list_options true -f dshow -i video="USB 2861 Device"
   ```

這將顯示與 `USB 2861 Device` 相關的所有選項和可用設備。

如果您需要列出所有可用的 DirectShow 設備，可以使用以下命令：

bash

```
ffmpeg -list_devices true -f dshow -i dummy
```

這會列出系統中所有可用的音頻和視頻輸入設備。

![image-20241222210707378](README_IMGs/接入USB CAMERA/image-20241222210707378.png)

/*
Command:
 ffmpeg -f dshow -i video="USB 2861 Device" -f dshow -i audio="线路 (3- USB Audio Device)" -vcodec libx264 -acodec aac -strict -2 mycamera.mkv
 上面的命令行用video=指定视频设备，用audio=指定音频设备，后面的参数是定义编码器的格式和属性，输出为一个名为mycamera.mkv的文件。

  **ffmpeg -list_options true -f dshow -i video="USB 2861 Device"**
  这个命令行的作用是获取指定视频采集设备支持的分辨率、帧率和像素格式等属性，返回的是一个列表，结果如下：

 ref:https://blog.csdn.net/fuhanghang/article/details/123553898
 输入下面的语句即可列出电脑的设备

**ffmpeg -list_devices true -f dshow -i dummy**

infoStreamII.strInput = "Integrated Camera";
命令 ffplay -f dshow -i video="Integrated Camera"  经测试是OK的
ffmpeg -f dshow -i video="Integrated Camera" -s 1280x720 -t 30 -b:v 1000k -b:a 128k output.mp4 经测试OK
-t 30 表示录像30秒

*/



## RtspStreamHandle.cpp 自適應改造

​	RtspStreamHandle.cpp 如何判斷不是 rtsp ulr  則自適應對應鏈接的方式. 備註於 2025-1-2
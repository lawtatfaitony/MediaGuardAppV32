# MediaGuard PROJECT


### device.json 配置節點 [carPlateRecogBusiness]
車牌識別：主要POST 到CarBusiness停車計時系統使用的。並且有具體的雲端API POST接口和限定鏡頭ID [permited_cameraId] list 等等

### 基於TASK 識別任務，如果Type_Mode 重複，則獲取的只會是updateTime最新的記錄。當前設計的任務識別功能有6大功能：
    
       1. 任務類型: UNDEFINED 未定義的任務
       UNDEFINED = 0,

       2. 任務類型:CAMERA_GUARD 桌面人臉識別系統  
        CAMERA_GUARD = 1, //不在任務中設置此功能

       3.  任務類型: CAMERA_DVR 桌面錄像系統
        CAMERA_DVR = 2,

       4. 任務類型: HIK_DATA_RETRIVE 桌面海康設備數據獲取系統
        DESKTOP_HIK_DATA_RETRIVE = 3,//不在任務中設置此功能

       5. 任務類型: HIK_DATA_ANDROID_RETRIVE  手機版的海康設備數據獲取系統
        ANDROID_HIK_DATA_RETRIVE = 4,//不在任務中設置此功能

       6. 任務類型: ANDROID_CIC_DATA_RETRIVE  手機版的 CIC的NFC拍卡數據獲取系統
        ANDROID_CIC_DATA_RETRIVE = 5, //不在任務中設置此功能

       7. 任務類型: 车牌识别 POST到Python处理后返回
        CAR_PLATE_RECOGNITION = 6,

       8. 任務類型: 人脸识别
        FACE_RECOGNITION = 7,

       9. 任務類型: 工程着装识别
        WORK_CLOTHES_RECOGNITION = 8,

      10. 任務類型: 佩戴头盔识别
        WEARING_HELMET_RECOGNITION = 9,

      11. 任務類型: 有人闖入
        SOMEONE_BROKE_IN = 10

2023-2-23  
HLS 保留20秒的切片文件（ts）记录 注意设置hls av_dist的时候不能大于这个时间，
2023-2-23  
HLS解码功能,完成，需要完善PTS和DTS精确度。需要增加删除清理过期的ts文件  
提示： pkt->duration = 0, maybe the hls segment duration will not precise  

2023-2-23    
HLS解码功能，无法进入解码流，exit退出后才能进入hls解码流输出，但由于系统任意键主线程几秒后退出，导致主线程停止，应该是之前的RtspStreamHandle.cpp内的其他输出解码流线程阻塞导致。
  
2023-2-13  
//解决pts dts 一开始出现负数的问题  

//关于ffmpeg 解码学习的代码干货：  
https://www.cnblogs.com/gongluck/archive/2019/05/07/10827950.html  

//關於MediaGuard.exe是否佔用端口可使用命令：netstat -anb
####  
請用UTF8-BOM （UTF8帶簽名編碼，是UNICODE的子集）打開文件。
####
2023-1-17
 Audio 沒有判斷是否有 ：dshow 提示 ： Could not enumerate video devices (or not found)

Open input[rtsp://root2:123456@192.168.10.90:554/axis-media/media.amp?videocodec=h264&resolution=640x480] success.
Input #0, rtsp, from 'rtsp://root2:123456@192.168.10.90:554/axis-media/media.amp?videocodec=h264&resolution=640x480':
  Metadata:
    title           : Session streamed with GStreamer
    comment         : rtsp-server
  Duration: N/A, start: 0.240000, bitrate: N/A
    Stream #0:0: Video: h264 (High), yuvj420p(pc, bt709, progressive), 640x480, 25 fps, 25 tbr, 90k tbn, 50 tbc
Couldn't find audio stream in input 
REF:https://blog.csdn.net/qq_42780025/article/details/113930914

2023-1-17 14:38:48
對FFmpeg 具體的接口函數加入注釋。

2023-1-17 18:19:35
估計內部線程有問題 導致強行退出。
###
# 跨平台开发 
参考 https://www.cnblogs.com/end-emptiness/p/15083829.html

----------------------------------------------------------  
# unicode开发
为了避免不同平台编码不同导致的乱码问题，通常会转成unicode编码来做

首先需要切换解决方案的字符集：
配置属性 -> 常规 -> 字符集 -> 使用 Unicode 字符集

cmake指令是：add_definitions(-DUNICODE -D_UNICODE)

类型：wstring（字符串）, wchar_t（字符）

# 常量字符初始化
wstring a = L"啊啊啊啊啊";
wchar_t b[] = {L'哈', L'呵'};  
###
----------------------------------------------------------  
swscaler @ 000001e81e060380] Warning: data is not aligned! This can lead to a speed loss  这是初始化包造成的 av_init_packet(&pktFrame);
----------------------------------------------------------  
###
Read frame failed,Code[-541478725]:End of file

###
關於App Path,如果编译器支持C++17，则建议使用std::filesystem::current_path
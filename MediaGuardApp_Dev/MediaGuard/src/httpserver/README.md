# MediaGuard Http Server  
# 开发日誌 2023-7-13 
## 引用的組件參考: https://gitee.com/AIQICcorg/cpp-httplib

# cpp-httplib (**httplib.h**) 

### 相關文件: httpserver.cc ; httpserver.h ; comm.h ; comm.cpp



基本完成http server 請求測試，具體測試連接如下 ：

 token來自device.json 》**device_config.local_authorization** 節點value

播放Mp4必須傳入參數Token(來自設備 MD5(username:passwrd).LowerCase() )

AIGUARD 的結果 ：http://192.168.0.39:180/video/2023-08-06/1691310913435.mp4?token=2ccd575f54be0d71277a82f8baf2e8ea



http://localhost:8080/video/2023-07-30/1688.mp4?token=7ad166bdb8395514bb54cc0ac21db289

鏡頭直播：http://localhost:8080/hls/12/index.m3u8?token=7ad166bdb8395514bb54cc0ac21db289     可能出現延時1-2秒不等，如果硬件和互聯網網速太慢甚至更慢。CameraId=12

主控台要求UTF8編碼 

mime content-type ("m3u8", "application/vnd.apple.mpegurl");
mime content-type ("ts", "video/mp2t");

### **PICTURE  SUPPORT**

圖片：http://localhost:8080/picture/1690813262155.jpg?token=7ad166bdb8395514bb54cc0ac21db289



## **HLS  SUPPORT**

 http://localhost:8080/hls/12/index.m3u8?token=7ad166bdb8395514bb54cc0ac21db289

http://localhost:8080/hls/8/index.m3u8?token=7ad166bdb8395514bb54cc0ac21db289

### **WEB  SUPPORT**

http://localhost:8080/web/index.html?token=7ad166bdb8395514bb54cc0ac21db289

http://localhost:8080/web/tokentest.html?token=7ad166bdb8395514bb54cc0ac21db289

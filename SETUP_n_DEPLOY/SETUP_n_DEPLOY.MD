# SETUP AND DEPLOY



## DEPLOY

### 	LINIX ENVIRONMENT



#### ‌ 			bash script file to run

​				runexecutable.sh

```
# 運行執行文件 executable (application/x-executable) 必須先指定共享文件(如 libswresample.so.4 FOR openssl-1.1.1q) 
# 共享文件:
sudo ldconfig
export LD_LIBRARY_PATH=/home/tonylaw/Desktop/MediaGuard_Cmake:$LD_LIBRARY_PATH
./MediaGuard
```

### 	WINDOWS ENVIRONMENT



## TESTING

​	http://192.168.0.128:180/test

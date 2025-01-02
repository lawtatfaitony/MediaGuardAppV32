# 運行執行文件 executable (application/x-executable) 必須先指定共享文件(如 libswresample.so.4) FOR openssl-1.1.1q 
# https://developer.nvidia.com/video-encode-and-decode-gpu-support-matrix-new?locale=en
# 共享文件:
sudo ldconfig
export LD_LIBRARY_PATH=/home/tonylaw/Desktop/MediaGuard_Cmake:$LD_LIBRARY_PATH
./MediaGuard
# NVIDA驅動組件安裝



## 安裝CUDA

**cuda_12.6.3_561.17_windows**



显卡算力 查询：https://developer.nvidia.cn/cuda-gpus

![img](README_IMGs/README/2323423456890586.png)

```
C:\Users\TonyLaw>nvidia-smi
 
nvidia-smi 
========================================================================================================
Mon Dec  9 15:36:37 2024
+---------------------------------------------------------------------------------------+
| NVIDIA-SMI 538.18                 Driver Version: 538.18       CUDA Version: 12.2     |
|-----------------------------------------+----------------------+----------------------+
| GPU  Name                     TCC/WDDM  | Bus-Id        Disp.A | Volatile Uncorr. ECC |
| Fan  Temp   Perf          Pwr:Usage/Cap |         Memory-Usage | GPU-Util  Compute M. |
|                                         |                      |               MIG M. |
|=========================================+======================+======================|
|   0  NVIDIA GeForce GTX 1650 ...  WDDM  | 00000000:01:00.0 Off |                  N/A |
| N/A   56C    P0              14W /  35W |      0MiB /  4096MiB |      3%      Default |
|                                         |                      |                  N/A |
+-----------------------------------------+----------------------+----------------------+
+---------------------------------------------------------------------------------------+
| Processes:                                                                            |
|  GPU   GI   CI        PID   Type   Process name                            GPU Memory |
|        ID   ID                                                             Usage      |
|=======================================================================================|
+---------------------------------------------------------------------------------------+
```

#####  

 

## 系统部署



### widnows部署

D:\MediaGuardAppV31\OutRelease\Win32_OutputRelease\MediaGuard.exe 直接打開運行

### linux部署

在Linux中，程序的打包部署通常涉及将程序的可执行文件、依赖库、配置文件、文档等集中在一起，并有时提供一个安装脚本。以下是一个基本的示例，使用`tar`命令创建一个程序的打包归档文件（例如，一个名为`myapp`的简单程序），首先，确保你已经安装了所有必要的依赖。然后，使用`ldd`命令找出程序所需的共享库：

```shell
 # 運行執行文件 executable (application/x-executable) 必須先指定共享文件(如 libswresample.so.4) FOR openssl-1.1.1q 

# 共享文件:
sudo ldconfig
export LD_LIBRARY_PATH=/home/TonyLaw/Desktop/MediaGuard_Cmake:$LD_LIBRARY_PATH
./MediaGuard
```

 





 



 

 

 

 

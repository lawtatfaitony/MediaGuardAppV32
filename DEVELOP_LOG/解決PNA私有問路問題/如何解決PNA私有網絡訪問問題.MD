# 如何解決PNA私有網絡訪問問題



### 獲取外網IP

### 	1. 通過路由器獲得

<img src="README_IMGs/如何解決PNA私有網絡訪問問題/image-20241226233156052.png" alt="image-20241226233156052" style="zoom:67%;" />

​				**Internet Dynamic IP : 219.77.139.89**

​				WAN MAC Address : C0-06-C3-5C-C8-79   Internet IP Address: 219.77.139.89

​				**測試效果:**

 				http://219.77.139.89:180/test
				 http://192.168.0.128:180/test	

​				<img src="README_IMGs/如何解決PNA私有網絡訪問問題/image-20241226234736782.png" alt="image-20241226234736782" style="zoom:80%;" />	

​				**對應 的 192.168.0.128:180 改為 219.77.139.89:180**

### 	2. C++ STUN 網絡穿透技術



### 	3. 命令工具 curl 獲取外網IP

		要使用 `curl` 命令獲取 ISP 分配的互聯網 IP 地址，可以通過訪問一些提供此服務的公共 API 來實現。以下是幾個常用的 API 和相應的 `curl` 命令：

1. **使用 `ipinfo.io`**：

   ```bash
   curl ipinfo.io/ip
   ```

2. **使用 `ifconfig.me`**：

   ```bash
   curl ifconfig.me
   ```

3. **使用 `api.ipify.org`**：

   ```bash
   curl api.ipify.org
   ```

 <img src="README_IMGs/如何解決PNA私有網絡訪問問題/image-20241226235139033.png" alt="image-20241226235139033" style="zoom:67%;" />



### 第一種 NAT HTTP轉發

​	<img src="README_IMGs/如何解決PNA私有網絡訪問問題/image-20241226232239882.jpg" alt="image-20241226232239882" style="zoom:67%;" />

### **Port Forwarding**

![image-20241226233608303](README_IMGs/如何解決PNA私有網絡訪問問題/image-20241226233608303.png)

### 第二種 NAT SOCKET轉發
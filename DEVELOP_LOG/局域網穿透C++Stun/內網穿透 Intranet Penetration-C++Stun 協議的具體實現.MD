# 內網穿透 Intranet Penetration

* 原理 
* https://arthurchiao.art/blog/how-nat-traversal-works-zh/ ★★★★★入門必讀
* https://arthurchiao.art/blog/how-nat-traversal-works-zh/ ★★★★★入門必讀
* [译] NAT 穿透是如何工作的：技术原理及企业级实践

## 請求的服務器

const char* STUN_SERVER_IP = "stun.l.google.com"; // STUN 伺服器 IP
const int STUN_SERVER_PORT = 19302; // STUN 伺服器端口 

## 最終結果

<img src="README_IMGs/局域網穿透C++Stun 協議的具體實現/image-20241228043930512.png" alt="image-20241228043930512" style="zoom:67%;" />

獲取的 IP: 204.204.204.204
獲取的端口: 52428

<img src="README_IMGs/局域網穿透 Intranet Penetration-C++Stun 協議的具體實現/image-20241228050216620.png" alt="image-20241228050216620" style="zoom:67%;" />

**根據stun協議 ,這個結果和 代碼開始設置 的 STUN_SERVER_IP = "stun.l.google.com"  有什麼關係或邏輯關係**

可能情況:

// 獲取 STUN 伺服器的 IP 和端口 2024-12-30
// 調試可以獲得 stun_server_ip = 118.90.3.42 但不是本公網的IP 應該是stun Server的 可能buffer[] 截取不對應的地方
// char* public_ip3;
// int public_port3;
// natHeartBean.get_server_internet_ip_port(public_ip3, public_port3);
// std::cout << "\nFUNC::NatHeartBean.get_server_internet_ip_port\n" << std::endl;
// std::cout << "Public IP Method By NAT Stun 3: " << public_ip3 << ":" << public_port3 << std::endl;

========================================================

在使用 STUN 協議時，獲取的 IP 地址和端口號與設置的 STUN 伺服器（如 `stun.l.google.com`）之間有特定的邏輯關係。以下是這種關係的解釋：

### STUN 協議的工作原理

1. **STUN 伺服器的角色**：
   - STUN（Session Traversal Utilities for NAT）伺服器的主要作用是幫助客戶端在 NAT（網絡地址轉換）環境中獲取其公共 IP 地址和端口號。這對於需要建立點對點連接的應用（如 VoIP 和視頻通話）非常重要。

2. **請求和響應**：
   - 當客戶端向 STUN 伺服器發送請求（例如 Binding Request）時，伺服器會根據請求包中的信息來回應客戶端的公共地址和端口。這些信息是從伺服器的角度來看客戶端的地址。

3. **結果解釋**：
   - 在您的例子中，您獲取到的 IP 地址 `204.204.204.204` 和端口 `52428` 是 STUN 伺服器返回的結果。這表示您的客戶端在 NAT 環境中的公共可見地址是 `204.204.204.204`，且該地址上的端口是 `52428`。
   - 這個 IP 和端口是 STUN 伺服器根據您的請求計算出來的，並不是直接從 `stun.l.google.com` 得到的，而是 STUN 伺服器知道如何將您的請求與其公共接口關聯起來。

### 總結

- `STUN_SERVER_IP = "stun.l.google.com"` 是您用來發送請求的伺服器地址。
- 獲取的 `204.204.204.204` 和 `52428` 是 STUN 伺服器根據您的請求返回的公共 IP 和端口，這些信息允許其他網絡實體（例如其他客戶端或服務器）能夠通過這個地址來直接與您的客戶端進行通信。

這樣的設計使得在 NAT 環境中，客戶端能夠獲取可用於建立連接的公共地址，從而實現 P2P 通信。

========================================================

**但是本機的的路由器信息如上所示:42.3.90.118, 並不是204.24.204.204** ?

您觀察到的情況是正常的，因為 STUN 協議的工作原理涉及 NAT（網絡地址轉換）和公共 IP 地址的獲取。以下是一些解釋：

### 1. **公共 IP 地址與 STUN 伺服器的關係**

- **STUN 伺服器的功能**：當您使用 STUN 協議時，您的客戶端會向 STUN 伺服器（如 `stun.l.google.com`）發送請求，伺服器會告訴您它所看到的您的公共 IP 和端口。這是因為 STUN 伺服器位於互聯網上，能夠看到 NAT 設備所映射的地址。

- **獲取的地址**：您獲取到的地址 `204.204.204.204` 是 STUN 伺服器返回的結果，這並不一定是您路由器的 WAN IP。它可能是 NAT 設備映射的地址，或者是 STUN 伺服器所在網絡的某種配置。

### 2. **本機路由器的信息**

- 您提到的路由器信息 `42.3.90.118` 是您本地網絡的公共 IP 地址。這是您路由器向外部互聯網提供的地址。

- 如果您在 NAT 環境中，STUN 伺服器可能會返回一個與您的實際 WAN IP 不同的地址，這可能是由於 NAT 配置或 STUN 伺服器的特定行為。

### 3. **可能的原因**

- **多個 NAT 層**：如果您的網絡中有多個 NAT 層，STUN 伺服器可能會返回一個不同的地址。

- **STUN 伺服器的行為**：有些 STUN 伺服器可能會返回與您的實際公共 IP 不同的地址，具體取決於它們的配置和網絡環境。

### 總結

在使用 STUN 協議時，獲取到的 IP 地址不一定總是與您的路由器 WAN IP 一致。建議您檢查 NAT 配置和使用其他工具（如 `traceroute` 或其他 STUN 伺服器）來進一步診斷問題。



最後是源代碼:

## NatHeartBean.h

```
#pragma once
#include <iostream> 
#include <cstring> 
#include <cstdint> // 用於 uint16_t, uint32_t 等類型
#include <io.h> 
#include <cerrno> 
#include <stdio.h> 
#include <cstdio>    // 包含 popen 和 pclose 的定義 
#include <memory>    // 包含 std::unique_ptr
#include <stdexcept> // 包含 std::runtime_error
#include <string>    // 包含 std::string
#include <array>

#ifdef _WIN32
#include <winsock2.h>  // Windows socket
#include <ws2tcpip.h>  // Windows TCP/IP 
#pragma comment(lib, "ws2_32.lib") // Windows socket library
#else
#include <arpa/inet.h> // POSIX socket
#include <unistd.h>    // POSIX standard 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h> 

#endif
  
 
// STUN 消息类型
#define STUN_BINDING_REQUEST 0x0001
#define STUN_BINDING_RESPONSE 0x0101

class NatHeartBean {
 
public:
	explicit NatHeartBean(); 
	~NatHeartBean();
private:
	const char* STUN_SERVER = "stun.l.google.com";    //免费的公共stun服务器(stun.l.google.com:19302、stun2.l.google.com:19302、global.stun.twilio.com:3478等) 
	const int STUN_SERVER_PORT = 19302; // STUN 伺服器端口 
	// STUN 請求的大小
	const int STUN_REQUEST_SIZE = 20; // 根據需要調整大小 

	//包裝 bind 函數 避免socket的bind函數名稱衝突
	int bind_socket(int sockfd, const struct sockaddr* addr, socklen_t addrlen);

	//設置 STUN 請求的內容
	void create_stun_request();

public:
	
	// 使用STUN 協議失敗,保留這兩個函數以後優化或者改造 結果 204.204.204.204
	// 獲取 IP and PORT 
	void get_server_internet_ip_port(char*& loacal_ip, int& local_port);
	 
	// 使用STUN 協議失敗,保留這兩個函數以後優化或者改造 結果 0.0.0.0 
	void get_local_internet_ip_and_port(char*& loacal_ip, int& local_port);
	
	

	// 通過curl 客戶端命令 獲取外網ip的方式是可用的 ok 2024-12-29
	// 保留log文件的方式
	std::string get_public_ip_by_curl();
	// 通過curl 客戶端命令 獲取外網ip的方式是可用的 ok 2024-12-29
	// 通過內存獲取 客戶端命令結果
	std::string get_public_ip_by_curl_memory();

	char stun_request[20];
	int request_length;
};
```

## NatHeartBean.cpp

```
#include "NatHeartBean.h"

NatHeartBean::~NatHeartBean()
{
    // 析構函數
}

NatHeartBean::NatHeartBean()
{
    // 構造函數
} 
 

/*
* 開發說明 
* https://arthurchiao.art/blog/how-nat-traversal-works-zh/ ★★★★★入門必讀
* https://arthurchiao.art/blog/how-nat-traversal-works-zh/ ★★★★★入門必讀
* [译] NAT 穿透是如何工作的：技术原理及企业级实践
* 
*/
 
// 獲取本地Internet IP 和端口 // STUN NAT 目前有點問題
void NatHeartBean::get_local_internet_ip_and_port(char* &local_ip, int &local_port) {
     
    char local_internet_ip[INET_ADDRSTRLEN]; // 定義 local_internet_ip
    unsigned short local_internet_port; // 定義 local_port
    
    // 創建一個 UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return;
    }

    // 設置本地地址結構
    struct sockaddr_in local_internet_addr;
    memset(&local_internet_addr, 0, sizeof(local_internet_addr));
    local_internet_addr.sin_family = AF_INET;
    local_internet_addr.sin_port = 0; // 讓系統自動分配端口
    local_internet_addr.sin_addr.s_addr = htonl(INADDR_ANY); // 綁定到所有可用接口
     
    // 獲取綁定的端口
    socklen_t addr_len = sizeof(local_internet_addr);
    if (bind_socket(sockfd, (struct sockaddr*)&local_internet_addr, sizeof(local_internet_addr)) < 0) {
        std::cerr << "Failed to bind socket." << std::endl;

#ifdef _WIN32
        closesocket(sockfd); // 在 Windows 中使用 closesocket
#else
        close(sockfd); // 在 Linux 中使用 close
#endif
        return;
    }
     
    // 獲取本機 IP 地址和端口
    inet_ntop(AF_INET, &local_internet_addr.sin_addr, local_internet_ip, sizeof(local_internet_ip));
    local_internet_port = ntohs(local_internet_addr.sin_port);

#ifdef DEBUG
    std::cout << "local_internet_ip: " << local_internet_ip << std::endl;
    std::cout << "local_internet_port: " << local_internet_port << std::endl;
#endif // DEBUG
     
    // 檢查是否是有效的互聯網 IP
    if (strcmp(local_internet_ip, "127.0.0.1") == 0 ||
        strncmp(local_internet_ip, "10.", 3) == 0 ||
        strncmp(local_internet_ip, "172.", 4) == 0 ||
        strncmp(local_internet_ip, "192.168.", 8) == 0) {
        std::cerr << "Obtained a private or loopback IP: " << local_internet_ip << std::endl;
        // 根據需求處理私有或回環地址
    }
    else {
        // 分配內存並拷貝 INTERNET IP 地址 
        strcpy(local_ip, local_internet_ip);
        local_port = local_internet_port;
    }
     
    // 關閉 socket
#ifdef _WIN32
    closesocket(sockfd);
#else
   close(sockfd); // 關閉 socket 句柄
#endif
}

// 包裝 bind 函數 避免socket的bind函數名稱衝突
int  NatHeartBean::bind_socket(int sockfd, const struct sockaddr* addr, socklen_t addrlen) {
    return ::bind(sockfd, addr, addrlen);
}

// 獲取 STUN 伺服器的 IP 和端口 2024-12-30
// 調試可以獲得 stun_server_ip = 118.90.3.42 但不是本公網的IP 應該是stun Server的 可能buffer[] 截取不對應的地方
// char* public_ip3;
// int public_port3;
// natHeartBean.get_server_internet_ip_port(public_ip3, public_port3);
// std::cout << "\nFUNC::NatHeartBean.get_server_internet_ip_port\n" << std::endl;
// std::cout << "Public IP Method By NAT Stun 3: " << public_ip3 << ":" << public_port3 << std::endl;
void NatHeartBean::get_server_internet_ip_port(char*& stun_server_ip, int& stun_server_port) {

    // 創建 STUN 請求
    create_stun_request();

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return;
    }

    struct addrinfo hints, * res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    // 解析主機名
    if (getaddrinfo(STUN_SERVER, nullptr, &hints, &res) != 0) {
        std::cerr << "Failed to resolve hostname." << std::endl;
        return;
    }

    // 將解析的地址設置到 server_addr 中
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(STUN_SERVER_PORT);
    server_addr.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
    // 釋放資源
    freeaddrinfo(res);
  
	// 將 STUN 伺服器的 IP 和端口設置到 server_addr 中
    inet_pton(AF_INET, STUN_SERVER, &server_addr.sin_addr);
     
    // 發送請求 
    sendto(sockfd, reinterpret_cast<const char*>(stun_request), request_length,  0, (struct sockaddr*)&server_addr, sizeof(server_addr));
     
    // 接收響應
    char buffer[1024];
    // 构造 STUN 绑定请求
    memset(buffer, 0, sizeof(buffer));
    //---------------------------------------------------
    buffer[0] = (STUN_BINDING_REQUEST >> 8) & 0xFF;
    buffer[1] = STUN_BINDING_REQUEST & 0xFF;
    buffer[2] = 0x00; // 事务 ID
    buffer[3] = 0x00; // 事务 ID
    buffer[4] = 0x00; // 事务 ID
    buffer[5] = 0x00; // 事务 ID
    buffer[6] = 0x00; // 事务 ID
    buffer[7] = 0x00; // 事务 ID
    buffer[8] = 0x00; // 事务 ID
    buffer[9] = 0x00; // 事务 ID
    socklen_t addr_len = sizeof(server_addr);

    struct timeval timeout;
    timeout.tv_sec = 5; // 超時時間，單位為秒
    timeout.tv_usec = 0; // 微秒
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)); 
    size_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &addr_len);

    if (recv_len < 0) {
        perror("\nrecvfrom failed\n");
        // 處理錯誤
    }
    else {
        // 打印接收到的 buffer 內容
        std::cout << "\n[func::get_server_internet_ip][stun Protocol] receive the socket data: recv_len = "<< recv_len <<"\n" << std::endl;
		//內容太大 不打印
       /* for (size_t i = 0; i < recv_len; ++i) {
            printf("%02x ", static_cast<unsigned char>(buffer[i]));
        }*/
    }

    // 假設 buffer 是接收到的響應
    if (recv_len > 0) {
         
        char c_server_ip[INET_ADDRSTRLEN]; 
       
        // 解析 STUN 响应
        if (buffer[0] == 0x01 && buffer[1] == 0x01) {
            // 公网 IP 和端口在响应中
            uint32_t mappedAddress = (buffer[28] << 24) | (buffer[29] << 16) | (buffer[30] << 8) | buffer[31];
            uint16_t mappedPort = (buffer[32] << 8) | buffer[33];

            // 输出公网 IP 和端口
            struct in_addr ipAddr;
            ipAddr.s_addr = mappedAddress;

            stun_server_ip = inet_ntoa(ipAddr);
            int ipublic_port = ntohs(mappedPort);
            std::string s_public_port = std::to_string(ipublic_port).c_str();

            printf("\n[fun::get_server_internet_ip] [printf] Public IP: %s , Port: %s\n", stun_server_ip, s_public_port.c_str());

            std::cout << "\n[fun::get_server_internet_ip] Public IP: " << stun_server_ip << ", Port: " << ipublic_port << std::endl;
        }
        else {
            std::cout << "\n[func::get_server_internet_ip] Invalid STUN response.\n" << std::endl;
        } 
    }
     

#ifdef _WIN32
    closesocket(sockfd);
#else
    close(sockfd); // 關閉 socket 句柄
#endif

    return;
}

// ...
void NatHeartBean::create_stun_request() {
    // 清空請求
    memset(stun_request, 0, STUN_REQUEST_SIZE);

    // 設置請求的標頭
    // stun_request 的結構設置如下：
    // stun_request[0]：版本和類型
    // stun_request[1]：方法碼（Binding Request）
    // stun_request[2] 和 stun_request[3]：消息長度（此處設置為 0，因為沒有額外屬性）
    // stun_request[8]：識別符的開始位置
	//--------------------------------------------------------------------------------
     // 設置請求的標頭
    stun_request[0] = 0x00; // 版本和類型
    stun_request[1] = 0x01; // 方法碼（Binding Request）

    // 設置消息長度（這裡為 0，因為沒有額外的屬性）
    stun_request[2] = 0x00;
    stun_request[3] = 0x00;
     
    // 設置識別符（隨機生成，這裡簡化處理）
    uint32_t transaction_id = rand(); // 隨機生成一個識別符
    memcpy(&stun_request[8], &transaction_id, sizeof(transaction_id)); // 將識別符放入請求中

    // 設置請求長度
    request_length = STUN_REQUEST_SIZE; 
}

//CURL獲取公網IP,並返回IP OK ★★★★★ 2024-12-29
std::string NatHeartBean::get_public_ip_by_curl() {
    // 使用 curl 命令獲取公共 IP
    system("curl -s http://ifconfig.me > public_ip_by_curl.txt");

    // 讀取 ip.txt 文件中的內容
    FILE* file = fopen("public_ip_by_curl.txt", "r");
    if (!file) {
        std::cerr << "Failed to open ip.txt" << std::endl;
        return "127.0.0.1";
    }

    char ip[16]; // IPv4 地址最大長度
    if (fgets(ip, sizeof(ip), file) != nullptr) {
        fclose(file);
        return std::string(ip);
    }

    fclose(file);
    return "127.0.0.1"; //默認失敗的值而不是 string.empty
}

// 通過內存獲取 curl客戶端命令結果 必須安裝curl(windows 內置的,Linux需要安裝package)
// curl 命令獲得公網IP OK  ★★★★★ 2024-12-29
std::string NatHeartBean::get_public_ip_by_curl_memory() {

    // 使用 curl 命令獲取公共 IP
    const char* command = "curl -s http://ifconfig.me";
     
    // 打開管道以執行命令
#ifdef WIN32
    int _pclose(FILE * stream);
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command, "r"), _pclose);
#endif

#ifdef _linux_
    int pclose(FILE * stream);
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
#endif
    

    if (!pipe) {
        throw std::runtime_error("_popen() failed!");
    }

    char buffer[128];
    std::string result;

    // 讀取命令的輸出
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }
    
    return result; 
}
```


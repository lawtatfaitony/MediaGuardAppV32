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
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints)); 
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    struct addrinfo* res;
    int status = getaddrinfo("example.com", nullptr, &hints, &res);
    // 解析主機名
    if (status != 0) {
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
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(command, "r"), _pclose);
#else
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
#endif

    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    char buffer[128];
    std::string result;

    // 讀取命令的輸出
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        result += buffer;
    }

    return result;
}


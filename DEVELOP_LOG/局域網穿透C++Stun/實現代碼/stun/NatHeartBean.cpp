#include "NatHeartBean.h"

NatHeartBean::~NatHeartBean()
{
    // 析構函數
}

NatHeartBean::NatHeartBean()
{
    // 構造函數
} 
// ...

int NatHeartBean::get_stun_ip() {

    // 創建 STUN 請求
    create_stun_request();

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket." << std::endl;
        return 1;
    }

    struct addrinfo hints, * res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP

    // 解析主機名
    if (getaddrinfo(STUN_SERVER_IP, nullptr, &hints, &res) != 0) {
        std::cerr << "Failed to resolve hostname." << std::endl;
        return 1;
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
    inet_pton(AF_INET, STUN_SERVER_IP, &server_addr.sin_addr);
     
    // 發送請求 
    sendto(sockfd, reinterpret_cast<const char*>(stun_request), request_length,  0, (struct sockaddr*)&server_addr, sizeof(server_addr));
     
    // 接收響應
    char buffer[1024];
    socklen_t addr_len = sizeof(server_addr);

    struct timeval timeout;
    timeout.tv_sec = 5; // 超時時間，單位為秒
    timeout.tv_usec = 0; // 微秒
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)); 
    size_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &addr_len);

    if (recv_len < 0) {
        perror("recvfrom failed");
        // 處理錯誤
    }

    // 假設 buffer 是接收到的響應
    if (recv_len > 0) {

        // 假設響應中包含了客戶端的 IP 和端口
        char client_ip[INET_ADDRSTRLEN]; 
        unsigned short client_port;

        // 確保接收到的數據足夠長
        if (recv_len >= 20) { // STUN 消息的最小長度
            // 檢查消息類型和標誌
            uint16_t message_type = (buffer[0] << 8) | buffer[1];
            if (message_type == 0x0101) { // 0x0101 是成功響應的消息類型
                // 提取 Mapped Address
                uint16_t attribute_type = (buffer[28] << 8) | buffer[29]; // 假設 Mapped Address 在第 28 和 29 字節
                uint16_t address_length = (buffer[30] << 8) | buffer[31]; // 地址長度
                uint32_t client_ip_addr = *(uint32_t*)&buffer[32]; // IP 地址
                unsigned short client_port = ntohs(*(uint16_t*)&buffer[36]); // 端口

                inet_ntop(AF_INET, &client_ip_addr, client_ip, sizeof(client_ip));
                std::cout << "獲取的 IP: " << client_ip << std::endl;
                std::cout << "獲取的端口: " << client_port << std::endl;
            }
        }
    }
     
    return 0;
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

#include "NatHeartBean.h"

NatHeartBean::~NatHeartBean()
{
    // �R�c���
}

NatHeartBean::NatHeartBean()
{
    // �c�y���
} 
// ...

int NatHeartBean::get_stun_ip() {

    // �Ы� STUN �ШD
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

    // �ѪR�D���W
    if (getaddrinfo(STUN_SERVER_IP, nullptr, &hints, &res) != 0) {
        std::cerr << "Failed to resolve hostname." << std::endl;
        return 1;
    }

    // �N�ѪR���a�}�]�m�� server_addr ��
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(STUN_SERVER_PORT);
    server_addr.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
    // ����귽
    freeaddrinfo(res);
  
	// �N STUN ���A���� IP �M�ݤf�]�m�� server_addr ��
    inet_pton(AF_INET, STUN_SERVER_IP, &server_addr.sin_addr);
     
    // �o�e�ШD 
    sendto(sockfd, reinterpret_cast<const char*>(stun_request), request_length,  0, (struct sockaddr*)&server_addr, sizeof(server_addr));
     
    // �����T��
    char buffer[1024];
    socklen_t addr_len = sizeof(server_addr);

    struct timeval timeout;
    timeout.tv_sec = 5; // �W�ɮɶ��A��쬰��
    timeout.tv_usec = 0; // �L��
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)); 
    size_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &addr_len);

    if (recv_len < 0) {
        perror("recvfrom failed");
        // �B�z���~
    }

    // ���] buffer �O�����쪺�T��
    if (recv_len > 0) {

        // ���]�T�����]�t�F�Ȥ�ݪ� IP �M�ݤf
        char client_ip[INET_ADDRSTRLEN]; 
        unsigned short client_port;

        // �T�O�����쪺�ƾڨ�����
        if (recv_len >= 20) { // STUN �������̤p����
            // �ˬd���������M�лx
            uint16_t message_type = (buffer[0] << 8) | buffer[1];
            if (message_type == 0x0101) { // 0x0101 �O���\�T������������
                // ���� Mapped Address
                uint16_t attribute_type = (buffer[28] << 8) | buffer[29]; // ���] Mapped Address �b�� 28 �M 29 �r�`
                uint16_t address_length = (buffer[30] << 8) | buffer[31]; // �a�}����
                uint32_t client_ip_addr = *(uint32_t*)&buffer[32]; // IP �a�}
                unsigned short client_port = ntohs(*(uint16_t*)&buffer[36]); // �ݤf

                inet_ntop(AF_INET, &client_ip_addr, client_ip, sizeof(client_ip));
                std::cout << "����� IP: " << client_ip << std::endl;
                std::cout << "������ݤf: " << client_port << std::endl;
            }
        }
    }
     
    return 0;
}

// ...

void NatHeartBean::create_stun_request() {
    // �M�ŽШD
    memset(stun_request, 0, STUN_REQUEST_SIZE);

    // �]�m�ШD�����Y
    // stun_request �����c�]�m�p�U�G
    // stun_request[0]�G�����M����
    // stun_request[1]�G��k�X�]Binding Request�^
    // stun_request[2] �M stun_request[3]�G�������ס]���B�]�m�� 0�A�]���S���B�~�ݩʡ^
    // stun_request[8]�G�ѧO�Ū��}�l��m
	//--------------------------------------------------------------------------------
     // �]�m�ШD�����Y
    stun_request[0] = 0x00; // �����M����
    stun_request[1] = 0x01; // ��k�X�]Binding Request�^

    // �]�m�������ס]�o�̬� 0�A�]���S���B�~���ݩʡ^
    stun_request[2] = 0x00;
    stun_request[3] = 0x00;
     
    // �]�m�ѧO�š]�H���ͦ��A�o��²�ƳB�z�^
    uint32_t transaction_id = rand(); // �H���ͦ��@���ѧO��
    memcpy(&stun_request[8], &transaction_id, sizeof(transaction_id)); // �N�ѧO�ũ�J�ШD��

    // �]�m�ШD����
    request_length = STUN_REQUEST_SIZE; 
}

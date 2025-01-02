/**************************************************************
* @brief:       HMAC UTILITY
* @desc         implementation of HMAC SHA1 algorithm
* @date:        2020-10-22
* @update:
* @auth:        Tony Law
* @copyright:
*
***************************************************************/
#include <cstdio>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include "hmac_sha1.h"

using namespace std;

std::string binToHex(const unsigned char *data, size_t size)
{
    std::ostringstream strHex;
    strHex << std::hex << std::setfill('0');
    for (size_t i = 0; i < size; ++i)
    {
        strHex << std::setw(2) << static_cast<unsigned int>(data[i]);
    }
    return strHex.str();
}

std::vector<unsigned char> hexToBin(const std::string &hex)
{
    std::vector<unsigned char> dest;
    auto len = hex.size();
    dest.reserve(len / 2);
    for (decltype(len) i = 0; i < len; i += 2)
    {
        unsigned int element;
        std::istringstream strHex(hex.substr(i, 2));
        strHex >> std::hex >> element;
        dest.push_back(static_cast<unsigned char>(element));
    }
    return dest;
}

int main12312()
{
	//result =  fe593abac32bf6d9d98d2276cca39c2c647ab807  //fe593abac32bf6d9d98d2276cca39c2c647ab807

	//I:  test OK
    std::string t = "DeviceSerialNo123456789";
    std::string k = "202010221040"; //
    BYTE *test = (BYTE *)t.c_str();
    BYTE *Key = (BYTE *)k.c_str();
    BYTE digest[20];
    CHMAC_SHA1 chs;
    chs.HMAC_SHA1(test, strlen(t.c_str()), Key, strlen(k.c_str()), digest);

    std::string resul = binToHex(digest, 20);
    std::cout << resul << std::endl;

	//II:  test OK
	string a = chs.hmac_sha1(t, k);
	std::cout << a << std::endl;

    // char buf[20] = {0};
    // sprintf(buf, "%02x%02x%02x%02x%02x", digest[0], digest[1], digest[2], digest[3], digest[4]);
    // printf("十六进制:\t%s\n", buf);
    printf("\n");

    // const char *strTest = "CED2CAC7B8F6B4F3D0DCC3A8";
    // auto charVec = hexToBin(strTest);
    // for (auto element : charVec)
    // {
    //     std::cout << element;
    // }
    // std::cout << std::endl;
    // auto strHex = binToHex(charVec.data(), charVec.size());
    // std::cout << strHex << std::endl;
    
    return 0;
}

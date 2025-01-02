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
#include <memory>
#include "hmac_sha1.h"

void CHMAC_SHA1::HMAC_SHA1(BYTE *text, int text_len, BYTE *key, int key_len, BYTE *digest)
{
	memset(SHA1_Key, 0, SHA1_BLOCK_SIZE);

	/* repeated 64 times for values in ipad and opad */
	memset(m_ipad, 0x36, sizeof(m_ipad));
	memset(m_opad, 0x5c, sizeof(m_opad));

	/* STEP 1 */
	if (key_len > SHA1_BLOCK_SIZE)
	{
		CSHA1::Reset();
		CSHA1::Update((UINT_8 *)key, key_len);
		CSHA1::Final();

		CSHA1::GetHash((UINT_8 *)SHA1_Key);
	}
	else
		memcpy(SHA1_Key, key, key_len);

	/* STEP 2 */
	for (int i=0; i<sizeof(m_ipad); i++)
	{
		m_ipad[i] ^= SHA1_Key[i];		
	}

	/* STEP 3 */
	memcpy(AppendBuf1, m_ipad, sizeof(m_ipad));
	memcpy(AppendBuf1 + sizeof(m_ipad), text, text_len);

	/* STEP 4 */
	CSHA1::Reset();
	CSHA1::Update((UINT_8 *)AppendBuf1, sizeof(m_ipad) + text_len);
	CSHA1::Final();

	CSHA1::GetHash((UINT_8 *)szReport);

	/* STEP 5 */
	for (int j=0; j<sizeof(m_opad); j++)
	{
		m_opad[j] ^= SHA1_Key[j];
	}

	/* STEP 6 */
	memcpy(AppendBuf2, m_opad, sizeof(m_opad));
	memcpy(AppendBuf2 + sizeof(m_opad), szReport, SHA1_DIGEST_LENGTH);

	/*STEP 7 */
	CSHA1::Reset();
	CSHA1::Update((UINT_8 *)AppendBuf2, sizeof(m_opad) + SHA1_DIGEST_LENGTH);
	CSHA1::Final();

	CSHA1::GetHash((UINT_8 *)digest);
}

std::string CHMAC_SHA1::binToHex(const unsigned char *data, size_t size)
{
	std::ostringstream strHex;
	strHex << std::hex << std::setfill('0');
	for (size_t i = 0; i < size; ++i)
	{
		strHex << std::setw(2) << static_cast<unsigned int>(data[i]);
	}
	return strHex.str();
}

std::vector<unsigned char> CHMAC_SHA1::hexToBin(const std::string &hex)
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

std::string CHMAC_SHA1::hmac_sha1(std::string& plain_text, std::string& key)
{ 
	BYTE *byte_plain_text = (BYTE *)plain_text.c_str();
	BYTE *Key = (BYTE *)key.c_str();
	BYTE digest[20];
	CHMAC_SHA1 chs;
	chs.HMAC_SHA1(byte_plain_text, strlen(plain_text.c_str()), Key, strlen(key.c_str()), digest); 
	std::string result = chs.binToHex(digest, 20);

	return result;
}

std::string CHMAC_SHA1::GetCurrentSystemTimeLong()
{
	time_t ltime = time(NULL);
	std::tm ltm = { 0 };
#ifdef _WIN32
	localtime_s(&ltm, &ltime);
#elif __linux__
	// 请自己补linux的函数

#endif
	char buffer[128] = { 0 };
	//strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", &ltm); 
	//Exact match to minute
	strftime(buffer, sizeof(buffer), "%Y%m%d%H%M", &ltm); //  Exact match to minute
	return buffer;
}

std::string CHMAC_SHA1::GetCurrentYear()
{
	time_t ltime = time(NULL);
	std::tm ltm = { 0 };

#ifdef _WIN32
	localtime_s(&ltm, &ltime);
#elif __linux__
	// 请自己补linux的函数
#endif

	char buffer[128] = { 0 }; 
	strftime(buffer, sizeof(buffer), "%Y", &ltm);
	return buffer;
}

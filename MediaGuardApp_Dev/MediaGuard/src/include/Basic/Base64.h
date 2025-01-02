#pragma once
#include <string>
#include "Basic.h"


NAMESPACE_BASIC_BEGIN
static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
class Base64
{
private:
	static inline bool is_base64(unsigned char c)
	{
		return (isalnum(c) || (c == '+') || (c == '/'));
	}

public:
	static std::string Encode(unsigned char const* pSrc, unsigned int nLen)
	{
		std::string ret;
		int i = 0;
		int j = 0;
		unsigned char arrSrcTmp[3];
		unsigned char arrRstTmp[4];

		while (nLen--)
		{
			arrSrcTmp[i++] = *(pSrc++);
			if (3 == i)
			{
				arrRstTmp[0] = (arrSrcTmp[0] & 0xfc) >> 2;
				arrRstTmp[1] = ((arrSrcTmp[0] & 0x03) << 4) + ((arrSrcTmp[1] & 0xf0) >> 4);
				arrRstTmp[2] = ((arrSrcTmp[1] & 0x0f) << 2) + ((arrSrcTmp[2] & 0xc0) >> 6);
				arrRstTmp[3] = arrSrcTmp[2] & 0x3f;

				for (i = 0; (i < 4); i++)
					ret += base64_chars[arrRstTmp[i]];
				i = 0;
			}
		}

		if (i)
		{
			for (j = i; j < 3; j++)
				arrSrcTmp[j] = '\0';

			arrRstTmp[0] = (arrSrcTmp[0] & 0xfc) >> 2;
			arrRstTmp[1] = ((arrSrcTmp[0] & 0x03) << 4) + ((arrSrcTmp[1] & 0xf0) >> 4);
			arrRstTmp[2] = ((arrSrcTmp[1] & 0x0f) << 2) + ((arrSrcTmp[2] & 0xc0) >> 6);

			for (j = 0; (j < i + 1); j++)
				ret += base64_chars[arrRstTmp[j]];

			while ((i++ < 3))
				ret += '=';
		}
		return ret;
	}

	static std::string Decode(std::string const& encoded_string)
	{
		int in_len = encoded_string.size();
		int i = 0;
		int j = 0;
		int in_ = 0;
		unsigned char arrSrcTmp[4], srrRstTmp[3];
		std::string ret;

		while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
		{
			arrSrcTmp[i++] = encoded_string[in_]; in_++;
			if (4 == i)
			{
				for (i = 0; i < 4; i++)
					arrSrcTmp[i] = base64_chars.find(arrSrcTmp[i]);

				srrRstTmp[0] = (arrSrcTmp[0] << 2) + ((arrSrcTmp[1] & 0x30) >> 4);
				srrRstTmp[1] = ((arrSrcTmp[1] & 0xf) << 4) + ((arrSrcTmp[2] & 0x3c) >> 2);
				srrRstTmp[2] = ((arrSrcTmp[2] & 0x3) << 6) + arrSrcTmp[3];

				for (i = 0; (i < 3); i++)
					ret += srrRstTmp[i];
				i = 0;
			}
		}

		if (i)
		{
			for (j = 0; j < i; j++)
				arrSrcTmp[j] = base64_chars.find(arrSrcTmp[j]);

			srrRstTmp[0] = (arrSrcTmp[0] << 2) + ((arrSrcTmp[1] & 0x30) >> 4);
			srrRstTmp[1] = ((arrSrcTmp[1] & 0xf) << 4) + ((arrSrcTmp[2] & 0x3c) >> 2);

			for (j = 0; (j < i - 1); j++) ret += srrRstTmp[j];
		}

		return ret;
	}
};
NAMESPACE_BASIC_END

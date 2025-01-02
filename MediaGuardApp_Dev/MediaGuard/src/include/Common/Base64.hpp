#ifndef _BASE64_HPP_
#define _BASE64_HPP_

#include <string>
#define char64(c)  ((c > 127) ? (char) 0xff : index_64[(c)])

class CBase64
{
public:
	// base64解密
	inline static unsigned int Encode(unsigned char* pData, unsigned int dataSize, std::string* base64)
	{
		const char base_64[128] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		signed int	padding;
		unsigned int	i = 0, j = 0;

		unsigned char* in;
		unsigned int	outSize;
		unsigned char* out;

		unsigned long iOut = ((dataSize + 2) / 3) * 4;
		outSize = iOut += 2 * ((iOut / 60) + 1);
		out = new unsigned char[outSize];

		in = pData;
		if (outSize < (dataSize * 4 / 3)) return 0;

		while (i < dataSize) {
			padding = 3 - (dataSize - i);
			if (padding == 2) {
				out[j] = base_64[in[i] >> 2];
				out[j + 1] = base_64[(in[i] & 0x03) << 4];
				out[j + 2] = '=';
				out[j + 3] = '=';
			}
			else if (padding == 1) {
				out[j] = base_64[in[i] >> 2];
				out[j + 1] = base_64[((in[i] & 0x03) << 4) | ((in[i + 1] & 0xf0) >> 4)];
				out[j + 2] = base_64[(in[i + 1] & 0x0f) << 2];
				out[j + 3] = '=';
			}
			else {
				out[j] = base_64[in[i] >> 2];
				out[j + 1] = base_64[((in[i] & 0x03) << 4) | ((in[i + 1] & 0xf0) >> 4)];
				out[j + 2] = base_64[((in[i + 1] & 0x0f) << 2) | ((in[i + 2] & 0xc0) >> 6)];
				out[j + 3] = base_64[in[i + 2] & 0x3f];
			}
			i += 3;
			j += 4;
		}
		out[j] = '\0';
		*base64 = (char*)out;
		delete[] out;
		return j;
	}

	// base64加密
	inline static unsigned int Decode(const std::string& base64, unsigned char* pBuffer, unsigned int bufferSize)
	{
		const unsigned char index_64[128] = {
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,   62, 0xff, 0xff, 0xff,   63,
			52,		53,   54,   55,   56,   57,   58,   59,   60,   61, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff,    0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,
			15,		16,   17,   18,   19,   20,   21,   22,   23,   24,   25, 0xff, 0xff, 0xff, 0xff, 0xff,
			0xff,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40,
			41,		42,   43,   44,   45,   46,   47,   48,   49,   50,   51, 0xff, 0xff, 0xff, 0xff, 0xff
		};

		unsigned int	i = 0, j = 0, padding;
		unsigned char	c[4], in[4];

		unsigned int	inSize = (unsigned int)base64.length();
		char* in_buf = _strdup(base64.c_str());
		unsigned char* out = pBuffer;

		if (bufferSize < (inSize * 3 / 4)) return 0;

		while ((i + 3) < inSize) {
			padding = 0;
			i = load_block(in_buf, inSize, i, (char*)in);
			c[0] = char64(in[0]);
			padding += (c[0] == 0xff);
			c[1] = char64(in[1]);
			padding += (c[1] == 0xff);
			c[2] = char64(in[2]);
			padding += (c[2] == 0xff);
			c[3] = char64(in[3]);
			padding += (c[3] == 0xff);
			if (padding == 2) {
				out[j++] = (c[0] << 2) | ((c[1] & 0x30) >> 4);
				out[j] = (c[1] & 0x0f) << 4;
			}
			else if (padding == 1) {
				out[j++] = (c[0] << 2) | ((c[1] & 0x30) >> 4);
				out[j++] = ((c[1] & 0x0f) << 4) | ((c[2] & 0x3c) >> 2);
				out[j] = (c[2] & 0x03) << 6;
			}
			else {
				out[j++] = (c[0] << 2) | ((c[1] & 0x30) >> 4);
				out[j++] = ((c[1] & 0x0f) << 4) | ((c[2] & 0x3c) >> 2);
				out[j++] = ((c[2] & 0x03) << 6) | (c[3] & 0x3f);
			}
			//i += 4;
		}
		free(in_buf);
		return j;
	}


private:
	// 载入数据库
	inline static unsigned int load_block(char* in, unsigned int size, unsigned int pos, char* out)
	{
		unsigned int i, len;
		unsigned int c;
		len = i = 0;
		while ((len < 4) && ((pos + i) < size)) {
			c = in[pos + i];
			if (((c >= 'A') && (c <= 'Z'))
				|| ((c >= 'a') && (c <= 'z'))
				|| ((c >= '0') && (c <= '9'))
				|| (c == '=') || (c == '+') || (c == '/')
				) {
				out[len] = c;
				len++;
			}
			i++;
		}
		while (len < 4) { out[len] = (char)0xFF; len++; }
		return pos + i;
	}
};

#endif

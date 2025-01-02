/*******************************************************************************
文件名称 : TransCoding.cpp
作    者 : 周水平(ZhouShuiping)
创建时间 : 10/14/2013
文件描述 : 字符编码处理相关类
版权声明 : Copyright (C) Public Limited 2013-2015 All Rights Reserved
修改历史 : 周水平 10/14/2013 1.00 初始版本
           周水平 05/30/2014 1.01 增加Trim函数和Replace函数以及对linux平台的支持
*******************************************************************************/
//#include "stdafx.h"
#include "TransCoding.h"

#ifdef _WIN32
#pragma warning(disable:4996)

std::wstring transcoding::Ansi2Unicode(const CHAR *lpszSrc, __in UINT unCodePage/* = CP_ACP*/)
{
	if (NULL == lpszSrc || '\0' == *lpszSrc) return L"";

#if defined(_WINDOWS) || defined(WIN32)
	// 预转换, 得到所需空间的大小
	int nBufferSize = ::MultiByteToWideChar(unCodePage, NULL, lpszSrc, strlen(lpszSrc), NULL, 0);

	// 声明临时保存变量
	WCHAR *lpwszDest = new WCHAR[nBufferSize + 1];
	if (NULL == lpwszDest) return L"";

	// 转换
	::MultiByteToWideChar(unCodePage, NULL, lpszSrc, strlen(lpszSrc), lpwszDest, nBufferSize);

	// 最后加上'\0'
	lpwszDest[nBufferSize] = L'\0';

	std::wstring wsTemp(lpwszDest);
	delete []lpwszDest;
	lpwszDest = NULL;

	return wsTemp;
#else

    return L"";
#endif
}

tstring transcoding::Ansi2T(const CHAR *lpszSrc, __in UINT unCodePage/* = CP_ACP*/)
{
    if (NULL == lpszSrc || '\0' == *lpszSrc) return _T("");

	tstring strDest ;
#ifdef _UNICODE
	strDest = Ansi2Unicode(lpszSrc,unCodePage);
#else
	strDest = lpszSrc;
#endif
	return strDest;
}

tstring transcoding::UTF82T(const CHAR *lpszSrc, BOOL b2Ansi /*= FALSE*/)
{
    if (NULL == lpszSrc || '\0' == *lpszSrc) return _T("");

	tstring strDest ;
#ifdef _UNICODE
	strDest = UTF82Unicode(lpszSrc);
#else
    wstring wText = UTF82Unicode(lpszSrc);
    strDest = Unicode2Ansi(wText.c_str());
#endif
	return strDest;	
}

std::string transcoding::Unicode2Ansi(const WCHAR *lpwszSrc, __in UINT unCodePage/* = CP_ACP*/)
{
	if (NULL == lpwszSrc || L'\0' == *lpwszSrc) return "";

#if defined(_WINDOWS) || defined(WIN32)
	// 预转换, 得到所需空间的大小
	int nBufferSize = ::WideCharToMultiByte(unCodePage, NULL, lpwszSrc, wcslen(lpwszSrc), NULL, 0, NULL, NULL);

	// 声明临时保存变量
	CHAR *lpszDest = new CHAR[nBufferSize + 1];
	if (NULL == lpszDest) return "";

	// 转换
	::WideCharToMultiByte(unCodePage, NULL, lpwszSrc, wcslen(lpwszSrc), lpszDest, nBufferSize, NULL, NULL);

	// 最后加上'\0'
	lpszDest[nBufferSize] = '\0';

	std::string sTemp(lpszDest);
	delete []lpszDest;
	lpszDest = NULL;

	return sTemp;
#else

	return "";
#endif
}

std::string transcoding::T2Ansi(const TCHAR *lpwszSrc, __in UINT unCodePage/* = CP_ACP*/)
{
    if (NULL == lpwszSrc || _T('\0') == *lpwszSrc) return "";

	std::string strDes;
#ifdef _UNICODE
	strDes = Unicode2Ansi(lpwszSrc, unCodePage);
#else
	strDes = lpwszSrc;
#endif
	return strDes;
}

string transcoding::T2UTF8(const TCHAR *lpwszSrc, __in UINT unCodePage /*= CP_ACP*/)
{
	std::string strDes;
#ifdef _UNICODE
	strDes = Unicode2UTF8(lpwszSrc);
#else
	wstring wText = T2Unicode(lpwszSrc);
	strDes = Unicode2UTF8(wText.c_str());
#endif
	return strDes;
}

tstring transcoding::Unicode2T(const WCHAR *lpwszSrc,__in UINT unCodePage/* = CP_ACP*/)
{
    if (NULL == lpwszSrc || L'\0' == *lpwszSrc) return _T("");

	tstring strDest ;
#ifdef _UNICODE
	strDest = lpwszSrc;
#else
	strDest = Unicode2Ansi(lpwszSrc,unCodePage);
#endif
	return strDest;
}

std::wstring transcoding::T2Unicode(const TCHAR *lpwszSrc, __in UINT unCodePage/* = CP_ACP*/)
{
    if (NULL == lpwszSrc || _T('\0') == *lpwszSrc) return L"";

	std::wstring strDes;
#ifdef _UNICODE
	strDes = lpwszSrc;
#else
	strDes = Ansi2Unicode(lpwszSrc, unCodePage);
#endif
	return strDes;
}

std::wstring transcoding::UTF82Unicode(const CHAR *lpszSrc)
{
    return Ansi2Unicode(lpszSrc, CP_UTF8);
}

std::string transcoding::Unicode2UTF8(const WCHAR *lpwszSrc)
{
    return Unicode2Ansi(lpwszSrc, CP_UTF8);
}

BOOL transcoding::IsBufferUTF8(const void* lpvdBuffer, DWORD dwSize)
{
	if (NULL == lpvdBuffer) return FALSE;
	if (dwSize <= 1) return FALSE;

	BOOL bIsUTF8 = TRUE;
	//unsigned char* lpbtStart = (unsigned char*)pBuffer;
	//unsigned char* lpbtEnd = (unsigned char*)pBuffer + size;
	BYTE *lpbtStart = (BYTE *)lpvdBuffer;
	BYTE *lpbtEnd = (BYTE *)lpvdBuffer + dwSize;
	while (lpbtStart < lpbtEnd)
	{
		if (*lpbtStart < 0x80) // (10000000): 值小于0x80的为ASCII字符
		{
			lpbtStart++;
		}
		else if (*lpbtStart < (0xC0)) // (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符
		{
			bIsUTF8 = FALSE;
			break;
		}
		else if (*lpbtStart < (0xE0)) // (11100000): 此范围内为2字节UTF-8字符
		{
			if (lpbtStart >= lpbtEnd - 1)
				break;
			if ((lpbtStart[1] & (0xC0)) != 0x80)
			{
				bIsUTF8 = FALSE;
				break;
			}
			lpbtStart += 2;
		}
		else if (*lpbtStart < (0xF0)) // (11110000): 此范围内为3字节UTF-8字符
		{
			if (lpbtStart >= lpbtEnd - 2)
				break;
			if ((lpbtStart[1] & (0xC0)) != 0x80 || (lpbtStart[2] & (0xC0)) != 0x80)
			{
				bIsUTF8 = FALSE;
				break;
			}
			lpbtStart += 3;
		}
		else
		{
			bIsUTF8 = FALSE;
			break;
		}
	}

	return bIsUTF8;
}

BOOL transcoding::IsCharAnsi(const char c)
{
	return (c >= 0 && c <= 127);
}

vector<tstring> transcoding::SpliteString(LPCTSTR lpctSrc, LPCTSTR lpctSeparator)
{
    vector<tstring> vecTemp;
    TCHAR *lptSrc = new TCHAR[(_tcslen(lpctSrc) + 1) * sizeof(TCHAR)];
    _tcscpy_s(lptSrc, (_tcslen(lpctSrc) + 1) * sizeof(TCHAR), lpctSrc);

    LPTSTR lptStart = lptSrc;
    LPTSTR lptEnd = _tcsstr(lptStart, lpctSeparator);
    while (lptEnd - lptStart == 0)
    {
        vecTemp.push_back(_T(""));
        if (_T('\0') == (lptStart += _tcslen(lpctSeparator)))
            break;
        lptEnd = _tcsstr(lptStart, lpctSeparator);
    }
    while (lptEnd - lptSrc > 0 && lptEnd != NULL)
    {
        *lptEnd = _T('\0');
        vecTemp.push_back(lptStart);
        lptEnd += _tcslen(lpctSeparator);
        if (_T('\0') == *lptEnd)
            break;
        lptStart = lptEnd;
        lptEnd = _tcsstr(lptStart, lpctSeparator);
        while (lptEnd - lptStart == 0)
        {
            vecTemp.push_back(_T(""));
            if (_T('\0') == (lptStart += _tcslen(lpctSeparator)))
                break;
            lptEnd = _tcsstr(lptStart, lpctSeparator);
        }
        if (NULL == lptEnd)
        {
            vecTemp.push_back(lptStart);
            break;
        }
    }

    return vecTemp;
}

LPTSTR transcoding::TrimLeft(LPTSTR lptBuf, int &nCount)
{
	while (_T(' ') == lptBuf[0])
	{
		for (int i = 0; i < nCount - 1; i++)
			lptBuf[i] = lptBuf[i + 1];
		nCount--;
	}
	lptBuf[nCount] = _T('\0');
	return lptBuf;
}

LPTSTR transcoding::TrimRight(LPTSTR lptBuf, int &nCount)
{
	while (nCount > 0 && _T(' ') == lptBuf[nCount - 1])
		lptBuf[nCount-- - 1] = _T('\0');
	return lptBuf;
}

LPTSTR transcoding::Trim(LPTSTR lptBuf, int &nCount)
{
    lptBuf = TrimLeft(lptBuf, nCount);
    lptBuf = TrimRight(lptBuf, nCount);
    return lptBuf;
}


LPSTR transcoding::ReplaceCharA(LPSTR lpBuf, size_t stCount, const CHAR &cSrc, const CHAR &cDst)
{
    for (size_t i = 0; i < stCount; i++)
    {
        if (lpBuf[i] == cSrc)
            lpBuf[i] = cDst;
    }
    return lpBuf;
}

LPWSTR transcoding::ReplaceCharW(LPWSTR lpwBuf, size_t stCount, const WCHAR &wcSrc, const WCHAR &wcDst)
{
    for (size_t i = 0; i < stCount; i++)
    {
        if (lpwBuf[i] == wcSrc)
            lpwBuf[i] = wcDst;
    }
    return lpwBuf;
}

LPTSTR transcoding::ReplaceChar(LPTSTR lptBuf, size_t stCount, const TCHAR &tcSrc, const TCHAR &tcDst)
{
#if defined(UNICODE) || defined(_UNICODE)
    return ReplaceCharW(lptBuf, stCount, tcSrc, tcDst);
#else
    return ReplaceCharA(lptBuf, stCount, tcSrc, tcDst);
#endif
}

LPSTR transcoding::ReplaceA(LPSTR lpBuf, size_t &stCount, LPCSTR lpcSrc, LPCSTR lpcDst)
{
    LPSTR lpTemp = NULL;
    while ((lpTemp = ::strstr(lpBuf, lpcSrc)) != NULL)
    {
        if (strlen(lpcSrc) <= strlen(lpcDst))
        {
            int nDiff = strlen(lpcDst) - strlen(lpcSrc);
            if (nDiff > 0)
                ::memmove(lpTemp + nDiff, lpTemp, stCount - (lpTemp - lpBuf));
            ::memmove(lpTemp, lpcDst, strlen(lpcDst));
            stCount += nDiff;
        }
        else
        {
            int nDiff = strlen(lpcSrc) - strlen(lpcDst);
            if ('\0' == *(lpTemp + strlen(lpcSrc)))
                *(lpTemp + strlen(lpcDst)) = '\0';
            else
                ::memmove(lpTemp + strlen(lpcDst), lpTemp + strlen(lpcSrc), stCount - (lpTemp + strlen(lpcSrc) - lpBuf));
            ::memmove(lpTemp, lpcDst, strlen(lpcDst));
            stCount -= nDiff;
        }
    }
    lpBuf[stCount] = _T('\0');
    return lpBuf;
}

LPWSTR transcoding::ReplaceW(LPWSTR lpwBuf, size_t &stCount, LPCWSTR lpcwSrc, LPCWSTR lpcwDst)
{
    LPWSTR lpwTemp = NULL;
    while ((lpwTemp = ::wcsstr(lpwBuf, lpcwSrc)) != NULL)
    {
        if (wcslen(lpcwSrc) <= wcslen(lpcwDst))
        {
            int nDiff = wcslen(lpcwDst) - wcslen(lpcwSrc);
            if (nDiff > 0)
                ::memmove(lpwTemp + nDiff, lpwTemp, stCount - (lpwTemp - lpwBuf));
            ::memmove(lpwTemp, lpcwDst, wcslen(lpcwDst));
            stCount += nDiff;
        }
        else
        {
            int nDiff = wcslen(lpcwSrc) - wcslen(lpcwDst);
            if (L'\0' == *(lpwTemp + wcslen(lpcwSrc)))
                *(lpwTemp + wcslen(lpcwDst)) = L'\0';
            else
                ::memmove(lpwTemp + wcslen(lpcwDst), lpwTemp + wcslen(lpcwSrc), stCount - (lpwTemp + wcslen(lpcwSrc) - lpwBuf));
            ::memmove(lpwTemp, lpcwDst, wcslen(lpcwDst));
            stCount -= nDiff;
        }
    }
    lpwBuf[stCount] = _T('\0');
    return lpwBuf;
}

LPTSTR transcoding::Replace(LPTSTR lptBuf, size_t &stCount, LPCTSTR lpctSrc, LPCTSTR lpctDst)
{
#if defined(UNICODE) || defined(_UNICODE)
    return ReplaceW(lptBuf, stCount, lpctSrc, lpctDst);
#else
    return ReplaceA(lptBuf, stCount, lpctSrc, lpctDst);
#endif
}

LPSTR WINAPI transcoding::CopyHelperA(LPSTR lpt, LPCSTR lpa)
{
	strcpy(lpt, lpa);
	return lpt;
}

LPCSTR WINAPI transcoding::CopyHelperCA(LPSTR lpt, LPCSTR lpa)
{
	strcpy(lpt, lpa);
	return (LPCSTR)lpt;
}

LPTSTR WINAPI transcoding::CopyHelperT(LPTSTR lpt1, LPCTSTR lpt2)
{
	_tcscpy(lpt1, lpt2);
	return lpt1;
}

LPCTSTR WINAPI transcoding::CopyHelperCT(LPTSTR lpt1, LPCTSTR lpt2)
{
	_tcscpy(lpt1, lpt2);
	return (LPCTSTR)lpt1;
}

LPWSTR WINAPI transcoding::CopyHelperW(LPWSTR lpw1, LPCWSTR lpw2)
{
	wcscpy(lpw1, lpw2);
	return lpw1;
}

LPCWSTR WINAPI transcoding::CopyHelperCW(LPWSTR lpw1, LPCWSTR lpw2)
{
	wcscpy(lpw1, lpw2);
	return (LPCWSTR)lpw1;
}

#elif __linux__

#endif
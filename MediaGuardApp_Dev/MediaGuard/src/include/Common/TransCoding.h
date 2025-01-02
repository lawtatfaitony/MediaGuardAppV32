#pragma once

#ifdef _WIN32 

#ifdef _WIN32 
#include <windows.h>
#include <tchar.h>
#endif

#include <vector>
#include <string>
using namespace std;

#ifdef _UNICODE
#define tstring wstring
#else
#define tstring string
#endif
#pragma warning(disable:4005)

class transcoding
{
public:
	transcoding(){};
	~transcoding(){};

public:
	static std::wstring Ansi2Unicode(const CHAR *lpszSrc, __in UINT unCodePage = CP_ACP);
	static string T2Ansi(const TCHAR *lpwszSrc, __in UINT unCodePage = CP_ACP);
	static string T2UTF8(const TCHAR *lpwszSrc, __in UINT unCodePage = CP_ACP);
	static tstring Ansi2T(const CHAR *lpszSrc, __in UINT unCodePage = CP_ACP);
	static tstring Unicode2T(const WCHAR *lpwszSrc,__in UINT unCodePage = CP_ACP);
	static std::wstring T2Unicode(const TCHAR *lpwszSrc, __in UINT unCodePage = CP_ACP);
	static string Unicode2Ansi(const WCHAR *lpwszSrc, __in UINT unCodePage = CP_ACP);
	static std::wstring UTF82Unicode(const CHAR *lpszSrc);
	static string Unicode2UTF8(const WCHAR *lpwszSrc);
	static tstring UTF82T(const CHAR *lpszSrc, BOOL b2Ansi = FALSE);
	static BOOL IsBufferUTF8(const void* lpvdBuffer, DWORD dwSize);
	static BOOL IsCharAnsi(const char c);
	static vector<tstring> SpliteString(LPCTSTR lpctSrc, LPCTSTR lpctSeparator);
	static LPTSTR Trim(LPTSTR lptBuf, int &nCount);
	static LPTSTR TrimLeft(LPTSTR lptBuf, int &nCount);
	static LPTSTR TrimRight(LPTSTR lptBuf, int &nCount);
	static LPSTR ReplaceCharA(LPSTR lptBuf, size_t stCount, const CHAR &cSrc, const CHAR &cDst);
	static LPWSTR ReplaceCharW(LPWSTR lptBuf, size_t stCount, const WCHAR &wcSrc, const WCHAR &wcDst);
	static LPTSTR ReplaceChar(LPTSTR lptBuf, size_t stCount, const TCHAR &tcSrc, const TCHAR &tcDst);
	static LPSTR ReplaceA(LPSTR lpBuf, size_t &stCount, LPCSTR lpcSrc, LPCSTR lpcDst);
	static LPWSTR ReplaceW(LPWSTR lpwBuf, size_t &stCount, LPCWSTR lpcwSrc, LPCWSTR lpcwDst);
	static LPTSTR Replace(LPTSTR lptBuf, size_t &stCount, LPCTSTR lpctSrc, LPCTSTR lpctDst);

    static string btoa(bool bValue)
    {
        return ((true == bValue) ? "true" : "false");
    }
    static string itoa(int nValue)
    {
        char szValue[32] = {0};
        _snprintf_s(szValue, _countof(szValue) - 1, "%d", nValue);
        return string(szValue);
    }
    static string ltoa(long lValue)
    {
        char szValue[32] = {0};
        _snprintf_s(szValue, _countof(szValue) - 1, "%ld", lValue);
        return string(szValue);
    }
    static string lltoa(LONGLONG llValue)
    {
        char szValue[32] = {0};
        _snprintf_s(szValue, _countof(szValue) - 1, "%I64d", llValue);
        return string(szValue);
    }
    static string ulltoa(LONGLONG ullValue)
    {
        char szValue[32] = {0};
        _snprintf_s(szValue, _countof(szValue) - 1, "%I64u", ullValue);
        return string(szValue);
    }
    static string ftoa(float fValue)
    {
        char szValue[32] = {0};
        _snprintf_s(szValue, _countof(szValue) - 1, "%f", fValue);
        return string(szValue);
    }
    static string dtoa(double dValue)
    {
        char szValue[32] = {0};
        _snprintf_s(szValue, _countof(szValue) - 1, "%.10f", dValue);
        return string(szValue);
    }

	static LPSTR WINAPI CopyHelperA(LPSTR lpt, LPCSTR lpa);
	static LPCSTR WINAPI CopyHelperCA(LPSTR lpt, LPCSTR lpa);
	static LPTSTR WINAPI CopyHelperT(LPTSTR lpt1, LPCTSTR lpt2);
	static LPCTSTR WINAPI CopyHelperCT(LPTSTR lpt1, LPCTSTR lpt2);
	static LPWSTR WINAPI CopyHelperW(LPWSTR lpw1, LPCWSTR lpw2);
	static LPCWSTR WINAPI CopyHelperCW(LPWSTR lpw1, LPCWSTR lpw2);
};

// #ifdef USES_CONVERSION
// #undef USES_CONVERSION
// #endif
// #define USES_CONVERSION ;
// #define T2A(X) transcoding::CopyHelperA((LPSTR)alloca((transcoding::T2Ansi(X).length()+5)*sizeof(char)),transcoding::T2Ansi(X).c_str())
// #define T2CA(X) transcoding::CopyHelperCA((LPSTR)alloca((transcoding::T2Ansi(X).length()+5)*sizeof(char)),transcoding::T2Ansi(X).c_str())
// #define A2T(X) transcoding::CopyHelperT((LPTSTR)alloca((transcoding::Ansi2T(X).length()+5)*sizeof(TCHAR)),transcoding::Ansi2T(X).c_str())
// #define A2CT(X) transcoding::CopyHelperCT((LPTSTR)alloca((transcoding::Ansi2T(X).length()+5)*sizeof(TCHAR)),transcoding::Ansi2T(X).c_str())
// #define T2W(X)  transcoding::CopyHelperW((LPWSTR)alloca((transcoding::T2Unicode(X).length()+5)*sizeof(WCHAR)),transcoding::T2Unicode(X).c_str())
// #define T2CW(X) transcoding::CopyHelperCW((LPWSTR)alloca((transcoding::T2Unicode(X).length()+5)*sizeof(WCHAR)),transcoding::T2Unicode(X).c_str())

#elif __linux__
// class transcoding
// {
// public:
// 	transcoding(){};
// 	~transcoding(){};

// public:
// 	static std::wstring Ansi2Unicode(const char *lpszSrc, __in UINT unCodePage = CP_ACP) {};
// 	static string T2Ansi(const char *lpwszSrc, __in UINT unCodePage = CP_ACP);
// 	static string T2UTF8(const char *lpwszSrc, __in UINT unCodePage = CP_ACP);
// 	static tstring Ansi2T(const char *lpszSrc, __in UINT unCodePage = CP_ACP);
// 	static tstring Unicode2T(const char *lpwszSrc,__in UINT unCodePage = CP_ACP);
// 	static std::wstring T2Unicode(const char *lpwszSrc, __in UINT unCodePage = CP_ACP);
// 	static string Unicode2Ansi(const char *lpwszSrc, __in UINT unCodePage = CP_ACP);
// 	static std::wstring UTF82Unicode(const char *lpszSrc);
// 	static string Unicode2UTF8(const char *lpwszSrc);
// 	static tstring UTF82T(const char *lpszSrc, BOOL b2Ansi = FALSE);
// 	static BOOL IsBufferUTF8(const void* lpvdBuffer, DWORD dwSize);
// 	static BOOL IsCharAnsi(const char c);
// 	static vector<tstring> SpliteString(LPCTSTR lpctSrc, LPCTSTR lpctSeparator);
// 	static LPTSTR Trim(LPTSTR lptBuf, int &nCount);
// 	static LPTSTR TrimLeft(LPTSTR lptBuf, int &nCount);
// 	static LPTSTR TrimRight(LPTSTR lptBuf, int &nCount);
// 	static LPSTR ReplaceCharA(LPSTR lptBuf, size_t stCount, const CHAR &cSrc, const CHAR &cDst);
// 	static LPWSTR ReplaceCharW(LPWSTR lptBuf, size_t stCount, const WCHAR &wcSrc, const WCHAR &wcDst);
// 	static LPTSTR ReplaceChar(LPTSTR lptBuf, size_t stCount, const TCHAR &tcSrc, const TCHAR &tcDst);
// 	static LPSTR ReplaceA(LPSTR lpBuf, size_t &stCount, LPCSTR lpcSrc, LPCSTR lpcDst);
// 	static LPWSTR ReplaceW(LPWSTR lpwBuf, size_t &stCount, LPCWSTR lpcwSrc, LPCWSTR lpcwDst);
// 	static LPTSTR Replace(LPTSTR lptBuf, size_t &stCount, LPCTSTR lpctSrc, LPCTSTR lpctDst);

//     static string btoa(bool bValue)
//     {
//         return ((true == bValue) ? "true" : "false");
//     }
//     static string itoa(int nValue)
//     {
//         char szValue[32] = {0};
//         _snprintf_s(szValue, _countof(szValue) - 1, "%d", nValue);
//         return string(szValue);
//     }
//     static string ltoa(long lValue)
//     {
//         char szValue[32] = {0};
//         _snprintf_s(szValue, _countof(szValue) - 1, "%ld", lValue);
//         return string(szValue);
//     }
//     static string lltoa(LONGLONG llValue)
//     {
//         char szValue[32] = {0};
//         _snprintf_s(szValue, _countof(szValue) - 1, "%I64d", llValue);
//         return string(szValue);
//     }
//     static string ulltoa(LONGLONG ullValue)
//     {
//         char szValue[32] = {0};
//         _snprintf_s(szValue, _countof(szValue) - 1, "%I64u", ullValue);
//         return string(szValue);
//     }
//     static string ftoa(float fValue)
//     {
//         char szValue[32] = {0};
//         _snprintf_s(szValue, _countof(szValue) - 1, "%f", fValue);
//         return string(szValue);
//     }
//     static string dtoa(double dValue)
//     {
//         char szValue[32] = {0};
//         _snprintf_s(szValue, _countof(szValue) - 1, "%.10f", dValue);
//         return string(szValue);
//     }
#endif
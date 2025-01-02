#pragma once
#include <string>
#include <mutex>
#include <thread>
#include <vector>
#include "curl/curl.h"

enum ContentType {
	kContentTypeNone = 0,
	kContentTypeTxt,
	kContentTypeHtml,
	kContentTypeXml,
	kContentTypeJson,
	kContentTypeUrlencoded,
	kContentTypeZip
};

struct HttpPara
{
	std::string strUrl;
	std::vector<std::string> vecHeaders;
	int nTransTimeout = 60000; //60 seconds 注意 上存下载必须设置合理时间
	int nConnectTimeout = 5000;  //请求响应的时间 5 seconds
	std::string strCertificate;
	std::string strUser;
	std::string strPassword;
	std::string Authorization;
};

struct FileNodeInfo
{
	int64_t nPart;  // part 编号
	int64_t nStartPos = 0;
	int64_t nEndPos = 0;
	FILE* fp;
	CURL* pCurl;
};

/*
Libcurl 相关应用帮助
ref:https://blog.csdn.net/weixin_46370795/article/details/104416053
*/
class LibcurlHelper
{
	static const int kSpeedInfo = 9;
	static const int kSecondsPerHour = 3600;
	static const int kSecondsPerMinute = 60;
	static const int kKByte = 1024;
	static const int kMByte = kKByte * kKByte;
	static const int64_t kGByte = kMByte * kKByte;
public:
	LibcurlHelper();
	~LibcurlHelper();

public:
	static int OnDebug(CURL*, curl_infotype itype, char* pData, size_t size, void*);
	static int64_t OnWriteString(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static int64_t OnWriteHeader(void* buffer, size_t size, size_t nmemb, void* lpVoid);
	static int64_t OnWriteFile(void* ptr, size_t size, size_t nMenmb, FILE* stream);
	static int64_t OnMultiWriteFile(void* ptr, size_t size, size_t nMenmb, void* pUserData);
	static int OnProcess(void* ptr, double nTotalToDownload, double nAlreadyDownload
		, double nTotalUploadSize, double nAlreadyUpload);

public:
	// 上传数据
	int Post(const HttpPara& paraHttp, const std::string& strData, std::string& strResponse);

	//上存含有Base64的Big Json
	int PostBase64Json(const HttpPara& paraHttp, const std::string& strData, std::string& strResponse);

	// with certificate
	int PostWithCer(const HttpPara& paraHttp, const std::string& strData, std::string& strResponse);

	// 上传文件
	int UploadFile(const HttpPara& paraHttp, const std::string& strFile, std::string& strResponse);
	// 上传表单数据
	int PostFormData(const HttpPara& paraHttp, const curl_httppost* pData, const std::string& strResponse);


	// 下载数据
	int Get(const HttpPara& paraHttp, std::string& strResponse);
	// 下载文件
	int DownloadFile(const HttpPara& paraHttp, const std::string& strFile = "");
	// 下载大文件
	int DownloadBigFile(const HttpPara& paraHttp, const std::string& strPath = "", const std::string& strFileName = "", int nThread = 1);

private:

	void get_header_type(int nType, std::string& strHeader);

	curl_slist* set_header(const std::vector<std::string>& vecHeaders);

	int64_t get_download_file_length(const HttpPara& paraHttp, std::string& strRemoteFileName);

	std::string generate_filename(const std::string& strUrl, const std::string& strCustonFile, const std::string& strFileInfo);
	void do_download(void* pData);

	void notify_download();

	long get_local_file_length(const std::string& strFile);

	std::string str_to_utf8(const std::string& str);


private:
	bool m_bDebug;
	int m_nTimeout;
	FILE* m_pFout;
	int64_t m_nPart;        // 分片记录
	std::mutex m_mtLockRecord;  // 线程数控制
	std::mutex m_mtWrite;       // 写数据控制 
};


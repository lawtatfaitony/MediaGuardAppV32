#include "LibcurlHelper.h" 
#include "Basic/RaiiHelper.h"
#include <fstream>
#ifdef _WIN32
#include <io.h>
#endif

Basic::RaiiHelper g_curlHandle
(
	[]() {curl_global_init(CURL_GLOBAL_ALL); },
	[]() {curl_global_cleanup(); }
);
int LibcurlHelper::OnDebug(CURL*, curl_infotype itype, char* pData, size_t size, void*)
{
#ifdef PRINT_INFO
	switch (itype)
	{
	case CURLINFO_TEXT:
		printf("[TEXT]%s\n", pData);
		break;
	case CURLINFO_HEADER_IN:
		printf("[HEADER_IN]%s\n", pData);
		break;
	case CURLINFO_HEADER_OUT:
		printf("[HEADER_OUT]%s\n", pData);
		break;
	case CURLINFO_DATA_IN:
		printf("[DATA_IN]%s\n", pData);
		break;
	case CURLINFO_DATA_OUT:
		printf("[DATA_OUT]%s\n", pData);
		break;
	default:
		printf("Undefined type");
		break;
	}
#endif // PRINT_INFO
	return 0;
}

int64_t LibcurlHelper::OnWriteString(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	std::string* str = dynamic_cast<std::string*>((std::string*)lpVoid);
	if (nullptr == str || nullptr == buffer) return -1;

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}

int64_t LibcurlHelper::OnWriteHeader(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	std::string* str = dynamic_cast<std::string*>((std::string*)lpVoid);
	if (nullptr == str || nullptr == buffer) return -1;
	std::string strHeader((char*)buffer);
	int nPos = strHeader.find("Content-Disposition");
	if (nPos != std::string::npos)
	{
		std::string strFileInfo = strHeader.substr(nPos);
		// file info:filename=filename.type
		std::string strFileTag = "filename=";
		std::string strHeaderTag = "\r\n";
		nPos = strFileInfo.find(strFileTag);
		if (nPos != std::string::npos)
		{
			strFileInfo = strFileInfo.substr(0, strFileInfo.length() - strHeaderTag.length());
			std::string strFileName = strFileInfo.substr(nPos + strFileTag.length());
			str->append(strFileName.c_str(), strFileName.length());
		}
	}
	return nmemb;
}

int64_t LibcurlHelper::OnWriteFile(void* ptr, size_t size, size_t nMenmb, FILE* stream)
{
	int64_t written = fwrite(ptr, size, nMenmb, stream);
	return written;
	// 接续写入数据
	//FILE* fp = NULL;  
	//fopen_s(&fp, "c:\\test.dat", "ab+");
	//size_t nWrite = fwrite(ptr, nSize, nmemb, fp);
	//fclose(fp);  
	//return nWrite; 
}

int64_t LibcurlHelper::OnMultiWriteFile(void* ptr, size_t size, size_t nMenmb, void* pUserData)
{
	FileNodeInfo* pNodeInfo = (FileNodeInfo*)pUserData;
	size_t written = 0;
	if (0 == pNodeInfo->nEndPos)
	{
		return fwrite(ptr, size, nMenmb, pNodeInfo->fp);
	}
	if (pNodeInfo->nStartPos + size * nMenmb <= pNodeInfo->nEndPos)
	{
		fseek(pNodeInfo->fp, pNodeInfo->nStartPos, SEEK_SET);
		written = fwrite(ptr, size, nMenmb, pNodeInfo->fp);
		pNodeInfo->nStartPos += size * nMenmb;
	}
	else
	{
		fseek(pNodeInfo->fp, pNodeInfo->nStartPos, SEEK_SET);
		written = fwrite(ptr, 1, pNodeInfo->nEndPos - pNodeInfo->nStartPos + 1, pNodeInfo->fp);
		pNodeInfo->nStartPos = pNodeInfo->nEndPos;
	}
	return written;
}

int LibcurlHelper::OnProcess(void* ptr, double nTotalToDownload, double nAlreadyDownload
	, double nTotalUploadSize, double nAlreadyUpload)
{
	if (nTotalToDownload <= 0)return 0;
	CURL* pCurl = static_cast<CURL*>(ptr);
	char szTimeInfo[kSpeedInfo] = { 0 };
	double nSpeed;
	std::string unit = "B";
	// curl_get_info必须在curl_easy_perform之后调用
	auto code = curl_easy_getinfo(pCurl, CURLINFO_SPEED_DOWNLOAD, &nSpeed);
	if (CURLE_OK == code)
	{
		if (nSpeed != 0)
		{
			// Time remaining
			double leftTime = (nTotalToDownload - nAlreadyDownload) / nSpeed;
			int hours = leftTime / 3600;
			int minutes = (leftTime - hours * kSecondsPerHour) / kSecondsPerMinute;
			int seconds = leftTime - hours * kSecondsPerHour - minutes * kSecondsPerMinute;

#ifdef _WIN32
			sprintf_s(szTimeInfo, kSpeedInfo, "%02d:%02d:%02d", hours, minutes, seconds);
#else
			sprintf(szTimeInfo, "%02d:%02d:%02d", hours, minutes, seconds);
#endif
		}
		if (nSpeed > kGByte)
		{
			unit = "G";
			nSpeed /= kGByte;
		}
		else if (nSpeed > kMByte)
		{
			unit = "M";
			nSpeed /= kMByte;
		}
		else if (nSpeed > kKByte)
		{
			unit = "kB";
			nSpeed /= kKByte;
		}
		printf("[speed]:%.2f%s--", nSpeed, unit.c_str());
		printf("[left-time]:%s--", szTimeInfo);
	}
	else
	{
		printf("%s\n", curl_easy_strerror(code));
	}

	int nPersent = (int)(nAlreadyDownload / nTotalToDownload * 100);
	printf("[下载进度]:%0d%%\n", nPersent);
	// return bigger than 0, it means pause
	return 0;
}

LibcurlHelper::LibcurlHelper()
{
	m_bDebug = false;
	m_pFout = nullptr;
	m_nPart = 0;
	m_nTimeout = 30 * 1000;
}

LibcurlHelper::~LibcurlHelper()
{
}

int LibcurlHelper::Post(const HttpPara& paraHttp, const std::string& strData, std::string& strResponse)
{
	CURL* pCurl = curl_easy_init();
	if (nullptr == pCurl)
		return CURLE_FAILED_INIT;
	if (m_bDebug)
	{
		curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);	//调试信息
		curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, &LibcurlHelper::OnDebug);
	}

	curl_slist* pList = set_header(paraHttp.vecHeaders);

	/* CURLOPT_HEADER 请求获取头部信息：
	HTTP/1.1 200 OK
	Date: Mon, 06 Mar 2023 04:44:45 GMT
	Content-Type: application/json; charset=utf-8
	Server: Kestrel
	Content-Length: 536
	*/
	curl_easy_setopt(pCurl, CURLOPT_HEADER, false);
	pList = curl_slist_append(pList, "Accept: application/json");
	pList = curl_slist_append(pList, "Content-Type:application/json");
	pList = curl_slist_append(pList, "charsets: utf-8");

	if (!paraHttp.Authorization.empty())
	{
		const std::string auth_header = "Authorization: " + paraHttp.Authorization;
		pList = curl_slist_append(pList, auth_header.c_str());
	}

	curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1);  //支持服务器跳转
	curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pList);
	curl_easy_setopt(pCurl, CURLOPT_URL, paraHttp.strUrl.c_str());
	curl_easy_setopt(pCurl, CURLOPT_POST, true);

	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, paraHttp.nTransTimeout); //链接超时
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, paraHttp.nConnectTimeout);

	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false);	//证书验证
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, false);	//ssl算法验证

	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strData.c_str());



	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &LibcurlHelper::OnWriteString);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&strResponse);

	CURLcode res = curl_easy_perform(pCurl);

	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() post fail，reason: %s\n", curl_easy_strerror(res));
	}

	if (pList)
	{
		curl_slist_free_all(pList);
	}
	curl_easy_cleanup(pCurl);

	return res;
}

int LibcurlHelper::PostBase64Json(const HttpPara& paraHttp, const std::string& strData, std::string& strResponse)
{
	CURL* pCurl = curl_easy_init();
	if (nullptr == pCurl)
		return CURLE_FAILED_INIT;
	if (m_bDebug)
	{
		curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);	//调试信息
		curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, &LibcurlHelper::OnDebug);
	}

	curl_slist* pList = set_header(paraHttp.vecHeaders);

	/* CURLOPT_HEADER 请求获取头部信息：
	HTTP/1.1 200 OK
	Date: Mon, 06 Mar 2023 04:44:45 GMT
	Content-Type: application/json; charset=utf-8
	Server: Kestrel
	Content-Length: 536
	上述导致响应json的时候会包含上述信息，导致破坏json解析
	*/
	curl_easy_setopt(pCurl, CURLOPT_HEADER, false);

	pList = curl_slist_append(pList, "Accept: application/json");
	pList = curl_slist_append(pList, "Content-Type:application/json");
	pList = curl_slist_append(pList, "charsets: utf-8");


	if (!paraHttp.Authorization.empty())
	{
		const std::string auth_header = "Authorization: " + paraHttp.Authorization;
		pList = curl_slist_append(pList, auth_header.c_str());
	}

	curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1);  //支持服务器跳转
	curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pList);
	curl_easy_setopt(pCurl, CURLOPT_URL, paraHttp.strUrl.c_str());
	curl_easy_setopt(pCurl, CURLOPT_POST, true);
	curl_easy_setopt(pCurl, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(pCurl, CURLOPT_TCP_KEEPINTVL, 60L);
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, paraHttp.nTransTimeout); //链接超时
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, paraHttp.nConnectTimeout);

	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false);	//证书验证
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, false);	//ssl算法验证


	//curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, -1); //方式I： -1：撤换使用 strData.len 作依据
	//curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, strData.size()); //方式II: -1：撤换使用 strData.len 作依据
	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strData.c_str());

	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &LibcurlHelper::OnWriteString);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&strResponse);

	CURLcode res = curl_easy_perform(pCurl);

	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() post fail，reason: %s\n", curl_easy_strerror(res));
	}

	if (pList)
	{
		curl_slist_free_all(pList);
	}
	curl_easy_cleanup(pCurl);

	return res;
}

int LibcurlHelper::PostWithCer(const HttpPara& paraHttp, const std::string& strData, std::string& strResponse)
{
	CURL* pCurl = curl_easy_init();
	if (nullptr == pCurl)
		return CURLE_FAILED_INIT;
	if (m_bDebug)
	{
		curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);	//调试信息
		curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, &LibcurlHelper::OnDebug);
	}
	curl_easy_setopt(pCurl, CURLOPT_URL, paraHttp.strUrl.c_str());
	curl_easy_setopt(pCurl, CURLOPT_POST, true);

	/* struct curl_slist pCurl_slist_append(struct curl_slist * list, const char * string);
	@ 添加Http消息头
	@ 属性string：形式为name+": "+contents
	*/
	curl_slist* pList = set_header(paraHttp.vecHeaders);
	/* CURLcode curl_easy_setopt(CURL *handle, CURLoption option, parameter);
	@ 设置属性及常用参数
	*/
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, true);	//证书验证
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, true);	//ssl算法验证

	//加入这段 引用证书----------------------------------------------------------
	/*curl_easy_setopt(pCurl, CURLOPT_SSLCERTTYPE, "PEM");
	curl_easy_setopt(pCurl, CURLOPT_SSLCERT, cert_file_path.c_str());
	curl_easy_setopt(pCurl, CURLOPT_SSLKEYTYPE, "PEM");
	curl_easy_setopt(pCurl, CURLOPT_SSLKEY, key_file_path.c_str());
	curl_easy_setopt(pCurl, CURLOPT_CAINFO, ca_file_path.c_str());
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDSIZE, -1L);*/

	//---------------------------------------------------------------------------
	curl_easy_setopt(pCurl, CURLOPT_USERNAME, "test");
	curl_easy_setopt(pCurl, CURLOPT_PASSWORD, "1234");
	curl_easy_setopt(pCurl, CURLOPT_POSTFIELDS, strData.c_str());
	curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &LibcurlHelper::OnWriteString);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1); //是否抓取跳转后的页面     
	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, m_nTimeout);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, m_nTimeout);

	CURLcode res = curl_easy_perform(pCurl);
	if (pList)
	{
		curl_slist_free_all(pList);
	}
	curl_easy_cleanup(pCurl);

	return res;
}

int LibcurlHelper::UploadFile(const HttpPara& paraHttp, const std::string& strFile, std::string& strResponse)
{
	curl_httppost* pFirst = nullptr;
	curl_httppost* pLast = nullptr;
	curl_slist* pHeaderList = nullptr;
	curl_formadd(&pFirst, &pLast,
		CURLFORM_COPYNAME, "file",
		CURLFORM_FILE, strFile.c_str(),
		CURLFORM_CONTENTTYPE, "multipart/form-data",
		CURLFORM_END);
	curl_formadd(&pFirst, &pLast,
		CURLFORM_COPYNAME, "submit",
		CURLFORM_COPYCONTENTS, "send",
		CURLFORM_END);
	static const char buffer[] = "Expect:";
	pHeaderList = curl_slist_append(pHeaderList, buffer);

	CURL* pCurl = curl_easy_init();
	if (m_bDebug)
	{
		curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);	//调试信息
		curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, &LibcurlHelper::OnDebug);
	}
	curl_easy_setopt(pCurl, CURLOPT_URL, paraHttp.strUrl.c_str());
	curl_easy_setopt(pCurl, CURLOPT_POST, 1L);
	curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pHeaderList);
	curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, pFirst);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &LibcurlHelper::OnWriteString);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, 5);
	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, 5);
	CURLcode res = curl_easy_perform(pCurl);
	int response_code = 0;
	curl_easy_getinfo(pCurl, CURLINFO_RESPONSE_CODE, &response_code);
	if (res != CURLE_OK || response_code != 200) {
		return -1;
	}
	return CURLE_OK;
}

int LibcurlHelper::PostFormData(const HttpPara& paraHttp, const curl_httppost* pData, const std::string& strResponse)
{
	CURL* pCurl = curl_easy_init();

	curl_easy_setopt(pCurl, CURLOPT_URL, paraHttp.strUrl.c_str());
	curl_easy_setopt(pCurl, CURLOPT_HTTPPOST, pData);
	curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &LibcurlHelper::OnWriteString);
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);

	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, false);
	curl_easy_setopt(pCurl, CURLOPT_USERNAME, paraHttp.strUser);
	curl_easy_setopt(pCurl, CURLOPT_PASSWORD, paraHttp.strPassword);

	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, m_nTimeout);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, m_nTimeout);
	CURLcode res = curl_easy_perform(pCurl);
	curl_easy_cleanup(pCurl);

	return res;
}

int LibcurlHelper::Get(const HttpPara& paraHttp, std::string& strResponse)
{
	CURLcode res;
	CURL* pCurl = curl_easy_init();
	if (nullptr == pCurl)
		return CURLE_FAILED_INIT;
	if (m_bDebug)
	{
		curl_easy_setopt(pCurl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(pCurl, CURLOPT_DEBUGFUNCTION, &LibcurlHelper::OnDebug);
	}
	curl_slist* pList = set_header(paraHttp.vecHeaders);
	pList = curl_slist_append(pList, "Accept: application/json");
	pList = curl_slist_append(pList, "Content-Type:application/json");
	pList = curl_slist_append(pList, "charsets: utf-8");
	if (!paraHttp.Authorization.empty())
	{
		const std::string auth_header = "Authorization: " + paraHttp.Authorization;
		pList = curl_slist_append(pList, auth_header.c_str());
	}
	curl_easy_setopt(pCurl, CURLOPT_HTTPHEADER, pList); //附加Header //-----------

	curl_easy_setopt(pCurl, CURLOPT_URL, paraHttp.strUrl.c_str());    //访问的URL
	curl_easy_setopt(pCurl, CURLOPT_READFUNCTION, NULL);     //
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &LibcurlHelper::OnWriteString); //数据回调函数
	curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, (void*)&strResponse); //数据回调参数，一般为buffer或者文件fd
																	 /*
																	 * 当多个线程都使用超时处理的时候，同时主线程中有sleep或是wait等操作。
																	 * 如果不设置这个选项，libcurl将会发信号打断这个wait从而导致程序退出。
																	 */
	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(pCurl, CURLOPT_CONNECTTIMEOUT, paraHttp.nConnectTimeout);
	curl_easy_setopt(pCurl, CURLOPT_TIMEOUT, paraHttp.nTransTimeout);
	//https
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, false);
	//curl_easy_setopt(pCurl,CURLOPT_CAINFO, "D:\\cert\\PC-12.crt");

	/* CURLcode curl_easy_perform(CURL *handle);
	@ 开始下载
	*/
	if (res = curl_easy_perform(pCurl), CURLE_OK != res)
		printf("%s\n", curl_easy_strerror(res));
	curl_easy_cleanup(pCurl);
	return res;
}

int LibcurlHelper::DownloadFile(const HttpPara& paraHttp, const std::string& strFile)
{
	std::string strRemoteFile;
	int64_t nFileLength = get_download_file_length(paraHttp, strRemoteFile);
	FILE* fp;
	CURL* pCurl = curl_easy_init();
	CURLcode res = CURLE_OK;
	if (nullptr == pCurl)
		return CURLE_FAILED_INIT;
	std::string strResultFile = generate_filename(paraHttp.strUrl, strFile, strRemoteFile);
#ifdef _WIN32
	fopen_s(&fp, strResultFile.c_str(), "wb");
#elif __linux__
	//fopen() 请自行补充
#endif
	res = curl_easy_setopt(pCurl, CURLOPT_URL, paraHttp.strUrl.c_str());
	if (CURLE_OK != res)
	{
		fclose(fp);
		curl_easy_cleanup(pCurl);
		printf("%s\n", curl_easy_strerror(res));
		return -1;
	}
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, false);
	std::string strUserInfo = paraHttp.strUser + ":" + paraHttp.strPassword;
	curl_easy_setopt(pCurl, CURLOPT_USERPWD, strUserInfo.c_str());
	curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &LibcurlHelper::OnWriteFile);
	res = curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);
	curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(pCurl, CURLOPT_PROGRESSFUNCTION, &LibcurlHelper::OnProcess);
	// get speed info by easy-handle
	curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, pCurl);
	curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
	//开始执行请求
	printf("Downloadind file...\n");
	res = curl_easy_perform(pCurl);
	fclose(fp);
	if (CURLE_OK != res)
	{
		curl_easy_cleanup(pCurl);
		printf("%s\n", curl_easy_strerror(res));
		return res;
	}
	curl_easy_cleanup(pCurl);	//释放内存
	printf("Download file finished\n");
	return res;
}

int LibcurlHelper::DownloadBigFile(const HttpPara& paraHttp, const std::string& strPath, const std::string& strFileName, int nThread)
{
	std::string strRemoteFile;
	int64_t nFileLength = get_download_file_length(paraHttp, strRemoteFile);
	if (0 >= nFileLength && strRemoteFile.empty())
	{
		printf("-------------------------Get file length and name failed----------------------\n");
		return -1;
	}
	std::string strResultFile = strPath + generate_filename(paraHttp.strUrl, strFileName, strRemoteFile);
#ifdef _WIN32
	fopen_s(&m_pFout, strResultFile.c_str(), "wb");
#elif __linux__
	//fopen() 请自行补充
#endif
	if (!m_pFout)
	{
		printf("Open file:%s failed\n", strResultFile.c_str());
		return -1;
	}
	int64_t nPartSize = nFileLength ? nFileLength / nThread : 0;
	m_nPart = nThread;
	for (int i = 0; i < nThread; i++)
	{
		CURL* pCurl = curl_easy_init();
		if (nullptr == pCurl)continue;
		FileNodeInfo* pNode(new FileNodeInfo());
		if (nFileLength > 0)
		{
			pNode->nStartPos = i * nPartSize;
			pNode->nEndPos = (i + 1) * nPartSize - 1;
			char range[64] = { 0 };
			snprintf(range, sizeof(range), "%lld-%lld", pNode->nStartPos, pNode->nEndPos);
			curl_easy_setopt(pCurl, CURLOPT_RANGE, range);
		}
		pNode->nPart = i;
		pNode->fp = m_pFout;
		pNode->pCurl = pCurl;

		// Download data
		curl_easy_setopt(pCurl, CURLOPT_URL, paraHttp.strUrl.c_str());
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, false);
		// set user and password info with user:password format, 
		// it passed in more situation
		//curl_easy_setopt(pCurl, CURLOPT_USERNAME, paraHttp.strUser.c_str());
		//curl_easy_setopt(pCurl, CURLOPT_PASSWORD, paraHttp.strPassword.c_str());
		std::string strUserInfo = paraHttp.strUser + ":" + paraHttp.strPassword;
		curl_easy_setopt(pCurl, CURLOPT_USERPWD, strUserInfo);
		curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, &LibcurlHelper::OnMultiWriteFile);
		curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, pNode);
		curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(pCurl, CURLOPT_PROGRESSFUNCTION, &LibcurlHelper::OnProcess);
		// get speed info by easy-handle
		curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, pCurl);
		curl_easy_setopt(pCurl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_LIMIT, 1L);
		curl_easy_setopt(pCurl, CURLOPT_LOW_SPEED_TIME, 5L);
		// resume download file
		//curl_easy_setopt(pCurl, CURLOPT_RESUME_FROM_LARGE, get_local_file_length(localFile));
		std::thread(std::bind(&LibcurlHelper::do_download, this, (void*)pNode)).detach();
	}
	while (m_nPart > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
	}
	return 0;
}

void LibcurlHelper::get_header_type(int nType, std::string& strHeader)
{
	switch (nType)
	{
	case kContentTypeTxt:
		strHeader = "Content-Type: text/plain";
		break;
	case kContentTypeHtml:
		strHeader = "Content-Type: text/html";
		break;
	case kContentTypeXml:
		strHeader = "Content-Type: text/xml";
		break;
	case kContentTypeJson:
		strHeader = "Content-Type: application/json";
		break;
	case kContentTypeUrlencoded:
		strHeader = "Content-Type: x-www-form-urlencoded";
		break;
	case kContentTypeZip:
		strHeader = "Content-Type: application/zip";
	default:
		strHeader = "Content-Type: application/octet-stream";
		break;
	}
}

curl_slist* LibcurlHelper::set_header(const std::vector<std::string>& vecHeaders)
{
	curl_slist* pHeader = nullptr;
	for (const auto& item : vecHeaders)
	{
		pHeader = curl_slist_append(pHeader, item.c_str());
	}
	return pHeader;
}

int64_t LibcurlHelper::get_download_file_length(const HttpPara& paraHttp, std::string& strRemoteFileName)
{
	double downloadFileLenth = 0;
	CURL* pCurl = curl_easy_init();
	curl_easy_setopt(pCurl, CURLOPT_URL, paraHttp.strUrl.c_str());
	curl_easy_setopt(pCurl, CURLOPT_HEADER, 1);    //只需要header头
	curl_easy_setopt(pCurl, CURLOPT_NOBODY, 1);    //不需要body
	curl_easy_setopt(pCurl, CURLOPT_HEADERFUNCTION, &LibcurlHelper::OnWriteHeader);
	// store header of response header, just parse filename
	curl_easy_setopt(pCurl, CURLOPT_HEADERDATA, (void*)&strRemoteFileName);
	curl_easy_setopt(pCurl, CURLOPT_PROXY_SSL_VERIFYPEER, 1L);
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(pCurl, CURLOPT_SSL_VERIFYHOST, false);
	/*std::string strUserInfo = paraHttp.strUser + ":" + paraHttp.strPassword;
	curl_easy_setopt(pCurl, CURLOPT_USERPWD, strUserInfo.c_str());*/
	//curl_easy_setopt(pCurl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/75.0.3770.100 Safari/537.36");
	CURLcode code = curl_easy_perform(pCurl);
	if (CURLE_OK == code)
	{
		// get file length
		code = curl_easy_getinfo(pCurl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
		// get content type
		/*char *strContentType;
		code = curl_easy_getinfo(pCurl, CURLINFO_CONTENT_TYPE, &strContentType);
		printf("Content-type:%s\n", strContentType);*/
	}
	else
	{
		std::string strError = curl_easy_strerror(code);
		printf("Get remote file length failed:%s\n", strError.c_str());
	}
	curl_easy_cleanup(pCurl);
	return downloadFileLenth;
}

std::string LibcurlHelper::generate_filename(const std::string& strUrl, const std::string& strCustonFile, const std::string& strFileInfo)
{
	if (!strCustonFile.empty())
		return strCustonFile;
	if (!strFileInfo.empty())
		return strFileInfo;
	// exact filename from url
	int nPos = strUrl.rfind("/");
	int nLimitPos = strUrl.find("\\");
	int nSartPos = nPos > nLimitPos ? nPos : nLimitPos;
	return strUrl.substr(nSartPos + 1);
}

void LibcurlHelper::do_download(void* pData)
{
	FileNodeInfo* pNode = (FileNodeInfo*)pData;
	printf("Downloadind file...\n");
	CURLcode res = curl_easy_perform(pNode->pCurl);
	if (res != CURLE_OK)
	{
		printf("Download data failed,part:%lld,error[%d]:%s\n", pNode->nPart, res, curl_easy_strerror((CURLcode)res));
	}
	curl_easy_cleanup(pNode->pCurl);

	notify_download();
	delete pNode;
}

void LibcurlHelper::notify_download()
{
	std::lock_guard<std::mutex> lock(m_mtLockRecord);
	if (0 == --m_nPart) {
		printf("Download finished\n");
	}
	fclose(m_pFout);
}

long LibcurlHelper::get_local_file_length(const std::string& strFile)
{
	long nLength = 0;
#ifdef _WIN32
	if (0 != _access(strFile.c_str(), 0))
		return nLength;
	std::fstream fin(strFile.c_str(), std::ios::binary | std::ios::in);
	if (fin.is_open())
	{
		fin.seekg(0, std::ios::end);
		std::streampos size = fin.tellp();
		fin.close();
		nLength = size;
	}
#elif __linux__
	// 请自行补充
#endif
	return nLength;
}

std::string LibcurlHelper::str_to_utf8(const std::string& str)
{
#ifdef _WIN32
	int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);
	wchar_t* pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴 
	ZeroMemory(pwBuf, nwLen * 2 + 2);
	::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);
	int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);
	char* pBuf = new char[nLen + 1];
	ZeroMemory(pBuf, nLen + 1);
	::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);
	std::string retStr(pBuf);
	delete[]pwBuf;
	delete[]pBuf;
	pwBuf = NULL;
	pBuf = NULL;
#elif __linux__
	// 请自行补充
	std::string retStr("");
#endif
	return retStr;
}


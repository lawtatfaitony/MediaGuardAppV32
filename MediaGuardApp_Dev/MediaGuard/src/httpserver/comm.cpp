#include "comm.h"

//int64_t getFileSize(const char* fileName)
//{
//#ifdef _WIN32
//	WIN32_FIND_DATA fileInfo;
//	HANDLE hFind = FindFirstFile(fileName, &fileInfo);
//	if (hFind != INVALID_HANDLE_VALUE)
//	{
//		//fileInfo.nFileSizeLow;
//		FindClose(hFind);
//		return MAKE64(fileInfo.nFileSizeHigh, fileInfo.nFileSizeLow);
//	}
//#else //-D_GNU_SOURCE 加编译选项 宏定义
//	// struct stat64 st;
//	// if (stat64(fileName, &st) == 0)
//	// {
//	// 	return st.st_size;
//	// }
//#endif
//	return -1;
//}

size_t getFileCtx(const char* fileName, char* ctx, size_t ctxsize)
{
	//FILE* fp = fopen(fileName, "rb"); //提示不安全 改為:
	
	FILE* fp;
	fopen_s(&fp, fileName, "r");
	 
	if (fp == NULL)
		return 0;

	size_t ret = fread(ctx, 1, ctxsize, fp);

	fclose(fp);

	return ret;
}

#ifndef DIR_SEP_CHAR
#define DIR_SEP_CHAR '\\'
#endif

int GetExecPath(char* p, int size)
{
#ifdef _WIN32
	int len = GetModuleFileName(NULL, p, size);
	char* ptr = NULL;
	if (len)
	{
		ptr = strrchr(p, DIR_SEP_CHAR);
		if (ptr)
			ptr++;
		else
			ptr = p;
	}
	else
		ptr = p;
	*ptr = '\0';
	return (int)strlen(p);
#elif __linux__
	return (int)strlen(p);
#endif
}

size_t split(const std::string& s, const std::string& seperator, std::vector<std::string>& strlist)
{
	//
	size_t posBegin = 0;
	size_t posSeperator = s.find(seperator);

	while (posSeperator != s.npos) {
		strlist.push_back(s.substr(posBegin, posSeperator - posBegin));// 
		posBegin = posSeperator + seperator.size(); // 分隔符的下一个元素
		posSeperator = s.find(seperator, posBegin);
	}

	if (posBegin != s.length()) // 指向最后一个元素，加进来
		strlist.push_back(s.substr(posBegin));
	return strlist.size();
}

void parse_web_param(const std::string& str, std::map<std::string, std::string>& mp_kv)
{
	std::vector<std::string> strlist;
	split(str, "&", strlist);
	//
	for (size_t i = 0; i < strlist.size(); i++)
	{
		const std::string& sitem = strlist[i];
		const char* kstart = sitem.c_str();
		const char* esep = strchr(kstart, '=');
		if (esep != NULL)
			mp_kv[std::string(kstart, esep - kstart)] = std::string(esep + 1);
		else
			mp_kv[sitem] = "";
	}
}

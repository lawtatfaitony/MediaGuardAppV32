#pragma once
#include <string>
#include <sstream> 

//項目本來有一個Time.h的文件,如果需要也是引用這個文件, 應該不需要下面的做法 引用系統的 2024-12-22
//#ifdef _WIN32
//#include <Time.h> 
//#elif __linux__
//// 请补linux内容 
//#endif

namespace Service
{
	// 响应结构 
	struct Meta
	{
		bool success;
		std::string message;
		int errorCode;
	};
}

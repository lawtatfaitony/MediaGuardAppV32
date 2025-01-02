#pragma once
#include <string>
#include <chrono>
#include <ctime> 
#include <iomanip>
#include <iostream>
#include <sstream> 
#ifdef __linux__
#include <time.h>
#endif

class Time
{
public:
	static std::string GetCurrentDate()
	{
		time_t ltime = time(NULL);
		std::tm ltm = { 0 };
#ifdef _WIN32
		localtime_s(&ltm, &ltime);
#elif __linux__
		localtime_r(&ltime, &ltm);
#endif
		char buffer[128] = { 0 }; 
		strftime(buffer, sizeof(buffer), "%Y-%m-%d", &ltm);
		return buffer;
	}

	/// <summary>
	/// 獲取日期時間 : 2024-12-25 17:44:16
	/// </summary>
	/// <returns></returns>
	static std::string GetCurrentSystemTime()
	{
		time_t ltime = time(NULL);
		std::tm ltm = { 0 };
#ifdef _WIN32
		localtime_s(&ltm, &ltime);
#elif __linux__
		struct tm result;
		localtime_r(&ltime, &result);
		ltm = result;
#endif
		char buffer[128] = { 0 };
		strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &ltm);
		return buffer;
	}
	
	static int64_t GetMilliTimestamp()
	{
		auto curr = std::chrono::system_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			curr.time_since_epoch());
		return ms.count();
	}

	static int64_t GetTimestamp()
	{
		auto curr = std::chrono::system_clock::now();
		auto ms = std::chrono::duration_cast<std::chrono::seconds>(
			curr.time_since_epoch());
		return ms.count();
	}
};


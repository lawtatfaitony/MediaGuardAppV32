#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>  // C++ std 17 
#include <chrono>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h> 
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

static const char kWindowSplash = '\\';
static const char kLinuxSplash = '/'; 
#ifdef _WIN32
static const char kSeprator = kWindowSplash;
#elif __linux__
static const char kSeprator = kLinuxSplash; 
#endif

namespace fs = std::filesystem;

class File
{
	static const int kSuccess = 0;
	static const int kError = -1;
public:
	 
	/*獲取 應用程式根路徑*/
	static std::string GetWorkPath()
	{
		const fs::path rootPath = fs::current_path(); 
		//std::cout << "APPCLICATION ROOT PATH (from File::GetWorkPath) : " << rootPath << std::endl;
		return rootPath.string(); 
	}

	/*
	* @brief: check the end of directory
	* @para[in]:    strDir, the directory for checking
	* @return:      bool, end with splash to return true, or false
	*/
	static bool CheckEndWithSplash(const std::string& strDir)
	{
		if ((strDir.back() != kWindowSplash)
			&& (strDir.back() != kLinuxSplash))
			return false;
		return true;
	}

	/*
	* @brief: create single directory
	* @data:  20200207
	* @update:
	* @para[in]:    strDir, the directory for creating
	* @return:      bool, success to return true, or false
	*/
	static bool CreateSingleDirectory(const std::string& strDir)
	{ 
		try {
			// 使用 std::filesystem 創建目錄
			fs::create_directory(strDir);
			std::cout << "\nCreated Directory Successfully: " << strDir << "\n" << std::endl;
			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "\nCreated Directory error (File::CreateSingleDirectory): " << e.what() << "\n" << std::endl;
			return false;
		}
	}

	/*
	* @brief: create multi-directories
	* @data:  20200207
	* @update:
	* @para[in]:    strDir, the directory for creating
	* @return:      bool, success to return true, or false
	*/
	static bool CreateMultiDirectory(const std::string& strDir)
	{ 
		if (isDirectoryExists(strDir))
			return true;

		try {
			// 使用 std::filesystem 創建多層目錄
			fs::create_directories(strDir);
			std::cout << "\nCreated directories successfully: " << strDir << "\n" << std::endl;
			return true;
		}
		catch (const std::exception& e) {
			std::cerr << "\nCreated directories error: " << e.what() << "\n" << std::endl;
			return false;
		}
	}

	/*
	* get_file_info win/linux
	* 获取文件的创建时间
	* https://www.cnblogs.com/longma8586/p/13849223.html
	* c++ 获取文件创建时间、修改时间、访问时间、文件内容长度
	* 然后转换为本地时间
	* 转换: FileTime --> LocalTime
	* FileTimeToSystemTime(&ftCreate, &stUTC1);
	* SystemTimeToTzSpecificLocalTime(NULL, &stUTC1, &stLocal1);
	*/  
	static bool get_file_info(const std::string& strFilePath, int& iCreateTime, int& iFileLen) {
		try {
			fs::path filepath(strFilePath);

			if (!fs::exists(filepath)) {
				std::cerr << "\nFile::get_file_info File Not Exist : " << strFilePath << "\n" << std::endl;
				return false;
			}

			auto fileStatus = fs::status(filepath);
		 
			iFileLen = static_cast<int>(fs::file_size(filepath)); 
			auto lstTime = fs::last_write_time(filepath); 
			iCreateTime = std::chrono::duration_cast<std::chrono::milliseconds>(lstTime.time_since_epoch()).count(); 
		}
		catch (const std::exception& e) {
			std::cerr << "\nError while getting file info: " << e.what() << "\n" << std::endl;
			return false;
		}

		return true;
	}
	/*
	* 刪除目錄 std::string directoryPath = "path/to/your/directory"; // 指定要刪除的目錄路徑
	* removeDirectory(directoryPath);
	*/ 
	static void removeDirectory(const std::string& path) {
		try {
			fs::path directoryPath(path);

			if (fs::exists(directoryPath)) {
				fs::remove_all(directoryPath);
				std::cout << "\nFile::removeDirectory Directory removed: " << path << "\n" << std::endl;
			}
			else {
				std::cerr << "\nFile::removeDirectory Directory does not exist: " << path << "\n" << std::endl;
			}
		}
		catch (const std::exception& e) {
			std::cerr << "\nFile::removeDirectory Error while removing directory: " << e.what() << "\n" << std::endl;
		}
	}

	/*
	* 判斷是否是一個文件夾
	*/
	static bool isDirectory(const std::string& path) { 
		if (fs::is_directory(path))
		{
			return true;
		}
		else
		{
			return false;
		} 
	}

	/*
	* 刪除文件函數
	*/
	static bool deleteFile(const std::string& filePathx) {

		std::filesystem::path filepath(filePathx);

		if (!std::filesystem::exists(filepath)) {
			std::cerr << "\nFile::deleteFile File Not Exist : " << filepath.string() << "\n" << std::endl;
			return false;
		}
		  
		if (fs::remove(filepath) == true) { 
			
#ifdef DEBUG
			std::cout << "\nFile::deleteFile() = [true] to delete file successfully: " << filepath.string() << "\n" << std::endl;
#endif // DEBUG 
			return true;
		}
		else {
			std::cerr << "\nFile::deleteFile() = [false] to deleted file fail!!! : " << filepath.string() << "\n" << std::endl;
			return false;
		}
	}

	/*
	* @brief: get files of directory
	* @return:      bool, success to return true, or false
	*/
	static bool GetFilesOfDir(const std::string& strDir, std::vector<std::string>& vecFile) {
		try {
			for (const auto& entry : fs::directory_iterator(strDir)) {
				if (fs::is_regular_file(entry.path())) {
					vecFile.push_back(entry.path().string());
				}
			}
		}
		catch (const std::exception& e) {
			std::cerr << "\nError while reading directory: " << e.what() << "\n" << std::endl;
			return false;
		}

		return true;
	}
	/*
	*獲取文件下的子文件夾集合
	*/
	static void GetDirsOfDir(const std::string& path, std::vector<std::string>& dirs) {
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			if (entry.is_directory()) {
				dirs.push_back(entry.path().string());
			}
		}
	}
	 
	/* 判斷目錄是否存在 */
	static bool isDirectoryExists(const std::string& directoryPath) {
		fs::path pathToCheck(directoryPath);
		return fs::exists(pathToCheck) && fs::is_directory(pathToCheck);
	}

	/*
	* support the json file reading
	* 讀取配置文件 deviceconfig.json
	*/ 
	static std::string readJsonFile(const std::string& strFile)
	{
		std::string jsonString = "";
		// 載入 JSON 檔案
		const char* filename = strFile.c_str();

		//FILE* fp = fopen(filename, "rb");

		FILE* fp = nullptr;
		fopen_s(&fp, filename, "rb");
		  
		if (!fp) {
			std::cerr << "\nCAN NOT OPEN JSON FILE : " << filename << "\n" << std::endl; 
			return jsonString;
		}

		char readBuffer[65536];
		rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

		rapidjson::Document document;
		document.ParseStream(is);

		if (document.HasParseError()) {
			std::cerr << "\nPARSE  JSON FILE  ERROR OCCURED!!!" << strFile << "\n" << std::endl;
			fclose(fp);
			return jsonString;
		}

		// 從 JSON 中讀取資料
		if (document.IsObject()) {
			// 將整個 JSON 文件轉換為字串
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			document.Accept(writer);
			std::string jsonString = buffer.GetString();
			buffer.Flush();
			buffer.Clear();

			return jsonString;
		}
		else {
			fclose(fp);
			return jsonString;
		}
	}

	/*讀取文件內容*/
	static int readFileContent(const std::string file_path, std::string& file_content)
	{
		std::ifstream file(file_path); // 打開 HTML 文件
		if (!file) {
			std::cerr << "\nCan't not open the file! "<< file_path  << std::endl;
			return 1;
		}
		 
		std::string line;

		// 逐行讀取文件內容
		while (std::getline(file, line)) {
			file_content += line + "\n"; // 將每行添加到內容中
		}

		file.close(); // 關閉文件
		 
		// 輸出文件內容
		//std::cout << "file_content:\n" << file_content << std::endl;

		return 0;
	}

	// 定義一個函數來替換所有匹配的子字符串
	static void replaceAll(std::string& str, const std::string& from, const std::string& to) {
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // 移動到替換後的位置，避免死循環
		}
	}


	/*
	* 保存Json 文件
	* savePath = ./conf/abcdxxx.json
	*/
	static bool saveJsonFile(const std::string& strContent, const std::string& savePath) {

		std::string newSavePath = File::ParseLocalPath(savePath);

		using namespace rapidjson;
		Document document;
		document.Parse(strContent.c_str());

		if (document.HasParseError()) {
			std::cout << "func::saveJsonFile :: JSON parsing error! " << newSavePath;
			return false;
		}

		StringBuffer buffer;
		Writer<StringBuffer> writer(buffer);
		document.Accept(writer);

		std::ofstream ofs(newSavePath);

		if (!ofs.is_open()) {

			std::cout << "func::saveJsonFile :: Unable to open file! " << newSavePath;
			return  false;
		}

		ofs << buffer.GetString();
		ofs.close();

		return true; //"JSON 檔案已成功儲存"
	}

	/*
	* 轉換為本地操作系統路徑
	*/
	static std::string ParseLocalPath(const std::string& path_file_name)
	{
		try {
			namespace fs = std::filesystem;
			fs::path filePath(path_file_name);

			// 將路徑格式轉換為本地格式
			return filePath.generic_string();
		}
		catch (const std::exception& e) {
			std::cout << "[EXCEPTION] func::ParseLocalPath exception:" << path_file_name << std::endl;
			// 錯誤處理：如果發生異常，返回原始路徑
			return path_file_name;
		}
	}
};
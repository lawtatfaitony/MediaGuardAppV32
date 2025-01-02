#pragma once
#include <cstdlib> // for system()
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <atomic>

#include <thread>
#include <list>
#include <memory>
#include <string>
#include <errno.h> 
#include <signal.h>
#include "StreamManager.h"
#include "stun/NatHeartBean.h"
#include "Common.h"

#include <chrono>
#include <filesystem>


class ManagerController
{
	typedef std::list<std::shared_ptr<StreamMangement>> StreamPtrList; //用於 函數 run_media_batch_list

private:
	StreamPtrList m_StreamPtrList; //用於 函數 run_media_batch_list 

	std::thread m_thread_hls_clear;

	std::atomic<bool> m_bExit/* = false*/;//bool m_bExit;
	std::mutex m_mtLock;
	std::condition_variable m_cvCond;

public:
	explicit ManagerController();
	~ManagerController();

public:
	//static std::string generate_directory(FileNameType type, std::string& video_path_str, std::string& audio_path_str, std::string& picture_path_str, std::string& hls_path_str);
	void Init();
	void Uninit();
	void run_media_list();
	void run_media_batch_list();

	ThreadPool m_ThreadPollForCtrl;
	 

public:
	static void clean_picture(int64_t picRemainMinutes);
	static HWDeviceType StrToHWDeviceType(const std::string& str);
	static void clean_video_store();
	static std::string current_working_directory(); 
	static double get_length(std::string filePath);
	static int64_t get_filesize(std::string path);

public:
	static void valid_op(int n);
	static void signal_check_main();
	static void main_initialize(); 
	static void http_server_start();
	static void create_main_media_folder();

	static void clearScreen();

	//void heartbean_nat_internet_ip_run();

	static void update_device_nat_internet_ip();
	 
	template<typename TP>
	static std::time_t to_time_t(TP tp);

};

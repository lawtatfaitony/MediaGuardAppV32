//目標對象:http://81.71.74.135:5002/hk/Device/GetMainComBySerialNo/BFEBFBFF000806EB
#pragma once
#include <iostream>
#include <sstream>
#include <string>
#include <iosfwd>

#include <thread>
#include "Basic/ThreadPool.h"
#include "Basic/ThreadObject.h"   
#include "Common/JsonHelper.h" 
#include "Common/Macro.h" 
#include "ErrorInfo/ErrorCode.h"
#include "Http/LibcurlHelper.h"
#include "../StreamHandle.h" 
#include "Config/DeviceConfig.h" 
#include "../hmac/hmac_sha1.h"
#include "DeviceInfo.h" 

DeviceInfo::DeviceInfo()
{  
}
DeviceInfo::~DeviceInfo()
{
}
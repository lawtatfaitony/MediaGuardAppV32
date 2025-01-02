#include "ErrorMessage.h"
#include "rapidjson/document.h" 
#include "rapidjson/writer.h" 
#include "Common/Macro.h"
#include "ErrorInfo/ErrorCode.h"


// global error message map
std::map<int, std::string> g_mapErrorInfo
{
	// common error code
	{ CP_OK , "" },
	{ CP_UNKNOW_ERROR , "unknow error" },
	{ CP_INVALID_JSON , "parse input json error" },
	{ CP_INVALID_PARA , "invalid para" },
	{ CP_EXCUTE_SQL_ERROR , "excute sql exception" },
	{ CP_INVALID_SESSION , "invalid session" },
	{ CP_NOT_EXIST , "data not exist" },
	{ CP_INVAOKE_TIMEOUT , "invoke with timeout" },
	{ CP_INVALID_DATABASE_CONN , "invalid database conntion instance" },

	{ CP_COMPARE_NOT_CONNECT , "can't connect compare server" },
	{ CP_INVALID_COMPARE_INSTANCE , "invalid compare server instance" },
	{ CP_TASK_SERVER_NOT_CONNECT , "can't connect task server" },
	{ CP_INVALID_TASK_SERVER_INSTANCE , "invalid task server instance" },

	// user
	{ CP_INVALID_USER , "unknow user" },
	{ CP_USER_ALREADY_EXIST , "name of user already exists" },
	{ CP_PASSWORD_ERROR , "error of password" },

	// library
	{ CP_LIBRARY_ALREADY_EXIST , "name of library exists" },
	{ CP_LIBRARY_NOT_EXIST , "library not exists" },
	{ CP_ADD_LIBRARY_COMPARE_FAILED , "can't connect compare server" },
	{ CP_ADD_LIBRARY_FAILED , "add library failed"},

	// camera
	{ CP_CAMERA_INVALID_RTSP , "invalid camera's rtsp" },
	{ CP_CAMERA_ALREADY_EXIST , "name of camera already exists" },
	{ CP_CAMERA_SEARCH_RTSP_ERROR , "search camera's rtsp error" },

	// task
	{ CP_TASK_ALREADY_EXIST , "name of task exists" },
	{ CP_TASK_SERVER_FAILED , "can't connect task server" },
	{ CP_COMPARE_1V1_FAILED,"1:1 compare failed" },

	// person
	{ CP_ADD_PERSON_FAILED, "add person failed" },
	{ CP_PERSON_ALREADY_EXIST, "identity of person already exists" },
	{ CP_EXTRACT_FAILED, "failed to extract feature" },
	{ CP_INVALID_PERSON_PHOTO, "invalid person's photo" },
	{ CP_PERSON_CARD_NO_EXIST, "person's card-no exist"},

	// business
	{ CP_TASK_IS_RUNNING,"task is running"},


};


using namespace Service;
ErrorMsgManagement::ErrorMsgManagement()
{
	init();
}

ErrorMsgManagement& ErrorMsgManagement::Instance()
{
	static ErrorMsgManagement g_Instance;
	return g_Instance;
}

ErrorMsgManagement::~ErrorMsgManagement()
{

}

std::string ErrorMsgManagement::GetErrorMsg(int nError, const std::string& strErrorInfo)
{
	if (strErrorInfo.empty())
	{
		SCOPE_LOCK(m_mtLock);
		auto itMatch = m_mapErrorMsg.find(nError);
		if (itMatch == m_mapErrorMsg.end())
			return std::string();
		return itMatch->second;
	}
	return generate_error_msg(nError, strErrorInfo);
}

void ErrorMsgManagement::init()
{
	for (const auto& item : g_mapErrorInfo)
		m_mapErrorMsg[item.first] = generate_error_msg(item.first, item.second);
}

std::string ErrorMsgManagement::generate_error_msg(int nError, const std::string& strErrorIn)
{
	std::string strErrorMsg;
	{
		rapidjson::Document doc(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& typeAllocate = doc.GetAllocator();
		doc.AddMember("code", nError, typeAllocate);
		doc.AddMember("msg", rapidjson::Value(strErrorIn.c_str(), typeAllocate), typeAllocate);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer>  writer(buffer);
		doc.Accept(writer);
		strErrorMsg = buffer.GetString();
	}
	return strErrorMsg;
}
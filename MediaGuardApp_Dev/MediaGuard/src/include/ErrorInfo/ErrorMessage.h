#pragma once
#include <string>
#include <mutex>
#include <map>

namespace Service
{
    class ErrorMsgManagement
    {
        ErrorMsgManagement();
    public:
        static ErrorMsgManagement& Instance();
        ~ErrorMsgManagement();
        std::string GetErrorMsg(int nError, const std::string& strErrorInfo = "");

    private:
        void init();
        std::string generate_error_msg(int nError, const std::string& strErrorIn);

    private:
        std::mutex m_mtLock;
        std::map<int, std::string> m_mapErrorMsg;

    };
}

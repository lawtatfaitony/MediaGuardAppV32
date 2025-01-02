//用於 curl 的回調函數
//LibcurlHelper.cpp
#pragma once
#include <functional>
#include "Basic.h"


NAMESPACE_BASIC_BEGIN
class RaiiHelper
{
    typedef std::function<void(void)> TFunc;
public:
    RaiiHelper(TFunc funcFirst, TFunc funcLast)
        : m_funcFirst(funcFirst)
        , m_funcLast(funcLast)
    {
        if (m_funcFirst)
        {
            m_funcFirst();
        }
    }

    ~RaiiHelper()
    {
        if (m_funcLast)
        {
            m_funcLast();
        }
    }
    RaiiHelper (const RaiiHelper&) = delete;
    RaiiHelper& operator=(const RaiiHelper&) = delete;

private:
    TFunc m_funcFirst;
    TFunc m_funcLast;

};
NAMESPACE_BASIC_END

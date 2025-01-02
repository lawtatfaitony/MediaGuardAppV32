#pragma once
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <assert.h>
#include "Basic.h"

NAMESPACE_BASIC_BEGIN
class ThreadObject
{
public:
    ThreadObject()
    {
        m_bStop.store(false);
    }
    ~ThreadObject()
    {

    }

    void Start(std::function<void()>proc, int nWaitMSecond = 0)
    {
        assert(proc);
        Stop();
        m_bStop.store(false);
        m_thRun = std::thread([=]() {
            while (!m_bStop.load())
            {
                if (proc)
                {
                    proc();
                }
                if (!m_bStop.load() && nWaitMSecond > 0)
                {
                    std::unique_lock<std::mutex>lck(m_mtLock);
                    m_cvConn.wait_for(lck, std::chrono::milliseconds(nWaitMSecond));
                }
            }
        });
    }

    void StartOnce(std::function<void()>proc)
    {
        assert(proc);
        Stop();
        m_bStop.store(false);
        m_thRun = std::thread([=]() {
            if (proc)
            {
                proc();
                m_bStop.store(true);
            }
        });
    }

    void SetEvent()
    {
        m_cvConn.notify_one();
    }

    bool IsRunning()
    {
        return (!m_bStop.load());
    }

    void Stop()
    {
        m_bStop.store(true);
        m_cvConn.notify_one();
        if (m_thRun.joinable())
        {
            m_thRun.join();
        }
    }

public:
    ThreadObject(const ThreadObject&) = delete;
    ThreadObject& operator=(const ThreadObject&) = delete;

private:
    std::thread m_thRun;
    std::atomic<bool> m_bStop;
    std::mutex m_mtLock;
    std::condition_variable m_cvConn;


};
NAMESPACE_BASIC_END
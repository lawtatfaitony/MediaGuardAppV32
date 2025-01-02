#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>



class ThreadPool
{
public:
    inline ThreadPool()
        : m_bStoped(false)
    {
    }

    inline ~ThreadPool()
    {
        Stop();
    }

public:
    void Start(unsigned short size = 1, int nMaxThread = 1)
    {
        m_nMaxThread = nMaxThread;
        add_thread(size);
    }

    void Stop()
    {
        m_bStoped.store(true);
        m_cvTask.notify_all(); // 唤醒所有线程执行
        for (std::thread& thread : m_vecPool)
        {
            if (thread.joinable())
                thread.join(); // 等待任务结束， 前提：线程一定会执行完
        }
    }

    // 提交一个任务
    // 调用.get()获取返回值会等待任务执行完,获取返回值
    // 有两种方法可以实现调用类成员，
    // 一种是使用   bind： .commit(std::bind(&Dog::sayHello, &dog));
    // 一种是用 mem_fn： .commit(std::mem_fn(&Dog::sayHello), &dog)
    template<class F, class... Args>
    auto Commit(F&& f, Args&&... args) ->std::future<decltype(f(args...))>
    {
        if (m_bStoped.load())    // stop == true ??
            throw std::runtime_error("commit on Threadm_vecPool is stopped.");

        using RetType = decltype(f(args...)); // typename std::result_of<F(Args...)>::type, 函数 f 的返回值类型
        auto task = std::make_shared<std::packaged_task<RetType()> >(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));    // wtf !
        std::future<RetType> future = task->get_future();
        {    // 添加任务到队列
             //对当前块的语句加锁  lock_guard 是 mutex 的 stack 封装类，构造的时候 lock()，析构的时候 unlock()
            std::lock_guard<std::mutex> lock{ m_lock };
            m_queTasks.emplace
            (
                [task]() {(*task)(); }
            );
        }
#ifdef THREAD_Pool_AUTO_GROW
        if (m_nThread < 1 && m_vecPool.size() < MAX_THREAD_NUM)
            addThread(1);
#endif // !THREADm_vecPool_AUTO_GROW
        m_cvTask.notify_one(); // 唤醒一个线程执行

        return future;
    }

    //空闲线程数量
    int GetAvailableThread() { return m_nThread; }
    //线程数量
    size_t GetPoolSize() { return m_vecPool.size(); }

private:
    void add_thread(int size)
    {
        for (; m_vecPool.size() < m_nMaxThread && size > 0; --size)
        {   //初始化线程数量
            m_vecPool.emplace_back([this] { // 工作线程函数
                while (!this->m_bStoped)
                {
                    std::function<void()> task;
                    {   // 获取一个待执行的 task
                        // unique_lock 相比 lock_guard 的好处是：可以随时 unlock() 和 lock()
                        std::unique_lock<std::mutex> lock{ this->m_lock };
                        this->m_cvTask.wait(lock,
                            [this] {return this->m_bStoped.load() || !this->m_queTasks.empty(); }
                        ); // wait 直到有 task
                        if (this->m_bStoped && this->m_queTasks.empty())return;
                        task = std::move(this->m_queTasks.front()); // 取一个 task
                        this->m_queTasks.pop();
                    }
                    --m_nThread;
                    task();
                    ++m_nThread;
                }
            });
            ++m_nThread;
        }
    }

private:
    using Task = std::function<void()>;
    // 线程池
    std::vector<std::thread> m_vecPool;
    // 任务队列
    std::queue<Task> m_queTasks;
    // 同步
    std::mutex m_lock;
    // 条件阻塞
    std::condition_variable m_cvTask;
    // 是否关闭提交
    std::atomic<bool> m_bStoped;
    //空闲线程数量
    std::atomic<int> m_nThread;
    int m_nMaxThread;

};

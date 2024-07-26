#include "../head/head.hpp"

template <typename T>
class TaskQueue
{
public:
    TaskQueue() = default;
    TaskQueue(const TaskQueue &&other) noexcept // noexpect 表示这个函数不会抛出任何异常
    {
        std::unique_lock <std::mutex> lock (other.m_mutex);
        m_queue = std::move(other.m_queue);
    }
    ~TaskQueue() = default;
    bool empty()//判断队列是否为空
    {
        std::unique_lock <std::mutex> lock (m_mutex);
        return m_queue.empty();
    }
    int size()
    {
        std::unique_lock <std::mutex> lock(m_mutex);
       return m_queue.size();
    }
    void enqueue(T &t)
    {
        std::unique_lock <std::mutex> lock(m_mutex);
        m_queue.emplace(std::move(t));
    }
    bool dequeue(T &t)
    {
        std::unique_lock < std::mutex> lock(m_mutex);
        if(m_queue.empty())
        {
            return false;
        }
        t = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
};

//类外定义成员函数的实现
// template <typename T>
// bool TaskQueue<T>::empty()
// {
// }

class ThreadPool
{
private:
    class ThreadWork
    {
    private:
        int m_id;
        ThreadPool *m_thread_pool;
    public:
        ThreadWork(int id,ThreadPool* thread_pool):m_id(id),m_thread_pool(thread_pool) {}
        void operator()()
        {
            std::function<void(void)> func;
            bool dequeued ;
            while( !m_thread_pool->m_shutdown )
            {
                {
                    std::unique_lock<std::mutex> lock(m_thread_pool->m_mutex);
                    if(m_thread_pool->m_queue.empty())
                    {
                        m_thread_pool->m_conditional_lock.wait(lock);
                    }
                    dequeued = m_thread_pool->m_queue.dequeue(func);
                }
                if(dequeued)
                {
                    func();
                }
            }
        }
    };
public:
    ThreadPool(const int n_threads = 4) : m_shutdown(false),m_threads(std::vector<std::thread>(n_threads)) { };

    ThreadPool(const ThreadPool &other) = delete;
    ThreadPool operator=(const ThreadPool &other) = delete;

    ThreadPool (ThreadPool &&other) = delete;
    ThreadPool operator=(ThreadPool &&other) = delete;

    void init()
    {
        for(size_t i = 0 ; i < m_threads.size() ; i++) 
        {
            m_threads.at(i) = std::thread(ThreadWork(i,this));
        }
    }

    void shutdown()
    {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_shutdown = true;
        } 
            m_conditional_lock.notify_all();
        
        for(size_t i = 0 ; i < m_threads.size(); i++)
        {
            if(m_threads.at(i).joinable())
            {
                m_threads.at(i).join();
            }
        }
    }

    template<typename T,typename ...Args>
    auto submit(T &&t , Args ...args) -> std::future<decltype(t(args...))>
    {
        std::function<decltype(t(args...))()> func = std::bind(std::forward<T>(t), std::forward<Args>(args)...);

        auto task_ptr = std::make_shared<std::packaged_task<decltype(t(args...))()>>(func);
        
        //auto future_ptr = task_ptr->get_future();

        std::function<void()> queue_func = [task_ptr]()
        {
            (*task_ptr)();
        };
        m_queue.enqueue(queue_func);
        m_conditional_lock.notify_one();
        m_needAdjust.store(true) ;
        return task_ptr->get_future();//要用-> 获取返回值
    }

    void Adjust_threads()
    {
        if(m_shutdown)
        {
            return ;
        }
        size_t current_threads = m_threads.size();
        size_t current_tasks = m_queue.size();
        if(current_tasks>current_threads && current_tasks < m_Maxthreads.load())
        {
            size_t new_threads = std::min(current_tasks,m_Maxthreads.load());
            size_t add_threads = new_threads-current_threads;
            for(size_t i=0;i<add_threads;i++)
            {
                m_threads.emplace_back(ThreadWork(current_threads+i,this));
            }
        }
        else if(current_threads>m_Minthreads.load() && current_tasks<current_threads/2)
        {
            size_t new_threads = std::max(current_tasks,m_Minthreads.load());  
            size_t del_threads = current_threads-new_threads;
            for(size_t i=0;i<del_threads;i++)
            {
                size_t last_index = m_threads.size() - 1;
                if (m_threads[last_index].joinable())
                {
                    m_threads[last_index].join();
                }
                m_threads.pop_back(); 
            }
        }
    }
private:
    bool m_shutdown;
    TaskQueue <std::function<void(void)>> m_queue;
    std::vector<std::thread> m_threads;
    std::mutex m_mutex;
    std::condition_variable m_conditional_lock;
    std::atomic<bool> m_needAdjust;
    std::atomic<size_t> m_Maxthreads;
    std::atomic<size_t> m_Minthreads;
};
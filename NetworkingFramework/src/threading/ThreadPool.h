#pragma once
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <vector>

#define MAX_THREADS 8

/*
This call is a utility for creating and managing threads.
A program should use as few threads as possible - they are performance costly. For that purpose, we create a thread pool which allows a
certain amount of running threads. The thread pool receives tasks and executes them when a thread is free.
*/
class ThreadPool
{
private:
    static ThreadPool *s_Instance;

    using job_t = std::function<void ()>;

    /* Queue of Jobs to execute */
    std::queue<job_t> m_Jobs;
    /* Vector of all Threads running */
    std::vector<std::thread> m_Threads;
    /* Maximum amount of Threads allowed */
    size_t m_MaxThreads;
    /* Is the ThreadPool running */
    bool m_IsRunning;
    /* The Mutex of the ThreadPool */
    std::mutex m_Mutex;
    /* Condition variable used to communicate between Threads */
    std::condition_variable m_Condition;

    /*
    Pop and invoke the next job in the queue.
    This function is to be called from within a Thread.
    */
    void NextJob()
    {
        /* Lock the ThreadPool until we popped a Job
        This is done to prevent multiple Threads popping the same Job at the same time */
        std::unique_lock<std::mutex> lock(m_Mutex);
        /* If we have no Jobs queued, exit */
        if (m_Jobs.empty())
            return;
        /* Save & pop the next Job in the queue */
        job_t job = m_Jobs.front();
        m_Jobs.pop();
        /* Unlock the ThreadPool & notify another next Thread that it can run */
        lock.unlock();
        m_Condition.notify_one();
        /* Run the Job (after unlocking ThreadPool) */
        job();
    }

    /*
    The function of each Thread in the ThreadPool.
    Executes queued Jobs consecutively as long as the ThreadPool is running.
    */
    void ThreadFunction()
    {
        using namespace std::chrono_literals;

        /* As long as the ThreadPool is running */
        while (m_IsRunning)
        {
            /* Pop & execute next Job */
            NextJob();

            std::this_thread::sleep_for(100ms);
        }
    }

    /*
    Initializes all Threads of the Pool.
    The Threads start running when the Pool is created, they simply wait for Jobs to be enqueued.
    */
    void InitThreads()
    {
        return;

        /* Created specified amount of Threads (m_MaxThreads) */
        for (size_t i = 0; i < m_MaxThreads; i++)
            /* Insert a new Thread to the back of the Thread Vector */
            m_Threads.push_back(std::thread([this] { ThreadFunction(); }));
    }

    /*
    Constructs a new ThreadPool with the given amount of threads.
    */
    ThreadPool(size_t maxThreads) :
        m_MaxThreads(maxThreads),
        m_IsRunning(true)
    {
        /* Initialize all Threads when the Pool is created */
        InitThreads();
    }

public:
    static ThreadPool *GetInstance()
    {
        if (s_Instance == NULL)
            s_Instance = new ThreadPool(MAX_THREADS);

        return s_Instance;
    }

    /*
    ThreadPool deconstructor.
    Stops the Pool & joins all Threads.
    */
    ~ThreadPool()
    {
        Stop();
        Join();
    }

    /*
    Enqueue a Job to the ThreadPool.
    @param callable, the function.
    @param args, the arguments of the function.
    */
    template <typename Callable, typename... Args>
    void Enqueue(Callable &&callable, Args&&... args)
    {
        /* Wrap the Job as a void function w/o parameters, then push it to the Job Queue */
        m_Jobs.push([&] {
            /* The Job lambda is simply a call to the given function with the given parameters */
            std::forward<Callable>(callable) (std::forward<Args>(args)...);
        });

        if (m_Threads.size() < MAX_THREADS)
            m_Threads.push_back(std::thread([this] { ThreadFunction(); }));
    }

    /*
    Joins all Threads in the Pool and waits for their completion.
    */
    void Join()
    {
        /* For each Thread in the Pool */
        for (std::thread &thread : m_Threads)
            /* Join current Thread */
            thread.join();
    }

    /*
    Stops the Pool.
    This prevents all Threads from running new Jobs.
    */
    void Stop()
    {
        /* Turn the boolean to false, signfying that the Pool has stopped running.
        This boolean is used within the ThreadFunction to check whether we are running more jobs. */
        m_IsRunning = false;
    }

    /*
    @return the active Threads.
    */
    size_t ActiveCount()
    {
        return m_Threads.size();
    }
};

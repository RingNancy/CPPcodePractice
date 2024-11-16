#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono> //增加时间记录

std::mutex mtx;
std::condition_variable cond_var;

//采用原子类型，对于会经常访问到的变量进行修饰，避免产生线程竞争
std::atomic<bool> stopThread(false);
std::atomic<int> disk_capacity(0); //磁盘容量

const int maxDiskCapacity = 100;

//线程1：下载文件
void downLoadFiles()
{
    while (!stopThread)
    {
        std::unique_lock<std::mutex> lock(mtx);
        //等待另外一个线程操作，有空余空间
        cond_var.wait(lock, []{return disk_capacity < maxDiskCapacity || stopThread;});

        std::cout << "线程1正在下载文件, 请等待!" <<std::endl;
        disk_capacity += 10;
        std::cout << "当前磁盘空间已使用：" << disk_capacity <<std::endl;

        if (disk_capacity >= maxDiskCapacity)
        {
            std::cout << "磁盘空间已满！！" << std::endl;
            cond_var.notify_all();
        }
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));//模拟线程下载耗时。
    }
    
}

//线程2：对所下载的文件进行处理
void processFiles()
{
    while (!stopThread)
    {
        std::unique_lock<std::mutex> lock(mtx);
        cond_var.wait(lock, []{ return disk_capacity > 0 || stopThread;});

        std::cout << "正在处理文件......"<<std::endl;
        disk_capacity -= 10;
        std::cout << "当前磁盘空间已使用：" << disk_capacity <<std::endl;

        if (disk_capacity <= 0)
        {
            std::cout << "磁盘空间已空，暂停处理文件。" << std::endl;
            cond_var.notify_all();//通知其他线程
        }
        lock.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));//模拟线程下载耗时。
        
    }
}

int main()
{
    std::thread threadA(downLoadFiles);
    std::thread threadB(processFiles);

    std::this_thread::sleep_for(std::chrono::seconds(20));
    stopThread = true;
    cond_var.notify_all();

    threadA.join();
    threadB.join();
    return 0;
}
#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

#include "Noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

class Channel;
class Poller;
class CurrentThread;

class EventLoop : Noncopyable
{
public:
    using Functor = std::function<void()>;
    EventLoop();
    ~EventLoop();

    // 开始事件循环
    void loop();
    // 关闭事件循环
    void quit();

    Timestamp pollReturnTime() const { return pollRetureTime_; }

    // 在当前loop中执行
    void runInLoop(Functor cb);
    // 把上层注册的回调函数cb放入队列中 唤醒loop所在的线程执行cb
    void queueInLoop(Functor cb);

    // 通过eventfd唤醒loop所在的线程
    void wakeup();

    // EveneLoop的方法 => Poller的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    // 判断EventLoop对象是否在自己的线程里
    //threadId_为EventLoop创建时的线程id CurrentThread::tid()为当前线程id
    bool isInLoopThread()const { return threadId_ == CurrntThread::tid(); }

private:
    void handleRead(); // 给eventfd返回的文件描述符wakeupFd_绑定的事件回调 当wakeup()时 即有事件发生时 调用handleRead()读wakeupFd_的8字节 同时唤醒阻塞的epoll_wait
    void doPendingFunctors(); // 执行上层回调

    using ChannelList = std::vector<Channel *>;
    
    std::atomic_bool looping_; // 表示事情循环是否在发生，决定是否执行loop()
    std::atomic_bool quit_;    // 标识退出loop循环

    const pid_t threadId_;     // 记录当前Eentloop是被哪个线程id创建的，用来判断当前操作是否发生在事件循环所在的线程
    
    Timestamp pollRetureTime_; // Poller返回发生事件的Channels的时间点
    std::unique_ptr<Poller> poller_; // 一个 Poller 的智能指针，负责底层的 I/O 多路复用，如 epoll。Poller 负责监听多个通道上的事件

    int wakeupFd_;// eventfd 文件描述符，用于唤醒阻塞中的事件循环线程。通常用于跨线程通知事件循环进行处理
    std::unique_ptr<Channel> wakeupChannel_; // wakeupFd_ 对应的 Channel 对象，封装了文件描述符，供 Poller 使用

    ChannelList activeChannels_; // 返回Poller检测到当前有事件发生的所有Channel列表

    std::atomic_bool callingPendingFunctors_; // 标识当前loop是否有需要执行的回调操作
    std::vector<Functor> pendingFunctors_; // 存储loop需要执行的所有回调操作
    std::mutex mutex_;  // 互斥锁 用来保护上面vector容器的线程安全操作

};
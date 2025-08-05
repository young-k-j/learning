#pragma once
//相当于epoll的event
#include <functional>
#include <memory>

#include "Noncopyable.h"
#include "Timestamp.h"

class EventLoop;

/**
 * 理清楚 EventLoop、Channel、Poller之间的关系  Reactor模型上对应多路事件分发器
 * Channel理解为通道 封装了sockfd和其感兴趣的event 如EPOLLIN、EPOLLOUT事件 还绑定了poller返回的具体事件revent
 **/
class Channel : Noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;
    
    Channel(EventLoop *loop, int fd);//初始化channel，传入事件循环loop和fd
    ~Channel();

    /*fd得到Poller通知以后 处理事件 handleEvent在EventLoop::loop()中调用
      handleEent会根据实际事件revents_ 决定执行哪个回调函数*/
    void handleEvent(Timestamp receiveTime);

    // 设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); } // 读
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }   // 写
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }   // 关闭
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }   //报错

    // 防止Channel被手动remove掉 channel汉在执行回调操作
    void tie(const std::shared_ptr<void>&);

    void set_revents(int rect) { revents_ = rect; }// 设置从 Poller 返回的实际事件 revents_

    // 设置fd相应的事件状态 相当于epoll_ctl add delete
    void enableReading() { events_ |= kReadEvent; update(); }
    void disableReading() { events_ &= ~kReadEvent; update(); }
    void enableWriting() { events_ |= kWriteEvent; update(); }
    void disableWriting() { events_ &= ~kWriteEvent; update(); }
    void disableAll() { events_ = kNoneEvent; update();}

    // 返回fd当前的事件状态
    bool isNoneEvent() const { return events_==kNoneEvent; }//检查当前 Channel 是否没有注册任何事件
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent;}
    
    //一些成员变量接口
    int fd() const{ return fd_; };  // 获取 Channel 关联的文件描述符
    int events() const { return events_; }// 获取当前 Channel 注册的事件类型
    int index() { return index_; }
    void set_index(int idx) { index_ = idx; }
    EventLoop *ownerLoop() { return loop_; }// one loop per thread

    void remove();  //删除channel 

private:
    void update();//更新 Channel 在 Poller 中的注册状态，通常用于 epoll_ctl 操作
    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent; 
    static const int kReadEvent;  //等于 EPOLLIN
    static const int kWriteEvent; //等于 EPOLLOUT

    EventLoop *loop_; // 事件循环 每个channel只能属于一个事件循环
    const int fd_;    // fd，Poller监听的对象 poller：轮询器
    int events_;      /* 注册到poller的事件集合（例如：EPOLLIN/EPOLLOUT）。
                       这个变量决定channel对哪些事件感兴趣*/
    int revents_;     // Poller返回的实际发生的事件
    int index_;       // 用于标识通道在事件循环中的位置或者顺序

    std::weak_ptr<void> tie_;// 用于管理 Channel 对象的生命周期，以避免循环引用
    bool tied_; //标记 tie_ 是否有效，通常用于确保 Channel 被移除时没有回调被执行。
    
    //回调函数
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;  
};

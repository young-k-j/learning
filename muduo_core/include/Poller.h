#pragma once
/*负责监听多个文件描述符（如网络套接字）并在某个文件描述符准备好进行某种 I/O 操作时通知应用程序*/
#include <vector>
#include <unordered_map>

#include "Noncopyable.h"
#include "Timestamp.h"

class Channel;
class EventLoop;

//这里要不要继承Noncopyable???????????????
class Poller
{
public:
    using ChannelList = std::vector<Channel *>; // 用于存储多个Channel 指针
    Poller(EventLoop *loop);//传入的 EventLoop 指针，表示当前 Poller 所属的事件循环
    virtual ~Poller() = default;

    // 给所有IO复用保留统一的接口
    /* 这是一个纯虚函数，表示所有继承自 Poller 的具体实现类必须提供一个 I/O 复用的实现（如 epoll、select 或 poll）。
    该函数接受一个超时时间（timeoutMs），并返回一个 Timestamp，表示最后一次 I/O 事件发生的时间。*/
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels) = 0;
    virtual void updateChannel(Channel *channel) = 0;
    virtual void removeChannel(Channel *channel) = 0;

    // 判断参数channel是否在当前的Poller当中
    bool hasChannel(Channel *channel) const;

    // EventLoop可通过该接口获取默认的IO复用的具体实现
    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    // map的key:sockfd, value:sockfd所属的channel通道类型
    using ChannelMap = std::unordered_map<int, Channel *>;
    /* 它的键是文件描述符（int 类型），值是与文件描述符关联的 Channel 对象。
    通过这个数据结构，Poller 可以在 I/O 复用时快速定位每个文件描述符
    所对应的 Channel，从而调用回调函数处理其事件。*/
    ChannelMap channels_;
private:
    EventLoop *ownerLoop_; // 定义Poller所属的事件循环EventLoop
};
#include <sys/epoll.h>

#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;//读事件 EPOLLPRI 优先数据可读事件
const int Channel::kWriteEvent = EPOLLOUT; //写事件

// 含参初始化
Channel::Channel(EventLoop *loop, int fd)
    : loop_(loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , index_(-1) // 表示这是一个新的 Channel，还未添加到 Poller
    , tied_(false)
{
}

Channel::~Channel()
{
}

// channel的tie方法什么时候调用过?  TcpConnection => channel
/**
 * TcpConnection中注册了Channel对应的回调函数，传入的回调函数均为TcpConnection
 * 对象的成员方法，因此可以说明一点就是：Channel的结束一定晚于TcpConnection对象！
 * 此处用tie去解决TcpConnection和Channel的生命周期时长问题，从而保证了Channel对象能够在
 * TcpConnection销毁前销毁。
 **/

/*绑定一个对象，确保该对象的生命周期至少与 Channel 相同
这通常用于处理 TcpConnection 对象与 Channel 之间的生命周期管理问题。*/

/*为什么需要？：Channel 负责监听 I/O 事件，TcpConnection 负责管理连接的状态和数据交换。
tie 方法可以保证，在 TcpConnection 被销毁前，Channel 不会被销毁。
否则，如果 Channel 提前销毁，回调函数可能会在一个已经销毁的对象上执行，从而导致未定义行为.*/
void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_ = obj;
    tied_ = true;
}
//update 和remove => EpollPoller 更新channel在poller中的状态
/**
 * 当改变channel所表示的fd的events事件后，update负责再poller里面更改fd相应的事件epoll_ctl
 **/
void Channel::update()
{
    // 通过channel所属的eventloop，调用poller的相应方法，注册fd的events事件
    loop_->updateChannel(this);
}

// 在channel所属的EventLoop中把当前的channel删除掉
void Channel::remove()
{
    loop_->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    if(tied_){
        std::shared_ptr<void> guard = tie_.lock();
        if(guard)//说明对象还活着
        {
            handleEventWithGuard(receiveTime);
        }
        // 如果提升失败了 就不做任何处理 说明Channel的TcpConnection对象已经不存在了
    }
    else{
        handleEventWithGuard(receiveTime);
    }
}

void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents:%d\n", revents_);
    // 关闭, EPOLLHUP 表示挂起事件，通常是连接被对方关闭的标志。
    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))
    {
        if(closeCallback_)
        {
            closeCallback_();
        }
    }
    //错误
    if(revents_ & EPOLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }
    //读
    if(revents_ & (EPOLLIN|EPOLLPRI))
    {
        if(readCallback_)
        {
            readCallback_(receiveTime);
        }
    }
    //写
    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_){
            writeCallback_();
        }
    }    
}



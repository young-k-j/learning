#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"

const int kNew = -1;    //某个channel还没添加到Poller // channel的成员index_初始化为-1
const int kAdded = 1;   //某个channel已经添加到Poller
const int kDeleted = 2; //某个channel已从Poller删除

EPollPoller :: EPollPoller(EventLoop *loop)
    : Poller(loop) //这里没看明白
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC)) //注意EPOLL_CLOEXEC
    , events_(kInitEventListSize)//vector<epoll_event>(16)
{
    if(epollfd_ < 0)
    {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

Timestamp EPollPoller::poll(int timeoutMs, ChannelList *activeChannels){
    // 由于频繁调用poll 实际上应该用LOG_DEBUG输出日志更为合理 当遇到并发场景 关闭DEBUG日志提升效率
    LOG_INFO("func=%s => fd total count:%lu\n", __FUNCTION__, channels_.size());

    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(), static_cast<int>(events_.size()),timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());//now是当前时间的时间辍

    if (numEvents > 0)
    {
        LOG_INFO("%d events happened\n",numEvents); // LOG_DEBUG最合理
        fillActiveChannels(numEvents, activeChannels);
        if(numEvents == events_.size())//扩容
        {
            events_.resize(events_.size()*2);
        }
    }
    else if (numEvents == 0)
    {
        LOG_DEBUG("%s timeout!\n",__FUNCTION__);
    }
    else
    {
        //EINTR,表示系统调用被信号中断,如果不是EINTR就会报错，并重置errno
        if(saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_ERROR("EPollPoller::poll() error!");
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel* channel)
{
    const int index = channel->index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d\n", __FUNCTION__, channel->fd(), channel->events(), index);

    if(index == kNew || index == kDeleted)
    {
        if(index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        else
        {
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd = channel->fd();
        if(channel->isNoneEvent()){//没注册任何事件 删除
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }
        else
        {
            update(EPOLL_CTL_MOD,channel);
        }
    }
}

// 从Poller中删除channel
void EPollPoller::removeChannel(Channel *channel)
{
    int fd = channel->fd();
    channels_.erase(fd);

    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);

    int index = channel->index();
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    //为了保证通道未来重新加入监听的可能性
    channel->set_index(kNew);
}

void EPollPoller::fillActiveChannels(int numEvents, ChannelList *activeChannels)const
{
    for(int i = 0; i < numEvents; ++i)
    {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
         // EventLoop就拿到了它的Poller给它返回的所有发生事件的channel列表了
        activeChannels->push_back(channel);
    }
}

// 更新channel通道 其实就是调用epoll_ctl add/mod/del
void EPollPoller::update(int operation, Channel *channel)
{
    epoll_event event;
    ::memset(&event, 0, sizeof(event));

    int fd = channel->fd();
    
    event.events = channel->events();
    event.data.fd = fd;
    event.data.ptr = channel;

    if(::epoll_ctl(epollfd_, operation, fd, &event) < 0){
        if(operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else
        {
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}




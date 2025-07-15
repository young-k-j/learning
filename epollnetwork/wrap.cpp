#include "wrap.h"

void sys_err(const char *str){
    perror(str);
    exit(1);
}

//封装错误处理函数
int Socket(int domian, int type, int protocol){
    int n = socket(domian, type, protocol);
    if(n == -1){
        sys_err("socket error!\n");
    }
    return n;
}

// 绑定bind
int Bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen){
    int n = bind(sockfd,addr,addrlen);
    if(n == -1){
        sys_err("bind error\n");
    }
    return n;
}

// 接受连接accept
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen){
    int n = accept(sockfd, addr, addrlen);
again:
    if(n == -1){
        //​​EINTR​​：系统调用被信号中断（如进程收到 SIGCHLD）
        //ECONNABORTED​​：客户端在三次握手完成前终止连接（如超时或主动关闭）
        if ((errno == ECONNABORTED) || (errno == EINTR))// 判断是否为可恢复错误
	        goto again; // 跳转重试
        else
	        sys_err("accept error");
    }
    return n;
}

//请求连接connect
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen){
    int n = connect(sockfd, addr,addrlen);
    if(n == -1){
        sys_err("connect error\n");
    }
    return n;
}
//监听listen
int Listen(int sockfd, int backlog){
    int n = 0;
    n = listen(sockfd,backlog);
    if(n == -1){
        sys_err("listen error\n");
    }
    return n;
}

//读（接收信息）
ssize_t Read(int fd, void *buf, size_t count){
    ssize_t n = read(fd, buf, count);
again:
    if(n == -1){
        if(errno == EINTR)
            goto again;
        else
            return n;
    }
    return n;   
}

//写（发送信息）
ssize_t Write(int fd, const void *ptr, size_t nbytes){
    ssize_t n = write(fd, ptr, nbytes);
again:
    if(n == -1){
        if(errno == EINTR)
            goto again;
        else
            return n;
    }
    return n;  
}

//关闭
int Close(int fd)
{
    int n;
	if ((n = close(fd)) == -1)
		sys_err("close error");

    return n;
}
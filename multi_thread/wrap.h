#ifndef _WRAP_H_
#define _WRAP_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctype.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <fcntl.h>

// 错误处理
void sys_err(const char *str);

// 套接字socket
int Socket(int domian, int type, int protocol);

// 绑定bind
int Bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen);

// 接受连接accept
int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

//请求连接connect
int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

// 监听
int Listen(int sockfd, int backlog);

//读（接收信息）
ssize_t Read(int fd, void *buf, size_t count);

//写（发送信息）
ssize_t Write(int fd, const void *ptr, size_t nbytes);

//关闭
int Close(int fd);

#endif
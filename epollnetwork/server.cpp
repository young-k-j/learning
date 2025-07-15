#include "wrap.h"

#define MAXSIZZ 1024
#define OPENFILE 1024
#define SERV_PORT 8000

int main(int argv, char *argc[]){
    int lfd, cfd, efd, sockfd, res, nready, flag, i, j, n;
    struct sockaddr_in serv_addr, clit_addr;
    socklen_t clit_len;
    char buf[MAXSIZZ], str[MAXSIZZ];
    clit_len = sizeof(clit_addr);

    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = Socket(AF_INET, SOCK_STREAM, 0);
    //启用端口复用
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));

    //绑定
    Bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    //监听
    Listen(lfd, 128);

    //开始创建epoll
    efd = epoll_create(OPENFILE);
    if(efd == -1){
        sys_err("epoll_create error \n");
    }

    struct epoll_event tep, teps[OPENFILE];
    tep.data.fd = lfd;
    tep.events = EPOLLIN; //lfd监听事件为 读
    res = epoll_ctl(efd, EPOLL_CTL_ADD, lfd, &tep);
    if(res == -1){
        sys_err("epoll_ctl error \n");
    }
    int num = 0;

    while(1){
        nready = epoll_wait(efd, teps, OPENFILE, -1);
        if(nready == -1){
            sys_err("epoll_wait error \n");
        }

        for(i = 0; i < nready; i++){
            if(teps[i].data.fd == lfd){ //添加新服务器fd
                if (!(teps[i].events & EPOLLIN))      //如果不是"读"事件, 继续循环
                    continue;

                cfd = Accept(lfd, (struct sockaddr*)&clit_addr, &clit_len);
                flag = fcntl(cfd, F_GETFL);
                flag |= O_NONBLOCK;
                fcntl(cfd, F_SETFL, flag); 
                if(fcntl(cfd, F_SETFL, flag) == -1){
                    sys_err("fcntl error \n");
                }

                printf("received from %s at PORT %d\n", 
                    inet_ntop(AF_INET, &clit_addr.sin_addr, str, sizeof(str)), 
                    ntohs(clit_addr.sin_port));
                printf("cfd %d---client %d\n", cfd, ++num);

                tep.data.fd = cfd;
                tep.events = EPOLLIN;
                res = epoll_ctl(efd, EPOLL_CTL_ADD, cfd, &tep);
                if(res == -1){
                    sys_err("epoll_ctl error \n");
                }
            }else{
                sockfd = teps[i].data.fd;
                n = Read(sockfd, buf, sizeof(buf));
                if(n < 0){
                    if(errno != EAGAIN){
                        perror("read n < 0 error: ");
                    }
                    continue;
                }else if(n == 0){
                    res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                    if(res == -1){
                        sys_err("epoll_wait error \n");
                    }
                    Close(sockfd);
                }else{
                    for(j = 0; j < n; j++){
                        buf[j] = toupper(buf[j]);
                    }
                    Write(sockfd, buf, strlen(buf));
                    Write(STDOUT_FILENO, buf, strlen(buf));
                }
            }
        }
    }
    Close(lfd);
    return 0;
}
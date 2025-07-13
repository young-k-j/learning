#include "wrap.h"
#include <signal.h>

#define SREV_PORT 9878


struct s_info{
    struct sockaddr_in clit_addr;
    int cfd;
};

void *do_work(void *arg){
    int n, i;
    struct s_info *ts = (struct s_info*)arg;
    char buf[BUFSIZ];
    char str[INET_ADDRSTRLEN];

    while(1){
        n = Read(ts->cfd,buf,sizeof(buf));
        //n==0说明对端已经关闭
        if(n == 0){
            printf("the client %d closed...\n", ts->cfd);
            break;
        }
        printf("received from %s at PORT %d\n",
                inet_ntop(AF_INET, &(*ts).clit_addr.sin_addr.s_addr, str, sizeof(str))),
                ntohs((*ts).clit_addr.sin_port);

        for(i = 0; i < n; i++){
            buf[i] = toupper(buf[i]);
        }
        Write(STDOUT_FILENO, buf, n);
        Write(ts->cfd, buf, n);
    }
    Close(ts->cfd);
    free(ts);
    return (void*)0;
}


int main(int argc, char *argv[]){
    int lfd,cfd;
    struct sockaddr_in serv_addr, clit_addr;
    socklen_t clit_len = sizeof(clit_addr);
    pthread_t tid;

    struct s_info ts;
    int i=0;
    bzero(&serv_addr,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SREV_PORT);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = Socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, (void*)&opt, sizeof(opt));

    Bind(lfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    Listen(lfd, 128);

    printf("Accepting client connect ...\n");

    while(1){
        clit_len = sizeof(clit_addr);
        cfd = Accept(lfd, (struct sockaddr*)&clit_addr, &clit_len);
        struct s_info* ts = (struct s_info*)malloc(sizeof(struct s_info));

        ts->clit_addr = clit_addr;
        ts->cfd = cfd;
        pthread_create(&tid, nullptr, do_work, (void*)ts);
        pthread_detach(tid); //线程分离
    }
    Close(lfd);
    return 0;
}
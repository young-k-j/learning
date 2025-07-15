#include "wrap.h"

#define MAXSIZZ 1024
#define OPENFILE 1024
#define SERV_PORT 8000

int main(int argc, char *argv[]){
    int cfd, n;
    char buf[MAXSIZZ];
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET,"127.0.0.1", &serv_addr.sin_addr);

    cfd = Socket(AF_INET, SOCK_STREAM, 0);

    Connect(cfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    while(fgets(buf, OPENFILE, stdin) != NULL){
        Write(cfd, buf, strlen(buf));
        n = Read(cfd, buf, OPENFILE);
        if (n == 0) {
            printf("the other side has been closed.\n");
            break;
        }       
        else
            Write(STDOUT_FILENO, buf, n);
    }
    Close(cfd);
    return 0;
}
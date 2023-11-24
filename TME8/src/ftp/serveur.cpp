#include <type.h>
#include <iostream>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>

#define PATH_M 4096
using namespace std;
//beej.us/guide ref


int main(int argc, char ** argv){
    int opt = 1;
    struct sockaddr_in addr;
    struct sockaddr_in addr_client;
    socklen_t addr_len_client;
    int fd_sock;
    int fd_sock_c;
    char buff[100];
    if(argc != 3){
        perror("Erreur : mauvais nombre d'argument");
        exit(EXIT_FAILURE);
    }   

    uint16_t port = atoi(argv[1]);
    char pathname[PATH_M ];
    strcpy(pathname,argv[2]);
    

    if((fd_sock = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }


    if (setsockopt(fd_sock, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    //this socket will use the port : port we need to specify 
    //for the server, which port we take
    //No need for the client because he doesn't choose the port 
    if (bind(fd_sock,(struct sockaddr*) &addr,sizeof(addr))< 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //0 is the length of the queue of client waiting for my accept
    if(listen(fd_sock,10) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }

    if((fd_sock_c = accept(fd_sock,(struct sockaddr*) &addr_client,&addr_len_client)) < 0){

        perror("accept");
        exit(EXIT_FAILURE);
    }

    




}
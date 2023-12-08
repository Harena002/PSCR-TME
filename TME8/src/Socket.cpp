#include "Socket.h"



using namespace pr;
using namespace std;


void Socket::connect(const std::string & host, int port){
    struct in_addr ipV4;
    struct addrinfo *adr;           //return a list of  adress for the host
    if(getaddrinfo(host.c_str(),nullptr,nullptr,&adr) != 0){
        perror(" DNS");
    }

    for (struct addrinfo *rp = adr; rp != nullptr ; rp = rp -> ai_next){

        if(rp -> ai_family == AF_INET && rp -> ai_socktype == SOCK_STREAM){
            ipV4 = ((struct sockaddr_in * ) (rp -> ai_addr)) -> sin_addr;
            break;
        }
    }

    freeaddrinfo(adr);
    connect (ipV4,port);
}



 void Socket::connect(in_addr ipv4, int port){
    fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd < 0 ){
        perror (" Socket ");
    }

    struct sockaddr_in server;
    server . sin_family = AF_INET;
    server . sin_port = htons (port);
    server . sin_addr = ipv4;
    if(::connect(fd , (struct sockaddr *) &server,sizeof(server)) < 0){
        fd = -1;
        perror("Connect");
    }
 }

 void Socket::close(){

    if(fd != -1){
        ::shutdown(fd,2);
        ::close(fd);
        
        fd = -1;
    }

 }



std::ostream & operator<< (std::ostream & os, struct sockaddr_in * addr){
    char IP_adrr[20];
    char host[1024];
    if(getnameinfo((struct sockaddr *) addr,sizeof(struct sockaddr_in),host,1024,nullptr,0,0 )){
        cout <<  "host : " << host << endl;
    }

    //innet_ntoa -> transforme l'adresse IP  décimal : en a.b.c.d
    cout<< inet_ntoa(addr->sin_addr) << " : " << ntohs(addr->sin_port);   
    
}
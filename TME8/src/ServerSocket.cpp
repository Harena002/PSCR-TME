#include "ServerSocket.h"


using namespace pr;
using namespace std;

ServerSocket :: ServerSocket(int port){

    int i = 1;
    socketfd = socket(AF_INET,SOCK_STREAM,0);
    if(socketfd < 0 ) perror(" Socket ");
    struct sockaddr_in adr; 
    adr.sin_family = AF_INET;
    adr.sin_port = htons(port);
    adr.sin_addr.s_addr = INADDR_ANY;
    if(::bind(socketfd,(struct sockaddr *)&adr,sizeof(adr)) < 0 ) perror("Bind ");
    if(setsockopt(socketfd,SOL_SOCKET,SO_REUSEPORT,&i,sizeof(i)) < 0){
        perror("Set Socket Option");
    }
}

Socket ServerSocket :: accept (){
    struct sockaddr_in exp;
    socklen_t l;
    int x  = ::accept(socketfd,(struct sockaddr*)&exp,&l);
    //cout << exp << " is connected";

}

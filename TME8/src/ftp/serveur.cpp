#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/dir.h>
#include <dirent.h>
#include <signal.h>
#include <sys/errno.h>
#include <string>
#define PACKET_SIZE 4096
#define PATH_M 4096
using namespace std;
//beej.us/guide ref
bool doIt = true;


void hand_SIGINT(int n){
	doIt = false;
}

void set_handler(){
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = hand_SIGINT;
	sigaction(SIGINT,&sa,NULL);
}

void list(int fd_sock_c, char *dir){   //exec portnumber directory 
    DIR * d = opendir(dir);
    struct dirent * entry;
    while((entry = readdir(d))){
        if(strcmp(entry->d_name,".") == 0) continue;
        if(strcmp(entry->d_name,"..") == 0) continue;
        if(entry->d_type != DT_REG) continue;
        send(fd_sock_c,"\n",1,0);
        send(fd_sock_c,entry->d_name,entry->d_namlen,0);
    }
    send(fd_sock_c,"\n\r",2,0);
}



void download(int fd_sock_c,char *dir,char *file){
    DIR * di = opendir(dir);
    struct dirent *entry;
    int d = 0;
    while((entry = readdir(di))){
        if(strcmp(entry->d_name,".") == 0) continue;
        if(strcmp(entry->d_name,"..") == 0) continue;
        if(strncmp(entry->d_name,file,entry->d_namlen) != 0) continue;
        d++;
        break ;
    } 
    
    if(d == 0){
        send(fd_sock_c,"1\n\n",1,0);                //the client will handle the error : the file doesn't exist
        cout<< "error : filename doesn't exist"<<endl;
        return;
    }
   
    string dirS = dir;
    string fileS = file;
    string path = dirS + "/" + fileS;
    ifstream f(path.c_str());

    if (!f.is_open()) {
        std::cerr << "Error opening file: " << path << std::endl;
        return;
    }

    string line ;
    while(getline(f,line)){
        send(fd_sock_c,line.c_str(),line.size(),0);
        send(fd_sock_c,"\n",1,0);
    }
    send(fd_sock_c,"\n\r",2,0);
 
    f.close();
}



void upload(int fd_sock, char * dir,char *file){
    
    string dirS = dir;
    string fileS = file;
    string path = dirS + "/" + fileS;
    bool doIt2 = true;
    char buff2[1024];
    int lgth;
    lgth = recv(fd_sock,buff2,1023,0);
    buff2[lgth] = '\0';

    if(strncmp(buff2,"1\n\n",lgth) == 0){
        return;
    }

    ofstream f(path.c_str());
    if (!f.is_open()) {
        std::cerr << "Error opening file: " << path << std::endl;
        return;
    }

    while(doIt2){
        f<<buff2;
        if(strncmp((buff2+(lgth-2)),"\n\r",2) == 0){
            doIt2 = false;
        }
        else{
            memset(buff2,0,1024);
            lgth = recv(fd_sock,buff2,1023,0);
        }
    }
    
    f.close();
}




int main(int argc, char ** argv){
    set_handler();
    bool compute = true;
    int opt = 1;
    struct sockaddr_in addr;
    struct sockaddr_in addr_client;
    socklen_t addr_len_client;
    int fd_sock;
    int fd_sock_c;
    char buff[PACKET_SIZE];
    int lgth;
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


    if (setsockopt(fd_sock, SOL_SOCKET,SO_REUSEPORT, &opt,sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    //this socket will use the port : port we need to specify 
    //for the server : which port we take
    //No need for the client because he doesn't choose the port 
    //::bind() otherwise, the compiler will use the std::bind !!!
   if (::bind(fd_sock, (struct sockaddr*)&addr,sizeof(addr)) < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
   }

    


    //0 is the length of the queue of client waiting for my accept
    //listen is passif - non bloquant
   
    //char opt[10];
    if(listen(fd_sock,10) < 0){
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while(doIt){
       
        //Bloquant
        cout<<"Waiting for connexion"<<endl;
        compute = true;
        if((fd_sock_c = accept(fd_sock,(struct sockaddr*) &addr_client,&addr_len_client)) < 0){
            shutdown(fd_sock,2);
            close(fd_sock);
            if(errno != EINTR && errno != 57  ){
                perror("accept");
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS); 
        }
        while(compute){
            lgth = recv(fd_sock_c,buff,PACKET_SIZE-1,0);
            buff[lgth] = '\0';
            if((strncmp(buff,"LIST",4) == 0) || (strncmp(buff,"list",4) == 0)){
                list(fd_sock_c,argv[2]);
            }

            if((strncmp(buff,"UPLOAD",6) == 0) || (strncmp(buff,"upload",6) == 0)){
                upload(fd_sock_c,argv[2],buff+7);
            }
        
            if((strncmp(buff,"DOWNLOAD",8) == 0) || (strncmp(buff,"download",8) == 0)){
                download(fd_sock_c,argv[2],buff+9);                      // client side : download file
            }

            if((strncmp(buff,"QUIT",4) == 0) || (strncmp(buff,"quit",4) == 0)){
                cout <<"shutting down the client connexion"<<endl;
                compute = false;
                shutdown(fd_sock_c,2);
                close(fd_sock_c);       
            }
        }
    }

    
    shutdown(fd_sock,2);
    close(fd_sock);
    return 0;

}
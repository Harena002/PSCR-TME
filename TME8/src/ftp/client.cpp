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
using namespace std;



void list(int fd_sock){  
    bool doIt = true;
    char buff[PACKET_SIZE];
    int lgt = recv(fd_sock,buff,PACKET_SIZE-1,0);
  
    while(doIt){
        
        buff[lgt] = '\0';
        cout<<buff<<endl;
        
        if(strncmp((buff+(lgt-2)),"\n\r",2) == 0){
            doIt = false;
        }
        else{
            memset(buff,0,PACKET_SIZE);
            lgt = recv(fd_sock,buff,PACKET_SIZE-1,0);
        }
    }
        
}


void upload(int fd_sock){
    char buff[256];
    cout << "file to Upload > ";
    cin.get(buff,256);
    if (send(fd_sock,buff,strlen(buff),0) == -1) {
        perror("write"); 
        exit(EXIT_FAILURE);
    }
}

void download(int fd_sock,char *file){
    char buff[256];
    char buff2[1024];

    bool doIt = true;
    int lgth;


    lgth = recv(fd_sock,buff2,1023,0);
    buff2[lgth] = '\0';
    if(strncmp(buff2,"1\n\n",lgth) == 0){
        cout << "The file : "<<file<<" doesn't exist"<<endl;
        return;
    }
    ofstream  f(file);
    if (!f.is_open()) {
        cerr << "Error opening file: " << buff << std::endl;
        return;
    }
    
    while(doIt){
        f<<buff2;
        if(strncmp((buff2+(lgth-2)),"\n\r",2) == 0){
            doIt = false;
        }
        else{
            memset(buff2,0,1024);
            lgth = recv(fd_sock,buff2,1023,0);
        }
    }

    f.close();

}

void upload (int fd_sock, char *file){

    DIR * di = opendir(".");
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
        send(fd_sock,"1\n\n",1,0);                //the client will handle the error : the file doesn't exist
        cout<< "error : filename doesn't exist"<<endl;
        return;
    }
   
    
    string fileS = file;
    string path = "./" + fileS;
    ifstream f(path.c_str());

    if (!f.is_open()) {
        std::cerr << "Error opening file: " << path << std::endl;
        return;
    }

    string line ;
    while(getline(f,line)){
        send(fd_sock,line.c_str(),line.size(),0);
        send(fd_sock,"\n",1,0);
    }
    send(fd_sock,"\n\r",2,0);
    f.close();
}


int main(int argc, char **argv){   //exec SERVERIP PORTIP
    bool doIt = true;
    int fd_sock;
    struct sockaddr_in dest;
    uint16_t port_serveur = atoi(argv[2]);
    string buffS;
    if(argc != 3){
        perror("Erreur : mauvais nombre d'argument");        
        exit(EXIT_FAILURE);
    }  

    if((fd_sock = socket(AF_INET,SOCK_STREAM,0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

        
    if (inet_pton(AF_INET, argv[1], &dest.sin_addr)<= 0) {  //don't forget this . convert the a.b.c.d adress from the terminal to an IP adress exploitable
        perror( "Invalid address/ Address not supported ");
        exit(EXIT_FAILURE);
    }
    dest.sin_family = AF_INET;
    dest.sin_port = htons(port_serveur);

    if (connect(fd_sock, (struct sockaddr *) &dest, sizeof(dest)) == -1) {
        perror("connect"); 
        exit(EXIT_FAILURE);
    }

    while(doIt){
        cout<<"FTP > ";
        getline(cin, buffS);
        char * buff = strdup(buffS.c_str());
        if (send(fd_sock,buff,buffS.size()+1,0) == -1) {
            perror("write"); 
            exit(EXIT_FAILURE);
        }
        
        if((strncmp(buff,"LIST",4) == 0) || (strncmp(buff,"list",4) == 0)){
            list(fd_sock);
            free(buff);
        }

        else if((strncmp(buff,"UPLOAD",6) == 0) || (strncmp(buff,"upload",6) == 0)){
            upload(fd_sock,buff+7);
            free(buff);
            
        }
        
        
        else if((strncmp(buff,"DOWNLOAD",8) == 0) || (strncmp(buff,"download",8) == 0)){
            download(fd_sock,buff+9);   
            free(buff);
            
        }
        else if((strncmp(buff,"QUIT",4) == 0) || (strncmp(buff,"quit",4) == 0)){
            doIt = false;
            free(buff);
            shutdown(fd_sock,2);
            close(fd_sock);
        }
        else {
            cerr << "Error: command not find"<<endl;
        }
    }

    return 0;
}
#include "chat_common.h"
#include <cstdio>
#include <iostream>



#define MAX_COM 10
using namespace std;
struct myshm *buff;
struct {
	int id_client;
	struct message *seg;
	sem_t *sem;
}com[MAX_COM];


void ask_connexion(struct message *mess){ //connexion message format : id file_name_shm_client file_sem_client
	char * ctt = mess -> content;
	int id;
	char buff[50];
	char buff2[50];
	sscanf(ctt,"%d %s %s",&id,buff,buff2);
	int fd_shm;
	void * space;
	struct message *shm_mess;
	sem_t *sem = sem_open(buff2,O_RDWR);

	if((fd_shm = shm_open(buff,O_RDWR,0600)) == -1){
	    perror("shm_open client");
	    return;
	}

        if((space = mmap(0,sizeof(message), PROT_READ|PROT_WRITE,MAP_SHARED,fd_shm,0)) == MAP_FAILED){
	    perror("MMAP");
            return;
	}
	shm_mess = static_cast <struct message*> (space);
	
	for (int i = 0; i<MAX_COM; i++){

	    if(com[i].id_client !=0)
		   continue;
	    
	   com[i].id_client = id;
	   com[i].seg = shm_mess;
	   com[i].sem = sem;
	   id = -1;
	   break; 
	}
	if(id != -1){     //Add feature to do a loop until to find a place into com 
			  //Use multi-thread so when a thread treats a deconnexion, it could notify a thread which is sleeping here because he didn't find place into com
		perror("Connexion Failed");
		return;
	}	
	


}



void read_request(){
	int n = 0;
	int r = buff -> read;
	int w = buff -> write;
	struct message *mess;
	//to find which request we didn't answer yet
	//Add feature that we transform it into while(doIt) 
	//If n == MAX_MESS the client would put his message over message we already compute 
	while ((r < n) || (n < MAX_MESS)){

	    sem_wait(&(buff->sem));
	    n = (buff -> nb);
	    sem_post(&(buff->sem));

	    buff->write = n-r;

	    if(r == n){            //If we compute all requests but still have place into our buffer
                 continue;
            }


	    mess = &(buff -> messages [n]);
	    switch(mess -> type){	
	        case 0 : 
	            //ask_connexion(mess);
		    break;
	        case 1 :
		    //send_message(mess);
		    break;
	        case 2 : 
		    //ask_deconnexion(mess);
		    break;
	     }
	    r++;
	}
}


int main(int argc,char ** argv){

	int fd_shm; 
	void *space;
	cout<<argv[1]<<endl;
	if((fd_shm = shm_open(argv[1],O_RDWR|O_CREAT,0600)) == -1){
	    perror("shm_open serveur ");
	    return 1;
	}	

	if(ftruncate(fd_shm,sizeof(myshm)) == -1){
	    perror("ftruncate");
	    return 1;
	}
	
	if((space = mmap(0,sizeof( myshm), PROT_READ|PROT_WRITE,MAP_SHARED,fd_shm,0)) == MAP_FAILED){
	    perror("MMAP");
            return 1;
	}
	//to convert the adress of the shared memory into a pointer of a structure (a pointer to the shared memory)
	//just to be clean
	
	buff = static_cast <struct myshm*> (space);
	buff -> nb = 0;
	buff -> read = -1;
	buff -> write = 0;
	sem_init(&(buff->sem),1,1);
        

        read_request();	
	sleep(20);

	sem_destroy(&(buff->sem));
	munmap(space,sizeof(myshm));
	shm_unlink(argv[1]);













	return 0;
}

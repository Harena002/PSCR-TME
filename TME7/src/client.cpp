#include "chat_common.h"
#include <string.h>
#include <cstring>
#include <string>
#include <iostream>


using namespace std;

sem_t *sem_client;
struct message *shm_mess_client;
struct myshm *shm_serv;
const char *sem_name;
char *shm_client;
int id_client;


int connection(){           //Return 0 if everything fine, else != 0 

	sem_wait(&(shm_serv->sem));  
	int id = shm_serv -> nb++;
        sem_post(&(shm_serv->sem));   
	//Release the semaphore just after incrementing nb, because it will give the index to put our message and it is a data race with the 
	//array messages, client doesn't touch read, write data into myshm structure

	if(id > MAX_MESS){
		cout<<"Server full"<<endl;
		return 1;
	}

	struct message m;
	m.type = 0;
	snprintf(m.content,TAILLE_MESS-1,"%d %s %s",id_client,shm_client,sem_name);
	shm_serv -> messages[id] = m;
	return 0;

}






int main(int argc, char **argv){    //client shm_client shm_serv id_client
	int fd_shm;
	int fd_shm2;
	void * space;
	void *space2;
	id_client = atoi(argv[3]);

	shm_client = argv[1];

	//Sem_name = Sema+id_client
	string s("Sema") ;
	s += argv[3];
	sem_name = s.c_str();

	//Creating the shm client
	if((fd_shm = shm_open(argv[1],O_RDWR|O_CREAT,0600)) == -1){
	    perror("shm_open client ");
	    return 1;
	}

        if((space = mmap(0,sizeof(message), PROT_READ|PROT_WRITE,MAP_SHARED,fd_shm,0)) == MAP_FAILED){
	    perror("MMAP");
            return 1;
	}
       shm_mess_client = static_cast <struct message*> (space);
	
       //Creating the shm server
       if((fd_shm2 = shm_open(argv[2],O_RDWR,0600)) == -1){
	    perror("shm_open server");
	    return 1;
	}

        if((space2 = mmap(0,sizeof(myshm), PROT_READ|PROT_WRITE,MAP_SHARED,fd_shm2,0)) == MAP_FAILED){
	    perror("MMAP");
            return 1;
	}
	
	shm_serv = static_cast <struct myshm *> (space2);


	//Creating the named semaphore
	sem_client = sem_open(sem_name,O_CREAT|O_RDWR,0600,0);


	//Close the  semaphore
	sem_close(sem_client);
	//Destroy the semaphore
	sem_unlink(sem_name);
	

	//Unmap the shm segment client
	munmap(space,sizeof(message));
	//Destroy the shm segment client
	shm_unlink(argv[1]);

	//Unmap the shm segment server
	munmap(space2,sizeof(myshm));
	
	sem_close(sem_client);
	


	return 0;


}

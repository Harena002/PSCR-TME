#include "chat_common.h"






struct myshm *buff;


void read_request(){

	//to find which request we didn't answer yet
	int id = (buff -> nb)%MAX_MESS;
	struct message * mess = buff -> messages [id];
	



}


int main(int argc,char ** argv){

	int fd_shm; 
	void *space;
	if(fd_shm = shm_open(argv[1],O_RDWR|O_CREAT,0600) == -1){
	    perror("shm_open");
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
	
	buff = static_cast <myshm*> space;
	buff -> nb = 0;
	buff -> read = 0;
	buff -> write = 0;
	sem_init(&(buff->sem),1,1);

	munmap(space,sizeof(myshm);
	shm_unlink(argv[1]);













	return 0;
}

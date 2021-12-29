#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include "webserver.h"

#define MAX_REQUEST 100

int port, numThread, in = 0, out = 0;

int request[MAX_REQUEST]; // shared buffer

// semaphores and a mutex lock
sem_t sem_full;
sem_t sem_empty;
pthread_mutex_t mutex;


//////////////////////////////////////////////
//Nothing was changed outside of these hashes

void *worker_thread(void *var)
{
  /*//Init Test 
  int *myid = (int *)var;
  
  printf("worker_thread id: %d\n", *myid);
  */

  int tid = (int *) var;  
  printf("Worker thread created. pid %d, tid %d\n", getpid(), tid);//////////////////////////
  
  while(1){
    int fd;
    sem_wait(&sem_full);
    pthread_mutex_lock(&mutex);
    fd = request[out];
    out = (out + 1) % MAX_REQUEST;
    pthread_mutex_unlock(&mutex);
    sem_post(&sem_empty);
    
    process(fd);
  } // while
  
} // worker_thread

//////////////////////////////////////////////


void *req_handler(void *var)
{
                int r;
		struct sockaddr_in sin;
		struct sockaddr_in peer;
		int peer_len = sizeof(peer);
	        int sock;

		sock = socket(AF_INET, SOCK_STREAM, 0);

		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = INADDR_ANY;
		sin.sin_port = htons(port);
		r = bind(sock, (struct sockaddr *) &sin, sizeof(sin));
		if(r < 0) {
				perror("Error binding socket:");
				exit(0);
		}

		r = listen(sock, 5);
		if(r < 0) {
				perror("Error listening socket:");
				exit(0);
		}

		printf("HTTP server listening on port %d\n", port);

		
		////////////////////////////////////////////////////////
		//Nothing was changed outside of these hashes
		
		while (1) // listener "thread" loop
		{

		  int s;
		  s = accept(sock, NULL, NULL);
		  
		  sem_wait(&sem_empty); // wait when there are no empty slots
		  pthread_mutex_lock(&mutex);
		  
		  request[in] = s;
		  in = (in + 1) % MAX_REQUEST;  

		  pthread_mutex_unlock(&mutex);
		  sem_post(&sem_full);
		  
		  if (s < 0) break;
		  
		}

		////////////////////////////////////////////////////////		

		close(sock);
}


int main(int argc, char *argv[])
{
		if(argc < 2 || atoi(argv[1]) < 2000 || atoi(argv[1]) > 50000)
		{
				fprintf(stderr, "./webserver PORT(2001 ~ 49999) (#_of_threads) (crash_rate(%))\n");
				return 0;
		}

		
		// port number
		port = atoi(argv[1]);
		
		// # of worker thread
		if(argc > 2) 
				numThread = atoi(argv[2]);
		else numThread = 1;

		// crash rate
		if(argc > 3) 
				CRASH = atoi(argv[3]);
		if(CRASH > 50) CRASH = 50;
		
  sem_init(&sem_empty, 0, MAX_REQUEST);
  sem_init(&sem_full, 0, 0);

		printf("[pid %d] CRASH RATE = %d\%\n", getpid(), CRASH);

		////////////////////////////////////////////////////////
		//Nothing was changed outside of these hashes
		
		pthread_t listener_id;
		pthread_create(&listener_id, NULL, req_handler, listener_id); // listener thread

		int i;
		pthread_t thread_id[numThread];

		for(i = 0; i < numThread; i++){ // create worker threads
		  
		  pthread_create(&thread_id[i], NULL, worker_thread, i+2);
		  
		} // for
		
		
		//req_handler();

		
		int retval, j;
		
		while(1){ // thread maintenance

		  for(j = 0; j< numThread; j++){
		    
		    retval = pthread_tryjoin_np(thread_id[j], NULL);

		    if(retval != 16){

		      printf("Worker thread %d killed. Create another.\n", j);
		      pthread_create(&thread_id[i], NULL, worker_thread, i+2);
		      
		    }
		    
		    
		  } // for

		} // while

		////////////////////////////////////////////////////////		

		return 0;
}

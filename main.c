/*
Tim Lindberg
sudo apt-get install libncurses5-dev libncursesw5-dev
gcc main.c -pthread -lncurses -lrt
run
*/

#include <stdio.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include </usr/include/semaphore.h>
#include <curses.h>
#include <sys/mman.h>
#include <fcntl.h>

#define BUFFER_SIZE 10

typedef struct item{
  int p_buffer[BUFFER_SIZE];
  int t_buffer[BUFFER_SIZE];
  int m_buffer[BUFFER_SIZE];
  
  int p_in;
  int t_in;
  int m_in;
  
  int p_out;
  int t_out;
  int m_out;
  
  int p_total;
  int t_total;
  int m_total;

  int p_state;
  int t_state;
  int m_state;
} item; 

sem_t mutex; 
item *rptr;
int fd;
int p_agent = 0;
int t_agent = 0;
int m_agent = 0;


void Consume(int option){
  // Paper   = 0
  // Tobacco = 1
  // Matches = 2
  sleep(4);
  int value1 = 0;
  int value2 = 0;
  // printf("My pid is: %d\n", getpid());
  while (1){
    sleep(1);
    if(option == 0){
      // mvprintw(3, 0, "Paper:" );
      // Wait untill the buffers aren't empty
      rptr->p_state = 1;
      // While we wait, check if there is an exit code. 
      while(rptr->t_in == rptr->t_out || rptr->m_in == rptr->m_out)
      { 
        sleep(.5);
        if(rptr->p_buffer[rptr->p_out] == -1){
          sem_wait(&mutex);
          rptr->p_state = 4;
          sem_post(&mutex);
          return;
        }
      }
           // Do nothing
      // Wait for writing permission
      sem_wait(&mutex);
      // adjust buffer bounds
      if(rptr->t_in != rptr->t_out && rptr->m_in != rptr->m_out){
        value1 = rptr->t_buffer[rptr->t_out];
        rptr->t_out = ( rptr->t_out + 1 ) % BUFFER_SIZE;
        value2 = rptr->m_buffer[rptr->m_out];
        rptr->m_out = ( rptr->m_out + 1 ) % BUFFER_SIZE;
       // mvprintw(4, 4, "got one" );
        rptr->p_state = 2;
        sleep( 2); 
        rptr->p_state = 3;
        sleep(4);
        rptr->p_total += 1; 
      }
      sem_post(&mutex);
    } 
    else if(option == 1){
      //mvprintw(6, 0, "Tobacco:" );
      // Wait untill the buffers aren't empty
      rptr->t_state = 1;
      // While we wait, check if there is an exit code. 
      while(rptr->p_in == rptr->p_out || rptr->m_in == rptr->m_out)
      { 
        sleep(.5);
        if(rptr->t_buffer[rptr->t_out] == -1){
          sem_wait(&mutex);
          rptr->t_state = 4;
          sem_post(&mutex);
          return;
        }
      }
      // Wait for writing permission
      sem_wait(&mutex);
      // adjust buffer bounds
      if(rptr->p_in != rptr->p_out && rptr->m_in != rptr->m_out){
        value1 = rptr->p_buffer[rptr->p_out];
        rptr->p_out = ( rptr->p_out + 1 ) % BUFFER_SIZE;
        value2 = rptr->m_buffer[rptr->m_out];
        rptr->m_out = ( rptr->m_out + 1 ) % BUFFER_SIZE;
        //mvprintw(7, 4, "Got one" );
        rptr->t_state = 2;
        sleep( 2); 
        rptr->t_state = 3;
        sleep(4);
	rptr->t_total += 1; 
      }
      sem_post(&mutex);
    } 
    else{
      //mvprintw(9, 0, "Matches:" );
      // Wait untill the buffers aren't empty
      rptr->m_state = 1;
      // While we wait, check if there is an exit code. 
      while(rptr->t_in == rptr->t_out || rptr->p_in == rptr->p_out)
      { 
        sleep(.5);
        if(rptr->m_buffer[rptr->m_out] == -1){
          sem_wait(&mutex);
          rptr->m_state = 4;
          sem_post(&mutex);
          return;
        }
      }
      // Wait for writing permission
      sem_wait(&mutex);
      // adjust buffer bounds
      if(rptr->t_in != rptr->t_out && rptr->p_in != rptr->p_out){
        value1 = rptr->t_buffer[rptr->t_out];
        rptr->t_out = ( rptr->t_out + 1 ) % BUFFER_SIZE;
        value2 = rptr->p_buffer[rptr->p_out];
        rptr->p_out = ( rptr->p_out + 1 ) % BUFFER_SIZE;
       // mvprintw(10, 4, "Got one" );
        rptr->m_state = 2;
        sleep( 2); 
        rptr->m_state = 3;
        sleep(4);
        rptr->m_total += 1; 
      }
      sem_post(&mutex);
    }
    // check to see if it is an exit code
    if ( value1 == -1 || value2 == -1 ){ 
      sem_wait(&mutex);
      rptr->p_state = 4;
      rptr->t_state = 4;
      rptr->m_state = 4;
      sem_post(&mutex);
      return;
    }
  }
} 

void Produce(){
  if( rand() % 2 == 0){
    if(((rptr->p_in + 1) % BUFFER_SIZE ) != rptr->p_out){
      rptr->p_buffer[rptr->p_in] = 1;
      rptr->p_in = ( rptr->p_in + 1 ) % BUFFER_SIZE;
      p_agent += 1;
    }
  }
  if( rand() % 2 == 0){
    if(((rptr->m_in + 1) % BUFFER_SIZE ) != rptr->m_out){
      rptr->m_buffer[rptr->m_in] = 1;
      rptr->m_in = ( rptr->m_in + 1 ) % BUFFER_SIZE;
      t_agent += 1;
    }
  }
  if( rand() % 2 == 0){
    if(((rptr->t_in + 1) % BUFFER_SIZE ) != rptr->t_out){
      rptr->t_buffer[rptr->t_in] = 1;
      rptr->t_in = ( rptr->t_in + 1 ) % BUFFER_SIZE;
      m_agent += 1;
    }
  }
}

void PrintData(){
  int i;
  system("clear");
  printf("Total resources on the table: \n");
  printf("Paper   : %d\n",abs(rptr->p_in - rptr->p_out));
  printf("Tobacco : %d\n",abs(rptr->t_in - rptr->t_out));
  printf("Matches : %d\n",abs(rptr->m_in - rptr->m_out));
  
  printf("\n");
  printf("Paper Client:\n\tTotal: %d\n", rptr->p_total); 
  if(rptr->p_state == 0)
    printf("IDLE\n");
  else if( rptr->p_state == 1)
    printf("WAITING\n");
  else if( rptr->p_state == 2)
    printf("ROLLING\n");
  else if( rptr->p_state == 3)
    printf("SMOKING\n");
  else 
    printf("TERMINATING\n");
  printf("\n");
  printf("Tobacco Client:\n\tTotal: %d\n",rptr->t_total); 
  if(rptr->t_state == 0)
    printf("IDLE\n");
  else if( rptr->t_state == 1)
    printf("WAITING\n");
  else if( rptr->t_state == 2)
    printf("ROLLING\n");
  else if( rptr->t_state == 3)
    printf("SMOKING\n");
  else 
    printf("TERMINATING\n");
  printf("\n");
  printf("Matches Client:\n\tTotal: %d\n",rptr->m_total); 
  if(rptr->m_state == 0)
    printf("IDLE\n");
  else if( rptr->m_state == 1)
    printf("WAITING\n");
  else if( rptr->m_state == 2)
    printf("ROLLING\n");
  else if( rptr->m_state == 3)
    printf("SMOKING\n");
  else 
    printf("TERMINATING\n");
  printf("\nAGENT:\n");
  printf("\tTotal Produced: \n");
  printf("Paper   : %d\n", p_agent);
  printf("Tobacco : %d\n", t_agent);
  printf("Matches : %d\n", m_agent);
}
  
int main (){
  int i;
  pid_t pid, pid1, pid2;
  time_t end;
  srand(time(NULL));

  sem_init(&mutex, 0, 1);

  fd = shm_open("/region", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if ( fd == -1 )
    printf("BAD");
   
  if( ftruncate(fd, sizeof(item)) == -1) 
    printf("BAD2");
  
  rptr = mmap(NULL, sizeof(item), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(rptr == MAP_FAILED)
    printf("BAD3");
  
  
  for(i = 0; i < BUFFER_SIZE; i++)
  {
    rptr->p_buffer[i] = 1;
    rptr->t_buffer[i] = 1;
    rptr->m_buffer[i] = 1;
  }
  rptr->p_in = 0;
  rptr->t_in = 0;
  rptr->m_in = 0;
  rptr->p_out =0;
  rptr->t_out =0;
  rptr->m_out =0;
  rptr->p_total =0;
  rptr->t_total =0;
  rptr->m_total =0;
  rptr->p_state = 0;
  rptr->t_state = 0;
  rptr->m_state = 0;
  
  pid = fork();
  if( pid > 0 ) {
    pid1 = fork();
    if( pid1 > 0 ) {
      // paper 
      Consume(0);
      printf("\nexit p\n");
      exit(0);
    }
    else if( pid1 == 0 ){
      // tobacco
      Consume(1);
      printf("\nexit p\n");
      exit(0);
    } 
    else{
      printf("ERROR") ;
      exit(1);
    }
    wait(NULL);
  }
  else if( pid == 0 ){
    pid2 = fork();
    if( pid2 > 0 ) {
      // matches
      Consume(2);
      printf("\nexit p\n");
      exit(0);
    }
    else if( pid2 == 0 ){
      // agent
      int count = 0;
      end = time(NULL) + 25; 
      do 
      {
        PrintData();
        if(count == 1)
          Produce();
        count = (count + 1 ) % 3;
        sleep(1);
      }while(end > time(NULL));
  
      printf("Exitting..."); 

      sem_wait(&mutex);
      // Change everything to the exit code
      for(i = 0; i < BUFFER_SIZE; i++)
      {
        rptr->p_buffer[i] = -1;
        rptr->t_buffer[i] = -1;
        rptr->m_buffer[i] = -1;
      }
      sem_post(&mutex);

      end = time(NULL) + 5; 
      do 
      {
        PrintData();
        sleep(1);
      }while(end > time(NULL));
      exit(1);
    } 
    else{
      printf("ERROR") ;
      exit(1);
    }
    wait(NULL);  
    return;
  } 
  else{
    printf("ERROR") ;
    exit(1);
  } 
  return 0;
}

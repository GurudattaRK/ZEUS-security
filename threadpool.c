#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<semaphore.h>

pthread_mutex_t mutex;
pthread_cond_t condition;
sem_t semaphore;
unsigned char buffer[512],Exit=0;
int x=0;
void* master(void*arg1) 
{
    FILE*reader=fopen("src.txt","rb");
    FILE* writer=fopen("dest.txt","wb");
    int waitcount,core= *(int*)arg1;
    free(arg1);
    while(!(feof(reader))) 
    {
        printf("\npre reading\n");
    //printf("\n1\n");
    
        printf("\nreading ...\n");
        fread(buffer,1,512,reader);

        pthread_mutex_lock(&mutex);
        //printf("\n2\n");

        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&condition);
        printf("\npost conditonal wait\n");

        for(waitcount=0; waitcount<core ;waitcount++)
        {
            sem_wait(&semaphore);  //waiting for processing
        }


        printf("\npost semaphore\n");
    
        fwrite(buffer,512,1,writer);
        printf("\nwriting\n====================\n");
        
    }
    Exit=255;
    pthread_cond_broadcast(&condition);

    printf("\nend of file\n");
    fclose(reader);
    fclose(writer);

    return NULL;
}

void* worker(void* arg2) 
{
    int count=*(int*)arg2;
    free(arg2);
    unsigned char inbuff[256];
    int jump=256*count;
    while(1)
    {
        if(Exit)
        {
            goto Exited;
        }

        else
        {
            pthread_mutex_lock(&mutex);
            pthread_cond_wait(&condition, &mutex);
            pthread_mutex_unlock(&mutex);

        
            memmove(inbuff,buffer+jump,256);

            //memset(inbuff,65,512);    //TEST
            for(int y=0;y<256;y++)
            {
                inbuff[y]=inbuff[y]+count+1;    //PROCESSING SECTION
            }
            
            memmove(buffer+jump,inbuff,256);


            printf("\nThread %d Processsing...\n",count+1);

            sem_post(&semaphore);
        }
        
    }
    Exited:
    printf("\nexiting process 1\n");

    return NULL;
}


int main() 
{
    pthread_t mainThread,workerThreads[64];
    int cores=2;

    sem_init(&semaphore,0,0);
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condition, NULL);

    int *arg=malloc(sizeof(int));
    *arg=cores;
    pthread_create(&mainThread, NULL, &master,arg) ;

    for(int i=0;i<cores;i++)
    {
        int* count = malloc(sizeof(int));
        *count= i;
        pthread_create(&workerThreads[i], NULL, &worker, count);
    }

    pthread_join(mainThread, NULL);
    printf("main joined\n");

    for(int i=0;i<cores;i++)
    {
        pthread_join(workerThreads[i], NULL);
        printf("worker %d joined\n",i+1);
    }

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&condition);

    return 0;
}
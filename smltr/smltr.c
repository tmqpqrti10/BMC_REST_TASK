#include "init.h"
#include "head.h"
#include "timer_func.h"

extern sem_t sem_mutex;

int main(int argc, char **argv)
{
    if(argc > 1)
        __INIT(1);
    else
        __INIT(0);
    
    int status, i;

	sem_init(&sem_mutex, 0, 0);

	queue_root root;
	init_queue(&root);

    pthread_t simulator_thread[4];

    pthread_create(&simulator_thread[0], NULL, PWR_FAN_HANDLER,	(void*) &root);
	pthread_create(&simulator_thread[1], NULL, IPMB_HANDLER,	(void*) &root);
	pthread_create(&simulator_thread[2], NULL, SENSOR_HANDLER, (void*) &root);
 	pthread_create(&simulator_thread[3], NULL, QUEUE_HANDLER,	(void*) &root);

    for(i = 0 ; i < 4 ; i++)
    {
        pthread_join(simulator_thread[i], (void **)&status);
		if(!status)
			printf("main : error : thread join : %d : %d \n", i, status);
    }

    printf("good bye simulator\n");

    return 0;
}
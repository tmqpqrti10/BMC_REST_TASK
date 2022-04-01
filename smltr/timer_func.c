#include "timer_func.h"

pthread_mutex_t DEV_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PWR_MUTEX = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t PWR_STATUS_CHANGED = PTHREAD_COND_INITIALIZER;

sem_t sem_mutex;

void *PWR_FAN_HANDLER(void *data)
{
    queue_root* root;
    root = (queue_root*) data;

	int msqid;
	uint8_t sensor_num;
	uint8_t value;

	smltr_data_t rcv_data;
    dev_handler devh;

	while (1)
	{
		if (-1 == (msqid = msgget((key_t)8999, IPC_CREAT | 0666)))
		{
			perror("msgget() 실패\n");
		}

		printf("PWR_FAN_HANDLER : log : got the ipc key \n");

		while (1)
		{
			if (-1 == msgrcv(msqid, &rcv_data, sizeof(smltr_data_t) - sizeof(long), 2, 0))
			{
				perror("msgrcv() 실패\n");
				break;
			}

			sensor_num = rcv_data.sensor_num;
            value = rcv_data.value;
            
			
            if(sensor_num >= SMLTR_SET_FAN)
            {
			    devh = set_dev(SET_FAN_SPEED, sensor_num-SMLTR_SET_FAN, value);
            }
	    else if(sensor_num >= SMLTR_POWER_OFF && sensor_num <= SMLTR_POWER_RESET)
            {
                devh = set_dev(SET_POWER, sensor_num, value);
                COND_SIGNAL(PWR_STATUS_CHANGED);
            }
            else
            {
                break;
            }
            MUTEX_LOCK(DEV_MUTEX);
            push_queue(root, &devh);
            MUTEX_UNLOCK(DEV_MUTEX);
            sem_post(&sem_mutex);
		}
	}
}

void *IPMB_HANDLER(void *data)
{
	printf("start IPMB handler\n");

    queue_root* root;
    root = (queue_root*) data;

	int msqid;
	uint8_t sensor_num;
	uint8_t value;

	smltr_data_t rcv_data, snd_data;

	while (1)
	{
		if (-1 == (msqid = msgget((key_t)8998, IPC_CREAT | 0666)))
		{
			perror("msgget() 실패\n");
		}

		printf("IPMB_HANDLER : log : got the ipc key \n");

		snd_data.data_type = 1;//rcv_data.data_type

		while (1)
		{
			if (-1 == msgrcv(msqid, &rcv_data, sizeof(smltr_data_t) - sizeof(long), 2, 0))
			{
				perror("msgrcv() 실패\n");
				break;
			}

			sensor_num = rcv_data.sensor_num;
			snd_data.sensor_num = sensor_num;
			MUTEX_LOCK(DEV_MUTEX);
			snd_data.value = get_dev(sensor_num, 0, GET_IPMB_SENSOR);
			MUTEX_UNLOCK(DEV_MUTEX);

			if (-1 == msgsnd(msqid, &snd_data, sizeof(smltr_data_t) - sizeof(long), 0))
			{
				perror("msgsnd() 실패\n");
				break;
			}
		}
	}
}

void *SENSOR_HANDLER(void *data)
{
	printf("start SENSOR handler\n");

    queue_root* root;
    root = (queue_root*) data;
	dev_handler devh;

	while(1)
	{
		MUTEX_LOCK(PWR_MUTEX);
		COND_WAIT(PWR_STATUS_CHANGED, PWR_MUTEX);
		MUTEX_UNLOCK(PWR_MUTEX);

		devh = set_dev(SET_SENSORS, 0, POWER_STATUS);
		MUTEX_LOCK(DEV_MUTEX);
		push_queue(root, &devh);
		MUTEX_UNLOCK(DEV_MUTEX);
		sem_post(&sem_mutex);
	}
}

void* QUEUE_HANDLER(void* data)
{
	printf("start QUEUE handler\n");

    queue_root* root;
    root = (queue_root*) data;
    while(1)
    {
        sem_wait(&sem_mutex);   //wait
        MUTEX_LOCK(DEV_MUTEX);
        DO_ACTION(root);        // it calls queue.pop() function + devh.action() function
        MUTEX_UNLOCK(DEV_MUTEX);
    }
}

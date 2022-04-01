#ifndef __HEAD_H
#define __HEAD_H
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FILE_FAN_RPM "/root/sys/devices/platform/ast_pwm_tacho.0/tacho%d_rpm"
#define FILE_ADC_VALUE "/root/sys/devices/platform/ast_adc.0/adc%d_value"
#define FILE_TMP75_DEVICE "/root/sys/class/i2c-adapter/i2c-6/6-00%x"
#define TMP75_DEVICE_U47 "/root/sys/class/i2c-adapter/i2c-6/6-0049"
#define TMP75_DEVICE_U48 "/root/sys/class/i2c-adapter/i2c-6/6-004a"
#define TMP75_DEVICE_U49 "/root/sys/class/i2c-adapter/i2c-6/6-004b"
#define TMP75_DEVICE_U50 "/root/sys/class/i2c-adapter/i2c-6/6-004c"
#define TMP75_DEVICE_U51 "/root/sys/class/i2c-adapter/i2c-6/6-004d"
#define TMP75_DEVICE_U52 "/root/sys/class/i2c-adapter/i2c-6/6-004e"
#define PDPB_TMP75_PSU1_DEVICE "/root/sys/class/i2c-adapter/i2c-6/6-004f"
#define PDPB_TMP75_PSU2_DEVICE "/root/sys/class/i2c-adapter/i2c-6/6-0050"

#define FILE_FAN_ENABLE "/root/sys/devices/platform/ast_pwm_tacho.0/tacho%d_en"
#define FILE_ADC_ENABLE "/root/sys/devices/platform/ast_adc.0/adc%d_en"

#define COUNT_TEMP 8
#define COUNT_FANS 8
#define COUNT_IPMB 26
#define COUNT_ADC 15
#define COUNT_PSU_WATT 2

#define POWER_OFF		0
#define POWER_ON		1
#define POWER_CYCLE		2
#define POWER_RESET		3

#define MUTEX_LOCK(x) 		pthread_mutex_lock(&x);
#define MUTEX_UNLOCK(x) 	pthread_mutex_unlock(&x);
#define COND_WAIT(x, y) 	pthread_cond_wait(&x, &y); 
#define COND_SIGNAL(x)		pthread_cond_signal(&x);

#define MAX_FILE_LENGTH		100

#define SMLTR_POWER_OFF 0xB0
#define SMLTR_POWER_ON 0xB1
#define SMLTR_POWER_CYCLE 0xB2
#define SMLTR_POWER_RESET 0xB3
#define SMLTR_SET_FAN 0xB4

#define PDPB_SENSOR_TEMP_CPU0 0x01
#define PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2 0x1A

typedef struct
{
  long data_type;
  uint8_t sensor_num;
  uint8_t value;
} smltr_data_t;


enum {
    TYPE_ADC = 0,
    TYPE_FAN,
    TYPE_TEMP,
    TYPE_PSU_WATT,
};

typedef int(*smltrPtr)(uint8_t, uint8_t);

typedef struct
{
    smltrPtr action;
    uint8_t val1;
    uint8_t val2;
} dev_handler;

typedef struct _ipc_remote
{

	long data_type;
	int sensor_id;
	int value;
} ipc_remote;

int POWER_STATUS;
int ADC[15];
int NVA[10];
int TMP75[8];
int PDPB_IPMB[26];

#endif

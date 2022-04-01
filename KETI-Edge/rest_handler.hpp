#pragma once
#include <redfish/stdafx.hpp>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <sys/msg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <ipmi/ipmi.hpp>



#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#define TIMER_500MS 600000 / 2
#define TIMER_250MS 600000 / 4
#define TIMER_100MS 600000 / 10
#define TIMER_1SECS 600000
#define TIMER_3SECS 600000 * 3
#define TIMER_5SECS 600000 * 5

typedef struct  {
	unsigned char* data;		// array with the data-bytes
	int length;					// length of array
} protocol_data;


void ipmb_get_cpu_temp(void);
void ipmb_get_cpu_temp1(void);
#define MIN(a,b) (((a)<(b))?(a):(b))
#define BSWAP_32(x) ((((x) & 0xff000000) >> 24) | (((x) & 0x00ff0000) >> 8) |(((x) & 0x0000ff00) << 8) | (((x) & 0x000000ff) << 24))

void *rest_handler(void *data);
void *run_ipmi_handle_rest(void *void_msq_req);
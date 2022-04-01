#include "head.h"
#include "file_func.h"
#include "queue.h"

#ifndef __TIMER_FUNC_H
#define __TIMER_FUNC_H

void *PWR_FAN_HANDLER(void *data);
void *IPMB_HANDLER(void *data);
void *SENSOR_HANDLER(void *data);
void* QUEUE_HANDLER(void* data);

#endif
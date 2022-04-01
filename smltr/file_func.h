#ifndef __FILE_FUNC_H
#define __FILE_FUNC_H

#include "head.h"
#include "queue.h"

int SET_SENSORS(uint8_t pin, uint8_t value);
int FIND_FILE_NAME(uint8_t type, uint8_t index, char f_name[MAX_FILE_LENGTH]);
int SET_POWER(uint8_t cmd, uint8_t val);
int SET_FAN_SPEED(uint8_t fan_idx, uint8_t rpm);
int GET_IPMB_SENSOR(uint8_t sid, uint8_t not_use);
void DO_ACTION(queue_root* root);
int get_dev(uint8_t value1, uint8_t value2, smltrPtr file_func);
dev_handler set_dev(smltrPtr file_func, uint8_t value1, uint8_t value2);

#endif
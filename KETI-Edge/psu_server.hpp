
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <ipmi/ipmi.hpp>
#include <redfish/ast_gpio.hpp>
#include <sqlite3.h>
#include <ipmi/sensor_define.hpp>
#include<ipmi/gpio.hpp>
#include<redfish/stdafx.hpp>

#include<ipmi/lightning_sensor.hpp>
#include <ipmi/common.hpp>
#include <ipmi/sdr.hpp>




#define POWER_USAGE_DB "/conf/ipmi/power.db"

#define INTERVAL 1000		/* number of milliseconds to go off */
#define I2C_PSU_DEV_NVA       "/dev/i2c-7"
#define LAST_MIN_COUNT_DB 30
#define LAST_HOUR_COUNT_DB 600
#define LAST_DAY_COUNT_DB 1440
#define PCH_PWR_GOOD	PIN_GPIOC0
#define MIN_1 1
#define MIN_2 2
#define HOUR_1 3
#define HOUR_2 4
#define DAY_1 5
#define DAY_2 6

unsigned char rest_main_power_status(void);
int get_temp_cpu0(char* ret);
void get_temp_cpu1(char* ret);
void get_temp_board(char* ret);
void get_voltage_fan_power(char* ret);
int get_fans(char* ret);
void *psu_handler(void);
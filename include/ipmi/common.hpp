#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <arpa/inet.h>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <uuid/uuid.h>

#define MAX_USERNAME 17
#define MAX_PASSWD 20
#define MAX_SESSION 5

enum {
  FRU_ALL = 0,
  FRU_PEB = 1,
  FRU_PDPB = 2,
  FRU_FCB = 3,
  FRU_NVA = 4,
};

enum {
  SENSOR_VALID = 0x0,
  UCR_THRESH = 0x01,
  UNC_THRESH,
  UNR_THRESH,
  LCR_THRESH,
  LNC_THRESH,
  LNR_THRESH,
  POS_HYST,
  NEG_HYST,
};
/**
 * @brief DIMM(dual in-line memory module) 등 센서 , psu
 */
enum {
  PDPB_SENSOR_TEMP_CPU0 = 0x01,
  PDPB_SENSOR_TEMP_CPU1 = 0x02,
  PDPB_SENSOR_TEMP_CPU0_CH0_DIMM0,
  PDPB_SENSOR_TEMP_CPU0_CH0_DIMM1,
  PDPB_SENSOR_TEMP_CPU0_CH0_DIMM2,
  PDPB_SENSOR_TEMP_CPU0_CH1_DIMM0,
  PDPB_SENSOR_TEMP_CPU0_CH1_DIMM1,
  PDPB_SENSOR_TEMP_CPU0_CH1_DIMM2,
  PDPB_SENSOR_TEMP_CPU0_CH2_DIMM0,
  PDPB_SENSOR_TEMP_CPU0_CH2_DIMM1,
  PDPB_SENSOR_TEMP_CPU0_CH2_DIMM2,
  PDPB_SENSOR_TEMP_CPU0_CH3_DIMM0,
  PDPB_SENSOR_TEMP_CPU0_CH3_DIMM1,
  PDPB_SENSOR_TEMP_CPU0_CH3_DIMM2,
  PDPB_SENSOR_TEMP_CPU1_CH0_DIMM0,
  PDPB_SENSOR_TEMP_CPU1_CH0_DIMM1,
  PDPB_SENSOR_TEMP_CPU1_CH0_DIMM2,
  PDPB_SENSOR_TEMP_CPU1_CH1_DIMM0,
  PDPB_SENSOR_TEMP_CPU1_CH1_DIMM1,
  PDPB_SENSOR_TEMP_CPU1_CH1_DIMM2,
  PDPB_SENSOR_TEMP_CPU1_CH2_DIMM0,
  PDPB_SENSOR_TEMP_CPU1_CH2_DIMM1,
  PDPB_SENSOR_TEMP_CPU1_CH2_DIMM2,
  PDPB_SENSOR_TEMP_CPU1_CH3_DIMM0,
  PDPB_SENSOR_TEMP_CPU1_CH3_DIMM1,
  PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2,
};
enum {
  PDPB_SENSOR_TEMP_REAR_RIGHT = 0x30,
  PDPB_SENSOR_TEMP_CPU_AMBIENT,
  PDPB_SENSOR_TEMP_FRONT_RIGHT,
  PDPB_SENSOR_TEMP_PCIE_AMBIENT,
  PDPB_SENSOR_TEMP_FRONT_LEFT,

};
// Sensors KTNF under PEB
enum {
  PEB_SENSOR_ADC_P12V_PSU1 = 0x20,
  PEB_SENSOR_ADC_P12V_PSU2 = 0x21,
  PEB_SENSOR_ADC_P3V3 = 0x22,
  PEB_SENSOR_ADC_P5V = 0x23,
  PEB_SENSOR_ADC_PVNN_PCH = 0x24,
  PEB_SENSOR_ADC_P1V05 = 0x25,
  PEB_SENSOR_ADC_P1V8 = 0x26,
  PEB_SENSOR_ADC_BAT = 0x27,
  PEB_SENSOR_ADC_PVCCIN = 0x28,
  PEB_SENSOR_ADC_PVNN_PCH_CPU0 = 0x29,
  PEB_SENSOR_ADC_P1V8_NACDELAY = 0x2A,
  PEB_SENSOR_ADC_P1V2_VDDQ = 0x2B,
  PEB_SENSOR_ADC_PVNN_NAC = 0x2C,
  PEB_SENSOR_ADC_P1V05_NAC = 0x2D,
  PEB_SENSOR_ADC_PVPP = 0x2E,
  PEB_SENSOR_ADC_PVTT = 0x2F,
};

// // Sensors under PEB
// enum {
//     PEB_SENSOR_ADC_P12V = 0x20,
//     PEB_SENSOR_ADC_P3V3_AUX = 0x21,
//     PEB_SENSOR_ADC_P1V0_STBY = 0x22,
//     PEB_SENSOR_ADC_P1V05_PCH_AUX = 0x23,
//     PEB_SENSOR_ADC_P12V_AUX = 0x24,
//     PEB_SENSOR_ADC_P1V8_PCH_AUX = 0x25,
//     PEB_SENSOR_ADC_P3V0_BAT = 0x26,
//     PEB_SENSOR_ADC_P1V7_CPU0 = 0x27,
//     PEB_SENSOR_ADC_P1V7_CPU1 = 0x28,
//     PEB_SENSOR_ADC_P1V2_ABC = 0x29,
//     PEB_SENSOR_ADC_P1V2_DEF = 0x2A,
//     PEB_SENSOR_ADC_P1V2_GHJ = 0x2B,
//     PEB_SENSOR_ADC_P1V2_KLM = 0x2C,
//     PEB_SENSOR_ADC_P1V0_CPU0 = 0x2D,
//     PEB_SENSOR_ADC_P1V0_CPU1 = 0x2E,
//     PEB_SENSOR_ADC_P1V0_CPU1 = 0x2E,
// };

enum {
  NVA_SENSOR_TEMP1 = 0x65,
  NVA_SENSOR_TEMP2 = 0x66,
  NVA_SENSOR_TEMP3 = 0x67,
  NVA_SENSOR_TEMP4 = 0x68,
  NVA_SENSOR_PSU1_TEMP = 0x90,
  NVA_SENSOR_PSU2_TEMP = 0x91,
  NVA_SENSOR_PSU1_FAN1 = 0x92,
  NVA_SENSOR_PSU1_FAN2 = 0x93,
  NVA_SENSOR_PSU1_WATT = 0x94,
  NVA_SENSOR_PSU2_WATT = 0x95,
  NVA_SENSOR_PSU2_FAN1 = 0x96,
  NVA_SENSOR_PSU2_FAN2 = 0x97,
  NVA_SENSOR_PSU1_TEMP2 = 0x98,
  NVA_SENSOR_PSU2_TEMP2 = 0x99,
  NVA_SYSTEM_FAN1 = 0xA0,
  NVA_SYSTEM_FAN2 = 0xA1,
  NVA_SYSTEM_FAN3 = 0xA2,
  NVA_SYSTEM_FAN4 = 0xA3,
  NVA_SYSTEM_FAN5 = 0xA4,
};

#endif

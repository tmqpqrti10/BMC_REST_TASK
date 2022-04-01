#include "file_func.h"

/**
 * @brief 인덱스 번호에 해당하는 파일 경로 반환
 * @param char *f_name 파일 경로
 * @return int 성공 0, 실패 -1
 **/
int FIND_FILE_NAME(uint8_t type, uint8_t index, char f_name[MAX_FILE_LENGTH])
{
	memset(f_name, 0, MAX_FILE_LENGTH);
	char buf[200];

    switch(type)
    {
        case TYPE_ADC:
            sprintf(buf, FILE_ADC_VALUE, index); // SYSFAN 1 ~ 6 , PSU FAN 1, PSU FAN 2
        break;
        case TYPE_FAN:
            sprintf(buf, FILE_FAN_RPM, index);
        break;
        case TYPE_TEMP:
            sprintf(buf, FILE_TMP75_DEVICE, index+0x49);
        break;
        case TYPE_PSU_WATT:
            sprintf(buf, FILE_FAN_RPM, index+8);
        break;
        default:
            return -1;
        break;
    }

	memcpy(f_name, buf, strlen(buf));
	memset(f_name + strlen(buf), 0, 1);

	return 0;
}

int SET_POWER(uint8_t cmd, uint8_t val)
{
    char buf[200] = {0};
    char path[MAX_FILE_LENGTH] = {0};

    switch(cmd)
    {
        case SMLTR_POWER_OFF:
            POWER_STATUS = 0;
        break;
        case SMLTR_POWER_ON:
            POWER_STATUS = 1;
        break;
        case SMLTR_POWER_CYCLE:
            POWER_STATUS = 1;
        break;
        case SMLTR_POWER_RESET:
            POWER_STATUS = 1;
        break;
    }

    if(POWER_STATUS)
        ADC[0] = 190;
    else
        ADC[0] = 0;

    sprintf(path, FILE_ADC_VALUE, 0);

    sprintf(buf, "echo \"%d\" > %s \n", ADC[0], path);
    system(buf);

    return 0;
}

int SET_FAN_SPEED(uint8_t fan_idx, uint8_t rpm)
{
    char path[MAX_FILE_LENGTH] = {0};
    char buf[200] = {0};
    int i = 0;
    int ret = 0;
    if(fan_idx == 0x0)
    {
        for(i = 0 ; i < 6 ; i++)
        {
            NVA[i] = (160*rpm)/255;
            ret = FIND_FILE_NAME(TYPE_FAN, i, path);
            if(ret != -1)
            {
                sprintf(buf, "echo \"%d\" > %s \n", NVA[i], path);
                system(buf);
            }
        }
    }
    else if(fan_idx > 0 && fan_idx <=6)
    {
        NVA[fan_idx-1] = (160*rpm) / 255;
        ret = FIND_FILE_NAME(TYPE_FAN, fan_idx-1, path);
        if(ret != -1)
        {
            sprintf(buf, "echo \"%d\" > %s \n", NVA[fan_idx-1], path);
            system(buf);
        }
    }
    else
        return -1;

    return 0;
}

int GET_IPMB_SENSOR(uint8_t sid, uint8_t not_use)
{
    int ret = 0;

    if(sid >= PDPB_SENSOR_TEMP_CPU0 && sid <= PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2)
        ret = PDPB_IPMB[sid-1];
    else
        return -1;

    return ret;
}

int SET_SENSORS(uint8_t pin, uint8_t value)
{
	char f_name[MAX_FILE_LENGTH];
	char buf[200];
	int i=0;

    if(value == 1)
    {
        ADC[0] = 0xbe;
        ADC[1] = 0xc2;
        ADC[2] = 0x96;
        ADC[3] = 0x96;
        ADC[4] = 0xbe;
        ADC[5] = 0xc8;
        ADC[6] = 0x96;
        ADC[7] = 0xbe;
        ADC[8] = 0xbe;
        ADC[9] = 0xab;
        ADC[10] = 0xab;
        ADC[11] = 0xab;
        ADC[12] = 0xab;
        ADC[13] = 0x9a;
        ADC[14] = 0x9a;

        PDPB_IPMB[0] = 90 - 35;
        PDPB_IPMB[1] = 90 - 32;
        PDPB_IPMB[2] = 0x16;
        PDPB_IPMB[3] = 0;
        PDPB_IPMB[4] = 0;
        PDPB_IPMB[5] = 0;
        PDPB_IPMB[6] = 0;
        PDPB_IPMB[7] = 0;
        PDPB_IPMB[8] = 0;
        PDPB_IPMB[9] = 0;
        PDPB_IPMB[10] = 0;
        PDPB_IPMB[11] = 0;
        PDPB_IPMB[12] = 0;
        PDPB_IPMB[13] = 0;
        PDPB_IPMB[14] = 0x18;
        PDPB_IPMB[15] = 0;
        PDPB_IPMB[16] = 0;
        PDPB_IPMB[17] = 0;
        PDPB_IPMB[18] = 0;
        PDPB_IPMB[19] = 0;
        PDPB_IPMB[20] = 0;
        PDPB_IPMB[21] = 0;
        PDPB_IPMB[22] = 0;
        PDPB_IPMB[23] = 0;
        PDPB_IPMB[24] = 0;
        PDPB_IPMB[25] = 0;

        NVA[0] = 0x3c;
        NVA[1] = 0x3f;
        NVA[2] = 0x3e;
        NVA[3] = 0x3f;
        NVA[4] = 0x3c;
        NVA[5] = 0x3c;
        NVA[6] = 0x17;
        NVA[7] = 0x21;
        NVA[8] = 0xa;
        NVA[9] = 0xc;

        TMP75[0] = 32*2;
        TMP75[1] = 33*2;
        TMP75[2] = 32*2;
        TMP75[3] = 32*2;
        TMP75[4] = 23*2;
        TMP75[5] = 24*2;
        TMP75[6] = 27;
        TMP75[7] = 29;
    }

    else{
        PDPB_IPMB[0] = 90;
        PDPB_IPMB[1] = 90;

        for(i = 0 ; i < 15 ; i++)
        {
            ADC[0] = 0;
        }

        for(i = 2 ; i < 26 ; i++)
        {
            PDPB_IPMB[i] = 0;
        }

        for(i = 0 ; i < 10 ; i++)
        {
            NVA[i] = 0;
        }

        for(i = 0 ; i < 8 ; i++)
        {
            TMP75[i] = 0;
        }
    }

    for(i=1 ; i < COUNT_ADC ; i++)
    {
		FIND_FILE_NAME(TYPE_ADC, i, f_name);
		sprintf(buf, "echo \"%d\" > %s \n", ADC[i], f_name);
		system(buf);
	}

    for(i = 0 ; i < COUNT_FANS ; i++)
    {
        FIND_FILE_NAME(TYPE_FAN, i, f_name);
        sprintf(buf, "echo \"%d\" > %s \n", NVA[i], f_name);
        system(buf);
    }
    
    for(i = 0 ; i < COUNT_TEMP ; i++)
    {
        FIND_FILE_NAME(TYPE_TEMP, i, f_name);
        sprintf(buf, "echo \"%d\" > %s/temp1_input \n", TMP75[i], f_name);
        system(buf);
    }

    for(i = 0 ; i < COUNT_PSU_WATT ; i++)
    {
        FIND_FILE_NAME(TYPE_PSU_WATT, i, f_name);
        sprintf(buf, "echo \"%d\" > %s \n", NVA[i+8], f_name);
        system(buf);
    }
}

void DO_ACTION(queue_root* root)
{
	//printf("==DO==\n");
	dev_handler* result;
	if( (result = pop_queue(root)) ) //check queue empty
		result->action(result->val1, result->val2); //SET_POWER, SET_FAN_SPEED, GET_FAN_SPEED, GET_IPMB_VALUE, ...
	else
		printf("queue is empty\n");
}

dev_handler set_dev(smltrPtr file_func, uint8_t value1, uint8_t value2)
{
	dev_handler devh;

	devh.action = file_func;        //SET_POWER, SET_FAN_SPEED, GET_FAN_SPEED, GET_IPMB_VALUE ...
	devh.val1 = value1;           //[unsigned int]
	devh.val2 = value2;           //[int]

	return devh;
}

int get_dev(uint8_t value1, uint8_t value2, smltrPtr file_func)
{
	return file_func(value1, value2);
}
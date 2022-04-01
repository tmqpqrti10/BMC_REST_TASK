#include "init.h"
#include "file_func.h"

/**
 * @brief 파일 시스템 초기화 함수
 * @param int value 초기화 유무
 * @todo 파일에 값을 넣을 때 uint8_t로 넣으면 안되는 듯 함.
 **/
void __INIT(int value)
{
    int i = 0;
    char buf[256] = {0,};
    FILE *fp;

    printf("__INIT() I: Start...\n");

    // 센서 구조체 초기화

    if(value == 1)
    {
            printf("__INIT() I: Make file system...\n");
            system("mkdir -p /root/sys/class/i2c-adapter/i2c-6/6-0049/");
            system("mkdir -p /root/sys/class/i2c-adapter/i2c-6/6-004a/");
            system("mkdir -p /root/sys/class/i2c-adapter/i2c-6/6-004b/");
            system("mkdir -p /root/sys/class/i2c-adapter/i2c-6/6-004c/");
            system("mkdir -p /root/sys/class/i2c-adapter/i2c-6/6-004d/");
            system("mkdir -p /root/sys/class/i2c-adapter/i2c-6/6-004e/");
            system("mkdir -p /root/sys/class/i2c-adapter/i2c-6/6-004f/");
            system("mkdir -p /root/sys/class/i2c-adapter/i2c-6/6-0050/");

            system("mkdir -p /root/sys/devices/platform/ast_adc.0/");
            system("mkdir -p /root/sys/devices/platform/ast_pwm_tacho.0/");

            for(i = 0 ; i < COUNT_ADC ; i++)
            {
                // en = enable flag, value = 
                memset(buf, 0, sizeof(char)*256);
                sprintf(buf, "touch /root/sys/devices/platform/ast_adc.0/adc%d_en", i);
                system(buf);
                memset(buf, 0, sizeof(char)*256);
                sprintf(buf, "touch /root/sys/devices/platform/ast_adc.0/adc%d_value", i);
                system(buf);
            }

            for(i = 0 ; i < COUNT_FANS ; i++)
            {
                memset(buf, 0, sizeof(char)*256);
                sprintf(buf, "touch /root/sys/devices/platform/ast_pwm_tacho.0/tacho%d_en", i);
                system(buf);
                memset(buf, 0, sizeof(char)*256);
                sprintf(buf, "touch /root/sys/devices/platform//ast_pwm_tacho.0/tacho%d_rpm", i);
                system(buf);
                memset(buf, 0, sizeof(char)*256);
                sprintf(buf, "touch /root/sys/devices/platform//ast_pwm_tacho.0/pwm%d_rising", i);
                system(buf);
                memset(buf, 0, sizeof(char)*256);
                sprintf(buf, "touch /root/sys/devices/platform//ast_pwm_tacho.0/pwm%d_en", i);
                system(buf);
            }
            system("touch /root/sys/class/i2c-adapter/i2c-6/6-0049/temp1_input");
            system("touch /root/sys/class/i2c-adapter/i2c-6/6-004a/temp1_input");
            system("touch /root/sys/class/i2c-adapter/i2c-6/6-004b/temp1_input");
            system("touch /root/sys/class/i2c-adapter/i2c-6/6-004c/temp1_input");
            system("touch /root/sys/class/i2c-adapter/i2c-6/6-004d/temp1_input");
            system("touch /root/sys/class/i2c-adapter/i2c-6/6-004e/temp1_input");
            system("touch /root/sys/class/i2c-adapter/i2c-6/6-004f/temp1_input");
            system("touch /root/sys/class/i2c-adapter/i2c-6/6-0050/temp1_input");

    }
    char *buf2 = malloc(10);
    int adc_val = 0;
    fp = fopen("/root/sys/devices/platform/ast_adc.0/adc0_value", "r");
    fread(buf2, sizeof(char), 10, fp);
    adc_val = strtol(buf2, NULL, 10);
    //adc0_value 못읽어오기때문에
    //adc_val=151;
    if(adc_val > 150)
        POWER_STATUS = 1;
    else    
        POWER_STATUS = 0;
    
    //현재 
    POWER_STATUS = 1;
    if(POWER_STATUS == 1)
    {
        // ADC 0~14 Analog - Digital Converter
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
        //순차적으로 PDPB_SENSOR_TEMP_CPU0 ..
        PDPB_IPMB[0] = 90 - 35;
        PDPB_IPMB[1] = 90 - 32;
        PDPB_IPMB[2] = 0x16;
        PDPB_IPMB[3] = 0x16;
        PDPB_IPMB[4] = 0;
        PDPB_IPMB[5] = 0;
        PDPB_IPMB[6] = 0;
        PDPB_IPMB[7] = 0;
        PDPB_IPMB[8] = 0;
        PDPB_IPMB[9] = 0;
        PDPB_IPMB[10] = 0;
        PDPB_IPMB[11] = 0;
        PDPB_IPMB[12] = 0;
        PDPB_IPMB[13] = 0x19;
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

        // NVA = Manufacturer (National Ventilation ACR) or Non-Volatile Memory에 저장되는 SEL
        // NVA 0~7 FAN
        // NVA 8~9 PSU WATT 
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

        // TMP75 0~7 Temperature
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

    char f_name[100];

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
        printf("buf : %s\n", buf);
        system(buf);
    }

    for(i = 0 ; i < COUNT_PSU_WATT ; i++)
    {
        FIND_FILE_NAME(TYPE_PSU_WATT, i, f_name);
        sprintf(buf, "echo \"%d\" > %s \n", NVA[i+8], f_name);
        system(buf);
    }
    fclose(fp);
    free(buf2);
}

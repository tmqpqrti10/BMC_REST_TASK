#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include "helper.hpp"
#include <ipmi/ipmi.hpp>
#include <inttypes.h>

#define IPMI_PASSWORD_ENABLE_USER   0x01
#define IPMI_PASSWORD_SET_PASSWORD  0x02
int get_total_power_usage(uint8_t index, char *input, int *res_len) {
	struct ipmi_rq req;
	req.msg.netfn = 0;
	req.msg.cmd = CMD_GET_POWER_USAGE;
	req.msg.data = &index;
	req.msg.data_len = 4;
    return test_sendrecv(&req, input, res_len);
}

int get_lan_param_select(int select, char *input, int *res_len) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_LANINFO;
    req.msg.data_len = 0;
    return test_sendrecv(&req, input, res_len);
}

void get_sys_param_select(int select, unsigned char *input, int *res_len) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_SYSINFO;
    req.msg.data_len = 0;
    test_sendrecv(&req, input, res_len);
}

// int get_ddns_param_select(int select, char *input, int *res_len) {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_GET_DDNSINFO;
//     req.msg.data_len = 0;
//     return test_sendrecv(&req, input, res_len);
// }

/**
*@brief fru 모든 정보 get
*@author doyoung
*/
int get_fru_param_select(int select, char * input, int *res_len) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_FRUINFO;
    req.msg.data_len = 0;
    return test_sendrecv(&req, input, res_len);
}


/**
*@brief fru 헤더 정보 get
*@author doyoung
*/
// Function: enable/disable board, product, chassis with FRU ID
int set_fru_header(char fru_id, char en_board, char en_product, char en_chassis) {
    cout << "enter set_fru_header" << endl;
    
    struct ipmi_rq req;
    char msg_data[4];
    msg_data[0] = fru_id;
    msg_data[1] = en_board;
    msg_data[2] = en_product;
    msg_data[3] = en_chassis;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_FRU_HEADER;
    req.msg.data = msg_data; 
    req.msg.data_len = 4;
    return sendrecv(&req);
}

/**
*@brief fru 보드 정보 get
*@author doyoung
*/
int set_fru_board(char fru_id, char* year, char* month, char* day, char* hour, char* minute, char* sec, 
	char* mfg, char* product, char* serial, char* part_num) {
    
    cout << "enter set_fru_board" << endl;
    
    struct ipmi_rq req;
    int len = 1+LEN_MFG_DATE+LEN_MFG+LEN_PRODUCT+LEN_SERIAL+LEN_PART_NUM+10;

    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_FRU_BOARD;
    
    char msg_data[len];
    memset(&msg_data, 0, len);
    msg_data[0] = fru_id;
    memcpy(msg_data+1, year, 5);
    memcpy(msg_data+1+5, month, 3);
    memcpy(msg_data+1+5+3, day, 3);
    memcpy(msg_data+1+5+3+3, hour, 3);
    memcpy(msg_data+1+5+3+3+3, minute, 3);
    memcpy(msg_data+1+5+3+3+3+3, sec, 3);
    memcpy(msg_data+1+LEN_MFG_DATE, mfg, LEN_MFG);
    memcpy(msg_data+1+LEN_MFG_DATE+LEN_MFG, product, LEN_PRODUCT);
    memcpy(msg_data+1+LEN_MFG_DATE+LEN_MFG+LEN_PRODUCT, serial, LEN_SERIAL);
    memcpy(msg_data+1+LEN_MFG_DATE+LEN_MFG+LEN_PRODUCT+LEN_SERIAL, part_num, LEN_PART_NUM);
    
    req.msg.data = msg_data;
    req.msg.data_len = len;
    return sendrecv(&req);
}

/**
*@brief fru product 정보 get
*@author doyoung
*/
int set_fru_product(char fru_id, char* name, char* mfg, char* version, char* serial, char* part_num) {
    cout << "enter set_fru_product" << endl;
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_FRU_PRODUCT;

    int len = 1+LEN_NAME+LEN_MFG+LEN_VERSION+LEN_SERIAL+LEN_PART_NUM+1;
    char msg_data[len];
    memset(&msg_data, 0, len);
    msg_data[0] = fru_id;
    memcpy(msg_data+1, name, LEN_NAME);
    memcpy(msg_data+1+LEN_NAME, mfg, LEN_MFG);
    memcpy(msg_data+1+LEN_NAME+LEN_MFG, version, LEN_VERSION);
    memcpy(msg_data+1+LEN_NAME+LEN_MFG+LEN_VERSION, serial, LEN_SERIAL);
    memcpy(msg_data+1+LEN_NAME+LEN_MFG+LEN_VERSION+LEN_SERIAL, part_num, LEN_PART_NUM);
    req.msg.data = msg_data;
    req.msg.data_len = len;
    return sendrecv(&req);
}

/**
*@brief fru 샷시 정보 get
*@author doyoung
*/
int set_fru_chassis(char fru_id, char type, char* serial, char* part_num) {
    cout << "enter set_fru_chassis" << endl;
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_FRU_CHASSIS;
    
    int len = 1+LEN_TYPE+LEN_SERIAL+LEN_PART_NUM+1;
    char msg_data[len];
    memset(&msg_data, 0, len);
    msg_data[0] = fru_id;
    msg_data[1] = type;
    memcpy(msg_data+1+LEN_TYPE, serial, LEN_SERIAL);
    memcpy(msg_data+1+LEN_TYPE+LEN_PART_NUM, part_num, LEN_PART_NUM);
    req.msg.data = msg_data;
    req.msg.data_len = len;
    return sendrecv(&req);
}

int get_sensor_param_select(int select, char *input, int *res_len) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_SENSOR;
    req.msg.data_len = 0;
    return test_sendrecv(&req, input, res_len);
}

int get_event_param_select(int select, char *input, int *res_len) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_EVENT;
    req.msg.data_len = 0;
    return test_sendrecv(&req, input, res_len);
}

// int save_event_log() {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_SAVE_EVENT;
//     req.msg.data_len = 0;
//     return sendrecv(&req);
// }

// int get_dcmi_bmcinfo(char *input, int *res_len) {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_DCMI_MC_INFO;
//     req.msg.data_len = 0;
//     return test_sendrecv(&req, input, res_len);
// }

// int get_dcmi_guid(char *input, int *res_len) {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_DCMI_MC_GUID;
//     req.msg.data_len = 0;
//     return test_sendrecv(&req, input, res_len);
// }

// int get_dcmi_asset_tag(char *input, int *res_len) {
//     struct ipmi_rq req; /* request data to send to the BMC */
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0; /* 0x2C per 1.1 spec */
//     req.msg.cmd = CMD_DCMI_ASSET_TAG;    //0x06; 
//     req.msg.data_len = 0; /* How many times does req.msg.data need to read */
//     return test_sendrecv(&req, input, res_len);
// }

// int set_dcmi_asset_tag(char *new_asset) {
//     struct ipmi_rs * rsp; /* ipmi response */
//     struct ipmi_rq req; /* request data to send to the BMC */
//     uint8_t msg_data[64]; /* 'raw' data to be sent to the BMC */
//     memset(&req, 0, sizeof(req));
//     memset(msg_data, 0, sizeof(msg_data));
//     memcpy(msg_data, new_asset, strlen(new_asset)+1);
//     req.msg.netfn = 0; /* 0x2C per 1.1 spec */
//     req.msg.cmd = CMD_SET_ASSETTAG; //0x08; 
//     req.msg.data = msg_data; /* msg_data above */
//     req.msg.data_len = 64; /* How many times does req.msg.data need to read */
//     return sendrecv(&req);
// }

// int get_dcmi_mc_id(char *input, int *res_len) {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_DCMI_MNGCTRL_ID;
//     req.msg.data_len = 0;
//     return test_sendrecv(&req, input, res_len);
// }

// int set_dcmi_mc_id(char *new_mc_id) {
//     struct ipmi_rq req;
//     uint8_t msg_data[64];
//     memset(&req, 0, sizeof(req));
//     memset(&msg_data, 0, sizeof(msg_data));
//     memcpy(msg_data, new_mc_id, strlen(new_mc_id)+1);
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_SET_MC_ID; //0x0A;
//     req.msg.data = msg_data;
//     req.msg.data_len = 64;
//     return sendrecv(&req);
// }

// int set_dcmi_power_ctl(int select) {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_SET_POWER_CTL; //0x08;
//     req.msg.data = &select;
//     req.msg.data_len = 4;
//     return sendrecv(&req);
// }
// int get_dcmi_inlet_temp(char *input, int *res_len) {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_DCMI_INLET_TEMP;
//     req.msg.data_len = 0;
//     return test_sendrecv(&req, input, res_len);
// }

// int get_dcmi_cpu_temp(char *input, int *res_len) {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_DCMI_CPU_TEMP;
//     req.msg.data_len = 0;
//     return test_sendrecv(&req, input, res_len);
// }

// int get_dcmi_baseboard_temp(char *input, int *res_len) {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_DCMI_BASEBOARD_TEMP;
//     req.msg.data_len = 0;
//     return test_sendrecv(&req, input, res_len);
// }

// int get_log_in_ipmi_sel_format(char *input, int *res_len) {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_DCMI_GET_SEL;
//     req.msg.data_len = 0;
//     return test_sendrecv(&req, input, res_len);
// }

// int clear_log() {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_DCMI_CLEAR_LOG;
//     req.msg.data_len = 0;
//     return sendrecv(&req);
// }

// int set_sensor_fan_speed(int speed) {
//     struct ipmi_rq req;
//     uint8_t msg_data[5];
//     memset(&msg_data, 0, sizeof(msg_data));
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_SENSOR_FAN_CTL;
//     req.msg.data = &speed;
//     req.msg.data_len = 4;
//     return sendrecv(&req);
// }

int get_cmdline_macaddr(char *arg, uint8_t *buf) {
    uint32_t m1 = 0;
    uint32_t m2 = 0;
    uint32_t m3 = 0;
    uint32_t m4 = 0;
    uint32_t m5 = 0;
    uint32_t m6 = 0;
    if (sscanf(arg, "%02x:%02x:%02x:%02x:%02x:%02x", &m1, &m2, &m3, &m4, &m5,
            &m6) != 6) {
        return -1;
    }
    if (m1 > UINT8_MAX || m2 > UINT8_MAX || m3 > UINT8_MAX || m4 > UINT8_MAX
            || m5 > UINT8_MAX || m6 > UINT8_MAX) {
        return -1;
    }
    buf[0] = (uint8_t) m1;
    buf[1] = (uint8_t) m2;
    buf[2] = (uint8_t) m3;
    buf[3] = (uint8_t) m4;
    buf[4] = (uint8_t) m5;
    buf[5] = (uint8_t) m6;
    return 0;
}

int set_network_mac(char dev, char* mac) {
    struct ipmi_rq req;
    uint8_t data[32];
    uint8_t msg_data[32];
    memset(&req, 0, sizeof(req));
    memset(&data, 0, sizeof(data));
    memset(&msg_data, 0, sizeof(msg_data));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_MAC_ADDR;

    get_cmdline_macaddr(mac, data);
    msg_data[0] = dev;
    memcpy(msg_data+1, data, 32);
    req.msg.data = msg_data;
    req.msg.data_len = 32;
    return sendrecv(&req);
}

int set_network_ipv4_dhcp(char dev, char dhcp_or_static) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_IPV4_DHCP;

    char msg_data[2];
    msg_data[0] = dev;
    msg_data[1] = dhcp_or_static; // static = 0, dhcp = 1
    
    req.msg.data = msg_data;
    req.msg.data_len = 2;
    return sendrecv(&req);
}

int get_cmdline_ipaddr(char * arg, uint8_t * buf) {
    uint32_t ip1, ip2, ip3, ip4;
    if (sscanf(arg,
            "%" PRIu32 ".%" PRIu32 ".%" PRIu32 ".%" PRIu32,
            &ip1, &ip2, &ip3, &ip4) != 4) {
        return (-1);
    }
    if (ip1 > UINT8_MAX || ip2 > UINT8_MAX || ip3 > UINT8_MAX
            || ip4 > UINT8_MAX) {
        return (-1);
    }
    buf[0] = (uint8_t) ip1;
    buf[1] = (uint8_t) ip2;
    buf[2] = (uint8_t) ip3;
    buf[3] = (uint8_t) ip4;
    return 0;
}

int set_network_ipv4_ip(char dev, char* ip, char* netmask) {
    struct ipmi_rq req;
    uint8_t data[32];
    uint8_t data2[32];
    uint8_t msg_data[32];
    memset(&req, 0, sizeof(req));
    memset(&data, 0, sizeof(data));
    memset(&data2, 0, sizeof(data2));
    get_cmdline_ipaddr(ip, data);
    get_cmdline_ipaddr(netmask, data2);

    msg_data[0] = dev ;
    memcpy(msg_data+1, data, 4);
    memcpy(msg_data+1+4, data2, 4);
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_IPV4_IP_NETMASK;
    req.msg.data = msg_data;
    req.msg.data_len = 32;
    return sendrecv(&req);
}

int set_network_gateway(char dev, char* gate) {
    struct ipmi_rq req;
    uint8_t data[32];
    uint8_t msg_data[32];
    memset(&req, 0, sizeof(req));
    memset(&data, 0, sizeof(data));
    get_cmdline_ipaddr(gate, data);
    msg_data[0] = dev ;
    memcpy(msg_data + 1 , data, 32);
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_IPV4_GATEWAY;
    req.msg.data = msg_data;
    req.msg.data_len = 32;
    return sendrecv(&req);
}

int set_network_vlan_enable(char dev, char enable) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_VLAN_ENABLE;
    char msg_data[2];
    msg_data[0] = dev ;
    msg_data[1] = enable;
    req.msg.data = msg_data;
    req.msg.data_len = 2;
    return sendrecv(&req);
}

int set_network_vlan_id(char dev, char* id) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_VLAN_ID;

    char msg_data[2];
    msg_data[0] = dev ;
    msg_data[1] = strtol(id, NULL, 16);
    //memcpy(msg_data+1, id, strlen(id));
    req.msg.data = msg_data;
    req.msg.data_len = 2;
    return sendrecv(&req);
}

int set_network_vlan_priority(char dev, char* priority) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_VLAN_PRIORITY;

    char msg_data[2];
    msg_data[0] = dev ;
    msg_data[1] = strtol(priority, NULL, 16);
    req.msg.data = msg_data;
    req.msg.data_len = 2;
    return sendrecv(&req);
}

int set_network_ipv6_enable(char dev, char en_disable) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_IPV6_ENABLE;

    char msg_data[2];
    msg_data[0] = dev;
    msg_data[1] = en_disable;
    req.msg.data = msg_data;
    req.msg.data_len = 2;
    return sendrecv(&req);
}

int set_network_ipv6_dhcp(char dev, char dhcp_or_static) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_IPV6_DHCP;

    char msg_data[2];
    msg_data[0] = dev;
    msg_data[1] = dhcp_or_static;
    req.msg.data = msg_data;
    req.msg.data_len = 2;
    return sendrecv(&req);
}

int set_network_ipv6_ip(char dev, char *ip) {
    struct ipmi_rq req;
    uint8_t msg_data[40] = {0, };
    memset(&req, 0, sizeof(req));
    
    msg_data[0] = dev;
    memcpy(msg_data + 1, ip, 39);
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_IPV6_IP;
    req.msg.data = msg_data;
    req.msg.data_len = 40;
    return sendrecv(&req);
}

int set_network_ipv6_prefix(char dev, char* prefix) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_IPV6_PREFIX;
    char msg_data[2];
    msg_data[0] = dev ;
    msg_data[1] = strtol(prefix, NULL, 10);
    req.msg.data = msg_data;
    req.msg.data_len = 2;
    return sendrecv(&req);
}

int set_network_ipv6_gateway(char dev, char *gw) {
    struct ipmi_rq req;
    uint8_t msg_data[40] = {0, };
    memset(&req, 0, sizeof(req));

    msg_data[0] = dev;
    memcpy(msg_data + 1, gw, 39);
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_IPV6_GATEWAY;
    req.msg.data = msg_data;
    req.msg.data_len = 40;
    return sendrecv(&req);  
}

int set_network_ipv6_priority(char dev) {
    struct ipmi_rq req;
    uint8_t msg_data[1];
    msg_data[0] = dev;
    memset(&req, 0, sizeof(req));

    printf("im here! %c \n", dev);
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LAN_PRIORITY;
    req.msg.data = msg_data;
    req.msg.data_len = 1;
    return sendrecv(&req);
}
// int get_cmdline_threshold(char * ucr, char* unc, char* unr, char* lcr,
//         char* lnc, char* lnr, float * buf) {
//     float t1, t2, t3, t4, t5, t6;
//     sscanf(ucr, "%f", &t1);
//     sscanf(unc, "%f", &t2);
//     sscanf(unr, "%f", &t3);
//     sscanf(lcr, "%f", &t4);
//     sscanf(lnc, "%f", &t5);
//     sscanf(lnr, "%f", &t6);
//     buf[0] = t1;
//     buf[1] = t2;
//     buf[2] = t3;
//     buf[3] = t4;
//     buf[4] = t5;
//     buf[5] = t6;
//     return 0;
// }

int set_sensor_threshold(float ucr, float unc, float unr, float lcr, float lnc, float lnr, int snr)
{
    printf("\t\t\tdy : enter set sensor threshold\n");
    printf("\t\t\tdy : threshold value.. ucr = %f, unc = %f, unr = %f, lcr = %f, lnc = %f, lnr = %f, snr = %d\n", ucr, unc, unr, lcr, lnc, lnr, snr);
    struct ipmi_rq req;
    float data[6];
    float msg_data[6];
    memset(msg_data, 0, sizeof(msg_data));
    msg_data[0] = ucr;
    msg_data[1] = unc;
    msg_data[2] = unr;
    msg_data[3] = lcr;
    msg_data[4] = lnc;
    msg_data[5] = lnr;
    msg_data[6] = (float)snr;
    req.msg.netfn = snr;
    printf("snr : %d\n", (int)msg_data[6]);
    
    req.msg.cmd = CMD_SENSOR_SET_THRESH;
    req.msg.data = (uint8_t *)msg_data;
    req.msg.data_len = sizeof(float) * 10;
    
    return sendrecv(&req);
}

// int get_pef_list() {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_GET_PEF_LIST;
//     req.msg.data_len = 0;
//     return sendrecv(&req);
// }

// typedef struct {
//     unsigned char policy_num[5];
//     unsigned char dst_select[10];
//     unsigned char address[30];
//     unsigned char alert_msg[30];
// } alert_policy_table_t;


// int get_pef_policy() {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
// 	req.msg.netfn = 0;
//     req.msg.cmd = CMD_GET_PEF_POLICY;  // CMD_GET_PEF_POLICY
//     req.msg.data_len = 0;
//     return sendrecv(&req);
// }

// int send_test_alert(int index) {
//     struct ipmi_rq req;
//     memset(&req, 0, sizeof(req));
//     req.msg.netfn = 0;
//     req.msg.cmd = CMD_ALERT_TEST;  // CMD_ALERT_TEST
//     req.msg.data = &index;
//     req.msg.data_len = 4;
//     return sendrecv(&req);
// }


int set_pef_policy(uint8_t index, uint8_t policy_set, uint8_t rule, uint8_t alert_num) {
	struct ipmi_rq req;
	uint8_t msg_data[4];
	msg_data[0] = index;
	msg_data[1] = policy_set;
	msg_data[2] = rule;
	msg_data[3] = alert_num;
	memset(&req, 0, sizeof(req));
	req.msg.netfn = 0;
	req.msg.cmd = CMD_SET_PEF_POLICY;
	req.msg.data = msg_data;
	req.msg.data_len = 4;
	return sendrecv(&req);
}

int set_lan_alert(uint8_t* index, uint8_t* ip, uint8_t ack) {
	struct ipmi_rq req;
	uint8_t msg_data[6];
	memset(&msg_data, 0, sizeof(msg_data));
	get_cmdline_ipaddr(ip, msg_data);
	strcat(msg_data, "\0");
	msg_data[5] = ack;

	memset(&req, 0, sizeof(req));
	req.msg.netfn = (int)strtol(index, NULL, 16); //used as alert number(index) to set ip address
	req.msg.cmd = CMD_SET_LAN_ALERT;
	req.msg.data = msg_data;
	req.msg.data_len = 6;
	return sendrecv(&req);
}

int del_pef_policy(u_int8_t index) {
	struct ipmi_rq req;
	memset(&req, 0, sizeof(req));
	req.msg.netfn = 0;
	req.msg.cmd = CMD_DEL_PEF_POLICY;
	req.msg.data = &index;
	req.msg.data_len = 4;
	return sendrecv(&req);
}

int get_lan_alert() {
	struct ipmi_rq req;
	memset(&req, 0, sizeof(req));
	req.msg.netfn = 0;
	req.msg.cmd = CMD_GET_LAN_DESTINATION;
	req.msg.data_len = 0;
	return sendrecv(&req);
}

int get_user_list(char *input, int *res_len) {
    struct ipmi_rq req;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_USER_LIST;
    req.msg.data_len = 0;
    return test_sendrecv(&req, input, res_len);
}

int set_user_enable(char* index, char* enable) {
    struct ipmi_rq req;
    uint8_t msg_data[2];
    int i = (int)strtol(index, NULL, 10);
    msg_data[0] = i;
	int enable_int = (int)strtol(enable, NULL, 10);
	msg_data[1] = enable_int; // 0 and 1 is to operate and select enable/disable
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_USER_PASSWORD;
    req.msg.data = msg_data;
    req.msg.data_len = strlen(msg_data)+1;
    i = sendrecv(&req);
    return i;
}

int set_user_password(char* index, char* password, char* type) {
    struct ipmi_rq req;
    uint8_t msg_data[22] = "\0";
    uint8_t password_type = 16;
    str2uchar(type, &password_type);
    if (password_type > 16)
        msg_data[0] = 0x80;
    else
        msg_data[1] = 0x00;
    msg_data[0] |= (int)strtol(index, NULL, 10) & 0x1F; // id
    msg_data[1] = 0x03 & IPMI_PASSWORD_SET_PASSWORD; // set password OR enable?
    strcat(msg_data, password); // load password data

    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_USER_PASSWORD;
    req.msg.data = msg_data;
    req.msg.data_len = 22;
    int i = 0;
    i = sendrecv(&req);
    return i;
}

int set_user_access(char* index, char* enable, char* callin, char* linkauth, char* ipmimsg, char* priv)
{
    struct ipmi_rq req;
    uint8_t msg_data[3];
    memset(&req, 0, sizeof(req));
	msg_data[0] = 0; // is_priv_only
	msg_data[1] = (int)strtol(index, NULL, 10);
	msg_data[2] = (int)strtol(priv, NULL, 10);
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_USER_ACCESS;
    req.msg.data = msg_data;
    req.msg.data_len = sizeof(uint8_t) * 3;
    int i = 0;
    i = sendrecv(&req);
    return i;
}

int delete_user(char index) {
    struct ipmi_rq req;
    uint8_t msg_data[1];
    msg_data[0] = index;
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_DEL_USER;
    req.msg.data = msg_data;
    req.msg.data_len = 1;
    int i = 0;
    i = sendrecv(&req);
    return i;
}

int show_main(uint8_t index, unsigned char *input, int *res_len) {
	struct ipmi_rq req;
	req.msg.netfn = 0;
	req.msg.cmd = CMD_SHOW_MAIN;
	req.msg.data = &index;
	req.msg.data_len = 4;
    return test_sendrecv(&req, input, res_len);
}

int get_smtp_configuration(char *input, int *res_len) {
	struct ipmi_rq req;
	req.msg.netfn = 0;
	req.msg.cmd = CMD_GET_SMTPINFO;
	req.msg.data_len = 0;
	return test_sendrecv(&req, input, res_len);
}

int set_smtp_sender(char* sender, char* machine) {
	struct ipmi_rq req;
	char msg_data[64];
	req.msg.netfn = 0;
	req.msg.cmd = CMD_SET_SMTP_SENDER;
	strncpy(msg_data, sender, 32);
	strncpy(msg_data+32, machine, 32);
	req.msg.data = msg_data;
	req.msg.data_len = 64;
	return sendrecv(&req);
}

int set_smtp_primary(char* server, char* id, char* pwd) {
	struct ipmi_rq req;
	char msg_data[64];
	req.msg.netfn = 0;
	req.msg.cmd = CMD_SET_SMTP_PRIMARY;
	strncpy(msg_data, server, 20);
	strncpy(msg_data+20, id, 20);
	strncpy(msg_data+40, pwd, 24);
	req.msg.data = msg_data;
	req.msg.data_len = 64;
	return sendrecv(&req);
}

int set_smtp_secondary(char* server, char* id, char* pwd) {
	struct ipmi_rq req;
	char msg_data[64];
	req.msg.netfn = 0;
	req.msg.cmd = CMD_SET_SMTP_SECONDARY;
	strncpy(msg_data, server, 20);
	strncpy(msg_data+20, id, 20);
	strncpy(msg_data+40, pwd, 24);
	req.msg.data = msg_data;
	req.msg.data_len = 64;
	return sendrecv(&req);
}

/**
*@brief ipmi power 상태 get
*@author doyoung
*/
int get_power_status(unsigned char *input, int *res_len) {
	struct ipmi_rq req;
	req.msg.netfn = 0;
	req.msg.cmd = CMD_GET_POWER_STATUS;
	req.msg.data_len = 0;
	return test_sendrecv(&req, input, res_len);
}

/**
*@brief ipmi power 상태 set
*@author doyoung
*/
int set_power_status(int status, char *res_msg, int *res_len) {
	struct ipmi_rq req;
	/* status number on REST -> status on KETI-IPMI */
	if (status == 1)
		status = 0x3;
	else if (status == 2)
		status = 0x0;
	else if (status == 3)
		status = 0x6;
	else if (status == 4)
		status = 0x1;
	else if (status == 5)
		status = 0x2;
	req.msg.netfn = 0;
	req.msg.cmd = CMD_SET_POWER_STATUS;
	req.msg.data = (uint8_t *)&status;
	req.msg.data_len = 4;
	return test_sendrecv(&req, res_msg, res_len);
}

int get_ldap_info(char *input, int *res_len) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_LDAP;
    req.msg.data_len = 0;
    return test_sendrecv(&req, input, res_len);
}

int set_ldap_enable(char *enable) {
	struct ipmi_rq req;
	req.msg.netfn = 0;
	req.msg.cmd = CMD_SET_LDAP_ENABLE;
	req.msg.data = enable;
	req.msg.data_len = 2;
	int i = 0;
    sendrecv(&req);
    return i;
}

int set_ldap_ip(char* ipaddr) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LDAP_IP;
    req.msg.data = ipaddr;
    req.msg.data_len = 32;
    int i = sendrecv(&req);
    return i;
}

int set_ldap_port(char* port) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LDAP_PORT;
    req.msg.data = port;
    req.msg.data_len = 16;
    int i = sendrecv(&req);
    return i;
}

int set_ldap_searchbase(char* searchbase) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LDAP_SEARCHBASE;
    req.msg.data = searchbase;
    req.msg.data_len = 32;
    int i = sendrecv(&req);
    return i;
}

int set_ldap_binddn(char* binddn) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LDAP_BINDDN;
    req.msg.data = binddn;
    req.msg.data_len = 32;
    int i = sendrecv(&req);
    return i;
}

int set_ldap_password(char* password) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LDAP_PASSWORD;
    req.msg.data = password;
    req.msg.data_len = 32;
    int i = sendrecv(&req);
    return i;
}

int set_ldap_ssl(char* ssl) {
	struct ipmi_rq req;
	req.msg.netfn = 0;
	req.msg.cmd = CMD_SET_LDAP_SSL;
	req.msg.data = ssl;
	req.msg.data_len = 2;
	int i = sendrecv(&req);
    return i;
}

/* ssl, timelimit could be transfered as integer simply so I bind them */
int set_ldap_timelimit(char *timelimit) {
    struct ipmi_rq req; 
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_LDAP_TIMELIMIT;
    req.msg.data[0] = atoi(timelimit);
    req.msg.data_len = sizeof(int);
    int i = sendrecv(&req);
    return i;
}

int get_dns_info(char *input, int *res_len) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_DNS;
    req.msg.data = "a";
    req.msg.data_len = 1;
    return test_sendrecv(&req, input, res_len);
}

int set_dns_domain_info(unsigned char *domain_name) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_DNS_DOMAIN;
    req.msg.data = domain_name;
    req.msg.data_len = strlen(domain_name)+1;
    return sendrecv(&req);
}

int set_dns_hostname_info(unsigned char *host_name) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_DNS_HOSTNAME;
    req.msg.data = host_name;
    req.msg.data_len = strlen(host_name)+1;
    int rets = 0;
    rets = sendrecv(&req);
    return rets;
}

int set_dns_ip_prefer(unsigned char *ipv4) {
    struct ipmi_rq req;
    unsigned char data[32];
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_DNS_IPV4_PREFER;
    req.msg.data = ipv4;
    req.msg.data_len = strlen(ipv4)+1;
    return sendrecv(&req);
}

int set_dns_ip_alter(unsigned char *ipv4) {
    struct ipmi_rq req;
    unsigned char data[32];
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_DNS_IPV4_ALTER;
    req.msg.data = ipv4;
    req.msg.data_len = strlen(ipv4)+1;
    return sendrecv(&req);
}

int set_dns_ipv6_prefer(unsigned char *ipv6) {
    struct ipmi_rq req;
    unsigned char data[32];
    memset(data, 0, sizeof(data));
    get_cmdline_macaddr(ipv6, data);
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_DNS_IPV6_PREFER;
    req.msg.data = data;
    req.msg.data_len = 32;
    return sendrecv(&req);
}

int get_radius_info(char *input, int *res_len) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_RADIUS;
    req.msg.data_len = 0;
    return test_sendrecv(&req, input, res_len);
}

int set_radius_disable() {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_RADIUS_DISABLE;
    req.msg.data_len = 0;
    int i = 0;
    i = sendrecv(&req);
    return i;
}

int set_radius_info(char* ip, char* port, char* secret) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_RADIUS;
    char msg_data[16+6+16]; 
    memcpy(msg_data, ip, 16);
    memcpy(msg_data+16, port, 6);
    memcpy(msg_data+6+16, secret, 16);
    req.msg.data = msg_data;
    req.msg.data_len = 16+6+16;
    int i = sendrecv(&req);
    return i;
}

int get_active_directory_info(char *input, int *res_len) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_ACTIVE_DIRECTORY;
    req.msg.data_len = 0;
    return test_sendrecv(&req, input, res_len);
}

int set_active_directory_enable(char *enable) {
    struct ipmi_rq req;
    req.msg.cmd = CMD_SET_ACTIVE_DIRECTORY_ENABLE;
    req.msg.data = enable;
    req.msg.data_len = 2;
    int i = sendrecv(&req);
    return i;
}

int set_active_directory_ip_pwd(char* ip, char* pwd) {
    struct ipmi_rq req;
	char msg_data[82];
	strcpy(msg_data, ip); 
	strcpy(msg_data+16, pwd); 
    req.msg.cmd = CMD_SET_ACTIVE_DIRECTORY_IP_PWD;
	req.msg.data = msg_data;
    req.msg.data_len = 82;
    int i = sendrecv(&req);
    return i;
}

int set_active_directory_domain(char* domain) {
    struct ipmi_rq req;
    req.msg.cmd = CMD_SET_ACTIVE_DIRECTORY_DOMAIN;
    req.msg.data = domain;
    req.msg.data_len = strlen(domain)+1;
    int i = sendrecv(&req);
    return i;
}

int set_active_directory_username(char* s_username) {
    struct ipmi_rq req;
    req.msg.cmd = CMD_SET_ACTIVE_DIRECTORY_USERNAME;
    req.msg.data = s_username;
    req.msg.data_len = strlen(s_username) + 1;
    int i = sendrecv(&req);
    return i;
}

int get_ntp_info(char *input, int *res_len) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_NTP;
    req.msg.data_len = 0;
    return test_sendrecv(&req, input, res_len);
}

int set_ntp_auto(char* server, char *input, int *res_len) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_NTP_AUTO;
    req.msg.data = server;
    req.msg.data_len = 32;
    return test_sendrecv(&req, input, res_len);
}
int get_ssl_info(char *input, int *res_len) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_GET_SSL;
    req.msg.data_len = 0;
    return test_sendrecv(&req, input, res_len);
}

// char* put_msg_data(char* argu) {
// 	int len = strlen(argu);
// 	if (len > 0) {
// 		char* newstrptr = (char*)malloc(sizeof(char)*len+1);
// 		strcpy(newstrptr, argu);
// 		return newstrptr;
// 	}
// }

int set_ssl_info_1(char* keylen, char* country, char* state, char* city, char* organ, char* valid) {
    struct ipmi_rq req;
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_SSL_1;
	char msg_data[64];

	strncpy(msg_data, keylen, 6);
	strncpy(msg_data+6, country, 3);
	strncpy(msg_data+6+3, state, 16);
	strncpy(msg_data+6+3+16, city, 16);
	strncpy(msg_data+6+3+16+16, organ, 16);
	strncpy(msg_data+6+3+16+16+16, valid, 4);
	req.msg.data = msg_data;
    req.msg.data_len = 64;
    return sendrecv(&req);
}

int set_ssl_info_2(char* organ_unit, char* cn, char* email) {
    struct ipmi_rq req;
	char msg_data[64];
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_SSL_2;
	strncpy(msg_data, organ_unit, 16);
	strncpy(msg_data+16, cn, 16);
	strncpy(msg_data+16+16, email, 32);
	req.msg.data = msg_data;
    req.msg.data_len = 64;
    return sendrecv(&req);
}

int try_login(char* username, char* pwd, unsigned char *input, int *res_len) {
    struct ipmi_rq req;
    char msg_data[32];
    req.msg.netfn = 0;
    req.msg.cmd = CMD_TRY_LOGIN;
    memset(&msg_data, 0, 32);
    memcpy(msg_data, username, 16);
    memcpy(msg_data+16, pwd, 16);

    req.msg.data = msg_data;
    req.msg.data_len = 32;
    return test_sendrecv(&req, input, res_len);
}

int set_user_name(char* index, char* name) {
    struct ipmi_rq req;
    uint8_t msg_data[15];
    int i = atoi(index);
    memset(msg_data, 0, sizeof(msg_data));
    msg_data[0] = i;
    printf("username : %s\n", name);
    strncpy(msg_data+1, name, strlen(name));
    memset(&req, 0, sizeof(req));
    req.msg.netfn = 0;
    req.msg.cmd = CMD_SET_USER_NAME;
    req.msg.data = msg_data;
    req.msg.data_len = strlen(msg_data)+1;
    i = sendrecv(&req);
    return i;
}

int request_kvm_close() {
	return send_to_kvm();
}

int request_get_setting_service(char *input, int *res_len) {
	cout << "enter request get setting service" << endl;
    struct ipmi_rq req;
	req.msg.netfn = 0;
	req.msg.cmd = CMD_GET_SETTING_SERVICE;
	req.msg.data_len = 0;
	return test_sendrecv(&req, input, res_len);
}

int request_set_setting_service(int flag, char* str) {

	struct ipmi_rq req;
	req.msg.netfn = 0;
	req.msg.cmd = CMD_SET_SETTING_SERVICE;
	char msg_data[32] = {0,};
	msg_data[0] = flag;
	strcpy(msg_data+1, str);
	req.msg.data = msg_data;
	req.msg.data_len = 32;
	return sendrecv(&req);
}

// #if 0
// /*-----------------------------------------------------------------*/
// char* ipmi_lanp_main(int argc, char** argv) {
//     char* ret;
//     //uint8_t chan = 1;

//     if (strcmp(argv[1], "lan") == 0) {
//         if (argc < 3) {
//             get_lan_param_select(0);
//         } 
//         else if (strcmp(argv[2], "set") == 0) {
//             char dev = 0        ;
//             if (strcmp(argv[3], "eth0") == 0)
//                 dev = 0;
//             else if (strcmp(argv[3], "eth1") == 0)
//                 dev = 1;
//             if (strcmp(argv[4], "ip") == 0) { //req lan lan set ip ip_addr netmask_addr
//                 set_network_ipv4_ip(dev, argv[5], argv[6]);
//             } 
//             else if (strcmp(argv[4], "dhcp") == 0 || strcmp(argv[4], "static") == 0) {
//                 set_network_ipv4_dhcp(dev, argv[4]);
//             } 
//             /*
//             else if (strcmp(argv[4], "static") == 0) {
//                 set_network_ipv4_dhcp(dev, "1");
//             } 
//             */
//             else if (strcmp(argv[4], "gateway") == 0) {
//                 set_network_gateway(dev, argv[5]);
//             } 
//             else if (strcmp(argv[4], "mac") == 0) {
//                 set_network_mac(dev, argv[5]);
//             } 

//             else if (strcmp(argv[4], "vlan") == 0) {
//                 if (strcmp(argv[5], "enable") == 0) {
//                     set_network_vlan_enable(dev, 1); // 1 or 0
//                 } 
//                 else if (strcmp(argv[5], "disable") == 0) {
//                     set_network_vlan_enable(dev, 0);
//                 }
//                 else if (strcmp(argv[5], "id") == 0) {
//                     set_network_vlan_id(dev, argv[6]);
//                 } 
//                 else if (strcmp(argv[5], "priority") == 0) {
//                     set_network_vlan_priority(dev, argv[6]);
//                 }
//             } else if (strcmp(argv[4], "ipv6") == 0) {
//                 if (strcmp(argv[5], "enable") == 0) {
//                     set_network_ipv6_enable(dev, 1); // 1 or 0
//                 }
//                 else if (strcmp(argv[5], "disable") == 0) {
//                     set_network_ipv6_enable(dev, 0);
//                 }
//                 else if (strcmp(argv[5], "dhcp") == 0) {
//                     set_network_ipv6_dhcp(dev, 0);
//                 } 
//                 else if (strcmp(argv[5], "static") == 0) {
//                     set_network_ipv6_dhcp(dev, 1);
//                 } 
//                 else if (strcmp(argv[5], "address") == 0) {
//                     set_network_ipv6_ip(dev, argv[6], argv[7], argv[8], argv[9], argv[10], \
//                         argv[11], argv[12], argv[13], argv[14], argv[15], argv[16], argv[17], \
//                         argv[18], argv[19], argv[20], argv[21]);
//                 } 
//                 else if (strcmp(argv[5], "prefix") == 0) {
//                     set_network_ipv6_prefix(dev, argv[6]);
//                 } 
//                 else if (strcmp(argv[5], "gateway") == 0) {
//                     //set_network_ipv6_gateway(dev, argv[6]);
//                     set_network_ipv6_gateway(dev, argv[6], argv[7], argv[8], argv[9], argv[10], \
//                         argv[11], argv[12], argv[13], argv[14], argv[15], argv[16], argv[17], \
//                         argv[18], argv[19], argv[20], argv[21]);                    
//                 }
//             }
//         }
// 		else if (strcmp(argv[2], "alert") == 0) {
// 			if (argc < 4) {
// 				get_lan_alert();
// 				// get lan alert json
// 			}
// 			else if (strcmp(argv[3], "print") == 0) {
// 				// get lan alert json
// 			}
// 			else if (strcmp(argv[3], "set") == 0) {
// 				// set lan alert json
// 				// ./req lan alert set <index> <ip> <ack>
// 				set_lan_alert(argv[4], argv[5], argv[6]);
// 			}
// 		}
//     } 
//     else if (strcmp(argv[1], "sysinfo") == 0) {
//         get_sys_param_select(0);
//     } 
//     else if (strcmp(argv[1], "ddnsinfo") == 0) {
//         get_ddns_param_select(0);
//     } 
//     else if (strcmp(argv[1], "fru") == 0) {
// 	if (strcmp(argv[2], "get") == 0)
//             get_fru_param_select(0);
// 	else if (strcmp(argv[2], "set") == 0){
// 	    if (strcmp(argv[3], "hdr") == 0) {
// 	        set_fru_header(argv[4], argv[5], argv[6], argv[7]);
// 		// req fru set hdr <fru_id> <board:1/0> <product:1/0> <chassis:1/0>
// 	    }
// 	    else if (strcmp(argv[3], "board") == 0) {
// 		set_fru_board(argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], argv[10], \
// 			argv[11], argv[12], argv[13], argv[14], argv[15]);
// 		// req fru set board <fru_id> <mfg_date> <mfg> <product> <serial> <part_num>
// 	    }
// 	    else if (strcmp(argv[3], "product") == 0) {
// 		set_fru_product(argv[4], argv[5], argv[6], argv[7], argv[8], argv[9]);
// 		// req fru set product <fru_id> <name> <mfg> <version> <serial> <part_num>
// 	    }
// 	    else if (strcmp(argv[3], "chassis") == 0) {
// 		set_fru_chassis(argv[4], argv[5], argv[6], argv[7]);
// 		// req fru set chassis <fru_id> <type> <serial> <part_num>
// 	    }
// 	}
	
//     }
//     else if (strcmp(argv[1], "sensor") == 0) {
//         if (argc < 3) {
//             get_sensor_param_select(0);
//         } 
//         else if (strcmp(argv[2], "fan") == 0) {
//             set_sensor_fan_speed(atoi(argv[4]));
//         } 
//         else if (strcmp(argv[2], "thresh") == 0) {
//             set_sensor_threshold(argv[3], argv[4], argv[5], argv[6],
//                     argv[7], argv[8], argv[9]);
//         }
//     } 
//     else if (strcmp(argv[1], "event") == 0) {
// 	if (argc < 3)
//             get_event_param_select(0);
// 	else if (strcmp(argv[2], "save") == 0)
// 	    save_event_log();
//     }
//     else if (strcmp(argv[1], "dcmi") == 0) {
//         if (strcmp(argv[2], "info") == 0) {
//             get_dcmi_bmcinfo();
//         } 
//         else if (strcmp(argv[2], "guid") == 0) {
//             get_dcmi_guid(0);
//         } 
//         else if (strcmp(argv[2], "asset_tag") == 0) {
//             get_dcmi_asset_tag();
//         } 
//         else if (strcmp(argv[2], "set_asset_tag") == 0) {
//             set_dcmi_asset_tag(argv[3]);
//         } 
//         else if (strcmp(argv[2], "get_mc_id_string") == 0) {
//             get_dcmi_mc_id();
//         } 
//         else if (strcmp(argv[2], "set_mc_id_string") == 0) {
//             set_dcmi_mc_id(argv[3]);
//         } 
//         else if (strcmp(argv[2], "power") == 0) {
//             if (strcmp(argv[3], "on") == 0) {
//                 set_dcmi_power_ctl(1);
//             } 
//             else if (strcmp(argv[3], "off") == 0) {
//                 set_dcmi_power_ctl(2);
//             } 
//             else if (strcmp(argv[3], "reset") == 0) {
//                 set_dcmi_power_ctl(3);
//             }
//         }
//         else if (strcmp(argv[2], "inlet_temp") == 0){
//             get_dcmi_inlet_temp();
//         }
//         else if (strcmp(argv[2], "cpu_temp") == 0){
//             get_dcmi_cpu_temp();
//         }
//         else if (strcmp(argv[2], "baseboard_temp") == 0){
//             get_dcmi_baseboard_temp();
//         }
// 	else if (strcmp(argv[2], "sel") == 0) {
// 	    get_log_in_ipmi_sel_format();
// 	}
// 	else if (strcmp(argv[2], "clear_log") == 0) {
// 	    clear_log();
// 	}
//     }
//     else if (strcmp(argv[1], "pef") == 0) {
// //        puts("pef 분기");
//         if (argc < 3 ) {
//             printf("%s\n", "Usage: req pef list / req pef policy\n");
            
//         }
//         else if (strcmp(argv[2], "list") == 0) {
//             get_pef_list();
//         }
//         else if (strcmp(argv[2], "policy") == 0) {
//             if (argc == 3) {
//                 get_pef_policy();
//             }
//             else if (strcmp(argv[3], "set") == 0) {
// 				if (argc < 7) {
// 					printf("Usage: req pef policy set <index> <policy#> <rule> <lan alert#>");
// 					return 1;
// 				}
// 				// req pef policy set   <index>     <policy_set>   <rule>         <lan alert#>
// 				set_pef_policy(atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7]));
// //				set_lan_alert(intf, argv[8], argv[9], argv[10]);
//             }
//             else if (strcmp(argv[3], "del") == 0) {
// 				// req pef policy del <policy#>
//                 del_pef_policy(atoi(argv[4]));
//             }
// 			else {
// 				printf("%s\n", "Usage: req pef policy set <index> <policy#> <rule> <lan alert#>");
// 				printf("%s\n", "---------------------------------------------------------------------------------");
// 				printf("%s\n", "<index>      the index of each policy just for alignment");
// 				printf("%s\n", "<policy#>    the policy number. It would be used to find the matching policy");
// 				printf("%s\n", "<rule>       0-Disable, 8-Match Always, 9-Try-next-entry");
// //				printf("%s\n", "<string key> Alert messages are saved in Alert_Strings[]. <string key> is a index of it.");
// 				printf("%s\n", "<lan alert#> Lan alert number is set by ipmitool lan alert set ..");
// //				printf("%s\n", "<ipaddr>     The ip address where you want to send a message. Destination server should open snmptrapd.");
// 				return -1;
// 			}
//         }
//     }
//     else if (strcmp(argv[1], "alert") == 0) {
//         puts("alert");
//         if (argc < 3) {
//             printf("%s\n", "Usage: req alert <policy number>");
//             ret = -1;
//         }
//         else {
//             send_test_alert(atoi(argv[2]));
//         }
    
// 	}

//     else if (strcmp(argv[1], "user") == 0) {
//         if (strcmp(argv[2], "list") == 0){
//             get_user_list();
//         }
//         else if (strcmp(argv[2], "set") == 0) {
//             if (argc < 9) {
//                 printf("%s\n", "[ERROR] inputs are not enough");
//                 ret = -1;
//         }
//             if (strcmp(argv[3], "enable") == 0) {
// 	              set_user_enable(argv[4], argv[5]);
// 	        }
// 	        else {
// 	        // req user set <id> <name> <pwd> <16|20> <enable> <callin> <linkauth> <ipmimsg> <privilege>
// 	       //  0   1    2   3     4      5      6        7        8        9          10         11
// 	            set_user_name(argv[3], argv[4]);
// 				set_user_enable(argv[3], argv[7]);
// 	            set_user_password(argv[3], argv[5], argv[6]);
// 	            set_user_access(argv[3],  argv[7], argv[8], argv[9], argv[10], argv[11]);
// 	        }
// 	    }
// 	    else if (strcmp(argv[2], "del") == 0) {
// 	        if (argc < 4) {
// 	             printf("%s\n", "[ERROR] inputs are not enouth");
// 	             exit(-1);
// 	        }
// 	        delete_user(argv[3]);
// 	    }
//     }

// 	else if (strcmp(argv[1], "main") == 0) {
// 		ret = show_main(atoi(argv[2]));
// 	}
// 	else if (strcmp(argv[1], "smtp") == 0) {
// 		if (strcmp(argv[2], "get") == 0) {
// 			get_smtp_configuration();
// 		}
// 		else if (strcmp(argv[2], "set") == 0) {
// 			// req smtp set <sender> <machine name> <server1> <id1> <pwd1> <server2> <id2> <pwd2>
// 			// 0    1    2     3          4            5       6      7       8       9     10
// 			set_smtp_sender(argv[3], argv[4]);
// 			set_smtp_primary(argv[5], argv[6], argv[7]);
// 			set_smtp_secondary(argv[8], argv[9], argv[10]);
// 		}
// 	}
// 	else if (strcmp(argv[1], "power") == 0) {
// 		if (strcmp(argv[2], "get") == 0) {
// 			get_power_status();
// 		}
// 		else if (strcmp(argv[2], "set") == 0) {
// 			set_power_status(atoi(argv[3]));
// 		}
// 	}
//     else if (strcmp(argv[1], "ldap") == 0) {
//         if (strcmp(argv[2], "get") == 0) {
//             ret = get_ldap_info();
//         }
//         else if (strcmp(argv[2], "set") == 0) {
// 			if (argc == 4) {

// 				ret = set_ldap_enable(argv[3]);
// 			}
// 			else if (argc < 10) {
// 				puts("Usage: req ldap set <enable> <IP> <PORT> <BASEDN> <BINDDN> <BINDPW> <SSL> <TIMELIMIT>");
// 				puts("enable: 0 or 1");
// 				puts("IP: ip address of the server where the ldap server installed");
// 				puts("PORT: ldap port. Set this 389 if you don't know well.");
// 				puts("BASEDN: the base directory which you would find accounts. ex) dc=keti-ldap,dc=com");
// 				puts("BINDDN: the admin account ex)cn=Administrator,dc=keti-ldap,dc=com");
// 				puts("BINDPW: the admin account password");
// 				puts("SSL: Not Supported for now");
// 				puts("TIMELIMIT: the time limit (number)");
// 				return 0;
// 			}
// 			else {
// 				ret = set_ldap_enable(argv[3]);
//             	ret = set_ldap_ip(argv[4]);
//             	ret = set_ldap_port(argv[5]);
//             	ret = set_ldap_searchbase(argv[6]);
//             	ret = set_ldap_binddn(argv[7]);
//             	ret = set_ldap_password(argv[8]);
// 				ret = set_ldap_ssl(atoi(argv[9]));
//             	ret = set_ldap_timelimit(atoi(argv[10]));
// 			}
//         }
//     }
// 	else if (strcmp(argv[1], "radius") == 0) {
// 		if (strcmp(argv[2], "get") == 0) {
// 			get_radius_info();
// 		}
// 		else if (strcmp(argv[2], "set") == 0) {
// 		    if (argc < 5)
// 			set_radius_disable();
// 		    else
// 			set_radius_info(argv[3], argv[4], argv[5]); // 3-ip, 4-port, 6-secret(pwd)
// 		}
// 	}
// 	else if (strcmp(argv[1], "ntp") == 0) {
// 		if (strcmp(argv[2], "get") == 0) {
// 			get_ntp_info();
// 		}
// 		else if (strcmp(argv[2], "set") == 0) {
// 			if (strcmp(argv[3], "auto") == 0) 
// 				set_ntp_auto(argv[4]);
// 		}
// 	}
// 	else if (strcmp(argv[1], "ad") == 0) {
// 		if (strcmp(argv[2], "get") == 0) {
// 			get_active_directory_info();
// 		}
// 		else if (strcmp(argv[2], "set") == 0) {
// 			//set_active_directory_enable(intf, atoi(argv[3]));
// 			if (argc == 4) {
// 				ret = set_active_directory_enable(argv[3]);
// 			}
// 			else if (argc > 4) {
// 				ret = set_active_directory_enable(argv[3]);
// 				ret = set_active_directory_ip_pwd(argv[4], argv[7]);
// 				if (ret == 0)
// 					ret = set_active_directory_domain(argv[5]);
// 					if (ret == 0)
// 						set_active_directory_username(argv[6]);
// 			}
// 			else {
// 				puts("Usage: req ad set <enable> <ip> <domain> <secret_username> <secret_password>");
// 				puts("enable: 1 or 0");
// 				puts("ip: domain controller IP address including port (default: 389)");
// 				puts("domain: server domain name (ex. dc=server,dc=local)");
// 				puts("secret_username: the administrator name (Ex. cn=Adminitrator,dc=Users,dc=server,dc=local)");
// 				puts("secret_password: the administrator password including CAPITAL word, small word and number (Ex. KETIlinux123)");
// 				return -1;
// 			}
// 			// req ad set dc_ipaddr(3) domain(4) secret_username(5) secret_password(6)
// 			// the sequence of functions is important!!!
// 		}
// 	}
// 	else if (strcmp(argv[1], "dns") == 0) {
// 		if (strcmp(argv[2], "get") == 0) {
// 			get_dns_info();
// 		}
// 		else if (strcmp(argv[2], "set") == 0) {
//             if(strcmp(argv[3], "domain") == 0)
// 			    set_dns_domain_info(argv[4]);
//             else if(strcmp(argv[3], "hostname") == 0)
//                 set_dns_hostname_info(argv[4]);
//             else if(strcmp(argv[3], "ip") == 0){
//                 if(strcmp(argv[4], "prefer") == 0){
//                     printf("prefer IP : %s\n", argv[5]);
//                     set_dns_ip_prefer(argv[5]);
                  
//                 }
//                 else if(strcmp(argv[4], "alter") == 0){
//                     set_dns_ip_alter(argv[5]);
//                 }
//             }
//             else if(strcmp(argv[3], "ipv6") == 0){
//                 if(strcmp(argv[4], "prefer") == 0){
//                     set_dns_ipv6_prefer(argv[5]);
//                 }
//             }
// 		}
// 	}
// 	else if (strcmp(argv[1], "ssl") == 0) {
// 		if (strcmp(argv[2], "get") == 0) {
// 			get_ssl_info();
// 		}
// 		else if (strcmp(argv[2], "set") == 0) {
// 			if (argc == 12) {
// 				set_ssl_info_1(argv[3], argv[4], argv[5], argv[6], argv[7], argv[11]);
// 				set_ssl_info_2(argv[8], argv[9], argv[10]);
// 			}
// 			else {
// 				puts("Usage: req ssl set <key length> <country> <state> <city> <organization> <organization unit> <common name> <email> <valid days>");
// 				puts("key length: 2048");
// 				puts("country: two capital words. (ex-KR, UK...)");
// 			}
			
// 			//req ssl set <keylen> <country> <state> <city> <organ> <organ_unit> <cn> <email> <valid_days>
// 			//               3        4        5        6       7      8          9     10      11

// 		}
// 	}
// 	else if (strcmp(argv[1], "login") == 0) {
// 		ret = try_login(argv[2], argv[3]);
// 	}
// 	else if (strcmp(argv[1], "kvm") == 0) {
// 		ret = request_kvm_close();
// 	}
	
// 	else if (strcmp(argv[1], "setting") == 0) {
// 		if (strcmp(argv[2], "get") == 0)
// 			ret = request_get_setting_service();
// 		else if (strcmp(argv[2], "set") == 0) {
// 			ret = request_set_setting_service(argv[3], argv[4]);
// 			// ./req set set 1 
//             (web port)
// 		}
// 	}
//     else if(strcmp(argv[1], "watt") == 0){
//         ret = get_total_power_usage(atoi(argv[2]));
//     }
// 	return ret;
// }

// int main(int argc, char * * argv) {
	
//     ipmi_lanp_main(argc, argv);
//     return 0;
// }
// #endif
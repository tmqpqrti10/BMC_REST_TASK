#ifndef REQ_H_
#define REQ_H_

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
#include <inttypes.h>

int set_active_directory_enable(char *enable);
int set_sensor_threshold(float ucr, float unc, float unr, float lcr, float lnc, float lnr, int snr);
int request_set_setting_service(int flag, char* str);
/*------------------bmc_intf--------------------------*/

#define	SEND_MAX_PAYLOAD_SIZE		34	/* MAX payload */
#define	RECV_MAX_PAYLOAD_SIZE		33	/* MAX payload */
#define	IOCTL_IPMI_KCS_ACTION		0x01



/*-------------------bmc_intf.h----------------------*/

#define IPMI_CHASSIS_NETFN      0x00
#define IPMI_BRIDGE_NETFN       0x02
#define IPMI_SENSOR_EVENT_NETFN     0x04
#define IPMI_APP_NETFN          0x06
#define IPMI_FIRMWARE_NETFN     0x08
#define IPMI_STORAGE_NETFN      0x0a
#define IPMI_TRANSPORT_NETFN        0x0c
#define IPMI_GROUP_EXTENSION_NETFN  0x2c
#define IPMI_OEM_GROUP_NETFN        0x2e

#define IPMI_CHASSIS_CTL_POWER_DOWN     0x0
#define IPMI_CHASSIS_CTL_POWER_UP       0x1
#define IPMI_CHASSIS_CTL_POWER_CYCLE    0x2
#define IPMI_CHASSIS_CTL_HARD_RESET     0x3
#define IPMI_CHASSIS_CTL_PULSE_DIAG     0x4
#define IPMI_CHASSIS_CTL_ACPI_SOFT      0x5

#define IPMI_CHASSIS_POLICY_NO_CHANGE   0x3
#define IPMI_CHASSIS_POLICY_ALWAYS_ON   0x2
#define IPMI_CHASSIS_POLICY_PREVIOUS    0x1
#define IPMI_CHASSIS_POLICY_ALWAYS_OFF  0x0

#define IPMI_CHASSIS_POWER_STATUS       0x01
#define IPMI_CHASSIS_POWER_CTL          0x02

#define IPMI_BUF_SIZE 1024

//USER COMMAND

#define IPMI_SET_USER_ACCESS           0x43
#define IPMI_GET_USER_ACCESS           0x44
#define IPMI_SET_USER_NAME             0x45
#define IPMI_GET_USER_NAME             0x46
#define IPMI_SET_USER_PASSWORD         0x47
// LAN COMMAND

#define IPMI_LAN_SET_CONFIG	0x01
#define IPMI_LAN_GET_CONFIG	0x02
#define IPMI_LAN_SUSPEND_ARP	0x03
# define IPMI_LAN_SUSPEND_ARP_RESP (2)
# define IPMI_LAN_SUSPEND_ARP_GRAT (1)
#define IPMI_LAN_GET_STAT	0x04

#define IPMI_CHANNEL_NUMBER_MAX	0xe

#define IPMI_LANP_TIMEOUT		3
#define IPMI_LANP_RETRIES		10
#define IPMI_LANP_WRITE_UNLOCK		0
#define IPMI_LANP_WRITE_LOCK		1
#define IPMI_LANP_WRITE_COMMIT		2
#define IPMI_SESSION_AUTHTYPE_NONE      0x0
#define IPMI_SESSION_AUTHTYPE_MD2       0x1
#define IPMI_SESSION_AUTHTYPE_MD5   	0x2
#define IPMI_SESSION_AUTHTYPE_KEY	0x4
#define IPMI_SESSION_AUTHTYPE_PASSWORD	IPMI_SESSION_AUTHTYPE_KEY
#define IPMI_SESSION_AUTHTYPE_OEM       0x5
#define IPMI_SESSION_AUTHTYPE_RMCP_PLUS 0x6

//ETC
#define IPMI_SESSION_PRIV_UNSPECIFIED   0x0
#define IPMI_SESSION_PRIV_CALLBACK  0x1
#define IPMI_SESSION_PRIV_USER      0x2
#define IPMI_SESSION_PRIV_OPERATOR  0x3
#define IPMI_SESSION_PRIV_ADMIN     0x4
#define IPMI_SESSION_PRIV_OEM       0x5
#define SEND_MAX_PAYLOAD_SIZE       34  /* MAX payload */
#define RECV_MAX_PAYLOAD_SIZE       33  /* MAX payload */

#define IPMI_GET_SDR_REPOSITORY_INFO            0x20
#define IPMI_SOL_ACTIVATING                     0x20
#define IPMI_SET_SOL_CONFIG_PARAMETERS          0x21
#define IPMI_GET_SOL_CONFIG_PARAMETERS          0x22
#define IPMI_SET_USER_ACCESS                    0x43
#define IPMI_GET_USER_ACCESS                    0x44
#define IPMI_SET_USER_NAME                      0x45
#define IPMI_GET_USER_NAME                      0x46
#define IPMI_SET_USER_PASSWORD                  0x47
#define IPMI_ACTIVATE_PAYLOAD                   0x48
#define IPMI_DEACTIVATE_PAYLOAD                 0x49
#define IPMI_SUSPEND_RESUME_PAYLOAD_ENCRYPTYION 0x55
#define IPMI_GET_SEL_TIME                       0x48
#define IPMI_SET_SEL_TIME                       0x49
#define IPMI_SET_USER_PAYLOAD_ACCESS        0x4c
#define IPMI_GET_USER_PAYLOAD_ACCESS        0x4d

#define IPMI_DEFAULT_PAYLOAD_SIZE   25




#define FRU_MULTIREC_CHUNK_SIZE     (255 + sizeof(struct fru_multirec_header))
#define dprintf(fmt, args...) fprintf(stderr, "\x1b[33m""[%s:%d:%s()]: " "\x1b[0m" fmt, \
		__FILE__, __LINE__, __func__, ##args)

int str2long(const char * str, int64_t * lng_ptr);
int str2int(const char * str, int32_t * int_ptr);

//static void dump_response(ipmi_res_t *response);

const char * val2str(uint16_t val, const struct valstr *vs);
uint16_t ipmi_intf_get_max_response_data_size(struct ipmi_intf * intf);
const char * oemval2str(uint32_t oem, uint16_t val, const struct oemvalstr *vs);
//void printbuf(const uint8_t * buf, int len, const char * desc);
int str2uchar(const char * str, uint8_t * uchr_ptr);
void print_valstr_2col(const struct valstr * vs, const char * title, int loglevel);
uint16_t buf2short(uint8_t * buf);
uint32_t buf2long(uint8_t * buf);
int str2ulong(const char * str, uint64_t * ulng_ptr);
const char * buf2str(uint8_t * buf, int len);
int try_login(char* username, char* pwd, unsigned char *input, int *res_len) ;
#endif /* REQ_H_ */
int set_user_name(char* index, char* name);
int get_user_list(char *input, int *res_len) ;
int set_user_enable(char* index, char* enable);
int set_user_password(char* index, char* password, char* type);
int delete_user(char index);
int set_user_access(char* index, char* enable, char* callin, char* linkauth, char* ipmimsg, char* priv);
int get_cmdline_ipaddr(char * arg, uint8_t * buf) ;
int request_get_setting_service(char *input, int *res_len);
int request_set_setting_service(int flag, char* str);
int show_main(uint8_t index, unsigned char *input, int *res_len);

/**
 * @author doyoung
 * @brief req.cpp 구현 목록
 * */
int get_power_status(unsigned char *input, int *res_len);
int set_power_status(int status, char *res_msg, int *res_len);
int request_kvm_close();

int get_fru_param_select(int select, char * input, int *res_len);
int set_fru_header(char fru_id, char en_board, char en_product, char en_chassis);
int set_fru_board(char fru_id, char* year, char* month, char* day, char* hour, char* minute, char* sec, char* mfg, char* product, char* serial, char* part_num);
int set_fru_product(char fru_id, char* name, char* mfg, char* version, char* serial, char* part_num);
int set_fru_chassis(char fru_id, char type, char* serial, char* part_num);

void get_sys_param_select(int select, unsigned char *input, int *res_len);

// server health
int get_sensor_param_select(int select, char *input, int *res_len);
int set_sensor_threshold(float ucr, float unc, float unr, float lcr, float lnc, float lnr, int snr);
int get_event_param_select(int select, char *input, int *res_len);
int get_total_power_usage(uint8_t index, char *input, int *res_len);

// configuration
int get_dns_info(char *input, int *res_len);
int set_dns_hostname_info(unsigned char *host_name);
int set_dns_domain_info(unsigned char *domain_name);
int set_dns_ip_prefer(unsigned char *ipv4);
int set_dns_ip_alter(unsigned char *ipv4);
int set_dns_ipv6_prefer(unsigned char *ipv6);

int get_lan_param_select(int select, char *input, int *res_len);
int set_network_mac(char dev, char* mac);
int get_cmdline_macaddr(char *arg, uint8_t *buf);
int set_network_ipv4_dhcp(char dev, char dhcp_or_static);
int set_network_ipv4_ip(char dev, char* ip, char* netmask);
int set_network_gateway(char dev, char* gate);
int set_network_ipv6_enable(char dev, char en_disable);
int set_network_ipv6_dhcp(char dev, char dhcp_or_static);
int set_network_ipv6_ip(char dev, char *ip);
int set_network_ipv6_prefix(char dev, char* prefix);
int set_network_ipv6_gateway(char dev, char *gw);
int set_network_ipv6_priority(char dev);
int set_network_vlan_enable(char dev, char enable);
int set_network_vlan_id(char dev, char* id);
int set_network_vlan_priority(char dev, char* priority);

int get_ntp_info(char *input, int *res_len);
int set_ntp_auto(char* server, char *input, int *res_len);

int get_smtp_configuration(char *input, int *res_len);
int set_smtp_sender(char* sender, char* machine);
int set_smtp_primary(char* server, char* id, char* pwd);
int set_smtp_secondary(char* server, char* id, char* pwd);

int get_ssl_info(char *input, int *res_len);
int set_ssl_info_1(char* keylen, char* country, char* state, char* city, char* organ, char* valid);
int set_ssl_info_2(char* organ_unit, char* cn, char* email);

int get_active_directory_info(char *input, int *res_len);
int set_active_directory_enable(char *enable);
int set_active_directory_ip_pwd(char* ip, char* pwd);
int set_active_directory_domain(char* domain);
int set_active_directory_username(char* s_username);

int set_ldap_enable(char *enable);
int get_ldap_info(char *input, int *res_len);
int set_ldap_ip(char* ipaddr);
int set_ldap_port(char* port);
int set_ldap_searchbase(char* searchbase);
int set_ldap_binddn(char* binddn);
int set_ldap_password(char* password);
int set_ldap_ssl(char* ssl);
int set_ldap_timelimit(char *timelimit);

int get_radius_info(char *input, int *res_len);
int set_radius_disable();
int set_radius_info(char* ip, char* port, char* secret);

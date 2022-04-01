/**
 * @file RHandler.hpp
 * @brief KTNF-WEB과 연동하기 위한 RESTSEVER 8080 포트사용
 * @todo 구현중
 */

#pragma once
#define NTP_PATH "/etc/ntp.conf"
#define POLICY_FILE "/conf/ipmi/policy.bin"
#define ALERT_JSON "/conf/ipmi/alert.json"
#define EVENT_FILTER_TABLE_FILE "/conf/ipmi/eft.bin"
#define SET_FILE "/conf/service_setting.bin"
#define DCMI_MANDATORY_CAPA_PATH "/conf/dcmi/dcmi_manda.bin"
#define DCMI_OPTION_CAPA_PATH "/conf/dcmi/dcmi_option.bin"
#define DCMI_MGNT_CAPA_PATH "/conf/dcmi/dcmi_mgnt.bin"
#define DCMI_CAPA_PATH "/conf/dcmi/dcmi_capa.bin"
#define DCMI_POWER_LIMIT_PATH "/conf/dcmi/dcmi_power_lmt.bin"
#define DCMI_ASSET_MNGCTRL_PATH "/conf/dcmi/dcmi_asset_mngctrl.bin"
#define DCMI_CONF_PARAM_PATH "/conf/dcmi/dcmi_conf.bin"
#define IPMI_FRU_PATH "/conf/ipmi/fru.bin"
#define IPMI_FRU_JSON_PATH "/conf/ipmi/fru.json"
#define IPMI_LAN_ALERT_PATH "/conf/ipmi/lan_alert.bin"
#define IPMI_LAN_ALERT_DEDI_PATH "/conf/ipmi/lan_alert_dedi.bin"
#define IPMI_USER_PATH "/conf/ipmi/user.bin"
#define IPMI_SYSINFO_PATH "/conf/ipmi/sysinfo.bin"
#define OEM_FAN_PATH "/conf/ipmi/auto_fan.bin"
#define NETWORK_PRIORITY "/conf/ipmi/eth_priority"
#define IPMI_SENSOR_THRESH_PATH "/conf/ipmi/sensor_thresh.bin"
#define SOL_CONFIG_PATH "/conf/ipmi/sol_config.bin"
#define IPMI_GLOBAL_EN_PATH "/conf/ipmi/global_enable.bin"
#define AUTO_FAN_PATH "/conf/ipmi/auto_fan.bin"
#define NETWORK_MAC_ADDR_SHARED "/conf/ipmi/network_mac.bin"
#define NETWORK_MAC_ADDR_DEDI "/conf/ipmi/network_mac_dedi.bin"
#define IPMI_SEL_PATH "/conf/ipmi/sel.bin"
#define SMTPCONF "/conf/.msmtprc"
#define SMTP_BIN "/conf/ipmi/stmp.bin"
#define ALERT_SMTP_PATH "/conf/content.txt"
#define RAD_BIN "/conf/ipmi/rad.bin"
#define KVM_PORT_BIN "/conf/ipmi/kvm.bin"
#define ALERT_PORT_BIN "/conf/ipmi/alert_port.bin"
#define SSH_SERVICE_BIN "/conf/ipmi/ssh_port.bin"
#define WEB_PORT_BIN "/conf/ipmi/web_port.bin"
#define LDAP_BIN "/conf/ipmi/ldap.bin"
#define AD_BIN "/conf/ipmi/ad.bin"
#define AUTH_PATH "/etc/raddb/"
#include <mongoose.h>
#include <iostream>
#include <vector>
#include <string>
#include <sys/syscall.h>
#include <linux/reboot.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>
#include <string.h>
#include "helper.hpp"
#include "req.hpp"
using namespace std;

/**
 * @namespace json사용하기 위한 web
 */
using namespace web;
using namespace utility;
#define MG_PORT 8000

static void handle_main_call(string uri, vector<string> uri_tokens, void *ev_data);
static void handle_login_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_power_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_sysinfo_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_fru_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_sensor_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_eventlog_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_network_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_ntp_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_smtp_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_ssl_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_ldap_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_radius_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_user_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_sol_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_kvm_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_setting_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_usb_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_upload_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_watt_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_warm_reset_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_bmc_reset_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_ddns_call(struct mg_connection *nc, int ev, void *ev_data);
static void handle_ad_call(struct mg_connection *nc, int ev, void *ev_data);
static void ev_handler(struct mg_connection *nc, int ev, void *ev_data);
void restful_init(void);

static uint8_t chassis_type_desc[30][50] = {"Unspecified", "Other", "Unknown", "Desktop", "Low Profile Desktop", "Pizza Box", "Mini Tower", "Tower", \
"Portable", "LapTop", "Notebook", "Hand Held", "Docking Station", "All in One", "Sub Notebook", "Space-saving", "Lunch Box", \
"Main Server Chassis", "Expansion Chassis", "SubChassis", "Bus Expansion Chassis", "Peripheral Chassis", "RAID Chassis", \
"Rack Mount Chassis", "Sealed-case PC", "Multi-system Chassis", "CompactPCI", "AdvancedTCA", "Blade", "Blade Enclosure"};
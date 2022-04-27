#pragma once
#include "ipmi/sel.hpp"
#include <vector>

#define MAX_IPMI_MSG_SIZE 100
// #define BOOST_LOG_DYN_LINK
void *kcs_handler(void *data);
void *dcmi_power_handler(void *data);
void *lanplus_handler(void *data);

void *redfish_handler(void *data);
void *restserver_handler(void *data);
void *ipmievent_handler(void *data);
// ssdp
void *ssdp_handler(void);
void *timer_handler(void);
// void *ssdp_handler(void *data);
void *ipmb_handler(void *bus_num);

void sel_generater(void);
void wdt_msq(void);

namespace SOL {
void *sol_handler(void *data);
}
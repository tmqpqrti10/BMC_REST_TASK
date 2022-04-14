#include <iostream>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
// #include "ipmi/apps.h"

/* Configuration file key works*/
#define IPMI_DAEMON "Daemon"
#define IPMI_TIMEOUT "Timeout"
#define IPMI_PRETIMEOUT "Pretimeout"
#define IPMI_INTERVAL "Interval"
#define IPMI_PRETIMEOUTINTERRUPT "INT_Pretimeout"
#define IPMI_ACTION "Action"
#define IPMI_PIDFILE "Pidfile"

#define IPMIWatchdogFile1 "/etc/init.d/ipmiwatchdog"
#define IPMIWatchdogFile2 "/etc/ipmiwatchdog.conf"
#define ConfigurationFileDir "/etc/ipmiwatchdog.conf"

#define CONFIG_LINE_LEN 100

typedef struct {
  unsigned char timer_use;
  unsigned char timer_actions;
  unsigned char pre_timeout;
  unsigned char timer_use_exp;
  unsigned char initial_countdown_lsb;
  unsigned char initial_countdown_msb;
  unsigned char present_countdown_lsb;
  unsigned char present_countdown_msb;
  unsigned char pretimeoutInterrupt;
  bool Islogging = false;

} bmc_watchdog_param_t;

static int ReadConfigurationFile(char *file);
static int spool(char *line, int *i, int offset);
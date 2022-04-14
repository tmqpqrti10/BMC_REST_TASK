#include "watchdog.hpp"

static bmc_watchdog_param_t g_watchdog_config;
int IPMI_Interval = 10, IPMI_Timeout = 600, IPMI_Pretimeout = 10,
    IPMI_PretimeoutInterrupt = 0, IPMI_Action = 1;
char *IPMI_Daemon = NULL, *IPMI_Pidfile = NULL, *IPMI_debug = NULL;

#define PID_FILE_PATH "/conf/ipmi/WDT.pid"
int main(int argc, char *argv[]){

    printf("configfile read\n");
    ReadConfigurationFile(ConfigurationFileDir);
    // unsigned int *pid = getpid();
    // printf("pid : %d\n",pid);
    // FILE *fp;
    // fp = fopen(PID_FILE_PATH, "w+");
    // printf("file\n");
    // if(fwrite(&pid, sizeof(unsigned int), 1, fp) <= 0){
	// 	printf("set_eft_entry_num: fwrite");
	// 	fclose(fp);
	// 	return -1;
	// }

}
static int ReadConfigurationFile(char *file) {
  FILE *ReadConfigurationFile;

  /* Open the configuration file with readonly parameter*/
  printf("Trying the configuration file %s \n", ConfigurationFileDir);
  if ((ReadConfigurationFile = fopen(ConfigurationFileDir, "r")) == NULL) {
    printf("There is no configuration file, use default values for IPMI "
           "watchdog \n");
    return (1);
  }

  /* Check to see the configuration has data or not*/
  while (!feof(ReadConfigurationFile)) {
    char Configurationline[CONFIG_LINE_LEN];

    /* Read the line from configuration file */
    if (fgets(Configurationline, CONFIG_LINE_LEN, ReadConfigurationFile) ==
        NULL) {
      if (!ferror(ReadConfigurationFile)) {
        break;
      } else {
        return (1);
      }
    } else {
      int i, j;

      /* scan the actual line for an option , first remove the leading blanks*/
      for (i = 0; Configurationline[i] == ' ' || Configurationline[i] == '\t';
           i++)
        ;

      /* if the next sign is a '#' we have a comment , so we ignore the
       * configuration line */
      if (Configurationline[i] == '#') {
        continue;
      }

      /* also remove the trailing blanks and the \n */
      for (j = strlen(Configurationline) - 1;
           Configurationline[j] == ' ' || Configurationline[j] == '\t' ||
           Configurationline[j] == '\n';
           j--)
        ;

      Configurationline[j + 1] = '\0';

      /* if the line is empty now, we don't have to parse it */
      if (strlen(Configurationline + i) == 0) {
        continue;
      }

      /* now check for an option , interval first */

      /*Interval */
      if (strncmp(Configurationline + i, IPMI_INTERVAL,
                  strlen(IPMI_INTERVAL)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_INTERVAL))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          IPMI_Interval = atol(Configurationline + i);

          { printf(" IPMI_Interval = %d \n", IPMI_Interval); }
        }
      }

      /*Timeout */
      else if (strncmp(Configurationline + i, IPMI_TIMEOUT,
                       strlen(IPMI_TIMEOUT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_TIMEOUT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          IPMI_Timeout = atol(Configurationline + i);
          g_watchdog_config.initial_countdown_lsb = IPMI_Timeout & 0xFF;
          g_watchdog_config.initial_countdown_msb = IPMI_Timeout >> 8;
          printf(" IPMI_Timeout = %d \n", IPMI_Timeout);
          printf(" initial_countdown_lsb = %d \n",
                 g_watchdog_config.initial_countdown_msb);
          printf(" initial_countdown_msb = %d \n",
                 g_watchdog_config.initial_countdown_msb);
        }
      }

      /*Pretimeout */
      else if (strncmp(Configurationline + i, IPMI_PRETIMEOUT,
                       strlen(IPMI_PRETIMEOUT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PRETIMEOUT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          IPMI_Pretimeout = atol(Configurationline + i);

          g_watchdog_config.pre_timeout = IPMI_Pretimeout;
          printf(" IPMI_Pretimeout = %d \n", IPMI_Pretimeout);
        }
      }

      /*Daemon */
      else if (strncmp(Configurationline + i, IPMI_DAEMON,
                       strlen(IPMI_DAEMON)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_DAEMON))) {
          IPMI_Daemon = NULL;
        } else {
          IPMI_Daemon = strdup(Configurationline + i);

          printf(" IPMI_Daemon = %s \n", IPMI_Daemon);
        }
      }

      /*PretimeoutInterrupt */
      else if (strncmp(Configurationline + i, IPMI_PRETIMEOUTINTERRUPT,
                       strlen(IPMI_PRETIMEOUTINTERRUPT)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PRETIMEOUTINTERRUPT))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          g_watchdog_config.pretimeoutInterrupt = atol(Configurationline + i);
          printf(" IPMI_PretimeoutInterrupt = %d \n",
                 g_watchdog_config.pretimeoutInterrupt);
        }
      }

      /*Action */
      else if (strncmp(Configurationline + i, IPMI_ACTION,
                       strlen(IPMI_ACTION)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_ACTION))) {
          fprintf(stderr, "Ignoring invalid line in config file:\n%s\n",
                  Configurationline);
        } else {
          IPMI_Action = atol(Configurationline + i);
          g_watchdog_config.timer_actions = IPMI_Action;
          printf(" IPMI_Action = %d \n", IPMI_Action);
        }
      }

      /*Pidfile */
      else if (strncmp(Configurationline + i, IPMI_PIDFILE,
                       strlen(IPMI_PIDFILE)) == 0) {
        if (spool(Configurationline, &i, strlen(IPMI_PIDFILE))) {
          IPMI_Pidfile = NULL;
        } else {
          IPMI_Pidfile = strdup(Configurationline + i);

          printf(" IPMI_Pidfile = %s \n", IPMI_Pidfile);
        }
      }

      else {
        fprintf(stderr, "Ignoring config Configurationline: %s\n",
                Configurationline);
      }
    }
  }

  /* Close the configuration file */
  if (fclose(ReadConfigurationFile) != 0) {
    return (1);
  }
}

static int spool(char *line, int *i, int offset) {
  for ((*i) += offset; line[*i] == ' ' || line[*i] == '\t'; (*i)++)
    ;
  if (line[*i] == '=')
    (*i)++;
  for (; line[*i] == ' ' || line[*i] == '\t'; (*i)++)
    ;
  if (line[*i] == '\0')
    return (1);
  else
    return (0);
}

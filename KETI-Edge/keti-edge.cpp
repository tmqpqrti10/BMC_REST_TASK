#pragma once
#include "khandler.hpp"
#include "psu_server.hpp"
#include "rest_handler.hpp"
#include <csignal>
#include <iostream>
#include <ipmi/apps.hpp>
#include <ipmi/common.hpp>
#include <ipmi/fru.hpp>
#include <ipmi/ipmi.hpp>
#include <ipmi/sdr.hpp>
#include <ipmi/setting_service.hpp>
#include <mutex>
#include <redfish/resource.hpp>
<<<<<<< HEAD
#include <thread>
#include <uuid/uuid.h>
=======
#include "psu_server.hpp"
#include "ipmi/efte.hpp"
>>>>>>> f5c5c914c5de1a00e1c485122e192df1021e6826
// #include "handler.hpp"

// #include <boost/log/trivial.hpp>
// unique_ptr<Handler> g_listener;
// unordered_map<string, Resource *> g_record;
// src::severity_logger<severity_level> g_logger;
// ServiceRoot *g_service_root;

#define RMCP_UDP_PORT 623

void exit_cleanup(int signo) {
  std::cout << "Cleanup ... " << std::endl;
  mutex_destroy();
  exit(1);
}

<<<<<<< HEAD
int main(void) {
  extern char uuid_hex[16];
  extern char uuid_str[37];
  extern uuid_t uuid;
  // BOOST_LOG_SEV(g_logger, info);
  boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                      boost::log::trivial::info);
  uint8_t dev_node[100] = "";
  int sockfd = 0, enable = 1, kcs_fd, result;
  struct sockaddr_in server;
  pthread_t tid, tid_dcmi_power, tid_kcs;

#if AST2500
  sprintf(dev_node, "/dev/ast-kcs.%d", 2);
  kcs_fd = open(dev_node, O_RDWR | O_APPEND);
  if (kcs_fd < 0) {
    exit(1);
  }
#endif

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    std::cout << "Socket opening error" << std::endl;
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_port = htons(RMCP_UDP_PORT);

  mutex_init();

  signal(SIGINT, exit_cleanup);
  signal(SIGKILL, exit_cleanup);
  signal(SIGTERM, exit_cleanup);

  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    ;

  if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1) {
    std::cout << "Socket bind error" << std::endl;
    exit_cleanup(0);
    exit(0);
  }

  std::cout << "Starting UDP Server Port : " << RMCP_UDP_PORT << std::endl;

  uuid_generate_time_safe(uuid);
  uuid_unparse_upper(uuid, uuid_str);

  memcpy(uuid_hex, uuid, sizeof(uuid_t));
  plat_ipmi_init(); // user, dcmi, lan, buildtime, sdr, sysinfo init
  plat_fru_device_init();
  if (plat_sdr_init() == -1)
    fprintf(stderr, "sdr init failed\n");
  if (load_g_setting() == -1)
    fprintf(stderr, "load g setting failed\n");
  std::thread t_lanplus(lanplus_handler, &sockfd);
  std::thread t_dcmi(dcmi_power_handler, &sockfd);
  std::thread t_redfish(redfish_handler, &sockfd);
  std::thread t_rest(rest_handler, &sockfd);
  std::thread t_ssdp(ssdp_handler);
  std::thread t_psuserver(psu_handler);
  int bus = IPMB_I2C_BUS;
  std::thread t_ipmb(ipmb_handler, &bus);
  // std::thread t_ssdp(ssdp_handler, &sockfd);
  std::thread t_timer(timer_handler);

  std::thread t_sel_generate(sel_generater);

  while (1) {
    pause();
  }
  // exit_cleanup(0);
  t_redfish.join();
  t_lanplus.join();
  t_ipmb.join();
  t_dcmi.join();
  t_rest.join();
  t_ssdp.join();
  t_timer.join();
  t_psuserver.join();
  t_sel_generate.join();

  return 0;
=======
int main(void)
{
	extern char uuid_hex[16];
	extern char uuid_str[37];
	extern uuid_t uuid;
	//BOOST_LOG_SEV(g_logger, info);
	boost::log::core::get()->set_filter (
    boost::log::trivial::severity >= boost::log::trivial::info
    );
	uint8_t dev_node[100] = "";
	int sockfd = 0, enable = 1, kcs_fd, result;
	struct sockaddr_in server;
	pthread_t tid, tid_dcmi_power, tid_kcs;
	
	#if AST2500
		sprintf(dev_node, "/dev/ast-kcs.%d", 2);
		kcs_fd = open(dev_node, O_RDWR | O_APPEND);
		if(kcs_fd < 0)
		{
			exit(1);
		}
	#endif

	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		std::cout << "Socket opening error" << std::endl;
		exit(1);
	}
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	server.sin_port = htons(RMCP_UDP_PORT);	

	mutex_init();
	
	signal(SIGINT, exit_cleanup);
	signal(SIGKILL, exit_cleanup);
	signal(SIGTERM, exit_cleanup);

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0);

	if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		std::cout << "Socket bind error" << std::endl;
		exit_cleanup(0);
		exit(0);
	}

	std::cout << "Starting UDP Server Port : " << RMCP_UDP_PORT << std::endl;

	uuid_generate_time_safe(uuid);
	uuid_unparse_upper(uuid, uuid_str);

	memcpy(uuid_hex, uuid, sizeof(uuid_t));
	plat_ipmi_init(); // user, dcmi, lan, buildtime, sdr, sysinfo init
	plat_fru_device_init();
	if (plat_sdr_init() == -1)
		fprintf(stderr, "sdr init failed\n");
	if (load_g_setting() == -1)
		fprintf(stderr, "load g setting failed\n");
	set_eft_init();
	std::thread t_lanplus(lanplus_handler, &sockfd);
	std::thread t_dcmi(dcmi_power_handler, &sockfd);
	std::thread t_redfish(redfish_handler, &sockfd);
	std::thread t_rest(rest_handler, &sockfd);
	std::thread t_ssdp(ssdp_handler);
	std::thread t_psuserver(psu_handler);
	// std::thread t_ssdp(ssdp_handler, &sockfd);
	std::thread t_timer(timer_handler);

	// 아래의 monitor는 동작하지않고 각 lib 클래스의 모니터나 redfish와 연동해놓음 
	//구현되지 않거나 모니터링이 필요한 파트를 위해 템플릿만 구현됨
	// std::thread t_chassis_monitor(chassis_monitor,&sockfd);
	// std::thread t_systems_monitor(systems_monitor,&sockfd);
	// std::thread t_account_monitor(account_monitor,&sockfd);
	// std::thread t_manager_monitor(manager_monitor,&sockfd);
	// std::thread t_sessions_monitor(sessions_monitor,&sockfd);
	
	std::thread t_sel_generate(sel_generater);
    
	while(1)
	{
		pause();
	}
	//exit_cleanup(0);
	t_redfish.join();
	t_lanplus.join();
	t_dcmi.join();
	t_rest.join();
	t_ssdp.join();
	t_timer.join();
	t_psuserver.join();
	// t_chassis_monitor.join();
	// t_systems_monitor.join();
	// t_account_monitor.join();
	// t_manager_monitor.join();
	// t_sessions_monitor.join();

	t_sel_generate.join();
	
	return 0;
>>>>>>> f5c5c914c5de1a00e1c485122e192df1021e6826
}

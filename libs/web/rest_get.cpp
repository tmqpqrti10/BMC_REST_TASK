#include <ipmi/rest_get.hpp>

extern std::map<uint8_t, std::map<uint8_t, Ipmisdr>> sdr_rec;
extern std::map<uint8_t, std::map<uint8_t, Ipmifru>> fru_rec;

const char *ipmi_generic_sensor_type_val[] = {
    "reserved",
    "Temperature", "Voltage", "Current", "Fan",
    "Physical Security", "Platform Security", "Processor",
    "Power Supply", "Power Unit", "Cooling Device", "Other",
    "Memory", "Drive Slot / Bay", "POST Memory Resize",
    "System Firmwares", "Event Logging Disabled", "Watchdog1",
    "System Event", "Critical Interrupt", "Button",
    "Module / Board", "Microcontroller", "Add-in Card",
    "Chassis", "Chip Set", "Other FRU", "Cable / Interconnect",
    "Terminator", "System Boot Initiated", "Boot Error",
    "OS Boot", "OS Critical Stop", "Slot / Connector",
    "System ACPI Power State", "Watchdog2", "Platform Alert",
    "Entity Presence", "Monitor ASIC", "LAN",
    "Management Subsys Health", "Battery", "Session Audit",
    "Version Change", "FRU State",
    NULL};



void Ipmiweb_GET::Get_Show_Main(int menu, json::value &response_json){
    
  cout << "menu num " << menu << endl;
     char buf[30000] = {
      0,
  };
  cout << "Enter rest_show_main" << endl;
  cout << "\t selected menu = " << menu << endl;
  // json::value obj = json::value::object();
  json::value main = json::value::object();
  json::value JHEALTH_SUMMARY = json::value::object();
  json::value JCPU0 = json::value::object();
  json::value JCPU1 = json::value::object();
  json::value JPOWER = json::value::object();
  json::value JFANS = json::value::object();
  json::value JBOARD_TEMP = json::value::object();
  json::value JSYS_INFO = json::value::object();
  json::value JEVENT_LIST;
  std::vector<json::value> VJ;

  switch (menu) {
  case ALL:
    Ipmiweb_GET::Get_Eventlog(JEVENT_LIST);
    // rest_get_eventlog_config(buf);
    // JEVENT_LIST = json::value::parse(buf);
    cout << "EVENT LIST " <<JEVENT_LIST<<endl;
    get_temp_cpu0(JCPU0);
    cout << "get_temp_cpu0" << endl;
    get_temp_cpu1(JCPU1);
    cout << "get_temp_cpu1" << endl;
    get_voltage_fan_power(JPOWER);
    cout << "get_voltage_fan_power" << endl;
    get_fans(JFANS);
    cout << "get_fans" << endl;
    get_temp_board(JBOARD_TEMP);
    cout << "get_temp_board" << endl;
    get_sys_info(JSYS_INFO);
    cout << "get_sys_info" << endl;
    get_health_summary(JHEALTH_SUMMARY);
    cout << "get_health_summary" << endl;
    main["EVENT_LIST"] = JEVENT_LIST["EVENT_INFO"]["SEL"];
    main["CPU1_TEMP"] = JCPU0;
    main["CPU2_TEMP"] = JCPU1;
    main["POWER"] = JPOWER;
    main["FANS"] = JFANS;
    main["BOARD_TEMP"] = JBOARD_TEMP;
    main["SYS_INFO"] = JSYS_INFO;
    main["HEALTH_SUMMARY"] = JHEALTH_SUMMARY;
    response_json["MAIN"] = main;
    break;
  case EVENT_LIST:
    cout << "select 1 " <<endl;
    rest_get_eventlog_config(buf);
    JEVENT_LIST = json::value::parse(buf);
    main["EVENT_LIST"] = JEVENT_LIST["EVENT_INFO"]["SEL"];
    response_json["MAIN"] = main;
    break;
  case CPU2_TEMP:
    get_temp_cpu1(JCPU1);
    cout << " JCPU1 " <<JCPU1.to_string()<<endl;
    main["CPU2_TEMP"] = JCPU1;
    response_json["MAIN"] = main;
    break;

  case CPU1_TEMP:
    get_temp_cpu1(JCPU1);
    cout << " JCPU1 " <<JCPU1.to_string()<<endl;
    main["CPU2_TEMP"] = JCPU1;
    get_temp_cpu0(JCPU0);
    cout << " JCPU0 " <<JCPU0.to_string()<<endl;
    main["CPU1_TEMP"] = JCPU0;
    get_temp_board(JBOARD_TEMP);
    cout << " JBOARD_TEMP " <<JBOARD_TEMP.to_string()<<endl;
    main["BOARD_TEMP"] = JBOARD_TEMP;
    response_json["MAIN"] = main;
    break;
  case BOARD_TEMP:
    get_temp_cpu1(JCPU1);
    main["CPU2_TEMP"] = JCPU1;
    get_temp_cpu0(JCPU0);
    main["CPU1_TEMP"] = JCPU0;
    get_temp_board(JBOARD_TEMP);
    main["BOARD_TEMP"] = JBOARD_TEMP;
    response_json["MAIN"] = main;
    break;
  case POWER:
    get_voltage_fan_power(JPOWER);
    main["POWER"] = JPOWER;
    response_json["MAIN"] = main;
    break;
  case FANS:
    get_fans(JFANS);
    main["FANS"] = JFANS;
    response_json["MAIN"] = main;
    break;
  case SYS:
    get_sys_info(JSYS_INFO);
    main["SYS_INFO"] = JSYS_INFO;
    response_json["MAIN"] = main;
    break;
  case SUMMARY:
    get_health_summary(JHEALTH_SUMMARY);
    main["HEALTH_SUMMARY"] = JHEALTH_SUMMARY;
    response_json["MAIN"] = main;
    break;
  default:
    cout << "rest_show_main:: not option number" << menu << endl;
    break;
  }
  
}


void Ipmiweb_GET::Get_Fru_Info(json::value &response_json){
  	cout << "Enter rest_get_flu_config" << endl;

	char buf[1024];
	unsigned char i = 0;
	unsigned char temp_number = 0;
	struct tm *strtm;
	time_t tval;

	// json::value obj = json::value::object();
  std::vector<json::value> fru_vec;
  json::value fru = json::value::object();
  json::value BOARD = json::value::object();
  json::value PRODUCT = json::value::object();
  json::value CHASSIS = json::value::object();

	// 현재 fru 0만 존재. fru 개수 늘어날 시, size만큼 반복해야 함.
	Ipmifru *fru_this = &fru_rec.find(0)->second.find(0)->second;
	Ipmisdr *sdr_this = &sdr_rec.find(0)->second.find(0)->second;
	sensor_thresh_t *c_sdr;
	
	c_sdr = sdr_this->sdr_get_entry();
	
	fru["ID"] = json::value::number(i);

	string fru_device_desc = "FRU Device Description : ";
	if (fru_this->fru_header.id == 0 && i == 0)
		fru_device_desc += "Builtin FRU Device";
	else if (fru_this->fru_header.id == 0 && i != 0)
		fru_device_desc += "Not Configured";
	else{
		string sdr_str(c_sdr->str);
		fru_device_desc += sdr_str;	
	}	
	fru["DEVICE"] = json::value::string(U(fru_device_desc));

	if (fru_this->fru_header.board != 0){
		// convert char [4] -> time_t (ex. 5f ee 66 00 -> 1609459200)
		tval = (((unsigned char *)fru_this->fru_board.mfg_date)[0] << 24) | (((unsigned char *)fru_this->fru_board.mfg_date)[1] << 16) | (((unsigned char *)fru_this->fru_board.mfg_date)[2] << 8) | ((unsigned char *)fru_this->fru_board.mfg_date)[3]; 
		
		strtm = localtime(&tval);
		unsigned char Dates[100];
		strftime(Dates, 100, "%Y-%m-%d %H:%M:%S", strtm);
	
		Dates[strlen(asctime(strtm)) - 1] = '\0';
		BOARD["MFG_DATE"] = json::value::string(U((char *)Dates));
		BOARD["MFG"] = json::value::string(U((char *)fru_this->fru_board.mfg));
		BOARD["PRODUCT"] = json::value::string(U((char *)fru_this->fru_board.product));
		BOARD["SERIAL"] = json::value::string(U((char *)fru_this->fru_board.serial));
		BOARD["PART_NUM"] = json::value::string(U((char *)fru_this->fru_board.part_number));	
	} else {
		BOARD["MFG_DATE"] = json::value::string(U(""));
		BOARD["MFG"] = json::value::string(U(""));
		BOARD["PRODUCT"] = json::value::string(U(""));
		BOARD["SERIAL"] = json::value::string(U(""));
		BOARD["PART_NUM"] = json::value::string(U(""));		
	}
	
	if (fru_this->fru_header.product != 0){
		PRODUCT["NAME"] = json::value::string(U((char *)fru_this->product.name));
		PRODUCT["MFG"] = json::value::string(U((char *)fru_this->product.mfg));
		PRODUCT["VERSION"] = json::value::string(U((char *)fru_this->product.version));
		PRODUCT["SERIAL"] = json::value::string(U((char *)fru_this->product.serial));
		PRODUCT["PART_NUM"] = json::value::string(U((char *)fru_this->product.part_number));
	}else {
		PRODUCT["NAME"] = json::value::string(U(""));
		PRODUCT["MFG"] = json::value::string(U(""));
		PRODUCT["VERSION"] = json::value::string(U(""));
		PRODUCT["SERIAL"] = json::value::string(U(""));
		PRODUCT["PART_NUM"] = json::value::string(U(""));
	}

	if (fru_this->fru_header.chassis != 0){
		CHASSIS["TYPE"] = json::value::string(U(chassis_type_desc_fru[fru_this->fru_chassis.type]));
		CHASSIS["SERIAL"] = json::value::string(U((char *)fru_this->fru_chassis.serial));
		CHASSIS["PART_NUM"] = json::value::string(U((char *)fru_this->fru_chassis.part_number));	
	}else {
		CHASSIS["TYPE"] = json::value::string(U(""));
		CHASSIS["SERIAL"] = json::value::string(U(""));
		CHASSIS["PART_NUM"] = json::value::string(U(""));
	}

	fru["BOARD"] = BOARD;
	fru["PRODUCT"] = PRODUCT;
	fru["CHASSIS"] = CHASSIS;
	fru_vec.push_back(fru);
	response_json["FRU_JSON"] = json::value::array(fru_vec);

  
	
}

void Ipmiweb_GET::Get_Sensor_Info(json::value &response_json){
  // json::value obj = json::value::object();
  json::value sensor_info = json::value::object();
  vector<json::value> sensor_vec;
  sensor_thresh_t *p_sdr;
  float temp = 0;
  int state;

  for (auto iter = sdr_rec.begin(); iter != sdr_rec.end(); iter++) {
    json::value SENSOR = json::value::object();
    p_sdr = iter->second.find(iter->first)->second.sdr_get_entry();
    if (strlen(p_sdr->str) > 2) {
      SENSOR["NAME"] = json::value::string(U(p_sdr->str));
      switch (p_sdr->sensor_num) {
      case PEB_SENSOR_ADC_P12V_PSU1:
      case PEB_SENSOR_ADC_P12V_PSU2:
      case PEB_SENSOR_ADC_P3V3:
      case PEB_SENSOR_ADC_P5V:
      case PEB_SENSOR_ADC_PVNN_PCH:
      case PEB_SENSOR_ADC_P1V05:
      case PEB_SENSOR_ADC_P1V8:
      case PEB_SENSOR_ADC_BAT:
      case PEB_SENSOR_ADC_PVCCIN:
      case PEB_SENSOR_ADC_PVNN_PCH_CPU0:
      case PEB_SENSOR_ADC_P1V8_NACDELAY:
      case PEB_SENSOR_ADC_P1V2_VDDQ:
      case PEB_SENSOR_ADC_PVNN_NAC:
      case PEB_SENSOR_ADC_P1V05_NAC:
      case PEB_SENSOR_ADC_PVPP:
      case PEB_SENSOR_ADC_PVTT:
      // case NVA_SENSOR_PSU1_TEMP:
      // case NVA_SENSOR_PSU2_TEMP:
      // case NVA_SENSOR_PSU1_WATT:
      // case NVA_SENSOR_PSU2_WATT:
      case NVA_SYSTEM_FAN1:
      case NVA_SYSTEM_FAN2:
      case NVA_SYSTEM_FAN3:
      case NVA_SYSTEM_FAN4:
      case NVA_SYSTEM_FAN5:
      // case NVA_SENSOR_PSU1_FAN1:
      // case NVA_SENSOR_PSU2_FAN1:
      case PDPB_SENSOR_TEMP_CPU0:
      case PDPB_SENSOR_TEMP_CPU1:
      case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM0:
      case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM1:
      case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM2:
      case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM0:
      case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM1:
      case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM2:
      case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM0:
      case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM1:
      case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM2:
      case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM0:
      case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM1:
      case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM2:
      case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM0:
      case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM1:
      case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM2:
      case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM0:
      case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM1:
      case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM2:
      case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM0:
      case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM1:
      case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM2:
      case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM0:
      case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM1:
      case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2:
      case PDPB_SENSOR_TEMP_REAR_RIGHT:
      case PDPB_SENSOR_TEMP_CPU_AMBIENT:
      case PDPB_SENSOR_TEMP_FRONT_RIGHT:
      case PDPB_SENSOR_TEMP_PCIE_AMBIENT:
      case PDPB_SENSOR_TEMP_FRONT_LEFT:
        SENSOR["READING"] = json::value::string(
            to_string(sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->nominal)));
        SENSOR["RB"] = json::value::string(
            to_string(sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->rb_exp)));
        temp = (p_sdr->lnc_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->lnc_thresh)
                    : 0);
        SENSOR["LNC"] = json::value::string(to_string(temp));
        temp = (p_sdr->lc_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->lc_thresh)
                    : 0);
        SENSOR["LC"] = json::value::string(to_string(temp));
        temp = (p_sdr->lnr_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->lnr_thresh)
                    : 0);
        SENSOR["LNR"] = json::value::string(to_string(temp));
        temp = (p_sdr->unc_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->unc_thresh)
                    : 0);
        SENSOR["UNC"] = json::value::string(to_string(temp));
        temp = (p_sdr->uc_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->uc_thresh)
                    : 0);
        SENSOR["UC"] = json::value::string(to_string(temp));
        temp = (p_sdr->unr_thresh != THRESH_NOT_AVAILABLE
                    ? sdr_convert_raw_to_sensor_value(p_sdr, p_sdr->unr_thresh)
                    : 0);
        SENSOR["UNR"] = json::value::string(to_string(temp));

        // if (bPowerGD == 0)
        //     state = 0;
        // else
        if (p_sdr->sensor_type == 0x2 || p_sdr->sensor_type == 0x1) {
          if (p_sdr->nominal == 0)
            state = 0;
          else if (p_sdr->nominal < p_sdr->unc_thresh &&
                   p_sdr->nominal > p_sdr->lnc_thresh)
            state = 6;
          else
            state = 1;
        } else if (p_sdr->sensor_type == 0x4) {
          if (p_sdr->nominal == 0)
            state = 0;
          else if (p_sdr->nominal < p_sdr->lnc_thresh)
            state = 6;
          else
            state = 1;
        } else if (p_sdr->sensor_type == 0x9) {
          if (p_sdr->nominal == 0)
            state = 0;
          else if (p_sdr->nominal < p_sdr->unc_thresh)
            state = 6;
          else
            state = 1;
        }
        SENSOR["STATE"] = json::value::string(to_string(state));
        break;
      }
      SENSOR["NUMBER"] = json::value::string(to_string(p_sdr->sensor_num));
    }
    sensor_vec.push_back(SENSOR);
  }
  sensor_info["SENSOR"] = json::value::array(sensor_vec);
  response_json["SENSOR_INFO"] = sensor_info;
  

}

void Ipmiweb_GET::Get_Eventlog(json::value &response_json){
    int i = 0;
    int getfail, next;
    sel_msg_t msg;
    struct tm *t;
    time_t time;
    char buf[100];
    char ret[30000] ={0,};
  
    cout << " statrt eventlog " <<endl;
    strcpy(ret, "{\n");
    strcat(ret, "\"EVENT_INFO\": {\n  \"SEL\":\n\t[\n");
    int eventlog_cnt = plat_sel_num_entries();
    while (i < eventlog_cnt)
    {
        (getfail = plat_sel_get_entry(i, &msg, &next));
        if (getfail)
            break;
        memcpy(&time, &msg.msg[3], sizeof(time_t));
        //    printf("time: %s\n", ipmi_sel_timestamp(time));
        (t = localtime(&time));

        strcat(ret, "\t\t{\n");

        if ((t->tm_year + 1900) < 2017)
            sprintf(buf, "\t\t \"TIME\": \"Pre-Init\",\n");
        else
            sprintf(buf, "\t\t \"TIME\": \"%04d-%02d-%02d %02d:%02d:%02d\",\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
        strcat(ret, buf);
        char sName[30];
        int namelen = 0;

        string sensorname;
        (namelen = find_sensor_name(msg.msg[10], msg.msg[11], sensorname));
        strcpy(sName, sensorname.c_str());
        if (namelen < 0)
            sprintf(sName, "%s : 0x%02X", ipmi_generic_sensor_type_val[msg.msg[10]], msg.msg[11]);
        sprintf(buf, "\t\t \"NAME\": \"%s\",\n", sName); // need parsing through sensor enum
        strcat(ret, buf);

        sprintf(buf, "\t\t \"SEL_RD\": \"%x\",\n", msg.msg[12]); // no use
        strcat(ret, buf);

        sprintf(buf, "\t\t \"TYPE\": \"%s\",\n", ipmi_generic_sensor_type_val[msg.msg[10]]);
        strcat(ret, buf);

        char buf_des[128] = {0};
        char buf_ret[256] = {0};

        msg.msg[13] = msg.msg[13] & 0x0f;

        if (msg.msg[10] != 0x08 && msg.msg[10] != 0x2b && msg.msg[10] != 0x0f)
            msg.msg[14] = 0xff;

        if (msg.msg[12] != 0x6f)
            msg.msg[14] = 0xff;

        ipmi_get_event_desc(buf_des, &msg);
        sprintf(buf_ret, "\t\t \"DESCRIPTION\": \"%s\",\n", buf_des);
        strcat(ret, buf_ret);

     

      sprintf(buf, "\t\t \"SENSOR_ID\": \"%02X\"\n", i);
      strcat(ret, buf);
      if (++i < eventlog_cnt && strlen(ret) < 34000)
      {
          strcat(ret, "\t\t},\n");
      }
      else
      {
          strcat(ret, "\t\t}\n");
          break;
      }
    }
    strcat(ret, "\t]\n  }\n}\n");
    response_json = json::value::parse(ret);
    cout << "eventlog json " <<endl ;
           
}

void Ipmiweb_GET::Get_DDNS_Info(json::value &response_json){
  char buf[128];
  unsigned char domain_name[50] = {0};
  unsigned char host_name[50] = {0};
  unsigned char nameserver_pre[30] = {0};
  unsigned char nameserver_alt[30] = {0};

  get_ddns_host_name(host_name);
  get_ddns_domain_name(domain_name);
  get_ddns_nameserver(1, nameserver_pre);
  get_ddns_nameserver(2, nameserver_alt);

  host_name[strlen(host_name) - 1] = '\0';

  // json::value obj = json::value::object();
  json::value DNS_INFO = json::value::object();
  json::value GENERIC = json::value::object();
  GENERIC["REGISTER_BMC_METHOD"] = json::value::string(U("DIRECT"));
  GENERIC["HOST_NAME"] = json::value::string(U((char *)host_name));
  GENERIC["DOMAIN_NAME"] = json::value::string(U((char *)domain_name));
  GENERIC["REGISTER_BMC"] = json::value::string(U("1"));

  json::value IPV6 = json::value::object();
  string s_ipaddr_v6(ipmiNetwork[0].ip_addr_v6.begin(),
                     ipmiNetwork[0].ip_addr_v6.end());
  IPV6["IPV6_PREFERRED"] = json::value::string(s_ipaddr_v6);
  IPV6["IPV6_ALTERNATE"] = json::value::string(U("localhost"));

  json::value IPV4 = json::value::object();
  IPV4["IPV4_PREFERRED"] = json::value::string(U((char *)nameserver_pre));
  IPV4["IPV4_ALTERNATE"] = json::value::string(U((char *)nameserver_alt));

  DNS_INFO["GENERIC"] = GENERIC;
  DNS_INFO["IPV6"] = IPV6;
  DNS_INFO["IPV4"] = IPV4;
  response_json["DNS_INFO"] = DNS_INFO;
  
}

void Ipmiweb_GET::Get_Lan_Info(json::value &response_json){
  int ipv4_srcs = 0;
  int vlan_enables = 0;
  int c = 0;
  int channel = 0;
  uint8_t r_ip_addr6[SIZE_IP_ADDR_V6] = {
      0,
  };
  uint8_t r_netmask6[SIZE_NET_MASK_V6] = {
      0,
  };
  uint8_t r_gateway6[SIZE_IP_ADDR_V6] = {
      0,
  };

  for (int i = 0; i < MAX_NIC; i++) {
    if (i == 0)
      channel = 1;
    else if (i == 1)
      channel = 8;
    else
      channel = 0; // undefined

    if (get_ipv6_info(channel, (unsigned char *)r_ip_addr6,
                      (unsigned char *)r_netmask6,
                      (unsigned char *)r_gateway6) == -1) {
      fprintf(stderr, "get ipv6 info failed\n");
      return 0;
    }

    // printf("ip : %s\nnetmask : %s\ngateway : %s\n", r_ip_addr6, r_netmask6,
    // r_gateway6); printf("\t\t\t dy : get lan checkpoint 1 in channel
    // %d===========\n", channel);

    ipmiNetwork[i].ip_addr_v6.assign(r_ip_addr6, r_ip_addr6 + SIZE_IP_ADDR_V6);
    ipmiNetwork[i].net_mask_v6.assign(r_netmask6,
                                      r_netmask6 + SIZE_NET_MASK_V6);
    ipmiNetwork[i].df_gw_ip_addr_v6.assign(r_gateway6,
                                           r_gateway6 + SIZE_IP_ADDR_V6);

    // printf("\t\t\t dy : get lan checkpoint 2 ===========\n");
    // string ip6(ipmiNetwork[i].ip_addr_v6.begin(),
    // ipmiNetwork[i].ip_addr_v6.end()); string
    // nm6(ipmiNetwork[i].net_mask_v6.begin(),
    // ipmiNetwork[i].net_mask_v6.end()); string
    // gw6(ipmiNetwork[i].df_gw_ip_addr_v6.begin(),
    // ipmiNetwork[i].df_gw_ip_addr_v6.end()); printf("ip_v6 : %s\nnet_mask_v6 :
    // %s\ngw_ip : %s\n", ip6.c_str(), nm6.c_str(), gw6.c_str());
  }

  switch (ipmiNetwork[0].ip_src) {
  case 1:
    ipv4_srcs = 0;
    break;
  case 2:
    ipv4_srcs = 1;
    break;
  }

  char prior[8];
  if (get_eth_priority() == 1) {
    strcpy(prior, "eth0");
  }

  else if (get_eth_priority() == 8) {
    strcpy(prior, "eth1");
  } else {
    strcpy(prior, "Unknown");
  }

  // json::value obj = json::value::object();
  response_json["NETWORK_PRIORITY"] = json::value::string(U(prior));
  vector<json::value> net_info_vec;

  char device[16];
  for (int i = 0; i < MAX_NIC; i++) {
    // i == 0 ==> DEV_NAME_SHARED, i == 1 ==> DEV_NAME_DEDI
    json::value NETWORK_INFO = json::value::object();

    sprintf(device, "eth%d", i);
    if ((ipmiNetwork[i].vlan_enable && 0x80) == 1)
      vlan_enables = 1;
    else
      vlan_enables = 0;

    switch (ipmiNetwork[i].ip_src) {
    case 1:
      ipv4_srcs = 0;
      break;
    case 2:
      ipv4_srcs = 1;
      break;
    }

    NETWORK_INFO["LAN_INTERFACE"] = json::value::string(U(device));
    json::value GENERIC = json::value::object();
    json::value IPV4 = json::value::object();
    json::value IPV6 = json::value::object();
    json::value VLAN = json::value::object();
    char buf[64] = {
        0,
    };

    GENERIC["LAN_SETTING_ENABLE"] =
        json::value::string(U(to_string(ipmiNetwork[i].set_enable)));
    sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", ipmiNetwork[i].mac_addr[0],
            ipmiNetwork[i].mac_addr[1], ipmiNetwork[i].mac_addr[2],
            ipmiNetwork[i].mac_addr[3], ipmiNetwork[i].mac_addr[4],
            ipmiNetwork[i].mac_addr[5]);
    GENERIC["MAC_ADDRESS"] = json::value::string(U(buf));

    memset(buf, 0, sizeof(buf));

    IPV4["IPV4_PREFERRED"] = json::value::string(U(""));
    sprintf(buf, "%d.%d.%d.%d", ipmiNetwork[i].df_gw_ip_addr[0],
            ipmiNetwork[i].df_gw_ip_addr[1], ipmiNetwork[i].df_gw_ip_addr[2],
            ipmiNetwork[i].df_gw_ip_addr[3]);
    IPV4["IPV4_GATEWAY"] = json::value::string(U(buf));
    sprintf(buf, "%d.%d.%d.%d", ipmiNetwork[i].net_mask[0],
            ipmiNetwork[i].net_mask[1], ipmiNetwork[i].net_mask[2],
            ipmiNetwork[i].net_mask[3]);
    IPV4["IPV4_NETMASK"] = json::value::string(U(buf));
    sprintf(buf, "%d.%d.%d.%d", ipmiNetwork[i].ip_addr[0],
            ipmiNetwork[i].ip_addr[1], ipmiNetwork[i].ip_addr[2],
            ipmiNetwork[i].ip_addr[3]);
    IPV4["IPV4_ADDRESS"] = json::value::string(U(buf));
    IPV4["IPV4_DHCP_ENABLE"] = json::value::string(U(to_string(ipv4_srcs)));

    string s_subnet_mask_v6(ipmiNetwork[i].net_mask_v6.begin(),
                            ipmiNetwork[i].net_mask_v6.end());
    IPV6["IPV6_SUBNET_PREFIX_LENGTH"] = json::value::string(s_subnet_mask_v6);
    string s_ip_addr_v6(ipmiNetwork[i].ip_addr_v6.begin(),
                        ipmiNetwork[i].ip_addr_v6.end());
    IPV6["IPV6_ADDRESS"] = json::value::string(s_ip_addr_v6);
    IPV6["IPV6_ENABLE"] =
        json::value::string(U(to_string(ipmiNetwork[i].set_enable_v6)));
    IPV6["IPV6_DHCP_ENABLE"] =
        json::value::string(U(to_string(ipmiNetwork[i].ip_src_v6)));
    string s_gateway(ipmiNetwork[i].df_gw_ip_addr_v6.begin(),
                     ipmiNetwork[i].df_gw_ip_addr_v6.end());
    IPV6["IPV6_GATEWAY"] = json::value::string(s_gateway);

    VLAN["VLAN_SETTINGS_ENABLE"] =
        json::value::string(U(to_string(vlan_enables)));
    VLAN["VLAN_ID"] = json::value::string(U(to_string(ipmiNetwork[i].vlan_id)));
    VLAN["VLAN_PRIORITY"] =
        json::value::string(U(to_string(ipmiNetwork[i].vlan_priority)));

    NETWORK_INFO["GENERIC"] = GENERIC;
    NETWORK_INFO["IPV4"] = IPV4;
    NETWORK_INFO["IPV6"] = IPV6;
    NETWORK_INFO["VLAN"] = VLAN;
    net_info_vec.push_back(NETWORK_INFO);
  }
  response_json["NETWORK_INFO"] = json::value::array(net_info_vec);
  
}


void Ipmiweb_GET::Get_Ntp_Info(json::value &response_json) {
  const char *etables[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
                        "Aug", "Sep", "Oct", "Nov", "Dec", NULL};
  // json::value obj = json::value::object();
  json::value NTP_INFO = json::value::object();
  json::value NTP = json::value::object();

  if (access(NTPFILE, F_OK) == 0) {
    char buf[64];
    char server[32];
    char cmds[100] = {
        0,
    };

    NTP["AUTO_SYNC"] = json::value::string(U("1"));

    // ntp server 백업 주소 가져오기.
    if (access("/etc/ntp.conf.bak", F_OK) == 0) {
      sprintf(cmds, "mv /etc/ntp.conf.bak /etc/ntp.conf");
      system(cmds);
    }
    sprintf(buf, "awk '$1 == \"server\" {print $2}' %s", NTPFILE);

    FILE *fp = popen(buf, "r");

    fgets(server, 32, fp);
    server[strlen(server) - 1] = '\0';

    pclose(fp);

    NTP["NTP_SERVER"] = json::value::string(U(server));
    NTP["TIME_ZONE"] = json::value::string(U(""));
    NTP["YEAR"] = json::value::string(U(""));
    NTP["MONTH"] = json::value::string(U(""));
    NTP["DAY"] = json::value::string(U(""));
    NTP["HOUR"] = json::value::string(U(""));
    NTP["MIN"] = json::value::string(U(""));
    NTP["SEC"] = json::value::string(U(""));
  } else {
    char date[32];
    char buf[8];

    FILE *fp = popen("date -R", "r");
    fgets(date, 32, fp);
    pclose(fp);

    char *token;
    token = strtok(date, " ");
    token = strtok(NULL, " ");

    NTP["AUTO_SYNC"] = json::value::string(U("0"));
    NTP["NTP_SERVER"] = json::value::string(U(""));
    NTP["DAY"] = json::value::string(U(token));

    token = strtok(NULL, " ");
    for (int i = 0; etables[i] != NULL; i++)
      if (!strcmp(token, etables[i]))
        NTP["MONTH"] = json::value::string(to_string(i + 1));

    token = strtok(NULL, " ");
    NTP["YEAR"] = json::value::string(U(token));

    token = strtok(NULL, " :");
    NTP["HOUR"] = json::value::string(U(token));

    token = strtok(NULL, " :");
    NTP["MIN"] = json::value::string(U(token));

    token = strtok(NULL, " :");
    NTP["SEC"] = json::value::string(U(token));

    token = strtok(NULL, " ");

    if (token[0] == '-')
      sprintf(buf, "GMT-%d", atoi(token + 1));
    else
      sprintf(buf, "GMT+%d", atoi(token + 1) / 100);

    NTP["TIME_ZONE"] = json::value::string(U(buf));
  }

  NTP_INFO["NTP"] = NTP;
  response_json["NTP_INFO"] = NTP_INFO;
  
}
void Ipmiweb_GET::Get_Smtp_Info(json::value &response_json, int flag){
  char machine_name[50] = "\0", sender_address[50] = "\0",
       primary_server_address[50] = "\0", primary_user_name[50] = "\0";
  char primary_user_password[50] = "\0", secondary_server_address[50] = "\0",
       secondary_user_name[50] = "\0", secondary_user_password[50] = "\0";
  smtp_config_t smtp_config;
  char res[3000]={0,};

  if (access(SMTP_BIN, F_OK) == -1) {
    fprintf(stderr, "\t\tWarning : No SMTP configured.\n");
  }

  FILE *fp = fopen(SMTP_BIN, "r");

  if (fp != NULL) {
    if (fread(&smtp_config, sizeof(smtp_config_t), 1, fp) < 1) {
      fprintf(stderr, "\t\tError : fread smtp_config for parsing failed\n");
      fclose(fp);
      return;
    }

    strcpy(machine_name, smtp_config.machine);
    strcpy(sender_address, smtp_config.sender);
    strcpy(primary_server_address, smtp_config.server1);
    strcpy(primary_user_name, smtp_config.id1);
    strcpy(primary_user_password, smtp_config.pwd1);
    strcpy(secondary_server_address, smtp_config.server2);
    strcpy(secondary_user_name, smtp_config.id2);
    strcpy(secondary_user_password, smtp_config.pwd2);
    fclose(fp);
  }
  primary_user_password[strlen(primary_user_password)] = '\0';

  char buf[200];
  // json::value obj = json::value::object();
  json::value SMTP_INFO = json::value::object();

  if (flag == 0) {
    json::value DEVICE = json::value::object();
    DEVICE["MACHINE_NAME"] = json::value::string(U(machine_name));
    DEVICE["SENDER_ADDRESS"] = json::value::string(U(sender_address));

    json::value PRIMARY = json::value::object();
    PRIMARY["PRIMARY_SERVER_ADDRESS"] =
        json::value::string(U(primary_server_address));
    PRIMARY["PRIMARY_USER_NAME"] = json::value::string(U(primary_user_name));
    PRIMARY["PRIMARY_USER_PASSWORD"] =
        json::value::string(U(primary_user_password));

    json ::value SECONDARY = json::value::object();
    SECONDARY["SECONDARY_SERVER_ADDRESS"] =
        json::value::string(U(secondary_server_address));
    SECONDARY["SECONDARY_USER_NAME"] =
        json::value::string(U(secondary_user_name));
    SECONDARY["SECONDARY_USER_PASSWORD"] =
        json::value::string(U(secondary_user_password));

    SMTP_INFO["DEVICE"] = DEVICE;
    SMTP_INFO["PRIMARY"] = PRIMARY;
    SMTP_INFO["SECONDARY"] = SECONDARY;
    response_json["SMTP_INFO"] = SMTP_INFO;
    // strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  } else if (flag == 1) { // get primary receiver
    if (strcmp(primary_server_address, "\0") == 0) {
      fprintf(stderr, "No primary receiver\n");
      // return 0;
    }
    sscanf(primary_server_address, "smtp.%s", buf);
    sprintf(res, "%s@%s", primary_user_name, buf);
  } else if (flag == 2) { // get secondary receiver
    if (strcmp(secondary_server_address, "\0") == 0) {
      fprintf(stderr, "No secondary receiver\n");
      // return 0;
    }
    sscanf(secondary_server_address, "smtp.%s", buf);
    sprintf(res, "%s@%s", secondary_user_name, buf);
  }
  response_json = json::value::parse(res);
}

void Ipmiweb_GET::Get_Ssl_Info(json::value &response_json){  
  char country[3] = "\0", state_province[32] = "\0", city_locality[32] = "\0",
       organ[32] = "\0";
  char organ_unit[32] = "\0", common[32] = "\0", email[32] = "\0",
       keylen[5] = "\0";
  char valid_from[16] = "\0", valid_to[16] = "\0";
  int valid_for = 0;
  FILE *fp_read = fopen(SSL_BIN, "r");
  CertificateService *certificate =
      ((CertificateService *)g_record[ODATA_CERTIFICATE_SERVICE_ID]);

  if (fp_read == NULL) {
    fprintf(stderr, "\t\tWarning : No ssl_bin.\n");
  } else {
    ssl_config_t ssl_config;
    if (fread(&ssl_config, sizeof(ssl_config_t), 1, fp_read) < 1) {
      fprintf(stderr, "\t\tError : fread ssl_config for parsing failed\n");
      fclose(fp_read);
      return 0;
    }

    fclose(fp_read);

    strcpy(country, ssl_config.country);
    strcpy(state_province, ssl_config.state_province);
    strcpy(city_locality, ssl_config.city_locality);
    strcpy(organ, ssl_config.organ);
    strcpy(organ_unit, ssl_config.organ_unit);
    strcpy(common, ssl_config.common);
    strcpy(email, ssl_config.email);
    strcpy(keylen, ssl_config.keylen);
    strcpy(valid_from, ssl_config.valid_from);
    strcpy(valid_to, ssl_config.valid_to);
    valid_for = ssl_config.valid_for;
  }

  // json::value obj = json::value::object();
  json::value SSL_INFO = json::value::object();

  json::value BASIC = json::value::object();
  BASIC["VERSION"] = json::value::string(U("1.0.2e"));
  BASIC["SERIAL_NUMBER"] = json::value::string(U("9FF7A"));
  BASIC["SIGNATURE_ALGORITHM"] = json::value::string(U("RSA"));

  json::value ISSUED_FROM = json::value::object();
  ISSUED_FROM["COMMON_NAME"] = json::value::string(U(common));
  ISSUED_FROM["ORGANIZATION"] = json::value::string(U(organ));
  ISSUED_FROM["ORGANIZATION_UNIT"] = json::value::string(U(organ_unit));
  ISSUED_FROM["CITY_OR_LOCALITY"] = json::value::string(U(city_locality));
  ISSUED_FROM["STATE_OR_PROVINCE"] = json::value::string(U(state_province));
  ISSUED_FROM["COUNTRY"] = json::value::string(U(country));
  ISSUED_FROM["EMAIL_ADDRESS"] = json::value::string(U(email));
  ISSUED_FROM["VALID_FOR"] = json::value::string(to_string(valid_for));
  ISSUED_FROM["KEY_LENGTH"] = json::value::string(U("1024"));

  json::value VALIDITY_INFORMATION = json::value::object();
  VALIDITY_INFORMATION["VALID_FROM"] = json::value::string(U(valid_from));
  VALIDITY_INFORMATION["VALID_FOR"] = json::value::string(U(valid_to));

  json::value ISSUED_TO = json::value::object();
  ISSUED_TO["COMMON_NAME"] = json::value::string(U(common));
  ISSUED_TO["ORGANIZATION"] = json::value::string(U(organ));
  ISSUED_TO["ORGANIZATION_UNIT"] = json::value::string(U(organ_unit));

  SSL_INFO["BASIC"] = BASIC;
  SSL_INFO["ISSUED_FROM"] = ISSUED_FROM;
  SSL_INFO["ISSUED_TO"] = ISSUED_TO;
  SSL_INFO["VALIDITY_INFORMATION"] = VALIDITY_INFORMATION;
  response_json["SSL_INFO"] = SSL_INFO;

  // strncpy(res, obj.serialize().c_str(), obj.serialize().length());
  // response_json = json::value::parse(res);
}


void Ipmiweb_GET::Get_Active_Dir(json::value &response_json){
 int enable = 0;
  char ip[16] = {
      0,
  };
  char domain[64] = {
      0,
  };
  char s_username[64] = {
      0,
  };
  char s_password[32] = {
      0,
  };

  FILE *fp_read = fopen(AD_BIN, "rb");
  if (fp_read != NULL) {
    ad_config_t ad_config;
    if (fread(&ad_config, sizeof(ad_config_t), 1, fp_read) < 1) {
      fprintf(stderr, "Error : fread ad_config for GET failed\n");
    }
    enable = ad_config.enable;
    if (enable) {
      strcpy(ip, ad_config.dc_ip);
      strcpy(domain, ad_config.domain);
      strcpy(s_username, ad_config.secret_name);
      strcpy(s_password, ad_config.secret_pwd);
    }
    fclose(fp_read);
  }

  // json::value obj = json::value::object();
  json::value ACTIVE_DIRECTORY = json::value::object();
  ACTIVE_DIRECTORY["ENABLE"] = json::value::string(U(std::to_string(enable)));
  ACTIVE_DIRECTORY["IP"] = json::value::string(U(ip));
  ACTIVE_DIRECTORY["DOMAIN"] = json::value::string(U(domain));
  ACTIVE_DIRECTORY["SECRET_NAME"] = json::value::string(U(s_username));
  ACTIVE_DIRECTORY["SECRET_PWD"] = json::value::string(U(s_password));
  response_json["ACTIVE_DIRECTORY"] = ACTIVE_DIRECTORY;

}


void Ipmiweb_GET::Get_Ldap(json::value &response_json){
char ip[16] =
      {
          0,
      },
       port[6] =
           {
               0,
           },
       searchbase[32] =
           {
               0,
           },
       binddn[32] =
           {
               0,
           },
       pwd[32] = {
           0,
       };
  int timelimit = 0;
  int enable = 0;
  int ssl = 0;
  ldap_config_t ldap_config;

  memset(&ldap_config, 0, sizeof(ldap_config_t));
  FILE *fp_read = fopen(LDAP_BIN, "rb");
  if (fp_read != NULL) {
    if (fread(&ldap_config, sizeof(ldap_config_t), 1, fp_read) < 1) {
      fprintf(stderr, "Error : fread ldap_config\n");
    }
    enable = ldap_config.enable;
    printf("\t\t\t\tcheckpoint2 : ldap en : %d\n", enable);

    if (enable) {
      strcpy(ip, ldap_config.ip);
      strcpy(port, ldap_config.port);
      strcpy(searchbase, ldap_config.basedn);
      strcpy(binddn, ldap_config.binddn);
      strcpy(pwd, ldap_config.bindpw);
      ssl = ldap_config.ssl;
      timelimit = ldap_config.timelimit;
    }
    fclose(fp_read);
  }

  // json::value obj = json::value::object();
  json::value LDAP_INFO = json::value::object();
  json::value LDAP = json::value::object();

  LDAP["LDAP_EN"] = json::value::string(U(to_string(enable)));
  LDAP["BIND_PW"] = json::value::string(U(pwd));
  LDAP["LDAP_IP"] = json::value::string(U(ip));
  LDAP["LDAP_PORT"] = json::value::string(U(port));
  LDAP["TIMEOUT"] = json::value::string(U(to_string(timelimit)));
  LDAP["BASE_DN"] = json::value::string(U(searchbase));
  LDAP["LDAP_SSL"] = json::value::string(U(to_string(ssl)));
  LDAP["BIND_DN"] = json::value::string(U(binddn));
  LDAP_INFO["LDAP"] = LDAP;
  response_json["LDAP_INFO"] = LDAP_INFO;
}

void Ipmiweb_GET::Get_Radius(json::value &response_json){
  char ip[16] = "\0", port[6] = "\0", secret[32] = "\0";
  FILE *fp = fopen(RAD_BIN, "r");
  rad_config_t rad_config;
  int enable;
  if (fp != NULL) {
    if (fread(&rad_config, sizeof(rad_config_t), 1, fp) < 1) {
      fprintf(stderr, "\t\tError : fread error radc_config");
      fclose(fp);
      return 0;
    }
    enable = rad_config.enable;
    strcpy(ip, rad_config.ip);
    strcpy(port, rad_config.port);
    strcpy(secret, rad_config.secret);
    fclose(fp);
  }

  // json::value obj = json::value::object();
  json::value RADIUS_INFO = json::value::object();
  json::value RADIUS = json::value::object();

  RADIUS["RADIUS_ENABLE"] = json::value::string(to_string(enable));
  RADIUS["IP"] = json::value::string(U(ip));
  RADIUS["PORT"] = json::value::string(U(port));
  RADIUS["SECRET"] = json::value::string(U(secret));
  RADIUS_INFO["RADIUS"] = RADIUS;
  response_json["RADIUS_INFO"] = RADIUS_INFO;
}


void Ipmiweb_GET::Get_Setting_Service(json::value &response_json){	
  // json::value obj = json::value::object();
	json::value SETTING_SERVICE = json::value::object();

	if ((access(SSH_SERVICE_BIN, F_OK) != 0) || (access(KVM_PORT_BIN, F_OK) != 0) || (access(ALERT_PORT_BIN, F_OK) != 0) || (access(WEB_PORT_BIN, F_OK) != 0)) {	
		if (init_setting_service() != 0)
			return FAIL;
	}

	SETTING_SERVICE["WEB_PORT"] = json::value::string(U(g_setting.web_port));
	SETTING_SERVICE["SSH_ENABLES"] = json::value::number(g_setting.ssh_enables);
	SETTING_SERVICE["SSH_PORT"] = json::value::string(U(g_setting.ssh_port));
	SETTING_SERVICE["ALERT_ENABLES"] = json::value::number(g_setting.alert_enables);
	SETTING_SERVICE["ALERT_PORT"] = json::value::string(U(g_setting.alert_port));
	SETTING_SERVICE["KVM_ENABLES"] = json::value::number(g_setting.kvm_enables);
	SETTING_SERVICE["KVM_PORT"] = json::value::string(U(g_setting.kvm_port));
	SETTING_SERVICE["KVM_PROXY_PORT"] = json::value::string(U(g_setting.kvm_proxy_port));
	response_json["SETTING_SERVICE"] = SETTING_SERVICE;
  }
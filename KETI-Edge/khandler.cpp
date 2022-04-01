
#include <ipmi/common.hpp>
#include <ipmi/ipmi.hpp>
#include <ipmi/apps.hpp>
#include <iostream>
#include "khandler.hpp"
#include <redfish/rmcp.hpp>

#include "handler.hpp"
#include <redfish/resource.hpp>
#include <redfish/hwcontrol.hpp>
#include <boost/log/trivial.hpp>
#include "lssdp.hpp"
#include<ipmi/network.hpp>

#include<ipmi/gpio.hpp>
#include<ipmi/dcmi.hpp>
#include<ipmi/sdr.hpp>
#include<ipmi/lightning_sensor.hpp>
// #include"rest_handler.hpp"
unsigned int f_fan_error;
//unique_ptr<RHandler> g_restlistener;
#define  ETH_COUNT 4
//ipmi
extern Ipminetwork ipmiNetwork[ETH_COUNT]; 
extern Dcmiconfiguration dcmiConfiguration;
extern std::map<uint8_t, std::map<uint8_t, Ipmisdr>> sdr_rec;

unique_ptr<Handler> g_listener;
unordered_map<string, Resource *> g_record;
src::severity_logger<severity_level> g_logger;
ServiceRoot *g_service_root;

extern char uuid_str[37];
int ipmi_inprogress = 0;
extern uint16_t dcmi_sample_time;
typedef struct
{
        unsigned char
            state : 3,
            count : 3, // count for successive reading fail
            event : 1,
            rsvd : 1;
} host_status_t;

host_status_t dcmi_state[4];

Rmcppacket rmcpInPacket; // RMCP 클래스 생성
uint8_t ERROR_CASE_1[24] = {6, 0, 255, 7, 6, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 8, 0,
                            32, 24, 200, 129, 4, 59, 4, 60};

uint8_t ERROR_CASE_2[48] = {6, 0, 255, 7, 6, 16, 0, 0,
                            0, 0, 0, 0, 0, 0, 32, 0,
                            0, 0, 0, 0, 164, 163, 162, 160,
                            0, 0, 0, 8, 1, 0, 0, 0,
                            1, 0, 0, 8, 1, 0, 0, 0,
                            2, 0, 0, 8, 1, 0, 0, 0};
// char ssdp_buff[] = "M-SEARCH * HTTP/1.1\r\n"\
// "HOST: 239.255.255.250:1900\r\n"\
// "MAN: \"ssdp:discover\"\r\n"\
// "MX: 3\r\n"\
// "ST: udap:rootservice\r\n"\
// "USER-AGENT: RTLINUX/5.0 UDAP/2.0 printer/4\r\n\r\n";

void *dcmi_power_handler(void *data)
{
        dcmi_sample_time = 10;

        while (1)
        {
                // printf("dcmi power handler\n");
                sleep(dcmi_sample_time);
        }
        
}
/**
 * @brief REST 서버 수행
 * @details REDFISH, KTNF REST 서버 실행
 * @param choice 0 = REDFISH, 1= KTNF SERVER
 * @bug 1 = KETNF 사용불가 몽구스 WEB으로 변경중 
 */
void start_server(utility::string_t &_url, http_listener_config _config, int choice)
{
        if (choice == 0)
        {
                g_listener = unique_ptr<Handler>(new Handler(_url, _config));
                log(info) << " BMC Redfish server start";
                g_listener->open().wait();
        }

        // else if (choice == 1)
        // {
        //         log(info) << " Rest server start";
        //         g_restlistener = unique_ptr<RHandler>(new RHandler(_url, _config));
        //         g_restlistener->open().wait();
        // }
}

void *kcs_handler(void *data)
{
        int req_len = 0;
        uint8_t res_len = 0;
        int kcs_fd = *((int *)data);
        uint8_t req_buf[MAX_IPMI_MSG_SIZE];
        uint8_t res_buf[MAX_IPMI_MSG_SIZE];

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(kcs_fd, &readfds);
        log(info) << "kcs_handler  start";
        while (1)
        {

                req_len = read(kcs_fd, req_buf, sizeof(req_buf));
                if (req_len > 0)
                {
                        while (ipmi_inprogress)
                        {
                                delay(1);
                        }
                        ipmi_inprogress = 1;

                        ipmi_handle(0, req_buf, req_len, res_buf, &res_len);

                        res_len = write(kcs_fd, res_buf, res_len);
                        cout << "res_buf =";
                        cout << res_buf << endl;
                        ipmi_inprogress = 0;
                }
                delay(1);
        }
}

int check_fan_status(int i)
{
        //int i;
        int rpm;
        int rpm0;
        int ret;
        static int count = 0;

        if (i==11) return 0;
        if (count <10){
                count++;
                return 0;
        }
        ret = 0;
        while(ipmi_inprogress){
                delay(1);
        };
        ipmi_inprogress = 1;
        //for (i=0; i<=11; i++){
                read_fan_value(i, &rpm);
                ipmi_inprogress = 0;

                if (rpm==0) {
                        f_fan_error &= ~(0x00000001<<i);
                        return (ret);
    }
    //printf("\nSKKIM) FAN LEVEL!!! : %d RPM\n", rpm);
        //      if (fan_level == 0) rpm0 = 16000;
        //      else rpm0 = (fan_level)*16000/60;
                if (rpm < 1000){//(75*rpm0/100)) {
//                      printf("fan error no=%d, rpm=%d\n", i, rpm);
    ret = 1;
    f_fan_error |= 0x00000001<<i;
    }else{
        f_fan_error &= ~(0x00000001<<i);
    }
    if (i==14){
//                      if (f_fan_error) printf("f_fan_error = 0x%08x\n", f_fan_error);
        if ((f_fan_error & 0x00000001) || (f_fan_error & (0x00000001<<6)) ) ast_set_gpio_value(FAN_FAULT_LED0, 1);
        else ast_set_gpio_value(FAN_FAULT_LED0, 0);
        if ((f_fan_error & (0x00000001<<1)) || (f_fan_error & (0x00000001<<7)) ) ast_set_gpio_value(FAN_FAULT_LED1, 1);
        else ast_set_gpio_value(FAN_FAULT_LED1, 0);
        if ((f_fan_error & (0x00000001<<2)) || (f_fan_error & (0x00000001<<8)) ) ast_set_gpio_value(FAN_FAULT_LED2, 1);
        else ast_set_gpio_value(FAN_FAULT_LED2, 0);
        if ((f_fan_error & (0x00000001<<3)) || (f_fan_error & (0x00000001<<9)) ) ast_set_gpio_value(FAN_FAULT_LED3, 1);
        else ast_set_gpio_value(FAN_FAULT_LED3, 0);
        if ((f_fan_error & (0x00000001<<4)) || (f_fan_error & (0x00000001<<10)) ) ast_set_gpio_value(FAN_FAULT_LED4, 1);
        else ast_set_gpio_value(FAN_FAULT_LED4, 0);
        if ((f_fan_error & (0x00000001<<5)) || (f_fan_error & (0x00000001<<11)) ) ast_set_gpio_value(FAN_FAULT_LED5, 1);
        else ast_set_gpio_value(FAN_FAULT_LED5, 0);

    }
//}

    return(ret);
}

#define TIMER_100MS_ID 10
#define TIMER_100MS_SECONDS 0
#define TIMER_250MS_SECONDS 0
#define TIMER_250MS_ID 25
#define TIMER_250MS_SECONDS 0
#define TIMER_500MS_ID 50
#define TIMER_500MS_SECONDS 0
#define TIMER_1SEC_ID    		1
#define TIMER_1SEC_SECONDS 			1
#define TIMER_60SEC_ID    		60
#define TIMER_60SEC_SECONDS 			60

#define TIMER_3SEC_ID    		3
#define TIMER_3SEC_SECONDS 			3
#define TIMER_5SEC_ID    		5
#define TIMER_5SEC_SECONDS 			5
bool edgetimer_stop_flag = false;

extern Ipminetwork ipmiNetwork[4];
/**
 * @brief power on off 유무
 */
int bPowerGD = 1; // temporary
void dcmi_power_limit_function()
{
        // std::vector<json::value> JVCPU, JCPU0_MEMORY, JCPU1_MEMORY, JPOWER, JFAN;
        char buf[64];
        int snr_id, idx;
        int index = 0;
        sensor_thresh_t psu1, psu2;
        int sensor_index = -1, namelen = -1;
        int t_watt, l_watt = 0;
        // sensor_thresh_t psu1,psu2;
        string sensorname;
        float watt[2];
        bPowerGD=ipmiChassis.get_power_status();
        if (bPowerGD)
        {
                if ((dcmiConfiguration.power_reading_state >> 6) == 1)
                {
                        //log(info)<<"dcmi_power_limit_function step 1 ";
                        sensor_index = plat_find_sdr_index(NVA_SENSOR_PSU1_WATT);

                        if (sensor_index > 0)
                        {
                                psu1 = sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
                        }
                        else
                        {
                                log(info) << "not find NVA_SENSOR_PSU1_WATT Sensor index";
                        }
                        //log(info)<<"dcmi_power_limit_function step 2";
                        sensor_index = plat_find_sdr_index(NVA_SENSOR_PSU2_WATT);
                        if (sensor_index > 0)
                        {
                                psu2 = sdr_rec[sensor_index].find(sensor_index)->second.get_sensor_thresh_t();
                        }
                        else
                        {
                                log(info) << "not find NVA_SENSOR_PSU2_WATT Sensor index";
                        }
                        
                        //log(info)<<"dcmi_power_limit_function step 3";
                        watt[0] = sdr_convert_raw_to_sensor_value(&psu1, psu1.nominal);
                        watt[1] = sdr_convert_raw_to_sensor_value(&psu2, psu2.nominal);
                        //log(info)<<"dcmi_power_limit_function step 4";
                        t_watt = (int)watt[0] + (int)watt[1];
                        l_watt = dcmiConfiguration.dcmi_power_lmt[0] | (dcmiConfiguration.dcmi_power_lmt[1] << 8);
                       //log(info)<<"t_watt ="<<t_watt<<" l_watt="<<l_watt;
                //       cout<<"plat gogo"<<endl;
                //         Theshold_type th;
                //         th=Theshold_type::lnc_high;
                //         Sensor_tpye se;
                //         se=Sensor_tpye::Fan;
                //        plat_sel_threshold_add_entry(se,1,th);
                        if (t_watt >= l_watt)
                        {
                                if (dcmi_state[0].state != 2)
                                {
                                        //log(info)<<"dcmi_power_limit_function step 5";
                                        plat_sel_power_limit_add_entry();
                                        //test
                                        
                                        //log(info)<<"dcmi_power_limit_function step 6";
                                        //SMTP 미구현
                                        //prepare_SMTP(psu_1, dcmi_state[0].state);
                                }
                                //log(info)<<"dcmi_power_limit_function step 6";
                                dcmi_state[0].state = 2;
                               //log(info)<<"dcmi_power_limit_function step 7";
                        }
                        else{
                                //log(info)<<"dcmi_power_limit_function step 8";
                                dcmi_state[0].state = 0;
                                //log(info)<<"dcmi_power_limit_function step 9";
                        }
                }
        }
        log(info)<<"Update  dcmi_power_limit_function power="<<bool(bPowerGD);
}

/**
 * @brief enable에 따라 정면 LED가 on/off 됨 
 */
int id_enable=0;

void front_LED_blink(int id_interval)
{

        if(id_enable){
        ast_set_gpio_value(FRONT_ID_LED, 0);
        id_enable++;
        if(id_enable > 2) {
            id_enable = 0;
            ast_set_gpio_value(FRONT_ID_LED, 1);
        }
    }
}
/**
 * @brief 주기적인 업데이트가 필요한 리소스들을 위한 핸들러
 * @bug redfish 업데이트/sensor관련 이 필요함 update_hw_status 구현 필요 !!
 */
void *timer_handler(void)
{
        int count = 0;
        while (true)
        {
                
                if (edgetimer_stop_flag)
                {
                        return;
                }
                if(count==10)
                {       
                        for(int i=0; i<4;i++)
                        {
                                ipmiNetwork[i].plat_lan_changed(ipmiNetwork[i].chan);
                        }
                        for(int j=0;j<10;j++)
                        {
                                check_fan_status(j);
                        }
                        //update_hw_status();
                        dcmi_power_limit_function();
                        //log(info)<<"timer_handler count="<<count;
                        update_sensor_reading();
                        //log(info)<<"Update Sensorreading"<<count;
                        count=0;
                        
                }
                front_LED_blink(count++);
                sleep(1);
                
        }
}
void *lanplus_handler(void *data)
{
        int sockfd = *((int *)data);
        int byte_recv = 0;
        int check_error = 1;
        int ndcontinue = 1;
        int continuetwise = 1;

        uint8_t mesg[1000];
        struct sockaddr_in clientAddr;

        socklen_t len;

        ipmi_inprogress = 0;

        while (1)
        {
                len = sizeof(clientAddr);
                byte_recv = recvfrom(sockfd, mesg, 1000, 0, (struct sockaddr *)&clientAddr, &len);
                std::vector<uint8_t> packet_in(mesg, mesg + byte_recv);
                std::vector<uint8_t> packet_out;
                //log(info) << "lanplus_handler  len"<<len;
                rmcpInPacket.setRmcpInformation(packet_in, clientAddr);

                if (check_error && byte_recv % 24 == 0)
                {
                        int errorsize = byte_recv / 2;
                        if (errorsize == 1)
                        {
                                for (int i = 0; i < 24; i++)
                                        if (packet_in[i] != ERROR_CASE_1[i])
                                        {
                                                check_error = 0;
                                                ndcontinue = 0;
                                        }
                        }
                        else if (errorsize == 2 && continuetwise == 1)
                        {
                                for (int i = 0; i < 48; i++)
                                        if (packet_in[i] != ERROR_CASE_2[i])
                                        {
                                                check_error = 0;
                                                ndcontinue = 0;
                                        }
                                continuetwise--;
                        }
                        if (ndcontinue)
                        {
                                //LOG_WARN("ERROR DETECTED : CONTINUE\n");
                                continue;
                        }
                }
                else
                {
                        check_error = 0;
                }
                if (byte_recv != -1)
                {
                        packet_out = rmcpInPacket.rmcp_process_packet();
                        /*
			printf("packet out size : %d\n", packet_out.size());
			printf("packet out : ");
			for(int i  = 0 ; i < packet_out.size() ; i++)
				printf("0x%02X ", packet_out[i]);
			printf("\n");
                        */
                }

                if (packet_out.size() > 0)
                {
                        sendto(sockfd, packet_out.data(), packet_out.size(), 0, (struct sockaddr *)&clientAddr, sizeof(clientAddr));
                }
        }
        return 0;
}

void *redfish_handler(void *data)
{
        cout << "Redfish resource Initailize" << endl;

        //record_print();

        //     if(init_i2c())
        //         log(info) << "I2C initialization complete";

        if (init_resource())
        {
                log(info) << "Redfish resource initialization complete";
                cout << "Redfish resource Init" << endl;
        }
        
        http_listener_config listen_config;
        listen_config.set_timeout(utility::seconds(10)); //10초로 변경
        // Set SSL certification
        listen_config.set_ssl_context_callback([](boost::asio::ssl::context &_ctx) {
                _ctx.set_options(
                    boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 // Not use SSL2
                    | boost::asio::ssl::context::no_tlsv1                                                // NOT use TLS1
                    | boost::asio::ssl::context::no_tlsv1_1                                              // NOT use TLS1.1
                    | boost::asio::ssl::context::single_dh_use);

                // Certificate Password Provider
                //     _ctx.set_password_callback([](size_t max_length,
                //                                   boost::asio::ssl::context::password_purpose purpose) {
                //         return "ketilinux";
                //     });

                //openssl 인증서 갱싱 및 -> scp 전송
                //log(info) << "Server crt file path: " << SERVER_CERTIFICATE_CHAIN_PATH;
                _ctx.use_certificate_chain_file(SERVER_CERTIFICATE_CHAIN_PATH);
                //     //log(info) << "Server key file path: " << SERVER_PRIVATE_KEY_PATH;
                _ctx.use_private_key_file(SERVER_PRIVATE_KEY_PATH, boost::asio::ssl::context::pem);
                //     //log(info) << "Server pem file path: " << SERVER_TMP_DH_PATH;
                _ctx.use_tmp_dh_file(SERVER_TMP_DH_PATH);
        });

        // Set request timeout
        log(info) << "Server request timeout: " << SERVER_REQUEST_TIMEOUT << " sec";
        listen_config.set_timeout(utility::seconds(SERVER_REQUEST_TIMEOUT));

        // Set server entry point
        log(info) << "Server entry point: " << SERVER_ENTRY_POINT;
        utility::string_t url = U(SERVER_ENTRY_POINT);

        // RESTful server start
        start_server(url, listen_config, 0);
 

        while (true)
                pause();

        // g_listener->close().wait();
        // exit(0);
}

void log_callback(const char *file, const char *tag, int level, int line, const char *func, const char * message) {
    char *level_name = "DEBUG";
    if (level == LSSDP_LOG_INFO)    level_name = "INFO";
    if (level == LSSDP_LOG_WARN)    level_name = "WARN";
    if (level == LSSDP_LOG_ERROR)   level_name = "ERROR";

    printf("[%-5s][%s] %s", level_name, tag, message);
    return;
}


long long get_current_time() {
    struct timeval time = {};
    if (gettimeofday(&time, NULL) == -1) {
        printf("gettimeofday failed, errno = %s (%d)\n", strerror(errno), errno);
        return -1;
    }
    return (long long) time.tv_sec * 1000 + (long long) time.tv_usec / 1000;
}
char ipaddr[255]={0,};
int show_interface_list_and_rebind_socket(lssdp_ctx * lssdp) {
    // 1. show interface list
    printf("\nNetwork Interface List (%zu):\n", lssdp->interface_num);
    size_t i;
    for (i = 0; i < lssdp->interface_num; i++) {
        printf("%zu. %-6s: %s\n",
            i + 1,
            lssdp->interface[i].name,
            lssdp->interface[i].ip
        );
        strcpy(ipaddr, lssdp->interface[i].ip);
    }
    printf("%s\n", i == 0 ? "Empty" : "");

    // 2. re-bind SSDP socket
    if (lssdp_socket_create(lssdp) != 0) {
        puts("SSDP create socket failed");
        return -1;
    }

    return 0;
}

int show_ssdp_packet(struct lssdp_ctx * lssdp, const char * packet, size_t packet_len){
    //printf("%s", packet);
    vector<string> packet_info = string_split(std::string(packet), '\n');
//     for (auto str : packet_info){
//         if (!strncmp(str.c_str(), "LOCATION:", 9))
//             log(info) << str;
//     }
    // log(info) << "end show_ssdp_packet";
    return 0;
}

int show_neighbor_list(lssdp_ctx * lssdp) {
    int i = 0;
    lssdp_nbr * nbr;
    puts("\nSSDP LIST:");
    for (nbr = lssdp->neighbor_list; nbr != NULL; nbr = nbr->next) {
        printf("%d. id = %-9s, ip = %-20s, name = %-12s, device_type = %-8s (%lld)\n",
            ++i,
            nbr->sm_id,
            nbr->location,
            nbr->usn,
            nbr->device_type,
            nbr->update_time
        );
    }
    printf("%s\n", i == 0 ? "Empty" : "");
    return 0;
}
//ssdp를위한 ipstr
 static char ip_str[128];
void *ssdp_handler(void)
{
    lssdp_set_log_callback(log_callback);
//     string ip="";
//     ip=get_value_from_cmd_str("ifconfig etho0 | grep \"inet addr\"", "inet addr")+string(+":");
//     //ip =ipaddr+string(":")+SERVER_ENTRY_PORT;

    memset(ip_str, 0, sizeof(ip_str));
    sprintf(ip_str, "%u.%u.%u.%u", ipmiNetwork[0].ip_addr[0], ipmiNetwork[0].ip_addr[1], ipmiNetwork[0].ip_addr[2], ipmiNetwork[0].ip_addr[3]);
    //cout << ip_str << "this ip : port\n\n\n\n"<< endl;
    lssdp_ctx lssdp = {
        .port = 1900,
        .neighbor_timeout = 15000, // 15seconds
        //.debug = true,
        .header = {
            "BMC-CM1",
            "CM1",
            .location = {
                "http://",
                "0.0.0.0",
                SERVER_ENTRY_PORT,
            },
            "CM1_TYPE",
        },
        // callback
        .network_interface_changed_callback = show_interface_list_and_rebind_socket,
        .neighbor_list_changed_callback = show_neighbor_list,
        .packet_received_callback = show_ssdp_packet,
    };
    memset(lssdp.header.location.domain, 0, sizeof(lssdp.header.location.domain, ip_str));
    sprintf(lssdp.header.location.domain, "%u.%u.%u.%u", ipmiNetwork[0].ip_addr[0], ipmiNetwork[0].ip_addr[1], ipmiNetwork[0].ip_addr[2], ipmiNetwork[0].ip_addr[3]);
    lssdp_network_interface_update(&lssdp);

    long long last_time = get_current_time();
    if (last_time < 0){
        log(error) << "Got Invalid Timestamp : " << last_time;
        return 0;
    }

    while (1){
        fd_set fs;
        FD_ZERO(&fs);
        FD_SET(lssdp.sock, &fs);
        struct timeval tv;
        tv.tv_usec = 500 * 1000; // 500ms

        int ret = select(lssdp.sock + 1, &fs, NULL, NULL, &tv);
        if (ret < 0){
            log(error) << "select error, ret = " << ret;
            break;
        }

        if (ret > 0){
            lssdp_socket_read(&lssdp);
        }
        long long current_time = get_current_time();
        if (current_time < 0){
            log(error) << "Got Invalid Timestamp : " << last_time;
            break;
        }
        // doing task per 5 seconds
        if (current_time - last_time >= 3500) {
            lssdp_network_interface_update(&lssdp);
            lssdp_send_msearch(&lssdp);
            
            lssdp_send_notify(&lssdp);
            lssdp_neighbor_check_timeout(&lssdp);

            last_time = current_time;
        }
    }

    return 0;
}

// void *restserver_handler(void *data)
// {
//         // l_interrupt=0;      
//         // rest_handler(data);
// }

// /**
//  * @brief ssdp client hanler 
//  * @details 해당 함수는 엣지서버 upnp 디바이스 정보를 ssdp 프로토콜로 CMM에게 전달하는 핸들러이다.
//  * @param *data void *형
//  * @author Kim Do Young
//  * @date 2021-08-18
//  */
// void *ssdp_handler(void *data)
// {
//         char rcvdbuff[1000];
//         int sockfd = *((int *)data);
//         int ret = 2, len;
//         struct sockaddr_in manager_addr;

//         manager_addr.sin_family = AF_INET;
//         manager_addr.sin_addr.s_addr = inet_addr("10.0.6.107");
//         manager_addr.sin_port = htons(DEFAULT_SSDP_PORT);
//         len = sizeof(struct sockaddr_in);

//         while (1){
//                 printf("buff:\n%s\n", ssdp_buff);
//                 ret = sendto(sockfd, ssdp_buff, strlen(ssdp_buff), 0, (struct sockaddr *)&manager_addr, len);
//                 if (ret < 0){
//                         log(warning) << "error in SENDTO() function in ssdp";
//                         return 0;
//                 }

//                 // Receiving Text from server
//                 printf("\n\n waiting to recv:\n");
//                 memset(rcvdbuff, 0, sizeof(rcvdbuff));
//                 ret = recvfrom(sockfd, rcvdbuff, sizeof(rcvdbuff), 0, (struct sockaddr *)&manager_addr, &len);
//                 if (ret < 0){
//                         log(warning) << "Error in Receiving in ssdp";
//                         return 0;
//                 }
//                 rcvdbuff[ret-1] = '\0';
//                 log(info) << "RECV MESSAGE FROM SERVER : " << rcvdbuff;

//                 // delay
//                 sleep(3*1000);
//         }
//         return 0;
// }


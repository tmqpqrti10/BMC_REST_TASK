#include "rhandler.hpp"
#include <cstring>
#include <json-c/json.h>
#define QSIZE 35000

#define ACCESS_RESPONSE_HEADER "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Headers: Origin, Accept, Content-Type, X-Requested-With\r\nAccess-Control-Allow-Methods: PUT, GET, POST, DELETE, OPTIONS\r\nAccess-Control-Allow-Origin: *\r\nContent-Type: application/json\r\n"
#define ACCESS_HEADER "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Headers: Origin, Accept, Content-Type, X-Requested-With\r\nAccess-Control-Allow-Methods: PUT, GET, POST, DELETE, OPTIONS\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: 0\r\nContent-Type: application/json\r\n\r\n"
static const uint8_t *s_http_port = "8000";
uint8_t upload_req, update_req = 0;
uint8_t sdr_en, fru_en, sel_en, ipmi_en, network_en, ntp_en, snmp_en, ssh_en, kvm_en, auth_en, web_en = 0;
static struct mg_serve_http_opts s_http_server_opts;
uint8_t fw_name[100] = {0};
uint8_t already_upload = 0;

// static const char *s_ssl_cert = "/etc/ssl/server.pem";
// static const char *s_ssl_key = "/etc/ssl/server.key";

struct file_writer_data
{
    FILE *fp;
    size_t bytes_written;
};

int file_exist (char *filename)
{
        struct stat buffer;
        return (stat (filename, &buffer) == 0);
}

/**
 * @brief str에서 ch 모두 제거
 * @date 21.05.14
 * @author doyoung
 */
void eliminate(char *str, char ch)
{
    for (; *str != '\0'; str++)
    {
        if (*str == ch)
        {
            strcpy(str, str + 1);
            str--;
        }
    }
}

/**
 * @brief input을 받아 token부분을 parsing하여 output으로 저장
 * @date 21.05.14
 * @author doyoung
 */
static int parse_query(char *input, char *token, char *output)
{
        char *query = strdup (input),
             *tokens = query,
             *p = query;

        while ((p = strsep (&tokens, "&\n")))
        {
                char *var = strtok (p, "="),
                     *val = NULL;
                if (var && (val = strtok (NULL, "=")))
                {
                        if(!strcmp(var, token))
                        {
                                strcpy(output, val);
                                free (query);
                                return 1;
                        }
                }
        }
        free (query);
        return 0;
}

static void send_result_message(struct mg_connection *nc, uint8_t flag){
	uint8_t header[1000], response[50];

	if(flag == 0)
	{
		memset(response, 0, sizeof(response));
		sprintf(response, "{\"CODE\":\"400\",\"MESSAGE\":\"FAIL\"}");
		sprintf(header,  "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, strlen(response)+2);
		mg_send_response_line(nc, 200, header);
                mg_send(nc, response, strlen(response)+2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if(flag == 2){
		memset(response, 0, sizeof(response));
		sprintf(response, "{\"CODE\":\"400\",\"MESSAGE\":\"FAIL\", \"DEBUG_CODE\":\"-1\"}");
		sprintf(header,  "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, strlen(response)+2);
		mg_send_response_line(nc, 200, header);
                mg_send(nc, response, strlen(response)+2);
                nc->flags |= MG_F_SEND_AND_CLOSE;	
	}
	else if(flag == 3){
		memset(response, 0, sizeof(response));
		sprintf(response, "{\"CODE\":\"400\",\"MESSAGE\":\"FAIL\", \"DEBUG_CODE\":\"-2\"}");
		sprintf(header,  "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, strlen(response)+2);
		mg_send_response_line(nc, 200, header);
                mg_send(nc, response, strlen(response)+2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if(flag == 4){
		memset(response, 0, sizeof(response));
		sprintf(response, "{\"CODE\":\"400\",\"MESSAGE\":\"FAIL\", \"DEBUG_CODE\":\"-3\"}");
		sprintf(header,  "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, strlen(response)+2);
		mg_send_response_line(nc, 200, header);
                mg_send(nc, response, strlen(response)+2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if(flag == 1){
		memset(response, 0, sizeof(response));
                sprintf(response, "{\"CODE\":\"200\",\"MESSAGE\":\"SUCCESS\"}");
                sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, strlen(response)+2);
                mg_send_response_line(nc, 200, header);
                mg_send(nc, response, strlen(response)+2);
        	nc->flags |= MG_F_SEND_AND_CLOSE;
	}
}

static int umount(){
	uint8_t cmds[100] = {0};
	uint8_t response[10000], result[10000] = {0};
	sprintf(cmds, "mount | grep /nfs_check");
	FILE *p = popen(cmds, "r");
	if(p != NULL){
		while(fgets(result, sizeof(result), p) != NULL)
			strncat(response, result, strlen(result));
		pclose(p);
	}

	if(strlen(response) > 0){
		// umount의 표준 에러 를 출력으로 redirection 후 띄우지않음.
		sprintf(cmds, "umount -l /nfs_check > /dev/null 2>&1");
		int rets = system(cmds);
		if (rets == 0)
			return 0;
		else
			return -1;	
	}
	else
		return 0;
}

/**
 * @brief domain + /power handle
 * @author doyoung
 */
static void handle_power_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_power_call"<<endl;
	
	if(!ev_data){
		cout << "ev_data is not exists in power handler" << endl;
		return;
	}
	  
	struct http_message *hm = (struct http_message *) ev_data;
	
	uint8_t data[100] = {0};
	uint8_t methods[10] = {0};
	uint8_t m_len, b_len = 0;
	json::value myobj;
	
	b_len = hm->body.len;
	m_len = hm->method.len;
	
	if(m_len != 0){
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(b_len != 0){
		strncpy(data, hm->body.p, b_len);
		data[b_len] = 0;
	}
	
	if(strcmp(methods, "GET") == 0){
		cout<<"\tDEBUG GET in handle power"<<endl;
	
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len=0;
		memset(res_msg, 0, sizeof(res_msg));

		get_power_status(res_msg, &res_len);
		strncpy(response, res_msg, res_len);
		strcat(response,"\r\n\r\n");
		
		uint8_t header[1000];
		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len+2);
		mg_send_response_line(nc, 200, header);
		mg_send(nc, response, res_len + 2);
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "POST") == 0) || (strcmp(methods, "PUT") == 0) || (strcmp(methods, "OPTIONS") == 0)){
		if(b_len == 0 && (strcmp(methods, "OPTIONS") == 0))
		{
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(b_len != 0 && ((strcmp(methods, "PUT") == 0) || (strcmp(methods, "POST") == 0))){
			uint8_t response[50];
			uint8_t header[1000];
			int c_status, res_len = 0;
            string sdata((char *)data);
			cout << "sdata in power_call : " << sdata << endl;
			string s_status;
			myobj = json::value::parse(sdata);

			if (myobj.at("STATUS").is_integer())
				c_status = myobj.at("STATUS").as_integer();
			else if (myobj.at("STATUS").is_string()){
				s_status = myobj.at("STATUS").as_string();
				c_status = stoi(s_status);
			}

			set_power_status(c_status, response, &res_len);
			memset(response, 0, sizeof(response));	
			sprintf(response, "{\"CODE\":\"200\",\"MESSAGE\":\"SUCCESS\"}");
			sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, strlen(response)+2);
			mg_send_response_line(nc, 200, header);
			mg_send(nc, response, strlen(response)+2);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
	}
}

/**
 * @brief domain + /sysinfo
 * @date 21.05.13
 * @author doyoung
 */
 static void handle_sysinfo_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_sysinfo_call"<<endl;

	if (!ev_data){
		cout << "ev_data is not exists in sysinfo" << endl;
		return;
	}	

	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t m_len = 0;

	m_len = hm->method.len;

	if(m_len != 0){
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		memset(res_msg, 0, sizeof(res_msg));
	
		int res_len = 0;
		get_sys_param_select(0, res_msg, &res_len);
		
		strncpy(response, res_msg, res_len);
		strcat(response,"\r\n\r\n");
		
		uint8_t header[1000];
		
		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
		mg_send_response_line(nc, 200, header);
		mg_send(nc, response, res_len + 2);
		nc->flags |= MG_F_SEND_AND_CLOSE;	
	}
}

/**
 * @brief domain + /fru handle
 * @bug impi_handle_rest 구현후 확인필요
 * @author doyoung
 */
static void handle_fru_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_fru_call"<<endl;

	if (!ev_data){
		cout << "ev_data is not exists in fru handler" << endl;
		return;
	}

	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t m_len = 0;
	uint8_t b_len = 0;
	uint8_t data[1000], response[QSIZE], res_msg[QSIZE];
	
	m_len = hm->method.len;
	b_len = hm->body.len;

	if(m_len != 0){
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}

	if(strcmp(methods, "GET") == 0)
	{
		int res_len = 0;

		get_fru_param_select(0, res_msg, &res_len);
		
		strncpy(response, res_msg, res_len);

		uint8_t header[1000];
		strcat(response,"\r\n\r\n");
		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len+2);
		mg_send_response_line(nc, 200, header);
		mg_send(nc, response, res_len + 2);
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "PUT") == 0) || (strcmp(methods, "OPTIONS") == 0))
	{
		if(strcmp(methods, "OPTIONS") == 0){
			if(b_len == 0 && (strcmp(methods, "OPTIONS") == 0))
	                {	
                        	mg_send_response_line(nc, 200, ACCESS_HEADER);
                	        mg_send(nc, "{}",2);
        	                mg_send_http_chunk(nc,"",0);
	                        nc->flags |= MG_F_SEND_AND_CLOSE;
                	}
		}
		if(strcmp(methods, "PUT") == 0){
			if(b_len != 0){
				strcpy(data, hm->body.p);
				
				json::value myobj, board_obj, product_obj, chassis_obj;
				string mfg, mfg_date, part_num, serial, product;
				string p_mfg, p_name, p_part_num, p_serial, p_version; 
				string c_part_num, c_serial, c_type;
				uint8_t years[5], month[3], days[3], hours[3], minutes[3], secs[3];
				uint8_t id;

				refine_data(data);
				string sdata((char *)data); 
				cout << "sdata in fru_call : " << sdata << endl;
            
				myobj = json::value::parse(sdata);
				id = myobj.at("ID").as_integer();
				
				board_obj = myobj.at("BOARD");
				mfg = board_obj.at("MFG").as_string();
				mfg_date = board_obj.at("MFG_DATE").as_string();
				part_num = board_obj.at("PART_NUM").as_string();
				serial = board_obj.at("SERIAL").as_string();
				product = board_obj.at("PRODUCT").as_string();
				
				product_obj = myobj.at("PRODUCT");
				p_mfg = product_obj.at("MFG").as_string();
				p_name = product_obj.at("NAME").as_string();
				p_part_num = product_obj.at("PART_NUM").as_string();
				p_serial = product_obj.at("SERIAL").as_string();
				p_version = product_obj.at("VERSION").as_string();
				
				chassis_obj = myobj.at("CHASSIS");
				c_part_num = chassis_obj.at("PART_NUM").as_string();
				c_serial = chassis_obj.at("SERIAL").as_string();
				c_type = chassis_obj.at("TYPE").as_string();
				
				struct tm temp;
				memset(&temp, 0, sizeof(struct tm));
				if (strptime(mfg_date.c_str(), "%F %T", &temp) == NULL){
					fprintf(stderr, "\t\tWarning : Date Format is invalid\n");
				}
				temp.tm_year += 1900;
				sprintf(years, "%d", temp.tm_year);
				sprintf(month, "%d", temp.tm_mon + 1);
				sprintf(days, "%d", temp.tm_mday);
				sprintf(hours, "%d", temp.tm_hour);
				sprintf(minutes, "%d", temp.tm_min);
				sprintf(secs, "%d", temp.tm_sec);
				
				printf("%s %s %s / %s %s %s\n", years, month,days,hours,minutes,secs);
				set_fru_header(id, 4, 2, 3);

				set_fru_board(id, years,month,days,hours,minutes,secs,mfg.c_str(),product.c_str(),serial.c_str(), part_num.c_str());

				set_fru_product(id, p_name.c_str(), p_mfg.c_str(), p_version.c_str(), p_serial.c_str(), p_part_num.c_str());

				uint8_t loop = 0;
				for(loop = 0 ; loop < 30 ; loop++)
				{
					if(strncmp(c_type.c_str(), chassis_type_desc[loop], strlen(c_type.c_str())) == 0)	
						break;
				}
				set_fru_chassis(id, loop, c_serial.c_str(), c_part_num.c_str());
				send_result_message(nc, 1);
			}
		}
	}
}

/**
 * @brief domain + /sensor handle
 * @bug impi_handle_rest 구현후 확인필요.
 * @date 21.05.14
 * @author doyoung
 */
static void handle_sensor_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_sensor_call"<<endl;
	
	if (!ev_data){
		cout << "ev_data is not exists in sensor handler" << endl;
		return;
	}
	cout << "ev_data ??" << endl;
	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t data[1000] = {0};
	uint8_t m_len = 0;
	uint8_t b_len = 0;

	m_len = hm->method.len;
	b_len = hm->body.len;
	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}

	if(strcmp(methods, "GET") == 0){
		
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;
		
		get_sensor_param_select(0, res_msg, &res_len);
		memset(response, 0, sizeof(response));
		strncpy(response, res_msg, res_len);
		strcat(response, "\r\n\r\n");

		uint8_t header[1000];
			sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len+2);
				mg_send_response_line(nc, 200, header);
				mg_send(nc, response, res_len + 2);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		
	}
	else if((strcmp(methods, "OPTIONS") == 0) || (strcmp(methods, "PUT") == 0)){
		if(b_len == 0 && strcmp(methods, "OPTIONS") == 0){
			mg_send_response_line(nc, 200, ACCESS_HEADER);
						mg_send(nc, "{}",2);
						mg_send_http_chunk(nc,"",0);
						nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(b_len != 0 && (strcmp(methods, "PUT") == 0)){
			
			string s_name, thresh;
			int num = 0;
			json::value temp;
			float unr, uc, unc, lnc, lc, lnr; 
			
			json::value myobj;
			strncpy(data, hm->body.p, b_len);
			string sdata((char *)data);
			cout << "sdata in sensor_call : " << sdata << endl;
            
			myobj = json::value::parse(sdata);
			
			s_name = myobj.at("SENSOR").as_string();
			unr = stof(myobj.at("UNR").as_string());
			uc = stof(myobj.at("UC").as_string());
			unc = stof(myobj.at("UNC").as_string());
			lnc = stof(myobj.at("LNC").as_string());
			lc = stof(myobj.at("LC").as_string());
			lnr = stof(myobj.at("LNR").as_string());
			
			uint8_t response[QSIZE], res_msg[QSIZE];
			int res_len = 0;
			get_sensor_param_select(0, res_msg, &res_len);
			strncpy(response, res_msg, res_len);
			
			json::value sensor_list, sensor;
			string sensor_name;
			string srsp((char *)response);
			
			myobj = json::value::parse(srsp);
			sensor_list = myobj.at("SENSOR_INFO").at("SENSOR");

			for (auto sensor : sensor_list.as_array()){
				sensor_name = sensor.at("NAME").as_string();
				if (sensor_name == s_name){
					num = stoi(sensor.at("NUMBER").as_string());
					break;
				}
			}
			
			set_sensor_threshold(uc, unc, unr, lc, lnc, lnr, num);
			send_result_message(nc, 1);
		}
	}
}

/**
 * @brief domain + /eventlog handle
 * @bug impi_handle_rest 구현후 확인필요.  
 * @date 21.05.14
 * @author doyoung
 */
static void handle_eventlog_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_eventlog_call"<<endl;

	if (!ev_data){
		cout << "ev_data is not exists in eventlog handler" << endl;
		return;
	}

	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t m_len = 0;

	m_len = hm->method.len;
	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;
		memset(response, 0, sizeof(response));
		get_event_param_select(0, res_msg, &res_len);
		strncpy(response, res_msg, res_len);
		strcat(response, "\r\n\r\n");
		uint8_t header[1000];

		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
                mg_send_response_line(nc, 200, header);
                mg_send(nc, response, res_len + 2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}

}

/**
 * @brief domain + /ddns handle
 * @date 21.05.14
 * @author doyoung
 */
static void handle_ddns_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_ddns_call"<<endl;

	if (!ev_data){
		cout << "ev_data is not exists in ddns handler" << endl;
		return;
	}

	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t data[1000] = {0};
	uint8_t m_len = 0;
	uint8_t b_len = 0;
	uint8_t q_len = 0;
	int rets = 0;

	m_len = hm->method.len;
	b_len = hm->body.len;
	q_len = hm->query_string.len;

	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(q_len != 0)
	{
		strncpy(data, hm->query_string.p, q_len);
		data[q_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;
		get_dns_info(res_msg, &res_len);
		strncpy(response, res_msg, res_len);
		strcat(response, "\r\n\r\n");
		uint8_t header[1000];
		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
                mg_send_response_line(nc, 200, header);
                mg_send(nc, response, res_len + 2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "POST") == 0) || (strcmp(methods, "OPTIONS") == 0)){
		if(q_len == 0 && (strcmp(methods, "OPTIONS") == 0))
		{
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(q_len != 0 && ((strcmp(methods, "POST") == 0))){

			json::value myobj, orgobj, orggen, orgipv4, orgipv6, orgval;

			uint8_t cmds[300], response[QSIZE], res_msg[QSIZE];
			uint8_t header[1000];
			int res_len = 0;
			
			memset(response, 0, sizeof(response));
			get_dns_info(res_msg, &res_len);
			strncpy(response, res_msg, res_len);

			string org_host_name, org_domain_name, org_ipv4_preferred, org_ipv4_alter, org_ipv6_preferred;
			string srsp((char *)response);
			cout << "sdata in dns_call : " << srsp << endl;
            orgobj = json::value::parse(srsp);
	
			orgval = orgobj.at("DNS_INFO");
			orggen = orgval.at("GENERIC");
			
			org_host_name = orggen.at("HOST_NAME").as_string();
			org_domain_name = orggen.at("DOMAIN_NAME").as_string();
			
			orgipv4 = orgval.at("IPV4");
			org_ipv4_preferred = orgipv4.at("IPV4_PREFERRED").as_string();
			org_ipv4_alter= orgipv4.at("IPV4_ALTERNATE").as_string();
			
			orgipv6 = orgval.at("IPV6");
			org_ipv6_preferred = orgipv6.at("IPV6_PREFERRED").as_string();
			
			uint8_t new_host_name[100], new_domain_name[300], new_ipv4_preferred[30], new_ipv4_alter[30], new_ipv6_preferred[100];
			
			parse_query(data, "HOST_NAME", new_host_name);
			parse_query(data, "DOMAIN_NAME", new_domain_name);
			parse_query(data, "IPV4_PREFERRED", new_ipv4_preferred);
			parse_query(data, "IPV4_ALTERNATE", new_ipv4_alter);
			parse_query(data, "IPV6_PREFERRED", new_ipv6_preferred);

			if((strcmp(new_host_name, "-") != 0) && (strcmp(org_host_name.c_str(), new_host_name) != 0))
			{                                                                                                                        
				int rets = 0;
				rets = set_dns_hostname_info(new_host_name);
				if(rets == 0){	
					send_result_message(nc, 1);
				}
				else{
					send_result_message(nc, 0);
				}
			}

			if((strcmp(new_domain_name, "-") != 0) && (strcmp(org_domain_name.c_str(), new_domain_name) != 0))
			{
				rets = set_dns_domain_info(new_domain_name);
				if(rets == 0){	
					send_result_message(nc, 1);
				}
				else{
					send_result_message(nc, 0);
				}
			}

			if((strcmp(new_ipv4_preferred, "-") != 0) && (strcmp(org_ipv4_preferred.c_str(), new_ipv4_preferred) != 0))
			{
				rets = set_dns_ip_prefer(new_ipv4_preferred);

				if(rets == 0){	
					send_result_message(nc, 1);
				}
				else{
					send_result_message(nc, 0);
				}
			}

			if((strcmp(new_ipv4_alter, "-") != 0) && (strcmp(org_ipv4_alter.c_str(), new_ipv4_alter) != 0))
			{
				rets = set_dns_ip_alter(new_ipv4_alter);

				if(rets == 0){	
					send_result_message(nc, 1);
				}
				else{
					send_result_message(nc, 0);
				}
			}

			if((strcmp(new_ipv6_preferred, "-") != 0) && (strcmp(org_ipv6_preferred.c_str(), new_ipv6_preferred) != 0))
			{
				rets = set_dns_ipv6_prefer(new_ipv6_preferred);
				
				if(rets == 0){	
					send_result_message(nc, 1);
				}
				else{
					send_result_message(nc, 0);
				}
			}
		}
	}
}


/**
 * @brief domain + /network handle
 * @bug impi_handle_rest 구현후 확인필요.  
 * @date 21.05.14
 * @author doyoung
 */
static void handle_network_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_network_call"<<endl;

	if (!ev_data){
		cout << "ev_data is not exists in network handler" << endl;
		return;
	}

	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10], cmds[300] = {0};
	uint8_t data[QSIZE],temp_data[QSIZE] = {0};
	uint8_t m_len = 0;
	uint8_t q_len = 0;
	int rets = 0;

	m_len = hm->method.len;
	q_len = hm->query_string.len;

	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(q_len != 0)
	{
		strcpy(temp_data, hm->query_string.p);

		uint8_t *r_index = strtok(temp_data, "\n");
		uint8_t *t_index = strtok(r_index, " ");
		strcpy(data, t_index);
		//data[q_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;

		get_lan_param_select(0, res_msg, &res_len);
		strncpy(response, res_msg, res_len);
		strcat(response, "\r\n\r\n");
		
		uint8_t header[1000];
		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
                mg_send_response_line(nc, 200, header);
                mg_send(nc, response, res_len + 2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "POST") == 0) || (strcmp(methods, "OPTIONS") == 0)){
		if(q_len == 0 && (strcmp(methods, "OPTIONS") == 0))
		{
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(q_len != 0 && ((strcmp(methods, "POST") == 0))){
			int changed=0;

			json::value myobj, ninfo, laninf, generic, ipv4, ipv6, vlan;

			uint8_t response[QSIZE], res_msg[QSIZE];
			uint8_t header[1000];
			int res_len = 0;
			
			get_lan_param_select(0, res_msg, &res_len);
			strncpy(response, res_msg, res_len + 1);

			string org_network_priority, org_lan_setting_enable, org_lan_interface, org_mac_address, org_ipv4_preferred, 
				org_ipv4_dhcp_enable, org_ipv4_address, org_ipv4_netmask, org_ipv4_gateway,
				org_ipv6_subnet_prefix_length, org_ipv6_address, org_ipv6_enable, org_ipv6_dhcp_enable, org_ipv6_gateway,
				org_vlan_settings_enable, org_vlan_id, org_vlan_priority;
			
			uint8_t new_network_priority[10], new_lan_setting_enable[10], new_lan_interface[100], new_mac_address[100], new_ipv4_preferred[10], 
				new_ipv4_dhcp_enable[10], new_ipv4_address[100], new_ipv4_netmask[100], new_ipv4_gateway[100],
				new_ipv6_subnet_prefix_length[100], new_ipv6_address[100], new_ipv6_enable[10], new_ipv6_dhcp_enable[10], new_ipv6_gateway[100],
				new_vlan_settings_enable[100], new_vlan_id[100], new_vlan_priority[100];

			string srsp((char *)response);
			cout << "sdata in network_call : " << srsp << endl;
            myobj = json::value::parse(srsp);
			
			org_network_priority = myobj.at("NETWORK_PRIORITY").as_string();
			
			ninfo = myobj.at("NETWORK_INFO");

			json::value tmpobj1, tmpobj2;
			string tmpprior1, tmpprior2;

			tmpobj1 = ninfo.at(0);
			tmpobj2 = ninfo.at(1);
			tmpprior1 = tmpobj1.at("LAN_INTERFACE").as_string();
			tmpprior2 = tmpobj2.at("LAN_INTERFACE").as_string();
			
			if(tmpprior1 == org_network_priority)
			{
				ninfo = tmpobj1;
			}
			else if(tmpprior2 == org_network_priority)
			{
				ninfo = tmpobj2;
			}
			else
			{
				printf("network set error\n");
			}
			
			org_lan_interface = ninfo.at("LAN_INTERFACE").as_string();
			
			generic = ninfo.at("GENERIC");
			org_lan_setting_enable = generic.at("LAN_SETTING_ENABLE").as_string();
			org_mac_address = generic.at("MAC_ADDRESS").as_string();

			ipv4 = ninfo.at("IPV4");
			org_ipv4_preferred = ipv4.at("IPV4_PREFERRED").as_string();
			org_ipv4_gateway = ipv4.at("IPV4_GATEWAY").as_string();
			org_ipv4_netmask = ipv4.at("IPV4_NETMASK").as_string();
			org_ipv4_address = ipv4.at("IPV4_ADDRESS").as_string();
			org_ipv4_dhcp_enable = ipv4.at("IPV4_DHCP_ENABLE").as_string();
			
			ipv6 = ninfo.at("IPV6");
			org_ipv6_subnet_prefix_length = ipv6.at("IPV6_SUBNET_PREFIX_LENGTH").as_string();
			org_ipv6_address = ipv6.at("IPV6_ADDRESS").as_string();
			org_ipv6_enable = ipv6.at("IPV6_ENABLE").as_string();
			org_ipv6_dhcp_enable = ipv6.at("IPV6_DHCP_ENABLE").as_string();
			org_ipv6_gateway = ipv6.at("IPV6_GATEWAY").as_string();

			vlan = ninfo.at("VLAN");
		    org_vlan_settings_enable = vlan.at("VLAN_SETTINGS_ENABLE").as_string();
		    org_vlan_id = vlan.at("VLAN_ID").as_string();
		    org_vlan_priority = vlan.at("VLAN_PRIORITY").as_string();
		    
			parse_query(data, "NETWORK_PRIORITY", new_network_priority);
			parse_query(data, "LAN_INTERFACE", new_lan_interface);
			parse_query(data, "LAN_SETTING_ENABLE", new_lan_setting_enable);
			parse_query(data, "MAC_ADDRESS", new_mac_address);
			parse_query(data, "IPV4_PREFERRED", new_ipv4_preferred);
			parse_query(data, "IPV4_GATEWAY", new_ipv4_gateway);
			parse_query(data, "IPV4_NETMASK", new_ipv4_netmask);
			parse_query(data, "IPV4_ADDRESS", new_ipv4_address);
			parse_query(data, "IPV4_DHCP_ENABLE", new_ipv4_dhcp_enable);
			parse_query(data, "IPV6_SUBNET_PREFIX_LENGTH", new_ipv6_subnet_prefix_length);
			parse_query(data, "IPV6_ADDRESS", new_ipv6_address);
			parse_query(data, "IPV6_ENABLE", new_ipv6_enable);
			parse_query(data, "IPV6_DHCP_ENABLE", new_ipv6_dhcp_enable);
			parse_query(data, "IPV6_GATEWAY", new_ipv6_gateway);
			parse_query(data, "VLAN_SETTINGS_ENABLE", new_vlan_settings_enable);
			parse_query(data, "VLAN_ID", new_vlan_id);
			parse_query(data, "VLAN_PRIORITY", new_vlan_priority);
			
			printf("\t\t\tdy : checkpoint1 : parse query\n");
			unsigned char new_intf = 0;
			if(strcmp(new_lan_interface,"eth0") == 0)
				new_intf = 0;
			else if(strcmp(new_lan_interface, "eth1") == 0)
				new_intf = 1;
			if(strcmp(org_mac_address.c_str(), new_mac_address))
			{
				rets = set_network_mac(new_intf, new_mac_address);
				changed=1;
				if(rets == 1)
					send_result_message(nc, 1);
				else
					send_result_message(nc, 0);
			}

			if (strcmp(org_ipv4_dhcp_enable.c_str(), new_ipv4_dhcp_enable))
				set_network_ipv4_dhcp(new_intf, new_ipv4_dhcp_enable);
			
			if(!strcmp(new_ipv4_dhcp_enable, "0"))
			{
				if(strcmp(org_ipv4_address.c_str(), new_ipv4_address) ||
					strcmp(org_ipv4_netmask.c_str(), new_ipv4_netmask) ||
					strcmp(org_ipv4_gateway.c_str(), new_ipv4_gateway))
				{
					rets += set_network_ipv4_ip(new_intf, new_ipv4_address, new_ipv4_netmask);
					rets += set_network_gateway(new_intf, new_ipv4_gateway);
					changed=1;

					if(rets == 0)
						send_result_message(nc, 1);
					else
						send_result_message(nc, 0);
				}
			}

			printf("\t\t\tdy : checkpoint2 : ipv4 set\n");

			if(strcmp(org_ipv6_enable.c_str(), new_ipv6_enable))
			{
				// ipv6 enable 변경 되었을 때만
				set_network_ipv6_enable(new_intf, new_ipv6_enable);
					
				if(!strcmp(new_ipv6_enable, "1"))
				{
					// dhcp 변경 되었을 때만
					if (strcmp(org_ipv6_dhcp_enable.c_str(), new_ipv6_dhcp_enable))
						set_network_ipv6_dhcp(new_intf, new_ipv6_dhcp_enable);
					
					if(!strcmp(new_ipv6_dhcp_enable, "0"))
					{
						set_network_ipv6_ip(new_intf, new_ipv6_address);
						set_network_ipv6_prefix(new_intf, new_ipv6_subnet_prefix_length);
						set_network_ipv6_gateway(new_intf, new_ipv6_gateway);
					}	
				}
			}


			printf("\t\t\tdy : checkpoint3 : ipv6 set\n");


			if (strcmp(org_vlan_settings_enable.c_str(), new_vlan_settings_enable))
				set_network_vlan_enable(new_intf, new_vlan_settings_enable);

			if(!strcmp(new_vlan_settings_enable, "1"))
			{
				if(strcmp(org_vlan_id.c_str(), new_vlan_id)){
					set_network_vlan_id(new_intf, new_vlan_id);
					set_network_vlan_priority(new_intf, new_vlan_priority);
				}else if(strcmp(org_vlan_priority.c_str(), new_vlan_priority))
					set_network_vlan_priority(new_intf, new_vlan_priority);
			}
			
			
			printf("\t\t\tdy : checkpoint4 : vlan set\n");


			char priority;
			if(!strcmp(new_network_priority, "eth0"))
			{
				priority = '1';
			}
			else if(!strcmp(new_network_priority, "eth1"))
			{
				priority = '8';
			}
			else
			{
				priority = '8';
			}
			
			if(strcmp(new_network_priority, org_network_priority.c_str()))
				set_network_ipv6_priority(priority);
			
			printf("\t\t\tdy : checkpoint5 : priority set\n");


			send_result_message(nc, 1);
		}
	}
}

/**
 * @brief domain + /ntp handle
 * @bug impi_handle_rest 구현후 확인필요.  
 * @date 21.05.14
 * @author doyoung
 */
static void handle_ntp_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_ntp_call"<<endl;

	if (!ev_data){
		cout << "ev_data is not exists in ntp handler" << endl;
		return;
	}

	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t cmds[100] = {0};
	uint8_t cmds2[100] = {0};
	uint8_t data[1000] = {0};
	uint8_t m_len = 0;
	uint8_t q_len = 0;

	m_len = hm->method.len;
	q_len = hm->query_string.len;

	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(q_len != 0)
	{
		strncpy(data, hm->query_string.p, q_len);
		data[q_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;
		get_ntp_info(res_msg, &res_len);
		memset(response, 0, sizeof(response));
		strncpy(response, res_msg, res_len);
		strcat(response, "\r\n\r\n");
		uint8_t header[1000];

		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
                mg_send_response_line(nc, 200, header);
                mg_send(nc, response, res_len + 2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "POST") == 0) || (strcmp(methods, "OPTIONS") == 0)){
		if(q_len == 0 && (strcmp(methods, "OPTIONS") == 0))
		{
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(q_len != 0 && ((strcmp(methods, "POST") == 0))){
			uint8_t res_msg[20], response[QSIZE];
			int res_len = 0;
			uint8_t header[1000];
			uint8_t auto_sync[2], ntp_server[100], year[100], month[100], day[100], hour[100], min[100], sec[100];
			string sdata((char *)data);
			cout << "sdata : " << sdata << endl;
			parse_query(data, "AUTO_SYNC", auto_sync);

			if(!strcmp(auto_sync, "1"))
			{
				parse_query(data, "NTP_SERVER", ntp_server);

				set_ntp_auto(ntp_server, res_msg, &res_len);
				if(res_len != 0){
					if(strncmp(res_msg, "success", res_len) == 0)
						send_result_message(nc, 1);
					else
						send_result_message(nc, 0);
				}
			} 
			else
			{
				parse_query(data, "YEAR", year);
				parse_query(data, "MONTH", month);
				parse_query(data, "DAY", day);
				parse_query(data, "HOUR", hour);
				parse_query(data, "MIN", min);
				parse_query(data, "SEC", sec);

				sprintf(cmds, "killall -9 ntpd");
				system(cmds);
				sprintf(cmds, "mv /etc/ntp.conf /etc/ntp.conf.bak");
				system(cmds);
				sprintf(cmds2, "date --set=\"%s-%s-%s %s:%s:%s\"", year, month, day, hour, min, sec);
				int rets = 0;
				rets = system(cmds2);
				if(rets == 0){	
					send_result_message(nc, 1);
				}
				else{
					send_result_message(nc, 0);
				}
			}
		}
	}
}

/**
 * @brief domain + /smtp handle
 * @bug impi_handle_rest 구현후 확인필요.  
 * @date 21.05.14
 * @author doyoung
 */
static void handle_smtp_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_smtp_call"<<endl;

	if (!ev_data){
		cout << "ev_data is not exists in smtp handler" << endl;
		return;
	}

	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t data[1000] = {0};
	uint8_t m_len = 0;
	uint8_t q_len = 0;

	m_len = hm->method.len;
	q_len = hm->query_string.len;

	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
	}
	if(q_len != 0)
	{
		strncpy(data, hm->query_string.p, q_len);
		data[q_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;
		get_smtp_configuration(res_msg, &res_len);
		memset(response, 0, sizeof(response));
		strncpy(response, res_msg, res_len);
		strcat(response, "\r\n\r\n");
		uint8_t header[1000];

		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
                mg_send_response_line(nc, 200, header);
                mg_send(nc, response, res_len + 2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "POST") == 0) || (strcmp(methods, "OPTIONS") == 0)){
		if(q_len == 0 && (strcmp(methods, "OPTIONS") == 0))
		{
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(q_len != 0 && ((strcmp(methods, "POST") == 0))){
			uint8_t sender_address[100] = {0, }, machine_name[100] = {0, }, primary_server_address[100] = {0, }, primary_user_name[100] = {0, }, primary_user_password[100] = {0, }, secondary_server_address[100] = {0, }, secondary_user_name[100] = {0, }, secondary_user_password[100] = {0, };
			
			parse_query(data, "SENDER_ADDRESS", sender_address);
			parse_query(data, "MACHINE_NAME", machine_name);
			parse_query(data, "PRIMARY_SERVER_ADDRESS", primary_server_address);
			parse_query(data, "PRIMARY_USER_NAME", primary_user_name);
			parse_query(data, "PRIMARY_USER_PASSWORD", primary_user_password);
			parse_query(data, "SECONDARY_SERVER_ADDRESS", secondary_server_address);
			parse_query(data, "SECONDARY_USER_NAME", secondary_user_name);
			parse_query(data, "SECONDARY_USER_PASSWORD", secondary_user_password);

			set_smtp_sender(sender_address, machine_name);
			set_smtp_primary(primary_server_address, primary_user_name, primary_user_password);
			set_smtp_secondary(secondary_server_address, secondary_user_name, secondary_user_password);
			
			send_result_message(nc, 1);
		}
	}
}

/**
 * @brief domain + /ssl handle
 * @date 21.05.14
 * @author doyoung
 */
static void handle_ssl_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_ssl_call"<<endl;

	if (!ev_data){
		cout << "ev_data is not exists in ssl handler" << endl;
		return;
	}

	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t cmds[100] = {0};
	uint8_t data[2000] = {0};
	uint8_t m_len = 0;
	uint8_t q_len = 0;

	m_len = hm->method.len;
	q_len = hm->query_string.len;

	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(q_len != 0)
	{
		strncpy(data, hm->query_string.p, q_len);
		data[q_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;

		get_ssl_info(res_msg, &res_len);
		strncpy(response, res_msg, res_len);
		strcat(response, "\r\n\r\n");
		uint8_t header[1000];
		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
                mg_send_response_line(nc, 200, header);
                mg_send(nc, response, res_len + 2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "POST") == 0) || (strcmp(methods, "OPTIONS") == 0)){
		if(q_len == 0 && (strcmp(methods, "OPTIONS") == 0))
		{
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(q_len != 0 && ((strcmp(methods, "POST") == 0))){
			uint8_t result[QSIZE], response[QSIZE];
			uint8_t header[1000];
			uint8_t key_length[100] = {0, }, country[100] = {0, }, state_or_province[100] = {0, }, city_or_locality[100] = {0, }, organization[100] = {0, }, organization_unit[100] = {0, }, common_name[100] = {0, }, email_address[100] = {0, }, valid_for[100] = {0, };

			parse_query(data, "KEY_LENGTH", key_length);
			parse_query(data, "COUNTRY", country);
			parse_query(data, "STATE_OR_PROVINCE", state_or_province);
			parse_query(data, "CITY_OR_LOCALITY", city_or_locality);
			parse_query(data, "ORGANIZATION", organization);
			parse_query(data, "ORGANIZATION_UNIT", organization_unit);
			parse_query(data, "COMMON_NAME", common_name);
			parse_query(data, "EMAIL_ADDRESS", email_address);
			parse_query(data, "VALID_FOR", valid_for);

			char *cert_config = "/conf/ssl/cert.conf";
			char *finalcsr_file_path = "/conf/ssl/cert.csr";
			char *server_key = "/conf/ssl/server.key";

			printf("[...] Setting SSL\n");
			if(!strcmp(key_length, "Default"))
			{
				memset(key_length, 0, sizeof(key_length));
				strcpy(key_length, "1024");
			}

			printf("[...] Erasing cert.conf\n");
			if(file_exist(cert_config))
			{
				sprintf(cmds, "rm -f %s", cert_config);
				system(cmds);
				memset(cmds, 0, sizeof(cmds));
			}
			else
			{
				printf("[...] No cert.conf file. Keep moving\n");
			}

			printf("[...] Erasing exist SSL Key\n");
			if(file_exist(server_key))
			{
				sprintf(cmds, "rm -f %s", server_key);
				system(cmds);
				memset(cmds, 0, sizeof(cmds));
			}
			else
			{
				printf("[...] No server.key file. Keep moving\n");
			}
			printf("[...] Create SSL Key\n");
			sprintf(cmds, "openssl genrsa -out /conf/ssl/server.key %s", key_length);
			system(cmds);

			FILE *cert_file = fopen(cert_config , "w"); 
			
			char cert_text[14][200];
			
			sprintf(cert_text[0], "[ req ]\n");	
			sprintf(cert_text[1], "default_bits\t= %d\n", key_length);
			sprintf(cert_text[2], "default_md\t= sha256\n");	
			sprintf(cert_text[3], "default_keyfile\t=%s\n", server_key);
			sprintf(cert_text[4], "prompt\t = no\n");
			sprintf(cert_text[5], "encrypt_key\t= no\n\n", state_or_province);
			sprintf(cert_text[6], "# base request\ndistinguished_name = req_distinguished_name\n");
			sprintf(cert_text[7], "\n# distinguished_name\n[ req_distinguished_name ]\n");	
			sprintf(cert_text[8], "countryName\t= \"%s\"\n", country);
			sprintf(cert_text[9], "localityName\t=\"%s\"\n", city_or_locality);
			sprintf(cert_text[10], "organizationName\t=\"%s\"\n", organization);	
			sprintf(cert_text[11], "organizationalUnitName\t=\"%s\"\n", organization_unit);	
			sprintf(cert_text[12], "commonName\t=\"%s\"\n", common_name);
			sprintf(cert_text[13], "emailAddress\t=\"%s\"\n", email_address);

			int i;
			for(i=0 ; i<14 ; i++)
				fprintf(cert_file, "%s", cert_text[i]);

			fclose(cert_file);
			
			sprintf(cmds, "openssl req -config %s -new -key %s -out %s -verbose", cert_config, server_key, finalcsr_file_path);
			if(!system(cmds))
			{
				printf("[...] Checking SSl Key & CSR File\n");
				if(file_exist(finalcsr_file_path))
				{
					printf("[###] CSR File checked\n");
				}
				else
				{
					printf("[ERROR] CSR File is not exist\n");
					send_result_message(nc, 0);
				}
				
				if(file_exist(server_key))
				{
					printf("[###] SSL Key File checked\n");
				}
				else
				{
					printf("[ERROR} SSL Key File is not exist\n");
					send_result_message(nc, 0);
				}
				memset(cmds, 0, sizeof(cmds));

				sprintf(cmds, "cp %s /backup_conf/ssl/", cert_config);
				system(cmds);
				memset(cmds, 0, sizeof(cmds));

				sprintf(cmds, "cp %s /backup_conf/ssl/", server_key);
				system(cmds);
				memset(cmds, 0, sizeof(cmds));

				sprintf(cmds, "cp %s /backup_conf/ssl/", finalcsr_file_path);
				system(cmds);
				memset(cmds, 0, sizeof(cmds));

				send_result_message(nc, 1);
			}
			else
			{
				printf("[ERROR] Failed to Create CSR and SSL Key\n");
				send_result_message(nc, 0);
			}		
			set_ssl_info_1(key_length, country, state_or_province, city_or_locality, organization, valid_for);
			set_ssl_info_2(organization_unit, common_name, email_address);
		}
	}
}

/**
 * @brief domain + /ad handle
 * @bug impi_handle_rest 구현후 확인필요.  
 * @date 21.05.14
 * @author doyoung
 */
static void handle_ad_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_ad_call"<<endl;

	if (!ev_data){
		cout << "ev_data is not exists in ad handler" << endl;
		return;
	}

	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t cmds[100] = {0};
	uint8_t data[1000] = {0};
	uint8_t m_len = 0;
	uint8_t b_len = 0;
	int rets = 0;
	json::value myobj, adobj;

	m_len = hm->method.len;
	b_len = hm->body.len;

	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;

		get_active_directory_info(res_msg, &res_len);
		strncpy(response, res_msg, res_len);
		strcat(response, "\r\n\r\n");
	
		uint8_t header[1000];
		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
                mg_send_response_line(nc, 200, header);
                mg_send(nc, response, res_len + 2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "POST") == 0) || (strcmp(methods, "OPTIONS") == 0)){
		if(b_len == 0 && (strcmp(methods, "OPTIONS") == 0))
		{
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(b_len != 0 && ((strcmp(methods, "POST") == 0))){
			uint8_t result[QSIZE], response[QSIZE];
			uint8_t header[1000];
			string enable, ip, domain, secret_name, secret_pwd;

			strcpy(data, hm->body.p);
			refine_data(data);
			string sdata((char *)data);
			cout << "sdata in ad_call : " << sdata << endl;
            
			myobj = json::value::parse(sdata);
			enable = myobj.at("ENABLE").as_string();
			
			if(enable == "1"){
				ip = myobj.at("IP").as_string();
				domain = myobj.at("DOMAIN").as_string();
				secret_name = myobj.at("SECRET_NAME").as_string();
				secret_pwd = myobj.at("SECRET_PWD").as_string();
				
				set_active_directory_enable(enable.c_str());
				set_active_directory_ip_pwd(ip.c_str(), secret_pwd.c_str());
				set_active_directory_domain(domain.c_str());
				set_active_directory_username(secret_name.c_str());
				set_ldap_enable("0");
				send_result_message(nc, 1);
			}
			else{
				rets = set_active_directory_enable(enable.c_str());
				if(rets == 0)
					send_result_message(nc, 1);
				else
					send_result_message(nc, 0);
			}
		}
	}
}

/**
 * @brief domain + /ldap handle
 * @bug impi_handle_rest 구현후 확인필요. 
 * @date 21.05.14
 * @author doyoung
 */
static void handle_ldap_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_ldap_call"<<endl;

	if (!ev_data){
		cout << "ev_data is not exists in ldap handler" << endl;
		return ;
	}

	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t cmds[100] = {0};
	uint8_t m_len = 0;
	uint8_t b_len = 0;

	m_len = hm->method.len;
	b_len = hm->body.len;

	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;		

		get_ldap_info(res_msg, &res_len);
		strncpy(response, res_msg, res_len);
		strcat(response, "\r\n\r\n");

		uint8_t header[1000];
		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
                mg_send_response_line(nc, 200, header);
                mg_send(nc, response, res_len + 2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "PUT") == 0) || (strcmp(methods, "OPTIONS") == 0)){
		if(b_len == 0 && (strcmp(methods, "OPTIONS") == 0))
		{
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(b_len != 0 && ((strcmp(methods, "PUT") == 0))){
		uint8_t response[QSIZE];
			
			uint8_t header[1000];
			string ldap_en, bind_dn, ldap_ip, ldap_port, ldap_ssl, base_dn, bind_pw, timeout;
			uint8_t data[1000];
			json::value myobj;

			strcpy(data, hm->body.p);
			refine_data(data);
			string sdata((char *)data);
			cout << "sdata in ldap_call : " << sdata << endl;
            
			myobj = json::value::parse(sdata);
			ldap_en = myobj.at("LDAP_EN").as_string();

			int rets = 0;

			if(ldap_en == "1")
			{
				bind_dn = myobj.at("BIND_DN").as_string();
				ldap_ip = myobj.at("LDAP_IP").as_string();
				ldap_port = myobj.at("LDAP_PORT").as_string();
				ldap_ssl = myobj.at("LDAP_SSL").as_string();
				base_dn = myobj.at("BASE_DN").as_string();
				bind_pw = myobj.at("BIND_PW").as_string();
				timeout = myobj.at("TIMEOUT").as_string();

				
				rets = set_ldap_enable("1");
				rets += set_ldap_ip(ldap_ip.c_str());
				rets += set_ldap_port(ldap_port.c_str());
				rets += set_ldap_searchbase(base_dn.c_str());
				rets += set_ldap_binddn(bind_dn.c_str());
				rets += set_ldap_password(bind_pw.c_str());
				rets += set_ldap_ssl((char*)ldap_ssl.c_str());
				rets += set_ldap_timelimit(timeout.c_str());
				set_active_directory_enable("0");

				if(rets == 0){	
					send_result_message(nc, 1);
				}
				else{
					send_result_message(nc, 0);
				}
			} 
			else
			{
				rets = set_ldap_enable("0");
				if(rets == 0){	
					send_result_message(nc, 1);
				}
				else{
					send_result_message(nc, 0);
				}
			}
		}
	}
}

/**
 * @brief domain + /radius handle
 * @bug impi_handle_rest 구현후 확인필요.  
 * @date 21.05.14
 * @author doyoung
 */
static void handle_radius_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_radius_call"<<endl;
	
	if (!ev_data){
		cout << "ev data is not exist in radius handler" << endl;
		return;
	}
	
	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t cmds[100] = {0};
	uint8_t data[1000] = {0};
	uint8_t m_len = 0;
	uint8_t b_len = 0;
	uint8_t q_len = 0;

	m_len = hm->method.len;
	b_len = hm->body.len;
	q_len = hm->query_string.len;

	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(q_len != 0)
	{
		strncpy(data, hm->query_string.p, q_len);
		data[q_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;

		get_radius_info(res_msg, &res_len);
		strncpy(response, res_msg, res_len);
		strcat(response, "\r\n\r\n");
		uint8_t header[1000];
		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
                mg_send_response_line(nc, 200, header);
                mg_send(nc, response, res_len + 2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "POST") == 0) || (strcmp(methods, "OPTIONS") == 0)){
		if(q_len == 0 && (strcmp(methods, "OPTIONS") == 0))
		{
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(q_len != 0 && ((strcmp(methods, "POST") == 0))){
			uint8_t result[QSIZE], response[QSIZE];
			uint8_t header[1000];
			uint8_t radius_enable[2], ip[100], port[50], secret[100]; 
			
			parse_query(data, "RADIUS_ENABLE", radius_enable);
			parse_query(data, "IP", ip);
			parse_query(data, "PORT", port);
			parse_query(data, "SECRET", secret);
				
			if(strcmp(radius_enable, "1")==0)
			{
				int rets = 0;
				rets = set_radius_disable();
				rets += set_radius_info(ip, port, secret);
				if(rets == 0){	
					send_result_message(nc, 1);
				}
				else{
					send_result_message(nc, 0);
				}
			} 
			else
			{
				int rets = 0;
				rets = set_radius_disable();
				if(rets == 0){
					memset(cmds, 0, sizeof(cmds));
					if(access("/etc/raddb/server", F_OK) == 0){
						sprintf(cmds, "mv /etc/raddb/server /etc/raddb/server.bak");
						rets = system(cmds);
						if(rets == 0)
							send_result_message(nc, 1);
						else
							send_result_message(nc, 0);
					}
					else
						send_result_message(nc, 1);
				}
				else{
					send_result_message(nc, 0);
				}
			}
		}
	}
}

static void handle_sol_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_sol_call"<<endl;

	if (!ev_data){
		cout << "ev data is not exist in sol handler" << endl;
		return;
	}
	
	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t cmds[100] = {0};
	uint8_t m_len = 0;

	m_len = hm->method.len;
	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		send_result_message(nc, 1);
	}
	else if(strcmp(methods,"DELETE") == 0){
		send_result_message(nc, 0);
	}
}

/**
 * @brief domain + /kvm handle
 * @bug ipmi_handle_rest 구현 후 확인 필요.
 * @author doyoung
 */
static void handle_kvm_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_kvm_call"<<endl;

	if(!ev_data){
		cout << "ev_data is not exists in kvm handler" << endl;
	  return;
	}	

	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10], data[500] = {0};
	uint8_t cmds[100] = {0};
	uint8_t m_len, b_len = 0;


	m_len = hm->method.len;
	b_len = hm->body.len;
	
	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	
	if(b_len != 0)
	{
		strncpy(data, hm->body.p, b_len);
		data[b_len] = 0;
	}
	else if ((strcmp(methods, "OPTIONS") == 0) || (strcmp(methods, "PUT") == 0) || (strcmp(methods, "POST") == 0)){
		if (b_len == 0 && strcmp(methods,"OPTIONS") == 0){
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}", 2);
			mg_send_http_chunk(nc, "", 0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}else if (strcmp(methods, "POST") == 0){
			send_result_message(nc, 1);
		}else if (strcmp(methods, "PUT") == 0){
			send_result_message(nc, 1);
		}
	}
	else if(strcmp(methods, "GET") == 0){
		sprintf(cmds, "ps | grep KETI-KVM | grep -v grep");
		FILE *p = popen(cmds, "r");
		uint8_t response[500], result[500];
		if(p != NULL)
		{
			while(fgets(result, sizeof(result), p) != NULL)
				strncat(response, result, strlen(result));
			pclose(p);
		}
		
		if(strlen(response) == 0)
		{
			memset(cmds, 0, sizeof(cmds));
			sprintf(cmds, "KETI-KVM &");
			system(cmds);
			printf("\t\t\tdy : kvm 실행\n");
			send_result_message(nc, 1);
		}
		else{
			int ret = 0;

			ret = request_kvm_close();
			printf("\t\t\tdy : kvm 종료 요청 전송 완료\n");
			
			if(ret == 0)
			{
				uint8_t res_msg[QSIZE];
				int res_len = 0;
				request_get_setting_service(res_msg, &res_len);
			printf("\t\t\tdy : setting service result : %s, %d\n", res_msg, res_len);
				if(res_len != 0)
				{
					json::value myobj, service;
					string kvm_proxy, kvm_port;
					refine_data(res_msg);
	            	string sdata((char *)res_msg);

					myobj = json::value::parse(sdata);								
					service = myobj.at("SETTING_SERVICE");							
				
					kvm_port = service.at("KVM_PORT").as_string();
					kvm_proxy = service.at("KVM_PROXY_PORT").as_string();
					
					memset(cmds, 0, sizeof(cmds));
					sprintf(cmds, "KETI-KVM %s %s &", kvm_port.c_str(), kvm_proxy.c_str());
					system(cmds);
	
					send_result_message(nc, 1);
				}
			}
		}
	}
}

/**
 * @brief domain + /usb handler
 * @bug impi_handle_rest 구현후 확인필요 
 * @author doyoung
 */
static void handle_usb_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_usb_call"<<endl;
	
	if(!ev_data){
		cout << "ev_data is not exists in usb handelr" << endl;
		return;
	}
	
	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t cmds[300] = {0};
	uint8_t data[1000] = {0};
	uint8_t m_len, b_len = 0;
	
	b_len = hm->body.len;
	m_len = hm->method.len;

	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(b_len != 0){
		strncpy(data, hm->body.p, b_len);
		data[b_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t *file="/conf/nfs_save.sh";
		json::value obj = json::value::object();
		json::value usb = json::value::object();

		if(access(file, F_OK) == 0){
			FILE *p = popen("cat /conf/nfs_save.sh", "r");
			uint8_t result[150], nfs_option[1500], ippath[1500], ipaddr[1500], path_np[1500], path[1500] = {0};
			
			if(p != NULL){
				memset(result, 0, sizeof(result));
				memset(nfs_option, 0, sizeof(nfs_option));
				while(fgets(result, sizeof(result), p) != NULL)
					strncat(nfs_option, result, strlen(result));
				pclose(p);
			}
			
			if(strlen(nfs_option) > 0){

				p = popen("cat /conf/nfs_save.sh | grep mount | awk \'{print $4}\'", "r");
				if(p != NULL){
					memset(ippath, 0, sizeof(ippath));
					memset(ipaddr, 0, sizeof(ipaddr));
					memset(result, 0, sizeof(result));
					while(fgets(result, sizeof(result), p) != NULL)
						strncat(ippath, result, strlen(result));
					pclose(p);
				}
				uint8_t *r_index;

				r_index = strrchr((char *)ippath, ':');
				strncpy(ipaddr, ippath, strlen(ippath));
				int loop = 0;
				for(loop = (strlen(ippath) - strlen(r_index)) ; loop < strlen(ippath) ; loop++)
					ipaddr[loop] = 0;
				
				p = popen("cat /conf/nfs_save.sh | grep modprobe | awk \'{print $3}\'", "r");
				if(p != NULL){
					memset(ippath, 0, sizeof(ippath));
					memset(result, 0, sizeof(result));
					while(fgets(result, sizeof(result), p) != NULL)
						strncat(ippath, result, strlen(result));
					pclose(p);
				}

				r_index = rindex((char *)ippath, '=');
				r_index[strlen(r_index) - 1] = '\0';
				
				usb["IP_ADDRESS"] = json::value::string(U((char *)ipaddr));
				usb["PATH"] = json::value::string(U((char *)(r_index + 1)));
				obj["USB"] = usb;
			}
			else{
				usb["IP_ADDRESS"] = json::value::string(U(""));
				usb["PATH"] = json::value::string(U(""));
				obj["USB"] = usb;
			}
			uint8_t header[1000], response[1000] = {0};

			strncpy(response, obj.serialize().c_str(), obj.serialize().length());
			strcat(response, "\r\n\r\n");
			sprintf(header,  "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, strlen(response)+2);
			mg_send_response_line(nc, 200, header);
			mg_send(nc, response, strlen(response) + 2);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else{
			uint8_t header[1000], response[1000] = {0};

			usb["IP_ADDRESS"] = json::value::string(U(""));
			usb["PATH"] = json::value::string(U(""));
			obj["USB"] = usb;
			
			strncpy(response, obj.serialize().c_str(), obj.serialize().length());
			strcat(response, "\r\n\r\n");
			sprintf(header,  "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, strlen(response)+2);
			mg_send_response_line(nc, 200, header);
			mg_send(nc, response, strlen(response)+2);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
	}
	else if((strcmp(methods,"OPTIONS") == 0) || (strcmp(methods, "PUT") == 0) || (strcmp(methods, "POST") == 0) || (strcmp(methods, "DELETE") == 0)){
		FILE *p = NULL;

		if(strcmp(methods, "OPTIONS") == 0){
			if(b_len == 0 && strcmp(methods, "OPTIONS") == 0){
				mg_send_response_line(nc, 200, ACCESS_HEADER);
	                        mg_send(nc, "{}",2);
        	                mg_send_http_chunk(nc,"",0);
                	        nc->flags |= MG_F_SEND_AND_CLOSE;
			}
		}
		else if(strcmp(methods, "PUT") == 0){
			uint8_t ip_addr[50], path[10], user[20], pwd[20] = {0};
			uint8_t result[1000], response[100][3000] = {0};
			uint8_t userset=0;
			json::value myobj;
			string u_ip, u_path, u_user, u_pwd;
			string sdata((char *)data);

			myobj = json::value::parse(sdata);
			
            u_ip = myobj.at("IP_ADDRESS").as_string();
			u_path = myobj.at("PATH").as_string();
			u_user = myobj.at("USER").as_string();
			u_pwd = myobj.at("PASSWORD").as_string();
			
			strcpy(ip_addr, u_ip.c_str());
			strcpy(path, u_path.c_str());
			strcpy(user, u_user.c_str());
			strcpy(pwd , u_pwd.c_str());

			if(strlen(user) > 0)
				userset = 1;
			else if(strlen(user) == 0)
				userset = 0;
			int u_ret, lp = 0;

			uint8_t	u_lp = 0;
			u_ret = umount();

			if(u_ret == -1)
				send_result_message(nc, 2);
	
			sprintf(cmds, "mount -t nfs %s:%s /nfs_check", ip_addr, path);
			u_ret = system(cmds);
			if(u_ret != 0)
				send_result_message(nc, 3);
			
			sprintf(cmds, "ls -lphc /nfs_check | grep -v / | grep .iso");
			
			p = popen(cmds, "r");

			if(p != NULL){
				while(fgets(result, sizeof(result), p) != NULL)
				{
					strcpy(response[lp], result);
					response[lp][strlen(response[lp]) - 1] = '\0';
					lp++;
				}
				for(u_lp = 0 ; u_lp < lp ; u_lp++){
				}
				pclose(p);
			}

			json::value obj = json::value::object();
			std::vector<json::value> files_vec;
			
			uint8_t query_string[20000], temp_string[20000] = {0};
			uint8_t temp_data[12][200] = {0};
			uint8_t u_year[5], u_time[10], u_month[5], u_date[3] = {0};
			uint8_t u_index[lp][3], u_size[lp][20], u_ctime[lp][150], u_name[lp][50];
			uint8_t *ptr = NULL;
			uint8_t header[1000] = {0};
			uint8_t u_lt = 0;
			u_lp = 0;
			
			for(u_lp = 0 ; u_lp < lp ; u_lp++){
				json::value files = json::value::object();
				
				ptr = strtok(response[u_lp], " ");
				u_lt = 0;
				while( ptr != NULL){
					if(u_lt == 4)
						strcpy(u_size[u_lp], ptr);
					if(u_lt == 6)
						strcpy(u_month, ptr);
					if(u_lt == 7)
						strcpy(u_date, ptr);
					if(u_lt == 8)
						strcpy(u_time, ptr);
					if(u_lt == 9)
						strcpy(u_year, ptr);
					if(u_lt == 10)
						strcpy(u_name[u_lp], ptr);
					
					ptr = strtok(NULL, " ");
					u_lt++;
				}
				sprintf(u_ctime[u_lp], "%s %s %s %s", u_year, u_month, u_date, u_time);
				
				files["INDEX"] = json::value::number(u_lp);
				files["SIZE"] = json::value::string(U((char *)u_size[u_lp]));
				files["CREATE_TIME"] = json::value::string(U((char *)u_ctime[u_lp]));
				files["NAME"] = json::value::string(U((char *)u_name[u_lp]));
				files_vec.push_back(files);		
			}
			obj["FILES"] = json::value::array(files_vec);
			int len = obj.serialize().length();
			sprintf(header,  "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, len + 2);
			mg_send_response_line(nc, 200, header);
			mg_send(nc, obj.serialize().c_str(), len+2);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(strcmp(methods, "POST") == 0){
			int rets = umount();

			if(rets == -1)
				send_result_message(nc, 2);
			
			json::value myobj;
			string u_ip, u_path, u_user, u_pwd;
			uint8_t ip_addr[50], user[20], path[50], pwd[20], path_dir[50] = {0};
			
			memset(path, 0, sizeof(uint8_t)*50);
			
			string sdata((char *)data);
			myobj = json::value::parse(sdata);
			
			u_ip = myobj.at("IP_ADDRESS").as_string();
			u_path = myobj.at("PATH").as_string();
			u_user = myobj.at("USER").as_string();
			u_pwd = myobj.at("PASSWORD").as_string();
			
			strcpy(ip_addr, u_ip.c_str());
			strcpy(path, u_path.c_str());
			strcpy(user, u_user.c_str());
			strcpy(pwd , u_pwd.c_str());            

			int i, count = 0;

			memset(cmds, 0, sizeof(uint8_t)*300);

			uint8_t l_mod_res[500], l_mod_ret[500] = {0};

			memset(l_mod_res, 0, sizeof(uint8_t)*500);
			memset(cmds, 0, sizeof(uint8_t)*300);
			sprintf(cmds, "lsmod | grep storage");
			FILE *p = popen(cmds, "r");
			if(p != NULL){
				while(fgets(l_mod_ret, sizeof(l_mod_ret), p) != NULL)
					strncat(l_mod_res, l_mod_ret, strlen(l_mod_ret));
	
				pclose(p);
			}

			if(strlen(l_mod_res) > 0){
				rets = system("rmmod g_mass_storage");
				if(rets != 0)
					send_result_message(nc, 4);
			}

			memset(l_mod_res, 0, sizeof(uint8_t)*500);
			memset(l_mod_ret, 0, sizeof(uint8_t)*500);
			memset(cmds, 0, sizeof(uint8_t)*300);
			sprintf(cmds, "df | grep /nfs | grep -v nfs_check");
			p = popen(cmds, "r");
			if(p != NULL){
				while(fgets(l_mod_ret, sizeof(l_mod_ret), p) != NULL)
					strncat(l_mod_res, l_mod_ret, strlen(l_mod_ret));
				pclose(p);
			}

			if(strlen(l_mod_res) > 0)
			{
				system("umount -l /nfs > /dev/null 2>&1");
			}
			for(i = 0 ; i < strlen(path) ; i++)
			{
				if(path[i] == '/')
					count++;
			}

			if(count == 1){
				send_result_message(nc, 3);	
			}
			else{
				if(strstr((char *)path, "iso") != NULL)
				{
					uint8_t *r_index;
					r_index = rindex((char *)path, '/');
					
					strncpy(path_dir, path, strlen(path) - strlen(r_index));
					FILE *fp = fopen("/conf/nfs_save.sh", "w");
					fputs("#!/bin/sh\n", fp);

					memset(cmds, 0, sizeof(cmds));
					sprintf(cmds, "mount -t nfs %s:%s /nfs\n", ip_addr, path_dir);
					fputs(cmds, fp);
					rets = system(cmds);
					
					if(rets != 0)
						send_result_message(nc, 3);			

					memset(cmds, 0, sizeof(cmds));
					sprintf(cmds, "modprobe g_mass_storage file=%s iSerialNumber=123456abcdef ro=y cdrom=y stall=0\n", path);
					fputs(cmds, fp);
					rets = system(cmds);
					if(rets != 0)
						send_result_message(nc, 4);
					sprintf(cmds, "chmod 777 /conf/nfs_save.sh");
					rets = system(cmds);
					
					fclose(fp);
					send_result_message(nc, 1);
				}
				else
				{
					send_result_message(nc, 3);
				}
			}
		}
		else if(strcmp(methods, "DELETE") == 0){

			uint8_t d_response[10000], d_result[10000] = {0};
			int rets = 0;
			memset(d_response, 0, sizeof(uint8_t)*10000);
			memset(cmds, 0, sizeof(uint8_t)*300);
			sprintf(cmds, "lsmod | grep storage");
			FILE *p = popen(cmds, "r");
			if(p != NULL){
				while(fgets(d_result, sizeof(d_result), p) != NULL)
					strncat(d_response, d_result, strlen(d_result));
				pclose(p);
			}

			if(strlen(d_response) > 0){
				rets = system("rmmod g_mass_storage");
				
				if(rets == 0){
					system("umount -l /nfs > /dev/null 2>&1");
					send_result_message(nc, 1);
				}
			}
			else{
				memset(cmds, 0, sizeof(300));
				sprintf(cmds, "umount -l /nfs > /dev/null 2>&1");
				system(cmds);
				send_result_message(nc, 1);
			}
		}
	}
}

/**
 * @brief domain + /upload handler
 * @bug mongoose header doc에 따르면 MG_EV_HTTP_PART 관련 define 상수들 없어진
 * 것으로 확인됨. 구현에 대한 의논 필요
 * @date 21.05.13
 * @author doyoung
 */
static void handle_upload_call(struct mg_connection *nc, int ev, void *p) {
  cout << "Enter handle_upload_call" << endl;

  struct file_writer_data *data = (struct file_writer_data *) nc->user_data;
  struct mg_http_multipart_part *mp = (struct mg_http_multipart_part *) p;
  struct http_message *hm = (struct http_message *) p;
  uint8_t methods[10], str[100] = {0};
  uint8_t data_in[1000] = {0};
  uint8_t m_len, b_len = 0;
  uint8_t file_name[100];
  size_t file_length = 0;
  int i, j = 0;
  int offset = 0x70000;
  int maxsize = 0x90;
  int mtd_size[10], mtd_offset[10] = {0};
  uint8_t cmds[500] = {0};
  uint8_t *result;
  printf("ev = %d \n",ev);
  m_len = hm->method.len;
  if(m_len !=0){
	  strncpy(methods, hm->method.p, m_len);
  }

//   switch (ev) {

  // 				case MG_EV_HTTP_REQUEST:

  // 					m_len = hm->method.len;
  // 					if(m_len != 0){
  // 						strncpy(methods, hm->method.p,
  // m_len);
  // 					}
  // 					b_len = hm->body.len;
  // 					if(strcmp(methods, "GET") == 0){

  // 						update_req = 1;

  // 						if(access(fw_name, F_OK) == 0){
  // 							FILE *fp = fopen(fw_name,
  // "r"); 							while(i < maxsize){ 								fseek(fp, offset+i, SEEK_SET); 								fgets(str, 16,
  // fp); 								str[strlen(str) - 1] = 0; 								result = strtok(str, ":"); 								result =
  // strtok(NULL, ":"); 								mtd_size[j] = strtol(result, NULL, 16); 								i += 16; 								j++;
  // 							}
  // 							for(j = 0 ; j < 10 ;
  // j++){ 								if(j >= 2){ 									mtd_offset[j] = mtd_offset[j - 1] + mtd_size[j-1];
  // 								}
  // 								printf("offset : %x / size : %x\n", mtd_offset[j],
  // mtd_size[j]);
  // 							}
  // 							fclose(fp);

  // 							// SDR BACKUP
  // 							if(sdr_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // IPMI_SENSOR_THRESH_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // IPMI_SENSOR_THRESH_PATH);
  // 							}
  // 							if(access(IPMI_SENSOR_THRESH_PATH, F_OK)
  // ==0) 								system(cmds);

  // 							// FRU BACKUP
  // 							if(fru_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // IPMI_FRU_JSON_PATH);
  // 							}
  // 							if(access(IPMI_FRU_JSON_PATH, F_OK)
  // ==0) 								system(cmds);

  // 							// SEL BACKUP
  // 							if(sel_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // IPMI_SEL_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // IPMI_SEL_PATH);
  // 							}
  // 							if(access(IPMI_SEL_PATH, F_OK)
  // ==0) 								system(cmds);

  // 							if(sel_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // POLICY_FILE);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // POLICY_FILE);
  // 							}
  // 							if(access(POLICY_FILE, F_OK)
  // ==0) 								system(cmds);

  // 							if(sel_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // EVENT_FILTER_TABLE_FILE);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // EVENT_FILTER_TABLE_FILE);
  // 							}
  // 							if(access(EVENT_FILTER_TABLE_FILE, F_OK)
  // ==0) 								system(cmds);

  // 							// IPMI & DCMI BACKUP
  // 							if(ipmi_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/dcnu/",
  // DCMI_MANDATORY_CAPA_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // DCMI_MANDATORY_CAPA_PATH);
  // 							}
  // 							if(access(DCMI_MANDATORY_CAPA_PATH, F_OK)
  // ==0) 								system(cmds);

  // 							if(ipmi_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/dcmi/",
  // DCMI_OPTION_CAPA_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // DCMI_OPTION_CAPA_PATH);
  // 							}
  // 							if(access(DCMI_OPTION_CAPA_PATH, F_OK)
  // ==0) 								system(cmds);

  // 							if(ipmi_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/dcmi/",
  // DCMI_MGNT_CAPA_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // DCMI_MGNT_CAPA_PATH);
  // 							}
  // 							if(access(DCMI_MGNT_CAPA_PATH, F_OK)
  // ==0) 								system(cmds);

  // 							if(ipmi_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/dcmi/",
  // DCMI_CAPA_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // DCMI_CAPA_PATH);
  // 							}
  // 							if(access(DCMI_CAPA_PATH, F_OK)
  // ==0) 								system(cmds);

  // 							if(ipmi_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/dcmi/",
  // DCMI_POWER_LIMIT_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // DCMI_POWER_LIMIT_PATH);
  // 							}
  // 							if(access(DCMI_POWER_LIMIT_PATH, F_OK)
  // ==0) 								system(cmds);

  // 							if(ipmi_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/dcmi/",
  // DCMI_ASSET_MNGCTRL_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // DCMI_ASSET_MNGCTRL_PATH);
  // 							}
  // 							if(access(DCMI_ASSET_MNGCTRL_PATH, F_OK)
  // ==0) 								system(cmds);

  // 							if(ipmi_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/dcmi/",
  // DCMI_CONF_PARAM_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // DCMI_CONF_PARAM_PATH);
  // 							}
  // 							if(access(DCMI_CONF_PARAM_PATH, F_OK)
  // ==0) 								system(cmds);

  // 							if(ipmi_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // SOL_CONFIG_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // SOL_CONFIG_PATH);
  // 							}
  // 							if(access(SOL_CONFIG_PATH, F_OK)
  // ==0) 								system(cmds);

  // 							// NETWORK BACKUP
  // 							if(network_en == 0)
  // 								sprintf(cmds, "cp /etc/network/interfaces
  // /backup_conf/ipmi/"); 								system(cmds);

  // 							if(network_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // NETWORK_PRIORITY);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s"
  // NETWORK_PRIORITY);
  // 							}
  // 							if(access(NETWORK_PRIORITY, F_OK) ==
  // 0) 								system(cmds);

  // 							if(network_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // IPMI_LAN_ALERT_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s"
  // IPMI_LAN_ALERT_PATH);
  // 							}
  // 							if(access(IPMI_LAN_ALERT_PATH, F_OK) ==
  // 0) 								system(cmds);

  // 							if(network_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // IPMI_LAN_ALERT_DEDI_PATH);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s"
  // IPMI_LAN_ALERT_DEDI_PATH);
  // 							}
  // 							if(access(IPMI_LAN_ALERT_DEDI_PATH, F_OK) ==
  // 0) 								system(cmds);

  // 							// NTP BACKUP

  // 							if(ntp_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // NTP_PATH);
  // 							}
  // 							if(access(NTP_PATH, F_OK) ==
  // 0) 								system(cmds);

  // 							if(snmp_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // ALERT_PORT_BIN); 								if(sel_en == 0) 									sprintf(cmds, "rm -r %s", ALERT_PORT_BIN);
  // 							}
  // 							if(access(ALERT_PORT_BIN, F_OK) ==
  // 0) 								system(cmds);

  // 							// SSH BACKUP

  // 							if(ssh_en == 0){
  // 								sprintf(cmds, "cp -R /etc/ssh/sshd_config
  // /backup_conf/ssh/"); 								system(cmds); 								if(access(SSH_SERVICE_BIN, F_OK) == 0){
  // 									sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // SSH_SERVICE_BIN); 									system(cmds);
  // 								}
  // 							}

  // 							// KVM BACKUP
  // 							if(kvm_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi",
  // KVM_PORT_BIN);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // KVM_PORT_BIN);
  // 							}
  // 							if(access(KVM_PORT_BIN, F_OK) ==
  // 0) 								system(cmds);

  // 							// AUTH BACKUP
  // 							if(auth_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // RAD_BIN);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // RAD_BIN);
  // 							}
  // 							if(access(RAD_BIN, F_OK) ==
  // 0) 								system(cmds);

  // 							if(auth_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // LDAP_BIN);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // LDAP_BIN);
  // 							}
  // 							if(access(LDAP_BIN, F_OK) ==
  // 0) 								system(cmds);

  // 							if(auth_en == 0){
  // 								sprintf(cmds, "cp %s /backup_conf/ipmi/",
  // AD_BIN);
  // 							}
  // 							else{
  // 								sprintf(cmds, "rm -r %s",
  // AD_BIN);
  // 							}
  // 							if(access(AD_BIN, F_OK) ==
  // 0) 								system(cmds);

  // 							if(auth_en == 0){
  // 								if(access(AD_BIN, F_OK) ==
  // 0){ 									sprintf(cmds, "cp -R %s /backup_conf/", AUTH_PATH); 									system(cmds);
  // 									sprintf(cmds, "cp /etc/nslcd.conf
  // /backup_conf/"); 									system(cmds);
  // 								}
  // 							}

  // 							// -------- start upgrade firmware ----------
  // //

  // 							printf("start upgrade
  // firmware"); 							if(sdr_en == 1 && fru_en == 1 && sel_en == 1 && ipmi_en == 1 &&
  // network_en == 1 && ntp_en == 1 && snmp_en == 1 && ssh_en == 1 && kvm_en ==
  // 1 && auth_en == 1 && web_en == 1)
  // 							{
  // 								sprintf(cmds, "mtd write %s /dev/mtd0",
  // fw_name); 								system(cmds);
  // 							}
  // 							else{
  // 								sprintf(cmds, "mtd erase /dev/mtd0 -O 0 -S 0x%x",
  // (mtd_size[1] + mtd_size[2] + mtd_size[3])); 								system(cmds); 								sprintf(cmds,
  // "mtd write %s /dev/mtd0 -n -O 0 -S 0x%x", fw_name, (mtd_size[1] +
  // mtd_size[2] + mtd_size[3])); 								system(cmds); 								sprintf(cmds, " rm -r
  // /conf/first_boot"); 								system(cmds);
  // 								// conf erase
  // 								if(!(sdr_en == 0 && fru_en == 0 && sel_en == 0 &&
  // ipmi_en == 0 && network_en == 0 && ntp_en == 0 && snmp_en == 0 && ssh_en ==
  // 0 && kvm_en == 0 && auth_en == 0 && web_en == 0)){ 									sprintf(cmds, "mtd erase
  // /dev/mtd0 -O 0x%x -S 0x%x", mtd_offset[4], mtd_size[4]); 									system(cmds);
  // 									sprintf(cmds, "mtd write %s /dev/mtd0 -n -p 0x%x
  // -O 0x%x -S 0x%x", fw_name, mtd_offset[4], mtd_offset[4], mtd_size[4]);
  // 									system(cmds);
  // 								}

  // 								if(web_en == 1){
  // 									sprintf(cmds, "mtd erase /dev/mtd0 -O 0x%x -S
  // 0x%x", mtd_offset[6], mtd_size[6]); 									system(cmds); 									sprintf(cmds, "mtd write
  // %s /dev/mtd0 -n -p 0x%x -O 0x%x -S 0x%x", fw_name, mtd_offset[6],
  // mtd_offset[6], mtd_size[6]); 									system(cmds);
  // 								}

  // 								// etc update
  // 								sprintf(cmds, "mtd erase /dev/mtd0 -O 0x%x -S
  // 0x%x", mtd_offset[7], mtd_size[7]); 								system(cmds); 								sprintf(cmds, "mtd write
  // %s /dev/mtd0 -n -p 0x%x -O 0x%x -S 0x%x", fw_name, mtd_offset[7],
  // mtd_offset[7], mtd_size[7]); 								system(cmds);

  // 								// rootfs update
  // 								sprintf(cmds, "mtd erase /dev/mtd0 -O 0x%x -S
  // 0x%x", mtd_offset[8], mtd_size[8]); 								system(cmds); 								sprintf(cmds, "mtd write
  // %s /dev/mtd0 -n -p 0x%x -O 0x%x -S 0x%x", fw_name, mtd_offset[8],
  // mtd_offset[8], mtd_size[8]); 								system(cmds);
  // 							}
  // 							update_req = 0;

  // 							sprintf(cmds, "rm -r %s",
  // fw_name); 							system(cmds); 							send_result_message(nc, 1);

  // 							syscall(SYS_reboot, LINUX_REBOOT_MAGIC1,
  // LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART, NULL);

  // 						}
  // 						else{
  // 							send_result_message(nc,
  // 0);
  // 						}
  // 					}
  // 					if(strcmp(methods, "PUT") == 0 || strcmp(methods, "OPTIONS")
  // == 0 || strcmp(methods, "POST") == 0){ 						if(strcmp(methods,"OPTIONS") == 0){
  // 								mg_send_response_line(nc, 200,
  // ACCESS_HEADER); 								mg_send(nc, "{}",2); 								mg_send_http_chunk(nc,"",0); 								nc->flags
  // |= MG_F_SEND_AND_CLOSE;
  // 						}
  // 						else if(strcmp(methods, "PUT") ==
  // 0){ 							if(b_len != 0){ 								strncpy(data_in, hm->body.p, b_len); 								printf("data :
  // %s\n", data_in);
  // 							}
  // 							if(strstr((char *)data_in, "STATUS") !=
  // NULL){ 								if(upload_req == 0 || upload_req == 1) 									send_status_message(nc,
  // upload_req); 								if(update_req == 1){ 									send_status_message(nc, 2);
  // 								}
  // 							}
  // 							else if(strstr((char *)data_in, "SDR") != NULL ||
  // strstr((char *)data_in, "FRU") != NULL){ 								json::value myobj; 								string
  // sdata((char *)data_in);

  // 								myobj =
  // json::value::parse(sdata); 								sdr_en = myobj.at("SDR").as_integer(); 								fru_en =
  // myobj.at("FRU").as_integer(); 								sel_en = myobj.at("SEL").as_integer();
  // 								ipmi_en =
  // myobj.at("IPMI").as_integer(); 								network_en =
  // myobj.at("NETWORK").as_integer(); 								ntp_en = myobj.at("NTP").as_integer();
  // 								snmp_en =
  // myobj.at("SNMP").as_integer(); 								ssh_en = myobj.at("SSH").as_integer();
  // 								kvm_en =
  // myobj.at("KVM").as_integer(); 								auth_en =
  // myobj.at("AUTHENTICATION").as_integer(); 								web_en =
  // myobj.at("WEB").as_integer();
  // 							}
  // 							send_result_message(nc,
  // 1);
  // 						}
  // 					}
  // 				break;
  // 				case MG_EV_HTTP_PART_BEGIN: {

  // 					if (data == NULL && already_upload == 0)
  // { 						memset(fw_name, 0, sizeof(fw_name)); 						sprintf(file_name, "/fw/%s",
  // mp->file_name); 						printf("file name : %s\n", file_name); 						strcpy(fw_name,
  // file_name); 						data = calloc(1, sizeof(struct file_writer_data));

  // 						data->fp =
  // fopen(file_name,"w+"); 						data->bytes_written = 0; 						already_upload = 1;
  // 						upload_req = 1;
  // 						if (data->fp == NULL) {
  // 							mg_printf(nc, "%s",
  // 												"HTTP/1.1 500 Failed to open a
  // file\r\n" 												"Content-Length: 0\r\n\r\n"); 							nc->flags |= MG_F_SEND_AND_CLOSE;
  // 							free(data);
  // 							return;
  // 						}

  // 						nc->user_data = (void *) data;
  // 					}

  // 					break;
  // 				}
  // 				case MG_EV_HTTP_PART_DATA: {

  // 					if(already_upload == 1){
  // 						file_length = fwrite(mp->data.p, 1, mp->data.len,
  // data->fp); 						if(file_length != mp->data.len){ 							mg_printf(nc, "%s", 												"HTTP/1.1
  // 500 Failed to write to a file\r\n" 												"Content-Length: 0\r\n\r\n"); 							nc->flags
  // |= MG_F_SEND_AND_CLOSE; 							return;
  // 						}
  // 						else
  // 							data->bytes_written +=
  // mp->data.len;
  // 					}
  // 					break;
  // 				}

  // 				case MG_EV_HTTP_PART_END: {
  // 					if(data != NULL){
  // 						nc->flags |=
  // MG_F_SEND_AND_CLOSE; 						fclose(data->fp); 						free(data); 						nc->user_data = NULL;
  // 						already_upload = 2;
  // 						break;
  // 					}
  // 				}
  // 				case MG_EV_HTTP_MULTIPART_REQUEST_END:{
  // 					already_upload = 0;
  // 					upload_req = 0;
  // 					send_result_message(nc, 1);
  // 					break;
  // 				}
  // 		}
}

/**
 * @brief domain + /watt handle
 * @bug impi_handle_rest 구현후 확인필요. 웹에서 기능이 확인되지 않음  
 * @date 21.05.14
 * @author doyoung
 */
static void handle_watt_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_watt_call"<<endl;

	if (!ev_data){
		cout << "ev_data is not exists in watt handler" << endl;
		return;
	}
	cout<<"evdata?"<<endl;
	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t data [100] = {0};
	uint8_t methods[10] = {0};
	uint8_t cmds[100] = {0};
	uint8_t m_len, q_len = 0;
	uint8_t index = 0;
	
	m_len = hm->method.len;
	q_len = hm->query_string.len;
	if(m_len != 0){
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(q_len != 0){
		strncpy(data, hm->query_string.p, q_len);
		data[q_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		char cli_bug[100] = {0};
		mg_sock_to_str(nc->sock, cli_bug, sizeof(cli_bug), MG_SOCK_STRINGIFY_REMOTE | MG_SOCK_STRINGIFY_IP | MG_SOCK_STRINGIFY_PORT);
		if(q_len != 0){
			index = data[q_len - 1];
			uint8_t res_msg[QSIZE];
			uint8_t response[QSIZE];
			// try
			// {
			// 	cout<<"handle_watt_call:STEP0 data:"<<(char *)data<<endl;
			// 	//refine_data(data);
			// }
			// catch (const std::exception&)
			// {
			// 	cout<<"handle_watt_call:refine_data error "<<endl;
			// }
			cout<<"handle_watt_call:STEP0 data:"<<(char *)data<<endl;
			cout<<"handle_watt_call:STEP1 data:"<<(char *)data<<endl;
			int res_len = 0;
			memset((void *)res_msg, 0, QSIZE);
			memset((void *)response, 0, QSIZE);
			get_total_power_usage(index, res_msg, &res_len);
			cout<<"handle_watt_call:STEP3 res_msg:"<<(char *)res_msg<<endl;
			cout<<"handle_watt_call:STEP4 response:"<<(char *)response<<endl;
			
			strncpy(response, res_msg, res_len);
			strcat(response, "\r\n\r\n");
			uint8_t header[1000];
			sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, strlen(response)+2);
			mg_send_response_line(nc, 200, header);
			mg_send(nc, response, strlen(response)+2);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
	}
}

/**
 * @brief domain + /warmReset handler
 * @date 21.05.13 
 * @author doyoung
 */
static void handle_warm_reset_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_warm_reset_call"<<endl;

	if (!ev_data){
		cout<<"ev_data is not exist in warm reset call"<<endl;
		return;
	}
	
	struct http_message *hm = (struct http_message *) ev_data;

	uint8_t cmds[100] = {0};
	uint8_t methods[10] = {0};
	uint8_t m_len = 0;

	m_len = hm->method.len;
	if(m_len != 0){
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}

	if((strcmp(methods, "OPTIONS") == 0) || (strcmp(methods, "POST") == 0)){
		if(strcmp(methods, "OPTIONS") == 0){
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}", 2);
			mg_send_http_chunk(nc, "", 0);
			nc->flags != MG_F_SEND_AND_CLOSE;
		}
		else if(strcmp(methods, "POST") == 0){
			send_result_message(nc, 1);
			sprintf(cmds, "/etc/init.d/rcK && /etc/init.d/rcS");
			system(cmds);
		}
	}
}

/**
 * @brief domain + /bmcReset handler -> reboot bmc
 * @date 21.05.13 
 * @author doyoung
 */
static void handle_bmc_reset_call(struct mg_connection *nc, int ev, void *ev_data){
	cout<<"Enter handle_bmc_reset_call"<<endl;
	
	struct http_message *hm = (struct http_message *) ev_data;

	uint8_t cmds[100] = {0};
	uint8_t methods[10] = {0};
	uint8_t m_len = 0;
	int rets = 0;

	if (!ev_data){
		cout<<"ev_data is not exists in bmc reset handler"<<endl;
		return;
	}

	m_len = hm->method.len;
	if(m_len != 0){
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}

	if((strcmp(methods, "OPTIONS") == 0) || (strcmp(methods, "POST") == 0)){
		if(strcmp(methods, "OPTIONS") == 0){
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}", 2);
			mg_send_http_chunk(nc, "", 0);
			nc->flags != MG_F_SEND_AND_CLOSE;
		}
		else if(strcmp(methods, "POST") == 0){	
			send_result_message(nc, 1);
			sprintf(cmds, "rm -r /conf/ipmi/* && rm -r /conf/dcmi/* && rm -r /backup_conf/ipmi/* && rm -r /backup_conf/dcmi/*");
			system(cmds);
			system("reboot");
		}
	}
}

static void handle_login_call(struct mg_connection *nc, int ev, void *ev_data) {
    cout<<"enter logincall "<<endl;
	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t data[100] = {0};
	uint8_t methods[10] = {0};
	uint8_t m_len, b_len = 0;
    json::value j;
	if(!ev_data)
	  return;

	m_len = hm->method.len;
	b_len = hm->body.len;
	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(b_len != 0)
	{
		strncpy(data, hm->body.p, b_len);
		data[b_len] = 0;
	}

	if((strcmp(methods, "POST") == 0) || (strcmp(methods, "OPTIONS") == 0))
	{
		if(b_len == 0 && (strcmp(methods, "OPTIONS") == 0))
		{
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(b_len != 0 && (strcmp(methods, "POST") == 0))
		{
			string username,password;
			char result[20];
            string sdata((char *)data);
            cout<<"data ="<<sdata<<endl;

            
            json::value myobj =json::value::parse(sdata);
            username=myobj.at("USERNAME").as_string();
            password=myobj.at("PASSWORD").as_string();
			int res_len = 0;
			try_login(username.c_str(), password.c_str(), result, &res_len);

			cout<<"\tresult ="<<result<<endl;
			cout<<"\tresult ="<<(uint8_t)result<<endl;
			uint8_t header[1000];
			uint8_t res_msg[50];
			memset(res_msg, 0, sizeof(res_msg));
			sprintf(res_msg, "{\"PRIVILEGE\":\"%c\"}\r\n\r\n", result[0]);
			sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, strlen(res_msg));
			mg_send_response_line(nc, 200, header);
			cout <<"\t res_msg :"<<res_msg<<" res_msg size :"<<strlen(res_msg)<<endl;
			mg_send(nc, res_msg, strlen(res_msg));
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
	}
	cout<<"end logincall "<<endl;
	
}

/**
 * @brief domain + /setting handler
 * @bug impi_handle_rest 구현후 확인필요
 * @date 21.05.13 
 * @author doyoung
 */
static void handle_setting_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_setting_call"<<endl;
	
	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10], data[1000] = {0};
	uint8_t cmds[100] = {0};
	uint8_t m_len, b_len = 0;

	string origin_value, req_value;
	if(!ev_data)
	{
		cout<<"ev_data not exist"<<endl;
		return;
	}
	
	m_len = hm->method.len;
	b_len = hm->body.len;
	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(b_len != 0)
	{
		strncpy(data, hm->body.p, b_len);
		data[b_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;

		request_get_setting_service(res_msg, &res_len);
		strncpy(response, res_msg, res_len);
		strcat(response,"\r\n\r\n");
		
		uint8_t header[1000];
		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
		mg_send_response_line(nc, 200, header);
		mg_send(nc, response, res_len + 2);
		nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "PUT") == 0) || (strcmp(methods, "OPTIONS") == 0)){
		if(b_len == 0 && strcmp(methods, "OPTIONS") == 0){
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(strcmp(methods, "PUT") == 0){
			
			uint8_t res_msg[QSIZE], response[QSIZE];
			int rets, res_len = 0;
				request_get_setting_service(res_msg, &res_len);
				memset(response, 0, sizeof(response));
				strncpy(response, res_msg, res_len);

				char result[20];
				
				refine_data(data);
            	refine_data(response);

				string sdata((char *)data);
				string sresponse((char *)response);
            	
				json::value myobj =json::value::parse(sresponse);
				json::value d_myobj=json::value::parse(sdata);
				json::value setting = myobj.at("SETTING_SERVICE");
				
				origin_value=setting.at("WEB_PORT").as_string();
				req_value= d_myobj.at("WEB_PORT").as_string();
				
				if(origin_value!=req_value){
					rets = request_set_setting_service(0, req_value.c_str());
					
					if(rets == 0)
						send_result_message(nc, 0);
					else
						send_result_message(nc, 1);
				}

				origin_value = to_string(setting.at("SSH_ENABLES").as_integer());
				req_value= d_myobj.at("SSH_ENABLES").as_string();
				
				if(req_value == "0")
				{
					rets = request_set_setting_service(1, req_value.c_str());
					if(rets != 0)
						send_result_message(nc,0);
				}
				else{
					req_value= d_myobj.at("SSH_PORT").as_string();

					rets = request_set_setting_service(1, req_value.c_str());
					if(rets != 0)
						send_result_message(nc, 0);
				}
				
				origin_value = to_string(setting.at("ALERT_ENABLES").as_integer());
				req_value= d_myobj.at("ALERT_ENABLES").as_string();
				
				if(req_value == "0")
				{
					rets = request_set_setting_service(2, req_value.c_str());
					if(rets != 0)
						send_result_message(nc,0);
				}
				else{
					memset(cmds, 0, sizeof(cmds));
					req_value= d_myobj.at("ALERT_PORT").as_string();

					rets = request_set_setting_service(2, req_value.c_str());
					if(rets != 0)
						send_result_message(nc, 0);
				}
				
				origin_value = setting.at("ALERT_PORT").as_string();
				req_value= d_myobj.at("ALERT_PORT").as_string();

				if(strcmp(origin_value.c_str(), req_value.c_str()) != 0){
					rets = request_set_setting_service(2, req_value.c_str());
					if(rets != 0)
						send_result_message(nc, 0);
				}

				string d_port, d_proxy, o_port, o_proxy;

				o_proxy = setting.at("KVM_PROXY_PORT").as_string();
				d_proxy = d_myobj.at("KVM_PROXY_PORT").as_string();

				o_port = setting.at("KVM_PORT").as_string();
				d_port = d_myobj.at("KVM_PORT").as_string();
				
				if((o_port != d_port) && (o_proxy == d_proxy)){
					rets = request_set_setting_service(3, d_port.c_str());
					if(rets != 0)
						send_result_message(nc, 0);
				}
				if((o_port == d_port) && (o_proxy != d_proxy)){
					rets = request_set_setting_service(4, d_proxy.c_str());
					if(rets != 0)
						send_result_message(nc, 0);
				}
				if((o_port != d_port) && (o_proxy != d_proxy)){
					memset(cmds, 0, sizeof(cmds));
					sprintf(cmds, "req setting set 5 %d %d %s %s",strlen(d_port.c_str()), strlen(d_proxy.c_str()), d_port.c_str(), d_proxy.c_str());
					rets = system(cmds);
					if(rets != 0)
						send_result_message(nc, 0);
				}
				send_result_message(nc, 1);
		}

	}
	cout<<"leave hadler_setting_call"<<endl;
}


static void handle_user_call(struct mg_connection *nc, int ev, void *ev_data) {
	cout<<"Enter handle_user_call"<<endl;
	struct http_message *hm = (struct http_message *) ev_data;
	uint8_t methods[10] = {0};
	uint8_t cmds[100] = {0};
	uint8_t data[1000] = {0};
	uint8_t q_string[100] = {0};
	uint8_t m_len, b_len, q_len = 0;
    string name, index, password, enable_status, callback, linkauth, ipmimsg, privilege;

	if(!ev_data)
	{
		cout<<"ev_data not exists in user_call"<<endl;
		return;
	}
	
	b_len = hm->body.len;
	m_len = hm->method.len;
	q_len = hm->query_string.len;

	if(m_len != 0)
	{
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(b_len != 0){
		strncpy(data, hm->body.p, b_len);
		data[b_len] = 0;
	}
	if(strcmp(methods, "GET") == 0){
		uint8_t response[QSIZE], res_msg[QSIZE];
		int res_len = 0;
		
		get_user_list(res_msg, &res_len);
		strncpy(response, res_msg, res_len);
		strcat(response, "\r\n\r\n");
		uint8_t header[1000];
		sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
                mg_send_response_line(nc, 200, header);
                mg_send(nc, response, res_len + 2);
                nc->flags |= MG_F_SEND_AND_CLOSE;
	}
	else if((strcmp(methods, "POST") == 0) || (strcmp(methods, "OPTIONS") == 0) || (strcmp(methods, "DELETE") == 0)){
		if(b_len == 0 && (strcmp(methods, "OPTIONS") == 0))
		{
			mg_send_response_line(nc, 200, ACCESS_HEADER);
			mg_send(nc, "{}",2);
			mg_send_http_chunk(nc,"",0);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
		else if(b_len != 0 && ((strcmp(methods, "POST") == 0))){
			uint8_t response[QSIZE];
			uint8_t header[1000];
            
            // uint8_t username [16], password[16];
			char result[20];
            string sdata((char *)data);
            
			cout << "sdata in user_call : " << sdata << endl;
            
			json::value myobj =json::value::parse(sdata);
            //cout<<myobj.serialize()<<endl;
            name=myobj.at("NAME").as_string();
            index=myobj.at("INDEX").as_string();
            password=myobj.at("PASSWORD").as_string();
            enable_status=myobj.at("ENABLE_STATUS").as_string();
            callback=myobj.at("CALLBACK").as_string();
            linkauth=myobj.at("LINKAUTH").as_string();
            ipmimsg=myobj.at("IPMIMSG").as_string();
            privilege=myobj.at("PRIVILEGE").as_string();

            char pwd_length[10] = {0};
			if(stoi(index) <= 10)
			{
				int rets = 0;
				rets = set_user_name(index.c_str(), name.c_str());
				rets += set_user_enable(index.c_str(), enable_status.c_str());
				sprintf(pwd_length, "%d", strlen(password.c_str()));
				rets += set_user_password(index.c_str(), password.c_str(), pwd_length);
				rets += set_user_access(index.c_str(), enable_status.c_str(), callback.c_str(), linkauth.c_str(), ipmimsg.c_str(), privilege.c_str());
				if(rets == 0)
					send_result_message(nc, 1);
				else
					send_result_message(nc, 0);
			} 
			else
			{
				printf("error(user index)\n");
			}
		}
		else if((q_len != 0) && ((strcmp(methods, "DELETE") == 0))){

			strncpy(data, hm->query_string.p, q_len);
			int rets = 0;
			char *dp;
			char index_num = 0;
			
			dp = strtok(data, "=");
			dp = strtok(NULL, "=");
			index_num = atoi(dp);
			rets = delete_user(index_num);
			if(rets == 0)
				send_result_message(nc, 1);
			else
				send_result_message(nc, 0);
		}
		
	}
}



static void handle_main_call(struct mg_connection *nc, int ev, void *ev_data){//void *ev_data) {
	cout <<"Enter handle_main_call"<<endl;
    struct http_message *hm = (struct http_message *) ev_data;
	cout <<"\tEnter handle_main_call 1"<<endl;
	uint8_t data [100] = {0};
	uint8_t methods[10] = {0};
	uint8_t m_len, q_len = 0;
	uint8_t index = 0;
	if(hm==nullptr)
	{
		cout <<"\t hm NULLPTR"<<endl;
		return ;
	}
	m_len = hm->method.len;
	q_len = hm->query_string.len;
	if(m_len != 0){
		strncpy(methods, hm->method.p, m_len);
		methods[m_len] = 0;
	}
	if(q_len != 0){
		strncpy(data, hm->query_string.p, q_len);
		printf("query_string in main_call : %s\n", data);
		data[q_len] = 0;
	}

	if(strcmp(methods, "GET") == 0){
		if(q_len != 0){
			index = data[q_len - 1];
			uint8_t result[QSIZE];
			uint8_t response[QSIZE];
			uint8_t res_msg[QSIZE];
		
			memset(res_msg, 0, sizeof(res_msg));
			memset(response, 0, sizeof(response));
			int res_len=0;
			show_main(index-48, res_msg, &res_len);
			strncpy(response, res_msg, res_len);
			uint8_t header[1000];
			strcat(response,"\r\n\r\n");
			sprintf(header, "%sContent-Length: %d\r\n\r\n", ACCESS_RESPONSE_HEADER, res_len + 2);
			mg_send_response_line(nc, 200, header);
			mg_send(nc, response, res_len + 2);
			nc->flags |= MG_F_SEND_AND_CLOSE;
		}
	}	
    cout <<"Leave main call"<<endl;
}

/**
 * @brief restful 초기화
 * @bug /usr/sbin일경우에만 정상작동
 * 
 */
void restful_init(void)
{
    cout << "restful_init" << endl;
    struct mg_mgr mgr;
    struct mg_connection *nc;
    struct mg_bind_opts bind_opts;
    int i;
    uint8_t *cp;
    const uint8_t *err_str;
// #if MG_ENABLE_SSL
//   const uint8_t *ssl_cert = NULL;
// #endif

    char *root ="../../";
    mg_mgr_init(&mgr, NULL);
    s_http_server_opts.document_root=root;
    cout << "document root =" << s_http_server_opts.document_root << endl;
    /* Set HTTP server options */
    memset(&bind_opts, 0, sizeof(bind_opts));
    bind_opts.error_string = &err_str;

// #if MG_ENABLE_SSL
//     if (ssl_cert != NULL)
//     {
//         bind_opts.ssl_cert = ssl_cert;
//     }
// #endif
    nc = mg_bind(&mgr, s_http_port, ev_handler); //mg_bind_opt(&mgr, s_http_port, ev_handler, bind_opts);
    cout << "mg_bind(&mgr, s_http_port, ev_handler)" << endl;
    if (nc == NULL)
    {
        fprintf(stderr, "Error starting server on port %s: %s\n", s_http_port,
                *bind_opts.error_string);
        exit(1);
    }
    mg_register_http_endpoint(nc, "/user", handle_user_call);
	mg_register_http_endpoint(nc, "/login", handle_login_call);

	mg_register_http_endpoint(nc, "/main", handle_main_call);
    
	mg_register_http_endpoint(nc, "/sysinfo", handle_sysinfo_call);
    
	mg_register_http_endpoint(nc, "/fru", handle_fru_call);
    
	mg_register_http_endpoint(nc, "/sensor", handle_sensor_call);
    mg_register_http_endpoint(nc, "/eventlog", handle_eventlog_call);

	
	
	mg_register_http_endpoint(nc, "/ddns", handle_ddns_call);
    mg_register_http_endpoint(nc, "/network", handle_network_call);
    mg_register_http_endpoint(nc, "/ntp", handle_ntp_call);
    mg_register_http_endpoint(nc, "/smtp", handle_smtp_call);
    mg_register_http_endpoint(nc, "/ssl", handle_ssl_call);
    mg_register_http_endpoint(nc, "/activedir", handle_ad_call);
    mg_register_http_endpoint(nc, "/ldap", handle_ldap_call);
    mg_register_http_endpoint(nc, "/radius", handle_radius_call);

	mg_register_http_endpoint(nc, "/kvm", handle_kvm_call);
	mg_register_http_endpoint(nc, "/power", handle_power_call);
    mg_register_http_endpoint(nc, "/usb", handle_usb_call);
	
	mg_register_http_endpoint(nc, "/upload", handle_upload_call MG_UD_ARG(NULL));
	mg_register_http_endpoint(nc, "/warmReset", handle_warm_reset_call);
	mg_register_http_endpoint(nc, "/bmcReset", handle_bmc_reset_call);
    mg_register_http_endpoint(nc, "/watt", handle_watt_call);
	mg_register_http_endpoint(nc, "/setting", handle_setting_call);    

    mg_register_http_endpoint(nc, "/sol", handle_sol_call);
	
	mg_set_protocol_http_websocket(nc);
    s_http_server_opts.enable_directory_listing = "yes";

    printf("Starting RESTful server on port %s, serving %s\n", s_http_port,
           s_http_server_opts.document_root);
    for (;;)
    {
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);
}


static void ev_handler(struct mg_connection *nc, int ev, void *ev_data) {
  struct http_message *hm = (struct http_message *) ev_data;

  switch (ev) {
    case MG_EV_HTTP_REQUEST:
		mg_serve_http(nc, hm, s_http_server_opts); /* Serve static content */
	//}
      break;
    default:
      break;
  }
}

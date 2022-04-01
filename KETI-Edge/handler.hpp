#pragma once
#include <redfish/stdafx.hpp>
#include <redfish/resource.hpp>

#define SERVER_CERTIFICATE_CHAIN_PATH "/conf/ssl/rootca.crt"
#define SERVER_PRIVATE_KEY_PATH "/conf/ssl/rootca.key"
#define SERVER_TMP_DH_PATH "/conf/ssl/dh2048.pem"

#define SERVER_REQUEST_TIMEOUT 10
#define SERVER_ENTRY_PORT ":443"
// #define SERVER_ENTRY_PORT ":8000"
#define SERVER_ENTRY_POINT "http://0.0.0.0" SERVER_ENTRY_PORT

#define MODULE_NAME "CM1"
#define MODULE_TYPE "CM"
#define CMM_ADDRESS "http://10.0.6.106:8000"
#define HA_ADDRESS "http://10.0.6.107:644"




class Handler
{
public:
    // Constructor
    Handler(){};
    Handler(utility::string_t _url, http_listener_config _config);

    // Destructor
    ~Handler(){};

    // Server operation
    pplx::task<void> open() { return m_listener.open(); }
    pplx::task<void> close() { return m_listener.close(); }

private:
    http_listener m_listener;
    vector<string> list;

    // Request handler
    void handle_get(http_request _request);
    void handle_put(http_request _request);
    void handle_post(http_request _request);
    void handle_patch(http_request _request);
    void handle_delete(http_request _request);
    void handle_options(http_request _request);

    int ipmi_handle_redfish(redfish_req_t *request, redfish_res_t *response);
    void all_handle();
};

unsigned int generate_task_resource(string _method, string _uri, json::value _jv, http_headers _header);
void complete_task_resource(unsigned int _num);

// void treat_get(http_request _request, http_response& _response);
void treat_post(http_request _request, json::value _jv, http_response& _response);
void do_actions(http_request _request, json::value _jv, http_response& _response);
void act_certificate(json::value _jv, string _resource, string _what, http_response& _response);
void act_certificate_service(json::value _jv, string _resource, string _what, http_response& _response);
void act_system(json::value _jv, string _resource, string _what, http_response& _response);
void act_eventservice(json::value _jv, string _resource, string _what, http_response& _response);
void act_logservice(json::value _jv, string _resource, string _what, http_response& _response);
void act_virtualmedia(json::value _jv, string _resource, string _what, http_response& _response);

void make_account(json::value _jv, http_response& _response);
void make_session(json::value _jv, http_response& _response);
void make_subscription(json::value _jv, http_response& _response);

void treat_patch(http_request _request, json::value _jv, http_response& _response);
void modify_account(http_request _request, json::value _jv, string _uri, http_response& _response);
void modify_role(http_request _request, json::value _jv, string _uri, http_response& _response);

bool patch_account_service(json::value _jv, string _record_uri);
bool patch_session_service(json::value _jv);
bool patch_manager(json::value _jv, string _record_uri);
bool patch_network_protocol(json::value _jv, string _record_uri);
void patch_fan_mode(string _mode, string _record_uri);
bool patch_ethernet_interface(json::value _jv, string _record_uri, int _flag);
bool patch_system(json::value _jv, string _record_uri);
bool patch_chassis(json::value _jv, string _record_uri);
bool patch_power_control(json::value _jv, string _record_uri);
bool patch_event_service(json::value _jv, string _record_uri);
bool patch_subscription(json::value _jv, string _record_uri);

void treat_delete(http_request _request, json::value _jv, http_response& _response);
void remove_account(json::value _jv, string _uri, string _service_uri, http_response& _response);
void remove_session(http_request _request, http_response& _response);
void remove_subscription(string _uri, string _service_uri, http_response& _response);

void error_reply(json::value _jv, status_code _status, http_response& _response);
json::value get_error_json(string _message);
void success_reply(json::value _jv, status_code _status, http_response& _response);

void report_last_command(string _uri);

void test_send_event(Event _event);
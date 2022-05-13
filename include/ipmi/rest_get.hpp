#pragma once
#include<string>
#include<ipmi/apps.hpp>
#include<ipmi/user.hpp>
#include<ipmi/fru.hpp>
#include<ipmi/sdr.hpp>
#include<ipmi/sel.hpp>
#include<redfish/resource.hpp>

using namespace std;
extern string chassis_type_desc_fru[100];
#define MAX_NIC 2



class Ipmiweb_GET{
    public:
        static void Get_Show_Main(int menu, json::value &response_json);
        static void Get_Fru_Info(json::value &response_json);
        static void Get_Sensor_Info(json::value &response_json);
        static void Get_Eventlog(json::value &response_json);
        static void Get_DDNS_Info(json::value &response_json);
        static void Get_Lan_Info(json::value &response_json);
        static void Get_Ntp_Info(json::value &response_json);
        static void Get_Smtp_Info(json::value &response_json, int flag);
        static void Get_Ssl_Info(json::value &response_json);
        static void Get_Active_Dir(json::value &response_json);
        static void Get_Ldap(json::value &response_json);
        static void Get_Radius(json::value &response_json);
        static void Get_Setting_Service(json::value &response_json);
       
};  

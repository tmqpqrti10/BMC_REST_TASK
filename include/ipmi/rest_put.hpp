#pragma once
#include<string>
#include<ipmi/apps.hpp>
#include<ipmi/user.hpp>
#include<ipmi/fru.hpp>
#include<ipmi/sdr.hpp>
#include<redfish/resource.hpp>

using namespace std;

class Ipmiweb_PUT{
    public:
        static void Set_Fru_Header(int fru_id, string hdr_board,string  hdr_chassis, string hdr_product);
};
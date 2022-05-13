#pragma once
#include<string>
#include<ipmi/apps.hpp>
#include<ipmi/user.hpp>
#include<ipmi/fru.hpp>
#include<ipmi/sdr.hpp>
#include<redfish/resource.hpp>

using namespace std;


class Ipmiweb_POST{
    public:
        static int Try_Login(string username , string pwd);
};
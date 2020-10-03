#ifndef PJSIPJSONCONFIG_HPP
#define PJSIPJSONCONFIG_HPP

#include <iostream>
#include <pjsua2.hpp>

using namespace pj;
using namespace std;

//#define PJSIP_JSON_ENABLE

class pjsipJsonConfig
{
public:
    pjsipJsonConfig(string &filename);
    ~pjsipJsonConfig();

    void pjsipJsonConfigLoad();
    void pjsipJsonConfigSave();

    string pjsipAppAgentName;
    string pjsip_acc_id;
    string pjsip_realm;
    string pjsip_address;
    string pjsip_username;
    string pjsip_password;
    int pjsip_proto = 0;
    int pjsip_port = 5070;
    int pjsip_level = 4;  
    bool pjsip_video_enable = false; 

private:
    string config_filename;
    void pjsipJsonConfigWriteObject(string &filename);
    void pjsipJsonConfigReadObject(string &filename);
};

#endif // PJSIPJSONCONFIG_HPP

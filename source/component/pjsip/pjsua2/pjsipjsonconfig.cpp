#include <fstream>
#include <iostream>
#include <pjsua2.hpp>
#include "pjsipaccount.hpp"
#include "pjsipcall.hpp"
#include "pjsipbuddy.hpp"
#include "pjsipapp.hpp"
#include "pjsiplogwriter.hpp"
#include "pjsipjsonconfig.hpp"
#include "json.h"


using namespace pj;
using namespace std;
/*
{ 

   "SipTransport":         { 
      "TransportConfig":      { 
         "port":                 5070,
         "portRange":            0,
         "publicAddress":        "",
         "boundAddress":         "",
         "qosType":              0,
         "qosParams":            { 
            "qos.flags":            0,
            "qos.dscp_val":         0,
            "qos.so_prio":          0,
            "qos.wmm_prio":         0
         },
         "TlsConfig":            { 
            "CaListFile":           "",
            "certFile":             "",
            "privKeyFile":          "",
            "password":             "",
            "CaBuf":                "",
            "certBuf":              "",
            "privKeyBuf":           "",
            "method":               0,
            "ciphers":              [ ],
            "verifyServer":         false,
            "verifyClient":         false,
            "requireClientCert":    false,
            "msecTimeout":          0,
            "qosType":              0,
            "qosParams":            { 
               "qos.flags":            0,
               "qos.dscp_val":         0,
               "qos.so_prio":          0,
               "qos.wmm_prio":         0
            },
            "qosIgnoreError":       true
         }
      }
   },
   "accounts":             [ 
      { 
         "AccountConfig":        { 
            "priority":             0,
            "idUri":                "sip:1007@192.168.2.253",
            "AccountRegConfig":     { 
               "registrarUri":         "sip:192.168.2.253",
               "registerOnAdd":        true,
               "timeoutSec":           3600,
               "retryIntervalSec":     600,
               "firstRetryIntervalSec": 0,
               "randomRetryIntervalSec": 10,
               "delayBeforeRefreshSec": 5,
               "dropCallsOnFail":      false,
               "unregWaitMsec":        4000,
               "proxyUse":             3,
               "contactParams":        "",
               "headers":              [ ]
            },
            "AccountSipConfig":     { 
               "proxies":              [ ],
               "contactForced":        "",
               "contactParams":        "",
               "contactUriParams":     "",
               "authInitialEmpty":     false,
               "authInitialAlgorithm": "",
               "transportId":          -1,
               "authCreds":            [ ]
            },
            "AccountCallConfig":    { 
               "holdType":             0,
               "prackUse":             0,
               "timerUse":             1,
               "timerMinSESec":        90,
               "timerSessExpiresSec":  1800
            },
            "AccountPresConfig":    { 
               "publishEnabled":       false,
               "publishQueue":         true,
               "publishShutdownWaitMsec": 2000,
               "pidfTupleId":          "",
               "headers":              [ ]
            },
            "AccountMwiConfig":     { 
               "enabled":              false,
               "expirationSec":        3600
            },
            "AccountNatConfig":     { 
               "sipStunUse":           0,
               "mediaStunUse":         2,
               "nat64Opt":             0,
               "iceEnabled":           false,
               "iceMaxHostCands":      -1,
               "iceAggressiveNomination": true,
               "iceNominatedCheckDelayMsec": 400,
               "iceWaitNominationTimeoutMsec": 10000,
               "iceNoRtcp":            false,
               "iceAlwaysUpdate":      true,
               "turnEnabled":          false,
               "turnServer":           "",
               "turnConnType":         17,
               "turnUserName":         "",
               "turnPasswordType":     0,
               "turnPassword":         "",
               "contactRewriteUse":    1,
               "contactRewriteMethod": 6,
               "viaRewriteUse":        1,
               "sdpNatRewriteUse":     0,
               "sipOutboundUse":       1,
               "sipOutboundInstanceId": "",
               "sipOutboundRegId":     "",
               "udpKaIntervalSec":     15,
               "udpKaData":            "\r\n",
               "contactUseSrcPort":    1
            },
            "AccountMediaConfig":   { 
               "lockCodecEnabled":     true,
               "streamKaEnabled":      false,
               "srtpUse":              0,
               "srtpSecureSignaling":  1,
               "SrtpOpt":              { 
                  "cryptos":              [ ],
                  "keyings":              [ ]
               },
               "ipv6Use":              0,
               "TransportConfig":      { 
                  "port":                 4000,
                  "portRange":            0,
                  "publicAddress":        "",
                  "boundAddress":         "",
                  "qosType":              0,
                  "qosParams":            { 
                     "qos.flags":            0,
                     "qos.dscp_val":         0,
                     "qos.so_prio":          0,
                     "qos.wmm_prio":         0
                  },
                  "TlsConfig":            { 
                     "CaListFile":           "",
                     "certFile":             "",
                     "privKeyFile":          "",
                     "password":             "",
                     "CaBuf":                "",
                     "certBuf":              "",
                     "privKeyBuf":           "",
                     "method":               0,
                     "ciphers":              [ ],
                     "verifyServer":         false,
                     "verifyClient":         false,
                     "requireClientCert":    false,
                     "msecTimeout":          0,
                     "qosType":              0,
                     "qosParams":            { 
                        "qos.flags":            0,
                        "qos.dscp_val":         0,
                        "qos.so_prio":          0,
                        "qos.wmm_prio":         0
                     },
                     "qosIgnoreError":       true
                  }
               },
               "rtcpMuxEnabled":       false
            },
            "AccountVideoConfig":   { 
               "autoShowIncoming":     true,
               "autoTransmitOutgoing": true,
               "windowFlags":          0,
               "defaultCaptureDevice": -1,
               "defaultRenderDevice":  -2,
               "rateControlMethod":    1,
               "rateControlBandwidth": 0,
               "startKeyframeCount":   5,
               "startKeyframeInterval": 1000
            }
         },
         "buddies":              [ ]
      }
   ]
}
*/

/** \brief Write a Value object to a string.
 * Example Usage:
 * $g++ stringWrite.cpp -ljsoncpp -std=c++11 -o stringWrite
 * $./stringWrite
 * {
 *     "action" : "run",
 *     "data" :
 *     {
 *         "number" : 1
 *     }
 * }
 */
/*
int main() {
  Json::Value root;
  Json::Value data;
  constexpr bool shouldUseOldWay = false;
  root["action"] = "run";
  data["number"] = 1;
  root["data"] = data;

  if (shouldUseOldWay) {
    Json::FastWriter writer;
    const std::string json_file = writer.write(root);
    std::cout << json_file << std::endl;
  } else {
    Json::StreamWriterBuilder builder;
    const std::string json_file = Json::writeString(builder, root);
    std::cout << json_file << std::endl;
  }
  return EXIT_SUCCESS;
}
int main() {
  const std::string rawJson = R"({"Age": 20, "Name": "colin"})";
  const auto rawJsonLength = static_cast<int>(rawJson.length());
  constexpr bool shouldUseOldWay = false;
  JSONCPP_STRING err;
  Json::Value root;

  if (shouldUseOldWay) {
    Json::Reader reader;
    reader.parse(rawJson, root);
  } else {
    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(rawJson.c_str(), rawJson.c_str() + rawJsonLength, &root,
                       &err)) {
      std::cout << "error" << std::endl;
      return EXIT_FAILURE;
    }
  }
  const std::string name = root["Name"].asString();
  const int age = root["Age"].asInt();

  std::cout << name << std::endl;
  std::cout << age << std::endl;
  return EXIT_SUCCESS;
}
*/
void pjsipJsonConfig::pjsipJsonConfigWriteObject(string &filename)
{
#ifdef PJSIP_JSON_ENABLE
	char* szJSON = NULL;
	cJSON* pRoot = cJSON_CreateObject();
   if(pRoot)
	{
      cJSON_AddStringToObject(pRoot, "pjsipAppAgentName", pjsipAppAgentName.c_str());
      cJSON_AddStringToObject(pRoot, "pjsip_acc_id", pjsip_acc_id.c_str());
      cJSON_AddStringToObject(pRoot, "pjsip_realm", pjsip_acc_id.c_str());
      cJSON_AddStringToObject(pRoot, "pjsip_address", pjsip_acc_id.c_str());

      cJSON_AddNumberToObject(pRoot, "pjsip_proto", pjsip_proto);
      cJSON_AddNumberToObject(pRoot, "pjsip_port", pjsip_port);
      cJSON_AddNumberToObject(pRoot, "pjsip_level", pjsip_level);
  
		szJSON = cJSON_Print(pRoot);
		if(szJSON)
		{
         ofstream load_file;
         load_file.open(filename, std::ofstream::out);
         if(load_file.is_open())
         {
            load_file << szJSON << endl;
            // 关闭打开的文件
            load_file.close();
         }
         utilLogWrite(3, "pjsipJsonConfig ", szJSON);
         //std::cout << "Enter " << szJSON << std::endl;
			cjson_free(szJSON);
		}
		cJSON_Delete(pRoot);
   }
#else
   Json::Value root;
   Json::StreamWriterBuilder builder;
   root["pjsipAppAgentName"] = pjsipAppAgentName;
   root["pjsip_acc_id"] = pjsip_acc_id;
   root["pjsip_realm"] = pjsip_realm;
   root["pjsip_address"] = pjsip_address;
   root["pjsip_username"] = pjsip_username;
   root["pjsip_password"] = pjsip_password;
   root["pjsip_proto"] = pjsip_proto;
   root["pjsip_port"] = pjsip_port;
   root["pjsip_level"] = pjsip_level;
   root["pjsip_video_enable"] = pjsip_video_enable;
   const std::string json_file = Json::writeString(builder, root);

   ofstream load_file;
   load_file.open(filename, std::ofstream::out);
   if(load_file.is_open())
   {
      load_file << json_file << endl;
      // 关闭打开的文件
      load_file.close();
   }
#endif   
}

void pjsipJsonConfig::pjsipJsonConfigReadObject(string &filename)
{
#ifdef PJSIP_JSON_ENABLE
   string data;
   ifstream load_file;
   load_file.open(filename, std::ifstream::in);
   if(!load_file.is_open())
      return;
   load_file >> data; 
   // 关闭打开的文件
   load_file.close();
   if(data.empty())
      return;
	cJSON* pItem = cJSON_Parse (data.c_str());
	cJSON* pj_tmp = NULL;
	if (pItem == NULL)
	{
		return;
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "pjsipAppAgentName");
	if (pj_tmp != NULL)
	{
      pjsipAppAgentName.clear();
      if(pj_tmp->valuestring)
         pjsipAppAgentName = string(pj_tmp->valuestring);
	}

	pj_tmp = cJSON_GetObjectItem (pItem, "pjsip_acc_id");
	if (pj_tmp)
	{
      pjsip_acc_id.clear();
      if(pj_tmp->valuestring)
         pjsip_acc_id = string(pj_tmp->valuestring);
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "pjsip_realm");
	if (pj_tmp)
	{
      pjsip_realm.clear();
      if(pj_tmp->valuestring)
         pjsip_realm = string(pj_tmp->valuestring);
	}
   pj_tmp = cJSON_GetObjectItem (pItem, "pjsip_address");
	if (pj_tmp)
	{
      pjsip_address.clear();
      if(pj_tmp->valuestring)
         pjsip_address = string(pj_tmp->valuestring);
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "pjsip_proto");
	if (pj_tmp)
	{
		pjsip_proto = pj_tmp->valueint;
	}
	pj_tmp = cJSON_GetObjectItem (pItem, "pjsip_port");
	if (pj_tmp)
	{
		pjsip_port = pj_tmp->valueint;
	}
   pj_tmp = cJSON_GetObjectItem (pItem, "pjsip_level");
	if (pj_tmp)
	{
		pjsip_level = pj_tmp->valueint;
	}
   cJSON_Delete(pItem);
#else
   string data;
   ifstream load_file;
   load_file.open(filename, std::ifstream::in);
   if(!load_file.is_open())
      return;
   load_file >> data; 
   // 关闭打开的文件
   load_file.close();
   if(data.empty())
      return;
   const auto rawJsonLength = static_cast<int>(data.length());
   JSONCPP_STRING err;
   Json::Value root;

   Json::CharReaderBuilder builder;
   const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
   if (!reader->parse(data.c_str(), data.c_str() + rawJsonLength, &root,
                       &err)) {
      //utilLogWrite(3, "pjsipJsonConfig ", "ERROR parse");
      return ;
   }
   pjsipAppAgentName = root["pjsipAppAgentName"].asString();
   pjsip_acc_id = root["pjsip_acc_id"].asString();
   pjsip_realm = root["pjsip_realm"].asString();
   pjsip_address = root["pjsip_address"].asString();
   pjsip_username = root["pjsip_username"].asString();
   pjsip_password = root["pjsip_password"].asString();

   pjsip_proto = root["pjsip_proto"].asInt();
   pjsip_port = root["pjsip_port"].asInt();
   pjsip_level = root["pjsip_level"].asInt();
   pjsip_video_enable = root["pjsip_level"].asBool();
   
   return ;
#endif   
}

void pjsipJsonConfig::pjsipJsonConfigLoad()
{
   if(!config_filename.empty())
      pjsipJsonConfigReadObject(config_filename);
}

void pjsipJsonConfig::pjsipJsonConfigSave()
{
   if(!config_filename.empty())
      pjsipJsonConfigWriteObject(config_filename);   
}

pjsipJsonConfig::pjsipJsonConfig(string &filename)
{
   config_filename = filename;
}

pjsipJsonConfig::~pjsipJsonConfig()
{
   config_filename.clear();
}



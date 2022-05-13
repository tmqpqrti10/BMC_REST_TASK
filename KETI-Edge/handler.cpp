#include "handler.hpp"
#include <ipmi/user.hpp>
#include <redfish/resource.hpp>
extern Ipmiuser ipmiUser[MAX_USER];
#include <fstream>
unsigned int g_count = 0;
extern ServiceRoot *g_service_root;

/**
 * @brief Handler class constructor with method connection
 *
 */
extern int redfish_power_ctrl;
Handler::Handler(utility::string_t _url, http_listener_config _config)
    : m_listener(_url, _config) {
  log(info) << "_url=" << _url;
  // Handler connection
  this->m_listener.support(methods::GET, std::bind(&Handler::handle_get, this,
                                                   std::placeholders::_1));
  this->m_listener.support(methods::PUT, std::bind(&Handler::handle_put, this,
                                                   std::placeholders::_1));
  this->m_listener.support(methods::POST, std::bind(&Handler::handle_post, this,
                                                    std::placeholders::_1));
  this->m_listener.support(
      methods::DEL,
      std::bind(&Handler::handle_delete, this, std::placeholders::_1));
  this->m_listener.support(
      methods::PATCH,
      std::bind(&Handler::handle_patch, this, std::placeholders::_1));
  this->m_listener.support(
      methods::OPTIONS, bind(&Handler::handle_options, this, placeholders::_1));
}
/**
 * @brief GET method request handler
 *
 * @param _request Request object
 * @todo ipmi_redfish_handler 연동중
 */
void Handler::handle_get(http_request _request) {

  log(info) << "Request method: GET";
  string uri = _request.request_uri().to_string();
  vector<string> uri_tokens = string_split(uri, '/');
  string filtered_uri = make_path(uri_tokens);
  string servicepath;
  json::value jv = _request.extract_json().get();
  unsigned int task_number;
  log(info) << "Request URI : " << filtered_uri;
  log(info) << "Request Body : " << _request.to_string();

  http_response response;
  json::value response_json;
  string rec_string;

  try {

    rec_string.clear();
    if (uri_tokens.size() == 1 && uri_tokens[0] == "redfish") {
      json::value j;
      j[U(REDFISH_VERSION)] = json::value::string(U(ODATA_SERVICE_ROOT_ID));
      // report_last_command(uri);
      _request.reply(status_codes::OK, j);
      return;
    }
    //rest
    else {
      
      if( uri_tokens[0].find("?")){
        uri_tokens = string_split(uri_tokens[0],'?');
        if(uri_tokens[0]=="main"){
          cout << " get main start" <<endl;
          int menu = filtered_uri.find("=") + 1;
          cout << filtered_uri[menu] <<endl;
          menu = filtered_uri[menu] - 48;
          Ipmiweb_GET::Get_Show_Main(menu,response_json);
          cout << "recv "<<rec_string <<endl;
          response.set_body(response_json);
        }
      } 
        cout << "uri token " << uri_tokens[0] <<endl;
        if(uri_tokens[0] == "fru"){
          cout << " get fru " << endl;
          Ipmiweb_GET::Get_Fru_Info(response_json);
          cout << " fru json " << response_json << endl;
          response.set_body(response_json);
        }
        else if ( uri_tokens[0] == "sensor"){
          cout << " get sensor " << endl;
          Ipmiweb_GET::Get_Sensor_Info(response_json);
          cout << " sensor json " << response_json << endl;
          response.set_body(response_json);
        }
        else if ( uri_tokens[0] == "eventlog"){
          cout << " get sensor " << endl;
          Ipmiweb_GET::Get_Eventlog(response_json);
          cout << " sensor json " << response_json << endl;
          response.set_body(response_json);
        }
        else if ( uri_tokens[0] == "ddns"){
          cout << " get ddns " << endl;
          Ipmiweb_GET::Get_DDNS_Info(response_json);
          cout << " ddns json " << response_json << endl;
          response.set_body(response_json);
        }
        else if ( uri_tokens[0] == "network"){
          cout << " get lan info " <<endl;
          Ipmiweb_GET::Get_Lan_Info(response_json);
          cout << " lan info json " << response_json << endl;
          response.set_body(response_json);
        }
        else if ( uri_tokens[0] == "ntp"){
          cout << " get ntp info " <<endl;
          Ipmiweb_GET::Get_Ntp_Info(response_json);
          cout << " lan ntp json " << response_json << endl;
          response.set_body(response_json);
        }
        else if ( uri_tokens[0] == "smtp"){
          cout << " get smtp info " <<endl;
          Ipmiweb_GET::Get_Smtp_Info(response_json,0);
          cout << " lan smtp json " << response_json << endl;
          response.set_body(response_json);
        }
        else if ( uri_tokens[0] == "ssl"){
          cout << " get ssl info " <<endl;
          Ipmiweb_GET::Get_Ssl_Info(response_json);
          cout << " lan ssl json " << response_json << endl;
          response.set_body(response_json);
        }        
        else if ( uri_tokens[0] == "activedir"){
          cout << " get activedir info " <<endl;
          Ipmiweb_GET::Get_Active_Dir(response_json);
          cout << " lan activedir json " << response_json << endl;
          response.set_body(response_json);
        }
        else if ( uri_tokens[0] == "ldap"){
          cout << " get ldap info " <<endl;
          Ipmiweb_GET::Get_Ldap(response_json);
          cout << " lan ldap json " << response_json << endl;
          response.set_body(response_json);
        }
        else if ( uri_tokens[0] == "radius"){
          cout << " get ldap info " <<endl;
          Ipmiweb_GET::Get_Radius(response_json);
          cout << " lan radius json " << response_json << endl;
          response.set_body(response_json);
        }
        else if ( uri_tokens[0] == "setting"){
          cout << " get setting info " <<endl;
          Ipmiweb_GET::Get_Setting_Service(response_json);
          cout << " lan setting json " << response_json << endl;
          response.set_body(response_json);
        }        
    }

    // 위에부분은 task를 만들지 않고 내부적처리

    // 여기부턴 만들고 처리
    // task_number = generate_task_resource("GET", uri, jv, _request.headers());
    response.headers().add("Access-Control-Allow-Origin", "*");
    response.headers().add("Access-Control-Allow-Credentials", "true");
    response.headers().add("Access-Control-Allow-Methods",
                           "POST, GET, OPTIONS, DELETE, PATCH");
    response.headers().add("Access-Control-Max-Age", "3600");
    response.headers().add(
        "Access-Control-Allow-Headers",
        "Content-Type, Accept, X-Requested-With, remember-me, X-Auth-Token");
    response.headers().add("Access-Control-Expose-Headers",
                           "X-Auth-Token, Location");
    // treat_get(_request, response);
    // #1 헤더에 X-Auth-Token있는지
    // if(!_request.headers().has("X-Auth-Token"))
    // {
    //     response.set_status_code(status_codes::NetworkAuthenticationRequired);
    // }
    // // #2 X토큰에 있는 값의 세션이 유효한지
    // else if(!is_session_valid(_request.headers()["X-Auth-Token"]))
    // {
    //     response.set_status_code(status_codes::Unauthorized);
    // }
    // #3 uri로 들어온 레코드가 존재하는지
    // /*else*/ if (!record_is_exist(uri)) {
    //   response.set_status_code(status_codes::NotFound);
    // }
    // #4 모두 통과하면 해당 레코드 정보 GET
    // else {
    //   // sync_ipmi(uri);
    //   response.set_status_code(status_codes::OK);
    //   response.set_body(record_get_json(uri));
    // }

    // complete_task_resource(task_number);

  } 
  catch (json::json_exception &e) {
    log(info) << "badRequest" << uri_tokens[0];
    _request.reply(status_codes::BadRequest);
    return;
  }

  // report_last_command(uri);
  response.set_status_code(status_codes::OK);
  _request.reply(response);
  return;
}

/**
 * @brief DELETE request handler
 *
 * @param _request Request object
 */
void Handler::handle_delete(http_request _request) {

  log(info) << "Request method: DELETE";
  string uri = _request.request_uri().to_string();
  vector<string> uri_tokens = string_split(uri, '/');
  string filtered_uri = make_path(uri_tokens);
  string servicepath;
  json::value jv = _request.extract_json().get();
  unsigned int task_number;
  // json::value j = _request.extract_json().get();

  log(info) << "Request URI : " << uri;
  log(info) << "Request Body : " << _request.to_string();

  http_response response;
  json::value response_json;

  try {

    task_number = generate_task_resource("DELETE", uri, jv, _request.headers());

    response.headers().add("Access-Control-Allow-Origin", "*");
    response.headers().add("Access-Control-Allow-Credentials", "true");
    response.headers().add("Access-Control-Allow-Methods",
                           "POST, GET, OPTIONS, DELETE, PATCH");
    response.headers().add("Access-Control-Max-Age", "3600");
    response.headers().add(
        "Access-Control-Allow-Headers",
        "Content-Type, Accept, X-Requested-With, remember-me, X-Auth-Token");
    response.headers().add("Access-Control-Expose-Headers",
                           "X-Auth-Token, Location");

    treat_delete(_request, jv, response);

    complete_task_resource(task_number);
    // report_last_command(uri);
  } catch (json::json_exception &e) {
    log(info) << "badRequest" << uri_tokens[0];
    _request.reply(status_codes::BadRequest);
    return;
  }

  // _request.reply(status_codes::NotImplemented);
  _request.reply(response);
  return;
}

/**
 * @brief PUT request handler
 *
 * @param _request Request object
 */
void Handler::handle_put(http_request _request) {
  log(info) << "Request method: PUT";
  cout << "handle_put request" << endl;

  auto j = _request.extract_json().get();

  _request.reply(status_codes::NotImplemented, U("PUT Request Response"));
}

/**
 * @brief PATCH request handler
 *
 * @param _request Request object
 */
extern Ipmiapplication ipmiApplication;
void Handler::handle_patch(http_request _request) {
  log(info) << "Request method: PATCH";
  string uri = _request.request_uri().to_string();
  vector<string> uri_tokens = string_split(uri, '/');
  string filtered_uri = make_path(uri_tokens);
  // json::value b = _request.extract_json(true).get();
  json::value jv = _request.extract_json().get();
  unsigned int task_number;

  log(info) << "Request URI : " << uri;
  log(info) << "Request Body : " << _request.to_string();

  Role *role;

  http_response response;
  json::value response_json;

  try {

    task_number = generate_task_resource("PATCH", uri, jv, _request.headers());

    response.headers().add("Access-Control-Allow-Origin", "*");
    response.headers().add("Access-Control-Allow-Credentials", "true");
    response.headers().add("Access-Control-Allow-Methods",
                           "POST, GET, OPTIONS, DELETE, PATCH");
    response.headers().add("Access-Control-Max-Age", "3600");
    response.headers().add(
        "Access-Control-Allow-Headers",
        "Content-Type, Accept, X-Requested-With, remember-me, X-Auth-Token");
    response.headers().add("Access-Control-Expose-Headers",
                           "X-Auth-Token, Location");

    treat_patch(_request, jv, response);

    complete_task_resource(task_number);

    // report_last_command(uri);
    /**
     * brief
     */
    //     if (filtered_uri == ODATA_ACCOUNT_ID)
    //     {
    //         string password;
    //         string username;
    //         string odata_id;
    //         string index;
    //         Account *account;
    //         string role_id;
    //         bool enabled = true;

    //         index = b.at("Id").as_string();
    //         username = b.at("UserName").as_string();
    //         password = b.at("Password").as_string();
    //         role_id=b.at("RoleId").as_string();
    //         enabled=b.at("Enabled").as_bool();

    //         Collection *account_collection = (Collection
    //         *)g_record[ODATA_ACCOUNT_ID]; Collection *roled_collection
    //         =(Collection *)g_record[ODATA_ROLE_ID]; json::value obj =
    //         account_collection->get_json();

    //         cout <<obj.serialize()<<endl;
    //         vector<Resource *> *members= account_collection->get_member();

    //         //vector capacity를 줄여줌
    //         //members->shrink_to_fit();

    //         for(int i=0;i<members->size();i++)
    //         {
    //             account=(Account *)members;
    //             string member_Id=((Account *)members->at(i))->id;
    //             cout<<"Resource ["<<i<<"] id="<<member_Id<<endl;
    //             if(member_Id ==index)
    //             {
    //                 if (password !="")
    //                     account->password=password;
    //                 if (username !="")
    //                     account->user_name=username;

    //                 //role 관련
    //                 if (role_id !="")
    //                 {
    //                     vector<Resource *> *roles=
    //                     roled_collection->get_member(); for(int
    //                     j=0;j<roles->size();j++)
    //                     {
    //                         string temp=((Role *)roles->at(i))->id;
    //                          if(member_Id ==role_id)
    //                          {
    //                              account->role=((Role *)roles->at(i));
    //                          }
    //                     }
    //                 }
    //                 if (enabled !=false)
    //                     account->enabled=enabled;
    //             }
    //         }
    //         account_collection->save_json();
    //         roled_collection->save_json();
    //         _request.reply(status_codes::Created);
    //         return;
    //     }
    //     //SESSION ID
    //     else if (filtered_uri == ODATA_SESSION_ID)
    //     {
    //         _request.reply(status_codes::Found);
    //         return ;
    //     }
    //     else if (filtered_uri == ODATA_CHASSIS_ID)
    //     {

    //         string c_assettag;
    //         string c_mfg;
    //         string c_serial;
    //         string c_partnumber;
    //         string c_chassistype;
    //         uint8_t c_ctype;
    //         Chassis *chassis = (Chassis *)g_record[ODATA_CHASSIS_ID];
    //         auto temp = b.as_object().find("c_assettag");

    //         if (temp != b.as_object().end())
    //         {
    //             c_assettag = b.at("AssetTag").as_string();
    //             strcpy(((char
    //             *)chassis->fru_this->product.asset_tag),c_assettag.c_str());
    //             chassis->asset_tag = c_assettag;
    //         }
    //         temp = b.as_object().find("Manufacturer");
    //         if (temp != b.as_object().end())
    //         {
    //             c_mfg = b.at("Manufacturer").as_string();
    //             strcpy(((char
    //             *)chassis->fru_this->fru_board.mfg),c_mfg.c_str());
    //             chassis->manufacturer = c_mfg;
    //         }
    //         temp = b.as_object().find("SerialNumber");
    //         if (temp != b.as_object().end())
    //         {
    //             c_serial = b.at("SerialNumber").as_string();
    //             strcpy(((char
    //             *)chassis->fru_this->fru_chassis.serial),c_serial.c_str());
    //             chassis->serial_number = c_serial;
    //         }
    //         temp = b.as_object().find("PartNumber");
    //         if (temp != b.as_object().end())
    //         {
    //             c_partnumber = b.at("PartNumber").as_string();
    //             strcpy(((char
    //             *)chassis->fru_this->fru_chassis.part_number),c_serial.c_str());
    //             chassis->part_number = c_partnumber;
    //         }
    //         temp = b.as_object().find("ChassisType");
    //         if (temp != b.as_object().end())
    //         {
    //             c_ctype = b.at("ChassisType").as_integer();
    //             chassis->fru_this->fru_chassis.type=c_ctype;
    //             chassis->type = c_ctype;
    //         }
    //         _request.reply(status_codes::Found);
    //         return ;
    //     }
    //     //SYSTEMS ID
    //     else if (filtered_uri == ODATA_SYSTEM_ID)
    //     {
    //         string s_assettag;
    //         string s_hostname;
    //         string s_indicator_led;

    //         Systems *systems = (Systems *)g_record[ODATA_SYSTEM_ID];
    //         auto temp = b.as_object().find("AssetTag");

    //         if (temp != b.as_object().end())
    //         {
    //             s_assettag = b.at("AssetTag").as_string();
    //             strcpy(((char
    //             *)systems->fru_this->product.asset_tag),s_assettag.c_str());
    //             systems->asset_tag=s_assettag;

    //         }
    //         temp = b.as_object().find("HostName");
    //         if (temp != b.as_object().end())
    //         {
    //             s_hostname = b.at("HostName").as_string();
    //             ofstream fout("/etc/hostname");
    //             fout<<s_hostname<<endl;
    //             fout.close();
    //             ipmiApplication.g_host_name=s_hostname;
    //             systems->hostname=s_hostname;
    //         }

    //         temp = b.as_object().find("IndicatorLED");
    //         if (temp != b.as_object().end())
    //         {
    //             s_indicator_led = b.at("IndicatorLED").as_string();
    //             if(s_indicator_led =="Off"||s_indicator_led =="off")
    //                 systems->indicator_led=LED_OFF;
    //             else if(s_indicator_led =="on"||s_indicator_led =="On")
    //             {
    //                 systems->indicator_led=15;
    //             }
    //             else{
    //                 log(info)<<"error : indicator_led not score";
    //                 systems->indicator_led=15;
    //             }
    //             systems->save_json();
    //             _request.reply(status_codes::Found);
    //             return;

    //         }
    //     }
    // _request.reply(status_codes::BadRequest);
  } catch (json::json_exception &e) {
    log(info) << "json_Exeception error ";
    _request.reply(status_codes::BadRequest);
    return;
  }

  // _request.reply(status_codes::NotImplemented, U("PATCH Request Response"));
  _request.reply(response);
  return;
}
/**
 * @brief POST request handler
 *
 * @param _request Request object
 */
void Handler::handle_post(http_request _request) {
  log(info) << "Request method: POST";
   
  string uri = _request.request_uri().to_string();
  vector<string> uri_tokens = string_split(uri, '/');
  string filtered_uri = make_path(uri_tokens);
  json::value jv = _request.extract_json().get();
  unsigned int task_number;
  json::value rj;

  log(info) << "Request URI : " << filtered_uri;
  log(info) << "Request Body : " << _request.to_string();

  http_response response;
  json::value response_json;

  try {
    if (uri_tokens.size() == 1 && uri_tokens[0] == "Event") {
      log(info) << "[RECEIVE][EVENT][SUCCESSFULLY]";
      cout << "JSON : " << jv << endl;
      _request.reply(status_codes::OK);
      return;
    }

    if( uri_tokens[0] == "login" ){
      cout << " start login " <<endl;
      cout << jv["USERNAME"].as_string() <<endl;
      string uname = jv["USERNAME"].as_string();
      string pwd = jv["PASSWORD"].as_string();
      int is_login=Ipmiweb_POST::Try_Login(uname,pwd);
      cout << " end login " <<endl;
      
      if(is_login>0){
      response_json["PRIVILEGE"] = json::value::string(to_string(is_login));

      };
    }
    response.set_body(response_json);
    response.set_status_code(status_codes::OK);
    // task_number = generate_task_resource("POST", uri, jv, _request.headers());
    
    response.headers().add("Access-Control-Allow-Origin", "*");
    response.headers().add("Access-Control-Allow-Credentials", "true");
    response.headers().add("Access-Control-Allow-Methods",
                           "POST, GET, OPTIONS, DELETE, PATCH");
    response.headers().add("Access-Control-Max-Age", "3600");
    response.headers().add(
        "Access-Control-Allow-Headers",
        "Content-Type, Accept, X-Requested-With, remember-me, X-Auth-Token");
    response.headers().add("Access-Control-Expose-Headers",
                           "X-Auth-Token, Location");

    // treat_post(_request, rj, response);

    // complete_task_resource(task_number);

    // report_last_command(uri);

    // Account handling
    // if (filtered_uri == ODATA_ACCOUNT_ID)
    // {
    //     string user_name;
    //     string password;
    //     string odata_id;
    //     Account *account;
    //     string role_id = "ReadOnly";
    //     bool enabled = true;

    //     user_name = b.at("UserName").as_string();
    //     password = b.at("Password").as_string();

    //     // Check password length enought
    //     if (password.size() < ((AccountService
    //     *)g_record[ODATA_ACCOUNT_SERVICE_ID])->min_password_length)
    //     {
    //         _request.reply(status_codes::BadRequest);
    //         return;
    //     }
    //     odata_id = ODATA_ACCOUNT_ID;
    //     odata_id = odata_id + '/' + user_name;

    //     // Check account already exist
    //     if (record_is_exist(odata_id))
    //     {
    //         _request.reply(status_codes::Conflict);
    //         return;
    //     }

    //     // Additinal account information check
    //     if (b.as_object().find("RoleId") != b.as_object().end())
    //     {
    //         odata_id = ODATA_ROLE_ID;
    //         role_id = b.at("RoleId").as_string();
    //         odata_id = odata_id + '/' + role_id;
    //         // Check role exist
    //         if (!record_is_exist(odata_id))
    //         {
    //             _request.reply(status_codes::BadRequest);
    //             return;
    //         }
    //     }
    //     if (b.as_object().find("Enabled") != b.as_object().end())
    //         enabled = b.at("Enabled").as_bool();

    //     // TODO id를 계정 이름 말고 숫자로 변경 필요
    //     odata_id = filtered_uri + '/' + user_name;
    //     int result = new_ipmi_user(user_name, password, enabled);
    //     if (result == -1)
    //     {
    //         cout << "error not create user" << endl;
    //         _request.reply(status_codes::BadRequest);
    //     }
    //     account = new Account(odata_id, role_id);
    //     account->name = "User Account";
    //     account->user_name = user_name;
    //     account->id = user_name;
    //     account->password = password;
    //     account->enabled = enabled;
    //     account->locked = false;

    //     Collection *account_collection = (Collection
    //     *)g_record[ODATA_ACCOUNT_ID];
    //     account_collection->add_member(account);

    //     cout << "user add" << endl;
    //     _request.reply(status_codes::Created);
    // }

    // /**
    //  * @brief Session ID 생성
    //  */
    // else if (filtered_uri == ODATA_SESSION_ID)
    // {

    //     user_name = b.at("UserName").as_string();
    //     password = b.at("Password").as_string();

    //     odata_id = ODATA_ACCOUNT_ID;
    //     odata_id = odata_id + '/' + user_name;

    //     // Check account exist
    //     if (!record_is_exist(odata_id))
    //     {
    //         _request.reply(status_codes::BadRequest);
    //         return;
    //     }

    //     account = (Account *)g_record[odata_id];
    //     // Check password correct
    //     if (account->password != password)
    //     {
    //         _request.reply(status_codes::BadRequest);
    //         return;
    //     }

    //     // TODO 세션 id 생성 필요
    //     string odata_id = ODATA_SESSION_ID;
    //     string token = generate_token(16);
    //     odata_id = odata_id + '/' + token;
    //     Session *session = new Session(odata_id, token, account);
    //     session->start();

    //     http_response response(status_codes::Created);
    //     response.headers().add("X-Auth-Token", token);
    //     response.headers().add("Location", session->odata.id);
    //     response.set_body(json::value::object());
    //     session->save_json();
    //     _request.reply(response);
    //     return;
    // }
    // redfish system action 수행

    //  else if (filtered_uri == ODATA_SYSTEM_ID)
    // {
    //     string s_reset_type;
    //     string s_hostname;
    //     string action;

    //     json::value temp;
    //     Systems *systems = (Systems *)g_record[ODATA_SYSTEM_ID];
    //     temp = b.at("ResetType");
    //     //무조건 ResetType이 있어야함
    //     if (temp == NULL)
    //     {
    //         _request.reply(status_codes::BadRequest);
    //         return;
    //     }
    //     action= b.at("ResetType").as_string();
    //     auto it = find(systems->actions.actions.begin(),
    //     systems->actions.actions.end(), action); if (it ==
    //     systems->actions.actions.end())
    //     {
    //         log(info)<<"request not offer command";
    //         _request.reply(status_codes::BadRequest);
    //         return;
    //     }
    //     else
    //     {
    //         if(action=="On"||action=="ForceOn")
    //         {
    //             //미구현상태
    //             redfish_power_ctrl=1;
    //         }
    //         else if(action=="Off"||action=="ForceOff"||action
    //         =="GracefulShutdown")
    //         {
    //             //미구현상태
    //             redfish_power_ctrl=0;
    //         }
    //         else if(action=="ForceRestart"||action=="Reset"||action
    //         =="GracefulRestart")
    //         {
    //             //미구현상태
    //             redfish_power_ctrl=1;
    //         }
    //         //cout << num << "는 존재하며 인덱스는 " << it - v.begin() << "
    //         입니다.\n";
    //     }

    // }
  } catch (json::json_exception &e) {
    log(info) << "json_Exeception error ";
    _request.reply(status_codes::BadRequest);
    return;
  }

  _request.reply(response);
  return;
  // _request.reply(status_codes::BadRequest);
}

void Handler::handle_options(http_request _request) {
  log(info) << "Request method: OPTIONS";
  string uri = _request.request_uri().to_string();
  vector<string> uri_tokens = string_split(uri, '/');
  string filtered_uri = make_path(uri_tokens);
  log(info) << "Request URI : " << filtered_uri;
  log(info) << "Request Body : " << _request.to_string();

  cout << "handle_options request" << endl;

  http_response response(status_codes::OK);
  response.headers().add("Access-Control-Allow-Origin", "*");
  response.headers().add("Access-Control-Allow-Credentials", "true");
  response.headers().add("Access-Control-Allow-Methods",
                         "POST, GET, OPTIONS, DELETE, PATCH");
  response.headers().add("Access-Control-Max-Age", "3600");
  response.headers().add(
      "Access-Control-Allow-Headers",
      "Content-Type, Accept, X-Requested-With, remember-me, X-Auth-Token");
  response.headers().add("Access-Control-Expose-Headers",
                         "X-Auth-Token, Location");
  _request.reply(response);
  return;
}

// http_response basic_response(status_codes _status)
// {
//     http_response response(_status);
//     response.headers().add("")

//     return response;
// }

unsigned int generate_task_resource(string _method, string _uri,
                                    json::value _jv, http_headers _header) {
  unsigned int task_number = allocate_numset_num(ALLOCATE_TASK_NUM);

  string task_odata = ODATA_TASK_ID;
  task_odata = task_odata + "/" + to_string(task_number);
  cout << "Task Resource in Generate_Task_Resource : " << task_odata << endl;

  Task *task = new Task(task_odata, to_string(task_number));
  Collection *col = (Collection *)g_record[ODATA_TASK_ID];
  col->add_member(task);

  // task->start_time =
  task->set_payload(_header, _method, _jv, _uri);
  resource_save_json(task);
  resource_save_json(col);

  return task_number;
}

void complete_task_resource(unsigned int _num) {
  string task_odata = ODATA_TASK_ID;
  task_odata = task_odata + "/" + to_string(_num);
  ((Task *)g_record[task_odata])->end_time = currentDateTime();
  ((Task *)g_record[task_odata])->task_state = TASK_STATE_COMPLETED;
  resource_save_json(g_record[task_odata]);
  return;
}

// void treat_get(http_request _request, http_response& _response)
// {
//     string uri = _request.request_uri().to_string();
//     json::value jv = _request.extract_json().get();
//     unsigned int task_number;

//     task_number = generate_task_resource("GET", uri, jv, _request.headers());

//     _response.headers().add("Access-Control-Allow-Origin", "*");
//     _response.headers().add("Access-Control-Allow-Credentials", "true");
//     _response.headers().add("Access-Control-Allow-Methods", "POST, GET,
//     OPTIONS, DELETE, PATCH");
//     _response.headers().add("Access-Control-Max-Age", "3600");
//     _response.headers().add("Access-Control-Allow-Headers", "Content-Type,
//     Accept, X-Requested-With, remember-me, X-Auth-Token");
//     _response.headers().add("Access-Control-Expose-Headers", "X-Auth-Token,
//     Location");

//     // #1 헤더에 X-Auth-Token있는지
//     // if(!_request.headers().has("X-Auth-Token"))
//     // {
//     //
//     _response.set_status_code(status_codes::NetworkAuthenticationRequired);
//     // }
//     // // #2 X토큰에 있는 값의 세션이 유효한지
//     // else if(!is_session_valid(_request.headers()["X-Auth-Token"]))
//     // {
//     //     _response.set_status_code(status_codes::Unauthorized);
//     // }
//     // #3 uri로 들어온 레코드가 존재하는지
//     /*else*/ if(!record_is_exist(uri))
//     {
//         _response.set_status_code(status_codes::NotFound);
//     }
//     // #4 모두 통과하면 해당 레코드 정보 GET
//     else
//     {
//         _response.set_status_code(status_codes::OK);
//         _response.set_body(record_get_json(uri));
//     }

//     complete_task_resource(task_number);
//     return ;

//     // task리소스 마무리부분 넣고 response생성해서 return; cors도 여기서
//     집어넣자
// }

void treat_post(http_request _request, json::value _jv,
                http_response &_response) {
  string uri = _request.request_uri().to_string();

  // uri분리 액션두있네 생각해보니
  string minus_one = get_parent_object_uri(uri, "/");
  cout << "MINUS_ONE : " << minus_one << endl;

  string minus_one_last = get_current_object_name(minus_one, "/");
  if (minus_one_last == "Actions") {
    do_actions(_request, _jv, _response);
    return;
  } // Action

  if (uri == ODATA_ACCOUNT_ID) {
    make_account(_jv, _response);
    return;
  } else if (uri == ODATA_SESSION_ID) {
    make_session(_jv, _response);
    return;
  }
  // else if(uri == ODATA_ROLE_ID)
  // {
  //     // make_role(_request, _jv, _response);
  //     return ;
  // }
  else if (uri == ODATA_EVENT_DESTINATION_ID) {
    make_subscription(_jv, _response);
    return;
  }

  error_reply(get_error_json("URI Input Error in treat POST"),
              status_codes::NotImplemented, _response);
  return;
}

void do_actions(http_request _request, json::value _jv,
                http_response &_response) {
  string uri = _request.request_uri().to_string();
  string resource_uri =
      get_parent_object_uri(get_parent_object_uri(uri, "/"), "/");
  string action_uri = get_current_object_name(uri, "/");

  cout << "Full uri : " << uri << endl;
  cout << "Resource : " << resource_uri << endl;
  cout << "Action : " << action_uri << endl;

  if (!record_is_exist(resource_uri)) {
    error_reply(get_error_json("URI Input Error in Resource part"),
                status_codes::BadRequest, _response);
    return;
  }

  string action_by, action_what;
  action_by = get_parent_object_uri(action_uri, ".");
  action_what = get_current_object_name(action_uri, ".");
  cout << "By : " << action_by << endl;
  cout << "What : " << action_what << endl;

  if (action_by == "Bios") {
    // _msg = act_bios(_request, _msg, _jv, resource_uri, action_what);
    // return _msg;
  } else if (action_by == "Certificate") {
    act_certificate(_jv, resource_uri, action_what, _response);
    return;
  } else if (action_by == "CertificateService") {
    act_certificate_service(_jv, resource_uri, action_what, _response);
    return;
  } else if (action_by == "ComputerSystem") {
    act_system(_jv, resource_uri, action_what, _response);
    return;
  } else if (action_by == "Chassis") {
  } else if (action_by == "Manager") {
  } else if (action_by == "EventService") {
    act_eventservice(_jv, resource_uri, action_what, _response);
    return;
  } else if (action_by == "LogService") {
    act_logservice(_jv, resource_uri, action_what, _response);
    return;
  } else if (action_by == "UpdateService") {
  } else if (action_by == "VirtualMedia") {
    act_virtualmedia(_jv, resource_uri, action_what, _response);
    return;
  } else {
    error_reply(get_error_json("URI Input Error in action_by check"),
                status_codes::BadRequest, _response);
    return;
  }

  // action_by로 캐스팅이 안돼서 action_by로 그냥 액션있는리소스들중에 뭔지
  // 비교일일이 해야함 그렇게 해서 리소스찾고 거기에 actions맵 에
  // actions[actions_what]으로 하면 해당 Actions구조체 접근가능 액션왓으로 맵에
  // 접근까지 되면 맞는 uri긴 하겟네 굳이 target하고inputuri랑
  // 비교안해도될거같고 찾으면 함수호출 ㅇㅋ

  error_reply(get_error_json("오면안되는곳"), status_codes::BadRequest,
              _response);
  return;
}

void act_certificate(json::value _jv, string _resource, string _what,
                     http_response &_response) {
  Certificate *certi = (Certificate *)g_record[_resource];
  if (certi->actions.find(_what) == certi->actions.end()) {
    error_reply(get_error_json("No Action in Certificate"),
                status_codes::BadRequest, _response);
    return;
  }

  if (_what == "Rekey") {
    json::value result;
    result = certi->Rekey(_jv);
    if (result == json::value::null()) {
      error_reply(get_error_json("Request Body error in Certificate Rekey"),
                  status_codes::BadRequest, _response);
      return;
    } else {
      json::value check;
      if (get_value_from_json_key(result, "Failed", check)) {
        error_reply(result, status_codes::BadRequest, _response);
        return;
      } else if (get_value_from_json_key(result, "CertificateCollection",
                                         check)) {

        success_reply(result, status_codes::OK, _response);
        return;
      }
    }
    // rsp에 Failed가 있으면 실패
    // rsp에 CertificatieCollection이 있으면 성공
  } else if (_what == "Renew") {
    json::value result;
    result = certi->Renew();
    if (result == json::value::null()) {
      error_reply(get_error_json("No Files Error in Certificate ReNew"),
                  status_codes::BadRequest, _response);
      return;
    } else {
      json::value check;
      if (get_value_from_json_key(result, "Failed", check)) {
        error_reply(result, status_codes::BadRequest, _response);
        return;
      } else if (get_value_from_json_key(result, "CertificateCollection",
                                         check)) {
        success_reply(result, status_codes::OK, _response);
        return;
      }
    }
  }
}

void act_certificate_service(json::value _jv, string _resource, string _what,
                             http_response &_response) {
  CertificateService *cert_service = (CertificateService *)g_record[_resource];
  if (cert_service->actions.find(_what) == cert_service->actions.end()) {
    error_reply(get_error_json("No Action in Certificate Service"),
                status_codes::BadRequest, _response);
    return;
  }

  if (_what == "GenerateCSR") {
    // json받아서 json으로 나옴
    json::value result;
    cert_service->GenerateCSR(_jv);
    json::value check;
    if (get_value_from_json_key(result, "Failed", check)) {
      error_reply(result, status_codes::BadRequest, _response);
      return;
    } else if (get_value_from_json_key(result, "CertificateCollection",
                                       check)) {
      success_reply(result, status_codes::OK, _response);
      return;
    }

  } else if (_what == "ReplaceCertificate") {
    if (!cert_service->ReplaceCertificate(_jv)) {
      error_reply(get_error_json("Problem occur in ReplaceCertificate()"),
                  status_codes::BadRequest, _response);
      return;
    }

    success_reply(json::value::null(), status_codes::OK, _response);
    return;
  }
}

void act_system(json::value _jv, string _resource, string _what,
                http_response &_response) {
  Systems *system = (Systems *)g_record[_resource];
  if (system->actions.find(_what) == system->actions.end()) {
    error_reply(get_error_json("No Action in ComputerSystem"),
                status_codes::BadRequest, _response);
    return;
  }

  if (!(system->Reset(_jv))) {
    error_reply(get_error_json("Problem occur in Reset()"),
                status_codes::BadRequest, _response);
    return;
  }

  success_reply(json::value::null(), status_codes::OK, _response);
  return;
}

void act_eventservice(json::value _jv, string _resource, string _what,
                      http_response &_response) {
  EventService *ev_service = (EventService *)g_record[_resource];
  if (ev_service->actions.find(_what) == ev_service->actions.end()) {
    error_reply(get_error_json("No Action in EventService"),
                status_codes::BadRequest, _response);
    return;
  }

  json::value result;
  result = ev_service->SubmitTestEvent(_jv);
  if (result == json::value::null()) {
    error_reply(get_error_json("Need Correct Request Body"),
                status_codes::BadRequest, _response);
    return;
  }

  success_reply(result, status_codes::OK, _response);
  return;
}

void act_logservice(json::value _jv, string _resource, string _what,
                    http_response &_response) {
  LogService *log_service = (LogService *)g_record[_resource];
  if (log_service->actions.find(_what) == log_service->actions.end()) {
    error_reply(get_error_json("No Action in LogService"),
                status_codes::BadRequest, _response);
    return;
  }

  if (!(log_service->ClearLog())) {
    error_reply(get_error_json("Problem occur in ClearLog()"),
                status_codes::BadRequest, _response);
    return;
  }

  success_reply(json::value::null(), status_codes::OK, _response);
  return;
}

void act_virtualmedia(json::value _jv, string _resource, string _what,
                      http_response &_response) {
  // 얘는 리소스가 무조건 virtualmedia가 아니라 insert면 virtual컬렉션 eject면
  // virtualmedia리소스임 그래서 리소스부터 따고 들어갈수가없네 _what부터
  // 구분해야겟네

  if (!(_what == "InsertMediaCD" || _what == "InsertMediaUSB" ||
        _what == "EjectMedia")) {
    error_reply(get_error_json("No Action in VirtualMedia"),
                status_codes::BadRequest, _response);
    return;
  }

  json::value result_value;
  VirtualMedia *vm = nullptr;
  unsigned int id_num;
  string type;

  if (_what == "EjectMedia") {
    VirtualMedia *v_media = (VirtualMedia *)g_record[_resource];
    result_value = v_media->EjectMedia();
  } else {
    string odata = _resource;

    if (_what == "InsertMediaCD") {
      id_num = allocate_numset_num(ALLOCATE_VM_CD_NUM);
      type = "CD";
      odata = odata + "/CD" + to_string(id_num);
      VirtualMedia *v_media = new VirtualMedia(odata);
      v_media->media_type.push_back("CD");
      vm = v_media;
      vm->id = "CD" + to_string(id_num);
    } else if (_what == "InsertMediaUSB") {
      id_num = allocate_numset_num(ALLOCATE_VM_USB_NUM);
      type = "USB";
      odata = odata + "/USB" + to_string(id_num);
      VirtualMedia *v_media = new VirtualMedia(odata);
      v_media->media_type.push_back("USBStick");
      vm = v_media;
      vm->id = "USB" + to_string(id_num);
    }

    vm->name = "VirtualMedia";
    vm->connected_via = "URI";
    vm->inserted = false;
    vm->write_protected = true;

    result_value = vm->InsertMedia(_jv);
  }

  if (result_value == json::value::null()) {
    // error
    if (vm != nullptr) {
      // insert에 실패하면 만들었던 리소스도 다시 지워줘야함
      delete (vm);
      if (type == "CD")
        delete_numset_num(ALLOCATE_VM_CD_NUM, id_num);
      else if (type == "USB")
        delete_numset_num(ALLOCATE_VM_USB_NUM, id_num);
    }
    error_reply(get_error_json("Problem Occur in VirtualMedia Actions"),
                status_codes::BadRequest, _response);
    return;
  } else {
    // success
    if (vm != nullptr) {
      Collection *col = (Collection *)g_record[_resource];
      col->add_member(vm);
      resource_save_json(col);
      resource_save_json(vm);
    }

    success_reply(result_value, status_codes::OK, _response);
    return;
  }
}

void make_account(json::value _jv, http_response &_response) {
  string user_name;
  string password;
  string odata_id;
  string role_id = "ReadOnly";
  Account *account;
  bool enabled = true;

  if (((AccountService *)g_record[ODATA_ACCOUNT_SERVICE_ID])->service_enabled ==
      false) {
    error_reply(get_error_json("Account Service isn't Enabled"),
                status_codes::BadRequest, _response);
    return;
  }

  if (get_value_from_json_key(_jv, "UserName", user_name) == false ||
      get_value_from_json_key(_jv, "Password", password) == false) {
    error_reply(get_error_json("No UserName or Password"),
                status_codes::BadRequest, _response);
    return;
  } // json request body에 UserName, Password없으면 BadRequest

  unsigned int min_pass_length =
      ((AccountService *)g_record[ODATA_ACCOUNT_SERVICE_ID])
          ->min_password_length;
  unsigned int max_pass_length =
      ((AccountService *)g_record[ODATA_ACCOUNT_SERVICE_ID])
          ->max_password_length;
  if (password.size() < min_pass_length) {
    error_reply(get_error_json("Password length must be at least " +
                               to_string(min_pass_length)),
                status_codes::BadRequest, _response);
    return;
  } // password 길이가 짧으면 BadRequest

  if (password.size() > max_pass_length) {
    error_reply(get_error_json("Password length must not exceed " +
                               to_string(max_pass_length)),
                status_codes::BadRequest, _response);
    return;
  } // password 길이가 길면 BadRequest

  // Collection col = *((Collection *)g_record[ODATA_ACCOUNT_ID]); // 이건 왜
  // 터졌지?
  Collection *col = (Collection *)g_record[ODATA_ACCOUNT_ID];
  std::vector<Resource *>::iterator iter;
  for (iter = col->members.begin(); iter != col->members.end(); iter++) {
    if (((Account *)(*iter))->user_name == user_name) {
      error_reply(get_error_json("UserName already exists"),
                  status_codes::Conflict, _response);
      return;
    }
  } // username이 이미 있으면 Conflict

  if (_jv.as_object().find("RoleId") != _jv.as_object().end()) {
    odata_id = ODATA_ROLE_ID;
    role_id = _jv.at("RoleId").as_string();
    odata_id = odata_id + '/' + role_id;
    // Check role exist
    if (!record_is_exist(odata_id)) {
      error_reply(get_error_json("No Role"), status_codes::BadRequest,
                  _response);
      return;
    }
  }

  if (_jv.as_object().find("Enabled") != _jv.as_object().end())
    enabled = _jv.at("Enabled").as_bool();

  odata_id = ODATA_ACCOUNT_ID;
  string acc_id = to_string(allocate_numset_num(ALLOCATE_ACCOUNT_NUM));
  odata_id = odata_id + "/" + acc_id;
  account = new Account(odata_id, acc_id, role_id);
  account->name = "User Account";
  account->user_name = user_name;
  account->password = password;
  account->enabled = enabled;
  account->locked = false;

  // 컬렉션에 add
  ((Collection *)g_record[ODATA_ACCOUNT_ID])->add_member(account);
  resource_save_json(account);
  resource_save_json(g_record[ODATA_ACCOUNT_ID]);

  _response.set_status_code(status_codes::Created);
  _response.set_body(record_get_json(odata_id));

  return;
}

void make_session(json::value _jv, http_response &_response) {
  string user_name;
  string password;
  string odata_id;
  Account *account;

  if (((SessionService *)g_record[ODATA_SESSION_SERVICE_ID])->service_enabled ==
      false) {
    error_reply(get_error_json("Session Service isn't Enabled"),
                status_codes::BadRequest, _response);
    return;
  }

  if (get_value_from_json_key(_jv, "UserName", user_name) == false ||
      get_value_from_json_key(_jv, "Password", password) == false) {
    error_reply(get_error_json("No UserName or Password"),
                status_codes::BadRequest, _response);
    return;
  } // json request body에 UserName, Password없으면 BadRequest

  Collection *col = (Collection *)g_record[ODATA_ACCOUNT_ID];
  std::vector<Resource *>::iterator iter;
  bool exist = false;
  for (iter = col->members.begin(); iter != col->members.end(); iter++) {
    if (((Account *)(*iter))->user_name == user_name) {
      // 계정존재
      exist = true;
      account = ((Account *)(*iter));
      break;
    }
  }

  if (!exist) {
    error_reply(get_error_json("No Account using that UserName"),
                status_codes::BadRequest, _response);
    return;
  } // 입력한 username에 해당하는 계정없음

  if (account->password != password) {
    error_reply(get_error_json("Password does not match"),
                status_codes::BadRequest, _response);
    return;
  } // 비밀번호 맞지않음

  odata_id = ODATA_SESSION_ID;
  string token = generate_token(16);
  string session_id = to_string(allocate_numset_num(ALLOCATE_SESSION_NUM));
  odata_id = odata_id + '/' + session_id;
  Session *session = new Session(odata_id, session_id, account);
  session->x_token = token;

  ((Collection *)g_record[ODATA_SESSION_ID])->add_member(session);
  session->start();
  resource_save_json(session);
  resource_save_json(g_record[ODATA_SESSION_ID]);

  _response.set_status_code(status_codes::Created);
  _response.headers().add("X-Auth-Token", token);
  _response.headers().add("Location", session->odata.id);

  return;
}

void make_subscription(json::value _jv, http_response &_response) {
  // todo : protocol, policy, subs_type은 들어갈수있는값이 정해져있어서
  // 검사하는것 추가하기
  string uri = ODATA_EVENT_DESTINATION_ID;
  string service_uri = get_parent_object_uri(uri, "/");

  // 필수요소 destination, protocol 존재여부 검사
  string destination;
  if (!(get_value_from_json_key(_jv, "Destination", destination))) {
    error_reply(get_error_json("Need Destination in Request Body"),
                status_codes::BadRequest, _response);
    return;
  }

  string protocol;
  if (!(get_value_from_json_key(_jv, "Protocol", protocol))) {
    error_reply(get_error_json("Need Protocol in Request Body"),
                status_codes::BadRequest, _response);
    return;
  }

  // 나머지 멤버변수들 처리
  string policy, context, subs_type;

  if (!(get_value_from_json_key(_jv, "Context", context)))
    context = "";

  if (get_value_from_json_key(_jv, "DeliveryRetryPolicy", policy)) {
    // enum 검사

  } else
    policy = "SuspendRetries";

  if (get_value_from_json_key(_jv, "SubscriptionType", subs_type)) {
    // enum 검사
  } else
    subs_type = "RedfishEvent";

  // EventDestination 생성
  unsigned int sub_num = allocate_numset_num(ALLOCATE_SUBSCRIPTION_NUM);
  EventService *service = (EventService *)g_record[service_uri];
  string odata_id = uri + "/" + to_string(sub_num);

  // record_is_exist검사... 는 있으면 안되는 구조긴 함 allocate를 한거라서
  // 그래서 뺌

  EventDestination *event_dest =
      new EventDestination(odata_id, to_string(sub_num));

  event_dest->destination = destination;
  event_dest->protocol = protocol;
  event_dest->context = context;
  event_dest->subscription_type = subs_type;
  event_dest->delivery_retry_policy = policy;
  event_dest->status.state = STATUS_STATE_ENABLED;
  event_dest->status.health = STATUS_HEALTH_OK;
  event_dest->name = "Event Subscription";

  service->subscriptions->add_member(event_dest);

  resource_save_json(event_dest);
  resource_save_json(service->subscriptions);

  success_reply(record_get_json(event_dest->odata.id), status_codes::Created,
                _response);
  return;
}

void treat_patch(http_request _request, json::value _jv,
                 http_response &_response) {
  string uri = _request.request_uri().to_string();
  vector<string> uri_tokens = string_split(uri, '/');
  string uri_part;
  for (int i = 0; i < uri_tokens.size(); i++) {
    if (i == 3)
      break;

    uri_part += "/";
    uri_part += uri_tokens[i];
  }
  // uri_part에는 /redfish/v1/something 까지만

  string minus_one = get_parent_object_uri(uri, "/");
  cout << "MINUS_ONE INFO : " << minus_one << endl;

  if (uri_part == ODATA_ACCOUNT_SERVICE_ID) {
    // /redfish/v1/AccountService 처리
    if (uri == ODATA_ACCOUNT_SERVICE_ID) {
      if (patch_account_service(_jv, ODATA_ACCOUNT_SERVICE_ID)) {
        resource_save_json(g_record[uri]);
        success_reply(record_get_json(uri), status_codes::OK, _response);
        return;
      } else {
        error_reply(get_error_json("Error Occur in AccountService PATCH"),
                    status_codes::BadRequest, _response);
        return;
      }
    }

    // /redfish/v1/AccountService/Accounts/[Account_id]
    if (minus_one == ODATA_ACCOUNT_ID) {
      modify_account(_request, _jv, uri, _response);
      return;
    }
    // /redfish/v1/AccountService/Roles/[Role_id]
    else if (minus_one == ODATA_ROLE_ID) {
      modify_role(_request, _jv, uri, _response);
      return;
    }
  }

  else if (uri_part == ODATA_SESSION_SERVICE_ID) {
    // /redfish/v1/SessionService 처리
    if (uri == ODATA_SESSION_SERVICE_ID) {
      if (patch_session_service(_jv)) {
        resource_save_json(g_record[uri]);
        success_reply(record_get_json(uri), status_codes::OK, _response);
        return;
      } else {
        error_reply(get_error_json("Error Occur in SessionService PATCH"),
                    status_codes::BadRequest, _response);
        return;
      }
    }
  }

  else if (uri_part == ODATA_MANAGER_ID) {
    string bmc_manager = ODATA_MANAGER_ID;

    // /redfish/v1/Managers
    if (uri == bmc_manager) {
      if (patch_manager(_jv, uri)) {
        resource_save_json(g_record[uri]);
        success_reply(record_get_json(uri), status_codes::OK, _response);
      } else
        error_reply(get_error_json("Error Occur in Manager PATCH"),
                    status_codes::BadRequest, _response);
      return;
    }

    // /redfish/v1/Managers/NetworkProtocol
    if (uri == bmc_manager + "/NetworkProtocol") {
      if (!record_is_exist(uri)) {
        error_reply(get_error_json("No NetworkProtocol"),
                    status_codes::BadRequest, _response);
        return;
      }

      if (patch_network_protocol(_jv, uri)) {
        NetworkProtocol *net = (NetworkProtocol *)g_record[uri];
        resource_save_json(g_record[uri]);
        success_reply(record_get_json(uri), status_codes::OK, _response);
        patch_iptable(net);
        return;
      } else {
        error_reply(get_error_json("Error Occur in NetworkProtocol PATCH"),
                    status_codes::BadRequest, _response);
        return;
      }
    }

    // /redfish/v1/Managers/FanMode
    if (uri == bmc_manager + "/FanMode") {
      string mode;
      if (_jv == json::value::null()) {
        error_reply(get_error_json("Json Body is Null"),
                    status_codes::BadRequest, _response);
        return;
      }

      if (get_value_from_json_key(_jv, "Mode", mode)) {
        if (!(mode == "Standard" || mode == "FullSpeed" || mode == "Auto")) {
          error_reply(get_error_json("Incorrect Mode"),
                      status_codes::BadRequest, _response);
          return;
        }
      } else {
        error_reply(get_error_json("No Mode in Json Body"),
                    status_codes::BadRequest, _response);
        return;
      }

      patch_fan_mode(mode, uri);

      success_reply(json::value::null(), status_codes::OK, _response);
      return;
    }

    // /redfish/v1/Managers/EthernetInterfaces/[Ethernet_id]
    if (minus_one == bmc_manager + "/EthernetInterfaces") {
      // Manager eth 뿐만 아니라 System eth까지 검사해야됨(같이 수정됨)
      string sys_uri = ODATA_SYSTEM_ID;
      string eth_num = get_current_object_name(uri, "/");
      sys_uri = sys_uri + "/EthernetInterfaces";
      sys_uri = sys_uri + "/" + eth_num;
      if (!record_is_exist(uri) || !record_is_exist(sys_uri)) {
        error_reply(get_error_json("No EthernetInterface"),
                    status_codes::BadRequest, _response);
        return;
      }

      if (patch_ethernet_interface(_jv, uri, 1) &&
          patch_ethernet_interface(_jv, sys_uri, 0)) {
        resource_save_json(g_record[uri]);
        resource_save_json(g_record[sys_uri]);
        success_reply(record_get_json(uri), status_codes::OK, _response);
        return;
      } else {
        error_reply(get_error_json("Error Occur in EthernetInterface PATCH"),
                    status_codes::BadRequest, _response);
        return;
      }
    }

  }

  else if (uri_part == ODATA_SYSTEM_ID) {
    string bmc_system = ODATA_SYSTEM_ID;

    if (uri == bmc_system) {
      if (patch_system(_jv, uri)) {
        resource_save_json(g_record[uri]);
        success_reply(record_get_json(uri), status_codes::OK, _response);
      } else
        error_reply(get_error_json("Error Occur in System PATCH"),
                    status_codes::BadRequest, _response);
      return;
    }
  }

  else if (uri_part == ODATA_CHASSIS_ID) {
    string bmc_chassis = ODATA_CHASSIS_ID;

    if (uri == bmc_chassis) {
      if (patch_chassis(_jv, uri)) {
        resource_save_json(g_record[uri]);
        success_reply(record_get_json(uri), status_codes::OK, _response);
      } else
        error_reply(get_error_json("Error Occur in Chassis PATCH"),
                    status_codes::BadRequest, _response);
      return;
    }

    if (minus_one == bmc_chassis + "/Power/PowerControl") {
      if (!record_is_exist(uri)) {
        error_reply(get_error_json("No PowerControl"), status_codes::BadRequest,
                    _response);
        return;
      }

      if (patch_power_control(_jv, uri)) {
        resource_save_json(g_record[uri]);
        success_reply(record_get_json(uri), status_codes::OK, _response);
      } else
        error_reply(get_error_json("Error Occur in PowerControl PATCH"),
                    status_codes::BadRequest, _response);
      return;
    }
  }

  else if (uri_part == ODATA_EVENT_SERVICE_ID) {
    if (uri == ODATA_EVENT_SERVICE_ID) {
      if (patch_event_service(_jv, uri))
        success_reply(record_get_json(uri), status_codes::OK, _response);
      else
        error_reply(get_error_json("Error Occur in EventService PATCH"),
                    status_codes::BadRequest, _response);

      return;
    }

    if (minus_one == ODATA_EVENT_DESTINATION_ID) {
      // sub/[id] 패치
      if (!record_is_exist(uri)) {
        error_reply(get_error_json("No Subscription"), status_codes::BadRequest,
                    _response);
        return;
      }

      if (patch_subscription(_jv, uri))
        success_reply(record_get_json(uri), status_codes::OK, _response);
      else
        error_reply(get_error_json("Error Occur in Subscription PATCH"),
                    status_codes::BadRequest, _response);

      return;
    }

  } else {
    error_reply(get_error_json("URI Input Error in Whole part"),
                status_codes::BadRequest, _response);
    return;
  }

  error_reply(get_error_json("URI Input Error in Below part"),
              status_codes::BadRequest, _response);
  return;
}

void modify_account(http_request _request, json::value _jv, string _uri,
                    http_response &_response) {
  if (((AccountService *)g_record[ODATA_ACCOUNT_SERVICE_ID])->service_enabled ==
      false) {
    error_reply(get_error_json("Account Service isn't Enabled"),
                status_codes::BadRequest, _response);
    return;
  }

  if (!(record_is_exist(_uri))) {
    error_reply(get_error_json("No Account"), status_codes::BadRequest,
                _response);
    return;
  }

  if (!_request.headers().has("X-Auth-Token")) {
    error_reply(get_error_json("Login Required"), status_codes::BadRequest,
                _response);
    return;
  }

  string session_token = _request.headers()["X-Auth-Token"];

  if (!is_session_valid(session_token)) {
    error_reply(get_error_json("Session Unvalid"), status_codes::BadRequest,
                _response);
    return;
  }

  string session_uri = get_session_odata_id_by_token(session_token);
  // session_uri = session_uri + "/" + session_token;
  cout << "SESSION ! : " << session_uri << endl;

  // Session session = *((Session *)g_record[session_uri]);
  Session *session = (Session *)g_record[session_uri];

  string last = get_current_object_name(_uri, "/"); // account_id임 last는

  bool op_role = false, op_name = false,
       op_pass = false; // 일괄처리를 위한 플래그
  string role_odata;
  string input_username;
  string input_password;
  Ipmiuser *user = NULL;

  if (session->account->role->id == "Administrator") {
    // 관리자만 role 변경가능하게 설계해봄
    string role_id;
    if (get_value_from_json_key(_jv, "RoleId", role_id)) {
      role_odata = ODATA_ROLE_ID;
      role_odata = role_odata + "/" + role_id;

      if (!record_is_exist(role_odata)) {
        error_reply(get_error_json(role_id + " does not exist"),
                    status_codes::BadRequest, _response);
        return;
      }
      op_role = true;
    }
  }

  if (session->account->role->id == "Administrator" ||
      session->account->id == last) {

    for (int i = 0; i < get_user_cnt(); i++) {
      user = &ipmiUser[i];
      if (session->account->user_name == user->getUsername()) {
        cout << "ipmiuser redfish user find" << endl;
        break;
      }
    }
    // username, password 변경
    if (get_value_from_json_key(_jv, "UserName", input_username)) {
      Collection *col = (Collection *)g_record[ODATA_ACCOUNT_ID];
      std::vector<Resource *>::iterator iter;
      for (iter = col->members.begin(); iter != col->members.end(); iter++) {
        if (((Account *)(*iter))->id == last)
          continue;

        if (((Account *)(*iter))->user_name == input_username) {
          error_reply(get_error_json("UserName is already in use"),
                      status_codes::BadRequest, _response);
          return;
        }

      } // user_name 중복검사

      op_name = true;
    }

    if (get_value_from_json_key(_jv, "Password", input_password)) {
      unsigned int min = ((AccountService *)g_record[ODATA_ACCOUNT_SERVICE_ID])
                             ->min_password_length;
      unsigned int max = ((AccountService *)g_record[ODATA_ACCOUNT_SERVICE_ID])
                             ->max_password_length;
      if (input_password.length() < min || input_password.length() > max) {
        error_reply(get_error_json("Password Range is " + to_string(min) +
                                   " ~ " + to_string(max)),
                    status_codes::BadRequest, _response);
        return;
      } // password 길이 검사
      op_pass = true;
    }
  }
  // role_id로 하기엔 role을 생성할 수도 있는 거여서 나중에 role안에
  // privileges에 권한 확인하는걸로 바꿔야 할 수도있음

  cout << record_get_json(_uri) << endl;

  if (!op_role && !op_name && !op_pass) // 다 false면
  {
    error_reply(get_error_json("Cannot Modify Account"),
                status_codes::BadRequest, _response);
    return;
  }
  if (user == NULL) {
    cout << "error ipmierror account modifiy" << endl;
  }
  if (op_role) {
    ((Account *)g_record[_uri])->role = ((Role *)g_record[role_odata]);
    ((Account *)g_record[_uri])->role_id =
        ((Role *)g_record[role_odata])->odata.id;
  }

  if (op_name) {
    ((Account *)g_record[_uri])->user_name = input_username;
    user->setUserName(input_username);
  }

  if (op_pass)
    ((Account *)g_record[_uri])->password = input_password;
  user->setUserName(input_password);
  // 검사중 error나면 적용안되고 error나지 않을때 변경일괄처리
  // plat_user_save();
  resource_save_json(g_record[_uri]);
  cout << record_get_json(_uri) << endl;
  cout << record_get_json(session->account->odata.id) << endl;

  success_reply(record_get_json(_uri), status_codes::OK, _response);
  return;
}

void modify_role(http_request _request, json::value _jv, string _uri,
                 http_response &_response) {
  string service_id =
      get_parent_object_uri(get_parent_object_uri(_uri, "/"), "/");
  if (((AccountService *)g_record[service_id])->service_enabled == false) {
    error_reply(get_error_json("Account Service isn't Enabled"),
                status_codes::BadRequest, _response);
    return;
  }

  if (!(record_is_exist(_uri))) {
    error_reply(get_error_json("No Role"), status_codes::BadRequest, _response);
    return;
  }

  if (!_request.headers().has("X-Auth-Token")) {
    error_reply(get_error_json("Login Required"), status_codes::BadRequest,
                _response);
    return;
  }

  string session_token = _request.headers()["X-Auth-Token"];

  if (!is_session_valid(session_token)) {
    error_reply(get_error_json("Session Unvalid"), status_codes::BadRequest,
                _response);
    return;
  }

  string session_uri = get_session_odata_id_by_token(session_token);
  // session_uri = session_uri + "/" + session_token;
  cout << "SESSION ! : " << session_uri << endl;

  Session *session = (Session *)g_record[session_uri];
  Role *login_role = session->account->role;
  Role *target_role = (Role *)g_record[_uri];

  if (login_role->id == "Administrator") {
    json::value arr = json::value::array();
    if (_jv.as_object().find("AssignedPrivileges") != _jv.as_object().end())
      arr = _jv.at("AssignedPrivileges");
    else {
      error_reply(get_error_json("No Privileges"), status_codes::BadRequest,
                  _response);
      return;
    }

    cout << "바뀌기전~~ " << endl;
    cout << record_get_json(_uri) << endl;
    cout << " $$$$$$$ " << endl;

    vector<string> store;
    for (int i = 0; i < arr.size(); i++) {
      string input = arr[i].as_string();
      if (check_role_privileges(input)) {
        bool exist = false;
        for (int j = 0; j < target_role->assigned_privileges.size(); j++) {
          if (input == target_role->assigned_privileges[j]) {
            exist = true;
            break;
          }
        }

        if (!exist)
          store.push_back(input); //모아서 한번에 붙이려고 store에 잠시 보관
        // target_role->assigned_privileges.push_back(input);
        else
          log(info) << "[" << input << "]"
                    << " is already exist";
      } else {
        //올바른 privilege 가 아님!!
        error_reply(get_error_json("Incorrect Previlege"),
                    status_codes::BadRequest, _response);
        return;
      }
    }

    for (int i = 0; i < store.size(); i++) {
      target_role->assigned_privileges.push_back(store[i]);
    }

    cout << "바꾼후~~ " << endl;
    cout << record_get_json(_uri) << endl;
  } else {
    error_reply(get_error_json("Only Administrator Can Use"),
                status_codes::BadRequest, _response);
    return;
  }
  // 일단 assignedprivileges 있는걸 추가하는식으로만 구현해봄 삭제는 보류

  resource_save_json(g_record[_uri]);
  success_reply(record_get_json(_uri), status_codes::OK, _response);
  return;
}

bool patch_account_service(json::value _jv, string _record_uri) {
  cout << "patch_account_service start" << endl;
  cout << record_get_json(_record_uri) << endl;

  bool result = false;

  // Min & Max Password Length
  unsigned int min_after;
  unsigned int min_before =
      ((AccountService *)g_record[_record_uri])->min_password_length;
  if (get_value_from_json_key(_jv, "MinPasswordLength", min_after)) {
    if (min_after == 0) {
      // 이건 minpassword가 입력으로 왔지만 0을 입력했다는걸로 오류
      return false;
    }
    result = true;
  }

  unsigned int max_after;
  unsigned int max_before =
      ((AccountService *)g_record[_record_uri])->max_password_length;
  if (get_value_from_json_key(_jv, "MaxPasswordLength", max_after)) {
    if (max_after == 0) {
      // 이건 maxpassword가 입력으로 왔지만 0을 입력했다는걸로 오류
      return false;
    }
    result = true;
  }

  if (min_after == 0 && max_after == 0) {
    ;
  } else if (min_after == 0 && max_after != 0) {
    if (min_before <= max_after) {
      ((AccountService *)g_record[_record_uri])->max_password_length =
          max_after;
    } else {
      return false;
    }
  } else if (min_after != 0 && max_after == 0) {
    if (min_after <= max_before) {
      ((AccountService *)g_record[_record_uri])->min_password_length =
          min_after;
    } else {
      return false;
    }
  } else {
    if (min_after <= max_after) {
      ((AccountService *)g_record[_record_uri])->max_password_length =
          max_after;
      ((AccountService *)g_record[_record_uri])->min_password_length =
          min_after;
    } else {
      return false;
    }
  }

  // check_password_range(min_before, min_after, max_before, max_after)

  bool service_enabled;
  if (get_value_from_json_key(_jv, "ServiceEnabled", service_enabled)) {
    ((AccountService *)g_record[_record_uri])->service_enabled =
        service_enabled;
    result = true;
  }

  unsigned int aflt;
  if (get_value_from_json_key(_jv, "AuthFailureLoggingThreshold", aflt)) {
    ((AccountService *)g_record[_record_uri])->auth_failure_logging_threshold =
        aflt;
    result = true;
  }

  unsigned int alt;
  if (get_value_from_json_key(_jv, "AccountLockoutThreshold", alt)) {
    ((AccountService *)g_record[_record_uri])->account_lockout_threshold = alt;
    result = true;
  }

  unsigned int ald;
  if (get_value_from_json_key(_jv, "AccountLockoutDuration", ald)) {
    ((AccountService *)g_record[_record_uri])->account_lockout_duration = ald;
    result = true;
  }

  unsigned int alcra;
  if (get_value_from_json_key(_jv, "AccountLockoutCounterResetAfter", alcra)) {
    ((AccountService *)g_record[_record_uri])
        ->account_lockout_counter_reset_after = alcra;
    result = true;
  }

  bool alcre;
  if (get_value_from_json_key(_jv, "AccountLockoutCounterResetEnabled",
                              alcre)) {
    ((AccountService *)g_record[_record_uri])
        ->account_lockout_counter_reset_enabled = alcre;
    result = true;
  }

  cout << record_get_json(_record_uri) << endl;

  return result;
}

bool patch_session_service(json::value _jv) {
  cout << "바뀌기전~~ " << endl;
  cout << record_get_json(ODATA_SESSION_SERVICE_ID) << endl;
  cout << " $$$$$$$ " << endl;

  bool result = false;

  unsigned int change_timeout;
  if (get_value_from_json_key(_jv, "SessionTimeout", change_timeout)) {
    ((SessionService *)g_record[ODATA_SESSION_SERVICE_ID])->session_timeout =
        change_timeout;
    // 타임아웃 시간 변경이 관리자가 할거 같긴한데 일단 권한검사는 추가안함
    result = true;

    Collection *col = (Collection *)g_record[ODATA_SESSION_ID];
    std::vector<Resource *>::iterator iter;
    for (iter = col->members.begin(); iter != col->members.end(); iter++) {
      if (((Session *)(*iter))->_remain_expires_time > change_timeout)
        ((Session *)(*iter))->_remain_expires_time = change_timeout;
    }
    // 타임아웃 시간을 변경한 것이 현재 만들어져있는 세션들에게도 적용되면 이
    // 코드 적용
  }

  bool service_enabled;
  if (get_value_from_json_key(_jv, "ServiceEnabled", service_enabled)) {
    ((SessionService *)g_record[ODATA_SESSION_SERVICE_ID])->service_enabled =
        service_enabled;
    result = true;
  }

  cout << "바꾼후~~ " << endl;
  cout << record_get_json(ODATA_SESSION_SERVICE_ID) << endl;

  return result;
}

bool patch_manager(json::value _jv, string _record_uri) {
  cout << "바뀌기전~~ " << endl;
  cout << record_get_json(_record_uri) << endl;
  cout << " $$$$$$$ " << endl;

  bool result = false;

  string description, datetime, offset;
  if (get_value_from_json_key(_jv, "Description", description)) {
    ((Manager *)g_record[_record_uri])->description = description;
    result = true;
  }

  if (get_value_from_json_key(_jv, "DateTime", datetime)) {
    ((Manager *)g_record[_record_uri])->datetime = datetime;
    result = true;
  }

  if (get_value_from_json_key(_jv, "DateTimeLocalOffset", offset)) {
    ((Manager *)g_record[_record_uri])->datetime_offset = offset;
    result = true;
  }

  cout << "바꾼후~~ " << endl;
  cout << record_get_json(_record_uri) << endl;

  return result;
}

bool patch_network_protocol(json::value _jv, string _record_uri) {
  cout << "바뀌기전~~ " << endl;
  cout << record_get_json(_record_uri) << endl;
  cout << " $$$$$$$ " << endl;

  bool result = false;
  NetworkProtocol *network = (NetworkProtocol *)g_record[_record_uri];

  string description;
  if (get_value_from_json_key(_jv, "Description", description)) {
    network->description = description;
    result = true;
  }

  json::value snmp;
  if (get_value_from_json_key(_jv, "SNMP", snmp)) {
    bool enabled;
    int port;
    if (get_value_from_json_key(snmp, "ProtocolEnabled", enabled)) {
      network->snmp_enabled = enabled;
      result = true;
    }

    if (get_value_from_json_key(snmp, "Port", port)) {
      network->snmp_port = port;
      result = true;
    }
    // 포트번호 범위검사 해줘야하나? QQQQ
  }

  json::value ipmi;
  if (get_value_from_json_key(_jv, "IPMI", ipmi)) {
    bool enabled;
    int port;
    if (get_value_from_json_key(ipmi, "ProtocolEnabled", enabled)) {
      network->ipmi_enabled = enabled;
      result = true;
    }

    if (get_value_from_json_key(ipmi, "Port", port)) {
      network->ipmi_port = port;
      result = true;
    }
  }

  json::value kvmip;
  if (get_value_from_json_key(_jv, "KVMIP", kvmip)) {
    bool enabled;
    int port;
    if (get_value_from_json_key(kvmip, "ProtocolEnabled", enabled)) {
      network->kvmip_enabled = enabled;
      result = true;
    }

    if (get_value_from_json_key(kvmip, "Port", port)) {
      network->kvmip_port = port;
      result = true;
    }
  }

  json::value http;
  if (get_value_from_json_key(_jv, "HTTP", http)) {
    bool enabled;
    int port;
    if (get_value_from_json_key(http, "ProtocolEnabled", enabled)) {
      network->http_enabled = enabled;
      result = true;
    }

    if (get_value_from_json_key(http, "Port", port)) {
      network->http_port = port;
      result = true;
    }
  }

  json::value https;
  if (get_value_from_json_key(_jv, "HTTPS", https)) {
    bool enabled;
    int port;
    if (get_value_from_json_key(https, "ProtocolEnabled", enabled)) {
      network->https_enabled = enabled;
      result = true;
    }

    if (get_value_from_json_key(https, "Port", port)) {
      network->https_port = port;
      result = true;
    }
  }

  json::value ssh;
  if (get_value_from_json_key(_jv, "SSH", ssh)) {
    bool enabled;
    int port;
    if (get_value_from_json_key(ssh, "ProtocolEnabled", enabled)) {
      network->ssh_enabled = enabled;
      result = true;
    }

    if (get_value_from_json_key(ssh, "Port", port)) {
      network->ssh_port = port;
      result = true;
    }
  }

  json::value ntp;
  if (get_value_from_json_key(_jv, "NTP", ntp)) {
    bool enabled;
    int port;
    json::value ntp_servers = json::value::array();
    if (get_value_from_json_key(ntp, "ProtocolEnabled", enabled)) {
      network->ntp_enabled = enabled;
      result = true;
    }

    if (get_value_from_json_key(ntp, "Port", port)) {
      network->ntp_port = port;
      result = true;
    }

    if (get_value_from_json_key(ntp, "NTPServers", ntp_servers)) {
      for (int i = 0; i < ntp_servers.size(); i++) {
        bool add = true;
        for (int j = 0; j < network->v_netservers.size(); j++) {
          if (ntp_servers[i].as_string() == network->v_netservers[j]) {
            add = false;
            break;
          }
        }
        if (add)
          network->v_netservers.push_back(ntp_servers[i].as_string());

        result = true;
      }
    }
    // ntpservers 는 현재 존재하지않으면 추가하는 식으로만 구현
  }

  cout << "바꾼후~~ " << endl;
  cout << record_get_json(_record_uri) << endl;

  return result;
}

void patch_fan_mode(string _mode, string _record_uri) {
  // Fan속도 조절
  // Auto의 경우 Fan과 sensor_num이 같은 Temperature 이용
  string thermal_odata = ODATA_CHASSIS_ID;
  thermal_odata = thermal_odata + "/Thermal";
  // cout << "thermal : " << thermal_odata << endl;
  Thermal *th = (Thermal *)g_record[thermal_odata]; // thermal없는거 에러나려나?
  vector<Resource *> v;
  v = th->fans->members;

  std::vector<Resource *>::iterator fan_iter;
  std::vector<Resource *>::iterator tem_iter;

  for (fan_iter = v.begin(); fan_iter != v.end(); fan_iter++) {
    cout << "바뀌기전~~ " << endl;
    cout << "fan info" << endl;
    cout << record_get_json((*fan_iter)->odata.id) << endl;
    cout << " $$$$$$$ " << endl;

    Fan *fan = (Fan *)*fan_iter;

    if (_mode == "Standard") {
      fan->reading = (fan->max_reading_range) / 2;
    } else if (_mode == "FullSpeed") {
      fan->reading = fan->max_reading_range;
    } else if (_mode == "Auto") {
      // iter
      vector<Resource *> tem_v;
      tem_v = th->temperatures->members;
      for (tem_iter = tem_v.begin(); tem_iter != tem_v.end(); tem_iter++) {
        Temperature *temp = (Temperature *)*tem_iter;
        if (fan->sensor_num == temp->sensor_num) {
          if (temp->reading_celsius < 40) {
            cout << "Memeber : " << temp->member_id << endl;
            cout << "Celcius : " << temp->reading_celsius << endl;
            fan->reading = (fan->max_reading_range) / 2;
          } else {
            cout << "Memeber : " << temp->member_id << endl;
            cout << "Celcius : " << temp->reading_celsius << endl;
            fan->reading = fan->max_reading_range;
          }
          break;
        }
      }
    }

    cout << "바꾼후~~ " << endl;
    cout << "fan info" << endl;
    cout << record_get_json(fan->odata.id) << endl;
    resource_save_json(fan);
  }
}

bool patch_ethernet_interface(json::value _jv, string _record_uri, int _flag) {
  // _flag=0 -- 값만 바꿈, _flag=1 -- 변경처리까지
  cout << "바뀌기전~~ " << endl;
  cout << record_get_json(_record_uri) << endl;
  cout << " $$$$$$$ " << endl;

  EthernetInterfaces *eth = (EthernetInterfaces *)g_record[_record_uri];
  bool result = false;

  string description;
  if (get_value_from_json_key(_jv, "Description", description)) {
    eth->description = description;
    result = true;
  }

  // string fqdn;
  // if(get_value_from_json_key(_jv, "FQDN", fqdn))
  // {
  //     eth->fqdn = fqdn;
  //     if(_flag == 1)
  //     {
  //         string buf = "hostname ";

  //     }
  // }

  string hostname;
  if (get_value_from_json_key(_jv, "HostName", hostname)) {
    if (_flag == 1) {
      string buf = "hostname ";
      buf = buf + hostname;
      cout << "hostname buf : " << buf << endl;
      system(buf.c_str());
    }
    eth->hostname = hostname;
    result = true;
  }

  string mac;
  if (get_value_from_json_key(_jv, "MACAddress", mac)) {
    if (_flag == 1) {
      string buf = "macchanger -m ";
      buf = buf + mac + " eth" + eth->id;
      cout << "macaddress buf : " << buf << endl;
      // system(buf.c_str());
    }
    eth->mac_address = mac;
    result = true;
  }

  int mtu;
  if (get_value_from_json_key(_jv, "MTUSize", mtu)) {
    if (_flag == 1) {
      string buf = "ip link set ";
      buf = buf + "eth" + eth->id;
      buf = buf + " mtu " + to_string(mtu);
      cout << "mtusize buf : " << buf << endl;
      system(buf.c_str());
    }
    eth->mtu_size = mtu;
    result = true;
  }

  // ipv4, ipv6 주소들은 array로 되어있는데 patch request body로는 일단
  // object하나라고 생각하고 구현
  json::value ipv4;
  if (get_value_from_json_key(_jv, "IPv4Addresses", ipv4)) {
    int size = eth->v_ipv4.size();
    if (size == 0) {
      return false;
    }
    // ipv4 1개이상있을때 첫번째꺼만 수정되는걸로

    string address;
    string netmask;
    string gateway;
    if (get_value_from_json_key(ipv4, "Address", address)) {
      if (_flag == 1) {
        string buf = "ifconfig eth";
        buf = buf + eth->id + " " + address;
        cout << "ipv4 address buf : " << buf << endl;
        // system(buf.c_str());
      }
      eth->v_ipv4[0].address = address;
      result = true;
    }

    if (get_value_from_json_key(ipv4, "SubnetMask", netmask)) {
      if (_flag == 1) {
        string buf = "ifconfig eth";
        buf = buf + eth->id + " netmask " + netmask;
        cout << "ipv4 subnetmask buf : " << buf << endl;
        // system(buf.c_str());
      }
      eth->v_ipv4[0].subnet_mask = netmask;
      result = true;
    }

    if (get_value_from_json_key(ipv4, "Gateway", gateway)) {
      if (_flag == 1) {
        string buf = "route add default gw ";
        buf = buf + gateway;
        cout << "ipv4 gateway buf : " << buf << endl;
        // system(buf.c_str());
      }
      eth->v_ipv4[0].gateway = gateway;
      result = true;
    }

    // 현재 여기 변수들이 바뀌면 107번 서버동작이 안돼서 명령어 주석처리
  }

  json::value ipv6;
  if (get_value_from_json_key(_jv, "IPv6Addresses", ipv6)) {
    // ipv6는 추가하는식
    int size = eth->v_ipv6.size();
    if (size == 0) {
      return false;
    }

    string address;
    if (get_value_from_json_key(ipv6, "Address", address)) {
      IPv6_Address new_ipv6;
      new_ipv6.address = address;
      new_ipv6.address_origin = eth->v_ipv6[0].address_origin;
      new_ipv6.prefix_length = eth->v_ipv6[0].prefix_length;
      new_ipv6.address_state = eth->v_ipv6[0].address_state;
      if (_flag == 1) {
        string buf = "ifconfig eth";
        buf = buf + eth->id + " inet6 add " + address + "/" +
              to_string(new_ipv6.prefix_length);
        cout << "ipv6 address buf : " << buf << endl;
        system(buf.c_str());
      }
      eth->v_ipv6.push_back(new_ipv6);
      result = true;
    }
  }

  cout << record_get_json(_record_uri) << endl;

  return result;
}

bool patch_system(json::value _jv, string _record_uri) {

  cout << record_get_json(_record_uri) << endl;
  // cout << " $$$$$$$ " << endl;

  bool result = false;

  string led;
  if (get_value_from_json_key(_jv, "IndicatorLED", led)) {
    if (led == "Off")
      ((Systems *)g_record[_record_uri])->indicator_led = LED_OFF;
    else if (led == "Blinking")
      ((Systems *)g_record[_record_uri])->indicator_led = LED_BLINKING;
    else if (led == "Lit")
      ((Systems *)g_record[_record_uri])->indicator_led = LED_LIT;
    else
      return false;

    result = true;
  }

  string description, hostname, asset_tag;
  if (get_value_from_json_key(_jv, "Description", description)) {
    ((Systems *)g_record[_record_uri])->description = description;
    result = true;
  }

  if (get_value_from_json_key(_jv, "HostName", hostname)) {
    ((Systems *)g_record[_record_uri])->hostname = hostname;
    result = true;
  }

  if (get_value_from_json_key(_jv, "AssetTag", asset_tag)) {
    ((Systems *)g_record[_record_uri])->asset_tag = asset_tag;
    result = true;
  }

  json::value boot;
  if (get_value_from_json_key(_jv, "Boot", boot)) {
    string bsoe, bsot, bsom, utbso;
    if (get_value_from_json_key(boot, "BootSourceOverrideEnabled", bsoe)) {
      ((Systems *)g_record[_record_uri])->boot.boot_source_override_enabled =
          bsoe;
      result = true;
    }

    if (get_value_from_json_key(boot, "BootSourceOverrideTarget", bsot)) {
      ((Systems *)g_record[_record_uri])->boot.boot_source_override_target =
          bsot;
      result = true;
    }

    if (get_value_from_json_key(boot, "BootSourceOverrideMode", bsom)) {
      ((Systems *)g_record[_record_uri])->boot.boot_source_override_mode = bsom;
      result = true;
    }

    if (get_value_from_json_key(boot, "UefiTargetBootSourceOverride", utbso)) {
      ((Systems *)g_record[_record_uri])
          ->boot.uefi_target_boot_source_override = utbso;
      result = true;
    }
  }

  cout << "바꾼후~~ " << endl;
  cout << record_get_json(_record_uri) << endl;

  return result;
}

bool patch_chassis(json::value _jv, string _record_uri) {
  cout << "바뀌기전~~ " << endl;
  cout << record_get_json(_record_uri) << endl;
  cout << " $$$$$$$ " << endl;

  bool result = false;

  string led;
  if (get_value_from_json_key(_jv, "IndicatorLED", led)) {
    if (led == "Off")
      ((Chassis *)g_record[_record_uri])->indicator_led = LED_OFF;
    else if (led == "Blinking")
      ((Chassis *)g_record[_record_uri])->indicator_led = LED_BLINKING;
    else if (led == "Lit")
      ((Chassis *)g_record[_record_uri])->indicator_led = LED_LIT;
    else
      return false;

    result = true;
  }

  string asset_tag;
  if (get_value_from_json_key(_jv, "AssetTag", asset_tag)) {
    ((Chassis *)g_record[_record_uri])->asset_tag = asset_tag;
    result = true;
  }

  json::value location;
  if (get_value_from_json_key(_jv, "Location", location)) {
    json::value postal_address, placement;
    if (get_value_from_json_key(location, "PostalAddress", postal_address)) {
      string country, territory, city, street, house_number, name, postal_code;
      if (get_value_from_json_key(postal_address, "Country", country)) {
        ((Chassis *)g_record[_record_uri])->location.postal_address.country =
            country;
        result = true;
      }
      if (get_value_from_json_key(postal_address, "Territory", territory)) {
        ((Chassis *)g_record[_record_uri])->location.postal_address.territory =
            territory;
        result = true;
      }
      if (get_value_from_json_key(postal_address, "City", city)) {
        ((Chassis *)g_record[_record_uri])->location.postal_address.city = city;
        result = true;
      }
      if (get_value_from_json_key(postal_address, "Street", street)) {
        ((Chassis *)g_record[_record_uri])->location.postal_address.street =
            street;
        result = true;
      }
      if (get_value_from_json_key(postal_address, "HouseNumber",
                                  house_number)) {
        ((Chassis *)g_record[_record_uri])
            ->location.postal_address.house_number = house_number;
        result = true;
      }
      if (get_value_from_json_key(postal_address, "Name", name)) {
        ((Chassis *)g_record[_record_uri])->location.postal_address.name = name;
        result = true;
      }
      if (get_value_from_json_key(postal_address, "PostalCode", postal_code)) {
        ((Chassis *)g_record[_record_uri])
            ->location.postal_address.postal_code = postal_code;
        result = true;
      }
    }

    if (get_value_from_json_key(location, "Placement", placement)) {
      string row, rack, rack_offset_units;
      unsigned int rack_offset;
      if (get_value_from_json_key(placement, "Row", row)) {
        ((Chassis *)g_record[_record_uri])->location.placement.row = row;
        result = true;
      }
      if (get_value_from_json_key(placement, "Rack", rack)) {
        ((Chassis *)g_record[_record_uri])->location.placement.rack = rack;
        result = true;
      }
      if (get_value_from_json_key(placement, "RackOffsetUnits",
                                  rack_offset_units)) {
        ((Chassis *)g_record[_record_uri])
            ->location.placement.rack_offset_units = rack_offset_units;
        result = true;
      }
      if (get_value_from_json_key(placement, "RackOffset", rack_offset)) {
        ((Chassis *)g_record[_record_uri])->location.placement.rack_offset =
            rack_offset;
        result = true;
      }
    }
  }

  cout << "바꾼후~~ " << endl;
  cout << record_get_json(_record_uri) << endl;

  return result;
}

bool patch_power_control(json::value _jv, string _record_uri) {
  cout << "바뀌기전~~ " << endl;
  cout << record_get_json(_record_uri) << endl;
  cout << " $$$$$$$ " << endl;

  bool result = false;

  json::value power_limit;
  if (get_value_from_json_key(_jv, "PowerLimit", power_limit)) {
    int correction_in_ms;
    string limit_exception;
    double limit_in_watts;
    if (get_value_from_json_key(power_limit, "CorrectionInMs",
                                correction_in_ms)) {
      ((PowerControl *)g_record[_record_uri])->power_limit.correction_in_ms =
          correction_in_ms;
      result = true;
    }
    if (get_value_from_json_key(power_limit, "LimitException",
                                limit_exception)) {
      ((PowerControl *)g_record[_record_uri])->power_limit.limit_exception =
          limit_exception;
      result = true;
    }
    if (get_value_from_json_key(power_limit, "LimitInWatts", limit_in_watts)) {
      ((PowerControl *)g_record[_record_uri])->power_limit.limit_in_watts =
          limit_in_watts;
      result = true;
    }
  }

  cout << "바꾼후~~ " << endl;
  cout << record_get_json(_record_uri) << endl;

  return result;
}

bool patch_event_service(json::value _jv, string _record_uri) {
  cout << "바뀌기전~~ " << endl;
  cout << record_get_json(_record_uri) << endl;
  cout << " $$$$$$$ " << endl;

  bool result = false;
  bool op_attempt = false, op_interval = false, op_enabled = false;

  int attempt, interval;
  bool service_enabled;
  EventService *service = (EventService *)g_record[_record_uri];

  if (get_value_from_json_key(_jv, "DeliveryRetryAttempts", attempt)) {
    if (attempt >= 0) {
      op_attempt = true;
      result = true;
    } else
      return false;
  }

  if (get_value_from_json_key(_jv, "DeliveryRetryIntervalSeconds", interval)) {
    if (interval >= 1) {
      op_interval = true;
      result = true;
    } else
      return false;
  }

  if (get_value_from_json_key(_jv, "ServiceEnabled", service_enabled)) {
    op_enabled = true;
    result = true;
  }

  if (result) {
    if (op_attempt)
      service->delivery_retry_attempts = attempt;
    if (op_interval)
      service->delivery_retry_interval_seconds = interval;
    if (op_enabled)
      service->service_enabled = service_enabled;

    resource_save_json(service);
  }

  cout << "바꾼후~~ " << endl;
  cout << record_get_json(_record_uri) << endl;

  return result;
}

bool patch_subscription(json::value _jv, string _record_uri) {
  cout << "바뀌기전~~ " << endl;
  cout << record_get_json(_record_uri) << endl;
  cout << " $$$$$$$ " << endl;

  bool result = false;
  bool op_context = false, op_policy = false;

  string context, policy;
  EventDestination *dest = (EventDestination *)g_record[_record_uri];

  if (get_value_from_json_key(_jv, "Context", context)) {
    result = true;
    op_context = true;
  }

  if (get_value_from_json_key(_jv, "DeliveryRetryPolicy", policy)) {
    if (policy == "RetryForever" || policy == "SuspendRetries" ||
        policy == "TerminateAfterRetries") {
      result = true;
      op_policy = true;
    } else
      return false;
  }

  if (result) {
    if (op_context)
      dest->context = context;
    if (op_policy)
      dest->delivery_retry_policy = policy;

    resource_save_json(dest);
  }

  cout << "바꾼후~~ " << endl;
  cout << record_get_json(_record_uri) << endl;

  return result;
}

void treat_delete(http_request _request, json::value _jv,
                  http_response &_response) {
  string uri = _request.request_uri().to_string();
  vector<string> uri_tokens = string_split(uri, '/');
  string uri_part;
  for (int i = 0; i < uri_tokens.size(); i++) {
    if (i == 3)
      break;

    uri_part += "/";
    uri_part += uri_tokens[i];
  }
  // uri_part에는 /redfish/v1/something 까지만

  string minus_one = get_parent_object_uri(uri, "/");
  cout << "MINUS_ONE INFO : " << minus_one << endl;

  if (uri_part == ODATA_ACCOUNT_SERVICE_ID) {
    if (minus_one == ODATA_ACCOUNT_ID) {
      remove_account(_jv, uri, ODATA_ACCOUNT_SERVICE_ID, _response);
      return;
    }
    // /redfish/v1/AccountService/Accounts/[id]로 들어오면 삭제되게끔 변경해놓음

    // if(minus_one == ODATA_ROLE_ID)
    // {
    //     remove_role(_request, _jv, ODATA_ACCOUNT_SERVICE_ID, _response);
    //     return ;
    // }
  } else if (uri_part == ODATA_SESSION_SERVICE_ID) {
    if (uri == ODATA_SESSION_ID) {
      remove_session(_request, _response);
      return;
    }
  } else if (uri_part == ODATA_EVENT_SERVICE_ID) {
    if (minus_one == ODATA_EVENT_DESTINATION_ID) {
      remove_subscription(uri, ODATA_EVENT_SERVICE_ID, _response);
      return;
    }
  }

  error_reply(get_error_json("URI Input Error in treat delete"),
              status_codes::BadRequest, _response);
  return;
}

void remove_account(json::value _jv, string _uri, string _service_uri,
                    http_response &_response) {
  // 방식 : /redfish/v1/AccountService/Accounts/[account_id] 까지 uri를 받고
  // json body 안받고 삭제
  string username, password, odata_id;
  odata_id = _uri;

  if (((AccountService *)g_record[_service_uri])->service_enabled == false) {
    error_reply(get_error_json("Account Service isn't Enabled"),
                status_codes::BadRequest, _response);
    return;
  }

  // if(get_value_from_json_key(_jv, "UserName", username) == false
  // || get_value_from_json_key(_jv, "Password", password) == false)
  // {
  //     reply_error(_request, get_error_json("No UserName or Password"),
  //     status_codes::BadRequest, _response); return ;
  // } // json request body에 UserName, Password없으면 BadRequest
  // ## 방식변경후 주석

  string account_id =
      get_current_object_name(odata_id, "/"); // ## 방식변경후 추가
  bool exist = false;
  AccountService *acc_service = (AccountService *)g_record[_service_uri];
  std::vector<Resource *>::iterator iter;
  for (iter = acc_service->account_collection->members.begin();
       iter != acc_service->account_collection->members.end(); iter++) {
    Account *t = (Account *)*iter;

    // if(t->user_name == username && t->password == password)
    // {
    //     odata_id = t->odata.id;
    //     exist = true;
    //     break;
    // }
    // ## 방식변경후 주석

    if (t->id == account_id) {
      // odata_id = t->odata.id;
      // exist = true;
      break;
    }
  }

  if (!record_is_exist(odata_id)) {
    error_reply(get_error_json("No Account"), status_codes::BadRequest,
                _response);
    return;
  }

  // if(!exist)
  // {
  //     reply_error(_request, get_error_json("No Account or Does not match
  //     Password"), status_codes::BadRequest, _response); return ;
  // } // ## 방식변경후 주석

  cout << "지우기전~~ " << endl;
  cout << record_get_json(acc_service->account_collection->odata.id) << endl;
  cout << " $$$$$$$ " << endl;

  unsigned int id_num;
  id_num = stoi(((Account *)g_record[odata_id])->id);
  delete_numset_num(ALLOCATE_ACCOUNT_NUM, id_num);

  delete (*iter);
  acc_service->account_collection->members.erase(iter);
  // account자체 객체삭제, g_record에서 삭제, account collection에서 삭제

  delete_resource(odata_id);
  // account json파일 삭제
  resource_save_json(acc_service->account_collection);
  // collection 반영

  cout << "지운후~~ " << endl;
  cout << "지워진놈 : " << odata_id << endl;
  cout << record_get_json(acc_service->account_collection->odata.id) << endl;

  success_reply(record_get_json(acc_service->account_collection->odata.id),
                status_codes::OK, _response);
  return;
}

void remove_session(http_request _request, http_response &_response) {
  if (((SessionService *)g_record[ODATA_SESSION_SERVICE_ID])->service_enabled ==
      false) {
    error_reply(get_error_json("Session Service isn't Enabled"),
                status_codes::BadRequest, _response);
    return;
  }

  if (!(is_session_valid(_request.headers()["X-Auth-Token"]))) {
    error_reply(get_error_json("Unvalid Session"), status_codes::BadRequest,
                _response);
    return;
  }

  // 토큰에 해당하는 세션 종료
  string token = _request.headers()["X-Auth-Token"];
  string session_uri = get_session_odata_id_by_token(token);
  Session *session;

  session = (Session *)g_record[session_uri];
  // cout << "YYYY : " << session->id << endl;
  session->_remain_expires_time = 0;
  // session->cts.cancel();
  log(info) << "Session Cancel "
               "Call~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
  success_reply(json::value::null(), status_codes::OK, _response);
  return;
}

void remove_subscription(string _uri, string _service_uri,
                         http_response &_response) {
  string uri = _uri; //_request.request_uri().to_string();
  if (!record_is_exist(uri)) {
    error_reply(get_error_json("No Subscription"), status_codes::BadRequest,
                _response);
    return;
  }

  EventService *service = (EventService *)g_record[_service_uri];
  Collection *col = service->subscriptions;
  std::vector<Resource *>::iterator iter;
  for (iter = col->members.begin(); iter != col->members.end(); iter++) {
    EventDestination *del = (EventDestination *)*iter;
    if (del->id == get_current_object_name(uri, "/")) {
      break;
    }
  }

  cout << record_get_json(col->odata.id) << endl;

  unsigned int id_num;
  id_num = stoi(get_current_object_name(uri, "/"));
  delete_numset_num(ALLOCATE_SUBSCRIPTION_NUM, id_num);

  delete (*iter);
  col->members.erase(iter);
  delete_resource(uri);
  resource_save_json(col);

  cout << record_get_json(col->odata.id) << endl;

  success_reply(json::value::null(), status_codes::OK, _response);
  return;
}

void error_reply(json::value _jv, status_code _status,
                 http_response &_response) {
  _response.set_status_code(_status);

  if (_jv != json::value::null()) {
    _response.set_body(_jv);
  }

  return;
}

json::value get_error_json(string _message) {
  json::value err;
  if (_message != "")
    err[U("Error")] = json::value::string(U(_message));
  else
    err = json::value::null();

  return err;
}

void success_reply(json::value _jv, status_code _status,
                   http_response &_response) {
  _response.set_status_code(_status);

  if (_jv != json::value::null()) {
    _response.set_body(_jv);
  }

  return;
}

void report_last_command(string _uri) {
  json::value j;
  j["Uri"] = json::value::string(_uri);
  j["Type"] = json::value::string(MODULE_TYPE);
  j["Name"] = json::value::string(MODULE_NAME);

  string cur_time = currentDateTime();
  j["Time"] = json::value::string(cur_time);

  http_client client(CMM_ADDRESS);
  http_request req(methods::POST);
  req.set_body(j);

  http_response response;

  try {
    pplx::task<http_response> responseTask = client.request(req);
    response = responseTask.get();

    if (response.status_code() == status_codes::OK) {
      log(info) << "[LAST COMMAND] : Success";
    } else
      log(info) << "[LAST COMMAND] : Error";

  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    log(warning) << "Error at Shooting Last Command";
  }

  return;
}

void test_send_event(Event _event) {
  // string, Event* ... event_map
  // 인자받아야겟네
  // 이벤트에서 subs에 event type이 맞는걸 골라서 각자의 sub type으로
  // 보내는거같음
  Collection *col = (Collection *)g_record[ODATA_EVENT_DESTINATION_ID];
  vector<Resource *>::iterator iter;
  EventDestination *dest;
  for (iter = col->members.begin(); iter != col->members.end(); iter++) {
    dest = (EventDestination *)*iter;

    if (dest->subscription_type == "RedfishEvent") {
      // break;
      http_client client(dest->destination);
      http_request req(methods::POST);
      req.set_body(_event.get_json());
      http_response res;

      try {
        pplx::task<http_response> responseTask = client.request(req);
        res = responseTask.get();
        log(info) << "[SUBMIT][TESTEVENT][STATUS][DEST : " << dest->context
                  << "] : " << res.status_code();
      } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
      }
    }
  }
}
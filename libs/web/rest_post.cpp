#include <ipmi/rest_post.hpp>
extern Ipmiuser ipmiUser[10];
int Ipmiweb_POST::Try_Login(string username , string pwd){

  int response = 0;

	log(info) << "[try login] username : " << username;
  log(info) << "[try login] password : " << pwd;

  if (response == NULL) {
    response = authenticate_ipmi(username, pwd);
  }
  if (response == NULL) {
    response = authenticate_ldap(username, pwd);
  }
  if (response == NULL) {
    response = authenticate_ad(username, pwd);
  }

	if (ipmiUser[0].getUsername() == username){
      if (ipmiUser[0].getUserpassword() == pwd)
        {
			sprintf(response, "%d", 4);
			cout << response << endl;
		}
	}

	return response ;
};


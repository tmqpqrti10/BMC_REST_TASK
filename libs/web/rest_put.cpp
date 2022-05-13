#include <ipmi/rest_put.hpp>


void Ipmiweb_PUT::Set_Fru_Header(int fru_id, string hdr_board, string  hdr_chassis, string hdr_product){
  cout << "Enter rest_set_fru_header" << endl;
	
	Ipmifru *fru_this = &fru_rec.find(0)->second.find(0)->second;
	
	fru_this->fru_header.id = (uint8_t)fru_id;
	fru_this->fru_header.board = hdr_board;
	fru_this->fru_header.chassis = hdr_chassis;
	fru_this->fru_header.product = hdr_product;

	return;

}

#ifndef HELPER_H__
#define HELPER_H__
#include <ipmi/ipmi.hpp>
#define SIZE 1024
#define WAITSECONDS 100
#define QSIZE 35000

char* test_sendrecv(struct ipmi_rq *req, unsigned char *input, int *res_length);
char* sendrecv(struct ipmi_rq *req);

int str2uint(const char * str, uint32_t * uint_ptr);
uint32_t buf2long(uint8_t * buf);
uint16_t buf2short(uint8_t * buf);
int str2uchar(const char * str, uint8_t * uchr_ptr);

int send_to_kvm();
/**
 * @brief response string 쓰레기 값 제거
 * @details 몽구스 REST 사용시 문자열 쓰레기값이 존재함
 * @param data char * 문자열
 */
void refine_data(uint8_t *data);
// int str2long(const char * str, int64_t * lng_ptr);
// int str2int(const char * str, int32_t * int_ptr);
// const char * val2str(uint16_t val, const struct valstr *vs);
// const char * buf2str(uint8_t * buf, int len);
// int str2ulong(const char * str, uint64_t * ulng_ptr);
// int str2uchar(const char * str, uint8_t * uchr_ptr);
// void print_valstr_2col(const struct valstr * vs, const char * title, int loglevel);
// uint16_t buf2short(uint8_t * buf);
// uint32_t buf2long(uint8_t * buf);
// int str2uint(const char * str, uint32_t * uint_ptr);

// char* sendrecv(struct ipmi_rq *req);

#endif

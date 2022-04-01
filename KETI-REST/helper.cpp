#include <errno.h>
#include <string.h>
#include <math.h>
#include <sys/msg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include "helper.hpp"
#include<ipmi/ipmi.hpp>



void signalHandler(int signo)
{
	puts("Clear.......................Exit.");
	exit(0);
}

char* sendrecv(struct ipmi_rq *req) {
	signal(SIGINT, signalHandler);
	rest_req_t rq;
	unsigned char res_buf[QSIZE] = "\0";
   
    rq.netfn_lun = req->msg.netfn;
    rq.cmd = req->msg.cmd;
	if (req->msg.data_len > 0)
	   	memcpy(rq.data, req->msg.data, req->msg.data_len);

    pid_t pid = getpid();
	
	int msqid_req, msqid_rsp;


	if ( -1 == ( msqid_req = msgget( (key_t)1234, IPC_CREAT | 0666)))  // 요청 큐 생성
    {
       perror( "msgget() in req 실패");
       exit( 1);
    }

	if (-1 == (msqid_rsp = msgget((key_t)5678, IPC_CREAT|0666))) {  // 응답 큐 생성
		perror("msgget() rsp in req failed");
		exit(1);
	}

    msq_req_t msq_req ;
    msq_rsp_t msq_rsp ;

    msq_req.type = pid;
    msq_req.index = 1; // will be used in future
    memcpy(&msq_req.ipmi_msg, &rq, sizeof(rest_req_t));

    if ( -1 == msgsnd(msqid_req, &msq_req, sizeof(msq_req_t)-sizeof(long), 0))
    {
        perror( "msgsnd() in req실패");
        exit( 1);
    }

	int offset = 0;
	int i, j; // WAITSECONDS 시간만큼만 응답 기다림
    for (i=0; i<WAITSECONDS; i++) {
	    if ( -1 == msgrcv( msqid_rsp, &msq_rsp, sizeof(msq_rsp_t)-sizeof(long), pid, 0))
    	{
			perror("msgrcv in req failed");
        	exit(1); // while문 돌면서 request msg가 오기를 기다림
			//usleep(500);	
			//continue;
    	}	
		//memcpy(res_buf, msq_rsp.data, QSIZE);
		if(strlen(msq_rsp.data) != 0){
			strcpy(res_buf, msq_rsp.data);
			return res_buf;
		}
		else
		{
			return 0;
		}
		

	/*	
		if (msq_rsp.next == 1) { // if response length > QSIZE, waiting for the next buffer

			offset += QSIZE;
			while (msq_rsp.next == 1) { // if the last buffer, next is zero and stop to receive
				for (j=0; j<WAITSECONDS; j++) {
					if (-1 == msgrcv(msqid_rsp, &msq_rsp, sizeof(msq_rsp_t)-sizeof(long), pid, 0)) {
						perror("msgrcv > QSIZE in req failed");
						exit(1);
					}
					memcpy(res_buf+offset, msq_rsp.data, QSIZE);
					offset += QSIZE;
					break;
				}
			}
		}
		*/
		return res_buf;
    	//printf("%s\n", res_buf);  // python에서 popen으로 응답 json을 읽으려면 출력 부분 필요
    	break;
    }
    
}

char* test_sendrecv(struct ipmi_rq *req, unsigned char *input, int *res_length) {
	printf("test_sendrecv\n");
    signal(SIGINT, signalHandler);
    //printf("test_sendrecv signalHandler\n");
	rest_req_t rq;
	unsigned char res_buf[QSIZE] = "\0";
	
    rq.netfn_lun = req->msg.netfn;
    rq.cmd = req->msg.cmd;
	if (req->msg.data_len > 0)
	   	memcpy(rq.data, req->msg.data, req->msg.data_len);

    pid_t pid = getpid();
	
	int msqid_req, msqid_rsp;
 
	if ( -1 == ( msqid_req = msgget( (key_t)1234, IPC_CREAT | 0666)))  // 요청 큐 생성
    {
       perror( "msgget() in req 실패");
       exit( 1);
    }	
	if (-1 == (msqid_rsp = msgget((key_t)5678, IPC_CREAT|0666))) {  // 응답 큐 생성
		perror("msgget() rsp in req failed");
		exit(1);
	}
    //printf("test_sendrecv 1\n");
    msq_req_t msq_req ;
    msq_rsp_t msq_rsp ;

    msq_req.type = pid;
    msq_req.index = 1; // will be used in future
	memset(msq_rsp.data, 0, sizeof(msq_rsp.data));
    memcpy(&msq_req.ipmi_msg, &rq, sizeof(rest_req_t));

    
    if ( -1 == msgsnd(msqid_req, &msq_req, sizeof(msq_req_t)-sizeof(long), 0))
    {
        perror( "msgsnd() in req실패");
        exit( 1);
    }
    //printf("\t test_sendrecv 2\n");
	int offset = 0;
	int i, j; // WAITSECONDS 시간만큼만 응답 기다림
    for (i=0; i<WAITSECONDS; i++) {
        printf("\twait ....%d\n",i);
	    if ( -1 == msgrcv( msqid_rsp, &msq_rsp, sizeof(msq_rsp_t)-sizeof(long), pid, 0))
    	{
			printf("\t test_sendrecv:msgrcv in req failed\n");
        	exit(1); // while문 돌면서 request msg가 오기를 기다림
			//usleep(500);	
			//continue;
    	}	
		//printf("\t wait1 ....%d\n",i);
		//memcpy(res_buf, msq_rsp.data, QSIZE);
		strcpy(res_buf, msq_rsp.data);
		strncpy(input, msq_rsp.data, strlen(msq_rsp.data));
		
		*res_length = strlen(msq_rsp.data);
		printf("end test_sendrecv\n");
    	break;
    }
    
	return 0;
}

/**
* @brief kvm process와 message queue로 통신
* @author doyoung
*/
int send_to_kvm() {
	signal(SIGINT, signalHandler);

    //pid_t pid = getpid();
	
	int msqid_req, msqid_rsp;

	if ( -1 == ( msqid_req = msgget( (key_t)1111, IPC_CREAT | 0666)))  // 요청 큐 생성
    {
       perror( "msgget() in req 실패");
       exit( 1);
    }
	
	if ( -1 == ( msqid_rsp = msgget( (key_t)2222, IPC_CREAT | 0666)))  // 요청 큐 생성
    {
       perror( "msgget() in req 실패");
       exit( 1);
    }
	
    kvm_msq_t msq_req, msq_rsp;

    msq_req.type = 1;
    msq_req.ccode = 0; 

	if ( -1 == msgsnd(msqid_req, &msq_req, sizeof(kvm_msq_t)-sizeof(long), 0))
    {
        perror( "msgsnd() in req실패");
        exit( 1);
    }
	
	   puts("Sent request. Waiting for response...");
	if ( -1 == msgrcv( msqid_rsp, &msq_rsp, sizeof(kvm_msq_t)-sizeof(long), 0, 0))
    {
		perror("msgrcv in req failed");
        exit(1); // while문 돌면서 request msg가 오기를 기다림
    }	

	   printf("ccode : %d\n", msq_rsp.ccode);  // python에서 popen으로 응답 json을 읽으려면 출력 부분 필요

    if (msq_rsp.ccode == 0)    
    	return 0;
    else {
    	printf("err: ccode is not 0 : %d\n", msq_rsp.ccode);
    }
}


int str2ulong(const char * str, uint64_t * ulng_ptr)
{
	char * end_ptr = 0;
	if (!str || !ulng_ptr)
		return (-1);

	*ulng_ptr = 0;
	errno = 0;
	*ulng_ptr = strtoul(str, &end_ptr, 0); 

	if (*end_ptr != '\0')
		return (-2);

	if (errno != 0)
		return (-3);

	return 0;
}
/* str2uchar - safely convert string to uint8
 *
 * @str: source string to convert from
 * @uchr_ptr: pointer where to store result
 *
 * returns zero on success
 * returns (-1) if one of args is NULL, (-2) or (-3) if conversion fails
 */
int str2uchar(const char * str, uint8_t * uchr_ptr)
{
	int rc = (-3);
	uint64_t arg_ulong = 0;
	if (!str || !uchr_ptr)
		return (-1);

	if ( (rc = str2ulong(str, &arg_ulong)) != 0 ) {
		*uchr_ptr = 0;
		return rc;
	}

	if (arg_ulong > UINT8_MAX)
		return (-3);

	*uchr_ptr = (uint8_t)arg_ulong;
	return 0;
} /* str2uchar(...) */

uint16_t buf2short(uint8_t * buf)
{           
	    return (uint16_t)(buf[1] << 8 | buf[0]);
}   

uint32_t buf2long(uint8_t * buf){
	return (uint32_t)(buf[3] << 24 | buf[2] << 16 | buf[1] << 8 | buf[0]);
}


int str2uint(const char * str, uint32_t * uint_ptr)
{
        int rc = 0;
        uint64_t arg_ulong = 0;
        if (!str || !uint_ptr)
                return (-1);

        if ( (rc = str2ulong(str, &arg_ulong)) != 0) {
                *uint_ptr = 0;
                return rc;
        }

        if (arg_ulong > UINT32_MAX)
                return (-3);

        *uint_ptr = (uint32_t)arg_ulong;
        return 0;
} /* str2uint(...) */

/**
 * @brief json data 뒤 쓰레기 값 존재할 시, 제거
 * @date 21.05.20
 * @author doyoung
 */
void refine_data(uint8_t *data){
	int data_idx = strlen(data);
	
	while (data[data_idx] != '}')
		data[data_idx--] = '\0';	
	return;
}

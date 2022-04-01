#include <ipmi/ipmb.hpp>
#include <ipmi/lightning_sensor.hpp>
#include  <signal.h>
#include <semaphore.h>
#include <poll.h>
#define SEQ_NUM_MAX 64

#define I2C_RETRIES_MAX 15

#define IPMB_PKT_MIN_SIZE 6
pthread_mutex_t m_i2c;
static int i2c_fd;
static int gseq = 0;

static int g_bus_id = 0; // store the i2c bus ID for debug print
// Structure for sequence number and buffer
typedef struct _seq_buf_t {
  bool in_use; // seq# is being used
  uint8_t len; // buffer size
  uint8_t *p_buf; // pointer to buffer
  sem_t s_seq; // semaphore for thread sync.
} seq_buf_t;
// Structure for holding currently used sequence number and
// array of all possible sequence number
typedef struct _ipmb_sbuf_t {
  uint8_t curr_seq; // currently used seq#
  seq_buf_t seq[SEQ_NUM_MAX]; //array of all possible seq# struct.
} ipmb_sbuf_t;

// Global storage for holding IPMB sequence number and buffer
ipmb_sbuf_t g_seq;
uint8_t rx_buf[MAX_BYTES] = { 0 };
uint8_t rx_len;
uint8_t rx_done=0;

static inline uint8_t
calc_cksum(uint8_t *buf, uint8_t len) {
  uint8_t i = 0;
  uint8_t cksum = 0;

  for (i = 0; i < len; i++) {
    cksum += buf[i];
  }

  return (ZERO_CKSUM_CONST - cksum);
}

static int
i2c_open(uint8_t bus_num) {
  int fd;
  char fn[32];
  int rc;
  snprintf(fn, sizeof(fn), "/dev/i2c-%d", bus_num);
  fd = open(fn, O_RDWR);
  if (fd == -1) {
    printf("Failed to open i2c device %s", fn);
    return -1;
  }

  rc = ioctl(fd, I2C_SLAVE, BRIDGE_SLAVE_ADDR);
  if (rc < 0) {
    printf("Failed to open slave @ address 0x%x", BRIDGE_SLAVE_ADDR);
    close(fd);
    return -1;
  }

  return fd;
}

static int
i2c_write(int fd, uint8_t *buf, uint8_t len) {
  struct i2c_rdwr_ioctl_data data;
  struct i2c_msg msg;
  int rc;
  int i;
//  struct timespec req;
//  struct timespec rem;

  memset(&msg, 0, sizeof(msg));

  msg.addr = buf[0] >> 1;
  msg.flags = 0;
  msg.len = len - 1; // 1st byte in addr
  msg.buf = &buf[1];

  data.msgs = &msg;
  data.nmsgs = 1;

  // Setup wait time
  //req.tv_sec = 0;
  //req.tv_nsec = 20000000;//20mSec

  pthread_mutex_lock(&m_i2c);

  for (i = 0; i < I2C_RETRIES_MAX; i++) {
    rc = ioctl(fd, I2C_RDWR, &data);
    if (rc < 0) {
      //nanosleep(&req, &rem);
      delay(20);
      continue;
    } else {
      break;
    }
  }

   if (rc < 0) {
    printf("bus: %d, Failed to do raw io", g_bus_id);
    pthread_mutex_unlock(&m_i2c);
    return -1;
  }

  pthread_mutex_unlock(&m_i2c);

  return 0;
}

static int
i2c_slave_open(uint8_t bus_num) {
  int fd;
  char fn[32];
  int rc;
  struct i2c_rdwr_ioctl_data data;
  struct i2c_msg msg;
  uint8_t read_bytes[MAX_BYTES] = { 0 };

  snprintf(fn, sizeof(fn), "/dev/i2c-%d", bus_num);
  fd = open(fn, O_RDWR);
  if (fd == -1) {
    printf("Failed to open i2c device %s", fn);
    return -1;
  }


  memset(&msg, 0, sizeof(msg));

  msg.addr = BMC_SLAVE_ADDR;
  msg.flags = I2C_S_EN;
  msg.len = 1;
  msg.buf = read_bytes;
  msg.buf[0] = 1;

  data.msgs = &msg;
  data.nmsgs = 1;

  rc = ioctl(fd, I2C_SLAVE_RDWR, &data);
  if (rc < 0) {
    printf("Failed to open slave @ address 0x%x", BMC_SLAVE_ADDR);
    close(fd);
  }

  return fd;
}

static int
i2c_slave_read(int fd, uint8_t *buf, uint8_t *len) {
  struct i2c_rdwr_ioctl_data data;
  struct i2c_msg msg;
  int rc;

  memset(&msg, 0, sizeof(msg));

  msg.addr = BMC_SLAVE_ADDR;
  msg.flags = 0;
  msg.len = MAX_BYTES;
  msg.buf = buf;

  data.msgs = &msg;
  data.nmsgs = 1;

  rc = ioctl(fd, I2C_SLAVE_RDWR, &data);
  if (rc < 0) {
    return -1;
  }

  *len = msg.len;

//  if(*len) printf("IPMB RX!!!\n");
  return 0;
}

/*
 * Function to handle IPMB messages
 */
void
lib_ipmb_handle(unsigned char bus_id,
            unsigned char *request, unsigned char req_len,
            unsigned char *response, unsigned char *res_len) {

  int s, t, len;
  struct sockaddr_un remote;
  char sock_path[64] = {0};
  struct timeval tv;

  sprintf(sock_path, "%s_%d", SOCK_PATH_IPMB, bus_id);

  // TODO: Need to update to reuse the socket instead of creating new
  if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
      printf("lib_ipmb_handle: socket() failed\n");
    return;
  }

  // setup timeout for receving on socket
  tv.tv_sec = TIMEOUT_IPMB + 1;
  tv.tv_usec = 0;

  setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

  remote.sun_family = AF_UNIX;
  strcpy(remote.sun_path, sock_path);
  len = strlen(remote.sun_path) + sizeof(remote.sun_family);

  if (connect(s, (struct sockaddr *)&remote, len) == -1) {
    printf("ipmb_handle: connect() failed\n");
    goto clean_exit;
  }

  if (send(s, request, req_len, 0) == -1) {
    printf("ipmb_handle: send() failed\n");
    goto clean_exit;
  }

  if ((t=recv(s, response, MAX_IPMB_RES_LEN, 0)) > 0) {
    *res_len = t;
  } else {
    if (t < 0) {
      printf("lib_ipmb_handle: recv() failed\n");
    } else {
      printf("Server closed connection\n");
    }
  }

clean_exit:
  close(s);

  return;
}


/*
 * Function to handle all IPMB requests
 */
static void ipmb_handle(int fd, unsigned char *request, unsigned char req_len,
                        unsigned char *response, unsigned char *res_len) {
  printf("ipmb request handler static void ipmb_handle\n");
  ipmb_req_t *req = (ipmb_req_t *)request;
  ipmb_res_t *res = (ipmb_res_t *)response;
  printf("req  res_slave_addr= %d\n",req->res_slave_addr);
  printf("req  netfn_lun= %d\n",req->netfn_lun);
  printf("req  hdr_cksum= %d\n",req->hdr_cksum);
  printf("req  req_slave_addr= %d\n",req->req_slave_addr);
  printf("req  seq_lun= %d\n",req->seq_lun);
  printf("req  cmd= %d\n",req->cmd);
  printf("req  dest_LUN  = %d\n",req->dest_LUN);
  printf("req  src_LUN  = %d\n",req->src_LUN);
  printf("req  data[IPMI_MSG_MAX_LENGTH]=");
  int j=0;
  for (j = 0; j < IPMI_MSG_MAX_LENGTH; j++) {
    printf("%x ", req->data[j]);
  }
  printf("\n");


  int8_t index;
  // Send request over i2c bus
  if (i2c_write(fd, request, req_len)) {
    goto ipmb_handle_out;
  }
ipmb_handle_out:
  return;
}

void*
ipmb_rx_handler(void *bus_num) {
  uint8_t *bnum = (uint8_t*) bus_num;
  int fd;
  uint8_t tlun;
//  uint8_t buf[MAX_BYTES] = { 0 };
  struct pollfd ufds[1];
  printf("bus =%d\n",*bnum);
  ipmb_res_t *res = (ipmb_res_t *)rx_buf;

  int i;
  fd = i2c_slave_open(*bnum);
  if (fd < 0) {
    printf("i2c_slave_open fails\n");

    goto cleanup;
  }
  ufds[0].fd = fd;
  ufds[0].events = POLLIN;

  while (1) {
     if (i2c_slave_read(fd, rx_buf, &rx_len) < 0) {
       //printf("ipmb_rx_handler i2c_slave_read return = 0\n");
      poll(ufds, 1, 10);
      continue;
    }
    printf("i2c read sucessfull \n\n");
    if (rx_len < IPMB_PKT_MIN_SIZE) {
      printf("bus: %d, IPMB Packet invalid size %d", g_bus_id, rx_len);
      continue;
    }
  if (res->hdr_cksum != calc_cksum(rx_buf, 2 )) {
        continue;
  }
    if (rx_buf[rx_len-1] != calc_cksum(&rx_buf[3], rx_len-4)) {
      continue;
    }
    if(res->cc == 0){
    	if(gseq == (rx_buf[4]>>2)) rx_done = 1;
    }

  }
cleanup:
  if (fd > 0) {
    close (fd);
  }
  pthread_exit(NULL);
}

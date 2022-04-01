/**
 * @file i2c or gpio를 통한 센서값 리딩 smbus pmbus
 * 
 */
#include <ipmi/lightning_sensor.hpp>
#include <ipmi/ipmi.hpp>
#include<util/smbus.hpp>

// g_TMAX 사용
#include <ipmi/snrctrl.hpp>
typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;
struct timing_negotiation
{
  u8 msg_timing;
  u8 addr_timing;
};
/**
 * @brief limit power 
 * @author 박기철
 */
int g_Tmax=90;
static pthread_mutex_t m_sensor;
int temp_data[64];
int tacho_data[16];

enum temp_sensor_type
{
  LOCAL_SENSOR = 0,
  REMOTE_SENSOR,
};
//  enum tmp75_peb_sensors {
//    PEB_TMP75_U136 = 0x4d,
//    PEB_TMP75_U134 = 0x4a,LOCAL_SENSOR
//    PEB_TMP421_U15 = 0x4c,
//    PEB_MAX6654_U4 = 0x18,
//  };

//  enum tmp75_pdpb_sensors {
//    PDPB_TMP75_U70 = 0x4f,
//    PDPB_TMP75_U71 = 0x49,
//    PDPB_TMP75_U72 = 0x4A,
//    PDPB_TMP75_U73 = 0x48,
//    PDPB_TMP75_U69 = 0x4E,// check duplicate address
//    PDPB_TMP75_U205 = 0x4C,
//  };

enum nct7904_registers
{
  NCT7904_TEMP_CH1 = 0x42,
  NCT7904_TEMP_CH2 = 0x46,
  NCT7904_VSEN6 = 0x4A,
  NCT7904_VSEN7 = 0x4C,
  NCT7904_VSEN9 = 0x50,
  NCT7904_3VDD = 0x5C,
  NCT7904_MONITOR_FLAG = 0xBA,
  NCT7904_BANK_SEL = 0xFF,
};

enum hsc_controllers
{
  HSC_ADM1276 = 0,
  HSC_ADM1278,
};

enum hsc_commands
{
  HSC_IN_VOLT = 0x88,
  HSC_OUT_CURR = 0x8c,
  HSC_IN_POWER = 0x97,
  HSC_TEMP = 0x8D,
};

//  enum ads1015_registers {
//    ADS1015_CONVERSION = 0,
//    ADS1015_CONFIG,
//  };

enum ads1015_channels
{
  ADS1015_CHANNEL0 = 0,
  ADS1015_CHANNEL1,
  ADS1015_CHANNEL2,
  ADS1015_CHANNEL3,
  ADS1015_CHANNEL4,
  ADS1015_CHANNEL5,
  ADS1015_CHANNEL6,
  ADS1015_CHANNEL7,
};

//  enum adc_pins {
//    ADC_PIN0 = 0,
//    ADC_PIN1,
//    ADC_PIN2,
//    ADC_PIN3,
//    ADC_PIN4,
//    ADC_PIN5,
//    ADC_PIN6,
//    ADC_PIN7,
//    ADC_PIN8,
//    ADC_PIN9,
//    ADC_PIN10,
//    ADC_PIN11,
//    ADC_PIN12,
//    ADC_PIN13,
//    ADC_PIN14,
//    ADC_PIN15,
//  };

//  enum tmp75_nva_sensors {
//    NVA_TMP75_1 = 0x48,
//    NVA_TMP75_2 = 0x49,
//    NVA_TMP75_3 = 0x4a,
//    NVA_TMP75_4 = 0x4b,
//  };

//  enum psu_nva_regs{
//    NVA_PSU_TEMP=0x8d,
//    NVA_PSU_TEMP2=0x8e,
//    NVA_PSU_FAN1=0x90,
//    NVA_PSU_FAN2=0x91,
//    NVA_PSU_WATT=0x96,
//  };

// Helper function for msleep
void msleep(int msec)
{
  struct timespec req;

  req.tv_sec = 0;
  req.tv_nsec = msec * 1000 * 1000;

  while (nanosleep(&req, &req) == -1 && errno == EINTR)
  {
    continue;
  }
}

int32_t signextend32(uint32_t val, int idx)
{
  uint8_t shift = 31 - idx;
  return (int32_t)(val << shift) >> shift;
}

/**
 * @brief 입력된 하드웨어 주소의 센서 값을 읽어오는 함수
 * 
 * @param device 센서 하드웨어 주소
 * @param value 센서 값
 * @return int 성공 유무 (0 : 성공 / errno : 실패)
 **/
static int read_device(const char *device, int *value)
{
  FILE *fp;
  int rc;

  fp = fopen(device, "r");
  if (!fp)
  {
    int err = errno;
    printf("failed to open device %s", device);
    return err;
  }
  // int ab=i2c_smbus_read_byte(fp);
  // cout<<"\n\n\n\n\n\n\nsmbus_readbyte = "<<ab<<endl;
  
  rc = fscanf(fp, "%d", value);
  printf("read device : %d\n", value);
  fclose(fp);

  if (rc != 1)
  {
    printf("failed to read device %s", device);
    return ENOENT;
  }
  else
  {
    return 0;
  }
}

static int read_device_float(const char *device, float *value)
{
  FILE *fp;
  int rc;
  char tmp[10];

  fp = fopen(device, "r");
  if (!fp)
  {
    int err = errno;
#ifdef DEBUG
    LOG_ALL_INFO(LOG_INFO, "failed to open device %s", device);
#endif
    return err;
  }

  rc = fscanf(fp, "%s", tmp);
  fclose(fp);

  if (rc != 1)
  {
#ifdef DEBUG
    LOG_ALL_INFO(LOG_INFO, "failed to read device %s", device);
#endif
    return ENOENT;
  }

  *value = atof(tmp);

  return 0;
}

/**
 * @brief 입력된 하드웨어 디바이스 드라이버의 센서 값를 읽어오는 함수
 * 
 * @param device 디바이스 드라이버 경로 ex)/root/sys/devices/platform/ast_adc.0//adc0_value
 * @param value 센서 값 ex)150

 * @return int 성공 유무 (0 : 성공 / errno : 실패)
 **/
static int read_device_int(const char *device, int *value)
{
  FILE *fp;
  int rc;
  char tmp[10];
  //cout<<"read_device_int start"<<endl;
  fp = fopen(device, "r");
  if (!fp)
  {
    int err = errno;
    printf("failed to open device %s", device);
    return err;
  }

  

  rc = fscanf(fp, "%s", tmp);
  fclose(fp);

  if (rc != 1)
  {

    printf("failed to read device %s", device);
    return ENOENT;
  }
  //cout<<"device ="<<device<<endl;
  // fp = open(device, O_RDWR);
  // if (!fp)
  // {
  //   int err = errno;
  //   printf("failed to open device %s", device);
  //   return err;
  // }
  
  *value = atoi(tmp);
  // unsigned char values[6];
  // if(i2c_smbus_write_word_data(fp, 0x03, 5555)<0)
  // {
  //   perror("i2c_smbus_write_word_data not ");
  // }
  // else{
  //   cout<<"smbus 정상수행"<<endl;
  // }
  // values[0] = i2c_smbus_read_word_data(fp, 0x03);
  // values[1] = i2c_smbus_read_word_data(fp, 0x04);
  // values[2] = i2c_smbus_read_word_data(fp, 0x05);
  // values[3] = i2c_smbus_read_word_data(fp, 0x06);
  // values[4] = i2c_smbus_read_word_data(fp, 0x07);
  // values[5] = i2c_smbus_read_word_data(fp, 0x08);
  // print out result
  // printf("Values: X MSB: %d, X LSB: %d, Y MSB: %d, Y LSB: %d, Z MSB: %d, Z LSB: %d\n",
  //   values[0],values[1],values[2],values[3],values[4],values[5]);
  //cout <<"i2c =read value ="<<*value<<endl;
  return 0;
}

int read_adc_value(const int pin, const char *device, int *value)
{
  char device_name[LARGEST_DEVICE_NAME];
  char full_name[LARGEST_DEVICE_NAME];
  int val = 0;
  int ret;

  snprintf(device_name, LARGEST_DEVICE_NAME, device, pin);
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/%s", ADC_DIR, device_name);
  ret = read_device_int(full_name, &val);
  *value = val; //>>2
  return (ret);
}

/**
 * @brief 팬의 RPM 속도를 읽어오는 함수
 * 
 * @param pin 팬의 Index 번호
 * @param value 팬 RPM 값
 * @return int 성공 유무
 **/
static int read_tacho_value(const int pin, int *value)
{
  char device_name[MAX_DVNAME_LEN] = {0};
  char full_name[MAX_DVNAME_LEN] = {0};
  int ret;
  int val = 0;

  snprintf(device_name, MAX_DVNAME_LEN, TACHO_VALUE, pin);
  // printf("device name : %s\n", device_name);
  snprintf(full_name, MAX_DVNAME_LEN, "%s/%s", TACHO_DIR, device_name);
  // printf("full name : %s\n", full_name);

  ret = read_device_int(full_name, &val);
  *value = val; /// 100;
  return ret;
}

static int i2c_write(int i2c_dev, int addr, char value)
{
  unsigned char val[2] = {0};

  val[0] = addr;
  val[1] = value;

  write(i2c_dev, val, sizeof(val));
  return 0;
}

/**
 * @brief TMP75 센서의 온도값을 읽어오는 함수\n
 * TMP75의 주소값을 통해 I2C 통신으로 센서 값을 읽어옴
 * 
 * @param device 읽을 센서의 디바이스 드라이버
 * @param addr 센서 주소값
 * @param type 센서 타입
 * @param value 센서 값
 * @return int 성공 유무
 */
static int i2c_read(int i2c_dev, int addr)
{
  unsigned char val[2] = {0};

  val[0] = addr;

  write(i2c_dev, &val[0], 1);
  read(i2c_dev, &val[1], 1);

  return val[1];
}
static int i2c_readw(int i2c_dev, int addr)
{
  unsigned char val[2] = {0};
  unsigned char rval[2] = {0};
  int ret;

  val[0] = addr;

  write(i2c_dev, &val[0], 1);
  read(i2c_dev, &rval[0], 2);

  ret = rval[0] * 256 + rval[1];
  return ret;
}

static unsigned int i2c_readw_r(int i2c_dev, int addr)
{
  unsigned char val[2] = {0};
  unsigned char rval[2] = {0};
  unsigned int ret;

  val[0] = addr;

  write(i2c_dev, &val[0], 1);
  read(i2c_dev, &rval[0], 2);

  ret = rval[0] + rval[1] * 256;
  return ret;
}

int read_tmp75_temp_value(const char *device, int *value)
{ // float *value
  char full_name[LARGEST_DEVICE_NAME + 1];
  int tmp, ret = 0;
  snprintf(full_name, LARGEST_DEVICE_NAME, "%s/temp1_input", device);

  /*
  if (read_device(full_name, &tmp)) {
    return -1;
  }

  *value = ((float)tmp);///UNIT_DIV
 */
  ret = read_device_int(full_name, &tmp);
  *value = tmp;
  return ret;
}

/**
 * @brief TMP75 센서의 온도값을 읽어오는 함수\n
 * TMP75의 주소값을 통해 I2C 통신으로 센서 값을 읽어옴
 * 
 * @param device 읽을 센서의 디바이스 드라이버
 * @param addr 센서 주소값
 * @param type 센서 타입
 * @param value 센서 값
 * @return int 성공 유무
 */
static int read_i2c_value(char *device, uint8_t addr, uint8_t type, uint32_t *value)
{

  int dev;
  int ret;
  int32_t res;
  cout<<"read_i2c_value device"<<device<<endl;
  dev = open(device, O_RDWR);
  if (dev < 0)
  {
    cout << ("read_i2c_value: open() failed");
    return -1;
  }
  cout<<"read_i2c_value dev"<<dev<<endl;
  cout<<"read_i2c_value addr"<<addr<<endl;
  if (ioctl(dev, I2C_SLAVE_FORCE, addr) < 0)
  {
    perror("ioctl() assigning i2c addr failed");
    close(dev);
    return -1;
  }
  if(i2c_smbus_write_word_data(dev, 0, 255)<0)
  {
    perror("i2c_smbus_write_word_data not ");
  }
  unsigned char values;
  
  values=i2c_smbus_read_word_data(dev, 0x03);
  printf("Values: X MSB: %d\n",values);

  cout<<"read_i2c_value type"<<type<<endl;
  res = i2c_readw(dev, type);
  res >>= 4;
  cout<<"read_i2c_value res"<<(int)res<<endl;
  *value = static_cast<uint32_t>(res);
   cout<<"read_i2c_value value"<<(int)*value<<endl;
  close(dev);
  return 1;
}

int read_temp_value(char *device, uint8_t addr, uint8_t type, uint32_t *value)
{
  int ret;
  int val;
  cout<<"read_temp_value"<<endl;
  ret = read_i2c_value(device, addr, type, &val);
  cout<<"ret"<<endl;
  *value = val >> 3;

  return ret;
}

/**
 * @brief PSU(Power Supply Unit)의 정보를 읽어오는 함수\n
 * PMBus를 통하여 PSU의 온도, 팬 RPM, 전력량 등의 정보를 수집함.
 * 
 * @param device PSU 디바이스 드라이버
 * @param addr PSU 번호
 * @param type 센서 타입 (온도, 팬, 전력량)
 * @param value 센서 값
 * @return int 성공 유무
 */
static int
read_psu_value(char *device, uint8_t addr, uint8_t type, int *value)
{

  int dev;
  int ret;
  int32_t res;
  int8_t n;
  int16_t y;
  dev = open(device, O_RDWR);
  if (dev < 0)
  {
    cout << ("read_psu_value: open() failed\n");
    return -1;
  }

  if (ioctl(dev, I2C_SLAVE_FORCE, addr) < 0)
  {
    cout << ("read_i2c_value: ioctl() assigning i2c addr failed\n");
    close(dev);
    return -1;
  }
  res = i2c_readw_r(dev, type);

  close(dev);

  n = (res >> 11) & 0x1f;
  y = res & 0x7FF;
  if (n > 0x0f)
    n |= 0xE0;
  if (y > 0x3ff)
    y |= 0xF800;
  *value = (y * pow(2, n));

  if ((type == NVA_PSU_FAN1) || (type == NVA_PSU_FAN2))
  {
    *value = (*value + 50) / 100;
  }
  else if (type == NVA_PSU_WATT)
  {
    *value = (*value + 5) / 10;
  }

  return 0;
}

static int
read_temp(const char *device, float *value)
{
  char full_name[LARGEST_DEVICE_NAME + 1];
  int tmp;

  snprintf(
      full_name, LARGEST_DEVICE_NAME, "%s/temp1_input", device);
  if (read_device(full_name, &tmp))
  {
    return -1;
  }

  *value = ((float)tmp) / UNIT_DIV;

  return 0;
}

static int
read_hsc_value(uint8_t reg, char *device, uint8_t addr, uint8_t cntlr, float *value)
{

  int dev;
  int ret;
  int32_t res;

  dev = open(device, O_RDWR);
  if (dev < 0)
  {
    cout << ("read_hsc_value: open() failed");
    return -1;
  }

  /* Assign the i2c device address */
  ret = ioctl(dev, I2C_SLAVE, addr);
  if (ret < 0)
  {
    cout << ("read_hsc_value: ioctl() assigning i2c addr failed");
    close(dev);
    return -1;
  }
  close(dev);

  switch (reg)
  {
  case HSC_IN_VOLT:
    res &= 0xFFF; // This Parameter uses bits [11:0]
    if (cntlr == HSC_ADM1276 || cntlr == HSC_ADM1278)
    {
      *value = (1.0 / 19599) * ((res * 100) - 0);
    }
    break;
  case HSC_OUT_CURR:
    res &= 0xFFF; // This Parameter uses bits [11:0]
    if (cntlr == HSC_ADM1276)
    {
      *value = ((res * 10) - 20475) / (807 * 0.1667);
    }
    else if (cntlr == HSC_ADM1278)
    {
      *value = (1.0 / 1600) * ((res * 10) - 20475);
    }
    break;
  case HSC_IN_POWER:
    res &= 0xFFFF; // This Parameter uses bits [15:0]
    if (cntlr == HSC_ADM1276)
    {
      *value = ((res * 100) - 0) / (6043 * 0.1667);
    }
    else if (cntlr == HSC_ADM1278)
    {
      *value = (1.0 / 12246) * ((res * 100) - 0);
    }
    *value *= 0.99; // This is to compensate the controller reading offset value
    break;
  case HSC_TEMP:
    res &= 0xFFF; // This Parameter uses bits [11:0]
    if (cntlr == HSC_ADM1278)
    {
      *value = (1.0 / 42) * ((res * 10) - 31880);
    }
    *value *= 0.97; // This is to compensate the controller reading offset value
    break;
  default:
    cout << ("read_hsc_value: wrong param");
    return -1;
  }

  return 0;
}

static int
read_nct7904_value(uint8_t reg, char *device, uint8_t addr, float *value)
{

  int dev;
  int ret;
  int res_h;
  int res_l;
  int bank;
  int retry;
  uint8_t peer_tray_exist;
  uint8_t location;
  uint8_t monitor_flag;
  uint16_t res;
  float multipler;

  dev = open(device, O_RDWR);
  if (dev < 0)
  {
    cout << ("read_nct7904_value: open() failed");
    return -1;
  }

  /* Assign the i2c device address */
  ret = ioctl(dev, I2C_SLAVE, addr);
  if (ret < 0)
  {
    cout << ("read_nct7904_value: ioctl() assigning i2c addr failed");
  }

  close(dev);

  if (reg >= FAN_REGISTER && reg <= FAN_REGISTER + 22)
  {
    res = ((res_h & 0xFF) << 5) | (res_l & 0x1F);
  }
  else
  {
    res = ((res_h & 0xFF) << 3) | (res_l & 0x7);
  }

  switch (reg)
  {
  case NCT7904_TEMP_CH1:
    multipler = (0.125 /* NCT7904 Section 6.4 */);
    break;
  case NCT7904_TEMP_CH2:
    multipler = (0.125 /* NCT7904 Section 6.4 */);
    break;
  case NCT7904_VSEN9:
    multipler = (12 /* Voltage Divider*/ * 0.002 /* NCT7904 Section 6.4 */);
    break;
  case NCT7904_VSEN6:
    multipler = (12 /* Voltage Divider*/ * 0.002 /* NCT7904 Section 6.4 */);
    break;
  case NCT7904_VSEN7:
    multipler = (12 /* Voltage Divider*/ * 0.002 /* NCT7904 Section 6.4 */);
    break;
  case NCT7904_3VDD:
    multipler = (0.006 /* NCT7904 Section 6.4 */);
    break;
  }

  if (reg >= FAN_REGISTER && reg <= FAN_REGISTER + 22)
  {
    /* fan speed reading */
    if (res == 0x1fff || res == 0)
      *value = 0;
    else
      *value = (float)(1350000 / res);
  }
  else
  {
    /* temp sensor reading */
    if (reg == NCT7904_TEMP_CH1 || reg == NCT7904_TEMP_CH2)
    {

      *value = signextend32(res, 10) * multipler;
      if (reg == NCT7904_TEMP_CH2)
        *value = *value - 2;
      /* other sensor reading */
    }
    else
      *value = (float)(res * multipler);
  }

  return 0;
}

static int
read_ads1015_value(uint8_t channel, char *device, uint8_t addr, float *value)
{

  int dev;
  int ret;
  int32_t config, config2;
  int32_t res;

  dev = open(device, O_RDWR);
  if (dev < 0)
  {
    cout << ("read_ads1015_value: open() failed");
    return -1;
  }

  ret = ioctl(dev, I2C_SLAVE, addr);
  if (ret < 0)
  {
    cout << ("read_ads1015_value: ioctl() assigning i2c addr failed");
    close(dev);
    return -1;
  }

  config = ADS1015_DEFAULT_CONFIG;
  config |= (channel & 0x7) << 4;

  close(dev);
  res = ((res & 0x0FF) << 8) | ((res & 0xFF00) >> 8);
  *value = (float)(res >> 4) * (4.096 / 2048);

  return 0;
}
/**
 *@brief ADC 데이터를 읽는 곳 가상시뮬레이터는 0~9까지 생성함
 */
int read_fan_value(int fanno, int *value)
{
  int ret;
  ret = read_tacho_value(fanno, value);
  tacho_data[fanno] = *value;
  cout << ("read_fan_value: fanno:%d value:%f", fanno, *value);
  return ret;
}
/**
@brief IPMB의 Checksum을 확인하는 함수이다.
**/
 static unsigned char calc_checksum(uint8_t *buf, uint8_t len) {
   uint8_t i = 0;
   uint8_t cksum = 0;

   for (i = 0; i < len; i++) {
     cksum += buf[i];
   }

   return (ZERO_CKSUM_CONST - cksum);
 }

void get_ipmb_sensor(int id, unsigned char *response, unsigned char res_len)
{
  ipmb_res_t *res = (ipmb_res_t *) response;
  int i;
  switch(id)
  {
    case 48:
      g_Tmax = res->data[0];
    break;
    case 0x4B:
      temp_data[0] = res->data[3];
      temp_data[1] = res->data[4];
      for(i=0; i<8; i++){
        if (res->data[5+i] != 0xff)
          temp_data[2+i] = res->data[5+i];
        else
          temp_data[2+i] = 0;
      }
      for(i=0; i<8; i++){
        if (res->data[13+i] != 0xff)
          temp_data[2+12+i] = res->data[13+i];
        else
          temp_data[2+12+i] = 0;
      }

    break;
    default:
    break;
  }
}
// void get_ipmb_sensor1(int id, unsigned char *response, unsigned char res_len)
// {
//         ipmb_res_t *res = (ipmb_res_t *) response;
//         int i;
//         switch(id)
//         {
//                 case 48:
//                         g_Tmax = res->data[0];

//                 break;
//                 case 0x4B:
//                         temp_data[0] = res->data[3];
//                         temp_data[1] = res->data[4];
//                         for(i=0; i<4; i++){
//                               if (res->data[5+i] != 0xff)
//                                 temp_data[2+8+i] = res->data[5+i];
//                               else
//                                 temp_data[2+8+i] = 0;
//                         }
//                         for(i=0; i<4; i++){
//                               if (res->data[5+4+i] != 0xff)
//                                 temp_data[2+12+8+i] = res->data[5+4+i];
//                               else
//                                 temp_data[2+12+8+i] = 0;
//                         }

//                 break;
//                 default:
//                 break;
//         }
// }

//  int
//  lightning_sensor_sdr_path(uint8_t fru, char *path) {
//    return 0;
//  }

//  int
//  lightning_sensor_sdr_init(uint8_t fru, sensor_info_t *sinfo) {
//    return -1;
//  }

unsigned char
ipmb_res_handle (unsigned char *response, unsigned char res_len)
{

  ipmb_res_t *res = (ipmb_res_t *) response;
  unsigned char netfn;
  uint8_t fbyte;
  uint8_t len;
  uint8_t tlun;

//  printf("ipmb_res_handle()\n");
  netfn = res->netfn_lun >> 2;
  len = res_len; 
#if 1
  if (res->hdr_cksum != calc_checksum(response, 2 )) {
	printf("IPMB Header cksum does not match\n");
          return 0xff;
  }
#if 0
    if (res->hdr_cksum != calc_cksum(response, 2)) {
      //handle wrong slave address
      if (res->req_slave_addr != BMC_SLAVE_ADDR<<1) {
        // Store the first byte
        fbyte = res->req_slave_addr;
        // Update the first byte with correct slave address
        res->req_slave_addr = BMC_SLAVE_ADDR<<1;
        // Check again if the cksum passes
        if (res->hdr_cksum != calc_cksum(response,2)) {
          //handle missing slave address
          // restore the first byte
          res->req_slave_addr = fbyte;
          //copy the buffer to temporary
          memcpy(tbuf, response, len);
          // correct the slave address
          res->req_slave_addr = BMC_SLAVE_ADDR<<1;
          // copy back from temp buffer
          // copy back from temp buffer
          memcpy(&response[1], tbuf, len);
          // increase length as we added slave address byte
          len++;
          // Check if the above hacks corrected the header
          if (res->hdr_cksum != calc_cksum(response,2)) {
            syslog(LOG_WARNING, "IPMB Header cksum error after correcting slave address\n");
	    printf("IPMB Header cksum error after correcting slave address\n");
            return ;
          }
        }
      } else {
          syslog(LOG_WARNING, "IPMB Header cksum does not match\n");
	  printf("IPMB Header cksum does not match\n");
          return ;
      }
    }
#endif
    // Verify the IPMB data cksum: data starts from 4-th byte
    if (response[len-1] != calc_checksum(&response[3], len-4)) {
  //    syslog(LOG_WARNING, "IPMB Data cksum does not match\n");
      printf("IPMB Data cksum does not match\n");
      return 0xff;
    }
#endif
 //  printf("IPMB Res NetFn = 0x%02x\n", netfn);

 // printf("<<<IPMI Handle>>>  netfn = %x, cmd=%x\n", netfn, req->cmd);
  switch (netfn)
  {
    case NETFN_SENSOR_RES:
  //	printf("IPMB NETFN_SENSOR_RES!\n");
 //     res->netfn_lun = NETFN_CHASSIS_RES << 2;
 //     ipmi_handle_chassis (request, req_len, response, res_len);
	//ipmb_handle_sensor(response, res_len);
      break;
    case NETFN_OEM_GROUP_RES:
//	ipmb_handle_oem_group(response, res_len);
//	printf("NETFN_OEM_GROUP_RES!\n");
      break;
    default:
   //   res->netfn_lun = (netfn + 1) << 2;
      break;
  }

  // This header includes NetFunction, Command, and Completion Code
 // *res_len += SIZE_IPMI_RES_HDR;

  return (res->cc); 
}


int set_value_to_SMLTR(uint8_t sid, uint8_t val)
{
  smltr_data_t req_data;

  int msqid;
  int ndx = 0;
  int ret;
  //	printf("get_value_from_SMLTR is called with sensor : %d \n", sensor_n);
  //	fflush(stdout);

  if (-1 == (msqid = msgget((key_t)8999, IPC_CREAT | 0666)))
  {
    perror("msgget() 실패");
    return;
    //exit(1);
  }
  printf("GET_VALUE_FROM__SMLTR : got the key for sensor  : %d \n", sid);
  fflush(stdout);
  req_data.data_type = 2;
  req_data.sensor_num = sid;
  req_data.value = val;
  if (-1 == msgsnd(msqid, &req_data, sizeof(smltr_data_t) - sizeof(long), 0))
  {
    perror("msgsnd() 실패");
    perror("set_value_to_SMLTR 실패 ??");
    return;
    //exit(1);
  }
}

int get_value_from_SMLTR(uint8_t sensor_n)
{
	smltr_data_t req_data;
	smltr_data_t res_data;

	int msqid;
	int ndx = 0;
	int ret;

	if ( -1 == ( msqid = msgget( (key_t)8998, IPC_CREAT | 0666)))
	{
		perror( "msgget() 실패");
		exit( 1);
	}

	req_data.data_type = 2; 
	req_data.sensor_num = (unsigned int) sensor_n;

	if ( -1 == msgsnd( msqid, &req_data, sizeof(smltr_data_t) - sizeof( long), 0))
	{
		perror( "msgsnd() 실패");
    perror("get_value_from_SMLTR 실패 ??");
		exit( 1);
	}

	if (-1 == msgrcv(msqid, &res_data, sizeof(smltr_data_t) - sizeof(long), 1, 0))
	{
		perror("msgrcv() 실패\n");
	}
	ret = res_data.value;

    //printf("\t\t communicate with smltr, sensor_num : %d, ret = %d\n", sensor_n, ret);
	return ret;
}
/**
 * @brief PSU WATT FAN 등 다양한 센서값을 읽는 함수 
 * @param value 전달할 데이터
 * @param fru fru 센서 정보
 * @param sensor_num fru 구역내 센서 값
 * @bug tacho 만 읽는 FAN CPU TEMP같은 센서값은 read_??_value를 통해 값을 얻어와야 함 임시로구현
 * @author 박기철
 */
int lightning_sensor_read(uint8_t fru, uint8_t sensor_num, int *value) {
    int ret = 0;
    int tmp = 0;
    //pthread_mutex_lock(&m_sensor);
    // log(info) << "[lightning_sensor_read..] sensor_num : " << (int)sensor_num << ", fru_num : " << (int)fru;
    //  cout<<"\t\t\t dy : sensor_num ="<<(int)sensor_num<<endl;
    //  cout<<"\t\t\t dy : fru_num ="<<(int)fru<<endl;
    switch (fru) {
        case FRU_PEB:
            switch(sensor_num) {
                case PEB_SENSOR_ADC_P12V:
                    ret = read_adc_value(ADC_PIN0, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P3V3_AUX :
                    ret = read_adc_value(ADC_PIN1, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P1V0_STBY :
                    ret = read_adc_value(ADC_PIN2, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P1V05_PCH_AUX  :
                    ret = read_adc_value(ADC_PIN3, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P12V_AUX :
                    ret = read_adc_value(ADC_PIN4, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P1V8_PCH_AUX :
                    ret = read_adc_value(ADC_PIN5, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P3V0_BAT  :
                    ret = read_adc_value(ADC_PIN6, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P1V7_CPU0 : 
                    ret = read_adc_value(ADC_PIN7, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P1V7_CPU1 :
                    ret = read_adc_value(ADC_PIN8, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P1V2_ABC :
                    ret = read_adc_value(ADC_PIN9, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P1V2_DEF :
                    ret = read_adc_value(ADC_PIN10, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P1V2_GHJ :
                    ret = read_adc_value(ADC_PIN11, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P1V2_KLM :
                    ret = read_adc_value(ADC_PIN12, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P1V0_CPU0 :
                    ret = read_adc_value(ADC_PIN13, ADC_VALUE, (int*) value);
                break;
                case PEB_SENSOR_ADC_P1V0_CPU1 :
                    ret =  read_adc_value(ADC_PIN14, ADC_VALUE, (int*) value);
                break;
                default:
                    ret = -1;
                break;
            }
        break;

        case FRU_PDPB:
            switch(sensor_num) {
                case PDPB_SENSOR_TEMP_CPU1_FRONT :
                    ret = read_tmp75_temp_value(PDPB_TMP75_U47_DEVICE, (int *) value);
                    //cout<<"PDPB_SENSOR_TEMP_CPU1_FRONT"<<endl;
                    //ret = read_temp_value(I2C_DEV_PDPB, PDPB_TMP75_U70 , LOCAL_SENSOR, (uint32_t*) value);
                break;
                case PDPB_SENSOR_TEMP_LEFT_REAR :
                    ret = read_tmp75_temp_value(PDPB_TMP75_U48_DEVICE, (int *) value);
                    //cout<<"PDPB_SENSOR_TEMP_LEFT_REAR"<<endl;
                    //ret = read_temp_value(I2C_DEV_PDPB, PDPB_TMP75_U71 , LOCAL_SENSOR, (uint32_t*) value);
                break;
                case PDPB_SENSOR_TEMP_NEAR_BMC :
                    ret = read_tmp75_temp_value(PDPB_TMP75_U49_DEVICE, (int *) value);//read_temp_value(I2C_DEV_PDPB, PDPB_TMP75_U72 , LOCAL_SENSOR, (uint32_t*) value);
                break;
                case PDPB_SENSOR_TEMP_NEAR_CPU0 :
                    ret = read_tmp75_temp_value(PDPB_TMP75_U51_DEVICE, (int *) value);//read_temp_value(I2C_DEV_PDPB, PDPB_TMP75_U73 , LOCAL_SENSOR, (uint32_t*) value);
                break;
                case PDPB_SENSOR_TEMP_RIGHT_REAR :
                    ret = read_tmp75_temp_value(PDPB_TMP75_U52_DEVICE, (int *) value);//read_temp_value(I2C_DEV_PDPB, PDPB_TMP75_U69 , LOCAL_SENSOR, (uint32_t*) value);
                break;
                case PDPB_SENSOR_TEMP_CPU1_FRONT2 :
                    ret = read_tmp75_temp_value(PDPB_TMP75_U53_DEVICE, (int *) value);//read_temp_value(I2C_DEV_PDPB, PDPB_TMP75_U205 , LOCAL_SENSOR, (uint32_t*) value);
                break;
                case PDPB_SENSOR_TEMP_CPU0:
                    tmp = get_value_from_SMLTR(sensor_num);
                    cout <<"lightning_sensor_read PDPB_SENSOR_TEMP_CPU0"<<tmp<<endl;
                    if (tmp != 0xff)//(temp_data[0] != 0xff)
                        *value = g_Tmax - tmp;//temp_data[0];
                    else
                        *value = 0;
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1:
                    tmp = get_value_from_SMLTR(sensor_num);
                    if(tmp != 0xff)//(temp_data[1] != 0xff)
                        *value = g_Tmax - tmp;//temp_data[1];
                    else
                        *value = 0;
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM0:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[2];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM1:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[3];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH0_DIMM2:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[4];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM0:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[5];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM1:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[6];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH1_DIMM2:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[7];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM0:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[8];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM1:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[9];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH2_DIMM2:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[10];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM0:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[11];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM1:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[12];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU0_CH3_DIMM2:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[13];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM0:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[14];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM1:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[15];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH0_DIMM2:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[16];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM0:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[17];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM1:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[18];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH1_DIMM2:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[19];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM0:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[20];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM1:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[21];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH2_DIMM2:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[22];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM0:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[23];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM1:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[24];
                    ret = 1;
                break;
                case PDPB_SENSOR_TEMP_CPU1_CH3_DIMM2:
                    *value = get_value_from_SMLTR(sensor_num);//temp_data[25];
                    ret = 1;
                break;
                case PDPB_SENSOR_LEFT_REAR_TEMP:
                    ret = read_tmp75_temp_value(PDPB_TMP75_U47_DEVICE, (int*) value);
                break;
                case PDPB_SENSOR_LEFT_FRONT_TEMP:
                    ret = read_tmp75_temp_value(PDPB_TMP75_U49_DEVICE, (int*) value);
                break;
                case PDPB_SENSOR_RIGHT_REAR_TEMP:
                    ret = read_tmp75_temp_value(PDPB_TMP75_U48_DEVICE, (int*) value);
                break;
                case PDPB_SENSOR_RIGHT_FRONT_TEMP:
                    ret = read_tmp75_temp_value(PDPB_TMP75_U51_DEVICE, (int*) value);
                break;
                default:
                    ret= -1;
                break;
            }
        break;
        case FRU_NVA:
            switch(sensor_num){
                case NVA_SENSOR_PSU1_TEMP:
                    //cout<<("lightning_sensor_read: %s %02X %02X",I2C_PSU_DEV_NVA,NVA_PSU_1,NVA_PSU_TEMP);
                    ret = read_tmp75_temp_value(PDPB_TMP75_PSU1_DEVICE, (int *) value);//read_psu_value(I2C_PSU_DEV_NVA, NVA_PSU_1, NVA_PSU_TEMP, (int*) value);	  
                break;
                case NVA_SENSOR_PSU2_TEMP:
                    //cout<<("lightning_sensor_read: %s %02X %02X",I2C_PSU_DEV_NVA,NVA_PSU_2,NVA_PSU_TEMP);
                    ret = read_tmp75_temp_value(PDPB_TMP75_PSU2_DEVICE, (int *) value);//read_psu_value(I2C_PSU_DEV_NVA, NVA_PSU_2, NVA_PSU_TEMP, (int*) value);
                break;
                case NVA_SENSOR_PSU1_TEMP2:
                    cout<<("lightning_sensor_read: %s %02X %02X",I2C_PSU_DEV_NVA,NVA_PSU_1,NVA_PSU_TEMP2);
                    ret = read_psu_value(I2C_PSU_DEV_NVA, NVA_PSU_1, NVA_PSU_TEMP, (int*) value);
                break;
                case NVA_SENSOR_PSU2_TEMP2:
                    cout<<("lightning_sensor_read: %s %02X %02X",I2C_PSU_DEV_NVA,NVA_PSU_2,NVA_PSU_TEMP2);
                    ret = read_psu_value(I2C_PSU_DEV_NVA, NVA_PSU_2, NVA_PSU_TEMP, (int*) value);
                break;
                case NVA_SENSOR_PSU1_FAN1:
                    cout<<("lightning_sensor_read: %s %02X %02X",I2C_PSU_DEV_NVA,NVA_PSU_1,NVA_PSU_FAN1);
                    *value = tacho_data[6];///100;//ret = read_psu_value(I2C_PSU_DEV_NVA, NVA_PSU_1, NVA_PSU_FAN1, (int*) value);
                break;
            
                case NVA_SENSOR_PSU2_FAN1:
                    //cout<<("lightning_sensor_read: %s %02X %02X",I2C_PSU_DEV_NVA,NVA_PSU_2,NVA_PSU_FAN1);
                    //*value = tacho_data[7];///100;//ret = read_psu_value(I2C_PSU_DEV_NVA, NVA_PSU_2, NVA_PSU_FAN1, (int*) value);
                break;
                case NVA_SENSOR_PSU1_WATT:
                    cout<<("NVA_SENSOR_PSU1_WATT read: %s %02X %02X",I2C_PSU_DEV_NVA,NVA_PSU_1,NVA_PSU_WATT);
                    *value = tacho_data[8];
                    //ret = read_psu_value(I2C_PSU_DEV_NVA, NVA_PSU_1, NVA_PSU_WATT, (int*) value);
                    
                break;
                case NVA_SENSOR_PSU2_WATT:
                    cout<<("NVA_SENSOR_PSU2_WATT read: %s %02X %02X",I2C_PSU_DEV_NVA,NVA_PSU_2,NVA_PSU_WATT);
                    *value = tacho_data[9]; 
                    //ret = read_psu_value(I2C_PSU_DEV_NVA, NVA_PSU_2, NVA_PSU_WATT, (int*) value);
                break;
                case NVA_SENSOR_BP_FAN1:
                    *value = tacho_data[0];///100;
                    //cout<<("lightning_sensor_read:" );
                break;
                case NVA_SENSOR_BP_FAN2:
                    *value = tacho_data[1];///100;
                    //cout<<("lightning_sensor_read:" );
                break;
                case NVA_SENSOR_BP_FAN3:
                    *value = tacho_data[2];///100;
                    //cout<<("lightning_sensor_read:" );
                break;
                case NVA_SENSOR_BP_FAN4:
                    *value = tacho_data[3];///100;
                    //cout<<("lightning_sensor_read:" );
                break;
                case NVA_SENSOR_BP_FAN5:
                    *value = tacho_data[4];///100;
                    //cout<<("lightning_sensor_read:" );
                break;
                case NVA_SENSOR_BP_FAN6:
                    *value = tacho_data[5];///100;
                    //cout<<("lightning_sensor_read:" );
                break;
            }	    
        break;
    }
    //pthread_mutex_unlock(&m_sensor);
    return ret;
 }
/**
 * @brief smbus test
 * @bug 테스트단계 보드가없어 확인불가
 */
 int wiringPiI2CRead (int fd)
{
  union i2c_smbus_data data ;

  if (i2c_smbus_access (fd, I2C_SMBUS_READ, 0, I2C_SMBUS_BYTE, &data))
    return -1 ;
  else
    return data.byte & 0xFF ;
}
/**
 * @brief smbus test reg 버전
 * @bug 테스트단계 보드가없어 확인불가
 */
int wiringPiI2CReadReg8 (int fd, int reg)
{
  union i2c_smbus_data data;

  if (i2c_smbus_access (fd, I2C_SMBUS_READ, reg, I2C_SMBUS_BYTE_DATA, &data))
    return -1 ;
  else
    return data.byte & 0xFF ;
}
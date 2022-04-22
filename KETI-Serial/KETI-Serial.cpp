
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

#include <dirent.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <errno.h>
#include <time.h>

#define BAUDRATE B115200
#define MODEMDEVICE "/dev/ttyS0"
#define _POSIX_SOURCE 1

#define FALSE 0
#define TRUE 1
#define CRTSCTS 020000000000
#define MAX 255
#define DATA_MAX 20
#define GET_DONE 0

////////////////////////
FILE *fp1;
FILE *fp2;

long buffsize = 0;

long rFilesize = 0;

char serial_buf[MAX];
int fd;
int res;
struct termios oldtio, newtio;
//////////////////////////////////////////////////////////////////

void serial_open() {

  fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);
  if (fd < 0) {
    perror(MODEMDEVICE);
    exit(-1);
  }

  tcgetattr(fd, &oldtio);
  bzero(&newtio, sizeof(newtio));

  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;

  newtio.c_iflag = ICRNL;

  newtio.c_lflag = ICANON;

  tcflush(fd, TCIFLUSH);
  tcsetattr(fd, TCSANOW, &newtio);
}

void serial_close() {
  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
}

int get_data() {
  fp2 = fopen("c", "wb"); // 파일을 복사할때는 binary 모드로 여는 편이 좋아요
  rFilesize = 72;
  while (rFilesize > 0) // 파일 크기만큼 반복
  {
    fread(serial_buf, buffsize, 1, fd);
    printf("1%s1", serial_buf);
    write(fp2, serial_buf, sizeof(serial_buf));

    rFilesize--;
    bzero(serial_buf, MAX);
  }

  return 0;
}

int main(void) {
  serial_open();
  fp2 = fopen("c", "wb");
  res = 0;

  while (read(fd, serial_buf, DATA_MAX)) {
    printf("%s\n\n %d", serial_buf, res);
    fwrite(serial_buf, 255, 1, fp2);
    bzero(serial_buf, MAX);
  }

  return 0;
}

// int main() {
//   ofstream output("/redfish/data.bin", ios::out | ios::binary);
//   printf("\n\n===============KETI-Serial Start \n");
//   // serial_open();
//   fp2 = fopen("c", "wb");
//   res = 0;
//   if (fp2 <= 0) {
//     printf("KETI-Serial Open Error");
//   }
//   while (read(fd, serial_buf, MAX)) {
//     printf("KETI_SERIAL : %s\n", serial_buf);
//     output.write(serial_buf, sizeof(serial_buf));
//     fwrite(serial_buf, sizeof(serial_buf), 1, fp2);
//     memset(serial_buf, 0, MAX);
//   }
//   output.close();

//   return 0;
// }
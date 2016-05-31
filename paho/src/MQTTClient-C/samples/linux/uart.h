#ifndef _UART_H_
#define _UART_H_

#include <cJSON.h>
#define FALSE  -1
#define TRUE   0
/*传感器类型*/
#define TEMPERATURE    	 0x01
#define HUMIDITY       	 0x02
#define VOLTAGE       	 0x03
#define ILLUMINANCE   	 0x04
#define PRESSURE      	 0x05
#define INFRARED     	 0x06
#define ACCELERATION  	 0x07
/*接受的数据格式*/
typedef signed      char int8;
typedef unsigned    char uint8;
typedef struct RFTXBUF
{
    uint8   No;
	uint8   myNWK[2];//自身网络地址
	uint8   myMAC[8];//mac;
}RFTX;
typedef struct
{
	uint8   type;
    uint8   record[3];
    RFTX    addr;
}Data_t;


int UART_Open(int fd,char* port);
void UART_Close(int fd);
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
int UART_Init(int fd, int speed,int flow_ctrlint ,int databits,int stopbits,char parity);
int UART_Recv(int fd, char *rcv_buf,int data_len);
int UART_Send(int fd, char *send_buf,int data_len);
void UART_JSON(char *rec_data,int len,cJSON *root );

#endif

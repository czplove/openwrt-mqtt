#ifndef _UART_H_
#define _UART_H_

#define FALSE  -1
#define TRUE   0
/*接受的数据格式*/
typedef signed      char int8;
typedef unsigned    char uint8;
typedef struct RFTXBUF
{
	uint8   myNWK[4];//自身网络地址
	uint8   myMAC[16];//mac;
	uint8   pNWK[4];//父节点网络地址
	uint8   pMAC[16];//mac
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

#endif

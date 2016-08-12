#ifndef _UART_H_
#define _UART_H_

#define FALSE  -1
#define TRUE   0


int UART_Open(int fd,char* port);
void UART_Close(int fd);
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
int UART_Init(int fd, int speed,int flow_ctrlint ,int databits,int stopbits,char parity);
int UART_Recv(int fd, char *rcv_buf,int data_len);
int UART_Send(int fd, char *send_buf,int data_len);

#endif

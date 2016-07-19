#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<termios.h>
#include<errno.h>
#include<string.h>

#define FALSE  -1
#define TRUE   0

int UART_Open(int fd,char* port);
void UART_Close(int fd);
int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
int UART_Init(int fd, int speed,int flow_ctrlint ,int databits,int stopbits,char parity);
int UART_Recv(int fd, unsigned char *rcv_buf,int data_len);
int UART_Send(int fd, unsigned char *send_buf,int data_len);


int UART_Open(int fd,char* port)
{

  	fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
  	if (FALSE == fd){
		perror("Can't Open Serial Port");
  		return(FALSE);
  	}
  	if(fcntl(fd, F_SETFL, 0) < 0){
		printf("fcntl failed!\n");
    		return(FALSE);
  	} else {
       	//	printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
  	}
  	if(0 == isatty(STDIN_FILENO)){
  		printf("standard input is not a terminal device\n");
        	return(FALSE);
  	}
  	return fd;
}
void UART_Close(int fd)
{
	close(fd);
}


int UART_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{

    	int   i;
  //	int   status;
  	int   speed_arr[] = { B38400, B19200, B9600, B4800, B2400, B1200, B300,
          		     B38400, B19200, B9600, B4800, B2400, B1200, B300
			    };
    	int   name_arr[] = {
			   38400,  19200,  9600,  4800,  2400,  1200,  300, 38400,
          		   19200,  9600, 4800, 2400, 1200,  300
			  };
	struct termios options;


	if(tcgetattr( fd,&options)  !=  0){
	   perror("SetupSerial 1");
	   return(FALSE);
    	}
	for(i= 0;i < sizeof(speed_arr) / sizeof(int);i++) {
		if  (speed == name_arr[i]) {
      			cfsetispeed(&options, speed_arr[i]);
      			cfsetospeed(&options, speed_arr[i]);
		}
    	}
	options.c_cflag |= CLOCAL;
	options.c_cflag |= CREAD;
	switch(flow_ctrl){
		case 0 :
			options.c_cflag &= ~CRTSCTS;
			break;
    		case 1 :
    			options.c_cflag |= CRTSCTS;
    			break;
    		case 2 :
    			options.c_cflag |= IXON | IXOFF | IXANY;
    			break;
	}

	options.c_cflag &= ~CSIZE;
	switch (databits){
		case 5 :
    			options.c_cflag |= CS5;
    			break;
    		case 6	:
    			options.c_cflag |= CS6;
    			break;
    		case 7	:
        		options.c_cflag |= CS7;
        		break;
    		case 8:
        		options.c_cflag |= CS8;
        		break;
       		default:
        		fprintf(stderr,"Unsupported data size\n");
        		return (FALSE);
	}
	switch (parity) {
		case 'n':
    		case 'N':
        		options.c_cflag &= ~PARENB;
        		options.c_iflag &= ~INPCK;
        		break;
    		case 'o':
    		case 'O':
        		options.c_cflag |= (PARODD | PARENB);
        		options.c_iflag |= INPCK;
        		break;
    		case 'e':
    		case 'E':
        		options.c_cflag |= PARENB;
        		options.c_cflag &= ~PARODD;
        		options.c_iflag |= INPCK;
        		break;
    		case 's':
    		case 'S':
        		options.c_cflag &= ~PARENB;
        		options.c_cflag &= ~CSTOPB;
        		break;
        	default:
        		fprintf(stderr,"Unsupported parity\n");
        		return (FALSE);
	}
	switch (stopbits){
		case 1:
			options.c_cflag &= ~CSTOPB;
        		break;
    		case 2:
        		options.c_cflag |= CSTOPB;
       			break;
    		default:
         		fprintf(stderr,"Unsupported stop bits\n");
         		return (FALSE);
	}

    	options.c_oflag &= ~OPOST;

	options.c_cc[VTIME] = 0;
	options.c_cc[VMIN] = 0;

	tcflush(fd,TCIFLUSH);

	if(tcsetattr(fd,TCSANOW,&options) != 0){
		perror("com set error!\n");
    		return (FALSE);
	}
	return (TRUE);
}


int UART_Init(int fd, int speed,int flow_ctrlint ,int databits,int stopbits,char parity)
{

	if (FALSE == UART_Set(fd,speed,flow_ctrlint,databits,stopbits,parity)) {
		return FALSE;
    	} else {
   		return  TRUE;
   	}
}




int UART_Recv(int fd, unsigned char *rcv_buf,int data_len)
{
    int len,fs_sel;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    time.tv_sec = 15;
    time.tv_usec = 0;

    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
    if(fs_sel){
	   len = read(fd,rcv_buf,data_len);
		while(len < data_len){
			len += read(fd,rcv_buf+len,data_len-len);
	//		printf("len is %d\n",len);
		}

	   return len;
    	} else {
		return FALSE;
	}
}


int UART_Send(int fd, unsigned char *send_buf,int data_len)
{
    int ret;

    ret = write(fd,send_buf,data_len);
    if (data_len == ret ){
	   return ret;
    } else {
	   tcflush(fd,TCOFLUSH);
	   return FALSE;

    }

}




int main(int argc, char **argv)
{
    int fd = FALSE;
    int ret;
    unsigned char rcv_buf[512];
    int i,j;
    unsigned char x;
    if(argc != 2){
	   printf("Usage: %s /dev/ttySn \n",argv[0]);
	   return FALSE;
    }
    fd = UART_Open(fd,argv[1]);
    if(FALSE == fd){
	   printf("open error\n");
	   exit(1);
    }else{
		printf("open successfully!\n");
	}
    ret  = UART_Init(fd,9600,0,8,1,'N');
    if (FALSE == fd){
	   printf("Set Port Error\n");
	   exit(1);
    }
	else
	{
		printf("Set Port successfully!\n");
	}
	printf("starting writing something...\n");
/*    ret  = UART_Send(fd,"*IDN?\n",6);
    if(FALSE == ret){
	   printf("write error!\n");
	   exit(1);
    }else{
		printf("write successfully!");
	}
*/
    //printf("command: %s\n","*IDN?");
    // memset(rcv_buf,0,sizeof(rcv_buf));
    for(i=0;;i++)
    {

//      ret = UART_Send(fd,"aaaaa\n",6);
//		printf("sending aaaaa ...\n");
	    ret = UART_Recv(fd, rcv_buf,9);
    	if( ret > 0){
		printf("receive something.\n");
	   	rcv_buf[ret]='\0';
		for (j=0;rcv_buf[j]!=0;j++)
		{
			printf("%x ",rcv_buf[j]);
		}
		printf("\n");
	   //	printf("%s",rcv_buf);
//		printf("sending aaaaa ...\n");
  //  	ret = UART_Send(fd,"aaaaa\n",6);
	   } else {
	   printf("cannot receive data1\n");
            break;
	   }
//	 if('\n' == rcv_buf[ret-1])
//		 break;
     }
    UART_Close(fd);
    return 0;
}

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#define BAUDRATE        115200
#define UART_DEVICE     "/dev/ttyS1"
#define FALSE  -1
#define TRUE   0
#define TEMPERATURE  0x01 //温度传感器
#define HUMIDITY     0x02 //湿度传感器
#define VOLTAGE		 0x03 //电压传感器
#define ILLUMINANCE	 0x04 //光照传感器
#define PRESSURE	 0x05 //压力传感器
#define INFRARED	 0x06 //红外线传感器
#define ACCELERATION 0x07 //加速度传感器

typedef unsigned char uint8;

int speed_arr[] = {B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
          		   B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300, };
int name_arr[] = {115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,
		  		  115200, 38400, 19200, 9600, 4800, 2400, 1200,  300, };

/**
*@brief  设置串口通信速率
*@param  fd     类型 int  打开串口的文件句柄
*@param rspeed  类型 int  串口速度
*@return  void
*/
void set_speed(int fd, int speed)
{
  int   i;
  int   status;
  struct termios   Opt;
  tcgetattr(fd, &Opt);
  for ( i = 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
  {
    if (speed == name_arr[i])
	{
      tcflush(fd, TCIOFLUSH);
      cfsetispeed(&Opt, speed_arr[i]);
      cfsetospeed(&Opt, speed_arr[i]);
      status = tcsetattr(fd, TCSANOW, &Opt);
      if  (status != 0)
	  {
        perror("tcsetattr fail!");
        return;
      }
      tcflush(fd,TCIOFLUSH);
    }
  }
  printf("Set Speed Success!\n");
}

/**
*@brief   设置串口数据位，停止位和效验位
*@param  fd     类型  int  打开的串口文件句柄
*@param  databits 类型  int 数据位   取值 为 7 或者8
*@param  stopbits 类型  int 停止位   取值为 1 或者2
*@param  parity  类型  int  效验类型 取值为N,E,O,,S
*/
int set_Parity(int fd,int databits,int stopbits,int parity)
{
	struct termios options;
	if( tcgetattr( fd,&options)  !=  0)
	{
		perror("Setup Serial fail!");
		return(FALSE);
	}
	options.c_cflag &= ~CSIZE;
	switch (databits) /*设置数据位数*/
	{
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr,"Unsupported data size\n"); return (FALSE);
	}
	switch (parity)
	{
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;    /* Clear parity enable */
			options.c_iflag &= ~INPCK;     /* Enable parity checking */
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
			options.c_iflag |= INPCK;             /* Disnable parity checking */
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     /* Enable parity */
			options.c_cflag &= ~PARODD;    /* 转换为偶效验*/
			options.c_iflag |= INPCK;      /* Disnable parity checking */
			break;
		case 'S':
		case 's':  /*as no parity*/
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;break;
		default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
		}
	/* 设置停止位*/
	switch (stopbits)
	{
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
	/* Set input parity option */
	if (parity != 'n')
		options.c_iflag |= INPCK;
	tcflush(fd,TCIFLUSH);
	options.c_cc[VTIME] = 150; /* 设置超时15 seconds*/
	options.c_cc[VMIN] = 0;
	if (tcsetattr(fd,TCSANOW,&options) != 0)
	{
		perror("Setup Serial 3");
		return (FALSE);
	}
	options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
	options.c_oflag  &= ~OPOST;   /*Output*/
	return (TRUE);
}

void To_string(uint8 *dest, char *src, int length)
{
	uint8 *xad;
	int i = 0;
	uint8 ch;
	xad = src + length - 1;
	for(i = 0; i < length; i++, xad--)
	{
		ch = (*xad >> 4) & 0x0F;
		dest[i << 1] = ch + ((ch < 10) ? '0' : '7');
		ch = *xad & 0x0F;
		dest[(i << 1) + 1] = ch + ((ch < 10) ? '0' : '7');
	}
}

int main(int argc, char *argv[])
{
    int   fd, res, ret;
    char  buf[255];
	char  buf2[40];
	int	  i = 0;
	FILE  *fp1, *fp2;
	time_t T;
	struct tm *timenow;
	char ch1;
	uint8 dest[16] = {0};

	fp1 = fp2 = NULL;
	fp1 = fopen("./log1.txt","a+");	//追加方式打开日志文件1(数据处理后的日志)
	fp2 = fopen("./log2.txt","a+");	//追加方式打开日志文件2(原始数据的日志)

	if (fp1 == NULL || fp2 == NULL)
	{
		printf("fopen fail!\n");
		exit(0);
	}
	printf("fopen success!\n");

	ch1 = fgetc(fp1);
	if (ch1 == EOF)//判断是否已有日志文件
	{
		fprintf(fp1,"%s","传感器类型    传感器读数    编号    节点网络地址    节点MAC地址         时间\n\n");
		fflush(fp1);
	}

    fd = open(UART_DEVICE, O_RDWR);//读写方式打开串口
    if (fd < 0)
	{
        printf("Open UART_DEVICE Error!\n");
        exit (1);
	}
	else
		printf("Open UART_DEVICE Success!\n");

    set_speed(fd,BAUDRATE);	//设置波特率
	if (set_Parity(fd,8,1,'N') == FALSE)  //设置数据位,结束位,校验位
	{
		printf("Set Parity Error\n");
		exit (0);
	}
	else
		printf("Set Parity Success!\n");


   while(1) //循环读取串口数据
	{
		memset(buf, 0, sizeof(buf));
        res = read(fd, buf, 1);
        if(res == 0)
            continue;
		else
		{
			time(&T);
			timenow = localtime(&T);//记录收到数据的时间
		}

		if(buf[0] != '@')	//数据传输的开始标识'@'
        {
            printf("is not @\n");
			continue;
        }
		memset(buf, 0, sizeof(buf));
		res = read(fd, buf, 1);//读取数据类型，判断是传感器数据还是控制数据
		if(res == 0)
			continue;

		if(buf[0] == 'x' || buf[0] == 'y')	//判断是否是控制信息
		{
			if(buf[0] == 'x')	//收到stop-send控制信息
			{
				sleep(5);
				printf("停止成功(指令下达或有延迟,若出现最后一次数据,在可接受范围内!)\n");
			res = read(fd, buf, 1);//将随后的结束符读走
			printf("============================================\n");
				continue;
			}
			else 	//收到startsend控制信息
			{
				sleep(10);
				printf("开启成功!\n");
			res = read(fd, buf, 1);//将随后的结束符读走
			printf("============================================\n");
				continue;
			}
		}
		else//收到传感器类型的数据
		{
			fprintf(fp2,"%-2x ", buf[0]&0xff);//记录传感器类型

			switch(buf[0])	//判断传感器类型
			{
				case 0x01://温度传感器(暂时没用到)
						memset(buf, 0, sizeof(buf));
						res = read(fd, buf, 4);	//读取传感器传来的数据值
						if(res == 0)
							continue;
						while(res < 4)
						{
							memset(buf2, 0, sizeof(buf2));
							ret = read(fd, buf2, 4 - res);
							res += ret;
							strcat(buf,buf2);
						}

						{
							printf("温度 %d 摄氏度\n",buf[0]);
							fprintf(fp1,"%s","温度传感器    ");
							fprintf(fp1,"    %-10d",buf[0]);
							fflush(fp1);
						}

					break;
				case 0x02://湿度传感器
						memset(buf, 0, sizeof(buf));
						res = read(fd, buf, 4);	//读取传感器传来的数据值
						if(res == 0)
							continue;
						while(res < 4)
						{
							memset(buf2, 0, sizeof(buf2));
							ret = read(fd, buf2, 4 - res);
							res += ret;
							strcat(buf,buf2);
						}

						{
							double temp ;
							temp = ((buf[0]&0xff) + (buf[2]&0xff) / 100.0);

							printf("温度 %.2lf ℃ \n",temp);
							printf("湿度 %d%% RH\n",buf[1]&0xff);

							fprintf(fp1,"%s","湿度传感器    ");
							fprintf(fp1,"%s:%.2lf\n","温度",temp);
							fprintf(fp1,"              %s:%-10d","湿度",buf[1]&0xff);
							fflush(fp1);
						}

					break;
				case 0x03://电压传感器
						memset(buf, 0, sizeof(buf));
						res = read(fd, buf, 4);	//读取传感器传来的数据值
						if(res == 0)
							continue;
						while(res < 4)
						{
							memset(buf2, 0, sizeof(buf2));
							ret = read(fd, buf2, 4 - res);
							res += ret;
							strcat(buf,buf2);
						}


						{
							printf("电压 %u V\n",buf[0]&0xff);
							fprintf(fp1,"%s","电压传感器    ");
							fprintf(fp1,"%-14d",buf[0]&0xff);
							fflush(fp1);
						}

					break;
				case 0x04://光照传感器
						memset(buf, 0, sizeof(buf));
						res = read(fd, buf, 4);	//读取传感器传来的数据值
						if(res == 0)
							continue;
						while(res < 4)
						{
							memset(buf2, 0, sizeof(buf2));
							ret = read(fd, buf2, 4 - res);
							res += ret;
							strcat(buf,buf2);
						}

						{
							uint8 illu;
							illu = ((buf[1]&0xff) << 8) | (buf[0]&0xff);

							printf("光照度 %u  Lx\n", illu);
							fprintf(fp1,"%s","光照传感器    ");
							fprintf(fp1,"%-14d",illu);
							fflush(fp1);
						}

					break;
				case 0x05://气压传感器
						memset(buf, 0, sizeof(buf));
						res = read(fd, buf, 4);	//读取传感器传来的数据值
						if(res == 0)
							continue;
						while(res < 4)
						{
							memset(buf2, 0, sizeof(buf2));
							ret = read(fd, buf2, 4 - res);
							res += ret;
							strcat(buf,buf2);
						}

						{
							printf("%s", "气压 ");
							fprintf(fp1,"%s","气压传感器    ");

							for(i = 3; i > 0; --i)
							{
								if(i == 1)
									printf("%c",'.');
								printf("%u",buf[i]&0xff);
							}
							printf("%s"," kPa");

							fprintf(fp1,"%u%u%c%u%u     ",buf[3]&0xff,buf[2]&0xff,'.',buf[1]&0xff,buf[0]&0xff);
							fflush(fp1);
						}

					break;
				case 0x06://红外传感器
						memset(buf, 0, sizeof(buf));
						res = read(fd, buf, 4);	//读取传感器传来的数据值
						if(res == 0)
							continue;
						while(res < 4)
						{
							memset(buf2, 0, sizeof(buf2));
							ret = read(fd, buf2, 4 - res);
							res += ret;
							strcat(buf,buf2);
						}

						{
							printf("红外线 %u mW\n",buf[0]&0xff);
							fprintf(fp1,"%s","红外传感器    ");
							fprintf(fp1,"%-14u",buf[0]&0xff);
							fflush(fp1);
						}

					break;
				case 0x07://加速度传感器
						memset(buf, 0, sizeof(buf));
						res = read(fd, buf, 4);	//读取传感器传来的数据值
						if(res == 0)
							continue;
						while(res < 4)
						{
							memset(buf2, 0, sizeof(buf2));
							ret = read(fd, buf2, 4 - res);
							res += ret;
							strcat(buf,buf2);
						}

						{
							printf("x方向加速度 %u m/s2\n",buf[0]&0xff);
							printf("y方向加速度 %u m/s2\n",buf[1]&0xff);
							printf("z方向加速度 %u m/s2\n",buf[2]&0xff);

							fprintf(fp1,"%s","加速度传感器  ");
							fprintf(fp1, "x:%-14u\n", buf[0]&0xff);
							fprintf(fp1, "              y:%-14u\n", buf[1]&0xff);
							fprintf(fp1, "              z:%-13u", buf[2]&0xff);
							fflush(fp1);
						}

					break;
				default:
					printf("未知传感器!\n");
					break;
			}

			for(i = 0; i < 4; i++)
			{
				fprintf(fp2,"%-2x ",buf[i]&0xff);
				fflush(fp2);
			}
		}

		memset(buf, 0, sizeof(buf));
        res = read(fd, buf, 11);	//读取数据包的地址信息
        if(res == 0)
            continue;
		while(res < 11)
		{
			memset(buf2, 0, sizeof(buf2));
			ret = read(fd, buf2, 11 - res);
			res += ret;
			strcat(buf,buf2);
		}

		printf("编号:%u\n",buf[0]&0xff);
		fprintf(fp1,"%-8u",buf[0]&0xff);
		fflush(fp1);
		fprintf(fp2,"%-2x ",buf[0]&0xff);
		fflush(fp2);

		printf("节点网络地址:");//	按定义的数据包结构解析地址
		for (i = 2; i > 0; --i)
		{
			printf("%x", buf[i]&0xff);
			fprintf(fp1,"%x",buf[i]&0xff);
		}
		printf("\n");
		fprintf(fp1,"%s","           ");
		fflush(fp1);

		for (i = 1; i < 3; ++i)
			fprintf(fp2,"%-2x ",buf[i]&0xff);
		fflush(fp2);

		printf("节点MAC地址:");
		To_string(dest, &buf[3], 8);
		for(i = 3; i < 11; ++i)
			fprintf(fp2, "%-2x ", buf[i]&0xff);
		fflush(fp2);

		for (i = 0; i < 16; ++i)
		{
			printf("%c", dest[i]);
			if((i + 1) % 2 == 0 && i < 15)
				printf("%c", '-');
			fprintf(fp1,"%c",dest[i]);
		}
			fprintf(fp1,"%s","    ");
			fflush(fp1);
//		printf("\n");

/*		printf("父节点的网络地址是:");
		fprintf(fp1,"%s","    ");
		for (i = 20; i < 24; ++i)
		{
			printf("%c", buf[i]);
			fprintf(fp1,"%c",buf[i]);
		}
			fprintf(fp1,"%s","            ");
			fflush(fp1);
		printf("\n");

		printf("父节点的MAC地址是:");
		for (i = 24; i < 40; ++i)
		{
			printf("%c", buf[i]);
			fprintf(fp1,"%c",buf[i]);
		}
*/
			fprintf(fp1,"%s",asctime(timenow));
			fflush(fp1);

        res = read(fd, buf, 1);
        if(res == 0)
            continue;
		if(buf[0] == '\n')	//将数据传输的结束标志@读走
		{
			printf("\n");
			fprintf(fp1,"%s","\n");
			fflush(fp1);
		}
	printf("============================================\n");

/*
	for(i = 0; i < 10; i++)
	{
		fprintf(fp2,"%-2x ",buf[i]&0x0f);
		fflush(fp2);
	}
*/
	fprintf(fp2,"\n%s%s\n","======================================================",asctime(timenow));
	fflush(fp2);

	}

	fclose(fp1);
	fclose(fp2);
    printf("Close...\n");
    close(fd);
    return 0;
}




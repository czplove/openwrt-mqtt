/*******************************************************************************
 * Copyright (c) 2012, 2013 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *   http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial contribution
 *    Ian Craggs - change delimiter option from char to string
 *    Al Stockdill-Mander - Version using the embedded C client
 *******************************************************************************/

/*

 stdout publish

 compulsory parameters:

  topic to publish to

 defaulted parameters:

	--host localhost
	--port 1883
	--qos 2
	--delimiter \n
	--clientid stdout_subscriber

	--userid none
	--password none

 for example:

    stdoutpub topic/of/interest --host iot.eclipse.org

*/
#include <stdio.h>
#include <signal.h>
#include <memory.h>

#include <sys/time.h>

#include "MQTTClient.h"
#include "uart.h"
#include "cJSON.h"

#define UART_DEVICE_1 "/dev/ttyS1"
#define UART_DEVICE_0 "/dev/ttyS0"
#define BAUDRATE 9600
#define MSG_LEN_PH 9
#define MSG_LEN_ORP 17
volatile int toStop = 0;



void usage()
{
	printf("MQTT stdout publisher\n");
	printf("Usage: stdoutpub topicname <options>, where options are:\n");
	printf("  --host <hostname> (default is localhost)\n");
	printf("  --port <port> (default is 1883)\n");
	printf("  --qos <qos> (default is 2)\n");
	printf("  --delimiter <delim> (default is \\n)\n");
	printf("  --clientid <clientid> (default is hostname+timestamp)\n");
	printf("  --username none\n");
	printf("  --password none\n");
	printf("  --showtopics <on or off> (default is on if the topic has a wildcard, else off)\n");
	exit(-1);
}


void cfinish(int sig)
{
	signal(SIGINT, NULL);
	toStop = 1;
}


struct opts_struct
{
	char* clientid;
	int nodelimiter;
	char* delimiter;
	enum QoS qos;
	char* username;
	char* password;
	char* host;
	int port;
	int showtopics;
} opts =
{
    (char*)"wrtnode-publisher", 0, (char*)"\n", QOS2, NULL, NULL, (char*)"localhost", 1883, 0
};


void getopts(int argc, char** argv)
{
	int count = 2;

	while (count < argc)
	{
		if (strcmp(argv[count], "--qos") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "0") == 0)
					opts.qos = QOS0;
				else if (strcmp(argv[count], "1") == 0)
					opts.qos = QOS1;
				else if (strcmp(argv[count], "2") == 0)
					opts.qos = QOS2;
				else
					usage();
			}
			else
				usage();
		}
		else if (strcmp(argv[count], "--host") == 0)
		{
			if (++count < argc)
				opts.host = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--port") == 0)
		{
			if (++count < argc)
				opts.port = atoi(argv[count]);
			else
				usage();
		}
		else if (strcmp(argv[count], "--clientid") == 0)
		{
			if (++count < argc)
				opts.clientid = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--username") == 0)
		{
			if (++count < argc)
				opts.username = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--password") == 0)
		{
			if (++count < argc)
				opts.password = argv[count];
			else
				usage();
		}
		else if (strcmp(argv[count], "--delimiter") == 0)
		{
			if (++count < argc)
				opts.delimiter = argv[count];
			else
				opts.nodelimiter = 1;
		}
		else if (strcmp(argv[count], "--showtopics") == 0)
		{
			if (++count < argc)
			{
				if (strcmp(argv[count], "on") == 0)
					opts.showtopics = 1;
				else if (strcmp(argv[count], "off") == 0)
					opts.showtopics = 0;
				else
					usage();
			}
			else
				usage();
		}
		count++;
	}

}


void messageArrived(MessageData* md)
{
	MQTTMessage* message = md->message;

	if (opts.showtopics)
		printf("%.*s\t", md->topicName->lenstring.len, md->topicName->lenstring.data);
	if (opts.nodelimiter)
		printf("%.*s", (int)message->payloadlen, (char*)message->payload);
	else
		printf("%.*s%s", (int)message->payloadlen, (char*)message->payload, opts.delimiter);
	//fflush(stdout);
}
/*
void To_string(uint8 *dest,char *src,uint8 length)
{
  uint8 *xad;
  uint8 i=0;
  uint8 ch;
  xad=src+length-1;
  for(i=0;i<length;i++,xad--)
  {
    ch=(*xad>>4)&0x0F;
    dest[i<<1]=ch+((ch<10)?'0':'7');
    ch=*xad&0x0F;
    dest[(i<<1)+1]=ch+((ch<10)?'0':'7');
  }
}
*/
int main(int argc, char** argv)
{
	int rc = 0;
	unsigned char buf[100];
	unsigned char readbuf[100];

    char ph_run = 0,orp_run = 0;
    float ph,ph_temp,orp,orp_temp;

	/**********UART************/
    int		ph_fd = FALSE;
    int		orp_fd = FALSE;
    int 	ret;
    unsigned char	rcv_buf[512];
	unsigned char 	send_buf[64];
    unsigned char  ph_cmd[]  = {0x00, 0x03, 0x00, 0x00, 0x00, 0x02, 0xC5, 0xDA};//用于获取PH传感器数据的发送数据
	unsigned char  orp_cmd[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x06, 0xC5, 0xC8};//用于获取ORP传感器数据的发送数据
    unsigned char  ph_rcv_buf[MSG_LEN_PH] = {0};
    unsigned char  orp_rcv_buf[MSG_LEN_ORP +1] = {0};

    char str[256] = {0};
	Data_t	rcv_data;
    int i;
	int j;

    char *out;
    cJSON *root;
    int     count = 0;

    double temperature;

    /* Open uart and init*/
    ph_fd = UART_Open(ph_fd,UART_DEVICE_1);
    if(FALSE == ph_fd){
	   printf("open error\n");
	   exit(1);
    }

    orp_fd = UART_Open(orp_fd,UART_DEVICE_0);
    if(FALSE == orp_fd){
	   printf("open error\n");
	   exit(1);
    }
    ret  = UART_Init(ph_fd,BAUDRATE,0,8,1,'N');
    if (FALSE == ph_fd){
	   printf("Set Port Error\n");
	   exit(1);
    }

    ret  = UART_Init(orp_fd,BAUDRATE,0,8,1,'N');
    if (FALSE == orp_fd){
	   printf("Set Port Error\n");
	   exit(1);
    }
	if (argc < 2)
		usage();

	char* topic = argv[1];

//	if (strchr(topic, '#') || strchr(topic, '+'))
//		opts.showtopics = 1;
//	if (opts.showtopics)
//		printf("topic is %s\n", topic);

	getopts(argc, argv);

	Network n;
	Client c;
	MQTTMessage message={0,0,0,0,"hello",5};//dup,qos,retained,packetid,payload,payloadlen


	NewNetwork(&n);
	ConnectNetwork(&n, opts.host, opts.port);
	MQTTClient(&c, &n, 1000, buf, 256, readbuf, 256);

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = opts.clientid;
	data.username.cstring = opts.username;
	data.password.cstring = opts.password;

	data.keepAliveInterval = 10;
	data.cleansession = 1;
	printf("Connecting to %s:%d\n", opts.host, opts.port);

	rc = MQTTConnect(&c, &data);
    if(!rc)
        printf("Connected successfully\n");
    else
    {
        printf("Connected to %s:%d failed.\n",opts.host,opts.port);
        exit(-1);
    }

	for(i=0;;i++)
    {
 //       /*
        //memset(ph_rcv_buf,0,sizeof(ph_rcv_buf));
		printf("\nsend cmd(ph):");

		for (i = 0; i < sizeof(ph_cmd); ++i)
			printf("%d:%x  ",i, ph_cmd[i]);
		printf("\n");
        ret = UART_Send(ph_fd, ph_cmd,sizeof(ph_cmd));//send cmd (ph)
        if(ret != sizeof(ph_cmd))
        {
            printf("(PH)Send commend error!\n");
        }
        sleep(1);
        ret = UART_Recv(ph_fd ,ph_rcv_buf ,MSG_LEN_PH );
        printf("(PH)Uart recvive ret is %d.\n",ret);
        if( ret == MSG_LEN_PH )
        {
            printf("The number of data is right,check crc and publish message now.\n");

            if(CRC16(ph_rcv_buf,MSG_LEN_PH))  //   check crc
            {
                // change data　
                ph = (ph_rcv_buf[3] * 16 * 16 + ph_rcv_buf[4]) / 100.0;
		        ph_temp = (ph_rcv_buf[5] * 16 *16 + ph_rcv_buf[6]) / 10.0;

	            root=cJSON_CreateObject();
                cJSON_AddStringToObject(root,"device","PH");
                cJSON_AddNumberToObject(root,"ph",ph);
                cJSON_AddNumberToObject(root,"ph_temp",ph_temp);

                out=cJSON_Print(root);
                printf("%s\n",out);
                message.payload = out ;
                message.payloadlen = strlen(out);
                ret = MQTTPublish(&c,topic,&message);
                printf("ret is %d\n",ret);
                cJSON_Delete(root);
                free(out);// Print to text, Delete the cJSON, print it, release the string.

            }

        }
//*/
		printf("\nsend cmd(orp):");
        for (i = 0; i < sizeof(orp_cmd); ++i)
			printf("%d:%x  ",i, orp_cmd[i]);
		printf("\n");
        ret = UART_Send(orp_fd, orp_cmd,sizeof(orp_cmd));//send cmd (ph)
        if(ret != sizeof(orp_cmd))
        {
            printf("(ORP)Send commend error!\n");
        }
        sleep(1);
        ret = UART_Recv(orp_fd ,orp_rcv_buf ,MSG_LEN_ORP );
        sleep(1);
        printf("(ORP)Uart recvive ret is %d.\n",ret);
        if( ret == MSG_LEN_ORP )
        {
            printf("The number of data is right,check crc and publish message now.\n");
            printf("receive:");
            for(i=0;i < MSG_LEN_ORP;++i)
                printf("%x ",orp_rcv_buf[i]);
            printf("\n");
            if(CRC16(orp_rcv_buf,MSG_LEN_ORP))  //   check crc
            {

                orp = (orp_rcv_buf[3]*16*16 + orp_rcv_buf[4])/1000.0;
                orp_temp = (orp_rcv_buf[5]*16*16 + orp_rcv_buf[6])/100.0;
	            root=cJSON_CreateObject();
                cJSON_AddStringToObject(root,"device","ORP");
                cJSON_AddNumberToObject(root,"orp",orp);
                cJSON_AddNumberToObject(root,"orp_temp",orp_temp);

                out=cJSON_Print(root);
                printf("%s\n",out);
                message.payload = out ;
                message.payloadlen = strlen(out);
                ret = MQTTPublish(&c,topic,&message);
                printf("ret is %d\n",ret);
                cJSON_Delete(root);
                free(out);/* Print to text, Delete the cJSON, print it, release the string. */


            }

			else
            {/*
                printf("the first bit is not ＠");
                while( rcv_buf[0] != '@')
                {
                   if( UART_Recv(fd, rcv_buf,1)>0)


                    {printf("%x-",rcv_buf[0]);
                   // printf("hahah not @\n");
                    if(count++ > 1000)
                    {

                        printf("error,no '＠'，波特率？\n");
                        exit(-1);
                    }

                    }
                }
                UART_Recv(fd, rcv_buf,MSG_LEN - 1);
            */
            }
        }
		else
		{
            /*
            for(j=0;j<ret;j++)
            {
                printf("j: %d char ---%x\n",j,rcv_buf[j]);
            }
    	    printf("cannot receive data1\n");
            */
        //    break;
	   	}

    }
	printf("Subscribed %d\n", rc);

/*	while (!toStop)
	{
		MQTTYield(&c, 1000);
	}
*/	printf("Stopping\n");
 	UART_Close(ph_fd);
	MQTTDisconnect(&c);
	n.disconnect(&n);

	return 0;
}



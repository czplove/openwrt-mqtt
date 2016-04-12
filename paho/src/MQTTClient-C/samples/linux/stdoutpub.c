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
#include "MQTTClient.h"

#include <stdio.h>
#include <signal.h>
#include <memory.h>

#include <sys/time.h>

#include "uart.h"

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
	(char*)"stdout-subscriber", 0, (char*)"\n", QOS2, NULL, NULL, (char*)"localhost", 1883, 0
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


int main(int argc, char** argv)
{
	int rc = 0;
	unsigned char buf[100];
	unsigned char readbuf[100];

	/**********UART************/
	int fd = FALSE;    			  
    int ret;   			   			  
    char	rcv_buf[512];
    int i;

	fd = UART_Open(fd,"/dev/ttyS1"); 
    if(FALSE == fd){	
	   printf("open error\n");	
	   exit(1); 	
    }
    ret  = UART_Init(fd,9600,0,8,1,'N');
    if (FALSE == fd){	
	   printf("Set Port Error\n");	
	   exit(1); 
    }
    ret  = UART_Send(fd,"*IDN?\n",6);
    if(FALSE == ret){
	   printf("write error!\n");
	   exit(1);
    }
    printf("command: %s\n","*IDN?");
    memset(rcv_buf,0,sizeof(rcv_buf));
	
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
	MQTTMessage message={0,0,0,0,"hello",5};


	NewNetwork(&n);
	ConnectNetwork(&n, opts.host, opts.port);
	MQTTClient(&c, &n, 1000, buf, 100, readbuf, 100);
 
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = opts.clientid;
	data.username.cstring = opts.username;
	data.password.cstring = opts.password;

	data.keepAliveInterval = 10;
	data.cleansession = 1;
	printf("Connecting to %s %d\n", opts.host, opts.port);
	
	rc = MQTTConnect(&c, &data);
	printf("Connected %d\n", rc);
    
	for(i=0;;i++)
    {
		ret = UART_Recv(fd, rcv_buf,512);	
    	if( ret > 1)
		{//ret > 0  
	   		rcv_buf[ret]='\0';		

    		printf("Publishing to %s\n", topic);

			message.payload = rcv_buf;
			message.payloadlen = strlen(rcv_buf);
			MQTTPublish(&c,topic,&message);
	   		printf("%s",rcv_buf);	
	   	}
		else 
		{	
	 	//  printf("cannot receive data1\n");	
        //    break;
	   	}
//	 if('\n' == rcv_buf[ret-1])
//		 break;
    }
	printf("Subscribed %d\n", rc);

/*	while (!toStop)
	{
		MQTTYield(&c, 1000);	
	}
*/	printf("Stopping\n");
 	UART_Close(fd);
	MQTTDisconnect(&c);
	n.disconnect(&n);

	return 0;
}



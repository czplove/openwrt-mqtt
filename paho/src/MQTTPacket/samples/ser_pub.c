/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - clarifications and/or documentation extension
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "MQTTPacket.h"
#include "transport.h"
#include "uart.h"


int main(int argc, char *argv[])
{
	/******MQTT*******/
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	int rc = 0;
	char buf[200];
	int buflen = sizeof(buf);
	int mysock = 0;
	MQTTString topicString = MQTTString_initializer;
	char* payload = "mypayload";
	int payloadlen = strlen(payload);
	int len = 0;
	char *host = "192.168.8.241";
	int port = 1883;

	/******UART*******/
	int fd = FALSE;
	int ret;
	char rcv_buf[512];
 	int i;
	if(argc < 2){
		printf("Usage: %s /dev/ttySn \n",argv[0]);
		return FALSE;
	}

	if (argc > 2)
		host = argv[2];

	if (argc > 3)
		port = atoi(argv[3]);
	
	fd = UART_Open(fd,argv[1]);
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

	/**************MQTT tansport*************/

	mysock = transport_open(host,port);
	if(mysock < 0)
		return mysock;

	printf("Sending to hostname %s port %d\n", host, port);

	data.clientID.cstring = "me";
	data.keepAliveInterval = 20;
	data.cleansession = 1;
	data.username.cstring = "testuser";
	data.password.cstring = "testpassword";
	data.MQTTVersion = 4;


	for(i=0;;i++)
	{
	   	ret = UART_Recv(fd, rcv_buf,512);	
		if(ret > 0)
		{
			rcv_buf[ret]='\0';
			printf("%s",rcv_buf);

			len = MQTTSerialize_connect((unsigned char *)buf, buflen, &data);
			topicString.cstring = "mytopic";
		
			len += MQTTSerialize_publish((unsigned char *)(buf + len), buflen - len, 0, 0, 0, 0, topicString, (unsigned char *)rcv_buf, ret+1);
			len += MQTTSerialize_disconnect((unsigned char *)(buf + len), buflen - len);
			rc = transport_sendPacketBuffer(mysock, buf, len);
			if (rc == len)
				printf("Successfully published\n");
			else
				printf("Publish failed\n");
		}
		else
		{
			printf("cannot receive data1\n");
			break;
		}
	}

exit:
	UART_Close(fd);
	transport_close(mysock);

	return 0;
}

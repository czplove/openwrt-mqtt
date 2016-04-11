ifndef srcdir
	srcdir = MQTTPacket/src 
endif

ifndef blddir
	blddir = build/output
endif
SAMPLE_FILES_C = qos0pub
SYNC_SAMPLES = ${addprefix ${blddir}/samples/,${SAMPLE_FILES_C}}
MQTT_EMBED_LIB_C = paho-embed-mqtt3c
FLAGS_EXE = -I ${srcdir}  -L ${blddir}

${SYNC_SAMPLES}: build/output/samples/%: MQTTPacket/src/../samples/%.c MQTTPacket/src/../samples/transport.o
	${CC} -o $@ $^  -l${MQTT_EMBED_LIB_C} ${FLAGS_EXE}
.PHONY:clean
clean:
	rm build/output/samples/* 

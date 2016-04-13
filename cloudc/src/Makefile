all:cloudc
cloudc:main.c cloudc_log.c ubus.c listener.c parser.c handle_sys_feature.c cJSON.c proto.c pthread.c task_queue.c socket.c 
	$(CC) $(LDFALGS) $(CFLAGS)  $^ -o $@ -luci -lubus -lubox -lblobmsg_json -lm -lpthread -lcurl

clean: 
	rm -rf cloudc

.PHONY: all clean

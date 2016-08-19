all:udpclient1
udpclient1:main.c cJSON.c proto.c
	$(CC) $(LDFALGS) $(CFLAGS)  $^ -o $@ -luci -lm

clean: 
	rm -rf udpclient1

.PHONY: all clean

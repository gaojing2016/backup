all:udpclient2
udpclient2:main.c cJSON.c proto.c
	$(CC) $(LDFALGS) $(CFLAGS)  $^ -o $@ -luci -lm

clean: 
	rm -rf udpclient2

.PHONY: all clean

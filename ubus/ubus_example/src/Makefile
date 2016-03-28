all:server client

server:server.c count.c
	$(CC) $(LDFALGS) $(CFLAGS)  $^ -o $@ -lubus -lubox -lblobmsg_json

client:client.c count.c
	$(CC) $(LDFLAGS) $(CFLAGS)  $^ -o $@ -lubus -lubox

clean: 
	rm -rf server client

.PHONY: all clean

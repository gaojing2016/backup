#CC = $(CXX)
#CFLAGS = $(CXXFLAGS)
#CC := g++  
#CFLAGS := -g  
all:wechat
wechat:pthread.o main.o wechat_log.o
#	$(CC) $(LDFALGS) $(CFLAGS) $^ -o $@ -luci -lubus -lubox -lblobmsg_json -lpthread -lrt libhello.a libwxcloud.a
	$(CXX) $(LDFALGS) $(CFLAGS) $^ -o $@ -luci -lubus -lubox -lblobmsg_json -lpthread -lrt -lstdc++ libhello.a libwxcloud.a

clean: 
	rm -rf wechat

.PHONY: all clean


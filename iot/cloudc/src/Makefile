all:cloudc
cloudc:main.o cloudc_log.o ubus.o listener.o parser.o handle_sys_feature.o cJSON.o proto.o pthread.o task_queue.o socket.o \
	cpptest.o alljoyConfigClient.o NewConfigClient.o DataConvert.o \
	AboutDataStore.o AboutObjApi.o AsyncSessionJoiner.o CommonBusListener.o CommonSampleUtil.o SessionListenerImpl.o SrpKeyXListener.o \
	NotificationReceiverTestImpl.o
	$(CXX) $(LDFALGS)  $^ -o $@ -luci -lubus -lubox -lblobmsg_json -lm -lpthread -lcurl \
		-lstdc++ -lmxml \
		-lalljoyn -lalljoyn_about -lalljoyn_config -lalljoyn_controlpanel -lalljoyn_services_common -lalljoyn_notification \
		-lsqlite3

%.o:%.c
	$(CC) $(CFALGS) -c $<

%.o:%.cc
	$(CXX) $(CXXFALGS) -DQCC_OS_LINUX -DQCC_OS_GROUP_POSIX -c $<

%.o:samples_common/%.cc
	$(CXX) $(CXXFALGS) -I./samples_common -DQCC_OS_GROUP_POSIX -DQCC_OS_LINUX -g -c $<

%.o:common/%.cc
	$(CXX) $(CXXFALGS) -I./samples_common -DQCC_OS_GROUP_POSIX -DQCC_OS_LINUX -g -c $<

clean: 
	rm -rf cloudc

.PHONY: all clean

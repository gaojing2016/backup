include $(TOPDIR)/rules.mk

DLIB  = -lubus -lubox -lblobmsg_json -lm -lpthread \
		-lalljoyn -lalljoyn_about -lalljoyn_config -lalljoyn_controlpanel -lalljoyn_services_common -lalljoyn_notification -lsqlite3 \
		-lmxml
DLIB_FLAGS = 
OBJ_PATH = ./obj
OBJ = ubus.o cJSON.o parser.o proto.o sqlite3.o \
	  OptParser.o ConfigServiceListenerImpl.o \
	AboutDataStore.o AsyncSessionJoiner.o \
	AnnounceHandlerImpl.o CommonSampleUtil.o \
	SrpKeyXListener.o AboutObjApi.o \
	CommonBusListener.o SessionListenerImpl.o \
	ConfigServiceMain.o 
temp = $(wildcard *~)

all_programs = testadapter
all:$(all_programs)
.PHONY:all

$(all_programs): $(OBJ)
#	cd $(OBJ_PATH)
	$(TARGET_CXX) $(TARGET_LDFLAGS) $(TARGET_CPPFLAGS) $(DLIB_FLAGS) $(DLIB) $^ -o $@ 

%.o:%.c
	#$(CC) $(CFALGS) -c $<
	$(TARGET_CC) $(TARGET_CFLAGS) -c $<  

%.o:%.cc
	$(TARGET_CXX) $(TARGET_CPPFLAGS) -I./samples_common -DQCC_OS_GROUP_POSIX -DQCC_OS_LINUX -g -c $<  
#	mv $@ $(OBJ_PATH)

%.o:samples_common/%.cc
	$(TARGET_CXX) $(TARGET_CPPFLAGS) -I./samples_common -DQCC_OS_GROUP_POSIX -DQCC_OS_LINUX -DNEED_DATA_STORE -g -c $<  
#	mv $@ $(OBJ_PATH)
clean:
	rm -rf $(OBJ_PATH) $(temp) $(all_programs)


/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#include <stdio.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <alljoyn/BusAttachment.h>
#include <alljoyn/config/ConfigService.h>
#include <alljoyn/AboutIconObj.h>
#include <SrpKeyXListener.h>
#include <CommonBusListener.h>
#include <CommonSampleUtil.h>
#include <sqlite3.h>

#include "AboutDataStore.h"
#include <alljoyn/AboutObj.h>
#include "ConfigServiceListenerImpl.h"
#include "OptParser.h"
#include <alljoyn/services_common/LogModulesNames.h>

#define DEFAULTPASSCODE "000000"
#define SERVICE_EXIT_OK       0
#define SERVICE_OPTION_ERROR  1
#define SERVICE_CONFIG_ERROR  2

#define CHECK_RETURN(x) if ((status = x) != ER_OK) { return status; }

using namespace ajn;
using namespace services;

const int ONLINE_MSG_HEAD_LEN = 5;
const int ONLINE_MSG_FACTORY_LEN = 8;
const int FACTORY_LEN = 2;
const int ONLINE_MSG_DEV_TYPE_LEN = 8;
const int DEV_TYPE_LEN = 3;
const int ONLINE_DEV_ID_LEN = 6;
const int DEV_ID_LEN = 6;
const int REPORT_MSG_LEN = 6;

const int CONFNAMELEN  =  FACTORY_LEN + DEV_TYPE_LEN + DEV_ID_LEN + 5 + 1;
typedef struct ConfData{
    struct ConfData *next;
    char confName[CONFNAMELEN];
}ConfigData;

struct ConfData *confDataHead = NULL;
struct ConfData *confDataTail = NULL;

/** static variables need for sample */
static BusAttachment* msgBus = NULL;

static SrpKeyXListener* keyListener = NULL;

static ConfigService* configService = NULL;

static AboutIcon* icon = NULL;
static AboutIconObj* aboutIconObj = NULL;


static AboutDataStore* aboutDataStore = NULL;
static AboutObj* aboutObj = NULL;

static ConfigServiceListenerImpl* configServiceListener = NULL;

static CommonBusListener* busListener = NULL;

static SessionPort servicePort = 900;

static volatile sig_atomic_t s_interrupt = false;

static volatile sig_atomic_t s_restart = false;


static struct ConfData* SearchConfData(char *confName){

    struct ConfData *conf = confDataHead;

    while (conf != NULL) {
        if (strcmp(conf->confName, confName) == 0) {
            return conf;
        }
        conf = conf->next;
    }
    
    return NULL;
}


static QStatus AddConfData(char *confName) {

    struct ConfData *conf = new ConfData;

    conf->next = NULL;
    memset(conf->confName, 0, sizeof(conf->confName));
    strncpy(conf->confName, confName, CONFNAMELEN); 

    if (confDataTail == NULL) {
        confDataTail = conf;
        confDataHead = confDataTail;
    } else {
        confDataTail->next = conf;
        confDataTail = conf;
    }
    return ER_OK;
}

static void SigIntHandler(int sig) {
    s_interrupt = true;
}

static void daemonDisconnectCB()
{
    s_restart = true;
}

static void cleanup() {

    if (AboutObjApi::getInstance()) {
        AboutObjApi::DestroyInstance();
    }

    if (configService) {
        delete configService;
        configService = NULL;
    }

    if (configServiceListener) {
        delete configServiceListener;
        configServiceListener = NULL;
    }

    if (keyListener) {
        delete keyListener;
        keyListener = NULL;
    }

    if (busListener) {
        msgBus->UnregisterBusListener(*busListener);
        delete busListener;
        busListener = NULL;
    }

    if (aboutIconObj) {
        delete aboutIconObj;
        aboutIconObj = NULL;
    }

    if (icon) {
        delete icon;
        icon = NULL;
    }

    if (aboutDataStore) {
        delete aboutDataStore;
        aboutDataStore = NULL;
    }

    if (aboutObj) {
        delete aboutObj;
        aboutObj = NULL;
    }
    /* Clean up msg bus */
    if (msgBus) {
        delete msgBus;
        msgBus = NULL;
    }
}

void readPassword(qcc::String& passCode) {

    ajn::MsgArg*argPasscode;
    char*tmp;
    aboutDataStore->GetField("Passcode", argPasscode);
    argPasscode->Get("s", &tmp);
    passCode = tmp;
    return;
}

void WaitForSigInt(void) {
    while (s_interrupt == false && s_restart == false) {
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100 * 1000);
#endif
    }
}

//int sqliteCallBack(void *

int main(int argc, char**argv, char**envArg) {
    QStatus status = ER_OK;
    std::cout << "AllJoyn Library version: " << ajn::GetVersion() << std::endl;
    std::cout << "AllJoyn Library build info: " << ajn::GetBuildInfo() << std::endl;
    QCC_SetLogLevels("ALLJOYN_ABOUT_SERVICE=7;");
    QCC_SetLogLevels("ALLJOYN_ABOUT_ICON_SERVICE=7;");
    QCC_SetDebugLevel(logModules::CONFIG_MODULE_LOG_NAME, logModules::ALL_LOG_LEVELS);

    OptParser opts(argc, argv);
    OptParser::ParseResultCode parseCode(opts.ParseResult());
    switch (parseCode) {
    case OptParser::PR_OK:
        break;

    case OptParser::PR_EXIT_NO_ERROR:
        return SERVICE_EXIT_OK;

    default:
        return SERVICE_OPTION_ERROR;
    }

    std::cout << "using port " << servicePort << std::endl;

    if (!opts.GetConfigFile().empty()) {
        std::cout << "using Config-file " << opts.GetConfigFile().c_str() << std::endl;
    }

    /* Install SIGINT handler so Ctrl + C deallocates memory properly */
    signal(SIGINT, SigIntHandler);

    //set Daemon password only for bundled app
    #ifdef QCC_USING_BD
    PasswordManager::SetCredentials("ALLJOYN_PIN_KEYX", "000000");
    #endif

start:
    std::cout << "Initializing application." << std::endl;



    /* Create message bus */
    keyListener = new SrpKeyXListener();
    keyListener->setPassCode(DEFAULTPASSCODE);
    keyListener->setGetPassCode(readPassword);

    /* Connect to the daemon */
    uint16_t retry = 0;
    do {
        msgBus = CommonSampleUtil::prepareBusAttachment(keyListener);
        if (msgBus == NULL) {
            std::cout << "Could not initialize BusAttachment. Retrying" << std::endl;
#ifdef _WIN32
            Sleep(1000);
#else
            sleep(1);
#endif
            retry++;
        }
    } while (msgBus == NULL && retry != 180 && !s_interrupt);

    if (msgBus == NULL) {
        std::cout << "Could not initialize BusAttachment." << std::endl;
        cleanup();
        return 1;
    }

    busListener = new CommonBusListener(msgBus, daemonDisconnectCB);
    busListener->setSessionPort(servicePort);


#if 0
    //create UDP socket and bind address
    int socketFd;
    int ret = ER_OK;
    socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketFd == -1) {
            std::cout << "socket error\r\n" << std::endl;
            return 1;
    }
    std::cout << "create socket ok!" <<std::endl;

    struct sockaddr_in addr;

    const int DEV_ONLINE_PORT = 6666;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(DEV_ONLINE_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(socketFd, (sockaddr*)&addr, sizeof(addr));
    if (ret == -1){  
            std::cout << "bind error \r\n" << std::endl;  
            return 1;  
    }  
    std::cout << "bind to port: " << DEV_ONLINE_PORT << std::endl;

    sqlite3 *db;

    /*create table and save the info */
    ret = sqlite3_open("device.db", &db);
    if (ret != ER_OK) {
            std::cout << "sqlite3_open error !" << std::endl;
            sqlite3_close(db);
            return 1;
    }
    
    std::cout << "sqlite open succ" << std::endl;

    /* create table */ 
    const char *sqlCreateTable = "CREATE TABLE deviceinfo(interface TEXT PRIMARY KEY ASC, \
                                  objpath TEXT, platformid TEXT, factory TEXT, devicetype TEXT, deviceid TEXT, STATUS TEXT)";

    char *errMsg = NULL;

    ret = sqlite3_exec(db, sqlCreateTable, NULL, 0, &errMsg);
    if (ret != ER_OK) {
            std::cout << "create table deviceinfo error" << errMsg << std::endl;
            sqlite3_free(errMsg);
            return 1;
    }
    
    std::cout << "sqlite create table deviceinfo succ" << std::endl;
#endif

    while (s_interrupt == false && s_restart == false) {
        
        const int RECVBUF_LEN = 512;
        char recv_buf[RECVBUF_LEN];
        const int SENDBUF_LEN = 512;
        char send_buf[SENDBUF_LEN];
        char *pbuf = NULL;

#if 0
        memset(recv_buf, 0, RECVBUF_LEN);
        struct sockaddr_in client;
        unsigned int addr_len = sizeof(client);
        ret = recvfrom(socketFd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr*)&client, &addr_len);
        if (ret == -1) {
            std::cout << "recvfrom error \r\n" << std::endl;
        }

        std::cout << "recv from client len: " << addr_len << "addr: " << inet_ntoa(client.sin_addr) << "msg: "<< recv_buf << std::endl;

        
        strcpy(recv_buf, "hello factory:TY devtype:led devid:334455");
        pbuf = recv_buf;
        if (strncmp("hello", recv_buf, ONLINE_MSG_HEAD_LEN) == 0){ 

            pbuf += ONLINE_MSG_HEAD_LEN + 1;

            char factory[FACTORY_LEN + 1];
            if (strncmp("factory:", pbuf, ONLINE_MSG_FACTORY_LEN) == 0) {
                pbuf += ONLINE_MSG_FACTORY_LEN; 
                memcpy(factory, pbuf, FACTORY_LEN);
                factory[FACTORY_LEN] = '\0';
                pbuf += FACTORY_LEN + 1;
            }

            char dev_type[DEV_TYPE_LEN + 1];
            if (strncmp("devtype:", pbuf, ONLINE_MSG_DEV_TYPE_LEN) == 0) {
                pbuf += ONLINE_MSG_DEV_TYPE_LEN;

                memcpy(dev_type, pbuf, DEV_TYPE_LEN);
                dev_type[DEV_TYPE_LEN] = '\0';
                pbuf += DEV_TYPE_LEN + 1;
            }

            char dev_id[DEV_ID_LEN + 1];
            if (strncmp("devid:", pbuf, ONLINE_DEV_ID_LEN) == 0) {
                pbuf += ONLINE_DEV_ID_LEN;

                memcpy(dev_id, pbuf, DEV_ID_LEN);
                dev_id[DEV_ID_LEN] = '\0';
            }
#endif

            //aboutDataStore = new AboutDataStore(opts.GetFactoryConfigFile().c_str(), opts.GetConfigFile().c_str());
            char factoryConfigFileName[32] = "FactoryBt.conf";
            char configFileName[16] = "Bt.conf";
#if 0
            memset(factoryConfigFileName, 0 , sizeof(factoryConfigFileName));
            memset(configFileName, 0, sizeof(configFileName));
            sprintf(factoryConfigFileName, "%s%s.conf", factory, dev_type);
            sprintf(configFileName, "%s%s%s.conf", factory, dev_type, dev_id);

            std::cout << "using factory config: " << factoryConfigFileName << std::endl;
            std::cout << "using config: " << configFileName << std::endl; 
#endif

            qcc::String factoryConfigFile(factoryConfigFileName); 
            qcc::String configFile(configFileName); 
            
            /* if device have been online, don't setup again */
            if (SearchConfData(configFileName) != NULL)
                continue;
    
            aboutDataStore = new AboutDataStore(factoryConfigFile.c_str(), configFile.c_str());
            aboutDataStore->Initialize();
            if (!opts.GetAppId().empty()) {
                std::cout << "using appID " << opts.GetAppId().c_str() << std::endl;
                aboutDataStore->SetAppId(opts.GetAppId().c_str());
            }

            if (status != ER_OK) {
                std::cout << "Could not fill aboutDataStore." << std::endl;
                cleanup();
                return 1;
            }
            aboutObj = new ajn::AboutObj(*msgBus, BusObject::ANNOUNCED);
            status = CommonSampleUtil::prepareAboutService(msgBus, dynamic_cast<AboutData*>(aboutDataStore), aboutObj, busListener, servicePort);
            if (status != ER_OK) {
                std::cout << "Could not set up the AboutService." << std::endl;
                cleanup();
                return 1;
            }

            AboutObjApi* aboutObjApi = AboutObjApi::getInstance();
            if (!aboutObjApi) {
                std::cout << "Could not set up the AboutService." << std::endl;
                cleanup();
                return 1;
            }

            ////////////////////////////////////////////////////////////////////////////////////////////////////
            //aboutIconService

            uint8_t aboutIconContent[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x0A,
                0x00, 0x00, 0x00, 0x0A, 0x08, 0x02, 0x00, 0x00, 0x00, 0x02, 0x50, 0x58, 0xEA, 0x00, 0x00, 0x00, 0x04, 0x67, 0x41, 0x4D, 0x41, 0x00, 0x00, 0xAF,
                0xC8, 0x37, 0x05, 0x8A, 0xE9, 0x00, 0x00, 0x00, 0x19, 0x74, 0x45, 0x58, 0x74, 0x53, 0x6F, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x00, 0x41, 0x64,
                0x6F, 0x62, 0x65, 0x20, 0x49, 0x6D, 0x61, 0x67, 0x65, 0x52, 0x65, 0x61, 0x64, 0x79, 0x71, 0xC9, 0x65, 0x3C, 0x00, 0x00, 0x00, 0x18, 0x49, 0x44,
                0x41, 0x54, 0x78, 0xDA, 0x62, 0xFC, 0x3F, 0x95, 0x9F, 0x01, 0x37, 0x60, 0x62, 0xC0, 0x0B, 0x46, 0xAA, 0x34, 0x40, 0x80, 0x01, 0x00, 0x06, 0x7C,
                0x01, 0xB7, 0xED, 0x4B, 0x53, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82 };

            qcc::String mimeType("image/png");
            icon = new ajn::AboutIcon();
            status = icon->SetContent(mimeType.c_str(), aboutIconContent, sizeof(aboutIconContent) / sizeof(*aboutIconContent));
            if (ER_OK != status) {
                printf("Failed to setup the AboutIcon.\n");
            }

            aboutIconObj = new ajn::AboutIconObj(*msgBus, *icon);
            const int IF_MAXLEN = 128;
            const int OBJPATH_MAXLEN = 128;
            char ifName[IF_MAXLEN] = "org.alljoyn.001A7DDA7111.Config";
            char objectPath[OBJPATH_MAXLEN] = "/001A7DDA7111/Config";
#if 0
            memset(ifName, 0, sizeof(ifName));
            memset(objectPath, 0, sizeof(objectPath));
            sprintf(ifName, "org.alljoyn.%u.Config", client.sin_addr.s_addr); 
            sprintf(objectPath, "/%u/Config", client.sin_addr.s_addr);
#endif

            ////////////////////////////////////////////////////////////////////////////////////////////////////
            //ConfigService
            std::cout << "interface: " << ifName << std::endl;
            std::cout << "object path :" << objectPath << std::endl;

            configServiceListener = new ConfigServiceListenerImpl(*aboutDataStore, *msgBus, *busListener);
            configService = new ConfigService(*msgBus, *aboutDataStore, *configServiceListener, objectPath);

            status = configService->Register(ifName);
            if (status != ER_OK) {
                std::cout << "Could not register the ConfigService." << std::endl;
                cleanup();
                return 1;
            }

            status = msgBus->RegisterBusObject(*configService);
            if (status != ER_OK) {
                std::cout << "Could not register the ConfigService BusObject." << std::endl;
                cleanup();
                return 1;
            }
        
            std::cout << "register object succ " << std::endl;

            InterfaceDescription* intf = const_cast<InterfaceDescription*>(msgBus->GetInterface(ifName));
#if 0
            std::cout << "set description socket" << std::endl;
            //CHECK_RETURN(intf->SetDescriptionSocket(socketFd));
            std::cout << "set description socket succ" << std::endl;
            std::cout << "set description client addr" << std::endl; 
            //CHECK_RETURN(intf->SetDescriptionClientAddr((struct sockaddr *)&client));
             
            std::cout << "set description client addr succ" << std::endl; 
#endif

            ////////////////////////////////////////////////////////////////////////////////////////////////////
            if (ER_OK == status) {
                status = aboutObjApi->Announce();
            }
            std::cout << "Announce succ" << std::endl;
            
      
#if 0
            /* save info */
            const int SQL_CMD_LEN = 128;
            char sqlCmd[SQL_CMD_LEN];
            
            memset(sqlCmd, 0, sizeof(sqlCmd));
            sprintf(sqlCmd, "INSERT INTO deviceinfo VALUES(\"%s\", \"%s\", \"\", \"%s\", \"%s\", \"%s\", \"on\")", 
                    ifName, objectPath, factory, dev_type, dev_id);
            
            ret = sqlite3_exec(db, sqlCmd, NULL/*sqliteCallback*/, 0, &errMsg);
            if (ret != ER_OK) {
                std::cout << "insert value error" << errMsg << std::endl;
                sqlite3_free(errMsg);
                return 1;
            }
            else
            {
                std::cout << "insert succeed" << std::endl;
            }

            sqlite3_close(db);
#endif
            
            AddConfData(configFileName);

#if 0
        } else if (strncmp("report", recv_buf, REPORT_MSG_LEN) == 0) {

        }
#endif

        usleep(10 *1000);
    }

#if 0
    /* Perform the service asynchronously until the user signals for an exit. */
    if (ER_OK == status) {
        WaitForSigInt();
    }
#endif
    cleanup();

    if (s_restart) {
        s_restart = false;
        goto start;
    }

    return 0;
} /* main() */

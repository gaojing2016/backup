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

/* notification add by gaojing start */
#include <iostream>
#include <sstream>
#include <cstdio>
#include <alljoyn/PasswordManager.h>
#include <alljoyn/notification/NotificationService.h>
#include <alljoyn/notification/NotificationSender.h>
#include <alljoyn/notification/NotificationEnums.h>
#include <alljoyn/notification/NotificationText.h>
#include <alljoyn/notification/RichAudioUrl.h>
#include <alljoyn/notification/Notification.h>
#include <alljoyn/services_common/GuidUtil.h>

#define SERVICE_PORT 900

using namespace ajn;
using namespace services;
using namespace qcc;

NotificationService* prodService = 0;
NotificationSender* Sender = 0;

/* notification add by gaojing end */


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

    // Initialize Service object
    prodService = NotificationService::getInstance(); //notification add by gaojing


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

    /* gaojing: need to add smart device online event here */

    int ret = ER_OK;
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
    /* gaojing: need to check if the table deviceinfo is already existed */
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

    while (s_interrupt == false && s_restart == false) {
        
            //aboutDataStore = new AboutDataStore(opts.GetFactoryConfigFile().c_str(), opts.GetConfigFile().c_str());
            char factoryConfigFileName[32] = "FactoryBt.conf";
            char configFileName[16] = "Bt.conf";

            //std::cout << "using factory config: " << factoryConfigFileName << std::endl;
            //std::cout << "using config: " << configFileName << std::endl; 

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
            
            /* notification add by gaojing start */
            NotificationMessageType messageType = NotificationMessageType(INFO);

            Sender = prodService->initSend(msgBus, dynamic_cast<AboutData*>(aboutDataStore));
            NotificationText textToSend1("en", "The fridge door is open");
            //NotificationText textToSend2("de", "Die Kuhlschranktur steht offen");
            std::cout << "000000000000000000000000000000000000000000000000000000000000000 prepare notification" << std::endl;

            std::vector<NotificationText> vecMessages;
            vecMessages.push_back(textToSend1);
            //vecMessages.push_back(textToSend2);
            Notification notification(messageType, vecMessages);
            //notification.setRichIconUrl("http://iconUrl.com/notification.jpeg");
            std::cout << "000000000000000000000000000000000000000000000000000000000000000 sending notification" << std::endl;
            status = Sender->send(notification, 30);
            if (status != ER_OK) {
                std::cout << "Could not send the message successfully" << std::endl;
            } else {
                std::cout << "Notification sent with ttl of " << std::endl;
            }
            /* notification add by gaojing end */
            
            /* notification add by gaojing start */
            NotificationMessageType messageType1 = NotificationMessageType(INFO);

            //NotificationText textToSend11("en", "The fridge door is open");
            //char * text = "{\"type\":\"set\", \"serial\":100, \"user_id\":\"testaccount\", \"device_id\":\"bluetooth_gh_99f35e5ef2ff_190e44bbd5e76a4c\",\"device_type\": \"led\",\"config\": {\"power_switch\": \"on\", \"color_rgb\": \"30\"}}" ; 

            NotificationText textToSend11("en", "{\"type\":\"set\", \"serial\":100, \"user_id\":\"testaccount\", \"device_id\":\"bluetooth_gh_99f35e5ef2ff_190e44bbd5e76a4c\",\"device_type\": \"led\",\"config\": {\"power_switch\": \"on\", \"color_rgb\": \"30\"}}");
            //NotificationText textToSend12("de", "Die Kuhlschranktur steht offen");
            std::cout << "000000000000000000000000000000000000000000000000000000000000000 prepare notification" << std::endl;

            std::vector<NotificationText> vecMessages1;
            vecMessages1.push_back(textToSend11);
            //vecMessages1.push_back(textToSend12);
            Notification notification1(messageType1, vecMessages1);
            //notification.setRichIconUrl("http://iconUrl.com/notification.jpeg");
            std::cout << "000000000000000000000000000000000000000000000000000000000000000 sending notification" << std::endl;
            status = Sender->send(notification1, 30);
            if (status != ER_OK) {
                std::cout << "Could not send the message successfully" << std::endl;
            } else {
                std::cout << "Notification sent with ttl of " << std::endl;
            }
            /* notification add by gaojing end */



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

            /* gaojing: need to define smart device's interfaceName and objectPath*/
            const int IF_MAXLEN = 128;
            const int OBJPATH_MAXLEN = 128;
            char ifName[IF_MAXLEN] = "org.alljoyn.001A7DDA7111.Config";
            char objectPath[OBJPATH_MAXLEN] = "/001A7DDA7111/Config";

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

            ////////////////////////////////////////////////////////////////////////////////////////////////////
            if (ER_OK == status) {
                status = aboutObjApi->Announce();
            }
            std::cout << "Announce succ" << std::endl;


            /* save info */
            /* gaojing: need to get smart device info such as factory, dev_type and dev_id */
            const int SQL_CMD_LEN = 128;
            char sqlCmd[SQL_CMD_LEN];
            char factory[FACTORY_LEN + 1] = "TY";
            char dev_type[DEV_TYPE_LEN + 1] = "led";
            char dev_id[64] = "bluetooth_gh_99f35e5ef2ff_190e44bbd5e76a4c";
            
            memset(sqlCmd, 0, sizeof(sqlCmd));
            sprintf(sqlCmd, "INSERT INTO deviceinfo VALUES(\"%s\", \"%s\", \"\", \"%s\", \"%s\", \"%s\", \"off\")", 
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
            
            AddConfData(configFileName);

            /* notification add by gaojing end */
            sleep(40);
            std::cout << "11111111111111111111111111111111111111111111111111111111111 2nd send" << std::endl;
            status = Sender->send(notification, 30);
            if (status != ER_OK) {
                std::cout << "Could not send the message successfully" << std::endl;
            } else {
                std::cout << "Notification sent with ttl of " << std::endl;
            }
            /* notification add by gaojing end */


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
